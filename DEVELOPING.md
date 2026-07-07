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

1. **Enums for Return Values**: Every non-void, non-math function **MUST** return an error enum (specifically `enum cdd_c_error`).
2. **Output Pointers**: Output values must be passed via pointer arguments rather than return values.
3. **Assertive Percolation**: Errors must be explicitly checked and percolated up the call stack assertively. Do not swallow errors.

**Correct Example**:
```c
enum cdd_c_error rc;
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
