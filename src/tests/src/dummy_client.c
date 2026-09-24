/**
 * @file dummy_client.c
 * @brief Dummy client implementation for testing generated client integration.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_c_error.h"
#include "lib_export.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

LIB_EXPORT cdd_c_error_t dummy_client(void);

LIB_EXPORT cdd_c_error_t dummy_client(void) { return CDD_C_SUCCESS; }

#ifdef __cplusplus
}
#endif /* __cplusplus */
