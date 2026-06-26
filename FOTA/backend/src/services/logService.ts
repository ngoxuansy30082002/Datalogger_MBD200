import winston from 'winston';
import DailyRotateFile from 'winston-daily-rotate-file';
import path from 'path';
import { env } from '../config/env';
import fs from 'fs';

// Ensure log directory exists
const logDir = env.LOG_PATH;
if (!fs.existsSync(logDir)) {
  fs.mkdirSync(logDir, { recursive: true });
}

const logFormat = winston.format.combine(
  winston.format.timestamp({ format: 'YYYY-MM-DD HH:mm:ss' }),
  winston.format.errors({ stack: true }),
  winston.format.printf(({ timestamp, level, message, ...meta }) => {
    const metaStr = Object.keys(meta).length ? ` ${JSON.stringify(meta)}` : '';
    return `[${timestamp}] [${level.toUpperCase()}] ${message}${metaStr}`;
  })
);

const createLogger = (filename: string): winston.Logger => {
  const transports: winston.transport[] = [
    new DailyRotateFile({
      filename: path.join(logDir, `${filename}-%DATE%.log`),
      datePattern: 'YYYY-MM-DD',
      maxSize: '20m',
      maxFiles: '30d',
      format: logFormat,
    }),
    // Always log to console (required for Docker log collection)
    new winston.transports.Console({
      format: winston.format.combine(
        winston.format.colorize(),
        winston.format.simple()
      ),
    }),
  ];

  return winston.createLogger({
    level: 'info',
    transports,
  });
};

export const logger = {
  upload: createLogger('upload'),
  mqtt: createLogger('mqtt'),
  download: createLogger('download'),
  system: createLogger('system'),
};
