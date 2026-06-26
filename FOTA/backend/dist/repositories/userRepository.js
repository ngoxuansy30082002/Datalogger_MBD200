"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.userRepository = exports.UserRepository = void 0;
const User_1 = require("../models/User");
class UserRepository {
    async create(data) {
        const user = new User_1.User(data);
        return user.save();
    }
    async findById(id) {
        return User_1.User.findById(id).select('-password');
    }
    async findByUsername(username) {
        return User_1.User.findOne({ username });
    }
    async findAll() {
        return User_1.User.find().select('-password').sort({ createdAt: -1 });
    }
    async updateLastLogin(id) {
        await User_1.User.findByIdAndUpdate(id, { lastLogin: new Date() });
    }
    async delete(id) {
        return User_1.User.findByIdAndDelete(id);
    }
    async count() {
        return User_1.User.countDocuments();
    }
}
exports.UserRepository = UserRepository;
exports.userRepository = new UserRepository();
//# sourceMappingURL=userRepository.js.map