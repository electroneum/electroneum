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

### Windows build note

On Windows, the build may fail with a symlink error when `electron-builder` extracts its code signing cache (`Cannot create symbolic link : A required privilege is not held by the client`). This is a Windows permissions issue, not a code problem. To fix it, either:

- Run the build from an **Administrator** command prompt / PowerShell, or
- Enable **Developer Mode** in Windows (Settings → Update & Security → For Developers → Developer Mode), which grants symlink permissions without admin.

The produced executable will still work even if this error appears — it only affects macOS-specific files in the cache that are not needed on Windows.

## Antivirus & Firewall Guide

The migration tool bundles `electroneum-wallet-rpc`, a background process that communicates with the Electroneum legacy blockchain. Antivirus software and OS firewalls may flag or block this binary because cryptocurrency-related executables commonly trigger false positive detections. If the app shows a `connect ECONNREFUSED 127.0.0.1:...` error on the first screen, the wallet-rpc process has been blocked from starting.

### Windows

1. **Windows Defender** — Open Windows Security → Virus & threat protection → Protection history. If `electroneum-wallet-rpc.exe` has been quarantined, select it and choose "Allow on device". Then go to Virus & threat protection settings → Exclusions → Add an exclusion → Folder, and add the app's installation directory (typically `C:\Users\<you>\AppData\Local\Programs\etn-migration-tool\`).

2. **Third-party antivirus (Avast, Norton, etc.)** — Add the same installation directory to the antivirus exclusion/whitelist. The exact steps vary by product — look for "Exclusions", "Exceptions", or "Whitelist" in the antivirus settings. You may need to restore the file from quarantine first.

3. **Windows Firewall** — The app only makes outbound connections to `legacy-rpc.electroneum.com` and communicates with wallet-rpc on `127.0.0.1` (localhost). If the firewall prompts you when opening the app, allow the connection. If it was already blocked: Control Panel → Windows Defender Firewall → Allow an app through firewall → Add `ETN Migration Tool.exe` and `electroneum-wallet-rpc.exe`.

### macOS

1. **Gatekeeper** — On first launch, macOS may show "app is from an unidentified developer". Right-click the app → Open → Open. This only needs to be done once.

2. **Notarisation warning** — If macOS says the app "cannot be opened because it is from an unidentified developer", go to System Settings → Privacy & Security → scroll down to the Security section where the blocked app will be listed → click "Open Anyway".

3. **wallet-rpc blocked separately** — macOS may also block the `electroneum-wallet-rpc` binary inside the app bundle. If the app opens but shows a connection error, go to System Settings → Privacy & Security and look for a second blocked item to allow.

4. **Firewall** — If the macOS firewall is enabled (System Settings → Network → Firewall), add the app to the allowed list or temporarily disable the firewall during migration.

### Linux

1. **AppImage permissions** — After downloading, make the AppImage executable: `chmod +x ETN-Migration-Tool-*.AppImage`. FUSE is required to run AppImages: `sudo apt-get install fuse`.

2. **SELinux / AppArmor** — On distributions with mandatory access control (e.g. Ubuntu with AppArmor, Fedora with SELinux), the embedded `electroneum-wallet-rpc` may be prevented from executing. Check `dmesg` or `journalctl -xe` for denial messages. If blocked, you can temporarily set SELinux to permissive (`sudo setenforce 0`) or add an AppArmor exception for the app.

3. **Firewall (ufw / firewalld)** — The app only makes outbound connections. Most Linux firewalls allow outbound by default. If you have a restrictive outbound policy, allow connections to `legacy-rpc.electroneum.com` on port `443`.

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
