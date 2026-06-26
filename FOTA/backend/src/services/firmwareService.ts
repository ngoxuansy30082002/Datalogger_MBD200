import path from 'path';
import fs from 'fs';
import { firmwareRepository } from '../repositories/firmwareRepository';
import { downloadLogRepository } from '../repositories/downloadLogRepository';
import { IFirmwareDocument } from '../models/Firmware';
import { FirmwareFilter, FirmwareUpdateResponse } from '../types';
import { hashBuffer, buildStoragePath } from '../utils/hash';
import { isNewerVersion } from '../utils/version';
import { buildFirmwareDownloadUrl } from '../utils/url';
import { env } from '../config/env';
import { logger } from './logService';

export class FirmwareService {
  async uploadFirmware(
    file: Express.Multer.File,
    metadata: {
      projectName: string;
      deviceModel: string;
      hardwareVersion: string;
      firmwareVersion: string;
      description: string;
      uploadedBy: string;
    }
  ): Promise<IFirmwareDocument> {
    // Validate file extension
    if (!file.originalname.toLowerCase().endsWith('.hex')) {
      throw new Error('Only .hex files are allowed');
    }

    // Calculate SHA256 hash
    const fileHash = hashBuffer(file.buffer);

    // Check for duplicate hash
    const existing = await firmwareRepository.findByHash(fileHash);
    if (existing) {
      throw new Error(`Duplicate file detected. This file already exists as: ${existing.fileName}`);
    }

    // Check for duplicate version
    const duplicateVersion = await firmwareRepository.findByModelAndVersion(
      metadata.deviceModel,
      metadata.hardwareVersion,
      metadata.firmwareVersion
    );
    if (duplicateVersion) {
      throw new Error(
        `Firmware version ${metadata.firmwareVersion} already exists for ${metadata.deviceModel} HW${metadata.hardwareVersion}`
      );
    }

    // Build storage path and save file
    const storageDir = buildStoragePath(env.STORAGE_PATH, metadata.projectName, metadata.deviceModel);
    const fileName = `${metadata.deviceModel}_v${metadata.firmwareVersion}.hex`;
    const filePath = path.join(storageDir, fileName);
    const storagePath = `/firmware/${metadata.projectName}/${metadata.deviceModel}/${fileName}`;

    fs.writeFileSync(filePath, file.buffer);

    // Save metadata
    const firmware = await firmwareRepository.create({
      ...metadata,
      fileName,
      fileSize: file.size,
      fileHash,
      storagePath,
      uploadTime: new Date(),
      downloadCount: 0,
      isLatest: false,
    } as Partial<IFirmwareDocument>);

    logger.upload.info(`Firmware uploaded: ${fileName}`, {
      projectName: metadata.projectName,
      deviceModel: metadata.deviceModel,
      version: metadata.firmwareVersion,
      size: file.size,
      hash: fileHash,
      uploadedBy: metadata.uploadedBy,
    });

    return firmware;
  }

  async deleteFirmware(id: string, deletedBy: string): Promise<void> {
    const firmware = await firmwareRepository.findById(id);
    if (!firmware) {
      throw new Error('Firmware not found');
    }

    // Delete file from disk
    const filePath = path.join(env.STORAGE_PATH, firmware.storagePath);
    if (fs.existsSync(filePath)) {
      fs.unlinkSync(filePath);
    }

    await firmwareRepository.delete(id);

    logger.upload.info(`Firmware deleted: ${firmware.fileName}`, {
      id,
      deletedBy,
    });
  }

  async downloadFirmware(
    id: string,
    ipAddress: string,
    userAgent: string
  ): Promise<{ filePath: string; fileName: string }> {
    const firmware = await firmwareRepository.findById(id);
    if (!firmware) {
      throw new Error('Firmware not found');
    }

    const filePath = path.join(env.STORAGE_PATH, firmware.storagePath);
    if (!fs.existsSync(filePath)) {
      throw new Error('Firmware file not found on disk');
    }

    // Increment download count
    await firmwareRepository.incrementDownloadCount(id);

    // Log download
    await downloadLogRepository.create({
      firmwareId: firmware._id,
      ipAddress,
      userAgent,
      timestamp: new Date(),
    } as any);

    logger.download.info(`Firmware downloaded: ${firmware.fileName}`, {
      id,
      ipAddress,
    });

    return { filePath, fileName: firmware.fileName };
  }

  async downloadByQuery(
    deviceModel: string,
    hardwareVersion: string,
    version: string,
    ipAddress: string,
    userAgent: string
  ): Promise<{ filePath: string; fileName: string }> {
    let firmware: IFirmwareDocument | null;

    if (version === 'latest') {
      firmware = await firmwareRepository.findLatest(deviceModel, hardwareVersion);
      if (!firmware) {
        firmware = await firmwareRepository.getNewestForDevice(deviceModel, hardwareVersion);
      }
    } else {
      firmware = await firmwareRepository.findByModelAndVersion(
        deviceModel,
        hardwareVersion,
        version
      );
    }

    if (!firmware) {
      throw new Error('Firmware not found');
    }

    return this.downloadFirmware(String(firmware._id), ipAddress, userAgent);
  }

  async markAsLatest(id: string): Promise<IFirmwareDocument> {
    const firmware = await firmwareRepository.findById(id);
    if (!firmware) {
      throw new Error('Firmware not found');
    }

    // Unset previous latest for the same device model and hardware version
    await firmwareRepository.unsetLatest(firmware.deviceModel, firmware.hardwareVersion);

    // Set this one as latest
    const updated = await firmwareRepository.update(id, { isLatest: true });
    if (!updated) {
      throw new Error('Failed to update firmware');
    }

    logger.upload.info(`Firmware marked as latest: ${firmware.fileName}`, { id });

    return updated;
  }

  async checkForUpdate(
    deviceModel: string,
    hardwareVersion: string,
    currentVersion: string
  ): Promise<FirmwareUpdateResponse> {
    // First check isLatest flag
    let latest = await firmwareRepository.findLatest(deviceModel, hardwareVersion);

    // If no isLatest, find the newest by semver
    if (!latest) {
      latest = await firmwareRepository.getNewestForDevice(deviceModel, hardwareVersion);
    }

    if (!latest) {
      return { hasUpdate: false };
    }

    const hasUpdate = isNewerVersion(currentVersion, latest.firmwareVersion);

    return {
      hasUpdate,
      latestVersion: latest.firmwareVersion,
      downloadUrl: buildFirmwareDownloadUrl(String(latest._id)),
      releaseNote: latest.description,
    };
  }

  async listFirmware(filter: FirmwareFilter) {
    return firmwareRepository.findAll(filter);
  }

  async getFirmwareById(id: string) {
    return firmwareRepository.findById(id);
  }
}

export const firmwareService = new FirmwareService();
