"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.socketService = void 0;
const socket_io_1 = require("socket.io");
const jsonwebtoken_1 = __importDefault(require("jsonwebtoken"));
const env_1 = require("../config/env");
const mqttService_1 = require("./mqttService");
const logService_1 = require("./logService");
class SocketService {
    io = null;
    connectedClients = 0;
    initialize(server) {
        this.io = new socket_io_1.Server(server, {
            cors: {
                origin: env_1.env.CORS_ORIGIN === '*' ? true : env_1.env.CORS_ORIGIN,
                methods: ['GET', 'POST'],
                credentials: true,
            },
            path: '/socket.io',
        });
        // JWT authentication middleware for Socket.IO
        this.io.use((socket, next) => {
            const token = socket.handshake.auth?.token || socket.handshake.query?.token;
            if (!token) {
                return next(new Error('Authentication required'));
            }
            try {
                const decoded = jsonwebtoken_1.default.verify(token, env_1.env.JWT_SECRET);
                socket.user = decoded;
                next();
            }
            catch {
                next(new Error('Invalid token'));
            }
        });
        this.io.on('connection', (socket) => {
            this.connectedClients++;
            logService_1.logger.system.info(`Socket client connected: ${socket.id}`, {
                user: socket.user?.username,
                total: this.connectedClients,
            });
            socket.join('dashboard');
            socket.on('disconnect', () => {
                this.connectedClients--;
                logService_1.logger.system.info(`Socket client disconnected: ${socket.id}`, {
                    total: this.connectedClients,
                });
            });
        });
        // Forward MQTT messages to Socket.IO clients
        mqttService_1.mqttService.onMessage((message) => {
            this.io?.to('dashboard').emit('mqtt:message', message);
        });
        logService_1.logger.system.info('✅ Socket.IO initialized');
    }
    emitFirmwareUploaded(firmware) {
        this.io?.to('dashboard').emit('firmware:uploaded', firmware);
    }
    emitFirmwareDeleted(firmwareId) {
        this.io?.to('dashboard').emit('firmware:deleted', { id: firmwareId });
    }
    emitDeviceUpdate(device) {
        this.io?.to('dashboard').emit('device:update', device);
    }
    emitStatsUpdate(stats) {
        this.io?.to('dashboard').emit('stats:update', stats);
    }
    getConnectedClients() {
        return this.connectedClients;
    }
    getIO() {
        return this.io;
    }
}
exports.socketService = new SocketService();
//# sourceMappingURL=socketService.js.map