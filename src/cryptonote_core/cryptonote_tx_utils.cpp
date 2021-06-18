// Copyrights(c) 2017-2021, The Electroneum Project
// Copyrights(c) 2014-2019, The Monero Project
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
// 
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#include <unordered_set>
#include <random>
#include "include_base_utils.h"
#include "string_tools.h"
using namespace epee;

#include "common/apply_permutation.h"
#include "cryptonote_tx_utils.h"
#include "cryptonote_config.h"
#include "cryptonote_basic/miner.h"
#include "cryptonote_basic/tx_extra.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "ringct/rctSigs.h"
#include "multisig/multisig.h"

using namespace crypto;

namespace cryptonote
{
  //---------------------------------------------------------------
  void classify_addresses(const std::vector<tx_destination_entry> &destinations, const boost::optional<cryptonote::account_public_address>& change_addr, size_t &num_stdaddresses, size_t &num_subaddresses, account_public_address &single_dest_subaddress)
  {
    num_stdaddresses = 0;
    num_subaddresses = 0;
    std::unordered_set<cryptonote::account_public_address> unique_dst_addresses;
    for(const tx_destination_entry& dst_entr: destinations)
    {
      if (change_addr && dst_entr.addr == change_addr)
        continue;
      if (unique_dst_addresses.count(dst_entr.addr) == 0)
      {
        unique_dst_addresses.insert(dst_entr.addr);
        if (dst_entr.is_subaddress)
        {
          ++num_subaddresses;
          single_dest_subaddress = dst_entr.addr;
        }
        else
        {
          ++num_stdaddresses;
        }
      }
    }
    LOG_PRINT_L2("destinations include " << num_stdaddresses << " standard addresses and " << num_subaddresses << " subaddresses");
  }
  //---------------------------------------------------------------
  bool construct_miner_tx(size_t height, size_t median_weight, uint64_t already_generated_coins, size_t current_block_weight, uint64_t fee, const account_public_address &miner_address, transaction& tx, const blobdata& extra_nonce, size_t max_outs, uint8_t hard_fork_version, network_type nettype) {
    tx.vin.clear();
    tx.vout.clear();
    tx.extra.clear();

    txin_gen in;
    in.height = height;

    uint64_t block_reward;
    if(!get_block_reward(median_weight, current_block_weight, already_generated_coins, block_reward, hard_fork_version, height, nettype))
    {
      LOG_PRINT_L0("Block is too big");
      return false;
    }

#if defined(DEBUG_CREATE_BLOCK_TEMPLATE)
    LOG_PRINT_L1("Creating block template: reward " << block_reward <<
      ", fee " << fee);
#endif
    block_reward += fee;

    // from hard fork 2, we cut out the low significant digits. This makes the tx smaller, and
    // keeps the paid amount almost the same. The unpaid remainder gets pushed back to the
    // emission schedule
    // from hard fork 4, we use a single "dusty" output. This makes the tx even smaller,
    // and avoids the quantization. These outputs will be added as rct outputs with identity
    // masks, to they can be used as rct inputs.
    if (hard_fork_version >= 2 && hard_fork_version < 4) {
      block_reward = block_reward - block_reward % ::config::BASE_REWARD_CLAMP_THRESHOLD;
    }

    std::vector<uint64_t> out_amounts;
    decompose_amount_into_digits(block_reward, hard_fork_version >= 2 ? 0 : ::config::DEFAULT_DUST_THRESHOLD,
      [&out_amounts](uint64_t a_chunk) { out_amounts.push_back(a_chunk); },
      [&out_amounts](uint64_t a_dust) { out_amounts.push_back(a_dust); });

    CHECK_AND_ASSERT_MES(1 <= max_outs, false, "max_out must be non-zero");
    if (height == 0 || hard_fork_version >= 4)
    {
      // the genesis block was not decomposed, for unknown reasons
      while (max_outs < out_amounts.size())
      {
        //out_amounts[out_amounts.size() - 2] += out_amounts.back();
        //out_amounts.resize(out_amounts.size() - 1);
        out_amounts[1] += out_amounts[0];
        for (size_t n = 1; n < out_amounts.size(); ++n)
          out_amounts[n - 1] = out_amounts[n];
        out_amounts.pop_back();
      }
    }
    else
    {
      CHECK_AND_ASSERT_MES(max_outs >= out_amounts.size(), false, "max_out exceeded");
    }

    uint64_t summary_amounts = 0;
    if (hard_fork_version >= HF_VERSION_PUBLIC_TX)
    {
      for (const auto out_amount : out_amounts)
      {
        txout_to_key_public tk;
        tk.address.m_view_public_key = miner_address.m_view_public_key;
        tk.address.m_spend_public_key = miner_address.m_spend_public_key;
        tk.m_address_prefix = get_config(nettype).CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;

        tx_out out;
        summary_amounts += out.amount = out_amount;
        out.target = tk;
        tx.vout.push_back(out);
      }

      tx.version = 3;
    }
    else
    {
      keypair txkey = keypair::generate(hw::get_device("default"));
      add_tx_pub_key_to_extra(tx, txkey.pub);
      if(!extra_nonce.empty())
        if(!add_extra_nonce_to_tx_extra(tx.extra, extra_nonce))
          return false;
      if (!sort_tx_extra(tx.extra, tx.extra))
        return false;

      for (size_t no = 0; no < out_amounts.size(); no++)
      {
        crypto::key_derivation derivation = AUTO_VAL_INIT(derivation);;
        crypto::public_key out_eph_public_key = AUTO_VAL_INIT(out_eph_public_key);
        bool r = crypto::generate_key_derivation(miner_address.m_view_public_key, txkey.sec, derivation);
        CHECK_AND_ASSERT_MES(r, false, "while creating outs: failed to generate_key_derivation(" << miner_address.m_view_public_key << ", " << txkey.sec << ")");

        r = crypto::derive_public_key(derivation, no, miner_address.m_spend_public_key, out_eph_public_key);
        CHECK_AND_ASSERT_MES(r, false, "while creating outs: failed to derive_public_key(" << derivation << ", " << no << ", "<< miner_address.m_spend_public_key << ")");

        txout_to_key tk;
        tk.key = out_eph_public_key;

        tx_out out;
        summary_amounts += out.amount = out_amounts[no];
        out.target = tk;
        tx.vout.push_back(out);
      }

      tx.version = 1;
    }


    CHECK_AND_ASSERT_MES(summary_amounts == block_reward, false, "Failed to construct miner tx, summary_amounts = " << summary_amounts << " not equal block_reward = " << block_reward);

    //lock
    tx.unlock_time = height + (hard_fork_version > 7 ? ETN_MINED_ETN_UNLOCK_WINDOW_V8 : CRYPTONOTE_MINED_ETN_UNLOCK_WINDOW);
    tx.vin.push_back(in);

    tx.invalidate_hashes();

    //LOG_PRINT("MINER_TX generated ok, block_reward=" << print_etn(block_reward) << "("  << print_etn(block_reward - fee) << "+" << print_etn(fee)
    //  << "), current_block_size=" << current_block_size << ", already_generated_coins=" << already_generated_coins << ", tx_id=" << get_transaction_hash(tx), LOG_LEVEL_2);
    return true;
  }
  //---------------------------------------------------------------
  crypto::public_key get_destination_view_key_pub(const std::vector<tx_destination_entry> &destinations, const boost::optional<cryptonote::account_public_address>& change_addr)
  {
    account_public_address addr = {null_pkey, null_pkey};
    size_t count = 0;
    for (const auto &i : destinations)
    {
      if (i.amount == 0)
        continue;
      if (change_addr && i.addr == *change_addr)
        continue;
      if (i.addr == addr)
        continue;
      if (count > 0)
        return null_pkey;
      addr = i.addr;
      ++count;
    }
    if (count == 0 && change_addr)
      return change_addr->m_view_public_key;
    return addr.m_view_public_key;
  }
  //---------------------------------------------------------------
  bool construct_tx_with_tx_key(const account_keys& sender_account_keys, const std::unordered_map<crypto::public_key, subaddress_index>& subaddresses, std::vector<tx_source_entry>& sources, std::vector<tx_destination_entry>& destinations, const boost::optional<cryptonote::account_public_address>& change_addr, const std::vector<uint8_t> &extra, transaction& tx, uint64_t unlock_time, const crypto::secret_key &tx_key, const std::vector<crypto::secret_key> &additional_tx_keys, bool rct, const rct::RCTConfig &rct_config, rct::multisig_out *msout, bool shuffle_outs, const uint32_t account_major_offset, const cryptonote::network_type nettype)
  {
    hw::device &hwdev = sender_account_keys.get_device();

    if (sources.empty())
    {
      LOG_ERROR("Empty sources");
      return false;
    }

    std::vector<rct::key> amount_keys;
    //test tx is sent through this scope TWICE, once as a test and once for real... therefore we set null before each run.
    tx.set_null_besides_version();
    amount_keys.clear();
    if (msout)
    {
      msout->c.clear();
    }

    tx.unlock_time = unlock_time;

    tx.extra = extra;
    crypto::public_key txkey_pub;

    // if we have a stealth payment id, find it and encrypt it with the tx key now
    std::vector<tx_extra_field> tx_extra_fields;
    if (parse_tx_extra(tx.extra, tx_extra_fields))
    {
      bool add_dummy_payment_id = true;
      tx_extra_nonce extra_nonce;
      if (find_tx_extra_field_by_type(tx_extra_fields, extra_nonce))
      {
        crypto::hash payment_id = null_hash;
        crypto::hash8 payment_id8 = null_hash8;
        if (get_encrypted_payment_id_from_tx_extra_nonce(extra_nonce.nonce, payment_id8))
        {
          LOG_PRINT_L2("Adding cleartext payment ID to extra nonce. Encrypted PIDs are now deprecated." << payment_id8);

          std::string extra_nonce;

          memcpy(payment_id.data, payment_id8.data, 8); // convert short pid to regular
          memset(payment_id.data + 8, 0, 24); // merely a sanity check

          set_payment_id_to_tx_extra_nonce(extra_nonce, payment_id);
          remove_field_from_tx_extra(tx.extra, typeid(tx_extra_nonce));
          if (!add_extra_nonce_to_tx_extra(tx.extra, extra_nonce))
          {
            LOG_ERROR("Failed to add payment id to tx extra");
            return false;
          }
          LOG_PRINT_L1("Encrypted payment ID: " << payment_id8);
          add_dummy_payment_id = false;
        }
        else if (get_payment_id_from_tx_extra_nonce(extra_nonce.nonce, payment_id))
        {
          add_dummy_payment_id = false;
        }
      }

      // we don't add one if we've got more than the usual 1 destination plus change
      if (destinations.size() > 2)
        add_dummy_payment_id = false;

      if (add_dummy_payment_id)
      {
        std::string extra_nonce;
        crypto::hash payment_id = null_hash;

        set_payment_id_to_tx_extra_nonce(extra_nonce, payment_id);
        if (!add_extra_nonce_to_tx_extra(tx.extra, extra_nonce))
        {
          LOG_ERROR("Failed to add dummy payment id to tx extra");
          // continue anyway
        }
      }
    }
    else
    {
      MWARNING("Failed to parse tx extra");
      tx_extra_fields.clear();
    }

    struct input_generation_context_data
    {
      keypair in_ephemeral;
    };
    std::vector<input_generation_context_data> in_contexts;

    uint64_t summary_inputs_etn = 0;
    //fill inputs
    int idx = -1;

    if(tx.version < 3) {
        for (const tx_source_entry &src_entr : sources) {
            ++idx;
            if (src_entr.real_output >= src_entr.outputs.size()) {//
                LOG_ERROR("real_output index (" << src_entr.real_output << ")bigger than output_keys.size()="
                                                << src_entr.outputs.size());
                return false;
            }
            summary_inputs_etn += src_entr.amount;

            //key_derivation recv_derivation;
            in_contexts.push_back(input_generation_context_data());
            // Tx output private key which gets its value assigned inside generate_key_image_helper
            keypair &in_ephemeral = in_contexts.back().in_ephemeral;
            crypto::key_image img;//
            const auto &out_key = reinterpret_cast<const crypto::public_key &>(src_entr.outputs[src_entr.real_output].second.dest);
            if (!generate_key_image_helper(sender_account_keys, subaddresses, out_key, src_entr.real_out_tx_key,
                                           src_entr.real_out_additional_tx_keys, src_entr.real_output_in_tx_index,
                                           in_ephemeral, img, hwdev, account_major_offset)) {
                LOG_ERROR("Key image generation failed!");
                return false;
            }

            //check that derivated key is equal with real output key (if non multisig)
            if (!msout && !(in_ephemeral.pub == src_entr.outputs[src_entr.real_output].second.dest)) {
                LOG_ERROR("derived public key mismatch with output public key at index "
                            << idx << ", real out "
                            << src_entr.real_output << "! "
                            << ENDL << "derived_key:"
                            << string_tools::pod_to_hex(in_ephemeral.pub)
                            << ENDL
                            << "real output_public_key:"
                            << string_tools::pod_to_hex(src_entr.outputs[src_entr.real_output].second.dest));

                LOG_ERROR("amount " << src_entr.amount << ", rct " << src_entr.rct);
                LOG_ERROR("tx pubkey " << src_entr.real_out_tx_key << ", real_output_in_tx_index "
                                       << src_entr.real_output_in_tx_index);
                return false;
            }

            //put key image into tx input
            txin_to_key input_to_key;
            input_to_key.amount = src_entr.amount;
            input_to_key.k_image = msout ? rct::rct2ki(src_entr.multisig_kLRki.ki) : img;

            //fill outputs array and use relative offsets
            for (const tx_source_entry::output_entry &out_entry: src_entr.outputs)
                input_to_key.key_offsets.push_back(out_entry.first);

            input_to_key.key_offsets = absolute_output_offsets_to_relative(input_to_key.key_offsets);
            tx.vin.push_back(input_to_key);
        }

        // sort ins by their key image
        std::vector<size_t> ins_order(sources.size());
        for (size_t n = 0; n < sources.size(); ++n)
            ins_order[n] = n;
        std::sort(ins_order.begin(), ins_order.end(), [&](const size_t i0, const size_t i1) {
            const txin_to_key &tk0 = boost::get<txin_to_key>(tx.vin[i0]);
            const txin_to_key &tk1 = boost::get<txin_to_key>(tx.vin[i1]);
            return memcmp(&tk0.k_image, &tk1.k_image, sizeof(tk0.k_image)) > 0;
        });
        tools::apply_permutation(ins_order, [&] (size_t i0, size_t i1) {
            std::swap(tx.vin[i0], tx.vin[i1]);
            std::swap(in_contexts[i0], in_contexts[i1]);
            std::swap(sources[i0], sources[i1]);
        });

        if (shuffle_outs)
        {
            std::shuffle(destinations.begin(), destinations.end(), std::default_random_engine(crypto::rand<unsigned int>()));
        }
    } else{ // tx v3 onwards
        for (const tx_source_entry &src_entr : sources) {
            ++idx;
            summary_inputs_etn += src_entr.amount;

            txin_to_key_public input;
            input.amount = src_entr.amount;
            input.tx_hash = src_entr.tx_hash;
            input.relative_offset = src_entr.real_output_in_tx_index;

            tx.vin.push_back(input);
        }
    } //END OF WORK WITH INPUTS

    // figure out if we need to make additional tx pubkeys
    size_t num_stdaddresses = 0;
    size_t num_subaddresses = 0;
    account_public_address single_dest_subaddress;
    classify_addresses(destinations, change_addr, num_stdaddresses, num_subaddresses, single_dest_subaddress);

    // if this is a single-destination transfer to a subaddress, we set the tx pubkey to R=s*D
    //todo: 4.0.0.0
    if (num_stdaddresses == 0 && num_subaddresses == 1)
    {
      txkey_pub = rct::rct2pk(hwdev.scalarmultKey(rct::pk2rct(single_dest_subaddress.m_spend_public_key), rct::sk2rct(tx_key)));
    }
    else
    {
      txkey_pub = rct::rct2pk(hwdev.scalarmultBase(rct::sk2rct(tx_key)));
    }
    remove_field_from_tx_extra(tx.extra, typeid(tx_extra_pub_key));
    add_tx_pub_key_to_extra(tx, txkey_pub);

    std::vector<crypto::public_key> additional_tx_public_keys;

    // we don't need to include additional tx keys if:
    //   - all the destinations are standard addresses
    //   - there's only one destination which is a subaddress
    bool need_additional_txkeys = num_subaddresses > 0 && (num_stdaddresses > 0 || num_subaddresses > 1);
    if (need_additional_txkeys)
      CHECK_AND_ASSERT_MES(destinations.size() == additional_tx_keys.size(), false, "Wrong amount of additional tx keys");

    uint64_t summary_outs_etn = 0;
    //fill outputs
    size_t output_index = 0;
        for (const tx_destination_entry &dst_entr: destinations) {
            if(tx.version == 1) {
                crypto::public_key out_eph_public_key;

                hwdev.generate_output_ephemeral_keys(tx.version, sender_account_keys, txkey_pub, tx_key,
                                                     dst_entr, change_addr, output_index,
                                                     need_additional_txkeys, additional_tx_keys,
                                                     additional_tx_public_keys, amount_keys, out_eph_public_key);

                tx_out out;
                out.amount = dst_entr.amount;
                txout_to_key tk;
                tk.key = out_eph_public_key;
                out.target = tk;
                tx.vout.push_back(out);
                output_index++;
                summary_outs_etn += dst_entr.amount;
            }else{
                tx_out out;
                out.amount = dst_entr.amount;
                txout_to_key_public tkp;
                tkp.address.m_view_public_key = dst_entr.addr.m_view_public_key;
                tkp.address.m_spend_public_key = dst_entr.addr.m_spend_public_key;
                tkp.m_address_prefix = dst_entr.is_subaddress ?
                        get_config(nettype).CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX :
                                       get_config(nettype).CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
                out.target = tkp;
                tx.vout.push_back(out);
                output_index++;
                summary_outs_etn += dst_entr.amount;
            }
        }

    CHECK_AND_ASSERT_MES(additional_tx_public_keys.size() == additional_tx_keys.size(), false, "Internal error creating additional public keys");

    remove_field_from_tx_extra(tx.extra, typeid(tx_extra_additional_pub_keys));

    LOG_PRINT_L2("tx pubkey: " << txkey_pub);
    if (need_additional_txkeys)
    {
      LOG_PRINT_L2("additional tx pubkeys: ");
      for (size_t i = 0; i < additional_tx_public_keys.size(); ++i)
        LOG_PRINT_L2(additional_tx_public_keys[i]);
      add_additional_tx_pub_keys_to_extra(tx.extra, additional_tx_public_keys);
    }

    if (!sort_tx_extra(tx.extra, tx.extra))
      return false;

    //check etn
    if(summary_outs_etn > summary_inputs_etn )
    {
      LOG_ERROR("Transaction inputs ETN ("<< summary_inputs_etn << ") less than outputs ETN (" << summary_outs_etn << ")");
      return false;
    }

    // check for watch only wallet
    bool zero_secret_key = true;
    for (size_t i = 0; i < sizeof(sender_account_keys.m_spend_secret_key); ++i)
      zero_secret_key &= (sender_account_keys.m_spend_secret_key.data[i] == 0);
    if (zero_secret_key)
    {
      MDEBUG("Null secret key, skipping signatures");
    }

    //generate ring signatures
    crypto::hash tx_prefix_hash;
    hwdev.get_transaction_prefix_hash(tx, tx_prefix_hash);

    std::stringstream ss_ring_s;
    size_t i = 0;
    
    if(tx.version < 3)
    {
      for(const tx_source_entry& src_entr:  sources)
      {
        ss_ring_s << "pub_keys:" << ENDL;
        std::vector<const crypto::public_key *> keys_ptrs;
        std::vector<crypto::public_key> keys(src_entr.outputs.size());
        size_t ii = 0;
        for (const tx_source_entry::output_entry &o: src_entr.outputs) {
            keys[ii] = rct2pk(o.second.dest);
            keys_ptrs.push_back(&keys[ii]);
            ss_ring_s << o.second.dest << ENDL;
            ++ii;
        }

        tx.signatures.push_back(std::vector<crypto::signature>());
        std::vector<crypto::signature> &sigs = tx.signatures.back();
        sigs.resize(src_entr.outputs.size());
        if (!zero_secret_key)
            hwdev.generate_ring_signature(tx_prefix_hash, boost::get<txin_to_key>(tx.vin[i]).k_image, keys_ptrs,
                                          in_contexts[i].in_ephemeral.sec, src_entr.real_output, sigs.data());
        ss_ring_s << "signatures:" << ENDL;
        std::for_each(sigs.begin(), sigs.end(), [&](const crypto::signature &s) { ss_ring_s << s << ENDL; });
        ss_ring_s << "prefix_hash:" << tx_prefix_hash << ENDL << "in_ephemeral_key: "
                  << in_contexts[i].in_ephemeral.sec << ENDL << "real_output: " << src_entr.real_output << ENDL;
        i++;

        MCINFO("construct_tx",
              "transaction_created: " << get_transaction_hash(tx) << ENDL << obj_to_json_str(tx) << ENDL
                                      << ss_ring_s.str());
      }
    }
    else
    { //new public signatures for v3 onwards
      for(uint64_t i = 0; i< tx.vin.size(); i++) {
        crypto::signature signature;
        std::vector<crypto::signature> signature_vec;
        
        if (!zero_secret_key)
          hwdev.generate_input_signature(tx_prefix_hash, i, sender_account_keys.m_view_secret_key,
                                      sender_account_keys.m_spend_secret_key, signature);

        signature_vec.push_back(signature);
        tx.signatures.push_back(signature_vec);
      }

      MCINFO("construct_tx",
              "transaction_created (v3): " << get_transaction_hash(tx) << ENDL << obj_to_json_str(tx));
    }
    
    tx.invalidate_hashes();

    return true;
  }
  //---------------------------------------------------------------
  bool construct_tx_and_get_tx_key(const account_keys& sender_account_keys, const std::unordered_map<crypto::public_key, subaddress_index>& subaddresses, std::vector<tx_source_entry>& sources, std::vector<tx_destination_entry>& destinations, const boost::optional<cryptonote::account_public_address>& change_addr, const std::vector<uint8_t> &extra, transaction& tx, uint64_t unlock_time, crypto::secret_key &tx_key, std::vector<crypto::secret_key> &additional_tx_keys, bool rct, const rct::RCTConfig &rct_config, rct::multisig_out *msout, const uint32_t account_major_offset, const cryptonote::network_type nettype)
  {
    hw::device &hwdev = sender_account_keys.get_device();
    hwdev.open_tx(tx_key);
    try {
      // figure out if we need to make additional tx pubkeys
      size_t num_stdaddresses = 0;
      size_t num_subaddresses = 0;
      account_public_address single_dest_subaddress;
      classify_addresses(destinations, change_addr, num_stdaddresses, num_subaddresses, single_dest_subaddress);
      bool need_additional_txkeys = num_subaddresses > 0 && (num_stdaddresses > 0 || num_subaddresses > 1);
      if (need_additional_txkeys)
      {
        additional_tx_keys.clear();
        for (const auto &d: destinations)
          additional_tx_keys.push_back(keypair::generate(sender_account_keys.get_device()).sec);
      }

      bool r = construct_tx_with_tx_key(sender_account_keys, subaddresses, sources, destinations, change_addr, extra, tx, unlock_time, tx_key, additional_tx_keys, rct, rct_config, msout, true, account_major_offset, nettype);
      hwdev.close_tx();
      return r;
    } catch(...) {
      hwdev.close_tx();
      throw;
    }
  }
  //---------------------------------------------------------------
  // called by tests only (use hf_version 1 for the time being)
  bool construct_tx(const account_keys& sender_account_keys, std::vector<tx_source_entry>& sources, const std::vector<tx_destination_entry>& destinations, const boost::optional<cryptonote::account_public_address>& change_addr, const std::vector<uint8_t> &extra, transaction& tx, uint64_t unlock_time)
  {
     std::unordered_map<crypto::public_key, cryptonote::subaddress_index> subaddresses;
     subaddresses[sender_account_keys.m_account_address.m_spend_public_key] = {0,0};
     crypto::secret_key tx_key;
     std::vector<crypto::secret_key> additional_tx_keys;
     std::vector<tx_destination_entry> destinations_copy = destinations;
     return construct_tx_and_get_tx_key(sender_account_keys, subaddresses, sources, destinations_copy, change_addr, extra, tx, unlock_time, tx_key, additional_tx_keys, false, { rct::RangeProofBorromean, 0}, NULL);
  }
  //---------------------------------------------------------------
  bool generate_genesis_block(
      block& bl
    , std::string const & genesis_tx
    , uint32_t nonce
    )
  {
    //genesis block
    bl = boost::value_initialized<block>();


    account_public_address ac = boost::value_initialized<account_public_address>();
    std::vector<size_t> sz;
    construct_miner_tx(0, 0, 0, 0, 0, ac, bl.miner_tx); // zero fee in genesis
    blobdata txb = tx_to_blob(bl.miner_tx);
    std::string hex_tx_represent = string_tools::buff_to_hex_nodelimer(txb);

    std::string genesis_coinbase_tx_hex = genesis_tx;

    blobdata tx_bl;
    bool r = string_tools::parse_hexstr_to_binbuff(genesis_tx, tx_bl);
    CHECK_AND_ASSERT_MES(r, false, "failed to parse coinbase tx from hard coded blob");
    r = parse_and_validate_tx_from_blob(tx_bl, bl.miner_tx);
    CHECK_AND_ASSERT_MES(r, false, "failed to parse coinbase tx from hard coded blob");
    bl.major_version = CURRENT_BLOCK_MAJOR_VERSION;
    bl.minor_version = CURRENT_BLOCK_MINOR_VERSION;
    bl.timestamp = 0;
    bl.nonce = nonce;
    miner::find_nonce_for_given_block(bl, 1, 0);
    bl.invalidate_hashes();
    return true;
  }
  //---------------------------------------------------------------
}