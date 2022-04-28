#include <ctype.h>
#include <stdio.h>

#include "c_cdd_utils.h"
#include "cst.h"

struct ScannerVars {
  ssize_t c_comment_char_at, cpp_comment_char_at, line_continuation_at;
  unsigned spaces;
  unsigned lparen, rparen, lsquare, rsquare, lbrace, rbrace, lchev, rchev;
  bool in_c_comment, in_cpp_comment, in_single, in_double, in_macro, in_init;
};

struct ScannerVars *clear_sv(struct ScannerVars *);

az_span add_word_clear_vars(az_span source, size_t i, const size_t *scan_from,
                            struct ScannerVars *sv, bool always_make_expr);

const struct az_span_elem *scanner(const az_span source) {
  struct ScannerVars sv;

  struct az_span_elem *scanned_ll = NULL;
  struct az_span_elem **scanned_cur_ptr = &scanned_ll;

  size_t i, scan_from, scanned_n = 0;

  clear_sv(&sv);

  /* Scanner algorithm:
   *   0. Read one char at a time;
   *   1. Handle cases: space | '\' | ';' | '\n' | char | string | comment;
   *   2. Handle cases: conditioned by `in_{char || string || comment}`;
   *   3. Handle any remaining char | string;
   *   4. Finish with a linked-list.
   */
  for (i = 0, scan_from = 0; i < az_span_size(source); i++) {
    bool in_comment = sv.in_c_comment || sv.in_cpp_comment;
    if (isspace(az_span_ptr(source)[i]))
      sv.spaces++;
    else
      switch (az_span_ptr(source)[i]) {
      case '/':
        if (!sv.in_single && !sv.in_double) {
          if (sv.cpp_comment_char_at == i - 1)
            sv.in_cpp_comment = true;
          else if (sv.c_comment_char_at == i - 1)
            sv.in_c_comment = false;
          else
            sv.cpp_comment_char_at = (ssize_t)i,
            sv.c_comment_char_at = (ssize_t)i;
        }
        break;
      case '*':
        if (!sv.in_single && !sv.in_double) {
          if (in_comment)
            sv.c_comment_char_at = (ssize_t)i;
          else if (sv.c_comment_char_at == i - 1)
            sv.in_c_comment = true;
        }
        break;
      case '"':
        if (!sv.in_single)
          sv.in_double = !sv.in_double;
        break;
      case '\'':
        if (!sv.in_double)
          sv.in_single = !sv.in_single;
        break;
      case ';':
      case '\n':
        if (sv.lbrace == sv.rbrace || in_comment) {
          const az_span expr =
              add_word_clear_vars(source, i, &scan_from, &sv,
                                  /* always_make_expr */ false);
          if (az_span_ptr(expr) != NULL && az_span_size(expr) > 0)
            scanned_cur_ptr = ll_push_span(&scanned_n, &scanned_cur_ptr, expr);
          break;
        }
      default:
        if (!sv.in_single && !sv.in_double && !in_comment) {
          switch (az_span_ptr(source)[i]) {
          case '=':
            sv.in_init = true;
            break;
          case '#':
            sv.in_macro = true;
            break;
          case '(':
            sv.lparen++;
            break;
          case ')':
            sv.rparen++;
            break;
          case '[':
            sv.lsquare++;
            break;
          case ']':
            sv.rsquare++;
            break;
          case '<':
            sv.lchev++;
            break;
          case '>':
            sv.rchev++;
            break;
          case '\\':
            sv.line_continuation_at = (ssize_t)i;
            break;
          case '{':
          case '}':
            /* TODO: Handle `f({{5,6}, {7,8}});` */
            if (!sv.in_init) {
              if (i != 0 && sv.spaces != i - scan_from) {
                const az_span expr = add_word_clear_vars(
                    source, i - 1, &scan_from, clear_sv(&sv),
                    /* always_make_expr */ true);
                scanned_cur_ptr =
                    ll_push_span(&scanned_n, &scanned_cur_ptr, expr);
              }
              { /* Push just the '{' | '}' to its own node */
                const az_span expr =
                    add_word_clear_vars(source, i, &scan_from, &sv,
                                        /* always_make_expr */ true);
                scanned_cur_ptr =
                    ll_push_span(&scanned_n, &scanned_cur_ptr, expr);
              }
            }
            break;
          }
        }
      }
  }
  if (i < az_span_size(source)) {
    const az_span expr = add_word_clear_vars(source, i, &scan_from, &sv,
                                             /* always_make_expr */ false);
    if (az_span_ptr(expr) != NULL && az_span_size(expr) > 0)
      scanned_cur_ptr = ll_push_span(&scanned_n, &scanned_cur_ptr, expr);
  }
  return scanned_ll;
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

az_span add_word_clear_vars(const az_span source, size_t i,
                            const size_t *scan_from, struct ScannerVars *sv,
                            bool always_make_expr) {
  if (always_make_expr ||
      (!sv->in_single && !sv->in_double && !sv->in_c_comment &&
       !sv->in_cpp_comment && sv->line_continuation_at != i - 1 &&
       sv->lparen == sv->rparen && sv->lsquare == sv->rsquare &&
       sv->lchev == sv->rchev)) {
    clear_sv(sv);
    return az_span_slice(source, (int32_t)*scan_from, (int32_t)i);
  }
  return az_span_empty();
}

const struct az_span_elem *tokenizer(struct az_span_elem *scanned) {
  /* tokenizes into words, from "unsigned long/ *stuff*\f()";
   * to {"unsigned long", "/ *stuff*\", "f", "(", ")", ";"} */
  struct az_span_elem *iter;
  struct az_span_elem *words = NULL;
  struct az_span_elem **words_cur_ptr = &words;
  printf("const struct str_elem *tokenizer(struct str_elem *scanned)");
  putchar('\n');
#define SUBSTR_PRINT 1
#include "c_cdd_utils.h"
  for (iter = scanned; iter != NULL; iter = iter->next) {
    char line[4095];
    size_t i = 0, scan_from = 0;
    ssize_t comment_started = -1, comment_ended = -1 /*, last_space = -1*/;
    struct ScannerVars sv;

    clear_sv(&sv);

    print_escaped_span("iter->s", iter->span);

    // printf("c: \'%c\'\n", c);
    /*
     "<words><name><lparen"
     sv.lparen could indicate:
       - function call
       - function prototype
       - function definition
       - declaration
     */
    for (i = 0; i < az_span_size(iter->span); i++) {
      bool in_comment = sv.in_c_comment || sv.in_cpp_comment;
      size_t words_n = 0;
      uint8_t ch = az_span_ptr(iter->span)[i];
      if (i > 0 && !in_comment && !isspace(az_span_ptr(iter->span)[i - 1]) &&
          !isspace(ch)) {
        words_cur_ptr = ll_push_span(
            &words_n, &words_cur_ptr,
            az_span_slice(iter->span, (int32_t)scan_from, (int32_t)i));
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
            if (i > 0)
              words_cur_ptr =
                  ll_push_span(&words_n, &words_cur_ptr,
                               az_span_slice(iter->span, (int32_t)scan_from,
                                             (int32_t)i - 1));
            words_cur_ptr = ll_push_span(
                &words_n, &words_cur_ptr,
                az_span_slice(iter->span, (int32_t)scan_from, (int32_t)i));
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
      // putchar('\n');
    }
  }
  return words;
}

const struct CstNode **parser(struct az_span_elem *scanned) { return NULL; }
