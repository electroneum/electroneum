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

#pragma once
#include "include_base_utils.h"

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <boost/serialization/version.hpp>
#include <boost/utility.hpp>

#include "string_tools.h"
#include "syncobj.h"
#include "math_helper.h"
#include "cryptonote_basic/cryptonote_basic_impl.h"
#include "cryptonote_basic/verification_context.h"
#include "blockchain_db/blockchain_db.h"
#include "crypto/hash.h"
#include "rpc/core_rpc_server_commands_defs.h"
#include "rpc/message_data_structs.h"

namespace cryptonote
{
  class Blockchain;
  /************************************************************************/
  /*                                                                      */
  /************************************************************************/

  //! pair of <transaction fee, transaction hash> for organization
  typedef std::pair<std::pair<double, std::time_t>, crypto::hash> tx_by_fee_and_receive_time_entry;

  class txCompare
  {
  public:
    bool operator()(const tx_by_fee_and_receive_time_entry& a, const tx_by_fee_and_receive_time_entry& b) const
    {
      // sort by greatest first, not least
      if (a.first.first > b.first.first) return true;
      else if (a.first.first < b.first.first) return false;
      else if (a.first.second < b.first.second) return true;
      else if (a.first.second > b.first.second) return false;
      else if (a.second != b.second) return true;
      else return false;
    }
  };

  //! container for sorting transactions by fee per unit size
  typedef std::set<tx_by_fee_and_receive_time_entry, txCompare> sorted_tx_container;

  /**
   * @brief Transaction pool, handles transactions which are not part of a block
   *
   * This class handles all transactions which have been received, but not as
   * part of a block.
   *
   * This handling includes:
   *   storing the transactions
   *   organizing the transactions by fee per weight unit
   *   taking/giving transactions to and from various other components
   *   saving the transactions to disk on shutdown
   *   helping create a new block template by choosing transactions for it
   *
   */
  class tx_memory_pool: boost::noncopyable
  {
  public:
    /**
     * @brief Constructor
     *
     * @param bchs a Blockchain class instance, for getting chain info
     */
    tx_memory_pool(Blockchain& bchs);


    /**
     * @copydoc add_tx(transaction&, tx_verification_context&, bool, bool, uint8_t)
     *
     * @param id the transaction's hash
     * @param tx_weight the transaction's weight
     */
    bool add_tx(transaction &tx, const crypto::hash &id, const cryptonote::blobdata &blob, size_t tx_weight, tx_verification_context& tvc, bool kept_by_block, bool relayed, bool do_not_relay, uint8_t version);

    /**
     * @brief add a transaction to the transaction pool
     *
     * Most likely the transaction will come from the network, but it is
     * also possible for transactions to come from popped blocks during
     * a reorg, or from local clients creating a transaction and
     * submitting it to the network
     *
     * @param tx the transaction to be added
     * @param tvc return-by-reference status about the transaction verification
     * @param kept_by_block has this transaction been in a block?
     * @param relayed was this transaction from the network or a local client?
     * @param do_not_relay to avoid relaying the transaction to the network
     * @param version the version used to create the transaction
     *
     * @return true if the transaction passes validations, otherwise false
     */
    bool add_tx(transaction &tx, tx_verification_context& tvc, bool kept_by_block, bool relayed, bool do_not_relay, uint8_t version);

    /**
     * @brief takes a transaction with the given hash from the pool
     *
     * @param id the hash of the transaction
     * @param tx return-by-reference the transaction taken
     * @param txblob return-by-reference the transaction as a blob
     * @param tx_weight return-by-reference the transaction's weight
     * @param fee the transaction fee
     * @param relayed return-by-reference was transaction relayed to us by the network?
     * @param do_not_relay return-by-reference is transaction not to be relayed to the network?
     * @param double_spend_seen return-by-reference was a double spend seen for that transaction?
     * @param nonexistent_utxo_seen return-by-reference was a nonexistent utxo seen for that transaction?
     *
     * @return true unless the transaction cannot be found in the pool
     */
    bool take_tx(const crypto::hash &id, transaction &tx, cryptonote::blobdata &txblob, size_t& tx_weight, uint64_t& fee, bool &relayed, bool &do_not_relay, bool &double_spend_seen, bool &nonexistent_utxo_seen);

    /**
     * @brief checks if the pool has a transaction with the given hash
     *
     * @param id the hash to look for
     *
     * @return true if the transaction is in the pool, otherwise false
     */
    bool have_tx(const crypto::hash &id) const;

    /**
     * @brief action to take when notified of a block added to the blockchain
     *
     * Currently does nothing
     *
     * @param new_block_height the height of the blockchain after the change
     * @param top_block_id the hash of the new top block
     *
     * @return true
     */
    bool on_blockchain_inc(uint64_t new_block_height, const crypto::hash& top_block_id);

    /**
     * @brief action to take when notified of a block removed from the blockchain
     *
     * Currently does nothing
     *
     * @param new_block_height the height of the blockchain after the change
     * @param top_block_id the hash of the new top block
     *
     * @return true
     */
    bool on_blockchain_dec(uint64_t new_block_height, const crypto::hash& top_block_id);

    /**
     * @brief action to take periodically
     *
     * Currently checks transaction pool for stale ("stuck") transactions
     */
    void on_idle();

    /**
     * @brief locks the transaction pool
     */
    void lock() const;

    /**
     * @brief unlocks the transaction pool
     */
    void unlock() const;

    // load/store operations

    /**
     * @brief loads pool state (if any) from disk, and initializes pool
     *
     * @param max_txpool_weight the max weight in bytes
     *
     * @return true
     */
    bool init(size_t max_txpool_weight = 0);

    /**
     * @brief attempts to save the transaction pool state to disk
     *
     * Currently fails (returns false) if the data directory from init()
     * does not exist and cannot be created, but returns true even if
     * saving to disk is unsuccessful.
     *
     * @return true in most cases (see above)
     */
    bool deinit();

    /**
     * @brief Chooses transactions for a block to include
     *
     * @param bl return-by-reference the block to fill in with transactions
     * @param median_weight the current median block weight
     * @param already_generated_coins the current total number of coins "minted"
     * @param total_weight return-by-reference the total weight of the new block
     * @param fee return-by-reference the total of fees from the included transactions
     * @param expected_reward return-by-reference the total reward awarded to the miner finding this block, including transaction fees
     * @param version hard fork version to use for consensus rules
     *
     * @return true
     */
    bool fill_block_template(block &bl, size_t median_weight, uint64_t already_generated_coins, size_t &total_weight, uint64_t &fee, uint64_t &expected_reward, uint8_t version);

    /**
     * @brief get a list of all transactions in the pool
     *
     * @param txs return-by-reference the list of transactions
     * @param include_unrelayed_txes include unrelayed txes in the result
     *
     */
    void get_transactions(std::vector<transaction>& txs, bool include_unrelayed_txes = true) const;

    /**
     * @brief get a list of all transaction hashes in the pool
     *
     * @param txs return-by-reference the list of transactions
     * @param include_unrelayed_txes include unrelayed txes in the result
     *
     */
    void get_transaction_hashes(std::vector<crypto::hash>& txs, bool include_unrelayed_txes = true) const;

    /**
     * @brief get (weight, fee, receive time) for all transaction in the pool
     *
     * @param txs return-by-reference that data
     * @param include_unrelayed_txes include unrelayed txes in the result
     *
     */
    void get_transaction_backlog(std::vector<tx_backlog_entry>& backlog, bool include_unrelayed_txes = true) const;

    /**
     * @brief get a summary statistics of all transaction hashes in the pool
     *
     * @param stats return-by-reference the pool statistics
     * @param include_unrelayed_txes include unrelayed txes in the result
     *
     */
    void get_transaction_stats(struct txpool_stats& stats, bool include_unrelayed_txes = true) const;

    /**
     * @brief get information about all transactions and key images in the pool
     *
     * see documentation on tx_info and spent_key_image_info for more details
     *
     * @param tx_infos return-by-reference the transactions' information
     * @param key_image_infos return-by-reference the spent key images' information
     * @param include_sensitive_data include unrelayed txes and fields that are sensitive to the node privacy
     *
     * @return true
     */
    bool get_transactions_and_spent_keys_info(std::vector<tx_info>& tx_infos, std::vector<spent_key_image_info>& key_image_infos, bool include_sensitive_data = true) const;

    /**
     * @brief get information about all transactions and key images in the pool
     *
     * see documentation on tx_in_pool and key_images_with_tx_hashes for more details
     *
     * @param tx_infos [out] the transactions' information
     * @param key_image_infos [out] the spent key images' information
     *
     * @return true
     */
    bool get_pool_for_rpc(std::vector<cryptonote::rpc::tx_in_pool>& tx_infos, cryptonote::rpc::key_images_with_tx_hashes& key_image_infos) const;

    /**
     * @brief check for presence of key images in the pool
     *
     * @param key_images [in] vector of key images to check
     * @param spent [out] vector of bool to return
     *
     * @return true
     */
    bool check_for_key_images(const std::vector<crypto::key_image>& key_images, std::vector<bool> spent) const;

    /**
     * @brief get a specific transaction from the pool
     *
     * @param h the hash of the transaction to get
     * @param tx return-by-reference the transaction blob requested
     *
     * @return true if the transaction is found, otherwise false
     */
    bool get_transaction(const crypto::hash& h, cryptonote::blobdata& txblob) const;

    /**
     * @brief get a list of all relayable transactions and their hashes
     *
     * "relayable" in this case means:
     *   nonzero fee
     *   hasn't been relayed too recently
     *   isn't old enough that relaying it is considered harmful
     * Note a transaction can be "relayable" even if do_not_relay is true
     *
     * @param txs return-by-reference the transactions and their hashes
     *
     * @return true
     */
    bool get_relayable_transactions(std::vector<std::pair<crypto::hash, cryptonote::blobdata>>& txs) const;

    /**
     * @brief tell the pool that certain transactions were just relayed
     *
     * @param txs the list of transactions (and their hashes)
     */
    void set_relayed(const std::vector<std::pair<crypto::hash, cryptonote::blobdata>>& txs);

    /**
     * @brief get the total number of transactions in the pool
     *
     * @return the number of transactions in the pool
     */
    size_t get_transactions_count(bool include_unrelayed_txes = true) const;

    /**
     * @brief get a string containing human-readable pool information
     *
     * @param short_format whether to use a shortened format for the info
     *
     * @return the string
     */
    std::string print_pool(bool short_format) const;

    /**
     * @brief remove transactions from the pool which are no longer valid
     *
     * With new versions of the currency, what conditions render a transaction
     * invalid may change.  This function clears those which were received
     * before a version change and no longer conform to requirements.
     *
     * @param version the version the transactions must conform to
     *
     * @return the number of transactions removed
     */
    size_t validate(uint8_t version);

     /**
      * @brief return the cookie
      *
      * @return the cookie
      */
    uint64_t cookie() const { return m_cookie; }

    /**
     * @brief get the cumulative txpool weight in bytes
     *
     * @return the cumulative txpool weight in bytes
     */
    size_t get_txpool_weight() const;

    /**
     * @brief set the max cumulative txpool weight in bytes
     *
     * @param bytes the max cumulative txpool weight in bytes
     */
    void set_txpool_max_weight(size_t bytes);

#define CURRENT_MEMPOOL_ARCHIVE_VER    11
#define CURRENT_MEMPOOL_TX_DETAILS_ARCHIVE_VER    13

    /**
     * @brief information about a single transaction
     */
    struct tx_details
    {
      transaction tx;  //!< the transaction
      size_t blob_size;  //!< the transaction's size
      size_t weight;  //!< the transaction's weight
      uint64_t fee;  //!< the transaction's fee amount
      crypto::hash max_used_block_id;  //!< the hash of the highest block referenced by an input
      uint64_t max_used_block_height;  //!< the height of the highest block referenced by an input

      //! whether or not the transaction has been in a block before
      /*! if the transaction was returned to the pool from the blockchain
       *  due to a reorg, then this will be true
       */
      bool kept_by_block;  

      //! the highest block the transaction referenced when last checking it failed
      /*! if verifying a transaction's inputs fails, it's possible this is due
       *  to a reorg since it was created (if it used recently created outputs
       *  as inputs).
       */
      uint64_t last_failed_height;  

      //! the hash of the highest block the transaction referenced when last checking it failed
      /*! if verifying a transaction's inputs fails, it's possible this is due
       *  to a reorg since it was created (if it used recently created outputs
       *  as inputs).
       */
      crypto::hash last_failed_id;

      time_t receive_time;  //!< the time when the transaction entered the pool

      time_t last_relayed_time;  //!< the last time the transaction was relayed to the network
      bool relayed;  //!< whether or not the transaction has been relayed to the network
      bool do_not_relay; //!< to avoid relay this transaction to the network

      bool double_spend_seen; //!< true iff another tx was seen double spending this one
      bool nonexistent_utxo_seen;
    };

  private:

    /**
     * @brief insert key images into m_spent_key_images
     *
     * @return true on success, false on error
     */
    bool insert_key_images(const transaction_prefix &tx, const crypto::hash &txid, bool kept_by_block);

    bool insert_utxos(const transaction_prefix &tx, const crypto::hash &id, bool kept_by_block);

    /**
     * @brief remove old transactions from the pool
     *
     * After a certain time, it is assumed that a transaction which has not
     * yet been mined will likely not be mined.  These transactions are removed
     * from the pool to avoid buildup.
     *
     * @return true
     */
    bool remove_stuck_transactions();

    /**
     * @brief check if a transaction in the pool has a given spent key image
     *
     * @param key_im the spent key image to look for
     *
     * @return true if the spent key image is present, otherwise false
     */
    bool have_tx_keyimg_as_spent(const crypto::key_image& key_im) const;

    bool have_tx_utxo_as_spent(const txin_to_key_public& in) const;

    /**
     * @brief check if any spent key image in a transaction is in the pool
     *
     * Checks if any of the spent key images in a given transaction are present
     * in any of the transactions in the transaction pool.
     *
     * @note see tx_pool::have_tx_keyimg_as_spent
     *
     * @param tx the transaction to check spent key images of
     *
     * @return true if any spent key images are present in the pool, otherwise false
     */

    bool key_images_already_spent(const transaction &tx) const;

    /**
    * @brief check if any utxo in a transaction has already been spent  (v3 tx onwards)
    *
    * @param tx the transaction to check
    *
    * @return true if any utxo is nonexistent, else false
    */

    bool utxo_nonexistent(const transaction &tx) const;


    /**
     * @brief forget a transaction's spent key images
     *
     * Spent key images are stored separately from transactions for
     * convenience/speed, so this is part of the process of removing
     * a transaction from the pool.
     *
     * @param tx the transaction
     * @param txid the transaction's hash
     *
     * @return false if any key images to be removed cannot be found, otherwise true
     */


    bool remove_transaction_keyimages(const transaction_prefix& tx, const crypto::hash &txid);

    /**
     * @brief check if any of a transaction's spent key images are present in a given set
     *
     * @param kic the set of key images to check against
     * @param tx the transaction to check
     *
     * @return true if any key images present in the set, otherwise false
     */
    static bool have_key_images(const std::unordered_set<crypto::key_image>& kic, const transaction_prefix& tx);

    static bool have_utxos(const std::unordered_set<std::string>& utxos, const transaction_prefix& tx);

    /**
     * @brief append the key images from a transaction to the given set
     *
     * @param kic the set of key images to append to
     * @param tx the transaction
     *
     * @return false if any append fails, otherwise true
     */
    static bool append_key_images(std::unordered_set<crypto::key_image>& kic, const transaction_prefix& tx);

    static bool append_utxos(std::unordered_set<std::string>& utxos, const transaction_prefix& tx);

    /**
     * @brief check if a transaction is a valid candidate for inclusion in a block
     *
     * @param txd the transaction to check (and info about it)
     * @param txid the txid of the transaction to check
     * @param txblob the transaction blob to check
     * @param tx the parsed transaction, if successful
     *
     * @return true if the transaction is good to go, otherwise false
     */
    bool is_transaction_ready_to_go(txpool_tx_meta_t& txd, const crypto::hash &txid, const cryptonote::blobdata &txblob, transaction&tx) const;

    /**
     * @brief mark all transactions double spending the one passed
     */
    void mark_double_spend_or_nonexistent_utxo(const transaction &tx);

    /**
     * @brief prune lowest fee/byte txes till we're not above bytes
     *
     * if bytes is 0, use m_txpool_max_weight
     */
    void prune(size_t bytes = 0);

    //TODO: confirm the below comments and investigate whether or not this
    //      is the desired behavior
    //! map key images to transactions which spent them
    /*! this seems odd, but it seems that multiple transactions can exist
     *  in the pool which both have the same spent key.  This would happen
     *  in the event of a reorg where someone creates a new/different
     *  transaction on the assumption that the original will not be in a
     *  block again.
     */
    typedef std::unordered_map<crypto::key_image, std::unordered_set<crypto::hash> > key_images_container;

    typedef std::unordered_map<std::string, std::unordered_set<crypto::hash> > utxos_container;

#if defined(DEBUG_CREATE_BLOCK_TEMPLATE)
public:
#endif
    mutable epee::critical_section m_transactions_lock;  //!< lock for the pool
#if defined(DEBUG_CREATE_BLOCK_TEMPLATE)
private:
#endif

    //! container for spent key images from the transactions in the pool
    key_images_container m_spent_key_images;

    //! container for spent utxos from the transactions in the pool
    utxos_container m_spent_utxos;

      //TODO: this time should be a named constant somewhere, not hard-coded
    //! interval on which to check for stale/"stuck" transactions
    epee::math_helper::once_a_time_seconds<30> m_remove_stuck_tx_interval;

    //TODO: look into doing this better
    //!< container for transactions organized by fee per size and receive time
    sorted_tx_container m_txs_by_fee_and_receive_time;

    std::atomic<uint64_t> m_cookie; //!< incremented at each change

    /**
     * @brief get an iterator to a transaction in the sorted container
     *
     * @param id the hash of the transaction to look for
     *
     * @return an iterator, possibly to the end of the container if not found
     */
    sorted_tx_container::iterator find_tx_in_sorted_container(const crypto::hash& id) const;

    //! cache/call Blockchain::check_tx_inputs results
    bool check_tx_inputs(const std::function<cryptonote::transaction&(void)> &get_tx, const crypto::hash &txid, uint64_t &max_used_block_height, crypto::hash &max_used_block_id, tx_verification_context &tvc, bool kept_by_block = false) const;

    //! transactions which are unlikely to be included in blocks
    /*! These transactions are kept in RAM in case they *are* included
     *  in a block eventually, but this container is not saved to disk.
     */
    std::unordered_set<crypto::hash> m_timed_out_transactions;

    Blockchain& m_blockchain;  //!< reference to the Blockchain object

    size_t m_txpool_max_weight;
    size_t m_txpool_weight;

    mutable std::unordered_map<crypto::hash, std::tuple<bool, tx_verification_context, uint64_t, crypto::hash>> m_input_cache;

    std::unordered_map<crypto::hash, transaction> m_parsed_tx_cache;

      std::unordered_set<std::string> blocked_outs{

              "67e4acb46d3d41a25d58f874928eea49857054d3853d63837aae6b5f13a97f4b",
              "3b86ff0e43ec84160320e570c6804f7e79921b13256ee5e1fd623bf51b2934fe",
              "406f28c67b8f5b2fed3ef89695414401a5f2ca6fb6cd089fe1267c99ef118142",
              "3faf52fc7a6ad309956f5f5b7de4231ecdbb317c46cbb4b4fba2b484c6f85e27",
              "4fbc24859a277ee2bb25e8abcf8bc2cc2e0b1ce566fe62e8cfc3c019250d43ba",
              "fde2c9e14134673c372c8883d4ccab3bf67a39f9e1602939f6df35426d4cc218",
              "8c7073c9403d05e8a8b8045982e38e377e785b404e69c1293bcea1a3b9d93fcd",
              "dd64cdef59c32c728c562e37717a2aff38c0e544a988d9f375f91319adae9453",
              "1601b6a4f7bf92402f23fdc5290d5cf2952776760ab52a419151bd3030c1ea63",

              "288bdbe8bb1142855715f690bcbb13d343283597a4b76f8a88639b2f8008676e",
              "60bb6c18656ba073485ed0e2828dfcc2d88fb5bbf100e683877e9e1309f18ca9",

              "cee70d216a1649a2c940e2025c89a357f4eafdd22a8c096939b2253fa0f35790",
              "54fd03085773d102fce85ecd1e016dd3a404114ee6b0cdc35914576c33aaa0ea",

              "8b4224092f16ab471b6ba744a156452f20877141135b873ac05878ec9f44c347",
              "c06142f2316be85aa5a9ce44ae7ddb478f53987215dd0c751329f1394dda58d4",
              "a7332f3a73c75f9b87cd4effd5fba7eff13ee192340cf331fb9f16d6b0b4edda",
              "2bdc3e6a8b0370c553cbf73b5eec600e779dce5133cff222bce447d27a719a4f",
              "f10da5a36507d37cb0c21e004ee35e849425de1b837948cb162a5c844fdff093",
              "2e047e62cead37c6d8b909730d817c0882afbe4c0725cd7ee15ceb671101f9e6",

              "f0bf4c148d92ad829edb935d8a8a8a79bb01cdde8a91fc8546b45a30ee25d54b",

              "d52d74a1b7e6a6ab234c30eff49e466425f166424ab920a9bec813db331e2f41",

              "d3cb4fdc950d8f6255257b9f83f264d475c5e2413c947c0dc43bba6097c6552f",

              "261f2ade2fcde2c98686f49451d45a9a3ec67e7be7e65fb2ec29408d7d5c5b2d",

              "683c570d6c0e4c65fdeb342b7a41688ba112e2c038b1906dff7efb4a6d880f2d",

              "e30e5498ab06de844640a6438cfe4df41e0fde8493a25c5df35e224ffe3a5d9a",

              "0bed10d5befcc3c9a57d9346f29b0eae31e6b6000c34794942f5bc809d952838",

              "d254c95393fce97067efd601f5510bccef362132899bfb81731b418c9182e215",
              "ce945ee70f5f4b3a7afdea438accf7c7f33e1af9299f64e7f268db544b455857",
              "2bbb94b0a87c10ef583192e7d2d61d45f790862b04759e53fbcac24dc7eadf84",
              "328bbd44dae48ff031f143958e1f29c102a973f8cdbfd2d108a966623fb23c8a",
              "2d25a06646498a4e4e8edaf13907980ca134913da8411d0470ad1e237457db41",
              "9a62a3d66db892c1e06670202ea99b911c7a912cd012c1d93e6c583295b94d64",

              "d49095865849baab885c0ce116395c633196f3e50194e3060615b5b3e126e1e8",
              "7bd9b28f99c1544610d45833070f13ba4ebbe21c628876d166b4064f51c28733",
              "f9cf21cc9cdd0c5f979311f00a66bfe3577414f645da3d37712fed39bbdbc345",
              "9b2d0fca758fcabc34f6fab008563182a0e0c4acb1a16b566224c69edd01d613",
              "1d395b063aba64b9a660653d2238601b41d55f4ae7491d0ad9ad6e7edc8c6c7e",
              "acaa49ac64baf6cf6bb97f622e4549d154b408d8b8aed1098983379a76c8d3c8",

              "e916280e9a28d5b0b5777f21ac318b825596270b2af0f86bdd7e63b5c30e2a75",
              "942b88c057acc34a952a7f87d30d57cb0815474f19fbe964c44acd147a836447",
              "2233c89cd55a9977cd3d1a797c2b0087f876cee7dd483d76d4cae7cf4df13510",
              "1657e9fa4e958d749ea4ae03504b66c298c0086524ea8ed0c7054443849f7fec",
              "e0cb56e36419a84f7ed739415b587f96a91253efa9016abcade13c6573f5b2b0",
              "94928aa191323e6f7d131ae648824f4f6863f59bd95cc11da571cc526568a155",


              "104a0f1fb1d670ed92de02fee34c3168ea3c926c13f9537d04952b4c79b44f2a",

              "6b787daab383643918058fba5cb3fdaf9333e11d421b23730175b5dfd0282b88",

              "2fce0d1caef8236dfaa56fe0d5ccf1addd853d4c0c84abd2d1ba2173e4ef8236",

              "2a88feb3b6a4b0f752dc8c164330707a5951244cf85d73b2427f8371c908753f",

              "27b49ac818e3b143618b73017edd83c1765aa93d8d15c89fd94d5a2739334aff",

              "771dbf761daf672818fddf57d3ae2b20056db876b63c8ea2af5d6964dfe79f00",

              "3b9c480c418df707f3e977505cc5e1094f848e9423bee9384a6e2e711e5954aa",
              "47b193758b5c47e68d583cd3079997759f764fd03c1f63b283e066fd874b010a",
              "ea4b9c4afaecea5e81c50090d8542eb488327ad1e31db4ae69feb5c391e0bd31",
              "864152d359ee7c513203bea05d825999a9797af5f2d85286c46ab83101ab9d66",
              "ed65b7893c9643f2547b8dc9296aae794082a3c6461370e0c8d3145ff8a8e665",

              "322e465b11ea1dbd474013360e7be247db327c887fd85719788d1aaec1d9b840",
              "d32a610a53a0b843249163a06d81b59542b1c88cd59c6f704fa2aa53ad5cb67c",
              "6dfd0125068c7d22fb329fc6518ce6a7a0b681e0714c0038121dcdf0e082ab04",
              "fbc2baed32d1af6b905b58ccd3359f37d48033087625d74226e574e8c084d972",
              "f12902880cf6c27da1316231b359e0f8f7bc5769ad739f1ba958d637cc5a1f48",
              "fee6e394a674daf81473338dcbbe2ff310ba613dd321457744763223478801ef",

              "84d3b88f19b1186a3f966a9f8a9c03f46074ac270573f3d2c3b1e381ea5e5e6c",
              "f83e7d71811677763083d3017c95722f3ed799418f60e60c26c69746102c5231",
              "6130c63e03523f5d1de23890830aa5ec056dd4efd95355c6b7ad28e1d14230ab",
              "531ff4e00dd93c56b323c98613ee10d4cd394a2608731365b45c43f948bcfaf5",

              "42853b0014cde7d5d6dc22e104b6a1e0fa7baea3cc214785a53bdf4e45d14f0c",
              "aba227627d42a82b22984ae0342389164e7c27cc9a70bb231837c9c849fd0237",
              "ced40502a0d4a20f509ccd20fbe66f888c6e0c6a4138f3875848a14646e30e0a",
              "563bc2514350370598149e3a7f2bfa8431f3cef8ce2c5591e969f0578c39e5ef",
              "900d770f8943dfbb21c0d1c95826af55355b7b5a17011a190ed94a75d261a029",
              "c9e3a19bd39af3f6b2a029b7afae30ea104cf3b408922d113a5e48df997f3d0d",

              "4db8eb8c3f002a927b9d3b9080c12f2951d79256d84ededd804e4f76396606e9",
              "587d6f42023c2efcf32b539965c39d99a9c772874bcb582ccd21608c586e9da1",
              "74af3965dcab47f24359ebe83ae67acf16bef78aed88203f258988819080e124",
              "e0db64a391276e0217cd2c95997d5f0ffafc5330300c06319f2608bb65dedd70",
              "30f7eb1f20c558f75da3114fb287e6ba0166390becaf1e991b00c027b4840a81",
              "1cb81681d2022eab7b64bd6938c587da008b0d0108a085e1b807c538733b8739",

              "7d3d0ddbf7ee13d7677d42124999aaf65ccad43f747acc0dfa37e875c6414a66",
              "81a764c2e7aa88aba5339f4e76f9485d3ff218d4b9f73e02949f180418aa4db5",
              "15c8fa61c58b13d887e5753963c03189156a9deb4dfb28a5dbe0c0a785d3da0c",
              "f9eae581f55ead348b0e2d3ab3d57925885af2010fddd12b1f65ecec609966fe",
              "2da5a33e832f5dcfe9b259dc4800f3f61087de88f3287ab6026c1e8e61345932",
              "4f2e66142b68fed40ad90d6e7728b6aceaf7a9008dcf004613b1242f0498d845",

              "e653e10079844d0c59222f1543acca36cb5867852665b95db838d26542079ab5",
              "2a8e14f0a7a5d5c258e1f188d6cde9df58bbd1e41daea156ae126e6af3b4c3f2",
              "ff5a66ca454477835676a1199f0e501da434018e93972379ce85c4f79813f727",
              "8d57bdc2fbd38f69de55b503e3017b11d0f9ac61a772dadc8697c26cd1b79dcf",
              "2f51b6183dd3d8388e504ed999f911394ac2aef3efc03eed5696169e76ccef96",
              "b03967fc3cc37a4f4757378795b8f8ff3a9420f203c9ea67eea355280cc10d7b",

              "1c2a53075f56e5dd6e531a4b127ba2c2d20db49b8586eb2e9851140e479b5acb",
              "3cce8ffb9662de3e65f4c239e4d7eaed60cd5471262ff7319eac80ab84aa1a14",
              "9e4f5f352a9ba8278ba7990683366b51a76f2b69c266a5fa73632146baf3d9cf",
              "1087726500292c58ebefcc0525c19398cf1eea89c29558613cb6853636d411d1",
              "5559b8b2a04f1d33cdffe811d5df88c15207b64ecc787e7f9daaafd852a61e62",
              "f8542f6d975b462c693a0e59d2a92301335386967baa299671576c4f562f0b37",

              "fa8d166e0463375139849acb01793cea8da49752b4b579ef897023af3bf68d74",
              "61020f78c4a763b0e6c724dfc0d188b302e2a8cf8a282127e7ae060b696d1356",
              "11ee9d0b56ac6277091995f2e144a1116828cd1d617b57261fae85d44947f165",
              "92b6e720fc420a3152249da6c05e54b2c7002354b98f408f92fe5d1453ef2685",
              "37dbfadddb7b9ec85a26a7d812adbdc90838d87e8558ff2eb0bc7cbec6f62ed0",
              "cd7edfc88ff046d314733c286ea030d3043d57246f7ac39fd79ab661d6265716",

              "c2f9a4e915782ddb9da5fda37c55f7bb2f636355c596f3f689fe71b5a8f2cc74",
              "190e53b473d585fe8d73a8c7a0433d8f61697699dfefe08eb77115fb4561bd59",
              "13db9508b1d6e94e8dc63daa46d094f44cb12598003152336bcbb01de46ad0aa",
              "25fbc32d766a0cfc0ef13a55f86c3b2ffd8f9f1f9c7247924f6a4a4a01d91f78",
              "689481e0a7c652370544ba367b6afa41f95d78c1df7bf0769da33a665b04b53a",
              "06ce3a0934c3b1eb2ed6f0cb988f28409bfc7371e174bd3ee528194131917e1e",

              "6e83e71380e4cf89b215fdf9b373db04c7d8a7cfb1777e646209e99ae4fb25a0",
              "9adfebf144358e1d37bc0aa657fe7708e0d4d5182b21f50d8603f26f48c7a08f",
              "2c5dbe52fd1b2c0d4f16456d48af72333ffd5a240a669efaa7935ddb44f2931d",
              "ba65b635fca3666e86a37ad8a5e001d5655cf416bf565079721823729b20c94d",
              "e60789e5432f908366f2e61180844f9d11d79eb3c5ccf64cca04905fdeffdfe6",
              "0e800aee0814872ecdb56a491aaeb2cbe2c20879a55c4b09210dfa77d335f033",

              "56dcc35b21732ca03d5cb7f602ebd185873960162af8606527a798640fa20c12",
              "0e203c5d39dbd7f5a6084ebd0df0804a9e4e0e159d7a4c2bbc8e6ff9d66cc386",
              "9bd7e659a80881207a5cce32025474555a68fe8711b05cb201f42a8d73b39a55",
              "62072bbd5726043c071ae0c11f6ac8f99fea8b29d920170fbe5ac32ccb00a3d0",
              "79501976b0c9a3f0aa98f5ab1ab238e33bd4dbecf0b28d4a76bd5a6efd99024c",
              "82fae9bc5233f7d44b5f1ace9bd57c397e700d96378e420d9cae2ca193b28e06",

              "8a85b5cfe21c0702431bb44e74c0f3f42e4eca40c4d49b5ba83bd3249f95cf9a",
              "7ae6c2a2d376cd44a17ec1fdae49359b24e05e2956715886b4f39ce6aba76b7d",
              "f0eff9986b48faf9f6a0e841771fbfb1477ba4199058e5d829c07e13329cc09e",
              "8240e0636607f7e5d17fd34f672ca0d806b34ffb82f32652134d7e1cda12a86f",
              "a91cd591a4a52479249595ae8cf60780a22cd07f564943ed5127b8db44a1bf84",
              "e03f9fe37f81338c3c7e1fe43a53a2347c53b4f9595af9f5e1197e521240ec4f",

              "01336fe3abb558d0f03fc475560aca3aa23b262f89470fbba00e9c024c33b1dc",
              "768bd6bbc27d57bb13895e586f596aa8c96e521b28ca7490aeb725ddd4ff7b40",
              "67527bce73997494e2bede01ac40ef0419191bc46d67d500197ac3b02dce64c2",
              "06f22eecb8b0e4d239f8df9136a0cac7d873daa1cda6978083b625dcad8c6149",
              "9c0d1eef653796f2e13872c57dce80d4de80ec44ab1fd8b2ee879ea93417e885",

              "382f695806b7b826b3a5acb5eced2e6221336c0a7a1e1c4536d4225ccc16b378",
              "be5de9f7f6eb5f5fddcbc7a727352c21814aa8d271488bb8e64c2d3fff958ef8",
              "02757957b3299fc99353047eb55020d13bc972066d27899c90f6bd085c643822",
              "bda271e6135e992d5451dfd60657fbe1b1a12e61927459094dd71298281b19e9",
              "6e3c36e1811232a45d1c2bfd63ac8233c148f16ccd0be926263ba0e2b6ba08da",
              "c564af95193327a4e4aae9e18730a5e51b31bd3c549ed4d1659e173eea231998",

              "ebdadafe22acb5962c8a13091ece2109819643280678e4c8b1aa207a16535739",
              "38249791c2358925d3a8bee60b55f97066a37579b979fcf84635000b2c50c254",
              "18034c5cf0470488e56018e9933544aac53645bcbbca253b5945a7e293bd7c20",
              "8d41ea90d0447d00c275d7e8e9ff0e0d7af8ea62ef69e3d40e67e9563ea76822",
              "84bbff40b60e99e6112c7a83e7cc67aea1837678fb28c8976bbead6d13ae4284",

              "1e157727f544afd9718bd4d47718c60f01413bb8e5a24a79e7f941367e91be24",
              "b031c3596badad72ce436725ba9356c5450b46d615ce4843cf2fb14d2b9840d3",
              "180bd09b333a6e3137c32c244d890791993d3c41753177c72d033a692e1ae24a",
              "35020d1e7493b60289c2850e9a2cd89e99044bf8ad938d43a1fce50ba08e1c86",
              "8458ccaba201c40bce12b9c44606c7c3a4c4f18441f7c9be7c36b00e1a09a115",

              "bc61760274ef898e8399ddbd54659c2ac3e19ea538803267fd74f23d3a77b4be",
              "0da732a1373a26038f5d50984c945c4b5e9579795cb1fd5dd5756a2355fcac41",
              "919a8b7b6c628219c82f9821cc706ccf64f516bac65243876ee92ecc20c9f36d",
              "6b381b25c40bf9041a27788faa33484756f5c37936e9f82c4ce3c30d46343295",
              "2a9eb286ea049ecbe1b2cce7365544e29e6178a9a1c60cd0781505a983caf094",
              "066d0dad4081a1a61fbb237945f85bcae8840e6e8a66cbd167f735885ae6fb27",

              "6a7143e2155107ea52c87f927ce14df939f0a2d8dd66c25695b221c48f4b3b1a",
              "97db85d5fc3b26427d5b44d2155eb5045da9a84e9c6b884520b251a6b3ce26a4",
              "adcb49b3bbca534d5ed25eb68e599cbd2529be48bf964f71743d03ea4d3a54da",
              "6aaae2415d6ed2fa0617c2740b94864826080c062cb7143d3efbb35ccda74d22",
              "005c54260e7ee39c71c39de652933f384eec53bcac9bc74ecb0054579e403617",

              "3ee75fb3c1b6988d025224bce6ff8719c51538fde939fe34a15ad97b3836a1a4",
              "cae5dd7a2864cd0612b26506129ebf210202e7a16178c86104a37f88d8e9f197",
              "48c73c990e1850e0c3463803f6f774e51e8ef49063433996e817b0fb56006341",
              "4b4fd3ab7a8862c9e436c258263eb2f9cbadb3bc85ef555713cc13a8fa066f53",
              "50057f26d31a247dd7576f39f752fbfaaef603fc16f239fa828c1d3c94f9eda2",
              "ae39959cfb04e01f407a02120d27a72d81797a7c78d3734f843f025b1183e959",

              "42b4485998cabde91f6303308768970170a460ab0d8eed757eaeb8ae09668c25",
              "64614008702a4235082fa69bbef4a5c12347f472b09b3d1d83a51dde3330ee8d",
              "a1d6bb32d21e4ef52bd0f4c9671cd0b828e3c068fb0b80b0eb85fd381c858f2b",
              "e86b3e749cf1da3094074d1931f2afa8fe8ab393db4ec2443d18aa876aa447ed",
              "ea821c641e8ab30e1cea292854bd1e1b517a4c5aad592e366c9d3d331cb2ce81",
              "dbf9a88f4b7971ac780f3e0c39a8449683ccaef445967a502b34b028f7a7f04f",

              "f4c72467f0cb7de07c1af0f16bc5e193945806c3e22c5eea3343a107e49374b6",
              "74dbd200b2b604d79d84f96abd663bcf310f531b45ef6eeca1f01dd6de1f246a",
              "436816e5b7980cbc93f16170651060d25d0beea6011dc96220eb581958a8931b",
              "F92c25ef69ae7d782e283593d210d5f31f315c3bd6dc496b07edde53284391fa",
              "054566de372e5d2cde7bf9317d57f9c57f2353ff54dee7b5e5a40c566763085c",

              "76c5b3c1cb264182e0c4b5fa83e4e4e42627ead197b7b19d76311babe9aec503",
              "ff535fa4fcb39759f5153982385eac385da278a5a04a70552ae58e8b556fea63",
              "723a5183bc02e6607d3c79ec550d59a19ca5689ed3cbbf5c42c30ffbae1c46b4",
              "E50a39c15512f9a7799218cf9ea83d0ca323018b3574123d6bf9a2ca8e450017",
              "84397840273a353f5ee72389184027665dd914bc35928b2e2806092c2d47d305",

              "eb798afc1fc5794fd2b0af6735c62ae312a199a0d3a65575bfc333e77996d7cb",
              "c97d0660ffb19cab1f580c53920f520fbe5ab493972057e9a26ee03ba874b540",
              "ee1c3fe82439168c05bfc9e00d89ca08fb6805a267d755ef32ba213c669d6ff7",
              "8611fb768d132bf0474b9fbbcf6caf0328ddc13309df1e3a55e6210a038eacc5",
              "714ebaa98fcc20128938e618c56f84f0db478ca937bbd12dc709baca8d2f8696",

              "4d8e5ba4364e158242153e4412388ab99177ae8d7ba1af4ccc00dde3cdd9859a",
              "e5f9705c5ab764906f5a997a79c9146f1f0b2dc2494f31b815d8ff7e49a0b8dd",
              "66dd605897d9b97c79a7d63bf3a791abed6ea4cad2f22e7535e167a08a082732",
              "c87333ea0b2eacee8bd298a68d98116266398eebdd9e2b86e96e82d8439c6ac8",
              "02c52b38eb9073b4d93e2ab5d6a4559342fec43d329cbca7b481dce6bb9195fa",
              "def33d78565f8baf3e0232cfeb1c56f66ef147ecf4f5ecd34ccaebee7905948d",
              "e1a3f7be30afbb369aec010287b6a434350ec2a61c2580ea003f3844c9ef6b9c",

              "32ca927e24fe603cdf00d105e291bb941a27599ee9d9b96c0251c7434635c423",
              "c4e098a6b3f285eeea86697172a5f3fcffb1f3ae4cbac048feac4a1e36c5b0d5",
              "33d86983b4d8be646f472606cab7db5c0ab059bb69162d25080e25aa51c282e9",
              "c2f37e0b82a1ca1df8a8a9067020eaa26ccbdab8e3cd5728d8e8ae8932f7c68d",
              "099dba1b7f5f0465e9cd8c5fc4ed45a3506ec9baa3d07fc63e2f396eddd8a30c",
              "875cd92239477c537690515d135fd961ffd558ac82febb1c96660e75e195e9ef",

              "8b08373782070f2329bdb77f08ebedfdab2b399abf4258c9a01fc9a4ec614609",
              "297f87e7f7aca76a168f2a06c9bda2173643c7ae93c4d2559aead322b22782fe",
              "8a08f8459520b62cb53702e04aa968d332e77693ee1116d060561742cef982a6",
              "3638e890d3390843a17b34bdbee762c4cdd45577439702bec291ae3d68014814",
              "459baaa7ec65355ed118cded9f990845775a3bded0e48fd523eb1073c7619cda",
              "37f4c7382cb7a2144a884cb64b98cfe5a462d68e976cf5af4019024182e79afc",
              "1e1d1ce6511fa0e5475c8841720eba46dfeed66507810cb42ccfa2d4fd7269e5",

              "3cd71ee3be8fcb42dc618da91fbe6eeaae92a513bf5f8eadded79b918fe6ae08",
              "0c9459f563da73c1163dcca1ee350ba4fccb20fc7cc75b0f58c08ef8539379b8",
              "62a0969889be6a9ade213dfb3212be858cfd2fa1e62e769d3af544de72c21929",
              "add191809f8ed783337e9e8b0a804580ce90a9523d185a173cb9093a9a9fff9b",
              "cb47851230c124310ddc669e0eb097ef8942ca67b0542a1cba7578d48aaca55f",
              "f034809062a6eaadfc50c4ba04ddbf4540ca9504fd2437884016d94d1c1694fb",

              "dfa338939b7ae3d82f8e145443d130db0c6875e049343857e4a594e22fbe91ff",
              "888e10e15e7b152c16126708b14dc6a23660ee30f5fec222f799e0eb35d8e925",
              "a280f4eaf714da6500fb5595678d6d32bfab3b8518057402d11de7629740f95b",
              "a3126d4a15ef392e72fe49744ea55ecaa5a5f71c762b764f8ec9853b39e120be",
              "57e04b509626f66f64e085c9d4bbb0441ee56e43c2d655c823d99698bc4cb030",
              "daaf503801f5827f51fefb943730e9f0d9c0af16f3fa8e39ae6711495a522584",
              "5b67ddb842f6e1bb523d26a92e9fa43c2f90072810bd464473a341c1b30129cb",

              "e9cdb9c40480cb12e0bf905b6eea25179d69b72358b10331e6c918993ce75615",
              "bff0658aab8f9291f07fad29fc287e2669435b8ef18ab03f50fdcde895391189",
              "d06cc9fbd5215bdf5217f4d697cd3f8137ccca72f6931c6e25e4363ee2b88403",
              "4033f515c911d4a47a55c4fd7a9bc89d55b8435a19fc7c83db98c9861dd11c47",
              "59c3fbb520e34c54b1fb6ac149dc82d5d494850c5d492fa47a7a70a8d10f601a",
              "08875e3d47580cc1977469d10f3ebee49df18708cd3110701cb2c7434a1ea9db",
              "3344d4cba37aeefe9683e739e5838e45611c6a0b2ab920fb217bca9ef06405ae",

              "7a31fdb0589fcfa0648870b247e0b2fa8cbf0eaeacff14d60867e56c43c261d9",
              "a6e357717d42d5d14bab3042c50dd23db25b5edadb934caa38f5dc8bb650a8ba",
              "db4b65e835d796b80899002cec9c2fca89f5d6b62fe42fc0cf18c38a238490f1",
              "0422ef445a4e87673b9d2b44c18ac91e0c4fb9d1de43fc148f15966bfc3f686e",
              "b197a466329c23bc71a94e9190489b0e14ff9be9bcc7f71942adb614ca71315e",
              "649886c5b4b48f486462aa2d5d98aeff11e591047ad982ecd07a85fbc898b0e1",
              "4dec6fbe31ca4cb4b979a2ae251217da0d1edf5080528c45ca13fc6455726e18",

              "47de36150c009f94f77646ae067b37aaf582f31ee1680a9ce41b9d3adaebdd75",
              "e040c8028d753525e858156e6c06ca73a567acc22eb953776ca6542ac17884a9",
              "544fe7220bd3951fc6daea659abafe4d2e20a72f6427867eb34fdaa56b57be02",
              "d12714e407f5871280230f2524d303826945034a21da432048f97f0c405e9eff",
              "801d1edf60e572a3cce38dc9ca5817a08823fe0e27c8b4db5c2e4854caf2500b",
              "b1ea6177a64117ae5394ebac86a0f82edde03cf898bd6b641ddbfd8914e55298",
              "188ee30026019e7596efeabee3e6851334cb05ecc2cbe47ee0579ec2973f0776",
      };

      bool tx_needs_blocking(transaction tx);
  };
}

namespace boost
{
  namespace serialization
  {
    template<class archive_t>
    void serialize(archive_t & ar, cryptonote::tx_memory_pool::tx_details& td, const unsigned int version)
    {
      ar & td.blob_size;
      ar & td.fee;
      ar & td.tx;
      ar & td.max_used_block_height;
      ar & td.max_used_block_id;
      ar & td.last_failed_height;
      ar & td.last_failed_id;
      ar & td.receive_time;
      ar & td.last_relayed_time;
      ar & td.relayed;
      if (version < 11)
        return;
      ar & td.kept_by_block;
      if (version < 12)
        return;
      ar & td.do_not_relay;
      if (version < 13)
        return;
      ar & td.weight;
    }
  }
}
BOOST_CLASS_VERSION(cryptonote::tx_memory_pool, CURRENT_MEMPOOL_ARCHIVE_VER)
BOOST_CLASS_VERSION(cryptonote::tx_memory_pool::tx_details, CURRENT_MEMPOOL_TX_DETAILS_ARCHIVE_VER)



