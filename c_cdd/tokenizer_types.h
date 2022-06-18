#ifndef C_CDD_SCANNER_TYPES_H
#define C_CDD_SCANNER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>

enum ScannerKind {
  UNKNOWN_SCAN,

  /* except one newline char that terminates CPP_COMMENT or MACRO */
  WHITESPACE,

  MACRO,

  /*Integer,
  Decimal,*/
  DOUBLE_QUOTED,
  SINGLE_QUOTED,

  C_COMMENT,
  CPP_COMMENT,

  LBRACE,
  RBRACE,
  LSQUARE,
  RSQUARE,
  LPAREN,
  RPAREN,

  EQUAL,
  EQ_OP,
  GREATER_THAN,
  GE_OP,
  LESS_THAN,
  LE_OP,
  NE_OP,

  RIGHT_SHIFT,
  LEFT_SHIFT,

  /* unary */
  AND,
  ASTERISK,
  PLUS,
  SUB,
  TILDE,
  EXCLAMATION,
  MODULO,

  INC_OP,
  DEC_OP,

  PTR_OP,

  DIVIDE,
  CARET,
  PIPE,

  AND_OP,
  OR_OP,

  MUL_ASSIGN,
  DIV_ASSIGN,
  MOD_ASSIGN,
  ADD_ASSIGN,
  SUB_ASSIGN,
  LEFT_ASSIGN,
  RIGHT_ASSIGN,
  AND_ASSIGN,
  XOR_ASSIGN,
  OR_ASSIGN,

  QUESTION,
  COLON,

  ELLIPSIS,

  WORD,

  COMMA,
  TERMINATOR /* ; */
};

struct StrScannerKind {
  char *s;
  enum ScannerKind kind;
};

extern C_CDD_EXPORT const char *ScannerKind_to_str(enum ScannerKind);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_SCANNER_TYPES_H */
