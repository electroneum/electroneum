import { useState, useEffect, useRef } from 'react';
import { getSyncStatus } from '../lib/walletRpc.js';
import { POLL_INTERVAL_MS, SYNC_THRESHOLD } from '../lib/constants.js';

export default function Syncing({ onSynced }) {
  const [walletHeight, setWalletHeight] = useState(0);
  const [daemonHeight, setDaemonHeight] = useState(0);
  const [error, setError] = useState('');
  const timerRef = useRef(null);

  const progress =
    daemonHeight > 0
      ? Math.min(100, Math.round((walletHeight / daemonHeight) * 100))
      : null; // null = daemon height unknown

  const isSynced = daemonHeight > 0 && daemonHeight - walletHeight < SYNC_THRESHOLD;

  useEffect(() => {
    let cancelled = false;

    async function poll() {
      try {
        const { walletHeight: wh, daemonHeight: dh } = await getSyncStatus();
        if (cancelled) return;
        setWalletHeight(wh);
        setDaemonHeight(dh);
        setError('');

        if (dh > 0 && dh - wh < SYNC_THRESHOLD) {
          onSynced();
          return;
        }
      } catch (err) {
        if (!cancelled) setError(err.message);
      }

      if (!cancelled) {
        timerRef.current = setTimeout(poll, POLL_INTERVAL_MS);
      }
    }

    poll();
    return () => {
      cancelled = true;
      clearTimeout(timerRef.current);
    };
  }, [onSynced]);

  return (
    <div className="screen syncing">
      <h2>Scanning the Blockchain</h2>
      <p className="subtitle">
        Scanning the legacy blockchain. This may take a few minutes.
      </p>

      <div className="progress-container">
        <div className="progress-bar">
          <div className="progress-fill" style={{ width: `${progress ?? 0}%` }} />
        </div>
        <span className="progress-label">
          {progress === null ? '…' : `${progress}%`}
        </span>
      </div>

      <div className="height-info">
        <div className="height-item">
          <span className="height-label">Wallet height</span>
          <span className="height-value">{walletHeight.toLocaleString()}</span>
        </div>
        <div className="height-item">
          <span className="height-label">Chain tip</span>
          <span className="height-value">{daemonHeight.toLocaleString()}</span>
        </div>
        <div className="height-item">
          <span className="height-label">Remaining</span>
          <span className="height-value">
            {daemonHeight > 0
              ? Math.max(0, daemonHeight - walletHeight).toLocaleString()
              : '—'}
          </span>
        </div>
      </div>

      {isSynced && (
        <p className="synced-msg">Sync complete. Loading results…</p>
      )}

      {error && (
        <p className="error">
          Network error: {error}. Retrying…
        </p>
      )}
    </div>
  );
}
