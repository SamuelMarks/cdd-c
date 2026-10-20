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
typedef unsigned char _c_cdd_bool;

#ifdef bool
#undef bool
#endif
#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif

/**
 * @brief Macro bool.
 */
#define bool _c_cdd_bool
/**
 * @brief Macro true.
 */
#define true 1
/**
 * @brief Macro false.
 */
#define false 0

#ifdef __cplusplus
}
#endif
#endif
