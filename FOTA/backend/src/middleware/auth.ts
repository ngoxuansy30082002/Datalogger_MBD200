import { Request, Response, NextFunction } from 'express';
import { authService } from '../services/authService';
import { AuthRequest } from '../types';

export const authenticate = (req: AuthRequest, res: Response, next: NextFunction): void => {
  const authHeader = req.headers.authorization;

  if (!authHeader || !authHeader.startsWith('Bearer ')) {
    res.status(401).json({
      success: false,
      message: 'Access token required',
    });
    return;
  }

  const token = authHeader.split(' ')[1];

  try {
    const decoded = authService.verifyAccessToken(token);
    req.user = decoded;
    next();
  } catch {
    res.status(401).json({
      success: false,
      message: 'Invalid or expired token',
    });
  }
};

/**
 * Optional authentication - sets user if token present but doesn't block
 */
export const optionalAuth = (req: AuthRequest, _res: Response, next: NextFunction): void => {
  const authHeader = req.headers.authorization;

  if (authHeader && authHeader.startsWith('Bearer ')) {
    const token = authHeader.split(' ')[1];
    try {
      req.user = authService.verifyAccessToken(token);
    } catch {
      // Token invalid, but that's OK for optional auth
    }
  }

  next();
};
