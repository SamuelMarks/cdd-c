/**
 * @file cst_printer.c
 * @brief Implementation of CST non-destructive printing.
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/cst_printer.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
static size_t cdd_fwrite_hook(const void *ptr, size_t size, size_t nmemb,
                              FILE *stream) {
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return 0;
  return fwrite(ptr, size, nmemb, stream);
}
/** @brief FWRITE_HOOK macro */
#define FWRITE_HOOK cdd_fwrite_hook
#else
/** @brief FWRITE_HOOK macro */
#define FWRITE_HOOK fwrite
#endif

enum cdd_c_error cst_print_tokens_exact(const struct TokenList *tokens,
                                        FILE *out) {
  size_t i;
  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < tokens->size; ++i) {
    const struct Token *t = &tokens->tokens[i];
    size_t written = FWRITE_HOOK(t->start, 1, t->length, out);
    if (written != t->length) {
      return CDD_C_ERROR_IO;
    }
  }

  return CDD_C_SUCCESS;
}
