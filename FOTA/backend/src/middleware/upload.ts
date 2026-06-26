import multer from 'multer';
import { env } from '../config/env';

// Use memory storage so we can compute hash before writing to disk
const storage = multer.memoryStorage();

const fileFilter = (_req: Express.Request, file: Express.Multer.File, cb: multer.FileFilterCallback) => {
  if (file.originalname.toLowerCase().endsWith('.hex')) {
    cb(null, true);
  } else {
    cb(new Error('Only .hex files are allowed'));
  }
};

export const upload = multer({
  storage,
  fileFilter,
  limits: {
    fileSize: env.MAX_FILE_SIZE,
    files: 10, // max 10 files simultaneously
  },
});
