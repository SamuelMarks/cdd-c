# Developing

> **Purpose of this file (`DEVELOPING.md`)**: To provide contributor guidelines, build instructions, and testing methodologies for developers working on the `cdd-c` library and CLI.

## Requirements
- CMake (3.10 or newer)
- A compliant C89 compiler (MSVC, GCC, or Clang)
- `vcpkg` (for dependency management like `parson` and `curl`)

## Building the Project

Use standard CMake commands to build the CLI and libraries:

```bash
mkdir build
cd build
cmake ..
cmake --build . -j$(nproc)
```

## Running the Test Suite

The test suite validates the C parser, the OpenAPI 3.2.0 dialect emitter, and the bidirectional synchronization engine.

```bash
cd build
ctest -V
```

### Coverage Reports
To generate a coverage report, configure CMake with coverage enabled:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
make -j$(nproc) coverage
```

## Contributor Guidelines

1. **Strict C89**: All modifications must compile as strict C89. Do not use inline variable declarations, VLAs, or non-standard GNU/MSVC extensions.
2. **Modular Placement**: When adding logic, place it strictly inside `src/<domain>/parse/` or `src/<domain>/emit/`. If the logic crosses boundaries, it belongs in the orchestrator pipeline.
3. **100% Test Coverage**: A PR will not be accepted if it drops coverage below 100%. Add explicit unit tests for all new code branches.
4. **Bidirectional Safety**: Always verify that emitting C code from an OpenAPI spec and re-parsing that C code yields the exact same logical OpenAPI spec.