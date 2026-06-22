#ifndef CDD_TOKEN_H
#define CDD_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#include <stdint.h>
#endif
#endif
/* clang-format on */

/**
 * @brief Classifies non-semantic characters.
 */
enum cdd_trivia_kind_t {
  TRIVIA_WHITESPACE,
  TRIVIA_NEWLINE,
  TRIVIA_LINE_COMMENT,
  TRIVIA_BLOCK_COMMENT
};

/**
 * @brief Represents a piece of trivia attached to a token.
 */
typedef struct cdd_trivia_t cdd_trivia_t;
/** @brief Struct definition */
struct cdd_trivia_t {
  enum cdd_trivia_kind_t kind; /**< Kind of trivia */
  const uint8_t *start;        /**< Pointer to start of trivia */
  size_t length;               /**< Length in bytes */
  cdd_trivia_t *next;          /**< Linked list of trivia */
};

/**
 * @brief Token types for the lossless lexer.
 */
enum cdd_token_kind_t {
  CDD_TOKEN_IDENTIFIER,
  CDD_TOKEN_NUMBER,
  CDD_TOKEN_STRING,
  CDD_TOKEN_CHAR,
  CDD_TOKEN_KEYWORD_INT,
  CDD_TOKEN_KEYWORD___INT128,
  CDD_TOKEN_KEYWORD_TYPEOF,
  CDD_TOKEN_KEYWORD___AUTO_TYPE,
  CDD_TOKEN_KEYWORD___COMPLEX__,
  CDD_TOKEN_KEYWORD___REAL__,
  CDD_TOKEN_KEYWORD___IMAG__,
  CDD_TOKEN_KEYWORD___LABEL__,
  CDD_TOKEN_KEYWORD__DECIMAL32,
  CDD_TOKEN_KEYWORD__DECIMAL64,
  CDD_TOKEN_KEYWORD__DECIMAL128,
  CDD_TOKEN_KEYWORD___FP16,
  CDD_TOKEN_KEYWORD__FLOAT16,
  CDD_TOKEN_KEYWORD___BF16,
  CDD_TOKEN_KEYWORD__FRACT,
  CDD_TOKEN_KEYWORD__ACCUM,
  CDD_TOKEN_KEYWORD_STRUCT,
  CDD_TOKEN_KEYWORD_IF,
  CDD_TOKEN_KEYWORD_ELSE,
  CDD_TOKEN_KEYWORD_RETURN,
  CDD_TOKEN_KEYWORD_GOTO,
  CDD_TOKEN_KEYWORD_CLASS,
  CDD_TOKEN_KEYWORD_PUBLIC,
  CDD_TOKEN_KEYWORD_PRIVATE,
  CDD_TOKEN_KEYWORD_PROTECTED,
  CDD_TOKEN_KEYWORD_VIRTUAL,
  CDD_TOKEN_KEYWORD_TEMPLATE,
  CDD_TOKEN_KEYWORD_TYPENAME,
  CDD_TOKEN_KEYWORD_NEW,
  CDD_TOKEN_KEYWORD_DELETE,
  CDD_TOKEN_KEYWORD_NAMESPACE,
  CDD_TOKEN_KEYWORD_USING,
  CDD_TOKEN_KEYWORD_CONSTEXPR,
  CDD_TOKEN_KEYWORD_OPERATOR,
  CDD_TOKEN_KEYWORD_TRY,
  CDD_TOKEN_KEYWORD_CATCH,
  CDD_TOKEN_KEYWORD_THROW,
  CDD_TOKEN_KEYWORD_NOEXCEPT,
  CDD_TOKEN_LBRACE,
  CDD_TOKEN_RBRACE,
  CDD_TOKEN_LPAREN,
  CDD_TOKEN_RPAREN,
  CDD_TOKEN_LBRACKET,
  CDD_TOKEN_RBRACKET,
  CDD_TOKEN_SEMICOLON,
  CDD_TOKEN_COMMA,
  CDD_TOKEN_DOT,
  CDD_TOKEN_ARROW,
  CDD_TOKEN_PLUS,
  CDD_TOKEN_MINUS,
  CDD_TOKEN_STAR,
  CDD_TOKEN_SLASH,
  CDD_TOKEN_ASSIGN,
  CDD_TOKEN_EQ,
  CDD_TOKEN_NEQ,
  CDD_TOKEN_LT,
  CDD_TOKEN_GT,
  CDD_TOKEN_COLON,
  CDD_TOKEN_TILDE,
  CDD_TOKEN_PREPROC_INCLUDE,
  CDD_TOKEN_PREPROC_DEFINE,
  CDD_TOKEN_PREPROC_IFDEF,
  CDD_TOKEN_PREPROC_ELIF,
  CDD_TOKEN_PREPROC_ELSE,
  CDD_TOKEN_PREPROC_IFNDEF,
  CDD_TOKEN_PREPROC_ENDIF,
  CDD_TOKEN_PREPROC_PRAGMA,
  CDD_TOKEN_OTHER,
  CDD_TOKEN_EOF
};

/**
 * @brief Represents a lossless lexical token.
 */
typedef struct cdd_token_t cdd_token_t;
/** @brief Struct definition */
struct cdd_token_t {
  enum cdd_token_kind_t kind;    /**< Token type */
  const uint8_t *start;          /**< Text span pointer */
  size_t length;                 /**< Text span length */
  size_t line;                   /**< 1-based line number */
  size_t column;                 /**< 1-based column number */
  size_t offset;                 /**< 0-based byte offset */
  cdd_trivia_t *leading_trivia;  /**< Trivia before this token */
  cdd_trivia_t *trailing_trivia; /**< Trivia after this token on same line */
};

/**
 * @brief A list of tokens.
 */
typedef struct cdd_token_list_t cdd_token_list_t;
/** @brief Struct definition */
struct cdd_token_list_t {
  /** @brief field */
  /** @brief field */
  cdd_token_t *tokens;
  /** @brief field */
  /** @brief field */
  size_t size;
  /** @brief field */
  /** @brief field */
  size_t capacity;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_TOKEN_H */
