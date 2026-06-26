import { IUserDocument } from '../models/User';
export declare class UserRepository {
    create(data: Partial<IUserDocument>): Promise<IUserDocument>;
    findById(id: string): Promise<IUserDocument | null>;
    findByUsername(username: string): Promise<IUserDocument | null>;
    findAll(): Promise<IUserDocument[]>;
    updateLastLogin(id: string): Promise<void>;
    delete(id: string): Promise<IUserDocument | null>;
    count(): Promise<number>;
}
export declare const userRepository: UserRepository;
//# sourceMappingURL=userRepository.d.ts.map