<p align="center">
    <img src="https://i.imgur.com/dbpW6gm.png">
</p>

* [Introduction](#Introduction)
  * [About Electroneum](#About-Electroneum)
  * [The Road to Mass Adoption](#The-Road-to-Mass-Adoption)
  * [Instant Cryptocurrency Payments](#Instant-Cryptocurrency-Payments)
  * [USD 3 Trillion Opportunity](#USD-3-Trillion-Opportunity)
  * [Exceptional Viral Growth](#Exceptional-Viral-Growth)
  * [Blockchain Technical Background](#Blockchain-Technical-Background)
  * [The CryptoNote Protocol](#The-CryptoNote-Protocol)
  * [Electroneum Blockchain](#Electroneum-Blockchain)
* [Exchange Listing Information](#Exchange-Listing-Information)
  * [Best practice set up for listing Electroneum on your exchange](#Best-practice-set-up-for-listing-Electroneum-on-your-exchange)
  * [Wallet creation](#Wallet-creation)
  * [Optimizing wallet transaction times](#Optimizing-wallet-transaction-times)
  * [Cold Storage](#Cold-Storage)
  * [Integrated Addresses](#Integrated-Addresses)
  * [Transaction Best Practice](#Transaction-Best-Practice)
  * [ETN Mobile App QR Code](#ETN-Mobile-App-QR-Code)
  * [Number of Confirmations](#Number-of-Confirmations)
* [Troubleshooting](#Troubleshooting)
  * [Problems when syncing the blockchain](#Problems-when-syncing-the-blockchain)
  * [Restarting the Daemon](#Restarting-the-Daemon)
* [Useful Electroneum Links](#Useful-Electroneum-Links)
  * [Technical Links](#Technical-Links)
  * [Social Media Links](#Social-Media-Links)
  * [Electroneum Support Desk & FAQ's](#Electroneum-Support-Desk-&-FAQ's)

## Introduction

Enabling the Electroneum Ecosystem

### About Electroneum

The Mobile Cryptocurrency

Electroneum has coined the phrase 'enablement currency' as it is going to enable a new 3 trillion-dollar market (Price Waterhouse Coopers predicts this market to be as large as USD 3 trillion in their 2017 Global Fintech Report) by allowing over 350m smartphone users in the developing world to enter the digital market for the first time.

With immediate benefits to the mobile operators in those regions, including meaningful transactional cost savings, there are operators who could take us to well over 100 million users and many more operators in negotiation.

Our unique mobile miner puts a small amount of ETN into the hands of users every day, providing market liquidity and driving viral growth. With over 1.6 million registered users, we have a faster viral growth than both Facebook and Twitter over the same time period.

Our Android app is already fully operational in 20 languages.

Our instant cryptocurrency payment system is planned to go live for users and vendors by the end of the third quarter of 2018.



### The Road to Mass Adoption

It's common for cryptocurrencies to talk about seeking mass adoption. It's far less common for them to make tangible progress towards that goal.

<p align="center">
    <img src="https://imgur.com/X7Q56YO.png">
</p>


### Instant Cryptocurrency Payments

We have a global patent pending on a technology that allows instant cryptocurrency payments. Instant payments will initially be in ETN but our patent also covers Bitcoin and other cryptocurrencies.

The ability to SPEND ETN instantly is only half the equation. We are developing an API that will allow easy integration to ACCEPT ETN instantly. This means it will be simple to allow an ecommerce system to give instant check out, and even allow easy integration into ePOS systems (electronic point of sales systems or tills).



### USD 3 Trillion Opportunity

By giving access to the 'gig economy' for the first time to over 350 million unbanked smartphone users in the developing world we are giving them access to the vast resources of the Western economies. We are not providing banking facilities but making digital earnings accessible for these users as they embrace the digital payment sector.

At the same time, the Western economies gain access to a community of competitively priced, millennial entrepreneurs, who can grow and learn a new digital skillset, in the knowledge that they can sell their new skills via a global digital payment method.

Price Waterhouse Coopers predicts this market to be as large as USD 3 trillion in their 2017 Global Fintech Report



### Exceptional Viral Growth

Our economic model enables us to inject small amounts of ETN into the hands of the smartphone user. This makes it easier for the user to try making real world digital payments, often for their first time.

Once users see how easy Electroneum is to use, they become part of the community and help drive us forward through recommendation.

We have already attracted over 1.6m registered users by viral growth even while our instant payment API system is still in BETA.

## Blockchain Technical Background

Electroneum blockchain is based on the Monero/CryptoNote open-source projects.

### The CryptoNote Protocol

Electroneum blockchain was built on top of the CryptoNote Protocol, an open-source technology with advanced mathematical and cryptograph features to empower end-users with the best blockchain technology currently available.

**Untraceable Payments**

CryptoNote provides users with a completely anonymous payment scheme. CryptoNote implements the ring signature technology which allows you to sign a message on behalf of a group. The signature only proves the message was created by someone from the group, but all the possible signers are indistinguishable from each other.

Source: [cryptonote.org](https://cryptonote.org/inside#untraceable-payments)


**Unlikable Transactions**

Even if outgoing transactions are untraceable, everyone may still be able to see the payments you have received and thus determine your income. However, by using a variation of the Diffie-Hellman exchange protocol, a receiver has multiple unique one-time addresses derived from his single public key. After funds are sent to these addresses they can only be redeemed by the receiver; and it would be impossible to cross-link these payments.

Source: [cryptonote.org](https://cryptonote.org/inside#unlinkable-transaction)


**Double-Spending Proof**

Nobody is able to spend the same money twice — even if all his signatures are anonymous. Every signature contains a key image — a kind of fingerprint of the secret key. It is based on a one-way cryptographic function; this implies that given only the key image it is impossible to restore the corresponding secret key. These key images are used to prevent double-spending.

Source: [cryptonote.org](https://cryptonote.org/inside#double-spend-proof)


**Blockchain Analysis Resistance**

Non-repeating one-time addresses and mixed keys in ring signatures make the whole blockchain resistant to analysis. Each future transaction will only increase the entropy and create additional obstacles for an analyst.

Source: [cryptonote.org](https://cryptonote.org/inside#blockchain-analysis-resistance)


**Egalitarian Proof of Work**

The proof of work mechanism acts as a voting system. Thus, it is crucial that during the voting process all the participants have equal voting privileges. CryptoNote brings this equality with its egalitarian proof of work, utilizing built-in CPU instructions, which are very hard and too expensive to implement in special purpose devices, but perfectly suitable for ordinary PCs.

Source: [cryptonote.org](https://cryptonote.org/inside#equal-proof-of-work)


**Adaptive Parameters**

A decentralized payment system must not depend on a single person's decisions, even if this person is a developer. CryptoNote has no hard-coded constants; magic numbers in the code are designed to be re-calculated based on the previous state of the network. Thus, they always change adaptively and independently, allowing the network to develop on its own.

Source: [cryptonote.org](https://cryptonote.org/inside#adaptive-limits)


### Electroneum Blockchain

Although Electroneum was forked from Monero project on November of 2017, we have been implementing several unique and essential changes to the protocol, being steps closer to become the mass adopted mobile cryptocurrency.

Some of the major features we have introduced enables higher transaction throughput in the blockchain and KYC & AML (Know Your Customer & Anti-Money Laundry) processes through the removal of some privacy features present in Monero.

We recommend reading the [**Daemon RPC**](https://github.com/electroneum/electroneum/blob/develop/docs/daemon-rpc-documentation.md) & [**Wallet RPC**](https://github.com/electroneum/electroneum/blob/develop/docs/wallet-rpc-documentation.md) documentation before integrating Electroneum into your exchange.


## Exchange Listing Information

This document is aimed at providing information and access to resources to ensure a seamless listing of Electroneum (ETN) on your exchange.

### Best practice set up for listing Electroneum on your exchange

Electroneum is a hard fork of Monero and therefore the management of Electroneum on your exchange is different from that of ERC20 tokens. If you have already listed Monero you can repeat that same process for listing Electroneum. This document is our current best practice guide on how to structure and manage Electroneum in the most efficient way for your exchange.

We recommend having at least one withdrawal wallet and one deposit wallet. Our best practice recommendation to ensure that your deposit and withdrawal wallets run as efficiently as possible is to rotate on a monthly basis, as illustrated below in Figure 1:
<p align="center">
    <img src="https://imgur.com/QfcLAXh.png">
</p>

You should periodically rotate your **WITHDRAW** wallet by ```sweep_all``` into your second withdraw wallet (**WALLET 3**). This allows you to open wallet 3 from the sweep block height (minus 720 blocks) and use with virtually no delay.

After another month the rotation should return to WALLET 2 by using ```sweep_all``` into **WALLET 2** from **WALLET 3**. The actual frequency of rotation can be amended by you based on the number of transactions you perform and the response time.

## Wallet creation

Our minimum best practice for wallet structure and management would be to create three wallets ( **DEPOSIT WALLET** , **WITHDRAWAL WALLET 1** , **WITHDRAWAL WALLET 2** ). Some exchanges use an additional wallet for offline cold storage for added security (as illustrated in Figure 1). The following commands should be executed in a Linux terminal.

**Process for the creation of the three required wallets**

**1. Start Electroneum Wallet RPC**

Base Command:

```
nohup ./electroneum-wallet-
```

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



**2. Create Wallet**

The wallet creation is done through REST Requests towards the Electroneum Wallet RPC.

The wallet will be automatically opened in Electroneum Wallet RPC after its creation process. Other RPC methods such as ```query_key``` and ```getaddress``` (demonstrated above) will be executed against this current opened wallet.
```
Operation: POST
URL: http://<wallet_rpc_address>:<port>
Method: json_rpc
HTTP Header: Content-Type:application/json

Request Body:
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

**It is possible to make REST Request on Linux by using the "curl" command.**

Using curl:
```
curl -X POST http://localhost:28071/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"create_wallet","params":{"filename":"deposit01","password":"depositpassword","language":"English"}}' -H 'Content-Type:application/json'
```



**3. Get View Key of previously created Wallet**

Get view key is also done through REST Request towards Electroneum Wallet RPC.
```
Operation: POST
URL: http://<wallet_rpc_address>:<port>
Method: json_rpc
HTTP Header: Content-Type:application/json
Request Body:
{
   "jsonrpc":"2.0",
   "id":"0",
   "method":"query_key",
   "params":{
      "key_type":"view_key"
   }
}

Response Body:
{
   "id":"0",
   "jsonrpc":"2.0",
   "result":{
      "key":"7c330c2133c65f975ac151442842b2295efce1dc8ee6dfc5683aa9Edeposit01"
   }
}
```

Using curl:
```
curl -X POST http://localhost:28071/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"query_key","params":{"key_type":"view_key"}}' -H 'Content-Type: application/json'
```

**4. Get Spend Key of previously created Wallet**

Get spend key is also done through REST Request towards Electroneum Wallet RPC.
```
Operation: POST
URL: http://<wallet_rpc_address>:<port>
Method: json_rpc
HTTP Header: Content-Type:application/json

Request Body:
{
   "jsonrpc":"2.0",
   "id":"0",
   "method":"query_key",
   "params":{
      "key_type":"spend_key"
   }
}

Response Body:
{
   "id":"0",
   "jsonrpc":"2.0",
   "result":{
      "key":"26206133b8174993fe27fa8240a716b404ec3c5697bfc04ccc60138deposit01"
   }
}
```

Using curl:
```
curl -X POST http://localhost:28071/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"query_key","params":{"key_type":"spend_key"}}' -H 'Content-Type: application/json'
```



**5. Get ETN Address of previously created Wallet**

Get ETN Address is also done through REST Request towards Electroneum Wallet RPC.
```
Operation: POST
URL: http://<wallet_rpc_address>:<port>
Method: json_rpc
HTTP Header: Content-Type:application/json

Request Body:
{
   "jsonrpc":"2.0",
   "id":"0",
   "method":"getaddress"
}

Response Body:
{
   "id":"0",
   "jsonrpc":"2.0",
   "result":{
"address":"etnjyug1v6i6y2BaNP1tQH4XkCNVFFBGP37v5rHkagQdZr84Eg7kKpWeYkcFYnfwNjeg9kXogoYupUk2QZwG5akKadeposit01"
   }
}
```

Using curl:
```
curl -X POST http://localhost:28071/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"getaddress"}' -H 'Content-Type:application/json'
```

Repeat the steps 2 to 5 at least two more times for the creation of the **WITHDRAW WALLETS**. Once all wallets have been created you can stop the RPC daemon by using the command:
```
curl -X POST http://localhost:28071/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"stop_wallet"}' -H 'Content-Type: application/json'
```


### Optimizing wallet transaction times

Over time the transaction speed of your **DEPOSIT WALLET** and **WITHDRAWAL WALLETS** will start to slow down. It's important to note that the reason the wallet slows down over time is that as a privacy coin there is a lot more going on mathematically than with Bitcoin or Ethereum. There is nothing wrong with the wallet, it is just having to process too much information.

To optimize your wallet transaction time, we recommend the following process being run on a monthly basis (this shouldn't take any longer than 1-2 hours per month).

This process can be done from **WITHDRAWAL WALLET 1** to **WITHDRAWAL WALLET 2** (and vice-versa) and/or from **DEPOSIT WALLET** to **COLD STORAGE WALLET**, significantly improving performance leading to virtually no delay in response.

The procedure for this is as follows:

**1. Start the Electroneum Wallet RPC**
```
nohup ./electroneum-wallet-rpc --rpc-bind-ip 0.0.0.0 --rpc-bind-port 28071 --confirm-external-bind --disable-rpc-login --wallet-dir "/home/wallets" & echo $1;
```



**2. Open the Withdrawal Wallet 1**
```
Operation: POST
URL: http://<wallet_rpc_address>:<port>
Method: json_rpc
HTTP Header: Content-Type:application/json

Request Body:
{
   "jsonrpc":"2.0",
   "id":"0",
   "method":"open_wallet",
   "params":{
      "filename":"<withdrawal_wallet_1>",
      "password":"<password>"
   }
}
```

Using curl:
```
curl -X POST http://localhost: 28071/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"open_wallet","params":{"filename":"mytestwallet","password":"mytestpassword"}}' -H 'Content-Type: application/json'
```

**3. Sweep all from the Withdrawal Wallet 1 to Withdrawal Wallet 2**
```
Operation: POST
URL: http://<wallet_rpc_address>:<port>
Method: json_rpc
HTTP Header: Content-Type:application/json

Request Body:
{
   "jsonrpc":"2.0",
   "id":"0",
   "method":"sweep_all",
   "params":{
      "address":"<withdrawal_wallet_2_address>",
      "mixin":0,
      "unlock_time":0,
      "get_tx_keys":true
   }
}

Response Body:
{
   "id":"0",
   "jsonrpc":"2.0",
   "result":{
      "tx_hash_list":[
         "a64860b87afe9ce6fef95e8b2c360b335b8e2ccf66925eb252ca92f5680bf102"
      ],
      "tx_key_list":[
         "af05bfdbb3d20367c965cff441a3ce367334df72da88903d1ad208a9db71280c"
      ]
   }
}
```

Using curl:
```
curl -X POST http://localhost:28075/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"sweep_all","params":{"address":"etnjyug1v6i6y2BaNP1tQH4XkCNVFFBGP37v5rHkagQdZr84Eg7kKpWeYkcFYnfwNjeg9kXogoYupUkQZwG5aKwithdrawal02","mixin":0,"unlock_time":0,"get_tx_keys":true}}' -H 'Content-Type: application/json'
```



**4. Get the Withdrawal Wallet 1 block height**
```
Operation: POST
URL: http://<wallet_rpc_address>:<port>
Method: json_rpc
HTTP Header: Content-Type:application/json

Request Body:
{
   "jsonrpc":"2.0",
   "id":"0",
   "method":"getheight"
}

Response Body:
{
   "id":"0",
   "jsonrpc":"2.0",
   "result":{
      "height":306720
   }
}
```

Using curl:
```
curl -X POST http://127.0.0.1:28075/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"getheight"}' -H 'Content-Type:application/json'
```


**5. Swap out Withdrawal Wallet 1 for Withdrawal Wallet 2**

There are two ways to do this you can change the code in your script to point to the **WIDTHDRAWAL WALLET 2** or you can close the **WITHDRAWAL WALLET 1** RPC and open the **WITHDRAWAL WALLET 2** RPC on the same port.



**6. Regenerate Withdrawal Wallet 2 ready to be used as the new withdrawal wallet**

This file should be created either manually or automated by an external script.

**JSON Template:**
```
{
   "address":"<withdrawal_wallet_2_address>",
   "viewkey":"<withdrawal_wallet_2_view_key>",
   "spendkey":"<withdrawal_wallet_2_spend_key>",
   "filename":"<withdrawal_wallet_2>",
   "password":"<password>",
   "scan_from_height":<blockheight_from_step_4 - 720>,
   "version":1
}
```

**7. Execute Rescan Spend command to prevent any double spend errors**
```
Operation: POST
URL: http://<wallet_rpc_address>:<port>
Method: json_rpc
HTTP Header: Content-Type:application/json

Request Body:
{
   "jsonrpc":"2.0",
   "id":"0",
   "method":"rescan_spent"
}
```

Using curl:
```
curl -X POST http://localhost:28075/json_rpc -d '{"jsonrpc":"2.0","id":"0", "method":"rescan_spent"}' -H 'Content-Type: application/json'
```

### Cold Storage

It is considered best practice to ensure you are keeping a percentage of your ETN in cold storage in a separate wallet or wallets. Again, depending on the transaction numbers you could rotate these wallets for increased wallet efficiency.

### Integrated Addresses

Integrated addresses are an amalgamation of an ETN address and a payment id bundled into a single string. If you are going to use integrated addresses you need to make sure the wallet is open in the RPC connected to the correct wallet.

**To make an integrated Address for the wallet you are connected to:**
```
Operation: POST
URL: http://<wallet_rpc_address>:<port>
Method: json_rpc
HTTP Header: Content-Type:application/json

Request Body:
{
   "jsonrpc":"2.0",
   "id":"0",
   "method":"make_integrated_address",
   "params":{
      "payment_id":"<arbitrary_payment_id>"
   }
}

Response Body:
{
   "id":"0",
   "jsonrpc":"2.0",
   "result":{
      "integrated_address":"f4VR57DFe5X4pkhgMKvJVaJ8RuAPRHEotJos2ZHoPKzSJfQ4y3V2Qk3AWM289t9vLXGwLfC9i84rRbCeXikxZCbeSQxqUusKXok544fjfNJcq",
      "payment_id":"<arbitrary_payment_id>"
   }
}
```

**Note:** If the payment_id is not provided during request time, a random payment_id will be generated for this Integrated Address and returned by the API.

Using curl:
```
curl -X POST http://127.0.0.1:28075/json_rpc -d '{"jsonrpc":"2.0","id":"0" "method":"make_integrated_address","params":{"payment_id":""}}' -H 'Content-Type: application/json'
```


**To get the details from an Integrated Address:**
```
Operation: POST
URL: http://<wallet_rpc_address>:<port>
Method: json_rpc
HTTP Header: Content-Type:application/json

Request Body:
{
   "jsonrpc":"2.0",
   "id":"0",
   "method":"split_integrated_address",
   "params":{
      "integrated_address":"<etn_integrated_address>"
   }
}

Response Body:
{
   "id":"0",
   "jsonrpc":"2.0",
   "result":{
      "payment_id":"859b346eff6af272",
      "standard_address":"etnk4JPm2p14pkhgMKvJVaJ8RuAPRHEotJos2ZHoPKzSJfQ4y3V2Qk3AWM289t9vLXGwLfC9i84rRbCeXikxZCbe6kwnvkBdk3"
   }
}
```

Using curl:
```
curl -X POST http://127.0.0.1: 28075/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"split_integrated_address","params":{"integrated_address":"f4VR57DFe5X4pkhgMKvJVaJ8RuAPRHEotJos2ZHoPKzSJfQ4y3V2Qk3AWM289t9vLXGwLfC9i84rbCeXikxZCbeSQxqUusKXok54fjfNJcq"}}' -H 'Content-Type: application/json'
```

### Transaction Best Practice

We recommend the **transfer_split** RPC command over the **transfer** RPC command due to it being a legacy command. The main reason for this is to stop the **'TX is too large'** error.

The **transfer** command is a legacy and **transfer_split** SHOULD be used. It has to create transfers based on the INPUTS to the wallet.

These are transfers that it matches up with OUTPUTS that it is generating. If the inputs (small transfers) add up to a certain kilobyte size, the system knows that it needs to split this transaction up into smaller transactions that will fit in to multiple blocks.

### ETN Mobile App QR Code

The ETN Mobile app can scan QR Codes to pre-populate fields for sending money. If you generate a QR code with the following information, it can be imported directly.
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

## Troubleshooting

### Problems when syncing the blockchain

If your Electroneum daemon is stuck syncing the blockchain, please follow the steps bellow.

**1. Make sure you're running Electroneum's latest software.**

Binaries can be found at: [https://github.com/electroneum/electroneum/releases](https://github.com/electroneum/electroneum/releases)

**2. Delete the p2pstate.bin file**

Make sure Electroneum daemon is not running by typing "exit" in electroneumd. After closing Electroneum's daemon and wallets, you can safely delete the p2pstate.bin file located at:
```
$HOME/.electroneum
```
This directory is hidden by default. On Linux you can use CTRL + H to unhide directories (note that this might not work on all Linux distributions).

After you've deleted p2pstate.bin you can restart electroneumd. If this did not resolve your issue, please read further and make sure electroneumd is closed again.

**3. Change the block sync size**

We're going to add the following flag:
```
--block-sync-size 10
```
This will change the batch of fetched blocks from 20 to 10.

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

## Useful Electroneum Links

### Technical Links

 * [GitHub Repository](https://github.com/electroneum/electroneum)
 * [Block explorer](https://blockexplorer.electroneum.com)
 * [Daemon RPC Documentation](https://github.com/electroneum/electroneum/blob/develop/docs/daemon-rpc-documentation.md)
 * [Wallet RPC Documentation](https://github.com/electroneum/electroneum/blob/develop/docs/wallet-rpc-documentation.md)



### Social Media Links

Official social media channels managed in office by our PR/social media team

* [Blog](https://electroneum.com/blog/)
* [Twitter](https://twitter.com/electroneum)
* [Facebook](https://www.facebook.com/electroneum/)
* [Electroneum Community Forum](https://community.electroneum.com/)



### Electroneum Support Desk & FAQ's

Support site: [https://electroneum.com/support/](https://electroneum.com/support/)