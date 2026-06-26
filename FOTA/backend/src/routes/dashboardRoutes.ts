import { Router } from 'express';
import { 
  getSummary, 
  getWeeklyActivity, 
  getDevices, 
  getRecentFirmware, 
  getMqttLogs 
} from '../controllers/dashboardController';
import { authenticate } from '../middleware/auth';

const router = Router();

router.get('/summary', authenticate, getSummary);
router.get('/weekly-activity', authenticate, getWeeklyActivity);
router.get('/devices', authenticate, getDevices);
router.get('/recent-firmware', authenticate, getRecentFirmware);
router.get('/mqtt-logs', authenticate, getMqttLogs);

export default router;
