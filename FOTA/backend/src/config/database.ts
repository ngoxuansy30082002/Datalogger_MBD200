import mongoose from 'mongoose';
import { env } from './env';
import { logger } from '../services/logService';

export const connectDatabase = async (): Promise<void> => {
  try {
    mongoose.set('strictQuery', false);
    
    await mongoose.connect(env.MONGO_URI, {
      serverSelectionTimeoutMS: 5000,
      socketTimeoutMS: 45000,
    });

    logger.system.info('✅ MongoDB connected successfully', {
      uri: env.MONGO_URI.replace(/\/\/.*@/, '//***@'),
    });

    mongoose.connection.on('error', (err) => {
      logger.system.error('MongoDB connection error:', err);
    });

    mongoose.connection.on('disconnected', () => {
      logger.system.warn('MongoDB disconnected. Attempting reconnection...');
    });

    mongoose.connection.on('reconnected', () => {
      logger.system.info('MongoDB reconnected successfully');
    });
  } catch (error) {
    logger.system.error('❌ MongoDB connection failed:', error);
    process.exit(1);
  }
};

export const disconnectDatabase = async (): Promise<void> => {
  await mongoose.disconnect();
  logger.system.info('MongoDB disconnected gracefully');
};
