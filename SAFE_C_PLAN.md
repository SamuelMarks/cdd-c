# Comprehensive Safe CRT Transformation Plan (SAFE_C_PLAN.md)

## 1. Exhaustive Safe CRT API Coverage
The current implementation hardcodes only 8 functions. The complete MSVC Safe CRT encompasses over 100 functions across multiple domains.

### 1.1 Formatted I/O Family
- [x] `snprintf` -> `snprintf_s`
- [x] `vsnprintf` -> `vsnprintf_s`
- [x] `sprintf` -> `sprintf_s`
- [x] `vsprintf` -> `vsprintf_s`
- [x] `printf` -> `printf_s`
- [x] `vprintf` -> `vprintf_s`
- [x] `fprintf` -> `fprintf_s`
- [x] `vfprintf` -> `vfprintf_s`
- [x] `_snprintf` -> `_snprintf_s`
- [x] `_vsnprintf` -> `_vsnprintf_s`

### 1.2 `scanf` Family (Requires Format String Analysis)
- [x] `scanf` -> `scanf_s`
- [x] `fscanf` -> `fscanf_s`
- [x] `sscanf` -> `sscanf_s`
- [x] `vscanf` -> `vscanf_s`
- [x] `vfscanf` -> `vfscanf_s`
- [x] `vsscanf` -> `vsscanf_s`
- [x] **Critical Sub-task:** The parser MUST understand the format string. For every `%s`, `%S`, `%c`, `%C`, and `%[` parsed, Safe CRT `scanf` functions require an additional `unsigned` size parameter passed *immediately after* the buffer pointer.

### 1.3 Standard String & Character Manipulation
- [ ] `strtok` -> `strtok_s` (Requires hoisting a `char *` context pointer variable).
- [x] `gets` -> `gets_s`
- [x] `strlen` -> `strnlen_s`
- [ ] `strerror` -> `strerror_s`
- [ ] `_strerror` -> `_strerror_s`
- [ ] `_stricmp` -> `_stricmp_s`
- [ ] `_strnicmp` -> `_strnicmp_s`
- [x] `_strlwr` -> `_strlwr_s`
- [x] `_strupr` -> `_strupr_s`
- [x] `_strnset` -> `_strnset_s`
- [x] `_strset` -> `_strset_s`

### 1.4 Multibyte Character Set (MBCS) Equivalents
- [x] `_mbscpy` -> `_mbscpy_s`
- [x] `_mbsncpy` -> `_mbsncpy_s`
- [x] `_mbscat` -> `_mbscat_s`
- [x] `_mbsncat` -> `_mbsncat_s`
- [ ] `_mbstok` -> `_mbstok_s`
- [x] `_mbslwr` -> `_mbslwr_s`
- [x] `_mbsupr` -> `_mbsupr_s`
- [x] `_mbsset` -> `_mbsset_s`
- [x] `_mbsnset` -> `_mbsnset_s`

### 1.5 Wide Character Equivalents
- [x] `wcscpy` -> `wcscpy_s`
- [x] `wcsncpy` -> `wcsncpy_s`
- [x] `wcscat` -> `wcscat_s`
- [x] `wcsncat` -> `wcsncat_s`
- [x] `swprintf` -> `swprintf_s`
- [x] `vswprintf` -> `vswprintf_s`
- [x] `wmemcpy` -> `wmemcpy_s`
- [x] `wmemmove` -> `wmemmove_s`
- [ ] `wcstok` -> `wcstok_s`
- [x] `_wcslwr` -> `_wcslwr_s`
- [x] `_wcsupr` -> `_wcsupr_s`
- [ ] `_wcserror` -> `_wcserror_s`

### 1.6 String & Type Conversion
- [ ] `mbstowcs` -> `mbstowcs_s`
- [ ] `wcstombs` -> `wcstombs_s`
- [ ] `wctomb` -> `wctomb_s`
- [x] `_itoa` -> `_itoa_s`
- [x] `_ltoa` -> `_ltoa_s`
- [x] `_ultoa` -> `_ultoa_s`
- [x] `_i64toa` -> `_i64toa_s`
- [x] `_ui64toa` -> `_ui64toa_s`
- [x] `_itow` -> `_itow_s`
- [x] `_ltow` -> `_ltow_s`
- [x] `_ultow` -> `_ultow_s`
- [ ] `_gcvt` -> `_gcvt_s`
- [ ] `_ecvt` -> `_ecvt_s`
- [ ] `_fcvt` -> `_fcvt_s`

### 1.7 File System, I/O, and Environment
- [x] `freopen` -> `freopen_s`
- [x] `_wfopen` -> `_wfopen_s`
- [x] `tmpfile` -> `tmpfile_s`
- [x] `tmpnam` -> `tmpnam_s`
- [x] `_splitpath` -> `_splitpath_s`
- [x] `_makepath` -> `_makepath_s`
- [x] `_wsplitpath` -> `_wsplitpath_s`
- [x] `_wmakepath` -> `_wmakepath_s`
- [ ] `getenv` -> `getenv_s` (or `_dupenv_s`)
- [ ] `_wgetenv` -> `_wgetenv_s`
- [ ] `_putenv` -> `_putenv_s`
- [ ] `_wputenv` -> `_wputenv_s`
- [ ] `_searchenv` -> `_searchenv_s`

### 1.8 Algorithms & Callbacks
- [ ] `qsort` -> `qsort_s`
- [ ] `bsearch` -> `bsearch_s`
- [ ] **Critical Sub-task:** AST must rewrite the comparison callback function signatures internally to accept the `void *context` pointer passed by Safe CRT sorting functions, then update all invocations.

## 2. Advanced Semantic Threats & Memory Vulnerabilities
The current logic relies on naive string replacements that produce dangerously incorrect C code.

- [ ] **Pointer Arithmetic Masking (`sizeof` bug):** If `dest` is `ptr + 5` or `&arr[2]`, `sizeof(ptr + 5)` returns the size of the pointer type (4 or 8 bytes), which leads to immediate buffer truncation or overflow. The parser must isolate the underlying buffer declaration to calculate remaining bounds correctly.
- [ ] **Heap vs Stack Buffer Resolution:** If the buffer is dynamically allocated (e.g., `char *buf = malloc(N)`), `sizeof(buf)` fails. The AST must trace the pointer definition and infer capacity from allocation sites, or fallback to an interactive prompt / explicit wrapper logic.
- [ ] **Variable Length Arrays (VLAs):** C99 VLAs (`char buf[len];`) evaluate `sizeof(buf)` dynamically at runtime, but relying on MSVC compatibility flags introduces cross-platform fragility. Ensure safe translation.
- [x] **Capacity vs Copy Length Separation (`memcpy_s`):** `memcpy_s(dest, size, src, size)` is fundamentally insecure. `destsz` MUST be the full capacity of `dest`, not the number of bytes being copied. Blindly passing the exact same `size` parameter defeats the purpose of the security check.
- [ ] **The `strncpy` Null-Termination Trap:** Standard `strncpy` *does not* guarantee null termination. `strncpy_s` *does*. Blindly refactoring `strncpy` to `strncpy_s` will alter the behavioral semantics of the program if downstream code expects a padded, non-null-terminated buffer. 
- [x] **Overflow from `size + 1`:** The tool generates `strncpy_s(dest, size + 1, src, size)`. If the original `size` represented the strict maximum bound of the array, `size + 1` causes memory corruption. The transformer must intelligently utilize `_TRUNCATE`.

## 3. Contextual Syntax & Preprocessor Clashes
The `#if defined(_MSC_VER)` textual macro injection breaks C compilation across numerous grammatical scopes.

- [ ] **Ternary Operators:** `condition ? strcpy(a, b) : strcpy(a, c);`. You cannot inject preprocessor blocks inside a ternary expression. The AST must hoist the logic into an `if-else` block *before* transforming.
- [ ] **Comma Operators:** `(do_prep(), strcpy(a, b))`. Injecting `#if` inside parentheses violates C syntax. The comma operator must be unrolled into a block statement.
- [ ] **Macro Expansions:** `MY_COPY_MACRO(a, b)`. If `strcpy` is embedded inside a macro definition, modifying the AST output directly might break other usages of the macro.
- [ ] **Unbraced Control Flow:** `if (cond) strcpy(a, b);`. The current replacement injects `#if` directives directly into an unbraced `if` statement body. The compiler will misinterpret the block scope. The transformer MUST wrap single-statement bodies in `{ ... }` before modifying.
- [ ] **Declaration Splitting:** `FILE *f = fopen("x", "r");`. Generates invalid syntax `fopen_s(&(FILE *f), ...);`. The transformer must split this into:
  ```c
  FILE *f;
  #if defined(_MSC_VER)
  fopen_s(&f, "x", "r");
  #else
  f = fopen("x", "r");
  #endif
  ```
- [ ] **Return Value Usage:** `if (strcpy(a, b) != NULL)`. The tool completely ignores these scenarios because `assign_idx == -1` is explicitly required. Safe CRT functions return `errno_t` codes, not pointers. The parser must transform these into block operations, capturing the state of `a` after the mutation.

## 4. Parser Architecture & Engine Flaws
The transformer implementation itself is fundamentally unsuited for complex ASTs.

- [x] **Flat Token Loop vs. Hierarchical Traversal:** The `for (j = 0; j < stmt->num_children; j++)` loop operates entirely flatly, relying entirely on `paren_depth`. This completely breaks on nested function calls: `strcpy(a, strcpy(b, c))`. The `paren_depth` will fail to associate commas with the correct function call boundaries. The AST engine must evaluate expressions recursively, strictly maintaining node parent-child relationships.
- [x] **Hardcoded Buffer Limits:** `dest_str[1024]` and `var_str[1024]`. In C, macro-expanded parameters or deep inline struct definitions can easily exceed 1024 bytes. The engine must use dynamic string buffers or stream writes directly.
- [x] **Token Truncation Bugs:** `extract_token_range_str` caps strings at `max_len`, meaning it silently emits broken, truncated C syntax instead of aborting the operation when limits are hit.
- [x] **Trivia Erasure (Comments/Whitespace):** `get_indent_string` only captures immediate leading whitespace. Inline comments (`strcpy(a, /* dest */ b)`), multi-line parameter formatting, and trailing spaces are destroyed during string extraction. Token extraction must preserve all inner trivia recursively.
- [x] **Memory Leaking (`buf`):** The transient buffer `buf` is intentionally leaked in `replace_safe_crt` to ensure tokens remain valid. On large repositories, this guarantees catastrophic OOM crashes. Tokens must deep-copy string content or use an arena allocator with proper lifecycle management.
- [x] **Missing Error Propagation:** The logic returns `0` silently when it encounters unsupported cases (e.g., bare `fopen`), meaning users cannot easily generate a report of files/lines that *failed* the automatic Safe CRT refactor. Provide structured logging/warnings.

## 5. Automated Testing & Verification
The current tests (`test_cdd_transform_safe_crt`) are insufficient for the depth of these transformations.

- [x] Add explicit AST tests for **nested function evaluation** (`strcpy` within a `sprintf`).
- [x] Add AST tests validating the **preservation of inline comments**.
- [ ] Add AST tests proving **single-line unbraced loops** correctly resolve into braced blocks.
- [ ] Add AST tests to ensure **`sizeof` is only applied** to verified Array types, not Pointers.
- [ ] Add compilation integration tests verifying the `#if defined(_MSC_VER)` outputs actually compile across both MSVC and GCC/Clang targets.