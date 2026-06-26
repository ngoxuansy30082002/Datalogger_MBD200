import http from 'http';
import https from 'https';
import app from './app';
import { env } from './config/env';
import { connectDatabase } from './config/database';
import { mqttService } from './services/mqttService';
import { socketService } from './services/socketService';
import { authService } from './services/authService';
import { loadCerts, generateSelfSignedCert } from './utils/certGenerator';
import { ensureDir } from './utils/hash';
import { logger } from './services/logService';

async function startServer(): Promise<void> {
  // Ensure storage directories exist
  ensureDir(env.STORAGE_PATH);
  ensureDir(`${env.STORAGE_PATH}/firmware`);
  ensureDir(env.LOG_PATH);

  // Connect to MongoDB
  await connectDatabase();

  // Seed default admin user
  await authService.seedAdmin();

  // Try to set up HTTPS
  let primaryServer: http.Server | https.Server;

  const sslCerts = loadCerts();

  if (sslCerts) {
    // HTTPS mode
    primaryServer = https.createServer(sslCerts, app);

    // HTTP redirect server
    const httpApp = http.createServer((req, res) => {
      const host = req.headers.host?.replace(/:\d+$/, '') || 'localhost';
      res.writeHead(301, {
        Location: `https://${host}:${env.HTTPS_PORT}${req.url}`,
      });
      res.end();
    });

    httpApp.listen(env.PORT, () => {
      logger.system.info(`🔄 HTTP redirect server running on port ${env.PORT}`);
    });

    // Start HTTPS server
    primaryServer.listen(env.HTTPS_PORT, () => {
      logger.system.info(`🔒 HTTPS server running on port ${env.HTTPS_PORT}`);
      logger.system.info(`📚 API docs: https://localhost:${env.HTTPS_PORT}/api-docs`);
    });
  } else {
    // HTTP-only mode (development)
    logger.system.warn('⚠️ SSL certificates not found, running in HTTP-only mode');
    logger.system.info('💡 To generate certificates, run: npm run generate-certs');

    primaryServer = http.createServer(app);

    primaryServer.listen(env.PORT, () => {
      logger.system.info(`🚀 HTTP server running on port ${env.PORT}`);
      logger.system.info(`📚 API docs: http://localhost:${env.PORT}/api-docs`);
    });
  }

  // Initialize Socket.IO
  socketService.initialize(primaryServer as http.Server);

  // Connect MQTT
  try {
    mqttService.connect();
  } catch (error) {
    logger.system.warn('⚠️ MQTT broker not available, MQTT service disabled', { error });
  }

  // Log startup info
  logger.system.info('═══════════════════════════════════════');
  logger.system.info('  Firmware Management Server Started   ');
  logger.system.info('═══════════════════════════════════════');
  logger.system.info(`  Environment: ${env.NODE_ENV}`);
  logger.system.info(`  MongoDB:     ${env.MONGO_URI}`);
  logger.system.info(`  MQTT:        ${env.MQTT_BROKER_URL}`);
  logger.system.info('═══════════════════════════════════════');

  // Graceful shutdown
  const shutdown = async (signal: string) => {
    logger.system.info(`\n${signal} received. Shutting down gracefully...`);
    mqttService.disconnect();
    primaryServer.close();
    process.exit(0);
  };

  process.on('SIGTERM', () => shutdown('SIGTERM'));
  process.on('SIGINT', () => shutdown('SIGINT'));
}

startServer().catch((error) => {
  logger.system.error('Failed to start server:', error);
  process.exit(1);
});
