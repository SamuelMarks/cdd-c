/**
 * @file codegen_types.h
 * @brief Generator for Advanced Types: Tagged Unions and Root Arrays.
 *
 * Handles polymorphism (oneOf) via discriminated unions and direct array types.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_TYPES_H
#define C_CDD_CODEGEN_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include "c_cdd_export.h"
#include "classes/emit_struct.h" /* Reuses StructFields */

/**
 * @brief Configuration for advanced type generation.
 */
struct CodegenTypesConfig {
  /**
   * @brief Guard macro for JSON functions (e.g. "ENABLE_JSON").
   */
  const char *json_guard;
  /**
   * @brief Guard macro for utility functions (e.g. "DATA_UTILS").
   */
  const char *utils_guard;
};

/* --- Union Functions (Polymorphism) --- */

/**
 * @brief Generate `_to_json` for a Tagged Union.
 * Emits a switch-statement based serializer using the `tag` field.
 *
 * @param[in] fp Output stream.
 * @param[in] union_name Name of the union wrapper struct.
 * @param[in] sf Fields representing the union options (each field == one
 * option).
 * @param[in] config Optional config for guards.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_union_to_json_func(FILE *fp, const char *union_name,
                         const struct StructFields *sf,
                         const struct CodegenTypesConfig *config);

/**
 * @brief Generate `_from_jsonObject` for a Tagged Union.
 * Emits logic to select an object variant using a discriminator or
 * schema-derived property hints and set the `tag` accordingly.
 *
 * @param[in] fp Output stream.
 * @param[in] union_name Name of the union wrapper struct.
 * @param[in] sf Fields representing the options.
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_union_from_jsonObject_func(FILE *fp, const char *union_name,
                                 const struct StructFields *sf,
                                 const struct CodegenTypesConfig *config);

/**
 * @brief Generate `_from_json` for a Tagged Union.
 * Parses a JSON string to a JSON value and dispatches to object/primitive
 * handlers based on the union variant metadata.
 *
 * @param[in] fp Output stream.
 * @param[in] union_name Name of the union wrapper struct.
 * @param[in] sf Fields representing the options.
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_union_from_json_func(FILE *fp, const char *union_name,
                           const struct StructFields *sf,
                           const struct CodegenTypesConfig *config);

/**
 * @brief Generate `_cleanup` for a Tagged Union.
 * Emits a switch-statement based free logic.
 *
 * @param[in] fp Output stream.
 * @param[in] union_name Name of the union wrapper struct.
 * @param[in] sf Fields representing the options.
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_union_cleanup_func(FILE *fp, const char *union_name,
                         const struct StructFields *sf,
                         const struct CodegenTypesConfig *config);

/* --- Root Array Functions --- */

/**
 * @brief Generate `_to_json` for a root array type.
 *
 * Serializes a raw array (pointer + length) into a JSON array string `[...]`.
 *
 * @param[in] fp Output stream.
 * @param[in] name The base name of the type.
 * @param[in] item_type Logical item type ("integer", "string", "object").
 * @param[in] item_ref Reference/inner type name.
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_root_array_to_json_func(FILE *fp, const char *name, const char *item_type,
                              const char *item_ref,
                              const struct CodegenTypesConfig *config);

/**
 * @brief Generate `_from_json` for a root array type.
 *
 * Generates parsing logic to read a top-level JSON array into a C array.
 * Signature: `int Name_from_json(const char *json, Type **out, size_t *len)`
 *
 * @param[in] fp Output stream.
 * @param[in] name The base name of the type.
 * @param[in] item_type Logical item type.
 * @param[in] item_ref Reference/inner type name.
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_root_array_from_json_func(FILE *fp, const char *name,
                                const char *item_type, const char *item_ref,
                                const struct CodegenTypesConfig *config);

/**
 * @brief Generate `_cleanup` for a root array type.
 *
 * @param[in] fp Output stream.
 * @param[in] name The base name of the type.
 * @param[in] item_type Logical item type.
 * @param[in] item_ref Reference/inner type name.
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_root_array_cleanup_func(FILE *fp, const char *name, const char *item_type,
                              const char *item_ref,
                              const struct CodegenTypesConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_TYPES_H */
