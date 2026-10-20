/**
 * @file tokenizer.h
 * @brief Lexical analyzer for C source code.
 *
 * Provides functionalities to convert raw source code strings into a stream of
 * categorized tokens.
 *
 * Compliance:
 * - Implements ISO C Translation Phase 1 (Trigraph replacement).
 * - Implements ISO C Translation Phase 2 (Line Splicing via backslash-newline).
 * - Implements ISO C Translation Phase 3 (Tokenization).
 *
 * Features:
 * - Greedily matches tokens even across line splices (e.g. `i\nnt` -> `int`).
 * - Supports C23 keywords matching.
 * - Supports C99 _Pragma operator.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TOKENIZER_H

#define C_CDD_TOKENIZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <stddef.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "classes/parse/cdd_token.h"
#include "mocks/c_cdd_stdbool.h"

#include <c_str_span.h>
/* clang-format on */

/**
 * @brief Convert source code into a list of tokens.
 *
 * Allocated a new `TokenList` and populates it by scanning the source span.
 * Detects all standard C operators and Punctuators greedily.
 * Handles Trigraphs and Line Splicing transparently.
 * The caller must free the returned list using `free_token_list`.
 *
 * @param[in] source The input string span to tokenize.
 * @param[out] out Double pointer to receive the allocated TokenList.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on invalid args.
 */

extern C_CDD_EXPORT cdd_c_error_t tokenize(az_span source,
                                           struct TokenList **out);

/**
 * @brief Free resources associated with a TokenList.
 *
 * Frees the internal token array and the struct itself.
 *
 * @param[in] tl The token list to free. Safe to pass NULL.
 */

extern C_CDD_EXPORT void free_token_list(struct TokenList *tl);

/**
 * @param[out] _out_val Pointer to store the result
 * @brief Check if a token's content matches a C string exactly.
 *
 * Performs a binary-safe comparison. Handles spliced tokens:
 * If a token contains ignored newlines, matches against logical content.
 *
 * @param[in] tok The token to examine. Must not be NULL.
 * @param[in] match The null-terminated string to compare against. Must not be
 * NULL.
 * @return true if token content equals `match`, false otherwise.
 */

extern C_CDD_EXPORT cdd_c_error_t token_matches_string(const struct Token *tok,

                                                       const char *match,
                                                       int *_out_val);

/**
 * @param[out] _out_val Pointer to store the result
 * @brief Helper to locate a specific token kind in a range.
 *
 * Scans the token list from `start_idx` up to `end_idx` looking for the
 * first token matching `kind`.
 *
 * @param[in] list The token list.
 * @param[in] start_idx Inclusive start index.
 * @param[in] end_idx Exclusive end index.
 * @param[in] kind The token kind to search for.
 * @return Index of the first matching token, or `list->size` (or `end_idx`) if
 * not found.
 */

extern C_CDD_EXPORT cdd_c_error_t token_find_next(const struct TokenList *list,

                                                  size_t start_idx,
                                                  size_t end_idx,

                                                  enum TokenKind kind,
                                                  size_t *_out_val);

/**
 * @param[out] _out_val Pointer to store the result
 * @brief Identify if a token text corresponds to a known keyword or identifier.
 *
 * @param[in] start Start of the token.
 * @param[in] len Length of the token.
 * @return The detected TokenKind (keyword kind or TOKEN_IDENTIFIER).
 */

extern C_CDD_EXPORT cdd_c_error_t
identify_keyword_or_id(const uint8_t *start,

                       size_t len, enum TokenKind *_out_val);

/**
 * @brief Appends a token to the token list.
 *
 * @param[in,out] tl The token list.
 * @param[in] kind The token category.
 * @param[in] start Pointer to start of token.
 * @param[in] length Length in bytes.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
extern C_CDD_EXPORT cdd_c_error_t token_list_add(struct TokenList *tl,
                                                 const enum TokenKind kind,
                                                 const uint8_t *start,
                                                 const size_t length);

/**
 * @brief Compare an az_span with a C-string.
 *
 * @param[in] span Span to compare.
 * @param[in] str C-string to compare against.
 * @param[out] _out_val Pointer to store 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if _out_val is
 * NULL.
 */
extern C_CDD_EXPORT cdd_c_error_t span_equals_str(const az_span span,
                                                  const char *str,
                                                  int *_out_val);

#ifdef CDD_BUILD_TESTS
/** @brief Test hook to fail identify_keyword_or_id. */
extern C_CDD_EXPORT int g_cdd_fail_identify_keyword_or_id;
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TOKENIZER_H */
