#ifndef C_CDD_ROUTES_EMIT_CLIENT_GUI_GEN_H
#define C_CDD_ROUTES_EMIT_CLIENT_GUI_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
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
extern C_CDD_EXPORT int
openapi_client_gui_generate(const struct OpenAPI_Spec *spec,
                            const struct OpenApiClientConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ROUTES_EMIT_CLIENT_GUI_GEN_H */
