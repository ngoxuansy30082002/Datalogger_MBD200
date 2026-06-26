"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.upload = void 0;
const multer_1 = __importDefault(require("multer"));
const env_1 = require("../config/env");
// Use memory storage so we can compute hash before writing to disk
const storage = multer_1.default.memoryStorage();
const fileFilter = (_req, file, cb) => {
    if (file.originalname.toLowerCase().endsWith('.hex')) {
        cb(null, true);
    }
    else {
        cb(new Error('Only .hex files are allowed'));
    }
};
exports.upload = (0, multer_1.default)({
    storage,
    fileFilter,
    limits: {
        fileSize: env_1.env.MAX_FILE_SIZE,
        files: 10, // max 10 files simultaneously
    },
});
//# sourceMappingURL=upload.js.map