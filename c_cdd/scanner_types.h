#ifndef C_CDD_SCANNER_TYPES_H
#define C_CDD_SCANNER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"

enum ScannerKind {
  UnknownScan,

  /* except one newline char that terminates CppComment or Macro */
  Whitespace,

  Macro,

  /*Integer,
  Decimal,*/
  DoubleQuoted,
  SingleQuoted,

  CComment,
  CppComment,

  Lbrace,
  Rbrace,
  Lsquare,
  Rsquare,
  Lparen,
  Rparen,

  Equal,
  Equality,
  GreaterThan,
  GreaterThanEqual,
  LessThan,
  LessThanEqual,

  RightShift,
  LeftShift,

  /* unary */
  And,
  Asterisk,
  Plus,
  Sub,
  Tilde,
  Exclamation,

  Increment,
  Decrement,

  Divide,
  Caret,
  Pipe,

  LogicalAnd,
  LogicalOr,

  Question,
  Colon,

  Ellipsis,

  Word,

  Comma,
  Terminator /* ; */
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
