#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#ifndef C_CDD_ROUTES_EMIT_CLIENT_GUI_GEN_H
#define C_CDD_ROUTES_EMIT_CLIENT_GUI_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/client_gen.h"
/* clang-format on */

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

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
