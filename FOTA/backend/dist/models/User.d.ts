import mongoose, { Document } from 'mongoose';
export interface IUserDocument extends Document {
    username: string;
    email: string;
    password: string;
    role: 'admin' | 'user';
    createdAt: Date;
    lastLogin: Date;
    comparePassword(candidatePassword: string): Promise<boolean>;
}
export declare const User: mongoose.Model<IUserDocument, {}, {}, {}, mongoose.Document<unknown, {}, IUserDocument, {}, {}> & IUserDocument & Required<{
    _id: mongoose.Types.ObjectId;
}> & {
    __v: number;
}, any>;
//# sourceMappingURL=User.d.ts.map