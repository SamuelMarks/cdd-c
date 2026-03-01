# Developing `cdd-c`

Welcome to `cdd-c` development! To get started:

## Prerequisites

- `gcc` or `clang`
- `cmake` >= 3.10
- `flex`
- `bison`

### Setup

```bash
make install_base
make build
```

## Directory Structure

We use a modular architecture organized by semantic responsibilities:

- `src/classes/{emit,parse}` - AST classes processing
- `src/docstrings/{emit,parse}` - Comments and documentation analysis
- `src/functions/{emit,parse}` - Function definitions
- `src/mocks/{emit,parse}` - Mocks for test stubs
- `src/openapi/{emit,parse}` - Main OpenAPI specification manipulation
- `src/routes/{emit,parse}` - API server endpoints mapping
- `src/tests/{emit,parse}` - Testing suites generation
- `c/main.c` - CLI entry point

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
