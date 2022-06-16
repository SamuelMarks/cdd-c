#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include "c_cdd_utils.h"
#include "cst.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define C89STRINGUTILS_IMPLEMENTATION
#include <c89stringutils_string_extras.h>
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

void az_span_list_push_valid(const az_span *const source,
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
   *   0. Read one char at a time;
   *   1. Handle cases: space | '\' | ';' | '\n' | char | string | comment;
   *   2. Handle cases: conditioned by `in_{char || string || comment}`;
   *   3. Handle any remaining char | string;
   *   4. Finish with a linked-list.
   */

  /* Maybe restrict to the following types?
   * - macro{if,endif,ifndef,elif,include,define}
   * - comment{c,cpp}
   * - noncomment nonmacro nonliteral chars before {'{', '}', ';', '\n'}
   * */

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
          /* ^ handle multi nonseparated C-style comments `/\*bar*\//\*foo*\/`
           */
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
      case '\t':
      case '\n':
      case '\r':
        i = eatWhitespace(&source, i, source_n, &scanned_cur_ptr, ll);
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
              asprintf(&s, "[%lu-1] slice_cur_ptr", i);
              print_escaped_span(s,
                                 az_span_slice(iter->span, start_index, i - 1));
              free(s);
            }
            {
              char *s;
              az_span_list_push(&ll_n, &slice_cur_ptr,
                                az_span_slice(iter->span, start_index, i));
              asprintf(&s, "[%lu] slice_cur_ptr", i);
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
    asprintf(&s, "eatCComment[%02lu:%02lu]", start_index, end_index);
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
    asprintf(&s, "eatCppComment[%02lu:%02lu]", start_index, end_index);
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
    asprintf(&s, "eatMacro[%02lu:%02lu]", start_index, end_index);
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
    asprintf(&s, "eatStrLiteral[%02lu:%02lu]", start_index, end_index);
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
    asprintf(&s, "eatStrLiteral[%02lu:%02lu]", start_index, end_index);
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
  asprintf(&s, "eatWhitespace[%02lu:%02lu]", start_index, end_index);
  print_escaped_span(s, az_span_slice(*source, start_index, end_index));
  free(s);
#endif /* DEBUG_SCANNER */
  scan_az_span_list_push(&ll->size, scanned_cur_ptr, Whitespace,
                         az_span_slice(*source, start_index, end_index));
}
  return end_index - 1;
}
