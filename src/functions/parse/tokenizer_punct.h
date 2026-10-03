/**
 * @file tokenizer_punct.h
 * @brief Lexical analyzer punctuators and comment processing.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TOKENIZER_PUNCT_H
#define C_CDD_TOKENIZER_PUNCT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <stddef.h>
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "c_cdd/msvc/stdint.h"
#else
#include <stdint.h>
#endif
#endif

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Peek the next logical character from the buffer.
 *
 * Handles Phase 1 (Trigraphs) and Phase 2 (Backslash-Newline Splicing).
 *
 * @param[in] base Source buffer.
 * @param[in] len Buffer length.
 * @param[in] pos Current physical position.
 * @param[out] out_consumed How many physical bytes constitute this logical
 * char.
 * @return The logical character, or -1 if end of buffer.
 */
extern C_CDD_EXPORT int peek_logical(const uint8_t *base, size_t len,
                                     size_t pos, size_t *out_consumed);

/**
 * @brief Tokenizes punctuators and comments starting with character c.
 *
 * @param[in] base Source buffer.
 * @param[in] len Buffer length.
 * @param[in,out] pos Pointer to current buffer position.
 * @param[in] start Start position of the token.
 * @param[in] c First logical character of the token.
 * @param[in] consumed Number of bytes consumed for character c.
 * @param[in,out] list Token list to append to.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t tokenize_punct(const uint8_t *base,
                                                 size_t len, size_t *pos,
                                                 size_t start, int c,
                                                 size_t consumed,
                                                 struct TokenList *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_TOKENIZER_PUNCT_H */
