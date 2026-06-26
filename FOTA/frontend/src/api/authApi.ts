import axiosInstance from './axiosInstance';
import { LoginResponse } from '../types';

export const authApi = {
  login: async (username: string, password: string): Promise<LoginResponse> => {
    const response = await axiosInstance.post('/auth/login', { username, password });
    return response.data;
  },

  register: async (username: string, password: string, email: string, role: string) => {
    const response = await axiosInstance.post('/auth/register', { username, password, email, role });
    return response.data;
  },

  refresh: async (refreshToken: string) => {
    const response = await axiosInstance.post('/auth/refresh', { refreshToken });
    return response.data;
  },

  getProfile: async () => {
    const response = await axiosInstance.get('/auth/profile');
    return response.data;
  },

  getUsers: async () => {
    const response = await axiosInstance.get('/auth/users');
    return response.data;
  },
};
