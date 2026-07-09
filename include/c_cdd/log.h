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
void c_cdd_log_debug(const char *fmt, ...);
#define C_CDD_LOG_DEBUG c_cdd_log_debug
#else
/**
 * @brief Logs debug messages.
 * @param fmt Format string.
 * @param ... Arguments.
 */
void c_cdd_log_debug(const char *fmt, ...);
#define C_CDD_LOG_DEBUG 1 ? (void)0 : c_cdd_log_debug
#endif /* DEBUG */
#endif /* !C_CDD_LOG_DEBUG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_LOG_H */
