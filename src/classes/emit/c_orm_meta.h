/**
 * @file c_orm_meta.h
 * @brief Common definitions and enums for c-orm code generation.
 */

#ifndef C_CDD_C_ORM_META_H
#define C_CDD_C_ORM_META_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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

/* clang-format off */
#include <stddef.h>
/* clang-format on */

/**
 * @brief Metadata describing a single property/field of an ORM mapped struct.
 */
typedef struct c_orm_prop_meta {
  const char *name;
  const char *type;
  size_t offset;
  int is_array;
  unsigned int length;
  int is_secure;
} c_orm_prop_meta_t;

/**
 * @brief Metadata describing a full ORM mapped struct layout.
 */
typedef struct c_orm_meta {
  const char *name;
  size_t size;
  size_t num_props;
  const c_orm_prop_meta_t *props;
  void *driver_ctx; /* Phase 4: Identity Map and Generic Driver Context
                       Extensions */
} c_orm_meta_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C_ORM_META_H */
