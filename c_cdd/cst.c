#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include "c_cdd_utils.h"
#include "cst.h"
#include "tokenizer_helpers.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define C89STRINGUTILS_IMPLEMENTATION
#include <c89stringutils_string_extras.h>
#define NUM_LONG_FMT "z"
#else
#define NUM_LONG_FMT "l"
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

struct ScannerVars {
  ssize_t c_comment_char_at, cpp_comment_char_at, line_continuation_at;
  uint32_t spaces;
  uint32_t lparen, rparen, lsquare, rsquare, lbrace, rbrace, lchev, rchev;
  bool in_c_comment, in_cpp_comment, in_single, in_double, in_macro, in_init,
      is_digit;
};

struct ScannerVars *clear_sv(struct ScannerVars *);

az_span make_slice_clear_vars(az_span source, size_t i, size_t *start_index,
                              struct ScannerVars *sv, bool always_make_expr);

void az_span_list_push_valid(const az_span *source,
                             struct scan_az_span_list *ll, size_t i,
                             struct ScannerVars *sv,
                             struct scan_az_span_elem ***scanned_cur_ptr,
                             size_t *start_index);

struct scan_az_span_list *scanner(const az_span source) {
  struct ScannerVars sv;

  struct scan_az_span_elem *scanned_ll = NULL;
  struct scan_az_span_elem **scanned_cur_ptr = &scanned_ll;

  struct scan_az_span_list *ll = calloc(1, sizeof *ll);

  size_t i;
  const size_t source_n = az_span_size(source);
  const uint8_t *const span_ptr = az_span_ptr(source);

  clear_sv(&sv);

  /* Scanner algorithm:
   *   0. Use last 2 chars—3 on comments—to determine type;
   *   1. Pass to type-eating function, returning whence finished reading;
   *   2. Repeat from 0. until end of `source`.
   *
   * Scanner types (line-continuation aware):
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
        i = eatCComment(&source, i - 1, source_n, &scanned_cur_ptr, ll);
        handled = true;
        break;
      case '/':
        if (span_ptr[i - 2] != '*') {
          /* ^ handle consecutive C-style comments `/\*bar*\/\*foo*\/` */
          i = eatCppComment(&source, i - 1, source_n, &scanned_cur_ptr, ll);
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
        i = eatMacro(&source, i, source_n, &scanned_cur_ptr, ll);
        break;

      /* Handle literals (single and double-quoted) */
      case '\'':
        i = eatCharLiteral(&source, i, source_n, &scanned_cur_ptr, ll);
        break;

      case '"':
        i = eatStrLiteral(&source, i, source_n, &scanned_cur_ptr, ll);
        break;

      case ' ':
      case '\n':
      case '\r':
      case '\t':
      case '\v':
        i = eatWhitespace(&source, i, source_n, &scanned_cur_ptr, ll);
        break;

      case '{':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, LBRACE);
        break;

      case '}':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, RBRACE);
        break;

      case '[':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, RSQUARE);
        break;

      case ']':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, LSQUARE);
        break;

      case '(':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, LPAREN);
        break;

      case ')':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, RPAREN);
        break;

      case ';':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, TERMINATOR);
        break;

      case ':':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, COLON);
        break;

      case '?':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, QUESTION);
        break;

      case '~':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, TILDE);
        break;

      case '!':
        if (next_ch == '=')
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, NE_OP);
        else
          eatOneChar(&source, i, &scanned_cur_ptr, ll, EXCLAMATION);
        break;

      case ',':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, COMMA);
        break;

      case '.':
        if (isdigit(next_ch))
          i = eatNumber(&source, i, source_n, &scanned_cur_ptr, ll);
        else if (next_ch == '.' && span_ptr[i + 2] == '.')
          i = eatThreeChars(&source, i, &scanned_cur_ptr, ll, ELLIPSIS);
        break;

      case '>':
        switch (next_ch) {
        case '>':
          if (span_ptr[i + 2] == '=')
            i = eatThreeChars(&source, i, &scanned_cur_ptr, ll, RIGHT_ASSIGN);
          else
            i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, RIGHT_SHIFT);
          break;
        case '=':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, GE_OP);
          break;
        default:
          eatOneChar(&source, i, &scanned_cur_ptr, ll, GREATER_THAN);
          break;
        }
        break;

      case '<':
        switch (next_ch) {
        case '<':
          if (span_ptr[i + 2] == '=')
            i = eatThreeChars(&source, i, &scanned_cur_ptr, ll, LEFT_ASSIGN);
          else
            i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, LEFT_SHIFT);
          break;
        case '=':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, LE_OP);
          break;
        default:
          eatOneChar(&source, i, &scanned_cur_ptr, ll, LESS_THAN);
          break;
        }
        break;

      case '+':
        switch (next_ch) {
        case '+':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, INC_OP);
          break;
        case '=':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, ADD_ASSIGN);
          break;
        default:
          eatOneChar(&source, i, &scanned_cur_ptr, ll, PLUS);
          break;
        }
        break;

      case '-':
        switch (next_ch) {
        case '-':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, DEC_OP);
          break;
        case '=':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, SUB_ASSIGN);
          break;
        case '>':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, PTR_OP);
          break;
        default:
          eatOneChar(&source, i, &scanned_cur_ptr, ll, SUB);
          break;
        }
        break;

      case '*':
        if (next_ch == '=')
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, MUL_ASSIGN);
        else
          eatOneChar(&source, i, &scanned_cur_ptr, ll, ASTERISK);
        break;

      case '/':
        switch (next_ch) {
        case '=':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, MUL_ASSIGN);
          break;
        case '/':
        case '*':
          break;
        default:
          eatOneChar(&source, i, &scanned_cur_ptr, ll, DIVIDE);
        }
        break;

      case '%':
        if (next_ch == '=')
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, MODULO);
        else
          eatOneChar(&source, i, &scanned_cur_ptr, ll, MOD_ASSIGN);
        break;

      case '&':
        switch (next_ch) {
        case '&':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, AND_OP);
          break;
        case '=':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, AND_ASSIGN);
          break;
        default:
          eatOneChar(&source, i, &scanned_cur_ptr, ll, AND);
          break;
        }
        break;

      case '^':
        if (next_ch == '=')
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, OR_ASSIGN);
        else
          eatOneChar(&source, i, &scanned_cur_ptr, ll, CARET);
        break;

      case '|':
        switch (next_ch) {
        case '|':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, OR_OP);
          break;
        case '=':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, OR_ASSIGN);
          break;
        default:
          eatOneChar(&source, i, &scanned_cur_ptr, ll, PIPE);
          break;
        }
        break;

      case '=':
        if (next_ch == '=')
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, EQ_OP);
        else
          eatOneChar(&source, i, &scanned_cur_ptr, ll, EQUAL);
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
        i = eatNumber(&source, i, source_n, &scanned_cur_ptr, ll);
        break;

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
        i = eatWord(&source, i, source_n, &scanned_cur_ptr, ll);
        break;

      default:
        break;
      }
  }

  /*az_span_list_push_valid(&source, ll, i, &sv, &scanned_cur_ptr,
   * &start_index);*/

  ll->list = scanned_ll;
  return ll;
}

void az_span_list_push_valid(const az_span *const source,
                             struct scan_az_span_list *ll, size_t i,
                             struct ScannerVars *sv,
                             struct scan_az_span_elem ***scanned_cur_ptr,
                             size_t *start_index) {
  const az_span expr = make_slice_clear_vars((*source), i, start_index, sv,
                                             /* always_make_expr */ true);
  if (az_span_ptr(expr) != NULL && az_span_size(expr) > 0)
    scan_az_span_list_push(&ll->size, scanned_cur_ptr, UNKNOWN_SCAN, expr);
}

struct ScannerVars *clear_sv(struct ScannerVars *sv) {
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
                              size_t *start_index, struct ScannerVars *sv,
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

const struct az_span_elem *
tokenizer(const struct scan_az_span_list *const scanned) {
  /* tokenizes into slices, from "unsigned long/ *stuff*\f()";
   * to {"unsigned long", "/ *stuff*\", "f", "(", ")", ";"} */
  struct scan_az_span_elem *iter;
  size_t i;

  /*for (iter = (struct scan_az_span_elem *)scanned->list, i = 0; iter != NULL;
       iter = iter->next, i++) {
    print_escaped_span(ScannerKind_to_str(iter->kind), iter->span);
  }*/
  return NULL;
}

const struct CstNode **parser(struct az_span_elem *scanned) { return NULL; }
