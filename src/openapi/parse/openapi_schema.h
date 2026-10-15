/**
 * @file openapi_schema.h
 * @brief Internal schema parsing, inline schema registration, and schema
 * reference utilities.
 * @author Samuel Marks
 */

#ifndef OPENAPI_SCHEMA_H
#define OPENAPI_SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <parson.h>
#include <stddef.h>
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "classes/emit/struct.h"
#include "openapi/parse/openapi_types.h"
/* clang-format on */

/**
 * @brief Parses a schema `type` field (string or union array) and detects
 * `null`.
 *
 * @param[in] schema JSON object representing the schema.
 * @param[out] out_nullable Set to 1 if `"null"` or `nullable: true` is present.
 * @param[out] _out_val Primary non-null type string (borrowed from JSON).
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_schema_type(const JSON_Object *schema,
                                                    int *out_nullable,
                                                    const char **_out_val);

/**
 * @brief Parses numeric, string, and array validation constraints from a schema
 * object into a target structure.
 *
 * @param[in] schema JSON object representing the schema.
 * @param[in,out] target Target pointers receiving parsed constraint values.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_schema_constraints(
    const JSON_Object *schema, struct SchemaConstraintTarget *target);

/**
 * @brief Parses a JSON array of strings (e.g. `enum` or `type` array).
 *
 * @param[in] arr JSON array of strings.
 * @param[out] out Output array of allocated strings.
 * @param[out] out_count Output number of strings.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_string_enum_array(const JSON_Array *arr,
                                                          char ***out,
                                                          size_t *out_count);

/**
 * @brief Frees an array of strings with a known element count.
 *
 * @param[in] arr Array of string pointers.
 * @param[in] n Number of elements in the array.
 */
extern C_CDD_EXPORT void free_string_array(char **arr, size_t n);

/**
 * @brief Deep-copies an array of strings.
 *
 * @param[out] dst Output allocated string array.
 * @param[out] dst_count Output number of strings.
 * @param[in] src Source string array.
 * @param[in] src_count Number of strings in source.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t copy_string_array(char ***dst,
                                                    size_t *dst_count,
                                                    char **src,
                                                    size_t src_count);

/**
 * @brief Checks if a JSON schema object represents a pure string `enum`.
 *
 * @param[in] schema_obj JSON object representing the schema.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if string enum only, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
schema_is_string_enum_only(const JSON_Object *schema_obj);

/**
 * @brief Checks if a JSON schema value/object is compatible with C struct
 * generation.
 *
 * @param[in] schema_val JSON value of the schema.
 * @param[in] schema_obj JSON object of the schema.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if struct-compatible, CDD_C_SUCCESS
 * (0) otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t schema_is_struct_compatible(
    const JSON_Value *schema_val, const JSON_Object *schema_obj);

/**
 * @brief Checks if a JSON schema object uses `allOf`, `anyOf`, or `oneOf`
 * composition.
 *
 * @param[in] schema_obj JSON object representing the schema.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if composition is present,
 * CDD_C_SUCCESS (0) otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
schema_has_composition(const JSON_Object *schema_obj);

/**
 * @brief Parses a JSON array of schemas into an array of OpenAPI_SchemaRef.
 *
 * @param[in] arr JSON array of schema objects.
 * @param[out] out Output array of parsed schema references.
 * @param[out] out_count Output number of parsed schema references.
 * @param[in] spec Parent OpenAPI specification context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_schema_array_ref(const JSON_Array *arr, struct OpenAPI_SchemaRef **out,
                       size_t *out_count, const struct OpenAPI_Spec *spec);

/**
 * @brief Allocates and parses a single heap-allocated OpenAPI_SchemaRef.
 *
 * @param[in] obj JSON object representing the schema.
 * @param[out] out Output pointer to the newly allocated OpenAPI_SchemaRef.
 * @param[in] spec Parent OpenAPI specification context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_schema_ref_ptr(const JSON_Object *obj, struct OpenAPI_SchemaRef **out,
                     const struct OpenAPI_Spec *spec);

/**
 * @brief Parses a JSON schema object into an OpenAPI_SchemaRef structure.
 *
 * @param[in] schema JSON object representing the schema.
 * @param[out] out Destination OpenAPI_SchemaRef structure.
 * @param[in] spec Parent OpenAPI specification context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_schema_ref(const JSON_Object *schema, struct OpenAPI_SchemaRef *out,
                 const struct OpenAPI_Spec *spec);

/**
 * @brief Applies type/items information from a SchemaRef to an
 * OpenAPI_Parameter.
 *
 * @param[in,out] out_param Parameter to update.
 * @param[in] schema_ref Source schema reference.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t
apply_schema_ref_to_param(struct OpenAPI_Parameter *out_param,
                          const struct OpenAPI_SchemaRef *schema_ref);

/**
 * @brief Applies type/items information from a SchemaRef to an OpenAPI_Header.
 *
 * @param[in,out] out_hdr Header to update.
 * @param[in] schema_ref Source schema reference.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t apply_schema_ref_to_header(
    struct OpenAPI_Header *out_hdr, const struct OpenAPI_SchemaRef *schema_ref);

/**
 * @brief Checks if a schema name is already registered in an OpenAPI_Spec.
 *
 * @param[in] spec OpenAPI specification to inspect.
 * @param[in] name Schema name to check.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if in use, CDD_C_SUCCESS (0) if free.
 */
extern C_CDD_EXPORT cdd_c_error_t
schema_name_in_use(const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Sanitizes a string into a valid OpenAPI component key
 * (`[A-Za-z0-9._-]`).
 *
 * @param[in] name Raw name string.
 * @param[out] _out_val Newly allocated sanitized component name.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t sanitize_component_name(const char *name,
                                                          char **_out_val);

/**
 * @brief Generates a unique schema name in `spec` based on `base`.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] base Base schema name.
 * @param[out] _out_val Newly allocated unique schema name.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t make_unique_schema_name(
    const struct OpenAPI_Spec *spec, const char *base, char **_out_val);

/**
 * @brief Checks if a JSON array of type strings contains `type`.
 *
 * @param[in] arr JSON array of type strings.
 * @param[in] type Target type string to find.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if present, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
schema_type_array_includes(const JSON_Array *arr, const char *type);

/**
 * @brief Checks if a JSON schema object represents an object-like structure.
 *
 * @param[in] schema_obj JSON object representing the schema.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if object-like, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
schema_object_is_object_like(const JSON_Object *schema_obj);

/**
 * @brief Appends a parsed struct schema definition to `spec->defined_schemas`.
 *
 * @param[in,out] spec Target OpenAPI specification.
 * @param[in] schema_name Owned schema name string (ownership transferred on
 * success).
 * @param[in] schema_fields Parsed struct fields (ownership transferred on
 * success).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
append_defined_schema(struct OpenAPI_Spec *spec, char *schema_name,
                      struct StructFields *schema_fields);

/**
 * @brief Checks if a raw JSON schema name already exists in `spec`.
 *
 * @param[in] spec OpenAPI specification to inspect.
 * @param[in] name Schema name to check.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if exists, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
raw_schema_name_exists(const struct OpenAPI_Spec *spec, const char *name);

/**
 * @brief Serializes and appends a raw JSON schema value to
 * `spec->raw_schema_json`.
 *
 * @param[in,out] spec Target OpenAPI specification.
 * @param[in] name Schema name.
 * @param[in] schema_val JSON value of the schema.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t append_raw_schema(
    struct OpenAPI_Spec *spec, const char *name, const JSON_Value *schema_val);

/**
 * @brief Registers an inline object schema into `spec` and outputs its
 * registered name.
 *
 * @param[in,out] spec Target OpenAPI specification.
 * @param[in] base_name Suggested base name for the inline schema.
 * @param[in] schema_obj JSON object of the inline schema.
 * @param[in] schema_val Wrapping JSON value of the inline schema.
 * @param[out] out_name Borrowed pointer to the registered schema name in
 * `spec`.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
register_inline_schema(struct OpenAPI_Spec *spec, const char *base_name,
                       const JSON_Object *schema_obj,
                       const JSON_Value *schema_val, char **out_name);

/**
 * @brief Replaces `schema_ref->ref_name` with a duplicate of `name`.
 *
 * @param[in,out] schema_ref Target schema reference.
 * @param[in] name Schema reference name to assign.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
assign_schema_ref_name(struct OpenAPI_SchemaRef *schema_ref, const char *name);

/**
 * @brief Builds a synthetic inline request body schema name
 * (`Inline_<op>_Request`).
 *
 * @param[in] op_id Operation identifier.
 * @param[in] is_item Non-zero if naming an item schema.
 * @param[out] _out_val Newly allocated inline request schema name.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t build_inline_request_name(const char *op_id,
                                                            int is_item,
                                                            char **_out_val);

/**
 * @brief Builds a synthetic inline response schema name
 * (`Inline_<op>_Response_<code>`).
 *
 * @param[in] op_id Operation identifier.
 * @param[in] code HTTP status code string.
 * @param[in] is_item Non-zero if naming an item schema.
 * @param[out] _out_val Newly allocated inline response schema name.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t build_inline_response_name(const char *op_id,
                                                             const char *code,
                                                             int is_item,
                                                             char **_out_val);

/**
 * @brief Builds a synthetic inline querystring parameter schema name.
 *
 * @param[in] param_name Parameter name.
 * @param[out] _out_val Newly allocated inline parameter schema name.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t
build_inline_param_name(const char *param_name, char **_out_val);

/**
 * @brief Checks if a root JSON value/object is a standalone JSON Schema
 * document.
 *
 * @param[in] root Root JSON value.
 * @param[in] root_obj Root JSON object (if object).
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if standalone schema document,
 * CDD_C_SUCCESS (0) otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
root_is_schema_document(const JSON_Value *root, const JSON_Object *root_obj);

/**
 * @brief Stores the serialized root JSON of a standalone schema document in
 * `spec`.
 *
 * @param[in,out] spec Target OpenAPI specification.
 * @param[in] root Root JSON value.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
extern C_CDD_EXPORT cdd_c_error_t
store_schema_root_json(struct OpenAPI_Spec *spec, const JSON_Value *root);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_SCHEMA_H */
