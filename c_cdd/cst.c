#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include "c_cdd_utils.h"
#include "cst.h"

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

size_t eatCComment(const az_span *, size_t, size_t,
                   struct scan_az_span_elem ***, struct scan_az_span_list *);

size_t eatCppComment(const az_span *, size_t, size_t,
                     struct scan_az_span_elem ***, struct scan_az_span_list *);

size_t eatMacro(const az_span *, size_t, size_t, struct scan_az_span_elem ***,
                struct scan_az_span_list *);

size_t eatCharLiteral(const az_span *, size_t, size_t,
                      struct scan_az_span_elem ***, struct scan_az_span_list *);
size_t eatStrLiteral(const az_span *, size_t, size_t,
                     struct scan_az_span_elem ***, struct scan_az_span_list *);

size_t eatWhitespace(const az_span *, size_t, size_t,
                     struct scan_az_span_elem ***, struct scan_az_span_list *);

void eatOneChar(const az_span *, size_t, struct scan_az_span_elem ***,
                struct scan_az_span_list *, enum ScannerKind);

size_t eatTwoChars(const az_span *, size_t, struct scan_az_span_elem ***,
                   struct scan_az_span_list *, enum ScannerKind);

size_t eatEllipsis(const az_span *, size_t, struct scan_az_span_elem ***,
                   struct scan_az_span_list *);

size_t eatWord(const az_span *, size_t, size_t, struct scan_az_span_elem ***,
               struct scan_az_span_list *);

size_t eatNumber(const az_span *, size_t, size_t, struct scan_az_span_elem ***,
                 struct scan_az_span_list *);

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
   *   - macro        [if|endif|ifndef|elif|include|define]
   *   - terminator   ;
   *   - lbrace       {
   *   - rbrace       }
   *   - word         space or comment separated that doesn't match^
   */

  for (i = 0; i < source_n; i++) {
    const uint8_t ch = span_ptr[i],
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
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Lbrace);
        break;

      case '}':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Rbrace);
        break;

      case '[':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Rsquare);
        break;

      case ']':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Lsquare);
        break;

      case '(':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Lparen);
        break;

      case ')':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Rparen);
        break;

      case ';':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Terminator);
        break;

      case ':':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Colon);
        break;

      case '?':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Question);
        break;

      case '*':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Asterisk);
        break;

      case '/':
        if (span_ptr[i + 1] != '/' && span_ptr[i + 1] != '*')
          eatOneChar(&source, i, &scanned_cur_ptr, ll, Divide);
        break;

      case '^':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Caret);
        break;

      case '~':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Tilde);
        break;

      case '!':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Exclamation);
        break;

      case ',':
        eatOneChar(&source, i, &scanned_cur_ptr, ll, Comma);
        break;

      case '|':
        if (span_ptr[i + 1] == '|')
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, LogicalOr);
        else
          eatOneChar(&source, i, &scanned_cur_ptr, ll, Pipe);
        break;

      case '+':
        if (span_ptr[i + 1] == '+')
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, Increment);
        else
          eatOneChar(&source, i, &scanned_cur_ptr, ll, Plus);
        break;

      case '-':
        if (span_ptr[i + 1] == '-')
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, Decrement);
        else
          eatOneChar(&source, i, &scanned_cur_ptr, ll, Sub);
        break;

      case '.':
        if (isdigit(span_ptr[i + 1]))
          i = eatNumber(&source, i, source_n, &scanned_cur_ptr, ll);
        else if (span_ptr[i + 1] == '.' && span_ptr[i + 2] == '.')
          i = eatEllipsis(&source, i, &scanned_cur_ptr, ll);
        break;

      case '<':
        switch (span_ptr[i + 1]) {
        case '<':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, LeftShift);
          break;
        case '=':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, LessThanEqual);
          break;
        default:
          eatOneChar(&source, i, &scanned_cur_ptr, ll, LessThan);
          break;
        }
        break;

      case '>':
        switch (span_ptr[i + 1]) {
        case '>':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, RightShift);
          break;
        case '=':
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, GreaterThanEqual);
          break;
        default:
          eatOneChar(&source, i, &scanned_cur_ptr, ll, GreaterThan);
          break;
        }
        break;

      case '=':
        if (span_ptr[i + 1] == '=')
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, Equality);
        else
          eatOneChar(&source, i, &scanned_cur_ptr, ll, Equal);
        break;

      case '&':
        if (span_ptr[i + 1] == '&')
          i = eatTwoChars(&source, i, &scanned_cur_ptr, ll, LogicalAnd);
        else
          eatOneChar(&source, i, &scanned_cur_ptr, ll, And);
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
    scan_az_span_list_push(&ll->size, scanned_cur_ptr, UnknownScan, expr);
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
tokenizer(const struct scan_az_span_elem *const scanned) {
  /* tokenizes into slices, from "unsigned long/ *stuff*\f()";
   * to {"unsigned long", "/ *stuff*\", "f", "(", ")", ";"} */
  struct scan_az_span_elem *iter;
  struct az_span_elem *slices = NULL;
  struct az_span_elem **slice_cur_ptr = &slices;
  printf("const struct scan_az_span_elem *tokenizer(const struct "
         "scan_az_span_elem "
         "*const scanned)");
  putchar('\n');
  for (iter = (struct scan_az_span_elem *)scanned; iter != NULL;
       iter = iter->next) {
    char line[4095];
    size_t i = 0, start_index = 0;
    ssize_t comment_started = -1, comment_ended = -1 /*, last_space = -1*/;
    struct ScannerVars sv;
    const uint8_t *const span_ptr = az_span_ptr(iter->span);

    clear_sv(&sv);

    print_escaped_span("iter->s", iter->span);

    /* printf("c: \'%c\'\n", c); */
    /*
     "<slices><name><lparen"
     sv.lparen could indicate:
       - function call
       - function prototype
       - function definition
       - declaration
     */
    for (i = 0; i < az_span_size(iter->span); i++) {
      bool in_comment = sv.in_c_comment || sv.in_cpp_comment;
      uint32_t ll_n = 0;
      const uint8_t ch = span_ptr[i];
      if (i > 0 && !in_comment && !isspace(span_ptr[i - 1]) && !isspace(ch)) {
        az_span_list_push(&ll_n, &slice_cur_ptr,
                          az_span_slice(iter->span, start_index, i));
      } else
        switch (ch) {
          /*case '\\':*/
        case '(':
        case '[':
        case '{':
        case '}':
        case ']':
        case ')':
        case ',': /* TODO: Handle `int a, *b` */
          if (!in_comment) {
            if (i > 0) {
              char *s;
              az_span_list_push(&ll_n, &slice_cur_ptr,
                                az_span_slice(iter->span, start_index, i - 1));
              asprintf(&s, "[%" NUM_LONG_FMT "u-1] slice_cur_ptr", i);
              print_escaped_span(s,
                                 az_span_slice(iter->span, start_index, i - 1));
              free(s);
            }
            {
              char *s;
              az_span_list_push(&ll_n, &slice_cur_ptr,
                                az_span_slice(iter->span, start_index, i));
              asprintf(&s, "[%" NUM_LONG_FMT "u] slice_cur_ptr", i);
              print_escaped_span(s, az_span_slice(iter->span, start_index, i));
              free(s);
            }
            az_span_list_push(&ll_n, &slice_cur_ptr,
                              az_span_slice(iter->span, start_index, i));
          }
          break;
        case '/':
          if (in_comment) {
            in_comment = line[i - 1] == '*';
            if (!in_comment)
              comment_ended = (ssize_t)i - 1;
          } else if (i > 0 && (line[i - 1] == '/' || line[i - 1] == '*'))
            in_comment = true, comment_started = (ssize_t)i - 1;
          break;
        case '\n':
          if (sv.in_cpp_comment) {
            sv.in_cpp_comment = false;
            comment_ended = (ssize_t)i - 1;
          }
          break;
        default:
          break;
        }
      /*escape:
        line[i + 1] = '\0';
        print_escaped("line", line);*/

      /*{
        char *rest = NULL;
        char *token;
        for (token = strtok_r(strdup(iter->s), " ", &rest);
             token != NULL;
             token = strtok_r(NULL, " ", &rest)) {
          print_escaped("token", token);
        }
      }*/
      /* putchar('\n'); */
    }
  }
  return slices;
}

const struct CstNode **parser(struct az_span_elem *scanned) { return NULL; }

size_t eatCComment(const az_span *const source, const size_t start_index,
                   const size_t n, struct scan_az_span_elem ***scanned_cur_ptr,
                   struct scan_az_span_list *ll) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index; end_index < n; end_index++) {
    if (span_ptr[end_index] == '/' && span_ptr[end_index - 1] == '*' &&
        span_ptr[end_index - 2] != '\\')
      break;
  }

  if (end_index > start_index) {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eatCComment[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index + 1));
    free(s);
#endif /* DEBUG_SCANNER */
    scan_az_span_list_push(&ll->size, scanned_cur_ptr, CComment,
                           az_span_slice(*source, start_index, ++end_index));
  }
  return end_index - 1;
}

size_t eatCppComment(const az_span *const source, const size_t start_index,
                     const size_t n,
                     struct scan_az_span_elem ***scanned_cur_ptr,
                     struct scan_az_span_list *ll) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index], last_ch = span_ptr[end_index - 1];
    if (ch == '\n' && last_ch != '\\')
      break;
  }

  if (end_index > start_index) {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eatCppComment[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index + 1));
    free(s);
#endif /* DEBUG_SCANNER */
    scan_az_span_list_push(&ll->size, scanned_cur_ptr, CppComment,
                           az_span_slice(*source, start_index, ++end_index));
  }
  return end_index;
}

size_t eatMacro(const az_span *const source, const size_t start_index,
                const size_t n, struct scan_az_span_elem ***scanned_cur_ptr,
                struct scan_az_span_list *ll) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index; end_index < n; end_index++) {
    if (span_ptr[end_index] == '\n' && span_ptr[end_index - 1] != '\\')
      break;
  }

  if (end_index > start_index) {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eatMacro[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index + 1));
    free(s);
#endif /* DEBUG_SCANNER */
    scan_az_span_list_push(&ll->size, scanned_cur_ptr, Macro,
                           az_span_slice(*source, start_index, ++end_index));
  }
  return end_index - 1;
}

size_t eatCharLiteral(const az_span *const source, const size_t start_index,
                      const size_t n,
                      struct scan_az_span_elem ***scanned_cur_ptr,
                      struct scan_az_span_list *ll) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index + 1; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index], last_ch = span_ptr[end_index - 1],
                  second_last_ch = span_ptr[end_index - 2],
                  third_last_ch = span_ptr[end_index - 3];
    if (ch == '\'' &&
        (last_ch != '\\' || (second_last_ch == '\'' && third_last_ch != '\\')))
      break;
  }

  {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eatStrLiteral[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index + 1));
    free(s);
#endif /* DEBUG_SCANNER */
    scan_az_span_list_push(&ll->size, scanned_cur_ptr, SingleQuoted,
                           az_span_slice(*source, start_index, ++end_index));
  }
  return end_index - 1;
}

size_t eatStrLiteral(const az_span *const source, const size_t start_index,
                     const size_t n,
                     struct scan_az_span_elem ***scanned_cur_ptr,
                     struct scan_az_span_list *ll) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index + 1; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index], last_ch = span_ptr[end_index - 1];
    if (ch == '"' && (last_ch != '\\' || (span_ptr[end_index - 3] == '"' &&
                                          span_ptr[end_index - 4] != '\\')))
      break;
  }

  {
#ifdef DEBUG_SCANNER
    char *s;
    asprintf(&s, "eatStrLiteral[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
             start_index, end_index);
    print_escaped_span(s, az_span_slice(*source, start_index, end_index + 1));
    free(s);
#endif /* DEBUG_SCANNER */
    scan_az_span_list_push(&ll->size, scanned_cur_ptr, DoubleQuoted,
                           az_span_slice(*source, start_index, ++end_index));
  }
  return end_index - 1;
}

size_t eatWhitespace(const az_span *const source, const size_t start_index,
                     const size_t n,
                     struct scan_az_span_elem ***scanned_cur_ptr,
                     struct scan_az_span_list *ll) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  /*uint8_t ch;
  for (end_index = start_index + 1, ch = az_span_ptr(*source)[end_index];
       end_index < n &&
       (ch == '\n' || ch == '\r' || ch == '\t' ||
        ch == ' ') //isspace(az_span_ptr(*source)[end_index]);
       end_index++, ch = az_span_ptr(*source)[end_index]) {
  }*/
  for (end_index = start_index + 1; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index];
    switch (ch) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case '\v':
      break;
    default:
      goto end;
    }
  }

end : {
#ifdef DEBUG_SCANNER
  char *s;
  asprintf(&s, "eatWhitespace[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
           start_index, end_index);
  print_escaped_span(s, az_span_slice(*source, start_index, end_index));
  free(s);
#endif /* DEBUG_SCANNER */
  scan_az_span_list_push(&ll->size, scanned_cur_ptr, Whitespace,
                         az_span_slice(*source, start_index, end_index));
}
  return end_index - 1;
}

size_t eatEllipsis(const az_span *const source, const size_t start_index,
                   struct scan_az_span_elem ***scanned_cur_ptr,
                   struct scan_az_span_list *ll) {
  const size_t end_index = start_index + 3;
#ifdef DEBUG_SCANNER
  char *s;
  asprintf(&s, "eatEllipsis[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
           start_index, end_index);
  print_escaped_span(s, az_span_slice(*source, start_index, end_index));
  free(s);
#endif /* DEBUG_SCANNER */
  scan_az_span_list_push(&ll->size, scanned_cur_ptr, Ellipsis,
                         az_span_slice(*source, start_index, end_index));
  return end_index - 1;
}

void eatOneChar(const az_span *const source, const size_t start_index,
                struct scan_az_span_elem ***scanned_cur_ptr,
                struct scan_az_span_list *ll, enum ScannerKind kind) {
#ifdef DEBUG_SCANNER
  char *s;
  asprintf(&s, "eat%s[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
           ScannerKind_to_str(kind), start_index, start_index + 1);
  print_escaped_span(s, az_span_slice(*source, start_index, start_index + 1));
  free(s);
#endif /* DEBUG_SCANNER */
  scan_az_span_list_push(&ll->size, scanned_cur_ptr, kind,
                         az_span_slice(*source, start_index, start_index + 1));
}

size_t eatTwoChars(const az_span *const source, const size_t start_index,
                   struct scan_az_span_elem ***scanned_cur_ptr,
                   struct scan_az_span_list *ll, enum ScannerKind kind) {
  const size_t end_index = start_index + 2;
#ifdef DEBUG_SCANNER
  char *s;
  asprintf(&s, "eat%s[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
           ScannerKind_to_str(kind), start_index, end_index);
  print_escaped_span(s, az_span_slice(*source, start_index, end_index));
  free(s);
#endif /* DEBUG_SCANNER */
  scan_az_span_list_push(&ll->size, scanned_cur_ptr, kind,
                         az_span_slice(*source, start_index, end_index));
  return end_index;
}

size_t eatWord(const az_span *const source, const size_t start_index,
               const size_t n, struct scan_az_span_elem ***scanned_cur_ptr,
               struct scan_az_span_list *ll) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index + 1; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index];
    switch (ch) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case '\v':
      goto end;
    default:
      break;
    }
  }

end : {
#ifdef DEBUG_SCANNER
  char *s;
  asprintf(&s, "eatWord[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
           start_index, end_index);
  print_escaped_span(s, az_span_slice(*source, start_index, end_index));
  free(s);
#endif /* DEBUG_SCANNER */
  scan_az_span_list_push(&ll->size, scanned_cur_ptr, Word,
                         az_span_slice(*source, start_index, end_index));
}
  return end_index - 1;
}

size_t eatNumber(const az_span *const source, const size_t start_index,
                 const size_t n, struct scan_az_span_elem ***scanned_cur_ptr,
                 struct scan_az_span_list *ll) {
  size_t end_index;
  const uint8_t *const span_ptr = az_span_ptr(*source);
  for (end_index = start_index + 1; end_index < n; end_index++) {
    const uint8_t ch = span_ptr[end_index], last_ch = span_ptr[end_index - 1];
    switch (ch) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case '\v':
      goto end;
    /* Handle comments, as there is only one digit in: "5//5" and one in "6/\*5"
     */
    case '/':
    case '*':
      if (last_ch == '/') {
        --end_index;
        goto end;
      }
    /* TODO: Scientific notation, hexadecimal, octal */
    default:
      break;
    }
  }

end : {
#ifdef DEBUG_SCANNER
  char *s;
  asprintf(&s, "eatWord[%02" NUM_LONG_FMT "u:%02" NUM_LONG_FMT "u]",
           start_index, end_index);
  print_escaped_span(s, az_span_slice(*source, start_index, end_index));
  free(s);
#endif /* DEBUG_SCANNER */
  scan_az_span_list_push(&ll->size, scanned_cur_ptr, Word,
                         az_span_slice(*source, start_index, end_index));
}
  return end_index - 1;
}
