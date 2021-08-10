<p  align="center">
	<img  src="https://i.imgur.com/dbpW6gm.png">
</p>

*  [About Electroneum](#About-Electroneum)
*  [Blockchain Technical Background](#Blockchain-Technical-Background)
	*  [The CryptoNote Protocol](#The-CryptoNote-Protocol)
		* [Double-Spending Proof](#Double-Spending-Proof)
	*  [Electroneum Blockchain](#Electroneum-Blockchain)
*  [Exchange Listing Guide](#Exchange-Listing-Guide)
	*  [Binaries](#Binaries)
		* [Relevant Binaries for Exchange Integration](#Relevant-Binaries-for-Exchange-Integration)
	* [Building Binaries from Source](#Building-Binaries-from-Source)
		* [Dependencies](#Dependencies)
		* [Install All Dependencies at Once on Ubuntu](#Install-All-Dependencies-at-Once-on-Ubuntu)
		* [Cloning the Repository](#Cloning-the-Repository)
		* [Build Instructions](#Build-Instructions)
	* [Executing Binaries](#Executing-Binaries)
		* [Start Electroneum Node](#Start-Electroneum-Node)
		* [Start Electroneum Wallet RPC](#Start-Electroneum-Wallet-RPC)
	* [Electroneum Wallet](#Electroneum-Wallet)
		* [Creating a new Wallet](#Creating-a-new-Wallet)
		* [Open Wallet](#Open-Wallet)
		* [Close Wallet](#Close-Wallet)
		* [Get Wallet Address](#Get-Wallet-Address)
		* [Get Wallet Balance](#Get-Wallet-Balance)
		* [Get Height](#Get-Height)
		* [Wallet Accounts](#Wallet-Accounts)
			* [Create Account](#Create-Account)
			* [Get Accounts](#Get-Accounts)
		* [Transfer Split](#Transfer-Split)
		* [Sweep All](#Sweep-All)
		* [Get Transfers](#Get-Transfers)
		* [Get Payments](#Get-Payments)
		* [Rescan Spent](#Rescan-Spent)
		* [Wallet Backup](#Wallet-Backup)
		* [Wallet Recovery](#Wallet-Recovery)
	*  [Best Practice Wallet Setup](#Best-Practice-Wallet-Setup)
		* [Optimizing Wallet Transaction Times](#Optimizing-Wallet-Transaction-Times)
		* [Cold Storage](#Cold-Storage)
		* [ETN Mobile App QR Code](#ETN-Mobile-App-QR-Code)
		* [Number of Confirmations](#Number-of-Confirmations)
		* [Identifying Deposits](#Identifying-Deposits)
			* [Accounts](#Accounts)
			* [Payment ID](#Payment-ID)
	*  [Troubleshooting](#Troubleshooting)
		*  [Problems when Syncing the Blockchain](#Problems-when-Syncing-the-Blockchain)
		*  [Restarting the Daemon](#Restarting-the-Daemon)
*  [Useful Electroneum Links](#Useful-Electroneum-Links)
	*  [Technical Links](#Technical-Links)
	*  [Social Media Links](#Social-Media-Links)
	*  [Electroneum Support Desk & FAQ's](#Electroneum-Support-Desk-&-FAQ's)

# About Electroneum
Electroneum has coined the phrase 'enablement cryptocurrency' as it is going to enable a new 3 trillion-dollar market (Price Waterhouse Coopers predicts this market to be as large as USD 3 trillion in their 2017 Global Fintech Report) by allowing over 350m smartphone users in the developing world to enter the digital market for the first time.

With immediate benefits to the mobile operators in those regions, including meaningful transactional cost savings, there are operators who could take us to well over 100 million users and many more operators in negotiation.

Our instant cryptocurrency payment system gives users instant payment notifications on their smartphone.

# Blockchain Technical Background
The Electroneum blockchain is a UTXO-model blockchain that was originally based on the Monero/CryptoNote open-source projects, but we have since removed all the privacy features. Starting from our V10 blockchain update, transactions and addresses are completely public in a very similar manner to that of Bitcoin's. That puts us in line with regulators around the globe.

## The CryptoNote Protocol
Electroneum blockchain was built on top of the CryptoNote Protocol, an open-source technology with advanced mathematical and cryptograph features to empower end-users with the best blockchain technology currently available.

### Double-Spending Proof
We have a database driven UTXO book-keeping, meaning that nobody is able to spend the same ETN twice

### Proof of Responsibility
Our block validation is permissioned, and those entities that are permissioned to validate blocks are contracted and monitored based on both their responsibility using the proceeds of their validations, and their good natured participation in our network. We have a number of charities who are using the proceeds to help developing nations and other people in need.

## Electroneum Blockchain
Although Electroneum was forked from Monero project on November of 2017, we have been implementing several unique and essential changes to the protocol, being steps closer to become the mass adopted mobile cryptocurrency.

Some of the major features we have introduced enables higher transaction throughput in the blockchain and KYC & AML (Know Your Customer & Anti-Money Laundry) processes through the removal of all of the privacy features present in Monero.

We recommend reading the [**Daemon RPC**](https://github.com/electroneum/electroneum/blob/develop/docs/daemon-rpc-documentation.md) & [**Wallet RPC**](https://github.com/electroneum/electroneum/blob/develop/docs/wallet-rpc-documentation.md) documentation before integrating Electroneum into your exchange.

# Exchange Listing Guide
This document is aimed at providing information and access to resources to ensure a seamless listing of Electroneum (ETN) on your exchange.

Electroneum has its own blockchain and therefore the management of Electroneum on your exchange is different from that of ERC20 tokens. This document is our current best practice guide on how to structure and manage Electroneum in the most efficient way for your exchange.

## Binaries
Electroneum is an open-source project. Binaries can be downloaded directly from our [**GitHub**](https://github.com/electroneum/electroneum/releases) or built by yourself on Ubuntu, Windows and MacOS.

### Relevant Binaries for Exchange Integration

| Binary | Description |
| --- | --- |
| electroneumd | This is the electroneum node software, responsible to sync and interact with the blockchain |
| electroneum-wallet-cli | CLI wallet client. Manages wallets through a command-line interface |
| electroneum-wallet-rpc | RPC wallet client. Manages wallets through a RPC-based interface |

## Building Binaries from Source
### Dependencies
The following table summarizes the tools and libraries required to build. A few of the libraries are also included in this repository (marked as "Vendored"). By default, the build uses the library installed on the system, and ignores the vendored sources. However, if no library is found installed on the system, then the vendored source will be built and used. The vendored sources are also used for tatically-linked builds because distribution packages often include only shared library binaries (`.so`) but not static library archives (`.a`).

| Dep | Min. version | Vendored | Debian/Ubuntu pkg | Arch pkg | Fedora | Optional | Purpose |
| ------------ | ------------- | -------- | ------------------ | ------------ | ----------------- | -------- | ---------------- |
| GCC | 4.7.3 | NO | `build-essential` | `base-devel` | `gcc` | NO | C/C++ compiler |
| CMake | 3.5 | NO | `cmake` | `cmake` | `cmake` | NO | Build system |
| pkg-config | any | NO | `pkg-config` | `base-devel` | `pkgconf` | NO | Compiler options |
| Boost | 1.58 | NO | `libboost-all-dev` | `boost` | `boost-devel` | NO | C++ libraries |
| OpenSSL | basically any | NO | `libssl-dev` | `openssl` | `openssl-devel` | NO | sha256 sum |
| libzmq | 3.0.0 | NO | `libzmq3-dev` | `zeromq` | `cppzmq-devel` | NO | ZeroMQ library |
| OpenPGM | ? | NO | `libpgm-dev` | `libpgm` | `openpgm-devel` | NO | For ZeroMQ |
| libnorm[2] | ? | NO | `libnorm-dev` | | ` | YES | For ZeroMQ |
| libunbound | 1.4.16 | YES | `libunbound-dev` | `unbound` | `unbound-devel` | NO | DNS resolver |
| libsodium | ? | NO | `libsodium-dev` | `libsodium` | `libsodium-devel` | NO | cryptography |
| libunwind | any | NO | `libunwind8-dev` | `libunwind` | `libunwind-devel` | YES | Stack traces |
| liblzma | any | NO | `liblzma-dev` | `xz` | `xz-devel` | YES | For libunwind |
| libreadline | 6.3.0 | NO | `libreadline6-dev` | `readline` | `readline-devel` | YES | Input editing |
| ldns | 1.6.17 | NO | `libldns-dev` | `ldns` | `ldns-devel` | YES | SSL toolkit |
| expat | 1.1 | NO | `libexpat1-dev` | `expat` | `expat-devel` | YES | XML parsing |
| GTest | 1.5 | YES | `libgtest-dev`[1] | `gtest` | `gtest-devel` | YES | Test suite |
| Doxygen | any | NO | `doxygen` | `doxygen` | `doxygen` | YES | Documentation |
| Graphviz | any | NO | `graphviz` | `graphviz` | `graphviz` | YES | Documentation |
| lrelease | ? | NO | `qttools5-dev-tools` | `qt5-tools` | `qt5-linguist` | YES | Translations |
| libhidapi | ? | NO | `libhidapi-dev` | `hidapi` | `hidapi-devel` | YES | Hardware wallet |
| libusb | ? | NO | `libusb-dev` | `libusb` | `libusb-devel` | YES | Hardware wallet |
| libprotobuf | ? | NO | `libprotobuf-dev` | `protobuf` | `protobuf-devel` | YES | Hardware wallet |
| protoc | ? | NO | `protobuf-compiler` | `protobuf` | `protobuf-compiler` | YES | Hardware wallet |

[1] On Debian/Ubuntu `libgtest-dev` only includes sources and headers. You must build the library binary manually. This can be done with the following command ```sudo apt-get install libgtest-dev && cd /usr/src/gtest && sudo cmake . && sudo make && sudo mv libg* /usr/lib/ ```

[2] libnorm-dev is needed if your zmq library was built with libnorm, and not needed otherwise.

### Install All Dependencies at Once on Ubuntu:
``` sudo apt update && sudo apt install build-essential cmake pkg-config libboost-all-dev libssl-dev libzmq3-dev libunbound-dev libsodium-dev libunwind8-dev liblzma-dev libreadline6-dev libldns-dev libexpat1-dev doxygen graphviz libpgm-dev libusb-dev```

### Cloning the Repository
Clone recursively to pull-in needed submodule(s):
`$ git clone --recursive https://github.com/electroneum/electroneum`

If you already have a repo cloned, initialize and update:
`$ cd electroneum && git submodule init && git submodule update`

### Build Instructions
Electroneum uses the CMake build system and a top-level Makefile that invokes cmake commands as needed.

* Install the dependencies
* Change to the root of the source code directory, change to the most recent release branch, and build:
```bash
cd electroneum
make
```
*Optional*: If your machine has several cores and enough memory, enable parallel build by running `make -j<number of threads>` instead of `make`. For this to be worthwhile, the machine should have one core and about 2GB of RAM
available per thread.
* The resulting executables can be found in `build/release/bin`
* Add `PATH="$PATH:$HOME/electroneum/build/release/bin"` to `.profile`

## Executing Binaries
### Start Electroneum Node

* Run electroneum node with `electroneumd`
	*Optional*: Use `electroneumd --help` to see a list of supported startup arguments.
	*Optional*: Use `electroneumd --detach` to startup the electroneum node in detached mode.

* The electroneum node will start synchronizing with network. The blockchain data will be saved by default at `~/.electroneum`, but can be changed with `--data-dir <path>` startup argument.
	*Optional*: We recommend using the startup argument `--block-sync-size 10` to speed up the sync process.

Please refer to [Daemon RPC Documentation](https://github.com/electroneum/electroneum/blob/develop/docs/daemon-rpc-documentation.md) for more information.

### Start Electroneum Wallet RPC

* Run electroneum wallet rpc with `nohup ./electroneum-wallet-rpc`

List of Parameters:
| Parameter Name | Description |
| --- | --- |
| rpc-bind-ip (arg) | Specify ip to bind rpc server |
| rpc-bind-port (arg) | Sets bind port for server |
| confirm-external-bind | Confirm rpc-bind-ip value is NOT a loopback (local) IP |
| disable-rpc-login | Disable HTTP authentication for RPC |
| wallet-dir (arg) | Directory for newly created wallets |

Example:
```
nohup ./electroneum-wallet-rpc --rpc-bind-ip 0.0.0.0 --rpc-bind-port 28071 --confirm-external-bind --disable-rpc-login --wallet-dir "/home/wallets" & echo $1;
```

Please refer to [Wallet RPC Documentation](https://github.com/electroneum/electroneum/blob/develop/docs/wallet-rpc-documentation.md) for more information.

**All requests to the Electroneum Wallet RPC needs to be sent to the following endpoint**
```
POST: http://<wallet_rpc_address>:<port>/json_rpc
```

## Electroneum Wallet
In this section, we list relevant RPC commands for exchange integration. Any wallet interaction is done through REST Requests towards the Electroneum Wallet RPC.

### Creating a new Wallet
Create a new wallet. You need to have set the argument `–wallet-dir` when launching Electroneum Wallet RPC to make this work.

The wallet will be automatically opened in Electroneum Wallet RPC after its creation process. Other RPC methods such as  `query_key`  and  `getaddress`  (demonstrated above) will be executed against this current opened wallet.

> Request ([create_wallet](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#create_wallet))
```
{
	"jsonrpc":"2.0",
	"id":"0",
	"method":"create_wallet",
	"params":{
		"filename":"<wallet_name>",
		"password":"<wallet_password>",
		"language":"English"
	}
}
```

### Open Wallet
Open a wallet. You need to have set the argument "–wallet-dir" when launching Electroneum Wallet RPC to make this work.

> Request ([open_wallet](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#open_wallet))
```
{
	"jsonrpc": "2.0",
	"id": "0",
	"method": "open_wallet",
	"params": {
		"filename": "<wallet_name>",
		"password": "<wallet_password>"
	}
}
```

### Close Wallet
Close the currently opened wallet, after trying to save it.

> Request ([close_wallet](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#close_wallet))
```
{
	"jsonrpc": "2.0",
	"id": "0",
	"method": "close_wallet"
}
```

### Get Wallet Address
Return the wallet's addresses for an account. Optionally filter for specific set of accounts.

> Request ([get_address](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#get_address))
```
{
	"jsonrpc":"2.0",
	"id":"0",
	"method":"get_address"
}
```

### Get Wallet Balance
Return the wallet's balance.

> Request ([get_balance](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#get_balance))
```
{
	"jsonrpc":"2.0",
	"id":"0",
	"method":"get_balance"
}
```
> Response
```
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
		}],
		"unlocked_balance": 150000
}
```

### Get Height
Returns the wallet's current block height.

> Request ([get_height](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#get_height))
```
{
	"jsonrpc": "2.0",
	"id": "0",
	"method": "get_height"
}
```
> Response
```
{
	"id": "0",
	"jsonrpc": "2.0",
	"result": {
		"height": 145545
	}
}
```


### Wallet Accounts
You have the ability to create _accounts_ inside each wallet. If your wallet was like a safe, accounts work as boxes inside to keep things tidy and ordered. Each box comes with its own address, so funds sent to each one appear under their respective account.

For exchange integration, wallet accounts can be used as a deposit address for each user. In this case, the `account_index` should be linked to the user in your exchange database.

#### Create Account
Create a new account with an optional label.
> Request ([create_account](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#create_account))
```
{
	"jsonrpc": "2.0",
	"id": "0",
	"method": "create_account"
}
```
> Response
```
{
	"id": "0",
	"jsonrpc": "2.0",
	"result": {
		"account_index": 1,
		"address": "etnkfduwUbTcLs9LtSF2ULduLV44wmyKr9JJgDGt26aUe82LxYFcJzAXWWLdj5jeKdhNXdo9wPNzGcW3s13yoBw65zr23u671f"
	}
}
```

#### Get Accounts
Get all accounts for a wallet. Optionally filter accounts by index or tag.

> Request ([get_accounts](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#get_accounts))
```
{
	"jsonrpc": "2.0",
	"id": "0",
	"method": "get_accounts",
	"params": {
		"account_index": "1"
	}
}
```
> Response
```
{
	"id": "0",
	"jsonrpc": "2.0",
	"result": {
		"account_major_offset": 0,
		"subaddress_accounts": [
		{
			"account_index": 1,
			"balance": 0,
			"base_address": "etnkfduwUbTcLs9LtSF2ULduLV44wmyKr9JJgDGt26aUe82LxYFcJzAXWWLdj5jeKdhNXdo9wPNzGcW3s13yoBw65zr23u671f",
			"label": "",
			"tag": "",
			"unlocked_balance": 0
		}],
		"total_balance": 0,
		"total_unlocked_balance": 0
	}
}
```

### Transfer Split
Send ETN to a number of recipients.

We recommend the usage of **transfer_split** over the **transfer** due to it being a legacy command.

If your wallet has multiple accounts, you need to specify the `account_index` (defaults to 0) from which ETN will be spent from.


> Request ([transfer_split](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#transfer_split))
```
{
	"jsonrpc": "2.0",
	"id": "0",
	"method": "transfer_split",
	"params": {
		"account_index": 0
		"destinations": [
		{
			"amount": 58000,
			"address": "etnkWaJUHsB6JWCNC1X8AMVms5ujU281mEKSNHuK7Ah4ETNeDfbTdbCNmxtjxunGtfiXUrxJVczRzSSTYxA38DDi7fN25Yq4eY"
		}],
		"get_tx_key": true,
		"new_algorithm": true
	}
}
```
> Response
```
{
	"id": "0",
	"jsonrpc": "2.0",
	"result": {
		"amount_list": [58000],
		"fee_list": [10],
		"multisig_txset": "",
		"tx_hash_list": ["4adcdc1af3f665770cdf8fb7a380887cd07ac53c2b771bd18df5ca375d5e7540"],
		"tx_key_list": ["5b455c0f97168be652a2c03c5c68a064bb84cdae4ddef01b5c48d73a0bbb27075fb714f2ca19ea6c8ff592417e606addea6deb1d6530e2969f75681ffcbfc4075677b94a8c9197963ae38fa6f543ee68f0a4c4bbda4c453f39538f00b28e980ea08509730b51c004960101ba2f3adbc34cbbdff0d5af9dba061b523090debd06"],
		"unsigned_txset": ""
	}
}
```

### Sweep All

Sends all unlocked balance to an address.

> Request ([sweep_all](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#sweep_all))
```
{
	"jsonrpc": "2.0",
	"id": "0",
	"method": "sweep_all",
	"params": {
		"address": "etnjw64JfkwKapwWyCenbu8Zj4odo4p6dDMJo6Dozdx3cwtbMKzgw4ScjVaWi8psbGdvdUKz4PJ6D7rU2bsLKhJE4EEuob3K1X",
		"unlock_time": 0,
		"get_tx_keys": true
	}
}
```
> Response
```
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

### Get Transfers
Returns a list of transfers.

You can get transfers for all accounts in this wallet or for a particular `account_index`. It is possible to filter by incoming, outgoing, pending, pool and failed transactions.

> Request ([get_transfers](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#get_transfers))
```
{
	"jsonrpc": "2.0",
	"id": "0",
	"method": "get_transfers",
	"params": {
		"account_index": 0,
		"pool": false,
		"in": true,
		"out": false,
		"migration": false,
		"pending": false
	}
}
```
> Response
```
{
	"id": "0",
	"jsonrpc": "2.0",
	"result": {
		"in": [
		{
			"address": "etnjw64JfkwKapwWyCenbu8Zj4odo4p6dDMJo6Dozdx3cwtbMKzgw4ScjVaWi8psbGdvdUKz4PJ6D7rU2bsLKhJE4EEuob3K1X",
			"amount": 10000,
			"confirmations": 1,
			"double_spend_seen": false,
			"fee": 0,
			"height": 1071196,
			"nonexistent_utxo_seen": false,
			"note": "",
			"payment_id": "0000000000000000",
			"subaddr_index": {
			"major": 0,
			"minor": 0
		},
		"subaddr_indices": [
			{
				"major": 0,
				"minor": 0
			}
		],
		"suggested_confirmations_threshold": 5,
		"timestamp": 1628486571,
		"txid": "fb80573819d20f1b13212fc407b7f8b881047742e9ebc8d7c8256c6d24efb2da",
		"type": "in",
		"unlock_time": 1071201
		}]
	}
}
```

### Get Payments
Get a list of incoming payments using a given payment id.

Electroneum wallet supports sending payments with a Payment ID (MEMO) to identify payments to a specific user. If you choose to use the Payment ID to identify user deposits rather than using Accounts, you should use this command to get all transfers for a given Payment ID.

> Request ([get_payments](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#get_payments))
```
{
	"jsonrpc": "2.0",
	"id": "0",
	"method": "get_payments",
	"params": {
		"payment_id": "4279257e0a20608e25dba8744949c9e1caff4fcdafc7d5362ecf14225f3d9030"
	}
}
```
> Response
```
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

### Rescan Spent
Rescan the blockchain for spent outputs.

> Request ([rescan_spent](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#rescan_spent))
```
{
	"jsonrpc":"2.0",
	"id":"0",
	"method":"rescan_spent"
}
```

### Wallet Backup
Wallet backup consists in saving a particular private key, known as private spend key. This key is mandatory for wallet recovery and should be saved offline for best security.

> Request ([query_key](https://github.com/electroneum/electroneum/blob/master/docs/wallet-rpc-documentation.md#query_key))
```
{
	"jsonrpc":"2.0",
	"id":"0",
	"method":"query_key",
	"params":{
		"key_type":"spend_key"
	}
}
```
The key retrieved in the response should be stored safely for wallet recovery. Anyone with access to this key can restore, view and spend ETN from this wallet.

### Wallet Recovery
The wallet recovery process is done through the **Electroneum Wallet CLI** software.

- Run the Wallet CLI with `electroneum-wallet-cli --generate-from-spend-key <wallet_file>`
- Specify the **private spend key** when asked in the CLI
- Specify & confirm a **new password** for the wallet
- Specify the **restore block height** or **restore date**. This should be the block height of the first transaction in this wallet, or its approximate date.
- Save the wallet with `save` and exit with `exit`
- Copy the wallet file (`<wallet_file>`) to the Electroneum Wallet RPC wallets directory
- Open the wallet in the Electroneum Wallet RPC


## Best Practice Wallet Setup

We recommend having at least one withdrawal wallet and one deposit wallet. Our best practice recommendation to ensure that your deposit and withdrawal wallets run as efficiently as possible is to rotate on a monthly basis, as illustrated below in Figure 1:

<p  align="center">
<img  src="https://imgur.com/QfcLAXh.png">
</p>

You should periodically rotate your **WITHDRAW WALLET 1** by ```sweep_all``` into your second withdraw wallet (**WITHDRAW WALLET 2**). This allows you to open wallet 2 from the sweep block height (minus 720 blocks) and use with virtually no delay.

After another month the rotation should return to **WITHDRAW WALLET 1** by using ```sweep_all``` into **WALLET 1** from **WITHDRAW WALLET 2**. The actual frequency of rotation can be amended by you based on the number of transactions you perform and the response time.

### Optimizing Wallet Transaction Times

Over time, the transaction speed of highly active **DEPOSIT WALLET** and **WITHDRAWAL WALLETS** wallets will start to decrease. This is due to the UTXO model of outputs used for the blockchain and wallet, which accumulates outputs in the wallet cache over time, thereby increasing the wallet size and the extent of balance fragmentation, which can degrade performance in a number of ways.

In order to negate this and optimize your wallet transaction time, we recommend the following housekeeping process to be run on a monthly basis (this shouldn't take any longer than 1-2 hours per month). You can view this process as being akin to defragmenting a hard drive.

This process can be done from **WITHDRAWAL WALLET 1** to **WITHDRAWAL WALLET 2** (and vice-versa) and/or from **DEPOSIT WALLET** to **COLD STORAGE WALLET**, significantly improving performance leading to virtually no delay in response.

**The procedure for this is as follows:**

- Start the Electroneum Wallet RPC (see [Start Wallet RPC](#Start-Electroneum-Wallet-RPC))
- Open the **WITHDRAWAL WALLET 1** with [open_wallet](#Open-Wallet)
- Sweep all from the **WITHDRAWAL WALLET 1**  to **WITHDRAWAL WALLET 2**  with [sweep_all](#Sweep-All)
- Get the **WITHDRAWAL WALLET 1** block height with [get_height](#Get-Height)
- Close **WITHDRAWAL WALLET 1** with [close_wallet](#Close-Wallet)
- Restore **WITHDRAWAL WALLET 2** (see [Wallet Recovery](#Wallet-Recovery)), using the block height from previous step
- Open the **WITHDRAWAL WALLET 2** with [open_wallet](#Open-Wallet)
- Execute [rescan_spent](#Rescan-Spent) command to prevent any errors


### Cold Storage

It is considered best practice to ensure you are keeping a percentage of your ETN in cold storage in a separate wallet or wallets. Again, depending on the transaction numbers you could rotate these wallets for increased wallet efficiency.

### ETN Mobile App QR Code

The ETN Mobile app can scan QR Codes to pre-populate fields for sending ETN. If you generate a QR code with the following information, it can be imported directly.

```
electroneum:WALLET_ADDRESS?tx_payment_id=YOUR_PAYMENT_ID
```

**Example:**
```
electroneum:etn12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345?tx_payment_id=a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9a9
```

### Number of Confirmations

NUMBER OF CONFIRMATIONS: **5 BLOCKS**

When a user transfer funds to an address, the transaction sits in the mempool until it gets processed into a block. The first few blocks are susceptible to "blockchain reorganizations" and, although block reorganizations being a natural behavior of the blockchain, it may cause a rollback in transactions contained in a given reorganized block(s). In other words, the transactions included in a recent block may get back to mempool or even back to the sender.

To assure that the funds sent from a user to your exchange is confirmed by the blockchain and an eventual rollback won't happen, your system needs to wait for a few more blocks to come. This "wait" is technically the number of confirmations (measured in blocks) you need before unlocking the funds to the user.

We recommend that you use **5 blocks as number of confirmations** required to optimal security of funds.

### Identifying Deposits

You can identify deposits by one of the two currently available methods.

#### Accounts
The wallet has the ability to create _accounts_ inside each wallet. If your wallet was like a safe, accounts work as boxes inside to keep things tidy and ordered. Each box comes with its own address, so funds sent to each one appear under their respective account.

When using [Get Wallet Balance](#Get-Wallet-Balance) in Electroneum Wallet RPC, you can see the total balance of all its accounts, but you can filter by _account_index_ if you want see the individual balances as well. 

The _account_index_ should be used to link your user to one of the wallet's account in a 1-to-1 relationship. Accounts can be created with [Create Account](#Create-Account) and retrieved with [Get Accounts](#Get-Accounts) Wallet RPC commands.

Transfers for a specific Wallet Account can be retrieved with [Get Transfers](#Get-Transfers) by specifying the _account_index_.

It's important to note that your wallet can sign transactions for all accounts within this particular wallet, since the address for each account is derived from the same private keys. This can be achieved with [Transfer Split](#Transfer-Split) by providing the _account_index_ attribute. However, you can send only one transaction per account at a time, so sending the funds to your Cold Storage would require multiple transactions (one for each account).

#### Payment ID
This is the most used method of identifying deposits by exchanges that supports ETN, it is simple, effective and most exchanges already use a similar method (MEMO).

In this case, the deposit wallet address should be shared among all exchange users and deposits should require an unique / per-user Payment ID (MEMO) to identify deposits to a particular user.

A Payment IDs are 16 or 64 hexadecimal character strings and can be created by your system.

Transactions that contains a particular Payment ID can be retrieved with [Get Payments](#Get-Payments) Wallet RPC command.

## Troubleshooting

### Problems when Syncing the Blockchain

If your Electroneum daemon is stuck syncing the blockchain, please follow the steps bellow.

#### 1. Make sure you're running Electroneum's latest software.

Binaries can be found at: [https://github.com/electroneum/electroneum/releases](https://github.com/electroneum/electroneum/releases)

#### 2. Delete the p2pstate.bin file

Make sure Electroneum daemon is not running by typing "exit" in electroneumd. After closing Electroneum's daemon and wallets, you can safely delete the p2pstate.bin file located at:

```
$HOME/.electroneum
```

This directory is hidden by default. On Linux you can use CTRL + H to unhide directories (note that this might not work on all Linux distributions).

After you've deleted p2pstate.bin you can restart electroneumd. If this did not resolve your issue, please read further and make sure electroneumd is closed again.

#### 3. Change the block sync size

We're going to add the following flag:
```
--block-sync-size 10
```

This will change the batch of fetched blocks from 100 to 10.

You can add the flag as follows:
```
./electroneumd --block-sync-size 10
```

If your blockchain is still stuck after adding this flag, please browse again to ```$HOME/.electroneum```, open electroneum.log and check it for any errors. If you see any of these errors (or similar to them), your blockchain is most likely corrupted.

```Error opening database: Failed to query m_blocks: MDB_BAD_TXN: Transaction must abort, has a child, or is invalid```

```Failed to parse and validate tx from blob```

You can use the ```--db-salvage``` flag to the daemon, sometimes it can recover from a corrupted data base. If the daemon is still struggling to sync, you may need to resync the blockchain from scratch.

This is done by deleting data.mdb from ```$HOME/.electroneum/lmdb```, which will trigger the resync.

### Restarting the Daemon

To force restart the daemon you should use the following command:

sudo systemctl restart electroneumd

# Useful Electroneum Links

## Technical Links

*  [GitHub Repository](https://github.com/electroneum/electroneum)
*  [Block explorer](https://blockexplorer.electroneum.com)
*  [Daemon RPC Documentation](https://github.com/electroneum/electroneum/blob/develop/docs/daemon-rpc-documentation.md)
*  [Wallet RPC Documentation](https://github.com/electroneum/electroneum/blob/develop/docs/wallet-rpc-documentation.md)

## Social Media Links

Official social media channels managed in office by our PR/social media team

*  [Blog](https://electroneum.com/blog/)
*  [Twitter](https://twitter.com/electroneum)
*  [Facebook](https://www.facebook.com/electroneum/)
*  [Electroneum Community Forum](https://community.electroneum.com/)

## Electroneum Support Desk & FAQ's

Support site: [https://electroneum.com/support/](https://electroneum.com/support/)