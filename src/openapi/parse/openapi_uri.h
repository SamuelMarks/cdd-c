/**
 * @file openapi_uri.h
 * @brief Internal URI resolution, JSON Pointer unescaping, and reference target
 * utilities for OpenAPI parsing.
 * @author Samuel Marks
 */

#ifndef OPENAPI_URI_H
#define OPENAPI_URI_H

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

#ifndef RESOLVED_REF_TARGET_DEF
#define RESOLVED_REF_TARGET_DEF
/**
 * @brief Resolved target specification and reference string for a `$ref`.
 */
struct ResolvedRefTarget {
  /** @brief Target OpenAPI specification containing the referenced object. */
  const struct OpenAPI_Spec *spec;
  /** @brief Effective reference string (borrowed or pointing to resolved_ref).
   */
  const char *ref;
  /** @brief Dynamically allocated resolved reference string, if any. */
  char *resolved_ref;
};
#endif

/**
 * @brief Unescapes a JSON Pointer token (`~1` -> `/`, `~0` -> `~`).
 *
 * @param[in] in Escaped JSON Pointer token string.
 * @param[out] _out_val Newly allocated unescaped string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t json_pointer_unescape(const char *in,
                                                        char **_out_val);

/**
 * @brief Checks whether a URI string starts with an RFC 3986 scheme prefix
 * within `len` bytes.
 *
 * @param[in] uri URI string to inspect.
 * @param[in] len Number of characters to inspect.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if scheme is present, CDD_C_SUCCESS
 * (0) otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t uri_has_scheme_prefix(const char *uri,
                                                        size_t len);

/**
 * @brief Computes the length of the base URI before any `#` fragment.
 *
 * @param[in] uri Input URI string.
 * @param[out] _out_val Length of the base URI portion.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t uri_base_len(const char *uri,
                                               size_t *_out_val);

/**
 * @brief Computes the length of the scheme prefix in a URI.
 *
 * @param[in] uri Input URI string.
 * @param[in] len Length of URI to inspect.
 * @param[out] _out_val Length of scheme prefix, or 0 if none.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t uri_scheme_len(const char *uri, size_t len,
                                                 size_t *_out_val);

/**
 * @brief Duplicates a substring of length `len`.
 *
 * @param[in] src Source string.
 * @param[in] len Number of bytes to copy.
 * @param[out] _out_val Newly allocated null-terminated string.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t dup_substr(const char *src, size_t len,
                                             char **_out_val);

/**
 * @brief Normalizes dot-segments (`.` and `..`) in a URI path.
 *
 * @param[in] path Input URI path string.
 * @param[out] _out_val Newly allocated normalized path string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t normalize_path(const char *path,
                                                 char **_out_val);

/**
 * @brief Resolves a relative URI reference against a base URI per RFC 3986.
 *
 * @param[in] base_uri Base URI string.
 * @param[in] ref Relative or absolute URI reference.
 * @param[out] _out_val Newly allocated resolved URI string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t resolve_uri_reference(const char *base_uri,
                                                        const char *ref,
                                                        char **_out_val);

/**
 * @brief Computes the canonical document base URI from `$self` or retrieval
 * URI.
 *
 * @param[in] self_uri Optional `$self` URI declared in the document.
 * @param[in] retrieval_uri Optional retrieval URI where the document was
 * loaded.
 * @param[out] _out_val Newly allocated canonical document URI.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t compute_document_uri(
    const char *self_uri, const char *retrieval_uri, char **_out_val);

/**
 * @brief Checks if a root JSON object contains top-level OpenAPI document
 * fields.
 *
 * @param[in] root_obj Root JSON object.
 * @return Non-zero if OpenAPI document fields are present, 0 otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
root_has_openapi_fields(const JSON_Object *root_obj);

/**
 * @brief Resolves a `$ref` against a specification and its document registry.
 *
 * @param[in] spec Source OpenAPI specification.
 * @param[in] ref Reference string to resolve.
 * @param[out] _out_val Resolved target specification and reference structure.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
resolve_ref_target(const struct OpenAPI_Spec *spec, const char *ref,
                   struct ResolvedRefTarget *_out_val);

/**
 * @brief Checks if a `$ref`'s base URI matches the specification's `$self` or
 * `document_uri`.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] ref Full reference string.
 * @param[in] hash Pointer to the `#` character within `ref`.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if base matches self, CDD_C_SUCCESS
 * (0) otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t ref_base_matches_self(
    const struct OpenAPI_Spec *spec, const char *ref, const char *hash);

/**
 * @brief Extracts the trailing component name from a `$ref` matching a prefix.
 *
 * @param[in] spec OpenAPI specification context.
 * @param[in] ref Reference string to match.
 * @param[in] prefix Component path prefix (e.g. `"#/components/schemas/"`).
 * @param[out] _out_val Pointer into `ref` after the prefix, or NULL if not
 * matching.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
extern C_CDD_EXPORT cdd_c_error_t
ref_name_from_prefix(const struct OpenAPI_Spec *spec, const char *ref,
                     const char *prefix, const char **_out_val);

/**
 * @brief Checks if a URL string contains a query (`?`) or fragment (`#`).
 *
 * @param[in] url URL string to inspect.
 * @return CDD_C_ERROR_UNKNOWN (non-zero) if query or fragment is present,
 * CDD_C_SUCCESS (0) otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t url_has_query_or_fragment(const char *url);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENAPI_URI_H */
