"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = require("express");
const firmwareRoutes_1 = __importDefault(require("./firmwareRoutes"));
const authRoutes_1 = __importDefault(require("./authRoutes"));
const dashboardRoutes_1 = __importDefault(require("./dashboardRoutes"));
const mqttRoutes_1 = __importDefault(require("./mqttRoutes"));
const deviceRoutes_1 = __importDefault(require("./deviceRoutes"));
const router = (0, express_1.Router)();
router.use('/firmware', firmwareRoutes_1.default);
router.use('/auth', authRoutes_1.default);
router.use('/dashboard', dashboardRoutes_1.default);
router.use('/mqtt', mqttRoutes_1.default);
router.use('/devices', deviceRoutes_1.default);
exports.default = router;
//# sourceMappingURL=index.js.map