import axiosInstance from './axiosInstance';

export const deviceApi = {
  list: async (params: Record<string, string | number> = {}) => {
    const response = await axiosInstance.get('/devices', { params });
    return response.data;
  },

  getById: async (id: string) => {
    const response = await axiosInstance.get(`/devices/${id}`);
    return response.data;
  },
};
