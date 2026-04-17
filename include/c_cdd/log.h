#ifndef C_CDD_LOG_H
#define C_CDD_LOG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef LOG_DEBUG
#ifdef DEBUG
void c_cdd_log_debug(const char *fmt, ...);
#define LOG_DEBUG c_cdd_log_debug
#else
void c_cdd_log_debug(const char *fmt, ...);
#define LOG_DEBUG 1 ? (void)0 : c_cdd_log_debug
#endif /* DEBUG */
#endif /* !LOG_DEBUG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_LOG_H */
