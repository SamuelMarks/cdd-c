# Developing `cdd-c`

Welcome to `cdd-c`! 

## Architecture
We use a 3-stage compiler approach (Frontend/AST -> IR -> Backend/Emitter).

## Core Rules
1. **Never write non-C89 code.**
2. Test driven. Add a test in `src/tests` to reproduce a bug *before* fixing it in the main library.
3. Keep AST components (`src/openapi/parse/openapi.h`) perfectly symmetrical. 

## Testing
```bash
make test
```
*(Runs standard cmake & ctest).*

To debug memory leaks:
```bash
cmake -S . -B build_asan -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DC_CDD_BUILD_TESTING=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_C_FLAGS="-fsanitize=address -g -O1 -fPIC" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
cmake --build build_asan -j
cd build_asan && ctest -V
```

## Adding new OpenAPI keywords
Update `struct OpenAPI_SchemaRef`, update `parse_schema_ref`, update `copy_schema_ref`, update `free_schema_ref_content`, update `write_schema_ref`. Ensure `json_value_deep_copy` handles pointer allocations.