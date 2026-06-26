import { DownloadLog, IDownloadLogDocument } from '../models/DownloadLog';

export class DownloadLogRepository {
  async create(data: Partial<IDownloadLogDocument>): Promise<IDownloadLogDocument> {
    const log = new DownloadLog(data);
    return log.save();
  }

  async findByFirmwareId(firmwareId: string): Promise<IDownloadLogDocument[]> {
    return DownloadLog.find({ firmwareId }).sort({ timestamp: -1 });
  }

  async findRecent(limit: number = 50): Promise<IDownloadLogDocument[]> {
    return DownloadLog.find()
      .populate('firmwareId', 'fileName firmwareVersion deviceModel')
      .sort({ timestamp: -1 })
      .limit(limit);
  }
}

export const downloadLogRepository = new DownloadLogRepository();
