/**
 * @file cdd_cst_query.c
 * @brief Concrete Syntax Tree (CST) query implementation and utilities.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_query.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Traverses a CST in pre-order, invoking `visitor` on each node before
 * its children.
 *
 * @param[in] root Root CST node to traverse.
 * @param[in] visitor Visitor callback invoked for each node.
 * @param[in,out] user_data Opaque context pointer passed to `visitor`.
 * @return CDD_C_SUCCESS on success, or the first non-zero error returned by
 * `visitor`.
 */
cdd_c_error_t cdd_cst_traverse_preorder(cdd_cst_node_t *root,
                                        cdd_cst_visitor_fn visitor,
                                        void *user_data) {
  size_t i;
  cdd_c_error_t rc;
  if (!root || !visitor)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = visitor(root, user_data);
  if (rc != CDD_C_SUCCESS)
    return rc;

  i = 0;
  while (i < root->num_children) {
    if (root->children[i].kind == CDD_CST_CHILD_NODE) {
      rc = cdd_cst_traverse_preorder(root->children[i].val.node, visitor,
                                     user_data);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    i++;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Traverses a CST in post-order, invoking `visitor` on each node after
 * its children.
 *
 * @param[in] root Root CST node to traverse.
 * @param[in] visitor Visitor callback invoked for each node.
 * @param[in,out] user_data Opaque context pointer passed to `visitor`.
 * @return CDD_C_SUCCESS on success, or the first non-zero error returned by
 * `visitor`.
 */
cdd_c_error_t cdd_cst_traverse_postorder(cdd_cst_node_t *root,
                                         cdd_cst_visitor_fn visitor,
                                         void *user_data) {
  size_t i;
  cdd_c_error_t rc;
  if (!root || !visitor)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < root->num_children; i++) {
    if (root->children[i].kind == CDD_CST_CHILD_NODE) {
      rc = cdd_cst_traverse_postorder(root->children[i].val.node, visitor,
                                      user_data);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return visitor(root, user_data);
}

/**
 * @brief Appends a matching CST node pointer to a query result set.
 *
 * @param[in,out] res Query result container to append to.
 * @param[in] node Matching CST node pointer.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static cdd_c_error_t append_result(cdd_cst_query_result_t *res,
                                   cdd_cst_node_t *node) {
  if (res->size >= res->capacity) {
    size_t new_cap = res->capacity == 0 ? 16 : res->capacity * 2;
    cdd_cst_node_t **new_arr = (cdd_cst_node_t **)C_CDD_REALLOC(
        res->nodes, new_cap * sizeof(cdd_cst_node_t *));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM");
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
  cdd_c_error_t err; /**< err */
} type_query_ctx_t;

/**
 * @brief Visitor callback that collects CST nodes matching `ctx->target_kind`.
 *
 * @param[in] node Current CST node being visited.
 * @param[in,out] user_data Pointer to `type_query_ctx_t`.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t type_visitor(cdd_cst_node_t *node, void *user_data) {
  type_query_ctx_t *ctx = (type_query_ctx_t *)user_data;
  if (node->kind == ctx->target_kind) {
    cdd_c_error_t rc = append_result(ctx->res, node);
    if (rc != CDD_C_SUCCESS) {
      ctx->err = rc;
      return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds all CST nodes of a specific `kind` within a subtree.
 *
 * @param[in] root Root CST node to search.
 * @param[in] kind Target CST node kind to match.
 * @param[out] out_result Output query result container receiving matching
 * nodes.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_find_nodes_by_type(cdd_cst_node_t *root,
                                         enum cdd_cst_node_kind_t kind,
                                         cdd_cst_query_result_t *out_result) {
  type_query_ctx_t ctx;
  if (!out_result)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  out_result->nodes = NULL;
  out_result->size = 0;
  out_result->capacity = 0;

  if (!root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  ctx.target_kind = kind;
  ctx.res = out_result;
  ctx.err = CDD_C_SUCCESS;

  {
    cdd_c_error_t rc = cdd_cst_traverse_preorder(root, type_visitor, &ctx);
    if (rc != CDD_C_SUCCESS) {
      if (out_result->nodes)
        C_CDD_FREE(out_result->nodes);
      out_result->nodes = NULL;
      out_result->size = 0;
      out_result->capacity = 0;
      return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/** @brief call_query_ctx_t struct */
typedef struct call_query_ctx_t {
  /** @brief err field */
  const char *func_name;
  /** @brief func_name_len field */
  size_t func_name_len;
  /** @brief err field */
  cdd_cst_query_result_t *res;
  cdd_c_error_t err; /**< err */
} call_query_ctx_t;

/**
 * @brief Visitor callback that collects function call CST nodes matching
 * `ctx->func_name`.
 *
 * @param[in] node Current CST node being visited.
 * @param[in,out] user_data Pointer to `call_query_ctx_t`.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t call_visitor(cdd_cst_node_t *node, void *user_data) {
  call_query_ctx_t *ctx = (call_query_ctx_t *)user_data;
  int matched = 0;
  if (node->kind == CDD_CST_CALL_EXPR) {
    size_t i;
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->kind == CDD_TOKEN_IDENTIFIER &&
            tok->length == ctx->func_name_len &&
            memcmp(tok->start, ctx->func_name, ctx->func_name_len) == 0) {
          matched = 1;
          break;
        }
      } else {
        cdd_cst_node_t *child = node->children[i].val.node;
        if (child->kind == CDD_CST_IDENTIFIER && child->num_children > 0 &&
            child->children[0].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *tok = child->children[0].val.token;
          if (tok->kind == CDD_TOKEN_IDENTIFIER &&
              tok->length == ctx->func_name_len &&
              memcmp(tok->start, ctx->func_name, ctx->func_name_len) == 0) {
            matched = 1;
            break;
          }
        }
      }
    }
  } else if (node->kind == CDD_CST_UNKNOWN) {
    size_t i;
    for (i = 0; i < node->num_children; i++) {
      if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = node->children[i].val.token;
        if (tok->kind == CDD_TOKEN_IDENTIFIER &&
            tok->length == ctx->func_name_len &&
            memcmp(tok->start, ctx->func_name, ctx->func_name_len) == 0 &&
            i + 1 < node->num_children &&
            node->children[i + 1].kind == CDD_CST_CHILD_TOKEN &&
            node->children[i + 1].val.token->kind == CDD_TOKEN_LPAREN) {
          matched = 1;
          break;
        }
      }
    }
  }
  if (matched) {
    ctx->err = append_result(ctx->res, node);
    return ctx->err;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds all CST nodes representing calls to a function named
 * `func_name`.
 *
 * @param[in] root Root CST node to search.
 * @param[in] func_name Name of the called function to match.
 * @param[out] out_result Output query result container receiving matching call
 * nodes.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t
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
  ctx.err = CDD_C_SUCCESS;

  {
    cdd_c_error_t rc = cdd_cst_traverse_preorder(root, call_visitor, &ctx);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(out_result->nodes);
      out_result->nodes = NULL;
      out_result->size = 0;
      out_result->capacity = 0;
      return rc;
    }
  }

  return CDD_C_SUCCESS;
}
