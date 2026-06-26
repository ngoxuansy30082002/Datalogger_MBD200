/**
 * Compare two semver version strings
 * Returns true if newVersion > currentVersion
 */
export declare const isNewerVersion: (currentVersion: string, newVersion: string) => boolean;
/**
 * Validate a version string as valid semver
 */
export declare const isValidVersion: (version: string) => boolean;
/**
 * Sort version strings in descending order
 */
export declare const sortVersionsDesc: (versions: string[]) => string[];
//# sourceMappingURL=version.d.ts.map