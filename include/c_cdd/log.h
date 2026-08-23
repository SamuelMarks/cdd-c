#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
/* clang-format off */
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
cdd_c_error_t c_cdd_log_debug(const char *fmt, ...);
#define C_CDD_LOG_DEBUG c_cdd_log_debug
#else
/**
 * @brief Logs debug messages.
 * @param fmt Format string.
 * @param ... Arguments.
 */
cdd_c_error_t c_cdd_log_debug(const char *fmt, ...);
#define C_CDD_LOG_DEBUG 1 ? (cdd_c_error_t)0 : c_cdd_log_debug
#endif /* DEBUG */
#endif /* !C_CDD_LOG_DEBUG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_LOG_H */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
