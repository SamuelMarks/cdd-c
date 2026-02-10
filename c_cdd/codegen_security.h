/**
 * @file codegen_security.h
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

#include <stdio.h>

#include "c_cdd_export.h"
#include "openapi_loader.h"

/**
 * @brief Generate code to apply authentication headers/params.
 *
 * Scans `spec->components.securitySchemes`.
 * Honors root or operation-level `security` requirements when present,
 * otherwise falls back to applying all schemes (legacy behavior).
 *
 * Generates C logic checking `ctx->security` fields and injecting:
 * - `Authorization: Bearer ...` (HTTP Bearer)
 * - `X-Api-Key: ...` (API Key in Header)
 *
 * @param[in] fp Output file stream.
 * @param[in] op The operation context (unused currently, for future scopes).
 * @param[in] spec The full spec containing security definitions.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
codegen_security_write_apply(FILE *fp, const struct OpenAPI_Operation *op,
                             const struct OpenAPI_Spec *spec);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_SECURITY_H */
