export declare class DeviceService {
    getDevices(page?: number, limit?: number): Promise<{
        data: import("../models/Device").IDeviceDocument[];
        total: number;
        page: number;
        totalPages: number;
    }>;
    getDeviceById(id: string): Promise<import("../models/Device").IDeviceDocument | null>;
    getOnlineCount(): Promise<number>;
}
export declare const deviceService: DeviceService;
//# sourceMappingURL=deviceService.d.ts.map