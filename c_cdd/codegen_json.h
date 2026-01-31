/**
 * @file codegen_json.h
 * @brief JSON Serialization Generator.
 *
 * Provides functionality to generate C functions that convert Structs to JSON
 * strings and parse JSON strings into Structs. Uses the `parson` library for
 * parsing and `jasprintf` (safe append) pattern for emission.
 *
 * This module replaces the JSON-specific portions of the monolithic
 * `codegen.c`.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_JSON_H
#define C_CDD_CODEGEN_JSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include "c_cdd_export.h"
#include "codegen_struct.h" /* Reuses StructFields definition */

/**
 * @brief Configuration for JSON generation.
 * Allows optional build guards (e.g. #ifdef ENABLE_JSON).
 */
struct CodegenJsonConfig {
  /**
   * @brief Macro name to guard generated functions.
   * If NULL, no guards are emitted.
   */
  const char *guard_macro;
};

/**
 * @brief Generate `_to_json` implementation.
 *
 * Generates serialization logic:
 * `int Struct_to_json(const struct Struct *obj, char **out);`
 *
 * @param[in] fp Output stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Optional config.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
write_struct_to_json_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenJsonConfig *config);

/**
 * @brief Generate `_from_json` implementation (Wrapper).
 *
 * Generates the top-level string parser wrapper:
 * `int Struct_from_json(const char *json, struct Struct **out);`
 *
 * @param[in] fp Output stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_from_json_func(FILE *fp, const char *struct_name,
                            const struct CodegenJsonConfig *config);

/**
 * @brief Generate `_from_jsonObject` implementation (Core Logic).
 *
 * Generates the recursive parser that reads from a `JSON_Object`:
 * `int Struct_from_jsonObject(const JSON_Object *obj, struct Struct **out);`
 * Includes validation logic (min/max/regex) if constraints are present in
 * fields.
 *
 * @param[in] fp Output stream.
 * @param[in] struct_name Name of the struct.
 * @param[in] sf Fields descriptor.
 * @param[in] config Optional config.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int
write_struct_from_jsonObject_func(FILE *fp, const char *struct_name,
                                  const struct StructFields *sf,
                                  const struct CodegenJsonConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_JSON_H */
