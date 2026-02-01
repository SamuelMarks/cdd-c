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

#include <stddef.h>

#include "c_cdd_export.h"

#include "c_cdd_stdbool.h"

#include <c_str_span.h>

/**
 * @brief Categorization of C lexical elements.
 */

enum TokenKind {

  TOKEN_WHITESPACE, /**< Spaces, tabs, newlines */

  TOKEN_COMMENT, /**< Single line (//) or block comments */

  TOKEN_MACRO, /**< Preprocessor directives (#...) */

  /* --- Keywords --- */

  TOKEN_KEYWORD_STRUCT, /**< 'struct' */

  TOKEN_KEYWORD_ENUM, /**< 'enum' */

  TOKEN_KEYWORD_UNION, /**< 'union' */

  TOKEN_KEYWORD_IF, /**< 'if' */

  TOKEN_KEYWORD_ELSE, /**< 'else' */

  TOKEN_KEYWORD_WHILE, /**< 'while' */

  TOKEN_KEYWORD_DO, /**< 'do' */

  TOKEN_KEYWORD_FOR, /**< 'for' */

  TOKEN_KEYWORD_RETURN, /**< 'return' */

  TOKEN_KEYWORD_SWITCH, /**< 'switch' */

  TOKEN_KEYWORD_CASE, /**< 'case' */

  TOKEN_KEYWORD_DEFAULT, /**< 'default' */

  TOKEN_KEYWORD_BREAK, /**< 'break' */

  TOKEN_KEYWORD_CONTINUE, /**< 'continue' */

  TOKEN_KEYWORD_GOTO, /**< 'goto' */

  TOKEN_KEYWORD_TYPEDEF, /**< 'typedef' */

  TOKEN_KEYWORD_EXTERN, /**< 'extern' */

  TOKEN_KEYWORD_STATIC, /**< 'static' */

  TOKEN_KEYWORD_AUTO, /**< 'auto' */

  TOKEN_KEYWORD_REGISTER, /**< 'register' */

  TOKEN_KEYWORD_INLINE, /**< 'inline' or '__inline' */

  TOKEN_KEYWORD_CONST, /**< 'const' */

  TOKEN_KEYWORD_VOLATILE, /**< 'volatile' */

  TOKEN_KEYWORD_RESTRICT, /**< 'restrict' or '__restrict' */

  TOKEN_KEYWORD_SIZEOF, /**< 'sizeof' */

  TOKEN_KEYWORD_VOID, /**< 'void' */

  TOKEN_KEYWORD_CHAR, /**< 'char' */

  TOKEN_KEYWORD_SHORT, /**< 'short' */

  TOKEN_KEYWORD_INT, /**< 'int' */

  TOKEN_KEYWORD_LONG, /**< 'long' */

  TOKEN_KEYWORD_FLOAT, /**< 'float' */

  TOKEN_KEYWORD_DOUBLE, /**< 'double' */

  TOKEN_KEYWORD_SIGNED, /**< 'signed' */

  TOKEN_KEYWORD_UNSIGNED, /**< 'unsigned' */

  TOKEN_KEYWORD_BOOL, /**< '_Bool' or 'bool' */

  TOKEN_KEYWORD_COMPLEX, /**< '_Complex' */

  TOKEN_KEYWORD_IMAGINARY, /**< '_Imaginary' */

  TOKEN_KEYWORD_ATOMIC, /**< '_Atomic' */

  TOKEN_KEYWORD_THREAD_LOCAL, /**< '_Thread_local' or 'thread_local' */

  TOKEN_KEYWORD_ALIGNAS, /**< '_Alignas' or 'alignas' */

  TOKEN_KEYWORD_ALIGNOF, /**< '_Alignof' or 'alignof' */

  TOKEN_KEYWORD_NORETURN, /**< '_Noreturn' */

  TOKEN_KEYWORD_CONSTEXPR, /**< 'constexpr' */

  TOKEN_KEYWORD_STATIC_ASSERT, /**< '_Static_assert' or 'static_assert' */

  TOKEN_KEYWORD_TYPEOF, /**< 'typeof' */

  TOKEN_KEYWORD_NULLPTR, /**< 'nullptr' */

  TOKEN_KEYWORD_TRUE, /**< 'true' */

  TOKEN_KEYWORD_FALSE, /**< 'false' */

  TOKEN_KEYWORD_EMBED, /**< 'embed' (C23 preprocessor) */

  TOKEN_KEYWORD_PRAGMA_OP, /**< '_Pragma' (C99 operator) */

  /* --- Identifiers & Literals --- */

  TOKEN_IDENTIFIER, /**< Alphanumeric identifiers */

  TOKEN_NUMBER_LITERAL, /**< Integer/Float literals */

  TOKEN_STRING_LITERAL, /**< Quoted strings */

  TOKEN_CHAR_LITERAL, /**< Quoted characters */

  /* --- Punctuators (Single & Multi-char) --- */

  TOKEN_LBRACE, /**< '{' */

  TOKEN_RBRACE, /**< '}' */

  TOKEN_LPAREN, /**< '(' */

  TOKEN_RPAREN, /**< ')' */

  TOKEN_LBRACKET, /**< '[' */

  TOKEN_RBRACKET, /**< ']' */

  TOKEN_SEMICOLON, /**< ';' */

  TOKEN_COMMA, /**< ',' */

  TOKEN_DOT, /**< '.' */

  TOKEN_ARROW, /**< '->' */

  TOKEN_PLUS, /**< '+' */

  TOKEN_MINUS, /**< '-' */

  TOKEN_STAR, /**< '*' */

  TOKEN_SLASH, /**< '/' */

  TOKEN_PERCENT, /**< '%' */

  TOKEN_AMP, /**< '&' */

  TOKEN_PIPE, /**< '|' */

  TOKEN_CARET, /**< '^' */

  TOKEN_TILDE, /**< '~' */

  TOKEN_BANG, /**< '!' */

  TOKEN_QUESTION, /**< '?' */

  TOKEN_COLON, /**< ':' */

  TOKEN_ASSIGN, /**< '=' */

  TOKEN_LESS, /**< '<' */

  TOKEN_GREATER, /**< '>' */

  TOKEN_EQ, /**< '==' */

  TOKEN_NEQ, /**< '!=' */

  TOKEN_LEQ, /**< '<=' */

  TOKEN_GEQ, /**< '>=' */

  TOKEN_LOGICAL_AND, /**< '&&' */

  TOKEN_LOGICAL_OR, /**< '||' */

  TOKEN_INC, /**< '++' */

  TOKEN_DEC, /**< '--' */

  TOKEN_PLUS_ASSIGN, /**< '+=' */

  TOKEN_MINUS_ASSIGN, /**< '-=' */

  TOKEN_MUL_ASSIGN, /**< '*=' */

  TOKEN_DIV_ASSIGN, /**< '/=' */

  TOKEN_MOD_ASSIGN, /**< '*=' */

  TOKEN_AND_ASSIGN, /**< '&=' */

  TOKEN_OR_ASSIGN, /**< '|=' */

  TOKEN_XOR_ASSIGN, /**< '^=' */

  TOKEN_LSHIFT, /**< '<<' */

  TOKEN_RSHIFT, /**< '>>' */

  TOKEN_LSHIFT_ASSIGN, /**< '<<=' */

  TOKEN_RSHIFT_ASSIGN, /**< '>>=' */

  TOKEN_ELLIPSIS, /**< '...' */

  TOKEN_HASH, /**< '#' */

  TOKEN_HASH_HASH, /**< '##' */

  TOKEN_OTHER, /**< Unclassified symbols */

  TOKEN_UNKNOWN = -1 /**< Error sentinel */

};

/**
 * @brief A single lexical unit.
 *
 * Represents a slice of the original source code via a pointer and length.
 * Note: If the token spans spliced lines (backslash-newline), the `length`
 * includes the spliced characters.
 */

struct Token {

  enum TokenKind kind; /**< Category of the token */

  const uint8_t

      *start; /**< Pointer to start of token in original source buffer */

  size_t length; /**< Length of the token in bytes (physical) */
};

/**
 * @brief Dynamic array of tokens.
 */

struct TokenList {

  struct Token *tokens; /**< Array of Token structures */

  size_t size; /**< Number of valid tokens used */

  size_t capacity; /**< Allocated capacity of the array */
};

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

extern C_CDD_EXPORT int tokenize(az_span source, struct TokenList **out);

/**
 * @brief Free resources associated with a TokenList.
 *
 * Frees the internal token array and the struct itself.
 *
 * @param[in] tl The token list to free. Safe to pass NULL.
 */

extern C_CDD_EXPORT void free_token_list(struct TokenList *tl);

/**
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

extern C_CDD_EXPORT bool token_matches_string(const struct Token *tok,

                                              const char *match);

/**
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

extern C_CDD_EXPORT size_t token_find_next(const struct TokenList *list,

                                           size_t start_idx, size_t end_idx,

                                           enum TokenKind kind);

/**
 * @brief Identify if a token text corresponds to a known keyword or identifier.
 *
 * @param start Start of the token.
 * @param len Length of the token.
 * @return The detected TokenKind (keyword kind or TOKEN_IDENTIFIER).
 */

extern C_CDD_EXPORT enum TokenKind identify_keyword_or_id(const uint8_t *start,

                                                          size_t len);

#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /* C_CDD_TOKENIZER_H */
