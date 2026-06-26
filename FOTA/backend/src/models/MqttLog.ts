import mongoose, { Schema, Document } from 'mongoose';

export interface IMqttLogDocument extends Document {
  topic: string;
  payload: string;
  timestamp: Date;
}

const mqttLogSchema = new Schema<IMqttLogDocument>(
  {
    topic: {
      type: String,
      required: true,
      index: true,
    },
    payload: {
      type: String,
      default: '',
    },
    timestamp: {
      type: Date,
      default: Date.now,
    },
  },
  {
    timestamps: true,
  }
);

mqttLogSchema.index({ timestamp: -1 });

export const MqttLog = mongoose.model<IMqttLogDocument>('MqttLog', mqttLogSchema);
