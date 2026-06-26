import { Firmware } from '../models/Firmware';
import { DownloadLog } from '../models/DownloadLog';
import { Device } from '../models/Device';
import { MqttLog } from '../models/MqttLog';

export class DashboardService {
  async getSummary() {
    const totalFirmware = await Firmware.countDocuments();
    const totalDownloads = await DownloadLog.countDocuments();
    
    // Online device: lastSeen within 5 minutes
    const fiveMinsAgo = new Date(Date.now() - 5 * 60 * 1000);
    const onlineDevices = await Device.countDocuments({ online: true, lastSeen: { $gte: fiveMinsAgo } });
    const offlineDevices = await Device.countDocuments({ $or: [{ online: false }, { lastSeen: { $lt: fiveMinsAgo } }] });

    const startOfDay = new Date();
    startOfDay.setHours(0, 0, 0, 0);
    const mqttMessagesToday = await MqttLog.countDocuments({ timestamp: { $gte: startOfDay } });

    const latestFirmware = await Firmware.findOne().sort({ uploadTime: -1 }).select('firmwareVersion');

    return {
      totalFirmware,
      totalDownloads,
      onlineDevices,
      offlineDevices,
      mqttMessagesToday,
      latestFirmwareVersion: latestFirmware ? latestFirmware.firmwareVersion : 'N/A',
    };
  }

  async getWeeklyActivity() {
    const data = [];
    for (let i = 6; i >= 0; i--) {
      const date = new Date();
      date.setDate(date.getDate() - i);
      date.setHours(0, 0, 0, 0);
      
      const nextDate = new Date(date);
      nextDate.setDate(date.getDate() + 1);

      const uploads = await Firmware.countDocuments({ uploadTime: { $gte: date, $lt: nextDate } });
      const downloads = await DownloadLog.countDocuments({ timestamp: { $gte: date, $lt: nextDate } });
      const mqttMessages = await MqttLog.countDocuments({ timestamp: { $gte: date, $lt: nextDate } });

      data.push({
        date: date.toISOString().split('T')[0],
        uploads,
        downloads,
        mqttMessages,
      });
    }
    return { data };
  }

  async getDevices() {
    const fiveMinsAgo = new Date(Date.now() - 5 * 60 * 1000);
    
    const online = await Device.countDocuments({ online: true, lastSeen: { $gte: fiveMinsAgo } });
    const offline = await Device.countDocuments({ $or: [{ online: false }, { lastSeen: { $lt: fiveMinsAgo } }] });
    const devices = await Device.find().sort({ lastSeen: -1 }).limit(50);

    return {
      online,
      offline,
      devices,
    };
  }

  async getRecentFirmware() {
    return await Firmware.find().sort({ uploadTime: -1 }).limit(10);
  }

  async getMqttLogs() {
    return await MqttLog.find().sort({ timestamp: -1 }).limit(20);
  }
}

export const dashboardService = new DashboardService();
