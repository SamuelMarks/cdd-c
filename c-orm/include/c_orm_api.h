/**
 * @file c_orm_api.h
 * @brief High-level API for c-orm: find, insert, update, delete.
 */

#ifndef C_ORM_API_H
#define C_ORM_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_orm_db.h"
#include "c_orm_meta.h"

/**
 * @brief Find a single record by its primary key.
 *
 * @param db Database connection.
 * @param meta Table metadata.
 * @param id_val Primary key value to search for (assumes string/int is passed
 * appropriately, but currently expects int32 for basic testing, we can pass as
 * void*). For safety we will pass as int32_t for now.
 * @param out_struct Pointer to an already allocated struct to hydrate.
 * @return C_ORM_OK on success, C_ORM_ERROR_NOT_FOUND if no row.
 */
c_orm_error_t c_orm_find_by_id_int32(c_orm_db_t *db,
                                     const c_orm_table_meta_t *meta,
                                     int32_t id_val, void *out_struct);

/**
 * @brief Find all records.
 *
 * @param db Database connection.
 * @param meta Table metadata.
 * @param out_array Pointer to the generic Array struct. Data will be allocated
 * dynamically.
 * @return C_ORM_OK on success.
 */
c_orm_error_t c_orm_find_all(c_orm_db_t *db, const c_orm_table_meta_t *meta,
                             void *out_array);

/**
 * @brief Insert a new record into the database.
 *
 * @param db Database connection.
 * @param meta Table metadata.
 * @param in_struct Pointer to the struct containing data to insert.
 * @return C_ORM_OK on success.
 */
c_orm_error_t c_orm_insert(c_orm_db_t *db, const c_orm_table_meta_t *meta,
                           const void *in_struct);

/**
 * @brief Update an existing record in the database by its primary key.
 *
 * @param db Database connection.
 * @param meta Table metadata.
 * @param in_struct Pointer to the struct containing the updated data (and PK).
 * @return C_ORM_OK on success.
 */
c_orm_error_t c_orm_update(c_orm_db_t *db, const c_orm_table_meta_t *meta,
                           const void *in_struct);

/**
 * @brief Delete a record from the database by its primary key.
 *
 * @param db Database connection.
 * @param meta Table metadata.
 * @param id_val Primary key value to delete.
 * @return C_ORM_OK on success.
 */
c_orm_error_t c_orm_delete_by_id_int32(c_orm_db_t *db,
                                       const c_orm_table_meta_t *meta,
                                       int32_t id_val);

/**
 * @brief Execute a raw query string that returns no results.
 */
c_orm_error_t c_orm_execute_raw(c_orm_db_t *db, const char *sql);

/* Transaction APIs */
c_orm_error_t c_orm_begin(c_orm_db_t *db);
c_orm_error_t c_orm_commit(c_orm_db_t *db);
c_orm_error_t c_orm_rollback(c_orm_db_t *db);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_ORM_API_H */
