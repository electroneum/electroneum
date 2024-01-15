<p align="center">
  <img src="https://imgur.com/3FpIaYL.png">
</p>
<h4 align="center">Electroneum is a Fast, Secure, Mobile Based Cryptocurrency </h4>

| :warning: WARNING                                                                                                                                                                                                                                                                   |
|:------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| This project is the legacy version of Electroneum. Electroneum has since migrated over to a new smart contract enabled blockchain: https://github.com/electroneum/electroneum-sc. For details on how to migrate your ETN, please see: [Smartchain-Migration](#Smartchain-Migration) |

## Table of Contents

 * [Development Resources](#Development-Resources)
   * [Building from Source](#Building-from-Source)
   * [Running an Electroneum Node](#Running-an-Electroneum-Node)
 * [Technical Documentation](#Technical-Documentation)
   * [Daemon RPC Documentation](docs/daemon-rpc-documentation.md)
   * [Wallet RPC Documentation](docs/wallet-rpc-documentation.md)
   * [Exchange Listing Guide](docs/exchange-listing-guide.md)
   * [Smartchain Migration](#Smartchain-Migration)
 * [Vulnerability Response Process](#Vulnerability-Response-Process)
 * [License](#License)
 * [Copyright](#Copyright)

# Development Resources

As with many development projects, the repository on Github is considered to be the "staging" area for the latest changes. Before changes are merged into that branch on the main repository, they are tested by individual developers in their own branches, submitted as a pull request, and then subsequently tested by contributors who focus on testing and code reviews. That having been said, the repository should be carefully considered before using it in a production environment, unless there is a patch in the repository for a particular show-stopping issue you are experiencing. It is generally a better idea to use a tagged release for stability.

**Anyone is welcome to contribute to Electroneum's codebase!** If you have a fix or code change, feel free to submit it as a pull request directly to the "master" branch. In cases where the change is relatively small or does not affect other parts of the codebase it may be merged in immediately by any one of the collaborators. On the other hand, if the change is particularly large or complex, it is expected that it will be discussed at length either well in advance of the pull request being submitted, or even directly on the pull request.

## Building from Source

See [Build & Run Documentation](docs/build-and-run.md).

## Software Releases and Pre-compiled Downloads

| Hard Fork upgrade height | Date       | Fork version     | Minimum Electroneum version    | Recommended Electroneum version | Name of Release               | Details                                                                                                                                                                                                                                        |
|--------------------------|------------|------------------|--------------------------------|---------------------------------|-------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 307500                   | 2018-05-30 | v6               | v2.0.0.0                       | v2.0.0.0                        | Spark                         | Disable Mixin, Disable RingCT, Base Fee to 0.10 from 0.01, 120s Block Time, Anti-Asic Resistance                                                                                                                                               |
| 324500                   | 2018-07-05 | v7               | v2.1.0.0                       | v2.1.0.0                        | Bolt                          | Enable ASIC                                                                                                                                                                                                                                    |
| 589169                   | 2019-07-08 | v8               | v3.0.0.0                       | v3.0.0.0                        | Pulse                         | Migration to our next generation moderated blockchain with Proof of Responsbility. 300kB Blocks : Increased TPS. Improved Unlock Time (5 Blocks), Many Security Improvements, HackerOne Fixes, Reduced Block Reward By 75% (A Double Halvening) |
| 862866                   | 2020-07-22 | v9               | v3.3.0.0                       | v3.3.0.0                        | Wave                          | Block Reward reduction and future halving scheduling (halving every 4 years, minimum 50etn until max supply. 25etn emission per block after reaching max supply).                                                                              
| 1175315                  | 2021-09-30 | v10              | v4.0.0.0                       | v4.0.0.1                        | Aurora                        | Migration to a transparent blockchain with permissioned validation                                                                                                                                                                             |             
| 1811310                  | 2024-03-05 | v11              | v5.0.0.0                       | v5.0.0.1                        | Flow (Final Version)          | Bridge to the Electroneum Smart chain (github.com/electroneum/electroneum-sc)                                                                                                                                                                  |

X's indicate that these details have not been determined as of commit date.

For pre-built downloads and further details of both our forks and intermediate releases, please go to [our github releases page.](https://github.com/electroneum/electroneum/releases)

## Running an Electroneum Node

See [Build & Run Documentation](docs/build-and-run.md).

# Technical Documentation

* [Daemon RPC Documentation](docs/daemon-rpc-documentation.md)
* [Wallet RPC Documentation](docs/wallet-rpc-documentation.md)
* [Exchange Listing Guide](docs/exchange-listing-guide.md)

## Smartchain Migration

Please see our  [gitbook documentation](https://developer.electroneum.com/)
which explains how to migrate your ETN over to the Electroneum Smartchain.

Some technical information about how the bridge address was created can be found below:
```

The bridge address spendkey is generated by doing Hash_to_point(genesis block hash).
This ensures that the bridge address is essentially a burn address for which there is no known private key. This means that nobody can access the funds, even Electroneum Ltd.
The same logic has been used for a coinbase burn address (hash_to_point(hash of v9 fork block: 1175315 )), which the remaining validators for this project will mine to after the Electroneum Flow fork block, meaning that the circulating supply can be considered frozen at the final fork block.
This is required for continuity in the circulating supply when we migrate to the smartchain.

The bridge address for the mainnet and testnet is:
```etnk6XD4xkmgsajaYyDD7SGsB93Ff6iUN2TaAaqageGkKj2yB1mtd5wJ8QgRfFWTzmJ8QgRfFWTzmJ8QgRfFWTzm4t51KXZBNg```

The coinbase burn address for the mainnet and testnet is:
```etnkCys4uGhSi9h48ajL9vBDJTcn2s2ttXtXq3SXWPAbiMHNhHitu5fJ8QgRfFWTzmJ8QgRfFWTzmJ8QgRfFWTzm4t51HTfCtK```

```

# Vulnerability Response Process

HackerOne is a vulnerability coordination and bug bounty platform that connects businesses with penetration testers and cybersecurity researchers. It was one of the first companies, along with Synack and Bugcrowd, to embrace and utilize crowd-sourced security and cybersecurity researchers as linchpins of its business model; it is the largest cybersecurity firm of its kind. As of July 2018, HackerOne's network consisted of approximately 200,000 researchers, had resolved 72,000 vulnerabilities across over 1,000 customer programs, and had paid $31 million in bounties.

See [HackerOne Electroneum's Page](https://hackerone.com/electroneum).

# License

See [LICENSE](LICENSE).

# Copyright

Copyright (c) 2017-Present, Electroneum

Copyright (c) 2014-2017, The Monero Project

Portions Copyright (c) 2012-2013, The Cryptonote developers