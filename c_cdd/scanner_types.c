#include "scanner_types.h"

const char *ScannerKind_to_str(const enum ScannerKind kind) {
  switch (kind) {
  case Whitespace:
    return "Whitespace";

  case Macro:
    return "Macro";

  case DoubleQuoted:
    return "DoubleQuoted";

  case SingleQuoted:
    return "SingleQuoted";

  case CComment:
    return "CComment";

  case CppComment:
    return "CppComment";

  case Lbrace:
    return "Lbrace";

  case Rbrace:
    return "Rbrace";

  case Lsquare:
    return "Lsquare";

  case Rsquare:
    return "Rsquare";

  case Lparen:
    return "Lparen";

  case Rparen:
    return "Rparen";

  case Equal:
    return "Equal";

  case Equality:
    return "Equality";

  case GreaterThan:
    return "GreaterThan";

  case GreaterThanEqual:
    return "GreaterThanEqual";

  case LessThan:
    return "LessThan";

  case LessThanEqual:
    return "LessThanEqual";

  case RightShift:
    return "RightShift";

  case LeftShift:
    return "LeftShift";

  case And:
    return "And";

  case Asterisk:
    return "Asterisk";

  case Plus:
    return "Plus";

  case Sub:
    return "Sub";

  case Tilde:
    return "Tilde";

  case Exclamation:
    return "Exclamation";

  case Increment:
    return "Increment";

  case Decrement:
    return "Decrement";

  case Divide:
    return "Divide";

  case Caret:
    return "Caret";

  case Pipe:
    return "Pipe";

  case LogicalAnd:
    return "LogicalAnd";

  case LogicalOr:
    return "LogicalOr";

  case Question:
    return "Question";

  case Colon:
    return "Colon";

  case Ellipsis:
    return "Ellipsis";

  case Word:
    return "Word";

  case Comma:
    return "Comma";

  case Terminator:
    return "Terminator";

  case UnknownScan:
  default:
    return "UnknownScan";
  }
}
