import { Request, Response } from 'express';
import { AuthRequest } from '../types';
export declare class AuthController {
    /**
     * Login
     * POST /api/auth/login
     */
    login(req: Request, res: Response): Promise<void>;
    /**
     * Register new user (admin only)
     * POST /api/auth/register
     */
    register(req: AuthRequest, res: Response): Promise<void>;
    /**
     * Refresh token
     * POST /api/auth/refresh
     */
    refresh(req: Request, res: Response): Promise<void>;
    /**
     * Get current user profile
     * GET /api/auth/profile
     */
    profile(req: AuthRequest, res: Response): Promise<void>;
    /**
     * Get all users (admin only)
     * GET /api/auth/users
     */
    getUsers(_req: AuthRequest, res: Response): Promise<void>;
}
export declare const authController: AuthController;
//# sourceMappingURL=authController.d.ts.map