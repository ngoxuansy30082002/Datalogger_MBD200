/**
 * Calculate SHA256 hash of a buffer
 */
export declare const hashBuffer: (buffer: Buffer) => string;
/**
 * Calculate SHA256 hash of a file from disk
 */
export declare const hashFile: (filePath: string) => Promise<string>;
/**
 * Ensure a directory exists, creating it recursively if needed
 */
export declare const ensureDir: (dirPath: string) => void;
/**
 * Build the storage path for a firmware file
 */
export declare const buildStoragePath: (basePath: string, projectName: string, deviceModel: string) => string;
//# sourceMappingURL=hash.d.ts.map