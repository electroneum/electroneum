// Copyrights(c) 2017-2018, The Electroneum Project
// Copyrights(c) 2014-2017, The Monero Project
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
#include <boost/asio/io_service.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/list.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <atomic>
#include <unordered_map>
#include <unordered_set>

#include "syncobj.h"
#include "string_tools.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "common/util.h"
#include "cryptonote_protocol/cryptonote_protocol_defs.h"
#include "rpc/core_rpc_server_commands_defs.h"
#include "cryptonote_basic/difficulty.h"
#include "cryptonote_tx_utils.h"
#include "cryptonote_basic/verification_context.h"
#include "crypto/hash.h"
#include "cryptonote_basic/checkpoints.h"
#include "cryptonote_basic/hardfork.h"
#include "blockchain_db/blockchain_db.h"

namespace cryptonote
{
  class tx_memory_pool;
  struct test_options;

  /** Declares ways in which the BlockchainDB backend should be told to sync
   *
   */
  enum blockchain_db_sync_mode
  {
    db_defaultsync, //!< user didn't specify, use db_async
    db_sync,  //!< handle syncing calls instead of the backing db, synchronously
    db_async, //!< handle syncing calls instead of the backing db, asynchronously
    db_nosync //!< Leave syncing up to the backing db (safest, but slowest because of disk I/O)
  };

  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  class Blockchain
  {
  public:
    /**
     * @brief Now-defunct (TODO: remove) struct from in-memory blockchain
     */
    struct transaction_chain_entry
    {
      transaction tx;
      uint64_t m_keeper_block_height;
      size_t m_blob_size;
      std::vector<uint64_t> m_global_output_indexes;
    };

    /**
     * @brief container for passing a block and metadata about it on the blockchain
     */
    struct block_extended_info
    {
      block   bl; //!< the block
      uint64_t height; //!< the height of the block in the blockchain
      size_t block_cumulative_size; //!< the size (in bytes) of the block
      difficulty_type cumulative_difficulty; //!< the accumulated difficulty after that block
      uint64_t already_generated_coins; //!< the total coins minted after that block
    };

    /**
     * @brief Blockchain constructor
     *
     * @param tx_pool a reference to the transaction pool to be kept by the Blockchain
     */
    Blockchain(tx_memory_pool& tx_pool);

    /**
     * @brief Initialize the Blockchain state
     *
     * @param db a pointer to the backing store to use for the blockchain
     * @param testnet true if on testnet, else false
     * @param test_options test parameters
     *
     * @return true on success, false if any initialization steps fail
     */
    bool init(BlockchainDB* db, const bool testnet = false, const cryptonote::test_options *test_options = NULL);

    /**
     * @brief Initialize the Blockchain state
     *
     * @param db a pointer to the backing store to use for the blockchain
     * @param hf a structure containing hardfork information
     * @param testnet true if on testnet, else false
     *
     * @return true on success, false if any initialization steps fail
     */
    bool init(BlockchainDB* db, HardFork*& hf, const bool testnet = false);

    /**
     * @brief Uninitializes the blockchain state
     *
     * Saves to disk any state that needs to be maintained
     *
     * @return true on success, false if any uninitialization steps fail
     */
    bool deinit();

    /**
     * @brief assign a set of blockchain checkpoint hashes
     *
     * @param chk_pts the set of checkpoints to assign
     */
    void set_checkpoints(checkpoints&& chk_pts) { m_checkpoints = chk_pts; }

    /**
     * @brief get blocks and transactions from blocks based on start height and count
     *
     * @param start_offset the height on the blockchain to start at
     * @param count the number of blocks to get, if there are as many after start_offset
     * @param blocks return-by-reference container to put result blocks in
     * @param txs return-by-reference container to put result transactions in
     *
     * @return false if start_offset > blockchain height, else true
     */
    bool get_blocks(uint64_t start_offset, size_t count, std::list<std::pair<cryptonote::blobdata,block>>& blocks, std::list<cryptonote::blobdata>& txs) const;

    /**
     * @brief get blocks from blocks based on start height and count
     *
     * @param start_offset the height on the blockchain to start at
     * @param count the number of blocks to get, if there are as many after start_offset
     * @param blocks return-by-reference container to put result blocks in
     *
     * @return false if start_offset > blockchain height, else true
     */
    bool get_blocks(uint64_t start_offset, size_t count, std::list<std::pair<cryptonote::blobdata,block>>& blocks) const;

    /**
     * @brief compiles a list of all blocks stored as alternative chains
     *
     * @param blocks return-by-reference container to put result blocks in
     *
     * @return true
     */
    bool get_alternative_blocks(std::list<block>& blocks) const;

    /**
     * @brief returns the number of alternative blocks stored
     *
     * @return the number of alternative blocks stored
     */
    size_t get_alternative_blocks_count() const;

    /**
     * @brief gets a block's hash given a height
     *
     * @param height the height of the block
     *
     * @return the hash of the block at the requested height, or a zeroed hash if there is no such block
     */
    crypto::hash get_block_id_by_height(uint64_t height) const;

    /**
     * @brief gets the block with a given hash
     *
     * @param h the hash to look for
     * @param blk return-by-reference variable to put result block in
     * @param orphan if non-NULL, will be set to true if not in the main chain, false otherwise
     *
     * @return true if the block was found, else false
     */
    bool get_block_by_hash(const crypto::hash &h, block &blk, bool *orphan = NULL) const;

    /**
     * @brief get all block hashes (main chain, alt chains, and invalid blocks)
     *
     * @param main return-by-reference container to put result main chain blocks' hashes in
     * @param alt return-by-reference container to put result alt chain blocks' hashes in
     * @param invalid return-by-reference container to put result invalid blocks' hashes in
     */
    void get_all_known_block_ids(std::list<crypto::hash> &main, std::list<crypto::hash> &alt, std::list<crypto::hash> &invalid) const;

    /**
     * @brief performs some preprocessing on a group of incoming blocks to speed up verification
     *
     * @param blocks a list of incoming blocks
     *
     * @return false on erroneous blocks, else true
     */
    bool prepare_handle_incoming_blocks(const std::list<block_complete_entry>  &blocks);

    /**
     * @brief incoming blocks post-processing, cleanup, and disk sync
     *
     * @param force_sync if true, and Blockchain is handling syncing to disk, always sync
     *
     * @return true
     */
    bool cleanup_handle_incoming_blocks(bool force_sync = false);

    /**
     * @brief search the blockchain for a transaction by hash
     *
     * @param id the hash to search for
     *
     * @return true if the tx exists, else false
     */
    bool have_tx(const crypto::hash &id) const;

    /**
     * @brief check if any key image in a transaction has already been spent
     *
     * @param tx the transaction to check
     *
     * @return true if any key image is already spent in the blockchain, else false
     */
    bool have_tx_keyimges_as_spent(const transaction &tx) const;

    /**
     * @brief check if a key image is already spent on the blockchain
     *
     * Whenever a transaction output is used as an input for another transaction
     * (a true input, not just one of a mixing set), a key image is generated
     * and stored in the transaction in order to prevent double spending.  If
     * this key image is seen again, the transaction using it is rejected.
     *
     * @param key_im the key image to search for
     *
     * @return true if the key image is already spent in the blockchain, else false
     */
    bool have_tx_keyimg_as_spent(const crypto::key_image &key_im) const;

    /**
     * @brief get the current height of the blockchain
     *
     * @return the height
     */
    uint64_t get_current_blockchain_height() const;

    /**
     * @brief get the hash of the most recent block on the blockchain
     *
     * @return the hash
     */
    crypto::hash get_tail_id() const;

    /**
     * @brief get the height and hash of the most recent block on the blockchain
     *
     * @param height return-by-reference variable to store the height in
     *
     * @return the hash
     */
    crypto::hash get_tail_id(uint64_t& height) const;

    /**
     * @brief returns the difficulty target the next block to be added must meet
     *
     * @return the target
     */
    difficulty_type get_difficulty_for_next_block();

    /**
     * @brief adds a block to the blockchain
     *
     * Adds a new block to the blockchain.  If the block's parent is not the
     * current top of the blockchain, the block may be added to an alternate
     * chain.  If the block does not belong, is already in the blockchain
     * or an alternate chain, or is invalid, return false.
     *
     * @param bl_ the block to be added
     * @param bvc metadata about the block addition's success/failure
     *
     * @return true on successful addition to the blockchain, else false
     */
    bool add_new_block(const block& bl_, block_verification_context& bvc);

    /**
     * @brief clears the blockchain and starts a new one
     *
     * @param b the first block in the new chain (the genesis block)
     *
     * @return true on success, else false
     */
    bool reset_and_set_genesis_block(const block& b);

    /**
     * @brief creates a new block to mine against
     *
     * @param b return-by-reference block to be filled in
     * @param miner_address address new coins for the block will go to
     * @param di return-by-reference tells the miner what the difficulty target is
     * @param height return-by-reference tells the miner what height it's mining against
     * @param expected_reward return-by-reference the total reward awarded to the miner finding this block, including transaction fees
     * @param ex_nonce extra data to be added to the miner transaction's extra
     *
     * @return true if block template filled in successfully, else false
     */
    bool create_block_template(block& b, const account_public_address& miner_address, difficulty_type& di, uint64_t& height, uint64_t& expected_reward, const blobdata& ex_nonce);

    /**
     * @brief checks if a block is known about with a given hash
     *
     * This function checks the main chain, alternate chains, and invalid blocks
     * for a block with the given hash
     *
     * @param id the hash to search for
     *
     * @return true if the block is known, else false
     */
    bool have_block(const crypto::hash& id) const;

    /**
     * @brief gets the total number of transactions on the main chain
     *
     * @return the number of transactions on the main chain
     */
    size_t get_total_transactions() const;

    /**
     * @brief gets the hashes for a subset of the blockchain
     *
     * puts into list <ids> a list of hashes representing certain blocks
     * from the blockchain in reverse chronological order
     *
     * the blocks chosen, at the time of this writing, are:
     *   the most recent 11
     *   powers of 2 less recent from there, so 13, 17, 25, etc...
     *
     * @param ids return-by-reference list to put the resulting hashes in
     *
     * @return true
     */
    bool get_short_chain_history(std::list<crypto::hash>& ids) const;

    /**
     * @brief get recent block hashes for a foreign chain
     *
     * Find the split point between us and foreign blockchain and return
     * (by reference) the most recent common block hash along with up to
     * BLOCKS_IDS_SYNCHRONIZING_DEFAULT_COUNT additional (more recent) hashes.
     *
     * @param qblock_ids the foreign chain's "short history" (see get_short_chain_history)
     * @param resp return-by-reference the split height and subsequent blocks' hashes
     *
     * @return true if a block found in common, else false
     */
    bool find_blockchain_supplement(const std::list<crypto::hash>& qblock_ids, NOTIFY_RESPONSE_CHAIN_ENTRY::request& resp) const;

    /**
     * @brief find the most recent common point between ours and a foreign chain
     *
     * This function takes a list of block hashes from another node
     * on the network to find where the split point is between us and them.
     * This is used to see what to send another node that needs to sync.
     *
     * @param qblock_ids the foreign chain's "short history" (see get_short_chain_history)
     * @param starter_offset return-by-reference the most recent common block's height
     *
     * @return true if a block found in common, else false
     */
    bool find_blockchain_supplement(const std::list<crypto::hash>& qblock_ids, uint64_t& starter_offset) const;

    /**
     * @brief get recent blocks for a foreign chain
     *
     * This function gets recent blocks relative to a foreign chain, starting either at
     * a requested height or whatever height is the most recent ours and the foreign
     * chain have in common.
     *
     * @param req_start_block if non-zero, specifies a start point (otherwise find most recent commonality)
     * @param qblock_ids the foreign chain's "short history" (see get_short_chain_history)
     * @param blocks return-by-reference the blocks and their transactions
     * @param total_height return-by-reference our current blockchain height
     * @param start_height return-by-reference the height of the first block returned
     * @param max_count the max number of blocks to get
     *
     * @return true if a block found in common or req_start_block specified, else false
     */
    bool find_blockchain_supplement(const uint64_t req_start_block, const std::list<crypto::hash>& qblock_ids, std::list<std::pair<cryptonote::blobdata, std::list<cryptonote::blobdata> > >& blocks, uint64_t& total_height, uint64_t& start_height, size_t max_count) const;

    /**
     * @brief retrieves a set of blocks and their transactions, and possibly other transactions
     *
     * the request object encapsulates a list of block hashes and a (possibly empty) list of
     * transaction hashes.  for each block hash, the block is fetched along with all of that
     * block's transactions.  Any transactions requested separately are fetched afterwards.
     *
     * @param arg the request
     * @param rsp return-by-reference the response to fill in
     *
     * @return true unless any blocks or transactions are missing
     */
    bool handle_get_objects(NOTIFY_REQUEST_GET_OBJECTS::request& arg, NOTIFY_RESPONSE_GET_OBJECTS::request& rsp);

    /**
     * @brief gets random outputs to mix with
     *
     * This function takes an RPC request for outputs to mix with
     * and creates an RPC response with the resultant output indices.
     *
     * Outputs to mix with are randomly selected from the utxo set
     * for each output amount in the request.
     *
     * @param req the output amounts and number of mixins to select
     * @param res return-by-reference the resultant output indices
     *
     * @return true
     */
    bool get_random_outs_for_amounts(const COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::request& req, COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::response& res) const;

    /**
     * @brief gets specific outputs to mix with
     *
     * This function takes an RPC request for outputs to mix with
     * and creates an RPC response with the resultant output indices.
     *
     * Outputs to mix with are specified in the request.
     *
     * @param req the outputs to return
     * @param res return-by-reference the resultant output indices and keys
     *
     * @return true
     */
    bool get_outs(const COMMAND_RPC_GET_OUTPUTS_BIN::request& req, COMMAND_RPC_GET_OUTPUTS_BIN::response& res) const;

    /**
     * @brief gets random ringct outputs to mix with
     *
     * This function takes an RPC request for outputs to mix with
     * and creates an RPC response with the resultant output indices
     * and the matching keys.
     *
     * Outputs to mix with are randomly selected from the utxo set
     * for each output amount in the request.
     *
     * @param req the output amounts and number of mixins to select
     * @param res return-by-reference the resultant output indices
     *
     * @return true
     */
    bool get_random_rct_outs(const COMMAND_RPC_GET_RANDOM_RCT_OUTPUTS::request& req, COMMAND_RPC_GET_RANDOM_RCT_OUTPUTS::response& res) const;

    /**
     * @brief gets the global indices for outputs from a given transaction
     *
     * This function gets the global indices for all outputs belonging
     * to a specific transaction.
     *
     * @param tx_id the hash of the transaction to fetch indices for
     * @param indexs return-by-reference the global indices for the transaction's outputs
     *
     * @return false if the transaction does not exist, or if no indices are found, otherwise true
     */
    bool get_tx_outputs_gindexs(const crypto::hash& tx_id, std::vector<uint64_t>& indexs) const;

    /**
     * @brief stores the blockchain
     *
     * If Blockchain is handling storing of the blockchain (rather than BlockchainDB),
     * this initiates a blockchain save.
     *
     * @return true unless saving the blockchain fails
     */
    bool store_blockchain();

    /**
     * @brief validates a transaction's inputs
     *
     * validates a transaction's inputs as correctly used and not previously
     * spent.  also returns the hash and height of the most recent block
     * which contains an output that was used as an input to the transaction.
     * The transaction's rct signatures, if any, are expanded.
     *
     * @param tx the transaction to validate
     * @param pmax_used_block_height return-by-reference block height of most recent input
     * @param max_used_block_id return-by-reference block hash of most recent input
     * @param tvc returned information about tx verification
     * @param kept_by_block whether or not the transaction is from a previously-verified block
     *
     * @return false if any input is invalid, otherwise true
     */
    bool check_tx_inputs(transaction& tx, uint64_t& pmax_used_block_height, crypto::hash& max_used_block_id, tx_verification_context &tvc, bool kept_by_block = false);

    /**
     * @brief get dynamic per kB fee for a given block size
     *
     * The dynamic fee is based on the block size in a past window, and
     * the current block reward. It is expressed by kB.
     *
     * @param block_reward the current block reward
     * @param median_block_size the median blob's size in the past window
     * @param version hard fork version for rules and constants to use
     *
     * @return the per kB fee
     */
    static uint64_t get_dynamic_per_kb_fee(uint64_t block_reward, size_t median_block_size, uint8_t version);

    /**
     * @brief get dynamic per kB fee estimate for the next few blocks
     *
     * The dynamic fee is based on the block size in a past window, and
     * the current block reward. It is expressed by kB. This function
     * calculates an estimate for a dynamic fee which will be valid for
     * the next grace_blocks
     *
     * @param grace_blocks number of blocks we want the fee to be valid for
     *
     * @return the per kB fee estimate
     */
    uint64_t get_dynamic_per_kb_fee_estimate(uint64_t grace_blocks) const;

    /**
     * @brief validate a transaction's fee
     *
     * This function validates the fee is enough for the transaction.
     * This is based on the size of the transaction blob, and, after a
     * height threshold, on the average size of transaction in a past window
     *
     * @param blob_size the transaction blob's size
     * @param fee the fee
     *
     * @return true if the fee is enough, false otherwise
     */
    bool check_fee(size_t blob_size, uint64_t fee) const;

    /**
     * @brief check that a transaction's outputs conform to current standards
     *
     * This function checks, for example at the time of this writing, that
     * each output is of the form a * 10^b (phrased differently, that if
     * written out would have only one non-zero digit in base 10).
     *
     * @param tx the transaction to check the outputs of
     * @param tvc returned info about tx verification
     *
     * @return false if any outputs do not conform, otherwise true
     */
    bool check_tx_outputs(const transaction& tx, tx_verification_context &tvc);

    /**
     * @brief gets the blocksize limit based on recent blocks
     *
     * @return the limit
     */
    uint64_t get_current_cumulative_blocksize_limit() const;

    /**
     * @brief checks if the blockchain is currently being stored
     *
     * Note: this should be meaningless in cases where Blockchain is not
     * directly managing saving the blockchain to disk.
     *
     * @return true if Blockchain is having the chain stored currently, else false
     */
    bool is_storing_blockchain()const{return false;}

    /**
     * @brief gets the difficulty of the block with a given height
     *
     * @param i the height
     *
     * @return the difficulty
     */
    uint64_t block_difficulty(uint64_t i) const;

    /**
     * @brief gets blocks based on a list of block hashes
     *
     * @tparam t_ids_container a standard-iterable container
     * @tparam t_blocks_container a standard-iterable container
     * @tparam t_missed_container a standard-iterable container
     * @param block_ids a container of block hashes for which to get the corresponding blocks
     * @param blocks return-by-reference a container to store result blocks in
     * @param missed_bs return-by-reference a container to store missed blocks in
     *
     * @return false if an unexpected exception occurs, else true
     */
    template<class t_ids_container, class t_blocks_container, class t_missed_container>
    bool get_blocks(const t_ids_container& block_ids, t_blocks_container& blocks, t_missed_container& missed_bs) const;

    /**
     * @brief gets transactions based on a list of transaction hashes
     *
     * @tparam t_ids_container a standard-iterable container
     * @tparam t_tx_container a standard-iterable container
     * @tparam t_missed_container a standard-iterable container
     * @param txs_ids a container of hashes for which to get the corresponding transactions
     * @param txs return-by-reference a container to store result transactions in
     * @param missed_txs return-by-reference a container to store missed transactions in
     *
     * @return false if an unexpected exception occurs, else true
     */
    template<class t_ids_container, class t_tx_container, class t_missed_container>
    bool get_transactions_blobs(const t_ids_container& txs_ids, t_tx_container& txs, t_missed_container& missed_txs) const;
    template<class t_ids_container, class t_tx_container, class t_missed_container>
    bool get_transactions(const t_ids_container& txs_ids, t_tx_container& txs, t_missed_container& missed_txs) const;

    //debug functions

    /**
     * @brief prints data about a snippet of the blockchain
     *
     * if start_index is greater than the blockchain height, do nothing
     *
     * @param start_index height on chain to start at
     * @param end_index height on chain to end at
     */
    void print_blockchain(uint64_t start_index, uint64_t end_index) const;

    /**
     * @brief prints every block's hash
     *
     * WARNING: This function will absolutely crush a terminal in prints, so
     * it is recommended to redirect this output to a log file (or null sink
     * if a log file is already set up, as should be the default)
     */
    void print_blockchain_index() const;

    /**
     * @brief currently does nothing, candidate for removal
     *
     * @param file
     */
    void print_blockchain_outs(const std::string& file) const;

    /**
     * @brief check the blockchain against a set of checkpoints
     *
     * If a block fails a checkpoint and enforce is enabled, the blockchain
     * will be rolled back to two blocks prior to that block.  If enforce
     * is disabled, as is currently the default case with DNS-based checkpoints,
     * an error will be printed to the user but no other action will be taken.
     *
     * @param points the checkpoints to check against
     * @param enforce whether or not to take action on failure
     */
    void check_against_checkpoints(const checkpoints& points, bool enforce);

    /**
     * @brief configure whether or not to enforce DNS-based checkpoints
     *
     * @param enforce the new enforcement setting
     */
    void set_enforce_dns_checkpoints(bool enforce);

    /**
     * @brief loads new checkpoints from a file and optionally from DNS
     *
     * @param file_path the path of the file to look for and load checkpoints from
     * @param check_dns whether or not to check for new DNS-based checkpoints
     *
     * @return false if any enforced checkpoint type fails to load, otherwise true
     */
    bool update_checkpoints(const std::string& file_path, bool check_dns);


    // user options, must be called before calling init()

    /**
     * @brief sets various performance options
     *
     * @param maxthreads max number of threads when preparing blocks for addition
     * @param blocks_per_sync number of blocks to cache before syncing to database
     * @param sync_mode the ::blockchain_db_sync_mode to use
     * @param fast_sync sync using built-in block hashes as trusted
     */
    void set_user_options(uint64_t maxthreads, uint64_t blocks_per_sync,
        blockchain_db_sync_mode sync_mode, bool fast_sync);

    /**
     * @brief Put DB in safe sync mode
     */
    void safesyncmode(const bool onoff);

    /**
     * @brief set whether or not to show/print time statistics
     *
     * @param stats the new time stats setting
     */
    void set_show_time_stats(bool stats) { m_show_time_stats = stats; }

    /**
     * @brief gets the hardfork voting state object
     *
     * @return the HardFork object
     */
    HardFork::State get_hard_fork_state() const;

    /**
     * @brief gets the current hardfork version in use/voted for
     *
     * @return the version
     */
    uint8_t get_current_hard_fork_version() const { return m_hardfork->get_current_version(); }

    /**
     * @brief returns the newest hardfork version known to the blockchain
     *
     * @return the version
     */
    uint8_t get_ideal_hard_fork_version() const { return m_hardfork->get_ideal_version(); }

    /**
     * @brief returns the next hardfork version
     *
     * @return the version
     */
    uint8_t get_next_hard_fork_version() const { return m_hardfork->get_next_version(); }

    /**
     * @brief returns the newest hardfork version voted to be enabled
     * as of a certain height
     *
     * @param height the height for which to check version info
     *
     * @return the version
     */
    uint8_t get_ideal_hard_fork_version(uint64_t height) const { return m_hardfork->get_ideal_version(height); }

    /**
     * @brief returns the actual hardfork version for a given block height
     *
     * @param height the height for which to check version info
     *
     * @return the version
     */
    uint8_t get_hard_fork_version(uint64_t height) const { return m_hardfork->get(height); }

    /**
     * @brief get information about hardfork voting for a version
     *
     * @param version the version in question
     * @param window the size of the voting window
     * @param votes the number of votes to enable <version>
     * @param threshold the number of votes required to enable <version>
     * @param earliest_height the earliest height at which <version> is allowed
     * @param voting which version this node is voting for/using
     *
     * @return whether the version queried is enabled 
     */
    bool get_hard_fork_voting_info(uint8_t version, uint32_t &window, uint32_t &votes, uint32_t &threshold, uint64_t &earliest_height, uint8_t &voting) const;

    /**
     * @brief remove transactions from the transaction pool (if present)
     *
     * @param txids a list of hashes of transactions to be removed
     *
     * @return false if any removals fail, otherwise true
     */
    bool flush_txes_from_pool(const std::list<crypto::hash> &txids);

    /**
     * @brief return a histogram of outputs on the blockchain
     *
     * @param amounts optional set of amounts to lookup
     * @param unlocked whether to restrict instances to unlocked ones
     * @param recent_cutoff timestamp to consider outputs as recent
     *
     * @return a set of amount/instances
     */
    std::map<uint64_t, std::tuple<uint64_t, uint64_t, uint64_t>> get_output_histogram(const std::vector<uint64_t> &amounts, bool unlocked, uint64_t recent_cutoff) const;

    /**
     * @brief perform a check on all key images in the blockchain
     *
     * @param std::function the check to perform, pass/fail
     *
     * @return false if any key image fails the check, otherwise true
     */
    bool for_all_key_images(std::function<bool(const crypto::key_image&)>) const;

    /**
     * @brief perform a check on all blocks in the blockchain in the given range
     *
     * @param h1 the start height
     * @param h2 the end height
     * @param std::function the check to perform, pass/fail
     *
     * @return false if any block fails the check, otherwise true
     */
    bool for_blocks_range(const uint64_t& h1, const uint64_t& h2, std::function<bool(uint64_t, const crypto::hash&, const block&)>) const;

    /**
     * @brief perform a check on all transactions in the blockchain
     *
     * @param std::function the check to perform, pass/fail
     *
     * @return false if any transaction fails the check, otherwise true
     */
    bool for_all_transactions(std::function<bool(const crypto::hash&, const cryptonote::transaction&)>) const;

    /**
     * @brief perform a check on all outputs in the blockchain
     *
     * @param std::function the check to perform, pass/fail
     *
     * @return false if any output fails the check, otherwise true
     */
    bool for_all_outputs(std::function<bool(uint64_t amount, const crypto::hash &tx_hash, size_t tx_idx)>) const;

    /**
     * @brief get a reference to the BlockchainDB in use by Blockchain
     *
     * @return a reference to the BlockchainDB instance
     */
    const BlockchainDB& get_db() const
    {
      return *m_db;
    }

    /**
     * @brief get a reference to the BlockchainDB in use by Blockchain
     *
     * @return a reference to the BlockchainDB instance
     */
    BlockchainDB& get_db()
    {
      return *m_db;
    }

    /**
     * @brief get a number of outputs of a specific amount
     *
     * @param amount the amount
     * @param offsets the indices (indexed to the amount) of the outputs
     * @param outputs return-by-reference the outputs collected
     * @param txs unused, candidate for removal
     */
    void output_scan_worker(const uint64_t amount,const std::vector<uint64_t> &offsets,
        std::vector<output_data_t> &outputs, std::unordered_map<crypto::hash,
        cryptonote::transaction> &txs) const;

    /**
     * @brief computes the "short" and "long" hashes for a set of blocks
     *
     * @param height the height of the first block
     * @param blocks the blocks to be hashed
     * @param map return-by-reference the hashes for each block
     */
    void block_longhash_worker(uint64_t height, const std::vector<block> &blocks,
        std::unordered_map<crypto::hash, crypto::hash> &map) const;

    /**
     * @brief returns a set of known alternate chains
     *
     * @return a list of chains
     */
    std::list<std::pair<block_extended_info,uint64_t>> get_alternative_chains() const;

    void add_txpool_tx(transaction &tx, const txpool_tx_meta_t &meta);
    void update_txpool_tx(const crypto::hash &txid, const txpool_tx_meta_t &meta);
    void remove_txpool_tx(const crypto::hash &txid);
    uint64_t get_txpool_tx_count() const;
    txpool_tx_meta_t get_txpool_tx_meta(const crypto::hash& txid) const;
    bool get_txpool_tx_blob(const crypto::hash& txid, cryptonote::blobdata &bd) const;
    cryptonote::blobdata get_txpool_tx_blob(const crypto::hash& txid) const;
    bool for_all_txpool_txes(std::function<bool(const crypto::hash&, const txpool_tx_meta_t&, const cryptonote::blobdata*)>, bool include_blob = false) const;
    uint32_t get_mempool_tx_livetime() const;

    bool is_within_compiled_block_hash_area(uint64_t height) const;
    bool is_within_compiled_block_hash_area() const { return is_within_compiled_block_hash_area(m_db->height()); }

    void lock();
    void unlock();

    void cancel();

    /**
     * @brief called when we see a tx originating from a block
     *
     * Used for handling txes from historical blocks in a fast way
     */
    void on_new_tx_from_block(const cryptonote::transaction &tx);

  private:

    // TODO: evaluate whether or not each of these typedefs are left over from blockchain_storage
    typedef std::unordered_map<crypto::hash, size_t> blocks_by_id_index;

    typedef std::unordered_map<crypto::hash, transaction_chain_entry> transactions_container;

    typedef std::unordered_set<crypto::key_image> key_images_container;

    typedef std::vector<block_extended_info> blocks_container;

    typedef std::unordered_map<crypto::hash, block_extended_info> blocks_ext_by_hash;

    typedef std::unordered_map<crypto::hash, block> blocks_by_hash;

    typedef std::map<uint64_t, std::vector<std::pair<crypto::hash, size_t>>> outputs_container; //crypto::hash - tx hash, size_t - index of out in transaction


    BlockchainDB* m_db;

    tx_memory_pool& m_tx_pool;

    mutable epee::critical_section m_blockchain_lock; // TODO: add here reader/writer lock

    // main chain
    transactions_container m_transactions;
    size_t m_current_block_cumul_sz_limit;

    // metadata containers
    std::unordered_map<crypto::hash, std::unordered_map<crypto::key_image, std::vector<output_data_t>>> m_scan_table;
    std::unordered_map<crypto::hash, crypto::hash> m_blocks_longhash_table;
    std::unordered_map<crypto::hash, std::unordered_map<crypto::key_image, bool>> m_check_txin_table;

    // SHA-3 hashes for each block and for fast pow checking
    std::vector<crypto::hash> m_blocks_hash_check;
    std::vector<crypto::hash> m_blocks_txs_check;

    blockchain_db_sync_mode m_db_sync_mode;
    bool m_fast_sync;
    bool m_show_time_stats;
    bool m_db_default_sync;
    uint64_t m_db_blocks_per_sync;
    uint64_t m_max_prepare_blocks_threads;
    uint64_t m_fake_pow_calc_time;
    uint64_t m_fake_scan_time;
    uint64_t m_sync_counter;
    std::vector<uint64_t> m_timestamps;
    std::vector<difficulty_type> m_difficulties;
    uint64_t m_timestamps_and_difficulties_height;

    boost::asio::io_service m_async_service;
    boost::thread_group m_async_pool;
    std::unique_ptr<boost::asio::io_service::work> m_async_work_idle;

    // all alternative chains
    blocks_ext_by_hash m_alternative_chains; // crypto::hash -> block_extended_info

    // some invalid blocks
    blocks_ext_by_hash m_invalid_blocks;     // crypto::hash -> block_extended_info


    checkpoints m_checkpoints;
    bool m_enforce_dns_checkpoints;

    HardFork *m_hardfork;

    bool m_testnet;

    std::atomic<bool> m_cancel;

    /**
     * @brief collects the keys for all outputs being "spent" as an input
     *
     * This function makes sure that each "input" in an input (mixins) exists
     * and collects the public key for each from the transaction it was included in
     * via the visitor passed to it.
     *
     * If pmax_related_block_height is not NULL, its value is set to the height
     * of the most recent block which contains an output used in the input set
     *
     * @tparam visitor_t a class encapsulating tx is unlocked and collect tx key
     * @param tx_in_to_key a transaction input instance
     * @param vis an instance of the visitor to use
     * @param tx_prefix_hash the hash of the associated transaction_prefix
     * @param pmax_related_block_height return-by-pointer the height of the most recent block in the input set
     * @param tx_version version of the tx, if > 1 we also get commitments
     *
     * @return false if any keys are not found or any inputs are not unlocked, otherwise true
     */
    template<class visitor_t>
    inline bool scan_outputkeys_for_indexes(size_t tx_version, const txin_to_key& tx_in_to_key, visitor_t &vis, const crypto::hash &tx_prefix_hash, uint64_t* pmax_related_block_height = NULL) const;

    /**
     * @brief collect output public keys of a transaction input set
     *
     * This function locates all outputs associated with a given input set (mixins)
     * and validates that they exist and are usable
     * (unlocked, unspent is checked elsewhere).
     *
     * If pmax_related_block_height is not NULL, its value is set to the height
     * of the most recent block which contains an output used in the input set
     *
     * @param tx_version the transaction version
     * @param txin the transaction input
     * @param tx_prefix_hash the transaction prefix hash, for caching organization
     * @param sig the input signature
     * @param output_keys return-by-reference the public keys of the outputs in the input set
     * @param rct_signatures the ringCT signatures, which are only valid if tx version > 1
     * @param pmax_related_block_height return-by-pointer the height of the most recent block in the input set
     *
     * @return false if any output is not yet unlocked, or is missing, otherwise true
     */
    bool check_tx_input(size_t tx_version,const txin_to_key& txin, const crypto::hash& tx_prefix_hash, const std::vector<crypto::signature>& sig, const rct::rctSig &rct_signatures, std::vector<rct::ctkey> &output_keys, uint64_t* pmax_related_block_height);

    /**
     * @brief validate a transaction's inputs and their keys
     *
     * This function validates transaction inputs and their keys.  Previously
     * it also performed double spend checking, but that has been moved to its
     * own function.
     * The transaction's rct signatures, if any, are expanded.
     *
     * If pmax_related_block_height is not NULL, its value is set to the height
     * of the most recent block which contains an output used in any input set
     *
     * Currently this function calls ring signature validation for each
     * transaction.
     *
     * @param tx the transaction to validate
     * @param tvc returned information about tx verification
     * @param pmax_related_block_height return-by-pointer the height of the most recent block in the input set
     *
     * @return false if any validation step fails, otherwise true
     */
    bool check_tx_inputs(transaction& tx, tx_verification_context &tvc, uint64_t* pmax_used_block_height = NULL);

    /**
     * @brief performs a blockchain reorganization according to the longest chain rule
     *
     * This function aggregates all the actions necessary to switch to a
     * newly-longer chain.  If any step in the reorganization process fails,
     * the blockchain is reverted to its previous state.
     *
     * @param alt_chain the chain to switch to
     * @param discard_disconnected_chain whether or not to keep the old chain as an alternate
     *
     * @return false if the reorganization fails, otherwise true
     */
    bool switch_to_alternative_blockchain(std::list<blocks_ext_by_hash::iterator>& alt_chain, bool discard_disconnected_chain);

    /**
     * @brief removes the most recent block from the blockchain
     *
     * @return the block removed
     */
    block pop_block_from_blockchain();

    /**
     * @brief validate and add a new block to the end of the blockchain
     *
     * This function is merely a convenience wrapper around the other
     * of the same name.  This one passes the block's hash to the other
     * as well as the block and verification context.
     *
     * @param bl the block to be added
     * @param bvc metadata concerning the block's validity
     *
     * @return true if the block was added successfully, otherwise false
     */
    bool handle_block_to_main_chain(const block& bl, block_verification_context& bvc);

    /**
     * @brief validate and add a new block to the end of the blockchain
     *
     * When a block is given to Blockchain to be added to the blockchain, it
     * is passed here if it is determined to belong at the end of the current
     * chain.
     *
     * @param bl the block to be added
     * @param id the hash of the block
     * @param bvc metadata concerning the block's validity
     *
     * @return true if the block was added successfully, otherwise false
     */
    bool handle_block_to_main_chain(const block& bl, const crypto::hash& id, block_verification_context& bvc);

    /**
     * @brief validate and add a new block to an alternate blockchain
     *
     * If a block to be added does not belong to the main chain, but there
     * is an alternate chain to which it should be added, that is handled
     * here.
     *
     * @param b the block to be added
     * @param id the hash of the block
     * @param bvc metadata concerning the block's validity
     *
     * @return true if the block was added successfully, otherwise false
     */
    bool handle_alternative_block(const block& b, const crypto::hash& id, block_verification_context& bvc);

    /**
     * @brief gets the difficulty requirement for a new block on an alternate chain
     *
     * @param alt_chain the chain to be added to
     * @param bei the block being added (and metadata, see ::block_extended_info)
     *
     * @return the difficulty requirement
     */
    difficulty_type get_next_difficulty_for_alternative_chain(const std::list<blocks_ext_by_hash::iterator>& alt_chain, block_extended_info& bei) const;

    /**
     * @brief sanity checks a miner transaction before validating an entire block
     *
     * This function merely checks basic things like the structure of the miner
     * transaction, the unlock time, and that the amount doesn't overflow.
     *
     * @param b the block containing the miner transaction
     * @param height the height at which the block will be added
     *
     * @return false if anything is found wrong with the miner transaction, otherwise true
     */
    bool prevalidate_miner_transaction(const block& b, uint64_t height);

    /**
     * @brief validates a miner (coinbase) transaction
     *
     * This function makes sure that the miner calculated his reward correctly
     * and that his miner transaction totals reward + fee.
     *
     * @param b the block containing the miner transaction to be validated
     * @param cumulative_block_size the block's size
     * @param fee the total fees collected in the block
     * @param base_reward return-by-reference the new block's generated coins
     * @param already_generated_coins the amount of currency generated prior to this block
     * @param partial_block_reward return-by-reference true if miner accepted only partial reward
     * @param version hard fork version for that transaction
     *
     * @return false if anything is found wrong with the miner transaction, otherwise true
     */
    bool validate_miner_transaction(const block& b, size_t cumulative_block_size, uint64_t fee, uint64_t& base_reward, uint64_t already_generated_coins, bool &partial_block_reward, uint8_t version);

    /**
     * @brief reverts the blockchain to its previous state following a failed switch
     *
     * If Blockchain fails to switch to an alternate chain when it means
     * to do so, this function reverts the blockchain to how it was before
     * the attempted switch.
     *
     * @param original_chain the chain to switch back to
     * @param rollback_height the height to revert to before appending the original chain
     *
     * @return false if something goes wrong with reverting (very bad), otherwise true
     */
    bool rollback_blockchain_switching(std::list<block>& original_chain, uint64_t rollback_height);

    /**
     * @brief gets recent block sizes for median calculation
     *
     * get the block sizes of the last <count> blocks, and return by reference <sz>.
     *
     * @param sz return-by-reference the list of sizes
     * @param count the number of blocks to get sizes for
     */
    void get_last_n_blocks_sizes(std::vector<size_t>& sz, size_t count) const;

    /**
     * @brief adds the given output to the requested set of random outputs
     *
     * @param result_outs return-by-reference the set the output is to be added to
     * @param amount the output amount
     * @param i the output index (indexed to amount)
     */
    void add_out_to_get_random_outs(COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount& result_outs, uint64_t amount, size_t i) const;

    /**
     * @brief adds the given output to the requested set of random ringct outputs
     *
     * @param outs return-by-reference the set the output is to be added to
     * @param amount the output amount (0 for rct inputs)
     * @param i the rct output index
     */
    void add_out_to_get_rct_random_outs(std::list<COMMAND_RPC_GET_RANDOM_RCT_OUTPUTS::out_entry>& outs, uint64_t amount, size_t i) const;

    /**
     * @brief checks if a transaction is unlocked (its outputs spendable)
     *
     * This function checks to see if a transaction is unlocked.
     * unlock_time is either a block index or a unix time.
     *
     * @param unlock_time the unlock parameter (height or time)
     *
     * @return true if spendable, otherwise false
     */
    bool is_tx_spendtime_unlocked(uint64_t unlock_time) const;

    /**
     * @brief stores an invalid block in a separate container
     *
     * Storing invalid blocks allows quick dismissal of the same block
     * if it is seen again.
     *
     * @param bl the invalid block
     * @param h the block's hash
     *
     * @return false if the block cannot be stored for some reason, otherwise true
     */
    bool add_block_as_invalid(const block& bl, const crypto::hash& h);

    /**
     * @brief stores an invalid block in a separate container
     *
     * Storing invalid blocks allows quick dismissal of the same block
     * if it is seen again.
     *
     * @param bei the invalid block (see ::block_extended_info)
     * @param h the block's hash
     *
     * @return false if the block cannot be stored for some reason, otherwise true
     */
    bool add_block_as_invalid(const block_extended_info& bei, const crypto::hash& h);

    /**
     * @brief checks a block's timestamp
     *
     * This function grabs the timestamps from the most recent <n> blocks,
     * where n = BLOCKCHAIN_TIMESTAMP_CHECK_WINDOW.  If there are not those many
     * blocks in the blockchain, the timestap is assumed to be valid.  If there
     * are, this function returns:
     *   true if the block's timestamp is not less than the timestamp of the
     *       median of the selected blocks
     *   false otherwise
     *
     * @param b the block to be checked
     *
     * @return true if the block's timestamp is valid, otherwise false
     */
    bool check_block_timestamp(const block& b) const;

    /**
     * @brief checks a block's timestamp
     *
     * If the block is not more recent than the median of the recent
     * timestamps passed here, it is considered invalid.
     *
     * @param timestamps a list of the most recent timestamps to check against
     * @param b the block to be checked
     *
     * @return true if the block's timestamp is valid, otherwise false
     */
    bool check_block_timestamp(std::vector<uint64_t>& timestamps, const block& b) const;

    /**
     * @brief get the "adjusted time"
     *
     * Currently this simply returns the current time according to the
     * user's machine.
     *
     * @return the current time
     */
    uint64_t get_adjusted_time() const;

    /**
     * @brief finish an alternate chain's timestamp window from the main chain
     *
     * for an alternate chain, get the timestamps from the main chain to complete
     * the needed number of timestamps for the BLOCKCHAIN_TIMESTAMP_CHECK_WINDOW.
     *
     * @param start_height the alternate chain's attachment height to the main chain
     * @param timestamps return-by-value the timestamps set to be populated
     *
     * @return true unless start_height is greater than the current blockchain height
     */
    bool complete_timestamps_vector(uint64_t start_height, std::vector<uint64_t>& timestamps);

    /**
     * @brief calculate the block size limit for the next block to be added
     *
     * @return true
     */
    bool update_next_cumulative_size_limit();
    void return_tx_to_pool(std::vector<transaction> &txs);

    /**
     * @brief make sure a transaction isn't attempting a double-spend
     *
     * @param tx the transaction to check
     * @param keys_this_block a cumulative list of spent keys for the current block
     *
     * @return false if a double spend was detected, otherwise true
     */
    bool check_for_double_spend(const transaction& tx, key_images_container& keys_this_block) const;

    /**
     * @brief validates a transaction input's ring signature
     *
     * @param tx_prefix_hash the transaction prefix' hash
     * @param key_image the key image generated from the true input
     * @param pubkeys the public keys for each input in the ring signature
     * @param sig the signature generated for each input in the ring signature
     * @param result false if the ring signature is invalid, otherwise true
     */
    void check_ring_signature(const crypto::hash &tx_prefix_hash, const crypto::key_image &key_image,
        const std::vector<rct::ctkey> &pubkeys, const std::vector<crypto::signature> &sig, uint64_t &result);

    /**
     * @brief loads block hashes from compiled-in data set
     *
     * A (possibly empty) set of block hashes can be compiled into the
     * electroneum daemon binary.  This function loads those hashes into
     * a useful state.
     */
    void load_compiled_in_block_hashes();

    /**
     * @brief expands v2 transaction data from blockchain
     *
     * RingCT transactions do not transmit some of their data if it
     * can be reconstituted by the receiver. This function expands
     * that implicit data.
     */
    bool expand_transaction_2(transaction &tx, const crypto::hash &tx_prefix_hash, const std::vector<std::vector<rct::ctkey>> &pubkeys);
  };
}  // namespace cryptonote
