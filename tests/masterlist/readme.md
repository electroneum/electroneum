#Master List Example

This folder contains a master list for testing purposes.

The JSON folder contains the json for the overall master list including master keys used to sign the set of permissions,
the base64 encoded blob of these permissions, and the corresponding signatures. mlist.json is the cleartext base64 decoded json version of this blob.
The list works on the assumption that you are going to create a new blockchain for testing purposes (the list does not contain
the keys of the mainnet miners).

## Keys

This subdirectory contains validator private and public keys which can be used for signing and verifying blocks, as well as master keys for signing the master list.
You can validate blocks by choosing one of the private keys and setting it via the daemon RPC method
set_validator_key.
    
##List Endpoint

This folder contains a basic node server which allows you to host your master list json so that clients can request it. To run this, first install node js, 
and then start the server by running

````console
foo@bar:~$ node server.js
````

The server runs on port 80 by default but you can change this inside server.js.
Please note that in order for clients to request the list from your server rather than Electroneum's, you will need to change
the go-to domain in the blockchain code: 

        string endpoint_addr = "vl.electroneum.com";
        string endpoint_port = "80";
        string testnet_endpoint_addr = "vl.thesecurityteam.rocks";
        string testnet_endpoint_port = "80";
        
## Security Notice
The validators of the official ELectroneum network follow a more secure procedure for setting their validator keys and signing blocks, which 
is made possible by custom programs and cryptography. The block validation/key usage paradigm offered here (in the public release) is meant to be a mock version
that enables simpler and straightforward testing.