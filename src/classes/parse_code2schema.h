/**
 * @file code2schema.h
 * @brief Functions for parsing C headers and converting to JSON Schema,
 * and utilities for managing dynamic schema data structures.
 * @author Samuel Marks
 */

#ifndef CODE2SCHEMA_H

#define CODE2SCHEMA_H

#ifdef __cplusplus

#include <cstdlib>

#include <cstring>

extern "C" {

#else

#include <stdlib.h>

#include <string.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

#include <stdbool.h>

#else

#include <c_cdd_stdbool.h>

#endif /* __STDC_VERSION__ >= 199901L */

#endif /* __cplusplus */

#include <parson.h>

#include <c_cdd_export.h>

#include "functions/emit_codegen.h"

/**
 * @brief CLI entry point for code2schema command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return 0 on success, EXIT_FAILURE on error.
 */

extern C_CDD_EXPORT int code2schema_main(int argc, char **argv);

/**
 * @brief Parse a line of C code declaring a struct member.
 * Extracts name and type and adds to StructFields.
 *
 * @param[in] line The line of code code to parse.
 * @param[out] sf The container to add the field to.
 * @return 0 on success (including ignored lines), ENOMEM on allocation failure.
 */

extern C_CDD_EXPORT int parse_struct_member_line(const char *line,

                                                 struct StructFields *sf);

/**
 * @brief Write a struct definition to a JSON schema object.
 *
 * @param[in,out] schemas_obj The parent "schemas" JSON object.
 * @param[in] struct_name The name of the struct.
 * @param[in] sf The fields of the struct.
 * @return 0 on success, ENOMEM on failure to allocate JSON nodes.
 */

extern C_CDD_EXPORT int

write_struct_to_json_schema(JSON_Object *schemas_obj, const char *struct_name,

                            const struct StructFields *sf);

/**
 * @brief Check if string starts with prefix.
 *
 * @param[in] str The string to check.
 * @param[in] prefix The prefix.
 * @return true if matches, false otherwise.
 */

extern C_CDD_EXPORT bool str_starts_with(const char *str, const char *prefix);

/**
 * @brief Trim trailing whitespace and semicolons from a string in place.
 *
 * @param[in,out] str The string to trim.
 */

extern C_CDD_EXPORT void trim_trailing(char *str);

/**
 * @brief Convert a JSON array of strings to an EnumMembers container.
 *
 * @param[in] enum_arr JSON array of strings.
 * @param[out] em The container to populate.
 * @return 0 on success, non-zero on failure.
 */

extern C_CDD_EXPORT int json_array_to_enum_members(const JSON_Array *enum_arr,

                                                   struct EnumMembers *em);

/**
 * @brief Convert a JSON schema object properties to StructFields container.
 * Handles $ref resolution if schemas_obj_root is provided.
 *
 * @param[in] schema_obj The JSON object representing the struct schema.
 * @param[out] fields The container to populate.
 * @param[in] schemas_obj_root Optional root schema object for resolving $ref
 * types.
 * @return 0 on success, non-zero on failure.
 */

extern C_CDD_EXPORT int

json_object_to_struct_fields(const JSON_Object *schema_obj,

                             struct StructFields *fields,

                             const JSON_Object *schemas_obj_root);

/**
 * @brief Convert a JSON schema object properties to StructFields container.
 * Supports union extraction for oneOf/anyOf when possible.
 *
 * @param[in] schema_obj The JSON object representing the schema.
 * @param[out] fields The container to populate.
 * @param[in] schemas_obj_root Optional root schema object for resolving $ref.
 * @param[in] schema_name Optional schema name for generated variant names.
 * @return 0 on success, non-zero on failure.
 */
extern C_CDD_EXPORT int json_object_to_struct_fields_ex(
    const JSON_Object *schema_obj, struct StructFields *fields,
    const JSON_Object *schemas_obj_root, const char *schema_name);

/**
 * @brief Convert a JSON schema object properties to StructFields container for
 *        code generation.
 * Supports union extraction for oneOf/anyOf including inline object/array
 * variants by promoting them to synthetic component schemas.
 *
 * @param[in] schema_obj The JSON object representing the schema.
 * @param[out] fields The container to populate.
 * @param[in,out] schemas_obj_root Root schema object for resolving $ref and
 *                                 registering synthetic components.
 * @param[in] schema_name Optional schema name for generated variant names.
 * @return 0 on success, non-zero on failure.
 */
extern C_CDD_EXPORT int json_object_to_struct_fields_ex_codegen(
    const JSON_Object *schema_obj, struct StructFields *fields,
    JSON_Object *schemas_obj_root, const char *schema_name);

#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /* CODE2SCHEMA_H */
