import { Response, NextFunction } from 'express';
import { AuthRequest } from '../types';
export declare const requireRole: (...roles: string[]) => (req: AuthRequest, res: Response, next: NextFunction) => void;
//# sourceMappingURL=roleGuard.d.ts.map