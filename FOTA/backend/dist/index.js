"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const http_1 = __importDefault(require("http"));
const https_1 = __importDefault(require("https"));
const app_1 = __importDefault(require("./app"));
const env_1 = require("./config/env");
const database_1 = require("./config/database");
const mqttService_1 = require("./services/mqttService");
const socketService_1 = require("./services/socketService");
const authService_1 = require("./services/authService");
const certGenerator_1 = require("./utils/certGenerator");
const hash_1 = require("./utils/hash");
const logService_1 = require("./services/logService");
async function startServer() {
    // Ensure storage directories exist
    (0, hash_1.ensureDir)(env_1.env.STORAGE_PATH);
    (0, hash_1.ensureDir)(`${env_1.env.STORAGE_PATH}/firmware`);
    (0, hash_1.ensureDir)(env_1.env.LOG_PATH);
    // Connect to MongoDB
    await (0, database_1.connectDatabase)();
    // Seed default admin user
    await authService_1.authService.seedAdmin();
    // Try to set up HTTPS
    let primaryServer;
    const sslCerts = (0, certGenerator_1.loadCerts)();
    if (sslCerts) {
        // HTTPS mode
        primaryServer = https_1.default.createServer(sslCerts, app_1.default);
        // HTTP redirect server
        const httpApp = http_1.default.createServer((req, res) => {
            const host = req.headers.host?.replace(/:\d+$/, '') || 'localhost';
            res.writeHead(301, {
                Location: `https://${host}:${env_1.env.HTTPS_PORT}${req.url}`,
            });
            res.end();
        });
        httpApp.listen(env_1.env.PORT, () => {
            logService_1.logger.system.info(`🔄 HTTP redirect server running on port ${env_1.env.PORT}`);
        });
        // Start HTTPS server
        primaryServer.listen(env_1.env.HTTPS_PORT, () => {
            logService_1.logger.system.info(`🔒 HTTPS server running on port ${env_1.env.HTTPS_PORT}`);
            logService_1.logger.system.info(`📚 API docs: https://localhost:${env_1.env.HTTPS_PORT}/api-docs`);
        });
    }
    else {
        // HTTP-only mode (development)
        logService_1.logger.system.warn('⚠️ SSL certificates not found, running in HTTP-only mode');
        logService_1.logger.system.info('💡 To generate certificates, run: npm run generate-certs');
        primaryServer = http_1.default.createServer(app_1.default);
        primaryServer.listen(env_1.env.PORT, () => {
            logService_1.logger.system.info(`🚀 HTTP server running on port ${env_1.env.PORT}`);
            logService_1.logger.system.info(`📚 API docs: http://localhost:${env_1.env.PORT}/api-docs`);
        });
    }
    // Initialize Socket.IO
    socketService_1.socketService.initialize(primaryServer);
    // Connect MQTT
    try {
        mqttService_1.mqttService.connect();
    }
    catch (error) {
        logService_1.logger.system.warn('⚠️ MQTT broker not available, MQTT service disabled', { error });
    }
    // Log startup info
    logService_1.logger.system.info('═══════════════════════════════════════');
    logService_1.logger.system.info('  Firmware Management Server Started   ');
    logService_1.logger.system.info('═══════════════════════════════════════');
    logService_1.logger.system.info(`  Environment: ${env_1.env.NODE_ENV}`);
    logService_1.logger.system.info(`  MongoDB:     ${env_1.env.MONGO_URI}`);
    logService_1.logger.system.info(`  MQTT:        ${env_1.env.MQTT_BROKER_URL}`);
    logService_1.logger.system.info('═══════════════════════════════════════');
    // Graceful shutdown
    const shutdown = async (signal) => {
        logService_1.logger.system.info(`\n${signal} received. Shutting down gracefully...`);
        mqttService_1.mqttService.disconnect();
        primaryServer.close();
        process.exit(0);
    };
    process.on('SIGTERM', () => shutdown('SIGTERM'));
    process.on('SIGINT', () => shutdown('SIGINT'));
}
startServer().catch((error) => {
    logService_1.logger.system.error('Failed to start server:', error);
    process.exit(1);
});
//# sourceMappingURL=index.js.map