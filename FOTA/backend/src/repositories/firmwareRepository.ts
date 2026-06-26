import semver from 'semver';
import { Firmware, IFirmwareDocument } from '../models/Firmware';
import { FirmwareFilter } from '../types';

export class FirmwareRepository {
  async create(data: Partial<IFirmwareDocument>): Promise<IFirmwareDocument> {
    const firmware = new Firmware(data);
    return firmware.save();
  }

  async findById(id: string): Promise<IFirmwareDocument | null> {
    return Firmware.findById(id);
  }

  async findByHash(hash: string): Promise<IFirmwareDocument | null> {
    return Firmware.findOne({ fileHash: hash });
  }

  async findLatest(deviceModel: string, hardwareVersion: string): Promise<IFirmwareDocument | null> {
    return Firmware.findOne({
      deviceModel,
      hardwareVersion,
      isLatest: true,
    });
  }

  async findByModelAndVersion(
    deviceModel: string,
    hardwareVersion: string,
    firmwareVersion: string
  ): Promise<IFirmwareDocument | null> {
    return Firmware.findOne({ deviceModel, hardwareVersion, firmwareVersion });
  }

  async findAll(filter: FirmwareFilter): Promise<{
    data: IFirmwareDocument[];
    total: number;
    page: number;
    totalPages: number;
  }> {
    const page = filter.page || 1;
    const limit = filter.limit || 20;
    const skip = (page - 1) * limit;
    const sortBy = filter.sortBy || 'uploadTime';
    const sortOrder = filter.sortOrder === 'asc' ? 1 : -1;

    const query: Record<string, unknown> = {};

    if (filter.projectName) {
      query.projectName = { $regex: filter.projectName, $options: 'i' };
    }
    if (filter.deviceModel) {
      query.deviceModel = { $regex: filter.deviceModel, $options: 'i' };
    }
    if (filter.hardwareVersion) {
      query.hardwareVersion = filter.hardwareVersion;
    }
    if (filter.isLatest !== undefined) {
      query.isLatest = filter.isLatest;
    }
    if (filter.search) {
      query.$or = [
        { projectName: { $regex: filter.search, $options: 'i' } },
        { deviceModel: { $regex: filter.search, $options: 'i' } },
        { firmwareVersion: { $regex: filter.search, $options: 'i' } },
        { fileName: { $regex: filter.search, $options: 'i' } },
        { description: { $regex: filter.search, $options: 'i' } },
      ];
    }

    const [data, total] = await Promise.all([
      Firmware.find(query)
        .sort({ [sortBy]: sortOrder })
        .skip(skip)
        .limit(limit),
      Firmware.countDocuments(query),
    ]);

    return {
      data,
      total,
      page,
      totalPages: Math.ceil(total / limit),
    };
  }

  async update(id: string, data: Partial<IFirmwareDocument>): Promise<IFirmwareDocument | null> {
    return Firmware.findByIdAndUpdate(id, data, { new: true });
  }

  async delete(id: string): Promise<IFirmwareDocument | null> {
    return Firmware.findByIdAndDelete(id);
  }

  async unsetLatest(deviceModel: string, hardwareVersion: string): Promise<void> {
    await Firmware.updateMany(
      { deviceModel, hardwareVersion, isLatest: true },
      { isLatest: false }
    );
  }

  async incrementDownloadCount(id: string): Promise<void> {
    await Firmware.findByIdAndUpdate(id, { $inc: { downloadCount: 1 } });
  }

  async getStats(): Promise<{ totalFirmware: number; totalDownloads: number }> {
    const [totalResult, downloadResult] = await Promise.all([
      Firmware.countDocuments(),
      Firmware.aggregate([
        { $group: { _id: null, totalDownloads: { $sum: '$downloadCount' } } },
      ]),
    ]);

    return {
      totalFirmware: totalResult,
      totalDownloads: downloadResult[0]?.totalDownloads || 0,
    };
  }

  async getLatestUploads(limit: number = 5): Promise<IFirmwareDocument[]> {
    return Firmware.find().sort({ uploadTime: -1 }).limit(limit);
  }

  async getNewestForDevice(
    deviceModel: string,
    hardwareVersion: string
  ): Promise<IFirmwareDocument | null> {
    const firmwares = await Firmware.find({ deviceModel, hardwareVersion })
      .sort({ uploadTime: -1 });
    
    if (firmwares.length === 0) return null;

    // Sort by semver to find the newest version
    const sorted = firmwares.sort((a, b) => {
      const va = semver.coerce(a.firmwareVersion);
      const vb = semver.coerce(b.firmwareVersion);
      if (!va || !vb) return 0;
      return semver.rcompare(va, vb);
    });

    return sorted[0];
  }
}

export const firmwareRepository = new FirmwareRepository();
