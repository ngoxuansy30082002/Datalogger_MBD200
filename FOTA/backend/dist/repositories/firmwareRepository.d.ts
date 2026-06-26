import { IFirmwareDocument } from '../models/Firmware';
import { FirmwareFilter } from '../types';
export declare class FirmwareRepository {
    create(data: Partial<IFirmwareDocument>): Promise<IFirmwareDocument>;
    findById(id: string): Promise<IFirmwareDocument | null>;
    findByHash(hash: string): Promise<IFirmwareDocument | null>;
    findLatest(deviceModel: string, hardwareVersion: string): Promise<IFirmwareDocument | null>;
    findByModelAndVersion(deviceModel: string, hardwareVersion: string, firmwareVersion: string): Promise<IFirmwareDocument | null>;
    findAll(filter: FirmwareFilter): Promise<{
        data: IFirmwareDocument[];
        total: number;
        page: number;
        totalPages: number;
    }>;
    update(id: string, data: Partial<IFirmwareDocument>): Promise<IFirmwareDocument | null>;
    delete(id: string): Promise<IFirmwareDocument | null>;
    unsetLatest(deviceModel: string, hardwareVersion: string): Promise<void>;
    incrementDownloadCount(id: string): Promise<void>;
    getStats(): Promise<{
        totalFirmware: number;
        totalDownloads: number;
    }>;
    getLatestUploads(limit?: number): Promise<IFirmwareDocument[]>;
    getNewestForDevice(deviceModel: string, hardwareVersion: string): Promise<IFirmwareDocument | null>;
}
export declare const firmwareRepository: FirmwareRepository;
//# sourceMappingURL=firmwareRepository.d.ts.map