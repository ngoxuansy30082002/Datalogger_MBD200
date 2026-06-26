export interface Firmware {
  _id: string;
  projectName: string;
  deviceModel: string;
  hardwareVersion: string;
  firmwareVersion: string;
  description: string;
  fileName: string;
  fileSize: number;
  fileHash: string;
  uploadTime: string;
  uploadedBy: string;
  downloadCount: number;
  isLatest: boolean;
  storagePath: string;
}

export interface Device {
  _id: string;
  deviceId: string;
  deviceModel: string;
  hardwareVersion: string;
  firmwareVersion: string;
  lastSeen: string;
  online: boolean;
  latestStatus?: Record<string, any>;
}

export interface User {
  id: string;
  username: string;
  role: 'admin' | 'user';
  email?: string;
}

export interface LoginResponse {
  success: boolean;
  message?: string;
  accessToken: string;
  refreshToken: string;
  user: User;
}

export interface DashboardSummary {
  totalFirmware: number;
  totalDownloads: number;
  onlineDevices: number;
  offlineDevices: number;
  mqttMessagesToday: number;
  latestFirmwareVersion: string;
}

export interface WeeklyActivity {
  date: string;
  uploads: number;
  downloads: number;
  mqttMessages: number;
}

export interface DevicesSummary {
  online: number;
  offline: number;
  devices: Device[];
}

export interface MqttMessage {
  topic: string;
  payload: string;
  timestamp: string;
}

export interface PaginatedResponse<T> {
  success: boolean;
  data: T[];
  total: number;
  page: number;
  totalPages: number;
}

export interface ApiResponse<T = unknown> {
  success: boolean;
  message?: string;
  data?: T;
}

export interface FirmwareUploadMeta {
  projectName: string;
  deviceModel: string;
  hardwareVersion: string;
  firmwareVersion: string;
  description: string;
}
