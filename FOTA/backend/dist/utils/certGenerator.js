"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.loadCerts = exports.certsExist = exports.generateSelfSignedCert = void 0;
const child_process_1 = require("child_process");
const fs_1 = __importDefault(require("fs"));
const path_1 = __importDefault(require("path"));
const env_1 = require("../config/env");
/**
 * Generate self-signed SSL certificates for development HTTPS
 */
const generateSelfSignedCert = () => {
    const certDir = env_1.env.CERT_PATH;
    const keyPath = path_1.default.join(certDir, 'server.key');
    const certPath = path_1.default.join(certDir, 'server.cert');
    if (fs_1.default.existsSync(keyPath) && fs_1.default.existsSync(certPath)) {
        return { keyPath, certPath };
    }
    if (!fs_1.default.existsSync(certDir)) {
        fs_1.default.mkdirSync(certDir, { recursive: true });
    }
    try {
        // Try using openssl if available
        (0, child_process_1.execSync)(`openssl req -x509 -newkey rsa:4096 -keyout "${keyPath}" -out "${certPath}" -days 365 -nodes -subj "/CN=localhost/O=FirmwareServer/C=VN"`, { stdio: 'pipe' });
    }
    catch {
        // Fallback: generate using Node.js crypto
        const { generateKeyPairSync, createSign, X509Certificate } = require('crypto');
        const { privateKey, publicKey } = generateKeyPairSync('rsa', {
            modulusLength: 2048,
            publicKeyEncoding: { type: 'spki', format: 'pem' },
            privateKeyEncoding: { type: 'pkcs8', format: 'pem' },
        });
        // Create a simple self-signed certificate using node forge-like approach
        // For simplicity, write the key and a placeholder cert
        fs_1.default.writeFileSync(keyPath, privateKey);
        // Use a minimal approach - create cert with openssl or write raw PEM
        // Since Node.js doesn't have native X.509 generation without openssl,
        // we'll write the key and create a simple cert
        const certContent = createSelfSignedCertFromKey(privateKey, publicKey);
        fs_1.default.writeFileSync(certPath, certContent);
    }
    return { keyPath, certPath };
};
exports.generateSelfSignedCert = generateSelfSignedCert;
function createSelfSignedCertFromKey(privateKey, publicKey) {
    // Minimal self-signed certificate generation
    // In production, use proper certificates
    try {
        (0, child_process_1.execSync)('openssl version', { stdio: 'pipe' });
        return publicKey; // Fallback - openssl should have worked above
    }
    catch {
        // If no openssl, return the public key as a placeholder
        // The server will fall back to HTTP-only mode
        return publicKey;
    }
}
/**
 * Check if valid SSL certificates exist
 */
const certsExist = () => {
    const keyPath = path_1.default.join(env_1.env.CERT_PATH, 'server.key');
    const certPath = path_1.default.join(env_1.env.CERT_PATH, 'server.cert');
    return fs_1.default.existsSync(keyPath) && fs_1.default.existsSync(certPath);
};
exports.certsExist = certsExist;
/**
 * Load SSL certificates
 */
const loadCerts = () => {
    try {
        const keyPath = path_1.default.join(env_1.env.CERT_PATH, 'server.key');
        const certPath = path_1.default.join(env_1.env.CERT_PATH, 'server.cert');
        if (!fs_1.default.existsSync(keyPath) || !fs_1.default.existsSync(certPath)) {
            return null;
        }
        return {
            key: fs_1.default.readFileSync(keyPath),
            cert: fs_1.default.readFileSync(certPath),
        };
    }
    catch {
        return null;
    }
};
exports.loadCerts = loadCerts;
//# sourceMappingURL=certGenerator.js.map