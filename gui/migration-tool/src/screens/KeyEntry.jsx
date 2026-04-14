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
  const [accepted, setAccepted] = useState(false);
  const [showSpendKey, setShowSpendKey] = useState(false);
  const [showViewKey, setShowViewKey] = useState(false);

  // Strip all whitespace (spaces, tabs, line breaks) so users can paste keys
  // copied from multi-line sources like old paper wallet PDFs without having
  // to clean them up first.
  const stripWhitespace = (s) => s.replace(/\s+/g, '');

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
            onChange={(e) => setAddress(stripWhitespace(e.target.value))}
            onPaste={(e) => {
              e.preventDefault();
              setAddress(stripWhitespace(e.clipboardData.getData('text')));
            }}
            spellCheck={false}
            autoComplete="off"
          />
        </div>

        <div className="field">
          <label htmlFor="spendKey">Private Spend Key</label>
          <div className="input-with-toggle">
            <input
              id="spendKey"
              type={showSpendKey ? 'text' : 'password'}
              placeholder="64 hex characters"
              value={spendKey}
              onChange={(e) => setSpendKey(stripWhitespace(e.target.value))}
              onPaste={(e) => {
                e.preventDefault();
                setSpendKey(stripWhitespace(e.clipboardData.getData('text')));
              }}
              spellCheck={false}
              autoComplete="off"
            />
            <button
              type="button"
              className="reveal-toggle"
              onClick={() => setShowSpendKey((v) => !v)}
              aria-label={showSpendKey ? 'Hide private spend key' : 'Show private spend key'}
            >
              {showSpendKey ? 'Hide' : 'Show'}
            </button>
          </div>
        </div>

        <div className="field">
          <label htmlFor="viewKey">Private View Key</label>
          <div className="input-with-toggle">
            <input
              id="viewKey"
              type={showViewKey ? 'text' : 'password'}
              placeholder="64 hex characters"
              value={viewKey}
              onChange={(e) => setViewKey(stripWhitespace(e.target.value))}
              onPaste={(e) => {
                e.preventDefault();
                setViewKey(stripWhitespace(e.clipboardData.getData('text')));
              }}
              spellCheck={false}
              autoComplete="off"
            />
            <button
              type="button"
              className="reveal-toggle"
              onClick={() => setShowViewKey((v) => !v)}
              aria-label={showViewKey ? 'Hide private view key' : 'Show private view key'}
            >
              {showViewKey ? 'Hide' : 'Show'}
            </button>
          </div>
        </div>

        {error && <p className="error">{error}</p>}

        <label className="accept-checkbox">
          <input
            type="checkbox"
            checked={accepted}
            onChange={(e) => setAccepted(e.target.checked)}
          />
          <span>
            I understand that by proceeding, I am authorising this software to use
            my private spend key to open my legacy wallet and migrate my ETN balance
            to the Electroneum Smart Chain. My private spend key never leaves this
            device — only my view key is used to sync with a remote blockchain node
            hosted by ETN-Network. The view key confers no spending powers and is
            not used on the Smart Chain. I understand the security implications of
            managing my own private keys and have read the best practices guide
            at <a href="https://electroneum.com/best-practices/" target="_blank" rel="noopener noreferrer">electroneum.com/best-practices</a>.
          </span>
        </label>

        <button type="submit" disabled={loading || !accepted} className="btn-primary">
          {loading ? 'Opening wallet…' : 'Migrate Wallet (or check migration result)'}
        </button>

        <p className="hint" style={{ textAlign: 'center', marginTop: '10px' }}>
          If you have already migrated this wallet, clicking the button above will
          also show your migration status. Migration status is only available for
          wallets that were migrated through this app.
        </p>
      </form>

    </div>
  );
}
