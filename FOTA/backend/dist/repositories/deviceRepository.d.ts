import { IDeviceDocument } from '../models/Device';
export declare class DeviceRepository {
    create(data: Partial<IDeviceDocument>): Promise<IDeviceDocument>;
    findById(id: string): Promise<IDeviceDocument | null>;
    findByDeviceId(deviceId: string): Promise<IDeviceDocument | null>;
    findAll(page?: number, limit?: number): Promise<{
        data: IDeviceDocument[];
        total: number;
        page: number;
        totalPages: number;
    }>;
    upsert(deviceId: string, data: Partial<IDeviceDocument>): Promise<IDeviceDocument>;
    updateOnlineStatus(deviceId: string, online: boolean): Promise<void>;
    updateLastSeen(deviceId: string): Promise<void>;
    getOnlineCount(): Promise<number>;
    delete(id: string): Promise<IDeviceDocument | null>;
}
export declare const deviceRepository: DeviceRepository;
//# sourceMappingURL=deviceRepository.d.ts.map