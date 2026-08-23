#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
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
/* clang-format on */
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

#ifdef __cplusplus
}
#endif
#endif

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
