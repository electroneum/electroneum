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

#include <boost/preprocessor/stringize.hpp>
#include "include_base_utils.h"
#include "string_tools.h"
#include <boost/format.hpp>
#include <numeric>
#include <stdio.h>
#include <time.h>
using namespace epee;

#include "core_rpc_server.h"
#include "common/command_line.h"
#include "common/updates.h"
#include "common/download.h"
#include "common/util.h"
#include "common/perf_timer.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "cryptonote_basic/account.h"
#include "cryptonote_basic/cryptonote_basic_impl.h"
#include "cryptonote_core/tx_sanity_check.h"
#include "misc_language.h"
#include "net/parse.h"
#include "storages/http_abstract_invoke.h"
#include "crypto/hash.h"
#include "rpc/rpc_args.h"
#include "rpc/rpc_handler.h"
#include "core_rpc_server_error_codes.h"
#include "p2p/net_node.h"
#include "version.h"

#undef ELECTRONEUM_DEFAULT_LOG_CATEGORY
#define ELECTRONEUM_DEFAULT_LOG_CATEGORY "daemon.rpc"

#define MAX_RESTRICTED_FAKE_OUTS_COUNT 40
#define MAX_RESTRICTED_GLOBAL_FAKE_OUTS_COUNT 5000

#define OUTPUT_HISTOGRAM_RECENT_CUTOFF_RESTRICTION (3 * 86400) // 3 days max, the wallet requests 1.8 days

namespace
{
  void add_reason(std::string &reasons, const char *reason)
  {
    if (!reasons.empty())
      reasons += ", ";
    reasons += reason;
  }

  uint64_t round_up(uint64_t value, uint64_t quantum)
  {
    return (value + quantum - 1) / quantum * quantum;
  }

  void store_difficulty(cryptonote::difficulty_type difficulty, uint64_t &sdiff, std::string &swdiff, uint64_t &stop64)
  {
    sdiff = (difficulty & 0xffffffffffffffff).convert_to<uint64_t>();
    swdiff = cryptonote::hex(difficulty);
    stop64 = ((difficulty >> 64) & 0xffffffffffffffff).convert_to<uint64_t>();
  }
}

#define OUTPUT_HISTOGRAM_RECENT_CUTOFF_RESTRICTION (3 * 86400) // 3 days max, the wallet requests 1.8 days

namespace cryptonote
{

  //-----------------------------------------------------------------------------------
  void core_rpc_server::init_options(boost::program_options::options_description& desc)
  {
    command_line::add_arg(desc, arg_rpc_bind_port);
    command_line::add_arg(desc, arg_rpc_restricted_bind_port);
    command_line::add_arg(desc, arg_restricted_rpc);
    command_line::add_arg(desc, arg_bootstrap_daemon_address);
    command_line::add_arg(desc, arg_bootstrap_daemon_login);
    cryptonote::rpc_args::init_options(desc, true);
  }
  //------------------------------------------------------------------------------------------------------------------------------
  core_rpc_server::core_rpc_server(
      core& cr
    , nodetool::node_server<cryptonote::t_cryptonote_protocol_handler<cryptonote::core> >& p2p
    )
    : m_core(cr)
    , m_p2p(p2p)
  {}
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::init(
      const boost::program_options::variables_map& vm
      , const bool restricted
      , const std::string& port
    )
  {
    m_restricted = restricted;
    m_net_server.set_threads_prefix("RPC");

    auto rpc_config = cryptonote::rpc_args::process(vm, true);
    if (!rpc_config)
      return false;

    m_bootstrap_daemon_address = command_line::get_arg(vm, arg_bootstrap_daemon_address);
    if (!m_bootstrap_daemon_address.empty())
    {
      const std::string &bootstrap_daemon_login = command_line::get_arg(vm, arg_bootstrap_daemon_login);
      const auto loc = bootstrap_daemon_login.find(':');
      if (!bootstrap_daemon_login.empty() && loc != std::string::npos)
      {
        epee::net_utils::http::login login;
        login.username = bootstrap_daemon_login.substr(0, loc);
        login.password = bootstrap_daemon_login.substr(loc + 1);
        m_http_client.set_server(m_bootstrap_daemon_address, login, epee::net_utils::ssl_support_t::e_ssl_support_autodetect);
      }
      else
      {
        m_http_client.set_server(m_bootstrap_daemon_address, boost::none, epee::net_utils::ssl_support_t::e_ssl_support_autodetect);
      }
      m_should_use_bootstrap_daemon = true;
    }
    else
    {
      m_should_use_bootstrap_daemon = false;
    }
    m_was_bootstrap_ever_used = false;

    boost::optional<epee::net_utils::http::login> http_login{};

    if (rpc_config->login)
      http_login.emplace(std::move(rpc_config->login->username), std::move(rpc_config->login->password).password());

    auto rng = [](size_t len, uint8_t *ptr){ return crypto::rand(len, ptr); };
    return epee::http_server_impl_base<core_rpc_server, connection_context>::init(
      rng, std::move(port), std::move(rpc_config->bind_ip), std::move(rpc_config->access_control_origins), std::move(http_login), std::move(rpc_config->ssl_options)
    );
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::check_core_ready()
  {
    if(!m_p2p.get_payload_object().is_synchronized())
    {
      return false;
    }
    return true;
  }
#define CHECK_CORE_READY() do { if(!check_core_ready()){res.status =  CORE_RPC_STATUS_BUSY;return true;} } while(0)

  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_height(const COMMAND_RPC_GET_HEIGHT::request& req, COMMAND_RPC_GET_HEIGHT::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_height);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_HEIGHT>(invoke_http_mode::JON, "/getheight", req, res, r))
      return r;

    crypto::hash hash;
    m_core.get_blockchain_top(res.height, hash);
    ++res.height; // block height to chain height
    res.hash = string_tools::pod_to_hex(hash);
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_info(const COMMAND_RPC_GET_INFO::request& req, COMMAND_RPC_GET_INFO::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_info);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_INFO>(invoke_http_mode::JON, "/getinfo", req, res, r))
    {
      res.bootstrap_daemon_address = m_bootstrap_daemon_address;
      crypto::hash top_hash;
      m_core.get_blockchain_top(res.height_without_bootstrap, top_hash);
      ++res.height_without_bootstrap; // turn top block height into blockchain height
      res.was_bootstrap_ever_used = true;
      return r;
    }

    const bool restricted = m_restricted && ctx;

    crypto::hash top_hash;
    m_core.get_blockchain_top(res.height, top_hash);
    ++res.height; // turn top block height into blockchain height
    res.top_block_hash = string_tools::pod_to_hex(top_hash);
    res.target_height = m_core.get_target_blockchain_height();
    store_difficulty(m_core.get_blockchain_storage().get_difficulty_for_next_block(), res.difficulty, res.wide_difficulty, res.difficulty_top64);
    res.target = m_core.get_blockchain_storage().get_difficulty_target();
    res.tx_count = m_core.get_blockchain_storage().get_total_transactions() - res.height; //without coinbase
    res.tx_pool_size = m_core.get_pool_transactions_count();
    res.alt_blocks_count = restricted ? 0 : m_core.get_blockchain_storage().get_alternative_blocks_count();
    uint64_t total_conn = restricted ? 0 : m_p2p.get_public_connections_count();
    res.outgoing_connections_count = restricted ? 0 : m_p2p.get_public_outgoing_connections_count();
    res.incoming_connections_count = restricted ? 0 : (total_conn - res.outgoing_connections_count);
    res.rpc_connections_count = restricted ? 0 : get_connections_count();
    res.white_peerlist_size = restricted ? 0 : m_p2p.get_public_white_peers_count();
    res.grey_peerlist_size = restricted ? 0 : m_p2p.get_public_gray_peers_count();

    cryptonote::network_type net_type = nettype();
    res.mainnet = net_type == MAINNET;
    res.testnet = net_type == TESTNET;
    res.stagenet = net_type == STAGENET;
    res.nettype = net_type == MAINNET ? "mainnet" : net_type == TESTNET ? "testnet" : net_type == STAGENET ? "stagenet" : "fakechain";
    store_difficulty(m_core.get_blockchain_storage().get_db().get_block_cumulative_difficulty(res.height - 1),
        res.cumulative_difficulty, res.wide_cumulative_difficulty, res.cumulative_difficulty_top64);
    res.block_size_limit = res.block_weight_limit = m_core.get_blockchain_storage().get_current_cumulative_block_weight_limit();
    res.block_size_median = res.block_weight_median = m_core.get_blockchain_storage().get_current_cumulative_block_weight_median();
    res.start_time = restricted ? 0 : (uint64_t)m_core.get_start_time();
    res.free_space = restricted ? std::numeric_limits<uint64_t>::max() : m_core.get_free_space();
    res.offline = m_core.offline();
    res.bootstrap_daemon_address = restricted ? "" : m_bootstrap_daemon_address;
    res.height_without_bootstrap = restricted ? 0 : res.height;
    if (restricted)
      res.was_bootstrap_ever_used = false;
    else
    {
      boost::shared_lock<boost::shared_mutex> lock(m_bootstrap_daemon_mutex);
      res.was_bootstrap_ever_used = m_was_bootstrap_ever_used;
    }
    res.database_size = m_core.get_blockchain_storage().get_db().get_database_size();
    if (restricted)
      res.database_size = round_up(res.database_size, 5ull* 1024 * 1024 * 1024);
    res.update_available = restricted ? false : m_core.is_update_available();
    res.version = restricted ? "" : ELECTRONEUM_VERSION;

    res.status = CORE_RPC_STATUS_OK;

    res.daemon_release_name = ELECTRONEUM_RELEASE_NAME;
    res.daemon_version = ELECTRONEUM_VERSION;
    res.daemon_version_full = ELECTRONEUM_VERSION_FULL;
    res.daemon_version_tag = ELECTRONEUM_VERSION_TAG;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_net_stats(const COMMAND_RPC_GET_NET_STATS::request& req, COMMAND_RPC_GET_NET_STATS::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_net_stats);
    // No bootstrap daemon check: Only ever get stats about local server
    res.start_time = (uint64_t)m_core.get_start_time();
    {
      CRITICAL_REGION_LOCAL(epee::net_utils::network_throttle_manager::m_lock_get_global_throttle_in);
      epee::net_utils::network_throttle_manager::get_global_throttle_in().get_stats(res.total_packets_in, res.total_bytes_in);
    }
    {
      CRITICAL_REGION_LOCAL(epee::net_utils::network_throttle_manager::m_lock_get_global_throttle_out);
      epee::net_utils::network_throttle_manager::get_global_throttle_out().get_stats(res.total_packets_out, res.total_bytes_out);
    }
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  class pruned_transaction {
    transaction& tx;
  public:
    pruned_transaction(transaction& tx) : tx(tx) {}
    BEGIN_SERIALIZE_OBJECT()
      bool r = tx.serialize_base(ar);
      if (!r) return false;
    END_SERIALIZE()
  };
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_blocks(const COMMAND_RPC_GET_BLOCKS_FAST::request& req, COMMAND_RPC_GET_BLOCKS_FAST::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_blocks);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_BLOCKS_FAST>(invoke_http_mode::BIN, "/getblocks.bin", req, res, r))
      return r;

      //Vector of pairs where each pair is:  item#1) pair<block blob, hash>  item#2)vector of pairs of <tx hash, tx blob>
    std::vector<
                std::pair<
                          std::pair<cryptonote::blobdata, crypto::hash>, std::vector<std::pair<crypto::hash, cryptonote::blobdata> >
                         >
    > bs;

    if(!m_core.find_blockchain_supplement(req.start_height, req.block_ids, bs, res.current_height, res.start_height, req.prune, !req.no_miner_tx, COMMAND_RPC_GET_BLOCKS_FAST_MAX_COUNT))
    {
      res.status = "Failed";
      return false;
    }

    size_t pruned_size = 0, unpruned_size = 0, ntxes = 0;

    // Preallocate space in the RPC result
    res.blocks.reserve(bs.size());
    res.output_indices.reserve(bs.size());

    // Loop over all of the block data and populate the rpc response
    uint64_t height_counter = res.start_height;
    for(auto& bd: bs)
    {
      res.blocks.resize(res.blocks.size()+1);
      res.blocks.back().block = bd.first.first;  // add block hash to response
      pruned_size += bd.first.first.size();
      unpruned_size += bd.first.first.size();
      res.output_indices.push_back(COMMAND_RPC_GET_BLOCKS_FAST::block_output_indices());
      ntxes += bd.second.size();
      res.output_indices.back().indices.reserve(1 + bd.second.size());
      if (req.no_miner_tx)
        res.output_indices.back().indices.push_back(COMMAND_RPC_GET_BLOCKS_FAST::tx_output_indices());
      res.blocks.back().txs.reserve(bd.second.size());

      //go over the transactions in bs first
      for (std::vector<std::pair<crypto::hash, cryptonote::blobdata>>::iterator i = bd.second.begin(); i != bd.second.end(); ++i)
      {
        unpruned_size += i->second.size();
        res.blocks.back().txs.push_back(std::move(i->second)); //add a tx blob to rpc response
        i->second.clear();
        i->second.shrink_to_fit();
        pruned_size += res.blocks.back().txs.back().size();
      }

      //no need to look up indexes for v2+ txes
      auto v10_height = nettype() == cryptonote::network_type::MAINNET ? 1175315 : 1086402;
      const size_t n_txes_to_lookup = height_counter >= v10_height ? 0 : (bd.second.size() + (req.no_miner_tx ? 0 : 1));
      if (n_txes_to_lookup > 0)
      {
        std::vector<std::vector<uint64_t>> indices;
        //either pass the first tx hash or the block hash depending on whether it was a miner tx or not
        bool r = m_core.get_tx_outputs_gindexs(req.no_miner_tx ? bd.second.front().first : bd.first.second, n_txes_to_lookup, indices);
        if (!r)
        {
          res.status = "Failed";
          return false;
        }
        if (indices.size() != n_txes_to_lookup || res.output_indices.back().indices.size() != (req.no_miner_tx ? 1 : 0))
        {
          res.status = "Failed";
          return false;
        }
        for (size_t i = 0; i < indices.size(); ++i)
          res.output_indices.back().indices.push_back({std::move(indices[i])});
      }
      ++height_counter;
    }

    MDEBUG("on_get_blocks: " << bs.size() << " blocks, " << ntxes << " txes, pruned size " << pruned_size << ", unpruned size " << unpruned_size);
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
    bool core_rpc_server::on_get_alt_blocks_hashes(const COMMAND_RPC_GET_ALT_BLOCKS_HASHES::request& req, COMMAND_RPC_GET_ALT_BLOCKS_HASHES::response& res, const connection_context *ctx)
    {
      PERF_TIMER(on_get_alt_blocks_hashes);
      bool r;
      if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_ALT_BLOCKS_HASHES>(invoke_http_mode::JON, "/get_alt_blocks_hashes", req, res, r))
        return r;

      std::vector<block> blks;

      if(!m_core.get_alternative_blocks(blks))
      {
          res.status = "Failed";
          return false;
      }

      res.blks_hashes.reserve(blks.size());

      for (auto const& blk: blks)
      {
          res.blks_hashes.push_back(epee::string_tools::pod_to_hex(get_block_hash(blk)));
      }

      MDEBUG("on_get_alt_blocks_hashes: " << blks.size() << " blocks " );
      res.status = CORE_RPC_STATUS_OK;
      return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_blocks_by_height(const COMMAND_RPC_GET_BLOCKS_BY_HEIGHT::request& req, COMMAND_RPC_GET_BLOCKS_BY_HEIGHT::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_blocks_by_height);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_BLOCKS_BY_HEIGHT>(invoke_http_mode::BIN, "/getblocks_by_height.bin", req, res, r))
      return r;

    res.status = "Failed";
    res.blocks.clear();
    res.blocks.reserve(req.heights.size());
    for (uint64_t height : req.heights)
    {
      block blk;
      try
      {
        blk = m_core.get_blockchain_storage().get_db().get_block_from_height(height);
      }
      catch (...)
      {
        res.status = "Error retrieving block at height " + std::to_string(height);
        return true;
      }
      std::vector<transaction> txs;
      std::vector<crypto::hash> missed_txs;
      m_core.get_transactions(blk.tx_hashes, txs, missed_txs);
      res.blocks.resize(res.blocks.size() + 1);
      res.blocks.back().block = block_to_blob(blk);
      for (auto& tx : txs)
        res.blocks.back().txs.push_back(tx_to_blob(tx));
    }
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_hashes(const COMMAND_RPC_GET_HASHES_FAST::request& req, COMMAND_RPC_GET_HASHES_FAST::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_hashes);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_HASHES_FAST>(invoke_http_mode::BIN, "/gethashes.bin", req, res, r))
      return r;

    res.start_height = req.start_height;
    if(!m_core.get_blockchain_storage().find_blockchain_supplement(req.block_ids, res.m_block_ids, res.start_height, res.current_height, false))
    {
      res.status = "Failed";
      return false;
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_outs_bin(const COMMAND_RPC_GET_OUTPUTS_BIN::request& req, COMMAND_RPC_GET_OUTPUTS_BIN::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_outs_bin);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_OUTPUTS_BIN>(invoke_http_mode::BIN, "/get_outs.bin", req, res, r))
      return r;

    res.status = "Failed";

    const bool restricted = m_restricted && ctx;
    if (restricted)
    {
      if (req.outputs.size() > MAX_RESTRICTED_GLOBAL_FAKE_OUTS_COUNT)
      {
        res.status = "Too many outs requested";
        return true;
      }
    }

    if(!m_core.get_outs(req, res))
    {
      return true;
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_outs(const COMMAND_RPC_GET_OUTPUTS::request& req, COMMAND_RPC_GET_OUTPUTS::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_outs);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_OUTPUTS>(invoke_http_mode::JON, "/get_outs", req, res, r))
      return r;

    res.status = "Failed";

    const bool restricted = m_restricted && ctx;
    if (restricted)
    {
      if (req.outputs.size() > MAX_RESTRICTED_GLOBAL_FAKE_OUTS_COUNT)
      {
        res.status = "Too many outs requested";
        return true;
      }
    }

    cryptonote::COMMAND_RPC_GET_OUTPUTS_BIN::request req_bin;
    req_bin.outputs = req.outputs;
    req_bin.get_txid = req.get_txid;
    cryptonote::COMMAND_RPC_GET_OUTPUTS_BIN::response res_bin;
    if(!m_core.get_outs(req_bin, res_bin))
    {
      return true;
    }

    // convert to text
    for (const auto &i: res_bin.outs)
    {
      res.outs.push_back(cryptonote::COMMAND_RPC_GET_OUTPUTS::outkey());
      cryptonote::COMMAND_RPC_GET_OUTPUTS::outkey &outkey = res.outs.back();
      outkey.key = epee::string_tools::pod_to_hex(i.key);
      outkey.mask = epee::string_tools::pod_to_hex(i.mask);
      outkey.unlocked = i.unlocked;
      outkey.height = i.height;
      outkey.txid = epee::string_tools::pod_to_hex(i.txid);
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_indexes(const COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES::request& req, COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_indexes);
    bool ok;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES>(invoke_http_mode::BIN, "/get_o_indexes.bin", req, res, ok))
      return ok;

    bool r = m_core.get_tx_outputs_gindexs(req.txid, res.o_indexes);
    if(!r)
    {
      res.status = "Failed";
      return true;
    }
    res.status = CORE_RPC_STATUS_OK;
    LOG_PRINT_L2("COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES: [" << res.o_indexes.size() << "]");
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_transactions(const COMMAND_RPC_GET_TRANSACTIONS::request& req, COMMAND_RPC_GET_TRANSACTIONS::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_transactions);
    bool ok;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_TRANSACTIONS>(invoke_http_mode::JON, "/gettransactions", req, res, ok))
      return ok;

    std::vector<crypto::hash> vh;
    for(const auto& tx_hex_str: req.txs_hashes)
    {
      blobdata b;
      if(!string_tools::parse_hexstr_to_binbuff(tx_hex_str, b))
      {
        res.status = "Failed to parse hex representation of transaction hash";
        return true;
      }
      if(b.size() != sizeof(crypto::hash))
      {
        res.status = "Failed, size of data mismatch";
        return true;
      }
      vh.push_back(*reinterpret_cast<const crypto::hash*>(b.data()));
    }
    std::vector<crypto::hash> missed_txs;
    std::vector<std::tuple<crypto::hash, cryptonote::blobdata, crypto::hash, cryptonote::blobdata>> txs;
    bool r = m_core.get_split_transactions_blobs(vh, txs, missed_txs);
    if(!r)
    {
      res.status = "Failed";
      return true;
    }
    LOG_PRINT_L2("Found " << txs.size() << "/" << vh.size() << " transactions on the blockchain");

    // try the pool for any missing txes
    size_t found_in_pool = 0;
    std::unordered_set<crypto::hash> pool_tx_hashes;
    std::unordered_map<crypto::hash, tx_info> per_tx_pool_tx_info;
    if (!missed_txs.empty())
    {
      std::vector<tx_info> pool_tx_info;
      std::vector<spent_key_image_info> pool_key_image_info;
      bool r = m_core.get_pool_transactions_and_spent_keys_info(pool_tx_info, pool_key_image_info);
      if(r)
      {
        // sort to match original request
        std::vector<std::tuple<crypto::hash, cryptonote::blobdata, crypto::hash, cryptonote::blobdata>> sorted_txs;
        std::vector<tx_info>::const_iterator i;
        unsigned txs_processed = 0;
        for (const crypto::hash &h: vh)
        {
          if (std::find(missed_txs.begin(), missed_txs.end(), h) == missed_txs.end())
          {
            if (txs.size() == txs_processed)
            {
              res.status = "Failed: internal error - txs is empty";
              return true;
            }
            // core returns the ones it finds in the right order
            if (std::get<0>(txs[txs_processed]) != h)
            {
              res.status = "Failed: tx hash mismatch";
              return true;
            }
            sorted_txs.push_back(std::move(txs[txs_processed]));
            ++txs_processed;
          }
          else if ((i = std::find_if(pool_tx_info.begin(), pool_tx_info.end(), [h](const tx_info &txi) { return epee::string_tools::pod_to_hex(h) == txi.id_hash; })) != pool_tx_info.end())
          {
            cryptonote::transaction tx;
            if (!cryptonote::parse_and_validate_tx_from_blob(i->tx_blob, tx))
            {
              res.status = "Failed to parse and validate tx from blob";
              return true;
            }
            std::stringstream ss;
            binary_archive<true> ba(ss);
            bool r = const_cast<cryptonote::transaction&>(tx).serialize_base(ba);
            if (!r)
            {
              res.status = "Failed to serialize transaction base";
              return true;
            }
            //TODO: Public
            const cryptonote::blobdata pruned = ss.str();
            sorted_txs.push_back(std::make_tuple(h, pruned, crypto::null_hash, std::string(i->tx_blob, pruned.size())));
            missed_txs.erase(std::find(missed_txs.begin(), missed_txs.end(), h));
            pool_tx_hashes.insert(h);
            const std::string hash_string = epee::string_tools::pod_to_hex(h);
            for (const auto &ti: pool_tx_info)
            {
              if (ti.id_hash == hash_string)
              {
                per_tx_pool_tx_info.insert(std::make_pair(h, ti));
                break;
              }
            }
            ++found_in_pool;
          }
        }
        txs = sorted_txs;
      }
      LOG_PRINT_L2("Found " << found_in_pool << "/" << vh.size() << " transactions in the pool");
    }

    std::vector<std::string>::const_iterator txhi = req.txs_hashes.begin();
    std::vector<crypto::hash>::const_iterator vhi = vh.begin();
    for(auto& tx: txs)
    {
      res.txs.push_back(COMMAND_RPC_GET_TRANSACTIONS::entry());
      COMMAND_RPC_GET_TRANSACTIONS::entry &e = res.txs.back();

      crypto::hash tx_hash = *vhi++;
      e.tx_hash = *txhi++;
      e.prunable_hash = epee::string_tools::pod_to_hex(std::get<2>(tx));
      if (req.split || req.prune || std::get<3>(tx).empty())
      {
        // use splitted form with pruned and prunable (filled only when prune=false and the daemon has it), leaving as_hex as empty
        e.pruned_as_hex = string_tools::buff_to_hex_nodelimer(std::get<1>(tx));
        if (!req.prune)
          e.prunable_as_hex = string_tools::buff_to_hex_nodelimer(std::get<3>(tx));
        if (req.decode_as_json)
        {
          cryptonote::blobdata tx_data;
          cryptonote::transaction t;
          if (req.prune || std::get<3>(tx).empty())
          {
            // decode pruned tx to JSON
            tx_data = std::get<1>(tx);
            if (cryptonote::parse_and_validate_tx_base_from_blob(tx_data, t))
            {
              pruned_transaction pruned_tx{t};
              e.as_json = obj_to_json_str(pruned_tx);
            }
            else
            {
              res.status = "Failed to parse and validate pruned tx from blob";
              return true;
            }
          }
          else
          {
            // decode full tx to JSON
            tx_data = std::get<1>(tx) + std::get<3>(tx);
            if (cryptonote::parse_and_validate_tx_from_blob(tx_data, t))
            {
              e.as_json = obj_to_json_str(t);
            }
            else
            {
              res.status = "Failed to parse and validate tx from blob";
              return true;
            }
          }
        }
      }
      else
      {
        // use non-splitted form, leaving pruned_as_hex and prunable_as_hex as empty
        cryptonote::blobdata tx_data = std::get<1>(tx) + std::get<3>(tx);
        e.as_hex = string_tools::buff_to_hex_nodelimer(tx_data);
        if (req.decode_as_json)
        {
          cryptonote::transaction t;
          if (cryptonote::parse_and_validate_tx_from_blob(tx_data, t))
          {
            e.as_json = obj_to_json_str(t);
          }
          else
          {
            res.status = "Failed to parse and validate tx from blob";
            return true;
          }
        }
      }
      e.in_pool = pool_tx_hashes.find(tx_hash) != pool_tx_hashes.end();
      if (e.in_pool)
      {
        e.block_height = e.block_timestamp = std::numeric_limits<uint64_t>::max();
        auto it = per_tx_pool_tx_info.find(tx_hash);
        if (it != per_tx_pool_tx_info.end())
        {
          e.double_spend_seen = it->second.double_spend_seen;
          e.nonexistent_utxo_seen = it->second.nonexistent_utxo_seen;
          e.relayed = it->second.relayed;
        }
        else
        {
          MERROR("Failed to determine pool info for " << tx_hash);
          e.double_spend_seen = false;
          e.nonexistent_utxo_seen = false;
          e.relayed = false;
        }
      }
      else
      {
        e.block_height = m_core.get_blockchain_storage().get_db().get_tx_block_height(tx_hash);
        e.block_timestamp = m_core.get_blockchain_storage().get_db().get_block_timestamp(e.block_height);
        e.double_spend_seen = false;
        e.relayed = false;
      }

      // fill up old style responses too, in case an old wallet asks
      res.txs_as_hex.push_back(e.as_hex);
      if (req.decode_as_json)
        res.txs_as_json.push_back(e.as_json);

      // output indices too if not in pool
      if (pool_tx_hashes.find(tx_hash) == pool_tx_hashes.end())
      {
        bool r = m_core.get_tx_outputs_gindexs(tx_hash, e.output_indices);
        if (!r)
        {
          res.status = "Failed";
          return false;
        }
      }
    }

    for(const auto& miss_tx: missed_txs)
    {
      res.missed_tx.push_back(string_tools::pod_to_hex(miss_tx));
    }

    LOG_PRINT_L2(res.txs.size() << " transactions found, " << res.missed_tx.size() << " not found");
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_is_key_image_spent(const COMMAND_RPC_IS_KEY_IMAGE_SPENT::request& req, COMMAND_RPC_IS_KEY_IMAGE_SPENT::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_is_key_image_spent);
    bool ok;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_IS_KEY_IMAGE_SPENT>(invoke_http_mode::JON, "/is_key_image_spent", req, res, ok))
      return ok;

    const bool restricted = m_restricted && ctx;
    const bool request_has_rpc_origin = ctx != NULL;
    std::vector<crypto::key_image> key_images;
    for(const auto& ki_hex_str: req.key_images)
    {
      blobdata b;
      if(!string_tools::parse_hexstr_to_binbuff(ki_hex_str, b))
      {
        res.status = "Failed to parse hex representation of key image";
        return true;
      }
      if(b.size() != sizeof(crypto::key_image))
      {
        res.status = "Failed, size of data mismatch";
      }
      key_images.push_back(*reinterpret_cast<const crypto::key_image*>(b.data()));
    }
    std::vector<bool> spent_status;
    bool r = m_core.are_key_images_spent(key_images, spent_status);
    if(!r)
    {
      res.status = "Failed";
      return true;
    }
    res.spent_status.clear();
    for (size_t n = 0; n < spent_status.size(); ++n)
      res.spent_status.push_back(spent_status[n] ? COMMAND_RPC_IS_KEY_IMAGE_SPENT::SPENT_IN_BLOCKCHAIN : COMMAND_RPC_IS_KEY_IMAGE_SPENT::UNSPENT);

    // check the pool too
    std::vector<cryptonote::tx_info> txs;
    std::vector<cryptonote::spent_key_image_info> ki;
    r = m_core.get_pool_transactions_and_spent_keys_info(txs, ki, !request_has_rpc_origin || !restricted);
    if(!r)
    {
      res.status = "Failed";
      return true;
    }
    for (std::vector<cryptonote::spent_key_image_info>::const_iterator i = ki.begin(); i != ki.end(); ++i)
    {
      crypto::hash hash;
      crypto::key_image spent_key_image;
      if (parse_hash256(i->id_hash, hash))
      {
        memcpy(&spent_key_image, &hash, sizeof(hash)); // a bit dodgy, should be other parse functions somewhere
        for (size_t n = 0; n < res.spent_status.size(); ++n)
        {
          if (res.spent_status[n] == COMMAND_RPC_IS_KEY_IMAGE_SPENT::UNSPENT)
          {
            if (key_images[n] == spent_key_image)
            {
              res.spent_status[n] = COMMAND_RPC_IS_KEY_IMAGE_SPENT::SPENT_IN_POOL;
              break;
            }
          }
        }
      }
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_send_raw_tx(const COMMAND_RPC_SEND_RAW_TX::request& req, COMMAND_RPC_SEND_RAW_TX::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_send_raw_tx);
    bool ok;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_SEND_RAW_TX>(invoke_http_mode::JON, "/sendrawtransaction", req, res, ok))
      return ok;

    CHECK_CORE_READY();

    std::string tx_blob;
    if(!string_tools::parse_hexstr_to_binbuff(req.tx_as_hex, tx_blob))
    {
      LOG_PRINT_L0("[on_send_raw_tx]: Failed to parse tx from hexbuff: " << req.tx_as_hex);
      res.status = "Failed";
      return true;
    }

    if (req.do_sanity_checks && !cryptonote::tx_sanity_check(m_core.get_blockchain_storage(), tx_blob))
    {
      res.status = "Failed";
      res.reason = "Sanity check failed";
      res.sanity_check_failed = true;
      return true;
    }
    res.sanity_check_failed = false;

    cryptonote_connection_context fake_context = AUTO_VAL_INIT(fake_context);
    tx_verification_context tvc = AUTO_VAL_INIT(tvc);
    if(!m_core.handle_incoming_tx(tx_blob, tvc, false, false, req.do_not_relay) || tvc.m_verification_failed)
    {
      res.status = "Failed";
      std::string reason = "";
      if ((res.low_mixin = tvc.m_low_mixin))
        add_reason(reason, "bad ring size");
      if ((res.double_spend = tvc.m_double_spend))
        add_reason(reason, "double spend");
      if ((res.double_spend = tvc.m_utxo_nonexistent))
        add_reason(reason, "utxo has been spent already or doesn't exist");
      if ((res.invalid_input = tvc.m_invalid_input))
        add_reason(reason, "invalid input");
      if ((res.invalid_output = tvc.m_invalid_output))
        add_reason(reason, "invalid output");
      if ((res.too_big = tvc.m_too_big))
        add_reason(reason, "too big");
      if ((res.overspend = tvc.m_overspend))
        add_reason(reason, "overspend");
      if ((res.fee_too_low = tvc.m_fee_too_low))
        add_reason(reason, "fee too low");
      if ((res.not_rct = tvc.m_not_rct))
        add_reason(reason, "tx is not ringct");
      const std::string punctuation = reason.empty() ? "" : ": ";
      if (tvc.m_verification_failed)
      {
        LOG_PRINT_L0("[on_send_raw_tx]: tx verification failed" << punctuation << reason);
      }
      else
      {
        LOG_PRINT_L0("[on_send_raw_tx]: Failed to process tx" << punctuation << reason);
      }
      return true;
    }

    if(!tvc.m_should_be_relayed)
    {
      LOG_PRINT_L0("[on_send_raw_tx]: tx accepted, but not relayed");
      res.reason = "Not relayed";
      res.not_relayed = true;
      res.status = CORE_RPC_STATUS_OK;
      return true;
    }

    NOTIFY_NEW_TRANSACTIONS::request r;
    r.txs.push_back(tx_blob);
    m_core.get_protocol()->relay_transactions(r, fake_context);
    //TODO: make sure that tx has reached other nodes here, probably wait to receive reflections from other nodes
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_start_mining(const COMMAND_RPC_START_MINING::request& req, COMMAND_RPC_START_MINING::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_start_mining);
    CHECK_CORE_READY();
    cryptonote::address_parse_info info;
    if(!get_account_address_from_str(info, nettype(), req.miner_address))
    {
      res.status = "Failed, wrong address";
      LOG_PRINT_L0(res.status);
      return true;
    }
    if (info.is_subaddress)
    {
      res.status = "Mining to subaddress isn't supported yet";
      LOG_PRINT_L0(res.status);
      return true;
    }

    unsigned int concurrency_count = boost::thread::hardware_concurrency() * 4;

    // if we couldn't detect threads, set it to a ridiculously high number
    if(concurrency_count == 0)
    {
      concurrency_count = 257;
    }

    // if there are more threads requested than the hardware supports
    // then we fail and log that.
    if(req.threads_count > concurrency_count)
    {
      res.status = "Failed, too many threads relative to CPU cores.";
      LOG_PRINT_L0(res.status);
      return true;
    }

    cryptonote::miner &miner= m_core.get_miner();
    if (miner.is_mining())
    {
      res.status = "Already mining";
      return true;
    }
    if(!miner.start(info.address, static_cast<size_t>(req.threads_count), req.do_background_mining, req.ignore_battery))
    {
      res.status = "Failed, mining not started";
      LOG_PRINT_L0(res.status);
      return true;
    }
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_stop_mining(const COMMAND_RPC_STOP_MINING::request& req, COMMAND_RPC_STOP_MINING::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_stop_mining);
    cryptonote::miner &miner= m_core.get_miner();
    if(!miner.is_mining())
    {
      res.status = "Mining never started";
      LOG_PRINT_L0(res.status);
      return true;
    }
    if(!miner.stop())
    {
      res.status = "Failed, mining not stopped";
      LOG_PRINT_L0(res.status);
      return true;
    }
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_mining_status(const COMMAND_RPC_MINING_STATUS::request& req, COMMAND_RPC_MINING_STATUS::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_mining_status);

    const miner& lMiner = m_core.get_miner();
    res.active = lMiner.is_mining();
    res.is_background_mining_enabled = lMiner.get_is_background_mining_enabled();
    store_difficulty(m_core.get_blockchain_storage().get_difficulty_for_next_block(), res.difficulty, res.wide_difficulty, res.difficulty_top64);

    res.block_target = m_core.get_blockchain_storage().get_current_hard_fork_version() < 6 ? DIFFICULTY_TARGET : DIFFICULTY_TARGET_V6;
    if ( lMiner.is_mining() ) {
      res.speed = lMiner.get_speed();
      res.threads_count = lMiner.get_threads_count();
      res.block_reward = lMiner.get_block_reward();
    }
    const account_public_address& lMiningAdr = lMiner.get_mining_address();
    res.address = get_account_address_as_str(nettype(), false, lMiningAdr);
    const uint8_t major_version = m_core.get_blockchain_storage().get_current_hard_fork_version();
    const unsigned variant = major_version >= 7 ? major_version - 6 : 0;
    switch (variant)
    {
      case 0: res.pow_algorithm = "Cryptonight"; break;
      case 1: res.pow_algorithm = "CNv1 (Cryptonight variant 1)"; break;
      case 2: case 3: res.pow_algorithm = "CNv2 (Cryptonight variant 2)"; break;
      case 4: case 5: res.pow_algorithm = "CNv4 (Cryptonight variant 4)"; break;
      default: res.pow_algorithm = "I'm not sure actually"; break;
    }
    if (res.is_background_mining_enabled)
    {
      res.bg_idle_threshold = lMiner.get_idle_threshold();
      res.bg_min_idle_seconds = lMiner.get_min_idle_seconds();
      res.bg_ignore_battery = lMiner.get_ignore_battery();
      res.bg_target = lMiner.get_mining_target();
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_save_bc(const COMMAND_RPC_SAVE_BC::request& req, COMMAND_RPC_SAVE_BC::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_save_bc);
    if( !m_core.get_blockchain_storage().store_blockchain() )
    {
      res.status = "Error while storing blockchain";
      return true;
    }
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_peer_list(const COMMAND_RPC_GET_PEER_LIST::request& req, COMMAND_RPC_GET_PEER_LIST::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_peer_list);
    std::vector<nodetool::peerlist_entry> white_list;
    std::vector<nodetool::peerlist_entry> gray_list;
    m_p2p.get_public_peerlist(gray_list, white_list);

    res.white_list.reserve(white_list.size());
    for (auto & entry : white_list)
    {
      if (entry.adr.get_type_id() == epee::net_utils::ipv4_network_address::get_type_id())
        res.white_list.emplace_back(entry.id, entry.adr.as<epee::net_utils::ipv4_network_address>().ip(),
            entry.adr.as<epee::net_utils::ipv4_network_address>().port(), entry.last_seen, entry.pruning_seed, entry.rpc_port);
      else
        res.white_list.emplace_back(entry.id, entry.adr.str(), entry.last_seen, entry.pruning_seed, entry.rpc_port);
    }

    res.gray_list.reserve(gray_list.size());
    for (auto & entry : gray_list)
    {
      if (entry.adr.get_type_id() == epee::net_utils::ipv4_network_address::get_type_id())
        res.gray_list.emplace_back(entry.id, entry.adr.as<epee::net_utils::ipv4_network_address>().ip(),
            entry.adr.as<epee::net_utils::ipv4_network_address>().port(), entry.last_seen, entry.pruning_seed, entry.rpc_port);
      else
        res.gray_list.emplace_back(entry.id, entry.adr.str(), entry.last_seen, entry.pruning_seed, entry.rpc_port);
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_set_log_hash_rate(const COMMAND_RPC_SET_LOG_HASH_RATE::request& req, COMMAND_RPC_SET_LOG_HASH_RATE::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_set_log_hash_rate);
    if(m_core.get_miner().is_mining())
    {
      m_core.get_miner().do_print_hashrate(req.visible);
      res.status = CORE_RPC_STATUS_OK;
    }
    else
    {
      res.status = CORE_RPC_STATUS_NOT_MINING;
    }
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_set_log_level(const COMMAND_RPC_SET_LOG_LEVEL::request& req, COMMAND_RPC_SET_LOG_LEVEL::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_set_log_level);
    if (req.level > 4)
    {
      res.status = "Error: log level not valid";
      return true;
    }
    mlog_set_log_level(req.level);
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_set_log_categories(const COMMAND_RPC_SET_LOG_CATEGORIES::request& req, COMMAND_RPC_SET_LOG_CATEGORIES::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_set_log_categories);
    mlog_set_log(req.categories.c_str());
    res.categories = mlog_get_categories();
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_transaction_pool(const COMMAND_RPC_GET_TRANSACTION_POOL::request& req, COMMAND_RPC_GET_TRANSACTION_POOL::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_transaction_pool);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_TRANSACTION_POOL>(invoke_http_mode::JON, "/get_transaction_pool", req, res, r))
      return r;

    const bool restricted = m_restricted && ctx;
    const bool request_has_rpc_origin = ctx != NULL;
    m_core.get_pool_transactions_and_spent_keys_info(res.transactions, res.spent_key_images, !request_has_rpc_origin || !restricted);
    for (tx_info& txi : res.transactions)
      txi.tx_blob = epee::string_tools::buff_to_hex_nodelimer(txi.tx_blob);
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_transaction_pool_hashes_bin(const COMMAND_RPC_GET_TRANSACTION_POOL_HASHES_BIN::request& req, COMMAND_RPC_GET_TRANSACTION_POOL_HASHES_BIN::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_transaction_pool_hashes);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_TRANSACTION_POOL_HASHES_BIN>(invoke_http_mode::JON, "/get_transaction_pool_hashes.bin", req, res, r))
      return r;

    const bool restricted = m_restricted && ctx;
    const bool request_has_rpc_origin = ctx != NULL;
    m_core.get_pool_transaction_hashes(res.tx_hashes, !request_has_rpc_origin || !restricted);
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_transaction_pool_hashes(const COMMAND_RPC_GET_TRANSACTION_POOL_HASHES::request& req, COMMAND_RPC_GET_TRANSACTION_POOL_HASHES::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_transaction_pool_hashes);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_TRANSACTION_POOL_HASHES>(invoke_http_mode::JON, "/get_transaction_pool_hashes", req, res, r))
      return r;

    const bool restricted = m_restricted && ctx;
    const bool request_has_rpc_origin = ctx != NULL;
    std::vector<crypto::hash> tx_hashes;
    m_core.get_pool_transaction_hashes(tx_hashes, !request_has_rpc_origin || !restricted);
    res.tx_hashes.reserve(tx_hashes.size());
    for (const crypto::hash &tx_hash: tx_hashes)
      res.tx_hashes.push_back(epee::string_tools::pod_to_hex(tx_hash));
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_transaction_pool_stats(const COMMAND_RPC_GET_TRANSACTION_POOL_STATS::request& req, COMMAND_RPC_GET_TRANSACTION_POOL_STATS::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_transaction_pool_stats);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_TRANSACTION_POOL_STATS>(invoke_http_mode::JON, "/get_transaction_pool_stats", req, res, r))
      return r;

    const bool restricted = m_restricted && ctx;
    const bool request_has_rpc_origin = ctx != NULL;
    m_core.get_pool_transaction_stats(res.pool_stats, !request_has_rpc_origin || !restricted);
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_stop_daemon(const COMMAND_RPC_STOP_DAEMON::request& req, COMMAND_RPC_STOP_DAEMON::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_stop_daemon);
    // FIXME: replace back to original m_p2p.send_stop_signal() after
    // investigating why that isn't working quite right.
    m_p2p.send_stop_signal();
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_getblockcount(const COMMAND_RPC_GETBLOCKCOUNT::request& req, COMMAND_RPC_GETBLOCKCOUNT::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_getblockcount);
    {
      boost::shared_lock<boost::shared_mutex> lock(m_bootstrap_daemon_mutex);
      if (m_should_use_bootstrap_daemon)
      {
        res.status = "This command is unsupported for bootstrap daemon";
        return false;
      }
    }
    res.count = m_core.get_current_blockchain_height();
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_getblockhash(const COMMAND_RPC_GETBLOCKHASH::request& req, COMMAND_RPC_GETBLOCKHASH::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_getblockhash);
    {
      boost::shared_lock<boost::shared_mutex> lock(m_bootstrap_daemon_mutex);
      if (m_should_use_bootstrap_daemon)
      {
        res = "This command is unsupported for bootstrap daemon";
        return false;
      }
    }
    if(req.size() != 1)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_WRONG_PARAM;
      error_resp.message = "Wrong parameters, expected height";
      return false;
    }
    uint64_t h = req[0];
    if(m_core.get_current_blockchain_height() <= h)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT;
      error_resp.message = std::string("Requested block height: ") + std::to_string(h) + " greater than current top block height: " +  std::to_string(m_core.get_current_blockchain_height() - 1);
    }
    res = string_tools::pod_to_hex(m_core.get_block_id_by_height(h));
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  // equivalent of strstr, but with arbitrary bytes (ie, NULs)
  // This does not differentiate between "not found" and "found at offset 0"
  size_t slow_memmem(const void* start_buff, size_t buflen,const void* pat,size_t patlen)
  {
    const void* buf = start_buff;
    const void* end=(const char*)buf+buflen;
    if (patlen > buflen || patlen == 0) return 0;
    while(buflen>0 && (buf=memchr(buf,((const char*)pat)[0],buflen-patlen+1)))
    {
      if(memcmp(buf,pat,patlen)==0)
        return (const char*)buf - (const char*)start_buff;
      buf=(const char*)buf+1;
      buflen = (const char*)end - (const char*)buf;
    }
    return 0;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_getblocktemplate(const COMMAND_RPC_GETBLOCKTEMPLATE::request& req, COMMAND_RPC_GETBLOCKTEMPLATE::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_getblocktemplate);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GETBLOCKTEMPLATE>(invoke_http_mode::JON_RPC, "getblocktemplate", req, res, r))
      return r;

    if(!check_core_ready())
    {
      error_resp.code = CORE_RPC_ERROR_CODE_CORE_BUSY;
      error_resp.message = "Core is busy";
      return false;
    }

    if(req.reserve_size > 255)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_TOO_BIG_RESERVE_SIZE;
      error_resp.message = "Too big reserved size, maximum 255";
      return false;
    }

    cryptonote::address_parse_info info;

    if(!req.wallet_address.size() || !cryptonote::get_account_address_from_str(info, nettype(), req.wallet_address))
    {
      error_resp.code = CORE_RPC_ERROR_CODE_WRONG_WALLET_ADDRESS;
      error_resp.message = "Failed to parse wallet address";
      return false;
    }
    if (info.is_subaddress)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_MINING_TO_SUBADDRESS;
      error_resp.message = "Mining to subaddress is not supported yet";
      return false;
    }

    block b;
    cryptonote::blobdata blob_reserve;
    blob_reserve.resize(req.reserve_size, 0);
    cryptonote::difficulty_type wdiff;
    crypto::hash prev_block;
    if (!req.prev_block.empty())
    {
      if (!epee::string_tools::hex_to_pod(req.prev_block, prev_block))
      {
        error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
        error_resp.message = "Invalid prev_block";
        return false;
      }
    }
    if(!m_core.get_block_template(b, req.prev_block.empty() ? NULL : &prev_block, info.address, wdiff, res.height, res.expected_reward, blob_reserve))
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: failed to create block template";
      LOG_ERROR("Failed to create block template");
      return false;
    }
    store_difficulty(wdiff, res.difficulty, res.wide_difficulty, res.difficulty_top64);
    blobdata block_blob = t_serializable_object_to_blob(b);
    crypto::public_key tx_pub_key = cryptonote::get_tx_pub_key_from_extra(b.miner_tx);
    if(tx_pub_key == crypto::null_pkey)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: failed to create block template";
      LOG_ERROR("Failed to get tx pub key in coinbase extra");
      return false;
    }
    res.reserved_offset = slow_memmem((void*)block_blob.data(), block_blob.size(), &tx_pub_key, sizeof(tx_pub_key));
    if(!res.reserved_offset)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: failed to create block template";
      LOG_ERROR("Failed to find tx pub key in blockblob");
      return false;
    }
    if (req.reserve_size)
      res.reserved_offset += sizeof(tx_pub_key) + 2; //2 bytes: tag for TX_EXTRA_NONCE(1 byte), counter in TX_EXTRA_NONCE(1 byte)
    else
      res.reserved_offset = 0;
    if(res.reserved_offset + req.reserve_size > block_blob.size())
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: failed to create block template";
      LOG_ERROR("Failed to calculate offset for ");
      return false;
    }
    blobdata hashing_blob = get_block_hashing_blob(b);
    res.prev_hash = string_tools::pod_to_hex(b.prev_id);
    res.blocktemplate_blob = string_tools::buff_to_hex_nodelimer(block_blob);
    res.blockhashing_blob =  string_tools::buff_to_hex_nodelimer(hashing_blob);
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_submitblock(const COMMAND_RPC_SUBMITBLOCK::request& req, COMMAND_RPC_SUBMITBLOCK::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_submitblock);
    {
      boost::shared_lock<boost::shared_mutex> lock(m_bootstrap_daemon_mutex);
      if (m_should_use_bootstrap_daemon)
      {
        res.status = "This command is unsupported for bootstrap daemon";
        return false;
      }
    }
    CHECK_CORE_READY();
    if(req.size()!=1)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_WRONG_PARAM;
      error_resp.message = "Wrong param";
      return false;
    }
    blobdata blockblob;
    if(!string_tools::parse_hexstr_to_binbuff(req[0], blockblob))
    {
      error_resp.code = CORE_RPC_ERROR_CODE_WRONG_BLOCKBLOB;
      error_resp.message = "Wrong block blob";
      return false;
    }
    
    // Fixing of high orphan issue for most pools
    // Thanks Boolberry!
    block b;
    if(!parse_and_validate_block_from_blob(blockblob, b))
    {
      error_resp.code = CORE_RPC_ERROR_CODE_WRONG_BLOCKBLOB;
      error_resp.message = "Wrong block blob";
      return false;
    }

    // Fix from Boolberry neglects to check block
    // size, do that with the function below
    if(!m_core.check_incoming_block_size(blockblob))
    {
      error_resp.code = CORE_RPC_ERROR_CODE_WRONG_BLOCKBLOB_SIZE;
      error_resp.message = "Block bloc size is too big, rejecting block";
      return false;
    }

    block_verification_context bvc;
    if(!m_core.handle_block_found(b, bvc))
    {
      error_resp.code = CORE_RPC_ERROR_CODE_BLOCK_NOT_ACCEPTED;
      error_resp.message = "Block not accepted";
      return false;
    }
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_generateblocks(const COMMAND_RPC_GENERATEBLOCKS::request& req, COMMAND_RPC_GENERATEBLOCKS::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_generateblocks);

    CHECK_CORE_READY();

    res.status = CORE_RPC_STATUS_OK;

    if(m_core.get_nettype() != FAKECHAIN)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_REGTEST_REQUIRED;
      error_resp.message = "Regtest required when generating blocks";
      return false;
    }

    COMMAND_RPC_GETBLOCKTEMPLATE::request template_req;
    COMMAND_RPC_GETBLOCKTEMPLATE::response template_res;
    COMMAND_RPC_SUBMITBLOCK::request submit_req;
    COMMAND_RPC_SUBMITBLOCK::response submit_res;

    template_req.reserve_size = 1;
    template_req.wallet_address = req.wallet_address;
    template_req.prev_block = req.prev_block;
    submit_req.push_back(boost::value_initialized<std::string>());
    res.height = m_core.get_blockchain_storage().get_current_blockchain_height();

    for(size_t i = 0; i < req.amount_of_blocks; i++)
    {
      bool r = on_getblocktemplate(template_req, template_res, error_resp, ctx);
      res.status = template_res.status;
      template_req.prev_block.clear();

      if (!r) return false;

      blobdata blockblob;
      if(!string_tools::parse_hexstr_to_binbuff(template_res.blocktemplate_blob, blockblob))
      {
        error_resp.code = CORE_RPC_ERROR_CODE_WRONG_BLOCKBLOB;
        error_resp.message = "Wrong block blob";
        return false;
      }
      block b;
      if(!parse_and_validate_block_from_blob(blockblob, b))
      {
        error_resp.code = CORE_RPC_ERROR_CODE_WRONG_BLOCKBLOB;
        error_resp.message = "Wrong block blob";
        return false;
      }
      b.nonce = req.starting_nonce;
      miner::find_nonce_for_given_block(b, template_res.difficulty, template_res.height);

      submit_req.front() = string_tools::buff_to_hex_nodelimer(block_to_blob(b));
      r = on_submitblock(submit_req, submit_res, error_resp, ctx);
      res.status = submit_res.status;

      if (!r) return false;

      res.blocks.push_back(epee::string_tools::pod_to_hex(get_block_hash(b)));
      template_req.prev_block = res.blocks.back();
      res.height = template_res.height;
    }

    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  uint64_t core_rpc_server::get_block_reward(const block& blk)
  {
    uint64_t reward = 0;
    for(const tx_out& out: blk.miner_tx.vout)
    {
      reward += out.amount;
    }
    return reward;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::fill_block_header_response(const block& blk, bool orphan_status, uint64_t height, const crypto::hash& hash, block_header_response& response, bool fill_pow_hash)
  {
    PERF_TIMER(fill_block_header_response);
    response.major_version = blk.major_version;
    response.minor_version = blk.minor_version;
    response.timestamp = blk.timestamp;
    response.prev_hash = string_tools::pod_to_hex(blk.prev_id);
    response.nonce = blk.nonce;
    response.orphan_status = orphan_status;
    response.height = height;
    response.depth = m_core.get_current_blockchain_height() - height - 1;
    response.hash = string_tools::pod_to_hex(hash);
    store_difficulty(m_core.get_blockchain_storage().block_difficulty(height),
        response.difficulty, response.wide_difficulty, response.difficulty_top64);
    store_difficulty(m_core.get_blockchain_storage().get_db().get_block_cumulative_difficulty(height),
        response.cumulative_difficulty, response.wide_cumulative_difficulty, response.cumulative_difficulty_top64);
    response.reward = get_block_reward(blk);
    response.block_size = response.block_weight = m_core.get_blockchain_storage().get_db().get_block_weight(height);
    response.num_txes = blk.tx_hashes.size();
    response.pow_hash = fill_pow_hash ? string_tools::pod_to_hex(get_block_longhash(blk, height)) : "";
    response.long_term_weight = m_core.get_blockchain_storage().get_db().get_block_long_term_weight(height);
    response.miner_tx_hash = string_tools::pod_to_hex(cryptonote::get_transaction_hash(blk.miner_tx));
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  template <typename COMMAND_TYPE>
  bool core_rpc_server::use_bootstrap_daemon_if_necessary(const invoke_http_mode &mode, const std::string &command_name, const typename COMMAND_TYPE::request& req, typename COMMAND_TYPE::response& res, bool &r)
  {
    res.untrusted = false;
    if (m_bootstrap_daemon_address.empty())
      return false;

    boost::unique_lock<boost::shared_mutex> lock(m_bootstrap_daemon_mutex);
    if (!m_should_use_bootstrap_daemon)
    {
      MINFO("The local daemon is fully synced. Not switching back to the bootstrap daemon");
      return false;
    }

    auto current_time = std::chrono::system_clock::now();
    if (current_time - m_bootstrap_height_check_time > std::chrono::seconds(30))  // update every 30s
    {
      m_bootstrap_height_check_time = current_time;

      uint64_t top_height;
      crypto::hash top_hash;
      m_core.get_blockchain_top(top_height, top_hash);
      ++top_height; // turn top block height into blockchain height

      // query bootstrap daemon's height
      cryptonote::COMMAND_RPC_GET_HEIGHT::request getheight_req;
      cryptonote::COMMAND_RPC_GET_HEIGHT::response getheight_res;
      bool ok = epee::net_utils::invoke_http_json("/getheight", getheight_req, getheight_res, m_http_client);
      ok = ok && getheight_res.status == CORE_RPC_STATUS_OK;

      m_should_use_bootstrap_daemon = ok && top_height + 10 < getheight_res.height;
      MINFO((m_should_use_bootstrap_daemon ? "Using" : "Not using") << " the bootstrap daemon (our height: " << top_height << ", bootstrap daemon's height: " << getheight_res.height << ")");
    }
    if (!m_should_use_bootstrap_daemon)
      return false;

    if (mode == invoke_http_mode::JON)
    {
      r = epee::net_utils::invoke_http_json(command_name, req, res, m_http_client);
    }
    else if (mode == invoke_http_mode::BIN)
    {
      r = epee::net_utils::invoke_http_bin(command_name, req, res, m_http_client);
    }
    else if (mode == invoke_http_mode::JON_RPC)
    {
      epee::json_rpc::request<typename COMMAND_TYPE::request> json_req = AUTO_VAL_INIT(json_req);
      epee::json_rpc::response<typename COMMAND_TYPE::response, std::string> json_resp = AUTO_VAL_INIT(json_resp);
      json_req.jsonrpc = "2.0";
      json_req.id = epee::serialization::storage_entry(0);
      json_req.method = command_name;
      json_req.params = req;
      r = net_utils::invoke_http_json("/json_rpc", json_req, json_resp, m_http_client);
      if (r)
        res = json_resp.result;
    }
    else
    {
      MERROR("Unknown invoke_http_mode: " << mode);
      return false;
    }
    m_was_bootstrap_ever_used = true;
    r = r && res.status == CORE_RPC_STATUS_OK;
    res.untrusted = true;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_last_block_header(const COMMAND_RPC_GET_LAST_BLOCK_HEADER::request& req, COMMAND_RPC_GET_LAST_BLOCK_HEADER::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_last_block_header);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_LAST_BLOCK_HEADER>(invoke_http_mode::JON_RPC, "getlastblockheader", req, res, r))
      return r;

    CHECK_CORE_READY();
    uint64_t last_block_height;
    crypto::hash last_block_hash;
    m_core.get_blockchain_top(last_block_height, last_block_hash);
    block last_block;
    bool have_last_block = m_core.get_block_by_hash(last_block_hash, last_block);
    if (!have_last_block)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: can't get last block.";
      return false;
    }
    const bool restricted = m_restricted && ctx;
    bool response_filled = fill_block_header_response(last_block, false, last_block_height, last_block_hash, res.block_header, req.fill_pow_hash && !restricted);
    if (!response_filled)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: can't produce valid response.";
      return false;
    }
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_block_header_by_hash(const COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH::request& req, COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_block_header_by_hash);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH>(invoke_http_mode::JON_RPC, "getblockheaderbyhash", req, res, r))
      return r;

    crypto::hash block_hash;
    bool hash_parsed = parse_hash256(req.hash, block_hash);
    if(!hash_parsed)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_WRONG_PARAM;
      error_resp.message = "Failed to parse hex representation of block hash. Hex = " + req.hash + '.';
      return false;
    }
    block blk;
    bool orphan = false;
    bool have_block = m_core.get_block_by_hash(block_hash, blk, &orphan);
    if (!have_block)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: can't get block by hash. Hash = " + req.hash + '.';
      return false;
    }
    if (blk.miner_tx.vin.size() != 1 || blk.miner_tx.vin.front().type() != typeid(txin_gen))
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: coinbase transaction in the block has the wrong type";
      return false;
    }
    uint64_t block_height = boost::get<txin_gen>(blk.miner_tx.vin.front()).height;
    const bool restricted = m_restricted && ctx;
    bool response_filled = fill_block_header_response(blk, orphan, block_height, block_hash, res.block_header, req.fill_pow_hash && !restricted);
    if (!response_filled)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: can't produce valid response.";
      return false;
    }
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_block_headers_range(const COMMAND_RPC_GET_BLOCK_HEADERS_RANGE::request& req, COMMAND_RPC_GET_BLOCK_HEADERS_RANGE::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_block_headers_range);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_BLOCK_HEADERS_RANGE>(invoke_http_mode::JON_RPC, "getblockheadersrange", req, res, r))
      return r;

    const uint64_t bc_height = m_core.get_current_blockchain_height();
    if (req.start_height >= bc_height || req.end_height >= bc_height || req.start_height > req.end_height)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT;
      error_resp.message = "Invalid start/end heights.";
      return false;
    }
    for (uint64_t h = req.start_height; h <= req.end_height; ++h)
    {
      crypto::hash block_hash = m_core.get_block_id_by_height(h);
      block blk;
      bool have_block = m_core.get_block_by_hash(block_hash, blk);
      if (!have_block)
      {
        error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
        error_resp.message = "Internal error: can't get block by height. Height = " + boost::lexical_cast<std::string>(h) + ". Hash = " + epee::string_tools::pod_to_hex(block_hash) + '.';
        return false;
      }
      if (blk.miner_tx.vin.size() != 1 || blk.miner_tx.vin.front().type() != typeid(txin_gen))
      {
        error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
        error_resp.message = "Internal error: coinbase transaction in the block has the wrong type";
        return false;
      }
      uint64_t block_height = boost::get<txin_gen>(blk.miner_tx.vin.front()).height;
      if (block_height != h)
      {
        error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
        error_resp.message = "Internal error: coinbase transaction in the block has the wrong height";
        return false;
      }
      res.headers.push_back(block_header_response());
      const bool restricted = m_restricted && ctx;
      bool response_filled = fill_block_header_response(blk, false, block_height, block_hash, res.headers.back(), req.fill_pow_hash && !restricted);
      if (!response_filled)
      {
        error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
        error_resp.message = "Internal error: can't produce valid response.";
        return false;
      }
    }
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_block_header_by_height(const COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT::request& req, COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_block_header_by_height);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT>(invoke_http_mode::JON_RPC, "getblockheaderbyheight", req, res, r))
      return r;

    if(m_core.get_current_blockchain_height() <= req.height)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT;
      error_resp.message = std::string("Requested block height: ") + std::to_string(req.height) + " greater than current top block height: " +  std::to_string(m_core.get_current_blockchain_height() - 1);
      return false;
    }
    crypto::hash block_hash = m_core.get_block_id_by_height(req.height);
    block blk;
    bool have_block = m_core.get_block_by_hash(block_hash, blk);
    if (!have_block)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: can't get block by height. Height = " + std::to_string(req.height) + '.';
      return false;
    }
    const bool restricted = m_restricted && ctx;
    bool response_filled = fill_block_header_response(blk, false, req.height, block_hash, res.block_header, req.fill_pow_hash && !restricted);
    if (!response_filled)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: can't produce valid response.";
      return false;
    }
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_block(const COMMAND_RPC_GET_BLOCK::request& req, COMMAND_RPC_GET_BLOCK::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_block);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_BLOCK>(invoke_http_mode::JON_RPC, "getblock", req, res, r))
      return r;

    crypto::hash block_hash;
    if (!req.hash.empty())
    {
      bool hash_parsed = parse_hash256(req.hash, block_hash);
      if(!hash_parsed)
      {
        error_resp.code = CORE_RPC_ERROR_CODE_WRONG_PARAM;
        error_resp.message = "Failed to parse hex representation of block hash. Hex = " + req.hash + '.';
        return false;
      }
    }
    else
    {
      if(m_core.get_current_blockchain_height() <= req.height)
      {
        error_resp.code = CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT;
        error_resp.message = std::string("Requested block height: ") + std::to_string(req.height) + " greater than current top block height: " +  std::to_string(m_core.get_current_blockchain_height() - 1);
        return false;
      }
      block_hash = m_core.get_block_id_by_height(req.height);
    }
    block blk;
    bool orphan = false;
    bool have_block = m_core.get_block_by_hash(block_hash, blk, &orphan);
    if (!have_block)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: can't get block by hash. Hash = " + req.hash + '.';
      return false;
    }
    if (blk.miner_tx.vin.size() != 1 || blk.miner_tx.vin.front().type() != typeid(txin_gen))
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: coinbase transaction in the block has the wrong type";
      return false;
    }
    uint64_t block_height = boost::get<txin_gen>(blk.miner_tx.vin.front()).height;
    const bool restricted = m_restricted && ctx;
    bool response_filled = fill_block_header_response(blk, orphan, block_height, block_hash, res.block_header, req.fill_pow_hash && !restricted);
    if (!response_filled)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Internal error: can't produce valid response.";
      return false;
    }
    res.miner_tx_hash = epee::string_tools::pod_to_hex(cryptonote::get_transaction_hash(blk.miner_tx));
    for (size_t n = 0; n < blk.tx_hashes.size(); ++n)
    {
      res.tx_hashes.push_back(epee::string_tools::pod_to_hex(blk.tx_hashes[n]));
    }
    res.blob = string_tools::buff_to_hex_nodelimer(t_serializable_object_to_blob(blk));
    res.json = obj_to_json_str(blk);
    res.status = CORE_RPC_STATUS_OK;
    
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_balance(const COMMAND_RPC_GET_BALANCE::request& req, COMMAND_RPC_GET_BALANCE::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_balance);

    CHECK_CORE_READY();

    if (req.etn_address.empty())
    {
      res.status = "Failed: Request attribute <etn_address> is mandatory.";
      return true;
    }

    address_parse_info addr_info;
    if(!get_account_address_from_str(addr_info, nettype(), req.etn_address))
    {
      res.status = "Failed: can't parse address from <etn_address> = " + req.etn_address;
      return true;
    }

    res.balance = m_core.get_balance(addr_info);
    res.status = "OK";

    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_address_batch_history(const COMMAND_RPC_GET_ADDRESS_BATCH_HISTORY::request& req, COMMAND_RPC_GET_ADDRESS_BATCH_HISTORY::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_address_batch_history);
    CHECK_CORE_READY();

    if (req.etn_address.empty())
    {
      res.status = "Failed: Request attribute <etn_address> is mandatory.";
      return true;
    }

    address_parse_info addr_info;
    if(!get_account_address_from_str(addr_info, nettype(), req.etn_address))
    {
      res.status = "Failed: can't parse address from <etn_address> = " + req.etn_address;
      return true;
    }

    try
    {
      std::vector<address_outputs> outs = m_core.get_address_batch_history(addr_info, req.start_out_id, req.batch_size, req.desc);
      if(!outs.empty() && outs.size() > req.batch_size) 
      {
        res.next_out_id = outs.at(outs.size() - 1).out_id;
        res.last_page = false;
        outs.pop_back();
      }
      else
      {
        res.last_page = true;
      }

      for(auto out: outs)
      {
        res.txs.push_back(epee::string_tools::pod_to_hex(out.tx_hash));
      }

      res.status = "OK";
    }
    catch(const std::exception& e)
    {
      res.status = "Failed: " + std::string(e.what());
      return true;
    }
    
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_addr_tx_batch_history(const COMMAND_RPC_GET_ADDR_TX_BATCH_HISTORY::request& req, COMMAND_RPC_GET_ADDR_TX_BATCH_HISTORY::response& res, const connection_context *ctx)
  {
      PERF_TIMER(on_get_addr_tx_batch_history);
      CHECK_CORE_READY();

      if (req.etn_address.empty())
      {
          res.status = "Failed: Request attribute <etn_address> is mandatory.";
          return true;
      }

      address_parse_info addr_info;
      if(!get_account_address_from_str(addr_info, nettype(), req.etn_address))
      {
          res.status = "Failed: can't parse address from <etn_address> = " + req.etn_address;
          return true;
      }

      try
      {
          std::vector<address_txs> addr_txs = m_core.get_addr_tx_batch_history(addr_info, req.start_addr_tx_id, req.batch_size, req.desc);
          if(!addr_txs.empty() && addr_txs.size() > req.batch_size)
          {
              res.next_addr_tx_id = addr_txs.at(addr_txs.size() - 1).addr_tx_id;
              res.last_page = false;
              addr_txs.pop_back();
          }
          else
          {
              res.last_page = true;
          }

          for(auto addr_tx: addr_txs)
          {
              res.txs.push_back(epee::string_tools::pod_to_hex(addr_tx.tx_hash));
          }

          res.status = "OK";
      }
      catch(const std::exception& e)
      {
          res.status = "Failed: " + std::string(e.what());
          return true;
      }

      return true;
  }
    //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_connections(const COMMAND_RPC_GET_CONNECTIONS::request& req, COMMAND_RPC_GET_CONNECTIONS::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_connections);

    res.connections = m_p2p.get_payload_object().get_connections();

    res.status = CORE_RPC_STATUS_OK;

    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_info_json(const COMMAND_RPC_GET_INFO::request& req, COMMAND_RPC_GET_INFO::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    return on_get_info(req, res, ctx);
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_hard_fork_info(const COMMAND_RPC_HARD_FORK_INFO::request& req, COMMAND_RPC_HARD_FORK_INFO::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_hard_fork_info);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_HARD_FORK_INFO>(invoke_http_mode::JON_RPC, "hard_fork_info", req, res, r))
      return r;

    const Blockchain &blockchain = m_core.get_blockchain_storage();
    uint8_t version = req.version > 0 ? req.version : blockchain.get_next_hard_fork_version();
    res.version = blockchain.get_current_hard_fork_version();
    res.enabled = blockchain.get_hard_fork_voting_info(version, res.window, res.votes, res.threshold, res.earliest_height, res.voting);
    res.state = blockchain.get_hard_fork_state();
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_bans(const COMMAND_RPC_GETBANS::request& req, COMMAND_RPC_GETBANS::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_bans);

    auto now = time(nullptr);
    std::map<std::string, time_t> blocked_hosts = m_p2p.get_blocked_hosts();
    for (std::map<std::string, time_t>::const_iterator i = blocked_hosts.begin(); i != blocked_hosts.end(); ++i)
    {
      if (i->second > now) {
        COMMAND_RPC_GETBANS::ban b;
        b.host = i->first;
        b.ip = 0;
        uint32_t ip;
        if (epee::string_tools::get_ip_int32_from_string(ip, i->first))
          b.ip = ip;
        b.seconds = i->second - now;
        res.bans.push_back(b);
      }
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_set_bans(const COMMAND_RPC_SETBANS::request& req, COMMAND_RPC_SETBANS::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_set_bans);

    for (auto i = req.bans.begin(); i != req.bans.end(); ++i)
    {
      epee::net_utils::network_address na;
      if (!i->host.empty())
      {
        auto na_parsed = net::get_network_address(i->host, 0);
        if (!na_parsed)
        {
          error_resp.code = CORE_RPC_ERROR_CODE_WRONG_PARAM;
          error_resp.message = "Unsupported host type";
          return false;
        }
        na = std::move(*na_parsed);
      }
      else
      {
        na = epee::net_utils::ipv4_network_address{i->ip, 0};
      }
      if (i->ban) {
          if (i->seconds < -1) {
              error_resp.code = CORE_RPC_ERROR_CODE_WRONG_PARAM;
              error_resp.message = "Please pick a value for seconds >= -1";
              return false;
          } else if (i->seconds == -1) {
              m_p2p.block_host(na, uint32_t(std::numeric_limits<time_t>::max()));
          } else {
              m_p2p.block_host(na, uint32_t(i->seconds));
          }
      }
      else{
          m_p2p.unblock_host(na);
      }

    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_flush_txpool(const COMMAND_RPC_FLUSH_TRANSACTION_POOL::request& req, COMMAND_RPC_FLUSH_TRANSACTION_POOL::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_flush_txpool);

    bool failed = false;
    std::vector<crypto::hash> txids;
    if (req.txids.empty())
    {
      std::vector<transaction> pool_txs;
      bool r = m_core.get_pool_transactions(pool_txs);
      if (!r)
      {
        res.status = "Failed to get txpool contents";
        return true;
      }
      for (const auto &tx: pool_txs)
      {
        txids.push_back(cryptonote::get_transaction_hash(tx));
      }
    }
    else
    {
      for (const auto &str: req.txids)
      {
        cryptonote::blobdata txid_data;
        if(!epee::string_tools::parse_hexstr_to_binbuff(str, txid_data))
        {
          failed = true;
        }
        else
        {
          crypto::hash txid = *reinterpret_cast<const crypto::hash*>(txid_data.data());
          txids.push_back(txid);
        }
      }
    }
    if (!m_core.get_blockchain_storage().flush_txes_from_pool(txids))
    {
      res.status = "Failed to remove one or more tx(es)";
      return false;
    }

    if (failed)
    {
      if (txids.empty())
        res.status = "Failed to parse txid";
      else
        res.status = "Failed to parse some of the txids";
      return false;
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_output_histogram(const COMMAND_RPC_GET_OUTPUT_HISTOGRAM::request& req, COMMAND_RPC_GET_OUTPUT_HISTOGRAM::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_output_histogram);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_OUTPUT_HISTOGRAM>(invoke_http_mode::JON_RPC, "get_output_histogram", req, res, r))
      return r;

    const bool restricted = m_restricted && ctx;
    if (restricted && req.recent_cutoff > 0 && req.recent_cutoff < (uint64_t)time(NULL) - OUTPUT_HISTOGRAM_RECENT_CUTOFF_RESTRICTION)
    {
      res.status = "Recent cutoff is too old";
      return true;
    }

    std::map<uint64_t, std::tuple<uint64_t, uint64_t, uint64_t>> histogram;
    try
    {
      histogram = m_core.get_blockchain_storage().get_output_histogram(req.amounts, req.unlocked, req.recent_cutoff, req.min_count);
    }
    catch (const std::exception &e)
    {
      res.status = "Failed to get output histogram";
      return true;
    }

    res.histogram.clear();
    res.histogram.reserve(histogram.size());
    for (const auto &i: histogram)
    {
      if (std::get<0>(i.second) >= req.min_count && (std::get<0>(i.second) <= req.max_count || req.max_count == 0))
        res.histogram.push_back(COMMAND_RPC_GET_OUTPUT_HISTOGRAM::entry(i.first, std::get<0>(i.second), std::get<1>(i.second), std::get<2>(i.second)));
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_version(const COMMAND_RPC_GET_VERSION::request& req, COMMAND_RPC_GET_VERSION::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_version);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_VERSION>(invoke_http_mode::JON_RPC, "get_version", req, res, r))
      return r;

    res.version = CORE_RPC_VERSION;
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_coinbase_tx_sum(const COMMAND_RPC_GET_COINBASE_TX_SUM::request& req, COMMAND_RPC_GET_COINBASE_TX_SUM::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_coinbase_tx_sum);
    std::pair<uint64_t, uint64_t> amounts = m_core.get_coinbase_tx_sum(req.height, req.count);
    res.emission_amount = amounts.first;
    res.fee_amount = amounts.second;
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_base_fee_estimate(const COMMAND_RPC_GET_BASE_FEE_ESTIMATE::request& req, COMMAND_RPC_GET_BASE_FEE_ESTIMATE::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_base_fee_estimate);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_BASE_FEE_ESTIMATE>(invoke_http_mode::JON_RPC, "get_fee_estimate", req, res, r))
      return r;

    res.fee = m_core.get_blockchain_storage().get_dynamic_base_fee_estimate(req.grace_blocks);
    res.quantization_mask = Blockchain::get_fee_quantization_mask();
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_alternate_chains(const COMMAND_RPC_GET_ALTERNATE_CHAINS::request& req, COMMAND_RPC_GET_ALTERNATE_CHAINS::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_alternate_chains);
    try
    {
      std::list<std::pair<Blockchain::block_extended_info, std::vector<crypto::hash>>> chains = m_core.get_blockchain_storage().get_alternative_chains();
      for (const auto &i: chains)
      {
        difficulty_type wdiff = i.first.cumulative_difficulty;
        res.chains.push_back(COMMAND_RPC_GET_ALTERNATE_CHAINS::chain_info{epee::string_tools::pod_to_hex(get_block_hash(i.first.bl)), i.first.height, i.second.size(), 0, "", 0, {}, std::string()});
        store_difficulty(wdiff, res.chains.back().difficulty, res.chains.back().wide_difficulty, res.chains.back().difficulty_top64);
        res.chains.back().block_hashes.reserve(i.second.size());
        for (const crypto::hash &block_id: i.second)
          res.chains.back().block_hashes.push_back(epee::string_tools::pod_to_hex(block_id));
        if (i.first.height < i.second.size())
        {
          res.status = "Error finding alternate chain attachment point";
          return true;
        }
        cryptonote::block main_chain_parent_block;
        try { main_chain_parent_block = m_core.get_blockchain_storage().get_db().get_block_from_height(i.first.height - i.second.size()); }
        catch (const std::exception &e) { res.status = "Error finding alternate chain attachment point"; return true; }
        res.chains.back().main_chain_parent_block = epee::string_tools::pod_to_hex(get_block_hash(main_chain_parent_block));
      }
      res.status = CORE_RPC_STATUS_OK;
    }
    catch (...)
    {
      res.status = "Error retrieving alternate chains";
    }
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_limit(const COMMAND_RPC_GET_LIMIT::request& req, COMMAND_RPC_GET_LIMIT::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_limit);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_LIMIT>(invoke_http_mode::JON, "/get_limit", req, res, r))
      return r;

    res.limit_down = epee::net_utils::connection_basic::get_rate_down_limit();
    res.limit_up = epee::net_utils::connection_basic::get_rate_up_limit();
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_set_limit(const COMMAND_RPC_SET_LIMIT::request& req, COMMAND_RPC_SET_LIMIT::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_set_limit);
    // -1 = reset to default
    //  0 = do not modify

    if (req.limit_down > 0)
    {
      epee::net_utils::connection_basic::set_rate_down_limit(req.limit_down);
    }
    else if (req.limit_down < 0)
    {
      if (req.limit_down != -1)
      {
        res.status = CORE_RPC_ERROR_CODE_WRONG_PARAM;
        return false;
      }
      epee::net_utils::connection_basic::set_rate_down_limit(nodetool::default_limit_down);
    }

    if (req.limit_up > 0)
    {
      epee::net_utils::connection_basic::set_rate_up_limit(req.limit_up);
    }
    else if (req.limit_up < 0)
    {
      if (req.limit_up != -1)
      {
        res.status = CORE_RPC_ERROR_CODE_WRONG_PARAM;
        return false;
      }
      epee::net_utils::connection_basic::set_rate_up_limit(nodetool::default_limit_up);
    }

    res.limit_down = epee::net_utils::connection_basic::get_rate_down_limit();
    res.limit_up = epee::net_utils::connection_basic::get_rate_up_limit();
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_out_peers(const COMMAND_RPC_OUT_PEERS::request& req, COMMAND_RPC_OUT_PEERS::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_out_peers);
    m_p2p.change_max_out_public_peers(req.out_peers);
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_in_peers(const COMMAND_RPC_IN_PEERS::request& req, COMMAND_RPC_IN_PEERS::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_in_peers);
    m_p2p.change_max_in_public_peers(req.in_peers);
    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_start_save_graph(const COMMAND_RPC_START_SAVE_GRAPH::request& req, COMMAND_RPC_START_SAVE_GRAPH::response& res, const connection_context *ctx)
  {
	  PERF_TIMER(on_start_save_graph);
	  m_p2p.set_save_graph(true);
	  res.status = CORE_RPC_STATUS_OK;
	  return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_stop_save_graph(const COMMAND_RPC_STOP_SAVE_GRAPH::request& req, COMMAND_RPC_STOP_SAVE_GRAPH::response& res, const connection_context *ctx)
  {
	  PERF_TIMER(on_stop_save_graph);
	  m_p2p.set_save_graph(false);
	  res.status = CORE_RPC_STATUS_OK;
	  return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_update(const COMMAND_RPC_UPDATE::request& req, COMMAND_RPC_UPDATE::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_update);

    if (m_core.offline())
    {
      res.status = "Daemon is running offline";
      return true;
    }

    static const char software[] = "electroneum";
#ifdef BUILD_TAG
    static const char buildtag[] = BOOST_PP_STRINGIZE(BUILD_TAG);
    static const char subdir[] = "cli";
#else
    static const char buildtag[] = "source";
    static const char subdir[] = "source";
#endif
    LOG_PRINT_L0(req.command << " for software: " << software << "; buildtag: " << buildtag << "; subdir:" << subdir);
    if (req.command != "check" && req.command != "download" && req.command != "update")
    {
      res.status = std::string("unknown command: '") + req.command + "'";
      return true;
    }

    std::string version, hash;
    if (!tools::check_updates(software, buildtag, version, hash))
    {
      res.status = "Error checking for updates";
      return true;
    }
    if (tools::vercmp(version.c_str(), ELECTRONEUM_VERSION) <= 0)
    {
      res.update = false;
      res.status = CORE_RPC_STATUS_OK;
      return true;
    }
    res.update = true;
    res.version = version;
    res.user_uri = tools::get_update_url(software, subdir, buildtag, version, true);
    res.auto_uri = tools::get_update_url(software, subdir, buildtag, version, false);
    res.hash = hash;
    if (req.command == "check")
    {
      res.status = CORE_RPC_STATUS_OK;
      return true;
    }

    boost::filesystem::path path;
    if (req.path.empty())
    {
      std::string filename;
      const char *slash = strrchr(res.auto_uri.c_str(), '/');
      if (slash)
        filename = slash + 1;
      else
        filename = std::string(software) + "-update-" + version;
      path = epee::string_tools::get_current_module_folder();
      path /= filename;
    }
    else
    {
      path = req.path;
    }

    crypto::hash file_hash;
    if (!tools::sha256sum(path.string(), file_hash) || (hash != epee::string_tools::pod_to_hex(file_hash)))
    {
      MDEBUG("We don't have that file already, downloading");
      if (!tools::download(path.string(), res.auto_uri))
      {
        MERROR("Failed to download " << res.auto_uri);
        return false;
      }
      if (!tools::sha256sum(path.string(), file_hash))
      {
        MERROR("Failed to hash " << path);
        return false;
      }
      if (hash != epee::string_tools::pod_to_hex(file_hash))
      {
        MERROR("Download from " << res.auto_uri << " does not match the expected hash");
        return false;
      }
      MINFO("New version downloaded to " << path);
    }
    else
    {
      MDEBUG("We already have " << path << " with expected hash");
    }
    res.path = path.string();

    if (req.command == "download")
    {
      res.status = CORE_RPC_STATUS_OK;
      return true;
    }

    res.status = "'update' not implemented yet";
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_pop_blocks(const COMMAND_RPC_POP_BLOCKS::request& req, COMMAND_RPC_POP_BLOCKS::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_pop_blocks);

    m_core.get_blockchain_storage().pop_blocks(req.nblocks);

    res.height = m_core.get_current_blockchain_height();
    res.status = CORE_RPC_STATUS_OK;

    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_tax_data(const COMMAND_RPC_GET_TAX_DATA::request &req,
                                        COMMAND_RPC_GET_TAX_DATA::response &res, const connection_context *ctx) {
      PERF_TIMER(on_get_tax_data);
      std::string filename =
              "taxes-" + std::to_string(req.start_height) + "-" + std::to_string(req.end_height) + ".csv";
      std::ofstream file(filename);
      // header
      file <<
           boost::format("%64.64s,%64.64s,%64.64s,%64.64s,%64.64s") %
           tr("height") % tr("timestamp") % tr("txid") % tr("fee") % tr("amount")
           << std::endl;

      auto formatter = boost::format("%64.64s,%64.64s,%64.64s,%64.64s,%64.64s");

      std::vector<std::pair<cryptonote::blobdata, block>> blocks;
      size_t num_blocks = req.end_height - req.start_height;
      m_core.get_blocks(req.start_height, num_blocks, blocks);

      uint64_t current_height = req.start_height;
      for (auto block: blocks) {
          std::vector<cryptonote::transaction> found_txs_vec;
          std::vector<crypto::hash> missed_vec;
          uint64_t tx_hash_counter = 0; //needed because of some weird compiler bug
          for(auto hash : block.second.tx_hashes){
            found_txs_vec.push_back(m_core.get_blockchain_storage().get_db().get_tx(hash));
          }
         // m_core.get_transactions(block.second.tx_hashes, found_txs_vec, missed_vec);
          for (transaction tx: found_txs_vec) {
              uint64_t fee = 0;
              if(tx.vin[0].type() == typeid(txin_to_key)){continue;}
              get_tx_fee(tx, fee);
              time_t time_date_stamp = time_t(block.second.timestamp);
              std::stringstream transTime;
              transTime << std::put_time(localtime(&time_date_stamp), "%D - %T");
              std::string myTime = transTime.str();

              unsigned char *byteData = reinterpret_cast<unsigned char*>(tx.hash.data);
              std::string dest;
              std::stringstream hexStringStream;

              hexStringStream << std::hex << std::setfill('0');
              for(size_t index = 0; index < 32; ++index)
                  hexStringStream << std::setw(2) << static_cast<int>(byteData[index]);
              dest = hexStringStream.str();

              file << formatter
                      % current_height
                      % myTime
                      % epee::string_tools::pod_to_hex(block.second.tx_hashes[tx_hash_counter])
                      % std::to_string(((double)fee / 100))
                      % std::to_string((double)(size_t((accumulate(tx.vin.begin(), tx.vin.end(), 0, [](size_t sum, const txin_v& input){ return sum + boost::get<txin_to_key_public>(input).amount; })) - fee)) / 100)
                   << std::endl;
              tx_hash_counter++;
          }
          current_height++;
      }
      file.close();
      res.status = CORE_RPC_STATUS_OK;
      return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------


  bool core_rpc_server::on_relay_tx(const COMMAND_RPC_RELAY_TX::request& req, COMMAND_RPC_RELAY_TX::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_relay_tx);

    bool failed = false;
    res.status = "";
    for (const auto &str: req.txids)
    {
      cryptonote::blobdata txid_data;
      if(!epee::string_tools::parse_hexstr_to_binbuff(str, txid_data))
      {
        if (!res.status.empty()) res.status += ", ";
        res.status += std::string("invalid transaction id: ") + str;
        failed = true;
        continue;
      }
      crypto::hash txid = *reinterpret_cast<const crypto::hash*>(txid_data.data());

      cryptonote::blobdata txblob;
      bool r = m_core.get_pool_transaction(txid, txblob);
      if (r)
      {
        cryptonote_connection_context fake_context = AUTO_VAL_INIT(fake_context);
        NOTIFY_NEW_TRANSACTIONS::request r;
        r.txs.push_back(txblob);
        m_core.get_protocol()->relay_transactions(r, fake_context);
        //TODO: make sure that tx has reached other nodes here, probably wait to receive reflections from other nodes
      }
      else
      {
        if (!res.status.empty()) res.status += ", ";
        res.status += std::string("transaction not found in pool: ") + str;
        failed = true;
        continue;
      }
    }

    if (failed)
    {
      return false;
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_sync_info(const COMMAND_RPC_SYNC_INFO::request& req, COMMAND_RPC_SYNC_INFO::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_sync_info);

    crypto::hash top_hash;
    m_core.get_blockchain_top(res.height, top_hash);
    ++res.height; // turn top block height into blockchain height
    res.target_height = m_core.get_target_blockchain_height();
    res.next_needed_pruning_seed = m_p2p.get_payload_object().get_next_needed_pruning_stripe().second;

    for (const auto &c: m_p2p.get_payload_object().get_connections())
      res.peers.push_back({c});
    const cryptonote::block_queue &block_queue = m_p2p.get_payload_object().get_block_queue();
    block_queue.foreach([&](const cryptonote::block_queue::span &span) {
      const std::string span_connection_id = epee::string_tools::pod_to_hex(span.connection_id);
      uint32_t speed = (uint32_t)(100.0f * block_queue.get_speed(span.connection_id) + 0.5f);
      std::string address = "";
      for (const auto &c: m_p2p.get_payload_object().get_connections())
        if (c.connection_id == span_connection_id)
          address = c.address;
      res.spans.push_back({span.start_block_height, span.nblocks, span_connection_id, (uint32_t)(span.rate + 0.5f), speed, span.size, address});
      return true;
    });
    res.overview = block_queue.get_overview(res.height);

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_txpool_backlog(const COMMAND_RPC_GET_TRANSACTION_POOL_BACKLOG::request& req, COMMAND_RPC_GET_TRANSACTION_POOL_BACKLOG::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_txpool_backlog);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_TRANSACTION_POOL_BACKLOG>(invoke_http_mode::JON_RPC, "get_txpool_backlog", req, res, r))
      return r;

    if (!m_core.get_txpool_backlog(res.backlog))
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Failed to get txpool backlog";
      return false;
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_set_validator_key(const COMMAND_RPC_SET_VALIDATOR_KEY::request& req, COMMAND_RPC_SET_VALIDATOR_KEY::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    if(!m_core.set_validator_key(req.validator_key)) {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Failed to set Validator Key. Wrong format.";
      return false;
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }

  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_generate_ed25519_keypair(const COMMAND_RPC_GENERATE_ED25519_KEYPAIR::request& req, COMMAND_RPC_GENERATE_ED25519_KEYPAIR::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    std::vector<std::string> v = m_core.generate_ed25519_keypair();
    if(v.size() == 0) {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Failed to generate ED25519-Donna keypair.";
      return false;
    }

    res.privateKey = v[0];
    res.publicKey = v[1];

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }

  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_sign_message(const COMMAND_RPC_SIGN_MESSAGE::request& req, COMMAND_RPC_SIGN_MESSAGE::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    try {
        std::string v = m_core.sign_message(boost::algorithm::unhex(req.privateKey), req.message);

        if (v.empty()) {
            error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
            error_resp.message = "Failed to sign message.";
            return false;
        }

        res.signature = v;
    }
    catch(const std::exception &e)
    {
        error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
        error_resp.message = "Failed to sign message. Please check that you are using a valid private key.";
        return false;
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------

  bool core_rpc_server::on_inject_emergency_vlist(const COMMAND_RPC_INJECT_EMERGENCY_VLIST::request& req, COMMAND_RPC_INJECT_EMERGENCY_VLIST::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
      if(!req.blob.empty()) {

      NOTIFY_EMERGENCY_VALIDATORS_LIST::request arg = AUTO_VAL_INIT(arg);
      arg.serialized_v_list = electroneum::basic::store_t_to_json(req);
      cryptonote_connection_context context = boost::value_initialized<cryptonote_connection_context>();


      electroneum::basic::list_update_outcome update_outcome = m_core.set_validators_list(arg.serialized_v_list, true);
        if(update_outcome == electroneum::basic::list_update_outcome::Same_Emergency_List
        || update_outcome == electroneum::basic::list_update_outcome::Emergency_Success ) {
        LOG_PRINT_CCONTEXT_L0("Local list is a legitimate emergency list and will now be relayed.");

          if(m_core.get_protocol()->relay_emergency_validator_list(arg, context)){
            LOG_PRINT_CCONTEXT_L0("List successfully deployed to peers via p2p.");
            return true;
          }
        }
        else {
          LOG_ERROR_CCONTEXT("Received incorrect emergency Validators List for relay.");
          return false;
        }
      }

    return false;
  }

  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_output_distribution(const COMMAND_RPC_GET_OUTPUT_DISTRIBUTION::request& req, COMMAND_RPC_GET_OUTPUT_DISTRIBUTION::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    PERF_TIMER(on_get_output_distribution);
    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_OUTPUT_DISTRIBUTION>(invoke_http_mode::JON_RPC, "get_output_distribution", req, res, r))
      return r;

    try
    {
      // 0 is placeholder for the whole chain
      const uint64_t req_to_height = req.to_height ? req.to_height : (m_core.get_current_blockchain_height() - 1);
      for (uint64_t amount: req.amounts)
      {
        auto data = rpc::RpcHandler::get_output_distribution([this](uint64_t amount, uint64_t from, uint64_t to, uint64_t &start_height, std::vector<uint64_t> &distribution, uint64_t &base) { return m_core.get_output_distribution(amount, from, to, start_height, distribution, base); }, amount, req.from_height, req_to_height, [this](uint64_t height) { return m_core.get_blockchain_storage().get_db().get_block_hash_from_height(height); }, req.cumulative, m_core.get_current_blockchain_height());
        if (!data)
        {
          error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
          error_resp.message = "Failed to get output distribution";
          return false;
        }

        res.distributions.push_back({std::move(*data), amount, "", req.binary, req.compress});
      }
    }
    catch (const std::exception &e)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Failed to get output distribution";
      return false;
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_get_output_distribution_bin(const COMMAND_RPC_GET_OUTPUT_DISTRIBUTION::request& req, COMMAND_RPC_GET_OUTPUT_DISTRIBUTION::response& res, const connection_context *ctx)
  {
    PERF_TIMER(on_get_output_distribution_bin);

    bool r;
    if (use_bootstrap_daemon_if_necessary<COMMAND_RPC_GET_OUTPUT_DISTRIBUTION>(invoke_http_mode::BIN, "/get_output_distribution.bin", req, res, r))
      return r;

    res.status = "Failed";

    if (!req.binary)
    {
      res.status = "Binary only call";
      return false;
    }
    try
    {
      // 0 is placeholder for the whole chain
      const uint64_t req_to_height = req.to_height ? req.to_height : (m_core.get_current_blockchain_height() - 1);
      for (uint64_t amount: req.amounts)
      {
        auto data = rpc::RpcHandler::get_output_distribution([this](uint64_t amount, uint64_t from, uint64_t to, uint64_t &start_height, std::vector<uint64_t> &distribution, uint64_t &base) { return m_core.get_output_distribution(amount, from, to, start_height, distribution, base); }, amount, req.from_height, req_to_height, [this](uint64_t height) { return m_core.get_blockchain_storage().get_db().get_block_hash_from_height(height); }, req.cumulative, m_core.get_current_blockchain_height());
        if (!data)
        {
          res.status = "Failed to get output distribution";
          return false;
        }

        res.distributions.push_back({std::move(*data), amount, "", req.binary, req.compress});
      }
    }
    catch (const std::exception &e)
    {
      res.status = "Failed to get output distribution";
      return false;
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------
  bool core_rpc_server::on_prune_blockchain(const COMMAND_RPC_PRUNE_BLOCKCHAIN::request& req, COMMAND_RPC_PRUNE_BLOCKCHAIN::response& res, epee::json_rpc::error& error_resp, const connection_context *ctx)
  {
    try
    {
      if (!(req.check ? m_core.check_blockchain_pruning() : m_core.prune_blockchain()))
      {
        error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
        error_resp.message = req.check ? "Failed to check blockchain pruning" : "Failed to prune blockchain";
        return false;
      }
      res.pruning_seed = m_core.get_blockchain_pruning_seed();
      res.pruned = res.pruning_seed != 0;
    }
    catch (const std::exception &e)
    {
      error_resp.code = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
      error_resp.message = "Failed to prune blockchain";
      return false;
    }

    res.status = CORE_RPC_STATUS_OK;
    return true;
  }
  //------------------------------------------------------------------------------------------------------------------------------


  const command_line::arg_descriptor<std::string, false, true, 2> core_rpc_server::arg_rpc_bind_port = {
      "rpc-bind-port"
    , "Port for RPC server"
    , std::to_string(config::RPC_DEFAULT_PORT)
    , {{ &cryptonote::arg_testnet_on, &cryptonote::arg_stagenet_on }}
    , [](std::array<bool, 2> testnet_stagenet, bool defaulted, std::string val)->std::string {
        if (testnet_stagenet[0] && defaulted)
          return std::to_string(config::testnet::RPC_DEFAULT_PORT);
        else if (testnet_stagenet[1] && defaulted)
          return std::to_string(config::stagenet::RPC_DEFAULT_PORT);
        return val;
      }
    };

  const command_line::arg_descriptor<std::string> core_rpc_server::arg_rpc_restricted_bind_port = {
      "rpc-restricted-bind-port"
    , "Port for restricted RPC server"
    , ""
    };

  const command_line::arg_descriptor<bool> core_rpc_server::arg_restricted_rpc = {
      "restricted-rpc"
    , "Restrict RPC to view only commands and do not return privacy sensitive data in RPC calls"
    , false
    };

  const command_line::arg_descriptor<std::string> core_rpc_server::arg_bootstrap_daemon_address = {
      "bootstrap-daemon-address"
    , "URL of a 'bootstrap' remote daemon that the connected wallets can use while this daemon is still not fully synced"
    , ""
    };

  const command_line::arg_descriptor<std::string> core_rpc_server::arg_bootstrap_daemon_login = {
      "bootstrap-daemon-login"
    , "Specify username:password for the bootstrap daemon login"
    , ""
    };
}  // namespace cryptonote
