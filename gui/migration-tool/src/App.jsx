import { useState, useEffect } from 'react';
import KeyEntry from './screens/KeyEntry.jsx';
import Syncing from './screens/Syncing.jsx';
import Result from './screens/Result.jsx';
import logo from './assets/electroneum-logo-symbol.png';
import './App.css';

// Screens: 'key-entry' → 'syncing' → 'result'
export default function App() {
  // TEMP: skip to result screen for testing paper wallet
  const [screen, setScreen] = useState('result');
  const [walletInfo, setWalletInfo] = useState({
    ethInfo: {
      address: '0x742d35Cc6634C0532925a3b844Bc9e7595f2bD18',
      privateKey: '0x4c0883a69102937d6231471b5dbb6204fe512961708279f23efb3c3b3e3e5e1c',
    },
  });
  const [shuttingDown, setShuttingDown] = useState(false);

  useEffect(() => {
    window.electronAPI.onShuttingDown(() => setShuttingDown(true));
  }, []);

  function handleKeyEntrySuccess(info) {
    setWalletInfo(info);
    setScreen('syncing');
  }

  function handleSynced() {
    setScreen('result');
  }

  function handleReset() {
    setWalletInfo(null);
    setScreen('key-entry');
  }

  if (shuttingDown) {
    return (
      <div className="app">
        <div className="screen shutdown-overlay">
          <img src={logo} alt="Electroneum" className="logo" />
          <h2>Saving wallet data...</h2>
          <p className="subtitle">Please wait. Do not close this window.</p>
        </div>
      </div>
    );
  }

  return (
    <div className="app">
      {screen === 'key-entry' && (
        <KeyEntry onSuccess={handleKeyEntrySuccess} />
      )}
      {screen === 'syncing' && (
        <Syncing onSynced={handleSynced} />
      )}
      {screen === 'result' && walletInfo && (
        <Result ethInfo={walletInfo.ethInfo} onReset={handleReset} />
      )}
    </div>
  );
}
