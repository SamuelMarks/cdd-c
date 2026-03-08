/**
 * @file diff_generator.h
 * @brief Unified diff generation for patch tracking.
 */

#ifndef C_CDD_DIFF_GENERATOR_H
#define C_CDD_DIFF_GENERATOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "functions/emit/patcher.h"

/**
 * @brief Generate a unified diff (.patch format) from applied token patches.
 *
 * This function compares the original token stream against the set of proposed
 * token patches and emits a string that represents the differences in the
 * standard unified diff format (used by git diff and patch tools).
 *
 * @param tokens The original token list.
 * @param list The list of proposed token substitutions.
 * @param filename The path or name of the file being patched (used in diff headers).
 * @param out_diff Output pointer to hold the generated string (must be freed by caller).
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT int patch_list_generate_diff(const struct TokenList *tokens,
                                          const struct PatchList *list,
                                          const char *filename,
                                          char **out_diff);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_DIFF_GENERATOR_H */
