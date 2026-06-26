import { IUserDocument } from '../models/User';
interface TokenPayload {
    id: string;
    username: string;
    role: 'admin' | 'user';
}
export declare class AuthService {
    generateAccessToken(user: IUserDocument): string;
    generateRefreshToken(user: IUserDocument): string;
    verifyAccessToken(token: string): TokenPayload;
    verifyRefreshToken(token: string): TokenPayload;
    login(username: string, password: string): Promise<{
        accessToken: string;
        refreshToken: string;
        user: {
            id: import("mongoose").Types.ObjectId;
            username: string;
            role: "admin" | "user";
        };
    }>;
    refreshToken(token: string): Promise<{
        accessToken: string;
        refreshToken: string;
    }>;
    register(username: string, password: string, email: string, role?: 'admin' | 'user'): Promise<{
        id: import("mongoose").Types.ObjectId;
        username: string;
        role: "admin" | "user";
    }>;
    seedAdmin(): Promise<void>;
}
export declare const authService: AuthService;
export {};
//# sourceMappingURL=authService.d.ts.map