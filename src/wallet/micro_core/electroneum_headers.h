#ifndef ETN01_ELECTRONEUM_HEADERS_H_H
#define ETN01_ELECTRONEUM_HEADERS_H_H

#define DB_LMDB   2
#define BLOCKCHAIN_DB DB_LMDB


#define UNSIGNED_TX_PREFIX "Electroneum unsigned tx set\003"
#define SIGNED_TX_PREFIX "Electroneum signed tx set\003"
#define KEY_IMAGE_EXPORT_FILE_MAGIC "Electroneum key image export\002"
#define OUTPUT_EXPORT_FILE_MAGIC "Electroneum output export\003"

#define FEE_ESTIMATE_GRACE_BLOCKS 10

#include "net/http_client.h"
#include "storages/http_abstract_invoke.h"

#include "cryptonote_core/tx_pool.h"
#include "cryptonote_core/blockchain.h"
#include "blockchain_db/lmdb/db_lmdb.h"

#include "serialization/binary_utils.h"

#include "ringct/rctTypes.h"
#include "ringct/rctOps.h"
#include "ringct/rctSigs.h"

#include "easylogging++.h"

#include "common/base58.h"

#include "string_coding.h"

#endif
