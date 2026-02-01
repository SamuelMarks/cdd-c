/**
 * @file refactor_api_sync.h
 * @brief API Synchronization Engine.
 *
 * Scans C implementation files to find functions corresponding to OpenAPI
 * operations. If the specification changes (e.g., new parameters, renamed
 * routes, changed styles), this module calculates and applies the necessary
 * patches to:
 * 1. The Function Signature (argument types/order).
 * 2. The Query Parameter Construction Block.
 * 3. The URL Construction Block.
 * 4. Header Parameter logic (inserted or updated).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_REFACTOR_API_SYNC_H
#define C_CDD_REFACTOR_API_SYNC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "openapi_loader.h"

/**
 * @brief Configuration for API synchronization.
 */
struct ApiSyncConfig {
  const char *func_prefix;  /**< Expected prefix of C functions (e.g. "api_") */
  const char *url_var_name; /**< Variable name used for URL strings in body
                               (default "url") */
};

/**
 * @brief Synchronize a C source file with an OpenAPI
 * specification.
 *
 * Reads `filename`, parses it into a CST, and iterates through
 * all operations defined in `spec`.
 *
 * For each operation found in the source:
 * 1. **Signature Sync**: Updates parameter list to match Spec.
 * 2. **Query Logic Sync**: Identifies existing
 * `url_query_init`...`build` blocks and replaces them with
 * generated logic handling Arrays/Explode. If no block exists but
 * params do, attempts partial insertion.
 * 3. **Header Sync**: Scans for `/* Header Parameter: name */` markers and 
 *    updates the associated logic statements. 
 * 4. **URL Sync**: Updates the `asprintf` call to use correct path variables. 
 * 
 * @param[in] filename Path to the C source file to update. 
 * @param[in] spec The parsed OpenAPI specification. 
 * @param[in] config Configuration options. 
 * @return 0 on success, error code on failure. 
 */ 
extern C_CDD_EXPORT int api_sync_file(const char *filename, 
                                      const struct OpenAPI_Spec *spec, 
                                      const struct ApiSyncConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_REFACTOR_API_SYNC_H */
