// Copyrights(c) 2017-2019, The Electroneum Project
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

#ifndef ELECTRONEUM_VALIDATORS_H
#define ELECTRONEUM_VALIDATORS_H

#include <boost/algorithm/hex.hpp>

#include "validators_commands_defs.h"
#include "include_base_utils.h"
#include "net/http_client.h"
#include "storages/http_abstract_invoke.h"
#include "storages/portable_storage.h"
#include "crypto/crypto.h"
#include "cryptonote_protocol/cryptonote_protocol_handler_common.h"
#include "math_helper.h"
#include "blockchain_db/blockchain_db.h"

namespace electroneum {
  namespace basic {
    using namespace std;
    using namespace std::chrono;
    using namespace boost::algorithm;
    using namespace epee::math_helper;
    using namespace epee::serialization;

    enum class ValidatorsState{
        Valid,
        NeedsUpdate,
        Expired,
        Invalid,
        Disabled
    };

    enum class list_update_outcome{
        Success,
        Invalid_Sig,
        Invalid_Etn_Pubkey,
        Old_List,
        Same_List
    };

    class Validator {
    private:
        string publicKey;
        uint64_t startHeight;
        uint64_t endHeight;
    public:

        Validator();
        Validator(const string &publicKey, uint64_t startHeight, uint64_t endHeight);

        inline const string getPublicKey() {
          return this->publicKey;
        }

        inline const uint64_t getStartHeight() {
          return this->startHeight;
        }

        inline const uint64_t getEndHeight() {
          return this->endHeight;
        }

        inline void setEndHeight(uint64_t end_height) {
          this->endHeight = end_height;
        }

        inline bool isWithinRange(uint64_t height) {
          return this->endHeight == 0 || (height >= this->startHeight && height <= this->endHeight);
        }
    };

    class Validators {
    private:
        vector<std::unique_ptr<Validator>> list;
        uint64_t current_list_timestamp;
        epee::net_utils::http::http_simple_client http_client;
        string endpoint_addr = "vl.electroneum.com";
        string endpoint_port = "80";
        string testnet_endpoint_addr = "vl.thesecurityteam.rocks";
        string testnet_endpoint_port = "80";
        milliseconds endpoint_timeout = milliseconds(10000);
        string serialized_v_list;
        ValidatorsState status = ValidatorsState::Invalid;
        time_t last_updated;
        uint32_t timeout = 60*60*12; //12 hours
        uint32_t timeout_grace_period = this->timeout * 0.1; //10% of timeout
        bool isInitial = true;
        once_a_time_seconds<60, true> m_load_validators_interval;
        cryptonote::BlockchainDB &m_db;
        bool testnet = false;

        cryptonote::i_cryptonote_protocol* m_p2p;

        void add(const string &key, uint64_t startHeight, uint64_t endHeight);
        void addOrUpdate(const string &key, uint64_t startHeight, uint64_t endHeight);
        void update(const string &key, uint64_t endHeight);
        std::unique_ptr<Validator> find(const string &key);
        bool exists(const string &key);
        list_update_outcome validate_and_update(v_list_struct res, bool saveToDB = false);
        ValidatorsState validate_expiration();

    public:
        explicit Validators(cryptonote::BlockchainDB &db, cryptonote::i_cryptonote_protocol* pprotocol, bool testnet) : m_db(db), current_list_timestamp(0) {
          testnet ? this->http_client.set_server(this->testnet_endpoint_addr, this->testnet_endpoint_port, boost::none) :
                    this->http_client.set_server(this->endpoint_addr, this->endpoint_port, boost::none);
          this->testnet = testnet;
          this->m_p2p = pprotocol;
        };

        inline vector<string> getApplicablePublicKeys(uint64_t height, bool convert_to_byte = false) {
          vector<string> keys;
          all_of(this->list.begin(), this->list.end(), [&height, &keys, &convert_to_byte](std::unique_ptr<Validator> &v) {
              if (v->isWithinRange(height)) {
                const string k = convert_to_byte ? unhex(v->getPublicKey()) : v->getPublicKey();
                keys.push_back(k);
              }
              return true;
          });
          return keys;
        }

        inline bool loadValidatorsList() {

          v_list_struct_request req = AUTO_VAL_INIT(req);
          v_list_struct res = AUTO_VAL_INIT(res);

          // Try fetching list of validators from JSON endpoint
          if(this->status == ValidatorsState::Invalid || this->status == ValidatorsState::Expired || this->status == ValidatorsState::NeedsUpdate) {
            if (!get_http_json("/", res, this->http_client)) {
              LOG_PRINT_L1("Unable to get validator_list json from " << this->endpoint_addr << ":" << this->endpoint_port);
            }

            this->timeout = 60*60*12;
            list_update_outcome isJsonValid = validate_and_update(res, true);

            if(isJsonValid == list_update_outcome::Success || isJsonValid == list_update_outcome::Same_List) {
              MGINFO_MAGENTA("Validators list successfully refreshed from JSON endpoint! Timeout = 12 hours");
              return true;
            }

            //If the list was old, invalid, or had an invalid public key, try getting list of validators from peers
              if(m_p2p->request_validators_list_to_all() && this->status == ValidatorsState::Valid) {
                this->timeout = 60*60*1;
                MGINFO_MAGENTA("Validators list successfully refreshed from peers! Timeout = 1 hour");
                return true;
              }

              //Keep returning true during the grace period
              if(this->status == ValidatorsState::NeedsUpdate) {
                LOG_PRINT_L1("Validator List Grace Period");
                return true;
              }

              // If we there was an issue with getting the list from peers, try getting list of validators from db.
              // This code will only be reached at the daemon start if both endpoint & p2p list are unavailable.
              string v = m_db.get_validator_list();
              if(!v.empty()) {
                this->timeout = 60*60*1;
                list_update_outcome isDBListValid = setValidatorsList(v, true);
                if(isDBListValid == electroneum::basic::list_update_outcome::Success || isDBListValid == list_update_outcome::Same_List) {
                  MGINFO_MAGENTA("Validators list successfully refreshed from database! Timeout = 1 hour");
                  return true;
                }
              }


            return false;
          }

          return true;
        }

        inline string getSerializedValidatorList() {

          if(this->status == ValidatorsState::NeedsUpdate || this->status == ValidatorsState::Expired) {
            return string("");
          }

          return this->serialized_v_list;
        }

        inline list_update_outcome setValidatorsList(const string &v_list, bool saveToDB = false) {
          v_list_struct res = AUTO_VAL_INIT(res);
          load_t_from_json(res, v_list);

          return validate_and_update(res, saveToDB);
        }

        inline bool isValid() {
          return this->status == ValidatorsState::Valid || this->status == ValidatorsState::NeedsUpdate;
        }

        inline bool isEnabled() {
          return this->status != ValidatorsState::Disabled;
        }

        inline void enable() {
          this->status = ValidatorsState::Expired;
        }

        inline void on_idle() {
          if(this->status != ValidatorsState::Disabled) {
            if(validate_expiration() != ValidatorsState::Valid) {
              m_load_validators_interval.do_call(boost::bind(&Validators::loadValidatorsList, this));
            }
          }
        }
    };
  }
}

#endif //ELECTRONEUM_VALIDATORS_H
