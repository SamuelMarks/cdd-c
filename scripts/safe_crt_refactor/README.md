# Safe CRT Refactoring Scripts

This directory contains automated scripts used to refactor a C codebase to utilize MSVC Safe CRT (`_s`) functions where available, while gracefully falling back to standard C functions on other platforms (like Linux, WASM, etc.) via preprocessor macros.

## Scripts Included

- **`fix_fopen.py`**: Replaces standard `fopen()` calls with MSVC's `fopen_s()`. It targets assignments of the form `var = fopen(path, mode);` and replaces them with an `#if defined(_MSC_VER)` guarded block.
- **`fix_strcpy.py`**: Replaces `strcpy(dest, src)` with `strcpy_s(dest, sizeof(dest), src)`. It assumes the destination is an array whose size can be determined by `sizeof()`.
- **`fix_strncpy.py`**: Replaces `strncpy(dest, src, size)` with `strncpy_s(dest, size + 1, src, size)`. It adds 1 to the destination size to account for null termination, which `strncpy_s` requires.
- **`fix_wincrypt.py`**: A specialized script used to correct the inclusion order of `<windows.h>` and `<wincrypt.h>` in specific files to avoid definition conflicts on Windows.

## Usage

These scripts are designed to be run from the root of the project repository. They iterate over `.c` files in the `src/` directory, excluding files in `tests/` or `mocks/` directories.

```bash
# Example usage from project root:
python3 scripts/safe_crt_refactor/fix_fopen.py
python3 scripts/safe_crt_refactor/fix_strcpy.py
python3 scripts/safe_crt_refactor/fix_strncpy.py
```

## Note on Preprocessor Guards

The scripts generate code similar to the following to ensure cross-platform compatibility:

```c
#if defined(_MSC_VER)
  fopen_s(&f, path, "w");
#else
  f = fopen(path, "w");
#endif
```

## Future Work

If this suite is expanded into a standalone project, it could benefit from:
- A unified orchestrator script to run all fixes.
- More robust AST parsing (e.g., via `tree-sitter` like the `refactor_returns.py` script) to handle edge cases and complex formatting better than regex.
- Configurable include/exclude paths.
