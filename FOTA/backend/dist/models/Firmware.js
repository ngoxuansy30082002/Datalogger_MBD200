"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.Firmware = void 0;
const mongoose_1 = __importStar(require("mongoose"));
const firmwareSchema = new mongoose_1.Schema({
    projectName: {
        type: String,
        required: true,
        trim: true,
        index: true,
    },
    deviceModel: {
        type: String,
        required: true,
        trim: true,
        index: true,
    },
    hardwareVersion: {
        type: String,
        required: true,
        trim: true,
    },
    firmwareVersion: {
        type: String,
        required: true,
        trim: true,
    },
    description: {
        type: String,
        default: '',
    },
    fileName: {
        type: String,
        required: true,
    },
    fileSize: {
        type: Number,
        required: true,
    },
    fileHash: {
        type: String,
        required: true,
        index: true,
    },
    uploadTime: {
        type: Date,
        default: Date.now,
    },
    uploadedBy: {
        type: String,
        required: true,
    },
    downloadCount: {
        type: Number,
        default: 0,
    },
    isLatest: {
        type: Boolean,
        default: false,
    },
    storagePath: {
        type: String,
        required: true,
    },
}, {
    timestamps: true,
});
// Compound unique index to prevent duplicate versions for same device
firmwareSchema.index({ deviceModel: 1, hardwareVersion: 1, firmwareVersion: 1 }, { unique: true });
// Index for latest queries
firmwareSchema.index({ deviceModel: 1, hardwareVersion: 1, isLatest: 1 });
exports.Firmware = mongoose_1.default.model('Firmware', firmwareSchema);
//# sourceMappingURL=Firmware.js.map