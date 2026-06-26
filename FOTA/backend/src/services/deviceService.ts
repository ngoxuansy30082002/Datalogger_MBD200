import { deviceRepository } from '../repositories/deviceRepository';

export class DeviceService {
  async getDevices(page: number = 1, limit: number = 20) {
    return deviceRepository.findAll(page, limit);
  }

  async getDeviceById(id: string) {
    return deviceRepository.findById(id);
  }

  async getOnlineCount(): Promise<number> {
    return deviceRepository.getOnlineCount();
  }
}

export const deviceService = new DeviceService();
