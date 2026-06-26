"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.mqttService = void 0;
const mqtt_1 = __importDefault(require("mqtt"));
const mqtt_2 = require("../config/mqtt");
const firmwareService_1 = require("./firmwareService");
const deviceRepository_1 = require("../repositories/deviceRepository");
const logService_1 = require("./logService");
const MqttLog_1 = require("../models/MqttLog");
const socketService_1 = require("./socketService");
class MqttService {
    client = null;
    eventListeners = [];
    isConnected = false;
    connect() {
        this.client = mqtt_1.default.connect(mqtt_2.mqttConfig.brokerUrl, mqtt_2.mqttConfig.options);
        this.client.on('connect', () => {
            this.isConnected = true;
            logService_1.logger.mqtt.info('✅ Connected to EMQX broker', { url: mqtt_2.mqttConfig.brokerUrl });
            // Subscribe to topics
            mqtt_2.mqttConfig.topics.subscribe.forEach((topic) => {
                this.client?.subscribe(topic, { qos: 1 }, (err) => {
                    if (err) {
                        logService_1.logger.mqtt.error(`Failed to subscribe to ${topic}:`, err);
                    }
                    else {
                        logService_1.logger.mqtt.info(`Subscribed to: ${topic}`);
                    }
                });
            });
        });
        this.client.on('message', (topic, payload) => {
            this.handleMessage(topic, payload.toString());
        });
        this.client.on('error', (err) => {
            logService_1.logger.mqtt.error('MQTT error:', err);
        });
        this.client.on('close', () => {
            this.isConnected = false;
            logService_1.logger.mqtt.warn('MQTT connection closed');
        });
        this.client.on('reconnect', () => {
            logService_1.logger.mqtt.info('MQTT reconnecting...');
        });
    }
    async handleMessage(topic, payload) {
        const mqttMessage = {
            topic,
            payload,
            timestamp: new Date().toISOString(),
        };
        logService_1.logger.mqtt.info(`Message received: ${topic}`, { payload: payload.substring(0, 500) });
        try {
            await MqttLog_1.MqttLog.create({
                topic,
                payload: payload || '',
                timestamp: new Date(),
            });
        }
        catch (err) {
            logService_1.logger.mqtt.error('Failed to save MQTT log:', err);
        }
        // Notify all listeners (for Socket.IO forwarding)
        this.eventListeners.forEach((cb) => cb(mqttMessage));
        // Skip processing if payload is empty
        if (!payload || !payload.trim()) {
            logService_1.logger.mqtt.warn(`Empty payload received on ${topic}, skipping`);
            return;
        }
        try {
            // Handle firmware query: datalogger/{deviceId}/firmware/query
            const firmwareQueryMatch = topic.match(/^datalogger\/(.+)\/firmware\/query$/);
            if (firmwareQueryMatch) {
                const deviceId = firmwareQueryMatch[1];
                await this.handleFirmwareQuery(deviceId, payload);
                return;
            }
            // Handle device status: datalogger/{deviceId}/status
            const statusMatch = topic.match(/^datalogger\/(.+)\/status$/);
            if (statusMatch) {
                const deviceId = statusMatch[1];
                await this.handleDeviceStatus(deviceId, payload);
                return;
            }
            // Handle heartbeat: datalogger/{deviceId}/heartbeat
            const heartbeatMatch = topic.match(/^datalogger\/(.+)\/heartbeat$/);
            if (heartbeatMatch) {
                const deviceId = heartbeatMatch[1];
                await this.handleHeartbeat(deviceId, payload);
                return;
            }
        }
        catch (error) {
            logService_1.logger.mqtt.error(`Error handling MQTT message on ${topic}:`, error);
        }
    }
    async handleFirmwareQuery(deviceId, payload) {
        try {
            const query = JSON.parse(payload);
            logService_1.logger.mqtt.info(`Firmware query from device: ${deviceId}`, query);
            // Update or register device
            const device = await deviceRepository_1.deviceRepository.upsert(deviceId, {
                deviceId: query.deviceId || deviceId,
                deviceModel: query.deviceModel || 'Unknown',
                hardwareVersion: query.hardwareVersion || 'Unknown',
                firmwareVersion: query.currentFirmware || 'Unknown',
                online: true,
            });
            socketService_1.socketService.emitDeviceUpdate(device);
            // Check for firmware update
            const result = await firmwareService_1.firmwareService.checkForUpdate(query.deviceModel, query.hardwareVersion, query.currentFirmware);
            // Publish response
            const responseTopic = mqtt_2.mqttConfig.topics.publish.deviceResponse(deviceId);
            this.publish(responseTopic, JSON.stringify(result));
            logService_1.logger.mqtt.info(`Firmware response sent to ${deviceId}:`, result);
        }
        catch (error) {
            logService_1.logger.mqtt.error(`Failed to handle firmware query from ${deviceId}:`, error);
        }
    }
    async handleDeviceStatus(deviceId, payload) {
        try {
            const status = JSON.parse(payload);
            const device = await deviceRepository_1.deviceRepository.upsert(deviceId, {
                deviceId,
                online: true,
                latestStatus: status,
            });
            socketService_1.socketService.emitDeviceUpdate(device);
            logService_1.logger.mqtt.info(`Device updated: ${deviceId}`);
        }
        catch (error) {
            logService_1.logger.mqtt.error(`Failed to handle device status from ${deviceId}:`, error);
        }
    }
    async handleHeartbeat(deviceId, payload) {
        try {
            const data = JSON.parse(payload);
            const device = await deviceRepository_1.deviceRepository.upsert(deviceId, {
                deviceId: data.deviceId || deviceId,
                deviceModel: data.deviceModel || 'Unknown',
                hardwareVersion: data.hardwareVersion || 'Unknown',
                firmwareVersion: data.firmwareVersion || 'Unknown',
                online: true,
            });
            socketService_1.socketService.emitDeviceUpdate(device);
            logService_1.logger.mqtt.info(`Device registered: ${deviceId}`);
        }
        catch (err) {
            // Fallback if payload is not JSON
            await deviceRepository_1.deviceRepository.updateLastSeen(deviceId);
            logService_1.logger.mqtt.info(`Device registered: ${deviceId} (via generic heartbeat)`);
        }
    }
    publish(topic, message) {
        if (!this.client || !this.isConnected) {
            logService_1.logger.mqtt.warn('MQTT not connected, cannot publish');
            return;
        }
        this.client.publish(topic, message, { qos: 1 }, (err) => {
            if (err) {
                logService_1.logger.mqtt.error(`Failed to publish to ${topic}:`, err);
            }
            else {
                logService_1.logger.mqtt.info(`Published to ${topic}:`, { message: message.substring(0, 200) });
            }
        });
    }
    publishFirmwareNotify(firmwareVersion) {
        const payload = {
            action: 'notify',
            timestamp: new Date().toISOString(),
            firmwareVersion,
        };
        this.publish(mqtt_2.mqttConfig.topics.publish.firmwareNotify, JSON.stringify(payload));
    }
    publishFirmwareUpdate(data) {
        this.publish(mqtt_2.mqttConfig.topics.publish.firmwareUpdate, JSON.stringify(data));
    }
    onMessage(callback) {
        this.eventListeners.push(callback);
    }
    removeListener(callback) {
        this.eventListeners = this.eventListeners.filter((cb) => cb !== callback);
    }
    getConnectionStatus() {
        return this.isConnected;
    }
    disconnect() {
        if (this.client) {
            this.client.end();
            this.isConnected = false;
            logService_1.logger.mqtt.info('MQTT disconnected');
        }
    }
}
exports.mqttService = new MqttService();
//# sourceMappingURL=mqttService.js.map