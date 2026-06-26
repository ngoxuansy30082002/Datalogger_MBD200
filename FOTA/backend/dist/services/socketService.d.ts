import { Server as HttpServer } from 'http';
import { Server as SocketServer } from 'socket.io';
declare class SocketService {
    private io;
    private connectedClients;
    initialize(server: HttpServer): void;
    emitFirmwareUploaded(firmware: object): void;
    emitFirmwareDeleted(firmwareId: string): void;
    emitDeviceUpdate(device: object): void;
    emitStatsUpdate(stats: object): void;
    getConnectedClients(): number;
    getIO(): SocketServer | null;
}
export declare const socketService: SocketService;
export {};
//# sourceMappingURL=socketService.d.ts.map