"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.deviceController = exports.DeviceController = void 0;
const deviceService_1 = require("../services/deviceService");
class DeviceController {
    /**
     * List all devices
     * GET /api/devices
     */
    async list(req, res) {
        try {
            const page = parseInt(String(req.query.page || '1'), 10);
            const limit = parseInt(String(req.query.limit || '20'), 10);
            const result = await deviceService_1.deviceService.getDevices(page, limit);
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
     * Get device by ID
     * GET /api/devices/:id
     */
    async getById(req, res) {
        try {
            const id = String(req.params.id);
            const device = await deviceService_1.deviceService.getDeviceById(id);
            if (!device) {
                res.status(404).json({ success: false, message: 'Device not found' });
                return;
            }
            res.json({ success: true, data: device });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                message: error.message,
            });
        }
    }
}
exports.DeviceController = DeviceController;
exports.deviceController = new DeviceController();
//# sourceMappingURL=deviceController.js.map