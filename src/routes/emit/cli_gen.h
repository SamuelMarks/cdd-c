#ifndef C_CDD_OPENAPI_CLI_GEN_H
#define C_CDD_OPENAPI_CLI_GEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c_cdd_export.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/client_gen.h"

extern C_CDD_EXPORT int
openapi_cli_generate(const struct OpenAPI_Spec *spec,
                     const struct OpenApiClientConfig *config);

#ifdef __cplusplus
}
#endif

#endif
