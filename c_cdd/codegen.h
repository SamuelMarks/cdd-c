/**
 * @file codegen.h
 * @brief Code generation utility functions for C structs, enums, and unions.
 *
 * Provides primitives for generating C implementations of serialization,
 * deserialization, memory management, and debugging functions.
 * Includes support for Forward Declarations, Arrays, and Tagged Unions.
 *
 * @author Samuel Marks
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include <c_cdd_export.h>

/**
 * @brief Configuration options for code generation.
 * Used to inject build-time guards (macros) around generated code.
 */
struct CodegenConfig {
  /**
   * @brief Macro name to guard enum conversion functions (e.g. "TO_ENUM").
   * If NULL, no #ifdef/#endif block is generated for enums.
   */
  const char *enum_guard;

  /**
   * @brief Macro name to guard JSON serialization functions (e.g. "TO_JSON").
   * Applied to _to_json, _from_json, and _from_jsonObject functions.
   * If NULL, no #ifdef/#endif block is generated for JSON functions.
   */
  const char *json_guard;

  /**
   * @brief Macro name to guard Utility functions (e.g. "DATA_UTILS").
   * Applied to _cleanup, _deep_copy, _eq, _debug, _default, _display.
   * If NULL, no #ifdef/#endif block is generated for Utility functions.
   */
  const char *utils_guard;
};

/**
 * @brief Container for enum members extracted from code or schema.
 */
struct EnumMembers {
  size_t capacity; /**< Allocated capacity of members array */
  size_t size;     /**< Number of members currently stored */
  char **members;  /**< Dynamic array of member names */
};

/**
 * @brief Represents a single field within a struct or union.
 */
struct StructField {
  char name[64];        /**< Field identifier */
  char type[64];        /**< C or Logical Field type (e.g. "string", "integer",
                           "object", "enum", "array") */
  char ref[64];         /**< Reference type name or item type */
  char default_val[64]; /**< Default value literal (e.g. "5", "\"foo\"") or
                           empty */

  /* Numeric Constraints */
  int has_min;       /**< 1 if minimum constraint exists */
  double min_val;    /**< The minimum value */
  int exclusive_min; /**< 1 if minimum is exclusive ( > ), 0 if inclusive ( >= )
                      */

  int has_max;       /**< 1 if maximum constraint exists */
  double max_val;    /**< The maximum value */
  int exclusive_max; /**< 1 if maximum is exclusive ( < ), 0 if inclusive ( <= )
                      */

  /* String Constraints */
  int has_min_len;   /**< 1 if minLength constraint exists */
  size_t min_len;    /**< The minLength value */
  int has_max_len;   /**< 1 if maxLength constraint exists */
  size_t max_len;    /**< The maxLength value */
  char pattern[128]; /**< Regex pattern string (or empty) */
};

/**
 * @brief Container for fields of a struct or union.
 */
struct StructFields {
  size_t capacity;            /**< allocated capacity */
  size_t size;                /**< number of fields */
  struct StructField *fields; /**< dynamic array of fields */
};

/* --- Header Generation Primitives --- */

/**
 * @brief Write a forward declaration for a struct.
 *
 * Generates `typedef struct name name;` or `struct name;` to allow circular
 * pointer references.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct to declare.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int write_forward_decl(FILE *fp, const char *struct_name);

/* --- Enum Functions --- */

/**
 * @brief Write `_from_str` implementation for an enum.
 * @param[in] fp Output file stream.
 * @param[in] enum_name Name of the C enum.
 * @param[in] em Container of enum member strings.
 * @param[in] config Optional configuration for guards (can be NULL).
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_enum_from_str_func(FILE *fp, const char *enum_name,
                         const struct EnumMembers *em,
                         const struct CodegenConfig *config);

/**
 * @brief Write `_to_str` implementation for an enum.
 * @param[in] fp Output file stream.
 * @param[in] enum_name Name of the C enum.
 * @param[in] em Container of enum member strings.
 * @param[in] config Optional configuration for guards (can be NULL).
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_enum_to_str_func(FILE *fp, const char *enum_name,
                       const struct EnumMembers *em,
                       const struct CodegenConfig *config);

/* --- Struct Functions --- */

/**
 * @brief Helper to initialize StructFields container.
 * @param[out] sf Pointer to container.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int struct_fields_init(struct StructFields *sf);

/**
 * @brief Helper to free StructFields container.
 * @param[in] sf Pointer to container.
 */
void struct_fields_free(struct StructFields *sf);

/**
 * @brief Add a field to the StructFields container.
 * Resizes internal storage if necessary.
 *
 * @param[in,out] sf Pointer to the container.
 * @param[in] name Name of the field.
 * @param[in] type Type of the field (e.g. "string", "integer").
 * @param[in] ref Reference type name if applicable, or NULL.
 * @param[in] default_val Default value literal (already formatted/quoted), or
 * NULL.
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT int struct_fields_add(struct StructFields *sf,
                                          const char *name, const char *type,
                                          const char *ref,
                                          const char *default_val);

/**
 * @brief Write `_cleanup` implementation for a struct.
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_cleanup_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenConfig *config);

/**
 * @brief Write `_debug` implementation for a struct.
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_debug_func(FILE *fp, const char *struct_name,
                        const struct StructFields *sf,
                        const struct CodegenConfig *config);

/**
 * @brief Write `_deepcopy` implementation for a struct.
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_deepcopy_func(FILE *fp, const char *struct_name,
                           const struct StructFields *sf,
                           const struct CodegenConfig *config);

/**
 * @brief Write `_default` implementation for a struct.
 * Populates fields with default values defined in schema if present.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_default_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenConfig *config);

/**
 * @brief Write `_display` implementation for a struct.
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_display_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenConfig *config);

/**
 * @brief Write `_eq` implementation for a struct.
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_eq_func(FILE *fp, const char *struct_name,
                     const struct StructFields *sf,
                     const struct CodegenConfig *config);

/**
 * @brief Write `_from_jsonObject` implementation for a struct.
 * Generates numeric validation logic (min/max) returning ERANGE on failure.
 * Generates string validation logic (len/pattern) returning ERANGE/EINVAL on
 * failure.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_from_jsonObject_func(FILE *fp, const char *struct_name,
                                  const struct StructFields *sf,
                                  const struct CodegenConfig *config);

/**
 * @brief Write `_from_json` wrapper implementation for a struct.
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_from_json_func(FILE *fp, const char *struct_name,
                            const struct CodegenConfig *config);

/**
 * @brief Write `_to_json` implementation for a struct.
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_to_json_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenConfig *config);

/* --- Union Functions (Polymorphism) --- */

/**
 * @brief Write `_to_json` implementation for a Tagged Union.
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union wrapper struct.
 * @param[in] sf Fields representing the union options.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_union_to_json_func(FILE *fp, const char *union_name,
                         const struct StructFields *sf,
                         const struct CodegenConfig *config);

/**
 * @brief Write `_from_jsonObject` implementation for a Tagged Union.
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union wrapper struct.
 * @param[in] sf Fields representing the union options.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_union_from_jsonObject_func(FILE *fp, const char *union_name,
                                 const struct StructFields *sf,
                                 const struct CodegenConfig *config);

/**
 * @brief Write `_cleanup` implementation for a Tagged Union.
 * @param[in] fp Output file stream.
 * @param[in] union_name Name of the union wrapper struct.
 * @param[in] sf Fields.
 * @param[in] config Optional configuration for guards.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_union_cleanup_func(FILE *fp, const char *union_name,
                         const struct StructFields *sf,
                         const struct CodegenConfig *config);

/* --- Root Array Marshalling Functions --- */

/**
 * @brief Write `_from_json` for a root array type.
 * Generates logic to parse a top-level JSON array into a C array.
 *
 * @param[in] fp Output file stream.
 * @param[in] name The base name of the type (e.g. "IntList").
 * @param[in] item_type The logical type of items (e.g. "integer", "string",
 * "object").
 * @param[in] item_ref The reference/inner type name (e.g. "struct Wrapper").
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_root_array_from_json_func(FILE *fp, const char *name,
                                const char *item_type, const char *item_ref,
                                const struct CodegenConfig *config);

/**
 * @brief Write `_to_json` for a root array type.
 * @param[in] fp Output file stream.
 * @param[in] name The base name of the type.
 * @param[in] item_type Logical item type.
 * @param[in] item_ref Reference/inner type name.
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_root_array_to_json_func(FILE *fp, const char *name, const char *item_type,
                              const char *item_ref,
                              const struct CodegenConfig *config);

/**
 * @brief Write `_cleanup` for a root array type.
 * Needed for complex types (strings, objects) that require deep freeing.
 *
 * @param[in] fp Output file stream.
 * @param[in] name The base name of the type.
 * @param[in] item_type Logical item type.
 * @param[in] item_ref Reference/inner type name.
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_root_array_cleanup_func(FILE *fp, const char *name, const char *item_type,
                              const char *item_ref,
                              const struct CodegenConfig *config);

/* helpers */

/**
 * @brief Extract type name from reference.
 * @param[in] ref Valid reference string.
 * @return Start of type name.
 */
extern C_CDD_EXPORT const char *get_type_from_ref(const char *ref);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CODEGEN_H */
