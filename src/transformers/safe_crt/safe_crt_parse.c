/**
 * @file safe_crt_parse.c
 * @brief AST expression parsing and analysis routines for Safe CRT transformer.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "transformers/safe_crt/safe_crt_internal.h"
#include "classes/parse/cdd_cst_query.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Parses an expression AST from CST statement node tokens.
 *
 * @param[in] stmt CST statement node.
 * @param[in,out] idx Token index within stmt children.
 * @param[in] stop_at_comma Whether to stop parsing when encountering a comma.
 * @param[out] out_expr Pointer to receive parsed expression AST.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t parse_expr_ast(cdd_cst_node_t *stmt, size_t *idx,
                                          int stop_at_comma,
                                          expr_t **out_expr) {
  expr_t *head = NULL;
  expr_t *tail = NULL;
  if (!out_expr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
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
          int tk = (int)stmt->children[*idx].val.token->kind;
          if (tk != CDD_TOKEN_COMMA) {
            (*idx)++;
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

/**
 * @brief Finds and marks fopen assignments in the expression AST.
 *
 * @param[in] head Head of the expression AST list.
 * @param[out] out_found Pointer to receive 1 if fopen assignment found, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t find_and_mark_fopen(expr_t *head, int *out_found) {
  expr_t *curr = head;
  expr_t *lhs_start = head;
  int found = 0;
  size_t i;
  if (!out_found)
    return CDD_C_ERROR_INVALID_ARGUMENT;

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
          while (t != curr) {
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
        int sub_found = 0;
        find_and_mark_fopen(curr->args[i], &sub_found);
        if (sub_found)
          found = 1;
      }
    }

    curr = curr->next;
  }
  *out_found = found;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks for unsupported bare calls.
 *
 * @param[in] head Head of the expression AST list.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t check_unsupported_calls(expr_t *head) {
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
  return CDD_C_SUCCESS;
}

/**
 * @brief Infers buffer size metadata for a given expression node.
 *
 * @param[in] node Target expression node.
 * @return Inferred size metadata structure.
 */
inferred_size_t infer_buffer_size(expr_t *node) {
  inferred_size_t res;
  cdd_cst_query_result_t stmts;
  size_t i, j;
  const char *name = NULL;
  size_t len = 0;

  memset(&res, 0, sizeof(res));
  memset(&stmts, 0, sizeof(stmts));

  if (!current_tree || !current_tree->root || !node)
    return res;

  /* Parse offset patterns */
  if (node->type == 0 && node->tok && node->tok->length == 1 &&
      node->tok->start[0] == '&' && node->next && node->next->type == 0 &&
      node->next->tok->kind == CDD_TOKEN_IDENTIFIER && node->next->next &&
      node->next->next->type == 2 &&
      node->next->next->tok->kind == CDD_TOKEN_LBRACKET) {
    /* &buf[expr] */
    name = (const char *)node->next->tok->start;
    len = node->next->tok->length;
    res.base_tok = node->next->tok;
    res.offset_expr = node->next->next->args[0];
  } else if (node->type == 0 && node->tok &&
             node->tok->kind == CDD_TOKEN_IDENTIFIER && node->next &&
             node->next->type == 0 && node->next->tok->kind == CDD_TOKEN_PLUS &&
             node->next->next) {
    /* buf + expr */
    name = (const char *)node->tok->start;
    len = node->tok->length;
    res.base_tok = node->tok;
    res.offset_expr = node->next->next;
  } else if (node->type == 0 && node->tok &&
             node->tok->kind == CDD_TOKEN_IDENTIFIER && !node->next) {
    /* buf */
    name = (const char *)node->tok->start;
    len = node->tok->length;
    res.base_tok = node->tok;
  }

  if (node->type == 1 && node->tok && node->tok->kind == CDD_TOKEN_IDENTIFIER) {
    if (node->num_args == 1 && node->tok->length == 6 &&
        memcmp(node->tok->start, "malloc", 6) == 0) {
      res.valid = 1;
      res.is_malloc = 1;
      res.malloc_size_expr = node->args[0];
    } else if (node->num_args == 2 && node->tok->length == 6 &&
               memcmp(node->tok->start, "calloc", 6) == 0) {
      res.valid = 1;
      res.is_malloc = 2;
      res.malloc_size_expr = node;
    } else if (node->num_args == 2 && node->tok->length == 7 &&
               memcmp(node->tok->start, "realloc", 7) == 0) {
      res.valid = 1;
      res.is_malloc = 1;
      res.malloc_size_expr = node->args[1];
    }
    return res;
  }

  if (!name)
    return res;

  if (cdd_cst_find_nodes_by_type(current_tree->root, CDD_CST_UNKNOWN, &stmts) !=
      0)
    return res;

  for (i = 0; i < stmts.size; i++) {
    cdd_cst_node_t *stmt = stmts.nodes[i];
    for (j = 0; j < stmt->num_children; j++) {
      if (stmt->children[j].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = stmt->children[j].val.token;
        if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length == len &&
            memcmp(tok->start, name, len) == 0) {

          /* Pattern 1: Array declaration -> char buf[...] */
          if (j + 1 < stmt->num_children && j > 0 &&
              stmt->children[j + 1].kind == CDD_CST_CHILD_TOKEN &&
              stmt->children[j + 1].val.token->kind == CDD_TOKEN_LBRACKET &&
              stmt->children[j - 1].kind == CDD_CST_CHILD_TOKEN &&
              stmt->children[j - 1].val.token->kind == CDD_TOKEN_IDENTIFIER) {
            res.valid = 1;
            free(stmts.nodes);
            return res;
          }

          /* Pattern 2: malloc/calloc/realloc assignment -> buf = malloc(...) OR
           * char *buf = malloc(...) */
          if (j + 2 < stmt->num_children &&
              stmt->children[j + 1].kind == CDD_CST_CHILD_TOKEN &&
              stmt->children[j + 1].val.token->kind == CDD_TOKEN_ASSIGN &&
              stmt->children[j + 2].kind == CDD_CST_CHILD_TOKEN) {
            cdd_token_t *m_tok = stmt->children[j + 2].val.token;
            if (m_tok->kind == CDD_TOKEN_IDENTIFIER && m_tok->length >= 6 &&
                (memcmp(m_tok->start, "malloc", 6) == 0 ||
                 memcmp(m_tok->start, "calloc", 6) == 0 ||
                 memcmp(m_tok->start, "realloc", 7) == 0)) {
              size_t idx = j + 2;
              expr_t *m_expr = NULL;
              parse_expr_ast(stmt, &idx, 0, &m_expr);
              if (m_expr->type == 1) {
                if (m_expr->num_args == 1) {
                  /* malloc */
                  res.valid = 1;
                  res.is_malloc = 1;
                  res.malloc_size_expr = m_expr->args[0];
                } else if (m_expr->num_args == 2 &&
                           memcmp(m_tok->start, "calloc", 6) == 0) {
                  /* calloc(n, size) -> n * size */
                  res.valid = 1;
                  res.is_malloc = 2;
                  res.malloc_size_expr = m_expr;
                } else if (m_expr->num_args == 2 &&
                           memcmp(m_tok->start, "realloc", 7) == 0) {
                  /* realloc(ptr, size) -> size */
                  res.valid = 1;
                  res.is_malloc = 1;
                  res.malloc_size_expr = m_expr->args[1];
                }
              }
            }
          }
        }
      }
      if (res.valid)
        break;
    }
    if (res.valid)
      break;
  }
  free(stmts.nodes);
  return res;
}

/**
 * @brief Checks if an expression represents NULL or numeric 0.
 *
 * @param[in] node Target expression node.
 * @return CDD_C_ERROR_UNKNOWN if NULL or 0, CDD_C_SUCCESS otherwise.
 */
C_CDD_EXPORT cdd_c_error_t expr_is_null_or_zero(expr_t *node) {
  if (node && node->type == 0 && node->tok && !node->next) {
    if (node->tok->length == 4 && memcmp(node->tok->start, "NULL", 4) == 0)
      return CDD_C_ERROR_UNKNOWN;
    if (node->tok->length == 1 && node->tok->start[0] == '0')
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if any call in the expression tree requires safe CRT
 * transformation.
 *
 * @param[in] head Head of the expression AST list.
 * @return CDD_C_SUCCESS if none, CDD_C_ERROR_PARSE if uninferrable size,
 * CDD_C_ERROR_UNKNOWN if transformation needed.
 */
C_CDD_EXPORT cdd_c_error_t check_needs_transform(expr_t *head) {
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
          strcmp(name, "_wmakepath") == 0 || strcmp(name, "_putenv") == 0 ||
          strcmp(name, "_wputenv") == 0 || strcmp(name, "_searchenv") == 0 ||
          strcmp(name, "_wsearchenv") == 0 || strcmp(name, "qsort") == 0 ||
          strcmp(name, "bsearch") == 0 || strcmp(name, "_ultow") == 0 ||
          strcmp(name, "getenv") == 0 || strcmp(name, "_wgetenv") == 0) {

        int need_verified_size = 0;
        int arg_idx = 0;
        if (strcmp(name, "strcpy") == 0 || strcmp(name, "strcat") == 0 ||
            strcmp(name, "strncpy") == 0 || strcmp(name, "strncat") == 0 ||
            strcmp(name, "sprintf") == 0 || strcmp(name, "vsprintf") == 0 ||
            strcmp(name, "memcpy") == 0 || strcmp(name, "memmove") == 0 ||
            strcmp(name, "wcscpy") == 0 || strcmp(name, "wcsncpy") == 0 ||
            strcmp(name, "wcscat") == 0 || strcmp(name, "wcsncat") == 0 ||
            strcmp(name, "_mbscpy") == 0 || strcmp(name, "_mbsncpy") == 0 ||
            strcmp(name, "_mbscat") == 0 || strcmp(name, "_mbsncat") == 0 ||
            strcmp(name, "swprintf") == 0 || strcmp(name, "vswprintf") == 0 ||
            strcmp(name, "wmemcpy") == 0 || strcmp(name, "wmemmove") == 0 ||
            strcmp(name, "_strlwr") == 0 || strcmp(name, "_strupr") == 0 ||
            strcmp(name, "_mbslwr") == 0 || strcmp(name, "_mbsupr") == 0 ||
            strcmp(name, "_wcslwr") == 0 || strcmp(name, "_wcsupr") == 0 ||
            strcmp(name, "gets") == 0 || strcmp(name, "tmpnam") == 0 ||
            strcmp(name, "strlen") == 0 || strcmp(name, "_makepath") == 0 ||
            strcmp(name, "_wmakepath") == 0 || strcmp(name, "mbstowcs") == 0 ||
            strcmp(name, "wcstombs") == 0 || strcmp(name, "wctomb") == 0 ||
            strcmp(name, "_strnset") == 0 || strcmp(name, "_strset") == 0 ||
            strcmp(name, "_mbsset") == 0 || strcmp(name, "_mbsnset") == 0) {
          need_verified_size = 1;
        } else if (strcmp(name, "_itoa") == 0 || strcmp(name, "_ltoa") == 0 ||
                   strcmp(name, "_ultoa") == 0 ||
                   strcmp(name, "_i64toa") == 0 ||
                   strcmp(name, "_ui64toa") == 0 ||
                   strcmp(name, "_itow") == 0 || strcmp(name, "_ltow") == 0 ||
                   strcmp(name, "_ultow") == 0) {
          need_verified_size = 1;
          arg_idx = 1;
        } else if (strcmp(name, "_gcvt") == 0 ||
                   strcmp(name, "_searchenv") == 0 ||
                   strcmp(name, "_wsearchenv") == 0) {
          need_verified_size = 1;
          arg_idx = 2;
        } else if (strcmp(name, "_splitpath") == 0 ||
                   strcmp(name, "_wsplitpath") == 0) {
          need_verified_size = 4;
          arg_idx = 1;
        }

        if (need_verified_size) {
          size_t k;
          for (k = 0; k < (size_t)need_verified_size; k++) {
            size_t a_idx = (size_t)arg_idx + k;
            if (a_idx < head->num_args) {
              expr_t *arg = head->args[a_idx];
              if (expr_is_null_or_zero(arg) != CDD_C_SUCCESS)
                continue;
              if (!infer_buffer_size(arg).valid) {
                return CDD_C_ERROR_PARSE;
              }
            }
          }
        }
        return CDD_C_ERROR_UNKNOWN;
      }
      for (i = 0; i < head->num_args; i++) {
        cdd_c_error_t err = check_needs_transform(head->args[i]);
        if (err == CDD_C_ERROR_PARSE)
          return err;
        if (err)
          return CDD_C_ERROR_UNKNOWN;
      }
    } else if (head->type == 2) {
      cdd_c_error_t err = check_needs_transform(head->args[0]);
      if (err == CDD_C_ERROR_PARSE)
        return err;
      if (err)
        return CDD_C_ERROR_UNKNOWN;
    } else if (head->type == 3) {
      return CDD_C_ERROR_UNKNOWN;
    }
    head = head->next;
  }
  return CDD_C_SUCCESS;
}
