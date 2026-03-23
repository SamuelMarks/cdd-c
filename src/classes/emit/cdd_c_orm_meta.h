/**
 * @file c_orm_meta.h
 * @brief Common definitions and enums for c-orm code generation.
 */

#ifndef C_CDD_C_ORM_META_H
#define C_CDD_C_ORM_META_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_orm_meta.h>

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
