/**
 * @file c_orm_meta.h
 * @brief Core definitions and structures for c-orm.
 */

#ifndef C_ORM_META_H
#define C_ORM_META_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(_MSC_VER) && _MSC_VER < 1800
#if !defined(__cplusplus)
#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#endif
#else
#include <stdbool.h>
#endif
#include <stddef.h>

/**
 * @brief Data types supported by c-orm.
 */
typedef enum {
  C_ORM_TYPE_INT32,
  C_ORM_TYPE_INT64,
  C_ORM_TYPE_FLOAT,
  C_ORM_TYPE_DOUBLE,
  C_ORM_TYPE_BOOL,
  C_ORM_TYPE_STRING,
  C_ORM_TYPE_DATE,
  C_ORM_TYPE_TIMESTAMP,
  C_ORM_TYPE_UNKNOWN
} c_orm_type_t;

/**
 * @brief Column metadata definition.
 */
typedef struct {
  const char *name;      /**< Column name. */
  c_orm_type_t type;     /**< Data type. */
  size_t offset;         /**< Offset in the target C struct. */
  bool is_pk;            /**< True if column is a Primary Key. */
  bool is_nullable;      /**< True if column can be NULL. */
  const char *fk_target; /**< Target table name if Foreign Key, else NULL. */
} c_orm_column_meta_t;

/**
 * @brief Table metadata definition.
 */
typedef struct {
  const char *name;                   /**< Table name. */
  const c_orm_column_meta_t *columns; /**< Array of column metadata. */
  size_t num_columns;                 /**< Number of columns. */
  size_t struct_size; /**< Size of the generated struct in bytes. */

  /* Pre-compiled basic query templates */
  const char *query_select_all;
  const char *query_select_by_pk;
  const char *query_insert;
  const char *query_update;
  const char *query_delete_by_pk;
} c_orm_table_meta_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_ORM_META_H */
