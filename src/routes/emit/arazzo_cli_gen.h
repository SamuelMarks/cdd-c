/**
 * @file arazzo_cli_gen.h
 * @brief Frontend SDK CLI Workflow Runner Generator from Arazzo v1.1.0
 * Specifications.
 *
 * Generates a standalone C CLI runner (`<base>_workflow_cli.c`) that
 * `#include`s
 * `<base>_workflows.h` and directly invokes the generated C SDK workflow
 * functions (`<prefix>workflow_<workflowId>`).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ARAZZO_CLI_GEN_H
/** @brief Header guard for arazzo_cli_gen.h */
#define C_CDD_ARAZZO_CLI_GEN_H

/* clang-format off */
#include "arazzo/parse/arazzo_types.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "c_cdd/no_discard.h"
#include "openapi/parse/openapi_types.h"
#include "routes/emit/client_gen.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Generates the C SDK workflow files and the CLI workflow runner source
 * file (`<base>_workflow_cli.c`) from an Arazzo v1.1.0 specification.
 *
 * @param[in] arazzo_spec Parsed Arazzo v1.1.0 specification.
 * @param[in] openapi_spec Optional linked OpenAPI specification (may be NULL).
 * @param[in] config Client generation configuration (`filename_base`,
 * `func_prefix`, etc.).
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on NULL,
 * `CDD_C_ERROR_MEMORY` on OOM, or `CDD_C_ERROR_IO` on file failure.
 */
C_CDD_EXPORT cdd_c_error_t
arazzo_cli_generate(const struct Arazzo_Spec *arazzo_spec,
                    const struct OpenAPI_Spec *openapi_spec,
                    const struct OpenApiClientConfig *config);

/**
 * @brief Renders the CLI workflow runner C source into an in-memory string.
 *
 * @param[in] arazzo_spec Parsed Arazzo v1.1.0 specification.
 * @param[in] base_name Base filename without directory.
 * @param[in] prefix Function name prefix (e.g. `"api_"` or `""`).
 * @param[out] out_cli_src Output heap-allocated CLI source string.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_cli_render_to_string(
    const struct Arazzo_Spec *arazzo_spec, const char *base_name,
    const char *prefix, char **out_cli_src);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ARAZZO_CLI_GEN_H */
