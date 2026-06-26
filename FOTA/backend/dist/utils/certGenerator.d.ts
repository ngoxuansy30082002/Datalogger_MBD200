/**
 * Generate self-signed SSL certificates for development HTTPS
 */
export declare const generateSelfSignedCert: () => {
    keyPath: string;
    certPath: string;
};
/**
 * Check if valid SSL certificates exist
 */
export declare const certsExist: () => boolean;
/**
 * Load SSL certificates
 */
export declare const loadCerts: () => {
    key: Buffer;
    cert: Buffer;
} | null;
//# sourceMappingURL=certGenerator.d.ts.map