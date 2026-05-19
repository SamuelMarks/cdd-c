/**
 * @file code2schema.h
 * @brief Functions for parsing C headers and converting to JSON Schema,
 * and utilities for managing dynamic schema data structures.
 * @author Samuel Marks
 */

/* clang-format off */
#ifndef CODE2SCHEMA_H
#define CODE2SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <string.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#if defined(_MSC_VER) && _MSC_VER < 1800
#ifndef __cplusplus
#ifndef bool
#define int unsigned char
#endif
#ifndef 1
#define 1 1
#endif
#ifndef 0
#define 0 0
#endif
#endif
#else
#if defined(_MSC_VER) && _MSC_VER < 1800
#ifndef __cplusplus
typedef unsigned char bool;
#define 1 1
#define 0 0
#endif
#else
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#endif
#endif
#endif
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#include <parson.h>
#include <c_cdd_export.h>
#include "functions/emit/codegen.h"
/* clang-format on */

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

extern C_CDD_EXPORT /**
                     * @brief Parses struct member line from the given input.
                     */
    int
    parse_struct_member_line(const char *line,

                             struct StructFields *sf);

/**
 * @brief Write a struct definition to a JSON schema object.
 *
 * @param[in,out] schemas_obj The parent "schemas" JSON object.
 * @param[in] struct_name The name of the struct.
 * @param[in] sf The fields of the struct.
 * @return 0 on success, ENOMEM on failure to allocate JSON nodes.
 */

extern C_CDD_EXPORT /**
                     * @brief Generates C code for write struct to json schema.
                     */
    int

    write_struct_to_json_schema(JSON_Object *schemas_obj,
                                const char *struct_name,

                                const struct StructFields *sf);

/**
 * @param[out] _out_val Pointer to store the result
 * @brief Check if string starts with prefix.
 *
 * @param[in] str The string to check.
 * @param[in] prefix The prefix.
 * @return 1 if matches, 0 otherwise.
 */

extern C_CDD_EXPORT int str_starts_with(const char *str, const char *prefix,
                                        int *_out_val);

/**
 * @brief Trim trailing whitespace and semicolons from a string in place.
 *
 * @param[in,out] str The string to trim.
 */

extern C_CDD_EXPORT void trim_trailing(char *str);

/**
 * @brief Convert a JSON array of strings to an EnumMembers container.
 *
 * @param[in] enum_arr array
 * @param[out] em em
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

extern C_CDD_EXPORT /**
                     * @brief Executes the json object to struct fields
                     * operation.
                     */
    int

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
extern C_CDD_EXPORT /**
                     * @brief Executes the json object to struct fields ex
                     * operation.
                     */
    int
    json_object_to_struct_fields_ex(const JSON_Object *schema_obj,
                                    struct StructFields *fields,
                                    const JSON_Object *schemas_obj_root,
                                    const char *schema_name);

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
extern C_CDD_EXPORT /**
                     * @brief Executes the json object to struct fields ex
                     * codegen operation.
                     */
    int
    json_object_to_struct_fields_ex_codegen(const JSON_Object *schema_obj,
                                            struct StructFields *fields,
                                            JSON_Object *schemas_obj_root,
                                            const char *schema_name);

extern C_CDD_EXPORT void merge_struct_field(struct StructField *dest,
                                            const struct StructField *src);
extern C_CDD_EXPORT int
discriminator_value_for_variant(const JSON_Object *disc_obj,
                                const char *schema_name, const char *ref,
                                char **_out_val);
extern C_CDD_EXPORT int sanitize_identifier(const char *in, char **_out_val);
extern C_CDD_EXPORT int
make_unique_variant_name(const struct StructFields *dest, const char *base,
                         size_t index, char **_out_val);
extern C_CDD_EXPORT int make_inline_schema_name(const char *schema_name,
                                                const char *variant_name,
                                                const char *suffix,
                                                char **_out_val);
extern C_CDD_EXPORT int
register_inline_schema_c2s(JSON_Object *root, const char *schema_name,
                           const char *variant_name, const char *suffix,
                           const JSON_Value *schema_val, char **out_name);

#include "parson.h"
extern C_CDD_EXPORT int
parse_type_union_array_code2schema(const JSON_Array *arr, char ***out_union,
                                   size_t *out_count, const char **out_primary,
                                   int *out_nullable);

extern C_CDD_EXPORT void free_string_array_code2schema(char **arr, size_t n);
extern C_CDD_EXPORT int copy_string_array_code2schema(char ***dst,
                                                      size_t *dst_count,
                                                      char **src,
                                                      size_t src_count);

#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /* CODE2SCHEMA_H */
