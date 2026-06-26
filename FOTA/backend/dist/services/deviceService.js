"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.deviceService = exports.DeviceService = void 0;
const deviceRepository_1 = require("../repositories/deviceRepository");
class DeviceService {
    async getDevices(page = 1, limit = 20) {
        return deviceRepository_1.deviceRepository.findAll(page, limit);
    }
    async getDeviceById(id) {
        return deviceRepository_1.deviceRepository.findById(id);
    }
    async getOnlineCount() {
        return deviceRepository_1.deviceRepository.getOnlineCount();
    }
}
exports.DeviceService = DeviceService;
exports.deviceService = new DeviceService();
//# sourceMappingURL=deviceService.js.map