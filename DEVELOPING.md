# Developing `cdd-c`

Welcome to `cdd-c` development! To get started:

## Prerequisites

- `gcc` or `clang`
- `cmake` >= 3.10

### Setup

```bash
make install_base
make build
```

## Error Handling & API Guidelines

When developing features for `cdd-c`, you **MUST** strictly adhere to the project's error handling patterns. This applies universally across the codebase.

1. **Enums for Return Values**: Every non-void, non-math function **MUST** return an error enum (specifically `cdd_c_error_t`).
2. **Output Pointers**: Output values must be passed via pointer arguments rather than return values.
3. **Assertive Percolation**: Errors must be explicitly checked and percolated up the call stack assertively. Do not swallow errors.

**Correct Example**:
```c
cdd_c_error_t rc;
rc = my_function();
if (rc != CDD_C_SUCCESS) {
    /* handle error, printing the nonzero exit code for debug purposes */
    fprintf(stderr, "Error occurred: %d\n", rc);
    return rc;
}
rc = another_function(&my_output);
if (rc != CDD_C_SUCCESS) {
    return rc;
}
```

## Directory Structure

We use a modular architecture organized by semantic responsibilities:

- `src/arazzo/{emit,parse}` - Arazzo 1.1.0 workflow parsing, evaluation, and emission
- `src/classes/{emit,parse}` - AST/CST classes, structs, enums, and JSON Schema processing
- `src/docstrings/parse` - Comments and Doxygen documentation analysis (`@route`, `@workflow`, `@step`, etc.)
- `src/functions/{dsl,emit,ffi,parse}` - Function definitions, CST/AST lifting, FFI binding generation (42 languages), and in-place sync
- `src/mocks/{emit,parse}` - Mocks for test stubs
- `src/openapi/{emit,parse}` - Main OpenAPI 3.2.0 / Swagger 2.0 specification manipulation
- `src/routes/{emit,parse}` - API server endpoints, client SDKs, CLI, JSON-RPC, and MCP mapping
- `src/tests/{emit,parse,transformers}` - Testing suites and test generators
- `src/transformers/` - CST/AST refactoring passes (`gnu_standardizer`, `error_percolator`, `safe_crt`, `msvc_port`, `extern_c`, `macros`)
- `src/bin_cdd.c` & `src/functions/parse/main.c` - CLI entry point (`cdd-c`)
- `src/cdd_api.c` & `src/cdd_api.h` - Public programmatic C SDK

When working on a specific feature, like emitting structs, look into `src/classes/emit/struct.c`.

## Testing

```bash
make test
```

We aim for 100% test coverage. Update the tests when you add or change features! Use `ctest` with Coverage configurations (if configured) to ensure complete test suites.

## WebAssembly

`make build_wasm` automatically downloads `wasi-sdk` (if not already present in `./wasi-sdk`) and builds `bin/cdd-c.wasm`:

```bash
make build_wasm
```
