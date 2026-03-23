# CDD-C & C-ORM: Comprehensive 100+ Step Integration Plan

This document outlines a granular, step-by-step roadmap to implement SQL parsing, C struct code generation (including collection arrays) in `cdd-c`, and the dynamic hydration and database execution engine in `c-orm`.

## Phase 1: `cdd-c` Lexer & Parser (SQL/DDL)
- [x] 1. Define AST node struct for SQL Table definition (`sql_table_t`).
- [x] 2. Define AST node struct for SQL Column definition (`sql_column_t`).
- [x] 3. Define enum for SQL Data Types (INT, VARCHAR, FLOAT, BOOLEAN, etc.).
- [x] 4. Define AST node struct for SQL Constraints (PK, FK, UNIQUE, NOT NULL).
- [x] 5. Implement SQL Lexer: handle standard identifiers and SQL keywords.
- [x] 6. Implement SQL Lexer: tokenize string literals and numeric constants.
- [x] 7. Implement SQL Lexer: tokenize operators and punctuation (parentheses, commas, semicolons).
- [x] 8. Implement SQL Parser skeleton for `CREATE TABLE` statements.
- [x] 9. Implement SQL Parser: parse column names and associate them with types.
- [x] 10. Implement SQL Parser: parse `INTEGER` / `INT` / `BIGINT` types.
- [x] 11. Implement SQL Parser: parse `VARCHAR(n)` / `TEXT` / `CHAR` types.
- [x] 12. Implement SQL Parser: parse `BOOLEAN` / `BOOL` types.
- [x] 13. Implement SQL Parser: parse `FLOAT` / `DOUBLE` / `DECIMAL` types.
- [x] 14. Implement SQL Parser: parse `DATE` / `TIMESTAMP` types.
- [x] 15. Implement SQL Parser: parse `PRIMARY KEY` column constraints.
- [x] 16. Implement SQL Parser: parse `FOREIGN KEY` table constraints and references.
- [x] 17. Implement SQL Parser: parse `NOT NULL` constraints.
- [x] 18. Implement SQL Parser: parse `DEFAULT` values (strings, numbers, booleans, CURRENT_TIMESTAMP).
- [x] 19. Implement SQL Parser: strict error handling with specific line/column reporting.
- [x] 20. Add unit tests for SQL Lexer and Parser components against various DDL dialects.

## Phase 2: `cdd-c` IR to C Struct Emission
- [x] 21. Create an emission module specifically for C header files (`.h`).
- [x] 22. Generate standard includes in headers (`<stdint.h>`, `<stdbool.h>`, `<stddef.h>`).
- [x] 23. Generate robust `#ifndef` / `#define` include guards based on table names.
- [x] 24. Map SQL `INTEGER`/`BIGINT` to C `int32_t` or `int64_t`.
- [x] 25. Map SQL `VARCHAR`/`TEXT` to C `char*`.
- [x] 26. Map SQL `BOOLEAN` to C `bool`.
- [x] 27. Map SQL floating types to C `float` or `double`.
- [x] 28. Define and emit generic `Option_<Type>` structs (or use pointers) for nullable columns.
- [x] 29. Emit the primary C `struct` definition representing a single database row.
- [x] 30. Emit the C `struct` definition for an Array/Collection of rows (e.g., `Type_Array`).
- [x] 31. Add `size_t length` and `size_t capacity` tracking fields to the Array struct.
- [x] 32. Emit memory allocation helper: `Type_Array_init(size_t initial_capacity)`.
- [x] 33. Emit memory management helper: `Type_free(Type* item)` (frees internal string pointers).
- [x] 34. Emit memory management helper: `Type_Array_free(Type_Array* arr)` (iterates and frees elements, then the buffer).
- [x] 35. Emit a deep copy helper function for a single row struct.
- [x] 36. Emit a deep copy helper function for the Array struct.
- [x] 37. Implement name collision handling and sanitize SQL identifiers to valid C identifiers.
- [x] 38. Emit auto-generated docstrings/comments in C code tracing back to the original SQL DDL.
- [x] 39. Add an automatic formatting pass (e.g., invoking `clang-format`) to generated C code.
- [x] 40. Write comprehensive unit tests for struct, array, and memory helper emission logic.

## Phase 3: `cdd-c` ORM Metadata Emission
- [x] 41. Define standard `c_orm_type_t` enum in the `cdd-c` emission context.
- [x] 42. Emit static string arrays of column names (`const char* Type_col_names[]`).
- [x] 43. Emit static arrays of column types mapped to `c_orm_type_t`.
- [x] 44. Generate `offsetof(Type, field)` mappings for every column in the struct.
- [x] 45. Emit boolean flags identifying the primary key column(s) in the metadata array.
- [x] 46. Emit relationship metadata identifying foreign key targets.
- [x] 47. Emit boolean flags identifying nullable vs not-null columns in the metadata.
- [x] 48. Emit the primary `c_orm_table_meta_t` struct instance linking all the above arrays together.
- [x] 49. Generate a global registry table or lookup function to fetch metadata by string name (`get_table_meta("users")`).
- [x] 50. Generate pre-compiled query string constants for basic `SELECT * FROM table`.
- [x] 51. Generate pre-compiled query string constants for `INSERT INTO table ...`.
- [x] 52. Generate pre-compiled query string constants for `UPDATE table SET ... WHERE pk = ?`.
- [x] 53. Generate pre-compiled query string constants for `DELETE FROM table WHERE pk = ?`.
- [x] 54. Emit generic offset and memory size information (`sizeof(Type)`).
- [x] 55. Add unit tests verifying generated metadata memory layout and offset correctness.

## Phase 4: `c-orm` Core Metadata & Interface Setup
- [x] 56. Define the matching `c_orm_column_meta_t` struct in `c-orm` headers.
- [x] 57. Define the matching `c_orm_table_meta_t` struct in `c-orm` headers.
- [x] 58. Define the matching `c_orm_type_t` enum in `c-orm` headers.
- [x] 59. Define generic `c_orm_db_t` opaque pointer struct for database connection state.
- [x] 60. Define an abstract `c_orm_driver_vtable_t` to allow swapping database backends (SQLite, PG).
- [x] 61. Define a `c_orm_error_t` enum for robust error handling.
- [x] 62. Implement `c_orm_get_last_error_message()` utility function.
- [x] 63. Define a `c_orm_query_t` struct for representing and managing active prepared statements.
- [x] 64. Implement query logging callbacks for debugging raw SQL execution.
- [x] 65. Set up CMake build system for `c-orm` as a consumable static or shared library.

## Phase 5: `c-orm` Database Driver Implementations (SQLite Focus)
- [x] 66. Implement SQLite driver initialization and `vtable` wiring.
- [x] 67. Implement `c_orm_connect(url, &db)` for SQLite backend.
- [x] 68. Implement `c_orm_disconnect(db)` and resource cleanup.
- [x] 69. Implement prepared statement compilation (`sqlite3_prepare_v2` wrapper).
- [x] 70. Implement parameter binding for integers (`sqlite3_bind_int` / `sqlite3_bind_int64`).
- [x] 71. Implement parameter binding for strings (`sqlite3_bind_text`).
- [x] 72. Implement parameter binding for floating point numbers (`sqlite3_bind_double`).
- [x] 73. Implement parameter binding for SQL NULLs (`sqlite3_bind_null`).
- [x] 74. Implement statement execution and cursor stepping (`sqlite3_step`).
- [x] 75. Implement column value retrieval functions (handling types safely).
- [x] 76. Implement statement finalization and reset logic (`sqlite3_finalize` / `sqlite3_reset`).

## Phase 6: `c-orm` Query Builder
- [x] 77. Implement a dynamic string builder utility (`c_orm_string_builder_t`) for safe SQL generation.
- [x] 78. Create `c_orm_select_builder_t` and its initialization function.
- [x] 79. Implement `where_eq` and `where_neq` builder filter functions.
- [x] 80. Implement `where_lt`, `where_gt`, `where_lte`, `where_gte` builder filter functions.
- [x] 81. Implement `where_like` and `where_in` builder filter functions.
- [x] 82. Add `order_by(column, ASC/DESC)` builder functionality.
- [x] 83. Add `limit(n)` and `offset(n)` pagination builder functionality.
- [x] 84. Implement a compilation function translating the builder state into a final SQL string + parameter array.
- [x] 85. Create an `insert_builder` for dynamic inserts with bound parameters.
- [x] 86. Create an `update_builder` for dynamic updates (partial field updates).

## Phase 7: `c-orm` Hydration & API Execution
- [x] 87. Implement reflection-based row hydrator: iterate over `c_orm_table_meta_t` and map cursor columns to struct offsets.
- [x] 88. Implement dynamic memory allocation during hydration (e.g., `strdup` for variable string sizes).
- [x] 89. Implement runtime type casting from Driver returned types to the target C struct offset types.
- [x] 90. Implement safe NULL handling during hydration (skipping offset writes or setting optionals).
- [x] 91. Implement Array struct hydration loop (stepping through multiple result set rows).
- [x] 92. Implement dynamic array reallocation logic when result counts exceed the Array's initial capacity.
- [x] 93. Implement `c_orm_find_by_id(db, meta, id_val, out_struct_ptr)`.
- [x] 94. Implement `c_orm_find_all(db, meta, out_array_ptr)`.
- [x] 95. Implement `c_orm_insert(db, meta, in_struct_ptr)`.
- [x] 96. Implement `c_orm_update(db, meta, in_struct_ptr)`.
- [x] 97. Implement `c_orm_delete(db, meta, id_val)`.
- [x] 98. Implement transaction wrappers (`c_orm_begin`, `c_orm_commit`, `c_orm_rollback`).

## Phase 8: E2E Integration, Testing & Documentation
- [x] 99. Setup an E2E testing directory containing both `cdd-c` and `c-orm` projects.
- [x] 100. Create a complex sample schema file (`schema.sql`) for E2E tests containing various constraints and types.
- [x] 101. Integrate `cdd-c` as a custom CMake command to generate models dynamically before `c-orm` compilation.
- [x] 102. Write E2E test: connect to DB, generate schema via raw query, and insert a generated struct.
- [x] 103. Write E2E test: fetch a single row via ID and assert struct data matches insertion data.
- [x] 104. Write E2E test: fetch multiple rows into the Array struct, verifying correct `count` and data fidelity.
- [x] 105. Run Valgrind or AddressSanitizer on all E2E tests to guarantee zero memory leaks during struct allocation/deallocation.
- [x] 106. Document the subset of SQL DDL syntax strictly supported by the `cdd-c` parser.
- [x] 107. Document memory ownership rules (how and when users should call `Type_free` and `Type_Array_free`).
- [x] 108. Write an end-to-end tutorial markdown file demonstrating the workflow from `.sql` to complete C application.
