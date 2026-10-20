/**
 * @file cdd_token.h
 * @brief Core cdd token definitions and implementation.
 */
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
#if defined(_MSC_VER) && _MSC_VER < 1800
#include "msvc/stdint.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "c_cdd/msvc/stdint.h"
#else
#include <stdint.h>
#endif
#endif
/* clang-format on */
#endif
#endif

/**
 * @brief Classifies non-semantic characters.
 */
enum cdd_trivia_kind_t {
  TRIVIA_WHITESPACE,   /**< Horizontal whitespace (spaces, tabs) */
  TRIVIA_NEWLINE,      /**< Line break sequences */
  TRIVIA_LINE_COMMENT, /**< Single-line comment */
  TRIVIA_BLOCK_COMMENT /**< Block comment */
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
 * @brief Unified token types for the lossless C/C++ lexer and tokenizer.
 */
enum cdd_token_kind_t {
  TOKEN_WHITESPACE = 0,                    /**< Spaces, tabs, newlines */
  CDD_TOKEN_WHITESPACE = TOKEN_WHITESPACE, /**< Alias for whitespace */
  TOKEN_COMMENT,                           /**< Comments */
  CDD_TOKEN_COMMENT = TOKEN_COMMENT,       /**< Alias for comment */
  TOKEN_MACRO,                             /**< Preprocessor directive */
  CDD_TOKEN_MACRO = TOKEN_MACRO,           /**< Alias for macro */

  /* --- GNU / C Type Keywords (contiguous for range checks) --- */
  CDD_TOKEN_KEYWORD_INT,                     /**< 'int' */
  TOKEN_KEYWORD_INT = CDD_TOKEN_KEYWORD_INT, /**< Alias for 'int' */
  CDD_TOKEN_KEYWORD___INT128,                /**< '__int128' */
  CDD_TOKEN_KEYWORD_TYPEOF,                  /**< 'typeof' / '__typeof__' */
  TOKEN_KEYWORD_TYPEOF = CDD_TOKEN_KEYWORD_TYPEOF, /**< Alias for 'typeof' */
  CDD_TOKEN_KEYWORD___AUTO_TYPE,                   /**< '__auto_type' */
  CDD_TOKEN_KEYWORD___COMPLEX__,                   /**< '__complex__' */
  CDD_TOKEN_KEYWORD___REAL__,                      /**< '__real__' */
  CDD_TOKEN_KEYWORD___IMAG__,                      /**< '__imag__' */
  CDD_TOKEN_KEYWORD___LABEL__,                     /**< '__label__' */
  CDD_TOKEN_KEYWORD__DECIMAL32,                    /**< '_Decimal32' */
  CDD_TOKEN_KEYWORD__DECIMAL64,                    /**< '_Decimal64' */
  CDD_TOKEN_KEYWORD__DECIMAL128,                   /**< '_Decimal128' */
  CDD_TOKEN_KEYWORD___FP16,                        /**< '__fp16' */
  CDD_TOKEN_KEYWORD__FLOAT16,                      /**< '_Float16' */
  CDD_TOKEN_KEYWORD___BF16,                        /**< '__bf16' */
  CDD_TOKEN_KEYWORD__FRACT,                        /**< '_Fract' */
  CDD_TOKEN_KEYWORD__ACCUM,                        /**< '_Accum' */

  /* --- Standard C & C++ Keywords --- */
  CDD_TOKEN_KEYWORD_STRUCT,                          /**< 'struct' */
  TOKEN_KEYWORD_STRUCT = CDD_TOKEN_KEYWORD_STRUCT,   /**< Alias for 'struct' */
  TOKEN_KEYWORD_ENUM,                                /**< 'enum' */
  CDD_TOKEN_KEYWORD_ENUM = TOKEN_KEYWORD_ENUM,       /**< Alias for 'enum' */
  TOKEN_KEYWORD_UNION,                               /**< 'union' */
  CDD_TOKEN_KEYWORD_UNION = TOKEN_KEYWORD_UNION,     /**< Alias for 'union' */
  CDD_TOKEN_KEYWORD_IF,                              /**< 'if' */
  TOKEN_KEYWORD_IF = CDD_TOKEN_KEYWORD_IF,           /**< Alias for 'if' */
  CDD_TOKEN_KEYWORD_ELSE,                            /**< 'else' */
  TOKEN_KEYWORD_ELSE = CDD_TOKEN_KEYWORD_ELSE,       /**< Alias for 'else' */
  TOKEN_KEYWORD_WHILE,                               /**< 'while' */
  CDD_TOKEN_KEYWORD_WHILE = TOKEN_KEYWORD_WHILE,     /**< Alias for 'while' */
  TOKEN_KEYWORD_DO,                                  /**< 'do' */
  CDD_TOKEN_KEYWORD_DO = TOKEN_KEYWORD_DO,           /**< Alias for 'do' */
  TOKEN_KEYWORD_FOR,                                 /**< 'for' */
  CDD_TOKEN_KEYWORD_FOR = TOKEN_KEYWORD_FOR,         /**< Alias for 'for' */
  CDD_TOKEN_KEYWORD_RETURN,                          /**< 'return' */
  TOKEN_KEYWORD_RETURN = CDD_TOKEN_KEYWORD_RETURN,   /**< Alias for 'return' */
  TOKEN_KEYWORD_SWITCH,                              /**< 'switch' */
  CDD_TOKEN_KEYWORD_SWITCH = TOKEN_KEYWORD_SWITCH,   /**< Alias for 'switch' */
  TOKEN_KEYWORD_CASE,                                /**< 'case' */
  CDD_TOKEN_KEYWORD_CASE = TOKEN_KEYWORD_CASE,       /**< Alias for 'case' */
  TOKEN_KEYWORD_DEFAULT,                             /**< 'default' */
  CDD_TOKEN_KEYWORD_DEFAULT = TOKEN_KEYWORD_DEFAULT, /**< Alias for 'default' */
  TOKEN_KEYWORD_BREAK,                               /**< 'break' */
  CDD_TOKEN_KEYWORD_BREAK = TOKEN_KEYWORD_BREAK,     /**< Alias for 'break' */
  TOKEN_KEYWORD_CONTINUE,                            /**< 'continue' */
  CDD_TOKEN_KEYWORD_CONTINUE =
      TOKEN_KEYWORD_CONTINUE,                  /**< Alias for 'continue' */
  CDD_TOKEN_KEYWORD_GOTO,                      /**< 'goto' */
  TOKEN_KEYWORD_GOTO = CDD_TOKEN_KEYWORD_GOTO, /**< Alias for 'goto' */
  TOKEN_KEYWORD_TYPEDEF,                       /**< 'typedef' */
  CDD_TOKEN_KEYWORD_TYPEDEF = TOKEN_KEYWORD_TYPEDEF, /**< Alias for 'typedef' */
  TOKEN_KEYWORD_EXTERN,                              /**< 'extern' */
  CDD_TOKEN_KEYWORD_EXTERN = TOKEN_KEYWORD_EXTERN,   /**< Alias for 'extern' */
  TOKEN_KEYWORD_STATIC,                              /**< 'static' */
  CDD_TOKEN_KEYWORD_STATIC = TOKEN_KEYWORD_STATIC,   /**< Alias for 'static' */
  TOKEN_KEYWORD_AUTO,                                /**< 'auto' */
  CDD_TOKEN_KEYWORD_AUTO = TOKEN_KEYWORD_AUTO,       /**< Alias for 'auto' */
  TOKEN_KEYWORD_REGISTER,                            /**< 'register' */
  CDD_TOKEN_KEYWORD_REGISTER =
      TOKEN_KEYWORD_REGISTER, /**< Alias for 'register' */
  TOKEN_KEYWORD_INLINE,       /**< 'inline' or '__inline' */
  CDD_TOKEN_KEYWORD_INLINE = TOKEN_KEYWORD_INLINE, /**< Alias for 'inline' */
  TOKEN_KEYWORD_CONST,                             /**< 'const' */
  CDD_TOKEN_KEYWORD_CONST = TOKEN_KEYWORD_CONST,   /**< Alias for 'const' */
  TOKEN_KEYWORD_VOLATILE,                          /**< 'volatile' */
  CDD_TOKEN_KEYWORD_VOLATILE =
      TOKEN_KEYWORD_VOLATILE, /**< Alias for 'volatile' */
  TOKEN_KEYWORD_RESTRICT,     /**< 'restrict' or '__restrict' */
  CDD_TOKEN_KEYWORD_RESTRICT =
      TOKEN_KEYWORD_RESTRICT,                      /**< Alias for 'restrict' */
  TOKEN_KEYWORD_SIZEOF,                            /**< 'sizeof' */
  CDD_TOKEN_KEYWORD_SIZEOF = TOKEN_KEYWORD_SIZEOF, /**< Alias for 'sizeof' */
  TOKEN_KEYWORD_VOID,                              /**< 'void' */
  CDD_TOKEN_KEYWORD_VOID = TOKEN_KEYWORD_VOID,     /**< Alias for 'void' */
  TOKEN_KEYWORD_CHAR,                              /**< 'char' */
  CDD_TOKEN_KEYWORD_CHAR = TOKEN_KEYWORD_CHAR,     /**< Alias for 'char' */
  TOKEN_KEYWORD_SHORT,                             /**< 'short' */
  CDD_TOKEN_KEYWORD_SHORT = TOKEN_KEYWORD_SHORT,   /**< Alias for 'short' */
  TOKEN_KEYWORD_LONG,                              /**< 'long' */
  CDD_TOKEN_KEYWORD_LONG = TOKEN_KEYWORD_LONG,     /**< Alias for 'long' */
  TOKEN_KEYWORD_FLOAT,                             /**< 'float' */
  CDD_TOKEN_KEYWORD_FLOAT = TOKEN_KEYWORD_FLOAT,   /**< Alias for 'float' */
  TOKEN_KEYWORD_DOUBLE,                            /**< 'double' */
  CDD_TOKEN_KEYWORD_DOUBLE = TOKEN_KEYWORD_DOUBLE, /**< Alias for 'double' */
  TOKEN_KEYWORD_SIGNED,                            /**< 'signed' */
  CDD_TOKEN_KEYWORD_SIGNED = TOKEN_KEYWORD_SIGNED, /**< Alias for 'signed' */
  TOKEN_KEYWORD_UNSIGNED,                          /**< 'unsigned' */
  CDD_TOKEN_KEYWORD_UNSIGNED =
      TOKEN_KEYWORD_UNSIGNED,                  /**< Alias for 'unsigned' */
  TOKEN_KEYWORD_BOOL,                          /**< '_Bool' or 'bool' */
  CDD_TOKEN_KEYWORD_BOOL = TOKEN_KEYWORD_BOOL, /**< Alias for 'bool' */
  TOKEN_KEYWORD_COMPLEX,                       /**< '_Complex' */
  CDD_TOKEN_KEYWORD_COMPLEX =
      TOKEN_KEYWORD_COMPLEX, /**< Alias for '_Complex' */
  TOKEN_KEYWORD_IMAGINARY,   /**< '_Imaginary' */
  CDD_TOKEN_KEYWORD_IMAGINARY =
      TOKEN_KEYWORD_IMAGINARY, /**< Alias for '_Imaginary' */
  TOKEN_KEYWORD_ATOMIC,        /**< '_Atomic' */
  CDD_TOKEN_KEYWORD_ATOMIC = TOKEN_KEYWORD_ATOMIC, /**< Alias for '_Atomic' */
  TOKEN_KEYWORD_THREAD_LOCAL, /**< '_Thread_local' or 'thread_local' */
  CDD_TOKEN_KEYWORD_THREAD_LOCAL =
      TOKEN_KEYWORD_THREAD_LOCAL, /**< Alias for 'thread_local' */
  TOKEN_KEYWORD_ALIGNAS,          /**< '_Alignas' or 'alignas' */
  CDD_TOKEN_KEYWORD_ALIGNAS = TOKEN_KEYWORD_ALIGNAS, /**< Alias for 'alignas' */
  TOKEN_KEYWORD_ALIGNOF, /**< '_Alignof' or 'alignof' */
  CDD_TOKEN_KEYWORD_ALIGNOF = TOKEN_KEYWORD_ALIGNOF, /**< Alias for 'alignof' */
  TOKEN_KEYWORD_NORETURN,                            /**< '_Noreturn' */
  CDD_TOKEN_KEYWORD_NORETURN =
      TOKEN_KEYWORD_NORETURN,  /**< Alias for '_Noreturn' */
  CDD_TOKEN_KEYWORD_CONSTEXPR, /**< 'constexpr' */
  TOKEN_KEYWORD_CONSTEXPR =
      CDD_TOKEN_KEYWORD_CONSTEXPR, /**< Alias for 'constexpr' */
  TOKEN_KEYWORD_STATIC_ASSERT,     /**< '_Static_assert' or 'static_assert' */
  CDD_TOKEN_KEYWORD_STATIC_ASSERT =
      TOKEN_KEYWORD_STATIC_ASSERT, /**< Alias for 'static_assert' */
  TOKEN_KEYWORD_NULLPTR,           /**< 'nullptr' */
  CDD_TOKEN_KEYWORD_NULLPTR = TOKEN_KEYWORD_NULLPTR, /**< Alias for 'nullptr' */
  TOKEN_KEYWORD_TRUE,                                /**< 'true' */
  CDD_TOKEN_KEYWORD_TRUE = TOKEN_KEYWORD_TRUE,       /**< Alias for 'true' */
  TOKEN_KEYWORD_FALSE,                               /**< 'false' */
  CDD_TOKEN_KEYWORD_FALSE = TOKEN_KEYWORD_FALSE,     /**< Alias for 'false' */
  TOKEN_KEYWORD_EMBED,                               /**< 'embed' */
  CDD_TOKEN_KEYWORD_EMBED = TOKEN_KEYWORD_EMBED,     /**< Alias for 'embed' */
  TOKEN_KEYWORD_PRAGMA_OP,                           /**< '_Pragma' */
  CDD_TOKEN_KEYWORD_PRAGMA_OP =
      TOKEN_KEYWORD_PRAGMA_OP, /**< Alias for '_Pragma' */
  TOKEN_KEYWORD_ATTRIBUTE,     /**< '__attribute__' */
  CDD_TOKEN_KEYWORD_ATTRIBUTE =
      TOKEN_KEYWORD_ATTRIBUTE, /**< Alias for '__attribute__' */
  CDD_TOKEN_KEYWORD_CLASS,     /**< 'class' */
  CDD_TOKEN_KEYWORD_PUBLIC,    /**< 'public' */
  CDD_TOKEN_KEYWORD_PRIVATE,   /**< 'private' */
  CDD_TOKEN_KEYWORD_PROTECTED, /**< 'protected' */
  CDD_TOKEN_KEYWORD_VIRTUAL,   /**< 'virtual' */
  CDD_TOKEN_KEYWORD_TEMPLATE,  /**< 'template' */
  CDD_TOKEN_KEYWORD_TYPENAME,  /**< 'typename' */
  CDD_TOKEN_KEYWORD_NEW,       /**< 'new' */
  CDD_TOKEN_KEYWORD_DELETE,    /**< 'delete' */
  CDD_TOKEN_KEYWORD_NAMESPACE, /**< 'namespace' */
  CDD_TOKEN_KEYWORD_USING,     /**< 'using' */
  CDD_TOKEN_KEYWORD_OPERATOR,  /**< 'operator' */
  CDD_TOKEN_KEYWORD_TRY,       /**< 'try' */
  CDD_TOKEN_KEYWORD_CATCH,     /**< 'catch' */
  CDD_TOKEN_KEYWORD_THROW,     /**< 'throw' */
  CDD_TOKEN_KEYWORD_NOEXCEPT,  /**< 'noexcept' */
  TOKEN_KEYWORD_DECLSPEC,      /**< '__declspec' */
  CDD_TOKEN_KEYWORD_DECLSPEC =
      TOKEN_KEYWORD_DECLSPEC, /**< Alias for '__declspec' */

  /* --- Identifiers & Literals --- */
  CDD_TOKEN_IDENTIFIER,                    /**< Identifier */
  TOKEN_IDENTIFIER = CDD_TOKEN_IDENTIFIER, /**< Alias for identifier */
  CDD_TOKEN_NUMBER,                        /**< Numeric literal */
  TOKEN_NUMBER_LITERAL = CDD_TOKEN_NUMBER, /**< Alias for numeric literal */
  CDD_TOKEN_STRING,                        /**< String literal */
  TOKEN_STRING_LITERAL = CDD_TOKEN_STRING, /**< Alias for string literal */
  CDD_TOKEN_CHAR,                          /**< Character literal */
  TOKEN_CHAR_LITERAL = CDD_TOKEN_CHAR,     /**< Alias for character literal */

  /* --- Punctuators & Operators --- */
  CDD_TOKEN_LBRACE,                              /**< '{' */
  TOKEN_LBRACE = CDD_TOKEN_LBRACE,               /**< Alias for '{' */
  CDD_TOKEN_RBRACE,                              /**< '}' */
  TOKEN_RBRACE = CDD_TOKEN_RBRACE,               /**< Alias for '}' */
  CDD_TOKEN_LPAREN,                              /**< '(' */
  TOKEN_LPAREN = CDD_TOKEN_LPAREN,               /**< Alias for '(' */
  CDD_TOKEN_RPAREN,                              /**< ')' */
  TOKEN_RPAREN = CDD_TOKEN_RPAREN,               /**< Alias for ')' */
  CDD_TOKEN_LBRACKET,                            /**< '[' */
  TOKEN_LBRACKET = CDD_TOKEN_LBRACKET,           /**< Alias for '[' */
  CDD_TOKEN_RBRACKET,                            /**< ']' */
  TOKEN_RBRACKET = CDD_TOKEN_RBRACKET,           /**< Alias for ']' */
  CDD_TOKEN_SEMICOLON,                           /**< ';' */
  TOKEN_SEMICOLON = CDD_TOKEN_SEMICOLON,         /**< Alias for ';' */
  CDD_TOKEN_COMMA,                               /**< ',' */
  TOKEN_COMMA = CDD_TOKEN_COMMA,                 /**< Alias for ',' */
  CDD_TOKEN_DOT,                                 /**< '.' */
  TOKEN_DOT = CDD_TOKEN_DOT,                     /**< Alias for '.' */
  CDD_TOKEN_ARROW,                               /**< '->' */
  TOKEN_ARROW = CDD_TOKEN_ARROW,                 /**< Alias for '->' */
  CDD_TOKEN_PLUS,                                /**< '+' */
  TOKEN_PLUS = CDD_TOKEN_PLUS,                   /**< Alias for '+' */
  CDD_TOKEN_MINUS,                               /**< '-' */
  TOKEN_MINUS = CDD_TOKEN_MINUS,                 /**< Alias for '-' */
  CDD_TOKEN_STAR,                                /**< '*' */
  TOKEN_STAR = CDD_TOKEN_STAR,                   /**< Alias for '*' */
  CDD_TOKEN_SLASH,                               /**< '/' */
  TOKEN_SLASH = CDD_TOKEN_SLASH,                 /**< Alias for '/' */
  TOKEN_PERCENT,                                 /**< '%' */
  CDD_TOKEN_PERCENT = TOKEN_PERCENT,             /**< Alias for '%' */
  TOKEN_AMP,                                     /**< '&' */
  CDD_TOKEN_AMP = TOKEN_AMP,                     /**< Alias for '&' */
  TOKEN_PIPE,                                    /**< '|' */
  CDD_TOKEN_PIPE = TOKEN_PIPE,                   /**< Alias for '|' */
  TOKEN_CARET,                                   /**< '^' */
  CDD_TOKEN_CARET = TOKEN_CARET,                 /**< Alias for '^' */
  CDD_TOKEN_TILDE,                               /**< '~' */
  TOKEN_TILDE = CDD_TOKEN_TILDE,                 /**< Alias for '~' */
  TOKEN_BANG,                                    /**< '!' */
  CDD_TOKEN_BANG = TOKEN_BANG,                   /**< Alias for '!' */
  TOKEN_QUESTION,                                /**< '?' */
  CDD_TOKEN_QUESTION = TOKEN_QUESTION,           /**< Alias for '?' */
  CDD_TOKEN_COLON,                               /**< ':' */
  TOKEN_COLON = CDD_TOKEN_COLON,                 /**< Alias for ':' */
  CDD_TOKEN_ASSIGN,                              /**< '=' */
  TOKEN_ASSIGN = CDD_TOKEN_ASSIGN,               /**< Alias for '=' */
  CDD_TOKEN_LT,                                  /**< '<' */
  TOKEN_LESS = CDD_TOKEN_LT,                     /**< Alias for '<' */
  CDD_TOKEN_GT,                                  /**< '>' */
  TOKEN_GREATER = CDD_TOKEN_GT,                  /**< Alias for '>' */
  CDD_TOKEN_EQ,                                  /**< '==' */
  TOKEN_EQ = CDD_TOKEN_EQ,                       /**< Alias for '==' */
  CDD_TOKEN_NEQ,                                 /**< '!=' */
  TOKEN_NEQ = CDD_TOKEN_NEQ,                     /**< Alias for '!=' */
  TOKEN_LEQ,                                     /**< '<=' */
  CDD_TOKEN_LEQ = TOKEN_LEQ,                     /**< Alias for '<=' */
  TOKEN_GEQ,                                     /**< '>=' */
  CDD_TOKEN_GEQ = TOKEN_GEQ,                     /**< Alias for '>=' */
  TOKEN_LOGICAL_AND,                             /**< '&&' */
  CDD_TOKEN_LOGICAL_AND = TOKEN_LOGICAL_AND,     /**< Alias for '&&' */
  TOKEN_LOGICAL_OR,                              /**< '||' */
  CDD_TOKEN_LOGICAL_OR = TOKEN_LOGICAL_OR,       /**< Alias for '||' */
  TOKEN_INC,                                     /**< '++' */
  CDD_TOKEN_INC = TOKEN_INC,                     /**< Alias for '++' */
  TOKEN_DEC,                                     /**< '--' */
  CDD_TOKEN_DEC = TOKEN_DEC,                     /**< Alias for '--' */
  TOKEN_PLUS_ASSIGN,                             /**< '+=' */
  CDD_TOKEN_PLUS_ASSIGN = TOKEN_PLUS_ASSIGN,     /**< Alias for '+=' */
  TOKEN_MINUS_ASSIGN,                            /**< '-=' */
  CDD_TOKEN_MINUS_ASSIGN = TOKEN_MINUS_ASSIGN,   /**< Alias for '-=' */
  TOKEN_MUL_ASSIGN,                              /**< '*=' */
  CDD_TOKEN_MUL_ASSIGN = TOKEN_MUL_ASSIGN,       /**< Alias for '*=' */
  TOKEN_DIV_ASSIGN,                              /**< '/=' */
  CDD_TOKEN_DIV_ASSIGN = TOKEN_DIV_ASSIGN,       /**< Alias for '/=' */
  TOKEN_MOD_ASSIGN,                              /**< '%=' */
  CDD_TOKEN_MOD_ASSIGN = TOKEN_MOD_ASSIGN,       /**< Alias for '%=' */
  TOKEN_AND_ASSIGN,                              /**< '&=' */
  CDD_TOKEN_AND_ASSIGN = TOKEN_AND_ASSIGN,       /**< Alias for '&=' */
  TOKEN_OR_ASSIGN,                               /**< '|=' */
  CDD_TOKEN_OR_ASSIGN = TOKEN_OR_ASSIGN,         /**< Alias for '|=' */
  TOKEN_XOR_ASSIGN,                              /**< '^=' */
  CDD_TOKEN_XOR_ASSIGN = TOKEN_XOR_ASSIGN,       /**< Alias for '^=' */
  TOKEN_LSHIFT,                                  /**< '<<' */
  CDD_TOKEN_LSHIFT = TOKEN_LSHIFT,               /**< Alias for '<<' */
  TOKEN_RSHIFT,                                  /**< '>>' */
  CDD_TOKEN_RSHIFT = TOKEN_RSHIFT,               /**< Alias for '>>' */
  TOKEN_LSHIFT_ASSIGN,                           /**< '<<=' */
  CDD_TOKEN_LSHIFT_ASSIGN = TOKEN_LSHIFT_ASSIGN, /**< Alias for '<<=' */
  TOKEN_RSHIFT_ASSIGN,                           /**< '>>=' */
  CDD_TOKEN_RSHIFT_ASSIGN = TOKEN_RSHIFT_ASSIGN, /**< Alias for '>>=' */
  TOKEN_ELLIPSIS,                                /**< '...' */
  CDD_TOKEN_ELLIPSIS = TOKEN_ELLIPSIS,           /**< Alias for '...' */
  TOKEN_HASH,                                    /**< '\#' */
  CDD_TOKEN_HASH = TOKEN_HASH,                   /**< Alias for '\#' */
  TOKEN_HASH_HASH,                               /**< '\#\#' */
  CDD_TOKEN_HASH_HASH = TOKEN_HASH_HASH,         /**< Alias for '\#\#' */

  /* --- Preprocessor Directives (contiguous for range checks) --- */
  CDD_TOKEN_PREPROC_INCLUDE, /**< '\#include' / '\#include_next' */
  CDD_TOKEN_PREPROC_DEFINE,  /**< '\#define' */
  CDD_TOKEN_PREPROC_IFDEF,   /**< '\#ifdef' / '\#if' */
  CDD_TOKEN_PREPROC_ELIF,    /**< '\#elif' */
  CDD_TOKEN_PREPROC_ELSE,    /**< '\#else' */
  CDD_TOKEN_PREPROC_IFNDEF,  /**< '\#ifndef' */
  CDD_TOKEN_PREPROC_ENDIF,   /**< '\#endif' */
  CDD_TOKEN_PREPROC_PRAGMA,  /**< '\#pragma' */

  CDD_TOKEN_OTHER,               /**< Unclassified symbol */
  TOKEN_OTHER = CDD_TOKEN_OTHER, /**< Alias for unclassified symbol */
  CDD_TOKEN_EOF,                 /**< End-of-file sentinel */
  TOKEN_UNKNOWN = -1             /**< Error sentinel */
};

/** @brief Compatibility alias for legacy enum TokenKind */
#define TokenKind cdd_token_kind_t

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

/** @brief Compatibility alias for legacy struct Token */
#define Token cdd_token_t

/**
 * @brief A list of tokens.
 */
typedef struct cdd_token_list_t cdd_token_list_t;
/** @brief Struct definition */
struct cdd_token_list_t {
  cdd_token_t *tokens; /**< Array of token structures */
  size_t size;         /**< Number of valid tokens used */
  size_t capacity;     /**< Allocated capacity of the array */
};

/** @brief Compatibility alias for legacy struct TokenList */
#define TokenList cdd_token_list_t

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_TOKEN_H */
