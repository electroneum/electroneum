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

    using namespace std;
    using namespace epee::serialization;

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
        vector<Validator*> _list;

    public:
        Validators();

        inline void add(const string &key, uint64_t startHeight, uint64_t endHeight) {
          if(!this->exists(key)) this->_list.emplace_back(new Validator(key, startHeight, endHeight));
        }

        inline void addOrUpdate(const string &key, uint64_t startHeight, uint64_t endHeight) {
          this->exists(key) ? this->update(key, endHeight) : this->_list.emplace_back(new Validator(key, startHeight, endHeight));
        }

        inline Validator* find(const string &key) {
          auto it = find_if(this->_list.begin(), this->_list.end(), [&key](Validator* &v) {
              return v->getPublicKey() == key ? true : false;
          });
          return *it;
        }

        inline bool exists(const string &key) {
          bool found = false;
          all_of(this->_list.begin(), this->_list.end(), [&key, &found](Validator* &v) {
            if(v->getPublicKey() == key) {
              found = true;
              return false;
            }
            return true;
          });
          return found;
        }

        inline void update(const string &key, uint64_t endHeight) {
          find_if(this->_list.begin(), this->_list.end(), [&key, &endHeight](Validator* &v) {
              if(v->getPublicKey() == key) {
                v->setEndHeight(endHeight);
                return true;
              }
              return false;
          });
        }

        inline vector<string> getApplicablePublickKeys(uint64_t height, bool convert_to_byte = false) {
          vector<string> keys;
          all_of(this->_list.begin(), this->_list.end(), [&height, &keys, &convert_to_byte](Validator* &v) {
              if(v->isWithinRange(height)) {
                const string k = convert_to_byte ? boost::algorithm::unhex(v->getPublicKey()) : v->getPublicKey();
                keys.push_back(k);
              }
              return true;
          });
          return keys;
        }

        inline bool isEmpty() {
          return this->_list.empty();
        }

        inline bool fetchFromURI() {
          struct http_request {} req;
          struct http_response {
              std::string public_key;
              std::string blob;
              std::string signature;
              int version = 0;
          } res;


          //TODO: HTTP Request
          //Mocking the HTTP response for now
          res.public_key = "684F8D23D23E634B3A5F3FBBA80B686B4640811F2CDAB15A5CD45FB60C44718C";
          res.blob = "eyJ2YWxpZGF0b3JzIjpbeyJ2YWxpZGF0aW9uX3B1YmxpY19rZXkiOiI2ODRGOEQyM0QyM0U2MzRCM0E1RjNGQkJBODBCNjg2QjQ2NDA4MTFGMkNEQUIxNUE1Q0Q0NUZCNjBDNDQ3MThDIiwidmFsaWRfZnJvbV9oZWlnaHQiOiAwLCJ2YWxpZF90b19oZWlnaHQiOiAtMX0seyJ2YWxpZGF0aW9uX3B1YmxpY19rZXkiOiI4NzUzNUYwNUU5RDVFNDc0NTVCODcyNzEyNzg3NzY1RDZFRjgzNjJBRTMyNUE3REJBMjI4Nzg4NTI0OTI2MDNCIiwidmFsaWRfZnJvbV9oZWlnaHQiOiAwLCJ2YWxpZF90b19oZWlnaHQiOiAtMX0seyJ2YWxpZGF0aW9uX3B1YmxpY19rZXkiOiJEQ0JBRUYwM0IyN0EwNzg2REU4ODA3NEIzNjZDMjlDQzU0RTAyMzg4M0Q3RkQ5NkMxN0M1RjQ5NDBCNDk5RTdEIiwidmFsaWRfZnJvbV9oZWlnaHQiOiAwLCJ2YWxpZF90b19oZWlnaHQiOiAtMX0seyJ2YWxpZGF0aW9uX3B1YmxpY19rZXkiOiJBREVDQjUwOEVCNDIyRTY5NjJBRkM3OUFFQjA5QjI0NDlDM0U0OTRENURERDlENEUyNTU5NTg4RjEwRjA3QkY1IiwidmFsaWRfZnJvbV9oZWlnaHQiOiAwLCJ2YWxpZF90b19oZWlnaHQiOiAtMX0seyJ2YWxpZGF0aW9uX3B1YmxpY19rZXkiOiIwRjQ1QTVBOTJERTg5NUJEMUZDQ0ZCMkU5ODlFODA3QjRCNzQ2OEI3QjAzQkZEMzEzRjc0N0IyNDE1OEMyNTQwIiwidmFsaWRfZnJvbV9oZWlnaHQiOiAwLCJ2YWxpZF90b19oZWlnaHQiOiAtMX1dfQ==";
          res.signature = "A4B8083257D343309986DD1258FEE97A9CAB8D993D6D03504EBB120E04C21C22F8FCA90C84DAC8971DEBD6B2CEFE0A9E7BFFE3166704B8234445DCC0D241240E";
          res.version = 1;

          bool is_signature_valid = crypto::verify_signature(res.blob,
                                                             boost::algorithm::unhex(string(res.public_key)),
                                                             boost::algorithm::unhex(string(res.signature)));

          if(!is_signature_valid) {
            LOG_PRINT_L1("Validator list has invalid signature and message will be ignored.");
            return false;
          }

          json_obj obj;
          load_t_from_json(obj, crypto::base64_decode(res.blob));
          for(const auto &v : obj.validators) {
            this->addOrUpdate(v.validation_public_key, v.start_height, v.end_height);
          }

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
    };
}




#endif //ELECTRONEUM_VALIDATORS_H
