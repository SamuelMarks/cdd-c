#ifndef CDD_DSL_TOKEN_H
#define CDD_DSL_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <stddef.h>
/* clang-format on */

/**
 * @enum cdd_dsl_token_kind_t
 * @brief Represents the kind of a token in the Human IR DSL.
 */
typedef enum cdd_dsl_token_kind_t {
  /** @brief End of file. */
  CDD_DSL_TOKEN_EOF,

  /** @brief Identifier (e.g., my_func, DataPacket). */
  CDD_DSL_TOKEN_IDENTIFIER,
  /** @brief Number literal (e.g., 1024, 0xFA, 1'000). */
  CDD_DSL_TOKEN_NUMBER,
  /** @brief String literal used in escapes. */
  CDD_DSL_TOKEN_STRING,

  /* Keywords */
  /** @brief 'func' keyword. */
  CDD_DSL_TOKEN_KW_FUNC,
  /** @brief 'struct' keyword. */
  CDD_DSL_TOKEN_KW_STRUCT,
  /** @brief 'enum' keyword. */
  CDD_DSL_TOKEN_KW_ENUM,
  /** @brief 'typedef' keyword. */
  CDD_DSL_TOKEN_KW_TYPEDEF,
  /** @brief 'namespace' keyword. */
  CDD_DSL_TOKEN_KW_NAMESPACE,
  /** @brief 'union' keyword. */
  CDD_DSL_TOKEN_KW_UNION,
  /** @brief 'macro_const' keyword. */
  CDD_DSL_TOKEN_KW_MACRO_CONST,
  /** @brief 'macro_expr' keyword. */
  CDD_DSL_TOKEN_KW_MACRO_EXPR,
  /** @brief 'macro_stmt' keyword. */
  CDD_DSL_TOKEN_KW_MACRO_STMT,
  /** @brief 'body' keyword. */
  CDD_DSL_TOKEN_KW_BODY,
  /** @brief 'virtual' keyword. */
  CDD_DSL_TOKEN_KW_VIRTUAL,
  /** @brief 'pure' keyword. */
  CDD_DSL_TOKEN_KW_PURE,
  /** @brief 'in' keyword. */
  CDD_DSL_TOKEN_KW_IN,
  /** @brief 'out' keyword. */
  CDD_DSL_TOKEN_KW_OUT,
  /** @brief 'inout' keyword. */
  CDD_DSL_TOKEN_KW_INOUT,
  /** @brief 'len' keyword. */
  CDD_DSL_TOKEN_KW_LEN,

  /* Types */
  /** @brief '*mut' modifier. */
  CDD_DSL_TOKEN_TYPE_MUT_PTR,
  /** @brief '*const' modifier. */
  CDD_DSL_TOKEN_TYPE_CONST_PTR,
  /** @brief 'void' type. */
  CDD_DSL_TOKEN_TYPE_VOID,
  /** @brief 'i8' type. */
  CDD_DSL_TOKEN_TYPE_I8,
  /** @brief 'u8' type. */
  CDD_DSL_TOKEN_TYPE_U8,
  /** @brief 'i16' type. */
  CDD_DSL_TOKEN_TYPE_I16,
  /** @brief 'u16' type. */
  CDD_DSL_TOKEN_TYPE_U16,
  /** @brief 'i32' type. */
  CDD_DSL_TOKEN_TYPE_I32,
  /** @brief 'u32' type. */
  CDD_DSL_TOKEN_TYPE_U32,
  /** @brief 'i64' type. */
  CDD_DSL_TOKEN_TYPE_I64,
  /** @brief 'u64' type. */
  CDD_DSL_TOKEN_TYPE_U64,
  /** @brief 'f32' type. */
  CDD_DSL_TOKEN_TYPE_F32,
  /** @brief 'f64' type. */
  CDD_DSL_TOKEN_TYPE_F64,
  /** @brief 'bool' type. */
  CDD_DSL_TOKEN_TYPE_BOOL,
  /** @brief 'size_t' type. */
  CDD_DSL_TOKEN_TYPE_SIZE_T,
  /** @brief 'usize' type. */
  CDD_DSL_TOKEN_TYPE_USIZE,
  /** @brief 'isize' type. */
  CDD_DSL_TOKEN_TYPE_ISIZE,
  /** @brief 'char' type. */
  CDD_DSL_TOKEN_TYPE_CHAR,

  /* Punctuation */
  /** @brief '{' */
  CDD_DSL_TOKEN_LBRACE,
  /** @brief '}' */
  CDD_DSL_TOKEN_RBRACE,
  /** @brief '(' */
  CDD_DSL_TOKEN_LPAREN,
  /** @brief ')' */
  CDD_DSL_TOKEN_RPAREN,
  /** @brief '[' */
  CDD_DSL_TOKEN_LBRACKET,
  /** @brief ']' */
  CDD_DSL_TOKEN_RBRACKET,
  /** @brief ';' */
  CDD_DSL_TOKEN_SEMICOLON,
  /** @brief ',' */
  CDD_DSL_TOKEN_COMMA,
  /** @brief '=' */
  CDD_DSL_TOKEN_EQUALS,

  /* Special Constructs */
  /** @brief '@expr' macro invocation. */
  CDD_DSL_TOKEN_MACRO_INVOKE_EXPR,
  /** @brief '@stmt' macro invocation. */
  CDD_DSL_TOKEN_MACRO_INVOKE_STMT,
  /** @brief '#!ws' trivia escape. */
  CDD_DSL_TOKEN_ESCAPE_WS,
  /** @brief '#!comment' trivia escape. */
  CDD_DSL_TOKEN_ESCAPE_COMMENT,

  /* Raw Blocks */
  /** @brief '%{' Start of raw block. */
  CDD_DSL_TOKEN_RAW_START,
  /** @brief '}%' End of raw block. */
  CDD_DSL_TOKEN_RAW_END,
  /** @brief Content inside %{ }%. */
  CDD_DSL_TOKEN_RAW_TEXT,

  /* Trivia (Implicitly gathered) */
  /** @brief Whitespace. */
  CDD_DSL_TOKEN_WHITESPACE,
  /** @brief Single line comment. */
  CDD_DSL_TOKEN_COMMENT_LINE,
  /** @brief Block comment. */
  CDD_DSL_TOKEN_COMMENT_BLOCK,

  /** @brief Unknown token. */
  CDD_DSL_TOKEN_UNKNOWN
} cdd_dsl_token_kind_t;

/**
 * @struct cdd_dsl_token_t
 * @brief Represents a single parsed token from the DSL.
 */
typedef struct cdd_dsl_token_t {
  /** @brief The kind of token. */
  cdd_dsl_token_kind_t kind;
  /** @brief Pointer to the start of the token text in the source. */
  const char *start;
  /** @brief The length of the token text. */
  size_t length;
  /** @brief Null-terminated copy of the token text (allocated). */
  char *text_copy;
  /** @brief The line number where this token appears. */
  size_t line;
  /** @brief The column number where this token starts. */
  size_t col;
} cdd_dsl_token_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_DSL_TOKEN_H */
