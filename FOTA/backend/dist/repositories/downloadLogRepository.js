"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.downloadLogRepository = exports.DownloadLogRepository = void 0;
const DownloadLog_1 = require("../models/DownloadLog");
class DownloadLogRepository {
    async create(data) {
        const log = new DownloadLog_1.DownloadLog(data);
        return log.save();
    }
    async findByFirmwareId(firmwareId) {
        return DownloadLog_1.DownloadLog.find({ firmwareId }).sort({ timestamp: -1 });
    }
    async findRecent(limit = 50) {
        return DownloadLog_1.DownloadLog.find()
            .populate('firmwareId', 'fileName firmwareVersion deviceModel')
            .sort({ timestamp: -1 })
            .limit(limit);
    }
}
exports.DownloadLogRepository = DownloadLogRepository;
exports.downloadLogRepository = new DownloadLogRepository();
//# sourceMappingURL=downloadLogRepository.js.map