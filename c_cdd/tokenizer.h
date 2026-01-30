/**
 * @file tokenizer.h
 * @brief Lexical analyzer for C code headers.
 * Converts raw source text into a list of tokens classified by kind.
 * @author Samuel Marks
 */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>
#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */
#include <c_str_span.h>

/**
 * @brief Classification of a token extracted from source.
 */
enum C_CDD_EXPORT TokenKind {
  TOKEN_WHITESPACE,     /**< Space, tab, newline sequence */
  TOKEN_COMMENT,        /**< Single line or block comment */
  TOKEN_MACRO,          /**< Preprocessor directive */
  TOKEN_KEYWORD_STRUCT, /**< "struct" keyword */
  TOKEN_KEYWORD_ENUM,   /**< "enum" keyword */
  TOKEN_KEYWORD_UNION,  /**< "union" keyword */
  TOKEN_IDENTIFIER,     /**< Variable, type, or function name */
  TOKEN_LBRACE,         /**< '{' */
  TOKEN_RBRACE,         /**< '}' */
  TOKEN_SEMICOLON,      /**< ';' */
  TOKEN_COMMA,          /**< ',' */
  TOKEN_NUMBER_LITERAL, /**< Numeric literal */
  TOKEN_STRING_LITERAL, /**< "string" */
  TOKEN_CHAR_LITERAL,   /**< 'c' */
  TOKEN_OTHER,          /**< Any other character/sequence */
  TOKEN_UNKNOWN = -1    /**< Error sentinel */
};

/**
 * @brief A single lexical token spanning a region of source code.
 */
struct C_CDD_EXPORT Token {
  enum TokenKind kind;  /**< The type of token */
  const uint8_t *start; /**< Pointer to start of token in source buffer */
  size_t length;        /**< Length of token in bytes */
};

/**
 * @brief A dynamic list of tokens.
 */
struct C_CDD_EXPORT TokenList {
  struct Token *tokens; /**< Array of tokens */
  size_t size;          /**< Number of active tokens */
  size_t capacity;      /**< Allocated capacity of array */
};

/**
 * @brief Convert source code into a list of tokens.
 * Allocates memory within the TokenList which must be freed by
 * `free_token_list`.
 *
 * @param[in] source Span containing source C code.
 * @param[out] out List to be populated. struct should be zero-initialized.
 * @return 0 on success, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT int tokenize(az_span source, struct TokenList *out);

/**
 * @brief Release memory associated with the token list.
 * Does not free the struct TokenList itself if it was stack allocated,
 * only the internal dyanmic array.
 *
 * @param[in] tl Pointer to token list.
 */
extern C_CDD_EXPORT void free_token_list(struct TokenList *tl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TOKENIZER_H */
