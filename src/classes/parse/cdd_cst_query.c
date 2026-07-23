/* clang-format off */
#include "cdd_cst_query.h"
#include "c_cdd_export.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_query_err_fail;
C_CDD_EXPORT int g_cdd_query_err_fail = 0;
C_CDD_EXPORT int g_cdd_query_realloc_fail = 0;
#endif
#include "c_cdd/log.h"
/* clang-format on */

enum cdd_c_error cdd_cst_traverse_preorder(cdd_cst_node_t *root,
                                           cdd_cst_visitor_fn visitor,
                                           void *user_data) {
  size_t i;
  int rc;
  if (!root || !visitor)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = visitor(root, user_data);
  if (rc != 0)
    return rc;

  for (i = 0; i < root->num_children; i++) {
    if (root->children[i].kind == CDD_CST_CHILD_NODE) {
      rc = cdd_cst_traverse_preorder(root->children[i].val.node, visitor,
                                     user_data);
      if (rc != 0)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

enum cdd_c_error cdd_cst_traverse_postorder(cdd_cst_node_t *root,
                                            cdd_cst_visitor_fn visitor,
                                            void *user_data) {
  size_t i;
  int rc;
  if (!root || !visitor)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < root->num_children; i++) {
    if (root->children[i].kind == CDD_CST_CHILD_NODE) {
      rc = cdd_cst_traverse_postorder(root->children[i].val.node, visitor,
                                      user_data);
      if (rc != 0)
        return rc;
    }
  }

  return visitor(root, user_data);
}

static enum cdd_c_error append_result(cdd_cst_query_result_t *res,
                                      cdd_cst_node_t *node) {
  if (res->size >= res->capacity) {
    size_t new_cap = res->capacity == 0 ? 16 : res->capacity * 2;
    cdd_cst_node_t **new_arr;
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_query_realloc_fail;
    if (g_cdd_query_realloc_fail)
      new_arr = NULL;
    else
#endif
      new_arr = (cdd_cst_node_t **)realloc(res->nodes,
                                           new_cap * sizeof(cdd_cst_node_t *));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    res->nodes = new_arr;
    res->capacity = new_cap;
  }
  res->nodes[res->size++] = node;
  return CDD_C_SUCCESS;
}

/** @brief type_query_ctx_t struct */
typedef struct type_query_ctx_t {
  /** @brief res field */
  enum cdd_cst_node_kind_t target_kind;
  /** @brief res field */
  cdd_cst_query_result_t *res;
  int err; /**< err */
} type_query_ctx_t;

static enum cdd_c_error type_visitor(cdd_cst_node_t *node, void *user_data) {
  type_query_ctx_t *ctx = (type_query_ctx_t *)user_data;
  if (node->kind == ctx->target_kind) {
    ctx->err = append_result(ctx->res, node);
#ifdef CDD_BUILD_TESTS

    if (g_cdd_query_err_fail)
      ctx->err = CDD_C_ERROR_MEMORY;
#endif
    if (ctx->err != 0)
      return ctx->err;
  }
  return CDD_C_SUCCESS;
}

enum cdd_c_error
cdd_cst_find_nodes_by_type(cdd_cst_node_t *root, enum cdd_cst_node_kind_t kind,
                           cdd_cst_query_result_t *out_result) {
  type_query_ctx_t ctx;
  if (!root || !out_result)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  out_result->nodes = NULL;
  out_result->size = 0;
  out_result->capacity = 0;

  ctx.target_kind = kind;
  ctx.res = out_result;
  ctx.err = 0;

  cdd_cst_traverse_preorder(root, type_visitor, &ctx);

  if (ctx.err != 0) {
    if (out_result->nodes)
      free(out_result->nodes);
    out_result->nodes = NULL;
    out_result->size = 0;
    out_result->capacity = 0;
    return ctx.err;
  }

  return CDD_C_SUCCESS;
}

/** @brief func_name field */
/** @brief call_query_ctx_t struct */
typedef struct call_query_ctx_t {
  /** @brief err field */
  const char *func_name;
  /** @brief func_name_len field */
  size_t func_name_len;
  /** @brief err field */
  cdd_cst_query_result_t *res;
  int err; /**< err */
} call_query_ctx_t;

static enum cdd_c_error call_visitor(cdd_cst_node_t *node, void *user_data) {
  call_query_ctx_t *ctx = (call_query_ctx_t *)user_data;
  if (node->kind == CDD_CST_CALL_EXPR) {
    /* For now, look at children to find an identifier token that matches */
    size_t i;
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->kind == CDD_TOKEN_IDENTIFIER &&
            tok->length == ctx->func_name_len) {
          if (memcmp(tok->start, ctx->func_name, ctx->func_name_len) == 0) {
            ctx->err = append_result(ctx->res, node);
#ifdef CDD_BUILD_TESTS

            if (g_cdd_query_err_fail)
              ctx->err = CDD_C_ERROR_MEMORY;
#endif
            if (ctx->err != 0)
              return ctx->err;
            break; /* found match for this call node */
          }
        }
      } else {
        /* Sometimes the call expr's first child is an IDENTIFIER node */
        cdd_cst_node_t *child = node->children[i].val.node;
        if (child->kind == CDD_CST_IDENTIFIER && child->num_children > 0 &&
            child->children[0].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *tok = child->children[0].val.token;
          if (tok->kind == CDD_TOKEN_IDENTIFIER &&
              tok->length == ctx->func_name_len) {
            if (memcmp(tok->start, ctx->func_name, ctx->func_name_len) == 0) {
              ctx->err = append_result(ctx->res, node);
#ifdef CDD_BUILD_TESTS

              if (g_cdd_query_err_fail)
                ctx->err = CDD_C_ERROR_MEMORY;
#endif
              if (ctx->err != 0)
                return ctx->err;
              break;
            }
          }
        }
      }
    }
  } else if (node->kind == CDD_CST_UNKNOWN) {
    /* Heuristic check for unknown flat structures containing `func_name(` */
    size_t i;
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->kind == CDD_TOKEN_IDENTIFIER &&
            tok->length == ctx->func_name_len) {
          if (memcmp(tok->start, ctx->func_name, ctx->func_name_len) == 0) {
            /* Next token should be LPAREN to be a call */
            if (i + 1 < node->num_children &&
                node->children[i + 1].kind == CDD_CST_CHILD_TOKEN) {
              if (node->children[i + 1].val.token->kind == CDD_TOKEN_LPAREN) {
                ctx->err = append_result(ctx->res, node);
#ifdef CDD_BUILD_TESTS

                if (g_cdd_query_err_fail)
                  ctx->err = CDD_C_ERROR_MEMORY;
#endif
                if (ctx->err != 0)
                  return ctx->err;
                break;
              }
            }
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

enum cdd_c_error
cdd_cst_find_function_calls_named(cdd_cst_node_t *root, const char *func_name,
                                  cdd_cst_query_result_t *out_result) {
  call_query_ctx_t ctx;
  if (!root || !func_name || !out_result)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  out_result->nodes = NULL;
  out_result->size = 0;
  out_result->capacity = 0;

  ctx.func_name = func_name;
  ctx.func_name_len = strlen(func_name);
  ctx.res = out_result;
  ctx.err = 0;

  cdd_cst_traverse_preorder(root, call_visitor, &ctx);

  if (ctx.err != 0) {
    if (out_result->nodes)
      free(out_result->nodes);
    out_result->nodes = NULL;
    out_result->size = 0;
    out_result->capacity = 0;
    return ctx.err;
  }

  return CDD_C_SUCCESS;
}
