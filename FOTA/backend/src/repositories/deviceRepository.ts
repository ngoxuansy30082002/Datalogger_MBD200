import { Device, IDeviceDocument } from '../models/Device';

export class DeviceRepository {
  async create(data: Partial<IDeviceDocument>): Promise<IDeviceDocument> {
    const device = new Device(data);
    return device.save();
  }

  async findById(id: string): Promise<IDeviceDocument | null> {
    return Device.findById(id);
  }

  async findByDeviceId(deviceId: string): Promise<IDeviceDocument | null> {
    return Device.findOne({ deviceId });
  }

  async findAll(page: number = 1, limit: number = 20): Promise<{
    data: IDeviceDocument[];
    total: number;
    page: number;
    totalPages: number;
  }> {
    const skip = (page - 1) * limit;
    const [data, total] = await Promise.all([
      Device.find().sort({ lastSeen: -1 }).skip(skip).limit(limit),
      Device.countDocuments(),
    ]);

    return {
      data,
      total,
      page,
      totalPages: Math.ceil(total / limit),
    };
  }

  async upsert(deviceId: string, data: Partial<IDeviceDocument>): Promise<IDeviceDocument> {
    const device = await Device.findOneAndUpdate(
      { deviceId },
      { ...data, lastSeen: new Date() },
      { new: true, upsert: true, setDefaultsOnInsert: true }
    );
    return device;
  }

  async updateOnlineStatus(deviceId: string, online: boolean): Promise<void> {
    await Device.findOneAndUpdate(
      { deviceId },
      { online, lastSeen: new Date() }
    );
  }

  async updateLastSeen(deviceId: string): Promise<void> {
    await Device.findOneAndUpdate(
      { deviceId },
      { lastSeen: new Date(), online: true }
    );
  }

  async getOnlineCount(): Promise<number> {
    // Consider devices seen in the last 5 minutes as online
    const fiveMinutesAgo = new Date(Date.now() - 5 * 60 * 1000);
    return Device.countDocuments({
      lastSeen: { $gte: fiveMinutesAgo },
      online: true,
    });
  }

  async delete(id: string): Promise<IDeviceDocument | null> {
    return Device.findByIdAndDelete(id);
  }
}

export const deviceRepository = new DeviceRepository();
