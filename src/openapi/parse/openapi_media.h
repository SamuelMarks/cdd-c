/**
 * @file openapi_media.h
 * @brief Internal media type, content map, and encoding parsing declarations.
 * @author Samuel Marks
 */

#ifndef OPENAPI_MEDIA_H
#define OPENAPI_MEDIA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <parson.h>
#include <stddef.h>
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi_types.h"
/* clang-format on */

/** @brief Symbol alias for openapi_media_type_base_len */
#define media_type_base_len openapi_media_type_base_len
/** @brief Symbol alias for openapi_media_type_is_json */
#define media_type_is_json openapi_media_type_is_json

/**
 * @brief Parses an OpenAPI Media Type object.
 *
 * @param[in] media_obj JSON object representing the Media Type.
 * @param[out] out Destination media type structure.
 * @param[in] spec Parent OpenAPI specification context.
 * @param[in] resolve_refs Non-zero to resolve `$ref` against components.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_media_type_object(
    const JSON_Object *media_obj, struct OpenAPI_MediaType *out,
    const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Computes the length of the media type essence (`type/subtype`) before
 * any `;` parameters.
 *
 * @param[in] media_type Media type string.
 * @param[out] _out_val Base length in bytes.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t
openapi_media_type_base_len(const char *media_type, size_t *_out_val);

/**
 * @brief Compares two media type strings ignoring `;` parameters.
 *
 * @param[in] a First media type string.
 * @param[in] b Second media type string.
 * @return Non-zero if base essences match, 0 otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_base_equal(const char *a,
                                                        const char *b);

/**
 * @brief Checks if a media type string represents a JSON media type
 * (`application/json` or `+json`).
 *
 * @param[in] name Media type string.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if JSON media type, CDD_C_SUCCESS (0)
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t openapi_media_type_is_json(const char *name);

/**
 * @brief Computes media type specificity (`0` for wildcard, `1` for type
 * wildcard, `2` for exact `type/subtype`).
 *
 * @param[in] name Media type string.
 * @param[out] out_spec Specificity score.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_specificity(const char *name,
                                                         int *out_spec);

/**
 * @brief Computes media type preference rank (`3` JSON, `2` form-urlencoded,
 * `1` multipart, `0` other).
 *
 * @param[in] name Media type string.
 * @param[out] out_rank Preference rank.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t media_type_preference_rank(const char *name,
                                                             int *out_rank);

/**
 * @brief Selects the preferred primary media type index from an array of media
 * types.
 *
 * @param[in] mts Array of media types.
 * @param[in] n Number of media types.
 * @param[out] out_idx Selected index, or -1 if none.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
extern C_CDD_EXPORT cdd_c_error_t select_primary_media_type_index(
    const struct OpenAPI_MediaType *mts, size_t n, int *out_idx);

/**
 * @brief Finds a media type JSON object in a `content` map by media type name.
 *
 * @param[in] content JSON object representing the `content` map.
 * @param[in] media_name Media type key to look up.
 * @param[out] _out_val Matching JSON_Object pointer, or NULL if not found.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_media_object_by_name(const JSON_Object *content, const char *media_name,
                          const JSON_Object **_out_val);

/**
 * @brief Parses a `content` map of media type objects.
 *
 * @param[in] content JSON object mapping media type ranges to Media Type
 * objects.
 * @param[out] out Output array of parsed media types.
 * @param[out] out_count Output number of parsed media types.
 * @param[in] spec Parent OpenAPI specification context.
 * @param[in] resolve_refs Non-zero to resolve `$ref` against components.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_content_object(
    const JSON_Object *content, struct OpenAPI_MediaType **out,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses an OpenAPI Encoding object.
 *
 * @param[in] enc_obj JSON object representing the Encoding.
 * @param[out] out Destination encoding structure.
 * @param[in] spec Parent OpenAPI specification context.
 * @param[in] resolve_refs Non-zero to resolve `$ref` against components.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
parse_encoding_object(const JSON_Object *enc_obj, struct OpenAPI_Encoding *out,
                      const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses an `encoding` map of Encoding objects.
 *
 * @param[in] enc_obj JSON object mapping property names to Encoding objects.
 * @param[out] out Output array of parsed encodings.
 * @param[out] out_count Output number of parsed encodings.
 * @param[in] spec Parent OpenAPI specification context.
 * @param[in] resolve_refs Non-zero to resolve `$ref` against components.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_encoding_map(
    const JSON_Object *enc_obj, struct OpenAPI_Encoding **out,
    size_t *out_count, const struct OpenAPI_Spec *spec, int resolve_refs);

/**
 * @brief Parses a `prefixEncoding` array of Encoding objects.
 *
 * @param[in] enc_arr JSON array of Encoding objects.
 * @param[out] out Output array of parsed encodings.
 * @param[out] out_count Output number of parsed encodings.
 * @param[in] spec Parent OpenAPI specification context.
 * @param[in] resolve_refs Non-zero to resolve `$ref` against components.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_encoding_array(
    const JSON_Array *enc_arr, struct OpenAPI_Encoding **out, size_t *out_count,
    const struct OpenAPI_Spec *spec, int resolve_refs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_MEDIA_H */
