import mongoose, { Schema, Document } from 'mongoose';

export interface IFirmwareDocument extends Document {
  projectName: string;
  deviceModel: string;
  hardwareVersion: string;
  firmwareVersion: string;
  description: string;
  fileName: string;
  fileSize: number;
  fileHash: string;
  uploadTime: Date;
  uploadedBy: string;
  downloadCount: number;
  isLatest: boolean;
  storagePath: string;
}

const firmwareSchema = new Schema<IFirmwareDocument>(
  {
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
  },
  {
    timestamps: true,
  }
);

// Compound unique index to prevent duplicate versions for same device
firmwareSchema.index(
  { deviceModel: 1, hardwareVersion: 1, firmwareVersion: 1 },
  { unique: true }
);

// Index for latest queries
firmwareSchema.index({ deviceModel: 1, hardwareVersion: 1, isLatest: 1 });

export const Firmware = mongoose.model<IFirmwareDocument>('Firmware', firmwareSchema);
