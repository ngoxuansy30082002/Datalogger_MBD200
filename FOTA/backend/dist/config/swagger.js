"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.swaggerSpec = void 0;
const swagger_jsdoc_1 = __importDefault(require("swagger-jsdoc"));
const path_1 = __importDefault(require("path"));
const options = {
    definition: {
        openapi: '3.0.0',
        info: {
            title: 'Firmware Management Server API',
            version: '1.0.0',
            description: 'API documentation for Firmware Management Server - Manage firmware (.hex) files for embedded devices',
            contact: {
                name: 'Admin',
            },
        },
        servers: [
            {
                url: '/api',
                description: 'API Server',
            },
        ],
        components: {
            securitySchemes: {
                bearerAuth: {
                    type: 'http',
                    scheme: 'bearer',
                    bearerFormat: 'JWT',
                },
            },
            schemas: {
                Firmware: {
                    type: 'object',
                    properties: {
                        _id: { type: 'string' },
                        projectName: { type: 'string', example: 'ControllerV5' },
                        deviceModel: { type: 'string', example: 'Relay64x2' },
                        hardwareVersion: { type: 'string', example: '1.0' },
                        firmwareVersion: { type: 'string', example: '5.1.0' },
                        description: { type: 'string', example: 'Release note' },
                        fileName: { type: 'string', example: 'Relay64x2_v5.1.0.hex' },
                        fileSize: { type: 'number', example: 123456 },
                        fileHash: { type: 'string', example: 'sha256hash...' },
                        uploadTime: { type: 'string', format: 'date-time' },
                        uploadedBy: { type: 'string', example: 'admin' },
                        downloadCount: { type: 'number', example: 0 },
                        isLatest: { type: 'boolean', example: true },
                        storagePath: { type: 'string' },
                    },
                },
                Device: {
                    type: 'object',
                    properties: {
                        _id: { type: 'string' },
                        deviceId: { type: 'string', example: 'ABC123456' },
                        deviceModel: { type: 'string', example: 'Relay64x2' },
                        hardwareVersion: { type: 'string', example: '1.0' },
                        currentFirmware: { type: 'string', example: '5.0.0' },
                        lastSeen: { type: 'string', format: 'date-time' },
                        status: { type: 'string', enum: ['online', 'offline'] },
                    },
                },
                LoginRequest: {
                    type: 'object',
                    required: ['username', 'password'],
                    properties: {
                        username: { type: 'string', example: 'admin' },
                        password: { type: 'string', example: 'admin123' },
                    },
                },
                LoginResponse: {
                    type: 'object',
                    properties: {
                        accessToken: { type: 'string' },
                        refreshToken: { type: 'string' },
                        user: {
                            type: 'object',
                            properties: {
                                id: { type: 'string' },
                                username: { type: 'string' },
                                role: { type: 'string' },
                            },
                        },
                    },
                },
                UpdateCheckResponse: {
                    type: 'object',
                    properties: {
                        hasUpdate: { type: 'boolean' },
                        latestVersion: { type: 'string' },
                        downloadUrl: { type: 'string' },
                        releaseNote: { type: 'string' },
                    },
                },
                Error: {
                    type: 'object',
                    properties: {
                        success: { type: 'boolean', example: false },
                        message: { type: 'string' },
                        error: { type: 'string' },
                    },
                },
            },
        },
        security: [{ bearerAuth: [] }],
    },
    apis: [path_1.default.resolve(__dirname, '../routes/*.{ts,js}')],
};
exports.swaggerSpec = (0, swagger_jsdoc_1.default)(options);
//# sourceMappingURL=swagger.js.map