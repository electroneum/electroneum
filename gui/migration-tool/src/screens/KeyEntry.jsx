import { useState } from 'react';
import { generateFromKeys } from '../lib/walletRpc.js';
import { deriveEthAddress } from '../lib/deriveEthAddress.js';
import logo from '../assets/electroneum-logo-symbol.png';

export default function KeyEntry({ onSuccess }) {
  const [address, setAddress] = useState('');
  const [spendKey, setSpendKey] = useState('');
  const [viewKey, setViewKey] = useState('');
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);

  function validate() {
    if (!address.trim().startsWith('etn')) {
      return 'Address must start with "etn"';
    }
    if (!/^[0-9a-fA-F]{64}$/.test(spendKey.trim())) {
      return 'Private spend key must be 64 hex characters';
    }
    if (!/^[0-9a-fA-F]{64}$/.test(viewKey.trim())) {
      return 'Private view key must be 64 hex characters';
    }
    return null;
  }

  async function handleSubmit(e) {
    e.preventDefault();
    setError('');

    const validationError = validate();
    if (validationError) {
      setError(validationError);
      return;
    }

    setLoading(true);
    try {
      await generateFromKeys(address.trim(), spendKey.trim(), viewKey.trim());
      const ethInfo = deriveEthAddress(spendKey.trim());
      onSuccess({ address: address.trim(), spendKey: spendKey.trim(), ethInfo });
    } catch (err) {
      setError(err.message || 'Failed to create wallet');
    } finally {
      setLoading(false);
    }
  }

  return (
    <div className="screen key-entry">
      <div className="logo-area">
        <img src={logo} alt="Electroneum" className="logo" />
        <h1>ETN Migration Tool</h1>
        <p className="subtitle">Migrate your legacy wallet to the Electroneum Smart Chain</p>
      </div>

      <form onSubmit={handleSubmit} className="key-form">
        <div className="field">
          <label htmlFor="address">ETN Address</label>
          <input
            id="address"
            type="text"
            placeholder="etnk..."
            value={address}
            onChange={(e) => setAddress(e.target.value)}
            spellCheck={false}
            autoComplete="off"
          />
        </div>

        <div className="field">
          <label htmlFor="spendKey">Private Spend Key</label>
          <input
            id="spendKey"
            type="password"
            placeholder="64 hex characters"
            value={spendKey}
            onChange={(e) => setSpendKey(e.target.value)}
            spellCheck={false}
            autoComplete="off"
          />
        </div>

        <div className="field">
          <label htmlFor="viewKey">Private View Key</label>
          <input
            id="viewKey"
            type="password"
            placeholder="64 hex characters"
            value={viewKey}
            onChange={(e) => setViewKey(e.target.value)}
            spellCheck={false}
            autoComplete="off"
          />
        </div>

        {error && <p className="error">{error}</p>}

        <button type="submit" disabled={loading} className="btn-primary">
          {loading ? 'Opening wallet…' : 'Migrate Wallet (or check migration result)'}
        </button>

        <p className="hint" style={{ textAlign: 'center', marginTop: '10px' }}>
          If you have already migrated this wallet, clicking the button above will
          also show your migration status. Migration status is only available for
          wallets that were migrated through this app.
        </p>
      </form>

      <p className="disclaimer">
        Your private spend key never leaves your device. A remote blockchain node hosted by Electroneum is used
        to sync your wallet.
      </p>
    </div>
  );
}
