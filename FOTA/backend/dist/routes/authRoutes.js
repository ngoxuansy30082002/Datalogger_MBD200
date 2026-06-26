"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = require("express");
const authController_1 = require("../controllers/authController");
const auth_1 = require("../middleware/auth");
const roleGuard_1 = require("../middleware/roleGuard");
const validation_1 = require("../middleware/validation");
const router = (0, express_1.Router)();
/**
 * @swagger
 * /auth/login:
 *   post:
 *     summary: User login
 *     tags: [Authentication]
 *     security: []
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/LoginRequest'
 *     responses:
 *       200:
 *         description: Login successful
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/LoginResponse'
 *       401:
 *         description: Invalid credentials
 */
router.post('/login', (0, validation_1.validate)('login'), authController_1.authController.login.bind(authController_1.authController));
/**
 * @swagger
 * /auth/register:
 *   post:
 *     summary: Register new user (admin only)
 *     tags: [Authentication]
 *     security:
 *       - bearerAuth: []
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required:
 *               - username
 *               - password
 *             properties:
 *               username:
 *                 type: string
 *               password:
 *                 type: string
 *               email:
 *                 type: string
 *               role:
 *                 type: string
 *                 enum: [admin, user]
 *     responses:
 *       201:
 *         description: User registered
 */
router.post('/register', auth_1.authenticate, (0, roleGuard_1.requireRole)('admin'), (0, validation_1.validate)('register'), authController_1.authController.register.bind(authController_1.authController));
/**
 * @swagger
 * /auth/refresh:
 *   post:
 *     summary: Refresh access token
 *     tags: [Authentication]
 *     security: []
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/RefreshTokenRequest'
 *     responses:
 *       200:
 *         description: Token refreshed
 */
router.post('/refresh', (0, validation_1.validate)('refreshToken'), authController_1.authController.refresh.bind(authController_1.authController));
/**
 * @swagger
 * /auth/profile:
 *   get:
 *     summary: Get current user profile
 *     tags: [Authentication]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: User profile
 */
router.get('/profile', auth_1.authenticate, authController_1.authController.profile.bind(authController_1.authController));
/**
 * @swagger
 * /auth/users:
 *   get:
 *     summary: Get all users (admin only)
 *     tags: [Authentication]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: Users list
 */
router.get('/users', auth_1.authenticate, (0, roleGuard_1.requireRole)('admin'), authController_1.authController.getUsers.bind(authController_1.authController));
exports.default = router;
//# sourceMappingURL=authRoutes.js.map