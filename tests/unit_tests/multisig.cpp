// Copyright (c) 2017-2019, The Monero Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gtest/gtest.h"

#include <cstdint>

#include "wallet/wallet2.h"

static const struct
{
  const char *address;
  const char *spendkey;
} test_addresses[] =
{
  {
    "etnkQfrWyE18qgir1RPR31Vcm8ibph8a4TArBzp9hx6qG5AtWJ4cg9BYcVmDkbLsqvag2xZuBVtzATMbGENvhbEr2GpNu7NkqX",
    "23a662c983674c3654d2f2d7c72e055523ef6bdb8bb8290d626268e73c5b4600"
  },
  {
    "etnkByEHsraTqRBdDrrV2m1kDCHa1NyBpDWJ1XeB5ZGRGtdbUcteBLwN9LBpiDF6EjDj8qEUFqasKC6RNwLHuUju7PCaanugxk",
    "77dbbfaa83fa79c9280340893dca48a868c23a1b9f8835ec421ce7281e882509"
  },
  {
    "etnkQNki9u1KyZvnh4WRDsLPQbZgcRo8WFo7JfAftDU7FZhyuxmxxHQMkkxLt8y7RwTKeH1hdMZ1V7ACPDm4HwEw1tLMWjW8Lv",
    "5f17f13c96c6fcc8ec3f9f539ea00e8087224cd9071fbecfec07441b9e64b60b"
  },
  {
    "etnk3G6WfJqgTMuuAZXcKxMmv7WpVGC977okK94spzsWc3acWLjebL1MdWN2uNqLygGEHCQWtzf81Phbz952LmK35vx7iuF2zV",
    "7083f2c357c0fc2f2fabc5d6751d989141aad8361899d427bbc0d03ca2f2850d"
  },
  {
    "etnkAo9GFotYKPmgKbEXBAHhvuwcUU1vDaedMf9TLMdQ3p7tiyE6mr1BNWtyrxDoGs9dPAJ9nKJ5jLt2Gk8oGcYE4JoSB2EreS",
    "4b754c17b2788826bf57eb028ac8af014c3a9c126a94920c1e00924a4a98cf0d"
  }
};

static const size_t KEYS_COUNT = 5;

static void make_wallet(unsigned int idx, tools::wallet2 &wallet)
{
  ASSERT_TRUE(idx < sizeof(test_addresses) / sizeof(test_addresses[0]));

  crypto::secret_key spendkey;
  epee::string_tools::hex_to_pod(test_addresses[idx].spendkey, spendkey);

  try
  {
    wallet.init("", boost::none, boost::asio::ip::tcp::endpoint{}, 0, true, epee::net_utils::ssl_support_t::e_ssl_support_disabled);
    wallet.set_subaddress_lookahead(1, 1);
    wallet.generate("", "", spendkey, true, false);
    ASSERT_TRUE(test_addresses[idx].address == wallet.get_account().get_public_address_str(cryptonote::TESTNET));
    wallet.decrypt_keys("");
    ASSERT_TRUE(test_addresses[idx].spendkey == epee::string_tools::pod_to_hex(wallet.get_account().get_keys().m_spend_secret_key));
    wallet.encrypt_keys("");
  }
  catch (const std::exception &e)
  {
    MFATAL("Error creating test wallet: " << e.what());
    ASSERT_TRUE(0);
  }
}

static std::vector<std::string> exchange_round(std::vector<tools::wallet2>& wallets, const std::vector<std::string>& mis)
{
  std::vector<std::string> new_infos;
  for (size_t i = 0; i < wallets.size(); ++i) {
      new_infos.push_back(wallets[i].exchange_multisig_keys("", mis));
  }

  return new_infos;
}

static void make_wallets(std::vector<tools::wallet2>& wallets, unsigned int M)
{
  ASSERT_TRUE(wallets.size() > 1 && wallets.size() <= KEYS_COUNT);
  ASSERT_TRUE(M <= wallets.size());

  std::vector<std::string> mis(wallets.size());

  for (size_t i = 0; i < wallets.size(); ++i) {
    make_wallet(i, wallets[i]);

    wallets[i].decrypt_keys("");
    mis[i] = wallets[i].get_multisig_info();
    wallets[i].encrypt_keys("");
  }

  for (auto& wallet: wallets) {
    ASSERT_FALSE(wallet.multisig());
  }

  std::vector<std::string> mxis;
  for (size_t i = 0; i < wallets.size(); ++i) {
    // it's ok to put all of multisig keys in this function. it throws in case of error
    mxis.push_back(wallets[i].make_multisig("", mis, M));
  }

  while (!mxis[0].empty()) {
    mxis = exchange_round(wallets, mxis);
  }

  for (size_t i = 0; i < wallets.size(); ++i) {
    ASSERT_TRUE(mxis[i].empty());
    bool ready;
    uint32_t threshold, total;
    ASSERT_TRUE(wallets[i].multisig(&ready, &threshold, &total));
    ASSERT_TRUE(ready);
    ASSERT_TRUE(threshold == M);
    ASSERT_TRUE(total == wallets.size());

    if (i != 0) {
      // "equals" is transitive relation so we need only to compare first wallet's address to each others' addresses. no need to compare 0's address with itself.
      ASSERT_TRUE(wallets[0].get_account().get_public_address_str(cryptonote::TESTNET) == wallets[i].get_account().get_public_address_str(cryptonote::TESTNET));
    }
  }
}

TEST(multisig, make_2_2)
{
  std::vector<tools::wallet2> wallets(2);
  make_wallets(wallets, 2);
}

TEST(multisig, make_3_3)
{
  std::vector<tools::wallet2> wallets(3);
  make_wallets(wallets, 3);
}

TEST(multisig, make_2_3)
{
  std::vector<tools::wallet2> wallets(3);
  make_wallets(wallets, 2);
}

TEST(multisig, make_2_4)
{
  std::vector<tools::wallet2> wallets(4);
  make_wallets(wallets, 2);
}

TEST(multisig, make_2_5)
{
  std::vector<tools::wallet2> wallets(5);
  make_wallets(wallets, 2);
}
