"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.firmwareController = exports.FirmwareController = void 0;
const firmwareService_1 = require("../services/firmwareService");
const socketService_1 = require("../services/socketService");
class FirmwareController {
    /**
     * Upload firmware files
     * POST /api/firmware/upload
     */
    async upload(req, res) {
        try {
            const files = req.files;
            if (!files || files.length === 0) {
                res.status(400).json({ success: false, message: 'No files uploaded' });
                return;
            }
            const { projectName, deviceModel, hardwareVersion, firmwareVersion, description } = req.body;
            const results = [];
            const errors = [];
            for (const file of files) {
                try {
                    // For multiple files, append index to version if needed
                    const version = files.length > 1 && !req.body[`firmwareVersion_${files.indexOf(file)}`]
                        ? firmwareVersion
                        : (req.body[`firmwareVersion_${files.indexOf(file)}`] || firmwareVersion);
                    const firmware = await firmwareService_1.firmwareService.uploadFirmware(file, {
                        projectName,
                        deviceModel,
                        hardwareVersion,
                        firmwareVersion: version,
                        description: description || '',
                        uploadedBy: req.user?.username || 'unknown',
                    });
                    results.push(firmware);
                    socketService_1.socketService.emitFirmwareUploaded(firmware.toObject());
                }
                catch (error) {
                    errors.push({
                        file: file.originalname,
                        error: error.message,
                    });
                }
            }
            res.status(201).json({
                success: true,
                message: `${results.length} file(s) uploaded successfully`,
                data: results,
                errors: errors.length > 0 ? errors : undefined,
            });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * List firmware with pagination, search, and filters
     * GET /api/firmware
     */
    async list(req, res) {
        try {
            const { page = '1', limit = '20', search, projectName, deviceModel, hardwareVersion, isLatest, sortBy = 'uploadTime', sortOrder = 'desc', } = req.query;
            const result = await firmwareService_1.firmwareService.listFirmware({
                page: parseInt(page, 10),
                limit: parseInt(limit, 10),
                search: search,
                projectName: projectName,
                deviceModel: deviceModel,
                hardwareVersion: hardwareVersion,
                isLatest: isLatest !== undefined ? isLatest === 'true' : undefined,
                sortBy: sortBy,
                sortOrder: sortOrder,
            });
            res.json({
                success: true,
                ...result,
            });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * Download firmware by ID
     * GET /api/firmware/download/:id
     */
    async downloadById(req, res) {
        try {
            const id = String(req.params.id);
            const ipAddress = String(req.ip || req.socket.remoteAddress || 'unknown');
            const userAgent = String(req.headers['user-agent'] || 'unknown');
            const { filePath, fileName } = await firmwareService_1.firmwareService.downloadFirmware(id, ipAddress, userAgent);
            res.download(filePath, fileName);
        }
        catch (error) {
            res.status(404).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * Download firmware by query parameters
     * GET /api/firmware/download?deviceModel=...&hw=...&version=...
     */
    async downloadByQuery(req, res) {
        try {
            const { deviceModel, hw, version = 'latest' } = req.query;
            if (!deviceModel || !hw) {
                res.status(400).json({
                    success: false,
                    message: 'deviceModel and hw parameters are required',
                });
                return;
            }
            const ipAddress = String(req.ip || req.socket.remoteAddress || 'unknown');
            const userAgent = String(req.headers['user-agent'] || 'unknown');
            const { filePath, fileName } = await firmwareService_1.firmwareService.downloadByQuery(String(deviceModel), String(hw), String(version), ipAddress, userAgent);
            res.download(filePath, fileName);
        }
        catch (error) {
            res.status(404).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * Check for firmware update
     * GET /api/firmware/latest
     */
    async checkLatest(req, res) {
        try {
            const { deviceModel, hardwareVersion, currentVersion } = req.query;
            if (!deviceModel || !hardwareVersion || !currentVersion) {
                res.status(400).json({
                    success: false,
                    message: 'deviceModel, hardwareVersion, and currentVersion are required',
                });
                return;
            }
            const result = await firmwareService_1.firmwareService.checkForUpdate(deviceModel, hardwareVersion, currentVersion);
            res.json({
                success: true,
                ...result,
            });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * Mark firmware as latest
     * PATCH /api/firmware/:id/latest
     */
    async markLatest(req, res) {
        try {
            const id = String(req.params.id);
            const firmware = await firmwareService_1.firmwareService.markAsLatest(id);
            socketService_1.socketService.emitFirmwareUploaded(firmware.toObject());
            res.json({
                success: true,
                message: 'Firmware marked as latest',
                data: firmware,
            });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * Delete firmware
     * DELETE /api/firmware/:id
     */
    async delete(req, res) {
        try {
            const id = String(req.params.id);
            await firmwareService_1.firmwareService.deleteFirmware(id, req.user?.username || 'unknown');
            socketService_1.socketService.emitFirmwareDeleted(id);
            res.json({
                success: true,
                message: 'Firmware deleted successfully',
            });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * Get firmware by ID
     * GET /api/firmware/:id
     */
    async getById(req, res) {
        try {
            const id = String(req.params.id);
            const firmware = await firmwareService_1.firmwareService.getFirmwareById(id);
            if (!firmware) {
                res.status(404).json({ success: false, message: 'Firmware not found' });
                return;
            }
            res.json({ success: true, data: firmware });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                message: error.message,
            });
        }
    }
}
exports.FirmwareController = FirmwareController;
exports.firmwareController = new FirmwareController();
//# sourceMappingURL=firmwareController.js.map