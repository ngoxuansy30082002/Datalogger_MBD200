import Joi from 'joi';
import { Request, Response, NextFunction } from 'express';

const schemas = {
  login: Joi.object({
    username: Joi.string().min(3).max(50).required(),
    password: Joi.string().min(6).required(),
  }),

  register: Joi.object({
    username: Joi.string().min(3).max(50).required(),
    password: Joi.string().min(6).required(),
    email: Joi.string().email().allow('').optional(),
    role: Joi.string().valid('admin', 'user').default('user'),
  }),

  firmwareUpload: Joi.object({
    projectName: Joi.string().required().trim(),
    deviceModel: Joi.string().required().trim(),
    hardwareVersion: Joi.string().required().trim(),
    firmwareVersion: Joi.string().required().trim(),
    description: Joi.string().allow('').default(''),
  }),

  firmwareQuery: Joi.object({
    deviceModel: Joi.string().required(),
    hw: Joi.string().required(),
    version: Joi.string().default('latest'),
  }),

  latestQuery: Joi.object({
    deviceModel: Joi.string().required(),
    hardwareVersion: Joi.string().required(),
    currentVersion: Joi.string().required(),
  }),

  refreshToken: Joi.object({
    refreshToken: Joi.string().required(),
  }),
};

export const validate = (schemaName: keyof typeof schemas, source: 'body' | 'query' | 'params' = 'body') => {
  return (req: Request, res: Response, next: NextFunction): void => {
    const schema = schemas[schemaName];
    if (!schema) {
      next();
      return;
    }

    const { error, value } = schema.validate(req[source], { abortEarly: false, stripUnknown: true });

    if (error) {
      const details = error.details.map((d) => d.message).join(', ');
      res.status(400).json({
        success: false,
        message: 'Validation error',
        details,
      });
      return;
    }

    req[source] = value;
    next();
  };
};
