export declare class DashboardService {
    getSummary(): Promise<{
        totalFirmware: number;
        totalDownloads: number;
        onlineDevices: number;
        offlineDevices: number;
        mqttMessagesToday: number;
        latestFirmwareVersion: string;
    }>;
    getWeeklyActivity(): Promise<{
        data: {
            date: string;
            uploads: number;
            downloads: number;
            mqttMessages: number;
        }[];
    }>;
    getDevices(): Promise<{
        online: number;
        offline: number;
        devices: (import("mongoose").Document<unknown, {}, import("../models/Device").IDeviceDocument, {}, {}> & import("../models/Device").IDeviceDocument & Required<{
            _id: import("mongoose").Types.ObjectId;
        }> & {
            __v: number;
        })[];
    }>;
    getRecentFirmware(): Promise<(import("mongoose").Document<unknown, {}, import("../models/Firmware").IFirmwareDocument, {}, {}> & import("../models/Firmware").IFirmwareDocument & Required<{
        _id: import("mongoose").Types.ObjectId;
    }> & {
        __v: number;
    })[]>;
    getMqttLogs(): Promise<(import("mongoose").Document<unknown, {}, import("../models/MqttLog").IMqttLogDocument, {}, {}> & import("../models/MqttLog").IMqttLogDocument & Required<{
        _id: import("mongoose").Types.ObjectId;
    }> & {
        __v: number;
    })[]>;
}
export declare const dashboardService: DashboardService;
//# sourceMappingURL=dashboardService.d.ts.map