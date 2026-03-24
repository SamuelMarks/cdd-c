/**
 * @file c_orm_meta.h
 * @brief Common definitions and enums for c-orm code generation.
 */

#ifndef C_CDD_C_ORM_META_H
#define C_CDD_C_ORM_META_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <c_orm_meta.h>
#include <stddef.h>
/* clang-format on */

/**
 * @brief Metadata describing a single property/field of an ORM mapped struct.
 */
typedef struct c_orm_prop_meta {
  const char *name;      /**< Name of the property */
  const char *type;      /**< Type of the property */
  size_t offset;         /**< Offset in bytes within the struct */
  int is_array;          /**< Boolean flag indicating if it is an array */
  unsigned int length;   /**< Array length if applicable */
  int is_secure;         /**< Boolean flag indicating if it contains secure data */
} c_orm_prop_meta_t;

/**
 * @brief Metadata describing a full ORM mapped struct layout.
 */
typedef struct c_orm_meta {
  const char *name;                /**< Name of the struct */
  size_t size;                     /**< Total size in bytes */
  size_t num_props;                /**< Number of properties in the struct */
  const c_orm_prop_meta_t *props;  /**< Pointer to the properties array */
  void *driver_ctx;                /**< Phase 4: Identity Map and Generic Driver Context Extensions */
} c_orm_meta_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C_ORM_META_H */
