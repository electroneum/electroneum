// Copyright (c) 2017-Present, Electroneum
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

#include "include_base_utils.h"
#include "file_io_utils.h"
#include "cryptonote_basic/blobdatatype.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "wallet/wallet2.h"
#include "fuzzer.h"

class SignatureFuzzer: public Fuzzer
{
public:
  SignatureFuzzer(): Fuzzer(), wallet(cryptonote::TESTNET) {}
  virtual int init();
  virtual int run(const std::string &filename);

private:
  tools::wallet2 wallet;
  cryptonote::account_public_address address;
};

int SignatureFuzzer::init()
{
  static const char * const spendkey_hex = "0b4f47697ec99c3de6579304e5f25c68b07afbe55b71d99620bf6cbf4e45a80f";
  crypto::secret_key spendkey;
  epee::string_tools::hex_to_pod(spendkey_hex, spendkey);

  try
  {
    wallet.init("", boost::none, boost::asio::ip::tcp::endpoint{}, 0, true, epee::net_utils::ssl_support_t::e_ssl_support_disabled);
    wallet.set_subaddress_lookahead(1, 1);
    wallet.generate("", "", spendkey, true, false);

    cryptonote::address_parse_info info;
    if (!cryptonote::get_account_address_from_str_or_url(info, cryptonote::TESTNET, "9uVsvEryzpN8WH2t1WWhFFCG5tS8cBNdmJYNRuckLENFimfauV5pZKeS1P2CbxGkSDTUPHXWwiYE5ZGSXDAGbaZgDxobqDN"))
    {
      std::cerr << "failed to parse address" << std::endl;
      return 1;
    }
    address = info.address;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error on SignatureFuzzer::init: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

int SignatureFuzzer::run(const std::string &filename)
{
  std::string s;

  if (!epee::file_io_utils::load_file_to_string(filename, s))
  {
    std::cout << "Error: failed to load file " << filename << std::endl;
    return 1;
  }

  bool valid = wallet.verify("test", address, s);
  std::cout << "Signature " << (valid ? "valid" : "invalid") << std::endl;

  return 0;
}

int main(int argc, const char **argv)
{
  TRY_ENTRY();
  SignatureFuzzer fuzzer;
  return run_fuzzer(argc, argv, fuzzer);
  CATCH_ENTRY_L0("main", 1);
}
