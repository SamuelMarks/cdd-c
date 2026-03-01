# Publishing Guide

> **Purpose of this file (`PUBLISH.md`)**: To describe the repository's release pipeline, versioning strategy, and the steps required to cut a new release of the `cdd-c` CLI and libraries.

## Versioning
This project strictly follows [Semantic Versioning 2.0.0](https://semver.org/).

## Publishing Process

To publish a new release:

1. **Pre-flight Checks**:
   - Verify `ctest` runs locally with 100% coverage.
   - Run `cdd-c audit .` to ensure internal compliance.
2. **Version Bump**:
   - Update `C_CDD_VERSION` in `CMakeLists.txt`.
   - Ensure the `CHANGELOG.md` reflects the changes (if applicable).
3. **Commit & Tag**:
   - Commit the changes: `git commit -m "chore: release vX.Y.Z"`
   - Create an annotated tag: `git tag -a vX.Y.Z -m "Release vX.Y.Z"`
4. **Push**:
   - Push the master branch and the tag: `git push origin master --tags`

The CI/CD pipeline will automatically detect the tag, execute cross-platform C89 compilation across Linux, macOS, and Windows, and bundle the binaries as GitHub Release assets.

## Supported Artifacts
- Source code tarballs.
- `cdd-c` binary executable for respective operating systems.
- `.a` / `.lib` static libraries.
- `.so` / `.dll` / `.dylib` shared objects.