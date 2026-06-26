import { MqttMessage } from '../types';
type MqttEventCallback = (message: MqttMessage) => void;
declare class MqttService {
    private client;
    private eventListeners;
    private isConnected;
    connect(): void;
    private handleMessage;
    private handleFirmwareQuery;
    private handleDeviceStatus;
    private handleHeartbeat;
    publish(topic: string, message: string): void;
    publishFirmwareNotify(firmwareVersion: string): void;
    publishFirmwareUpdate(data: object): void;
    onMessage(callback: MqttEventCallback): void;
    removeListener(callback: MqttEventCallback): void;
    getConnectionStatus(): boolean;
    disconnect(): void;
}
export declare const mqttService: MqttService;
export {};
//# sourceMappingURL=mqttService.d.ts.map