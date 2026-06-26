"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = require("express");
const dashboardController_1 = require("../controllers/dashboardController");
const auth_1 = require("../middleware/auth");
const router = (0, express_1.Router)();
router.get('/summary', auth_1.authenticate, dashboardController_1.getSummary);
router.get('/weekly-activity', auth_1.authenticate, dashboardController_1.getWeeklyActivity);
router.get('/devices', auth_1.authenticate, dashboardController_1.getDevices);
router.get('/recent-firmware', auth_1.authenticate, dashboardController_1.getRecentFirmware);
router.get('/mqtt-logs', auth_1.authenticate, dashboardController_1.getMqttLogs);
exports.default = router;
//# sourceMappingURL=dashboardRoutes.js.map