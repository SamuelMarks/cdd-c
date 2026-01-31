/**
 * @file text_patcher.h
 * @brief Generic engine for applying text substitutions to token streams.
 *
 * Provides functionality to queue a list of replacements (patches) targeting
 * specific token ranges, and then reconstruct the source code with those
 * patches applied. This separates the mechanism of text manipulation from
 * the policy of refactoring logic.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TEXT_PATCHER_H
#define C_CDD_TEXT_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

#include "c_cdd_export.h"
#include "tokenizer.h"

/**
 * @brief Represents a single substitution operation.
 *
 * A patch replaces the tokens in the range [start_token_idx, end_token_idx)
 * with the string `text`.
 */
struct Patch {
  size_t start_token_idx; /**< Inclusive start index of tokens to remove */
  size_t end_token_idx;   /**< Exclusive end index of tokens to remove */
  char *text;             /**< Null-terminated string to insert */
};

/**
 * @brief Container for a collection of patches.
 */
struct PatchList {
  struct Patch *patches; /**< Array of patch objects */
  size_t size;           /**< Number of patches currently queued */
  size_t capacity;       /**< allocated capacity of the array */
};

/**
 * @brief Initialize a new patch list.
 *
 * @param[out] list Pointer to the list structure to initialize.
 * @return 0 on success, EINVAL if list is NULL, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT int patch_list_init(struct PatchList *list);

/**
 * @brief Free resources associated with a patch list.
 *
 * Frees the internal array and all text strings within patches.
 * Does not free the `struct PatchList` pointer itself.
 *
 * @param[in] list The list to free. Safe to pass NULL.
 */
extern C_CDD_EXPORT void patch_list_free(struct PatchList *list);

/**
 * @brief Add a replacement patch to the list.
 *
 * Takes ownership of the `text` string (it will be freed by `patch_list_free`).
 * If this function fails, `text` is freed immediately to prevent leaks.
 *
 * @param[in,out] list The list to append to.
 * @param[in] start_idx Inclusive index of the first token to replace.
 * @param[in] end_idx Exclusive index of the token ending the range.
 * @param[in] text Malloc'd string to insert.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on bad args.
 */
extern C_CDD_EXPORT int patch_list_add(struct PatchList *list, size_t start_idx,
                                       size_t end_idx, char *text);

/**
 * @brief Apply patches to the token stream and generate new source code.
 *
 * Sorts the patches by start index, then iterates through the token stream.
 * For regions not covered by a patch, the original source text (via token
 * pointers) is copied. For regions covered by a patch, the patch text is
 * inserted.
 *
 * Note: Overlapping patches result in undefined behavior (usually the one
 * starting first or processed first wins, but the engine assumes disjoint
 * ranges).
 *
 * @param[in] list The list of patches to apply.
 * @param[in] tokens The original token stream representing the source code.
 * @param[out] out_code Pointer to a char* where the new code will be allocated.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on bad args.
 */
extern C_CDD_EXPORT int patch_list_apply(struct PatchList *list,
                                         const struct TokenList *tokens,
                                         char **out_code);

/**
 * @brief Helper to sort patches by position.
 * Exposed primarily for advanced usage or verification; `patch_list_apply`
 * calls this internally.
 *
 * @param[in] list The list to sort.
 */
extern C_CDD_EXPORT void patch_list_sort(struct PatchList *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEXT_PATCHER_H */
