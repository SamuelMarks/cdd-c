/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

/**
 * @file log.h
 * @brief Logging utilities.
 */
#ifndef C_CDD_LOG_H
#define C_CDD_LOG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef C_CDD_LOG_DEBUG
#ifdef DEBUG
/**
 * @brief Logs debug messages.
 * @param fmt Format string.
 * @param ... Arguments.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_log_debug(const char *fmt, ...);
#define C_CDD_LOG_DEBUG c_cdd_log_debug
#else
/**
 * @brief Logs debug messages.
 * @param fmt Format string.
 * @param ... Arguments.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_log_debug(const char *fmt, ...);
#if defined(__GNUC__)
#pragma GCC system_header
#define C_CDD_LOG_DEBUG(...) ((void)0)
#else
#if _MSC_VER >= 1400
#define C_CDD_LOG_DEBUG(...) ((void)0)
#else
#define C_CDD_LOG_DEBUG 1 ? (void)0 : (void)c_cdd_log_debug
#endif
#endif
#endif /* DEBUG */
#endif /* !C_CDD_LOG_DEBUG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_LOG_H */
