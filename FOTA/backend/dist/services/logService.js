"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.logger = void 0;
const winston_1 = __importDefault(require("winston"));
const winston_daily_rotate_file_1 = __importDefault(require("winston-daily-rotate-file"));
const path_1 = __importDefault(require("path"));
const env_1 = require("../config/env");
const fs_1 = __importDefault(require("fs"));
// Ensure log directory exists
const logDir = env_1.env.LOG_PATH;
if (!fs_1.default.existsSync(logDir)) {
    fs_1.default.mkdirSync(logDir, { recursive: true });
}
const logFormat = winston_1.default.format.combine(winston_1.default.format.timestamp({ format: 'YYYY-MM-DD HH:mm:ss' }), winston_1.default.format.errors({ stack: true }), winston_1.default.format.printf(({ timestamp, level, message, ...meta }) => {
    const metaStr = Object.keys(meta).length ? ` ${JSON.stringify(meta)}` : '';
    return `[${timestamp}] [${level.toUpperCase()}] ${message}${metaStr}`;
}));
const createLogger = (filename) => {
    const transports = [
        new winston_daily_rotate_file_1.default({
            filename: path_1.default.join(logDir, `${filename}-%DATE%.log`),
            datePattern: 'YYYY-MM-DD',
            maxSize: '20m',
            maxFiles: '30d',
            format: logFormat,
        }),
        // Always log to console (required for Docker log collection)
        new winston_1.default.transports.Console({
            format: winston_1.default.format.combine(winston_1.default.format.colorize(), winston_1.default.format.simple()),
        }),
    ];
    return winston_1.default.createLogger({
        level: 'info',
        transports,
    });
};
exports.logger = {
    upload: createLogger('upload'),
    mqtt: createLogger('mqtt'),
    download: createLogger('download'),
    system: createLogger('system'),
};
//# sourceMappingURL=logService.js.map