/**
 * @file strategy_safety.h
 * @brief Safety Injection Strategies for C code refactoring.
 *
 * Defines policies for injecting error handling checks into existing code.
 * Replaces the hardcoded logic previously embedded in rewriter loops.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_STRATEGY_SAFETY_H
#define C_CDD_STRATEGY_SAFETY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "analysis.h"
#include "text_patcher.h"
#include "tokenizer.h"

/**
 * @brief Inject safety checks for unchecked allocations.
 *
 * Scans the provided allocation sites. If a site is unchecked, it calculates
 * the appropriate insertion point (e.g., after the assignment semicolon)
 * and adds a text patch containing the check logic.
 *
 * Policies:
 * - NULL pointer check: `if (!ptr) { return ENOMEM; }`
 * - Negative Int check: `if (rc < 0) { return ENOMEM; }`
 * - Non-zero Int check: `if (rc != 0) { return ENOMEM; }`
 *
 * @param[in] tokens The token stream of the function body.
 * @param[in] allocs The list of allocation sites identified by analysis.
 * @param[in,out] patches The patch list to populate with injections.
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT int
strategy_inject_safety_checks(const struct TokenList *tokens,
                              const struct AllocationSiteList *allocs,
                              struct PatchList *patches);

/**
 * @brief Inject safety specifically for `realloc` patterns.
 *
 * Handles the pattern `p = realloc(p, size)` by rewriting it to:
 * `{ void *tmp = realloc(p, size); if (!tmp) return ENOMEM; p = tmp; }`
 * preventing memory leaks on failure.
 *
 * @param[in] tokens Token stream.
 * @param[in] site The specific allocation site to process.
 * @param[in] semi_idx Index of the semicolon terminating the statement.
 * @param[in,out] patches Patch list.
 * @return 0 on success (or if pattern doesn't match), ENOMEM on failure.
 */
extern C_CDD_EXPORT int
strategy_rewrite_realloc(const struct TokenList *tokens,
                         const struct AllocationSite *site, size_t semi_idx,
                         struct PatchList *patches);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_STRATEGY_SAFETY_H */
