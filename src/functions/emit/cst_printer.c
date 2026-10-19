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
/**
 * @brief Mock hook for `fwrite` to simulate I/O failures during testing.
 *
 * @param[in] ptr Pointer to the buffer of elements to write.
 * @param[in] size Size in bytes of each element.
 * @param[in] nmemb Number of elements to write.
 * @param[in] stream Output file stream.
 * @return Number of elements written, or 0 on simulated failure.
 */
static size_t test_cdd_fwrite_hook(const void *ptr, size_t size, size_t nmemb,
                                   FILE *stream) {
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return 0;
  return fwrite(ptr, size, nmemb, stream);
}
/** @brief FWRITE_HOOK macro */
#define FWRITE_HOOK test_cdd_fwrite_hook
#else
/** @brief FWRITE_HOOK macro */
#define FWRITE_HOOK fwrite
#endif

/**
 * @brief Reconstructs and writes the exact original source text from a token
 * list to `out`.
 *
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
