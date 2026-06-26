import axiosInstance from './axiosInstance';
import { FirmwareUploadMeta } from '../types';

export const firmwareApi = {
  list: async (params: Record<string, string | number | boolean | undefined> = {}) => {
    const response = await axiosInstance.get('/firmware', { params });
    return response.data;
  },

  upload: async (
    files: File[],
    metadata: FirmwareUploadMeta,
    onProgress?: (progress: number) => void
  ) => {
    const formData = new FormData();
    files.forEach((file) => formData.append('files', file));
    formData.append('projectName', metadata.projectName);
    formData.append('deviceModel', metadata.deviceModel);
    formData.append('hardwareVersion', metadata.hardwareVersion);
    formData.append('firmwareVersion', metadata.firmwareVersion);
    formData.append('description', metadata.description || '');

    const response = await axiosInstance.post('/firmware/upload', formData, {
      headers: { 'Content-Type': 'multipart/form-data' },
      onUploadProgress: (progressEvent) => {
        if (onProgress && progressEvent.total) {
          const percent = Math.round((progressEvent.loaded * 100) / progressEvent.total);
          onProgress(percent);
        }
      },
    });
    return response.data;
  },

  download: (id: string) => {
    window.open(`/api/firmware/download/${id}`, '_blank');
  },

  delete: async (id: string) => {
    const response = await axiosInstance.delete(`/firmware/${id}`);
    return response.data;
  },

  markLatest: async (id: string) => {
    const response = await axiosInstance.patch(`/firmware/${id}/latest`);
    return response.data;
  },

  checkUpdate: async (deviceModel: string, hardwareVersion: string, currentVersion: string) => {
    const response = await axiosInstance.get('/firmware/latest', {
      params: { deviceModel, hardwareVersion, currentVersion },
    });
    return response.data;
  },

  notifyUpdate: async (firmwareVersion: string = 'latest') => {
    const response = await axiosInstance.post('/mqtt/notify', { firmwareVersion });
    return response.data;
  },
};
