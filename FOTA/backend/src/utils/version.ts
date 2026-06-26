import semver from 'semver';

/**
 * Compare two semver version strings
 * Returns true if newVersion > currentVersion
 */
export const isNewerVersion = (currentVersion: string, newVersion: string): boolean => {
  const current = semver.valid(semver.coerce(currentVersion));
  const latest = semver.valid(semver.coerce(newVersion));

  if (!current || !latest) return false;
  return semver.gt(latest, current);
};

/**
 * Validate a version string as valid semver
 */
export const isValidVersion = (version: string): boolean => {
  return semver.valid(semver.coerce(version)) !== null;
};

/**
 * Sort version strings in descending order
 */
export const sortVersionsDesc = (versions: string[]): string[] => {
  return versions
    .filter((v) => semver.valid(semver.coerce(v)))
    .sort((a, b) => {
      const va = semver.coerce(a)!;
      const vb = semver.coerce(b)!;
      return semver.rcompare(va, vb);
    });
};
