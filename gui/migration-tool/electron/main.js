const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const { spawn } = require('child_process');
const net = require('net');
const fs = require('fs');
const axios = require('axios');
const crypto = require('crypto');

const REMOTE_NODE = 'legacy-rpc.electroneum.com:443';
const REMOTE_NODE_URL = 'https://legacy-rpc.electroneum.com';
const WALLET_RPC_READY_TIMEOUT_MS = 60000;
const WALLET_RPC_POLL_INTERVAL_MS = 500;

let walletRpcProcess = null;
let walletRpcPort = null;
let mainWindow = null;

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
    platformDir = arch === 'arm64' ? 'mac-arm64' : 'mac-x64';
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
    // __dirname is out/main/ at runtime; walk up to gui/migration-tool then up two more to repo root
    const repoRoot = path.join(__dirname, '..', '..', '..', '..');
    const osDir = platform === 'darwin' ? 'Darwin' : platform === 'linux' ? 'Linux' : 'Windows';
    return path.join(repoRoot, 'build', osDir, 'master', 'release', 'bin', binaryName);
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

// ── Spawn wallet-rpc ─────────────────────────────────────────────────────────

async function startWalletRpc() {
  walletRpcPort = await findFreePort();
  const binaryPath = getWalletRpcBinaryPath();
  const walletDir = getWalletDir();

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
    '--log-level', '1',
    '--wallet-dir', walletDir,
    '--rpc-ssl=disabled',
    '--daemon-ssl=enabled',
    '--daemon-ssl-allow-any-cert',
    '--trusted-daemon',
  ];

  console.log(`[main] Spawning wallet-rpc on port ${walletRpcPort}`);
  walletRpcProcess = spawn(binaryPath, args, { stdio: ['ignore', 'pipe', 'pipe'] });

  walletRpcProcess.stdout.on('data', (d) => console.log(`[wallet-rpc] ${d.toString().trim()}`));
  walletRpcProcess.stderr.on('data', (d) => console.error(`[wallet-rpc] ${d.toString().trim()}`));
  walletRpcProcess.on('exit', (code) => {
    console.log(`[main] wallet-rpc exited with code ${code}`);
    walletRpcProcess = null;
  });

  await waitForWalletRpcReady();
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

function stopWalletRpc() {
  if (walletRpcProcess) {
    console.log('[main] Killing wallet-rpc');
    walletRpcProcess.kill();
    walletRpcProcess = null;
  }
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

  try {
    const result = await rpc('generate_from_keys', {
      restore_height: 0,
      filename,
      address,
      spendkey: spendKey,
      viewkey: viewKey,
      password: '',
      autosave_current: true,
    }, 300000);
    return { ok: true, result };
  } catch (err) {
    // If wallet file already exists wallet-rpc returns an error; try opening it instead
    const msg = err.message || '';
    if (msg.includes('already exists') || msg.includes('already open')) {
      try {
        await rpc('open_wallet', { filename, password: '' }, 300000);
        return { ok: true, result: { address, info: 'Wallet already exists, opened.' } };
      } catch (openErr) {
        return { ok: false, error: openErr.message };
      }
    }
    return { ok: false, error: msg };
  }
});

ipcMain.handle('get-sync-status', async () => {
  console.log('[sync] polling get_height...');
  try {
    const heightRes = await rpc('get_height');
    const walletHeight = heightRes.height;
    console.log(`[sync] walletHeight=${walletHeight}`);

    let daemonHeight = 0;
    try {
      const daemonRes = await axios.post(
        `${REMOTE_NODE_URL}/json_rpc`,
        { jsonrpc: '2.0', id: '0', method: 'get_info' },
        { timeout: 120000 }
      );
      daemonHeight = daemonRes.data.result.height;
      console.log(`[sync] daemonHeight=${daemonHeight}`);
    } catch (daemonErr) {
      console.warn('[sync] Daemon get_info failed:', daemonErr.message);
    }

    console.log(`[sync] result: wallet=${walletHeight} daemon=${daemonHeight} remaining=${daemonHeight - walletHeight}`);
    return { ok: true, walletHeight, daemonHeight };
  } catch (err) {
    return { ok: false, error: err.message };
  }
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

app.whenReady().then(async () => {
  try {
    await startWalletRpc();
  } catch (err) {
    console.error('[main] Failed to start wallet-rpc:', err.message);
    // Don't crash — renderer will show an error via get-rpc-port returning null
    walletRpcPort = null;
  }
  createWindow();
});

app.on('before-quit', () => {
  stopWalletRpc();
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow();
});
