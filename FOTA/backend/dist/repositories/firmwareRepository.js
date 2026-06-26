"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.firmwareRepository = exports.FirmwareRepository = void 0;
const semver_1 = __importDefault(require("semver"));
const Firmware_1 = require("../models/Firmware");
class FirmwareRepository {
    async create(data) {
        const firmware = new Firmware_1.Firmware(data);
        return firmware.save();
    }
    async findById(id) {
        return Firmware_1.Firmware.findById(id);
    }
    async findByHash(hash) {
        return Firmware_1.Firmware.findOne({ fileHash: hash });
    }
    async findLatest(deviceModel, hardwareVersion) {
        return Firmware_1.Firmware.findOne({
            deviceModel,
            hardwareVersion,
            isLatest: true,
        });
    }
    async findByModelAndVersion(deviceModel, hardwareVersion, firmwareVersion) {
        return Firmware_1.Firmware.findOne({ deviceModel, hardwareVersion, firmwareVersion });
    }
    async findAll(filter) {
        const page = filter.page || 1;
        const limit = filter.limit || 20;
        const skip = (page - 1) * limit;
        const sortBy = filter.sortBy || 'uploadTime';
        const sortOrder = filter.sortOrder === 'asc' ? 1 : -1;
        const query = {};
        if (filter.projectName) {
            query.projectName = { $regex: filter.projectName, $options: 'i' };
        }
        if (filter.deviceModel) {
            query.deviceModel = { $regex: filter.deviceModel, $options: 'i' };
        }
        if (filter.hardwareVersion) {
            query.hardwareVersion = filter.hardwareVersion;
        }
        if (filter.isLatest !== undefined) {
            query.isLatest = filter.isLatest;
        }
        if (filter.search) {
            query.$or = [
                { projectName: { $regex: filter.search, $options: 'i' } },
                { deviceModel: { $regex: filter.search, $options: 'i' } },
                { firmwareVersion: { $regex: filter.search, $options: 'i' } },
                { fileName: { $regex: filter.search, $options: 'i' } },
                { description: { $regex: filter.search, $options: 'i' } },
            ];
        }
        const [data, total] = await Promise.all([
            Firmware_1.Firmware.find(query)
                .sort({ [sortBy]: sortOrder })
                .skip(skip)
                .limit(limit),
            Firmware_1.Firmware.countDocuments(query),
        ]);
        return {
            data,
            total,
            page,
            totalPages: Math.ceil(total / limit),
        };
    }
    async update(id, data) {
        return Firmware_1.Firmware.findByIdAndUpdate(id, data, { new: true });
    }
    async delete(id) {
        return Firmware_1.Firmware.findByIdAndDelete(id);
    }
    async unsetLatest(deviceModel, hardwareVersion) {
        await Firmware_1.Firmware.updateMany({ deviceModel, hardwareVersion, isLatest: true }, { isLatest: false });
    }
    async incrementDownloadCount(id) {
        await Firmware_1.Firmware.findByIdAndUpdate(id, { $inc: { downloadCount: 1 } });
    }
    async getStats() {
        const [totalResult, downloadResult] = await Promise.all([
            Firmware_1.Firmware.countDocuments(),
            Firmware_1.Firmware.aggregate([
                { $group: { _id: null, totalDownloads: { $sum: '$downloadCount' } } },
            ]),
        ]);
        return {
            totalFirmware: totalResult,
            totalDownloads: downloadResult[0]?.totalDownloads || 0,
        };
    }
    async getLatestUploads(limit = 5) {
        return Firmware_1.Firmware.find().sort({ uploadTime: -1 }).limit(limit);
    }
    async getNewestForDevice(deviceModel, hardwareVersion) {
        const firmwares = await Firmware_1.Firmware.find({ deviceModel, hardwareVersion })
            .sort({ uploadTime: -1 });
        if (firmwares.length === 0)
            return null;
        // Sort by semver to find the newest version
        const sorted = firmwares.sort((a, b) => {
            const va = semver_1.default.coerce(a.firmwareVersion);
            const vb = semver_1.default.coerce(b.firmwareVersion);
            if (!va || !vb)
                return 0;
            return semver_1.default.rcompare(va, vb);
        });
        return sorted[0];
    }
}
exports.FirmwareRepository = FirmwareRepository;
exports.firmwareRepository = new FirmwareRepository();
//# sourceMappingURL=firmwareRepository.js.map