# AST-Based C Return Type Refactoring Tool

This directory previously contained a standalone Python `tree-sitter` powered script for refactoring C `void` or `bool` return types into `int` based error percolation chains (`rc = ...; if (rc) return rc;`).

This script has been superseded by a native, robust C parser (Abstract/Concrete Syntax Tree Weaver) inside `cdd-c` that preserves original macro and trivia states seamlessly.

## New Native Tool

To use the new error percolator tool native to `cdd-c` on C files:

```bash
cdd-c transformer error_percolator --fix src/my_file.c
```
