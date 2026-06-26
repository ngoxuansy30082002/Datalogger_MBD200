"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.authService = exports.AuthService = void 0;
const jsonwebtoken_1 = __importDefault(require("jsonwebtoken"));
const env_1 = require("../config/env");
const userRepository_1 = require("../repositories/userRepository");
const logService_1 = require("./logService");
class AuthService {
    generateAccessToken(user) {
        const payload = {
            id: String(user._id),
            username: user.username,
            role: user.role,
        };
        return jsonwebtoken_1.default.sign(payload, env_1.env.JWT_SECRET, {
            expiresIn: env_1.env.JWT_EXPIRY,
        });
    }
    generateRefreshToken(user) {
        const payload = {
            id: String(user._id),
            username: user.username,
            role: user.role,
        };
        return jsonwebtoken_1.default.sign(payload, env_1.env.JWT_REFRESH_SECRET, {
            expiresIn: env_1.env.JWT_REFRESH_EXPIRY,
        });
    }
    verifyAccessToken(token) {
        return jsonwebtoken_1.default.verify(token, env_1.env.JWT_SECRET);
    }
    verifyRefreshToken(token) {
        return jsonwebtoken_1.default.verify(token, env_1.env.JWT_REFRESH_SECRET);
    }
    async login(username, password) {
        const user = await userRepository_1.userRepository.findByUsername(username);
        if (!user) {
            throw new Error('Invalid username or password');
        }
        const isMatch = await user.comparePassword(password);
        if (!isMatch) {
            throw new Error('Invalid username or password');
        }
        await userRepository_1.userRepository.updateLastLogin(String(user._id));
        const accessToken = this.generateAccessToken(user);
        const refreshToken = this.generateRefreshToken(user);
        logService_1.logger.system.info(`User logged in: ${username}`, { role: user.role });
        return {
            accessToken,
            refreshToken,
            user: {
                id: user._id,
                username: user.username,
                role: user.role,
            },
        };
    }
    async refreshToken(token) {
        try {
            const payload = this.verifyRefreshToken(token);
            const user = await userRepository_1.userRepository.findById(payload.id);
            if (!user) {
                throw new Error('User not found');
            }
            const accessToken = this.generateAccessToken(user);
            const refreshToken = this.generateRefreshToken(user);
            return { accessToken, refreshToken };
        }
        catch {
            throw new Error('Invalid refresh token');
        }
    }
    async register(username, password, email, role = 'user') {
        const existing = await userRepository_1.userRepository.findByUsername(username);
        if (existing) {
            throw new Error('Username already exists');
        }
        const user = await userRepository_1.userRepository.create({
            username,
            password,
            email,
            role,
        });
        logService_1.logger.system.info(`New user registered: ${username}`, { role });
        return {
            id: user._id,
            username: user.username,
            role: user.role,
        };
    }
    async seedAdmin() {
        const count = await userRepository_1.userRepository.count();
        if (count === 0) {
            await this.register(env_1.env.ADMIN_USERNAME, env_1.env.ADMIN_PASSWORD, 'admin@firmware-server.local', 'admin');
            logService_1.logger.system.info('✅ Default admin account seeded');
        }
    }
}
exports.AuthService = AuthService;
exports.authService = new AuthService();
//# sourceMappingURL=authService.js.map