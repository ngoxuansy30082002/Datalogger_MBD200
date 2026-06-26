import Joi from 'joi';
import { Request, Response, NextFunction } from 'express';
declare const schemas: {
    login: Joi.ObjectSchema<any>;
    register: Joi.ObjectSchema<any>;
    firmwareUpload: Joi.ObjectSchema<any>;
    firmwareQuery: Joi.ObjectSchema<any>;
    latestQuery: Joi.ObjectSchema<any>;
    refreshToken: Joi.ObjectSchema<any>;
};
export declare const validate: (schemaName: keyof typeof schemas, source?: "body" | "query" | "params") => (req: Request, res: Response, next: NextFunction) => void;
export {};
//# sourceMappingURL=validation.d.ts.map