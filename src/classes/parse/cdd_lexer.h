#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#ifndef CDD_LEXER_H
#define CDD_LEXER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_token.h"
#include <c_str_span.h>
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

/**
 * @brief Run the lossless lexer over the given source code.
 *
 * @param source The input source code.
 * @param out_list Pointer to receive the generated token list.
 * @return 0 on success, or error code (ENOMEM, etc).
 */
C_CDD_EXPORT cdd_c_error_t cdd_lexer_tokenize(az_span source,
                                              cdd_token_list_t **out_list);

/**
 * @brief Free a token list and its associated trivia.
 *
 * @param list The token list to free.
 */
C_CDD_EXPORT void cdd_lexer_free_token_list(cdd_token_list_t *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_LEXER_H */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
