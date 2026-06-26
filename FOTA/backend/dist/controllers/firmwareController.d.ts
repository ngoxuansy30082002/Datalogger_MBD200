import { Request, Response } from 'express';
import { AuthRequest } from '../types';
export declare class FirmwareController {
    /**
     * Upload firmware files
     * POST /api/firmware/upload
     */
    upload(req: AuthRequest, res: Response): Promise<void>;
    /**
     * List firmware with pagination, search, and filters
     * GET /api/firmware
     */
    list(req: Request, res: Response): Promise<void>;
    /**
     * Download firmware by ID
     * GET /api/firmware/download/:id
     */
    downloadById(req: Request, res: Response): Promise<void>;
    /**
     * Download firmware by query parameters
     * GET /api/firmware/download?deviceModel=...&hw=...&version=...
     */
    downloadByQuery(req: Request, res: Response): Promise<void>;
    /**
     * Check for firmware update
     * GET /api/firmware/latest
     */
    checkLatest(req: Request, res: Response): Promise<void>;
    /**
     * Mark firmware as latest
     * PATCH /api/firmware/:id/latest
     */
    markLatest(req: AuthRequest, res: Response): Promise<void>;
    /**
     * Delete firmware
     * DELETE /api/firmware/:id
     */
    delete(req: AuthRequest, res: Response): Promise<void>;
    /**
     * Get firmware by ID
     * GET /api/firmware/:id
     */
    getById(req: Request, res: Response): Promise<void>;
}
export declare const firmwareController: FirmwareController;
//# sourceMappingURL=firmwareController.d.ts.map