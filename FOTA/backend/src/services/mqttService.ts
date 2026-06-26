import mqtt, { MqttClient } from 'mqtt';
import { mqttConfig } from '../config/mqtt';
import { firmwareService } from './firmwareService';
import { deviceRepository } from '../repositories/deviceRepository';
import { logger } from './logService';
import { MqttFirmwareQuery, MqttMessage } from '../types';
import { IDeviceDocument } from '../models/Device';
import { MqttLog } from '../models/MqttLog';
import { socketService } from './socketService';

type MqttEventCallback = (message: MqttMessage) => void;

class MqttService {
  private client: MqttClient | null = null;
  private eventListeners: MqttEventCallback[] = [];
  private isConnected: boolean = false;

  connect(): void {
    this.client = mqtt.connect(mqttConfig.brokerUrl, mqttConfig.options);

    this.client.on('connect', () => {
      this.isConnected = true;
      logger.mqtt.info('✅ Connected to EMQX broker', { url: mqttConfig.brokerUrl });

      // Subscribe to topics
      mqttConfig.topics.subscribe.forEach((topic) => {
        this.client?.subscribe(topic, { qos: 1 }, (err) => {
          if (err) {
            logger.mqtt.error(`Failed to subscribe to ${topic}:`, err);
          } else {
            logger.mqtt.info(`Subscribed to: ${topic}`);
          }
        });
      });
    });

    this.client.on('message', (topic, payload) => {
      this.handleMessage(topic, payload.toString());
    });

    this.client.on('error', (err) => {
      logger.mqtt.error('MQTT error:', err);
    });

    this.client.on('close', () => {
      this.isConnected = false;
      logger.mqtt.warn('MQTT connection closed');
    });

    this.client.on('reconnect', () => {
      logger.mqtt.info('MQTT reconnecting...');
    });
  }

  private async handleMessage(topic: string, payload: string): Promise<void> {
    const mqttMessage: MqttMessage = {
      topic,
      payload,
      timestamp: new Date().toISOString(),
    };

    logger.mqtt.info(`Message received: ${topic}`, { payload: payload.substring(0, 500) });

    try {
      await MqttLog.create({
        topic,
        payload: payload || '',
        timestamp: new Date(),
      });
    } catch (err) {
      logger.mqtt.error('Failed to save MQTT log:', err);
    }

    // Notify all listeners (for Socket.IO forwarding)
    this.eventListeners.forEach((cb) => cb(mqttMessage));

    // Skip processing if payload is empty
    if (!payload || !payload.trim()) {
      logger.mqtt.warn(`Empty payload received on ${topic}, skipping`);
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

    } catch (error) {
      logger.mqtt.error(`Error handling MQTT message on ${topic}:`, error);
    }
  }

  private async handleFirmwareQuery(deviceId: string, payload: string): Promise<void> {
    try {
      const query: MqttFirmwareQuery = JSON.parse(payload);

      logger.mqtt.info(`Firmware query from device: ${deviceId}`, query);

      // Update or register device
      const device = await deviceRepository.upsert(deviceId, {
        deviceId: query.deviceId || deviceId,
        deviceModel: query.deviceModel || 'Unknown',
        hardwareVersion: query.hardwareVersion || 'Unknown',
        firmwareVersion: query.currentFirmware || 'Unknown',
        online: true,
      } as Partial<IDeviceDocument>);
      
      socketService.emitDeviceUpdate(device);

      // Check for firmware update
      const result = await firmwareService.checkForUpdate(
        query.deviceModel,
        query.hardwareVersion,
        query.currentFirmware
      );

      // Publish response
      const responseTopic = mqttConfig.topics.publish.deviceResponse(deviceId);
      this.publish(responseTopic, JSON.stringify(result));

      logger.mqtt.info(`Firmware response sent to ${deviceId}:`, result);
    } catch (error) {
      logger.mqtt.error(`Failed to handle firmware query from ${deviceId}:`, error);
    }
  }

  private async handleDeviceStatus(deviceId: string, payload: string): Promise<void> {
    try {
      const status = JSON.parse(payload);
      const device = await deviceRepository.upsert(deviceId, {
        deviceId,
        online: true,
        latestStatus: status,
      } as Partial<IDeviceDocument>);

      socketService.emitDeviceUpdate(device);
      logger.mqtt.info(`Device updated: ${deviceId}`);
    } catch (error) {
      logger.mqtt.error(`Failed to handle device status from ${deviceId}:`, error);
    }
  }

  private async handleHeartbeat(deviceId: string, payload: string): Promise<void> {
    try {
      const data = JSON.parse(payload);
      const device = await deviceRepository.upsert(deviceId, {
        deviceId: data.deviceId || deviceId,
        deviceModel: data.deviceModel || 'Unknown',
        hardwareVersion: data.hardwareVersion || 'Unknown',
        firmwareVersion: data.firmwareVersion || 'Unknown',
        online: true,
      } as Partial<IDeviceDocument>);

      socketService.emitDeviceUpdate(device);
      logger.mqtt.info(`Device registered: ${deviceId}`);
    } catch (err) {
      // Fallback if payload is not JSON
      await deviceRepository.updateLastSeen(deviceId);
      logger.mqtt.info(`Device registered: ${deviceId} (via generic heartbeat)`);
    }
  }

  publish(topic: string, message: string): void {
    if (!this.client || !this.isConnected) {
      logger.mqtt.warn('MQTT not connected, cannot publish');
      return;
    }

    this.client.publish(topic, message, { qos: 1 }, (err) => {
      if (err) {
        logger.mqtt.error(`Failed to publish to ${topic}:`, err);
      } else {
        logger.mqtt.info(`Published to ${topic}:`, { message: message.substring(0, 200) });
      }
    });
  }

  publishFirmwareNotify(firmwareVersion: string): void {
    const payload = {
      action: 'notify',
      timestamp: new Date().toISOString(),
      firmwareVersion,
    };
    this.publish(mqttConfig.topics.publish.firmwareNotify, JSON.stringify(payload));
  }

  publishFirmwareUpdate(data: object): void {
    this.publish(mqttConfig.topics.publish.firmwareUpdate, JSON.stringify(data));
  }

  onMessage(callback: MqttEventCallback): void {
    this.eventListeners.push(callback);
  }

  removeListener(callback: MqttEventCallback): void {
    this.eventListeners = this.eventListeners.filter((cb) => cb !== callback);
  }

  getConnectionStatus(): boolean {
    return this.isConnected;
  }

  disconnect(): void {
    if (this.client) {
      this.client.end();
      this.isConnected = false;
      logger.mqtt.info('MQTT disconnected');
    }
  }
}

export const mqttService = new MqttService();
