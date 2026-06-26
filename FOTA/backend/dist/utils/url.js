"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.buildFirmwareDownloadUrl = buildFirmwareDownloadUrl;
const env_1 = require("../config/env");
/**
 * Build a full firmware download URL by combining SERVER_BASE_URL with the download path.
 * Ensures no duplicate or missing slashes between base URL and path.
 *
 * @param firmwareId - The firmware document ID
 * @returns Complete download URL, e.g. "http://20.243.20.10/api/firmware/download/abc123"
 */
function buildFirmwareDownloadUrl(firmwareId) {
    const baseUrl = env_1.env.SERVER_BASE_URL.replace(/\/+$/, ''); // Remove trailing slashes
    return `${baseUrl}/api/firmware/download/${firmwareId}`;
}
//# sourceMappingURL=url.js.map