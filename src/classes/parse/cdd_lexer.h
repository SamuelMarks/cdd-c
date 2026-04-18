#ifndef CDD_LEXER_H
#define CDD_LEXER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_token.h"
#include <c_str_span.h>
#include "c_cdd_export.h"
/* clang-format on */

/**
 * @brief Run the lossless lexer over the given source code.
 *
 * @param source The input source code.
 * @param out_list Pointer to receive the generated token list.
 * @return 0 on success, or error code (ENOMEM, etc).
 */
C_CDD_EXPORT int cdd_lexer_tokenize(az_span source,
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
