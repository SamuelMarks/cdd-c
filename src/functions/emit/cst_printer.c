/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/cst_printer.h"
/* clang-format on */

/**
 * @brief Executes the cst print tokens exact operation.
 */
int cst_print_tokens_exact(const struct TokenList *tokens, FILE *out) {
  size_t i;
  if (!tokens || !out)
    return EINVAL;

  for (i = 0; i < tokens->size; ++i) {
    const struct Token *t = &tokens->tokens[i];
    size_t written = fwrite(t->start, 1, t->length, out);
    if (written != t->length) {
      return EIO;
    }
  }

  return 0;
}
