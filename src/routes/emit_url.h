/**
 * @file codegen_url.h
 * @brief URL Generation Logic for Client Code.
 *
 * Provides functionality to:
 * - Construct path URL string.
 * - Construct path URL string with matrix/label/simple styles for path params,
 *   including object params provided as `struct OpenAPI_KV` pairs.
 * - Generate Query Parameter block logic (including Arrays and Explode style).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_URL_H
#define C_CDD_CODEGEN_URL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include "c_cdd_export.h"
#include "routes/parse_openapi.h"

/**
 * @brief Configuration for URL code generation.
 */
struct CodegenUrlConfig {
  const char *base_variable; /**< Variable name for base URL (default
                                "ctx->base_url") */
  const char
      *out_variable; /**< Variable name for the output char* (default "url") */
};

/**
 * @brief Generate C code to construct a URL from a path template.
 *
 * @param[in] fp The file stream to write C code to.
 * @param[in] path_template The raw OpenAPI route string (e.g. "/users/{id}").
 * @param[in] params Array of operation parameters to look up types.
 * @param[in] n_params param count.
 * @param[in] config Configuration options (can be NULL for defaults).
 * @return 0 on success, error code (EINVAL/ENOMEM) on failure.
 */
extern C_CDD_EXPORT int codegen_url_write_builder(
    FILE *fp, const char *path_template, const struct OpenAPI_Parameter *params,
    size_t n_params, const struct CodegenUrlConfig *config);

/**
 * @brief Generate C code for adding query parameters to the URL builder.
 *
 * Handles scalar values (by mapping to `url_query_add`) and Array values.
 * For Arrays with `style=form, explode=true`, generates a `for` loop emitting
 * multiple key-value pairs (e.g. `?id=1&id=2`). For `style=form,
 * explode=false`, generates a comma-separated value and uses
 * `url_query_add_encoded` to preserve the delimiter. Also supports
 * `spaceDelimited` and `pipeDelimited` array styles. Object parameters in
 * `style=form` and `style=deepObject` are serialized from
 * `struct OpenAPI_KV` pairs.
 *
 * @param[in] fp The file stream to write to.
 * @param[in] op The operation containing parameters.
 * @param[in] qp_tracking Non-zero if `qp_initialized` is available in scope.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
codegen_url_write_query_params(FILE *fp, const struct OpenAPI_Operation *op,
                               int qp_tracking);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_URL_H */
