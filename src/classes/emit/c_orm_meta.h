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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C_ORM_META_H */
