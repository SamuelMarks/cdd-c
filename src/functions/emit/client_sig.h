/**
 * @file client_sig.h
 * @brief Logic for generating C Client Function Prototypes.
 *
 * Supports standard types and arrays (pointer + len).
 * Includes support for resource-oriented grouping prefixes.
 */

#ifndef C_CDD_CODEGEN_CLIENT_SIG_H
#define C_CDD_CODEGEN_CLIENT_SIG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

/**
 * @brief Configuration for signature generation.
 */
struct CodegenSigConfig {
  const char *prefix; /**< Prefix for function name (e.g. "api_") */
  const char
      *ctx_type; /**< Type of the context arg (default "struct HttpClient *") */
  int include_semicolon;  /**< 1 to append ";\n", 0 for definition start " {\n"
                           */
  const char *group_name; /**< Optional resource grouping name (e.g. "Pet"),
                             results in "Pet_prefix_OpId" */
};

/**
 * @brief Generate a C function prototype for an API operation.
 *
 * @param[in] fp The file stream to write to.
 * @param[in] op The parsed operation definition.
 * @param[in] config Configuration options (can be NULL for defaults).
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
codegen_client_write_signature(FILE *fp, const struct OpenAPI_Operation *op,
                               const struct CodegenSigConfig *config);

#ifdef CDD_BUILD_TESTS
/**
 * @brief Test helper to map OpenAPI type to C argument string.
 * @param[in] oa_type OpenAPI type string.
 * @param[out] out_val Pointer to receive C type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_map_type_to_c_arg(const char *oa_type, const char **out_val);

/**
 * @brief Test helper to check if type is primitive.
 * @param[in] oa_type OpenAPI type string.
 * @param[out] out Pointer to receive 1 if primitive, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_is_primitive_type(const char *oa_type, int *out);

/**
 * @brief Test helper to check if param is object KV.
 * @param[in] p Parameter pointer.
 * @param[out] out Pointer to receive 1 if object KV, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_param_is_object_kv(const struct OpenAPI_Parameter *p, int *out);

/**
 * @brief Test helper to compute base length of media type.
 * @param[in] media_type Media type string.
 * @param[out] out_val Pointer to receive base length.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_media_type_base_len(const char *media_type, size_t *out_val);

/**
 * @brief Test helper to check if media type has prefix.
 * @param[in] media_type Media type string.
 * @param[in] prefix Prefix string.
 * @param[out] out Pointer to receive 1 if prefix matches, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sig_media_type_has_prefix(
    const char *media_type, const char *prefix, int *out);

/**
 * @brief Test helper to check if media type has suffix.
 * @param[in] media_type Media type string.
 * @param[in] suffix Suffix string.
 * @param[out] out Pointer to receive 1 if suffix matches, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sig_media_type_has_suffix(
    const char *media_type, const char *suffix, int *out);

/**
 * @brief Test helper for media type equality.
 * @param[in] media_type Media type string.
 * @param[in] expected Expected string.
 * @param[out] out Pointer to receive 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sig_media_type_ieq(
    const char *media_type, const char *expected, int *out);

/**
 * @brief Test helper to check if media type is JSON.
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if JSON, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_media_type_is_json(const char *media_type, int *out);

/**
 * @brief Test helper to check if media type is form.
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if form, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_media_type_is_form(const char *media_type, int *out);

/**
 * @brief Test helper to check if media type is text/plain.
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if text/plain, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_media_type_is_text_plain(const char *media_type, int *out);

/**
 * @brief Test helper to check if media type is multipart.
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if multipart, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_media_type_is_multipart(const char *media_type, int *out);

/**
 * @brief Test helper to check if media type is multipart/form-data.
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if multipart/form-data, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_media_type_is_multipart_form(const char *media_type, int *out);

/**
 * @brief Test helper to find media type by name.
 * @param[in] mts Array of media types.
 * @param[in] n Array size.
 * @param[in] name Target name.
 * @param[out] out_val Pointer to receive matched media type.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sig_find_media_type(
    const struct OpenAPI_MediaType *mts, size_t n, const char *name,
    const struct OpenAPI_MediaType **out_val);

/**
 * @brief Test helper to check if media type is textual.
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if textual, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_media_type_is_textual(const char *media_type, int *out);

/**
 * @brief Test helper to check if media type is binary.
 * @param[in] media_type Media type string.
 * @param[out] out Pointer to receive 1 if binary, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_media_type_is_binary(const char *media_type, int *out);

/**
 * @brief Test helper to check if querystring param is form object.
 * @param[in] p Parameter pointer.
 * @param[out] out Pointer to receive 1 if form object, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sig_querystring_param_is_form_object(
    const struct OpenAPI_Parameter *p, int *out);

/**
 * @brief Test helper to check if querystring param is JSON ref.
 * @param[in] p Parameter pointer.
 * @param[out] out Pointer to receive 1 if JSON ref, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sig_querystring_param_is_json_ref(
    const struct OpenAPI_Parameter *p, int *out);

/**
 * @brief Test helper to determine querystring param JSON primitive type.
 * @param[in] p Parameter pointer.
 * @param[out] out_val Pointer to receive primitive type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_querystring_param_json_primitive_type(
    const struct OpenAPI_Parameter *p, const char **out_val);

/**
 * @brief Test helper to determine querystring param JSON array item type.
 * @param[in] p Parameter pointer.
 * @param[out] out_val Pointer to receive item type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_querystring_param_json_array_item_type(
    const struct OpenAPI_Parameter *p, const char **out_val);

/**
 * @brief Test helper to determine querystring param JSON array item ref.
 * @param[in] p Parameter pointer.
 * @param[out] out_val Pointer to receive item ref string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_querystring_param_json_array_item_ref(
    const struct OpenAPI_Parameter *p, const char **out_val);

/**
 * @brief Test helper to determine querystring param raw primitive type.
 * @param[in] p Parameter pointer.
 * @param[out] out_val Pointer to receive primitive type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_querystring_param_raw_primitive_type(
    const struct OpenAPI_Parameter *p, const char **out_val);

/**
 * @brief Test helper to map OpenAPI array item type to C type.
 * @param[in] oa_type OpenAPI type string.
 * @param[out] out_val Pointer to receive C type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_map_array_item_type(const char *oa_type, const char **out_val);

/**
 * @brief Test helper to sanitize an identifier.
 * @param[out] out Output buffer.
 * @param[in] outsz Size of output buffer.
 * @param[in] in Input string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sig_sanitize_ident(char *out,
                                                              size_t outsz,
                                                              const char *in);

/**
 * @brief Test helper to construct multipart header param name.
 * @param[out] out Output buffer.
 * @param[in] outsz Size of output buffer.
 * @param[in] field Field name.
 * @param[in] header Header name.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sig_multipart_header_param_name(
    char *out, size_t outsz, const char *field, const char *header);

/**
 * @brief Test helper to check if header is Content-Type.
 * @param[in] name Header name.
 * @param[out] out Pointer to receive 1 if Content-Type, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_header_name_is_content_type(const char *name, int *out);

/**
 * @brief Test helper to map OpenAPI type to C output parameter type.
 * @param[in] oa_type OpenAPI type string.
 * @param[out] out_val Pointer to receive C out type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_map_type_to_c_out(const char *oa_type, const char **out_val);

/**
 * @brief Test helper to map OpenAPI array item type to C output parameter type.
 * @param[in] oa_type OpenAPI type string.
 * @param[out] out_val Pointer to receive C out array item type string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_map_array_item_type_out(const char *oa_type, const char **out_val);

/**
 * @brief Test helper to check if schema has inline type.
 * @param[in] schema Schema reference pointer.
 * @param[out] out Pointer to receive 1 if inline, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sig_schema_has_inline(
    const struct OpenAPI_SchemaRef *schema, int *out);

/**
 * @brief Test helper to get success response.
 * @param[in] op Operation pointer.
 * @param[out] out_val Pointer to receive success response.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_get_success_response(const struct OpenAPI_Operation *op,
                                  const struct OpenAPI_Response **out_val);

/**
 * @brief Test helper to check if response is binary success.
 * @param[in] op Operation pointer.
 * @param[out] out Pointer to receive 1 if binary success, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sig_response_is_binary_success(
    const struct OpenAPI_Operation *op, int *out);

/**
 * @brief Test helper to get success schema.
 * @param[in] op Operation pointer.
 * @param[out] out_val Pointer to receive success schema reference.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sig_get_success_schema(const struct OpenAPI_Operation *op,
                                const struct OpenAPI_SchemaRef **out_val);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_CLIENT_SIG_H */
