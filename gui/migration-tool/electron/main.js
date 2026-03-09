const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const { spawn, execSync } = require('child_process');
const net = require('net');
const fs = require('fs');
const axios = require('axios');
const crypto = require('crypto');
// On Windows, Node's proc.kill('SIGTERM') does an unconditional TerminateProcess
// — no signal handler runs, so wallet-rpc can't save. We use ctrlc-windows to
// send CTRL_C_EVENT instead, which triggers the C++ win_handler and saves.
const isWin = process.platform === 'win32';
let ctrlc;
if (isWin) {
  try { ctrlc = require('ctrlc-windows').ctrlc; } catch { /* not available */ }
}

/**
 * Send a graceful termination signal to a process.
 * - Unix: SIGTERM (caught by posix_handler → store → exit)
 * - Windows: CTRL_C_EVENT via ctrlc-windows (caught by win_handler → store → exit)
 *   Falls back to proc.kill() if ctrlc-windows is unavailable.
 */
function gracefulKill(proc) {
  if (isWin && ctrlc) {
    console.log(`[main] Sending CTRL_C_EVENT to PID ${proc.pid}`);
    ctrlc(proc.pid);
  } else {
    proc.kill('SIGTERM');
  }
}

const REMOTE_NODE = 'legacy-rpc.electroneum.com:443';
const REMOTE_NODE_URL = 'https://legacy-rpc.electroneum.com';
const WALLET_RPC_READY_TIMEOUT_MS = 60000;
const WALLET_RPC_POLL_INTERVAL_MS = 500;

let walletRpcProcess = null;
let walletRpcPort = null;
let walletRpcMode = null; // 'dir' or 'file'
let mainWindow = null;

// Sync progress parsed from wallet-rpc stdout. We track this instead of
// calling get_height RPC, because wallet-rpc is single-threaded and the
// RPC server is blocked while a refresh is in progress.
//
// The height advances in increments of ~999 (not 1000) because the daemon
// returns 1000 blocks per batch but includes one overlapping block that the
// wallet already knows. This overlap exists for fork detection — the wallet
// verifies the first returned block matches its chain tip, and if it doesn't,
// a chain reorganisation has occurred and the wallet rewinds.
let syncProgressHeight = 0;

// ── Port helpers ─────────────────────────────────────────────────────────────

function findFreePort() {
  return new Promise((resolve, reject) => {
    const server = net.createServer();
    server.listen(0, '127.0.0.1', () => {
      const { port } = server.address();
      server.close(() => resolve(port));
    });
    server.on('error', reject);
  });
}

// ── Binary path resolution ───────────────────────────────────────────────────

function getWalletRpcBinaryPath() {
  const isDev = !app.isPackaged;
  const platform = process.platform; // darwin, linux, win32
  const arch = process.arch;         // arm64, x64

  let platformDir;
  if (platform === 'darwin') {
    platformDir = 'mac-arm64';
  } else if (platform === 'linux') {
    platformDir = 'linux-x64';
  } else if (platform === 'win32') {
    platformDir = 'win32-x64';
  } else {
    throw new Error(`Unsupported platform: ${platform}`);
  }

  const binaryName = platform === 'win32'
    ? 'electroneum-wallet-rpc.exe'
    : 'electroneum-wallet-rpc';

  if (isDev) {
    // __dirname is out/main/ at runtime; walk up to gui/migration-tool/
    const projectRoot = path.join(__dirname, '..', '..');
    return path.join(projectRoot, 'bin', platformDir, binaryName);
  }

  // In production: electron-builder copies bin/ into resourcesPath
  return path.join(process.resourcesPath, 'bin', binaryName);
}

// ── Wallet dir ───────────────────────────────────────────────────────────────

function getWalletDir() {
  const dir = path.join(app.getPath('userData'), 'wallets');
  if (!fs.existsSync(dir)) {
    fs.mkdirSync(dir, { recursive: true });
  }
  return dir;
}

// ── Stale process cleanup ────────────────────────────────────────────────────

function getPidFilePath() {
  return path.join(getWalletDir(), 'wallet-rpc.pid');
}

function killStaleWalletRpc() {
  const pidsToKill = new Set();

  // 1. Check our PID file
  const pidFile = getPidFilePath();
  try {
    const pid = parseInt(fs.readFileSync(pidFile, 'utf-8').trim(), 10);
    if (pid && !isNaN(pid)) {
      try { process.kill(pid, 0); pidsToKill.add(pid); } catch { /* already dead */ }
    }
  } catch { /* no pid file or unreadable */ }

  // 2. Find any electroneum-wallet-rpc using our wallet dir (catches orphans)
  if (process.platform !== 'win32') {
    try {
      const walletDir = getWalletDir();
      const out = execSync(
        `pgrep -f "electroneum-wallet-rpc.*(--wallet-dir|--wallet-file).*${walletDir}"`,
        { encoding: 'utf-8', timeout: 5000 }
      ).trim();
      for (const line of out.split('\n')) {
        const pid = parseInt(line.trim(), 10);
        if (pid && pid !== process.pid) pidsToKill.add(pid);
      }
    } catch { /* pgrep returns exit 1 when no matches — that's fine */ }
  }

  // 3. Kill them all
  for (const pid of pidsToKill) {
    console.log(`[main] Killing stale wallet-rpc (PID ${pid})`);
    try {
      if (isWin && ctrlc) {
        ctrlc(pid);
      } else {
        process.kill(pid, 'SIGTERM');
      }
    } catch { continue; }
    // Wait up to 3s for graceful exit, then force kill
    const deadline = Date.now() + 3000;
    while (Date.now() < deadline) {
      try { process.kill(pid, 0); } catch { break; } // gone
      const wait = Date.now() + 200;
      while (Date.now() < wait) { /* spin */ }
    }
    try { process.kill(pid, 'SIGKILL'); } catch { /* already gone */ }
  }

  try { fs.unlinkSync(pidFile); } catch { /* ignore */ }
}

function writePidFile(pid) {
  try {
    fs.writeFileSync(getPidFilePath(), String(pid), 'utf-8');
  } catch (err) {
    console.warn('[main] Could not write PID file:', err.message);
  }
}

function removePidFile() {
  try { fs.unlinkSync(getPidFilePath()); } catch { /* ignore */ }
}

// ── Parse wallet-rpc stdout for sync progress ───────────────────────────────

function parseWalletRpcOutput(data) {
  const text = data.toString();
  for (const line of text.split('\n')) {
    const trimmed = line.trim();
    if (!trimmed) continue;

    // Match: "Processed block: <hash>, height 123456, ..."
    const m = trimmed.match(/Processed block: .*, height (\d+)/);
    if (m) {
      syncProgressHeight = parseInt(m[1], 10);
      // Don't log every block — too noisy
      continue;
    }

    // Skip lines that are just timestamps/log prefixes with no useful content
    // (these come from the DEBUG log lines surrounding "Processed block")
    if (trimmed.match(/^\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+\s+[A-Z]\s*$/) ||
        trimmed.match(/^PERF\s/) ||
        trimmed.match(/process_new_transaction/)) {
      continue;
    }

    // Log all other lines normally
    console.log(`[wallet-rpc] ${trimmed}`);
  }
}

// ── Spawn wallet-rpc ─────────────────────────────────────────────────────────

// mode: 'dir'  — --wallet-dir (for creating wallets via RPC)
// mode: 'file' — --wallet-file (for syncing; initial refresh is interruptible
//                by SIGTERM, which saves the wallet cache before exiting)
async function startWalletRpc(mode, walletFile) {
  killStaleWalletRpc();

  walletRpcPort = await findFreePort();
  walletRpcMode = mode;
  const binaryPath = getWalletRpcBinaryPath();

  if (!fs.existsSync(binaryPath)) {
    throw new Error(`wallet-rpc binary not found at: ${binaryPath}`);
  }

  // Ensure binary is executable on unix
  if (process.platform !== 'win32') {
    fs.chmodSync(binaryPath, 0o755);
  }

  const args = [
    '--rpc-bind-port', String(walletRpcPort),
    '--daemon-address', REMOTE_NODE,
    '--disable-rpc-login',
    '--non-interactive',
    // Level 0 defaults + wallet.wallet2 at DEBUG to get per-block "Processed block" lines
    '--log-level', '0,wallet.wallet2:DEBUG',
    '--max-log-files', '1',
    '--rpc-ssl=disabled',
    '--daemon-ssl=enabled',
    '--daemon-ssl-allow-any-cert',
    '--trusted-daemon',
  ];

  if (mode === 'file') {
    args.push('--wallet-file', walletFile, '--password', '');
  } else {
    args.push('--wallet-dir', getWalletDir());
  }

  console.log(`[main] Spawning wallet-rpc in ${mode} mode on port ${walletRpcPort}`);
  walletRpcProcess = spawn(binaryPath, args, { stdio: ['ignore', 'pipe', 'pipe'] });
  writePidFile(walletRpcProcess.pid);

  walletRpcProcess.stdout.on('data', (d) => {
    parseWalletRpcOutput(d);
  });
  walletRpcProcess.stderr.on('data', (d) => {
    parseWalletRpcOutput(d);
  });
  const thisProc = walletRpcProcess;
  thisProc.on('exit', (code) => {
    console.log(`[main] wallet-rpc (PID ${thisProc.pid}) exited with code ${code}`);
    // Only clear the global ref if it still points to this process —
    // a new process may have been spawned before the old one exited.
    if (walletRpcProcess === thisProc) {
      walletRpcProcess = null;
      removePidFile();
    }
  });

  // In 'dir' mode we need the RPC server ready before we can create wallets.
  // In 'file' mode the initial refresh blocks the server from starting, so
  // we skip this — sync progress comes from stdout, and the RPC server will
  // become available once the refresh completes.
  if (mode === 'dir') {
    await waitForWalletRpcReady();
  }
}

async function waitForWalletRpcReady() {
  const deadline = Date.now() + WALLET_RPC_READY_TIMEOUT_MS;
  while (Date.now() < deadline) {
    try {
      await axios.post(
        `http://127.0.0.1:${walletRpcPort}/json_rpc`,
        { jsonrpc: '2.0', id: '0', method: 'get_version' },
        { timeout: 1000 }
      );
      console.log('[main] wallet-rpc is ready');
      return;
    } catch {
      await new Promise((r) => setTimeout(r, WALLET_RPC_POLL_INTERVAL_MS));
    }
  }
  throw new Error('wallet-rpc did not become ready in time');
}

// Synchronous stop for dir mode (no wallet loaded, nothing to save).
// Used when switching from dir→file mode during create-wallet.
function stopWalletRpcSync() {
  if (!walletRpcProcess) {
    removePidFile();
    return;
  }

  const proc = walletRpcProcess;
  const pid = proc.pid;
  walletRpcProcess = null;
  walletRpcPort = null;
  walletRpcMode = null;

  console.log(`[main] Stopping wallet-rpc sync (PID ${pid}, dir mode)...`);
  gracefulKill(proc);

  const deadline = Date.now() + 3000;
  while (Date.now() < deadline) {
    try { process.kill(pid, 0); } catch { removePidFile(); return; }
    const wait = Date.now() + 200;
    while (Date.now() < wait) { /* spin */ }
  }
  try { process.kill(pid, 'SIGKILL'); } catch { /* already gone */ }
  removePidFile();
}

// Async stop for file mode — sends SIGTERM and returns a Promise that
// resolves when wallet-rpc exits. Does not block the event loop, so Node
// can reap the child process properly. SIGTERM during the initial refresh
// interrupts it and triggers store() to save the wallet cache.
function stopWalletRpcAsync() {
  return new Promise((resolve) => {
    if (!walletRpcProcess) {
      removePidFile();
      resolve();
      return;
    }

    const proc = walletRpcProcess;
    const pid = proc.pid;
    const mode = walletRpcMode;
    walletRpcProcess = null;
    walletRpcPort = null;
    walletRpcMode = null;

    console.log(`[main] Stopping wallet-rpc (PID ${pid}, mode=${mode})...`);
    const start = Date.now();

    proc.on('exit', (code) => {
      console.log(`[main] wallet-rpc (PID ${pid}) exited with code ${code} after ${Date.now() - start}ms`);
      removePidFile();
      resolve();
    });

    gracefulKill(proc);
  });
}

// ── RPC helper ───────────────────────────────────────────────────────────────

async function rpc(method, params = {}, timeoutMs = 300000) {
  const start = Date.now();
  console.log(`[rpc] --> ${method} (timeout=${timeoutMs}ms)`);
  try {
    const res = await axios.post(
      `http://127.0.0.1:${walletRpcPort}/json_rpc`,
      { jsonrpc: '2.0', id: '0', method, params },
      { timeout: timeoutMs }
    );
    console.log(`[rpc] <-- ${method} OK (${Date.now() - start}ms)`);
    if (res.data.error) {
      console.error(`[rpc] <-- ${method} RPC error:`, res.data.error);
      throw new Error(res.data.error.message || JSON.stringify(res.data.error));
    }
    return res.data.result;
  } catch (err) {
    console.error(`[rpc] <-- ${method} FAILED after ${Date.now() - start}ms (timeout=${timeoutMs}ms):`, err.message);
    throw err;
  }
}

// ── IPC handlers ─────────────────────────────────────────────────────────────

ipcMain.handle('get-rpc-port', () => walletRpcPort);

ipcMain.handle('create-wallet', async (_event, { address, spendKey, viewKey }) => {
  // Derive a stable filename from the address
  const filename = crypto.createHash('sha256').update(address).digest('hex').slice(0, 16);
  const walletFile = path.join(getWalletDir(), filename);
  syncProgressHeight = 0;

  // If wallet file already exists, skip creation and go straight to file mode
  if (fs.existsSync(walletFile)) {
    console.log(`[main] Wallet file exists, restarting in file mode: ${walletFile}`);
    stopWalletRpcSync();
    await startWalletRpc('file', walletFile);
    return { ok: true, result: { address, info: 'Wallet already exists, opened.' } };
  }

  // Phase 1: create the wallet via RPC in dir mode
  try {
    await rpc('generate_from_keys', {
      restore_height: 0,
      filename,
      address,
      spendkey: spendKey,
      viewkey: viewKey,
      password: '',
      autosave_current: true,
    }, 300000);
  } catch (err) {
    const msg = err.message || '';
    if (!msg.includes('already exists') && !msg.includes('already open')) {
      return { ok: false, error: msg };
    }
  }

  // Phase 2: restart wallet-rpc in file mode so the initial refresh is
  // interruptible by SIGTERM and saves the wallet cache on exit
  console.log(`[main] Wallet created, restarting in file mode: ${walletFile}`);
  stopWalletRpcSync();
  await startWalletRpc('file', walletFile);
  return { ok: true, result: { address } };
});

ipcMain.handle('get-sync-status', async () => {
  // Wallet height comes from parsing stdout — no RPC call needed (which
  // would be blocked while the single-threaded wallet-rpc is refreshing).
  const walletHeight = syncProgressHeight;

  // Daemon height fetched directly over HTTPS (bypasses wallet-rpc entirely)
  let daemonHeight = 0;
  try {
    const daemonRes = await axios.post(
      `${REMOTE_NODE_URL}/json_rpc`,
      { jsonrpc: '2.0', id: '0', method: 'get_info' },
      { timeout: 120000 }
    );
    daemonHeight = daemonRes.data.result.height;
  } catch (daemonErr) {
    console.warn('[sync] Daemon get_info failed:', daemonErr.message);
  }

  return { ok: true, walletHeight, daemonHeight };
});

ipcMain.handle('get-migration-status', async () => {
  try {
    const result = await rpc('get_transfers', { sc_migration: true, out: true });
    return { ok: true, scMigrations: result.sc_migration || [] };
  } catch (err) {
    return { ok: false, error: err.message };
  }
});

// ── Window ───────────────────────────────────────────────────────────────────

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    resizable: false,
    webPreferences: {
      preload: path.join(__dirname, '..', 'preload', 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
    },
    titleBarStyle: 'hiddenInset',
    title: 'ETN Migration Tool',
  });

  if (!app.isPackaged) {
    // electron-vite dev server (default port 5173)
    mainWindow.loadURL(process.env['ELECTRON_RENDERER_URL'] || 'http://localhost:5173');
    mainWindow.webContents.openDevTools();
  } else {
    // electron-vite puts renderer output in out/renderer/
    mainWindow.loadFile(path.join(__dirname, '../renderer/index.html'));
  }
}

// ── App lifecycle ────────────────────────────────────────────────────────────

let isQuitting = false;

app.whenReady().then(async () => {
  try {
    await startWalletRpc('dir');
  } catch (err) {
    console.error('[main] Failed to start wallet-rpc:', err.message);
    walletRpcPort = null;
  }
  createWindow();

  // Intercept the window close to keep it visible while wallet-rpc saves.
  mainWindow.on('close', (event) => {
    if (isQuitting) return; // already shutting down, let it close
    event.preventDefault();
    isQuitting = true;

    console.log('[main] Window close requested — saving wallet...');
    mainWindow.webContents.send('shutting-down');

    stopWalletRpcAsync().then(() => {
      console.log('[main] wallet-rpc stopped, closing window');
      mainWindow.destroy();
    });
  });
});

app.on('window-all-closed', () => {
  app.quit();
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow();
});
