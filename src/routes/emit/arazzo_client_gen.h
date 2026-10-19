/**
 * @file arazzo_client_gen.h
 * @brief Frontend C SDK Workflow Generator from Arazzo v1.1.0 Specifications.
 *
 * Generates `<base>_workflows.h` and `<base>_workflows.c` containing strongly
 * typed workflow client contexts, workflow input/output structures, state
 * machine step execution functions (`<prefix>workflow_<workflowId>`), retry
 * loops, criteria assertions, and round-trip Doxygen `@workflow`/`@step`
 * annotations.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ARAZZO_CLIENT_GEN_H
/** @brief Header guard for arazzo_client_gen.h */
#define C_CDD_ARAZZO_CLIENT_GEN_H

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
 * @brief Generates C Client SDK workflow header and implementation files
 * (`<base>_workflows.h` and `<base>_workflows.c`) from an Arazzo v1.1.0
 * specification.
 *
 * @param[in] arazzo_spec Parsed Arazzo v1.1.0 specification.
 * @param[in] openapi_spec Optional linked OpenAPI specification (may be NULL).
 * @param[in] config Client generation configuration (`filename_base`,
 * `func_prefix`, etc.).
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on NULL,
 * `CDD_C_ERROR_MEMORY` on OOM, or `CDD_C_ERROR_IO` on file failure.
 */
C_CDD_EXPORT cdd_c_error_t
arazzo_client_generate(const struct Arazzo_Spec *arazzo_spec,
                       const struct OpenAPI_Spec *openapi_spec,
                       const struct OpenApiClientConfig *config);

/**
 * @brief Renders the C Client SDK workflow header (`_workflows.h`) and source
 * (`_workflows.c`) into in-memory strings.
 *
 * @param[in] arazzo_spec Parsed Arazzo v1.1.0 specification.
 * @param[in] openapi_spec Optional linked OpenAPI specification (may be NULL).
 * @param[in] base_name Base filename without directory.
 * @param[in] prefix Function name prefix (e.g. `"api_"` or `""`).
 * @param[out] out_hdr Output heap-allocated header content string.
 * @param[out] out_src Output heap-allocated source content string.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_client_render_to_strings(
    const struct Arazzo_Spec *arazzo_spec,
    const struct OpenAPI_Spec *openapi_spec, const char *base_name,
    const char *prefix, char **out_hdr, char **out_src);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ARAZZO_CLIENT_GEN_H */
