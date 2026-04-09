# Safe CRT Refactoring

This directory previously contained automated Python scripts used to refactor a C codebase to utilize MSVC Safe CRT (`_s`) functions.

These regex-based scripts were brittle and have now been replaced by a full C-based Abstract Syntax Tree (AST) parser built natively into `cdd-c`.

## New Native Tool

You can now automatically and robustly migrate any C file to Safe CRT functions (`fopen_s`, `strcpy_s`, `strncpy_s`, `sprintf_s`, `strcat_s`, `strncat_s`, `memcpy_s`, `memmove_s`) wrapped in `#if defined(_MSC_VER)` macros, whilst perfectly preserving original formatting and trivia.

To use the new tool:

```bash
cdd-c transformer safe_crt --fix src/my_file.c
```

For batch processing the whole codebase:
```bash
fd -t f -e c -e h -x cdd-c transformer safe_crt --fix {}
```
