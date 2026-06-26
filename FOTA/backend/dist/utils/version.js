"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.sortVersionsDesc = exports.isValidVersion = exports.isNewerVersion = void 0;
const semver_1 = __importDefault(require("semver"));
/**
 * Compare two semver version strings
 * Returns true if newVersion > currentVersion
 */
const isNewerVersion = (currentVersion, newVersion) => {
    const current = semver_1.default.valid(semver_1.default.coerce(currentVersion));
    const latest = semver_1.default.valid(semver_1.default.coerce(newVersion));
    if (!current || !latest)
        return false;
    return semver_1.default.gt(latest, current);
};
exports.isNewerVersion = isNewerVersion;
/**
 * Validate a version string as valid semver
 */
const isValidVersion = (version) => {
    return semver_1.default.valid(semver_1.default.coerce(version)) !== null;
};
exports.isValidVersion = isValidVersion;
/**
 * Sort version strings in descending order
 */
const sortVersionsDesc = (versions) => {
    return versions
        .filter((v) => semver_1.default.valid(semver_1.default.coerce(v)))
        .sort((a, b) => {
        const va = semver_1.default.coerce(a);
        const vb = semver_1.default.coerce(b);
        return semver_1.default.rcompare(va, vb);
    });
};
exports.sortVersionsDesc = sortVersionsDesc;
//# sourceMappingURL=version.js.map