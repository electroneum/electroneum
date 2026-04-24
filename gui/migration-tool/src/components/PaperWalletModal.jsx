import { useState } from 'react';

function getPasswordStrength(pw) {
  if (!pw) return { label: '', level: 0 };
  let score = 0;
  if (pw.length >= 8) score++;
  if (pw.length >= 12) score++;
  if (/[a-z]/.test(pw) && /[A-Z]/.test(pw)) score++;
  if (/\d/.test(pw)) score++;
  if (/[^a-zA-Z0-9]/.test(pw)) score++;

  if (score <= 2) return { label: 'Weak', level: 1 };
  if (score <= 3) return { label: 'Medium', level: 2 };
  return { label: 'Strong', level: 3 };
}

export default function PaperWalletModal({ address, privateKey, onClose }) {
  const [password, setPassword] = useState('');
  const [confirm, setConfirm] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [status, setStatus] = useState('idle'); // idle | generating | success | error
  const [error, setError] = useState('');

  const strength = getPasswordStrength(password);
  const hasLetter = /[a-zA-Z]/.test(password);
  const hasNumber = /\d/.test(password);
  const longEnough = password.length >= 8;
  const passwordsMatch = password === confirm;
  const valid = longEnough && hasLetter && hasNumber && passwordsMatch && confirm.length > 0;

  async function handleGenerate() {
    if (!valid) return;
    setStatus('generating');
    setError('');

    try {
      const result = await window.electronAPI.generatePaperWallet({
        address,
        privateKey,
        password,
      });

      if (result.cancelled) {
        setStatus('idle');
        return;
      }

      if (!result.ok) {
        setError(result.error || 'Failed to generate PDF');
        setStatus('error');
        return;
      }

      setStatus('success');
      setTimeout(() => onClose(), 2000);
    } catch (err) {
      setError(err.message || 'Unexpected error');
      setStatus('error');
    }
  }

  return (
    <div className="modal-backdrop" onClick={onClose}>
      <div className="modal-container" onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <h3>Download Paper Wallet</h3>
          <button className="modal-close" onClick={onClose}>&times;</button>
        </div>

        {status === 'success' ? (
          <div className="modal-success">
            <span className="modal-success-icon">&#10003;</span>
            <p>Paper wallet saved successfully!</p>
          </div>
        ) : (
          <>
            <p className="modal-hint">
              Set a password to encrypt your paper wallet PDF. You will need this
              password to open the file.
            </p>

            <div className="modal-field">
              <label>Password</label>
              <div className="modal-input-row">
                <input
                  type={showPassword ? 'text' : 'password'}
                  value={password}
                  onChange={(e) => setPassword(e.target.value)}
                  placeholder="Min 8 characters, letter + number"
                  disabled={status === 'generating'}
                />
                <button
                  className="btn-reveal"
                  type="button"
                  onClick={() => setShowPassword((v) => !v)}
                >
                  {showPassword ? 'Hide' : 'Show'}
                </button>
              </div>
              {password.length > 0 && (
                <div className="password-strength">
                  <div className={`strength-bar level-${strength.level}`} />
                  <span className={`strength-label level-${strength.level}`}>
                    {strength.label}
                  </span>
                </div>
              )}
              {password.length > 0 && !longEnough && (
                <span className="modal-validation">At least 8 characters required</span>
              )}
              {password.length > 0 && longEnough && !hasLetter && (
                <span className="modal-validation">Must contain a letter</span>
              )}
              {password.length > 0 && longEnough && hasLetter && !hasNumber && (
                <span className="modal-validation">Must contain a number</span>
              )}
            </div>

            <div className="modal-field">
              <label>Confirm Password</label>
              <input
                type={showPassword ? 'text' : 'password'}
                value={confirm}
                onChange={(e) => setConfirm(e.target.value)}
                placeholder="Re-enter password"
                disabled={status === 'generating'}
              />
              {confirm.length > 0 && !passwordsMatch && (
                <span className="modal-validation">Passwords do not match</span>
              )}
            </div>

            {error && <p className="modal-error">{error}</p>}

            <div className="modal-actions">
              <button className="btn-secondary" onClick={onClose} disabled={status === 'generating'}>
                Cancel
              </button>
              <button
                className="btn-primary"
                onClick={handleGenerate}
                disabled={!valid || status === 'generating'}
              >
                {status === 'generating' ? 'Generating…' : 'Generate & Save PDF'}
              </button>
            </div>
          </>
        )}
      </div>
    </div>
  );
}
