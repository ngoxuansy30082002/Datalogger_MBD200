import { IDownloadLogDocument } from '../models/DownloadLog';
export declare class DownloadLogRepository {
    create(data: Partial<IDownloadLogDocument>): Promise<IDownloadLogDocument>;
    findByFirmwareId(firmwareId: string): Promise<IDownloadLogDocument[]>;
    findRecent(limit?: number): Promise<IDownloadLogDocument[]>;
}
export declare const downloadLogRepository: DownloadLogRepository;
//# sourceMappingURL=downloadLogRepository.d.ts.map