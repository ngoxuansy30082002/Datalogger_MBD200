import { IFirmwareDocument } from '../models/Firmware';
import { FirmwareFilter, FirmwareUpdateResponse } from '../types';
export declare class FirmwareService {
    uploadFirmware(file: Express.Multer.File, metadata: {
        projectName: string;
        deviceModel: string;
        hardwareVersion: string;
        firmwareVersion: string;
        description: string;
        uploadedBy: string;
    }): Promise<IFirmwareDocument>;
    deleteFirmware(id: string, deletedBy: string): Promise<void>;
    downloadFirmware(id: string, ipAddress: string, userAgent: string): Promise<{
        filePath: string;
        fileName: string;
    }>;
    downloadByQuery(deviceModel: string, hardwareVersion: string, version: string, ipAddress: string, userAgent: string): Promise<{
        filePath: string;
        fileName: string;
    }>;
    markAsLatest(id: string): Promise<IFirmwareDocument>;
    checkForUpdate(deviceModel: string, hardwareVersion: string, currentVersion: string): Promise<FirmwareUpdateResponse>;
    listFirmware(filter: FirmwareFilter): Promise<{
        data: IFirmwareDocument[];
        total: number;
        page: number;
        totalPages: number;
    }>;
    getFirmwareById(id: string): Promise<IFirmwareDocument | null>;
}
export declare const firmwareService: FirmwareService;
//# sourceMappingURL=firmwareService.d.ts.map