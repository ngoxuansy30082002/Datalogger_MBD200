"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.buildStoragePath = exports.ensureDir = exports.hashFile = exports.hashBuffer = void 0;
const crypto_1 = __importDefault(require("crypto"));
const fs_1 = __importDefault(require("fs"));
const path_1 = __importDefault(require("path"));
/**
 * Calculate SHA256 hash of a buffer
 */
const hashBuffer = (buffer) => {
    return crypto_1.default.createHash('sha256').update(buffer).digest('hex');
};
exports.hashBuffer = hashBuffer;
/**
 * Calculate SHA256 hash of a file from disk
 */
const hashFile = async (filePath) => {
    return new Promise((resolve, reject) => {
        const hash = crypto_1.default.createHash('sha256');
        const stream = fs_1.default.createReadStream(filePath);
        stream.on('data', (data) => hash.update(data));
        stream.on('end', () => resolve(hash.digest('hex')));
        stream.on('error', reject);
    });
};
exports.hashFile = hashFile;
/**
 * Ensure a directory exists, creating it recursively if needed
 */
const ensureDir = (dirPath) => {
    if (!fs_1.default.existsSync(dirPath)) {
        fs_1.default.mkdirSync(dirPath, { recursive: true });
    }
};
exports.ensureDir = ensureDir;
/**
 * Build the storage path for a firmware file
 */
const buildStoragePath = (basePath, projectName, deviceModel) => {
    const dir = path_1.default.join(basePath, 'firmware', projectName, deviceModel);
    (0, exports.ensureDir)(dir);
    return dir;
};
exports.buildStoragePath = buildStoragePath;
//# sourceMappingURL=hash.js.map