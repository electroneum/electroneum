import { useState, useEffect, useRef } from 'react';
import { getMigrationStatus } from '../lib/walletRpc.js';
import {
  LEGACY_EXPLORER_TX_URL,
  SMARTCHAIN_EXPLORER_ADDR_URL,
} from '../lib/constants.js';
import logo from '../assets/electroneum-logo-symbol.png';

const MIGRATION_POLL_MS = 5000;
const MIGRATION_POLL_MAX_ATTEMPTS = 60; // 5 minutes of polling

function CopyButton({ text }) {
  const [copied, setCopied] = useState(false);

  async function handleCopy() {
    try {
      await navigator.clipboard.writeText(text);
      setCopied(true);
      setTimeout(() => setCopied(false), 2000);
    } catch {
      // clipboard not available (sandboxed) — ignore
    }
  }

  return (
    <button className="btn-copy" onClick={handleCopy}>
      {copied ? 'Copied!' : 'Copy'}
    </button>
  );
}

function RevealField({ label, value }) {
  const [visible, setVisible] = useState(false);
  return (
    <div className="reveal-field">
      <div className="reveal-header">
        <span className="reveal-label">{label}</span>
        <button className="btn-reveal" onClick={() => setVisible((v) => !v)}>
          {visible ? 'Hide' : 'Show'}
        </button>
        <CopyButton text={value} />
      </div>
      <div className={`reveal-value ${visible ? 'visible' : 'hidden'}`}>
        {visible ? value : '••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••'}
      </div>
    </div>
  );
}

export default function Result({ ethInfo, onReset }) {
  const [migrations, setMigrations] = useState(null);
  const [error, setError] = useState('');
  const [polling, setPolling] = useState(true);
  const timerRef = useRef(null);

  useEffect(() => {
    let cancelled = false;
    let attempts = 0;

    async function poll() {
      try {
        const result = await getMigrationStatus();
        if (cancelled) return;
        setMigrations(result);
        setError('');

        // If migration found, stop polling
        if (result && result.length > 0) {
          setPolling(false);
          return;
        }
      } catch (err) {
        if (!cancelled) setError(err.message);
      }

      attempts++;
      if (!cancelled && attempts < MIGRATION_POLL_MAX_ATTEMPTS) {
        timerRef.current = setTimeout(poll, MIGRATION_POLL_MS);
      } else if (!cancelled) {
        setPolling(false);
      }
    }

    poll();
    return () => {
      cancelled = true;
      clearTimeout(timerRef.current);
    };
  }, []);

  const hasMigrated = migrations && migrations.length > 0;

  return (
    <div className="screen result">
      <img src={logo} alt="Electroneum" className="logo logo-sm" />
      <h2>Migration Result</h2>

      {/* Migration status */}
      <section className="card">
        <h3>Legacy Chain Migration</h3>
        {migrations === null && !error && <p className="loading">Loading…</p>}
        {error && <p className="error">{error}</p>}
        {migrations !== null && !hasMigrated && polling && (
          <div className="status pending">
            <span className="status-icon">⏳</span>
            <div>
              <strong>Checking for migration...</strong>
              <p>
                Your wallet has synced. If you have a balance, the migration
                transaction will be sent automatically. This may take a moment.
              </p>
            </div>
          </div>
        )}
        {migrations !== null && !hasMigrated && !polling && (
          <div className="status pending">
            <span className="status-icon">⏳</span>
            <div>
              <strong>No migration found</strong>
              <p>
                No smart chain migration transaction was found for this wallet.
                This could mean the wallet has no balance, or the migration has
                not yet been processed. You can leave the app open and it will
                continue checking.
              </p>
            </div>
          </div>
        )}
        {hasMigrated && (
          <div className="status confirmed">
            <span className="status-icon">✓</span>
            <div>
              <strong>Migration confirmed</strong>
              <p>{migrations.length} migration transaction{migrations.length > 1 ? 's' : ''} found.</p>
            </div>
          </div>
        )}

        {hasMigrated && (
          <table className="tx-table">
            <thead>
              <tr>
                <th>TX ID</th>
                <th>Amount (ETN)</th>
                <th>Confirmations</th>
                <th>Explorer</th>
              </tr>
            </thead>
            <tbody>
              {migrations.map((tx) => (
                <tr key={tx.txid}>
                  <td className="txid">{tx.txid.slice(0, 16)}…</td>
                  <td>{(tx.amount / 1e2).toFixed(2)}</td>
                  <td>{tx.confirmations ?? '—'}</td>
                  <td>
                    <a
                      href={`${LEGACY_EXPLORER_TX_URL}${tx.txid}`}
                      target="_blank"
                      rel="noopener noreferrer"
                      className="link"
                    >
                      View
                    </a>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </section>

      {/* ETN Smart Chain address */}
      <section className="card">
        <h3>Your ETN Smart Chain Address</h3>
        <p className="hint">
          This address was derived from your private spend key using secp256k1.
          It is your EVM-compatible address on the Electroneum Smart Chain.
        </p>

        <div className="address-row">
          <span className="address-value">{ethInfo.address}</span>
          <CopyButton text={ethInfo.address} />
          <a
            href={`${SMARTCHAIN_EXPLORER_ADDR_URL}${ethInfo.address}`}
            target="_blank"
            rel="noopener noreferrer"
            className="link"
          >
            View on Explorer
          </a>
        </div>
      </section>

      {/* New chain private key */}
      <section className="card">
        <h3>New Chain Private Key</h3>
        <p className="hint">
          Your private spend key doubles as your private key on the Electroneum Smart Chain.
          Keep this secret and import it into a compatible wallet (e.g. MetaMask) to access
          your funds.
        </p>
        <RevealField label="Private key (hex)" value={ethInfo.privateKey} />
      </section>

      <button className="btn-secondary" onClick={onReset}>
        Check another wallet
      </button>
    </div>
  );
}
