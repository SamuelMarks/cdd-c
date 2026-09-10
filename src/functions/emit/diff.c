/**
 * @file diff.c
 * @brief Implementation of Unified Diff generator.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/format_specifiers.h"
#include "c_cdd/memory.h"
#include "functions/emit/diff.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_find_line_for_token;
extern C_CDD_EXPORT int g_cdd_fail_append_to_diff;
#endif

/** @brief Block struct */
struct Block {
  /** @brief patch start index */
  size_t patch_start_idx;
  /** @brief patch end index */
  size_t patch_end_idx;
  /** @brief old start line */
  size_t old_start_line;
  /** @brief old end line */
  size_t old_end_line;
};

/**
 * @brief Splits a string into lines.
 *
 * @param[in] str Input string.
 * @param[in] len Length of input string.
 * @param[out] out_lines Pointer to receive allocated array of DiffLine.
 * @param[out] out_count Pointer to receive number of lines.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t split_lines(const char *str, size_t len,
                                 struct DiffLine **out_lines,
                                 size_t *out_count) {
  size_t count = 0;
  size_t i;
  size_t line_idx = 0;
  size_t start = 0;

  if (len == 0) {
    *out_lines = NULL;
    *out_count = 0;
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < len; i++) {
    if (str[i] == '\n')
      count++;
  }
  if (str[len - 1] != '\n')
    count++;

  *out_lines = (struct DiffLine *)C_CDD_MALLOC(count * sizeof(struct DiffLine));
  if (!*out_lines) {
    *out_count = 0;
    return CDD_C_ERROR_MEMORY;
  }

  *out_count = count;

  for (i = 0; i < len; i++) {
    if (str[i] == '\n') {
      (*out_lines)[line_idx].text = str + start;
      (*out_lines)[line_idx].len = (i - start) + 1;
      line_idx++;
      start = i + 1;
    }
  }
  if (start < len) {
    (*out_lines)[line_idx].text = str + start;
    (*out_lines)[line_idx].len = len - start;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates block new text.
 *
 * @param[in] b Pointer to Block.
 * @param[in] list Pointer to PatchList.
 * @param[in] tokens Pointer to TokenList.
 * @param[in] old_lines Pointer to array of DiffLine.
 * @param[out] out_text Pointer to receive newly allocated string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t generate_block_new_text(const struct Block *b,
                                             struct PatchList *list,
                                             const struct TokenList *tokens,
                                             const struct DiffLine *old_lines,
                                             char **out_text) {
  const char *block_start_ptr = old_lines[b->old_start_line - 1].text;
  const char *block_end_ptr =
      old_lines[b->old_end_line - 1].text + old_lines[b->old_end_line - 1].len;
  size_t est_cap = (size_t)(block_end_ptr - block_start_ptr) + 1;
  char *res = (char *)C_CDD_MALLOC(est_cap);
  size_t res_len = 0;
  const char *cursor = block_start_ptr;
  size_t p;

  if (!res) {
    *out_text = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  for (p = b->patch_start_idx; p < b->patch_end_idx; p++) {
    struct Patch *patch = &list->patches[p];
    const char *patch_start_ptr =
        (const char *)tokens->tokens[patch->start_token_idx].start;
    const char *patch_end_ptr;

    if (patch->end_token_idx > patch->start_token_idx) {
      patch_end_ptr =
          (const char *)(tokens->tokens[patch->end_token_idx - 1].start +
                         tokens->tokens[patch->end_token_idx - 1].length);
    } else {
      patch_end_ptr = patch_start_ptr;
    }

    if (patch_start_ptr > cursor) {
      size_t unchanged_len = (size_t)(patch_start_ptr - cursor);
      memcpy(res + res_len, cursor, unchanged_len);
      res_len += unchanged_len;
    }

    if (patch->text) {
      size_t ptext_len = strlen(patch->text);
      if (res_len + ptext_len + (size_t)(block_end_ptr - patch_end_ptr) + 1 >
          est_cap) {
        char *new_res;
        est_cap =
            res_len + ptext_len + (size_t)(block_end_ptr - patch_end_ptr) + 64;
        new_res = (char *)C_CDD_REALLOC(res, est_cap);
        if (!new_res) {
          C_CDD_FREE(res);
          *out_text = NULL;
          return CDD_C_ERROR_MEMORY;
        }
        res = new_res;
      }
      memcpy(res + res_len, patch->text, ptext_len);
      res_len += ptext_len;
    }

    cursor = patch_end_ptr;
  }

  if (block_end_ptr > cursor) {
    size_t rem = (size_t)(block_end_ptr - cursor);
    memcpy(res + res_len, cursor, rem);
    res_len += rem;
  }

  res[res_len] = '\0';
  *out_text = res;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the append to diff operation.
 *
 * @param[in,out] diff_str Pointer to diff string pointer.
 * @param[in,out] diff_len Pointer to diff string length.
 * @param[in,out] diff_cap Pointer to diff string capacity.
 * @param[in] format Format string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 4, 5)))
#endif
static cdd_c_error_t
append_to_diff(char **diff_str, size_t *diff_len, size_t *diff_cap,
               const char *format, ...) {
  va_list args;
  int printed;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_append_to_diff && --g_cdd_fail_append_to_diff == 0)
    return CDD_C_ERROR_MEMORY;
#endif

  if (!*diff_str) {
    *diff_cap = 16;
    *diff_str = (char *)C_CDD_MALLOC(*diff_cap);
    if (!*diff_str)
      return CDD_C_ERROR_MEMORY;
    (*diff_str)[0] = '\0';
    *diff_len = 0;
  }

  va_start(args, format);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  printed = _vscprintf(format, args);
#else
  printed = vsnprintf(NULL, 0, format, args);
#endif
  va_end(args);

  if (*diff_len + (size_t)printed + 1 > *diff_cap) {
    char *new_str;
    *diff_cap = *diff_len + (size_t)printed + 128;
    new_str = (char *)C_CDD_REALLOC(*diff_str, *diff_cap);
    if (!new_str)
      return CDD_C_ERROR_MEMORY;
    *diff_str = new_str;
  }
  va_start(args, format);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  vsprintf_s(*diff_str + *diff_len, *diff_cap - *diff_len, format, args);
#else
  vsprintf(*diff_str + *diff_len, format, args);
#endif
  va_end(args);
  *diff_len += (size_t)printed;

  return CDD_C_SUCCESS;
}

/**
 * @brief Finds the 1-based line number for a given token.
 *
 * @param[in] tok Pointer to token.
 * @param[in] old_lines Array of diff lines.
 * @param[in] old_line_count Number of lines in old_lines.
 * @param[out] out_line Pointer to store 1-based line number.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t find_line_for_token(const struct Token *tok,
                                         const struct DiffLine *old_lines,
                                         size_t old_line_count,
                                         size_t *out_line) {
  size_t i;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_find_line_for_token && --g_cdd_fail_find_line_for_token == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif

  for (i = 0; i < old_line_count; i++) {
    const char *line_start = old_lines[i].text;
    const char *line_end = line_start + old_lines[i].len;
    if ((const char *)tok->start >= line_start &&
        (const char *)tok->start < line_end) {
      *out_line = i + 1; /* 1-based */
      return CDD_C_SUCCESS;
    }
  }
  *out_line = 1;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
/**
 * @brief Test helper to call find_line_for_token directly.
 * @param[in] tok Pointer to token.
 * @param[in] old_lines Array of diff lines.
 * @param[in] old_line_count Number of lines in old_lines.
 * @param[out] out_line Pointer to store 1-based line number.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_find_line_for_token(
    const struct Token *tok, const struct DiffLine *old_lines,
    size_t old_line_count, size_t *out_line) {
  return find_line_for_token(tok, old_lines, old_line_count, out_line);
}

/**
 * @brief Test helper to call split_lines directly.
 * @param[in] str Input string.
 * @param[in] len Length of input string.
 * @param[out] out_lines Pointer to receive allocated array of DiffLine.
 * @param[out] out_count Pointer to receive number of lines.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_test_split_lines(const char *str, size_t len,
                                                struct DiffLine **out_lines,
                                                size_t *out_count) {
  return split_lines(str, len, out_lines, out_count);
}
#endif

/**
 * @brief Executes the patch list to diff operation.
 *
 * @param[in] list The patch list (will be sorted internally).
 * @param[in] tokens The original token stream.
 * @param[in] filename The name of the file to put in the diff header.
 * @param[out] out_diff Pointer to a char* where the diff string will be stored.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t patch_list_to_diff(struct PatchList *list,
                                 const struct TokenList *tokens,
                                 const char *filename, char **out_diff) {
  struct DiffLine *old_lines = NULL;
  size_t old_line_count = 0;
  const char *orig_src;
  size_t orig_len;
  struct Block *blocks = NULL;
  size_t block_count = 0;
  size_t block_cap = 0;
  char *diff_str = NULL;
  size_t diff_len = 0;
  size_t diff_cap = 0;
  size_t p;
  size_t current_line_delta = 0;
  char *new_text = NULL;
  struct DiffLine *new_lines = NULL;
  cdd_c_error_t rc;

  if (!list || !tokens || !out_diff)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (list->size == 0 || tokens->size == 0) {
    *out_diff = (char *)C_CDD_MALLOC(1);
    if (!*out_diff)
      return CDD_C_ERROR_MEMORY;
    (*out_diff)[0] = '\0';
    return CDD_C_SUCCESS;
  }

  rc = patch_list_sort(list);
  if (rc != CDD_C_SUCCESS)
    return rc;

  orig_src = (const char *)tokens->tokens[0].start;
  orig_len = (size_t)((tokens->tokens[tokens->size - 1].start +
                       tokens->tokens[tokens->size - 1].length) -
                      (const uint8_t *)orig_src);

  rc = split_lines(orig_src, orig_len, &old_lines, &old_line_count);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  /* Build blocks */
  for (p = 0; p < list->size; p++) {
    struct Patch *patch = &list->patches[p];
    size_t p_start;
    size_t p_end;
    size_t ctx_start, ctx_end;

    rc = find_line_for_token(&tokens->tokens[patch->start_token_idx], old_lines,
                             old_line_count, &p_start);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    if (patch->end_token_idx > patch->start_token_idx) {
      rc = find_line_for_token(&tokens->tokens[patch->end_token_idx - 1],
                               old_lines, old_line_count, &p_end);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else {
      p_end = p_start;
    }

    ctx_start = (p_start > 3) ? p_start - 3 : 1;
    ctx_end = (p_end + 3 <= old_line_count) ? p_end + 3 : old_line_count;

    if (block_count > 0 && ctx_start <= blocks[block_count - 1].old_end_line) {
      if (ctx_end > blocks[block_count - 1].old_end_line)
        blocks[block_count - 1].old_end_line = ctx_end;
      blocks[block_count - 1].patch_end_idx = p + 1;
    } else {
      if (block_count >= block_cap) {
        struct Block *new_blocks;
        block_cap = (block_cap == 0) ? 2 : block_cap * 2;
        new_blocks = (struct Block *)C_CDD_REALLOC(
            blocks, block_cap * sizeof(struct Block));
        if (!new_blocks) {
          rc = CDD_C_ERROR_MEMORY;
          goto cleanup;
        }
        blocks = new_blocks;
      }
      blocks[block_count].patch_start_idx = p;
      blocks[block_count].patch_end_idx = p + 1;
      blocks[block_count].old_start_line = ctx_start;
      blocks[block_count].old_end_line = ctx_end;
      block_count++;
    }
  }

  rc = append_to_diff(&diff_str, &diff_len, &diff_cap, "--- %s\n+++ %s\n",
                      filename ? filename : "", filename ? filename : "");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (p = 0; p < block_count; p++) {
    struct Block *b = &blocks[p];
    size_t new_line_count = 0;
    size_t i;
    size_t min_mod_line, max_mod_line, keep_start_count, keep_end_count;

    rc = generate_block_new_text(b, list, tokens, old_lines, &new_text);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    rc = split_lines(new_text, strlen(new_text), &new_lines, &new_line_count);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    rc = append_to_diff(
        &diff_str, &diff_len, &diff_cap, "@@ -%lu,%lu +%lu,%lu @@\n",
        (unsigned long)b->old_start_line,
        (unsigned long)(b->old_end_line - b->old_start_line + 1),
        (unsigned long)(b->old_start_line + current_line_delta),
        (unsigned long)new_line_count);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    rc = find_line_for_token(
        &tokens->tokens[list->patches[b->patch_start_idx].start_token_idx],
        old_lines, old_line_count, &min_mod_line);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    if (list->patches[b->patch_end_idx - 1].end_token_idx > 0) {
      rc = find_line_for_token(
          &tokens
               ->tokens[list->patches[b->patch_end_idx - 1].end_token_idx - 1],
          old_lines, old_line_count, &max_mod_line);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else {
      rc = find_line_for_token(&tokens->tokens[0], old_lines, old_line_count,
                               &max_mod_line);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    if (max_mod_line < min_mod_line)
      max_mod_line = min_mod_line;

    keep_start_count = (min_mod_line > b->old_start_line)
                           ? min_mod_line - b->old_start_line
                           : 0;
    keep_end_count =
        (b->old_end_line > max_mod_line) ? b->old_end_line - max_mod_line : 0;

    for (i = b->old_start_line; i < min_mod_line && i <= b->old_end_line; i++) {
      rc = append_to_diff(&diff_str, &diff_len, &diff_cap, " %.*s",
                          (int)old_lines[i - 1].len, old_lines[i - 1].text);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    for (i = min_mod_line; i <= max_mod_line && i <= b->old_end_line; i++) {
      rc = append_to_diff(&diff_str, &diff_len, &diff_cap, "-%.*s",
                          (int)old_lines[i - 1].len, old_lines[i - 1].text);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      if (old_lines[i - 1].text[old_lines[i - 1].len - 1] != '\n') {
        rc = append_to_diff(&diff_str, &diff_len, &diff_cap,
                            "\n\\ No newline at end of file\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }

    for (i = keep_start_count; i < new_line_count - keep_end_count; i++) {
      rc = append_to_diff(&diff_str, &diff_len, &diff_cap, "+%.*s",
                          (int)new_lines[i].len, new_lines[i].text);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      if (new_lines[i].text[new_lines[i].len - 1] != '\n') {
        rc = append_to_diff(&diff_str, &diff_len, &diff_cap,
                            "\n\\ No newline at end of file\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }

    for (i = b->old_end_line - keep_end_count + 1; i <= b->old_end_line; i++) {
      rc = append_to_diff(&diff_str, &diff_len, &diff_cap, " %.*s",
                          (int)old_lines[i - 1].len, old_lines[i - 1].text);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      if (old_lines[i - 1].text[old_lines[i - 1].len - 1] != '\n') {
        rc = append_to_diff(&diff_str, &diff_len, &diff_cap,
                            "\n\\ No newline at end of file\n");
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }

    current_line_delta +=
        new_line_count - (b->old_end_line - b->old_start_line + 1);

    C_CDD_FREE(new_text);
    new_text = NULL;
    C_CDD_FREE(new_lines);
    new_lines = NULL;
  }

cleanup:
  if (new_text)
    C_CDD_FREE(new_text);
  if (new_lines)
    C_CDD_FREE(new_lines);
  if (old_lines)
    C_CDD_FREE(old_lines);
  if (blocks)
    C_CDD_FREE(blocks);

  if (rc != CDD_C_SUCCESS) {
    if (diff_str)
      C_CDD_FREE(diff_str);
    *out_diff = NULL;
    return rc;
  }

  *out_diff = diff_str;
  return CDD_C_SUCCESS;
}
