/**
 * @file text_patcher.c
 * @brief Implementation of the text patching engine.
 *
 * Handles dynamic array management for patches, sorting strategy,
 * and the reconstruction loop that merges original tokens with patch text.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "str_utils.h" /* For c_cdd_strdup, though we use raw memcpy here mostly */
#include "text_patcher.h"

int patch_list_init(struct PatchList *const list) {
  if (!list)
    return EINVAL;
  list->size = 0;
  list->capacity = 8;
  list->patches = (struct Patch *)calloc(list->capacity, sizeof(struct Patch));
  if (!list->patches)
    return ENOMEM;
  return 0;
}

void patch_list_free(struct PatchList *const list) {
  size_t i;
  if (!list)
    return;
  if (list->patches) {
    for (i = 0; i < list->size; ++i) {
      if (list->patches[i].text) {
        free(list->patches[i].text);
      }
    }
    free(list->patches);
    list->patches = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

int patch_list_add(struct PatchList *const list, const size_t start_idx,
                   const size_t end_idx, char *text) {
  if (!list || !text) {
    if (text)
      free(text); /* Prevent leak on bad args */
    return EINVAL;
  }

  if (list->size >= list->capacity) {
    const size_t new_cap = (list->capacity == 0) ? 8 : list->capacity * 2;
    struct Patch *new_arr =
        (struct Patch *)realloc(list->patches, new_cap * sizeof(struct Patch));
    if (!new_arr) {
      free(text); /* Prevent leak on alloc failure */
      return ENOMEM;
    }
    list->patches = new_arr;
    list->capacity = new_cap;
  }

  list->patches[list->size].start_token_idx = start_idx;
  list->patches[list->size].end_token_idx = end_idx;
  list->patches[list->size].text = text;
  list->size++;

  return 0;
}

/**
 * @brief Comparator for qsort.
 * Sorts primarily by start index.
 */
static int compare_patches(const void *a, const void *b) {
  const struct Patch *pa = (const struct Patch *)a;
  const struct Patch *pb = (const struct Patch *)b;

  if (pa->start_token_idx < pb->start_token_idx)
    return -1;
  if (pa->start_token_idx > pb->start_token_idx)
    return 1;
  return 0;
}

void patch_list_sort(struct PatchList *const list) {
  if (list && list->patches && list->size > 1) {
    qsort(list->patches, list->size, sizeof(struct Patch), compare_patches);
  }
}

int patch_list_apply(struct PatchList *const list,
                     const struct TokenList *const tokens, char **out_code) {
  char *output = NULL;
  size_t current_token = 0;
  size_t patch_idx = 0;
  size_t out_len = 0;
  size_t out_cap = 1024;
  int rc = 0;

  if (!list || !tokens || !out_code)
    return EINVAL;

  /* Ensure patches are ordered so we can iterate linearly */
  patch_list_sort(list);

  output = (char *)malloc(out_cap);
  if (!output)
    return ENOMEM;
  output[0] = '\0';

  while (current_token < tokens->size) {
    /* Check if current token matches the start of the next patch */
    if (patch_idx < list->size &&
        list->patches[patch_idx].start_token_idx == current_token) {
      const struct Patch *p = &list->patches[patch_idx];
      size_t text_len = strlen(p->text);

      /* Ensure capacity */
      while (out_len + text_len + 1 > out_cap) {
        char *tmp;
        out_cap = out_cap * 2 + text_len;
        tmp = (char *)realloc(output, out_cap);
        if (!tmp) {
          rc = ENOMEM;
          goto cleanup;
        }
        output = tmp;
      }

      /* Append Patch Text */
      memcpy(output + out_len, p->text, text_len);
      out_len += text_len;
      output[out_len] = '\0';

      /* Advance token stream past the replaced range */
      /* If insert-only (start == end), we don't consume tokens */
      if (p->end_token_idx > current_token) {
        current_token = p->end_token_idx;
      }

      patch_idx++;

      /* Skip any other patches starting at this exact token to avoid confusion
         or double-application, unless they are distinct insertions.
         (Current logic processes one patch per start index greedily) */
      while (patch_idx < list->size &&
             list->patches[patch_idx].start_token_idx < current_token) {
        patch_idx++; /* Skip overlapped patches */
      }

    } else {
      /* No patch starts here, copy original token content */
      const struct Token *tok = &tokens->tokens[current_token];
      size_t tok_len = tok->length;

      while (out_len + tok_len + 1 > out_cap) {
        char *tmp;
        out_cap = out_cap * 2 + tok_len; /* Ensure growth */
        tmp = (char *)realloc(output, out_cap);
        if (!tmp) {
          rc = ENOMEM;
          goto cleanup;
        }
        output = tmp;
      }

      memcpy(output + out_len, tok->start, tok_len);
      out_len += tok_len;
      output[out_len] = '\0';

      current_token++;
    }
  }

  /* Append any patches that might be appended at the very end of the
   * file/stream */
  while (patch_idx < list->size) {
    if (list->patches[patch_idx].start_token_idx >= tokens->size) {
      const struct Patch *p = &list->patches[patch_idx];
      size_t text_len = strlen(p->text);

      while (out_len + text_len + 1 > out_cap) {
        char *tmp;
        out_cap = out_cap * 2 + text_len;
        tmp = (char *)realloc(output, out_cap);
        if (!tmp) {
          rc = ENOMEM;
          goto cleanup;
        }
        output = tmp;
      }
      memcpy(output + out_len, p->text, text_len);
      out_len += text_len;
      output[out_len] = '\0';
    }
    patch_idx++;
  }

  *out_code = output;
  return 0;

cleanup:
  if (output)
    free(output);
  return rc;
}
