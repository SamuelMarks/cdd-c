#include <assert.h>
#include <string.h>

#include "c_cdd_utils.h"
#include "str_includes.h"
#include "tokenizer_types.h"

/* "\n".join(map('case {0}##Suffix:\\\n\treturn \"{0}\";\\\n\\'.format,
 *               map(lambda _s: _s.partition(" ")[0], s.splitlines()))) */
#define KeywordCase(Suffix)                                                    \
  case _Alignas##Suffix:                                                       \
    return "_Alignas";                                                         \
                                                                               \
  case _Alignof##Suffix:                                                       \
    return "_Alignof";                                                         \
                                                                               \
  case _Atomic##Suffix:                                                        \
    return "_Atomic";                                                          \
                                                                               \
  case _BitInt##Suffix:                                                        \
    return "_BitInt";                                                          \
                                                                               \
  case _Bool##Suffix:                                                          \
    return "_Bool";                                                            \
                                                                               \
  case _Complex##Suffix:                                                       \
    return "_Complex";                                                         \
                                                                               \
  case _Decimal128##Suffix:                                                    \
    return "_Decimal128";                                                      \
                                                                               \
  case _Decimal32##Suffix:                                                     \
    return "_Decimal32";                                                       \
                                                                               \
  case _Decimal64##Suffix:                                                     \
    return "_Decimal64";                                                       \
                                                                               \
  case _Generic##Suffix:                                                       \
    return "_Generic";                                                         \
                                                                               \
  case _Imaginary##Suffix:                                                     \
    return "_Imaginary";                                                       \
                                                                               \
  case _Noreturn##Suffix:                                                      \
    return "_Noreturn";                                                        \
                                                                               \
  case _Static_assert##Suffix:                                                 \
    return "_Static_assert";                                                   \
                                                                               \
  case _Thread_local##Suffix:                                                  \
    return "_Thread_local";                                                    \
                                                                               \
  case alignas##Suffix:                                                        \
    return "alignas";                                                          \
                                                                               \
  case alignof##Suffix:                                                        \
    return "alignof";                                                          \
                                                                               \
  case auto##Suffix:                                                           \
    return "auto";                                                             \
                                                                               \
  case bool##Suffix:                                                           \
    return "bool";                                                             \
                                                                               \
  case break##Suffix:                                                          \
    return "break";                                                            \
                                                                               \
  case case##Suffix:                                                           \
    return "case";                                                             \
                                                                               \
  case char##Suffix:                                                           \
    return "char";                                                             \
                                                                               \
  case const##Suffix:                                                          \
    return "const";                                                            \
                                                                               \
  case constexpr##Suffix:                                                      \
    return "constexpr";                                                        \
                                                                               \
  case continue##Suffix:                                                       \
    return "continue";                                                         \
                                                                               \
  case default##Suffix:                                                        \
    return "default";                                                          \
                                                                               \
  case do##Suffix:                                                             \
    return "do";                                                               \
                                                                               \
  case double##Suffix:                                                         \
    return "double";                                                           \
                                                                               \
  case else##Suffix:                                                           \
    return "else";                                                             \
                                                                               \
  case enum##Suffix:                                                           \
    return "enum";                                                             \
                                                                               \
  case extern##Suffix:                                                         \
    return "extern";                                                           \
                                                                               \
  case false##Suffix:                                                          \
    return "false";                                                            \
                                                                               \
  case float##Suffix:                                                          \
    return "float";                                                            \
                                                                               \
  case for##Suffix:                                                            \
    return "for";                                                              \
                                                                               \
  case goto##Suffix:                                                           \
    return "goto";                                                             \
                                                                               \
  case if##Suffix:                                                             \
    return "if";                                                               \
                                                                               \
  case inline##Suffix:                                                         \
    return "inline";                                                           \
                                                                               \
  case int##Suffix:                                                            \
    return "int";                                                              \
                                                                               \
  case long##Suffix:                                                           \
    return "long";                                                             \
                                                                               \
  case nullptr##Suffix:                                                        \
    return "nullptr";                                                          \
                                                                               \
  case register##Suffix:                                                       \
    return "register";                                                         \
                                                                               \
  case restrict##Suffix:                                                       \
    return "restrict";                                                         \
                                                                               \
  case return ##Suffix:                                                        \
    return "return";                                                           \
                                                                               \
  case short##Suffix:                                                          \
    return "short";                                                            \
                                                                               \
  case signed##Suffix:                                                         \
    return "signed";                                                           \
                                                                               \
  case sizeof##Suffix:                                                         \
    return "sizeof";                                                           \
                                                                               \
  case static##Suffix:                                                         \
    return "static";                                                           \
                                                                               \
  case static_assert##Suffix:                                                  \
    return "static_assert";                                                    \
                                                                               \
  case struct##Suffix:                                                         \
    return "struct";                                                           \
                                                                               \
  case switch##Suffix:                                                         \
    return "switch";                                                           \
                                                                               \
  case thread_local##Suffix:                                                   \
    return "thread_local";                                                     \
                                                                               \
  case true##Suffix:                                                           \
    return "true";                                                             \
                                                                               \
  case typedef##Suffix:                                                        \
    return "typedef";                                                          \
                                                                               \
  case typeof##Suffix:                                                         \
    return "typeof";                                                           \
                                                                               \
  case typeof_unqual##Suffix:                                                  \
    return "typeof_unqual";                                                    \
                                                                               \
  case union##Suffix:                                                          \
    return "union";                                                            \
                                                                               \
  case unsigned##Suffix:                                                       \
    return "unsigned";                                                         \
                                                                               \
  case void##Suffix:                                                           \
    return "void";                                                             \
                                                                               \
  case volatile##Suffix:                                                       \
    return "volatile";                                                         \
                                                                               \
  case while##Suffix:                                                          \
    return "while";

/*
 * "\\\n".join(map("else if (strcmp(s, \"{0}\") == 0)\t\\\n\t
 *       return {0}##Suffix;\t\\\n".format, map(lambda _s: _s.partition(" ")[0],
 *                 s.splitlines())))
 * */

#define KeywordCmp(Suffix)                                                     \
  if (strcmp(s, "_Alignas") == 0)                                              \
    return _Alignas##Suffix;                                                   \
                                                                               \
  else if (strcmp(s, "_Alignof") == 0)                                         \
    return _Alignof##Suffix;                                                   \
                                                                               \
  else if (strcmp(s, "_Atomic") == 0)                                          \
    return _Atomic##Suffix;                                                    \
                                                                               \
  else if (strcmp(s, "_BitInt") == 0)                                          \
    return _BitInt##Suffix;                                                    \
                                                                               \
  else if (strcmp(s, "_Bool") == 0)                                            \
    return _Bool##Suffix;                                                      \
                                                                               \
  else if (strcmp(s, "_Complex") == 0)                                         \
    return _Complex##Suffix;                                                   \
                                                                               \
  else if (strcmp(s, "_Decimal128") == 0)                                      \
    return _Decimal128##Suffix;                                                \
                                                                               \
  else if (strcmp(s, "_Decimal32") == 0)                                       \
    return _Decimal32##Suffix;                                                 \
                                                                               \
  else if (strcmp(s, "_Decimal64") == 0)                                       \
    return _Decimal64##Suffix;                                                 \
                                                                               \
  else if (strcmp(s, "_Generic") == 0)                                         \
    return _Generic##Suffix;                                                   \
                                                                               \
  else if (strcmp(s, "_Imaginary") == 0)                                       \
    return _Imaginary##Suffix;                                                 \
                                                                               \
  else if (strcmp(s, "_Noreturn") == 0)                                        \
    return _Noreturn##Suffix;                                                  \
                                                                               \
  else if (strcmp(s, "_Static_assert") == 0)                                   \
    return _Static_assert##Suffix;                                             \
                                                                               \
  else if (strcmp(s, "_Thread_local") == 0)                                    \
    return _Thread_local##Suffix;                                              \
                                                                               \
  else if (strcmp(s, "alignas") == 0)                                          \
    return alignas##Suffix;                                                    \
                                                                               \
  else if (strcmp(s, "alignof") == 0)                                          \
    return alignof##Suffix;                                                    \
                                                                               \
  else if (strcmp(s, "auto") == 0)                                             \
    return auto##Suffix;                                                       \
                                                                               \
  else if (strcmp(s, "bool") == 0)                                             \
    return bool##Suffix;                                                       \
                                                                               \
  else if (strcmp(s, "break") == 0)                                            \
    return break##Suffix;                                                      \
                                                                               \
  else if (strcmp(s, "case") == 0)                                             \
    return case##Suffix;                                                       \
                                                                               \
  else if (strcmp(s, "char") == 0)                                             \
    return char##Suffix;                                                       \
                                                                               \
  else if (strcmp(s, "const") == 0)                                            \
    return const##Suffix;                                                      \
                                                                               \
  else if (strcmp(s, "constexpr") == 0)                                        \
    return constexpr##Suffix;                                                  \
                                                                               \
  else if (strcmp(s, "continue") == 0)                                         \
    return continue##Suffix;                                                   \
                                                                               \
  else if (strcmp(s, "default") == 0)                                          \
    return default##Suffix;                                                    \
                                                                               \
  else if (strcmp(s, "do") == 0)                                               \
    return do##Suffix;                                                         \
                                                                               \
  else if (strcmp(s, "double") == 0)                                           \
    return double##Suffix;                                                     \
                                                                               \
  else if (strcmp(s, "else") == 0)                                             \
    return else##Suffix;                                                       \
                                                                               \
  else if (strcmp(s, "enum") == 0)                                             \
    return enum##Suffix;                                                       \
                                                                               \
  else if (strcmp(s, "extern") == 0)                                           \
    return extern##Suffix;                                                     \
                                                                               \
  else if (strcmp(s, "false") == 0)                                            \
    return false##Suffix;                                                      \
                                                                               \
  else if (strcmp(s, "float") == 0)                                            \
    return float##Suffix;                                                      \
                                                                               \
  else if (strcmp(s, "for") == 0)                                              \
          return for##Suffix;                                                  \
                                                                               \
  else if (strcmp(s, "goto") == 0)                                             \
    return goto##Suffix;                                                       \
                                                                               \
  else if (strcmp(s, "if") == 0)                                               \
    return if##Suffix;                                                         \
                                                                               \
  else if (strcmp(s, "inline") == 0)                                           \
    return inline##Suffix;                                                     \
                                                                               \
  else if (strcmp(s, "int") == 0)                                              \
    return int##Suffix;                                                        \
                                                                               \
  else if (strcmp(s, "long") == 0)                                             \
    return long##Suffix;                                                       \
                                                                               \
  else if (strcmp(s, "nullptr") == 0)                                          \
    return nullptr##Suffix;                                                    \
                                                                               \
  else if (strcmp(s, "register") == 0)                                         \
    return register##Suffix;                                                   \
                                                                               \
  else if (strcmp(s, "restrict") == 0)                                         \
    return restrict##Suffix;                                                   \
                                                                               \
  else if (strcmp(s, "return") == 0)                                           \
    return return ##Suffix;                                                    \
                                                                               \
  else if (strcmp(s, "short") == 0)                                            \
    return short##Suffix;                                                      \
                                                                               \
  else if (strcmp(s, "signed") == 0)                                           \
    return signed##Suffix;                                                     \
                                                                               \
  else if (strcmp(s, "sizeof") == 0)                                           \
    return sizeof##Suffix;                                                     \
                                                                               \
  else if (strcmp(s, "static") == 0)                                           \
    return static##Suffix;                                                     \
                                                                               \
  else if (strcmp(s, "static_assert") == 0)                                    \
    return static_assert##Suffix;                                              \
                                                                               \
  else if (strcmp(s, "struct") == 0)                                           \
    return struct##Suffix;                                                     \
                                                                               \
  else if (strcmp(s, "switch") == 0)                                           \
    return switch##Suffix;                                                     \
                                                                               \
  else if (strcmp(s, "thread_local") == 0)                                     \
    return thread_local##Suffix;                                               \
                                                                               \
  else if (strcmp(s, "true") == 0)                                             \
    return true##Suffix;                                                       \
                                                                               \
  else if (strcmp(s, "typedef") == 0)                                          \
    return typedef##Suffix;                                                    \
                                                                               \
  else if (strcmp(s, "typeof") == 0)                                           \
    return typeof##Suffix;                                                     \
                                                                               \
  else if (strcmp(s, "typeof_unqual") == 0)                                    \
    return typeof_unqual##Suffix;                                              \
                                                                               \
  else if (strcmp(s, "union") == 0)                                            \
    return union##Suffix;                                                      \
                                                                               \
  else if (strcmp(s, "unsigned") == 0)                                         \
    return unsigned##Suffix;                                                   \
                                                                               \
  else if (strcmp(s, "void") == 0)                                             \
    return void##Suffix;                                                       \
                                                                               \
  else if (strcmp(s, "volatile") == 0)                                         \
    return volatile##Suffix;                                                   \
                                                                               \
  else if (strcmp(s, "while") == 0)                                            \
    return while##Suffix;

const char *TokenizerKind_to_str(const enum TokenizerKind kind) {
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

    KeywordCase(Keyword);

  case UNKNOWN_SCAN:
  default:
    return "UNKNOWN_SCAN";
  }
}

enum TokenizerKind str_to_TokenizerKind(const char *const s) {
  /* "\n".join(map(lambda s: "else if (strcmp(s, \"{0}\") == 0)\n\treturn
   * {0};\n".format(s), map(str.strip, s.split(','))))*/

  /* TODO: Use a set for O(1) membership querying instead of `strcmp` in these
   * big `else if`s */

  if (strcmp(s, "WHITESPACE") == 0)
    return WHITESPACE;

  else if (strcmp(s, "MACRO") == 0)
    return MACRO;

  else if (strcmp(s, "DOUBLE_QUOTED") == 0)
    return DOUBLE_QUOTED;

  else if (strcmp(s, "SINGLE_QUOTED") == 0)
    return SINGLE_QUOTED;

  else if (strcmp(s, "C_COMMENT") == 0)
    return C_COMMENT;

  else if (strcmp(s, "CPP_COMMENT") == 0)
    return CPP_COMMENT;

  else if (strcmp(s, "LBRACE") == 0)
    return LBRACE;

  else if (strcmp(s, "RBRACE") == 0)
    return RBRACE;

  else if (strcmp(s, "LSQUARE") == 0)
    return LSQUARE;

  else if (strcmp(s, "RSQUARE") == 0)
    return RSQUARE;

  else if (strcmp(s, "LPAREN") == 0)
    return LPAREN;

  else if (strcmp(s, "RPAREN") == 0)
    return RPAREN;

  else if (strcmp(s, "EQUAL") == 0)
    return EQUAL;

  else if (strcmp(s, "EQ_OP") == 0)
    return EQ_OP;

  else if (strcmp(s, "GREATER_THAN") == 0)
    return GREATER_THAN;

  else if (strcmp(s, "GE_OP") == 0)
    return GE_OP;

  else if (strcmp(s, "LESS_THAN") == 0)
    return LESS_THAN;

  else if (strcmp(s, "LE_OP") == 0)
    return LE_OP;

  else if (strcmp(s, "NE_OP") == 0)
    return NE_OP;

  else if (strcmp(s, "RIGHT_SHIFT") == 0)
    return RIGHT_SHIFT;

  else if (strcmp(s, "LEFT_SHIFT") == 0)
    return LEFT_SHIFT;

  else if (strcmp(s, "AND") == 0)
    return AND;

  else if (strcmp(s, "ASTERISK") == 0)
    return ASTERISK;

  else if (strcmp(s, "PLUS") == 0)
    return PLUS;

  else if (strcmp(s, "SUB") == 0)
    return SUB;

  else if (strcmp(s, "TILDE") == 0)
    return TILDE;

  else if (strcmp(s, "EXCLAMATION") == 0)
    return EXCLAMATION;

  else if (strcmp(s, "MODULO") == 0)
    return MODULO;

  else if (strcmp(s, "INC_OP") == 0)
    return INC_OP;

  else if (strcmp(s, "DEC_OP") == 0)
    return DEC_OP;

  else if (strcmp(s, "PTR_OP") == 0)
    return PTR_OP;

  else if (strcmp(s, "DIVIDE") == 0)
    return DIVIDE;

  else if (strcmp(s, "CARET") == 0)
    return CARET;

  else if (strcmp(s, "PIPE") == 0)
    return PIPE;

  else if (strcmp(s, "AND_OP") == 0)
    return AND_OP;

  else if (strcmp(s, "OR_OP") == 0)
    return OR_OP;

  else if (strcmp(s, "MUL_ASSIGN") == 0)
    return MUL_ASSIGN;

  else if (strcmp(s, "DIV_ASSIGN") == 0)
    return DIV_ASSIGN;

  else if (strcmp(s, "MOD_ASSIGN") == 0)
    return MOD_ASSIGN;

  else if (strcmp(s, "ADD_ASSIGN") == 0)
    return ADD_ASSIGN;

  else if (strcmp(s, "SUB_ASSIGN") == 0)
    return SUB_ASSIGN;

  else if (strcmp(s, "LEFT_ASSIGN") == 0)
    return LEFT_ASSIGN;

  else if (strcmp(s, "RIGHT_ASSIGN") == 0)
    return RIGHT_ASSIGN;

  else if (strcmp(s, "AND_ASSIGN") == 0)
    return AND_ASSIGN;

  else if (strcmp(s, "XOR_ASSIGN") == 0)
    return XOR_ASSIGN;

  else if (strcmp(s, "OR_ASSIGN") == 0)
    return OR_ASSIGN;

  else if (strcmp(s, "QUESTION") == 0)
    return QUESTION;

  else if (strcmp(s, "COLON") == 0)
    return COLON;

  else if (strcmp(s, "ELLIPSIS") == 0)
    return ELLIPSIS;

  else if (strcmp(s, "WORD") == 0)
    return WORD;

  else if (strcmp(s, "COMMA") == 0)
    return COMMA;

  else if (strcmp(s, "TERMINATOR") == 0)
    return TERMINATOR;

  else
    KeywordCmp(Keyword);

  /* else if (strcmp(s, "UNKNOWN_SCAN") == 0) */
  return UNKNOWN_SCAN;
}

const char *TokenKeyword_to_str(const enum TokenKeyword token_keyword) {
  /* "\n".join(map(lambda _s: "case {0}Keyword:\n\treturn
   * \"{0}\";\n".format(_s), map(lambda _s: _s.rpartition('"')[0][1:],
   * s.splitlines()))) */
  switch (token_keyword) {
    KeywordCase(Token);

  case unknownKeyword:
  default:
    return "unknownKeyword";
  }
}

enum TokenKeyword str_to_TokenKeyword(const char *const s) {
  /* TODO: Use a set for O(1) membership querying instead of `strcmp` in these
   * big `else if`s */

  KeywordCmp(Token);

  return unknownKeyword;
}

void tokenizer_az_span_arr_print(
    const struct tokenizer_az_span_arr *const tokens_arr) {
  size_t i;
  for (i = 0; i < tokens_arr->size; i++) {
    char *name = NULL;
    assert(az_span_ptr(tokens_arr->elem[i].span) != NULL);
    assert(tokens_arr->elem[i].kind != UNKNOWN_SCAN);
    asprintf(&name, "array::tokens_arr[%" NUM_LONG_FMT "d]:%s", i,
             TokenizerKind_to_str(tokens_arr->elem[i].kind));
    print_escaped_span(name, tokens_arr->elem[i].span);
    free(name);
  }
}

#undef KeywordCase
#undef KeywordCmp
