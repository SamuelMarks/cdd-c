/**
 * @file tokenizer.h
 * @brief Lexical analyzer for C source code.
 *
 * Converts raw text into a stream of categorized tokens.
 * Provides utilities for inspecting token content safely.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TOKENIZER_H
#define C_CDD_TOKENIZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

#include "c_cdd_export.h"
#include "c_cdd_stdbool.h"
#include <c_str_span.h>

/**
 * @brief Categorization of C lexical elements.
 */
enum TokenKind {
  TOKEN_WHITESPACE,     /**< Spaces, tabs, newlines */
  TOKEN_COMMENT,        /**< Single line (//) or block comments */
  TOKEN_MACRO,          /**< Preprocessor directives (#...) */
  TOKEN_KEYWORD_STRUCT, /**< 'struct' keyword */
  TOKEN_KEYWORD_ENUM,   /**< 'enum' keyword */
  TOKEN_KEYWORD_UNION,  /**< 'union' keyword */
  TOKEN_IDENTIFIER,     /**< Alphanumeric identifiers (vars, types, funcs) */
  TOKEN_LBRACE,         /**< '{' */
  TOKEN_RBRACE,         /**< '}' */
  TOKEN_LPAREN,         /**< '(' */
  TOKEN_RPAREN,         /**< ')' */
  TOKEN_SEMICOLON,      /**< ';' */
  TOKEN_COMMA,          /**< ',' */
  TOKEN_NUMBER_LITERAL, /**< Integer or Floating point literals */
  TOKEN_STRING_LITERAL, /**< Double-quoted strings ("...") */
  TOKEN_CHAR_LITERAL,   /**< Single-quoted characters ('c') */
  TOKEN_OTHER,          /**< Operators and unclassified symbols */
  TOKEN_UNKNOWN = -1    /**< Error sentinel */
};

/**
 * @brief A single lexical unit.
 */
struct Token {
  enum TokenKind kind; /**< Category of the token */
  const uint8_t
      *start;    /**< Pointer to start of token in original source buffer */
  size_t length; /**< Length of the token in bytes */
};

/**
 * @brief dynamic array of tokens.
 */
struct TokenList {
  struct Token *tokens; /**< Array of Token structures */
  size_t size;          /**< Number of valid tokens used */
  size_t capacity;      /**< Allocated capacity of the array */
};

/**
 * @brief Convert source code into a list of tokens.
 *
 * Allocates a new `TokenList` and populates it. The caller must free it.
 *
 * @param[in] source The input string span.
 * @param[out] out Pointer to the destination pointer.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on bad args.
 */
extern C_CDD_EXPORT int tokenize(az_span source, struct TokenList **out);

/**
 * @brief Free resources associated with a TokenList.
 *
 * Frees the inner array and the struct itself.
 *
 * @param[in] tl The list to free. Safe to pass NULL.
 */
extern C_CDD_EXPORT void free_token_list(struct TokenList *tl);

/**
 * @brief Check if a token's content matches a C string exactly.
 *
 * Performs a length-aware binary safe comparison.
 *
 * @param[in] tok NOT NULL. The token to examine.
 * @param[in] match NOT NULL. The null-terminated string to compare against.
 * @return true if token content equals `match`, false otherwise.
 */
extern C_CDD_EXPORT bool token_matches_string(const struct Token *tok,
                                              const char *match);

/**
 * @brief Helper to locate a specific token kind in a range.
 *
 * @param[in] list The token list.
 * @param[in] start_idx Inclusive start index.
 * @param[in] end_idx Exclusive end index.
 * @param[in] kind The kind to search for.
 * @return Index of the first matching token, or `list->size` (or end limit) if
 * not found.
 */
extern C_CDD_EXPORT size_t token_find_next(const struct TokenList *list,
                                           size_t start_idx, size_t end_idx,
                                           enum TokenKind kind);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TOKENIZER_H */
