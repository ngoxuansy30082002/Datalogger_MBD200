import { Request, Response } from 'express';
import { dashboardService } from '../services/dashboardService';
import { logger } from '../services/logService';

export const getSummary = async (req: Request, res: Response) => {
  try {
    const summary = await dashboardService.getSummary();
    res.json({ success: true, data: summary });
  } catch (error) {
    logger.system.error('Error fetching dashboard summary:', error);
    res.status(500).json({ success: false, message: 'Server error' });
  }
};

export const getWeeklyActivity = async (req: Request, res: Response) => {
  try {
    const activity = await dashboardService.getWeeklyActivity();
    res.json({ success: true, data: activity.data });
  } catch (error) {
    logger.system.error('Error fetching dashboard weekly activity:', error);
    res.status(500).json({ success: false, message: 'Server error' });
  }
};

export const getDevices = async (req: Request, res: Response) => {
  try {
    const devices = await dashboardService.getDevices();
    res.json({ success: true, data: devices });
  } catch (error) {
    logger.system.error('Error fetching dashboard devices:', error);
    res.status(500).json({ success: false, message: 'Server error' });
  }
};

export const getRecentFirmware = async (req: Request, res: Response) => {
  try {
    const firmware = await dashboardService.getRecentFirmware();
    res.json({ success: true, data: firmware });
  } catch (error) {
    logger.system.error('Error fetching dashboard recent firmware:', error);
    res.status(500).json({ success: false, message: 'Server error' });
  }
};

export const getMqttLogs = async (req: Request, res: Response) => {
  try {
    const logs = await dashboardService.getMqttLogs();
    res.json({ success: true, data: logs });
  } catch (error) {
    logger.system.error('Error fetching dashboard mqtt logs:', error);
    res.status(500).json({ success: false, message: 'Server error' });
  }
};
