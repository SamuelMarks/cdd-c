/**
 * @file cst_printer.c
 * @brief Implementation of CST non-destructive printing.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/cst_printer.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
/** @brief FWRITE_HOOK macro */
#define FWRITE_HOOK(ptr, size, nmemb, stream)                                  \
  ((g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)                    \
       ? (size_t)0                                                             \
       : fwrite((ptr), (size), (nmemb), (stream)))
#else
/** @brief FWRITE_HOOK macro */
#define FWRITE_HOOK fwrite
#endif

/**
 * @brief Reconstructs and writes the exact original source text from a token
 * list to `out`.
 *
 * @param[in] tokens Lexical token list to print.
 * @param[in,out] out Output file stream.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cst_print_tokens_exact(const struct TokenList *tokens,
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
