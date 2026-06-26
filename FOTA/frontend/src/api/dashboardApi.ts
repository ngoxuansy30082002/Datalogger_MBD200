import axiosInstance from './axiosInstance';

export const dashboardApi = {
  getSummary: async () => {
    const response = await axiosInstance.get('/dashboard/summary');
    return response.data;
  },
  getWeeklyActivity: async () => {
    const response = await axiosInstance.get('/dashboard/weekly-activity');
    return response.data;
  },
  getDevices: async () => {
    const response = await axiosInstance.get('/dashboard/devices');
    return response.data;
  },
  getRecentFirmware: async () => {
    const response = await axiosInstance.get('/dashboard/recent-firmware');
    return response.data;
  },
  getMqttLogs: async () => {
    const response = await axiosInstance.get('/dashboard/mqtt-logs');
    return response.data;
  }
};
