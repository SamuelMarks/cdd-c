/**
 * @file c_orm_db.h
 * @brief Core interfaces and vtables for c-orm drivers.
 */

#ifndef C_ORM_DB_H
#define C_ORM_DB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_orm_meta.h"
#include <stdint.h>

/**
 * @brief Opaque database connection handle.
 */
typedef struct c_orm_db c_orm_db_t;

/**
 * @brief Opaque query/prepared statement handle.
 */
typedef struct c_orm_query c_orm_query_t;

/**
 * @brief Error codes returned by c-orm functions.
 */
typedef enum {
  C_ORM_OK = 0,
  C_ORM_ERROR_MEMORY,
  C_ORM_ERROR_CONNECTION,
  C_ORM_ERROR_SQL,
  C_ORM_ERROR_BIND,
  C_ORM_ERROR_STEP,
  C_ORM_ERROR_TYPE_MISMATCH,
  C_ORM_ERROR_NOT_FOUND,
  C_ORM_ERROR_NOT_IMPLEMENTED,
  C_ORM_ERROR_UNKNOWN
} c_orm_error_t;

/**
 * @brief Get the last error message from the database driver.
 *
 * @param db The database connection.
 * @return A string detailing the last error, or NULL.
 */
const char *c_orm_get_last_error_message(c_orm_db_t *db);

/**
 * @brief Query logging callback signature.
 */
typedef void (*c_orm_log_cb)(const char *sql, void *user_data);

/**
 * @brief Virtual table for database driver implementations.
 */
typedef struct c_orm_driver_vtable {
  c_orm_error_t (*connect)(const char *url, c_orm_db_t **out_db);
  c_orm_error_t (*disconnect)(c_orm_db_t *db);
  c_orm_error_t (*prepare)(c_orm_db_t *db, const char *sql,
                           c_orm_query_t **out_query);
  c_orm_error_t (*bind_int32)(c_orm_query_t *query, int index, int32_t val);
  c_orm_error_t (*bind_int64)(c_orm_query_t *query, int index, int64_t val);
  c_orm_error_t (*bind_double)(c_orm_query_t *query, int index, double val);
  c_orm_error_t (*bind_string)(c_orm_query_t *query, int index,
                               const char *val);
  c_orm_error_t (*bind_null)(c_orm_query_t *query, int index);
  c_orm_error_t (*step)(c_orm_query_t *query, int *out_has_row);
  c_orm_error_t (*get_int32)(c_orm_query_t *query, int index, int32_t *out_val);
  c_orm_error_t (*get_int64)(c_orm_query_t *query, int index, int64_t *out_val);
  c_orm_error_t (*get_double)(c_orm_query_t *query, int index, double *out_val);
  c_orm_error_t (*get_string)(c_orm_query_t *query, int index,
                              const char **out_val);
  c_orm_error_t (*is_null)(c_orm_query_t *query, int index, int *out_is_null);
  c_orm_error_t (*finalize)(c_orm_query_t *query);
  c_orm_error_t (*reset)(c_orm_query_t *query);
  const char *(*get_last_error)(c_orm_db_t *db);
} c_orm_driver_vtable_t;

/**
 * @brief Structure holding the generic DB context.
 */
struct c_orm_db {
  const c_orm_driver_vtable_t *vtable;
  void *driver_data;
  c_orm_log_cb log_cb;
  void *log_user_data;
};

/**
 * @brief Set the global logging callback for a database connection.
 *
 * @param db Database handle.
 * @param cb The callback function.
 * @param user_data Opaque pointer passed to the callback.
 */
void c_orm_set_log_callback(c_orm_db_t *db, c_orm_log_cb cb, void *user_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_ORM_DB_H */
