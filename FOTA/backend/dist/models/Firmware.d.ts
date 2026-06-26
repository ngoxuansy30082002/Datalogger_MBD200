import mongoose, { Document } from 'mongoose';
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
export declare const Firmware: mongoose.Model<IFirmwareDocument, {}, {}, {}, mongoose.Document<unknown, {}, IFirmwareDocument, {}, {}> & IFirmwareDocument & Required<{
    _id: mongoose.Types.ObjectId;
}> & {
    __v: number;
}, any>;
//# sourceMappingURL=Firmware.d.ts.map