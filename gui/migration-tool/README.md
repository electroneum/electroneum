# ETN Migration Tool

Electron + React GUI that migrates legacy Electroneum wallets to the EVM-compatible Smart Chain. Users enter their wallet address and keys, the tool syncs via `electroneum-wallet-rpc`, detects migration transactions, derives the Smart Chain address, and offers a password-protected PDF paper wallet for offline backup.

## Prerequisites

- Node.js 18+
- `electroneum-wallet-rpc` binary for each target platform placed in `bin/`:
  - `bin/mac-arm64/electroneum-wallet-rpc`
  - `bin/linux-x64/electroneum-wallet-rpc`
  - `bin/win-x64/electroneum-wallet-rpc.exe`

## Install

```bash
cd gui/migration-tool
npm install
```

## Dev

```bash
npm run dev
```

## Build

```bash
npm run build          # builds for current platform
npm run build:mac      # macOS DMG (arm64)
npm run build:linux    # Linux AppImage + deb (x64)
npm run build:win      # Windows NSIS installer (x64)
```

### Linux build note

Building the AppImage target requires FUSE:

```bash
sudo apt-get install fuse
```

Without this, `electron-builder` will fail when packaging the AppImage. The `.deb` target does not require FUSE.

## Project Structure

```
electron/
  main.js           — spawns wallet-rpc, IPC handlers, paper wallet generation
  preload.js        — contextBridge exposing electronAPI to renderer
  paperWallet.js    — generates password-protected PDF with QR codes (bundled into main.js by electron-vite)
src/
  screens/          — KeyEntry → Syncing → Result flow
  components/       — PaperWalletModal (password entry for PDF generation)
  lib/              — deriveEthAddress, walletRpc wrappers, constants
bin/                — platform-specific electroneum-wallet-rpc binaries
resources/          — app icons
```

## Toy Blockchain Testing

The `toy-blockchain-testing-files/` directory contains everything needed to test the full migration flow locally without syncing to mainnet or testnet.

### What it does

A patch (`testnet-toy-hardforks.patch`) compresses all testnet hardfork heights down to blocks 1–60, so you can mine a complete chain with all forks in seconds using `--fixed-difficulty 1`. It also mocks validator signatures so no real validator keys are needed.

### How to use it

1. **Apply the patch** to the Electroneum source (from repo root):
   ```bash
   git apply gui/migration-tool/toy-blockchain-testing-files/testnet-toy-hardforks.patch
   ```

2. **Rebuild** `electroneumd` and `electroneum-wallet-rpc`:
   ```bash
   mkdir -p build/Darwin/gui-migration-app/release && cd build/Darwin/gui-migration-app/release
   cmake -DCMAKE_BUILD_TYPE=Release ../../../..
   make -j$(nproc) electroneumd electroneum-wallet-rpc electroneum-wallet-cli
   ```

3. **Run the test script** — starts two local daemons, mines through all forks, and leaves the chain ready:
   ```bash
   cd gui/migration-tool/toy-blockchain-testing-files
   ./migration-test.sh
   ```

4. **Change networking in the GUI app** — the patch updates `electron/main.js` and `src/lib/constants.js` to point at `127.0.0.1:34568` with SSL disabled and `--testnet` enabled. If applying the patch manually, change:
   - `REMOTE_NODE` → `'127.0.0.1:34568'`
   - `REMOTE_NODE_URL` → `'http://127.0.0.1:34568'`
   - wallet-rpc args: `--daemon-ssl=disabled` and add `--testnet`

5. **Get wallet keys** from the included test wallets:
   ```bash
   electroneum-wallet-cli --testnet \
     --wallet-file gui/migration-tool/toy-blockchain-testing-files/test-wallets/miner1 \
     --password "" --offline
   # then: spendkey / viewkey / exit
   ```

6. **Run the GUI** (`npm run dev`) and enter the wallet details.

7. **Clean up**:
   ```bash
   ./migration-test.sh stop    # stop daemons
   ./migration-test.sh clean   # remove blockchain data
   ```
   Revert the source patch: `git checkout -- src/ gui/migration-tool/electron/main.js gui/migration-tool/src/lib/constants.js`

See `toy-blockchain-testing-files/MIGRATION-TEST-INSTRUCTIONS.txt` for full details including manual commands, wallet seeds, port layout, and troubleshooting.
