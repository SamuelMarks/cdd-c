/**
 * @file cdd_cst_builder_trivia.c
 * @brief Implementation of CST builder trivia and comment routines.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_cst_builder.h"
#include "cdd_cst_builder_internal.h"
#include "cdd_cst_factory.h"
#include "cdd_cst_mutate.h"
#include "cdd_lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static cdd_c_error_t get_first_token(cdd_cst_node_t *node,
                                     cdd_token_t **out_tok);

cdd_c_error_t cdd_cst_bld_line_comment(cdd_cst_builder_t *builder,
                                       const char *text) {
  /* Line comments aren't standard C89 but commonly accepted or we can just
     inject a block comment. The prompt specified strictly C89, so we will
     actually map line comments to block comments! */
  {
    cdd_c_error_t rc = cdd_cst_bld_block_comment(builder, text);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
}

static cdd_c_error_t create_trivia(cdd_cst_tree_t *tree, const char *text,
                                   cdd_trivia_t **out_trivia) {
#ifdef CDD_BUILD_TESTS
  extern int g_cdd_cst_alloc_token_fail;
#endif
  cdd_trivia_t *t;
  const char *dup;
  cdd_c_error_t pool_rc;
  if (!tree)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_trivia = NULL;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_cst_alloc_token_fail && --g_cdd_cst_alloc_token_fail == 0)
    t = NULL;
  else
#endif
    t = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));

  if (!t) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  pool_rc = cdd_cst_pool_string(tree, text, (const char **)&dup);
  if (pool_rc != CDD_C_SUCCESS) {
    C_CDD_FREE(t);
    return pool_rc;
  }

  t->kind = TRIVIA_BLOCK_COMMENT;

  t->start = (const uint8_t *)dup;
  t->length = strlen(dup);
  *out_trivia = t;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_cst_bld_block_comment(cdd_cst_builder_t *builder,
                                        const char *text) {
  char buf[1024];
  cdd_trivia_t *trivia = NULL;
  cdd_token_t *target_tok = NULL;
  cdd_c_error_t rc;

  if (!builder || !text)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return (cdd_c_error_t)builder->error_state;

#if defined(_MSC_VER) && _MSC_VER >= 1400
  sprintf_s(buf, sizeof(buf), "/* %s */", text);
#else
  CDD_SNPRINTF(buf, sizeof(buf), "/* %s */", text);
#endif

  rc = create_trivia(builder->tree, buf, &trivia);
  if (rc != CDD_C_SUCCESS) {
    builder->error_state = rc;
    return rc;
  }

  if (builder->target_node->num_children > 0) {
    cdd_cst_child_t *last =
        &builder->target_node->children[builder->target_node->num_children - 1];
    if (last->kind == CDD_CST_CHILD_TOKEN) {
      target_tok = last->val.token;
      /* append as trailing trivia */
      if (target_tok->trailing_trivia) {
        cdd_trivia_t *tail = target_tok->trailing_trivia;
        while (tail->next)
          tail = tail->next;
        tail->next = trivia;
      } else {
        target_tok->trailing_trivia = trivia;
      }
      return CDD_C_SUCCESS;
    }
  }

  /* Fallback: append as an ACTUAL token if there's no token to attach to.
     Or we can synthesize a blank token to hold it. We'll append an OTHER token.
   */
  {
    const char *pooled = NULL;
    cdd_c_error_t pool_rc = cdd_cst_pool_string(builder->tree, buf, &pooled);

    if (pool_rc != CDD_C_SUCCESS) {
      C_CDD_FREE(trivia);
      return pool_rc;
    }
    rc = cdd_cst_bld_token(builder, CDD_TOKEN_OTHER, pooled);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(trivia);
      return rc;
    }
    C_CDD_FREE(
        trivia); /* since it became a real token via string pool mapping */
    return CDD_C_SUCCESS;
  }
}

static cdd_c_error_t get_first_token(cdd_cst_node_t *node,
                                     cdd_token_t **out_tok) {
  size_t i;
  if (!node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_tok = NULL;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      *out_tok = node->children[i].val.token;
      return CDD_C_SUCCESS;
    } else {
      cdd_token_t *t = NULL;
      cdd_c_error_t rc = get_first_token(node->children[i].val.node, &t);
      if (rc != CDD_C_SUCCESS && rc != CDD_C_ERROR_NOT_FOUND)
        return rc;
      if (rc == CDD_C_SUCCESS) {
        *out_tok = t;
        return CDD_C_SUCCESS;
      }
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

cdd_c_error_t cdd_cst_get_last_token(cdd_cst_node_t *node,
                                     cdd_token_t **out_tok) {
  int i;
  if (!node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_tok = NULL;
  for (i = (int)node->num_children - 1; i >= 0; i--) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      *out_tok = node->children[i].val.token;
      return CDD_C_SUCCESS;
    } else {
      cdd_token_t *t = NULL;
      cdd_c_error_t rc = cdd_cst_get_last_token(node->children[i].val.node, &t);
      if (rc != CDD_C_SUCCESS && rc != CDD_C_ERROR_NOT_FOUND)
        return rc;
      if (rc == CDD_C_SUCCESS) {
        *out_tok = t;
        return CDD_C_SUCCESS;
      }
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

cdd_c_error_t cdd_cst_extract_leading_trivia(cdd_cst_node_t *node,
                                             cdd_trivia_t **out_trivia) {
  cdd_token_t *t = NULL;
  cdd_c_error_t rc;
  if (!out_trivia)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_trivia = NULL;
  rc = get_first_token(node, &t);
  if (rc != CDD_C_SUCCESS && rc != CDD_C_ERROR_NOT_FOUND)
    return rc;
  if (t) {
    *out_trivia = t->leading_trivia;
    t->leading_trivia = NULL;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_cst_extract_trailing_trivia(cdd_cst_node_t *node,
                                              cdd_trivia_t **out_trivia) {
  cdd_token_t *t = NULL;
  cdd_c_error_t rc;
  if (!out_trivia)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_trivia = NULL;
  rc = cdd_cst_get_last_token(node, &t);
  if (rc != CDD_C_SUCCESS && rc != CDD_C_ERROR_NOT_FOUND)
    return rc;
  if (t) {
    *out_trivia = t->trailing_trivia;
    t->trailing_trivia = NULL;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_cst_transfer_trivia(cdd_cst_node_t *source_node,
                                      cdd_cst_node_t *target_node) {
  cdd_trivia_t *lead = NULL;
  cdd_trivia_t *trail = NULL;
  cdd_token_t *t_first = NULL;
  cdd_token_t *t_last = NULL;
  cdd_c_error_t rc;
  if (!source_node || !target_node)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_cst_extract_leading_trivia(source_node, &lead);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_cst_extract_trailing_trivia(source_node, &trail);
  if (rc != CDD_C_SUCCESS) {
    if (lead) {
      cdd_trivia_t *cur = lead;
      while (cur) {
        cdd_trivia_t *next = cur->next;
        C_CDD_FREE(cur);
        cur = next;
      }
    }
    return rc;
  }

  rc = get_first_token(target_node, &t_first);
  if (rc != CDD_C_SUCCESS && rc != CDD_C_ERROR_NOT_FOUND) {
    if (lead) {
      cdd_trivia_t *cur = lead;
      while (cur) {
        cdd_trivia_t *next = cur->next;
        C_CDD_FREE(cur);
        cur = next;
      }
    }
    if (trail) {
      cdd_trivia_t *cur = trail;
      while (cur) {
        cdd_trivia_t *next = cur->next;
        C_CDD_FREE(cur);
        cur = next;
      }
    }
    return rc;
  }
  rc = cdd_cst_get_last_token(target_node, &t_last);
  if (rc != CDD_C_SUCCESS && rc != CDD_C_ERROR_NOT_FOUND) {
    if (lead) {
      cdd_trivia_t *cur = lead;
      while (cur) {
        cdd_trivia_t *next = cur->next;
        C_CDD_FREE(cur);
        cur = next;
      }
    }
    if (trail) {
      cdd_trivia_t *cur = trail;
      while (cur) {
        cdd_trivia_t *next = cur->next;
        C_CDD_FREE(cur);
        cur = next;
      }
    }
    return rc;
  }

  if (lead && t_first) {
    cdd_trivia_t *tail = lead;
    while (tail->next)
      tail = tail->next;
    tail->next = t_first->leading_trivia;
    t_first->leading_trivia = lead;
  } else if (lead) {
    cdd_trivia_t *cur = lead;
    while (cur) {
      cdd_trivia_t *next = cur->next;
      C_CDD_FREE(cur);
      cur = next;
    }
  }

  if (trail && t_last) {
    if (t_last->trailing_trivia) {
      cdd_trivia_t *tail = t_last->trailing_trivia;
      while (tail->next)
        tail = tail->next;
      tail->next = trail;
    } else {
      t_last->trailing_trivia = trail;
    }
  } else if (trail) {
    cdd_trivia_t *cur = trail;
    while (cur) {
      cdd_trivia_t *next = cur->next;
      C_CDD_FREE(cur);
      cur = next;
    }
  }

  return CDD_C_SUCCESS;
}

cdd_c_error_t
cdd_cst_replace_node_preserve_trivia(cdd_cst_builder_t *builder,
                                     cdd_cst_node_t *target_node,
                                     cdd_cst_node_t *replacement_node) {
  cdd_c_error_t rc;
  if (!builder || !target_node || !replacement_node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (builder->error_state != 0)
    return (cdd_c_error_t)builder->error_state;

  /* transfer_trivia unbinds trivia from target and moves to replacement */
  rc = cdd_cst_transfer_trivia(target_node, replacement_node);
  if (rc != CDD_C_SUCCESS)
    return rc;

  /* utilize underlying replace mechanism which handles parent array swapping */
  rc = cdd_cst_replace_node(builder->tree, target_node, replacement_node);
  if (rc != CDD_C_SUCCESS) {
    builder->error_state = rc;
  }
  return rc;
}
