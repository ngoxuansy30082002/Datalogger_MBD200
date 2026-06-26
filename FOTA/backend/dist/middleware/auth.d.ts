import { Response, NextFunction } from 'express';
import { AuthRequest } from '../types';
export declare const authenticate: (req: AuthRequest, res: Response, next: NextFunction) => void;
/**
 * Optional authentication - sets user if token present but doesn't block
 */
export declare const optionalAuth: (req: AuthRequest, _res: Response, next: NextFunction) => void;
//# sourceMappingURL=auth.d.ts.map