import { Request, Response } from 'express';
import { mqttService } from '../services/mqttService';
import { AuthRequest } from '../types';

export class MqttController {
  /**
   * Publish firmware update notification
   * POST /api/mqtt/notify
   */
  async publishNotify(req: AuthRequest, res: Response): Promise<void> {
    try {
      const { firmwareVersion = 'latest' } = req.body;

      mqttService.publishFirmwareNotify(firmwareVersion);

      res.json({
        success: true,
        message: 'Firmware notification published',
        data: {
          action: 'notify',
          firmwareVersion,
          timestamp: new Date().toISOString(),
        },
      });
    } catch (error) {
      res.status(500).json({
        success: false,
        message: (error as Error).message,
      });
    }
  }

  /**
   * Get MQTT connection status
   * GET /api/mqtt/status
   */
  async getStatus(_req: Request, res: Response): Promise<void> {
    res.json({
      success: true,
      data: {
        connected: mqttService.getConnectionStatus(),
      },
    });
  }
}

export const mqttController = new MqttController();
