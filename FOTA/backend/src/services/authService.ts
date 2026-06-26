import jwt from 'jsonwebtoken';
import { env } from '../config/env';
import { userRepository } from '../repositories/userRepository';
import { IUserDocument } from '../models/User';
import { logger } from './logService';

interface TokenPayload {
  id: string;
  username: string;
  role: 'admin' | 'user';
}

export class AuthService {
  generateAccessToken(user: IUserDocument): string {
    const payload: TokenPayload = {
      id: String(user._id),
      username: user.username,
      role: user.role,
    };
    return jwt.sign(payload, env.JWT_SECRET, {
      expiresIn: env.JWT_EXPIRY as string,
    } as jwt.SignOptions);
  }

  generateRefreshToken(user: IUserDocument): string {
    const payload: TokenPayload = {
      id: String(user._id),
      username: user.username,
      role: user.role,
    };
    return jwt.sign(payload, env.JWT_REFRESH_SECRET, {
      expiresIn: env.JWT_REFRESH_EXPIRY as string,
    } as jwt.SignOptions);
  }

  verifyAccessToken(token: string): TokenPayload {
    return jwt.verify(token, env.JWT_SECRET) as TokenPayload;
  }

  verifyRefreshToken(token: string): TokenPayload {
    return jwt.verify(token, env.JWT_REFRESH_SECRET) as TokenPayload;
  }

  async login(username: string, password: string) {
    const user = await userRepository.findByUsername(username);
    if (!user) {
      throw new Error('Invalid username or password');
    }

    const isMatch = await user.comparePassword(password);
    if (!isMatch) {
      throw new Error('Invalid username or password');
    }

    await userRepository.updateLastLogin(String(user._id));

    const accessToken = this.generateAccessToken(user);
    const refreshToken = this.generateRefreshToken(user);

    logger.system.info(`User logged in: ${username}`, { role: user.role });

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

  async refreshToken(token: string) {
    try {
      const payload = this.verifyRefreshToken(token);
      const user = await userRepository.findById(payload.id);

      if (!user) {
        throw new Error('User not found');
      }

      const accessToken = this.generateAccessToken(user as IUserDocument);
      const refreshToken = this.generateRefreshToken(user as IUserDocument);

      return { accessToken, refreshToken };
    } catch {
      throw new Error('Invalid refresh token');
    }
  }

  async register(username: string, password: string, email: string, role: 'admin' | 'user' = 'user') {
    const existing = await userRepository.findByUsername(username);
    if (existing) {
      throw new Error('Username already exists');
    }

    const user = await userRepository.create({
      username,
      password,
      email,
      role,
    } as Partial<IUserDocument>);

    logger.system.info(`New user registered: ${username}`, { role });

    return {
      id: user._id,
      username: user.username,
      role: user.role,
    };
  }

  async seedAdmin() {
    const count = await userRepository.count();
    if (count === 0) {
      await this.register(
        env.ADMIN_USERNAME,
        env.ADMIN_PASSWORD,
        'admin@firmware-server.local',
        'admin'
      );
      logger.system.info('✅ Default admin account seeded');
    }
  }
}

export const authService = new AuthService();
