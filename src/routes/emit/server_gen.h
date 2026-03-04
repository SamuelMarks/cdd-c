#ifndef CDD_SERVER_GEN_H
#define CDD_SERVER_GEN_H

#include "c_cddConfig.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/client_gen.h" /* For OpenApiClientConfig */

C_CDD_EXPORT int
openapi_server_generate(const struct OpenAPI_Spec *spec,
                        const struct OpenApiClientConfig *config);

#endif /* CDD_SERVER_GEN_H */
