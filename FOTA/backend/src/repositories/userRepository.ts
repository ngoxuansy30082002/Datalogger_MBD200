import { User, IUserDocument } from '../models/User';

export class UserRepository {
  async create(data: Partial<IUserDocument>): Promise<IUserDocument> {
    const user = new User(data);
    return user.save();
  }

  async findById(id: string): Promise<IUserDocument | null> {
    return User.findById(id).select('-password');
  }

  async findByUsername(username: string): Promise<IUserDocument | null> {
    return User.findOne({ username });
  }

  async findAll(): Promise<IUserDocument[]> {
    return User.find().select('-password').sort({ createdAt: -1 });
  }

  async updateLastLogin(id: string): Promise<void> {
    await User.findByIdAndUpdate(id, { lastLogin: new Date() });
  }

  async delete(id: string): Promise<IUserDocument | null> {
    return User.findByIdAndDelete(id);
  }

  async count(): Promise<number> {
    return User.countDocuments();
  }
}

export const userRepository = new UserRepository();
