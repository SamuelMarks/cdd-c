/**
 * @file code2schema.h
 * @brief Functions for parsing C headers and converting to JSON Schema,
 * and utilities for managing dynamic schema data structures.
 * @author Samuel Marks
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "cdd_c_error.h"
#include <c_cdd_stdbool.h>
#include <parson.h>
#include <c_cdd_export.h>
#include "classes/emit/struct.h"
#include "functions/emit/codegen.h"
/* clang-format on */

#ifndef CODE2SCHEMA_H
#define CODE2SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief CLI entry point for code2schema command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return 0 on success, EXIT_FAILURE on error.
 */
extern C_CDD_EXPORT cdd_c_error_t code2schema_main(int argc, char **argv);

/**
 * @brief Parse a line of C code declaring a struct member.
 * Extracts name and type and adds to StructFields.
 *
 * @param[in] line The line of code code to parse.
 * @param[out] sf The container to add the field to.
 * @return 0 on success (including ignored lines), ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_struct_member_line(const char *line, struct StructFields *sf);

/**
 * @brief Write a struct definition to a JSON schema object.
 *
 * @param[in,out] schemas_obj The parent "schemas" JSON object.
 * @param[in] struct_name The name of the struct.
 * @param[in] sf The fields of the struct.
 * @return 0 on success, ENOMEM on failure to allocate JSON nodes.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_struct_to_json_schema(JSON_Object *schemas_obj, const char *struct_name,
                            const struct StructFields *sf);

/**
 * @brief Check if string starts with prefix.
 *
 * @param[in] str The string to check.
 * @param[in] prefix The prefix.
 * @param[out] _out_val Pointer to store the result (1 if matches, 0 otherwise).
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t str_starts_with(const char *str,
                                                  const char *prefix,
                                                  int *_out_val);

/**
 * @brief Trim trailing whitespace and semicolons from a string in place.
 *
 * @param[in,out] str The string to trim.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t trim_trailing(char *str);

/**
 * @brief Convert a JSON array of strings to an EnumMembers container.
 *
 * @param[in] enum_arr array
 * @param[out] em em
 * @return 0 on success, non-zero on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
json_array_to_enum_members(const JSON_Array *enum_arr, struct EnumMembers *em);

/**
 * @brief Convert a JSON schema object properties to StructFields container.
 * Handles reference resolution if schemas_obj_root is provided.
 *
 * @param[in] schema_obj The JSON object representing the struct schema.
 * @param[out] fields The container to populate.
 * @param[in] schemas_obj_root Optional root schema object for resolving
 * reference types.
 * @return 0 on success, non-zero on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t json_object_to_struct_fields(
    const JSON_Object *schema_obj, struct StructFields *fields,
    const JSON_Object *schemas_obj_root);

/**
 * @brief Convert a JSON schema object properties to StructFields container.
 * Supports union extraction for oneOf/anyOf when possible.
 *
 * @param[in] schema_obj The JSON object representing the schema.
 * @param[out] fields The container to populate.
 * @param[in] schemas_obj_root Optional root schema object for resolving
 * references.
 * @param[in] schema_name Optional schema name for generated variant names.
 * @return 0 on success, non-zero on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t json_object_to_struct_fields_ex(
    const JSON_Object *schema_obj, struct StructFields *fields,
    const JSON_Object *schemas_obj_root, const char *schema_name);

/**
 * @brief Convert a JSON schema object properties to StructFields container for
 * code generation. Supports union extraction for oneOf/anyOf including inline
 * object/array variants by promoting them to synthetic component schemas.
 *
 * @param[in] schema_obj The JSON object representing the schema.
 * @param[out] fields The container to populate.
 * @param[in,out] schemas_obj_root Root schema object for resolving
 * references and registering synthetic components.
 * @param[in] schema_name Optional schema name for generated variant names.
 * @return 0 on success, non-zero on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t json_object_to_struct_fields_ex_codegen(
    const JSON_Object *schema_obj, struct StructFields *fields,
    JSON_Object *schemas_obj_root, const char *schema_name);

/**
 * @brief Merges a source struct field into a destination struct field.
 *
 * @param[in,out] dest Destination struct field.
 * @param[in] src Source struct field.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
merge_struct_field(struct StructField *dest, const struct StructField *src);

/**
 * @brief Retrieves the discriminator mapping value for a given schema variant.
 *
 * @param[in] disc_obj Discriminator JSON object.
 * @param[in] schema_name Schema name.
 * @param[in] ref Reference string.
 * @param[out] _out_val Pointer receiving allocated discriminator value.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t discriminator_value_for_variant(
    const JSON_Object *disc_obj, const char *schema_name, const char *ref,
    char **_out_val);

/**
 * @brief Sanitizes a string to be used as a valid C identifier.
 *
 * @param[in] in Input string.
 * @param[out] _out_val Pointer receiving allocated sanitized string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t sanitize_identifier(const char *in,
                                                      char **_out_val);

/**
 * @brief Generates a unique variant name for a union type.
 *
 * @param[in] dest Destination struct fields container.
 * @param[in] base Base identifier.
 * @param[in] index Variant index.
 * @param[out] _out_val Pointer receiving unique allocated variant name.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
make_unique_variant_name(const struct StructFields *dest, const char *base,
                         size_t index, char **_out_val);

/**
 * @brief Generates a schema name for an inline anonymous struct/union.
 *
 * @param[in] schema_name Base schema name.
 * @param[in] variant_name Variant name.
 * @param[in] suffix Optional suffix string.
 * @param[out] _out_val Pointer receiving allocated inline schema name.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
make_inline_schema_name(const char *schema_name, const char *variant_name,
                        const char *suffix, char **_out_val);

/**
 * @brief Registers an inline schema within the root OpenAPI components.
 *
 * @param[in,out] root Root components/schemas JSON object.
 * @param[in] schema_name Base schema name.
 * @param[in] variant_name Variant name.
 * @param[in] suffix Optional suffix string.
 * @param[in] schema_val JSON value of inline schema.
 * @param[out] out_name Pointer receiving registered schema name.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t register_inline_schema_c2s(
    JSON_Object *root, const char *schema_name, const char *variant_name,
    const char *suffix, const JSON_Value *schema_val, char **out_name);

/**
 * @brief Parses a type union array into an array of string type names.
 *
 * @param[in] arr JSON array of type strings.
 * @param[out] out_union Pointer receiving allocated array of type strings.
 * @param[out] out_count Pointer receiving count of types in union.
 * @param[out] out_primary Pointer receiving primary type string.
 * @param[out] out_nullable Pointer receiving 1 if null is present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_type_union_array_code2schema(
    const JSON_Array *arr, char ***out_union, size_t *out_count,
    const char **out_primary, int *out_nullable);

/**
 * @brief Safely frees an array of dynamically allocated string pointers.
 *
 * @param[in,out] arr The string array to free.
 * @param[in] n The number of initialized elements in the array.
 */
extern C_CDD_EXPORT void free_string_array_code2schema(char **arr, size_t n);

/**
 * @brief Deep copies an array of string pointers into a new allocated array.
 *
 * @param[out] dst Pointer to receive allocated destination array.
 * @param[out] dst_count Pointer to receive element count.
 * @param[in] src Array to copy elements from.
 * @param[in] src_count Number of elements in src.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_string_array_code2schema(
    char ***dst, size_t *dst_count, char **src, size_t src_count);

/**
 * @brief Reads a line from a file pointer and strips trailing newlines.
 *
 * @param[in] fp The file pointer to read from.
 * @param[out] buf The buffer to store the read string.
 * @param[in] bufsz The size of the buffer.
 * @param[out] has_line Pointer to int receiving 1 if a line was read, 0 on
 * EOF/error.
 * @return CDD_C_SUCCESS on success or EOF, or error code on invalid argument.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_read_line(FILE *fp, char *buf,
                                                size_t bufsz, int *has_line);

/**
 * @brief Checks if a given key exists in an array of strings.
 *
 * @param[in] key The string to look for.
 * @param[in] list A pointer to an array of string pointers.
 * @param[in] count The size of the list array.
 * @param[out] out_found Pointer to int receiving 1 if found, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_key_in_list(const char *key,
                                                  const char **list,
                                                  size_t count, int *out_found);

/**
 * @brief Deep clones a Parson JSON value.
 *
 * @param[in] val The value to clone.
 * @param[out] _out_val Pointer to receive cloned JSON_Value.
 * @return CDD_C_SUCCESS on success, or memory error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_clone_json_value(const JSON_Value *val,
                                                       JSON_Value **_out_val);

/**
 * @brief Collects extra schema attributes not in the skip list.
 *
 * @param[in] obj The JSON object to inspect.
 * @param[in] skip_keys Array of keys to exclude.
 * @param[in] skip_count Number of keys in skip_keys.
 * @param[out] out_json Pointer receiving serialized JSON string of extras.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2s_collect_schema_extras(const JSON_Object *obj, const char **skip_keys,
                          size_t skip_count, char **out_json);

/**
 * @brief Merges extra attributes described by a JSON string into a target
 * Parson JSON Object.
 *
 * @param[in,out] target The JSON_Object to merge properties into.
 * @param[in] extras_json A serialized JSON string of extra properties.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2s_merge_schema_extras_object(JSON_Object *target, const char *extras_json);

/**
 * @brief Merges two serialized JSON object strings.
 *
 * @param[in,out] dest_json Pointer to destination JSON string.
 * @param[in] src_json Source JSON string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2s_merge_schema_extras_strings(char **dest_json, const char *src_json);

/**
 * @brief Checks if an OpenAPI type name is primitive.
 *
 * @param[in] type The type name string.
 * @param[out] out_is_primitive Pointer receiving 1 if primitive, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2s_openapi_type_is_primitive(const char *type, int *out_is_primitive);

/**
 * @brief Strips enclosing quotes from a string.
 *
 * @param[in] in Input string.
 * @param[out] buf Buffer to receive unquoted string.
 * @param[in] bufsz Size of buffer.
 * @param[out] _out_val Pointer receiving stripped string or input string.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_strip_quotes(const char *in, char *buf,
                                                   size_t bufsz,
                                                   const char **_out_val);

/**
 * @brief Parses a boolean default value from a string.
 *
 * @param[in] in Input string.
 * @param[out] out Pointer receiving boolean 1 or 0.
 * @param[out] out_has_val Pointer receiving 1 if parsed, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_parse_bool_default(const char *in,
                                                         int *out,
                                                         int *out_has_val);

/**
 * @brief Parses a numeric default value from a string.
 *
 * @param[in] in Input string.
 * @param[out] out Pointer receiving numeric value.
 * @param[out] out_has_val Pointer receiving 1 if parsed, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_parse_number_default(const char *in,
                                                           double *out,
                                                           int *out_has_val);

/**
 * @brief Detects the underlying JSON type of a union schema object.
 *
 * @param[in] schema_obj The schema object.
 * @param[out] _out_val Pointer receiving detected UnionVariantJsonType.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_detect_union_json_type(
    const JSON_Object *schema_obj, enum UnionVariantJsonType *_out_val);

/**
 * @brief Collects an array of strings from a JSON array.
 *
 * @param[in] arr The JSON array.
 * @param[out] out Pointer receiving allocated string array.
 * @param[out] out_count Pointer receiving number of strings.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2s_collect_string_array(const JSON_Array *arr, char ***out, size_t *out_count);

/**
 * @brief Collects property names from a JSON Schema object.
 *
 * @param[in] schema_obj The schema object.
 * @param[out] out Pointer receiving allocated array of property names.
 * @param[out] out_count Pointer receiving count of property names.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_collect_property_names(
    const JSON_Object *schema_obj, char ***out, size_t *out_count);

/**
 * @brief Checks if array items within a union are supported.
 *
 * @param[in] schema_obj Schema object with items.
 * @param[in] root Root schema object for resolving refs.
 * @param[in] allow_inline Whether inline schemas are allowed.
 * @param[out] out_supported Pointer receiving 1 if supported, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_union_array_items_supported(
    const JSON_Object *schema_obj, const JSON_Object *root, int allow_inline,
    int *out_supported);

/**
 * @brief Writes the default value of a StructField to a JSON schema object.
 *
 * @param[in,out] pobj Target JSON object.
 * @param[in] field Source struct field.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2s_write_default_value(JSON_Object *pobj, const struct StructField *field);

/**
 * @brief Writes numeric constraints of a StructField to a JSON schema object.
 *
 * @param[in,out] pobj Target JSON object.
 * @param[in] field Source struct field.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_write_numeric_constraints(
    JSON_Object *pobj, const struct StructField *field);

/**
 * @brief Writes string constraints of a StructField to a JSON schema object.
 *
 * @param[in,out] pobj Target JSON object.
 * @param[in] field Source struct field.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_write_string_constraints(
    JSON_Object *pobj, const struct StructField *field);

/**
 * @brief Writes array constraints of a StructField to a JSON schema object.
 *
 * @param[in,out] pobj Target JSON object.
 * @param[in] field Source struct field.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
c2s_write_array_constraints(JSON_Object *pobj, const struct StructField *field);

/**
 * @brief Writes a type union array to a JSON schema object.
 *
 * @param[in,out] obj Target JSON object.
 * @param[in] type Primary type name string.
 * @param[in] type_union Array of union type name strings.
 * @param[in] n_type_union Count of types in type_union.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_write_type_union(JSON_Object *obj,
                                                       const char *type,
                                                       char **type_union,
                                                       size_t n_type_union);

/**
 * @brief Parses a union definition and writes it to a JSON Schema object.
 *
 * @param[in] fp File pointer to read from.
 * @param[in,out] schemas_obj Target schemas JSON object.
 * @param[in] union_name Name of the union.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_parse_union_and_write(
    FILE *fp, JSON_Object *schemas_obj, const char *union_name);

/**
 * @brief Collapses pointer + length field pairs into array representations.
 *
 * @param[in,out] sf The StructFields structure to modify.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_collapse_arrays(struct StructFields *sf);

/**
 * @brief Internal conversion from JSON schema object properties to StructFields
 * container.
 *
 * @param[in] o The JSON object representing the schema.
 * @param[out] f The container to populate.
 * @param[in,out] root Optional root schema object for resolving refs.
 * @param[in] schema_name Optional schema name for generated variant names.
 * @param[in] allow_inline_union Whether inline object/array union variants
 * should be promoted.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t c2s_json_object_to_struct_fields_internal(
    const JSON_Object *o, struct StructFields *f, JSON_Object *root,
    const char *schema_name, int allow_inline_union);

/**
 * @brief Determines if a JSON Schema object represents a string enum.
 *
 * @param[in] schema_obj Schema object to check.
 * @param[out] enum_arr_out Optional pointer to receive JSON_Array of enum
 * values.
 * @param[out] out_is_enum Pointer receiving 1 if string enum, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
schema_object_is_string_enum(const JSON_Object *schema_obj,
                             const JSON_Array **enum_arr_out, int *out_is_enum);

/**
 * @brief Checks if a JSON Schema reference points to a string enum.
 *
 * @param[in] root Root schema object.
 * @param[in] ref Reference string.
 * @param[out] out_points_to_enum Pointer receiving 1 if points to enum, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t ref_points_to_string_enum(
    const JSON_Object *root, const char *ref, int *out_points_to_enum);

/**
 * @brief Checks if a given property name is present in a required properties
 * list.
 *
 * @param[in] required Array of required property names.
 * @param[in] name Name to check.
 * @param[out] out_in_list Pointer receiving 1 if in list, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t required_name_in_list(
    const JSON_Array *required, const char *name, int *out_in_list);

/**
 * @brief Resolves a JSON Schema reference to its corresponding object.
 *
 * @param[in] root Root schema object.
 * @param[in] ref Reference string.
 * @param[out] _out_val Pointer receiving resolved JSON_Object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t resolve_schema_ref_object(
    const JSON_Object *root, const char *ref, JSON_Object **_out_val);

/**
 * @brief Merges source struct fields into destination struct fields.
 *
 * @param[in,out] dest Destination StructFields.
 * @param[in] src Source StructFields.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
merge_struct_fields(struct StructFields *dest, const struct StructFields *src);

/**
 * @brief Applies an allOf JSON Schema array to a StructFields object.
 *
 * @param[in] all_of The allOf JSON array.
 * @param[in,out] dest Destination StructFields.
 * @param[in] root Root schema object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t apply_allof_to_struct_fields(
    const JSON_Array *all_of, struct StructFields *dest,
    const JSON_Object *root);

/**
 * @brief Fallback method to apply a union (oneOf/anyOf) to StructFields.
 *
 * @param[in] union_arr The union JSON array.
 * @param[in,out] dest Destination StructFields.
 * @param[in] root Root schema object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t apply_union_to_struct_fields_fallback(
    const JSON_Array *union_arr, struct StructFields *dest,
    const JSON_Object *root);

/**
 * @brief Extended method to apply a union (oneOf/anyOf) to StructFields.
 *
 * @param[in] union_arr The union JSON array.
 * @param[in,out] dest Destination StructFields.
 * @param[in,out] root Root schema object.
 * @param[in] schema_name Optional schema name.
 * @param[in] is_anyof 1 if anyOf, 0 if oneOf.
 * @param[in] schema_obj Schema object containing discriminator.
 * @param[in] allow_inline Whether inline schemas are allowed.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t apply_union_to_struct_fields_ex(
    const JSON_Array *union_arr, struct StructFields *dest, JSON_Object *root,
    const char *schema_name, int is_anyof, const JSON_Object *schema_obj,
    int allow_inline);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CODE2SCHEMA_H */
