/**
 * @file arazzo_server_gen.h
 * @brief Backend Server Orchestrator, Stateful Mock Runner & E2E `greatest.h`
 * Test Suite Generator from Arazzo v1.1.0 Specifications.
 *
 * Generates `<base>_workflow_server.c` containing composite server workflow
 * route handlers (`POST /workflows/<workflowId>`) on `c_rest_router` and
 * `<base>_workflow_tests.h` containing `greatest.h` E2E workflow tests
 * (`TEST test_workflow_<workflowId>(void)`).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ARAZZO_SERVER_GEN_H
/** @brief Header guard for arazzo_server_gen.h */
#define C_CDD_ARAZZO_SERVER_GEN_H

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
 * @brief Generates the backend server workflow orchestrator
 * (`<base>_workflow_server.c`) and `greatest.h` E2E workflow test suite
 * (`<base>_workflow_tests.h`) from an Arazzo v1.1.0 specification.
 *
 * @param[in] arazzo_spec Parsed Arazzo v1.1.0 specification.
 * @param[in] openapi_spec Optional linked OpenAPI specification (may be NULL).
 * @param[in] config Server/client generation configuration.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on NULL,
 * `CDD_C_ERROR_MEMORY` on OOM, or `CDD_C_ERROR_IO` on file failure.
 */
C_CDD_EXPORT cdd_c_error_t
arazzo_server_generate(const struct Arazzo_Spec *arazzo_spec,
                       const struct OpenAPI_Spec *openapi_spec,
                       const struct OpenApiClientConfig *config);

/**
 * @brief Renders the backend server workflow orchestrator C source and E2E
 * `greatest.h` test header into in-memory strings.
 *
 * @param[in] arazzo_spec Parsed Arazzo v1.1.0 specification.
 * @param[in] base_name Base filename without directory.
 * @param[in] prefix Function name prefix (e.g. `"api_"` or `""`).
 * @param[out] out_server_src Output heap-allocated server C source string.
 * @param[out] out_test_hdr Output heap-allocated `greatest.h` test header
 * string.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_server_render_to_strings(
    const struct Arazzo_Spec *arazzo_spec, const char *base_name,
    const char *prefix, char **out_server_src, char **out_test_hdr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ARAZZO_SERVER_GEN_H */
