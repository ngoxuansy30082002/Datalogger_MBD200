import mongoose, { Document } from 'mongoose';
export interface IDeviceDocument extends Document {
    deviceId: string;
    deviceModel: string;
    hardwareVersion: string;
    firmwareVersion: string;
    lastSeen: Date;
    online: boolean;
    latestStatus?: Record<string, any>;
}
export declare const Device: mongoose.Model<IDeviceDocument, {}, {}, {}, mongoose.Document<unknown, {}, IDeviceDocument, {}, {}> & IDeviceDocument & Required<{
    _id: mongoose.Types.ObjectId;
}> & {
    __v: number;
}, any>;
//# sourceMappingURL=Device.d.ts.map