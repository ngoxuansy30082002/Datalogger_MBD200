"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = require("express");
const deviceController_1 = require("../controllers/deviceController");
const auth_1 = require("../middleware/auth");
const router = (0, express_1.Router)();
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
router.get('/', auth_1.authenticate, deviceController_1.deviceController.list.bind(deviceController_1.deviceController));
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
router.get('/:id', auth_1.authenticate, deviceController_1.deviceController.getById.bind(deviceController_1.deviceController));
exports.default = router;
//# sourceMappingURL=deviceRoutes.js.map