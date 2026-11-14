/**
 * @file cdd_int128.h
 * @brief Standalone header-only 128-bit integer runtime companion for
 * `cdd-c`'s `__int128` CST lowering (`#include <cdd_int128.h>`).
 *
 * Compatible with Legacy MSVC (2005-2017), Modern MSVC (2019-2026), OpenWatcom,
 * GCC, and Clang under strict ISO C90 (C89).
 */

#ifndef CDD_INT128_H
#define CDD_INT128_H

#ifndef C_CDD_EXPORT
/** @brief Standalone empty export macro when used outside `cdd-c` CMake. */
#define C_CDD_EXPORT
#endif

/* clang-format off */
#include "c_cdd/int128.h"
#include "c_cdd/int128_math.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_INT128_H */
