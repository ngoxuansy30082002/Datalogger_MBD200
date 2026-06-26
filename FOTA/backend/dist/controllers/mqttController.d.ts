import { Request, Response } from 'express';
import { AuthRequest } from '../types';
export declare class MqttController {
    /**
     * Publish firmware update notification
     * POST /api/mqtt/notify
     */
    publishNotify(req: AuthRequest, res: Response): Promise<void>;
    /**
     * Get MQTT connection status
     * GET /api/mqtt/status
     */
    getStatus(_req: Request, res: Response): Promise<void>;
}
export declare const mqttController: MqttController;
//# sourceMappingURL=mqttController.d.ts.map