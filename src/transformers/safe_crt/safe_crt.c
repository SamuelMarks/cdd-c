/* clang-format off */
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include "c_str_span.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
/* clang-format on */

typedef struct safe_crt_arena_t safe_crt_arena_t;
/** @brief Struct definition */
struct safe_crt_arena_t {
  /** @brief field */
  /** @brief field */
  safe_crt_arena_t *next;
  /** @brief field */
  /** @brief field */
  char data[1];
};

static safe_crt_arena_t *global_arena = NULL;
static cdd_cst_tree_t *current_tree = NULL;

static void arena_free_all(void) {
  safe_crt_arena_t *node = global_arena;
  while (node) {
    safe_crt_arena_t *next = node->next;
    free(node);
    node = next;
  }
  global_arena = NULL;
}

static void *arena_alloc(size_t len) {
  safe_crt_arena_t *node =
      (safe_crt_arena_t *)malloc(sizeof(safe_crt_arena_t) + len);
  if (!node)
    return NULL;
  node->next = global_arena;
  global_arena = node;
  return node->data;
}

typedef struct expr_t expr_t;
/** @brief Struct definition */
struct expr_t {
  int type; /* 0: token, 1: call, 2: group, 3: fopen assign, 4: hidden */
  /** @brief field */
  /** @brief field */
  cdd_token_t *tok;
  /** @brief field */
  /** @brief field */
  cdd_token_t *close_tok;
  /** @brief field */
  /** @brief field */
  expr_t *args[16];
  /** @brief field */
  /** @brief field */
  size_t num_args;
  /** @brief field */
  /** @brief field */
  expr_t *next;
};

static void append_trivia(char **buf, size_t *len, size_t *cap,
                          cdd_trivia_t *tr) {
  while (tr) {
    if (*len + tr->length + 1 >= *cap) {
      *cap = (*cap * 2) + tr->length + 1;
      *buf = (char *)realloc(*buf, *cap);
    }
    memcpy(*buf + *len, tr->start, tr->length);
    *len += tr->length;
    (*buf)[*len] = '\0';
    tr = tr->next;
  }
}

static void append_str(char **buf, size_t *len, size_t *cap, const char *str,
                       size_t slen) {
  if (*len + slen + 1 >= *cap) {
    *cap = (*cap * 2) + slen + 1;
    *buf = (char *)realloc(*buf, *cap);
  }
  memcpy(*buf + *len, str, slen);
  *len += slen;
  (*buf)[*len] = '\0';
}

static expr_t *parse_expr_ast(cdd_cst_node_t *stmt, size_t *idx,
                              int stop_at_comma) {
  expr_t *head = NULL;
  expr_t *tail = NULL;

  while (*idx < stmt->num_children) {
    cdd_token_t *t = stmt->children[*idx].val.token;
    expr_t *node;

    if (t->kind == CDD_TOKEN_SEMICOLON && stop_at_comma)
      break;
    if (t->kind == CDD_TOKEN_RPAREN || t->kind == CDD_TOKEN_RBRACKET ||
        t->kind == CDD_TOKEN_RBRACE)
      break;
    if (t->kind == CDD_TOKEN_COMMA && stop_at_comma)
      break;

    node = (expr_t *)arena_alloc(sizeof(expr_t));
    memset(node, 0, sizeof(expr_t));
    node->tok = t;

    if (t->kind == CDD_TOKEN_IDENTIFIER && (*idx + 1) < stmt->num_children &&
        stmt->children[*idx + 1].val.token->kind == CDD_TOKEN_LPAREN) {
      node->type = 1;
      (*idx) += 2;
      while (*idx < stmt->num_children) {
        size_t old_idx;
        if (stmt->children[*idx].val.token->kind == CDD_TOKEN_RPAREN) {
          node->close_tok = stmt->children[*idx].val.token;
          (*idx)++;
          break;
        }
        if (stmt->children[*idx].val.token->kind == CDD_TOKEN_SEMICOLON) {
          break;
        }
        old_idx = *idx;
        if (node->num_args < 16) {
          node->args[node->num_args++] = parse_expr_ast(stmt, idx, 1);
        } else {
          parse_expr_ast(stmt, idx, 1);
        }
        if (*idx == old_idx) {
          if (*idx < stmt->num_children) {
            int tk = stmt->children[*idx].val.token->kind;
            if (tk != CDD_TOKEN_COMMA && tk != CDD_TOKEN_RPAREN &&
                tk != CDD_TOKEN_SEMICOLON) {
              (*idx)++;
            }
          } else {
            break;
          }
        }
        if (*idx < stmt->num_children &&
            stmt->children[*idx].val.token->kind == CDD_TOKEN_COMMA) {
          (*idx)++;
        }
      }
    } else if (t->kind == CDD_TOKEN_LPAREN || t->kind == CDD_TOKEN_LBRACKET ||
               t->kind == CDD_TOKEN_LBRACE) {
      enum cdd_token_kind_t closing =
          t->kind == CDD_TOKEN_LPAREN     ? CDD_TOKEN_RPAREN
          : t->kind == CDD_TOKEN_LBRACKET ? CDD_TOKEN_RBRACKET
                                          : CDD_TOKEN_RBRACE;
      node->type = 2;
      (*idx)++;
      node->args[0] = parse_expr_ast(stmt, idx, 0);
      if (*idx < stmt->num_children &&
          stmt->children[*idx].val.token->kind == closing) {
        node->close_tok = stmt->children[*idx].val.token;
        (*idx)++;
      }
    } else {
      node->type = 0;
      (*idx)++;
    }

    if (!head) {
      head = tail = node;
    } else {
      tail->next = node;
      tail = node;
    }
  }
  return head;
}

static int find_and_mark_fopen(expr_t *head) {
  expr_t *curr = head;
  expr_t *lhs_start = head;
  int found = 0;
  size_t i;

  while (curr) {
    if (curr->type == 0 && curr->tok->kind == CDD_TOKEN_SEMICOLON) {
      lhs_start = curr->next;
    } else if (curr->type == 0 && curr->tok->kind == CDD_TOKEN_ASSIGN) {
      if (curr->next && curr->next->type == 1) {
        char name[16] = {0};
        size_t nlen =
            curr->next->tok->length < 15 ? curr->next->tok->length : 15;
        memcpy(name, curr->next->tok->start, nlen);
        if (strcmp(name, "fopen") == 0 || strcmp(name, "_wfopen") == 0 ||
            strcmp(name, "freopen") == 0 || strcmp(name, "tmpfile") == 0) {
          expr_t *t;
          curr->type = 3;
          curr->args[0] = lhs_start;
          curr->args[1] = curr->next;
          t = lhs_start;
          while (t && t != curr) {
            t->type = 4;
            t = t->next;
          }
          curr->next->type = 5;
          found = 1;
        }
      }
    }

    if (curr->type == 1 || curr->type == 2) {
      for (i = 0; i < curr->num_args; i++) {
        if (find_and_mark_fopen(curr->args[i]))
          found = 1;
      }
      if (curr->type == 2 && find_and_mark_fopen(curr->args[0]))
        found = 1;
    }

    curr = curr->next;
  }
  return found;
}

static void check_unsupported_calls(expr_t *head) {
  size_t i;
  while (head) {
    if (head->type == 1) {
      char name[128] = {0};
      size_t nlen = head->tok->length < 127 ? head->tok->length : 127;
      memcpy(name, head->tok->start, nlen);
      if (strcmp(name, "fopen") == 0 || strcmp(name, "_wfopen") == 0 ||
          strcmp(name, "freopen") == 0 || strcmp(name, "tmpfile") == 0) {
        fprintf(stderr,
                "WARNING: Bare %s call detected without assignment. Cannot "
                "refactor automatically.\n",
                name);
      }
      for (i = 0; i < head->num_args; i++) {
        check_unsupported_calls(head->args[i]);
      }
    } else if (head->type == 2) {
      check_unsupported_calls(head->args[0]);
    }
    head = head->next;
  }
}

static int check_needs_transform(expr_t *head) {
  size_t i;
  while (head) {
    if (head->type == 1 || head->type == 5) {
      char name[128] = {0};
      size_t nlen = head->tok->length < 127 ? head->tok->length : 127;
      memcpy(name, head->tok->start, nlen);
      if (strcmp(name, "strcpy") == 0 || strcmp(name, "strncpy") == 0 ||
          strcmp(name, "sprintf") == 0 || strcmp(name, "vsprintf") == 0 ||
          strcmp(name, "strcat") == 0 || strcmp(name, "strncat") == 0 ||
          strcmp(name, "memcpy") == 0 || strcmp(name, "memmove") == 0 ||
          strcmp(name, "snprintf") == 0 || strcmp(name, "vsnprintf") == 0 ||
          strcmp(name, "printf") == 0 || strcmp(name, "vprintf") == 0 ||
          strcmp(name, "fprintf") == 0 || strcmp(name, "vfprintf") == 0 ||
          strcmp(name, "_snprintf") == 0 || strcmp(name, "_vsnprintf") == 0 ||
          strcmp(name, "wmemcpy") == 0 || strcmp(name, "wmemmove") == 0 ||
          strcmp(name, "wcscpy") == 0 || strcmp(name, "wcsncpy") == 0 ||
          strcmp(name, "wcscat") == 0 || strcmp(name, "wcsncat") == 0 ||
          strcmp(name, "_mbscpy") == 0 || strcmp(name, "_mbsncpy") == 0 ||
          strcmp(name, "_mbscat") == 0 || strcmp(name, "_mbsncat") == 0 ||
          strcmp(name, "swprintf") == 0 || strcmp(name, "vswprintf") == 0 ||
          strcmp(name, "scanf") == 0 || strcmp(name, "fscanf") == 0 ||
          strcmp(name, "sscanf") == 0 || strcmp(name, "vscanf") == 0 ||
          strcmp(name, "vfscanf") == 0 || strcmp(name, "vsscanf") == 0 ||
          strcmp(name, "_itoa") == 0 || strcmp(name, "_ltoa") == 0 ||
          strcmp(name, "_ultoa") == 0 || strcmp(name, "_i64toa") == 0 ||
          strcmp(name, "_ui64toa") == 0 || strcmp(name, "_itow") == 0 ||
          strcmp(name, "_ltow") == 0 || strcmp(name, "strtok") == 0 ||
          strcmp(name, "gets") == 0 || strcmp(name, "strlen") == 0 ||
          strcmp(name, "strerror") == 0 || strcmp(name, "_strerror") == 0 ||
          strcmp(name, "_stricmp") == 0 || strcmp(name, "_strnicmp") == 0 ||
          strcmp(name, "_strlwr") == 0 || strcmp(name, "_strupr") == 0 ||
          strcmp(name, "_strnset") == 0 || strcmp(name, "_strset") == 0 ||
          strcmp(name, "_mbslwr") == 0 || strcmp(name, "_mbsupr") == 0 ||
          strcmp(name, "_mbsset") == 0 || strcmp(name, "_mbsnset") == 0 ||
          strcmp(name, "_mbstok") == 0 || strcmp(name, "wcstok") == 0 ||
          strcmp(name, "_wcslwr") == 0 || strcmp(name, "_wcsupr") == 0 ||
          strcmp(name, "_wcserror") == 0 || strcmp(name, "mbstowcs") == 0 ||
          strcmp(name, "wcstombs") == 0 || strcmp(name, "wctomb") == 0 ||
          strcmp(name, "_gcvt") == 0 || strcmp(name, "_ecvt") == 0 ||
          strcmp(name, "_fcvt") == 0 || strcmp(name, "tmpfile") == 0 ||
          strcmp(name, "tmpnam") == 0 || strcmp(name, "_splitpath") == 0 ||
          strcmp(name, "_makepath") == 0 || strcmp(name, "_wsplitpath") == 0 ||
          strcmp(name, "_wmakepath") == 0 || strcmp(name, "getenv") == 0 ||
          strcmp(name, "_wgetenv") == 0 || strcmp(name, "_putenv") == 0 ||
          strcmp(name, "_wputenv") == 0 || strcmp(name, "_searchenv") == 0 ||
          strcmp(name, "_wsearchenv") == 0 || strcmp(name, "qsort") == 0 ||
          strcmp(name, "bsearch") == 0 || strcmp(name, "_ultow") == 0) {
        return 1;
      }
      for (i = 0; i < head->num_args; i++) {
        if (check_needs_transform(head->args[i]))
          return 1;
      }
    } else if (head->type == 2) {
      if (check_needs_transform(head->args[0]))
        return 1;
    } else if (head->type == 3) {
      return 1;
    }
    head = head->next;
  }
  return 0;
}

typedef struct {
  int is_msc;
  int needs_errbuf;
  int needs_wcserrbuf;
  int needs_strtokctx;
  int needs_wcstokctx;
  int needs_mbstokctx;
  int needs_ecvtbuf;
  int needs_fcvtbuf;
  int needs_retbuf;
  int needs_getenv_ptr;
  int needs_wgetenv_ptr;
} emit_ctx_t;

static emit_ctx_t *g_msc_ctx = NULL;

static void emit_ast(expr_t *node, char **buf, size_t *len, size_t *cap,
                     int is_msc) {
  size_t k;
  emit_ctx_t *ctx = is_msc ? g_msc_ctx : NULL;
  while (node) {
    if (node->type == 0 || (node->type == 4 && !is_msc)) {
      append_trivia(buf, len, cap, node->tok->leading_trivia);
      append_str(buf, len, cap, (const char *)node->tok->start,
                 node->tok->length);
      append_trivia(buf, len, cap, node->tok->trailing_trivia);
    } else if (node->type == 2) {
      append_trivia(buf, len, cap, node->tok->leading_trivia);
      append_str(buf, len, cap, (const char *)node->tok->start,
                 node->tok->length);
      append_trivia(buf, len, cap, node->tok->trailing_trivia);

      emit_ast(node->args[0], buf, len, cap, is_msc);

      if (node->close_tok) {
        append_trivia(buf, len, cap, node->close_tok->leading_trivia);
        append_str(buf, len, cap, (const char *)node->close_tok->start,
                   node->close_tok->length);
        append_trivia(buf, len, cap, node->close_tok->trailing_trivia);
      }
    } else if (node->type == 1 || (node->type == 5 && !is_msc)) {
      char name[128];
      size_t nlen = node->tok->length < 127 ? node->tok->length : 127;
      int is_safe = 0;

      memcpy(name, node->tok->start, nlen);
      name[nlen] = '\0';

      if (is_msc) {
        if (strcmp(name, "strcpy") == 0 || strcmp(name, "strcat") == 0 ||
            strcmp(name, "sprintf") == 0 || strcmp(name, "vsprintf") == 0 ||
            strcmp(name, "wcscpy") == 0 || strcmp(name, "wcscat") == 0 ||
            strcmp(name, "_mbscpy") == 0 || strcmp(name, "_mbscat") == 0 ||
            strcmp(name, "swprintf") == 0 || strcmp(name, "vswprintf") == 0 ||
            strcmp(name, "_strnset") == 0 || strcmp(name, "_strset") == 0 ||
            strcmp(name, "_mbsset") == 0)
          is_safe = 1;
        else if (strcmp(name, "_strlwr") == 0 || strcmp(name, "_strupr") == 0 ||
                 strcmp(name, "_mbslwr") == 0 || strcmp(name, "_mbsupr") == 0 ||
                 strcmp(name, "_wcslwr") == 0 || strcmp(name, "_wcsupr") == 0 ||
                 strcmp(name, "gets") == 0 || strcmp(name, "tmpnam") == 0 ||
                 strcmp(name, "strlen") == 0)
          is_safe = 9;
        else if (strcmp(name, "strncpy") == 0 || strcmp(name, "strncat") == 0 ||
                 strcmp(name, "wcsncpy") == 0 || strcmp(name, "wcsncat") == 0 ||
                 strcmp(name, "_mbsncpy") == 0 ||
                 strcmp(name, "_mbsncat") == 0 || strcmp(name, "_mbsnset") == 0)
          is_safe = 2;
        else if (strcmp(name, "snprintf") == 0 ||
                 strcmp(name, "vsnprintf") == 0 ||
                 strcmp(name, "_snprintf") == 0 ||
                 strcmp(name, "_vsnprintf") == 0)
          is_safe = 3;
        else if (strcmp(name, "printf") == 0 || strcmp(name, "vprintf") == 0 ||
                 strcmp(name, "fprintf") == 0 || strcmp(name, "vfprintf") == 0)
          is_safe = 4;
        else if (strcmp(name, "memcpy") == 0 || strcmp(name, "memmove") == 0 ||
                 strcmp(name, "wmemcpy") == 0 || strcmp(name, "wmemmove") == 0)
          is_safe = 6;
        else if (strcmp(name, "scanf") == 0 || strcmp(name, "fscanf") == 0 ||
                 strcmp(name, "sscanf") == 0 || strcmp(name, "vscanf") == 0 ||
                 strcmp(name, "vfscanf") == 0 || strcmp(name, "vsscanf") == 0)
          is_safe = 7;
        else if (strcmp(name, "_itoa") == 0 || strcmp(name, "_ltoa") == 0 ||
                 strcmp(name, "_ultoa") == 0 || strcmp(name, "_i64toa") == 0 ||
                 strcmp(name, "_ui64toa") == 0 || strcmp(name, "_itow") == 0 ||
                 strcmp(name, "_ltow") == 0 || strcmp(name, "_ultow") == 0)
          is_safe = 8;
        else if (strcmp(name, "strlen") == 0)
          is_safe = 9;
        else if (strcmp(name, "_splitpath") == 0 ||
                 strcmp(name, "_wsplitpath") == 0)
          is_safe = 10;
        else if (strcmp(name, "_makepath") == 0 ||
                 strcmp(name, "_wmakepath") == 0)
          is_safe = 11;
        else if (strcmp(name, "_gcvt") == 0)
          is_safe = 19;
        else if (strcmp(name, "mbstowcs") == 0 || strcmp(name, "wcstombs") == 0)
          is_safe = 20;
        else if (strcmp(name, "wctomb") == 0)
          is_safe = 21;
        else if (strcmp(name, "_searchenv") == 0 ||
                 strcmp(name, "_wsearchenv") == 0)
          is_safe = 22;
        else if (strcmp(name, "getenv") == 0 || strcmp(name, "_wgetenv") == 0)
          is_safe = 23;
        else if (strcmp(name, "_putenv") == 0 || strcmp(name, "_wputenv") == 0)
          is_safe = 24;
        else if (strcmp(name, "qsort") == 0 || strcmp(name, "bsearch") == 0)
          is_safe = 25;
      }

      if (is_safe && is_safe != 18 && is_safe != 19 && is_safe != 20 &&
          is_safe != 21 && is_safe != 22 && is_safe != 23 && is_safe != 24 &&
          is_safe != 25) {
        char safe_name[128];
        if (strcmp(name, "strlen") == 0) {
          strcpy(safe_name, "strnlen_s");
        } else {
          if (strcmp(name, "strlen") == 0) {
            strcpy(safe_name, "strnlen_s");
          } else {
            sprintf(safe_name, "%s_s", name);
          }
        }
        append_trivia(buf, len, cap, node->tok->leading_trivia);
        append_str(buf, len, cap, safe_name, strlen(safe_name));
        append_trivia(buf, len, cap, node->tok->trailing_trivia);
        append_str(buf, len, cap, "(", 1);
      } else if (!is_safe) {
        append_trivia(buf, len, cap, node->tok->leading_trivia);
        append_str(buf, len, cap, (const char *)node->tok->start,
                   node->tok->length);
        append_trivia(buf, len, cap, node->tok->trailing_trivia);
        append_str(buf, len, cap, "(", 1);
      } else {
        /* Do not append function name or ( for custom formatted functions */
        append_trivia(buf, len, cap, node->tok->leading_trivia);
      }
      if (is_safe == 1 && node->num_args >= 2) {
        char *dummy_buf;
        size_t dummy_len = 0;
        size_t dummy_cap = 1024;
        char *stripped;

        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", sizeof(", 9);

        dummy_buf = (char *)malloc(1024);
        dummy_buf[0] = 0;
        emit_ast(node->args[0], &dummy_buf, &dummy_len, &dummy_cap, 0);

        stripped = dummy_buf;
        while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
               *stripped == '\r')
          stripped++;
        append_str(buf, len, cap, stripped, strlen(stripped));
        free(dummy_buf);

        append_str(buf, len, cap, ")", 1);

        for (k = 1; k < node->num_args; k++) {
          append_str(buf, len, cap, ", ", 2);
          emit_ast(node->args[k], buf, len, cap, is_msc);
        }
      } else if (is_safe == 2 && node->num_args >= 3) {
        char *dummy_buf;
        size_t dummy_len = 0;
        size_t dummy_cap = 1024;
        char *stripped;

        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", sizeof(", 9);

        dummy_buf = (char *)malloc(1024);
        dummy_buf[0] = 0;
        emit_ast(node->args[0], &dummy_buf, &dummy_len, &dummy_cap, 0);

        stripped = dummy_buf;
        while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
               *stripped == '\r')
          stripped++;
        append_str(buf, len, cap, stripped, strlen(stripped));
        free(dummy_buf);

        append_str(buf, len, cap, "), ", 3);
        emit_ast(node->args[1], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", _TRUNCATE", 11);
      } else if (is_safe == 3 && node->num_args >= 3) {
        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);
        emit_ast(node->args[1], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", _TRUNCATE", 11);
        for (k = 2; k < node->num_args; k++) {
          append_str(buf, len, cap, ", ", 2);
          emit_ast(node->args[k], buf, len, cap, is_msc);
        }
      } else if (is_safe == 6 && node->num_args >= 3) {
        char *dummy_buf;
        size_t dummy_len = 0;
        size_t dummy_cap = 1024;
        char *stripped;

        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", sizeof(", 9);

        dummy_buf = (char *)malloc(1024);
        dummy_buf[0] = 0;
        emit_ast(node->args[0], &dummy_buf, &dummy_len, &dummy_cap, 0);

        stripped = dummy_buf;
        while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
               *stripped == '\r')
          stripped++;
        append_str(buf, len, cap, stripped, strlen(stripped));
        free(dummy_buf);

        append_str(buf, len, cap, "), ", 3);
        emit_ast(node->args[1], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);
        emit_ast(node->args[2], buf, len, cap, is_msc);
      } else if (is_safe == 8 && node->num_args >= 3) {
        char *dummy_buf;
        size_t dummy_len = 0;
        size_t dummy_cap = 1024;
        char *stripped;

        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);
        emit_ast(node->args[1], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", sizeof(", 9);

        dummy_buf = (char *)malloc(1024);
        dummy_buf[0] = 0;
        emit_ast(node->args[1], &dummy_buf, &dummy_len, &dummy_cap, 0);

        stripped = dummy_buf;
        while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
               *stripped == '\r')
          stripped++;
        append_str(buf, len, cap, stripped, strlen(stripped));
        free(dummy_buf);

        append_str(buf, len, cap, "), ", 3);
        emit_ast(node->args[2], buf, len, cap, is_msc);
      } else if (is_safe == 9 && node->num_args == 1) {
        char *dummy_buf;
        size_t dummy_len = 0;
        size_t dummy_cap = 1024;
        char *stripped;

        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", sizeof(", 9);

        dummy_buf = (char *)malloc(1024);
        dummy_buf[0] = 0;
        emit_ast(node->args[0], &dummy_buf, &dummy_len, &dummy_cap, 0);

        stripped = dummy_buf;
        while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
               *stripped == '\r')
          stripped++;
        append_str(buf, len, cap, stripped, strlen(stripped));
        free(dummy_buf);

        append_str(buf, len, cap, ")", 1);
      } else if (is_safe == 10 && node->num_args >= 5) {
        emit_ast(node->args[0], buf, len, cap, is_msc);
        for (k = 1; k < 5; k++) {
          append_str(buf, len, cap, ", ", 2);
          emit_ast(node->args[k], buf, len, cap, is_msc);
          append_str(buf, len, cap, ", ", 2);

          {
            char *dummy_buf;
            size_t dummy_len = 0;
            size_t dummy_cap = 1024;
            char *stripped;

            dummy_buf = (char *)malloc(1024);
            dummy_buf[0] = 0;
            emit_ast(node->args[k], &dummy_buf, &dummy_len, &dummy_cap, 0);

            stripped = dummy_buf;
            while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
                   *stripped == '\r')
              stripped++;

            if (strcmp(stripped, "NULL") == 0 || strcmp(stripped, "0") == 0) {
              append_str(buf, len, cap, "0", 1);
            } else {
              append_str(buf, len, cap, "sizeof(", 7);
              append_str(buf, len, cap, stripped, strlen(stripped));
              append_str(buf, len, cap, ")", 1);
            }
            free(dummy_buf);
          }
        }
      } else if (is_safe == 11 && node->num_args >= 5) {
        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", sizeof(", 9);
        {
          char *dummy_buf;
          size_t dummy_len = 0;
          size_t dummy_cap = 1024;
          char *stripped;

          dummy_buf = (char *)malloc(1024);
          dummy_buf[0] = 0;
          emit_ast(node->args[0], &dummy_buf, &dummy_len, &dummy_cap, 0);

          stripped = dummy_buf;
          while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
                 *stripped == '\r')
            stripped++;

          append_str(buf, len, cap, stripped, strlen(stripped));
          free(dummy_buf);
        }
        append_str(buf, len, cap, ")", 1);

        for (k = 1; k < node->num_args; k++) {
          append_str(buf, len, cap, ", ", 2);
          emit_ast(node->args[k], buf, len, cap, is_msc);
        }
      } else if (is_safe == 19 && node->num_args == 3) {
        char *dummy_buf;
        size_t dummy_len = 0;
        size_t dummy_cap = 1024;
        char *stripped;

        append_str(buf, len, cap, "(_gcvt_s(", 9);
        emit_ast(node->args[2], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);

        dummy_buf = (char *)malloc(1024);
        dummy_buf[0] = 0;
        emit_ast(node->args[2], &dummy_buf, &dummy_len, &dummy_cap, 0);
        stripped = dummy_buf;
        while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
               *stripped == '\r')
          stripped++;
        append_str(buf, len, cap, "sizeof(", 7);
        append_str(buf, len, cap, stripped, strlen(stripped));
        append_str(buf, len, cap, "), ", 3);
        free(dummy_buf);

        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);
        emit_ast(node->args[1], buf, len, cap, is_msc);
        append_str(buf, len, cap, "), ", 3);
        emit_ast(node->args[2], buf, len, cap, is_msc);
      } else if (is_safe == 20 && node->num_args == 3) {
        char *dummy_buf;
        size_t dummy_len = 0;
        size_t dummy_cap = 1024;
        char *stripped;

        append_str(buf, len, cap, "(", 1);
        append_str(buf, len, cap, name, strlen(name));
        append_str(buf, len, cap, "_s(NULL, ", 9);
        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);

        dummy_buf = (char *)malloc(1024);
        dummy_buf[0] = 0;
        emit_ast(node->args[0], &dummy_buf, &dummy_len, &dummy_cap, 0);
        stripped = dummy_buf;
        while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
               *stripped == '\r')
          stripped++;
        append_str(buf, len, cap, "(sizeof(", 8);
        append_str(buf, len, cap, stripped, strlen(stripped));
        if (strcmp(name, "wcstombs") == 0) {
          append_str(buf, len, cap, ")), ", 4);
        } else {
          append_str(buf, len, cap, ")) / sizeof(wchar_t), ", 22);
        }
        free(dummy_buf);

        emit_ast(node->args[1], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);
        emit_ast(node->args[2], buf, len, cap, is_msc);
        append_str(buf, len, cap, "), ", 3);
        emit_ast(node->args[0], buf, len, cap, is_msc);
      } else if (is_safe == 21 && node->num_args == 2) {
        char *dummy_buf;
        size_t dummy_len = 0;
        size_t dummy_cap = 1024;
        char *stripped;

        append_str(buf, len, cap, "(wctomb_s(NULL, ", 16);
        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);

        dummy_buf = (char *)malloc(1024);
        dummy_buf[0] = 0;
        emit_ast(node->args[0], &dummy_buf, &dummy_len, &dummy_cap, 0);
        stripped = dummy_buf;
        while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
               *stripped == '\r')
          stripped++;
        append_str(buf, len, cap, "sizeof(", 7);
        append_str(buf, len, cap, stripped, strlen(stripped));
        append_str(buf, len, cap, "), ", 3);
        free(dummy_buf);

        emit_ast(node->args[1], buf, len, cap, is_msc);
        append_str(buf, len, cap, "), ", 3);
        emit_ast(node->args[0], buf, len, cap, is_msc);
      } else if (is_safe == 22 && node->num_args == 3) {
        char *dummy_buf;
        size_t dummy_len = 0;
        size_t dummy_cap = 1024;
        char *stripped;

        append_str(buf, len, cap, name, strlen(name));
        append_str(buf, len, cap, "_s(", 3);
        emit_ast(node->args[0], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);
        emit_ast(node->args[1], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);
        emit_ast(node->args[2], buf, len, cap, is_msc);
        append_str(buf, len, cap, ", ", 2);

        dummy_buf = (char *)malloc(1024);
        dummy_buf[0] = 0;
        emit_ast(node->args[2], &dummy_buf, &dummy_len, &dummy_cap, 0);
        stripped = dummy_buf;
        while (*stripped == ' ' || *stripped == '\t' || *stripped == '\n' ||
               *stripped == '\r')
          stripped++;
        append_str(buf, len, cap, "sizeof(", 7);
        append_str(buf, len, cap, stripped, strlen(stripped));
        if (strcmp(name, "_wsearchenv") == 0) {
          append_str(buf, len, cap, ") / sizeof(wchar_t)", 19);
        } else {
          append_str(buf, len, cap, ")", 1);
        }
        free(dummy_buf);
      } else if (is_safe == 23 && node->num_args == 1) {
        if (strcmp(name, "_wgetenv") == 0) {
          if (ctx)
            ctx->needs_wgetenv_ptr = 1;
          append_str(buf, len, cap, "(_wdupenv_s(&__wgetenv_ptr, NULL, ", 34);
          emit_ast(node->args[0], buf, len, cap, is_msc);
          append_str(buf, len, cap, "), __wgetenv_ptr)", 17);
        } else {
          if (ctx)
            ctx->needs_getenv_ptr = 1;
          append_str(buf, len, cap, "(_dupenv_s(&__getenv_ptr, NULL, ", 32);
          emit_ast(node->args[0], buf, len, cap, is_msc);
          append_str(buf, len, cap, "), __getenv_ptr)", 16);
        }
      } else if (is_safe == 24 && node->num_args == 1) {
        char *dummy_buf;
        size_t dummy_len = 0;
        size_t dummy_cap = 1024;
        char *eq_ptr = NULL;

        dummy_buf = (char *)malloc(1024);
        dummy_buf[0] = 0;
        emit_ast(node->args[0], &dummy_buf, &dummy_len, &dummy_cap, 0);

        eq_ptr = strchr(dummy_buf, '=');
        if (eq_ptr &&
            (dummy_buf[0] == '"' ||
             (dummy_buf[0] == 'L' && dummy_buf[1] == '"')) &&
            eq_ptr > dummy_buf) {
          *eq_ptr = '\0';
          if (strcmp(name, "_wputenv") == 0) {
            append_str(buf, len, cap, "_wputenv_s(", 11);
            append_str(buf, len, cap, dummy_buf, strlen(dummy_buf));
            append_str(buf, len, cap, "\", L\"", 5);
          } else {
            append_str(buf, len, cap, "_putenv_s(", 10);
            append_str(buf, len, cap, dummy_buf, strlen(dummy_buf));
            append_str(buf, len, cap, "\", \"", 4);
          }
          append_str(buf, len, cap, eq_ptr + 1, strlen(eq_ptr + 1) - 1);
          append_str(buf, len, cap, "\"", 1);
        } else {
          append_str(buf, len, cap, name, strlen(name));
          append_str(buf, len, cap, "(", 1);
          emit_ast(node->args[0], buf, len, cap, is_msc);
        }
        free(dummy_buf);
      } else if (is_safe == 25 &&
                 (node->num_args == 4 || node->num_args == 5)) {
        append_str(buf, len, cap, name, strlen(name));
        append_str(buf, len, cap, "_s(", 3);
        for (k = 0; k < node->num_args; k++) {
          if (k > 0)
            append_str(buf, len, cap, ", ", 2);
          emit_ast(node->args[k], buf, len, cap, is_msc);
        }
        append_str(buf, len, cap, ", NULL", 6);
      } else if (is_safe == 7) {
        size_t format_idx = 0;
        if (strcmp(name, "fscanf") == 0 || strcmp(name, "sscanf") == 0 ||
            strcmp(name, "vfscanf") == 0 || strcmp(name, "vsscanf") == 0) {
          format_idx = 1;
        }

        if (format_idx < node->num_args) {
          char *fmt_buf = (char *)malloc(1024);
          size_t fmt_len = 0;
          size_t fmt_cap = 1024;
          int needs_size[32] = {0};
          int num_specifiers = 0;
          char *start;

          fmt_buf[0] = 0;
          emit_ast(node->args[format_idx], &fmt_buf, &fmt_len, &fmt_cap, 0);

          start = strchr(fmt_buf, '"');
          if (start) {
            start++;
            while (*start && *start != '"') {
              if (*start == '%') {
                start++;
                if (*start == '%') {
                  start++;
                  continue;
                }
                if (*start == '*') {
                  start++;
                  while (*start && !isalpha(*start) && *start != '[')
                    start++;
                  if (*start)
                    start++;
                  continue;
                }
                while (*start && !isalpha(*start) && *start != '[')
                  start++;
                if (*start) {
                  if (*start == 's' || *start == 'S' || *start == 'c' ||
                      *start == 'C' || *start == '[') {
                    if (num_specifiers < 32)
                      needs_size[num_specifiers] = 1;
                  }
                  num_specifiers++;
                  if (*start == '[') {
                    start++;
                    if (*start == '^')
                      start++;
                    if (*start == ']')
                      start++;
                    while (*start && *start != ']')
                      start++;
                  }
                  start++;
                }
              } else {
                start++;
              }
            }
          }

          for (k = 0; k < node->num_args; k++) {
            if (k > 0)
              append_str(buf, len, cap, ", ", 2);
            emit_ast(node->args[k], buf, len, cap, is_msc);

            if (k > format_idx) {
              int arg_idx = (int)(k - (format_idx + 1));
              if (arg_idx < num_specifiers && needs_size[arg_idx]) {
                char *dummy_buf;
                size_t dummy_len = 0;
                size_t dummy_cap = 1024;
                char *stripped;

                append_str(buf, len, cap, ", (unsigned)sizeof(", 19);

                dummy_buf = (char *)malloc(1024);
                dummy_buf[0] = 0;
                emit_ast(node->args[k], &dummy_buf, &dummy_len, &dummy_cap, 0);

                stripped = dummy_buf;
                while (*stripped == ' ' || *stripped == '\t' ||
                       *stripped == '\n' || *stripped == '\r')
                  stripped++;

                if (stripped[0] == '&') {
                  append_str(buf, len, cap, stripped + 1, strlen(stripped) - 1);
                } else {
                  append_str(buf, len, cap, stripped, strlen(stripped));
                }
                free(dummy_buf);

                append_str(buf, len, cap, ")", 1);
              }
            }
          }
          free(fmt_buf);
        } else {
          for (k = 0; k < node->num_args; k++) {
            if (k > 0)
              append_str(buf, len, cap, ", ", 2);
            emit_ast(node->args[k], buf, len, cap, is_msc);
          }
        }
      } else {
        for (k = 0; k < node->num_args; k++) {
          if (k > 0)
            append_str(buf, len, cap, ", ", 2);
          emit_ast(node->args[k], buf, len, cap, is_msc);
        }
      }

      if (node->close_tok) {
        append_trivia(buf, len, cap, node->close_tok->leading_trivia);
        append_str(buf, len, cap, ")", 1);
        append_trivia(buf, len, cap, node->close_tok->trailing_trivia);
      } else {
        append_str(buf, len, cap, ")", 1);
      }
    } else if (node->type == 3) {
      if (is_msc) {
        expr_t *lhs = node->args[0];
        expr_t *call = node->args[1];
        int is_decl = 0;
        int token_count = 0;
        int has_dot_arrow_bracket = 0;
        int first_is_star = 0;
        expr_t *last_t = NULL;
        expr_t *t_iter = lhs;

        while (t_iter && t_iter != node) {
          if (t_iter->type == 0 && t_iter->tok) {
            if (token_count == 0 && t_iter->tok->kind == CDD_TOKEN_STAR) {
              first_is_star = 1;
            }
            if (t_iter->tok->kind == CDD_TOKEN_DOT ||
                t_iter->tok->kind == CDD_TOKEN_ARROW ||
                t_iter->tok->kind == CDD_TOKEN_LBRACKET) {
              has_dot_arrow_bracket = 1;
            }
          }
          token_count++;
          last_t = t_iter;
          t_iter = t_iter->next;
        }

        if (token_count > 1 && !has_dot_arrow_bracket && !first_is_star) {
          is_decl = 1;
        }

        if (is_decl) {
          char safe_call[32];
          size_t nlen = call->tok->length < 15 ? call->tok->length : 15;
          char call_name[16] = {0};
          expr_t *t = lhs;
          memcpy(call_name, call->tok->start, nlen);
          sprintf(safe_call, "%s_s(&", call_name);
          while (t && t != node) {
            append_trivia(buf, len, cap, t->tok->leading_trivia);
            append_str(buf, len, cap, (const char *)t->tok->start,
                       t->tok->length);
            append_trivia(buf, len, cap, t->tok->trailing_trivia);
            t = t->next;
          }
          append_str(buf, len, cap, ";\n", 2);
          append_str(buf, len, cap, safe_call, strlen(safe_call));
          append_str(buf, len, cap, (const char *)last_t->tok->start,
                     last_t->tok->length);
        } else {
          char safe_call[32];
          size_t nlen = call->tok->length < 15 ? call->tok->length : 15;
          char call_name[16] = {0};
          expr_t *t = lhs;
          memcpy(call_name, call->tok->start, nlen);
          sprintf(safe_call, "%s_s(&", call_name);
          append_trivia(buf, len, cap, node->tok->leading_trivia);
          append_str(buf, len, cap, safe_call, strlen(safe_call));
          while (t && t != node) {
            append_trivia(buf, len, cap, t->tok->leading_trivia);
            append_str(buf, len, cap, (const char *)t->tok->start,
                       t->tok->length);
            append_trivia(buf, len, cap, t->tok->trailing_trivia);
            t = t->next;
          }
        }
        if (call->num_args >= 1) {
          append_str(buf, len, cap, ", ", 2);
          emit_ast(call->args[0], buf, len, cap, is_msc);
        }
        if (call->num_args >= 2) {
          append_str(buf, len, cap, ", ", 2);
          emit_ast(call->args[1], buf, len, cap, is_msc);
        }
        if (call->num_args >= 3) {
          append_str(buf, len, cap, ", ", 2);
          emit_ast(call->args[2], buf, len, cap, is_msc);
        }
        append_str(buf, len, cap, ")", 1);
        append_trivia(buf, len, cap, node->tok->trailing_trivia);
      } else {
        append_trivia(buf, len, cap, node->tok->leading_trivia);
        append_str(buf, len, cap, "=", 1);
        append_trivia(buf, len, cap, node->tok->trailing_trivia);
      }
    }

    node = node->next;
  }
}

static void get_indent_string(cdd_token_t *tok, char *out_indent) {
  cdd_trivia_t *tr = tok->leading_trivia;
  cdd_trivia_t *last_ws = NULL;

  out_indent[0] = '\0';

  while (tr) {
    if (tr->kind == TRIVIA_WHITESPACE) {
      last_ws = tr;
    } else if (tr->kind == TRIVIA_NEWLINE) {
      last_ws = NULL;
    }
    tr = tr->next;
  }

  if (last_ws) {
    size_t len = last_ws->length < 63 ? last_ws->length : 63;
    memcpy(out_indent, last_ws->start, len);
    out_indent[len] = '\0';
  } else {
    strcpy(out_indent, "  ");
  }
}

static int replace_safe_crt(cdd_cst_tree_t *tree, cdd_cst_node_t *stmt,
                            const char *msc_ver_stmt, const char *else_stmt,
                            const char *indent) {
  size_t buf_len =
      strlen(msc_ver_stmt) + strlen(else_stmt) + strlen(indent) * 2 + 128;
  char *buf = (char *)arena_alloc(buf_len);
  cdd_cst_node_t *new_node;
  cdd_token_t *new_tok;
  cdd_token_t *old_first, *old_last;
  (void)old_first;
  (void)old_last;

  if (!buf)
    return ENOMEM;

  sprintf(buf,
          "\n#if defined(_MSC_VER)\n/*CDD_SAFE_CRT*/ "
          "%s\n#else\n/*CDD_SAFE_CRT*/ %s\n#endif\n",
          msc_ver_stmt, else_stmt);

  new_node = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
  if (!new_node)
    return ENOMEM;
  new_node->kind = CDD_CST_UNKNOWN;

  new_tok = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
  if (!new_tok) {
    free(new_node);
    return ENOMEM;
  }
  new_tok->kind = CDD_TOKEN_IDENTIFIER;
  new_tok->start = (const uint8_t *)buf;
  new_tok->length = strlen(buf);

  new_node->children = (cdd_cst_child_t *)calloc(1, sizeof(cdd_cst_child_t));
  if (!new_node->children) {
    free(new_tok);
    free(new_node);
    return ENOMEM;
  }
  new_node->children[0].kind = CDD_CST_CHILD_TOKEN;
  new_node->children[0].val.token = new_tok;
  new_node->num_children = 1;

  if (tree->num_synthesized >= tree->synthesized_capacity) {
    size_t new_cap =
        tree->synthesized_capacity ? tree->synthesized_capacity * 2 : 128;
    tree->synthesized_tokens = (cdd_token_t **)realloc(
        tree->synthesized_tokens, new_cap * sizeof(cdd_token_t *));
    tree->synthesized_capacity = new_cap;
  }
  tree->synthesized_tokens[tree->num_synthesized++] = new_tok;

  old_first =
      stmt->num_children > 0 && stmt->children[0].kind == CDD_CST_CHILD_TOKEN
          ? stmt->children[0].val.token
          : NULL;
  old_last =
      stmt->num_children > 0 &&
              stmt->children[stmt->num_children - 1].kind == CDD_CST_CHILD_TOKEN
          ? stmt->children[stmt->num_children - 1].val.token
          : NULL;

  return cdd_cst_replace_node(tree, stmt, new_node);
}

int cdd_transform_safe_crt(cdd_cst_tree_t *tree,
                           const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i;
  int rc;
  int replaced_any;

  (void)config;

  if (!tree || !tree->root)
    return EINVAL;

  if (current_tree != tree) {
    arena_free_all();
    current_tree = tree;
  }

  do {
    replaced_any = 0;
    rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_UNKNOWN, &res);
    if (rc != 0)
      return rc;

    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *stmt = res.nodes[i];
      size_t idx = 0;
      expr_t *ast;
      cdd_token_t *first_tok = NULL;

      if (stmt->num_children > 0 &&
          stmt->children[0].kind == CDD_CST_CHILD_TOKEN) {
        first_tok = stmt->children[0].val.token;
      }

      /* Single token skip logic */
      if (stmt->num_children == 1 &&
          stmt->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = stmt->children[0].val.token;
        if (tok->length > 14 &&
            memcmp(tok->start, "\n#if defined(_M", 15) == 0) {
          continue; /* Skip our synthesized nodes! */
        }
      }

      {
        cdd_trivia_t *tr = first_tok ? first_tok->leading_trivia : NULL;
        int skip = 0;
        while (tr) {
          if (tr->length >= 14 &&
              memcmp(tr->start, "/*CDD_SAFE_CRT*/", 16) == 0) {
            skip = 1;
            break;
          }
          tr = tr->next;
        }
        if (skip)
          continue;
      }

      {
        cdd_trivia_t *tr = first_tok ? first_tok->leading_trivia : NULL;
        int skip = 0;
        while (tr) {
          if (tr->length >= 14 &&
              memcmp(tr->start, "/*CDD_SAFE_CRT*/", 16) == 0) {
            skip = 1;
            break;
          }
          tr = tr->next;
        }
        if (skip)
          continue;
      }
      printf("PROCESSING STATEMENT: %.*s\n", (int)first_tok->length,
             first_tok->start);
      ast = parse_expr_ast(stmt, &idx, 0);
      find_and_mark_fopen(ast);
      check_unsupported_calls(ast);

      if (check_needs_transform(ast)) {
        char indent[64];
        size_t msc_cap = 1024, msc_len = 0;
        size_t else_cap = 1024, else_len = 0;
        char *msc_buf = (char *)malloc(msc_cap);
        char *else_buf = (char *)malloc(else_cap);
        cdd_trivia_t *saved_trivia =
            first_tok ? first_tok->leading_trivia : NULL;
        emit_ctx_t msc_ctx = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        msc_buf[0] = '\0';
        else_buf[0] = '\0';

        get_indent_string(first_tok, indent);

        if (first_tok)
          first_tok->leading_trivia = NULL;

        g_msc_ctx = &msc_ctx;
        emit_ast(ast, &msc_buf, &msc_len, &msc_cap, 1);
        g_msc_ctx = NULL;

        {
          emit_ast(ast, &else_buf, &else_len, &else_cap, 0);
        }

        if (first_tok)
          first_tok->leading_trivia = saved_trivia;

        if (msc_ctx.needs_errbuf || msc_ctx.needs_wcserrbuf ||
            msc_ctx.needs_strtokctx || msc_ctx.needs_wcstokctx ||
            msc_ctx.needs_mbstokctx || msc_ctx.needs_ecvtbuf ||
            msc_ctx.needs_fcvtbuf || msc_ctx.needs_retbuf ||
            msc_ctx.needs_getenv_ptr || msc_ctx.needs_wgetenv_ptr) {
          char prefix[1024] = "{\n";
          if (msc_ctx.needs_errbuf) {
            strcat(prefix, indent);
            strcat(prefix, "  char __errbuf[94];\n");
          }
          if (msc_ctx.needs_wcserrbuf) {
            strcat(prefix, indent);
            strcat(prefix, "  wchar_t __wcserrbuf[94];\n");
          }
          if (msc_ctx.needs_strtokctx) {
            strcat(prefix, indent);
            strcat(prefix, "  char *__strtokctx = NULL;\n");
          }
          if (msc_ctx.needs_wcstokctx) {
            strcat(prefix, indent);
            strcat(prefix, "  wchar_t *__wcstokctx = NULL;\n");
          }
          if (msc_ctx.needs_mbstokctx) {
            strcat(prefix, indent);
            strcat(prefix, "  unsigned char *__mbstokctx = NULL;\n");
          }
          if (msc_ctx.needs_ecvtbuf) {
            strcat(prefix, indent);
            strcat(prefix, "  char __ecvtbuf[128];\n");
          }
          if (msc_ctx.needs_fcvtbuf) {
            strcat(prefix, indent);
            strcat(prefix, "  char __fcvtbuf[128];\n");
          }
          if (msc_ctx.needs_retbuf) {
            strcat(prefix, indent);
            strcat(prefix, "  size_t __ret;\n");
          }
          if (msc_ctx.needs_getenv_ptr) {
            strcat(prefix, indent);
            strcat(prefix, "  char *__getenv_ptr;\n");
          }
          if (msc_ctx.needs_wgetenv_ptr) {
            strcat(prefix, indent);
            strcat(prefix, "  wchar_t *__wgetenv_ptr;\n");
          }
          strcat(prefix, indent);
          strcat(prefix, "  ");

          {
            char *new_msc_buf = (char *)malloc(msc_len + strlen(prefix) + 64);
            strcpy(new_msc_buf, prefix);
            strcat(new_msc_buf, msc_buf);
            strcat(new_msc_buf, "\n");
            strcat(new_msc_buf, indent);
            strcat(new_msc_buf, "}");
            free(msc_buf);
            msc_buf = new_msc_buf;
          }
        }

        if (strcmp(msc_buf, else_buf) != 0) {
          replace_safe_crt(tree, stmt, msc_buf, else_buf, indent);
          replaced_any = 1;
        }

        free(msc_buf);
        free(else_buf);

        if (replaced_any) {
          break;
        }
      }
    }

    free(res.nodes);
  } while (replaced_any);

  return 0;
}
