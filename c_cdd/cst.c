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

void clear_sv(struct ScannerVars *);

const char *add_word_clear_vars(const char *source, size_t i, size_t *scan_from,
                                struct ScannerVars *sv, bool always_make_expr);

const struct str_elem *scanner(const char *source) {
  struct ScannerVars sv;

  struct str_elem *scanned_ll = NULL;
  struct str_elem **scanned_cur_ptr = &scanned_ll;

  size_t i, n = strlen(source), scan_from, scanned_n = 0;

  clear_sv(&sv);

  /* Scanner algorithm:
   *   0. Read one char at a time;
   *   1. Handle cases: space | '\' | ';' | '\n' | char | string | comment;
   *   2. Handle cases: conditioned by `in_{char || string || comment}`;
   *   3. Handle any remaining char | string;
   *   4. Finish with a linked-list.
   */
  for (i = 0, scan_from = 0; i < n; i++) {
    bool in_comment = sv.in_c_comment || sv.in_cpp_comment;
    if (isspace(source[i]))
      sv.spaces++;
    else
      switch (source[i]) {
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
          const char *expr = add_word_clear_vars(source, i, &scan_from, &sv,
                                                 /* always_make_expr */ false);
          if (expr != NULL)
            scanned_cur_ptr =
                str_push_to_ll(&scanned_n, &scanned_cur_ptr, expr);
          break;
        }
      default:
        if (!sv.in_single && !sv.in_double && !in_comment) {
          switch (source[i]) {
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

                clear_sv(&sv);

                const char *expr =
                    add_word_clear_vars(source, i - 1, &scan_from, &sv,
                                        /* always_make_expr */ true);
                scanned_cur_ptr =
                    str_push_to_ll(&scanned_n, &scanned_cur_ptr, expr);
              }
              { /* Push just the '{' | '}' to its own node */
                const char *expr =
                    add_word_clear_vars(source, i, &scan_from, &sv,
                                        /* always_make_expr */ true);
                scanned_cur_ptr =
                    str_push_to_ll(&scanned_n, &scanned_cur_ptr, expr);
              }
            }
            break;
          }
        }
      }
  }
  if (i < n) {
    const char *expr = add_word_clear_vars(source, i, &scan_from, &sv,
                                           /* always_make_expr */ false);
    if (expr != NULL)
      scanned_cur_ptr = str_push_to_ll(&scanned_n, &scanned_cur_ptr, expr);
  }
  return scanned_ll;
}

void clear_sv(struct ScannerVars *sv) {
  (*sv) = (struct ScannerVars){-1,    -1,    -1,    0,     0,     0,
                               0,     0,     0,     0,     0,     0,
                               false, false, false, false, false, false};
}

const char *add_word_clear_vars(const char *source, size_t i, size_t *scan_from,
                                struct ScannerVars *sv, bool always_make_expr) {
  if (always_make_expr ||
      !sv->in_single && !sv->in_double && !sv->in_c_comment &&
          !sv->in_cpp_comment && sv->line_continuation_at != i - 1 &&
          sv->lparen == sv->rparen && sv->lsquare == sv->rsquare &&
          sv->lchev == sv->rchev) {
    clear_sv(sv);
    return add_word(source, i, scan_from);
  }
  return NULL;
}

const struct CstNode **parser(struct str_elem *scanned) {
  struct str_elem *iter;
  putchar('\n');
  for (iter = scanned; iter != NULL; iter = iter->next) {
    char line[4095], *ch;
    size_t i = 0, j = 0, scan_from = 0, comment_started = 0, last_space = 0;
    struct ScannerVars sv;
    struct str_elem *words = NULL;
    struct str_elem **words_cur_ptr = &words;

    clear_sv(&sv);

    print_escaped("iter->s", (char *)iter->s);

    // printf("c: \'%c\'\n", c);
    /*
     "<words><name><lparen"
     sv.lparen could indicate:
       - function call
       - function prototype
       - function definition
       - declaration
     */
    for (ch = strdup(iter->s), i = 0; *ch; (ch)++, i++) {
      bool in_comment = sv.in_c_comment || sv.in_cpp_comment;
      if (isspace(*ch) && i > 0 && !in_comment && !isspace(ch[i - 1])) {
        words_cur_ptr = append(words_cur_ptr, add_word(iter->s, i, &scan_from));
      } else
        switch (*ch) {
        case '(':
          if (!in_comment) {
            if (i > 0)
              words_cur_ptr =
                  append(words_cur_ptr, add_word(iter->s, i - 1, &scan_from));
            words_cur_ptr =
                append(words_cur_ptr, add_word(iter->s, i, &scan_from));
          }
          break;
          /*case '\\':*/
        case ',':
          if (i > 0)
            words_cur_ptr =
                append(words_cur_ptr, add_word(iter->s, i - 1, &scan_from));
          words_cur_ptr =
              append(words_cur_ptr, add_word(iter->s, i, &scan_from));
          break;
        case '/':
          if (in_comment)
            in_comment = line[i - 1] == '*';
          else if (i > 0 && (line[i - 1] == '/' || line[i - 1] == '*'))
            in_comment = true, comment_started = i;
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
      putchar('\n');
    }
  }
  return NULL;
}
