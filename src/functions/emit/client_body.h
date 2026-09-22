/**
 * @file client_body.h
 * @brief Client Function Body Generator Interface.
 *
 * Orchestrates the generation of the full implementation of an API client
 * function. Supports parameter serialization (Query, Header, Path), Body
 * serialization (JSON, multipart, form-urlencoded), and Security injection.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_CLIENT_BODY_H
#define C_CDD_CODEGEN_CLIENT_BODY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

struct StructFields;

/**
 * @brief Generate the implementation body for a client function.
 *
 * Emits code inside the function braces.
 *
 * @param[in] fp The file stream to write to.
 * @param[in] op The operation definition.
 * @param[in] spec The full specification (needed for Security Definitions).
 * @param[in] path_template The raw path string (e.g. "/pets/{id}").
 * @param[in] base_url_expr Optional C expression to override base URL.
 *                          Pass NULL to use ctx->base_url.
 * @return 0 on success, error code (EIO, etc) on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
codegen_client_write_body(FILE *fp, const struct OpenAPI_Operation *op,
                          const struct OpenAPI_Spec *spec,
                          const char *path_template, const char *base_url_expr);

/**
 * @brief Map an OpenAPI HTTP verb enum to its uppercase string literal.
 *
 * @param[in] v The OpenAPI verb enum value.
 * @param[out] _out_val Pointer to receive string constant (e.g. "HTTP_GET").
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_verb_to_enum_str(enum OpenAPI_Verb v, const char **_out_val);

/**
 * @brief Map a method name string to its HTTP enum string constant.
 *
 * @param[in] method HTTP method name string (e.g. "get", "post").
 * @param[out] _out_val Pointer to receive string constant or NULL if unknown.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_method_str_to_enum_str(const char *method, const char **_out_val);

/**
 * @brief Map an HTTP status code to an appropriate error code symbol name.
 *
 * @param[in] status HTTP integer status code (e.g. 400, 404, 500).
 * @param[out] _out_val Pointer to receive the error code string constant.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_mapped_err_code(int status, const char **_out_val);

/**
 * @brief Find matching media type in an array of media types.
 *
 * @param[in] mts Array of media types.
 * @param[in] n Number of elements in array.
 * @param[in] name Media type name to find.
 * @param[out] _out_val Pointer to receive found element or NULL.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_find_media_type(
    const struct OpenAPI_MediaType *mts, size_t n, const char *name,
    const struct OpenAPI_MediaType **_out_val);

/**
 * @brief Find encoding object by name within a media type.
 *
 * @param[in] mt The media type object.
 * @param[in] name Field name whose encoding is looked up.
 * @param[out] _out_val Pointer to receive found encoding pointer or NULL.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_find_encoding(const struct OpenAPI_MediaType *mt, const char *name,
                          struct OpenAPI_Encoding **_out_val);

/**
 * @brief Check if a type name corresponds to an OpenAPI primitive type.
 *
 * @param[in] type Type name string.
 * @param[out] out_is_prim Receives 1 if primitive, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_is_prim
 * is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_is_primitive_type(const char *type, int *out_is_prim);

/**
 * @brief Check if a type name corresponds to an object reference type.
 *
 * @param[in] type Type name string.
 * @param[out] out_is_obj Receives 1 if object ref, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_is_obj
 * is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_is_object_ref_type(const char *type, int *out_is_obj);

/**
 * @brief Check if all fields of a struct are primitive types.
 *
 * @param[in] sf Struct fields definition.
 * @param[out] out_all_prim Receives 1 if all primitive, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if
 * out_all_prim is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_struct_fields_all_primitive(
    const struct StructFields *sf, int *out_all_prim);

/**
 * @brief Check if a schema reference has an inline type definition.
 *
 * @param[in] schema Schema reference.
 * @param[out] out_has Receives 1 if inline type exists, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_has is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_schema_has_inline(
    const struct OpenAPI_SchemaRef *schema, int *out_has);

/**
 * @brief Compute base length of a media type string before optional parameters.
 *
 * @param[in] media_type Media type string (e.g. "application/json;
 * charset=utf-8").
 * @param[out] _out_val Receives the length of the base media type.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_media_type_base_len(const char *media_type, size_t *_out_val);

/**
 * @brief Case-insensitively check if media type starts with a given prefix.
 *
 * @param[in] media_type Media type string.
 * @param[in] prefix Prefix string to check.
 * @param[out] out_has Receives 1 if prefix matches, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_has is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_media_type_has_prefix(
    const char *media_type, const char *prefix, int *out_has);

/**
 * @brief Case-insensitively check if media type ends with a given suffix.
 *
 * @param[in] media_type Media type string.
 * @param[in] suffix Suffix string to check.
 * @param[out] out_has Receives 1 if suffix matches, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_has is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_media_type_has_suffix(
    const char *media_type, const char *suffix, int *out_has);

/**
 * @brief Case-insensitively compare media type base portion against expected
 * string.
 *
 * @param[in] media_type Media type string.
 * @param[in] expected Expected media type string.
 * @param[out] out_eq Receives 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_eq is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_media_type_ieq(
    const char *media_type, const char *expected, int *out_eq);

/**
 * @brief Test if media type represents JSON.
 *
 * @param[in] media_type Media type string.
 * @param[out] out_is_json Receives 1 if JSON, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_is_json
 * is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_media_type_is_json(const char *media_type, int *out_is_json);

/**
 * @brief Test if media type represents form urlencoded data.
 *
 * @param[in] media_type Media type string.
 * @param[out] out_is_form Receives 1 if form urlencoded, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_is_form
 * is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_media_type_is_form(const char *media_type, int *out_is_form);

/**
 * @brief Test if media type represents text/plain.
 *
 * @param[in] media_type Media type string.
 * @param[out] out_is_text_plain Receives 1 if text/plain, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if
 * out_is_text_plain is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_media_type_is_text_plain(
    const char *media_type, int *out_is_text_plain);

/**
 * @brief Test if media type is multipart.
 *
 * @param[in] media_type Media type string.
 * @param[out] out_is_mp Receives 1 if multipart, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_is_mp
 * is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_media_type_is_multipart(const char *media_type, int *out_is_mp);

/**
 * @brief Test if media type is multipart/form-data.
 *
 * @param[in] media_type Media type string.
 * @param[out] out_is_mp_form Receives 1 if multipart/form-data, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if
 * out_is_mp_form is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_media_type_is_multipart_form(
    const char *media_type, int *out_is_mp_form);

/**
 * @brief Extract first content type token from comma-separated list into
 * buffer.
 *
 * @param[in] content_type Content type list string.
 * @param[out] buf Output buffer.
 * @param[in] buf_sz Size of output buffer.
 * @param[out] _out_val Receives pointer to buf.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * pointers.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_first_content_type_entry(
    const char *content_type, char *buf, size_t buf_sz, const char **_out_val);

/**
 * @brief Sanitize input string into valid C identifier characters.
 *
 * @param[out] out Output buffer.
 * @param[in] outsz Size of output buffer.
 * @param[in] in Input string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * pointers.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_sanitize_ident(char *out,
                                                             size_t outsz,
                                                             const char *in);

/**
 * @brief Build C variable identifier for multipart header parameter.
 *
 * @param[out] out Output buffer.
 * @param[in] outsz Size of output buffer.
 * @param[in] field Field name.
 * @param[in] header Header name.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_multipart_header_param_name(
    char *out, size_t outsz, const char *field, const char *header);

/**
 * @brief Check if header name case-insensitively matches Content-Type.
 *
 * @param[in] name Header name string.
 * @param[out] out_is_ct Receives 1 if Content-Type, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_is_ct
 * is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_header_name_is_content_type(const char *name, int *out_is_ct);

/**
 * @brief Test if media type represents textual content.
 *
 * @param[in] media_type Media type string.
 * @param[out] out_is_textual Receives 1 if textual, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if
 * out_is_textual is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_media_type_is_textual(const char *media_type, int *out_is_textual);

/**
 * @brief Test if media type represents binary content.
 *
 * @param[in] media_type Media type string.
 * @param[out] out_is_bin Receives 1 if binary, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_is_bin
 * is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_media_type_is_binary(const char *media_type, int *out_is_bin);

/**
 * @brief Test if schema inline type is string.
 *
 * @param[in] schema Schema reference.
 * @param[out] out_is_string Receives 1 if inline string, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if
 * out_is_string is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_schema_inline_is_string(
    const struct OpenAPI_SchemaRef *schema, int *out_is_string);

/**
 * @brief Test if response schema represents textual string body.
 *
 * @param[in] resp Response definition.
 * @param[out] out_is_textual_string Receives 1 if textual string, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if
 * out_is_textual_string is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_response_is_textual_string(
    const struct OpenAPI_Response *resp, int *out_is_textual_string);

/**
 * @brief Test if response definition represents binary payload.
 *
 * @param[in] resp Response definition.
 * @param[out] out_is_binary Receives 1 if binary, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if
 * out_is_binary is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_response_is_binary(
    const struct OpenAPI_Response *resp, int *out_is_binary);

/**
 * @brief Check if schema reference represents a payload with contents.
 *
 * @param[in] schema Schema reference.
 * @param[out] out_has_payload Receives 1 if payload present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if
 * out_has_payload is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_schema_has_payload(
    const struct OpenAPI_SchemaRef *schema, int *out_has_payload);

/**
 * @brief Emit C code for handling text/plain success response.
 *
 * @param[in] fp File stream to write to.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_write_text_plain_success(FILE *fp);

/**
 * @brief Emit C code for handling binary success response.
 *
 * @param[in] fp File stream to write to.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_write_binary_success(FILE *fp);

/**
 * @brief Emit C code for parsing inline JSON schema response.
 *
 * @param[in] fp File stream to write to.
 * @param[in] schema Schema reference with inline type.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_write_inline_json_parse(
    FILE *fp, const struct OpenAPI_SchemaRef *schema);

/**
 * @brief Emit C code for joining array fields into form-urlencoded query
 * parameters.
 *
 * @param[in] fp File stream to write to.
 * @param[in] field Field name.
 * @param[in] len_field Field length variable name.
 * @param[in] items_type Items element type name.
 * @param[in] delim Delimiter character.
 * @param[in] encode_fn Encoding function name or NULL.
 * @param[in] add_encoded Non-zero to use url_query_add_encoded.
 * @param[in] is_object Non-zero if items are object references.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_write_joined_form_array(
    FILE *fp, const char *field, const char *len_field, const char *items_type,
    char delim, const char *encode_fn, int add_encoded, int is_object);

/**
 * @brief Emit C code for adding header parameters to HTTP request.
 *
 * @param[in] fp File stream to write to.
 * @param[in] op Operation definition.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_write_header_param_logic(
    FILE *fp, const struct OpenAPI_Operation *op);

/**
 * @brief Emit C code for building form-urlencoded request body.
 *
 * @param[in] fp File stream to write to.
 * @param[in] op Operation definition.
 * @param[in] spec Specification containing schema definitions.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_write_form_urlencoded_body(
    FILE *fp, const struct OpenAPI_Operation *op,
    const struct OpenAPI_Spec *spec);

/**
 * @brief Emit C code for building cookie header parameters.
 *
 * @param[in] fp File stream to write to.
 * @param[in] op Operation definition.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_write_cookie_param_logic(
    FILE *fp, const struct OpenAPI_Operation *op);

/**
 * @brief Emit C code for multipart part headers.
 *
 * @param[in] fp File stream to write to.
 * @param[in] enc Encoding definition for part.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t client_body_write_multipart_part_headers(
    FILE *fp, const struct OpenAPI_Encoding *enc);

/**
 * @brief Emit C code for constructing multipart/form-data request body.
 *
 * @param[in] fp File stream to write to.
 * @param[in] op Operation definition.
 * @param[in] spec Specification containing schema definitions.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_write_multipart_body(FILE *fp, const struct OpenAPI_Operation *op,
                                 const struct OpenAPI_Spec *spec);

/**
 * @brief Test if status code string is a range code (e.g. "2XX", "4XX").
 *
 * @param[in] code HTTP status code string.
 * @param[out] out_is_range Receives 1 if range code, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if
 * out_is_range is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_is_status_range_code(const char *code, int *out_is_range);

/**
 * @brief Extract numerical prefix from status range code.
 *
 * @param[in] code HTTP status code string.
 * @param[out] out_prefix Receives prefix digit (e.g. 2, 4, 5).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_prefix
 * is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_status_range_prefix(const char *code, int *out_prefix);

/**
 * @brief Test if status code string is a 3-digit literal (e.g. "200", "404").
 *
 * @param[in] code HTTP status code string.
 * @param[out] out_is_lit Receives 1 if 3-digit literal, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_is_lit
 * is NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t
client_body_is_status_code_literal(const char *code, int *out_is_lit);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_CLIENT_BODY_H */
