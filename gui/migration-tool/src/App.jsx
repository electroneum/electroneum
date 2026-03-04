import { useState } from 'react';
import KeyEntry from './screens/KeyEntry.jsx';
import Syncing from './screens/Syncing.jsx';
import Result from './screens/Result.jsx';
import './App.css';

// Screens: 'key-entry' → 'syncing' → 'result'
export default function App() {
  const [screen, setScreen] = useState('key-entry');
  const [walletInfo, setWalletInfo] = useState(null);

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
