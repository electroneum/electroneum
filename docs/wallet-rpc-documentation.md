# Wallet RPC Documentation

## Introduction

This is a list of the electroneum-wallet-rpc calls, their Inputs and Outputs, and examples of each.
```
Please note that you must specify your own RPC bind port when starting the program with the flag --rpc-bind-port. 
In this guide we have chosen 40000. You must choose a port that isn't already in use.
```
```
IP=127.0.0.1
PORT=40000
METHOD="make_integrated_address"
PARAMS="{\"payment_id\":\"1234567890123456789012345678900012345678901234567890123456789000\"}"
curl \
    -X POST http://$IP:$PORT/json_rpc \
    -d '{"jsonrpc":"2.0","id":"0","method":"'$METHOD'","params":'"$PARAMS"'}' \
    -H 'Content-Type: application/json'
```

If the electroneum-wallet-rpc was executed with the  `--rpc-login`  argument as  `username:password` , then follow this example:

```
IP=127.0.0.1
PORT=40000
METHOD="make_integrated_address"
PARAMS="{\"payment_id\":\"1234567890123456789012345678900012345678901234567890123456789000\"}"
curl \
    -u username:password --digest \
    -X POST http://$IP:$PORT/json_rpc \
    -d '{"jsonrpc":"2.0","id":"0","method":"'$METHOD'","params":'"$PARAMS"'}' \
    -H 'Content-Type: application/json'
```

Note: "Atomic Units" refer to the smallest fraction of 1 ETN according to the electroneumd implementation.  **1 ETN = 100 Atomic Units.**


### Index of JSON RPC Methods:

* [get_balance](#get_balance)
* [get_address](#get_address)
* [get_address_index](#get_address_index)
* [create_address](#create_address)
* [label_address](#label_address)
* [get_accounts](#get_accounts)
* [create_account](#create_account)
* [label_account](#label_account)
* [get_account_tags](#get_account_tags)
* [tag_accounts](#tag_accounts)
* [untag_accounts](#untag_accounts)
* [set_account_tag_description](#set_account_tag_description)
* [get_height](#get_height)
* [transfer](#transfer)
* [transfer_split](#transfer_split)
* [sign_transfer](#sign_transfer)
* [submit_transfer](#submit_transfer)
* [sweep_dust](#sweep_dust)
* [sweep_all](#sweep_all)
* [sweep_single](#sweep_single)
* [relay_tx](#relay_tx)
* [store](#store)
* [get_payments](#get_payments)
* [get_bulk_payments](#get_bulk_payments)
* [incoming_transfers](#incoming_transfers)
* [query_key](#query_key)
* [make_integrated_address](#make_integrated_address)
* [split_integrated_address](#split_integrated_address)
* [stop_wallet](#stop_wallet)
* [rescan_blockchain](#rescan_blockchain)
* [set_tx_notes](#set_tx_notes)
* [get_tx_notes](#get_tx_notes)
* [set_attribute](#set_attribute)
* [get_attribute](#get_attribute)
* [get_tx_key](#get_tx_key)
* [check_tx_key](#check_tx_key)
* [get_tx_proof](#get_tx_proof)
* [check_tx_proof](#check_tx_proof)
* [get_spend_proof](#get_spend_proof)
* [check_spend_proof](#check_spend_proof)
* [get_reserve_proof](#get_reserve_proof)
* [check_reserve_proof](#check_reserve_proof)
* [get_transfers](#get_transfers)
* [get_transfer_by_txid](#get_transfer_by_txid)
* [sign](#sign)
* [verify](#verify)
* [export_outputs](#export_outputs)
* [import_outputs](#import_outputs)
* [export_key_images](#export_key_images)
* [import_key_images](#import_key_images)
* [make_uri](#make_uri)
* [parse_uri](#parse_uri)
* [get_address_book](#get_address_book)
* [add_address_book](#add_address_book)
* [delete_address_book](#delete_address_book)
* [refresh](#refresh)
* [rescan_spent](#rescan_spent)
* [start_mining](#start_mining)
* [stop_mining](#stop_mining)
* [get_languages](#get_languages)
* [create_wallet](#create_wallet)
* [open_wallet](#open_wallet)
* [close_wallet](#close_wallet)
* [change_wallet_password](#change_wallet_password)
* [is_multisig](#is_multisig)
* [prepare_multisig](#prepare_multisig)
* [make_multisig](#make_multisig)
* [export_multisig_info](#export_multisig_info)
* [import_multisig_info](#import_multisig_info)
* [finalize_multisig](#finalize_multisig)
* [sign_multisig](#sign_multisig)
* [submit_multisig](#submit_multisig)
* [get_version](#get_version)

## JSON RPC Methods:

### **get_balance**

Return the wallet's balance.

Alias:  *getbalance* .

Inputs:

* *account_index*  - unsigned int; (Optional) Return balance for this account.
* *address_indices*  - array of unsigned int; (Optional) Return balance detail for those subaddresses.

Outputs:

* *balance*  - unsigned int; The total balance of the current electroneum-wallet-rpc in session.
* *unlocked_balance*  - unsigned int; Unlocked funds are those funds that are sufficiently deep enough in the Electroneum blockchain to be considered safe to spend.
* *multisig_import_needed*  - boolean; True if importing multisig data is needed for returning a correct balance.
* *per_subaddress*  - array of subaddress information; Balance information for each subaddress in an account.
  * *address_index*  - unsigned int; Index of the subaddress in the account.
  * *address*  - string; Address at this index. Base58 representation of the public keys.
  * *balance*  - unsigned int; Balance for the subaddress (locked or unlocked).
  * *blocks_to_unlock* - unsigned int; Blocks remaining to unlock pending funds.
  * *unlocked_balance*  - unsigned int; Unlocked balance for the subaddress.
  * *label*  - string; Label for the subaddress.
  * *num_unspent_outputs*  - unsigned int; Number of unspent outputs available for the subaddress.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_balance","params":{"account_index":0,"address_indices":[0,1]}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "balance": 300000,
    "blocks_to_unlock": 0,
    "multisig_import_needed": false,
    "per_subaddress": [{
      "address": "etnkKzqmNPQMk7DehZMif4FLJa1hpUQXjH6ULFEmyVUkRfJfrbphCZsCQt8G2dq1hbC2h4QsDMeohMrUeHcTmtSF4uG2NL756Z",
      "address_index": 0,
      "balance": 100000,
      "blocks_to_unlock": 4,
      "label": "Primary account",
      "num_unspent_outputs": 5281,
      "unlocked_balance": 50000
    },{
      "address": "etnkufaYsjMEraaDRD37pSiNYsCzodhabEAmMjDGhcNTD41wZHZnMx8KK4hbTQ1Gv6N558r7DFTJShYQUMPmRkab1ZraHdTczk",
      "address_index": 1,
      "balance": 200000,
      "blocks_to_unlock": 4,
      "label": "",
      "num_unspent_outputs": 1,
      "unlocked_balance": 100000
    }],
    "unlocked_balance": 150000
  }
}
```

### **get_address**

Return the wallet's addresses for an account. Optionally filter for specific set of subaddresses.

Alias:  *getaddress* .

Inputs:

* *account_index*  - unsigned int; (Optional) Return subaddresses for this account.
* *address_index*  - array of unsigned int; (Optional) List of subaddresses to return from an account.

Outputs:

* *address*  - string; The 95-character hex address string of the electroneum-wallet-rpc in session.
* *addresses*  array of addresses informations
  * *address*  string; The 95-character hex (sub)address string.
  * *label*  string; Label of the (sub)address
  * *address_index*  unsigned int; index of the subaddress
  * *used*  boolean; states if the (sub)address has already received funds

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_address","params":{"account_index":0,"address_index":[0,1,4]}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "address": "etnk4YGjvutPsJvihejok8h63PadX47sgYmVttR9aCdbNeDgKWemFkzcaVHJYyZTnh8Po4UMwQmYp4HVGss7zyiQ3PEC7VVkc7",
    "addresses": [{
      "address": "etnk4YGjvutPsJvihejok8h63PadX47sgYmVttR9aCdbNeDgKWemFkzcaVHJYyZTnh8Po4UMwQmYp4HVGss7zyiQ3PEC7VVkc7",
      "address_index": 0,
      "label": "Primary account",
      "used": true
    },{
      "address": "etnkjHMY5Cmbjmjuw8eEeXK41sJn7JS8Wfr4Px1SwVEXLMU8Zzafmm7QmFcCbsWgcaLqT7jQ7vF2zFnBgXQGpU1V4qGEsTE9T4",
      "address_index": 1,
      "label": "",
      "used": true
    },{
      "address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY",
      "address_index": 4,
      "label": "test2",
      "used": true
    }]
  }
}
```

### **get_address_index**

Get account and address indexes from a specific (sub)address

Alias:  *None* .

Inputs:

* *address*  - String; (sub)address to look for.

Outputs:

* *index*  - subaddress informations
  * *major*  unsigned int; Account index.
  * *minor*  unsigned int; Address index.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_address_index","params":{"address":"etnkufaYsjMEraaDRD37pSiNYsCzodhabEAmMjDGhcNTD41wZHZnMx8KK4hbTQ1Gv6N558r7DFTJShYQUMPmRkab1ZraHdTczk"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "index": {
      "major": 0,
      "minor": 1
    }
  }
}
```

### **create_address**

Create a new address for an account. Optionally, label the new address.

Alias:  *None* .

Inputs:

* *account_index*  - unsigned int; Create a new address for this account.
* *label*  - string; (Optional) Label for the new address.

Outputs:

* *address*  - string; Newly created address. Base58 representation of the public keys.
* *address_index*  - unsigned int; Index of the new address under the input account.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"create_address","params":{"account_index":0,"label":"new-sub"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY",
    "address_index": 5
  }
}
```

### **label_address**

Label an address.

Alias:  *None* .

Inputs:

* *index*  - subaddress index; JSON Object containing the major & minor address index:
  * *major*  - unsigned int; Account index for the subaddress.
  * *minor*  - unsigned int; Index of the subaddress in the account.
* *label*  - string; Label for the address.

Outputs:  *None* .

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"label_address","params":{"index":{"major":0,"minor":5},"label":"myLabel"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **get_accounts**

Get all accounts for a wallet. Optionally filter accounts by tag.

Alias:  *None* .

Inputs:

* *tag*  - string; (Optional) Tag for filtering accounts.

Outputs:

* *subaddress_accounts*  - array of subaddress account information:
  * *account_index*  - unsigned int; Index of the account.
  * *balance*  - unsigned int; Balance of the account (locked or unlocked).
  * *base_address*  - string; Base64 representation of the first subaddress in the account.
  * *label*  - string; (Optional) Label of the account.
  * *tag*  - string; (Optional) Tag for filtering accounts.
  * *unlocked_balance*  - unsigned int; Unlocked balance for the account.
* *total_balance*  - unsigned int; Total balance of the selected accounts (locked or unlocked).
* *total_unlocked_balance*  - unsigned int; Total unlocked balance of the selected accounts.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_accounts","params":{"tag":"myTag"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "subaddress_accounts": [{
      "account_index": 0,
      "balance": 100000,
      "base_address": "etnkjHMY5Cmbjmjuw8eEeXK41sJn7JS8Wfr4Px1SwVEXLMU8Zzafmm7QmFcCbsWgcaLqT7jQ7vF2zFnBgXQGpU1V4qGEsTE9T4",
      "label": "Primary account",
      "tag": "myTag",
      "unlocked_balance": 100000
    },{
      "account_index": 1,
      "balance": 0,
      "base_address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY",
      "label": "Secondary account",
      "tag": "myTag",
      "unlocked_balance": 0
    }],
    "total_balance": 100000,
    "total_unlocked_balance": 100000
  }
}
```

### **create_account**

Create a new account with an optional label.

Alias:  *None* .

Inputs:

* *label*  - string; (Optional) Label for the account.

Outputs:

* *account_index*  - unsigned int; Index of the new account.
* *address*  - string; Address for this account. Base58 representation of the public keys.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"create_account","params":{"label":"Secondary account"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "account_index": 1,
    "address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY"
  }
}
```

### **label_account**

Label an account.

Alias:  *None* .

Inputs:

* *account_index*  - unsigned int; Apply label to account at this index.
* *label*  - string; Label for the account.

Outputs:  *None* .

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"label_account","params":{"account_index":0,"label":"Primary account"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "account_tags": [{
      "accounts": [0,1],
      "label": "",
      "tag": "myTag"
    }]
  }
}
```

### **get_account_tags**

Get a list of user-defined account tags.

Alias:  *None* .

Inputs:  *None* .

Outputs:

* *account_tags*  - array of account tag information:
  * *tag*  - string; Filter tag.
  * *label*  - string; Label for the tag.
  * *accounts*  - array of int; List of tagged account indices.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_account_tags","params":""}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "account_tags": [{
      "accounts": [0],
      "label": "Test tag",
      "tag": "myTag"
    }]
  }
}
```

### **tag_accounts**

Apply a filtering tag to a list of accounts.

Alias:  *None* .

Inputs:

* *tag*  - string; Tag for the accounts.
* *accounts*  - array of unsigned int; Tag this list of accounts.

Outputs:  *None* .

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"tag_accounts","params":{"tag":"myTag","accounts":[0,1]}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **untag_accounts**

Remove filtering tag from a list of accounts.

Alias:  *None* .

Inputs:

* *accounts*  - array of unsigned int; Remove tag from this list of accounts.

Outputs:  *None* .

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"untag_accounts","params":{"accounts":[1]}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **set_account_tag_description**

Set description for an account tag.

Alias:  *None* .

Inputs:

* *tag*  - string; Set a description for this tag.
* *description*  - string; Description for the tag.

Outputs:  *None* .

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"set_account_tag_description","params":{"tag":"myTag","description":"Test tag"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **get_height**

Returns the wallet's current block height.

Alias:  *getheight* .

Inputs:  *None* .

Outputs:

* *height*  - unsigned int; The current electroneum-wallet-rpc's blockchain height. If the wallet has been offline for a long time, it may need to catch up with the daemon.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_height"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "height": 145545
  }
}
```

### **transfer**

Send ETN to a number of recipients.

Alias:  *None* .

Inputs:

* *destinations*  - array of destinations to receive ETN:
  * *amount*  - unsigned int; Amount to send to each destination, in Atomic Units.
  * *address*  - string; Destination public address.
* *account_index*  - unsigned int; (Optional) Transfer from this account index. (Defaults to 0)
* *subaddr_indices*  - array of unsigned int; (Optional) Transfer from this set of subaddresses. (Defaults to empty - all indices)
* *priority*  - unsigned int; Set a priority for the transaction. Accepted Values are: 0-3 for: default, unimportant, normal, elevated, priority.
* *mixin*  - unsigned int; Number of outputs from the blockchain to mix with (0 means no mixing).
* *ring_size*  - unsigned int; Number of outputs to mix in the transaction (this output + N decoys from the blockchain).
* *unlock_time*  - unsigned int; Number of blocks before the ETN can be spent (0 to not add a lock).
* *payment_id*  - string; (Optional) Random 32-byte/64-character hex string to identify a transaction.
* *get_tx_key*  - boolean; (Optional) Return the transaction key after sending.
* *do_not_relay*  - boolean; (Optional) If true, the newly created transaction will not be relayed to the electroneum network. (Defaults to false)
* *get_tx_hex*  - boolean; Return the transaction as hex string after sending (Defaults to false)
* *get_tx_metadata*  - boolean; Return the metadata needed to relay the transaction. (Defaults to false)

Outputs:

* *amount*  - Amount transferred for the transaction.
* *fee*  - Integer value of the fee charged for the txn.
* *multisig_txset*  - Set of multisig transactions in the process of being signed (empty for non-multisig).
* *tx_blob*  - Raw transaction represented as hex string, if get_tx_hex is true.
* *tx_hash*  - String for the publically searchable transaction hash.
* *tx_key*  - String for the transaction key if get_tx_key is true, otherwise, blank string.
* *tx_metadata*  - Set of transaction metadata needed to relay this transfer later, if get_tx_metadata is true.
* *unsigned_txset*  - String. Set of unsigned tx for cold-signing purposes.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"transfer","params":{"destinations":[{"amount":10000,"address":"etnkjHMY5Cmbjmjuw8eEeXK41sJn7JS8Wfr4Px1SwVEXLMU8Zzafmm7QmFcCbsWgcaLqT7jQ7vF2zFnBgXQGpU1V4qGEsTE9T4"},{"amount":20000,"address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY"}],"account_index":0,"subaddr_indices":[0],"priority":0,"get_tx_key": true}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "amount": 30000,
    "fee": 10,
    "multisig_txset": "",
    "tx_blob": "",
    "tx_hash": "7663438de4f72b25a0e395b770ea9ecf7108cd2f0c4b75be0b14a103d3362be9",
    "tx_key": "25c9d8ec20045c80c93d665c9d3684aab7335f8b2cd02e1ba2638485afd1c70e236c4bdd7a2f1cb511dbf466f13421bdf8df988b7b969c448ca6239d7251490e4bf1bbf9f6ffacffdcdc93b9d1648ec499eada4d6b4e02ce92d4a1c0452e5d009fbbbf15b549df8856205a4c7bda6338d82c823f911acd00cb75850b198c5803",
    "tx_metadata": "",
    "unsigned_txset": ""
  }
}
```

### **transfer_split**

Same as transfer, but can split into more than one tx if necessary.

Alias:  *None* .

Inputs:

* *destinations*  - array of destinations to receive ETN:
  * *amount*  - unsigned int; Amount to send to each destination, in Atomic Units.
  * *address*  - string; Destination public address.
* *account_index*  - unsigned int; (Optional) Transfer from this account index. (Defaults to 0)
* *subaddr_indices*  - array of unsigned int; (Optional) Transfer from this set of subaddresses. (Defaults to empty - all indices)
* *mixin*  - unsigned int; Number of outputs from the blockchain to mix with (0 means no mixing).
* *ring_size*  - unsigned int; Sets ringsize to n (mixin + 1).
* *unlock_time*  - unsigned int; Number of blocks before the ETN can be spent (0 to not add a lock).
* *payment_id*  - string; (Optional) Random 32-byte/64-character hex string to identify a transaction.
* *get_tx_keys*  - boolean; (Optional) Return the transaction keys after sending.
* *priority*  - unsigned int; Set a priority for the transactions. Accepted Values are: 0-3 for: default, unimportant, normal, elevated, priority.
* *do_not_relay*  - boolean; (Optional) If true, the newly created transaction will not be relayed to the electroneum network. (Defaults to false)
* *get_tx_hex*  - boolean; Return the transactions as hex string after sending
* *new_algorithm*  - boolean; True to use the new transaction construction algorithm, defaults to false.
* *get_tx_metadata*  - boolean; Return list of transaction metadata needed to relay the transfer later.

Outputs:

* *tx_hash_list*  - array of: string. The tx hashes of every transaction.
* *tx_key_list*  - array of: string. The transaction keys for every transaction.
* *amount_list*  - array of: integer. The amount transferred for every transaction.
* *fee_list*  - array of: integer. The amount of fees paid for every transaction.
* *tx_blob_list*  - array of: string. The tx as hex string for every transaction.
* *tx_metadata_list*  - array of: string. List of transaction metadata needed to relay the transactions later.
* *multisig_txset*  - string. The set of signing keys used in a multisig transaction (empty for non-multisig).
* *unsigned_txset*  - string. Set of unsigned tx for cold-signing purposes.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"transfer_split","params":{"destinations":[{"amount":10000,"address":"etnkjHMY5Cmbjmjuw8eEeXK41sJn7JS8Wfr4Px1SwVEXLMU8Zzafmm7QmFcCbsWgcaLqT7jQ7vF2zFnBgXQGpU1V4qGEsTE9T4"},{"amount":20000,"address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY"}],"account_index":0,"subaddr_indices":[0],"priority":0,"get_tx_keys": true}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "amount_list": [30000],
    "fee_list": [10],
    "multisig_txset": "",
    "tx_hash_list": ["4adcdc1af3f665770cdf8fb7a380887cd07ac53c2b771bd18df5ca375d5e7540"],
    "tx_key_list": ["5b455c0f97168be652a2c03c5c68a064bb84cdae4ddef01b5c48d73a0bbb27075fb714f2ca19ea6c8ff592417e606addea6deb1d6530e2969f75681ffcbfc4075677b94a8c9197963ae38fa6f543ee68f0a4c4bbda4c453f39538f00b28e980ea08509730b51c004960101ba2f3adbc34cbbdff0d5af9dba061b523090debd06"],
    "unsigned_txset": ""
  }
}
```

### **sign_transfer**

Sign a transaction created on a read-only wallet (in cold-signing process)

Alias:  *None* .

Inputs:

* *unsigned_txset*  - string. Set of unsigned tx returned by "transfer" or "transfer_split" methods.
* *export_raw*  - boolean; (Optional) If true, return the raw transaction data. (Defaults to false)

Outputs:

* *signed_txset*  - string. Set of signed tx to be used for submitting transfer.
* *tx_hash_list*  - array of: string. The tx hashes of every transaction.
* *tx_raw_list*  - array of: string. The tx raw data of every transaction.

In the example below, we first generate an unsigned_txset on a read only wallet before signing it:

Generate unsigned_txset using the above "transfer" method on read-only wallet:

```
curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"transfer","params":{"destinations":[{"amount":10000,"address":"etnkjHMY5Cmbjmjuw8eEeXK41sJn7JS8Wfr4Px1SwVEXLMU8Zzafmm7QmFcCbsWgcaLqT7jQ7vF2zFnBgXQGpU1V4qGEsTE9T4"}],"account_index":0,"subaddr_indices":[0],"priority":0,"do_not_relay":true,"get_tx_hex":true}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "amount": 10000,
    "fee": 10,
    "multisig_txset": "",
    "tx_blob": "...long_hex...",
    "tx_hash": "c648ba0a049e5ce4ec21361dbf6e4b21eac0f828eea9090215de86c76b31d0a4",
    "tx_key": "",
    "tx_metadata": "",
    "unsigned_txset": "...long_hex..."
  }
}
```

Sign tx using the previously generated unsigned_txset

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"sign_transfer","params":{"unsigned_txset":...long_hex..."}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "signed_txset": "...long_hex...",
    "tx_hash_list": ["ff2e2d49fbfb1c9a55754f786576e171c8bf21b463a74438df604b7fa6cebc6d"]
  }
}
```

### **submit_transfer**

Submit a previously signed transaction on a read-only wallet (in cold-signing process).

Alias:  *None* .

Inputs:

* *tx_data_hex*  - string; Set of signed tx returned by "sign_transfer"

Outputs:

* *tx_hash_list*  - array of: string. The tx hashes of every transaction.

In the example below, we submit the transfer using the signed_txset generated above:

```
curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"submit_transfer","params":{"tx_data_hex":...long_hex..."}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "tx_hash_list": ["40fad7c828bb383ac02648732f7afce9adc520ba5629e1f5d9c03f584ac53d74"]
  }
}
```

### **sweep_dust**

Send all dust outputs back to the wallet's, to make them easier to spend (and mix).

Alias:  *sweep_unmixable* .

Inputs:

* *get_tx_keys*  - boolean; (Optional) Return the transaction keys after sending.
* *do_not_relay*  - boolean; (Optional) If true, the newly created transaction will not be relayed to the electroneum network. (Defaults to false)
* *get_tx_hex*  - boolean; (Optional) Return the transactions as hex string after sending. (Defaults to false)
* *get_tx_metadata*  - boolean; (Optional) Return list of transaction metadata needed to relay the transfer later. (Defaults to false)

Outputs:

* *tx_hash_list*  - array of: string. The tx hashes of every transaction.
* *tx_key_list*  - array of: string. The transaction keys for every transaction.
* *amount_list*  - array of: integer. The amount transferred for every transaction.
* *fee_list*  - array of: integer. The amount of fees paid for every transaction.
* *tx_blob_list*  - array of: string. The tx as hex string for every transaction.
* *tx_metadata_list*  - array of: string. List of transaction metadata needed to relay the transactions later.
* *multisig_txset*  - string. The set of signing keys used in a multisig transaction (empty for non-multisig).
* *unsigned_txset*  - string. Set of unsigned tx for cold-signing purposes.

Example (In this example,  `sweep_dust`  returns nothing because there are no funds to sweep):

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"sweep_dust","params":{"get_tx_keys":true}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "multisig_txset": "",
    "unsigned_txset": ""
  }
}
```

### **sweep_all**

Send all unlocked balance to an address.

Alias:  *None* .

Inputs:

* *address*  - string; Destination public address.
* *account_index*  - unsigned int; Sweep transactions from this account.
* *subaddr_indices*  - array of unsigned int; (Optional) Sweep from this set of subaddresses in the account.
* *priority*  - unsigned int; (Optional) Priority for sending the sweep transfer, partially determines fee.
* *mixin*  - unsigned int; Number of outputs from the blockchain to mix with (0 means no mixing).
* *ring_size*  - unsigned int; Sets ringsize to n (mixin + 1).
* *unlock_time*  - unsigned int; Number of blocks before the ETN can be spent (0 to not add a lock).
* *payment_id*  - string; (Optional) Random 32-byte/64-character hex string to identify a transaction.
* *get_tx_keys*  - boolean; (Optional) Return the transaction keys after sending.
* *below_amount*  - unsigned int; (Optional) Include outputs below this amount.
* *do_not_relay*  - boolean; (Optional) If true, do not relay this sweep transfer. (Defaults to false)
* *get_tx_hex*  - boolean; (Optional) return the transactions as hex encoded string. (Defaults to false)
* *get_tx_metadata*  - boolean; (Optional) return the transaction metadata as a string. (Defaults to false)

Outputs:

* *tx_hash_list*  - array of: string. The tx hashes of every transaction.
* *tx_key_list*  - array of: string. The transaction keys for every transaction.
* *amount_list*  - array of: integer. The amount transferred for every transaction.
* *fee_list*  - array of: integer. The amount of fees paid for every transaction.
* *tx_blob_list*  - array of: string. The tx as hex string for every transaction.
* *tx_metadata_list*  - array of: string. List of transaction metadata needed to relay the transactions later.
* *multisig_txset*  - string. The set of signing keys used in a multisig transaction (empty for non-multisig).
* *unsigned_txset*  - string. Set of unsigned tx for cold-signing purposes.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"sweep_all","params":{"address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY","subaddr_indices":[4],"unlock_time":0,"get_tx_keys":true}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "amount_list": [30000],
    "fee_list": [10],
    "multisig_txset": "",
    "tx_hash_list": ["ab4b6b65cc8cd8c9dd317d0b90d97582d68d0aa1637b0065b05b61f9a66ea5c5"],
    "tx_key_list": ["b9b4b39d3bb3062ddb85ec0266d4df39058f4c86077d99309f218ce4d76af607"],
    "unsigned_txset": ""
  }
}
```

### **sweep_single**

Send all of a specific unlocked output to an address.

Alias:  *None* .

Inputs:

* *address*  - string; Destination public address.
* *account_index*  - unsigned int; Sweep transactions from this account.
* *subaddr_indices*  - array of unsigned int; (Optional) Sweep from this set of subaddresses in the account.
* *priority*  - unsigned int; (Optional) Priority for sending the sweep transfer, partially determines fee.
* *mixin*  - unsigned int; Number of outputs from the blockchain to mix with (0 means no mixing).
* *ring_size*  - unsigned int; Sets ringsize to n (mixin + 1).
* *unlock_time*  - unsigned int; Number of blocks before the ETN can be spent (0 to not add a lock).
* *payment_id*  - string; (Optional) Random 32-byte/64-character hex string to identify a transaction.
* *get_tx_keys*  - boolean; (Optional) Return the transaction keys after sending.
* *key_image*  - string; Key image of specific output to sweep.
* *below_amount*  - unsigned int; (Optional) Include outputs below this amount.
* *do_not_relay*  - boolean; (Optional) If true, do not relay this sweep transfer. (Defaults to false)
* *get_tx_hex*  - boolean; (Optional) return the transactions as hex encoded string. (Defaults to false)
* *get_tx_metadata*  - boolean; (Optional) return the transaction metadata as a string. (Defaults to false)

Outputs:

* *tx_hash_list*  - array of: string. The tx hashes of every transaction.
* *tx_key_list*  - array of: string. The transaction keys for every transaction.
* *amount_list*  - array of: integer. The amount transferred for every transaction.
* *fee_list*  - array of: integer. The amount of fees paid for every transaction.
* *tx_blob_list*  - array of: string. The tx as hex string for every transaction.
* *tx_metadata_list*  - array of: string. List of transaction metadata needed to relay the transactions later.
* *multisig_txset*  - string. The set of signing keys used in a multisig transaction (empty for non-multisig).
* *unsigned_txset*  - string. Set of unsigned tx for cold-signing purposes.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"sweep_single","params":{"address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY","unlock_time":0,"key_image":"a7834459ef795d2efb6f665d2fd758c8d9288989d8d4c712a68f8023f7804a5e","get_tx_keys":true}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "amount": 30000,
    "fee": 10,
    "multisig_txset": "",
    "tx_blob": "",
    "tx_hash": "106d4391a031e5b735ded555862fec63233e34e5fa4fc7edcfdbe461c275ae5b",
    "tx_key": "",
    "tx_metadata": "",
    "unsigned_txset": ""
  }
}
```

### **relay_tx**

Relay a transaction previously created with  `"do_not_relay":true` .

Alias:  *None* .

Inputs:

* *hex*  - string; transaction metadata returned from a  `transfer`  method with  `get_tx_metadata`  set to  `true` .

Outputs:

* *tx_hash*  - String for the publically searchable transaction hash.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"relay_tx","params":{"hex":"...tx_metadata..."}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "tx_hash": "1c42dcc5672bb09bccf33fb1e9ab4a498af59a6dbd33b3d0cfb289b9e0e25fa5"
  }
}
```

### **store**

Save the wallet file.

Alias:  *None* .

Inputs:  *None* .

Outputs:  *None* .

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"store"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **get_payments**

Get a list of incoming payments using a given payment id.

Alias:  *None* .

Inputs:

* *payment_id*  - string; Payment ID used to find the payments (16 characters hex).

Outputs:

* *payments*  - list of:
  * *payment_id*  - string; Payment ID matching the input parameter.
  * *tx_hash*  - string; Transaction hash used as the transaction ID.
  * *amount*  - unsigned int; Amount for this payment.
  * *block_height*  - unsigned int; Height of the block that first confirmed this payment.
  * *unlock_time*  - unsigned int; Time (in block height) until this payment is safe to spend.
  * *subaddr_index*  - subaddress index:
    * *major*  - unsigned int; Account index for the subaddress.
    * *minor*  - unsigned int; Index of the subaddress in the account.
  * *address*  - string; Address receiving the payment; Base58 representation of the public keys.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_payments","params":{"payment_id":"60900e5603bf96e3"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "payments": [{
      "address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY",
      "amount": 1000000,
      "block_height": 727606,
      "payment_id": "60900e5603bf96e3",
      "subaddr_index": {
        "major": 0,
        "minor": 0
      },
      "tx_hash": "3292e83ad28fc1cc7bc26dbd38862308f4588680fbf93eae3e803cddd1bd614f",
      "unlock_time": 0
    }]
  }
}
```

### **get_bulk_payments**

Get a list of incoming payments using a given payment id, or a list of payments ids, from a given height. This method is the preferred method over  `get_payments`  because it has the same functionality but is more extendable. Either is fine for looking up transactions by a single payment ID.

Alias:  *None* .

Inputs:

* *payment_ids*  - array of: string; Payment IDs used to find the payments (16 characters hex).
* *min_block_height*  - unsigned int; The block height at which to start looking for payments.

Outputs:

* *payments*  - list of:
  * *payment_id*  - string; Payment ID matching one of the input IDs.
  * *tx_hash*  - string; Transaction hash used as the transaction ID.
  * *amount*  - unsigned int; Amount for this payment.
  * *block_height*  - unsigned int; Height of the block that first confirmed this payment.
  * *unlock_time*  - unsigned int; Time (in block height) until this payment is safe to spend.
  * *subaddr_index*  - subaddress index:
    * *major*  - unsigned int; Account index for the subaddress.
    * *minor*  - unsigned int; Index of the subaddress in the account.
  * *address*  - string; Address receiving the payment; Base58 representation of the public keys.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_bulk_payments","params":{"payment_ids":["60900e5603bf96e3"],"min_block_height":"727000"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "payments": [{
      "address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY",
      "amount": 1000000,
      "block_height": 727606,
      "payment_id": "60900e5603bf96e3",
      "subaddr_index": {
        "major": 0,
        "minor": 0
      },
      "tx_hash": "3292e83ad28fc1cc7bc26dbd38862308f4588680fbf93eae3e803cddd1bd614f",
      "unlock_time": 0
    }]
  }
}
```

### **incoming_transfers**

Return a list of incoming transfers to the wallet.

Inputs:

* *transfer_type*  - string; "all": all the transfers, "available": only transfers which are not yet spent, OR "unavailable": only transfers which are already spent.
* *account_index*  - unsigned int; (Optional) Return transfers for this account. (defaults to 0)
* *subaddr_indices*  - array of unsigned int; (Optional) Return transfers sent to these subaddresses.
* *verbose*  - boolean; (Optional) Enable verbose output, return key image if true.

Outputs:

* *transfers*  - list of:
  * *amount*  - unsigned int; Amount of this transfer.
  * *global_index*  - unsigned int; Mostly internal use, can be ignored by most users.
  * *key_image*  - string; Key image for the incoming transfer's unspent output (empty unless verbose is true).
  * *spent*  - boolean; Indicates if this transfer has been spent.
  * *subaddr_index*  - unsigned int; Subaddress index for incoming transfer.
  * *tx_hash*  - string; Several incoming transfers may share the same hash if they were in the same transaction.
  * *tx_size*  - unsigned int; Size of transaction in bytes.

Example, get all transfers:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"incoming_transfers","params":{"transfer_type":"all","account_index":0,"subaddr_indices":[3],"verbose":true}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "transfers": [{
      "amount": 60000,
      "global_index": 122405,
      "key_image": "768f5144777eb23477ab7acf83562581d690abaf98ca897c03a9d2b900eb479b",
      "spent": true,
      "subaddr_index": 3,
      "tx_hash": "f53401f21c6a43e44d5dd7a90eba5cf580012ad0e15d050059136f8a0da34f6b",
      "tx_size": 159
    },{
      "amount": 27100,
      "global_index": 594994,
      "key_image": "7e561394806afd1be61980cc3431f6ef3569fa9151cd8d234f8ec13aa145695e",
      "spent": false,
      "subaddr_index": 3,
      "tx_hash": "106d4391a031e5b735ded555862fec63233e34e5fa4fc7edcfdbe461c275ae5b",
      "tx_size": 157
    },{
      "amount": 27100,
      "global_index": 594997,
      "key_image": "e76c0a3bfeaae35e4173712f782eb34011198e26b990225b71aa787c8ba8a157",
      "spent": false,
      "subaddr_index": 3,
      "tx_hash": "0bd959b59117ee1254bd8e5aa8e77ec04ef744144a1ffb2d5c1eb9380a719621",
      "tx_size": 158
    }]
  }
}
```

Example, get available transfers:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"incoming_transfers","params":{"transfer_type":"available","account_index":0,"subaddr_indices":[3],"verbose":true}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "transfers": [{
      "amount": 27120,
      "global_index": 594994,
      "key_image": "7e561394806afd1be61980cc3431f6ef3569fa9151cd8d234f8ec13aa145695e",
      "spent": false,
      "subaddr_index": 3,
      "tx_hash": "106d4391a031e5b735ded555862fec63233e34e5fa4fc7edcfdbe461c275ae5b",
      "tx_size": 157
    },{
      "amount": 27160,
      "global_index": 594997,
      "key_image": "e76c0a3bfeaae35e4173712f782eb34011198e26b990225b71aa787c8ba8a157",
      "spent": false,
      "subaddr_index": 3,
      "tx_hash": "0bd959b59117ee1254bd8e5aa8e77ec04ef744144a1ffb2d5c1eb9380a719621",
      "tx_size": 158
    }]
  }
}
```

Example, get unavailable transfers:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"incoming_transfers","params":{"transfer_type":"unavailable","account_index":0,"subaddr_indices":[3],"verbose":true}}' -H 'Content-Type: application/json'
{
"id": "0",
"jsonrpc": "2.0",
"result": {
  "transfers": [{
    "amount": 60000,
    "global_index": 122405,
    "key_image": "768f5144777eb23477ab7acf83562581d690abaf98ca897c03a9d2b900eb479b",
    "spent": true,
    "subaddr_index": 3,
    "tx_hash": "f53401f21c6a43e44d5dd7a90eba5cf580012ad0e15d050059136f8a0da34f6b",
    "tx_size": 159
  }]
}
}
```

### **query_key**

Return the spend or view private key.

Alias:  *None* .

Inputs:

* *key_type*  - string; Which key to retrieve: "mnemonic" - the mnemonic seed (older wallets do not have one) OR "view_key" - the view key

Outputs:

* *key*  - string; The view key will be hex encoded, while the mnemonic will be a string of words.

Example (Query view key):

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"query_key","params":{"key_type":"view_key"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "key": "0a1a38f6d246e894600a3e27238a064bf5e8d91801df47a17107596b1378e501"
  }
}
```

Example (Query mnemonic key):

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"query_key","params":{"key_type":"mnemonic"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "key": "vocal either anvil films dolphin zeal bacon cuisine quote syndrome rejoices envy okay pancakes tulips lair greater petals organs enmity dedicated oust thwart tomorrow tomorrow"
  }
}
```

### **make_integrated_address**

Make an integrated address from the wallet address and a payment id.

Alias:  *None* .

Inputs:

* *standard_address*  - string; (Optional, defaults to primary address) Destination public address.
* *payment_id*  - string; (Optional, defaults to a random ID) 16 characters hex encoded.

Outputs:

* *integrated_address*  - string
* *payment_id*  - string; hex encoded;

Example (Payment ID is empty, use a random ID):

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"make_integrated_address","params":{"standard_address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "integrated_address": "f4VR5M6EYBQPsJvihejok8h63PadX47sgYmVttR9aCdbNeDgKWemFkzcaVHJYyZTnh8Po4UMwQmYp4HVGss7zyiQBX7DsG9xrkW27C7WndyvR",
    "payment_id": "420fa29b2d9a49f5"
  }
}
```

### **split_integrated_address**

Retrieve the standard address and payment id corresponding to an integrated address.

Alias:  *None* .

Inputs:

* *integrated_address*  - string

Outputs:

* *is_subaddress*  - boolean; States if the address is a subaddress
* *payment*  - string; hex encoded
* *standard_address*  - string

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"split_integrated_address","params":{"integrated_address": "f4VR5M6EYBQPsJvihejok8h63PadX47sgYmVttR9aCdbNeDgKWemFkzcaVHJYyZTnh8Po4UMwQmYp4HVGss7zyiQBX7DsG9xrkW27C7WndyvR"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "is_subaddress": false,
    "payment_id": "420fa29b2d9a49f5",
    "standard_address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY"
  }
}
```

### **stop_wallet**

Stops the wallet, storing the current state.

Alias:  *None* .

Inputs:  *None* .

Outputs:  *None* .

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"stop_wallet"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **rescan_blockchain**

Rescan the blockchain from scratch, losing any information which can not be recovered from the blockchain itself.
This includes destination addresses, tx secret keys, tx notes, etc.

Alias:  *None* .

Inputs:  *None* .

Outputs:  *None* .

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"rescan_blockchain"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **set_tx_notes**

Set arbitrary string notes for transactions.

Alias:  *None* .

Inputs:

* *txids*  - array of string; transaction ids
* *notes*  - array of string; notes for the transactions

Outputs:  *None* .

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"set_tx_notes","params":{"txids":["3292e83ad28fc1cc7bc26dbd38862308f4588680fbf93eae3e803cddd1bd614f"],"notes":["This is an example"]}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **get_tx_notes**

Get string notes for transactions.

Alias:  *None* .

Inputs:

* *txids*  - array of string; transaction ids

Outputs:

* *notes*  - array of string; notes for the transactions

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_tx_notes","params":{"txids":["3292e83ad28fc1cc7bc26dbd38862308f4588680fbf93eae3e803cddd1bd614f"]}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "notes": ["This is an example"]
  }
}
```

### **set_attribute**

Set arbitrary attribute.

Alias:  *None* .

Inputs:

* *key*  - string; attribute name
* *value*  - string; attribute value

Outputs:  *None* .

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"set_attribute","params":{"key":"my_attribute","value":"my_value"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **get_attribute**

Get attribute value by name.

Alias:  *None* .

Inputs:

* *key*  - string; attribute name

Outputs:

* *value*  - string; attribute value

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_attribute","params":{"key":"my_attribute"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "value": "my_value"
  }
}
```

### **get_tx_key**

Get transaction secret key from transaction id.

Alias:  *None* .

Inputs:

* *txid*  - string; transaction id.

Outputs:

* *tx_key*  - string; transaction secret key.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_tx_key","params":{"txid":"19d5089f9469db3d90aca9024dfcb17ce94b948300101c8345a5e9f7257353be"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "tx_key": "feba662cf8fb6d0d0da18fc9b70ab28e01cc76311278fdd7fe7ab16360762b06"
  }
}
```

### **check_tx_key**

Check a transaction in the blockchain with its secret key.

Alias:  *None* .

Inputs:

* *txid*  - string; transaction id.
* *tx_key*  - string; transaction secret key.
* *address*  - string; destination public address of the transaction.

Outputs:

* *confirmations*  - unsigned int; Number of block mined after the one with the transaction.
* *in_pool*  - boolean; States if the transaction is still in pool or has been added to a block.
* *received*  - unsigned int; Amount of the transaction.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"check_tx_key","params":{"txid":"19d5089f9469db3d90aca9024dfcb17ce94b948300101c8345a5e9f7257353be","tx_key":"feba662cf8fb6d0d0da18fc9b70ab28e01cc76311278fdd7fe7ab16360762b06","address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "confirmations": 0,
    "in_pool": false,
    "received": 10000
  }
}
```

### **get_tx_proof**

Get transaction signature to prove it.

Alias:  *None* .

Inputs:

* *txid*  - string; transaction id.
* *address*  - string; destination public address of the transaction.
* *message*  - string; (Optional) add a message to the signature to further authenticate the prooving process.

Outputs:

* *signature*  - string; transaction signature.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_tx_proof","params":{"txid":"19d5089f9469db3d90aca9024dfcb17ce94b948300101c8345a5e9f7257353be","address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY","message":"this is my transaction"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "signature": "InProofV13vqBCT6dpSAXkypZmSEMPGVnNRFDX2vscUYeVS4WnSVnV5BwLs31T9q6Etfj9Wts6tAxSAS4gkMeSYzzLS7Gt4vvCSQRh9niGJMUDJsB5hTzb2XJiCkUzWkkcjLFBBRVD5QZ"
  }
}
```

### **check_tx_proof**

Prove a transaction by checking its signature.

Alias:  *None* .

Inputs:

* *txid*  - string; transaction id.
* *address*  - string; destination public address of the transaction.
* *message*  - string; (Optional) Should be the same message used in  `get_tx_proof` .
* *signature*  - string; transaction signature to confirm.

Outputs:

* *confirmations*  - unsigned int; Number of block mined after the one with the transaction.
* *good*  - boolean; States if the inputs proves the transaction.
* *in_pool*  - boolean; States if the transaction is still in pool or has been added to a block.
* *received*  - unsigned int; Amount of the transaction.

In the example below, the transaction has been proven:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"check_tx_proof","params":{"txid":"19d5089f9469db3d90aca9024dfcb17ce94b948300101c8345a5e9f7257353be","address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY","message":"this is my transaction","signature":"InProofV13vqBCT6dpSAXkypZmSEMPGVnNRFDX2vscUYeVS4WnSVnV5BwLs31T9q6Etfj9Wts6tAxSAS4gkMeSYzzLS7Gt4vvCSQRh9niGJMUDJsB5hTzb2XJiCkUzWkkcjLFBBRVD5QZ"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "confirmations": 482,
    "good": true,
    "in_pool": false,
    "received": 10000
  }
}
```

In the example below, the wrong message is used, avoiding the transaction to be proved:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"check_tx_proof","params":{"txid":"19d5089f9469db3d90aca9024dfcb17ce94b948300101c8345a5e9f7257353be","address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY","message":"wrong message","signature":"InProofV13vqBCT6dpSAXkypZmSEMPGVnNRFDX2vscUYeVS4WnSVnV5BwLs31T9q6Etfj9Wts6tAxSAS4gkMeSYzzLS7Gt4vvCSQRh9niGJMUDJsB5hTzb2XJiCkUzWkkcjLFBBRVD5QZ"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "confirmations": 0,
    "good": false,
    "in_pool": false,
    "received": 0
  }
}
```

### **get_spend_proof**

Generate a signature to prove a spend. Unlike proving a transaction, it does not requires the destination public address.

Alias:  *None* .

Inputs:

* *txid*  - string; transaction id.
* *message*  - string; (Optional) add a message to the signature to further authenticate the prooving process.

Outputs:

* *signature*  - string; spend signature.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_spend_proof","params":{"txid":"19d5089f9469db3d90aca9024dfcb17ce94b948300101c8345a5e9f7257353be","message":"this is my transaction"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "signature": "SpendProofV1aSh8Todhk54736iXgV6vJAFP7egxByuMWZeyNDaN2JY737S95X5zz5mNMQSuCNSLjjhi5HJCsndpNWSNVsuThxwv285qy1KkUrLFRkxMSCjfL6bbycYN33ScZ5UB4Fzseceo1ndpL393T1q638VmcU3a56dhNHF1RPZFiGPS61FA78nXFSqE9uoKCCoHkEz83M1dQVhxZV5CEPF2P6VioGTKgprLCH9vvj9k1ivd4SX19L2VSMc3zD1u3mkR24ioETvxBoLeBSpxMoikyZ6inhuPm8yYo9YWyFtQK4XYfAV9mJ9knz5fUPXR8vvh7KJCAg4dqeJXTVb4mbMzYtsSZXHd6ouWoyCd6qMALdW8pKhgMCHcVYMWp9X9WHZuCo9rsRjRpg15sJUw7oJg1JoGiVgj8P4JeGDjnZHnmLVa5bpJhVCbMhyM7JLXNQJzFWTGC27TQBbthxCfQaKdusYnvZnKPDJWSeceYEFzepUnsWhQtyhbb73FzqgWC4eKEFKAZJqT2LuuSoxmihJ9acnFK7Ze23KTVYgDyMKY61VXADxmSrBvwUtxCaW4nQtnbMxiPMNnDMzeixqsFMBtN72j5UqhiLRY99k6SE7Qf5f29haNSBNSXCFFHChPKNTwJrehkofBdKUhh2VGPqZDNoefWUwfudeu83t85bmjv8Q3LrQSkFgFjRT5tLo8TMawNXoZCrQpyZrEvnodMDDUUNf3NL7rxyv3gM1KrTWjYaWXFU2RAsFee2Q2MTwUW7hR25cJvSFuB1BX2bfkoCbiMk923tHZGU2g7rSKF1GDDkXAc1EvFFD4iGbh1Q5t6hPRhBV8PEncdcCWGq5uAL5D4Bjr6VXG8uNeCy5oYWNgbZ5JRSfm7QEhPv8Fy9AKMgmCxDGMF9dVEaU6tw2BAnJavQdfrxChbDBeQXzCbCfep6oei6n2LZdE5Q84wp7eoQFE5Cwuo23tHkbJCaw2njFi3WGBbA7uGZaGHJPyB2rofTWBiSUXZnP2hiE9bjJghAcDm1M4LVLfWvhZmFEnyeru3VWMETnetz1BYLUC5MJGFXuhnHwWh7F6r74FDyhdswYop4eWPbyrXMXmUQEccTGd2NaT8g2VHADZ76gMC6BjWESvcnz2D4n8XwdmM7ZQ1jFwhuXrBfrb1dwRasyXxxHMGAC2onatNiExyeQ9G1W5LwqNLAh9hvcaNTGaYKYXoceVzLkgm6e5WMkLsCwuZXvB"
  }
}
```

### **check_spend_proof**

Prove a spend using a signature. Unlike proving a transaction, it does not requires the destination public address.

Alias:  *None* .

Inputs:

* *txid*  - string; transaction id.
* *message*  - string; (Optional) Should be the same message used in  `get_spend_proof` .
* *signature*  - string; spend signature to confirm.

Outputs:

* *good*  - boolean; States if the inputs proves the spend.

In the example below, the spend has been proven:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"check_spend_proof","params":{"txid":"19d5089f9469db3d90aca9024dfcb17ce94b948300101c8345a5e9f7257353be","message":"this is my transaction","signature":"SpendProofV1aSh8Todhk54736iXgV6vJAFP7egxByuMWZeyNDaN2JY737S95X5zz5mNMQSuCNSLjjhi5HJCsndpNWSNVsuThxwv285qy1KkUrLFRkxMSCjfL6bbycYN33ScZ5UB4Fzseceo1ndpL393T1q638VmcU3a56dhNHF1RPZFiGPS61FA78nXFSqE9uoKCCoHkEz83M1dQVhxZV5CEPF2P6VioGTKgprLCH9vvj9k1ivd4SX19L2VSMc3zD1u3mkR24ioETvxBoLeBSpxMoikyZ6inhuPm8yYo9YWyFtQK4XYfAV9mJ9knz5fUPXR8vvh7KJCAg4dqeJXTVb4mbMzYtsSZXHd6ouWoyCd6qMALdW8pKhgMCHcVYMWp9X9WHZuCo9rsRjRpg15sJUw7oJg1JoGiVgj8P4JeGDjnZHnmLVa5bpJhVCbMhyM7JLXNQJzFWTGC27TQBbthxCfQaKdusYnvZnKPDJWSeceYEFzepUnsWhQtyhbb73FzqgWC4eKEFKAZJqT2LuuSoxmihJ9acnFK7Ze23KTVYgDyMKY61VXADxmSrBvwUtxCaW4nQtnbMxiPMNnDMzeixqsFMBtN72j5UqhiLRY99k6SE7Qf5f29haNSBNSXCFFHChPKNTwJrehkofBdKUhh2VGPqZDNoefWUwfudeu83t85bmjv8Q3LrQSkFgFjRT5tLo8TMawNXoZCrQpyZrEvnodMDDUUNf3NL7rxyv3gM1KrTWjYaWXFU2RAsFee2Q2MTwUW7hR25cJvSFuB1BX2bfkoCbiMk923tHZGU2g7rSKF1GDDkXAc1EvFFD4iGbh1Q5t6hPRhBV8PEncdcCWGq5uAL5D4Bjr6VXG8uNeCy5oYWNgbZ5JRSfm7QEhPv8Fy9AKMgmCxDGMF9dVEaU6tw2BAnJavQdfrxChbDBeQXzCbCfep6oei6n2LZdE5Q84wp7eoQFE5Cwuo23tHkbJCaw2njFi3WGBbA7uGZaGHJPyB2rofTWBiSUXZnP2hiE9bjJghAcDm1M4LVLfWvhZmFEnyeru3VWMETnetz1BYLUC5MJGFXuhnHwWh7F6r74FDyhdswYop4eWPbyrXMXmUQEccTGd2NaT8g2VHADZ76gMC6BjWESvcnz2D4n8XwdmM7ZQ1jFwhuXrBfrb1dwRasyXxxHMGAC2onatNiExyeQ9G1W5LwqNLAh9hvcaNTGaYKYXoceVzLkgm6e5WMkLsCwuZXvB"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "good": true
  }
}
```

In the example below, the wrong message is used, avoiding the spend to be proved:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"check_spend_proof","params":{"txid":"19d5089f9469db3d90aca9024dfcb17ce94b948300101c8345a5e9f7257353be","message":"wrong message","signature":"SpendProofV1aSh8Todhk54736iXgV6vJAFP7egxByuMWZeyNDaN2JY737S95X5zz5mNMQSuCNSLjjhi5HJCsndpNWSNVsuThxwv285qy1KkUrLFRkxMSCjfL6bbycYN33ScZ5UB4Fzseceo1ndpL393T1q638VmcU3a56dhNHF1RPZFiGPS61FA78nXFSqE9uoKCCoHkEz83M1dQVhxZV5CEPF2P6VioGTKgprLCH9vvj9k1ivd4SX19L2VSMc3zD1u3mkR24ioETvxBoLeBSpxMoikyZ6inhuPm8yYo9YWyFtQK4XYfAV9mJ9knz5fUPXR8vvh7KJCAg4dqeJXTVb4mbMzYtsSZXHd6ouWoyCd6qMALdW8pKhgMCHcVYMWp9X9WHZuCo9rsRjRpg15sJUw7oJg1JoGiVgj8P4JeGDjnZHnmLVa5bpJhVCbMhyM7JLXNQJzFWTGC27TQBbthxCfQaKdusYnvZnKPDJWSeceYEFzepUnsWhQtyhbb73FzqgWC4eKEFKAZJqT2LuuSoxmihJ9acnFK7Ze23KTVYgDyMKY61VXADxmSrBvwUtxCaW4nQtnbMxiPMNnDMzeixqsFMBtN72j5UqhiLRY99k6SE7Qf5f29haNSBNSXCFFHChPKNTwJrehkofBdKUhh2VGPqZDNoefWUwfudeu83t85bmjv8Q3LrQSkFgFjRT5tLo8TMawNXoZCrQpyZrEvnodMDDUUNf3NL7rxyv3gM1KrTWjYaWXFU2RAsFee2Q2MTwUW7hR25cJvSFuB1BX2bfkoCbiMk923tHZGU2g7rSKF1GDDkXAc1EvFFD4iGbh1Q5t6hPRhBV8PEncdcCWGq5uAL5D4Bjr6VXG8uNeCy5oYWNgbZ5JRSfm7QEhPv8Fy9AKMgmCxDGMF9dVEaU6tw2BAnJavQdfrxChbDBeQXzCbCfep6oei6n2LZdE5Q84wp7eoQFE5Cwuo23tHkbJCaw2njFi3WGBbA7uGZaGHJPyB2rofTWBiSUXZnP2hiE9bjJghAcDm1M4LVLfWvhZmFEnyeru3VWMETnetz1BYLUC5MJGFXuhnHwWh7F6r74FDyhdswYop4eWPbyrXMXmUQEccTGd2NaT8g2VHADZ76gMC6BjWESvcnz2D4n8XwdmM7ZQ1jFwhuXrBfrb1dwRasyXxxHMGAC2onatNiExyeQ9G1W5LwqNLAh9hvcaNTGaYKYXoceVzLkgm6e5WMkLsCwuZXvB"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "good": false
  }
}
```

### **get_reserve_proof**

Generate a signature to prove of an available amount in a wallet.

Alias:  *None* .

Inputs:

* *all*  - boolean; Proves all wallet balance to be disposable.
* *account_index*  - unsigned int; Specify the account from witch to prove reserve. (ignored if  `all`  is set to true)
* *amount*  - unsigned int; Amount (in Atomic Units) to prove the account has for reserve. (ignored if  `all`  is set to true)
* *message*  - string; (Optional) add a message to the signature to further authenticate the prooving process.

Outputs:

* *signature*  - string; reserve signature.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_reserve_proof","params":{"all":false,"account_index":0,"amount":10000}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "signature": "ReserveProofV11BZ23sBt9sZJeGccf84mzyAmNCP3KzYbE1111112VKmH111118NfCYJQjZ6c46gT2kXgcHCaSSZeL8sRdzqjqx7i1e7FQfQGu2o113UYFVdwzHQi3iENDPa76Kn1BvywbKz3bMkXdZkBEEhBSF4kjjGaiMJ1ucKb6wvMVC4A8sA4nZEdL2Mk3wBucJCYTZwKqA8i1M113kqakDkG25FrjiDqdQTCYz2wDBmfKxF3eQiV5FWzZ6HmAyxnqTWUiMWukP9A3Edy3ZXqjP1b23dhz7Mbj39bBxe3ZeDNu9HnTSqYvHNRyqCkeUMJpHyQweqjGUJ1DSfFYr33J1E7MkhMnEi1o7trqWjVix32XLetYfePG73yvHbS24837L7Q64i5n1LSpd9yMiQZ3Dyaysi5y6jPx7TpAvnSqBFtuCciKoNzaXoA3dqt9cuVFZTXzdXKqdt3cXcVJMNxY8RvKPVQHhUur94Lpo1nSpxf7BN5a5rHrbZFqoZszsZmiWikYPkLX72XUdw6NWjLrTBxSy7KuPYH86c6udPEXLo2xgN6XHMBMBJzt8FqqK7EcpNUBkuHm2AtpGkf9CABY3oSjDQoRF5n4vNLd3qUaxNsG4XJ12L9gJ7GrK273BxkfEA8fDdxPrb1gpespbgEnCTuZHqj1A"
  }
}
```

### **check_reserve_proof**

Proves a wallet has a disposable reserve using a signature.

Alias:  *None* .

Inputs:

* *address*  - string; Public address of the wallet.
* *message*  - string; (Optional) Should be the same message used in  `get_reserve_proof` .
* *signature*  - string; reserve signature to confirm.

Outputs:

* *good*  - boolean; States if the inputs proves the reserve.

In the example below, the reserve has been proven:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"check_reserve_proof","params":{"address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY","signature":"ReserveProofV11BZ23sBt9sZJeGccf84mzyAmNCP3KzYbE1111112VKmH111118NfCYJQjZ6c46gT2kXgcHCaSSZeL8sRdzqjqx7i1e7FQfQGu2o113UYFVdwzHQi3iENDPa76Kn1BvywbKz3bMkXdZkBEEhBSF4kjjGaiMJ1ucKb6wvMVC4A8sA4nZEdL2Mk3wBucJCYTZwKqA8i1M113kqakDkG25FrjiDqdQTCYz2wDBmfKxF3eQiV5FWzZ6HmAyxnqTWUiMWukP9A3Edy3ZXqjP1b23dhz7Mbj39bBxe3ZeDNu9HnTSqYvHNRyqCkeUMJpHyQweqjGUJ1DSfFYr33J1E7MkhMnEi1o7trqWjVix32XLetYfePG73yvHbS24837L7Q64i5n1LSpd9yMiQZ3Dyaysi5y6jPx7TpAvnSqBFtuCciKoNzaXoA3dqt9cuVFZTXzdXKqdt3cXcVJMNxY8RvKPVQHhUur94Lpo1nSpxf7BN5a5rHrbZFqoZszsZmiWikYPkLX72XUdw6NWjLrTBxSy7KuPYH86c6udPEXLo2xgN6XHMBMBJzt8FqqK7EcpNUBkuHm2AtpGkf9CABY3oSjDQoRF5n4vNLd3qUaxNsG4XJ12L9gJ7GrK273BxkfEA8fDdxPrb1gpespbgEnCTuZHqj1A"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "good": true,
    "spent": 0,
    "total": 10000
  }
}
```

In the example below, all wallet reserve has been proven:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"check_reserve_proof","params":{"address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY","message":"I have 10 at least","signature":"...signature..."}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "good": true,
    "spent": 0,
    "total": 16411
  }
}
```

In the example below, the wrong message is used, avoiding the reserve to be proved:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"check_spend_proof","params":{"txid":"19d5089f9469db3d90aca9024dfcb17ce94b948300101c8345a5e9f7257353be","message":"wrong message","signature":"SpendProofV1aSh8Todhk54736iXgV6vJAFP7egxByuMWZeyNDaN2JY737S95X5zz5mNMQSuCNSLjjhi5HJCsndpNWSNVsuThxwv285qy1KkUrLFRkxMSCjfL6bbycYN33ScZ5UB4Fzseceo1ndpL393T1q638VmcU3a56dhNHF1RPZFiGPS61FA78nXFSqE9uoKCCoHkEz83M1dQVhxZV5CEPF2P6VioGTKgprLCH9vvj9k1ivd4SX19L2VSMc3zD1u3mkR24ioETvxBoLeBSpxMoikyZ6inhuPm8yYo9YWyFtQK4XYfAV9mJ9knz5fUPXR8vvh7KJCAg4dqeJXTVb4mbMzYtsSZXHd6ouWoyCd6qMALdW8pKhgMCHcVYMWp9X9WHZuCo9rsRjRpg15sJUw7oJg1JoGiVgj8P4JeGDjnZHnmLVa5bpJhVCbMhyM7JLXNQJzFWTGC27TQBbthxCfQaKdusYnvZnKPDJWSeceYEFzepUnsWhQtyhbb73FzqgWC4eKEFKAZJqT2LuuSoxmihJ9acnFK7Ze23KTVYgDyMKY61VXADxmSrBvwUtxCaW4nQtnbMxiPMNnDMzeixqsFMBtN72j5UqhiLRY99k6SE7Qf5f29haNSBNSXCFFHChPKNTwJrehkofBdKUhh2VGPqZDNoefWUwfudeu83t85bmjv8Q3LrQSkFgFjRT5tLo8TMawNXoZCrQpyZrEvnodMDDUUNf3NL7rxyv3gM1KrTWjYaWXFU2RAsFee2Q2MTwUW7hR25cJvSFuB1BX2bfkoCbiMk923tHZGU2g7rSKF1GDDkXAc1EvFFD4iGbh1Q5t6hPRhBV8PEncdcCWGq5uAL5D4Bjr6VXG8uNeCy5oYWNgbZ5JRSfm7QEhPv8Fy9AKMgmCxDGMF9dVEaU6tw2BAnJavQdfrxChbDBeQXzCbCfep6oei6n2LZdE5Q84wp7eoQFE5Cwuo23tHkbJCaw2njFi3WGBbA7uGZaGHJPyB2rofTWBiSUXZnP2hiE9bjJghAcDm1M4LVLfWvhZmFEnyeru3VWMETnetz1BYLUC5MJGFXuhnHwWh7F6r74FDyhdswYop4eWPbyrXMXmUQEccTGd2NaT8g2VHADZ76gMC6BjWESvcnz2D4n8XwdmM7ZQ1jFwhuXrBfrb1dwRasyXxxHMGAC2onatNiExyeQ9G1W5LwqNLAh9hvcaNTGaYKYXoceVzLkgm6e5WMkLsCwuZXvB"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "good": false
  }
}
```

### **get_transfers**

Returns a list of transfers.

Alias:  *None* .

Inputs:

* *in*  - boolean; (defaults to false) Include incoming transfers.
* *out*  - boolean; (defaults to false) Include outgoing transfers.
* *pending*  - boolean; (defaults to false) Include pending transfers.
* *failed*  - boolean; (defaults to false) Include failed transfers.
* *pool*  - boolean; (defaults to false) Include transfers from the daemon's transaction pool.
* *filter_by_height*  - boolean; (Optional) Filter transfers by block height.
* *min_height*  - unsigned int; (Optional) Minimum block height to scan for transfers, if filtering by height is enabled.
* *max_height*  - unsigned int; (Optional) Maximum block height to scan for transfers, if filtering by height is enabled (defaults to max block height).
* *account_index*  - unsigned int; (Optional) Index of the account to query for transfers. (defaults to 0)
* *subaddr_indices*  - array of unsigned int; (Optional) List of subaddress indices to query for transfers. (Defaults to empty - all indices)

Outputs:

* *in*  array of transfers:
  * *address*  - string; Public address of the transfer.
  * *amount*  - unsigned int; Amount transferred.
  * *confirmations*  - unsigned int; Number of block mined since the block containing this transaction (or block height at which the transaction should be added to a block if not yet confirmed).
  * *double_spend_seen*  - boolean; True if the key image(s) for the transfer have been seen before.
  * *fee*  - unsigned int; Transaction fee for this transfer.
  * *height*  - unsigned int; Height of the first block that confirmed this transfer (0 if not mined yet).
  * *note*  - string; Note about this transfer.
  * *payment_id*  - string; Payment ID for this transfer.
  * *subaddr_index*  - JSON object containing the major & minor subaddress index:
    * *major*  - unsigned int; Account index for the subaddress.
    * *minor*  - unsigned int; Index of the subaddress under the account.
  * *suggested_confirmations_threshold*  - unsigned int; Estimation of the confirmations needed for the transaction to be included in a block.
  * *timestamp*  - unsigned int; POSIX timestamp for when this transfer was first confirmed in a block (or timestamp submission if not mined yet).
  * *txid*  - string; Transaction ID for this transfer.
  * *type*  - string; Transfer type: "in"
  * *unlock_time*  - unsigned int; Number of blocks until transfer is safely spendable.
* *out*  array of transfers (see above).
* *pending*  array of transfers (see above).
* *failed*  array of transfers (see above).
* *pool*  array of transfers (see above).

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_transfers","params":{"in":true,"account_index":1}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "in": [{
      "address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY",
      "amount": 200000,
      "confirmations": 1,
      "double_spend_seen": false,
      "fee": 10,
      "height": 153624,
      "note": "",
      "payment_id": "0000000000000000",
      "subaddr_index": {
        "major": 1,
        "minor": 0
      },
      "suggested_confirmations_threshold": 1,
      "timestamp": 1535918400,
      "txid": "c36258a276018c3a4bc1f195a7fb530f50cd63a4fa765fb7c6f7f49fc051762a",
      "type": "in",
      "unlock_time": 0
    }]
  }
}
```

### **get_transfer_by_txid**

Show information about a transfer to/from this address.

Alias:  *None* .

Inputs:

* *txid*  - string; Transaction ID used to find the transfer.
* *account_index*  - unsigned int; (Optional) Index of the account to query for the transfer.

Outputs:

* *transfer*  - JSON object containing payment information:
  * *address*  - string; Address that transferred the funds. Base58 representation of the public keys.
  * *amount*  - unsigned int; Amount of this transfer.
  * *confirmations*  - unsigned int; Number of block mined since the block containing this transaction (or block height at which the transaction should be added to a block if not yet confirmed).
  * *destinations*  - array of JSON objects containing transfer destinations:
    * *amount*  - unsigned int; Amount transferred to this destination.
    * *address*  - string; Address for this destination. Base58 representation of the public keys.
  * *double_spend_seen*  - boolean; True if the key image(s) for the transfer have been seen before.
  * *fee*  - unsigned int; Transaction fee for this transfer.
  * *height*  - unsigned int; Height of the first block that confirmed this transfer.
  * *note*  - string; Note about this transfer.
  * *payment_id*  - string; Payment ID for this transfer.
  * *subaddr_index*  - JSON object containing the major & minor subaddress index:
    * *major*  - unsigned int; Account index for the subaddress.
    * *minor*  - unsigned int; Index of the subaddress under the account.
  * *suggested_confirmations_threshold*  - unsigned int; Estimation of the confirmations needed for the transaction to be included in a block.
  * *timestamp*  - unsigned int; POSIX timestamp for the block that confirmed this transfer (or timestamp submission if not mined yet).
  * *txid*  - string; Transaction ID of this transfer (same as input TXID).
  * *type*  - string; Type of transfer, one of the following: "in", "out", "pending", "failed", "pool"
  * *unlock_time*  - unsigned int; Number of blocks until transfer is safely spendable.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_transfer_by_txid","params":{"txid":"c36258a276018c3a4bc1f195a7fb530f50cd63a4fa765fb7c6f7f49fc051762a"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "transfer": {
      "address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY",
      "amount": 30000,
      "confirmations": 1,
      "destinations": [{
        "address": "etnkjHMY5Cmbjmjuw8eEeXK41sJn7JS8Wfr4Px1SwVEXLMU8Zzafmm7QmFcCbsWgcaLqT7jQ7vF2zFnBgXQGpU1V4qGEsTE9T4",
        "amount": 10000
      },{
        "address": "etnkjHMY5Cmbjmjuw8eEeXK41sJn7JS8Wfr4Px1SwVEXLMU8Zzafmmdaha6Ajda92aJKda5dada6D3AD6ddawgbtyik3Fd84GQ",
        "amount": 20000
      }],
      "double_spend_seen": false,
      "fee": 10,
      "height": 153624,
      "note": "",
      "payment_id": "0000000000000000",
      "subaddr_index": {
        "major": 0,
        "minor": 0
      },
      "suggested_confirmations_threshold": 1,
      "timestamp": 1535918400,
      "txid": "c36258a276018c3a4bc1f195a7fb530f50cd63a4fa765fb7c6f7f49fc051762a",
      "type": "out",
      "unlock_time": 0
    }
  }
}
```

### **sign**

Sign a string.

Alias:  *None* .

Inputs:

* *data*  - string; Anything you need to sign.

Outputs:

* *signature*  - string; Signature generated against the "data" and the account public address.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"sign","params":{"data":"This is sample data to be signed"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "signature": "SigV14K6G151gycjiGxjQ74tKX6A2LwwghvuHjcDeuRFQio5LS6Gb27BNxjYQY1dPuUvXkEbGQUkiHSVLPj4nJAHRrrw3"
  }
}
```

### **verify**

Verify a signature on a string.

Alias:  *None* .

Inputs:

* *data*  - string; What should have been signed.
* *address*  - string; Public address of the wallet used to  `sign`  the data.
* *signature*  - string; signature generated by  `sign`  method.

Outputs:

* *good*  - boolean;

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"verify","params":{"data":"This is sample data to be signed","address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY","signature":"SigV14K6G151gycjiGxjQ74tKX6A2LwwghvuHjcDeuRFQio5LS6Gb27BNxjYQY1dPuUvXkEbGQUkiHSVLPj4nJAHRrrw3"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "good": true
  }
}
```

### **export_outputs**

Export all outputs in hex format.

Alias:  *None* .

Inputs:  *None* .

Outputs:

* *outputs_data_hex*  - string; wallet outputs in hex format.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"export_outputs"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "outputs_data_hex": "...outputs..."
  }
}
```

### **import_outputs**

Import outputs in hex format.

Alias:  *None* .

Inputs:

* *outputs_data_hex*  - string; wallet outputs in hex format.

Outputs:

* *num_imported*  - unsigned int; number of outputs imported.

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"import_outputs","params":{"outputs_data_hex":"...outputs..."}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "num_imported": 6400
  }
}
```

### **export_key_images**

Export a signed set of key images.

Alias:  *None* .

Inputs:  *None* .

Outputs:

* *signed_key_images*  - array of signed key images:
  * *key_image*  - string;
  * *signature*  - string;

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"export_key_images"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "signed_key_images": [{
      "key_image": "cd35239b72a35e26a57ed17400c0b66944a55de9d5bda0f21190fed17f8ea876",
      "signature": "c9d736869355da2538ab4af188279f84138c958edbae3c5caf388a63cd8e780b8c5a1aed850bd79657df659422c463608ea4e0c730ba9b662c906ae933816d00"
    },{
      "key_image": "65158a8ee5a3b32009b85a307d85b375175870e560e08de313531c7dbbe6fc19",
      "signature": "c96e40d09dfc45cfc5ed0b76bfd7ca793469588bb0cf2b4d7b45ef23d40fd4036057b397828062e31700dc0c2da364f50cd142295a8405b9fe97418b4b745d0c"
    },...]
  }
}
```

### **import_key_images**

Import signed key images list and verify their spent status.

Alias:  *None* .

Inputs:

* *signed_key_images*  - array of signed key images:
  * *key_image*  - string;
  * *signature*  - string;

Outputs:

* *height*  - unsigned int;
* *spent*  - unsigned int; Amount (in Atomic Units) spent from those key images.
* *unspent*  - unsigned int; Amount (in Atomic Units) still available from those key images.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"import_key_images", "params":{"signed_key_images":[{"key_image":"cd35239b72a35e26a57ed17400c0b66944a55de9d5bda0f21190fed17f8ea876","signature":"c9d736869355da2538ab4af188279f84138c958edbae3c5caf388a63cd8e780b8c5a1aed850bd79657df659422c463608ea4e0c730ba9b662c906ae933816d00"},{"key_image":"65158a8ee5a3b32009b85a307d85b375175870e560e08de313531c7dbbe6fc19","signature":"c96e40d09dfc45cfc5ed0b76bfd7ca793469588bb0cf2b4d7b45ef23d40fd4036057b397828062e31700dc0c2da364f50cd142295a8405b9fe97418b4b745d0c"}]}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "height": 76428,
    "spent": 62708,
    "unspent": 0
  }
}
```

### **make_uri**

Create a payment URI using the official URI spec.

Alias:  *None* .

Inputs:

* *address*  - string; Wallet address
* *amount*  - unsigned int; (optional) the integer amount to receive, in  **Atomic Units**
* *payment_id*  - string; (optional) 16 or 64 character hexadecimal payment id
* *recipient_name*  - string; (optional) name of the payment recipient
* *tx_description*  - string; (optional) Description of the reason for the tx

Outputs:

* *uri*  - string; This contains all the payment input information as a properly formatted payment URI

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"make_uri","params":{"address":"etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY","amount":10,"payment_id":"420fa29b2d9a49f5","tx_description":"Testing out the make_uri function.","recipient_name":"el00ruobuob Stagenet wallet"}}'  -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "uri": "electroneum:etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY?tx_payment_id=420fa29b2d9a49f5&tx_amount=10.05&recipient_name=el00ruobuob%20Stagenet%20wallet&tx_description=Testing%20out%20the%20make_uri%20function."
  }
}
```

### **parse_uri**

Parse a payment URI to get payment information.

Alias:  *None* .

Inputs:

* *uri*  - string; This contains all the payment input information as a properly formatted payment URI

Outputs:

* *uri*  - JSON object containing payment information:
  * *address*  - string; Wallet address
  * *amount*  - unsigned int; Integer amount to receive, in  **Atomic Units**  (0 if not provided)
  * *payment_id*  - string; 16 or 64 character hexadecimal payment id (empty if not provided)
  * *recipient_name*  - string; Name of the payment recipient (empty if not provided)
  * *tx_description*  - string; Description of the reason for the tx (empty if not provided)

Example:

```
$ curl -X POST http://127.0.0.1:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"parse_uri","params":{"uri":"electroneum:etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY?tx_payment_id=420fa29b2d9a49f5&tx_amount=10.00&recipient_name=el00ruobuob%20Stagenet%20wallet&tx_description=Testing%20out%20the%20make_uri%20function."}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "uri": {
      "address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY",
      "amount": 1000,
      "payment_id": "420fa29b2d9a49f5",
      "recipient_name": "el00ruobuob Stagenet wallet",
      "tx_description": "Testing out the make_uri function."
    }
  }
}
```

### **get_address_book**

Retrieves entries from the address book.

Alias:  *None* .

Inputs:

* *entries*  - array of unsigned int; indices of the requested address book entries

Outputs:

* *entries*  - array of entries:
  * *address*  - string; Public address of the entry
  * *description*  - string; Description of this address entry
  * *index*  - unsigned int;
  * *payment_id*  - string;

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_address_book","params":{"entries":[0,1]}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "entries": [{
      "address": "etnku32HZrgZBqT2D3gtMBVvyAYobaPpQP38Ju7TBq89Yz5o8TVp4NnXkVX4D5f3sLHgpD8haep9HKk69eu4sQmw8jbVUfQ1cY",
      "description": "Second account",
      "index": 0,
      "payment_id": "0000000000000000000000000000000000000000000000000000000000000000"
    },{
      "address": "etnk4YGjvutPsJvihejok8h63PadX47sgYmVttR9aCdbNeDgKWemFkzcaVHJYyZTnh8Po4UMwQmYp4HVGss7zyiQ3PEC7VVkc7",
      "description": "Third account",
      "index": 1,
      "payment_id": "0000000000000000000000000000000000000000000000000000000000000000"
    }]
  }
}
```

### **add_address_book**

Add an entry to the address book.

Alias:  *None* .

Inputs:

* *address*  - string;
* *payment_id*  - (optional) string, defaults to "0000000000000000000000000000000000000000000000000000000000000000";
* *description*  - (optional) string, defaults to "";

Outputs:

* *index*  - unsigned int; The index of the address book entry.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"add_address_book","params":{"address":"etnk4YGjvutPsJvihejok8h63PadX47sgYmVttR9aCdbNeDgKWemFkzcaVHJYyZTnh8Po4UMwQmYp4HVGss7zyiQ3PEC7VVkc7","description":"Third account"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "index": 1
  }
}
```

### **delete_address_book**

Delete an entry from the address book.

Alias:  *None* .

Inputs:

* *index*  - unsigned int; The index of the address book entry.

Outputs:  *None* .

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"delete_address_book","params":{"index":1}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **refresh**

Refresh a wallet after openning.

Alias:  *None* .

Inputs:

* *start_height*  - unsigned int; (Optional) The block height from which to start refreshing.

Outputs:

* *blocks_fetched*  - unsigned int; Number of new blocks scanned.
* *received_money*  - boolean; States if transactions to the wallet have been found in the blocks.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"refresh","params":{"start_height":100000}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "blocks_fetched": 24,
    "received_money": true
  }
}
```

### **rescan_spent**

Rescan the blockchain for spent outputs.

Alias:  *None* .

Inputs:  *None* .

Outputs:  *None* .

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"rescan_spent"}' -H 'Content-Type: application/json'

{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **get_languages**

Get a list of available languages for your wallet's seed.

Alias:  *None* .

Inputs:  *None* .

Outputs:

* *languages*  - array of string; List of available languages

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_languages"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "languages": ["Deutsch","English","Español","Français","Italiano","Nederlands","Português","русский язык","日本語","简体中文 (中国)","Esperanto","Lojban"]
  }
}
```

### **create_wallet**

Create a new wallet. You need to have set the argument "–wallet-dir" when launching electroneum-wallet-rpc to make this work.

Alias:  *None* .

Inputs:

* *filename*  - string; Wallet file name.
* *password*  - string; (Optional) password to protect the wallet.
* *language*  - string; Language for your wallets' seed.

Outputs:  *None* .

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"create_wallet","params":{"filename":"mytestwallet","password":"mytestpassword","language":"English"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **open_wallet**

Open a wallet. You need to have set the argument "–wallet-dir" when launching electroneum-wallet-rpc to make this work.

Alias:  *None* .

Inputs:

* *filename*  - string; wallet name stored in –wallet-dir.
* *password*  - string; (Optional) only needed if the wallet has a password defined.

Outputs:  *None* .

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"open_wallet","params":{"filename":"mytestwallet","password":"mytestpassword"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **close_wallet**

Close the currently opened wallet, after trying to save it.

Alias:  *None* .

Inputs:  *None* .

Outputs:  *None* .

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"close_wallet"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **change_wallet_password**

Change a wallet password.

Alias:  *None* .

Inputs:

* *old_password*  - string; (Optional) Current wallet password, if defined.
* *new_password*  - string; (Optional) New wallet password, if not blank.

Outputs:  *None* .

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"change_wallet_password","params":{"old_password":"theCurrentSecretPassPhrase","new_password":"theNewSecretPassPhrase"}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
  }
}
```

### **is_multisig**

Check if a wallet is a multisig one.

Alias:  *None* .

Inputs:  *None* .

Outputs:

* *multisig*  - boolean; States if the wallet is multisig
* *ready*  - boolean;
* *threshold*  - unsigned int; Amount of signature needed to sign a transfer.
* *total*  - unsigned int; Total amount of signature in the multisig wallet.

Example for a non-multisig wallet:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"is_multisig"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "multisig": false,
    "ready": false,
    "threshold": 0,
    "total": 0
  }
}
```

Example for a multisig wallet:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"is_multisig"}' -H 'Content-Type: application/json'                  {
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "multisig": true,
    "ready": true,
    "threshold": 2,
    "total": 2
  }
}
```

### **prepare_multisig**

Prepare a wallet for multisig by generating a multisig string to share with peers.

Alias:  *None* .

Inputs:  *None* .

Outputs:

* *multisig_info*  - string; Multisig string to share with peers to create the multisig wallet.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"prepare_multisig"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "multisig_info": "MultisigV1BFdxQ653cQHB8wsj9WJQd2VdnjxK89g5M94dKPBNw22reJnyJYKrz6rJeXdjFwJ3Mz6n4qNQLd6eqUZKLiNzJFi3UPNVcTjtkG2aeSys9sYkvYYKMZ7chCxvoEXVgm74KKUcUu4V8xveCBFadFuZs8shnxBWHbcwFr5AziLr2mE7KHJT"
  }
}
```

### **make_multisig**

Make a wallet multisig by importing peers multisig string.

Alias:  *None* .

Inputs:

* *multisig_info*  - array of string; List of multisig string from peers.
* *threshold*  - unsigned int; Amount of signatures needed to sign a transfer. Must be less or equal than the amount of signature in  `multisig_info` .
* *password*  - string; Wallet password

Outputs:

* *address*  - string; multisig wallet address.
* *multisig_info*  - string; Multisig string to share with peers to create the multisig wallet (extra step for N-1/N wallets).

Example for 2/2 Multisig Wallet:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"make_multisig","params":{"multisig_info":["MultisigV1K4tGGe8QirZdHgTYoBZMumSug97fdDyM3Z63M3ZY5VXvAdoZvx16HJzPCP4Rp2ABMKUqLD2a74ugMdBfrVpKt4BwD8qCL5aZLrsYWoHiA7JJwDESuhsC3eF8QC9UMvxLXEMsMVh16o98GnKRYz1HCKXrAEWfcrCHyz3bLW1Pdggyowop"],"threshold":2}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "address": "etnk4YGjvutPsJvihejok8h63PadX47sgYmVttR9aCdbNeDgKWemFkzcaVHJYyZTnh8Po4UMwQmYp4HVGss7zyiQ3PEC7VVkc7",
    "multisig_info": ""
  }
}
```

Example for 2/3 Multisig Wallet:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"make_multisig","params":{"multisig_info":["MultisigV1MTVm4DZAdJw1PyVutpSy8Q4WisZBCFRAaZY7hhQnMwr5AZ4swzThyaSiVVQM5FHj1JQi3zPKhQ4k81BZkPSEaFjwRJtbfqfJcVvCqRnmBVcWVxhnihX5s8fZWBCjKrzT3CS95spG4dzNzJSUcjheAkLzCpVmSzGtgwMhAS3Vuz9Pas24","MultisigV1TEx58ycKCd6ADCfxF8hALpcdSRAkhZTi1bu4Rs6FdRC98EdB1LY7TAkMxasM55khFgcxrSXivaSr5FCMyJGHmojm1eE4HpGWPeZKv6cgCTThRzC4u6bkkSoFQdbzWN92yn1XEjuP2XQrGHk81mG2LMeyB51MWKJAVF99Pg9mX2BpmYFj"],"threshold":2}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "address": "etnk4YGjvutPsJvihejok8h63Pa1111111111111111111111111111111111111nh8Po4UMwQmYp4HVGss7zyiQ3PEC7VVkc7",
    "multisig_info": "MultisigxV18jCaYAQQvzCMUJaAWMCaAbAoHpAD6WPmYDmLtBtazD654E8RWkLaGRf29fJ3stU471MELKxwufNYeigP7LoE4tn2Sscwn5g7PyCfcBc1V4ffRHY3Kxqq6VocSCUTncpVeUskaDKuTAWtdB9VTBGW7iG1cd7Zm1dYgur3CiemkGjRUAj9bL3xTEuyaKGYSDhtpFZFp99HQX57EawhiRHk3qq4hjWX"
  }
}
```

### **export_multisig_info**

Export multisig info for other participants.

Alias:  *None* .

Inputs:  *None* .

Outputs:

* *info*  - string; Multisig info in hex format for other participants.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"export_multisig_info"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "info": "4d6f6e65726f206d756c7469736967206578706f72740105cf6442b09b75f5eca9d846771fe1a879c9a97ab0553ffbcec64b1148eb7832b51e7898d7944c41cee000415c5a98f4f80dc0efdae379a98805bb6eacae743446f6f421cd03e129eb5b27d6e3b73eb6929201507c1ae706c1a9ecd26ac8601932415b0b6f49cbbfd712e47d01262c59980a8f9a8be776f2bf585f1477a6df63d6364614d941ecfdcb6e958a390eb9aa7c87f056673d73bc7c5f0ab1f74a682e902e48a3322c0413bb7f6fd67404f13fb8e313f70a0ce568c853206751a334ef490068d3c8ca0e"
  }
}
```

### **import_multisig_info**

Import multisig info from other participants.

Alias:  *None* .

Inputs:

* *info*  - array of string; List of multisig info in hex format from other participants.

Outputs:

* *n_outputs*  - unsigned int; Number of outputs signed with those multisig info.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"import_multisig_info","params":{"info":["...multisig_info..."]}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "n_outputs": 35
  }
}
```

### **finalize_multisig**

Turn this wallet into a multisig wallet, extra step for N-1/N wallets.

Alias:  *None* .

Inputs:

* *multisig_info*  - array of string; List of multisig string from peers.
* *password*  - string; Wallet password

Outputs:

* *address*  - string; multisig wallet address.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"finalize_multisig","params":{"multisig_info":["MultisigxV1JNC6Ja2oBt5Sqea9LN2YEF7WYZCpHqr2EKvPG89Trf3X4E8RWkLaGRf29fJ3stU471MELKxwufNYeigP7LoE4tn2McPr4SbL9q15xNvZT5uwC9YRr7UwjXqSZHmTWN9PBuZEKVAQ4HPPyQciSCdNjgwsuFRBzrskMdMUwNMgKst1debYfm37i6PSzDoS2tk4kYTYj83kkAdR7kdshet1axQPd6HQ","MultisigxV1Unma7Ko4zdd8Ps3Af4oZwtj2JdWKzwNfP6s2G9ZvXhMoSscwn5g7PyCfcBc1V4ffRHY3Kxqq6VocSCUTncpVeUskMcPr4SbL9q15xNvZT5uwC9YRr7UwjXqSZHmTWN9PBuZE1LTpWxLoC3vPMSrqVVcjnmL9LYfdCZz3fECjNZbCEDq3PHDiUuY5jurQTcNoGhDTio5WM9xaAdim9YByiS5KyqF4"]}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "address": "etnk4YGjvutPsJvihejok8h63PadX47sgYmVttR9aCdbNeDgKWemFkzcaVHJYyZTnh8Po4UMwQmYp4HVGss7zyiQ3PEC7VVkc7"
  }
}
```

### **sign_multisig**

Sign a transaction in multisig.

Alias:  *None* .

Inputs:

* *tx_data_hex*  - string; Multisig transaction in hex format, as returned by  `transfer`  under  `multisig_txset` .

Outputs:

* *tx_data_hex*  - string; Multisig transaction in hex format.
* *tx_hash_list*  - array of string; List of transaction Hash.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"sign_multisig","params":{"tx_data_hex":"...multisig_txset..."}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "tx_data_hex": "...multisig_txset...",
    "tx_hash_list": ["4996091b61c1be112c1097fd5e97d8ff8b28f0e5e62e1137a8c831bacf034f2d"]
  }
}
```

### **submit_multisig**

Submit a signed multisig transaction.

Alias:  *None* .

Inputs:

* *tx_data_hex*  - string; Multisig transaction in hex format, as returned by  `sign_multisig`  under  `tx_data_hex` .

Outputs:

* *tx_hash_list*  - array of string; List of transaction Hash.

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"submit_multisig","params":{"tx_data_hex":"...tx_data_hex..."}}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "tx_hash_list": ["4996091b61c1be112c1097fd5e97d8ff8b28f0e5e62e1137a8c831bacf034f2d"]
  }
}
```

### **get_version**

Get RPC version Major & Minor integer-format, where Major is the first 16 bits and Minor the last 16 bits.

Alias:  *None* .

Inputs:  *None* .

Outputs:

* *version*  - unsigned int; RPC version, formatted with  `Major * 2^16 + Minor`  (Major encoded over the first 16 bits, and Minor over the last 16 bits).

Example:

```
$ curl -X POST http://localhost:40000/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"get_version"}' -H 'Content-Type: application/json'
{
  "id": "0",
  "jsonrpc": "2.0",
  "result": {
    "version": 65539
  }
}
```