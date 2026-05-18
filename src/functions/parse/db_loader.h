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
extern int check_libpq_available(int *out_avail);

/**
 * @brief Checks if sqlite3 is available.
 * @param[out] out_avail 1 if available, 0 otherwise.
 * @return 0 on success, error code otherwise.
 */
extern int check_sqlite3_available(int *out_avail);

/**
 * @brief Checks if mysql is available.
 * @param[out] out_avail 1 if available, 0 otherwise.
 * @return 0 on success, error code otherwise.
 */
extern int check_mysql_available(int *out_avail);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_DB_LOADER_H */
