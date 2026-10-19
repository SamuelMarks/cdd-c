/**
 * @file url.h
 * @brief URL Generation Logic for Client Code.
 *
 * Provides functionality to:
 * - Construct path URL string.
 * - Construct path URL string with matrix/label/simple styles for path params,
 *   including object params provided as `struct OpenAPI_KV` pairs.
 * - Generate Query Parameter block logic (including Arrays and Explode style).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_URL_H
#define C_CDD_CODEGEN_URL_H

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
 * @brief Configuration for URL code generation.
 */
struct CodegenUrlConfig {
  const char *base_variable; /**< Variable name for base URL (default
                                "ctx->base_url") */
  const char
      *out_variable; /**< Variable name for the output char* (default "url") */
};

/**
 * @brief Generate C code to construct a URL from a path template.
 *
 * @param[in] fp The file stream to write C code to.
 * @param[in] path_template The raw OpenAPI route string (e.g. "/users/{id}").
 * @param[in] params Array of operation parameters to look up types.
 * @param[in] n_params param count.
 * @param[in] config Configuration options (can be NULL for defaults).
 * @return 0 on success, error code (EINVAL/ENOMEM) on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t codegen_url_write_builder(
    FILE *fp, const char *path_template, const struct OpenAPI_Parameter *params,
    size_t n_params, const struct CodegenUrlConfig *config);

/**
 * @brief Generate C code for adding query parameters to the URL builder.
 *
 * Handles scalar values (by mapping to `url_query_add`) and Array values.
 * For Arrays with `style=form, explode=true`, generates a `for` loop emitting
 * multiple key-value pairs (e.g. `?id=1&id=2`). For `style=form,
 * explode=false`, generates a comma-separated value and uses
 * `url_query_add_encoded` to preserve the delimiter. Also supports
 * `spaceDelimited` and `pipeDelimited` array styles. Object parameters in
 * `style=form` and `style=deepObject` are serialized from
 * `struct OpenAPI_KV` pairs.
 *
 * @param[in] fp The file stream to write to.
 * @param[in] op The operation containing parameters.
 * @param[in] qp_tracking Non-zero if `qp_initialized` is available in scope.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t codegen_url_write_query_params(
    FILE *fp, const struct OpenAPI_Operation *op, int qp_tracking);

/**
 * @brief Checks if an OpenAPI parameter type string is a primitive scalar.
 *
 * @param[in] type Parameter type string.
 * @return Non-zero if primitive, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t is_primitive_type_url(const char *type);

/**
 * @brief Checks if an OpenAPI parameter is represented as key-value object
 * pairs.
 *
 * @param[in] p The OpenAPI parameter structure.
 * @return Non-zero if object KV parameter, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
param_is_object_kv_url(const struct OpenAPI_Parameter *p);

/**
 * @brief Computes the base essence length (before `;`) of a media type string.
 *
 * @param[in] media_type The media type string.
 * @param[out] _out_val Output length of the base media type.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
media_type_base_len_url(const char *media_type, size_t *_out_val);

/**
 * @brief Compares a media type's base essence case-insensitively against an
 * expected string.
 *
 * @param[in] media_type The media type string to inspect.
 * @param[in] expected The expected base media type string.
 * @return Non-zero if equal, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_ieq_url(const char *media_type,
                                                     const char *expected);

/**
 * @brief Checks whether a media type is `application/json` or has a `+json`
 * suffix.
 *
 * @param[in] media_type The media type string.
 * @return Non-zero if JSON media type, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
media_type_is_json_url(const char *media_type);

/**
 * @brief Checks whether a media type is `application/x-www-form-urlencoded`.
 *
 * @param[in] media_type The media type string.
 * @return Non-zero if form-urlencoded media type, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
media_type_is_form_url(const char *media_type);

/**
 * @brief Checks whether a `querystring` parameter has a form-encoded object
 * schema.
 *
 * @param[in] p The OpenAPI parameter structure.
 * @return Non-zero if form object querystring, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
querystring_param_is_form_object(const struct OpenAPI_Parameter *p);

/**
 * @brief Checks whether a `querystring` parameter references a JSON object
 * model.
 *
 * @param[in] p The OpenAPI parameter structure.
 * @return Non-zero if JSON model ref querystring, CDD_C_SUCCESS otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
querystring_param_is_json_ref(const struct OpenAPI_Parameter *p);

/**
 * @brief Gets the primitive type name of a JSON-encoded `querystring`
 * parameter.
 *
 * @param[in] p The OpenAPI parameter structure.
 * @param[out] _out_val Output primitive type string, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t querystring_param_json_primitive_type(
    const struct OpenAPI_Parameter *p, const char **_out_val);

/**
 * @brief Gets the array item primitive type of a JSON-encoded `querystring`
 * parameter.
 *
 * @param[in] p The OpenAPI parameter structure.
 * @param[out] _out_val Output item primitive type string, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t querystring_param_json_array_item_type(
    const struct OpenAPI_Parameter *p, const char **_out_val);

/**
 * @brief Gets the array item model reference of a JSON-encoded `querystring`
 * parameter.
 *
 * @param[in] p The OpenAPI parameter structure.
 * @param[out] _out_val Output item model reference string, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t querystring_param_json_array_item_ref(
    const struct OpenAPI_Parameter *p, const char **_out_val);

/**
 * @brief Emits C code to serialize a JSON-encoded query parameter.
 *
 * @param[in,out] fp Output file stream.
 * @param[in] p The OpenAPI parameter structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_query_json_param(FILE *fp, const struct OpenAPI_Parameter *p);

/** @brief UrlSegment structure */
struct UrlSegment {
  /** @brief is_var */
  int is_var;
  /** @brief text */
  char *text;
};

/**
 * @brief Gets the raw primitive type of a non-JSON `querystring` parameter.
 *
 * @param[in] p The OpenAPI parameter structure.
 * @param[out] _out_val Output primitive type string, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t querystring_param_raw_primitive_type(
    const struct OpenAPI_Parameter *p, const char **_out_val);

/**
 * @brief Emits C code to serialize an object query parameter (`form` or
 * `deepObject`).
 *
 * @param[in,out] fp Output file stream.
 * @param[in] p The OpenAPI parameter structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_query_object_param(FILE *fp, const struct OpenAPI_Parameter *p);

/**
 * @brief Emits C code to serialize an object path parameter (`simple`, `label`,
 * `matrix`).
 *
 * @param[in,out] fp Output file stream.
 * @param[in] p The OpenAPI parameter structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_path_object_serialization(FILE *fp, const struct OpenAPI_Parameter *p);

/**
 * @brief Emits C code to serialize an array path parameter.
 *
 * @param[in,out] fp Output file stream.
 * @param[in] p The OpenAPI parameter structure.
 * @param[in] prefix Style prefix string (e.g. `.` or `;`).
 * @param[in] delim Element delimiter string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_path_array_serialization(FILE *fp, const struct OpenAPI_Parameter *p,
                               const char *prefix, const char *delim);

/**
 * @brief Emits C code to serialize a joined (non-exploded) query array
 * parameter.
 *
 * @param[in,out] fp Output file stream.
 * @param[in] p The OpenAPI parameter structure.
 * @param[in] delim Delimiter character.
 * @param[in] encode_fn Name of the C URL-encoding helper function.
 * @param[in] add_encoded Non-zero to add the joined string as pre-encoded.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
write_joined_query_array(FILE *fp, const struct OpenAPI_Parameter *p,
                         char delim, const char *encode_fn, int add_encoded);

/**
 * @brief Emits C code to serialize a joined query array with a pre-encoded
 * delimiter.
 *
 * @param[in,out] fp Output file stream.
 * @param[in] p The OpenAPI parameter structure.
 * @param[in] delim_enc Percent-encoded delimiter string.
 * @param[in] encode_fn Name of the C URL-encoding helper function.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t write_joined_query_array_encoded_delim(
    FILE *fp, const struct OpenAPI_Parameter *p, const char *delim_enc,
    const char *encode_fn);

/**
 * @brief Finds a parameter by name in a parameter array.
 *
 * @param[in] name Parameter name to search for.
 * @param[in] params Array of OpenAPI parameters.
 * @param[in] n_params Number of parameters in `params`.
 * @param[out] _out_val Output pointer to the found parameter, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_param(const char *name, const struct OpenAPI_Parameter *params,
           size_t n_params, const struct OpenAPI_Parameter **_out_val);

/**
 * @brief Parses a URL path template into literal and variable segments.
 *
 * @param[in] tmpl URL path template string.
 * @param[out] out_segments Output array of parsed URL segments.
 * @param[out] out_count Output number of segments.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_segments(
    const char *tmpl, struct UrlSegment **out_segments, size_t *out_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_URL_H */
