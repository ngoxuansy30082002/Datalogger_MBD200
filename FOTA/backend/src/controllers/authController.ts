import { Request, Response } from 'express';
import { authService } from '../services/authService';
import { AuthRequest } from '../types';
import { userRepository } from '../repositories/userRepository';

export class AuthController {
  /**
   * Login
   * POST /api/auth/login
   */
  async login(req: Request, res: Response): Promise<void> {
    try {
      const { username, password } = req.body;
      const result = await authService.login(username, password);

      res.json({
        success: true,
        ...result,
      });
    } catch (error) {
      res.status(401).json({
        success: false,
        message: (error as Error).message,
      });
    }
  }

  /**
   * Register new user (admin only)
   * POST /api/auth/register
   */
  async register(req: AuthRequest, res: Response): Promise<void> {
    try {
      const { username, password, email, role } = req.body;
      const user = await authService.register(username, password, email, role);

      res.status(201).json({
        success: true,
        message: 'User registered successfully',
        data: user,
      });
    } catch (error) {
      res.status(400).json({
        success: false,
        message: (error as Error).message,
      });
    }
  }

  /**
   * Refresh token
   * POST /api/auth/refresh
   */
  async refresh(req: Request, res: Response): Promise<void> {
    try {
      const { refreshToken } = req.body;
      const result = await authService.refreshToken(refreshToken);

      res.json({
        success: true,
        ...result,
      });
    } catch (error) {
      res.status(401).json({
        success: false,
        message: (error as Error).message,
      });
    }
  }

  /**
   * Get current user profile
   * GET /api/auth/profile
   */
  async profile(req: AuthRequest, res: Response): Promise<void> {
    try {
      if (!req.user) {
        res.status(401).json({ success: false, message: 'Not authenticated' });
        return;
      }

      const user = await userRepository.findById(req.user.id);
      if (!user) {
        res.status(404).json({ success: false, message: 'User not found' });
        return;
      }

      res.json({
        success: true,
        data: user,
      });
    } catch (error) {
      res.status(500).json({
        success: false,
        message: (error as Error).message,
      });
    }
  }

  /**
   * Get all users (admin only)
   * GET /api/auth/users
   */
  async getUsers(_req: AuthRequest, res: Response): Promise<void> {
    try {
      const users = await userRepository.findAll();
      res.json({
        success: true,
        data: users,
      });
    } catch (error) {
      res.status(500).json({
        success: false,
        message: (error as Error).message,
      });
    }
  }
}

export const authController = new AuthController();
