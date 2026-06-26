"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.deviceRepository = exports.DeviceRepository = void 0;
const Device_1 = require("../models/Device");
class DeviceRepository {
    async create(data) {
        const device = new Device_1.Device(data);
        return device.save();
    }
    async findById(id) {
        return Device_1.Device.findById(id);
    }
    async findByDeviceId(deviceId) {
        return Device_1.Device.findOne({ deviceId });
    }
    async findAll(page = 1, limit = 20) {
        const skip = (page - 1) * limit;
        const [data, total] = await Promise.all([
            Device_1.Device.find().sort({ lastSeen: -1 }).skip(skip).limit(limit),
            Device_1.Device.countDocuments(),
        ]);
        return {
            data,
            total,
            page,
            totalPages: Math.ceil(total / limit),
        };
    }
    async upsert(deviceId, data) {
        const device = await Device_1.Device.findOneAndUpdate({ deviceId }, { ...data, lastSeen: new Date() }, { new: true, upsert: true, setDefaultsOnInsert: true });
        return device;
    }
    async updateOnlineStatus(deviceId, online) {
        await Device_1.Device.findOneAndUpdate({ deviceId }, { online, lastSeen: new Date() });
    }
    async updateLastSeen(deviceId) {
        await Device_1.Device.findOneAndUpdate({ deviceId }, { lastSeen: new Date(), online: true });
    }
    async getOnlineCount() {
        // Consider devices seen in the last 5 minutes as online
        const fiveMinutesAgo = new Date(Date.now() - 5 * 60 * 1000);
        return Device_1.Device.countDocuments({
            lastSeen: { $gte: fiveMinutesAgo },
            online: true,
        });
    }
    async delete(id) {
        return Device_1.Device.findByIdAndDelete(id);
    }
}
exports.DeviceRepository = DeviceRepository;
exports.deviceRepository = new DeviceRepository();
//# sourceMappingURL=deviceRepository.js.map