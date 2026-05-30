/**
 * @file diff_generator.c
 * @brief Implementation of CST diff generation.
 */

/* clang-format off */
#include "c_cdd_export.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/diff_generator.h"
#include "c_cdd/log.h"
/* clang-format on */

int patch_list_generate_diff(const struct TokenList *tokens,
                             const struct PatchList *list, const char *filename,
                             char **out_diff) {
  char *diff_buf = NULL;
  size_t diff_cap = 4096;
  size_t diff_len = 0;
  size_t i;

  if (!tokens || !list || !filename || !out_diff)
    return EINVAL;

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    if (g_cdd_fail_alloc == 5555)
      diff_buf = NULL;
    else
      diff_buf = (char *)malloc(diff_cap);
  }
#else
  diff_buf = (char *)malloc(diff_cap);
#endif
  if (!diff_buf) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  diff_buf[0] = '\0';

  if (list->size == 0) {
    *out_diff = diff_buf;
    return 0;
  }

/* Print header */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  diff_len += sprintf_s(diff_buf + diff_len, diff_cap - diff_len,
                        "--- a/%s\n+++ b/%s\n", filename, filename);
#else
  diff_len +=
      sprintf(diff_buf + diff_len, "--- a/%s\n+++ b/%s\n", filename, filename);
#endif
  for (i = 0; i < list->size; ++i) {
    const struct Patch *p = &list->patches[i];
    /* Find line numbers to give context */
    /* This is a simplification; we'll output a chunk */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    diff_len += sprintf_s(diff_buf + diff_len, diff_cap - diff_len,
                          "@@ -patch %d @@\n", (int)i);
#else
    diff_len += sprintf(diff_buf + diff_len, "@@ -patch %d @@\n", (int)i);
#endif

    /* Emit old tokens (we prefix with '-') */
    diff_buf[diff_len++] = '-';
    diff_buf[diff_len] = '\0';
    {
      size_t j;
      for (j = p->start_token_idx; j < p->end_token_idx; ++j) {
        if (diff_len + tokens->tokens[j].length + 10 > diff_cap) {
          diff_cap *= 2;
          {
#ifdef CDD_BUILD_TESTS
            extern C_CDD_EXPORT int g_cdd_fail_alloc;
            char *new_buf;
            if (g_cdd_fail_alloc == 6666)
              new_buf = NULL;
            else
              new_buf = (char *)realloc(diff_buf, diff_cap);
#else
            char *new_buf = (char *)realloc(diff_buf, diff_cap);
#endif
            if (!new_buf) {
              free(diff_buf);
              C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
              return ENOMEM;
            }
            diff_buf = new_buf;
          }
        }
        memcpy(diff_buf + diff_len, tokens->tokens[j].start,
               tokens->tokens[j].length);
        diff_len += tokens->tokens[j].length;
      }
    }
    diff_buf[diff_len++] = '\n';
    diff_buf[diff_len] = '\0';

    /* Emit new tokens (we prefix with '+') */
    if (diff_len + strlen(p->text) + 10 > diff_cap) {
      diff_cap = diff_cap * 2 + strlen(p->text);
      {
#ifdef CDD_BUILD_TESTS
        extern C_CDD_EXPORT int g_cdd_fail_alloc;
        char *new_buf;
        if (g_cdd_fail_alloc == 7777)
          new_buf = NULL;
        else
          new_buf = (char *)realloc(diff_buf, diff_cap);
#else
        char *new_buf = (char *)realloc(diff_buf, diff_cap);
#endif
        if (!new_buf) {
          free(diff_buf);
          C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
          return ENOMEM;
        }
        diff_buf = new_buf;
      }
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    diff_len +=
        sprintf_s(diff_buf + diff_len, diff_cap - diff_len, "+%s\n", p->text);
#else
    diff_len += sprintf(diff_buf + diff_len, "+%s\n", p->text);
#endif
  }

  *out_diff = diff_buf;
  return 0;
}
