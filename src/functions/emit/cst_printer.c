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

enum cdd_c_error cst_print_tokens_exact(const struct TokenList *tokens,
                                        FILE *out) {
  size_t i;
  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < tokens->size; ++i) {
    const struct Token *t = &tokens->tokens[i];
    size_t written = fwrite(t->start, 1, t->length, out);
    if (written != t->length) {
      return CDD_C_ERROR_IO;
    }
  }

  return CDD_C_SUCCESS;
}
