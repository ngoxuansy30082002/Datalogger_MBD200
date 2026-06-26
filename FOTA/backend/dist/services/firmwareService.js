"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.firmwareService = exports.FirmwareService = void 0;
const path_1 = __importDefault(require("path"));
const fs_1 = __importDefault(require("fs"));
const firmwareRepository_1 = require("../repositories/firmwareRepository");
const downloadLogRepository_1 = require("../repositories/downloadLogRepository");
const hash_1 = require("../utils/hash");
const version_1 = require("../utils/version");
const url_1 = require("../utils/url");
const env_1 = require("../config/env");
const logService_1 = require("./logService");
class FirmwareService {
    async uploadFirmware(file, metadata) {
        // Validate file extension
        if (!file.originalname.toLowerCase().endsWith('.hex')) {
            throw new Error('Only .hex files are allowed');
        }
        // Calculate SHA256 hash
        const fileHash = (0, hash_1.hashBuffer)(file.buffer);
        // Check for duplicate hash
        const existing = await firmwareRepository_1.firmwareRepository.findByHash(fileHash);
        if (existing) {
            throw new Error(`Duplicate file detected. This file already exists as: ${existing.fileName}`);
        }
        // Check for duplicate version
        const duplicateVersion = await firmwareRepository_1.firmwareRepository.findByModelAndVersion(metadata.deviceModel, metadata.hardwareVersion, metadata.firmwareVersion);
        if (duplicateVersion) {
            throw new Error(`Firmware version ${metadata.firmwareVersion} already exists for ${metadata.deviceModel} HW${metadata.hardwareVersion}`);
        }
        // Build storage path and save file
        const storageDir = (0, hash_1.buildStoragePath)(env_1.env.STORAGE_PATH, metadata.projectName, metadata.deviceModel);
        const fileName = `${metadata.deviceModel}_v${metadata.firmwareVersion}.hex`;
        const filePath = path_1.default.join(storageDir, fileName);
        const storagePath = `/firmware/${metadata.projectName}/${metadata.deviceModel}/${fileName}`;
        fs_1.default.writeFileSync(filePath, file.buffer);
        // Save metadata
        const firmware = await firmwareRepository_1.firmwareRepository.create({
            ...metadata,
            fileName,
            fileSize: file.size,
            fileHash,
            storagePath,
            uploadTime: new Date(),
            downloadCount: 0,
            isLatest: false,
        });
        logService_1.logger.upload.info(`Firmware uploaded: ${fileName}`, {
            projectName: metadata.projectName,
            deviceModel: metadata.deviceModel,
            version: metadata.firmwareVersion,
            size: file.size,
            hash: fileHash,
            uploadedBy: metadata.uploadedBy,
        });
        return firmware;
    }
    async deleteFirmware(id, deletedBy) {
        const firmware = await firmwareRepository_1.firmwareRepository.findById(id);
        if (!firmware) {
            throw new Error('Firmware not found');
        }
        // Delete file from disk
        const filePath = path_1.default.join(env_1.env.STORAGE_PATH, firmware.storagePath);
        if (fs_1.default.existsSync(filePath)) {
            fs_1.default.unlinkSync(filePath);
        }
        await firmwareRepository_1.firmwareRepository.delete(id);
        logService_1.logger.upload.info(`Firmware deleted: ${firmware.fileName}`, {
            id,
            deletedBy,
        });
    }
    async downloadFirmware(id, ipAddress, userAgent) {
        const firmware = await firmwareRepository_1.firmwareRepository.findById(id);
        if (!firmware) {
            throw new Error('Firmware not found');
        }
        const filePath = path_1.default.join(env_1.env.STORAGE_PATH, firmware.storagePath);
        if (!fs_1.default.existsSync(filePath)) {
            throw new Error('Firmware file not found on disk');
        }
        // Increment download count
        await firmwareRepository_1.firmwareRepository.incrementDownloadCount(id);
        // Log download
        await downloadLogRepository_1.downloadLogRepository.create({
            firmwareId: firmware._id,
            ipAddress,
            userAgent,
            timestamp: new Date(),
        });
        logService_1.logger.download.info(`Firmware downloaded: ${firmware.fileName}`, {
            id,
            ipAddress,
        });
        return { filePath, fileName: firmware.fileName };
    }
    async downloadByQuery(deviceModel, hardwareVersion, version, ipAddress, userAgent) {
        let firmware;
        if (version === 'latest') {
            firmware = await firmwareRepository_1.firmwareRepository.findLatest(deviceModel, hardwareVersion);
            if (!firmware) {
                firmware = await firmwareRepository_1.firmwareRepository.getNewestForDevice(deviceModel, hardwareVersion);
            }
        }
        else {
            firmware = await firmwareRepository_1.firmwareRepository.findByModelAndVersion(deviceModel, hardwareVersion, version);
        }
        if (!firmware) {
            throw new Error('Firmware not found');
        }
        return this.downloadFirmware(String(firmware._id), ipAddress, userAgent);
    }
    async markAsLatest(id) {
        const firmware = await firmwareRepository_1.firmwareRepository.findById(id);
        if (!firmware) {
            throw new Error('Firmware not found');
        }
        // Unset previous latest for the same device model and hardware version
        await firmwareRepository_1.firmwareRepository.unsetLatest(firmware.deviceModel, firmware.hardwareVersion);
        // Set this one as latest
        const updated = await firmwareRepository_1.firmwareRepository.update(id, { isLatest: true });
        if (!updated) {
            throw new Error('Failed to update firmware');
        }
        logService_1.logger.upload.info(`Firmware marked as latest: ${firmware.fileName}`, { id });
        return updated;
    }
    async checkForUpdate(deviceModel, hardwareVersion, currentVersion) {
        // First check isLatest flag
        let latest = await firmwareRepository_1.firmwareRepository.findLatest(deviceModel, hardwareVersion);
        // If no isLatest, find the newest by semver
        if (!latest) {
            latest = await firmwareRepository_1.firmwareRepository.getNewestForDevice(deviceModel, hardwareVersion);
        }
        if (!latest) {
            return { hasUpdate: false };
        }
        const hasUpdate = (0, version_1.isNewerVersion)(currentVersion, latest.firmwareVersion);
        return {
            hasUpdate,
            latestVersion: latest.firmwareVersion,
            downloadUrl: (0, url_1.buildFirmwareDownloadUrl)(String(latest._id)),
            releaseNote: latest.description,
        };
    }
    async listFirmware(filter) {
        return firmwareRepository_1.firmwareRepository.findAll(filter);
    }
    async getFirmwareById(id) {
        return firmwareRepository_1.firmwareRepository.findById(id);
    }
}
exports.FirmwareService = FirmwareService;
exports.firmwareService = new FirmwareService();
//# sourceMappingURL=firmwareService.js.map