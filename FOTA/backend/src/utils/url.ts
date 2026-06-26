import { env } from '../config/env';

/**
 * Build a full firmware download URL by combining SERVER_BASE_URL with the download path.
 * Ensures no duplicate or missing slashes between base URL and path.
 *
 * @param firmwareId - The firmware document ID
 * @returns Complete download URL, e.g. "http://20.243.20.10/api/firmware/download/abc123"
 */
export function buildFirmwareDownloadUrl(firmwareId: string): string {
  const baseUrl = env.SERVER_BASE_URL.replace(/\/+$/, ''); // Remove trailing slashes
  return `${baseUrl}/api/firmware/download/${firmwareId}`;
}
