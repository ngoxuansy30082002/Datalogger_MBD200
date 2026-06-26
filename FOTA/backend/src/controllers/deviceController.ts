import { Request, Response } from 'express';
import { deviceService } from '../services/deviceService';

export class DeviceController {
  /**
   * List all devices
   * GET /api/devices
   */
  async list(req: Request, res: Response): Promise<void> {
    try {
      const page = parseInt(String(req.query.page || '1'), 10);
      const limit = parseInt(String(req.query.limit || '20'), 10);
      const result = await deviceService.getDevices(page, limit);

      res.json({
        success: true,
        ...result,
      });
    } catch (error) {
      res.status(500).json({
        success: false,
        message: (error as Error).message,
      });
    }
  }

  /**
   * Get device by ID
   * GET /api/devices/:id
   */
  async getById(req: Request, res: Response): Promise<void> {
    try {
      const id = String(req.params.id);
      const device = await deviceService.getDeviceById(id);

      if (!device) {
        res.status(404).json({ success: false, message: 'Device not found' });
        return;
      }

      res.json({ success: true, data: device });
    } catch (error) {
      res.status(500).json({
        success: false,
        message: (error as Error).message,
      });
    }
  }
}

export const deviceController = new DeviceController();
