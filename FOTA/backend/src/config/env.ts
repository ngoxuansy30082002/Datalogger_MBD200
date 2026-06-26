import dotenv from 'dotenv';
import path from 'path';

// Load .env from backend/ first, then fallback to project root
dotenv.config({ path: path.resolve(__dirname, '../../.env') });
dotenv.config({ path: path.resolve(__dirname, '../../../.env') });

export const env = {
  NODE_ENV: process.env.NODE_ENV || 'development',
  PORT: parseInt(process.env.PORT || '80', 10),
  HTTPS_PORT: parseInt(process.env.HTTPS_PORT || '443', 10),
  MONGO_URI: process.env.MONGO_URI || 'mongodb://localhost:27017/firmware_manager',
  MQTT_BROKER_URL: process.env.MQTT_BROKER_URL || 'mqtt://broker.emqx.io:1883',
  JWT_SECRET: process.env.JWT_SECRET || 'fw-mgmt-jwt-secret-change-me',
  JWT_REFRESH_SECRET: process.env.JWT_REFRESH_SECRET || 'fw-mgmt-refresh-secret-change-me',
  JWT_EXPIRY: process.env.JWT_EXPIRY || '1h',
  JWT_REFRESH_EXPIRY: process.env.JWT_REFRESH_EXPIRY || '7d',
  STORAGE_PATH: process.env.STORAGE_PATH || path.resolve(__dirname, '../../storage'),
  LOG_PATH: process.env.LOG_PATH || path.resolve(__dirname, '../../logs'),
  CERT_PATH: process.env.CERT_PATH || path.resolve(__dirname, '../../../certs'),
  MAX_FILE_SIZE: parseInt(process.env.MAX_FILE_SIZE || '52428800', 10), // 50MB
  ADMIN_USERNAME: process.env.ADMIN_USERNAME || 'admin',
  ADMIN_PASSWORD: process.env.ADMIN_PASSWORD || 'admin123',
  CORS_ORIGIN: process.env.CORS_ORIGIN || '*',
  SERVER_BASE_URL: process.env.SERVER_BASE_URL || 'http://localhost',
};
