# CDD Compliance Requirements

The `cdd-c` tool is governed by strict compliance constraints across two primary dimensions:
1. **OpenAPI Compatibility**: Bound to `OAS 3.2.0`.
2. **C Standard Portability**: Bound to `ISO C89 (ANSI C)`.

## OpenAPI 3.2.0
The AST must successfully parse and emit standard and drafted `3.2.0` schema definitions losslessly. Native C structures parse primitives (objects, properties, parameters) while complex polymorphic definitions (`allOf`, `oneOf`, `if`/`then`/`else`) are maintained via native pointer arrays and deep-copy replication in the AST.

## C89 (ANSI C) Strict Mode
Because CDD targets maximal embedded and legacy environment compatibility:
* No `//` inline comments.
* Variables must be declared at the top of a block scope (`{`).
* Variadic macros are discouraged without `#ifdef` guards.
* Memory allocations (`malloc`, `calloc`) must rigorously verify boundaries without depending on C99+ safety macros.
* Code must compile cleanly using `-Wextra -Wall -pedantic`.

The AST and code generation templates enforce this uniformly for both the compiler itself and the SDK output.