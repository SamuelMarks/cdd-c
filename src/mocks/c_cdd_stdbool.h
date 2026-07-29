/**
 * @file c_cdd_stdbool.h
 */
#ifndef C_CDD_STDBOOL_H_INTERNAL
#define C_CDD_STDBOOL_H_INTERNAL

#ifdef __cplusplus
extern "C" {
#endif
/* clang-format off */
#include <stddef.h>
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || (defined(_MSC_VER) && _MSC_VER >= 1800) || defined(__GNUC__) || defined(__clang__)
#include <stdbool.h>
#else
typedef size_t _c_cdd_bool;

#ifdef bool
#undef bool
#endif
#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif

#define bool _c_cdd_bool
#define true 1
#define false 0
#endif
/* clang-format on */

#ifdef __cplusplus
}
#endif
#endif
