#ifndef CDD_DSL_TOKENIZER_H
#define CDD_DSL_TOKENIZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "cdd_dsl_token.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct cdd_dsl_token_list_t
 * @brief An array of parsed tokens.
 */
typedef struct cdd_dsl_token_list_t {
  /** @brief Array of tokens. */
  cdd_dsl_token_t *tokens;
  /** @brief The number of tokens. */
  size_t count;
  /** @brief The allocated capacity. */
  size_t capacity;
} cdd_dsl_token_list_t;

/**
 * @brief Tokenizes a CDD FFI IR DSL source string.
 *
 * @param source The null-terminated source string.
 * @param out_tokens Pointer to a cdd_dsl_token_list_t to populate.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
C_CDD_EXPORT cdd_c_error_t cdd_dsl_tokenize(const char *source,
                                            cdd_dsl_token_list_t *out_tokens);

/**
 * @brief Frees a token list.
 *
 * @param list The token list to free.
 */
C_CDD_EXPORT void cdd_dsl_token_list_free(cdd_dsl_token_list_t *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_DSL_TOKENIZER_H */
