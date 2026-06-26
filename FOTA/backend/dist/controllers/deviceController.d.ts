import { Request, Response } from 'express';
export declare class DeviceController {
    /**
     * List all devices
     * GET /api/devices
     */
    list(req: Request, res: Response): Promise<void>;
    /**
     * Get device by ID
     * GET /api/devices/:id
     */
    getById(req: Request, res: Response): Promise<void>;
}
export declare const deviceController: DeviceController;
//# sourceMappingURL=deviceController.d.ts.map