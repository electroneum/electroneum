const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  getRpcPort: () => ipcRenderer.invoke('get-rpc-port'),
  createWallet: (params) => ipcRenderer.invoke('create-wallet', params),
  getSyncStatus: () => ipcRenderer.invoke('get-sync-status'),
  getMigrationStatus: () => ipcRenderer.invoke('get-migration-status'),
  onShuttingDown: (callback) => ipcRenderer.on('shutting-down', callback),
});
