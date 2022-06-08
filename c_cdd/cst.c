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

az_span make_slice_clear_vars(az_span source, int32_t i, int32_t *start_index,
                              struct ScannerVars *sv, bool always_make_expr);

void az_span_list_push_valid(const az_span *source,
                             const struct az_span_list *ll, int32_t i,
                             struct ScannerVars *sv,
                             struct az_span_elem ***scanned_cur_ptr,
                             int32_t *start_index);

int eatCComment(const az_span *, int32_t, int32_t, struct az_span_elem ***,
                struct az_span_list *);

int eatCppComment(const az_span *, int32_t, int32_t, struct az_span_elem ***,
                  struct az_span_list *);

struct az_span_list *scanner(const az_span source) {
  struct ScannerVars sv;

  struct az_span_elem *scanned_ll = NULL;
  struct az_span_elem **scanned_cur_ptr = &scanned_ll;

  struct az_span_list *ll = calloc(1, sizeof *ll);

  int32_t i, start_index;
  const int32_t source_n = az_span_size(source);
  ll->size = 0;

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

  for (i = 0, start_index = 0; i < source_n; i++) {
    /* Handle comments */
    if (i > 0) {
      const uint8_t ch = az_span_ptr(source)[i];
      const uint8_t last_ch = az_span_ptr(source)[i - 1];
      if (last_ch == '/' && (i < 2 || az_span_ptr(source)[i - 2] != '\\')) {
        /*const az_span *source, const int32_t start_index, const int32_t n,
        struct az_span_elem ***scanned_cur_ptr, struct az_span_list *ll*/

        if (ch == '*')
          i = start_index =
              eatCComment(&source, start_index, source_n, &scanned_cur_ptr, ll);
        else if (ch == '/')
          i = start_index = eatCppComment(&source, start_index, source_n,
                                          &scanned_cur_ptr, ll);
      }
    }
  }

  /*az_span_list_push_valid(&source, ll, i, &sv, &scanned_cur_ptr,
   * &start_index);*/

  ll->list = scanned_ll;
  return ll;
}

void az_span_list_push_valid(const az_span *source,
                             const struct az_span_list *ll, int32_t i,
                             struct ScannerVars *sv,
                             struct az_span_elem ***scanned_cur_ptr,
                             int32_t *start_index) {
  const az_span expr = make_slice_clear_vars((*source), i, start_index, sv,
                                             /* always_make_expr */ true);
  if (az_span_ptr(expr) != NULL && az_span_size(expr) > 0)
    az_span_list_push(&ll->size, scanned_cur_ptr, expr);
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

az_span make_slice_clear_vars(const az_span source, int32_t i,
                              int32_t *start_index, struct ScannerVars *sv,
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

const struct az_span_elem *tokenizer(const struct az_span_elem *const scanned) {
  /* tokenizes into slices, from "unsigned long/ *stuff*\f()";
   * to {"unsigned long", "/ *stuff*\", "f", "(", ")", ";"} */
  struct az_span_elem *iter;
  struct az_span_elem *slices = NULL;
  struct az_span_elem **slice_cur_ptr = &slices;
  printf("const struct az_span_elem *tokenizer(const struct az_span_elem "
         "*const scanned)");
  putchar('\n');
  for (iter = (struct az_span_elem *)scanned; iter != NULL; iter = iter->next) {
    char line[4095];
    int32_t i = 0, start_index = 0;
    ssize_t comment_started = -1, comment_ended = -1 /*, last_space = -1*/;
    struct ScannerVars sv;

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
      uint8_t ch = az_span_ptr(iter->span)[i];
      if (i > 0 && !in_comment && !isspace(az_span_ptr(iter->span)[i - 1]) &&
          !isspace(ch)) {
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
              asprintf(&s, "[%d-1] slice_cur_ptr", i);
              print_escaped_span(s,
                                 az_span_slice(iter->span, start_index, i - 1));
              free(s);
            }
            {
              char *s;
              az_span_list_push(&ll_n, &slice_cur_ptr,
                                az_span_slice(iter->span, start_index, i));
              asprintf(&s, "[%d] slice_cur_ptr", i);
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

int eatCComment(const az_span *source, const int32_t start_index,
                const int32_t n, struct az_span_elem ***scanned_cur_ptr,
                struct az_span_list *ll) {
  int32_t end_index = start_index;
  do {
    end_index++;
    if (az_span_ptr(*source)[end_index] == '/' &&
        az_span_ptr(*source)[end_index - 1] == '*' &&
        az_span_ptr(*source)[end_index - 2] != '\\')
      break;
  } while (end_index != n);

  if (end_index > start_index)
    az_span_list_push(&ll->size, scanned_cur_ptr,
                      az_span_slice(*source, start_index, ++end_index));
  return end_index;
}

int eatCppComment(const az_span *source, const int32_t start_index,
                  const int32_t n, struct az_span_elem ***scanned_cur_ptr,
                  struct az_span_list *ll) {
  int32_t end_index = start_index;
  do {
    end_index++;
    if (az_span_ptr(*source)[end_index] == '\n' &&
        az_span_ptr(*source)[end_index - 1] != '\\')
      break;
  } while (end_index != n);

  if (end_index > start_index)
    az_span_list_push(&ll->size, scanned_cur_ptr,
                      az_span_slice(*source, start_index, ++end_index));
  return end_index;
}
