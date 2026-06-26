import { Request } from 'express';

export interface IUser {
  _id: string;
  username: string;
  email: string;
  password: string;
  role: 'admin' | 'user';
  createdAt: Date;
  lastLogin: Date;
  comparePassword(candidatePassword: string): Promise<boolean>;
}

export interface IFirmware {
  _id: string;
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

export interface IDevice {
  _id: string;
  deviceId: string;
  deviceModel: string;
  hardwareVersion: string;
  currentFirmware: string;
  lastSeen: Date;
  status: 'online' | 'offline';
}

export interface IDownloadLog {
  _id: string;
  firmwareId: string;
  deviceId?: string;
  ipAddress: string;
  timestamp: Date;
  userAgent: string;
}

export interface AuthRequest extends Request {
  user?: {
    id: string;
    username: string;
    role: 'admin' | 'user';
  };
}

export interface PaginationQuery {
  page?: number;
  limit?: number;
  search?: string;
  sortBy?: string;
  sortOrder?: 'asc' | 'desc';
}

export interface FirmwareFilter extends PaginationQuery {
  projectName?: string;
  deviceModel?: string;
  hardwareVersion?: string;
  isLatest?: boolean;
}

export interface MqttFirmwareQuery {
  deviceId: string;
  deviceModel: string;
  hardwareVersion: string;
  currentFirmware: string;
}

export interface FirmwareUpdateResponse {
  hasUpdate: boolean;
  latestVersion?: string;
  downloadUrl?: string;
  releaseNote?: string;
}

export interface DashboardStats {
  totalFirmware: number;
  totalDownloads: number;
  connectedDevices: number;
  latestUploads: IFirmware[];
}

export interface MqttMessage {
  topic: string;
  payload: string;
  timestamp: string;
}
