import mongoose, { Document } from 'mongoose';
export interface IDownloadLogDocument extends Document {
    firmwareId: mongoose.Types.ObjectId;
    deviceId: string;
    ipAddress: string;
    timestamp: Date;
    userAgent: string;
}
export declare const DownloadLog: mongoose.Model<IDownloadLogDocument, {}, {}, {}, mongoose.Document<unknown, {}, IDownloadLogDocument, {}, {}> & IDownloadLogDocument & Required<{
    _id: mongoose.Types.ObjectId;
}> & {
    __v: number;
}, any>;
//# sourceMappingURL=DownloadLog.d.ts.map