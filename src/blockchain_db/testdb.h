// Copyright (c) 2014-2019, The Monero Project
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

#pragma once

#include <string>
#include <vector>
#include <map>

#include "blockchain_db.h"

namespace cryptonote
{

class BaseTestDB: public cryptonote::BlockchainDB {
public:
  BaseTestDB() {}
  virtual void open(const std::string& filename, const int db_flags = 0) override { }
  virtual void close() override {}
  virtual void sync() override {}
  virtual void safesyncmode(const bool onoff) override {}
  virtual void reset() override {}
  virtual std::vector<std::string> get_filenames() const override { return std::vector<std::string>(); }
  virtual bool remove_data_file(const std::string& folder) const override { return true; }
  virtual std::string get_db_name() const override { return std::string(); }
  virtual bool lock() override { return true; }
  virtual void unlock() override { }
  virtual bool batch_start(uint64_t batch_num_blocks=0, uint64_t batch_bytes=0) override { return true; }
  virtual void batch_stop() override {}
  virtual void batch_abort() override {}
  virtual void set_batch_transactions(bool) override {}
  virtual void block_wtxn_start() override {}
  virtual void block_wtxn_stop() override {}
  virtual void block_wtxn_abort() override {}
  virtual bool block_rtxn_start() const override { return true; }
  virtual void block_rtxn_stop() const override {}
  virtual void block_rtxn_abort() const override {}

  virtual void drop_hard_fork_info() override {}
  virtual bool block_exists(const crypto::hash& h, uint64_t *height) const override { return false; }
  virtual cryptonote::blobdata get_block_blob_from_height(const uint64_t& height) const override { return cryptonote::t_serializable_object_to_blob(get_block_from_height(height)); }
  virtual cryptonote::blobdata get_block_blob(const crypto::hash& h) const override { return cryptonote::blobdata(); }
  virtual bool get_tx_blob(const crypto::hash& h, cryptonote::blobdata &tx) const override { return false; }
  virtual bool get_pruned_tx_blob(const crypto::hash& h, cryptonote::blobdata &tx) const override { return false; }
  virtual bool get_prunable_tx_blob(const crypto::hash& h, cryptonote::blobdata &tx) const override { return false; }
  virtual bool get_prunable_tx_hash(const crypto::hash& tx_hash, crypto::hash &prunable_hash) const override { return false; }
  virtual uint64_t get_block_height(const crypto::hash& h) const override { return 0; }
  virtual cryptonote::block_header get_block_header(const crypto::hash& h) const override { return cryptonote::block_header(); }
  virtual uint64_t get_block_timestamp(const uint64_t& height) const override { return 0; }
  virtual std::vector<uint64_t> get_block_cumulative_rct_outputs(const std::vector<uint64_t> &heights) const override { return {}; }
  virtual uint64_t get_top_block_timestamp() const override { return 0; }
  virtual size_t get_block_weight(const uint64_t& height) const override { return 128; }
  virtual std::vector<uint64_t> get_block_weights(uint64_t start_height, size_t count) const override { return {}; }
  virtual cryptonote::difficulty_type get_block_cumulative_difficulty(const uint64_t& height) const override { return 10; }
  virtual cryptonote::difficulty_type get_block_difficulty(const uint64_t& height) const override { return 0; }
  virtual uint64_t get_block_already_generated_coins(const uint64_t& height) const override { return 10000000000; }
  virtual uint64_t get_block_long_term_weight(const uint64_t& height) const override { return 128; }
  virtual std::vector<uint64_t> get_long_term_block_weights(uint64_t start_height, size_t count) const override { return {}; }
  virtual crypto::hash get_block_hash_from_height(const uint64_t& height) const override { return crypto::hash(); }
  virtual std::vector<cryptonote::block> get_blocks_range(const uint64_t& h1, const uint64_t& h2) const override { return std::vector<cryptonote::block>(); }
  virtual std::vector<crypto::hash> get_hashes_range(const uint64_t& h1, const uint64_t& h2) const override { return std::vector<crypto::hash>(); }
  virtual crypto::hash top_block_hash(uint64_t *block_height = NULL) const override { if (block_height) *block_height = 0; return crypto::hash(); }
  virtual cryptonote::block get_top_block() const override { return cryptonote::block(); }
  virtual uint64_t height() const override { return 1; }
  virtual bool tx_exists(const crypto::hash& h) const override { return false; }
  virtual bool tx_exists(const crypto::hash& h, uint64_t& tx_index) const override { return false; }
  virtual uint64_t get_tx_unlock_time(const crypto::hash& h) const override { return 0; }
  virtual cryptonote::transaction get_tx(const crypto::hash& h) const override { return cryptonote::transaction(); }
  virtual bool get_tx(const crypto::hash& h, cryptonote::transaction &tx) const override { return false; }
  virtual uint64_t get_tx_count() const override { return 0; }
  virtual std::vector<cryptonote::transaction> get_tx_list(const std::vector<crypto::hash>& hlist) const override { return std::vector<cryptonote::transaction>(); }
  virtual uint64_t get_tx_block_height(const crypto::hash& h) const override { return 0; }
  virtual uint64_t get_num_outputs(const uint64_t& amount) const override { return 1; }
  virtual uint64_t get_indexing_base() const override { return 0; }
  virtual cryptonote::output_data_t get_output_key(const uint64_t& amount, const uint64_t& index, bool include_commitmemt) const override { return cryptonote::output_data_t(); }
  virtual cryptonote::tx_out_index get_output_tx_and_index_from_global(const uint64_t& index) const override { return cryptonote::tx_out_index(); }
  virtual cryptonote::tx_out_index get_output_tx_and_index(const uint64_t& amount, const uint64_t& index) const override { return cryptonote::tx_out_index(); }
  virtual void get_output_tx_and_index(const uint64_t& amount, const std::vector<uint64_t> &offsets, std::vector<cryptonote::tx_out_index> &indices) const override {}
  virtual void get_output_key(const epee::span<const uint64_t> &amounts, const std::vector<uint64_t> &offsets, std::vector<cryptonote::output_data_t> &outputs, bool allow_partial = false) const override {}
  virtual bool can_thread_bulk_indices() const override { return false; }
  virtual std::vector<std::vector<uint64_t>> get_tx_amount_output_indices(const uint64_t tx_index, size_t n_txes) const override { return std::vector<std::vector<uint64_t>>(); }
  virtual bool has_key_image(const crypto::key_image& img) const override { return false; }
  virtual void remove_block() override { }
  virtual uint64_t add_transaction_data(const crypto::hash& blk_hash, const std::pair<cryptonote::transaction, cryptonote::blobdata>& tx, const crypto::hash& tx_hash, const crypto::hash& tx_prunable_hash) override {return 0;}
  virtual void remove_transaction_data(const crypto::hash& tx_hash, const cryptonote::transaction& tx) override {}
  virtual uint64_t add_output(const crypto::hash& tx_hash, const cryptonote::tx_out& tx_output, const uint64_t& local_index, const uint64_t unlock_time, const rct::key *commitment) override {return 0;}
  virtual void add_tx_amount_output_indices(const uint64_t tx_index, const std::vector<uint64_t>& amount_output_indices) override {}
  virtual void add_spent_key(const crypto::key_image& k_image) override {}
  virtual void remove_spent_key(const crypto::key_image& k_image) override {}

  virtual bool for_all_key_images(std::function<bool(const crypto::key_image&)>) const override { return true; }
  virtual bool for_blocks_range(const uint64_t&, const uint64_t&, std::function<bool(uint64_t, const crypto::hash&, const cryptonote::block&)>) const override { return true; }
  virtual bool for_all_transactions(std::function<bool(const crypto::hash&, const cryptonote::transaction&)>, bool pruned) const override { return true; }
  virtual bool for_all_outputs(std::function<bool(uint64_t amount, const crypto::hash &tx_hash, uint64_t height, size_t tx_idx)> f) const override { return true; }
  virtual bool for_all_outputs(uint64_t amount, const std::function<bool(uint64_t height)> &f) const override { return true; }
  virtual bool is_read_only() const override { return false; }
  virtual std::map<uint64_t, std::tuple<uint64_t, uint64_t, uint64_t>> get_output_histogram(const std::vector<uint64_t> &amounts, bool unlocked, uint64_t recent_cutoff, uint64_t min_count) const override { return std::map<uint64_t, std::tuple<uint64_t, uint64_t, uint64_t>>(); }
  virtual bool get_output_distribution(uint64_t amount, uint64_t from_height, uint64_t to_height, std::vector<uint64_t> &distribution, uint64_t &base) const override { return false; }

  virtual void add_txpool_tx(const crypto::hash &txid, const cryptonote::blobdata &blob, const cryptonote::txpool_tx_meta_t& details) override {}
  virtual void update_txpool_tx(const crypto::hash &txid, const cryptonote::txpool_tx_meta_t& details) override {}
  virtual uint64_t get_txpool_tx_count(bool include_unrelayed_txes = true) const override { return 0; }
  virtual bool txpool_has_tx(const crypto::hash &txid) const override { return false; }
  virtual void remove_txpool_tx(const crypto::hash& txid) override {}
  virtual bool get_txpool_tx_meta(const crypto::hash& txid, cryptonote::txpool_tx_meta_t &meta) const override { return false; }
  virtual bool get_txpool_tx_blob(const crypto::hash& txid, cryptonote::blobdata &bd) const override { return false; }
  virtual uint64_t get_database_size() const override { return 0; }
  virtual cryptonote::blobdata get_txpool_tx_blob(const crypto::hash& txid) const override { return ""; }
  virtual bool for_all_txpool_txes(std::function<bool(const crypto::hash&, const cryptonote::txpool_tx_meta_t&, const cryptonote::blobdata*)>, bool include_blob = false, bool include_unrelayed_txes = false) const override { return false; }

  virtual void add_block( const cryptonote::block& blk
                        , size_t block_weight
                        , uint64_t long_term_block_weight
                        , const cryptonote::difficulty_type& cumulative_difficulty
                        , const uint64_t& coins_generated
                        , uint64_t num_rct_outs
                        , const crypto::hash& blk_hash
                        ) override { }
  virtual cryptonote::block get_block_from_height(const uint64_t& height) const override { return cryptonote::block(); }
  virtual void set_hard_fork_version(uint64_t height, uint8_t version) override {}
  virtual uint8_t get_hard_fork_version(uint64_t height) const override { return 0; }
  virtual void check_hard_fork_info() override {}

  virtual void set_block_cumulative_difficulty(uint64_t height, difficulty_type diff) override {};

  virtual void set_validator_list(std::string, uint32_t expiration_date) override {};
  virtual std::string get_validator_list() const override {return std::string();};

  virtual uint32_t get_blockchain_pruning_seed() const override { return 0; }
  virtual bool prune_blockchain(uint32_t pruning_seed = 0) override { return true; }
  virtual bool update_pruning() override { return true; }
  virtual bool check_pruning() override { return true; }
  virtual void prune_outputs(uint64_t amount) override {}

  virtual uint64_t get_max_block_size() override { return 100000000; }
  virtual void add_max_block_size(uint64_t sz) override { }
  virtual void add_chainstate_utxo(const crypto::hash tx_hash, const uint32_t relative_out_index, const crypto::public_key combined_key, uint64_t amount, uint64_t unlock_time, bool is_coinbase = false) override {}
  virtual bool check_chainstate_utxo(const crypto::hash tx_hash, const uint32_t relative_out_index) override { return true; };
  virtual uint64_t get_utxo_unlock_time(const crypto::hash tx_hash, const uint32_t relative_out_index){return 0;}
  virtual void remove_chainstate_utxo(const crypto::hash tx_hash, const uint32_t relative_out_index) override {}

  virtual void add_addr_output(const crypto::hash tx_hash, const uint32_t relative_out_index, const crypto::public_key& combined_key, uint64_t amount, uint64_t unlock_time) override {}
  virtual std::vector<address_outputs> get_addr_output_all(const crypto::public_key& combined_key) override { return std::vector<address_outputs> {}; }
  virtual std::vector<address_outputs> get_addr_output_batch(const crypto::public_key& combined_key, uint64_t start_db_index = 0, uint64_t batch_size = 100, bool desc = false) override { return std::vector<address_outputs> {}; }
  virtual void remove_addr_output(const crypto::hash tx_hash, const uint32_t relative_out_index, const crypto::public_key& combined_key, uint64_t amount, uint64_t unlock_time) override {}
  virtual void add_addr_tx(const crypto::hash tx_hash, const crypto::public_key& combined_key) override {}
  virtual std::vector<address_txs> get_addr_tx_all(const crypto::public_key& combined_key) override { return std::vector<address_txs> {}; }
  virtual std::vector<address_txs> get_addr_tx_batch(const crypto::public_key& combined_key, uint64_t start_db_index = 0, uint64_t batch_size = 100, bool desc = false) override { return std::vector<address_txs> {}; }virtual uint64_t get_balance(const crypto::public_key& combined_key) override { return 0; }
  virtual void remove_addr_tx(const crypto::hash tx_hash, const crypto::public_key& combined_key) override {}
  virtual void add_tx_input(const crypto::hash tx_hash, const uint32_t relative_out_index, const crypto::hash parent_tx_hash, const uint64_t in_index) override {}
  virtual tx_input_t get_tx_input(const crypto::hash tx_hash, const uint32_t relative_out_index) override { return tx_input_t(); }
  virtual void remove_tx_input(const crypto::hash tx_hash, const uint32_t relative_out_index) override {}
};

}
