# Developing `cdd-c`

Welcome to `cdd-c` development! To get started:

## Prerequisites

- `gcc`, `clang`, or `msvc`
- `cmake` >= 3.11
- Existing HTTP/REST libraries: `c-rest-framework` (local dependency)
- Database libraries: `libpq`, `sqlite3`

### Setup

```bash
cmake -B build_cmake -S .
cmake --build build_cmake
```

## Directory Structure

We use a modular architecture organized by semantic responsibilities:

- `src/classes/{emit,parse}` - AST classes processing
- `src/docstrings/{emit,parse}` - Comments and documentation analysis
- `src/functions/{emit,parse}` - Function definitions (including CLI dispatch)
- `src/mocks/{emit,parse}` - Mocks for test stubs
- `src/openapi/{emit,parse}` - Main OpenAPI specification manipulation
- `src/routes/{emit,parse}` - API server endpoints mapping (generates `c-rest-framework` and `c-orm` bindings)
- `src/tests/{emit,parse}` - Testing suites generation
- `src/bin_cdd.c` - CLI entry point

When working on a specific feature, like emitting structs, look into `src/classes/emit/struct.c`.

## Testing

```bash
make test
```

We aim for 100% test coverage. Update the tests when you add or change features! Use `ctest` with Coverage configurations (if configured) to ensure complete test suites.

## WebAssembly

For WASM testing, you need `emsdk` installed in `../emsdk`.

```bash
make build_wasm
```
