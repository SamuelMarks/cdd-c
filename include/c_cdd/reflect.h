/**
 * @file reflect.h
 * @brief Runtime Type Information (RTTI) descriptors, visitor API, and type
 * registry for C89.
 *
 * Provides table-driven struct/enum field introspection using `offsetof` and
 * `sizeof`, bit-field trampoline accessors, runtime field/enum lookup, mutable
 * and const visitor pattern traversal, and dynamic type registration.
 */

#ifndef C_CDD_REFLECT_H
#define C_CDD_REFLECT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#include <stdint.h>
#endif
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
 * @brief Bit-field integer getter trampoline signature.
 *
 * @param[in] obj Pointer to parent struct instance.
 * @param[out] out_val Output integer value read from the bit-field.
 * @return CDD_C_SUCCESS on success, or error code on NULL arguments.
 */
typedef cdd_c_error_t (*cdd_bitfield_get_fn)(const void *obj, int *out_val);

/**
 * @brief Bit-field integer setter trampoline signature.
 *
 * @param[in,out] obj Pointer to parent struct instance.
 * @param[in] val Integer value to assign to the bit-field.
 * @return CDD_C_SUCCESS on success, or error code on NULL arguments.
 */
typedef cdd_c_error_t (*cdd_bitfield_set_fn)(void *obj, int val);

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
  const char *de_wire_name;   /**< Effective deserialized wire name */
  int is_pointer;             /**< Non-zero if field is a pointer */
  int is_nullable;            /**< Non-zero if field is a nullable pointer */
  int is_flexible_array;      /**< Non-zero if field is a flexible array */
  size_t fixed_array_len;     /**< Fixed inline array length, or 0 */
  int skip_serializing;       /**< Non-zero if omitted during serialization */
  int skip_deserializing;     /**< Non-zero if ignored during deserialization */
  cdd_bitfield_get_fn get_bitfield; /**< Optional bit-field getter callback */
  cdd_bitfield_set_fn set_bitfield; /**< Optional bit-field setter callback */
  size_t elem_size;                 /**< Array element stride in bytes */
  enum CddFieldTypeKind elem_kind;  /**< Array element logical kind */
  const char *const *aliases;       /**< Deserialization alias strings */
  size_t n_aliases;                 /**< Count of `aliases` */
  const char *doc;                  /**< Field description / Doxygen */
  const char *format;               /**< OpenAPI format (e.g. "int64") */
  const char *pattern;              /**< Regex validation pattern */
  const char *default_val;          /**< Default value literal string */
  int deprecated;                   /**< Non-zero if deprecated */
  int read_only;                    /**< Non-zero if readOnly */
  int write_only;                   /**< Non-zero if writeOnly */
  int flatten;                      /**< Non-zero if `@cdd flatten` */
  int has_min;                      /**< Non-zero if minimum set */
  double min_val;                   /**< Minimum numeric bound */
  int has_max;                      /**< Non-zero if maximum set */
  double max_val;                   /**< Maximum numeric bound */
  int has_min_len;                  /**< Non-zero if minLength set */
  size_t min_len;                   /**< Minimum string/array length */
  int has_max_len;                  /**< Non-zero if maxLength set */
  size_t max_len;                   /**< Maximum string/array length */
  int exclusive_min;                /**< Non-zero if exclusiveMinimum */
  int exclusive_max;                /**< Non-zero if exclusiveMaximum */
  int unique_items;                 /**< Non-zero if uniqueItems */
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
  size_t tag_offset; /**< Byte offset of `tag` discriminator for unions */
};

/**
 * @brief Runtime reflection descriptor for a single enum variant.
 */
struct cdd_enum_variant_info {
  const char *name;           /**< C enum variant suffix name */
  const char *wire_name;      /**< Serialized wire representation */
  int value;                  /**< Integer enumerator value */
  int skip;                   /**< Non-zero if skipped during serde */
  int is_other;               /**< Non-zero if fallback `@cdd(other)` variant */
  const char *const *aliases; /**< Alternative deserialization wire names */
  size_t n_aliases;           /**< Number of `aliases` */
};

/**
 * @brief Runtime reflection descriptor for a C enum container.
 */
struct cdd_enum_info {
  const char *name;                             /**< C enum tag name */
  const struct cdd_enum_variant_info *variants; /**< Array of variants */
  size_t n_variants;                            /**< Number of variants */
  int other_idx; /**< Index of `@cdd(other)` fallback variant, or -1 */
};

/**
 * @brief Runtime reflection descriptor for a single C function parameter.
 */
struct cdd_param_info {
  const char *name;           /**< Parameter identifier */
  const char *c_type;         /**< Raw C parameter type string */
  enum CddFieldTypeKind kind; /**< Logical parameter kind */
  int is_pointer;             /**< Non-zero if pointer parameter */
  int is_const;               /**< Non-zero if const-qualified */
  int is_nullable;            /**< Non-zero if nullable */
  int is_out;                 /**< Non-zero if output pointer param */
  int required;               /**< Non-zero if required */
  const char *doc;            /**< Optional parameter documentation */
};

/**
 * @brief Type-erased function invocation trampoline signature.
 *
 * @param[in] args Array of `n_params` pointers to argument values.
 * @param[out] ret_out Optional pointer to store the return value.
 * @return CDD_C_SUCCESS on success, or error code from the invoked function.
 */
typedef cdd_c_error_t (*cdd_func_invoke_fn)(const void *const *args,
                                            void *ret_out);

/**
 * @brief Runtime reflection descriptor for a C function or method signature.
 */
struct cdd_func_info {
  const char *name;                    /**< C function identifier */
  const char *return_c_type;           /**< Return C type string */
  enum CddFieldTypeKind return_kind;   /**< Logical return kind */
  const struct cdd_param_info *params; /**< Parameter descriptor table */
  size_t n_params;                     /**< Number of parameters */
  const char *doc;                     /**< Function docstring */
  const char *route_method;            /**< Optional HTTP verb */
  const char *route_path;              /**< Optional HTTP route path */
  const char *summary;                 /**< Optional summary text */
  int is_variadic;                     /**< Non-zero if variadic */
  int is_deprecated;                   /**< Non-zero if deprecated */
  cdd_func_invoke_fn invoke;           /**< Optional invocation trampoline */
};

/**
 * @brief Callback signature for visiting each field of a const reflected struct
 * instance at runtime. Note: `field_ptr` is `NULL` for bit-fields (`bit_width >
 * 0`).
 */
typedef cdd_c_error_t (*cdd_field_visitor_fn)(
    const struct cdd_field_info *field, const void *field_ptr, void *user_data);

/**
 * @brief Callback signature for visiting and mutating each field of a reflected
 * struct instance at runtime. Note: `field_ptr` is `NULL` for bit-fields
 * (`bit_width > 0`).
 */
typedef cdd_c_error_t (*cdd_field_mut_visitor_fn)(
    const struct cdd_field_info *field, void *field_ptr, void *user_data);

/**
 * @brief Looks up a reflected field descriptor by C member name, serialization
 * wire name, or deserialization wire name.
 *
 * @param[in] ti Runtime type descriptor table.
 * @param[in] name C field name or serialized/deserialized wire name to look up.
 * @param[out] out_field Output pointer receiving the matching field descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_find_field(const struct cdd_type_info *ti, const char *name,
                       const struct cdd_field_info **out_field);

/**
 * @brief Iterates over all fields of a const struct instance `obj` using its
 * runtime `cdd_type_info` table and invokes `visitor` with the computed member
 * address `(const char *)obj + field->offset` (or `NULL` for bit-fields).
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

/**
 * @brief Iterates over all fields of a mutable struct instance `obj` using its
 * runtime `cdd_type_info` table and invokes `visitor` with the computed member
 * address `(char *)obj + field->offset` (or `NULL` for bit-fields).
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] visitor Mutable callback invoked for each non-skipped field.
 * @param[in] user_data Opaque context pointer forwarded to `visitor`.
 * @return CDD_C_SUCCESS on success, or the first non-zero error code returned
 * by `visitor`.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_visit_fields_mut(void *obj, const struct cdd_type_info *ti,
                             cdd_field_mut_visitor_fn visitor, void *user_data);

/**
 * @brief Reads an integer or boolean field (including bit-fields with a
 * registered `get_bitfield` trampoline) from a struct instance by field
 * descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[out] out_val Output integer value.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_get_int(
    const void *obj, const struct cdd_field_info *field, int *out_val);

/**
 * @brief Writes an integer or boolean field (including bit-fields with a
 * registered `set_bitfield` trampoline) on a struct instance by field
 * descriptor.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[in] val Integer value to store.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_set_int(void *obj, const struct cdd_field_info *field, int val);

/**
 * @brief Reads a 64-bit integer field (or narrower integer/bit-field widened to
 * 64 bits) from a struct instance by field descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[out] out_val Output 64-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_get_int64(
    const void *obj, const struct cdd_field_info *field, int64_t *out_val);

/**
 * @brief Writes a 64-bit integer value onto an integer field (width-aware for
 * 1, 2, 4, or 8 byte integer fields and bit-fields) on a struct instance.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[in] val 64-bit integer value to store.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_set_int64(
    void *obj, const struct cdd_field_info *field, int64_t val);

/**
 * @brief Reads an unsigned 64-bit integer field (or narrower integer/bit-field
 * widened to 64 bits) from a struct instance by field descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[out] out_val Output unsigned 64-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_get_uint64(
    const void *obj, const struct cdd_field_info *field, uint64_t *out_val);

/**
 * @brief Writes an unsigned 64-bit integer value onto an integer field
 * (width-aware for 1, 2, 4, or 8 byte integer fields and bit-fields) on a
 * struct instance.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[in] val Unsigned 64-bit integer value to store.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_set_uint64(
    void *obj, const struct cdd_field_info *field, uint64_t val);

/**
 * @brief Reads a floating-point field (`double` or `float`) from a struct
 * instance by field descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor (`CDD_FIELD_KIND_NUMBER`).
 * @param[out] out_val Output `double` value.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_get_double(
    const void *obj, const struct cdd_field_info *field, double *out_val);

/**
 * @brief Writes a floating-point value (`double` or `float`) onto a struct
 * instance by field descriptor.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor (`CDD_FIELD_KIND_NUMBER`).
 * @param[in] val Floating-point value to store.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_set_double(
    void *obj, const struct cdd_field_info *field, double val);

/**
 * @brief Reads a boolean field (`0` or `1`) from a struct instance by field
 * descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor (`CDD_FIELD_KIND_BOOLEAN`).
 * @param[out] out_val Output boolean value (`0` or `1`).
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_get_bool(
    const void *obj, const struct cdd_field_info *field, int *out_val);

/**
 * @brief Writes a boolean value (`0` or `1`) onto a struct instance by field
 * descriptor.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor (`CDD_FIELD_KIND_BOOLEAN`).
 * @param[in] val Boolean value to store (`0` or non-zero).
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_set_bool(void *obj, const struct cdd_field_info *field, int val);

/**
 * @brief Reads a string field (`char *`) from a struct instance by field
 * descriptor.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[out] out_str Output string pointer.
 * @return CDD_C_SUCCESS on success, or error code on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_get_string(
    const void *obj, const struct cdd_field_info *field, const char **out_str);

/**
 * @brief Writes a newly duplicated string (`char *`) onto a struct instance by
 * field descriptor, freeing any previously non-NULL heap string.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @param[in] val NUL-terminated string to duplicate (or NULL to clear).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on invalid type/arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_set_string(
    void *obj, const struct cdd_field_info *field, const char *val);

/**
 * @brief Reads the runtime element count of an array field (either from the
 * companion `n_<field>` counter at `field->array_len_offset` or from
 * `field->fixed_array_len` for inline fixed arrays).
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected array field descriptor (`CDD_FIELD_KIND_ARRAY`).
 * @param[out] out_len Output element count.
 * @return CDD_C_SUCCESS on success, or error code on invalid arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_array_len(
    const void *obj, const struct cdd_field_info *field, size_t *out_len);

/**
 * @brief Computes the runtime pointer to element `index` of a reflected array
 * field (supporting both heap pointer arrays and inline fixed-size arrays).
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected array field descriptor (`CDD_FIELD_KIND_ARRAY`).
 * @param[in] index Zero-based element index.
 * @param[out] out_elem_ptr Output pointer to element `index`.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `index` is out of
 * bounds or array pointer is NULL, or CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_array_at(const void *obj, const struct cdd_field_info *field,
                     size_t index, const void **out_elem_ptr);

/**
 * @brief Computes the mutable runtime pointer to element `index` of a reflected
 * array field (supporting both heap pointer arrays and inline fixed-size
 * arrays).
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected array field descriptor (`CDD_FIELD_KIND_ARRAY`).
 * @param[in] index Zero-based element index.
 * @param[out] out_elem_ptr Output mutable pointer to element `index`.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `index` is out of
 * bounds or array pointer is NULL, or CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_array_at_mut(void *obj, const struct cdd_field_info *field,
                         size_t index, void **out_elem_ptr);

/**
 * @brief Dynamically resizes a heap-allocated array field (`is_pointer != 0`,
 * `fixed_array_len == 0`) on a struct instance, zero-initializing any newly
 * grown elements and updating the companion `n_<field>` counter at
 * `field->array_len_offset`.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] field Reflected array field descriptor (`CDD_FIELD_KIND_ARRAY`).
 * @param[in] new_len Desired new element count (`0` frees the array buffer).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT if `field` is not a dynamic heap array.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_array_resize(
    void *obj, const struct cdd_field_info *field, size_t new_len);

/**
 * @brief Validates a single reflected field on `obj` against its schema
 * constraints (`required`, `has_min`/`min_val`, `has_max`/`max_val`,
 * `has_min_len`/`min_len`, `has_max_len`/`max_len`).
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] field Reflected field descriptor.
 * @return CDD_C_SUCCESS if valid, or CDD_C_ERROR_INVALID_ARGUMENT if a
 * constraint is violated or arguments are NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_validate_field(const void *obj, const struct cdd_field_info *field);

/**
 * @brief Validates all non-skipped fields of a struct or active union branch on
 * `obj` against their schema constraints in `ti`.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @return CDD_C_SUCCESS if all fields pass validation, or
 * CDD_C_ERROR_INVALID_ARGUMENT on constraint violation or NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_validate(const void *obj, const struct cdd_type_info *ti);

/**
 * @brief Recursively visits all fields of `obj` and any nested
 * `CDD_FIELD_KIND_OBJECT` or `CDD_FIELD_KIND_ARRAY` of objects whose target
 * `ref_name` is registered in the runtime type registry.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] visitor Callback invoked for each visited field.
 * @param[in] user_data Opaque context pointer forwarded to `visitor`.
 * @return CDD_C_SUCCESS on success, or the first non-zero error code returned
 * by `visitor`.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_visit_deep(const void *obj, const struct cdd_type_info *ti,
                       cdd_field_visitor_fn visitor, void *user_data);

/**
 * @brief Looks up a reflected enum variant descriptor by C member name or wire
 * name.
 *
 * @param[in] ei Runtime enum descriptor table.
 * @param[in] name C variant suffix name or wire name to look up.
 * @param[out] out_variant Output pointer receiving the matching variant.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_find_enum_variant(const struct cdd_enum_info *ei, const char *name,
                              const struct cdd_enum_variant_info **out_variant);

/**
 * @brief Looks up a reflected enum variant descriptor by integer enumerator
 * value.
 *
 * @param[in] ei Runtime enum descriptor table.
 * @param[in] value Integer enumerator value to search for.
 * @param[out] out_variant Output pointer receiving the matching variant.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_find_enum_by_value(
    const struct cdd_enum_info *ei, int value,
    const struct cdd_enum_variant_info **out_variant);

/**
 * @brief Registers a `cdd_type_info` descriptor in the runtime type registry.
 *
 * @param[in] ti Runtime struct/union type descriptor to register.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL/unnamed descriptor.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_register_type(const struct cdd_type_info *ti);

/**
 * @brief Looks up a registered `cdd_type_info` descriptor by struct name.
 *
 * @param[in] name Struct/union tag name.
 * @param[out] out_ti Output pointer receiving the registered descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_type_by_name(const char *name, const struct cdd_type_info **out_ti);

/**
 * @brief Registers a `cdd_enum_info` descriptor in the runtime enum registry.
 *
 * @param[in] ei Runtime enum descriptor to register.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL/unnamed descriptor.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_register_enum(const struct cdd_enum_info *ei);

/**
 * @brief Looks up a registered `cdd_enum_info` descriptor by enum name.
 *
 * @param[in] name Enum tag name.
 * @param[out] out_ei Output pointer receiving the registered enum descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_enum_by_name(const char *name, const struct cdd_enum_info **out_ei);

/**
 * @brief Registers a `cdd_func_info` descriptor in the runtime function
 * registry.
 *
 * @param[in] fi Runtime function descriptor to register.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL/unnamed descriptor.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_register_func(const struct cdd_func_info *fi);

/**
 * @brief Looks up a registered `cdd_func_info` descriptor by function name.
 *
 * @param[in] name C function identifier.
 * @param[out] out_fi Output pointer receiving the registered function
 * descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_func_by_name(const char *name, const struct cdd_func_info **out_fi);

/**
 * @brief Looks up a parameter descriptor inside a `cdd_func_info` by parameter
 * name.
 *
 * @param[in] fi Runtime function descriptor.
 * @param[in] name Parameter identifier to search for.
 * @param[out] out_param Output pointer receiving the matching parameter
 * descriptor.
 * @return CDD_C_SUCCESS if found, CDD_C_ERROR_NOT_FOUND if absent, or
 * CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_find_param(const struct cdd_func_info *fi, const char *name,
                       const struct cdd_param_info **out_param);

/**
 * @brief Retrieves the number of currently registered struct/union types in the
 * runtime type registry.
 *
 * @param[out] out_count Output count of registered types.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_count` is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_registered_types_count(size_t *out_count);

/**
 * @brief Retrieves the registered `cdd_type_info` descriptor at `index`.
 *
 * @param[in] index Zero-based index into the type registry.
 * @param[out] out_ti Output pointer receiving the descriptor.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `index` is out of
 * bounds, or CDD_C_ERROR_INVALID_ARGUMENT if `out_ti` is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_registered_type_at(
    size_t index, const struct cdd_type_info **out_ti);

/**
 * @brief Retrieves the number of currently registered enum descriptors in the
 * runtime enum registry.
 *
 * @param[out] out_count Output count of registered enums.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_count` is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_registered_enums_count(size_t *out_count);

/**
 * @brief Retrieves the registered `cdd_enum_info` descriptor at `index`.
 *
 * @param[in] index Zero-based index into the enum registry.
 * @param[out] out_ei Output pointer receiving the descriptor.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `index` is out of
 * bounds, or CDD_C_ERROR_INVALID_ARGUMENT if `out_ei` is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_registered_enum_at(
    size_t index, const struct cdd_enum_info **out_ei);

/**
 * @brief Retrieves the number of currently registered function descriptors in
 * the runtime function registry.
 *
 * @param[out] out_count Output count of registered functions.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if
 * `out_count` is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_registered_funcs_count(size_t *out_count);

/**
 * @brief Retrieves the registered `cdd_func_info` descriptor at `index`.
 *
 * @param[in] index Zero-based index into the function registry.
 * @param[out] out_fi Output pointer receiving the descriptor.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `index` is out of
 * bounds, or CDD_C_ERROR_INVALID_ARGUMENT if `out_fi` is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_registered_func_at(
    size_t index, const struct cdd_func_info **out_fi);

/**
 * @brief Clears all dynamically registered struct, enum, and function
 * descriptors from the runtime type registry and frees internal storage.
 *
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_clear_registry(void);

/**
 * @brief Recursively visits and mutates all fields of `obj` and any nested
 * `CDD_FIELD_KIND_OBJECT` or `CDD_FIELD_KIND_ARRAY` of objects whose target
 * `ref_name` is registered in the runtime type registry.
 *
 * @param[in,out] obj Mutable pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] visitor Mutable callback invoked for each visited field.
 * @param[in] user_data Opaque context pointer forwarded to `visitor`.
 * @return CDD_C_SUCCESS on success, or the first non-zero error code returned
 * by `visitor`.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_visit_deep_mut(void *obj, const struct cdd_type_info *ti,
                           cdd_field_mut_visitor_fn visitor, void *user_data);

/**
 * @brief Resolves a JSON Pointer (`"/user/items/0/id"`) or dot/bracket path
 * (`"user.items[0].id"`) against a reflected struct hierarchy.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path Path string to resolve.
 * @param[out] out_field Output pointer receiving the leaf field descriptor.
 * @param[out] out_parent_obj Output pointer receiving the immediate parent
 * struct containing the resolved leaf field.
 * @param[out] out_field_ptr Output pointer receiving the address of the leaf
 * field or array element (or `NULL` for bit-fields).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if any segment is
 * missing or NULL, or CDD_C_ERROR_INVALID_ARGUMENT on invalid arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_resolve_path(
    const void *obj, const struct cdd_type_info *ti, const char *path,
    const struct cdd_field_info **out_field, const void **out_parent_obj,
    const void **out_field_ptr);

/**
 * @brief Reads a 64-bit integer value at a nested reflection path.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[out] out_val Output 64-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_get_int64_by_path(const void *obj, const struct cdd_type_info *ti,
                              const char *path, int64_t *out_val);

/**
 * @brief Writes a 64-bit integer value at a nested reflection path.
 *
 * @param[in,out] obj Mutable pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[in] val 64-bit integer value to store.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_set_int64_by_path(
    void *obj, const struct cdd_type_info *ti, const char *path, int64_t val);

/**
 * @brief Reads an unsigned 64-bit integer value at a nested reflection path.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[out] out_val Output unsigned 64-bit integer value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_get_uint64_by_path(const void *obj, const struct cdd_type_info *ti,
                               const char *path, uint64_t *out_val);

/**
 * @brief Writes an unsigned 64-bit integer value at a nested reflection path.
 *
 * @param[in,out] obj Mutable pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[in] val Unsigned 64-bit integer value to store.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_set_uint64_by_path(
    void *obj, const struct cdd_type_info *ti, const char *path, uint64_t val);

/**
 * @brief Reads a boolean value (`0` or `1`) at a nested reflection path.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[out] out_val Output boolean value (`0` or `1`).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_get_bool_by_path(const void *obj, const struct cdd_type_info *ti,
                             const char *path, int *out_val);

/**
 * @brief Writes a boolean value (`0` or `1`) at a nested reflection path.
 *
 * @param[in,out] obj Mutable pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[in] val Boolean value to store (`0` or non-zero).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_set_bool_by_path(
    void *obj, const struct cdd_type_info *ti, const char *path, int val);

/**
 * @brief Reads a floating-point value (`double`) at a nested reflection path.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[out] out_val Output `double` value.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_get_double_by_path(const void *obj, const struct cdd_type_info *ti,
                               const char *path, double *out_val);

/**
 * @brief Writes a floating-point value (`double`) at a nested reflection path.
 *
 * @param[in,out] obj Mutable pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[in] val Floating-point value to store.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_set_double_by_path(
    void *obj, const struct cdd_type_info *ti, const char *path, double val);

/**
 * @brief Reads a string pointer at a nested reflection path.
 *
 * @param[in] obj Pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[out] out_str Output string pointer.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_get_string_by_path(const void *obj, const struct cdd_type_info *ti,
                               const char *path, const char **out_str);

/**
 * @brief Writes a string value at a nested reflection path.
 *
 * @param[in,out] obj Mutable pointer to root struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in] path JSON Pointer or dot/bracket path.
 * @param[in] val NUL-terminated string to store (or NULL to clear).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_set_string_by_path(void *obj, const struct cdd_type_info *ti,
                               const char *path, const char *val);

/**
 * @brief Allocates and initializes a struct instance with default values using
 * its runtime `cdd_type_info` table.
 *
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_obj Output pointer receiving the newly allocated struct.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on invalid arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_struct_default(const struct cdd_type_info *ti, void **out_obj);

/**
 * @brief Recursively frees all heap-allocated members (strings, dynamic arrays,
 * nested heap objects) owned by `obj` according to `ti` (does not free `obj`
 * itself).
 *
 * @param[in,out] obj Mutable pointer to struct instance to clean up.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * `ti`.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_struct_cleanup(void *obj, const struct cdd_type_info *ti);

/**
 * @brief Performs a deep recursive copy of `src` into a newly heap-allocated
 * struct `*out_dest` driven by `ti`.
 *
 * @param[in] src Source struct instance pointer.
 * @param[out] out_dest Output pointer receiving the cloned struct instance.
 * @param[in] ti Runtime type descriptor for `src`.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_struct_deepcopy(
    const void *src, void **out_dest, const struct cdd_type_info *ti);

/**
 * @brief Performs a deep structural equality comparison between two struct
 * instances `a` and `b` driven by `ti`.
 *
 * @param[in] a First struct instance pointer (may be NULL).
 * @param[in] b Second struct instance pointer (may be NULL).
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_eq Output flag set to `1` if equal, `0` otherwise.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * `ti` or `out_eq`.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_struct_eq(
    const void *a, const void *b, const struct cdd_type_info *ti, int *out_eq);

/**
 * @brief Serializes a reflected struct instance `obj` into a newly allocated
 * JSON string driven purely by its runtime `cdd_type_info` table.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[out] out_json Output pointer receiving the heap-allocated JSON string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on invalid arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_to_json(
    const void *obj, const struct cdd_type_info *ti, char **out_json);

/**
 * @brief Populates an existing `JSON_Object` in-place from a struct instance
 * `obj` driven purely by its runtime `cdd_type_info` table.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[in,out] jobj Target JSON object (as `void *` for header decoupling).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_to_json_object(
    const void *obj, const struct cdd_type_info *ti, void *jobj);

/**
 * @brief Deserializes a JSON string into a newly allocated struct instance
 * `*out_obj` driven purely by its runtime `cdd_type_info` table.
 *
 * @param[in] json_str NUL-terminated JSON string.
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_obj Output pointer receiving the allocated struct instance.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on invalid JSON or constraint failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_from_json(
    const char *json_str, const struct cdd_type_info *ti, void **out_obj);

/**
 * @brief Populates an existing struct instance `obj` in-place from a
 * `JSON_Object` driven purely by its runtime `cdd_type_info` table.
 *
 * @param[in] jobj Source JSON object (as `const void *` for header decoupling).
 * @param[in] ti Runtime type descriptor.
 * @param[in,out] obj Target struct instance to populate.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_from_json_object(
    const void *jobj, const struct cdd_type_info *ti, void *obj);

/**
 * @brief Computes an RFC 7396 JSON Merge Patch string representing the changes
 * from `source` to `target` for a reflected struct type `ti`.
 *
 * @param[in] source Baseline struct instance.
 * @param[in] target Modified struct instance.
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_patch_json Output pointer receiving the JSON Merge Patch
 * string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_merge_patch(const void *source, const void *target,
                        const struct cdd_type_info *ti, char **out_patch_json);

/**
 * @brief Applies an RFC 7396 JSON Merge Patch string onto an existing mutable
 * struct instance `target` using its runtime `cdd_type_info` table.
 *
 * @param[in,out] target Mutable struct instance to patch in-place.
 * @param[in] ti Runtime type descriptor.
 * @param[in] patch_json NUL-terminated JSON Merge Patch string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_apply_merge_patch(
    void *target, const struct cdd_type_info *ti, const char *patch_json);

/**
 * @brief Generates an OpenAPI 3.2.0 / JSON Schema Draft 2020-12 object string
 * directly from a runtime `cdd_type_info` descriptor table.
 *
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_schema_json Output pointer receiving the heap-allocated JSON
 * Schema string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_to_json_schema(
    const struct cdd_type_info *ti, char **out_schema_json);

/**
 * @brief Serializes a reflected struct instance `obj` into an
 * `application/x-www-form-urlencoded` query string (`key=value&...`) driven
 * by its runtime `cdd_type_info` table.
 *
 * @param[in] obj Pointer to struct instance.
 * @param[in] ti Runtime type descriptor for `obj`.
 * @param[out] out_form Output pointer receiving the heap-allocated form string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 * or CDD_C_ERROR_INVALID_ARGUMENT on NULL arguments.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_to_form_urlencoded(
    const void *obj, const struct cdd_type_info *ti, char **out_form);

/**
 * @brief Deserializes an `application/x-www-form-urlencoded` string
 * (`key=value&...`) into a newly allocated struct instance `*out_obj` driven by
 * `ti`.
 *
 * @param[in] form_str NUL-terminated form-urlencoded string.
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_obj Output pointer receiving the allocated struct instance.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_from_form_urlencoded(
    const char *form_str, const struct cdd_type_info *ti, void **out_obj);

/**
 * @brief Parses CLI `argc`/`argv` flags (`--<field> <val>` or
 * `--<field>=<val>`) into a newly allocated struct instance `*out_obj` driven
 * by `ti`.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @param[in] ti Runtime type descriptor.
 * @param[out] out_obj Output pointer receiving the allocated struct instance.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_reflect_from_cli(int argc, const char *const *argv,
                     const struct cdd_type_info *ti, void **out_obj);

/**
 * @brief Applies an RFC 6902 JSON Patch array (`[{"op": "add"|"remove"|
 * "replace"|"move"|"copy"|"test", "path": "/...", "value": ...}]`) onto a
 * mutable reflected struct instance `target`.
 *
 * @param[in,out] target Mutable struct instance to patch.
 * @param[in] ti Runtime type descriptor for `target`.
 * @param[in] patch_json NUL-terminated RFC 6902 JSON Patch array string.
 * @return CDD_C_SUCCESS on success, or error code on invalid patch or failed
 * `test` assertion.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_apply_json_patch(
    void *target, const struct cdd_type_info *ti, const char *patch_json);

/**
 * @brief Invokes a reflected function descriptor using its type-erased
 * `invoke` trampoline.
 *
 * @param[in] fi Runtime function descriptor.
 * @param[in] args Array of pointers to argument values (may be NULL if
 * `fi->n_params == 0`).
 * @param[out] ret_out Optional pointer to receive the return value.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if `fi->invoke` is
 * NULL, or error code returned by the trampoline.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_invoke_func(
    const struct cdd_func_info *fi, const void *const *args, void *ret_out);

/**
 * @brief Looks up a registered function by `func_name` and invokes its
 * type-erased trampoline.
 *
 * @param[in] func_name Registered function identifier.
 * @param[in] args Array of pointers to argument values.
 * @param[out] ret_out Optional pointer to receive the return value.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_NOT_FOUND if not registered or
 * no trampoline, or error code returned by the trampoline.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_reflect_invoke_by_name(
    const char *func_name, const void *const *args, void *ret_out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_REFLECT_H */
