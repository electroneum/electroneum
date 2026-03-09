import { useState, useEffect } from 'react';
import KeyEntry from './screens/KeyEntry.jsx';
import Syncing from './screens/Syncing.jsx';
import Result from './screens/Result.jsx';
import logo from './assets/electroneum-logo-symbol.png';
import './App.css';

// Screens: 'key-entry' → 'syncing' → 'result'
export default function App() {
  const [screen, setScreen] = useState('key-entry');
  const [walletInfo, setWalletInfo] = useState(null);
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
