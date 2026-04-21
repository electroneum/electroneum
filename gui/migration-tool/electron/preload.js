const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  getRpcPort: () => ipcRenderer.invoke('get-rpc-port'),
  createWallet: (params) => ipcRenderer.invoke('create-wallet', params),
  getSyncStatus: () => ipcRenderer.invoke('get-sync-status'),
  waitForRpcReady: () => ipcRenderer.invoke('wait-for-rpc-ready'),
  getMigrationStatus: () => ipcRenderer.invoke('get-migration-status'),
  getPreliminaryMigrationStatus: () => ipcRenderer.invoke('get-preliminary-migration-status'),
  generatePaperWallet: (params) => ipcRenderer.invoke('generate-paper-wallet', params),
  onShuttingDown: (callback) => ipcRenderer.on('shutting-down', callback),
});
