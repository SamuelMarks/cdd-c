#include "tokenizer_types.h"

const char *ScannerKind_to_str(const enum ScannerKind kind) {
  /* "\n".join(map(lambda _s: "case {0}:\n\treturn \"{0}\";\n".format(_s),
   * map(str.strip, s.split(',')))) */
  switch (kind) {
  case WHITESPACE:
    return "WHITESPACE";

  case MACRO:
    return "MACRO";

  case DOUBLE_QUOTED:
    return "DOUBLE_QUOTED";

  case SINGLE_QUOTED:
    return "SINGLE_QUOTED";

  case C_COMMENT:
    return "C_COMMENT";

  case CPP_COMMENT:
    return "CPP_COMMENT";

  case LBRACE:
    return "LBRACE";

  case RBRACE:
    return "RBRACE";

  case LSQUARE:
    return "LSQUARE";

  case RSQUARE:
    return "RSQUARE";

  case LPAREN:
    return "LPAREN";

  case RPAREN:
    return "RPAREN";

  case EQUAL:
    return "EQUAL";

  case EQ_OP:
    return "EQ_OP";

  case GREATER_THAN:
    return "GREATER_THAN";

  case GE_OP:
    return "GE_OP";

  case LESS_THAN:
    return "LESS_THAN";

  case LE_OP:
    return "LE_OP";

  case NE_OP:
    return "NE_OP";

  case RIGHT_SHIFT:
    return "RIGHT_SHIFT";

  case LEFT_SHIFT:
    return "LEFT_SHIFT";

  case AND:
    return "AND";

  case ASTERISK:
    return "ASTERISK";

  case PLUS:
    return "PLUS";

  case SUB:
    return "SUB";

  case TILDE:
    return "TILDE";

  case EXCLAMATION:
    return "EXCLAMATION";

  case MODULO:
    return "MODULO";

  case INC_OP:
    return "INC_OP";

  case DEC_OP:
    return "DEC_OP";

  case PTR_OP:
    return "PTR_OP";

  case DIVIDE:
    return "DIVIDE";

  case CARET:
    return "CARET";

  case PIPE:
    return "PIPE";

  case AND_OP:
    return "AND_OP";

  case OR_OP:
    return "OR_OP";

  case MUL_ASSIGN:
    return "MUL_ASSIGN";

  case DIV_ASSIGN:
    return "DIV_ASSIGN";

  case MOD_ASSIGN:
    return "MOD_ASSIGN";

  case ADD_ASSIGN:
    return "ADD_ASSIGN";

  case SUB_ASSIGN:
    return "SUB_ASSIGN";

  case LEFT_ASSIGN:
    return "LEFT_ASSIGN";

  case RIGHT_ASSIGN:
    return "RIGHT_ASSIGN";

  case AND_ASSIGN:
    return "AND_ASSIGN";

  case XOR_ASSIGN:
    return "XOR_ASSIGN";

  case OR_ASSIGN:
    return "OR_ASSIGN";

  case QUESTION:
    return "QUESTION";

  case COLON:
    return "COLON";

  case ELLIPSIS:
    return "ELLIPSIS";

  case WORD:
    return "WORD";

  case COMMA:
    return "COMMA";

  case TERMINATOR:
    return "TERMINATOR";

  case UNKNOWN_SCAN:
  default:
    return "UNKNOWN_SCAN";
  }
}
