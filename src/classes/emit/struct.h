/**
 * @file codegen_struct.h
 * @brief Struct Lifecycle generation module.
 *
 * Provides functionality to generate C "Data Class" utilities:
 * - `_cleanup`: Recursive memory freeing.
 * - `_deepcopy`: Recursive independent copying.
 * - `_eq`: Deep equality comparison.
 * - `_default`: Initialization with default values.
 * - `_debug` / `_display`: Inspection utilities.
 *
 * Updates:
 * - Support for `nullptr` keyword (mapped to `NULL`).
 * - Support for binary literals `0b...` (mapped to decimal).
 * - Flexible array member support.
 * - Bit-field support via `bit_width`.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_STRUCT_H
#define C_CDD_CODEGEN_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdio.h>

#include "c_cdd_export.h"
#include "c_cdd_stdbool.h"
#include "classes/emit/enum.h"

/**
 * @brief Represents a single field within a struct.
 * Used to drive generation logic based on type traits.
 */
struct StructField {
  char name[64]; /**< Field identifier */
  char type[64]; /**< C or Logical Field type (e.g. "string", "integer",
                    "object") */
  char ref[64];  /**< Reference type name (for objects/enums) or item type (for
                    arrays) */
  char **type_union;       /**< Optional type array (e.g. ["string","null"]) */
  size_t n_type_union;     /**< Count of type_union entries */
  char default_val[64];    /**< Default value literal (e.g. "5", "0b101",
                              "nullptr") or empty */
  char *schema_extra_json; /**< Serialized JSON for extra schema keywords */
  char *items_extra_json;  /**< Serialized JSON for array items keywords */
  char **items_type_union; /**< Optional items type array for arrays */
  size_t n_items_type_union; /**< Count of items_type_union entries */

  /* Validation Constraints */
  int has_min;           /**< 1 if minimum constraint exists */
  double min_val;        /**< Minimum value */
  int exclusive_min;     /**< 1 if exclusive minimum */
  int has_max;           /**< 1 if maximum constraint exists */
  double max_val;        /**< Maximum value */
  int exclusive_max;     /**< 1 if exclusive maximum */
  int has_min_len;       /**< 1 if minLength constraint exists */
  size_t min_len;        /**< Minimum length for strings */
  int has_max_len;       /**< 1 if maxLength constraint exists */
  size_t max_len;        /**< Maximum length for strings */
  char pattern[256];     /**< Regex pattern string */
  char format[32];       /**< Optional JSON Schema format (e.g. "uuid") */
  char description[256]; /**< Optional field description */
  int deprecated;        /**< 1 if deprecated=true */
  int deprecated_set;    /**< 1 if deprecated explicitly set */
  int read_only;         /**< 1 if readOnly=true */
  int read_only_set;     /**< 1 if readOnly explicitly set */
  int write_only;        /**< 1 if writeOnly=true */
  int write_only_set;    /**< 1 if writeOnly explicitly set */
  int has_min_items;     /**< 1 if minItems constraint exists */
  size_t min_items;      /**< Minimum items for arrays */
  int has_max_items;     /**< 1 if maxItems constraint exists */
  size_t max_items;      /**< Maximum items for arrays */
  int unique_items;      /**< 1 if uniqueItems constraint exists */
  int required;          /**< 1 if required in schema, 0 optional */

  /* C Type Properties */
  int is_flexible_array; /**< 1 if field is a Flexible Array Member `type
                            name[]`, 0 otherwise */
  char bit_width[16]; /**< Bit-field width literal (e.g. "3", "8"), or empty if
                         not a bit-field */
};

/**
 * @brief Union variant JSON type (for oneOf/anyOf codegen).
 */
enum UnionVariantJsonType {
  UNION_JSON_UNKNOWN = 0,
  UNION_JSON_OBJECT,
  UNION_JSON_STRING,
  UNION_JSON_INTEGER,
  UNION_JSON_NUMBER,
  UNION_JSON_BOOLEAN,
  UNION_JSON_ARRAY,
  UNION_JSON_NULL
};

/**
 * @brief Metadata for a union variant.
 */
struct UnionVariantMeta {
  enum UnionVariantJsonType json_type; /**< Expected JSON value type */
  char **required_props;               /**< Required property names */
  size_t n_required_props;             /**< Count of required properties */
  char **property_names;               /**< Defined property names */
  size_t n_property_names;             /**< Count of properties */
  char *disc_value; /**< Discriminator value for this variant */
};

/**
 * @brief Container for fields of a struct.
 */
struct StructFields {
  size_t capacity;                 /**< allocated capacity */
  size_t size;                     /**< number of fields */
  struct StructField *fields;      /**< dynamic array of fields */
  int is_enum;                     /**< 1 if schema is an enum */
  struct EnumMembers enum_members; /**< Enum values when is_enum=1 */
  char *schema_extra_json; /**< Serialized JSON for extra schema keywords */
  int is_union;            /**< 1 if schema represents a union (oneOf/anyOf) */
  int union_is_anyof;      /**< 1 if union came from anyOf (else oneOf) */
  char *union_discriminator; /**< Discriminator property name, if any */
  struct UnionVariantMeta *union_variants; /**< Per-variant metadata */
  size_t n_union_variants;                 /**< Count of union variants */
};

/**
 * @brief Configuration options for struct code generation.
 */
struct CodegenStructConfig {
  /**
   * @brief Macro name to guard generated functions (e.g. "DATA_UTILS").
   * If NULL, no #ifdef/#endif block is generated.
   */
  const char *guard_macro;
};

/**
 * @brief Initialize a StructFields container.
 *
 * @param[out] sf Pointer to container.
 * @return 0 on success, EINVAL if NULL, ENOMEM if alloc fails.
 */
extern C_CDD_EXPORT int struct_fields_init(struct StructFields *sf);

/**
 * @brief Free memory within a StructFields container.
 *
 * @param[in] sf Pointer to container.
 */
extern C_CDD_EXPORT void struct_fields_free(struct StructFields *sf);

/**
 * @brief Add a field to the container.
 *
 * @param[in,out] sf Pointer to the container.
 * @param[in] name Field name.
 * @param[in] type Field type ("integer", "string", "object", etc.)
 * @param[in] ref Reference type (nullable).
 * @param[in] default_val Default value literal (nullable).
 * @param[in] bit_width Bit-field width literal (nullable, e.g. "3").
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT int struct_fields_add(struct StructFields *sf,
                                          const char *name, const char *type,
                                          const char *ref,
                                          const char *default_val,
                                          const char *bit_width);

/**
 * @brief Search for a field by name.
 *
 * @param[in] sf Pointer to the container.
 * @param[in] name Field name to find.
 * @return Pointer to the field if found, NULL otherwise.
 */
extern C_CDD_EXPORT struct StructField *
struct_fields_get(const struct StructFields *sf, const char *name);

/* --- Generator Functions --- */

/**
 * @brief Generate the `_cleanup` function.
 * Frees memory recursively.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_cleanup_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenStructConfig *config);

/**
 * @brief Generate the `_deepcopy` function.
 * Creates an independent copy of the struct.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_deepcopy_func(FILE *fp, const char *struct_name,
                           const struct StructFields *sf,
                           const struct CodegenStructConfig *config);

/**
 * @brief Generate the `_eq` function.
 * Performs deep equality checking.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_eq_func(FILE *fp, const char *struct_name,
                     const struct StructFields *sf,
                     const struct CodegenStructConfig *config);

/**
 * @brief Generate the `_default` function.
 * Allocates and initializes the struct with default values.
 * Handles `nullptr` and `0b` binary literals by checking `numeric_parser`
 * logic.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_default_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenStructConfig *config);

/**
 * @brief Generate the `_debug` function.
 * Prints struct contents for debugging.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_debug_func(FILE *fp, const char *struct_name,
                        const struct StructFields *sf,
                        const struct CodegenStructConfig *config);

/**
 * @brief Generate the `_display` function.
 * Wrapper around JSON serialization for printing.
 *
 * @param[in] fp File pointer to write to.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_display_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenStructConfig *config);

/**
 * @brief Helper to extract type name from a reference path.
 *
 * @param[in] ref Reference string (e.g. "#/components/schemas/Type").
 * @return The base type name ("Type") or the input string if no slash.
 */
extern C_CDD_EXPORT const char *get_type_from_ref(const char *ref);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_STRUCT_H */
