/**
 * @file c_mapping.h
 * @brief Reusable type mapper for converting C types to OpenAPI Schemas.
 *
 * Provides a centralized registry rule set for translating:
 * - Primitives (`int` -> `integer`, `char *` -> `string`).
 * - Containers (`Type *` -> `array/object`, `Type []` -> `array`).
 * - References (`struct X` -> `$ref: X`).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_C_MAPPING_H
#define C_CDD_C_MAPPING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include <stddef.h>

/**
 * @brief Discriminator for the resulting OpenAPI type category.
 */
enum OpenApiTypeKind {
  OA_TYPE_PRIMITIVE, /**< integer, string, boolean, number */
  OA_TYPE_OBJECT,    /**< $ref to another schema */
  OA_TYPE_ARRAY,     /**< Array of items */
  OA_TYPE_UNKNOWN    /**< Unmappable type */
};

/**
 * @brief Result structure for a type mapping operation.
 */
struct OpenApiTypeMapping {
  enum OpenApiTypeKind kind; /**< The high-level category */
  char *oa_type;             /**< The OpenAPI type string (e.g. "integer") */
  char *oa_format;           /**< The format string (e.g. "int64"), or NULL */
  char *ref_name; /**< The referenced schema name (for OBJECT/ARRAY), or NULL */
};

/**
 * @brief Initialize a mapping result structure.
 * @param[out] out The structure to zero.
 */
extern C_CDD_EXPORT void c_mapping_init(struct OpenApiTypeMapping *out);

/**
 * @brief Free resources in a mapping result.
 * @param[in] out The structure to clean.
 */
extern C_CDD_EXPORT void c_mapping_free(struct OpenApiTypeMapping *out);

/**
 * @brief Map a C type string to an OpenAPI Schema definition.
 *
 * Analyzes the C type string (e.g. "const char *", "struct User", "int")
 * and populates the output structure with the corresponding OpenAPI properties.
 *
 * Rules:
 * - `int`, `long`, `short` -> integer (with format).
 * - `float`, `double` -> number (float/double).
 * - `char *`, `char[]` -> string.
 * - `struct X` -> object ($ref: X).
 * - `enum X` -> string ($ref: X) or integer depending on enum usage (defaults
 * to ref).
 * - `Type *` (non-char) -> array or object depending on context?
 *   - Note: This mapper assumes `Type *` is a reference to one object by
 * default, unless `is_array` hint is provided.
 *
 * @param[in] c_type The raw C type string (e.g. "unsigned int").
 * @param[in] decl_name The declaration name (e.g. "users[]"), used for array
 * heuristics.
 * @param[out] out The result structure.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on invalid args.
 */
extern C_CDD_EXPORT int c_mapping_map_type(const char *c_type,
                                           const char *decl_name,
                                           struct OpenApiTypeMapping *out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C_MAPPING_H */
