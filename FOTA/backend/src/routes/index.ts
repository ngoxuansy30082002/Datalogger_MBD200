import { Router } from 'express';
import firmwareRoutes from './firmwareRoutes';
import authRoutes from './authRoutes';
import dashboardRoutes from './dashboardRoutes';
import mqttRoutes from './mqttRoutes';
import deviceRoutes from './deviceRoutes';

const router = Router();

router.use('/firmware', firmwareRoutes);
router.use('/auth', authRoutes);
router.use('/dashboard', dashboardRoutes);
router.use('/mqtt', mqttRoutes);
router.use('/devices', deviceRoutes);

export default router;
