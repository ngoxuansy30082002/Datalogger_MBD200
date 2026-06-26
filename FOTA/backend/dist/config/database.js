"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.disconnectDatabase = exports.connectDatabase = void 0;
const mongoose_1 = __importDefault(require("mongoose"));
const env_1 = require("./env");
const logService_1 = require("../services/logService");
const connectDatabase = async () => {
    try {
        mongoose_1.default.set('strictQuery', false);
        await mongoose_1.default.connect(env_1.env.MONGO_URI, {
            serverSelectionTimeoutMS: 5000,
            socketTimeoutMS: 45000,
        });
        logService_1.logger.system.info('✅ MongoDB connected successfully', {
            uri: env_1.env.MONGO_URI.replace(/\/\/.*@/, '//***@'),
        });
        mongoose_1.default.connection.on('error', (err) => {
            logService_1.logger.system.error('MongoDB connection error:', err);
        });
        mongoose_1.default.connection.on('disconnected', () => {
            logService_1.logger.system.warn('MongoDB disconnected. Attempting reconnection...');
        });
        mongoose_1.default.connection.on('reconnected', () => {
            logService_1.logger.system.info('MongoDB reconnected successfully');
        });
    }
    catch (error) {
        logService_1.logger.system.error('❌ MongoDB connection failed:', error);
        process.exit(1);
    }
};
exports.connectDatabase = connectDatabase;
const disconnectDatabase = async () => {
    await mongoose_1.default.disconnect();
    logService_1.logger.system.info('MongoDB disconnected gracefully');
};
exports.disconnectDatabase = disconnectDatabase;
//# sourceMappingURL=database.js.map