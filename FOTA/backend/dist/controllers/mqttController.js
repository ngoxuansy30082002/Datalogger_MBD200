"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.mqttController = exports.MqttController = void 0;
const mqttService_1 = require("../services/mqttService");
class MqttController {
    /**
     * Publish firmware update notification
     * POST /api/mqtt/notify
     */
    async publishNotify(req, res) {
        try {
            const { firmwareVersion = 'latest' } = req.body;
            mqttService_1.mqttService.publishFirmwareNotify(firmwareVersion);
            res.json({
                success: true,
                message: 'Firmware notification published',
                data: {
                    action: 'notify',
                    firmwareVersion,
                    timestamp: new Date().toISOString(),
                },
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
     * Get MQTT connection status
     * GET /api/mqtt/status
     */
    async getStatus(_req, res) {
        res.json({
            success: true,
            data: {
                connected: mqttService_1.mqttService.getConnectionStatus(),
            },
        });
    }
}
exports.MqttController = MqttController;
exports.mqttController = new MqttController();
//# sourceMappingURL=mqttController.js.map