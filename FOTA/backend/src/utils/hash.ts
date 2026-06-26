import crypto from 'crypto';
import fs from 'fs';
import path from 'path';

/**
 * Calculate SHA256 hash of a buffer
 */
export const hashBuffer = (buffer: Buffer): string => {
  return crypto.createHash('sha256').update(buffer).digest('hex');
};

/**
 * Calculate SHA256 hash of a file from disk
 */
export const hashFile = async (filePath: string): Promise<string> => {
  return new Promise((resolve, reject) => {
    const hash = crypto.createHash('sha256');
    const stream = fs.createReadStream(filePath);

    stream.on('data', (data) => hash.update(data));
    stream.on('end', () => resolve(hash.digest('hex')));
    stream.on('error', reject);
  });
};

/**
 * Ensure a directory exists, creating it recursively if needed
 */
export const ensureDir = (dirPath: string): void => {
  if (!fs.existsSync(dirPath)) {
    fs.mkdirSync(dirPath, { recursive: true });
  }
};

/**
 * Build the storage path for a firmware file
 */
export const buildStoragePath = (basePath: string, projectName: string, deviceModel: string): string => {
  const dir = path.join(basePath, 'firmware', projectName, deviceModel);
  ensureDir(dir);
  return dir;
};
