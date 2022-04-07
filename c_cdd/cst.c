#include "cst.h"
#include <printf.h>
#include <sys/errno.h>

struct str_elem **append(struct str_elem **root, const char *s) {
  struct str_elem **insert = &*root;
  while (*insert)
    insert = &insert[0]->next;
  *insert = malloc(sizeof **insert);
  if (!*insert)
    exit(ENOMEM);
  insert[0]->s = s;
  insert[0]->n = strlen(s);
  insert[0]->next = NULL;
  return insert;
  /* return root; */
}

const char *process_as_expression(
    const char *source, size_t i, size_t *scan_from,
    ssize_t line_continuation_at, bool in_comment, bool in_single,
    bool in_double, ssize_t *c_comment_char_at, ssize_t *cpp_comment_char_at,
    unsigned int *spaces, unsigned int *lparen, unsigned int *rparen,
    unsigned int *lsquare, unsigned int *rsquare, unsigned int *lbrace,
    unsigned int *rbrace, unsigned int *rchev, unsigned int *lchev);

struct str_elem **push_expr_to_ll(size_t *scanned_n,
                                  struct str_elem ***scanned_ll,
                                  const char *expr);

const struct str_elem *scanner(const char *source) {
  size_t i, n = strlen(source), scan_from, scanned_n = 0;
  ssize_t c_comment_char_at, cpp_comment_char_at = -1,
                             line_continuation_at = -1;
  unsigned spaces = 0, lparen = 0, rparen = 0, lsquare = 0, rsquare = 0,
           lbrace = 0, rbrace = 0, rchev = 0, lchev = 0;
  bool in_comment = false, in_single = false, in_double = false,
       in_macro = false;

  struct str_elem *scanned_ll = NULL;
  struct str_elem **scanned_cur_ptr = &scanned_ll;

  for (i = 0, scan_from = 0; i < n; i++) {
    if (isspace(source[i]))
      spaces++;
    else
      switch (source[i]) {
      case '/':
        if (!in_single && !in_double) {
          if (cpp_comment_char_at == i - 1)
            in_comment = true;
          else if (c_comment_char_at == i - 1)
            in_comment = false;
          else {
            cpp_comment_char_at = (ssize_t)i;
            c_comment_char_at = (ssize_t)i;
          }
        }
        break;
      case '*':
        if (!in_single && !in_double) {
          if (in_comment)
            c_comment_char_at = (ssize_t)i;
          else if (c_comment_char_at == i - 1)
            in_comment = true;
        }
        break;
      case '"':
        if (!in_single)
          in_double = !in_double;
        break;
      case '\'':
        if (!in_double)
          in_single = !in_single;
        break;
      case '#':
        if (!in_single && !in_double && !in_comment)
          in_macro = true;
        break;
      case '(':
        if (!in_single && !in_double && !in_comment)
          lparen++;
        break;
      case ')':
        if (!in_single && !in_double && !in_comment)
          rparen++;
        break;
      case '[':
        if (!in_single && !in_double && !in_comment)
          lsquare++;
        break;
      case ']':
        if (!in_single && !in_double && !in_comment)
          rsquare++;
        break;
      case '{':
        if (!in_single && !in_double && !in_comment) {
          lbrace++;
          /* printf("{ found: %ld -> \"%.*s\"\n",
           *        i, (int)(scan_from-i), source+i); */
          {
            const char *expr = process_as_expression(
                source, i, &scan_from, line_continuation_at, in_comment,
                in_single, in_double, &c_comment_char_at, &cpp_comment_char_at,
                &spaces, &lparen, &rparen, &lsquare, &rsquare, &lbrace, &rbrace,
                &rchev, &lchev);
            if (expr != NULL)
              scanned_cur_ptr =
                  push_expr_to_ll(&scanned_n, &scanned_cur_ptr, expr);
          }
        }
        break;
      case '}':
        if (!in_single && !in_double && !in_comment)
          rbrace++;
        break;
      case '<':
        if (!in_single && !in_double && !in_comment)
          lchev++;
        break;
      case '>':
        if (!in_single && !in_double && !in_comment)
          rchev++;
        break;
      case '\\':
        if (!in_single && !in_double && !in_comment)
          line_continuation_at = (ssize_t)i;
        break;
      case ';':
      case '\n':
      {
        const size_t substr_length = i - scan_from + 1;

        char *substr = malloc(sizeof(char) * substr_length);
        sprintf(substr, "%.*s", (int) substr_length, source + scan_from);
        substr[substr_length] = '\0';
        printf("[%s] substr: \"%s\"\n", lbrace == rbrace? "true": "false", substr);
        free(substr);
      }

        if (lbrace == rbrace) {
          const char *expr = process_as_expression(
              source, i, &scan_from, line_continuation_at, in_comment,
              in_single, in_double, &c_comment_char_at, &cpp_comment_char_at,
              &spaces, &lparen, &rparen, &lsquare, &rsquare, &lbrace, &rbrace,
              &rchev, &lchev);
          if (expr != NULL)
            scanned_cur_ptr =
                push_expr_to_ll(&scanned_n, &scanned_cur_ptr, expr);
          break;
        }
      }
  }
  printf("i = %ld ; scan_from = %ld ; n = %ld\n", i, scan_from, n);
  {
    const size_t substr_length = i - scan_from + 1;

    char *substr = malloc(sizeof(char) * substr_length);
    sprintf(substr, "%.*s", (int) substr_length, source + scan_from);
    substr[substr_length] = '\0';
    printf("leftover substr: \"%s\"\n", substr);
    free(substr);
  }
  {
    const char *expr = process_as_expression(
        source, i, &scan_from, line_continuation_at, in_comment,
        in_single, in_double, &c_comment_char_at, &cpp_comment_char_at,
        &spaces, &lparen, &rparen, &lsquare, &rsquare, &lbrace, &rbrace,
        &rchev, &lchev);
    if (expr != NULL)
      scanned_cur_ptr =
          push_expr_to_ll(&scanned_n, &scanned_cur_ptr, expr);
  }
  return scanned_ll;
}

struct str_elem **push_expr_to_ll(size_t *scanned_n,
                                  struct str_elem ***scanned_ll,
                                  const char *expr) {
  if (expr != NULL) {
    /* struct StrNode cur_node = {expr, strlen(expr), NULL}; */
    /*struct str_elem *cur_node = malloc(sizeof (struct StrNode));
    cur_node->s = strdup(expr);
    cur_node->n = strlen(expr);
    cur_node->next = NULL;*/
    (*scanned_n)++;
    return append(*scanned_ll, expr);

    /* scanned = realloc(scanned, ++scanned_n * sizeof(*scanned)); */
    /* strncpy(scanned[scanned_n], = expr; */
  }
  return *scanned_ll;
}

const char *process_as_expression(
    const char *source, size_t i, size_t *scan_from,
    ssize_t line_continuation_at, bool in_comment, bool in_single,
    bool in_double, ssize_t *c_comment_char_at, ssize_t *cpp_comment_char_at,
    unsigned int *spaces, unsigned int *lparen, unsigned int *rparen,
    unsigned int *lsquare, unsigned int *rsquare, unsigned int *lbrace,
    unsigned int *rbrace, unsigned int *rchev, unsigned int *lchev) {
  if (!in_single && !in_double && !in_comment &&
      line_continuation_at != i - 1 && (*lparen) == (*rparen) &&
      (*lsquare) == (*rsquare) && (*lchev) == (*rchev)) {
    const size_t substr_length = i - *scan_from + 1;

    char *substr = malloc(sizeof(char) * substr_length);
    sprintf(substr, "%.*s", (int)substr_length, source + *scan_from);
    substr[substr_length] = '\0';
    printf("process_as_expression: \"%s\"\n", substr);

    (*cpp_comment_char_at) = -1;
    (*c_comment_char_at) = -1;
    (*spaces) = 0, (*lparen) = 0, (*rparen) = 0, (*lsquare) = 0, (*rsquare) = 0,
    (*rbrace) = 0, (*rchev) = 0, (*lchev) = 0;
    *scan_from = i + 1;
    return substr;
  }
  return NULL;
}

const struct CstNode **parser(struct str_elem *scanned) { return NULL; }
