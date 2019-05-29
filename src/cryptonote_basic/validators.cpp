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

#include "validators.h"

#undef ELECTRONEUM_DEFAULT_LOG_CATEGORY
#define ELECTRONEUM_DEFAULT_LOG_CATEGORY "Validators"

namespace electroneum {
    namespace basic {
        Validator::Validator(const std::string &publicKey, uint64_t startHeight, uint64_t endHeight)
                : publicKey(publicKey), startHeight(startHeight), endHeight(endHeight) {}

        Validator::Validator() = default;

        list_update_outcome Validators::validate_and_update(electroneum::basic::v_list_struct res, bool saveToDB) {

          LOG_PRINT_L2("Validator List Struct received: " << store_t_to_json(res));

          std::vector<std::string> testnet_vl_publicKeys = {"BC41B6767BCCF23AD25A2D9A528FF47C7FABA4790B8FC2E61D11050E95E01878",
                                    "1B74C1751E67E01AF775201AF37554B1B62AF43454CA26E8621BAD81A1CFBC9B",
                                    "953C06A618F276D19B303B04BA9858ECFFD885895D84A72F32C559DC5B82323C"};

          std::vector<std::string> mainnet_vl_publicKeys = {"F669F5CDD45CE7C540A5E85CAB04F970A30E20D2C939FD5ACEB18280C9319C1D",
                                   "0CACB4F4691FC0CE024064BCC16E1288B0FEB5A2424CACEEBFB82C11DE3C070C",
                                   "9C4D0765201F78C46A7FA0EBDDF556AB98F624193FCDB1352194AAAE93F6461B"};

          std::vector<std::string> vl_publicKeys = this->testnet ? testnet_vl_publicKeys : mainnet_vl_publicKeys;

           //Check against our hardcoded public-keys to make sure it's a valid message
          if (res.pubkeys.size() != vl_publicKeys.size()) {
            LOG_PRINT_L1("Validator list has too few public keys.");
            return list_update_outcome::Invalid_Etn_Pubkey;
          }

          if (res.signatures.size() != vl_publicKeys.size()) {
            LOG_PRINT_L1("Validator list has too few signatures.");
            return list_update_outcome::Invalid_Sig;
          }

          //Check against our hardcoded public-keys to make sure it's a valid message
          if (res.pubkeys != vl_publicKeys) {
            LOG_PRINT_L1("Validator list has one or more invalid public keys.");
            return list_update_outcome::Invalid_Etn_Pubkey;
          }

          //We sign our validator lists with multiple keys for security purposes.
          for (unsigned int i = 0; i < vl_publicKeys.size(); ++i){
              if(!crypto::verify_signature(res.blob, unhex(string(vl_publicKeys[i])), unhex(string(res.signatures[i])))){
                  LOG_PRINT_L1("Validator list has an invalid signature and will be ignored.");
                  return list_update_outcome::Invalid_Sig;
              }
          }

          LOG_PRINT_L2("Validator List received: " << crypto::base64_decode(res.blob));

          LOG_PRINT_L2("BEFORE");
          int v_counter = 0;
          all_of(this->list.begin(), this->list.end(), [&v_counter](std::unique_ptr<Validator> &v) {
              LOG_PRINT_L2("Validator " << v_counter << " :: PublicKey=" << v->getPublicKey() << ",\n FromHeight=" << v->getStartHeight() << ", ToHeight=" << v->getEndHeight());
              v_counter++;
              return true;
          });

          json_obj obj;
          load_t_from_json(obj, crypto::base64_decode(res.blob));

          if(obj.list_timestamp < this->current_list_timestamp) {
            LOG_PRINT_L1("Validator list received is older than our local list.");
            return list_update_outcome::Old_List;
          } else if(obj.list_timestamp == this->current_list_timestamp) {

            this->last_updated = time(nullptr);
            this->status = ValidatorsState::Valid;

            LOG_PRINT_L1("Validator list received has the same timestamp than our local list.");

            return list_update_outcome::Same_List;
          }

          for (const auto &v : obj.validators) {
            this->addOrUpdate(v.validation_public_key, v.valid_from_height, v.valid_to_height);
          }

          LOG_PRINT_L2("AFTER");
          v_counter = 0;
          all_of(this->list.begin(), this->list.end(), [&v_counter](std::unique_ptr<Validator> &v) {
            LOG_PRINT_L2("Validator " << v_counter << " :: PublicKey=" << v->getPublicKey() << ",\n FromHeight=" << v->getStartHeight() << ", ToHeight=" << v->getEndHeight());
            v_counter++;
            return true;
          });

          //Serialize & save valid http response to propagate to p2p upon request
          this->serialized_v_list = store_t_to_json(res);
          this->last_updated = time(nullptr);
          this->current_list_timestamp = obj.list_timestamp;
          this->status = ValidatorsState::Valid;

          if(this->isInitial) {
            MGINFO_MAGENTA(ENDL << "**********************************************************************" << ENDL
                                << "List of validator nodes was updated successfully." << ENDL
                                << ENDL
                                << "The daemon will start receiving/sending blocks accordingly." << ENDL
                                << ENDL
                                << "From Electroneum V8, only Validators have the right to mine blocks but every node still" << ENDL
                                << "contributes to the network by verifying mined blocks according to latest consensus rules." << ENDL
                                << ENDL
                                << "This list will be refreshed in " << (this->timeout/(60*60)) << " hours." << ENDL
                                << "**********************************************************************" << ENDL);
            this->isInitial = false;
          }

          if(saveToDB) {
            m_db.set_validator_list(this->serialized_v_list, this->last_updated + this->timeout);
          }

          return list_update_outcome::Success;
        }

        void Validators::add(const string &key, uint64_t startHeight, uint64_t endHeight) {
          if (!this->exists(key)) this->list.emplace_back(std::unique_ptr<Validator>(new Validator(key, startHeight, endHeight)));
        }

        void Validators::addOrUpdate(const string &key, uint64_t startHeight, uint64_t endHeight) {
          this->exists(key) ? this->update(key, endHeight) : this->list.emplace_back(
                  std::unique_ptr<Validator>(new Validator(key, startHeight, endHeight)));
        }

        std::unique_ptr<Validator> Validators::find(const string &key) {
          auto it = find_if(this->list.begin(), this->list.end(), [&key](std::unique_ptr<Validator> &v) {
              return v->getPublicKey() == key;
          });
          return std::move(*it);
        }

        bool Validators::exists(const string &key) {
          bool found = false;
          all_of(this->list.begin(), this->list.end(), [&key, &found](std::unique_ptr<Validator> &v) {
              if (v->getPublicKey() == key) {
                found = true;
                return false;
              }
              return true;
          });
          return found;
        }

        void Validators::update(const string &key, uint64_t endHeight) {
          find_if(this->list.begin(), this->list.end(), [&key, &endHeight](std::unique_ptr<Validator> &v) {
              if (v->getPublicKey() == key) {
                v->setEndHeight(endHeight);
                return true;
              }
              return false;
          });
        }

        void Validators::invalidate() {
          this->serialized_v_list = string("");
          this->status = ValidatorsState::Invalid;
        }

        ValidatorsState Validators::validate_expiration() {
          if((time(nullptr) - this->last_updated) >= this->timeout && this->status == ValidatorsState::Valid) {
            this->status = ValidatorsState::NeedsUpdate;
          }

          if((time(nullptr) - this->last_updated) >= this->timeout + this->timeout_grace_period && this->status == ValidatorsState::Valid) {
            this->status = ValidatorsState::Expired;
          }

          return this->status;
        }
    }
}