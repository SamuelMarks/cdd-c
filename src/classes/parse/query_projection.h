/**
 * @file query_projection.h
 * @brief Core query projection definitions and implementation.
 */
#ifndef C_CDD_QUERY_PROJECTION_H
#define C_CDD_QUERY_PROJECTION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief SQL types supported in query projections.
 */
typedef enum { SQL_TYPE_INT = 1, SQL_TYPE_VARCHAR = 2 } cdd_c_sql_type_t;

/**
 * @brief Represents a single projected field in a query.
 */
typedef struct {
  char *name;            /**< Field alias or final output name */
  char *original_name;   /**< Original column or expression name */
  cdd_c_sql_type_t type; /**< SQL type */
  int is_aggregate;      /**< 1 if aggregate function, 0 otherwise */
} cdd_c_query_projection_field_t;

/**
 * @brief List of projected query fields.
 */
typedef struct {
  size_t n_fields;                        /**< Number of fields */
  size_t capacity;                        /**< Allocated capacity */
  cdd_c_query_projection_field_t *fields; /**< Array of projected fields */
} cdd_c_query_projection_t;

/**
 * @brief Initializes a query projection structure.
 *
 * @param[out] proj Pointer to the projection structure to initialize.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_c_query_projection_init(cdd_c_query_projection_t *proj);

/**
 * @brief Appends a projected field to a query projection list.
 *
 * @param[in,out] proj Target query projection list.
 * @param[in] field Field descriptor to copy and append.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_c_query_projection_add_field(cdd_c_query_projection_t *proj,
                                 const cdd_c_query_projection_field_t *field);

/**
 * @brief Frees all resources owned by a query projection structure.
 *
 * @param[in,out] proj Pointer to the projection structure to free.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_c_query_projection_free(cdd_c_query_projection_t *proj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_QUERY_PROJECTION_H */
