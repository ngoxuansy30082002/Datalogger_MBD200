import { Router } from 'express';
import { firmwareController } from '../controllers/firmwareController';
import { authenticate, optionalAuth } from '../middleware/auth';
import { requireRole } from '../middleware/roleGuard';
import { upload } from '../middleware/upload';

const router = Router();

/**
 * @swagger
 * /firmware:
 *   get:
 *     summary: List firmware with pagination and filters
 *     tags: [Firmware]
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
 *       - in: query
 *         name: search
 *         schema:
 *           type: string
 *       - in: query
 *         name: projectName
 *         schema:
 *           type: string
 *       - in: query
 *         name: deviceModel
 *         schema:
 *           type: string
 *       - in: query
 *         name: hardwareVersion
 *         schema:
 *           type: string
 *       - in: query
 *         name: isLatest
 *         schema:
 *           type: boolean
 *       - in: query
 *         name: sortBy
 *         schema:
 *           type: string
 *           default: uploadTime
 *       - in: query
 *         name: sortOrder
 *         schema:
 *           type: string
 *           enum: [asc, desc]
 *           default: desc
 *     responses:
 *       200:
 *         description: Firmware list
 */
router.get('/', authenticate, firmwareController.list.bind(firmwareController));

/**
 * @swagger
 * /firmware/upload:
 *   post:
 *     summary: Upload firmware files (admin only)
 *     tags: [Firmware]
 *     security:
 *       - bearerAuth: []
 *     consumes:
 *       - multipart/form-data
 *     requestBody:
 *       content:
 *         multipart/form-data:
 *           schema:
 *             type: object
 *             required:
 *               - files
 *               - projectName
 *               - deviceModel
 *               - hardwareVersion
 *               - firmwareVersion
 *             properties:
 *               files:
 *                 type: array
 *                 items:
 *                   type: string
 *                   format: binary
 *               projectName:
 *                 type: string
 *               deviceModel:
 *                 type: string
 *               hardwareVersion:
 *                 type: string
 *               firmwareVersion:
 *                 type: string
 *               description:
 *                 type: string
 *     responses:
 *       201:
 *         description: Firmware uploaded
 */
router.post(
  '/upload',
  authenticate,
  requireRole('admin'),
  upload.array('files', 10),
  firmwareController.upload.bind(firmwareController)
);

/**
 * @swagger
 * /firmware/download:
 *   get:
 *     summary: Download firmware by query parameters
 *     tags: [Firmware]
 *     security: []
 *     parameters:
 *       - in: query
 *         name: deviceModel
 *         required: true
 *         schema:
 *           type: string
 *       - in: query
 *         name: hw
 *         required: true
 *         schema:
 *           type: string
 *       - in: query
 *         name: version
 *         schema:
 *           type: string
 *           default: latest
 *     responses:
 *       200:
 *         description: Firmware file
 *         content:
 *           application/octet-stream:
 *             schema:
 *               type: string
 *               format: binary
 */
router.get('/download', firmwareController.downloadByQuery.bind(firmwareController));

/**
 * @swagger
 * /firmware/download/{id}:
 *   get:
 *     summary: Download firmware by ID
 *     tags: [Firmware]
 *     security: []
 *     parameters:
 *       - in: path
 *         name: id
 *         required: true
 *         schema:
 *           type: string
 *     responses:
 *       200:
 *         description: Firmware file
 */
router.get('/download/:id', firmwareController.downloadById.bind(firmwareController));

/**
 * @swagger
 * /firmware/latest:
 *   get:
 *     summary: Check for firmware updates
 *     tags: [Firmware]
 *     security: []
 *     parameters:
 *       - in: query
 *         name: deviceModel
 *         required: true
 *         schema:
 *           type: string
 *       - in: query
 *         name: hardwareVersion
 *         required: true
 *         schema:
 *           type: string
 *       - in: query
 *         name: currentVersion
 *         required: true
 *         schema:
 *           type: string
 *     responses:
 *       200:
 *         description: Update check result
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/UpdateCheckResponse'
 */
router.get('/latest', firmwareController.checkLatest.bind(firmwareController));

/**
 * @swagger
 * /firmware/{id}:
 *   get:
 *     summary: Get firmware by ID
 *     tags: [Firmware]
 *     parameters:
 *       - in: path
 *         name: id
 *         required: true
 *         schema:
 *           type: string
 *     responses:
 *       200:
 *         description: Firmware details
 */
router.get('/:id', authenticate, firmwareController.getById.bind(firmwareController));

/**
 * @swagger
 * /firmware/{id}/latest:
 *   patch:
 *     summary: Mark firmware as latest version (admin only)
 *     tags: [Firmware]
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
 *         description: Firmware marked as latest
 */
router.patch(
  '/:id/latest',
  authenticate,
  requireRole('admin'),
  firmwareController.markLatest.bind(firmwareController)
);

/**
 * @swagger
 * /firmware/{id}:
 *   delete:
 *     summary: Delete firmware (admin only)
 *     tags: [Firmware]
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
 *         description: Firmware deleted
 */
router.delete(
  '/:id',
  authenticate,
  requireRole('admin'),
  firmwareController.delete.bind(firmwareController)
);

export default router;
