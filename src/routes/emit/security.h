/**
 * @file security.h
 * @brief Security Logic Generator for API Clients.
 *
 * Provides functionality to generate C code that applies authentication
 * credentials (API Keys, Bearer Tokens) to HTTP requests based on the
 * OpenAPI Security Schemes defined in the specification.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_SECURITY_H
#define C_CDD_CODEGEN_SECURITY_H

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
 * @brief Generate code to apply authentication headers/params.
 *
 * Scans `spec->components.securitySchemes`.
 * Honors root or operation-level `security` requirements when present,
 * otherwise falls back to applying all schemes (legacy behavior).
 *
 * Generates C logic checking `ctx->security` fields and injecting:
 * - `Authorization: Bearer ...` (HTTP Bearer, OAuth2, OpenID Connect)
 * - `Authorization: Basic ...` (HTTP Basic; token is base64 of "user:pass")
 * - `X-Api-Key: ...` (API Key in Header)
 * - Query params for `apiKey` in query
 * - Cookie params for `apiKey` in cookie
 *
 * @param[in] fp Output file stream.
 * @param[in] op The operation context (unused currently, for future scopes).
 * @param[in] spec The full spec containing security definitions.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
codegen_security_write_apply(FILE *fp, const struct OpenAPI_Operation *op,
                             const struct OpenAPI_Spec *spec);

/**
 * @brief Determine whether the active security schemes require query params.
 *
 * @param[in] op The operation context.
 * @param[in] spec The full OpenAPI specification.
 * @return Non-zero if query security params are required, CDD_C_SUCCESS
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t codegen_security_requires_query(
    const struct OpenAPI_Operation *op, const struct OpenAPI_Spec *spec);

/**
 * @brief Determine whether the active security schemes require cookie params.
 *
 * @param[in] op The operation context.
 * @param[in] spec The full OpenAPI specification.
 * @return Non-zero if cookie security params are required, CDD_C_SUCCESS
 * otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t codegen_security_requires_cookie(
    const struct OpenAPI_Operation *op, const struct OpenAPI_Spec *spec);

/**
 * @brief Generate middleware check for servers.
 *
 * @param[in,out] fp Output file stream.
 * @param[in] op The operation context.
 * @param[in] spec The full OpenAPI specification.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t codegen_security_write_server_apply(
    FILE *fp, const struct OpenAPI_Operation *op,
    const struct OpenAPI_Spec *spec);

/**
 * @brief Checks if a URI string begins with a scheme prefix (`scheme:`).
 *
 * @param[in] uri URI string to inspect.
 * @param[in] len Length of the prefix to inspect.
 * @return Non-zero if a scheme prefix is present, 0 otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t uri_has_scheme_prefix_test(const char *uri,
                                                             size_t len);

/**
 * @brief Checks if a reference base URI matches the specification `$self` URI.
 *
 * @param[in] self_uri Specification `$self` URI.
 * @param[in] ref Reference string to compare.
 * @param[in] base_len Length of the reference base portion.
 * @return Non-zero if matching, 0 otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t ref_base_matches_self_uri_test(
    const char *self_uri, const char *ref, size_t base_len);

/**
 * @brief Checks if a security requirement scheme reference matches a named
 * security scheme.
 *
 * @param[in] req_scheme Requirement scheme name or `$ref`.
 * @param[in] scheme_name Defined security scheme name.
 * @param[in] spec OpenAPI specification context.
 * @return Non-zero if matching, 0 otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t
scheme_ref_matches_name_test(const char *req_scheme, const char *scheme_name,
                             const struct OpenAPI_Spec *spec);

/**
 * @brief Checks if a named security scheme is referenced in an array of
 * security requirement sets.
 *
 * @param[in] sets Array of security requirement sets.
 * @param[in] n_sets Number of requirement sets.
 * @param[in] scheme_name Security scheme name to search for.
 * @param[in] spec OpenAPI specification context.
 * @return Non-zero if present, 0 otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t scheme_in_security_sets_test(
    const struct OpenAPI_SecurityRequirementSet *sets, size_t n_sets,
    const char *scheme_name, const struct OpenAPI_Spec *spec);

/**
 * @brief Resolves the active security requirement sets for an operation.
 *
 * @param[in] op Operation context (may be NULL).
 * @param[in] spec OpenAPI specification context.
 * @param[out] out_sets Output pointer to the active requirement sets.
 * @param[out] out_count Output count of requirement sets.
 * @param[out] out_set_flag Output flag indicating whether security was
 * explicitly set.
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t resolve_active_security_test(
    const struct OpenAPI_Operation *op, const struct OpenAPI_Spec *spec,
    const struct OpenAPI_SecurityRequirementSet **out_sets, size_t *out_count,
    int *out_set_flag);

/**
 * @brief Determines if a security scheme is active for the resolved security
 * sets.
 *
 * @param[in] sch Security scheme definition.
 * @param[in] sets Active security requirement sets.
 * @param[in] n_sets Number of active security requirement sets.
 * @param[in] security_set Non-zero if security requirements were explicitly
 * specified.
 * @param[in] spec OpenAPI specification context.
 * @return Non-zero if active, 0 otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t scheme_is_active_test(
    const struct OpenAPI_SecurityScheme *sch,
    const struct OpenAPI_SecurityRequirementSet *sets, size_t n_sets,
    int security_set, const struct OpenAPI_Spec *spec);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_SECURITY_H */
