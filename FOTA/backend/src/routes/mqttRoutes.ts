import { Router } from 'express';
import { mqttController } from '../controllers/mqttController';
import { authenticate } from '../middleware/auth';
import { requireRole } from '../middleware/roleGuard';

const router = Router();

/**
 * @swagger
 * /mqtt/notify:
 *   post:
 *     summary: Publish firmware update notification (admin only)
 *     tags: [MQTT]
 *     security:
 *       - bearerAuth: []
 *     requestBody:
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               firmwareVersion:
 *                 type: string
 *                 default: latest
 *     responses:
 *       200:
 *         description: Notification published
 */
router.post('/notify', authenticate, requireRole('admin'), mqttController.publishNotify.bind(mqttController));

/**
 * @swagger
 * /mqtt/status:
 *   get:
 *     summary: Get MQTT connection status
 *     tags: [MQTT]
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: MQTT status
 */
router.get('/status', authenticate, mqttController.getStatus.bind(mqttController));

export default router;
