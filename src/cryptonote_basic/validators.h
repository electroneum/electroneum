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

#include "include_base_utils.h"
#include "net/http_client.h"
#include "storages/http_abstract_invoke.h"
#include "storages/portable_storage.h"
#include "crypto/crypto.h"

namespace cryptonote {
namespace electroneum {
    using namespace std;
    using namespace std::chrono;
    using namespace boost::algorithm;
    using namespace epee::serialization;
    using namespace epee::net_utils;

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
        vector<Validator*> list;
        http::http_simple_client http_client;
        string endpoint_addr = "localhost";
        string endpoint_port = "3000";
        milliseconds endpoint_timeout = milliseconds(10000);

    public:
        Validators();

        inline void add(const string &key, uint64_t startHeight, uint64_t endHeight) {
          if(!this->exists(key)) this->list.emplace_back(new Validator(key, startHeight, endHeight));
        }

        inline void addOrUpdate(const string &key, uint64_t startHeight, uint64_t endHeight) {
          this->exists(key) ? this->update(key, endHeight) : this->list.emplace_back(new Validator(key, startHeight, endHeight));
        }

        inline Validator* find(const string &key) {
          auto it = find_if(this->list.begin(), this->list.end(), [&key](Validator* &v) {
              return v->getPublicKey() == key;
          });
          return *it;
        }

        inline bool exists(const string &key) {
          bool found = false;
          all_of(this->list.begin(), this->list.end(), [&key, &found](Validator* &v) {
            if(v->getPublicKey() == key) {
              found = true;
              return false;
            }
            return true;
          });
          return found;
        }

        inline void update(const string &key, uint64_t endHeight) {
          find_if(this->list.begin(), this->list.end(), [&key, &endHeight](Validator* &v) {
              if(v->getPublicKey() == key) {
                v->setEndHeight(endHeight);
                return true;
              }
              return false;
          });
        }

        inline vector<string> getApplicablePublickKeys(uint64_t height, bool convert_to_byte = false) {
          vector<string> keys;
          all_of(this->list.begin(), this->list.end(), [&height, &keys, &convert_to_byte](Validator* &v) {
              if(v->isWithinRange(height)) {
                const string k = convert_to_byte ? unhex(v->getPublicKey()) : v->getPublicKey();
                keys.push_back(k);
              }
              return true;
          });
          return keys;
        }

        inline bool isEmpty() {
          return this->list.empty();
        }

        inline bool fetchFromURI() {

          http::request req;
          http::response res;

          bool r = invoke_http_json("/", req, res, this->http_client, this->endpoint_timeout);
          if(!r) {
            LOG_PRINT_L1("Unable to get validator_list json from " << this->endpoint_addr << ":" << this->endpoint_port);
            return false;
          }

          //Check against our hardcoded public-key to make sure it's a valid message
          if(res.public_key != "F669F5CDD45CE7C540A5E85CAB04F970A30E20D2C939FD5ACEB18280C9319C1D") {
            LOG_PRINT_L1("Validator list has invalid public_key.");
            return false;
          }

          bool is_signature_valid = crypto::verify_signature(res.blob, unhex(string(res.public_key)), unhex(string(res.signature)));
          if(!is_signature_valid) {
            LOG_PRINT_L1("Validator list has invalid signature and will be ignored.");
            return false;
          }

          json_obj obj;
          load_t_from_json(obj, crypto::base64_decode(res.blob));
          for(const auto &v : obj.validators) {
            this->addOrUpdate(v.validation_public_key, v.start_height, v.end_height);
          }

          LOG_PRINT_L1("Validator list successfully updated!");
          return true;
        }

        struct json_obj {
          struct validator {
            string validation_public_key;
            uint64_t start_height;
            uint64_t end_height;

            BEGIN_KV_SERIALIZE_MAP()
              KV_SERIALIZE(validation_public_key)
              KV_SERIALIZE(start_height)
              KV_SERIALIZE(start_height)
            END_KV_SERIALIZE_MAP()
          };

          vector<validator> validators;

          BEGIN_KV_SERIALIZE_MAP()
            KV_SERIALIZE(validators)
          END_KV_SERIALIZE_MAP()
        };

        struct http {
            struct response {
                string public_key;
                string blob;
                string signature;
                int version = 0;

              BEGIN_KV_SERIALIZE_MAP()
                KV_SERIALIZE(public_key)
                KV_SERIALIZE(blob)
                KV_SERIALIZE(signature)
                KV_SERIALIZE(version)
              END_KV_SERIALIZE_MAP()
            };

            struct request {
              BEGIN_KV_SERIALIZE_MAP()
              END_KV_SERIALIZE_MAP()
            };
        };
    };
}
}




#endif //ELECTRONEUM_VALIDATORS_H
