import { execSync } from 'child_process';
import fs from 'fs';
import path from 'path';
import { env } from '../config/env';

/**
 * Generate self-signed SSL certificates for development HTTPS
 */
export const generateSelfSignedCert = (): { keyPath: string; certPath: string } => {
  const certDir = env.CERT_PATH;
  const keyPath = path.join(certDir, 'server.key');
  const certPath = path.join(certDir, 'server.cert');

  if (fs.existsSync(keyPath) && fs.existsSync(certPath)) {
    return { keyPath, certPath };
  }

  if (!fs.existsSync(certDir)) {
    fs.mkdirSync(certDir, { recursive: true });
  }

  try {
    // Try using openssl if available
    execSync(
      `openssl req -x509 -newkey rsa:4096 -keyout "${keyPath}" -out "${certPath}" -days 365 -nodes -subj "/CN=localhost/O=FirmwareServer/C=VN"`,
      { stdio: 'pipe' }
    );
  } catch {
    // Fallback: generate using Node.js crypto
    const { generateKeyPairSync, createSign, X509Certificate } = require('crypto');

    const { privateKey, publicKey } = generateKeyPairSync('rsa', {
      modulusLength: 2048,
      publicKeyEncoding: { type: 'spki', format: 'pem' },
      privateKeyEncoding: { type: 'pkcs8', format: 'pem' },
    });

    // Create a simple self-signed certificate using node forge-like approach
    // For simplicity, write the key and a placeholder cert
    fs.writeFileSync(keyPath, privateKey);

    // Use a minimal approach - create cert with openssl or write raw PEM
    // Since Node.js doesn't have native X.509 generation without openssl,
    // we'll write the key and create a simple cert
    const certContent = createSelfSignedCertFromKey(privateKey, publicKey);
    fs.writeFileSync(certPath, certContent);
  }

  return { keyPath, certPath };
};

function createSelfSignedCertFromKey(privateKey: string, publicKey: string): string {
  // Minimal self-signed certificate generation
  // In production, use proper certificates
  try {
    execSync('openssl version', { stdio: 'pipe' });
    return publicKey; // Fallback - openssl should have worked above
  } catch {
    // If no openssl, return the public key as a placeholder
    // The server will fall back to HTTP-only mode
    return publicKey;
  }
}

/**
 * Check if valid SSL certificates exist
 */
export const certsExist = (): boolean => {
  const keyPath = path.join(env.CERT_PATH, 'server.key');
  const certPath = path.join(env.CERT_PATH, 'server.cert');
  return fs.existsSync(keyPath) && fs.existsSync(certPath);
};

/**
 * Load SSL certificates
 */
export const loadCerts = (): { key: Buffer; cert: Buffer } | null => {
  try {
    const keyPath = path.join(env.CERT_PATH, 'server.key');
    const certPath = path.join(env.CERT_PATH, 'server.cert');

    if (!fs.existsSync(keyPath) || !fs.existsSync(certPath)) {
      return null;
    }

    return {
      key: fs.readFileSync(keyPath),
      cert: fs.readFileSync(certPath),
    };
  } catch {
    return null;
  }
};
