/**
 * @file codegen_client_body.h
 * @brief Client Function Body Generator Interface.
 *
 * Orchestrates the generation of the full implementation of an API client
 * function. Supports parameter serialization (Query, Header, Path), Body
 * serialization (JSON, multipart, form-urlencoded), and Security injection.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_CLIENT_BODY_H
#define C_CDD_CODEGEN_CLIENT_BODY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include "c_cdd_export.h"
#include "openapi_loader.h"

/**
 * @brief Generate the implementation body for a client function.
 *
 * Emits code inside the function braces.
 *
 * Logic Sequence:
 * 1.  **Declarations**: Variables for Request, Response, URL, Body, etc.
 * 2.  **Initialization**: `http_request_init` and sanity checks.
 * 3.  **Security**: Call `codegen_security` to inject Auth headers.
 * 4.  **Header Parameters**: Iterate headers and generate `http_headers_add`.
 * 5.  **Query Parameters**: Iterate query params and generate `url_query_add`.
 * 6.  **Path Construction**: Call `codegen_url` to build the base path.
 * 7.  **URL Assembly**: Concatenate Path + Query String.
 * 8.  **Body Serialization**:
 *     - Detect content type (JSON vs Multipart vs Form).
 *     - Call `_to_json`, build multipart payload, or form-encode.
 * 9.  **Execution**: Call `ctx->send`.
 * 10. **Response Handling**: Switch statement on status code for Success/Error
 * deserialization.
 * 11. **Cleanup**: Free all temporary variables.
 *
 * @param[in] fp The file stream to write to.
 * @param[in] op The operation definition.
 * @param[in] spec The full specification (needed for Security Definitions).
 * @param[in] path_template The raw path string (e.g. "/pets/{id}").
 * @return 0 on success, error code (EIO, etc) on failure.
 */
extern C_CDD_EXPORT int
codegen_client_write_body(FILE *fp, const struct OpenAPI_Operation *op,
                          const struct OpenAPI_Spec *spec,
                          const char *path_template);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_CLIENT_BODY_H */
