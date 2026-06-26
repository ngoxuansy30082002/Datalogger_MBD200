import mongoose, { Schema, Document } from 'mongoose';

export interface IDeviceDocument extends Document {
  deviceId: string;
  deviceModel: string;
  hardwareVersion: string;
  firmwareVersion: string;
  lastSeen: Date;
  online: boolean;
  latestStatus?: Record<string, any>;
}

const deviceSchema = new Schema<IDeviceDocument>(
  {
    deviceId: {
      type: String,
      required: true,
      unique: true,
      trim: true,
      index: true,
    },
    deviceModel: {
      type: String,
      required: true,
      trim: true,
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
    lastSeen: {
      type: Date,
      default: Date.now,
    },
    online: {
      type: Boolean,
      default: false,
    },
    latestStatus: {
      type: Schema.Types.Mixed,
      default: {},
    },
  },
  {
    timestamps: true,
  }
);

export const Device = mongoose.model<IDeviceDocument>('Device', deviceSchema);
