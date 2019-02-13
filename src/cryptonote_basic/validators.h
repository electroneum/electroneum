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

namespace electroneum {
  namespace basic {
    using namespace std;
    using namespace std::chrono;
    using namespace boost::algorithm;
    using namespace epee::serialization;

    enum class ValidatorsState{
        Valid,
        Expired,
        Invalid
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

        inline void setEndHeight(uint64_t end_height) {
          this->endHeight = end_height;
        }

        inline bool isWithinRange(uint64_t height) {
          return this->endHeight == 0 || (height >= this->startHeight && height <= this->endHeight);
        }
    };

    class Validators {
    private:
        vector<Validator *> list;
        epee::net_utils::http::http_simple_client http_client;
        string endpoint_addr = "localhost";
        string endpoint_port = "3000";
        milliseconds endpoint_timeout = milliseconds(10000);
        string serialized_v_list;
        ValidatorsState status = ValidatorsState::Expired;
        time_t last_updated;
        uint32_t timeout = 60*60*12; //12 hours
        bool isInitial = true;

        cryptonote::i_cryptonote_protocol* m_p2p;

        void add(const string &key, uint64_t startHeight, uint64_t endHeight);
        void addOrUpdate(const string &key, uint64_t startHeight, uint64_t endHeight);
        void update(const string &key, uint64_t endHeight);
        Validator* find(const string &key);
        bool exists(const string &key);
        bool validate_and_update(v_list_struct res);
        ValidatorsState validate_expiration();
        void invalidate();

    public:
        explicit Validators(cryptonote::i_cryptonote_protocol* pprotocol) {
          this->http_client.set_server(this->endpoint_addr, this->endpoint_port, boost::none);
          this->m_p2p = pprotocol;
        };

        inline vector<string> getApplicablePublicKeys(uint64_t height, bool convert_to_byte = false) {
          vector<string> keys;
          all_of(this->list.begin(), this->list.end(), [&height, &keys, &convert_to_byte](Validator *&v) {
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

          if(this->status == ValidatorsState::Invalid || this->status == ValidatorsState::Expired) {
            if (!invoke_http_json("/", req, res, this->http_client, this->endpoint_timeout)) {
              LOG_PRINT_L1("Unable to get validator_list json from " << this->endpoint_addr << ":" << this->endpoint_port);

              return m_p2p->request_validators_list_to_all();
            }

            return validate_and_update(res);
          }

          return true;
        }

        inline string getSerializedValidatorList() {
          return this->serialized_v_list;
        }

        inline bool setValidatorsList(const string &v_list) {
          v_list_struct res = AUTO_VAL_INIT(res);
          load_t_from_json(res, v_list);

          return validate_and_update(res);
        }

        inline bool isValid() {
          return this->status == ValidatorsState::Valid;
        }

        inline void on_idle() {
          if(validate_expiration() == ValidatorsState::Valid) {
            return;
          }

          loadValidatorsList();
        }
    };
  }
}

#endif //ELECTRONEUM_VALIDATORS_H
