"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getMqttLogs = exports.getRecentFirmware = exports.getDevices = exports.getWeeklyActivity = exports.getSummary = void 0;
const dashboardService_1 = require("../services/dashboardService");
const logService_1 = require("../services/logService");
const getSummary = async (req, res) => {
    try {
        const summary = await dashboardService_1.dashboardService.getSummary();
        res.json({ success: true, data: summary });
    }
    catch (error) {
        logService_1.logger.system.error('Error fetching dashboard summary:', error);
        res.status(500).json({ success: false, message: 'Server error' });
    }
};
exports.getSummary = getSummary;
const getWeeklyActivity = async (req, res) => {
    try {
        const activity = await dashboardService_1.dashboardService.getWeeklyActivity();
        res.json({ success: true, data: activity.data });
    }
    catch (error) {
        logService_1.logger.system.error('Error fetching dashboard weekly activity:', error);
        res.status(500).json({ success: false, message: 'Server error' });
    }
};
exports.getWeeklyActivity = getWeeklyActivity;
const getDevices = async (req, res) => {
    try {
        const devices = await dashboardService_1.dashboardService.getDevices();
        res.json({ success: true, data: devices });
    }
    catch (error) {
        logService_1.logger.system.error('Error fetching dashboard devices:', error);
        res.status(500).json({ success: false, message: 'Server error' });
    }
};
exports.getDevices = getDevices;
const getRecentFirmware = async (req, res) => {
    try {
        const firmware = await dashboardService_1.dashboardService.getRecentFirmware();
        res.json({ success: true, data: firmware });
    }
    catch (error) {
        logService_1.logger.system.error('Error fetching dashboard recent firmware:', error);
        res.status(500).json({ success: false, message: 'Server error' });
    }
};
exports.getRecentFirmware = getRecentFirmware;
const getMqttLogs = async (req, res) => {
    try {
        const logs = await dashboardService_1.dashboardService.getMqttLogs();
        res.json({ success: true, data: logs });
    }
    catch (error) {
        logService_1.logger.system.error('Error fetching dashboard mqtt logs:', error);
        res.status(500).json({ success: false, message: 'Server error' });
    }
};
exports.getMqttLogs = getMqttLogs;
//# sourceMappingURL=dashboardController.js.map