/**
 * @file diff.h
 * @brief Simple Unified Diff generator for PatchLists.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_DIFF_H
#define C_CDD_DIFF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "functions/emit/patcher.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Generate a Unified Diff from a list of patches.
 *
 * Produces a standard unified diff format string (.patch) that can be applied
 * using the `patch` utility.
 *
 * @param[in] list The patch list (will be sorted internally).
 * @param[in] tokens The original token stream.
 * @param[in] filename The name of the file to put in the diff header.
 * @param[out] out_diff Pointer to a char* where the diff string will be stored.
 * @return 0 on success, ENOMEM or EINVAL on error.
 */
extern C_CDD_EXPORT cdd_c_error_t
patch_list_to_diff(struct PatchList *list, const struct TokenList *tokens,
                   const char *filename, char **out_diff);

#ifdef CDD_BUILD_TESTS
/**
 * @brief DiffLine struct representing a line in a diff
 */
struct DiffLine {
  /** @brief Pointer to line text */
  const char *text;
  /** @brief Length of line */
  size_t len;
};

/**
 * @brief Test helper to call find_line_for_token directly.
 *
 * @param[in] tok Pointer to token.
 * @param[in] old_lines Array of diff lines.
 * @param[in] old_line_count Number of lines in old_lines.
 * @param[out] out_line Pointer to store 1-based line number.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_line_for_token(
    const struct Token *tok, const struct DiffLine *old_lines,
    size_t old_line_count, size_t *out_line);

/**
 * @brief Test helper to call split_lines directly.
 *
 * @param[in] str Input string.
 * @param[in] len Length of input string.
 * @param[out] out_lines Pointer to receive allocated array of DiffLine.
 * @param[out] out_count Pointer to receive number of lines.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_split_lines(const char *str, size_t len, struct DiffLine **out_lines,
                     size_t *out_count);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_DIFF_H */
