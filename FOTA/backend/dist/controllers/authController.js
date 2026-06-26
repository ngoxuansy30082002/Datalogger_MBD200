"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.authController = exports.AuthController = void 0;
const authService_1 = require("../services/authService");
const userRepository_1 = require("../repositories/userRepository");
class AuthController {
    /**
     * Login
     * POST /api/auth/login
     */
    async login(req, res) {
        try {
            const { username, password } = req.body;
            const result = await authService_1.authService.login(username, password);
            res.json({
                success: true,
                ...result,
            });
        }
        catch (error) {
            res.status(401).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * Register new user (admin only)
     * POST /api/auth/register
     */
    async register(req, res) {
        try {
            const { username, password, email, role } = req.body;
            const user = await authService_1.authService.register(username, password, email, role);
            res.status(201).json({
                success: true,
                message: 'User registered successfully',
                data: user,
            });
        }
        catch (error) {
            res.status(400).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * Refresh token
     * POST /api/auth/refresh
     */
    async refresh(req, res) {
        try {
            const { refreshToken } = req.body;
            const result = await authService_1.authService.refreshToken(refreshToken);
            res.json({
                success: true,
                ...result,
            });
        }
        catch (error) {
            res.status(401).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * Get current user profile
     * GET /api/auth/profile
     */
    async profile(req, res) {
        try {
            if (!req.user) {
                res.status(401).json({ success: false, message: 'Not authenticated' });
                return;
            }
            const user = await userRepository_1.userRepository.findById(req.user.id);
            if (!user) {
                res.status(404).json({ success: false, message: 'User not found' });
                return;
            }
            res.json({
                success: true,
                data: user,
            });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                message: error.message,
            });
        }
    }
    /**
     * Get all users (admin only)
     * GET /api/auth/users
     */
    async getUsers(_req, res) {
        try {
            const users = await userRepository_1.userRepository.findAll();
            res.json({
                success: true,
                data: users,
            });
        }
        catch (error) {
            res.status(500).json({
                success: false,
                message: error.message,
            });
        }
    }
}
exports.AuthController = AuthController;
exports.authController = new AuthController();
//# sourceMappingURL=authController.js.map