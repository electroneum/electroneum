import { useState, useEffect, useRef } from 'react';
import { getMigrationStatus, getPreliminaryMigrationStatus } from '../lib/walletRpc.js';
import {
  LEGACY_EXPLORER_TX_URL,
  SMARTCHAIN_EXPLORER_ADDR_URL,
} from '../lib/constants.js';
import PaperWalletModal from '../components/PaperWalletModal.jsx';
import logo from '../assets/electroneum-logo-symbol.png';

const MIGRATION_POLL_MS = 5000;
const MIGRATION_POLL_MAX_ATTEMPTS = 720; // 1 hour of polling

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

function TxTable({ txs }) {
  return (
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
        {txs.map((tx) => (
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
  );
}

export default function Result({ ethInfo, onReset }) {
  const [migrations, setMigrations] = useState(null);
  const [prelimMigrations, setPrelimMigrations] = useState(null);
  const [error, setError] = useState('');
  const [polling, setPolling] = useState(true);
  const [showPaperWallet, setShowPaperWallet] = useState(false);
  const timerRef = useRef(null);

  useEffect(() => {
    let cancelled = false;
    let attempts = 0;

    async function poll() {
      try {
        const [scResult, prelimResult] = await Promise.all([
          getMigrationStatus(),
          getPreliminaryMigrationStatus(),
        ]);
        if (cancelled) return;
        setMigrations(scResult);
        setPrelimMigrations(prelimResult);
        setError('');
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
  const hasPrelim = prelimMigrations && prelimMigrations.length > 0;

  return (
    <div className="screen result">
      <img src={logo} alt="Electroneum" className="logo logo-sm" />
      <h2>Migration Result</h2>

      {/* Preliminary private→public migration (v2) */}
      {hasPrelim && (
        <section className="card">
          <h3>Preliminary Hardfork Migration</h3>
          <p className="hint">
            While scanning your wallet, we detected funds that predate the
            private-to-public hardfork on the legacy blockchain. To prepare for
            your Smart Chain migration, we have first migrated these funds between
            hardforks. This preliminary step typically takes around 10 minutes to
            confirm, so please be patient while you wait for your migration
            transfer to the legacy bridge address to take place — those details
            will appear in the "Smart Chain Bridge Migration Transfers" section below.
          </p>
          <TxTable txs={prelimMigrations} />
        </section>
      )}

      {/* Smart chain bridge migration status (v3) */}
      <section className="card">
        <h3>Smart Chain Bridge Migration Transfers</h3>
        <p className="hint">
          This section shows transfers from your legacy wallet to the bridge
          address on the legacy blockchain. Once these transactions have
          confirmed (which may take 10 minutes or more), the Electroneum Smart
          Chain Oracle will observe them and send your balance to your new Smart
          Chain address. You can check your Smart Chain balance using the explorer
          link in the "Your ETN Smart Chain Address" section below.
        </p>
        {migrations === null && !error && <p className="loading">Loading…</p>}
        {error && <p className="error">{error}</p>}
        {migrations !== null && !hasMigrated && polling && (
          <div className="status pending">
            <span className="status-icon">⏳</span>
            <div>
              <strong>Checking for migration...</strong>
              <p>
                Your wallet has synced. If you have a balance, the migration
                transaction will be sent automatically.
                {hasPrelim
                  ? ' The preliminary hardfork migration above must confirm first — this may take around 10 minutes.'
                  : ' This may take a moment.'}
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

        {hasMigrated && <TxTable txs={migrations} />}
      </section>

      {/* ETN Smart Chain address */}
      <section className="card">
        <h3>Your ETN Smart Chain Address</h3>
        <p className="hint">
          This address was derived from your private spend key using secp256k1.
          It is your EVM-compatible address on the Electroneum Smart Chain.
          Once the Oracle has observed your confirmed bridge transfers, your
          balance will be sent to this address. Please be patient — this process
          may take some time. Use the explorer link below to check your balance.
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
          Keep this secret. You can copy the key above and manually import it into MetaMask
          or another compatible wallet via "Import Account", or use the "Download Smart Chain
          Paper Wallet" button below to generate a scannable QR code for importing.
        </p>
        <RevealField label="Private key (hex)" value={ethInfo.privateKey} />
      </section>

      <button
        className="btn-primary"
        style={{ width: '100%' }}
        onClick={() => setShowPaperWallet(true)}
      >
        Download Smart Chain Paper Wallet
      </button>

      <button className="btn-secondary" onClick={onReset}>
        Check another wallet
      </button>

      {showPaperWallet && (
        <PaperWalletModal
          address={ethInfo.address}
          privateKey={ethInfo.privateKey}
          onClose={() => setShowPaperWallet(false)}
        />
      )}
    </div>
  );
}
