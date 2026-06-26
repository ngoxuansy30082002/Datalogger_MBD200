import { Server as HttpServer } from 'http';
import { Server as SocketServer, Socket } from 'socket.io';
import jwt from 'jsonwebtoken';
import { env } from '../config/env';
import { mqttService } from './mqttService';
import { logger } from './logService';

class SocketService {
  private io: SocketServer | null = null;
  private connectedClients: number = 0;

  initialize(server: HttpServer): void {
    this.io = new SocketServer(server, {
      cors: {
        origin: env.CORS_ORIGIN === '*' ? true : env.CORS_ORIGIN,
        methods: ['GET', 'POST'],
        credentials: true,
      },
      path: '/socket.io',
    });

    // JWT authentication middleware for Socket.IO
    this.io.use((socket: Socket, next) => {
      const token = socket.handshake.auth?.token || socket.handshake.query?.token;
      if (!token) {
        return next(new Error('Authentication required'));
      }

      try {
        const decoded = jwt.verify(token as string, env.JWT_SECRET);
        (socket as any).user = decoded;
        next();
      } catch {
        next(new Error('Invalid token'));
      }
    });

    this.io.on('connection', (socket: Socket) => {
      this.connectedClients++;
      logger.system.info(`Socket client connected: ${socket.id}`, {
        user: (socket as any).user?.username,
        total: this.connectedClients,
      });

      socket.join('dashboard');

      socket.on('disconnect', () => {
        this.connectedClients--;
        logger.system.info(`Socket client disconnected: ${socket.id}`, {
          total: this.connectedClients,
        });
      });
    });

    // Forward MQTT messages to Socket.IO clients
    mqttService.onMessage((message) => {
      this.io?.to('dashboard').emit('mqtt:message', message);
    });

    logger.system.info('✅ Socket.IO initialized');
  }

  emitFirmwareUploaded(firmware: object): void {
    this.io?.to('dashboard').emit('firmware:uploaded', firmware);
  }

  emitFirmwareDeleted(firmwareId: string): void {
    this.io?.to('dashboard').emit('firmware:deleted', { id: firmwareId });
  }

  emitDeviceUpdate(device: object): void {
    this.io?.to('dashboard').emit('device:update', device);
  }

  emitStatsUpdate(stats: object): void {
    this.io?.to('dashboard').emit('stats:update', stats);
  }

  getConnectedClients(): number {
    return this.connectedClients;
  }

  getIO(): SocketServer | null {
    return this.io;
  }
}

export const socketService = new SocketService();
