/**
 * Thin wrappers around the Electron IPC bridge to wallet-rpc.
 * All functions return the result directly or throw on error.
 */

const api = window.electronAPI;

export async function generateFromKeys(address, spendKey, viewKey) {
  const res = await api.createWallet({ address, spendKey, viewKey });
  if (!res.ok) throw new Error(res.error);
  return res.result;
}

export async function getSyncStatus() {
  const res = await api.getSyncStatus();
  if (!res.ok) throw new Error(res.error);
  return { walletHeight: res.walletHeight, daemonHeight: res.daemonHeight };
}

export async function waitForRpcReady() {
  const res = await api.waitForRpcReady();
  if (!res.ok) throw new Error(res.error);
}

export async function getMigrationStatus() {
  const res = await api.getMigrationStatus();
  if (!res.ok) throw new Error(res.error);
  return res.scMigrations; // array of transfer_entry objects
}

export async function getPreliminaryMigrationStatus() {
  const res = await api.getPreliminaryMigrationStatus();
  if (!res.ok) throw new Error(res.error);
  return res.migrations; // array of transfer_entry objects (v2 private→public)
}
