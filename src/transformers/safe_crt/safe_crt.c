/**
 * @file safe_crt.c
 * @brief Implementation of the Safe CRT transformer.
 */

/* clang-format off */
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_factory.h"

#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include "c_str_span.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "c_cdd/log.h"
#include "c_cdd_export.h"
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
#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_safe_crt_malloc_fail = 0;
#endif

static void arena_free_all(void) {
  safe_crt_arena_t *node = global_arena;
  while (node) {
    safe_crt_arena_t *next = node->next;
    free(node);
    node = next;
  }
  global_arena = NULL;
}

static int arena_alloc(size_t len, void **out_ptr) {
  safe_crt_arena_t *node;
  if (!out_ptr)
    return EINVAL;
  *out_ptr = NULL;
#ifdef CDD_BUILD_TESTS
  if (g_safe_crt_malloc_fail == 1) {
    node = NULL;
  } else {
#endif
    node = (safe_crt_arena_t *)malloc(sizeof(safe_crt_arena_t) + len);
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!node) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }
  node->next = global_arena;
  global_arena = node;
  *out_ptr = node->data;
  return 0;
}

typedef struct expr_t expr_t;
/** @brief Struct definition */
struct expr_t {
  /** @brief type field */
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

static int parse_expr_ast(cdd_cst_node_t *stmt, size_t *idx, int stop_at_comma,
                          expr_t **out_expr) {
  expr_t *head = NULL;
  expr_t *tail = NULL;
  if (!out_expr)
    return EINVAL;
  *out_expr = NULL;

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

    if (arena_alloc(sizeof(expr_t), (void **)&node) != 0) {
      break;
    }
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
          parse_expr_ast(stmt, idx, 1, &node->args[node->num_args++]);
        } else {
          {
            expr_t *tmp_expr = NULL;
            parse_expr_ast(stmt, idx, 1, &tmp_expr);
          }
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
      parse_expr_ast(stmt, idx, 0, &node->args[0]);
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
  *out_expr = head;
  return head ? 0 : ENOENT;
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
  /** @brief is_msc field */
  int is_msc;
  /** @brief needs_wcserrbuf field */
  int needs_errbuf;
  /** @brief needs_wcstokctx field */
  int needs_wcserrbuf;
  /** @brief needs_ecvtbuf field */
  int needs_strtokctx;
  /** @brief needs_retbuf field */
  int needs_wcstokctx;
  /** @brief needs_wgetenv_ptr field */
  int needs_mbstokctx;
  /** @brief needs_ecvtbuf field */
  int needs_ecvtbuf;
  /** @brief needs_retbuf field */
  int needs_fcvtbuf;
  /** @brief needs_wgetenv_ptr field */
  int needs_retbuf;
  int needs_getenv_ptr;  /**< needs_getenv_ptr */
  int needs_wgetenv_ptr; /**< needs_wgetenv_ptr */
} emit_ctx_t;

static emit_ctx_t *g_msc_ctx = NULL;

static int expr_is_null_or_zero(expr_t *node) {
  if (node && node->type == 0 && node->tok && !node->next) {
    if (node->tok->length == 4 && memcmp(node->tok->start, "NULL", 4) == 0)
      return 1;
    if (node->tok->length == 1 && node->tok->start[0] == '0')
      return 1;
  }
  return 0;
}

static const char *pool_string_safe(cdd_cst_tree_t *tree, const char *str) {
  char *dup;
  if (!tree || !str)
    return NULL;
  dup = strdup(str);
  if (!dup)
    return NULL;
  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap =
        tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
    char **new_pool =
        (char **)realloc(tree->string_pool, new_cap * sizeof(char *));
    if (!new_pool) {
      free(dup);
      return NULL;
    }
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  tree->string_pool[tree->num_strings++] = dup;
  return dup;
}

static int clone_trivia(cdd_trivia_t *head, cdd_trivia_t **out_trivia) {
  cdd_trivia_t *new_head = NULL;
  cdd_trivia_t *tail = NULL;
  if (!out_trivia)
    return EINVAL;
  *out_trivia = NULL;

  while (head) {
    cdd_trivia_t *tr = (cdd_trivia_t *)calloc(1, sizeof(cdd_trivia_t));
    if (!tr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return ENOMEM;
    }
    tr->kind = head->kind;
    tr->start = head->start;
    tr->length = head->length;
    if (!new_head)
      new_head = tr;
    else
      tail->next = tr;
    tail = tr;
    head = head->next;
  }
  *out_trivia = new_head;
  return 0;
}

static int clone_token(cdd_token_t *tok, cdd_token_t **out_token) {
  cdd_token_t *ct = NULL;
  if (!tok || !out_token)
    return EINVAL;
  *out_token = NULL;
  ct = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
  if (!ct) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }
  ct->kind = tok->kind;
  ct->start = tok->start;
  ct->length = tok->length;
  clone_trivia(tok->leading_trivia, &ct->leading_trivia);
  clone_trivia(tok->trailing_trivia, &ct->trailing_trivia);
  *out_token = ct;
  return 0;
}

static int emit_ast_bld(expr_t *node, cdd_cst_builder_t *bld, int is_msc);

static int emit_ast_bld_strip(expr_t *node, cdd_cst_builder_t *bld,
                              int is_msc) {
  int rc = 0;
  size_t old_num_children =
      bld->target_node ? bld->target_node->num_children : 0;
  rc = emit_ast_bld(node, bld, is_msc);
  if (bld->target_node && bld->target_node->num_children > old_num_children) {
    if (bld->target_node->children[old_num_children].kind ==
        CDD_CST_CHILD_TOKEN) {
      cdd_trivia_t *tr = bld->target_node->children[old_num_children]
                             .val.token->leading_trivia;
      while (tr) {
        cdd_trivia_t *next = tr->next;
        free(tr);
        tr = next;
      }
      bld->target_node->children[old_num_children].val.token->leading_trivia =
          NULL;
    }
  }
  return rc;
}

static int emit_ast_bld_strip_ampersand(expr_t *node, cdd_cst_builder_t *bld,
                                        int is_msc) {
  if (node && node->type == 0 && node->tok && node->tok->length == 1 &&
      node->tok->start[0] == '&') {
    return emit_ast_bld_strip(node->next, bld, is_msc);
  }
  return emit_ast_bld_strip(node, bld, is_msc);
}

static int emit_ast_bld(expr_t *node, cdd_cst_builder_t *bld, int is_msc) {
  int changes = 0;
  size_t k;
  emit_ctx_t *ctx = is_msc ? g_msc_ctx : NULL;
  cdd_cst_tree_t *tree = bld->tree;

  while (node) {
    if (node->type == 0 || (node->type == 4 && !is_msc)) {
      cdd_token_t *ct = NULL;
      clone_token(node->tok, &ct);
      if (ct)
        cdd_cst_append_child_token(bld->target_node, ct);
    } else if (node->type == 2) {
      cdd_token_t *ct = NULL;
      clone_token(node->tok, &ct);
      if (ct)
        cdd_cst_append_child_token(bld->target_node, ct);
      changes += emit_ast_bld(node->args[0], bld, is_msc);
      if (node->close_tok) {
        cdd_token_t *ct_close = NULL;
        clone_token(node->close_tok, &ct_close);
        if (ct_close)
          cdd_cst_append_child_token(bld->target_node, ct_close);
      }
    } else if (node->type == 1 || (node->type == 5 && !is_msc)) {
      char name[128] = {0};
      size_t nlen = node->tok->length < 127 ? node->tok->length : 127;
      int is_safe = 0;

      memcpy(name, node->tok->start, nlen);

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
        else if (strcmp(name, "strerror") == 0 ||
                 strcmp(name, "_strerror") == 0)
          is_safe = 27;
        else if (strcmp(name, "_wcserror") == 0)
          is_safe = 26;
        else if (strcmp(name, "_ecvt") == 0 || strcmp(name, "_fcvt") == 0)
          is_safe = 28;
        else if (strcmp(name, "strtok") == 0 || strcmp(name, "wcstok") == 0 ||
                 strcmp(name, "_mbstok") == 0)
          is_safe = 29;
        else if (strcmp(name, "qsort") == 0 || strcmp(name, "bsearch") == 0)
          is_safe = 25;
      }

      if (is_safe > 0)
        changes++;

      if (is_safe && is_safe != 18 && is_safe != 19 && is_safe != 20 &&
          is_safe != 21 && is_safe != 22 && is_safe != 23 && is_safe != 24 &&
          is_safe != 25 && is_safe != 26 && is_safe != 27 && is_safe != 28 &&
          is_safe != 29) {
        char safe_name[256];
        cdd_token_t *ct = NULL;
        if (strcmp(name, "strlen") == 0) {
          strcpy(safe_name, "strnlen_s");
        } else {
          sprintf(safe_name, "%s_s", name);
        }

        clone_token(node->tok, &ct);
        if (ct) {
          char *pooled = (char *)malloc(strlen(safe_name) + 1);
          strcpy(pooled, safe_name);
          if (tree->num_strings >= tree->string_capacity) {
            tree->string_capacity =
                tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
            tree->string_pool = (char **)realloc(
                tree->string_pool, tree->string_capacity * sizeof(char *));
          }
          tree->string_pool[tree->num_strings++] = pooled;
          ct->start = (const uint8_t *)pooled;
          ct->length = strlen(pooled);
          cdd_cst_append_child_token(bld->target_node, ct);
        }
        cdd_cst_bld_punct(bld, "(");
      } else if (!is_safe) {
        cdd_token_t *ct = NULL;
        clone_token(node->tok, &ct);
        if (ct)
          cdd_cst_append_child_token(bld->target_node, ct);
        cdd_cst_bld_punct(bld, "(");
      } else {
        cdd_token_t *ct = NULL;
        clone_token(node->tok, &ct);
        if (ct) {
          ct->length = 0;
          ct->start = (const uint8_t *)"";
          cdd_cst_append_child_token(bld->target_node, ct);
        }
      }

      if (is_safe == 1 && node->num_args >= 2) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "sizeof");
        cdd_cst_bld_punct(bld, "(");
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
        for (k = 1; k < node->num_args; k++) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
      } else if (is_safe == 2 && node->num_args >= 3) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "sizeof");
        cdd_cst_bld_punct(bld, "(");
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "_TRUNCATE");
      } else if (is_safe == 3 && node->num_args >= 3) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "_TRUNCATE");
        for (k = 2; k < node->num_args; k++) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
      } else if (is_safe == 6 && node->num_args >= 3) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "sizeof");
        cdd_cst_bld_punct(bld, "(");
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[2], bld, is_msc);
      } else if (is_safe == 8 && node->num_args >= 3) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "sizeof");
        cdd_cst_bld_punct(bld, "(");
        emit_ast_bld_strip(node->args[1], bld, 0);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[2], bld, is_msc);
      } else if (is_safe == 9 && node->num_args == 1) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "sizeof");
        cdd_cst_bld_punct(bld, "(");
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
      } else if (is_safe == 10 && node->num_args >= 5) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        for (k = 1; k < 5; k++) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[k], bld, is_msc);
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          if (expr_is_null_or_zero(node->args[k])) {
            cdd_cst_bld_int(bld, 0);
          } else {
            cdd_cst_bld_ident(bld, "sizeof");
            cdd_cst_bld_punct(bld, "(");
            emit_ast_bld_strip(node->args[k], bld, 0);
            cdd_cst_bld_punct(bld, ")");
          }
        }
      } else if (is_safe == 11 && node->num_args >= 5) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "sizeof");
        cdd_cst_bld_punct(bld, "(");
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
        for (k = 1; k < node->num_args; k++) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
      } else if (is_safe == 19 && node->num_args == 3) {
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, "_gcvt_s");
        cdd_cst_bld_punct(bld, "(");
        changes += emit_ast_bld(node->args[2], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "sizeof");
        cdd_cst_bld_punct(bld, "(");
        emit_ast_bld_strip(node->args[2], bld, 0);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_ast_bld_strip(node->args[2], bld, 0);
        cdd_cst_bld_punct(bld, ")");
      } else if (is_safe == 20 && node->num_args == 3) {
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, pool_string_safe(bld->tree, name));
        cdd_cst_bld_ident(bld, "_s");
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, "NULL");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, "sizeof");
        cdd_cst_bld_punct(bld, "(");
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
        if (strcmp(name, "wcstombs") == 0) {
          cdd_cst_bld_punct(bld, ")");
        } else {
          cdd_cst_bld_punct(bld, ")");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_punct(bld, "/");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "sizeof");
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_ident(bld, "wchar_t");
          cdd_cst_bld_punct(bld, ")");
        }
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[2], bld, is_msc);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
      } else if (is_safe == 21 && node->num_args == 2) {
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, "wctomb_s");
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, "NULL");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "sizeof");
        cdd_cst_bld_punct(bld, "(");
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
      } else if (is_safe == 22 && node->num_args == 3) {
        cdd_cst_bld_ident(bld, pool_string_safe(bld->tree, name));
        cdd_cst_bld_ident(bld, "_s");
        cdd_cst_bld_punct(bld, "(");
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[2], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "sizeof");
        cdd_cst_bld_punct(bld, "(");
        emit_ast_bld_strip(node->args[2], bld, 0);
        cdd_cst_bld_punct(bld, ")");
        if (strcmp(name, "_wsearchenv") == 0) {
          cdd_cst_bld_space(bld);
          cdd_cst_bld_punct(bld, "/");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "sizeof");
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_ident(bld, "wchar_t");
          cdd_cst_bld_punct(bld, ")");
        }
      } else if (is_safe == 23 && node->num_args == 1) {
        if (strcmp(name, "_wgetenv") == 0) {
          if (ctx)
            ctx->needs_wgetenv_ptr = 1;
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_ident(bld, "_wdupenv_s");
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_punct(bld, "&");
          cdd_cst_bld_ident(bld, "__wgetenv_ptr");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "NULL");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[0], bld, is_msc);
          cdd_cst_bld_punct(bld, ")");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "__wgetenv_ptr");
          cdd_cst_bld_punct(bld, ")");
        } else {
          if (ctx)
            ctx->needs_getenv_ptr = 1;
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_ident(bld, "_dupenv_s");
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_punct(bld, "&");
          cdd_cst_bld_ident(bld, "__getenv_ptr");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "NULL");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[0], bld, is_msc);
          cdd_cst_bld_punct(bld, ")");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "__getenv_ptr");
          cdd_cst_bld_punct(bld, ")");
        }
      } else if (is_safe == 24 && node->num_args == 1) {
        /* _putenv. We need to split the "A=B" string if it is a literal */
        int split = 0;
        expr_t *str_node = node->args[0];
        if (str_node && str_node->type == 0 && str_node->tok &&
            str_node->tok->kind == CDD_TOKEN_IDENTIFIER &&
            str_node->tok->length == 1 && str_node->tok->start[0] == 'L') {
          str_node = str_node->next;
        }
        if (str_node && str_node->type == 0 && str_node->tok &&
            str_node->tok->kind == CDD_TOKEN_STRING) {
          const char *s = (const char *)str_node->tok->start;
          size_t slen = str_node->tok->length;
          char *eq = memchr(s, '=', slen);
          if (eq && (s[0] == '"' || (s[0] == 'L' && s[1] == '"')) && eq > s) {
            split = 1;
            cdd_cst_bld_ident(bld, strcmp(name, "_wputenv") == 0 ? "_wputenv_s"
                                                                 : "_putenv_s");
            cdd_cst_bld_punct(bld, "(");
            {
              char left[256] = {0};
              char right[256] = {0};
              size_t ll = (size_t)(eq - s);
              size_t rl = slen - (size_t)(eq - s) - 1;
              if (strcmp(name, "_wputenv") == 0) {
                left[0] = 'L';
                left[1] = '"';
                memcpy(left + 2, s + 1, ll - 1);
                left[ll + 1] = '"';
                right[0] = 'L';
                right[1] = '"';
                memcpy(right + 2, eq + 1, rl);
              } else {
                memcpy(left, s, ll);
                left[ll] = '"';
                right[0] = '"';
                memcpy(right + 1, eq + 1, rl);
              }
              cdd_cst_bld_token(bld, CDD_TOKEN_STRING,
                                pool_string_safe(bld->tree, left));
              cdd_cst_bld_punct(bld, ",");
              cdd_cst_bld_space(bld);
              cdd_cst_bld_token(bld, CDD_TOKEN_STRING,
                                pool_string_safe(bld->tree, right));
            }
          }
        }
        if (!split) {
          cdd_cst_bld_ident(bld, pool_string_safe(bld->tree, name));
          cdd_cst_bld_punct(bld, "(");
          changes += emit_ast_bld(node->args[0], bld, is_msc);
        }
      } else if (is_safe == 25 &&
                 (node->num_args == 4 || node->num_args == 5)) {
        cdd_cst_bld_ident(bld, pool_string_safe(bld->tree, name));
        cdd_cst_bld_ident(bld, "_s");
        cdd_cst_bld_punct(bld, "(");
        for (k = 0; k < node->num_args; k++) {
          if (k > 0) {
            cdd_cst_bld_punct(bld, ",");
            cdd_cst_bld_space(bld);
          }
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "NULL");
      } else if (is_safe == 29) {
        cdd_cst_bld_ident(bld, pool_string_safe(bld->tree, name));
        cdd_cst_bld_ident(bld, "_s");
        cdd_cst_bld_punct(bld, "(");
        for (k = 0; k < node->num_args; k++) {
          if (k > 0) {
            cdd_cst_bld_punct(bld, ",");
            cdd_cst_bld_space(bld);
          }
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_punct(bld, "&");
        if (strcmp(name, "strtok") == 0)
          cdd_cst_bld_ident(bld, "__strtokctx");
        else if (strcmp(name, "wcstok") == 0)
          cdd_cst_bld_ident(bld, "__wcstokctx");
        else
          cdd_cst_bld_ident(bld, "__mbstokctx");
      } else if (is_safe == 26 || is_safe == 27 || is_safe == 28) {
        const char *bufname =
            is_safe == 26
                ? "__wcserrbuf"
                : (is_safe == 27 ? "__errbuf"
                                 : (strcmp(name, "_ecvt") == 0 ? "__ecvtbuf"
                                                               : "__fcvtbuf"));
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, pool_string_safe(bld->tree, name));
        cdd_cst_bld_ident(bld, "_s");
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, pool_string_safe(bld->tree, bufname));
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, is_safe == 28 ? "128" : "94");
        for (k = 0; k < node->num_args; k++) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, pool_string_safe(bld->tree, bufname));
        cdd_cst_bld_punct(bld, ")");
      } else if (is_safe == 7) {
        size_t format_idx = 0;
        if (strcmp(name, "fscanf") == 0 || strcmp(name, "sscanf") == 0 ||
            strcmp(name, "vfscanf") == 0 || strcmp(name, "vsscanf") == 0) {
          format_idx = 1;
        }

        if (format_idx < node->num_args) {
          int needs_size[32] = {0};
          int num_specifiers = 0;

          if (node->args[format_idx] && node->args[format_idx]->type == 0 &&
              node->args[format_idx]->tok &&
              node->args[format_idx]->tok->kind == CDD_TOKEN_STRING) {
            const char *start =
                (const char *)node->args[format_idx]->tok->start;
            const char *end = start + node->args[format_idx]->tok->length;
            while (start < end) {
              if (*start == '%') {
                start++;
                if (start < end && *start == '%') {
                  start++;
                  continue;
                }
                if (start < end && *start == '*') {
                  start++;
                  while (start < end && !isalpha((unsigned char)*start) &&
                         *start != '[')
                    start++;
                  if (start < end)
                    start++;
                  continue;
                }
                while (start < end && !isalpha((unsigned char)*start) &&
                       *start != '[')
                  start++;
                if (start < end) {
                  if (*start == 's' || *start == 'S' || *start == 'c' ||
                      *start == 'C' || *start == '[') {
                    if (num_specifiers < 32)
                      needs_size[num_specifiers] = 1;
                  }
                  num_specifiers++;
                  if (*start == '[') {
                    start++;
                    if (start < end && *start == '^')
                      start++;
                    if (start < end && *start == ']')
                      start++;
                    while (start < end && *start != ']')
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
            if (k > 0) {
              cdd_cst_bld_punct(bld, ",");
              cdd_cst_bld_space(bld);
            }
            changes += emit_ast_bld(node->args[k], bld, is_msc);

            if (k > format_idx) {
              int arg_idx = (int)(k - (format_idx + 1));
              if (arg_idx < num_specifiers && needs_size[arg_idx]) {
                cdd_cst_bld_punct(bld, ",");
                cdd_cst_bld_space(bld);
                cdd_cst_bld_punct(bld, "(");
                cdd_cst_bld_ident(bld, "unsigned");
                cdd_cst_bld_punct(bld, ")");
                cdd_cst_bld_ident(bld, "sizeof");
                cdd_cst_bld_punct(bld, "(");
                emit_ast_bld_strip_ampersand(node->args[k], bld, 0);
                cdd_cst_bld_punct(bld, ")");
              }
            }
          }
        } else {
          if (node->num_args > 0) {
            changes += emit_ast_bld(node->args[0], bld, is_msc);
          }
        }
      } else {
        for (k = 0; k < node->num_args; k++) {
          if (k > 0) {
            cdd_cst_bld_punct(bld, ",");
            cdd_cst_bld_space(bld);
          }
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
      }

      if (node->close_tok) {
        cdd_token_t *ct_close = NULL;
        clone_token(node->close_tok, &ct_close);
        if (ct_close)
          cdd_cst_append_child_token(bld->target_node, ct_close);
      } else {
        cdd_cst_bld_punct(bld, ")");
      }
    } else if (node->type == 3) {
      changes++;
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
            if (token_count == 0 && t_iter->tok->kind == CDD_TOKEN_STAR)
              first_is_star = 1;
            if (t_iter->tok->kind == CDD_TOKEN_DOT ||
                t_iter->tok->kind == CDD_TOKEN_ARROW ||
                t_iter->tok->kind == CDD_TOKEN_LBRACKET)
              has_dot_arrow_bracket = 1;
          }
          token_count++;
          last_t = t_iter;
          t_iter = t_iter->next;
        }

        if (token_count > 1 && !has_dot_arrow_bracket && !first_is_star)
          is_decl = 1;

        if (is_decl) {
          char safe_call[32];
          size_t nlen = call->tok->length < 15 ? call->tok->length : 15;
          char call_name[16] = {0};
          expr_t *t = lhs;
          memcpy(call_name, call->tok->start, nlen);
          sprintf(safe_call, "%s_s", call_name);
          while (t && t != node) {
            cdd_token_t *ct = NULL;
            clone_token(t->tok, &ct);
            if (ct)
              cdd_cst_append_child_token(bld->target_node, ct);
            t = t->next;
          }
          cdd_cst_bld_punct(bld, ";");
          cdd_cst_bld_newline(bld);
          cdd_cst_bld_ident(bld, pool_string_safe(bld->tree, safe_call));
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_punct(bld, "&");
          {
            cdd_token_t *ct = NULL;
            clone_token(last_t->tok, &ct);
            if (ct) {
              ct->leading_trivia = NULL;
              ct->trailing_trivia = NULL;
              cdd_cst_append_child_token(bld->target_node, ct);
            }
          }
        } else {
          char safe_call[32];
          size_t nlen = call->tok->length < 15 ? call->tok->length : 15;
          char call_name[16] = {0};
          expr_t *t = lhs;
          memcpy(call_name, call->tok->start, nlen);
          sprintf(safe_call, "%s_s", call_name);

          if (node->tok->leading_trivia) {
            cdd_token_t *ct_space =
                (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
            ct_space->kind = CDD_TOKEN_OTHER;
            ct_space->start = (const uint8_t *)"";
            clone_trivia(node->tok->leading_trivia, &ct_space->leading_trivia);
            cdd_cst_append_child_token(bld->target_node, ct_space);
          }
          cdd_cst_bld_ident(bld, pool_string_safe(bld->tree, safe_call));
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_punct(bld, "&");
          while (t && t != node) {
            cdd_token_t *ct = NULL;
            clone_token(t->tok, &ct);
            if (ct)
              cdd_cst_append_child_token(bld->target_node, ct);
            t = t->next;
          }
        }
        if (call->num_args >= 1) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          emit_ast_bld(call->args[0], bld, is_msc);
        }
        if (call->num_args >= 2) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          emit_ast_bld(call->args[1], bld, is_msc);
        }
        if (call->num_args >= 3) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          emit_ast_bld(call->args[2], bld, is_msc);
        }
        cdd_cst_bld_punct(bld, ")");
        if (node->tok->trailing_trivia) {
          cdd_token_t *ct_space = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
          ct_space->kind = CDD_TOKEN_OTHER;
          ct_space->start = (const uint8_t *)"";
          clone_trivia(node->tok->trailing_trivia, &ct_space->trailing_trivia);
          cdd_cst_append_child_token(bld->target_node, ct_space);
        }
      } else {
        if (node->tok->leading_trivia) {
          cdd_token_t *ct_space = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
          ct_space->kind = CDD_TOKEN_OTHER;
          ct_space->start = (const uint8_t *)"";
          clone_trivia(node->tok->leading_trivia, &ct_space->leading_trivia);
          cdd_cst_append_child_token(bld->target_node, ct_space);
        }
        cdd_cst_bld_punct(bld, "=");
        if (node->tok->trailing_trivia) {
          cdd_token_t *ct_space = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
          ct_space->kind = CDD_TOKEN_OTHER;
          ct_space->start = (const uint8_t *)"";
          clone_trivia(node->tok->trailing_trivia, &ct_space->trailing_trivia);
          cdd_cst_append_child_token(bld->target_node, ct_space);
        }
      }
    }
    node = node->next;
  }
  return changes;
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
      /* printf("PROCESSING STATEMENT: %.*s\n", (int)first_tok->length,
             first_tok->start); */
      parse_expr_ast(stmt, &idx, 0, &ast);
      find_and_mark_fopen(ast);
      check_unsupported_calls(ast);

      if (check_needs_transform(ast)) {
        char indent[64];
        cdd_cst_builder_t msc_bld, else_bld, bld;
        cdd_cst_node_t *msc_node = NULL, *else_node = NULL, *new_node = NULL;
        int msc_changes = 0;

        cdd_trivia_t *saved_trivia =
            first_tok ? first_tok->leading_trivia : NULL;
        emit_ctx_t msc_ctx = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        get_indent_string(first_tok, indent);

        if (first_tok)
          first_tok->leading_trivia = NULL;

        cdd_cst_alloc_node(CDD_CST_PREPROC_CONDITIONAL, &new_node);
        if (!new_node)
          break;
        cdd_cst_builder_init(&bld, tree, new_node);
        cdd_cst_bld_newline(&bld);
        cdd_cst_bld_ifdef(&bld, "_MSC_VER");
        cdd_cst_bld_block_comment(&bld, "CDD_SAFE_CRT");
        cdd_cst_bld_space(&bld);

        cdd_cst_alloc_node(CDD_CST_UNKNOWN, &msc_node);
        cdd_cst_builder_init(&msc_bld, tree, msc_node);
        g_msc_ctx = &msc_ctx;
        msc_changes = emit_ast_bld(ast, &msc_bld, 1);
        g_msc_ctx = NULL;

        cdd_cst_alloc_node(CDD_CST_UNKNOWN, &else_node);
        cdd_cst_builder_init(&else_bld, tree, else_node);
        emit_ast_bld(ast, &else_bld, 0);

        if (first_tok)
          first_tok->leading_trivia = saved_trivia;

        if (msc_changes > 0) {
          size_t _ci;
          if (msc_ctx.needs_errbuf || msc_ctx.needs_wcserrbuf ||
              msc_ctx.needs_strtokctx || msc_ctx.needs_wcstokctx ||
              msc_ctx.needs_mbstokctx || msc_ctx.needs_ecvtbuf ||
              msc_ctx.needs_fcvtbuf || msc_ctx.needs_retbuf ||
              msc_ctx.needs_getenv_ptr || msc_ctx.needs_wgetenv_ptr) {
            cdd_cst_bld_punct(&bld, "{");
            cdd_cst_bld_newline(&bld);
            if (msc_ctx.needs_errbuf) {
              if (indent[0] != '\0')
                cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                  pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "__errbuf");
              cdd_cst_bld_punct(&bld, "[");
              cdd_cst_bld_int(&bld, 94);
              cdd_cst_bld_punct(&bld, "]");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_wcserrbuf) {
              if (indent[0] != '\0')
                cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                  pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "wchar_t");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "__wcserrbuf");
              cdd_cst_bld_punct(&bld, "[");
              cdd_cst_bld_int(&bld, 94);
              cdd_cst_bld_punct(&bld, "]");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_strtokctx) {
              if (indent[0] != '\0')
                cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                  pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "*");
              cdd_cst_bld_ident(&bld, "__strtokctx");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "=");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "NULL");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_wcstokctx) {
              if (indent[0] != '\0')
                cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                  pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "wchar_t");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "*");
              cdd_cst_bld_ident(&bld, "__wcstokctx");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "=");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "NULL");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_mbstokctx) {
              if (indent[0] != '\0')
                cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                  pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "unsigned");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "*");
              cdd_cst_bld_ident(&bld, "__mbstokctx");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "=");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "NULL");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_ecvtbuf) {
              if (indent[0] != '\0')
                cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                  pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "__ecvtbuf");
              cdd_cst_bld_punct(&bld, "[");
              cdd_cst_bld_int(&bld, 128);
              cdd_cst_bld_punct(&bld, "]");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_fcvtbuf) {
              if (indent[0] != '\0')
                cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                  pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "__fcvtbuf");
              cdd_cst_bld_punct(&bld, "[");
              cdd_cst_bld_int(&bld, 128);
              cdd_cst_bld_punct(&bld, "]");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_retbuf) {
              if (indent[0] != '\0')
                cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                  pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "size_t");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "__ret");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_getenv_ptr) {
              if (indent[0] != '\0')
                cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                  pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "*");
              cdd_cst_bld_ident(&bld, "__getenv_ptr");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_wgetenv_ptr) {
              if (indent[0] != '\0')
                cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                  pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "wchar_t");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "*");
              cdd_cst_bld_ident(&bld, "__wgetenv_ptr");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (indent[0] != '\0')
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                pool_string_safe(tree, indent));
            cdd_cst_bld_space(&bld);
          }

          for (_ci = 0; _ci < msc_node->num_children; _ci++) {
            if (msc_node->children[_ci].kind == CDD_CST_CHILD_TOKEN)
              cdd_cst_append_child_token(bld.target_node,
                                         msc_node->children[_ci].val.token);
            else
              cdd_cst_append_child_node(bld.target_node,
                                        msc_node->children[_ci].val.node);
          }

          if (msc_ctx.needs_errbuf || msc_ctx.needs_wcserrbuf ||
              msc_ctx.needs_strtokctx || msc_ctx.needs_wcstokctx ||
              msc_ctx.needs_mbstokctx || msc_ctx.needs_ecvtbuf ||
              msc_ctx.needs_fcvtbuf || msc_ctx.needs_retbuf ||
              msc_ctx.needs_getenv_ptr || msc_ctx.needs_wgetenv_ptr) {
            cdd_cst_bld_newline(&bld);
            if (indent[0] != '\0')
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                pool_string_safe(tree, indent));
            cdd_cst_bld_punct(&bld, "}");
          }
          cdd_cst_bld_newline(&bld);
          cdd_cst_bld_else(&bld);
          cdd_cst_bld_block_comment(&bld, "CDD_SAFE_CRT");
          cdd_cst_bld_space(&bld);

          for (_ci = 0; _ci < else_node->num_children; _ci++) {
            if (else_node->children[_ci].kind == CDD_CST_CHILD_TOKEN)
              cdd_cst_append_child_token(bld.target_node,
                                         else_node->children[_ci].val.token);
            else
              cdd_cst_append_child_node(bld.target_node,
                                        else_node->children[_ci].val.node);
          }
          cdd_cst_bld_newline(&bld);
          cdd_cst_bld_endif(&bld);

#ifdef CDD_BUILD_TESTS
          if (g_safe_crt_malloc_fail == 6)
            bld.error_state = 1;
#endif
          if (!cdd_cst_builder_has_error(&bld)) {
            cdd_cst_replace_node(tree, stmt, new_node);
            replaced_any = 1;
          } else {
            cdd_cst_free_node_only(new_node);
          }
        } else {
          cdd_cst_free_node_only(new_node);
        }

        cdd_cst_builder_free(&msc_bld);
        cdd_cst_builder_free(&else_bld);
        cdd_cst_builder_free(&bld);
        cdd_cst_free_node_only(msc_node);
        cdd_cst_free_node_only(else_node);

        if (replaced_any) {
          break;
        }
      }
    }

    free(res.nodes);
  } while (replaced_any);

  return 0;
}
