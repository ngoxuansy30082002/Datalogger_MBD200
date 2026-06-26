"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.optionalAuth = exports.authenticate = void 0;
const authService_1 = require("../services/authService");
const authenticate = (req, res, next) => {
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
        const decoded = authService_1.authService.verifyAccessToken(token);
        req.user = decoded;
        next();
    }
    catch {
        res.status(401).json({
            success: false,
            message: 'Invalid or expired token',
        });
    }
};
exports.authenticate = authenticate;
/**
 * Optional authentication - sets user if token present but doesn't block
 */
const optionalAuth = (req, _res, next) => {
    const authHeader = req.headers.authorization;
    if (authHeader && authHeader.startsWith('Bearer ')) {
        const token = authHeader.split(' ')[1];
        try {
            req.user = authService_1.authService.verifyAccessToken(token);
        }
        catch {
            // Token invalid, but that's OK for optional auth
        }
    }
    next();
};
exports.optionalAuth = optionalAuth;
//# sourceMappingURL=auth.js.map