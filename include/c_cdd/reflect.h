/**
 * @file reflect.h
 * @brief Runtime Type Information (RTTI) descriptors and visitor API for C89.
 *
 * Provides table-driven struct/enum field introspection using `offsetof` and
 * `sizeof`, runtime field lookup, and visitor pattern traversal.
 */

#ifndef C_CDD_REFLECT_H
#define C_CDD_REFLECT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

/**
 * @brief Logical kind of a reflected struct field.
 */
enum CddFieldTypeKind {
  CDD_FIELD_KIND_UNKNOWN = 0, /**< Unclassified field kind */
  CDD_FIELD_KIND_INTEGER = 1, /**< Signed/unsigned integer field */
  CDD_FIELD_KIND_NUMBER = 2,  /**< Floating-point number field */
  CDD_FIELD_KIND_BOOLEAN = 3, /**< Boolean (0/1) field */
  CDD_FIELD_KIND_STRING = 4,  /**< NUL-terminated C string (`char *`) */
  CDD_FIELD_KIND_ENUM = 5,    /**< Enumerated value field */
  CDD_FIELD_KIND_OBJECT = 6,  /**< Nested struct pointer (`struct T *`) */
  CDD_FIELD_KIND_ARRAY = 7    /**< Dynamic or fixed array field */
};

/**
 * @brief Runtime reflection descriptor for a single struct field.
 */
struct cdd_field_info {
  const char *name;           /**< C struct member identifier */
  const char *wire_name;      /**< Effective serialized wire name */
  const char *c_type;         /**< Exact C type string if specified */
  const char *ref_name;       /**< Referenced struct/enum or item type */
  enum CddFieldTypeKind kind; /**< Logical field type kind */
  size_t offset;              /**< Byte offset via `offsetof(struct T, f)` */
  size_t size;                /**< Field size in bytes via `sizeof` */
  size_t array_len_offset;    /**< Byte offset of `n_<field>` counter */
  unsigned int bit_width;     /**< Bit-field width (0 if not a bit-field) */
  int required;               /**< Non-zero if field is required */
  int skip;                   /**< Non-zero if skipped during serde */
};

/**
 * @brief Runtime reflection descriptor for a C struct or union container.
 */
struct cdd_type_info {
  const char *name;                    /**< C struct/union tag name */
  size_t size;                         /**< Total `sizeof(struct T)` in bytes */
  int is_union;                        /**< Non-zero if tagged union struct */
  const struct cdd_field_info *fields; /**< Array of reflected fields */
  size_t n_fields;                     /**< Number of reflected fields */
};

/**
 * @brief Callback signature for visiting each field of a reflected struct
 * instance at runtime.
 */
typedef cdd_c_error_t (*cdd_field_visitor_fn)(
    const struct cdd_field_info *field, const void *field_ptr, void *user_data);

/**
 * @brief Looks up a reflected field descriptor by C member name or wire name.
 *
 * @param[in] ti Runtime type descriptor table.
 * @param[in] name C field name or serialized wire name to look up.
 * @param[out] out_field Output pointer receiving the matching field descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_find_field(const struct cdd_type_info *ti, const char *name,
                       const struct cdd_field_info **out_field);

/**
 * @brief Iterates over all fields of a struct instance `obj` using its runtime
 * `cdd_type_info` table and invokes `visitor` with the computed member address
 * `(const char *)obj + field->offset`.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] visitor Callback invoked for each non-skipped field.
 * @param[in] user_data Opaque context pointer forwarded to `visitor`.
 * @return CDD_C_SUCCESS on success, or the first non-zero error code returned
 * by `visitor`.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_visit_fields(const void *obj, const struct cdd_type_info *ti,
                         cdd_field_visitor_fn visitor, void *user_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_REFLECT_H */
