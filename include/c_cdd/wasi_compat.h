/**
 * @file wasi_compat.h
 * @brief WebAssembly System Interface (WASI) compatibility shims.
 *
 * Provides fallback definitions for standard C library functions (such as
 * `tmpfile`) that are not implemented in `wasi-libc`.
 */

#ifndef C_CDD_WASI_COMPAT_H
#define C_CDD_WASI_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stdio.h>
/* clang-format on */

#if defined(__wasi__)
#ifndef tmpfile
/**
 * @brief WASI fallback for `tmpfile()`, returning `NULL` since `wasi-libc`
 * does not provide an ambient temporary directory.
 */
#define tmpfile() ((FILE *)0)
#endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_WASI_COMPAT_H */
