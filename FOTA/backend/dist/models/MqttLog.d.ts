import mongoose, { Document } from 'mongoose';
export interface IMqttLogDocument extends Document {
    topic: string;
    payload: string;
    timestamp: Date;
}
export declare const MqttLog: mongoose.Model<IMqttLogDocument, {}, {}, {}, mongoose.Document<unknown, {}, IMqttLogDocument, {}, {}> & IMqttLogDocument & Required<{
    _id: mongoose.Types.ObjectId;
}> & {
    __v: number;
}, any>;
//# sourceMappingURL=MqttLog.d.ts.map