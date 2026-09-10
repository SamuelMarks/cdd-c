/**
 * @file sync.h
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

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

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
 * 3. **Header Sync**: Scans for markers like `Header Parameter: name` and
 *    updates the associated logic statements.
 * 4. **URL Sync**: Updates the `asprintf` call to use correct path variables.
 *
 * @param[in] filename Path to the C source file to update.
 * @param[in] spec The parsed OpenAPI specification.
 * @param[in] config Configuration options.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
api_sync_file(const char *filename, const struct OpenAPI_Spec *spec,
              const struct ApiSyncConfig *config);

#ifdef CDD_BUILD_TESTS
#include "functions/emit/patcher.h"
#include "functions/parse/cst.h"
#include "functions/parse/tokenizer.h"

/**
 * @brief Test wrapper for make_cdd_tmpfile.
 * @param[out] out_file Output file pointer.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_sync_make_cdd_tmpfile(FILE **out_file);

/**
 * @brief Test wrapper for generate_expected_sig.
 * @param[in] op Operation.
 * @param[in] cfg Sync config.
 * @param[out] out_val Output string.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sync_generate_expected_sig(
    const struct OpenAPI_Operation *op, const struct ApiSyncConfig *cfg,
    char **out_val);

/**
 * @brief Test wrapper for generate_expected_query.
 * @param[in] op Operation.
 * @param[out] out_val Output string.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sync_generate_expected_query(
    const struct OpenAPI_Operation *op, char **out_val);

/**
 * @brief Test wrapper for generate_expected_header_line.
 * @param[in] p Parameter.
 * @param[out] out_val Output string.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sync_generate_expected_header_line(
    const struct OpenAPI_Parameter *p, char **out_val);

/**
 * @brief Test wrapper for generate_expected_url.
 * @param[in] path Path.
 * @param[in] op Operation.
 * @param[in] cfg Sync config.
 * @param[out] out_val Output string.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sync_generate_expected_url(
    const char *path, const struct OpenAPI_Operation *op,
    const struct ApiSyncConfig *cfg, char **out_val);

/**
 * @brief Test wrapper for find_function_node.
 * @param[in] cst CST list.
 * @param[in] tokens Token list.
 * @param[in] func_name Function name.
 * @param[out] out_val Output node.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sync_find_function_node(
    struct CstNodeList *cst, struct TokenList *tokens, const char *func_name,
    struct CstNode **out_val);

/**
 * @brief Test wrapper for extract_current_sig.
 * @param[in] tokens Token list.
 * @param[in] node Node.
 * @param[out] out_end_idx Output end index.
 * @param[out] out_val Output string.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sync_extract_current_sig(
    struct TokenList *tokens, struct CstNode *node, size_t *out_end_idx,
    char **out_val);

/**
 * @brief Test wrapper for apply_query_sync.
 * @param[in] op Operation.
 * @param[in] tokens Token list.
 * @param[in] node Node.
 * @param[in,out] patches Patches list.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sync_apply_query_sync(
    const struct OpenAPI_Operation *op, struct TokenList *tokens,
    struct CstNode *node, struct PatchList *patches);

/**
 * @brief Test wrapper for apply_header_sync.
 * @param[in] op Operation.
 * @param[in] tokens Token list.
 * @param[in] node Node.
 * @param[in,out] patches Patches list.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sync_apply_header_sync(
    const struct OpenAPI_Operation *op, struct TokenList *tokens,
    struct CstNode *node, struct PatchList *patches);

/**
 * @brief Test wrapper for apply_updates.
 * @param[in] filename Filename.
 * @param[in] tokens Token list.
 * @param[in] cst CST list.
 * @param[in] spec Spec.
 * @param[in] cfg Sync config.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_sync_apply_updates(
    const char *filename, struct TokenList *tokens, struct CstNodeList *cst,
    const struct OpenAPI_Spec *spec, const struct ApiSyncConfig *cfg);
#endif /* CDD_BUILD_TESTS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_REFACTOR_API_SYNC_H */
