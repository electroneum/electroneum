import { secp256k1 } from 'ethereum-cryptography/secp256k1.js';
import { keccak256 } from 'ethereum-cryptography/keccak.js';

/**
 * Derives the ETN smart chain (EVM-compatible) address and reuses the spend key
 * as the new-chain private key.
 *
 * Algorithm:
 *   1. Interpret the 32-byte private spend key as a secp256k1 scalar.
 *   2. Derive the uncompressed public key (65 bytes, starts with 0x04).
 *   3. Strip the 0x04 prefix → 64 bytes.
 *   4. keccak256(64 bytes) → 32-byte digest.
 *   5. Take the last 20 bytes → Ethereum-style address.
 *
 * @param {string} spendKeyHex - 64 hex-char (32 byte) private spend key
 * @returns {{ address: string, privateKey: string }}
 */
export function deriveEthAddress(spendKeyHex) {
  const privateKeyBytes = hexToBytes(spendKeyHex);
  if (privateKeyBytes.length !== 32) {
    throw new Error('Private spend key must be 32 bytes (64 hex characters)');
  }

  // Uncompressed public key: 04 || x (32 bytes) || y (32 bytes) = 65 bytes
  const pubKeyUncompressed = secp256k1.getPublicKey(privateKeyBytes, false);

  // Strip the 0x04 prefix → 64 bytes
  const pubKeyBody = pubKeyUncompressed.slice(1);

  // keccak256 over the 64-byte body
  const hash = keccak256(pubKeyBody);

  // Last 20 bytes of the hash
  const addressBytes = hash.slice(-20);
  const address = '0x' + bytesToHex(addressBytes);
  const privateKey = '0x' + spendKeyHex.toLowerCase().replace(/^0x/, '');

  return { address, privateKey };
}

function hexToBytes(hex) {
  const clean = hex.startsWith('0x') ? hex.slice(2) : hex;
  const bytes = new Uint8Array(clean.length / 2);
  for (let i = 0; i < bytes.length; i++) {
    bytes[i] = parseInt(clean.slice(i * 2, i * 2 + 2), 16);
  }
  return bytes;
}

function bytesToHex(bytes) {
  return Array.from(bytes).map((b) => b.toString(16).padStart(2, '0')).join('');
}
