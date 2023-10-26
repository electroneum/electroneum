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

#include <boost/range/adaptor/reversed.hpp>
#include <unordered_set>

#include "string_tools.h"
#include "blockchain_db.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "profile_tools.h"
#include "ringct/rctOps.h"

#include "lmdb/db_lmdb.h"
#ifdef BERKELEY_DB
#include "berkeleydb/db_bdb.h"
#endif

static const char *db_types[] = {
  "lmdb",
#ifdef BERKELEY_DB
  "berkeley",
#endif
  NULL
};

#undef ELECTRONEUM_DEFAULT_LOG_CATEGORY
#define ELECTRONEUM_DEFAULT_LOG_CATEGORY "blockchain.db"

using epee::string_tools::pod_to_hex;

namespace cryptonote
{

bool blockchain_valid_db_type(const std::string& db_type)
{
  int i;
  for (i=0; db_types[i]; i++)
  {
    if (db_types[i] == db_type)
      return true;
  }
  return false;
}

std::string blockchain_db_types(const std::string& sep)
{
  int i;
  std::string ret = "";
  for (i=0; db_types[i]; i++)
  {
    if (i)
      ret += sep;
    ret += db_types[i];
  }
  return ret;
}

std::string arg_db_type_description = "Specify database type, available: " + cryptonote::blockchain_db_types(", ");
const command_line::arg_descriptor<std::string> arg_db_type = {
  "db-type"
, arg_db_type_description.c_str()
, DEFAULT_DB_TYPE
};
const command_line::arg_descriptor<std::string> arg_db_sync_mode = {
  "db-sync-mode"
, "Specify sync option, using format [safe|fast|fastest]:[sync|async]:[<nblocks_per_sync>[blocks]|<nbytes_per_sync>[bytes]]." 
, "fast:async:250000000bytes"
};
const command_line::arg_descriptor<bool> arg_db_salvage  = {
  "db-salvage"
, "Try to salvage a blockchain database if it seems corrupted"
, false
};

BlockchainDB *new_db(const std::string& db_type)
{
  if (db_type == "lmdb")
    return new BlockchainLMDB();
#if defined(BERKELEY_DB)
  if (db_type == "berkeley")
    return new BlockchainBDB();
#endif
  return NULL;
}

void BlockchainDB::init_options(boost::program_options::options_description& desc)
{
  command_line::add_arg(desc, arg_db_type);
  command_line::add_arg(desc, arg_db_sync_mode);
  command_line::add_arg(desc, arg_db_salvage);
}

void BlockchainDB::pop_block()
{
  block blk;
  std::vector<transaction> txs;
  pop_block(blk, txs);
}

void BlockchainDB::add_transaction(const crypto::hash& blk_hash, const std::pair<transaction, blobdata>& txp, const crypto::hash* tx_hash_ptr, const crypto::hash* tx_prunable_hash_ptr)
{
  const transaction &tx = txp.first;

  bool miner_tx = false;
  crypto::hash tx_hash, tx_prunable_hash;
  if (!tx_hash_ptr)
  {
    // should only need to compute hash for miner transactions
    tx_hash = get_transaction_hash(tx);
    LOG_PRINT_L3("null tx_hash_ptr - needed to compute: " << tx_hash);
  }
  else
  {
    tx_hash = *tx_hash_ptr;
  }

  std::vector<std::pair<crypto::hash, uint64_t>> utxos_to_remove;
  // keep a set of the etn addresses (derived from BOTH ins and outs) associated with the tx for removal from addr_tx db
  std::unordered_set<cryptonote::account_public_address> addr_tx_addresses;

  // Sanity check on supported input types
  for (size_t i = 0; i < tx.vin.size(); ++i)
  {
    const txin_v& tx_input = tx.vin[i];
    if (tx_input.type() == typeid(txin_to_key))
    {
      add_spent_key(boost::get<txin_to_key>(tx_input).k_image);
    }
    else if (tx_input.type() == typeid(txin_to_key_public))
    {
      const auto &txin = boost::get<txin_to_key_public>(tx_input);
      utxos_to_remove.push_back({txin.tx_hash, txin.relative_offset});
      add_tx_input(txin.tx_hash, txin.relative_offset, tx.hash, i);

      //work for addr_tx db
      transaction parent_tx = get_tx(txin.tx_hash);
      const auto &txout = boost::get<txout_to_key_public>(parent_tx.vout[txin.relative_offset].target); //previous tx out that this tx in references
      if(addr_tx_addresses.find(txout.address) == addr_tx_addresses.end()){ //if addr hasn't been used for another input yet, add the unique addr tx record for this address
          add_addr_tx(tx.hash, addKeys(txout.address.m_view_public_key, txout.address.m_spend_public_key));
          addr_tx_addresses.insert(txout.address);
      }
    }
    else if (tx_input.type() == typeid(txin_gen))
    {
      /* nothing to do here */
      miner_tx = true;
    }
    else
    {
      LOG_PRINT_L1("Unsupported input type, removing key images and aborting transaction addition");
      for (const txin_v& tx_input : tx.vin)
      {
        if (tx_input.type() == typeid(txin_to_key))
        {
          remove_spent_key(boost::get<txin_to_key>(tx_input).k_image); // inputs are already checked here regardless of version
        }
        if (tx_input.type() == typeid(txin_to_key_public)) {
            //rewind tx inputs added to tx input db if the transaction aborts
            const auto &txin = boost::get<txin_to_key_public>(tx_input);
            remove_tx_input(txin.tx_hash, txin.relative_offset);
            //work for addr_tx db
            transaction parent_tx = get_tx(txin.tx_hash);
            const auto &txout = boost::get<txout_to_key_public>(parent_tx.vout[txin.relative_offset].target); //previous tx out that this tx in references
            if (addr_tx_addresses.find(txout.address) != addr_tx_addresses.end()) { // dont do a remove for every input. there is only one entry per address per tx in the addr tx db
                remove_addr_tx(tx.hash, addKeys(txout.address.m_view_public_key, txout.address.m_spend_public_key));
                addr_tx_addresses.erase(txout.address);
            }
        }
      }
      return;
    }
  }

  if (tx.version == 1)
  {
    uint64_t tx_id = add_transaction_data(blk_hash, txp, tx_hash, tx_prunable_hash);

    std::vector<uint64_t> amount_output_indices(tx.vout.size());

    // iterate tx.vout using indices instead of C++11 foreach syntax because
    // we need the index
    for (uint64_t i = 0; i < tx.vout.size(); ++i)
    {
      amount_output_indices[i] = add_output(tx_hash, tx.vout[i], i, tx.unlock_time, NULL);
    }
    add_tx_amount_output_indices(tx_id, amount_output_indices);
  }
  else if (tx.version >= 2)
  {
    add_transaction_data(blk_hash, txp, tx_hash, tx_prunable_hash);

    // Sanity check on supported output types
    for (uint64_t i = 0; i < tx.vout.size(); ++i)
    {
      if(tx.vout[i].target.type() != typeid(txout_to_key_public))
      {
        LOG_PRINT_L1("Unsupported output type, reinstating UTXOs, removing key images and aborting transaction addition");
        for (const txin_v& tx_input : tx.vin) {
            if (tx_input.type() == typeid(txin_to_key)) {
                remove_spent_key(boost::get<txin_to_key>(tx_input).k_image);
            }
            if (tx_input.type() == typeid(txin_to_key_public)) {
                //rewind tx inputs added to tx input db if the transaction aborts
                const auto &txin = boost::get<txin_to_key_public>(tx_input);
                remove_tx_input(txin.tx_hash, txin.relative_offset);
                //work for addr_tx db
                transaction parent_tx = get_tx(txin.tx_hash);
                const auto &txout = boost::get<txout_to_key_public>(
                        parent_tx.vout[txin.relative_offset].target); //previous tx out that this tx in references
                if (addr_tx_addresses.find(txout.address) !=
                    addr_tx_addresses.end()) { // dont do a remove for every input, only a remove for every addr that was uniquely added to the addr tx db
                    remove_addr_tx(tx.hash, addKeys(txout.address.m_view_public_key, txout.address.m_spend_public_key));
                    addr_tx_addresses.erase(txout.address);
                }
            }
        }
        return;
      }// if outs are all of the right type, we're ok to proceed by removing the utxos that are now spent

      for(auto utxo: utxos_to_remove)
      {
            remove_chainstate_utxo(utxo.first, utxo.second);
      }

      const auto &txout = boost::get<txout_to_key_public>(tx.vout[i].target);
      add_chainstate_utxo(tx.hash, i, addKeys(txout.address.m_view_public_key, txout.address.m_spend_public_key) , tx.vout[i].amount, txp.first.unlock_time, miner_tx);
      add_addr_output(tx.hash, i, addKeys(txout.address.m_view_public_key, txout.address.m_spend_public_key), tx.vout[i].amount, txp.first.unlock_time);
      if(addr_tx_addresses.find(txout.address) == addr_tx_addresses.end()){ //if addr hasn't been used for another input yet, add the unique addr tx record for this address
          add_addr_tx(tx.hash, addKeys(txout.address.m_view_public_key, txout.address.m_spend_public_key));
          addr_tx_addresses.insert(txout.address);
      }
    }//end of v2+ processing
  }
}

uint64_t BlockchainDB::add_block( const std::pair<block, blobdata>& blck
                                , size_t block_weight
                                , uint64_t long_term_block_weight
                                , const difficulty_type& cumulative_difficulty
                                , const uint64_t& coins_generated
                                , const std::vector<std::pair<transaction, blobdata>>& txs
                                )
{
  const block &blk = blck.first;

  // sanity
  if (blk.tx_hashes.size() != txs.size())
    throw std::runtime_error("Inconsistent tx/hashes sizes");

  TIME_MEASURE_START(time1);
  crypto::hash blk_hash = get_block_hash(blk);
  TIME_MEASURE_FINISH(time1);
  time_blk_hash += time1;

  uint64_t prev_height = height();

  // call out to add the transactions

  time1 = epee::misc_utils::get_tick_count();

  add_transaction(blk_hash, std::make_pair(blk.miner_tx, tx_to_blob(blk.miner_tx)));
  int tx_i = 0;
  crypto::hash tx_hash = crypto::null_hash;
  for (const std::pair<transaction, blobdata>& tx : txs)
  {
    tx_hash = blk.tx_hashes[tx_i];
    add_transaction(blk_hash, tx, &tx_hash);
    ++tx_i;
  }
  TIME_MEASURE_FINISH(time1);
  time_add_transaction += time1;

  // call out to subclass implementation to add the block & metadata
  time1 = epee::misc_utils::get_tick_count();
  add_block(blk, block_weight, long_term_block_weight, cumulative_difficulty, coins_generated, 0, blk_hash);
  TIME_MEASURE_FINISH(time1);
  time_add_block1 += time1;
  //voting mechanism
  m_hardfork->add(blk, prev_height);

  ++num_calls;

  return prev_height;
}

void BlockchainDB::set_hard_fork(HardFork* hf)
{
  m_hardfork = hf;
}

void BlockchainDB::pop_block(block& blk, std::vector<transaction>& txs)
{
  blk = get_top_block();

  remove_block();

  for (const auto& h : boost::adaptors::reverse(blk.tx_hashes))
  {
    cryptonote::transaction tx;
    if (!get_tx(h, tx) && !get_pruned_tx(h, tx))
      throw DB_ERROR("Failed to get pruned or unpruned transaction from the db");
    txs.push_back(std::move(tx));
    remove_transaction(h);
  }
  remove_transaction(get_transaction_hash(blk.miner_tx));
}

bool BlockchainDB::is_open() const
{
  return m_open;
}

void BlockchainDB::remove_transaction(const crypto::hash& tx_hash)
{
  transaction tx = get_pruned_tx(tx_hash);

  // keep a set of the etn addresses (derived from BOTH ins and outs) associated with the tx for removal from addr_tx db
  std::unordered_set<cryptonote::account_public_address> addr_tx_addresses;

  for (const txin_v& tx_input : tx.vin)
  {
    if (tx_input.type() == typeid(txin_to_key))
    {
      remove_spent_key(boost::get<txin_to_key>(tx_input).k_image);
    }
    else if (tx_input.type() == typeid(txin_to_key_public))
    {
      const auto &txin = boost::get<txin_to_key_public>(tx_input); // input being used in the tx to be removed.

      transaction parent_tx = get_tx(txin.tx_hash);
      const auto &txout = boost::get<txout_to_key_public>(parent_tx.vout[txin.relative_offset].target); //previous tx out that this tx in references
      //reinstate that out as a utxo
      bool reinstate_coinbase = cryptonote::is_coinbase(get_pruned_tx(txin.tx_hash));
      add_chainstate_utxo(txin.tx_hash, txin.relative_offset, addKeys(txout.address.m_view_public_key, txout.address.m_spend_public_key), txin.amount, get_tx_unlock_time(txin.tx_hash), reinstate_coinbase);
      remove_tx_input(txin.tx_hash, txin.relative_offset);


      if(addr_tx_addresses.find(txout.address) == addr_tx_addresses.end()){
          remove_addr_tx(tx.hash, addKeys(txout.address.m_view_public_key, txout.address.m_spend_public_key));
          addr_tx_addresses.insert(txout.address);
      }
    }
  }

  if (tx.version >= 2)
  {
    for (uint64_t i = 0; i < tx.vout.size(); ++i)
    {
      const auto &txout = boost::get<txout_to_key_public>(tx.vout[i].target);

      remove_chainstate_utxo(tx.hash, i);
      remove_addr_output(tx_hash, i, addKeys(txout.address.m_view_public_key, txout.address.m_spend_public_key), tx.vout[i].amount, tx.unlock_time);

      // remove addr tx entries for outputs involving addr that weren't used for ins
      if(addr_tx_addresses.find(txout.address) == addr_tx_addresses.end()){
          remove_addr_tx(tx.hash, addKeys(txout.address.m_view_public_key, txout.address.m_spend_public_key));
          addr_tx_addresses.insert(txout.address);
      }
    }
  }

  // need tx as tx.vout has the tx outputs, and the output amounts are needed
  remove_transaction_data(tx_hash, tx);
}

block BlockchainDB::get_block_from_height(const uint64_t& height) const
{
  blobdata bd = get_block_blob_from_height(height);
  block b;
  if (!parse_and_validate_block_from_blob(bd, b))
    throw DB_ERROR("Failed to parse block from blob retrieved from the db");

  return b;
}

block BlockchainDB::get_block(const crypto::hash& h) const
{
  blobdata bd = get_block_blob(h);
  block b;
  if (!parse_and_validate_block_from_blob(bd, b))
    throw DB_ERROR("Failed to parse block from blob retrieved from the db");

  return b;
}

bool BlockchainDB::get_tx(const crypto::hash& h, cryptonote::transaction &tx) const
{
  blobdata bd;
  if (!get_tx_blob(h, bd))
    return false;
  if (!parse_and_validate_tx_from_blob(bd, tx))
    throw DB_ERROR("Failed to parse transaction from blob retrieved from the db");

  return true;
}

bool BlockchainDB::get_pruned_tx(const crypto::hash& h, cryptonote::transaction &tx) const
{
  blobdata bd;
  if (!get_pruned_tx_blob(h, bd))
    return false;
  if (!parse_and_validate_tx_base_from_blob(bd, tx))
    throw DB_ERROR("Failed to parse transaction base from blob retrieved from the db");

  return true;
}

transaction BlockchainDB::get_tx(const crypto::hash& h) const
{
  transaction tx;
  if (!get_tx(h, tx))
    throw TX_DNE(std::string("tx with hash ").append(epee::string_tools::pod_to_hex(h)).append(" not found in db").c_str());
  return tx;
}

transaction BlockchainDB::get_pruned_tx(const crypto::hash& h) const
{
  transaction tx;
  if (!get_pruned_tx(h, tx))
    throw TX_DNE(std::string("pruned tx with hash ").append(epee::string_tools::pod_to_hex(h)).append(" not found in db").c_str());
  return tx;
}

void BlockchainDB::reset_stats()
{
  num_calls = 0;
  time_blk_hash = 0;
  time_tx_exists = 0;
  time_add_block1 = 0;
  time_add_transaction = 0;
  time_commit1 = 0;
}

void BlockchainDB::show_stats()
{
  LOG_PRINT_L1(ENDL
    << "*********************************"
    << ENDL
    << "num_calls: " << num_calls
    << ENDL
    << "time_blk_hash: " << time_blk_hash << "ms"
    << ENDL
    << "time_tx_exists: " << time_tx_exists << "ms"
    << ENDL
    << "time_add_block1: " << time_add_block1 << "ms"
    << ENDL
    << "time_add_transaction: " << time_add_transaction << "ms"
    << ENDL
    << "time_commit1: " << time_commit1 << "ms"
    << ENDL
    << "*********************************"
    << ENDL
  );
}

void BlockchainDB::fixup()
{
  if (is_read_only()) {
    LOG_PRINT_L1("Database is opened read only - skipping fixup check");
    return;
  }

  // Apply fixes to DB if needed.

  batch_stop();
}

}  // namespace cryptonote
