"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.dashboardService = exports.DashboardService = void 0;
const Firmware_1 = require("../models/Firmware");
const DownloadLog_1 = require("../models/DownloadLog");
const Device_1 = require("../models/Device");
const MqttLog_1 = require("../models/MqttLog");
class DashboardService {
    async getSummary() {
        const totalFirmware = await Firmware_1.Firmware.countDocuments();
        const totalDownloads = await DownloadLog_1.DownloadLog.countDocuments();
        // Online device: lastSeen within 5 minutes
        const fiveMinsAgo = new Date(Date.now() - 5 * 60 * 1000);
        const onlineDevices = await Device_1.Device.countDocuments({ online: true, lastSeen: { $gte: fiveMinsAgo } });
        const offlineDevices = await Device_1.Device.countDocuments({ $or: [{ online: false }, { lastSeen: { $lt: fiveMinsAgo } }] });
        const startOfDay = new Date();
        startOfDay.setHours(0, 0, 0, 0);
        const mqttMessagesToday = await MqttLog_1.MqttLog.countDocuments({ timestamp: { $gte: startOfDay } });
        const latestFirmware = await Firmware_1.Firmware.findOne().sort({ uploadTime: -1 }).select('firmwareVersion');
        return {
            totalFirmware,
            totalDownloads,
            onlineDevices,
            offlineDevices,
            mqttMessagesToday,
            latestFirmwareVersion: latestFirmware ? latestFirmware.firmwareVersion : 'N/A',
        };
    }
    async getWeeklyActivity() {
        const data = [];
        for (let i = 6; i >= 0; i--) {
            const date = new Date();
            date.setDate(date.getDate() - i);
            date.setHours(0, 0, 0, 0);
            const nextDate = new Date(date);
            nextDate.setDate(date.getDate() + 1);
            const uploads = await Firmware_1.Firmware.countDocuments({ uploadTime: { $gte: date, $lt: nextDate } });
            const downloads = await DownloadLog_1.DownloadLog.countDocuments({ timestamp: { $gte: date, $lt: nextDate } });
            const mqttMessages = await MqttLog_1.MqttLog.countDocuments({ timestamp: { $gte: date, $lt: nextDate } });
            data.push({
                date: date.toISOString().split('T')[0],
                uploads,
                downloads,
                mqttMessages,
            });
        }
        return { data };
    }
    async getDevices() {
        const fiveMinsAgo = new Date(Date.now() - 5 * 60 * 1000);
        const online = await Device_1.Device.countDocuments({ online: true, lastSeen: { $gte: fiveMinsAgo } });
        const offline = await Device_1.Device.countDocuments({ $or: [{ online: false }, { lastSeen: { $lt: fiveMinsAgo } }] });
        const devices = await Device_1.Device.find().sort({ lastSeen: -1 }).limit(50);
        return {
            online,
            offline,
            devices,
        };
    }
    async getRecentFirmware() {
        return await Firmware_1.Firmware.find().sort({ uploadTime: -1 }).limit(10);
    }
    async getMqttLogs() {
        return await MqttLog_1.MqttLog.find().sort({ timestamp: -1 }).limit(20);
    }
}
exports.DashboardService = DashboardService;
exports.dashboardService = new DashboardService();
//# sourceMappingURL=dashboardService.js.map