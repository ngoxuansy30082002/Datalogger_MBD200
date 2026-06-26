"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.validate = void 0;
const joi_1 = __importDefault(require("joi"));
const schemas = {
    login: joi_1.default.object({
        username: joi_1.default.string().min(3).max(50).required(),
        password: joi_1.default.string().min(6).required(),
    }),
    register: joi_1.default.object({
        username: joi_1.default.string().min(3).max(50).required(),
        password: joi_1.default.string().min(6).required(),
        email: joi_1.default.string().email().allow('').optional(),
        role: joi_1.default.string().valid('admin', 'user').default('user'),
    }),
    firmwareUpload: joi_1.default.object({
        projectName: joi_1.default.string().required().trim(),
        deviceModel: joi_1.default.string().required().trim(),
        hardwareVersion: joi_1.default.string().required().trim(),
        firmwareVersion: joi_1.default.string().required().trim(),
        description: joi_1.default.string().allow('').default(''),
    }),
    firmwareQuery: joi_1.default.object({
        deviceModel: joi_1.default.string().required(),
        hw: joi_1.default.string().required(),
        version: joi_1.default.string().default('latest'),
    }),
    latestQuery: joi_1.default.object({
        deviceModel: joi_1.default.string().required(),
        hardwareVersion: joi_1.default.string().required(),
        currentVersion: joi_1.default.string().required(),
    }),
    refreshToken: joi_1.default.object({
        refreshToken: joi_1.default.string().required(),
    }),
};
const validate = (schemaName, source = 'body') => {
    return (req, res, next) => {
        const schema = schemas[schemaName];
        if (!schema) {
            next();
            return;
        }
        const { error, value } = schema.validate(req[source], { abortEarly: false, stripUnknown: true });
        if (error) {
            const details = error.details.map((d) => d.message).join(', ');
            res.status(400).json({
                success: false,
                message: 'Validation error',
                details,
            });
            return;
        }
        req[source] = value;
        next();
    };
};
exports.validate = validate;
//# sourceMappingURL=validation.js.map