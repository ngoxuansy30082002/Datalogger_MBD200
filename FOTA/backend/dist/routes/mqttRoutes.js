"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = require("express");
const mqttController_1 = require("../controllers/mqttController");
const auth_1 = require("../middleware/auth");
const roleGuard_1 = require("../middleware/roleGuard");
const router = (0, express_1.Router)();
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
router.post('/notify', auth_1.authenticate, (0, roleGuard_1.requireRole)('admin'), mqttController_1.mqttController.publishNotify.bind(mqttController_1.mqttController));
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
router.get('/status', auth_1.authenticate, mqttController_1.mqttController.getStatus.bind(mqttController_1.mqttController));
exports.default = router;
//# sourceMappingURL=mqttRoutes.js.map