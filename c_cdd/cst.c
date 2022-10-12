#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include "c_cdd_utils.h"
#include "cst.h"
#include "cst_parser_helpers.h"
#include "tokenizer_helpers.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define C89STRINGUTILS_IMPLEMENTATION
#include <c89stringutils_string_extras.h>
#define NUM_LONG_FMT "z"
#else
#define NUM_LONG_FMT "l"
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

#define BOOLALPHA(e) ((e) ? "true" : "false")

struct TokenizerVars {
  ssize_t c_comment_char_at, cpp_comment_char_at, line_continuation_at;
  uint32_t spaces;
  uint32_t lparen, rparen, lsquare, rsquare, lbrace, rbrace, lchev, rchev;
  bool in_c_comment, in_cpp_comment, in_single, in_double, in_macro, in_init,
      is_digit;
};

struct TokenizerVars *clear_sv(struct TokenizerVars *);

az_span make_slice_clear_vars(az_span source, size_t i, size_t *start_index,
                              struct TokenizerVars *sv, bool always_make_expr);

void az_span_list_push_valid(const az_span *source,
                             struct tokenizer_az_span_list *ll, size_t i,
                             struct TokenizerVars *sv,
                             struct tokenizer_az_span_elem ***tokenized_cur_ptr,
                             size_t *start_index);

void print_tokenizer_az_span_list(const struct tokenizer_az_span_list *token_ll,
                                  size_t i);

struct tokenizer_az_span_list *tokenizer(const az_span source) {
  struct tokenizer_az_span_elem *tokenized_ll = NULL;
  struct tokenizer_az_span_elem **tokenized_cur_ptr = &tokenized_ll;

  struct tokenizer_az_span_list *ll = calloc(1, sizeof *ll);

  size_t i;
  const size_t source_n = az_span_size(source);
  const uint8_t *const span_ptr = az_span_ptr(source);

  /* Tokenizer algorithm:
   *   0. Use last 2 chars—3 on comments—to determine type;
   *   1. Pass to type-eating function, returning whence finished reading;
   *   2. Repeat from 0. until end of `source`.
   *
   * Tokenizer types (line-continuation aware):
   *   - whitespace   [ \t\v\n]+
   *   - macro        [if|ifdef|ifndef|elif|endif|include|define]
   *   - terminator   ;
   *   - parentheses  {}[]()
   *   - word         space or comment separated that doesn't match^
   */

  for (i = 0; i < source_n; i++) {
    const uint8_t next_ch = span_ptr[i + 1], ch = span_ptr[i],
                  last_ch = span_ptr[i - 1] /* NUL when i=0 */;
    bool handled = false;

    if (last_ch == '/' && (i < 2 || span_ptr[i - 2] != '\\')) {
      /* Handle comments */
      switch (ch) {
      case '*':
        i = eatCComment(&source, i - 1, source_n, &tokenized_cur_ptr, ll);
        handled = true;
        break;

      case '/':
        if (span_ptr[i - 2] != '*') {
          /* ^ handle consecutive C-style comments `/\*bar*\/\*foo*\/` */
          i = eatCppComment(&source, i - 1, source_n, &tokenized_cur_ptr, ll);
          handled = true;
        }
        break;

      default:
        break;
      }
    }

    if (!handled && last_ch != '\\')
      switch (ch) {
      /* Handle macros */
      case '#':
        i = eatMacro(&source, i, source_n, &tokenized_cur_ptr, ll);
        break;

      /* Handle literals (single and double-quoted) */
      case '\'':
        i = eatCharLiteral(&source, i, source_n, &tokenized_cur_ptr, ll);
        break;

      case '"':
        i = eatStrLiteral(&source, i, source_n, &tokenized_cur_ptr, ll);
        break;

      case ' ':
      case '\n':
      case '\r':
      case '\t':
      case '\v':
        i = eatWhitespace(&source, i, source_n, &tokenized_cur_ptr, ll);
        break;

      case '{':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, LBRACE);
        break;

      case '}':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, RBRACE);
        break;

      case '[':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, RSQUARE);
        break;

      case ']':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, LSQUARE);
        break;

      case '(':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, LPAREN);
        break;

      case ')':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, RPAREN);
        break;

      case ';':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, TERMINATOR);
        break;

      case ':':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, COLON);
        break;

      case '?':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, QUESTION);
        break;

      case '~':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, TILDE);
        break;

      case '!':
        if (next_ch == '=')
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, NE_OP);
        else
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, EXCLAMATION);
        break;

      case ',':
        eatOneChar(&source, i, &tokenized_cur_ptr, ll, COMMA);
        break;

      case '.':
        if (isdigit(next_ch))
          i = eatNumber(&source, i, source_n, &tokenized_cur_ptr, ll);
        else if (next_ch == '.' && span_ptr[i + 2] == '.')
          i = eatSlice(&source, i, 3, &tokenized_cur_ptr, ll, ELLIPSIS);
        break;

      case '>':
        switch (next_ch) {
        case '>':
          if (span_ptr[i + 2] == '=')
            i = eatSlice(&source, i, 3, &tokenized_cur_ptr, ll, RIGHT_ASSIGN);
          else
            i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, RIGHT_SHIFT);
          break;
        case '=':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, GE_OP);
          break;
        default:
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, GREATER_THAN);
          break;
        }
        break;

      case '<':
        switch (next_ch) {
        case '<':
          if (span_ptr[i + 2] == '=')
            i = eatSlice(&source, i, 3, &tokenized_cur_ptr, ll, LEFT_ASSIGN);
          else
            i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, LEFT_SHIFT);
          break;
        case '=':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, LE_OP);
          break;
        default:
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, LESS_THAN);
          break;
        }
        break;

      case '+':
        switch (next_ch) {
        case '+':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, INC_OP);
          break;
        case '=':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, ADD_ASSIGN);
          break;
        default:
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, PLUS);
          break;
        }
        break;

      case '-':
        switch (next_ch) {
        case '-':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, DEC_OP);
          break;
        case '=':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, SUB_ASSIGN);
          break;
        case '>':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, PTR_OP);
          break;
        default:
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, SUB);
          break;
        }
        break;

      case '*':
        if (next_ch == '=')
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, MUL_ASSIGN);
        else
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, ASTERISK);
        break;

      case '/':
        switch (next_ch) {
        case '=':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, MUL_ASSIGN);
          break;
        case '/':
        case '*':
          break;
        default:
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, DIVIDE);
        }
        break;

      case '%':
        if (next_ch == '=')
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, MODULO);
        else
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, MOD_ASSIGN);
        break;

      case '&':
        switch (next_ch) {
        case '&':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, AND_OP);
          break;
        case '=':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, AND_ASSIGN);
          break;
        default:
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, AND);
          break;
        }
        break;

      case '^':
        if (next_ch == '=')
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, XOR_ASSIGN);
        else
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, CARET);
        break;

      case '|':
        switch (next_ch) {
        case '|':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, OR_OP);
          break;
        case '=':
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, OR_ASSIGN);
          break;
        default:
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, PIPE);
          break;
        }
        break;

      case '=':
        if (next_ch == '=')
          i = eatSlice(&source, i, 2, &tokenized_cur_ptr, ll, EQ_OP);
        else
          eatOneChar(&source, i, &tokenized_cur_ptr, ll, EQUAL);
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        /* Misses numbers with a sign [+-] */
        i = eatNumber(&source, i, source_n, &tokenized_cur_ptr, ll);
        break;

      case '_':
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
      case 'h':
      case 'i':
      case 'j':
      case 'k':
      case 'l':
      case 'm':
      case 'n':
      case 'o':
      case 'p':
      case 'q':
      case 'r':
      case 's':
      case 't':
      case 'u':
      case 'v':
      case 'w':
      case 'x':
      case 'y':
      case 'z':
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'G':
      case 'H':
      case 'I':
      case 'J':
      case 'K':
      case 'L':
      case 'M':
      case 'N':
      case 'O':
      case 'P':
      case 'Q':
      case 'R':
      case 'S':
      case 'T':
      case 'U':
      case 'V':
      case 'W':
      case 'X':
      case 'Y':
      case 'Z':
        i = eatWord(&source, i, source_n, &tokenized_cur_ptr, ll);
        break;

      default:
        break;
      }
  }

  /*az_span_list_push_valid(&source, ll, i, &sv, &tokenized_cur_ptr,
   * &start_index);*/

  /*printf("tokenized_ll: {kind=%s};\n",
  TokenizerKind_to_str(tokenized_ll->kind)); print_escaped_span("tokenized_ll",
  tokenized_ll->span);

  printf("tokenized_ll->next: {kind=%s};\n",
  TokenizerKind_to_str(tokenized_ll->next->kind));
  print_escaped_span("tokenized_ll->next", tokenized_ll->next->span);
  tokenized_ll->next = NULL;
  printf("tokenized_ll->next: %s;\n", tokenized_ll->next);*/

  ll->list = tokenized_ll;
  return ll;
}

void az_span_list_push_valid(const az_span *const source,
                             struct tokenizer_az_span_list *ll, size_t i,
                             struct TokenizerVars *sv,
                             struct tokenizer_az_span_elem ***tokenized_cur_ptr,
                             size_t *start_index) {
  const az_span expr = make_slice_clear_vars((*source), i, start_index, sv,
                                             /* always_make_expr */ true);
  if (az_span_ptr(expr) != NULL && az_span_size(expr) > 0)
    tokenizer_az_span_list_push(&ll->size, tokenized_cur_ptr, UNKNOWN_SCAN,
                                expr);
}

struct TokenizerVars *clear_sv(struct TokenizerVars *sv) {
  sv->c_comment_char_at = 0, sv->cpp_comment_char_at = 0,
  sv->line_continuation_at = 0;
  sv->spaces = 0;
  sv->lparen = 0, sv->rparen = 0, sv->lsquare = 0, sv->rsquare = 0,
  sv->lbrace = 0, sv->rbrace = 0, sv->lchev = 0, sv->rchev = 0;
  sv->in_c_comment = false, sv->in_cpp_comment = false, sv->in_single = false,
  sv->in_double = false, sv->in_macro = false, sv->in_init = false;
  return sv;
}

az_span make_slice_clear_vars(const az_span source, size_t i,
                              size_t *start_index, struct TokenizerVars *sv,
                              bool always_make_expr) {
  if (always_make_expr ||
      (!sv->in_single && !sv->in_double && !sv->in_c_comment &&
       !sv->in_cpp_comment && sv->line_continuation_at != i - 1 &&
       sv->lparen == sv->rparen && sv->lsquare == sv->rsquare &&
       sv->lchev == sv->rchev)) {
    const az_span slice = az_span_slice(source, *start_index, i);
    clear_sv(sv);
    *start_index = i;
    return slice;
  }
  return az_span_empty();
}

struct CstParseVars {
  bool is_union, is_struct, is_enum, is_function;
  size_t lparens, rparens, lbraces, rbraces, lsquare, rsquare;
};

void clear_CstParseVars(struct CstParseVars *pv) {
  pv->is_enum = false, pv->is_union = false, pv->is_struct = false;
  pv->lparens = 0, pv->rparens = 0, pv->lbraces = 0, pv->rbraces = 0,
  pv->lsquare = 0, pv->rsquare = 0;
}

struct parse_cst_list *
cst_parser(const struct tokenizer_az_span_list *const tokens_ll) {
  /* recognise start of function, struct, enum, union */

  struct tokenizer_az_span_elem *tokenized_ll = NULL;
  struct tokenizer_az_span_elem **tokenized_cur_ptr = &tokenized_ll;
  struct tokenizer_az_span_list *token_ll = calloc(1, sizeof *token_ll);

  struct parse_cst_elem *parse_ll = NULL;
  /*struct parse_cst_elem **tokenized_cur_ptr = &tokenized_ll;*/

  struct parse_cst_list *ll = calloc(1, sizeof *ll);
  struct tokenizer_az_span_elem *iter;

  size_t i, parse_start;

  struct CstParseVars vars = {false, false, false, false};

  /*enum TokenizerKind expression[] = {TERMINATOR};
  enum TokenizerKind function_start[] = {
      WORD / * | Keyword* /, WHITESPACE | C_COMMENT | CPP_COMMENT, LPAREN,
      WORD / * | Keyword* /, WHITESPACE | C_COMMENT | CPP_COMMENT, RPAREN,
      WORD / * | Keyword* /, WHITESPACE | C_COMMENT | CPP_COMMENT, LBRACE};*/

  /*enum TokenizerKind *tokenizer_kinds[] = {
      {LBRACE, MUL_ASSIGN},
      {LBRACE, MUL_ASSIGN},
  };*/

  struct tokenizer_az_span_element **tokens_arr = NULL;
  tokenizer_az_span_list_to_array(&tokens_arr, tokens_ll);
  assert(tokens_arr != NULL);

  {
    struct tokenizer_az_span_elem *token_el;
    for (token_el = (struct tokenizer_az_span_elem *)tokens_ll->list, i = 0;
         az_span_ptr(token_el->span) != NULL; token_el++, i++) {
      char *name;
      asprintf(&name, "list_::tokens_ll[%ld]:%s", i,
               TokenizerKind_to_str(token_el->kind));
      print_escaped_span(name, token_el->span);
      free(name);
    }
  }

  putchar('\n');

  {
    struct tokenizer_az_span_element **el;
    for (el = tokens_arr, i = 0; *el != NULL; el++, i++) {
      char *name;
      asprintf(&name, "array::tokens_arr[%ld]:%s", i,
               TokenizerKind_to_str((**el).kind));
      print_escaped_span(name, (**el).span);
      free(name);
    }
  }

  putchar('\n');

  for (i = 0; tokens_arr[i] != NULL; i++) {
  }
  printf("b4::tokens_arr::n     = %" NUM_LONG_FMT "u\n", i);
  {
    struct tokenizer_az_span_element *token_el;
    for (token_el = *tokens_arr, i = 0; az_span_ptr(token_el->span) != NULL;
         token_el++, i++) {
      char *name;
      asprintf(&name, "tokens_arr[%ld]:%s", i,
               TokenizerKind_to_str(token_el->kind));
      print_escaped_span(name, token_el->span);
      free(name);
    }
    printf("l8::tokens_arr::n     = %" NUM_LONG_FMT "u\n", i);
  }

  for (i = 0, parse_start = 0; tokens_arr[i] != NULL; i++) {
    switch (tokens_arr[i]->kind) {
    /*`
    for (iter = (struct tokenizer_az_span_elem *)tokens_ll->list, i = 0;
         iter != NULL; iter = iter->next, i++) {
      switch (iter->kind) */
    case C_COMMENT:
    case CPP_COMMENT:
    case WHITESPACE:
      break;

    case enumKeyword:
      vars.is_union = false;
      break;

    case unionKeyword:
      vars.is_enum = false;
      break;

    case WORD:
      /* can still be `enum` or `union` at this point */
      break;

    case ASTERISK:
    case autoKeyword:
    case charKeyword:
    case constKeyword:
    case doubleKeyword:
    case externKeyword:
    case floatKeyword:
    case intKeyword:
    case inlineKeyword:
    case longKeyword:
    case unsignedKeyword:
    case _NoreturnKeyword:
      vars.is_enum = false, vars.is_union = false;
      break;

    case structKeyword:
      vars.is_enum = false, vars.is_union = false, vars.is_struct = true;
      break;

      /* can still be struct, enum, union, function here */
    case TERMINATOR: {
      size_t j;
      clear_CstParseVars(&vars);

      puts("<EXPRESSION>");
      for (j = parse_start; j < i; j++) {
        print_escaped_span(TokenizerKind_to_str(tokens_arr[j]->kind),
                           tokens_arr[j]->span);
      }
      puts("</EXPRESSION>");
      parse_start = i;
      break;
    }

    case LBRACE:
      vars.lbraces++;

      if (vars.lparens == vars.rparens && vars.lsquare == vars.rsquare) {
        if (!vars.is_enum && !vars.is_union && !vars.is_struct &&
            vars.lparens > 0 && vars.lparens == vars.rparens)
          i = eatFunction(tokens_arr, parse_start, i, &tokenized_cur_ptr, ll) -
              1;
        else if (vars.is_enum && !vars.is_union && !vars.is_struct)
          /* could be an anonymous enum at the start of a function def */
          puts("WITHIN ENUM");
        else if (!vars.is_enum && vars.is_union && !vars.is_struct)
          puts("WITHIN UNION");
        else if (!vars.is_enum && !vars.is_union && vars.is_struct)
          puts("WITHIN STRUCT");

        clear_CstParseVars(&vars);
      }

      /*
      tokenizer_az_span_list_push(&ll->size, &tokenized_cur_ptr, iter->kind,
      iter->span); print_tokenizer_az_span_list(token_ll, i); token_ll->list =
      tokenized_ll; tokenizer_az_span_list_cleanup(token_ll);
      */
      break;

    case RBRACE:
      vars.rbraces++;
      /* TODO: Handle initializer and nested initializer lists */
      if (vars.lparens == vars.rparens && vars.lsquare == vars.rsquare &&
          vars.lbraces == vars.rbraces) {
        size_t j;

        char *parse_kind;
        if (vars.is_function)
          parse_kind = "FUNCTION";
        else if (vars.is_enum && !vars.is_union && !vars.is_struct)
          parse_kind = "ENUM";
        else if (!vars.is_enum && vars.is_union && !vars.is_struct)
          parse_kind = "UNION";
        else if (!vars.is_enum && !vars.is_union && vars.is_struct)
          parse_kind = "STRUCT";
        else
          parse_kind = "UNKNOWN";
        printf("<%s>\n", parse_kind);
        for (j = parse_start; j < i; j++) {
          print_escaped_span(TokenizerKind_to_str(tokens_arr[j]->kind),
                             tokens_arr[j]->span);
        }
        printf("</%s>\n", parse_kind);

        clear_CstParseVars(&vars);
        parse_start = i;
      }
      break;

    case LPAREN:
      vars.lparens++;
      break;

    case RPAREN:
      vars.rparens++;
      break;

    case LSQUARE:
      vars.lsquare++;
      break;

    case RSQUARE:
      vars.rsquare++;
      break;

    default:
      puts("<DEFAULT>");
      print_escaped_span(TokenizerKind_to_str(tokens_arr[i]->kind),
                         tokens_arr[i]->span);
      puts("</DEFAULT>");
      break;
    }
  }
  puts("*******************");
  token_ll->list = tokenized_ll;

  tokenizer_az_span_list_cleanup(token_ll);

  return ll;
}

void print_tokenizer_az_span_list(const struct tokenizer_az_span_list *token_ll,
                                  size_t i) {
  struct tokenizer_az_span_elem *iter1;
  for (iter1 = (struct tokenizer_az_span_elem *)token_ll->list, i = 0;
       iter1 != NULL; iter1 = iter1->next, i++) {
    print_escaped_span(TokenizerKind_to_str(iter1->kind), iter1->span);
  }
}
