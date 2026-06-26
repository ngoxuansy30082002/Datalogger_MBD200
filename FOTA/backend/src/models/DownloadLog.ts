import mongoose, { Schema, Document } from 'mongoose';

export interface IDownloadLogDocument extends Document {
  firmwareId: mongoose.Types.ObjectId;
  deviceId: string;
  ipAddress: string;
  timestamp: Date;
  userAgent: string;
}

const downloadLogSchema = new Schema<IDownloadLogDocument>(
  {
    firmwareId: {
      type: Schema.Types.ObjectId,
      ref: 'Firmware',
      required: true,
    },
    deviceId: {
      type: String,
      default: '',
    },
    ipAddress: {
      type: String,
      required: true,
    },
    timestamp: {
      type: Date,
      default: Date.now,
    },
    userAgent: {
      type: String,
      default: '',
    },
  },
  {
    timestamps: true,
  }
);

downloadLogSchema.index({ firmwareId: 1 });
downloadLogSchema.index({ timestamp: -1 });

export const DownloadLog = mongoose.model<IDownloadLogDocument>('DownloadLog', downloadLogSchema);
