/**
 * @file codegen_client_sig.h
 * @brief Logic for generating C Client Function Prototypes.
 *
 * Supports standard types and arrays (pointer + len).
 */

#ifndef C_CDD_CODEGEN_CLIENT_SIG_H
#define C_CDD_CODEGEN_CLIENT_SIG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include "c_cdd_export.h"
#include "openapi_loader.h"

/**
 * @brief Configuration for signature generation.
 */
struct CodegenSigConfig {
  const char *prefix; /**< Prefix for function name (e.g. "api_") */
  const char
      *ctx_type; /**< Type of the context arg (default "struct HttpClient *") */
  int include_semicolon; /**< 1 to append ";\n", 0 for definition start " {\n"
                          */
};

/**
 * @brief Generate a C function prototype for an API operation.
 *
 * @param[in] fp The file stream to write to.
 * @param[in] op The parsed operation definition.
 * @param[in] config Configuration options (can be NULL for defaults).
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
codegen_client_write_signature(FILE *fp, const struct OpenAPI_Operation *op,
                               const struct CodegenSigConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_CLIENT_SIG_H */
