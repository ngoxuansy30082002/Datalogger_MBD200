import { Router } from 'express';
import { deviceController } from '../controllers/deviceController';
import { authenticate } from '../middleware/auth';

const router = Router();

/**
 * @swagger
 * /devices:
 *   get:
 *     summary: List registered devices
 *     tags: [Devices]
 *     security:
 *       - bearerAuth: []
 *     parameters:
 *       - in: query
 *         name: page
 *         schema:
 *           type: integer
 *           default: 1
 *       - in: query
 *         name: limit
 *         schema:
 *           type: integer
 *           default: 20
 *     responses:
 *       200:
 *         description: Devices list
 */
router.get('/', authenticate, deviceController.list.bind(deviceController));

/**
 * @swagger
 * /devices/{id}:
 *   get:
 *     summary: Get device details
 *     tags: [Devices]
 *     security:
 *       - bearerAuth: []
 *     parameters:
 *       - in: path
 *         name: id
 *         required: true
 *         schema:
 *           type: string
 *     responses:
 *       200:
 *         description: Device details
 */
router.get('/:id', authenticate, deviceController.getById.bind(deviceController));

export default router;
