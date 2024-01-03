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

#include "checkpoints.h"

#include "common/dns_utils.h"
#include "string_tools.h"
#include "storages/portable_storage_template_helper.h" // epee json include
#include "serialization/keyvalue_serialization.h"
#include <functional>
#include <vector>

using namespace epee;

#undef ELECTRONEUM_DEFAULT_LOG_CATEGORY
#define ELECTRONEUM_DEFAULT_LOG_CATEGORY "checkpoints"

namespace cryptonote
{
  /**
   * @brief struct for loading a checkpoint from json
   */
  struct t_hashline
  {
    uint64_t height; //!< the height of the checkpoint
    std::string hash; //!< the hash for the checkpoint
        BEGIN_KV_SERIALIZE_MAP()
          KV_SERIALIZE(height)
          KV_SERIALIZE(hash)
        END_KV_SERIALIZE_MAP()
  };

  /**
   * @brief struct for loading many checkpoints from json
   */
  struct t_hash_json {
    std::vector<t_hashline> hashlines; //!< the checkpoint lines from the file
        BEGIN_KV_SERIALIZE_MAP()
          KV_SERIALIZE(hashlines)
        END_KV_SERIALIZE_MAP()
  };

  //---------------------------------------------------------------------------
  checkpoints::checkpoints()
  {
  }
  //---------------------------------------------------------------------------
  bool checkpoints::add_checkpoint(uint64_t height, const std::string& hash_str)
  {
    crypto::hash h = crypto::null_hash;
    bool r = epee::string_tools::hex_to_pod(hash_str, h);
    CHECK_AND_ASSERT_MES(r, false, "Failed to parse checkpoint hash string into binary representation!");

    // return false if adding at a height we already have AND the hash is different
    if (m_points.count(height))
    {
      CHECK_AND_ASSERT_MES(h == m_points[height], false, "Checkpoint at given height already exists, and hash for new checkpoint was different!");
    }
    m_points[height] = h;
    return true;
  }
  //---------------------------------------------------------------------------
  bool checkpoints::is_in_checkpoint_zone(uint64_t height) const
  {
    return !m_points.empty() && (height <= (--m_points.end())->first);
  }
  //---------------------------------------------------------------------------
  bool checkpoints::check_block(uint64_t height, const crypto::hash& h, bool& is_a_checkpoint) const
  {
    auto it = m_points.find(height);
    is_a_checkpoint = it != m_points.end();
    if(!is_a_checkpoint)
      return true;

    if(it->second == h)
    {
      MINFO("CHECKPOINT PASSED FOR HEIGHT " << height << " " << h);
      return true;
    }else
    {
      MWARNING("CHECKPOINT FAILED FOR HEIGHT " << height << ". EXPECTED HASH: " << it->second << ", FETCHED HASH: " << h);
      return false;
    }
  }
  //---------------------------------------------------------------------------
  bool checkpoints::check_block(uint64_t height, const crypto::hash& h) const
  {
    bool ignored;
    return check_block(height, h, ignored);
  }
  //---------------------------------------------------------------------------
  //FIXME: is this the desired behavior?
  bool checkpoints::is_alternative_block_allowed(uint64_t blockchain_height, uint64_t block_height) const
  {
    if (0 == block_height)
      return false;

    auto it = m_points.upper_bound(blockchain_height);
    // Is blockchain_height before the first checkpoint?
    if (it == m_points.begin())
      return true;

    --it;
    uint64_t checkpoint_height = it->first;
    return checkpoint_height < block_height;
  }
  //---------------------------------------------------------------------------
  uint64_t checkpoints::get_max_height() const
  {
    if (m_points.empty())
      return 0;
    return m_points.rbegin()->first;
  }
  //---------------------------------------------------------------------------
  const std::map<uint64_t, crypto::hash>& checkpoints::get_points() const
  {
    return m_points;
  }

  bool checkpoints::check_for_conflicts(const checkpoints& other) const
  {
    for (auto& pt : other.get_points())
    {
      if (m_points.count(pt.first))
      {
        CHECK_AND_ASSERT_MES(pt.second == m_points.at(pt.first), false, "Checkpoint at given height already exists, and hash for new checkpoint was different!");
      }
    }
    return true;
  }

  bool checkpoints::init_default_checkpoints(network_type nettype)
  {
    if (nettype == MAINNET)
    {
      ADD_CHECKPOINT(1, "4536e1e23ff7179a126a7e61cd9e89ded0e258176f2bc879c999caa155f68cc3");
      ADD_CHECKPOINT(10, "e5aefcb1d575a788ecfb65bb7be3bdd135eb76ccefb38a60d7800e86d25d408e");
      ADD_CHECKPOINT(100, "e3548600cc0e2991af4f36bbf44addd95051748fc09e8cac5f8237fd841132c0");
      ADD_CHECKPOINT(1000, "d7ec8a6329948fee02cdc95b13f286bd69fe9540863a80dfff7fe14940756293");
      ADD_CHECKPOINT(10000, "95dad4575ba43eb0d4ba9b6081d5d52e6a74fc8fe3391d9628f78ddd3b71c965");
      ADD_CHECKPOINT(100000, "a7b51ca66b2525903efbd4a32604a7ad5000df4b9da8bdd9cb3062cb014b0cad");
      ADD_CHECKPOINT(150000, "e9b66d3f12f9cedece7d9925721b15f1ec6cb2f6b438b3ddd288237c27ffe20e");
      ADD_CHECKPOINT(179839, "f8631f50ef79b840cba9fe3484764d0c7515ff2884e1f5be2f7298a4d08e88ee");
      ADD_CHECKPOINT(179840, "74958c1b19505ab49babc91dfd14251146256873ae875ac97c26fb2000490e70");
      ADD_CHECKPOINT(179841, "8a793f1aef368e83fa72ac3a236309c06ae7726958120514e0f6d33ff3b24548");
      ADD_CHECKPOINT(200000, "9a7853584fbe0d88746d3d7bb6a3efd02ecaa3f0158808fde9f3c8339b3d5d8f");
      ADD_CHECKPOINT(250000, "0b4d542eeaf6fbd5ceb196ca5ed44edb6ee0c0f1cdf391ba62710b04e1da9f29");
      ADD_CHECKPOINT(300000, "e3d192196c70c259164fec04868d5cf21a57242cccf54ca0ba9fd8eaeddb8d72");
      ADD_CHECKPOINT(307499, "c08f9eecdebf4f1f99c4c273368df54422e6861829213ad5d0dbde1cf6f4f08c");
      ADD_CHECKPOINT(307500, "a65844a64acf5d0aa421892dee64e3552ef390fd29894b7d6c77d08d6609952e");
      ADD_CHECKPOINT(307501, "4a5a86a4c6ecee29c2d41d8f633e82a5b3340e354c55f2767f722fc03b950fae");
      ADD_CHECKPOINT(310000, "82dfd6ee74a5ae7526af923f1637b59082ef917d84ad80944edb1debd557bb96");
      ADD_CHECKPOINT(400000, "251e07872ce8b05dedafbce2dd27f978e71a46aca7fa3b6cdbd94d6b357b5078");
      ADD_CHECKPOINT(500000, "c2f37b178f4e1cf3460bcefd6eaf14eeaf4258ee67b97e6146b171766bd8101d");
      ADD_CHECKPOINT(600000, "e8eeceec76ea9718ebf7fac34250403f586d01224b343c6e122baa0799023a4f");
      ADD_CHECKPOINT(700000, "09af67f7f5b01219dd18c59a424a7570cadfe3c50408180fcaf12cec205155f7");
      ADD_CHECKPOINT(800000, "748bfc3c736ac106a8c731114648b8c11c0c2bc3d6b9ab074bbbfc8cc1914a3f");
      ADD_CHECKPOINT(900000, "e0fce3c156932d38d8bde4e21fabbf2a8208e96fb1b0848cc796d7338a70f7de");
      ADD_CHECKPOINT(1000000, "26da34bde63cfb6447e0dcc190b00e4db04127e2673d48316e63e64c661ab19c");
      ADD_CHECKPOINT(1100000, "d9d9637c9468c3d8a3a552cef38e52effbe79d0f140b0a0551d5c15f61e07a08");
    }
    if (nettype == TESTNET){
      ADD_CHECKPOINT(1455170, "543ab0413ecfeec4f66472f36dbc8fe7ce707dc855fc82aec6d83f918ed482fc");
    }
    return true;
  }

  bool checkpoints::load_checkpoints_from_json(const std::string &json_hashfile_fullpath)
  {
    boost::system::error_code errcode;
    if (! (boost::filesystem::exists(json_hashfile_fullpath, errcode)))
    {
      LOG_PRINT_L1("Blockchain checkpoints file not found");
      return true;
    }

    LOG_PRINT_L1("Adding checkpoints from blockchain hashfile");

    uint64_t prev_max_height = get_max_height();
    LOG_PRINT_L1("Hard-coded max checkpoint height is " << prev_max_height);
    t_hash_json hashes;
    if (!epee::serialization::load_t_from_json_file(hashes, json_hashfile_fullpath))
    {
      MERROR("Error loading checkpoints from " << json_hashfile_fullpath);
      return false;
    }
    for (std::vector<t_hashline>::const_iterator it = hashes.hashlines.begin(); it != hashes.hashlines.end(); )
    {
      uint64_t height;
      height = it->height;
      if (height <= prev_max_height) {
	LOG_PRINT_L1("ignoring checkpoint height " << height);
      } else {
	std::string blockhash = it->hash;
	LOG_PRINT_L1("Adding checkpoint height " << height << ", hash=" << blockhash);
	ADD_CHECKPOINT(height, blockhash);
      }
      ++it;
    }

    return true;
  }

  bool checkpoints::load_checkpoints_from_dns(network_type nettype)
  {
    std::vector<std::string> records;

    // All four ElectroneumPulse domains have DNSSEC on and valid
    static const std::vector<std::string> dns_urls = {
      "checkpoints.electroneumpulse.com",
      "checkpoints.electroneumpulse.info",
      "checkpoints.electroneumpulse.net",
      "checkpoints.electroneumpulse.org"
    };

    static const std::vector<std::string> testnet_dns_urls = {
      "testpoints.electroneumpulse.com",
      "testpoints.electroneumpulse.info",
      "testpoints.electroneumpulse.net",
      "testpoints.electroneumpulse.org"
    };

    static const std::vector<std::string> stagenet_dns_urls = { 
      "stagenetpoints.electroneumpulse.com",
      "stagenetpoints.electroneumpulse.info",
      "stagenetpoints.electroneumpulse.net",
      "stagenetpoints.electroneumpulse.org"
    };

    if (!tools::dns_utils::load_txt_records_from_dns(records, nettype == TESTNET ? testnet_dns_urls : nettype == STAGENET ? stagenet_dns_urls : dns_urls, "checkpoints"))
      return true; // why true ?

    for (const auto& record : records)
    {
      auto pos = record.find(":");
      if (pos != std::string::npos)
      {
        uint64_t height;
        crypto::hash hash;

        // parse the first part as uint64_t,
        // if this fails move on to the next record
        std::stringstream ss(record.substr(0, pos));
        if (!(ss >> height))
        {
    continue;
        }

        // parse the second part as crypto::hash,
        // if this fails move on to the next record
        std::string hashStr = record.substr(pos + 1);
        if (!epee::string_tools::hex_to_pod(hashStr, hash))
        {
    continue;
        }

        ADD_CHECKPOINT(height, hashStr);
      }
    }
    return true;
  }

  bool checkpoints::load_new_checkpoints(const std::string &json_hashfile_fullpath, network_type nettype, bool dns)
  {
    bool result;

    result = load_checkpoints_from_json(json_hashfile_fullpath);
    if (dns)
    {
      result &= load_checkpoints_from_dns(nettype);
    }

    return result;
  }
}
