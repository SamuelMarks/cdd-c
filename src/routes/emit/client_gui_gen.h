/**
 * @file client_gui_gen.h
 * @brief Core client gui gen definitions and implementation.
 */
#ifndef C_CDD_ROUTES_EMIT_CLIENT_GUI_GEN_H
#define C_CDD_ROUTES_EMIT_CLIENT_GUI_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <stdio.h>
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/client_gen.h"
/* clang-format on */

/**
 * @brief Writes the generated OAuth2 GUI header and source contents to open
 * streams.
 *
 * @param[in,out] fp_h Open header output stream.
 * @param[in,out] fp_c Open C source output stream.
 * @param[in] base Base filename prefix.
 * @param[in] default_server Default server base URL.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` on NULL
 * inputs, or `CDD_C_ERROR_IO` on stream write failure.
 */
extern C_CDD_EXPORT cdd_c_error_t openapi_client_gui_write_streams(
    FILE *fp_h, FILE *fp_c, const char *base, const char *default_server);

/**
 * @brief Generate a c-multiplatform OAuth2 GUI and Token Flow logic.
 *
 * This generates the GUI code that pre-fills the API URL and available scopes
 * from the OpenAPI spec. It also generates the automated request-handling logic
 * for the 'password' flow object to fetch access tokens.
 *
 * @param[in] spec The parsed OpenAPI specification.
 * @param[in] config Configuration options for the client.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t openapi_client_gui_generate(
    const struct OpenAPI_Spec *spec, const struct OpenApiClientConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ROUTES_EMIT_CLIENT_GUI_GEN_H */
