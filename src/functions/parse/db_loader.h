#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */
#ifndef C_CDD_DB_LOADER_H
#define C_CDD_DB_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Checks if libpq is available.
 * @param[out] out_avail 1 if available, 0 otherwise.
 * @return 0 on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t check_libpq_available(int *out_avail);

/**
 * @brief Checks if sqlite3 is available.
 * @param[out] out_avail 1 if available, 0 otherwise.
 * @return 0 on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t check_sqlite3_available(int *out_avail);

/**
 * @brief Checks if mysql is available.
 * @param[out] out_avail 1 if available, 0 otherwise.
 * @return 0 on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t check_mysql_available(int *out_avail);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_DB_LOADER_H */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
