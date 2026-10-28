/**
 * @file cdd_cst_emit.c
 * @brief CST emit implementation
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_emit.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Context for emitting CST to string */
typedef struct emit_ctx_t {
  /** @brief Pointer to dynamically allocated buffer */
  char *buf;
  /** @brief Current size of the buffer */
  size_t size;
  /** @brief Capacity of the buffer */
  size_t capacity;
} emit_ctx_t;

/**
 * @brief Appends a string to the emit context buffer.
 * @param[in,out] ctx The emit context.
 * @param[in] str The string to append.
 * @param[in] len The length of the string to append.
 * @return 0 on success, error code otherwise.
 */
static cdd_c_error_t append_str(emit_ctx_t *ctx, const uint8_t *str,
                                size_t len) {
  if (len == 0)
    return CDD_C_SUCCESS;
  if (len > (size_t)-1 - ctx->size - 1) {
    C_CDD_LOG_DEBUG("ENOMEM: Overflow\n");
    return CDD_C_ERROR_MEMORY;
  }
  if (ctx->size + len + 1 > ctx->capacity) {
    char *new_buf;
    size_t new_cap = ctx->capacity == 0 ? 1024 : ctx->capacity * 2;
    while (ctx->size + len + 1 > new_cap) {
      size_t next_cap = new_cap * 2;
      if (next_cap < new_cap) {
        new_cap = ctx->size + len + 1;
        break;
      }
      new_cap = next_cap;
    }
    new_buf = (char *)C_CDD_REALLOC(ctx->buf, new_cap);
    if (!new_buf) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    ctx->buf = new_buf;
    ctx->capacity = new_cap;
  }
  memcpy(ctx->buf + ctx->size, str, len);
  ctx->size += len;
  ctx->buf[ctx->size] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits trivia from a linked list.
 * @param[in,out] ctx The emit context.
 * @param[in,out] t The trivia list.
 * @return 0 on success, error code otherwise.
 */
static cdd_c_error_t emit_trivia(emit_ctx_t *ctx, cdd_trivia_t *t) {
  while (t) {
    cdd_c_error_t rc = append_str(ctx, t->start, t->length);
    if (rc != CDD_C_SUCCESS)
      return rc;
    t = t->next;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a token.
 * @param[in,out] ctx The emit context.
 * @param[in,out] tok The token to emit.
 * @return 0 on success, error code otherwise.
 */
static cdd_c_error_t emit_token(emit_ctx_t *ctx, cdd_token_t *tok) {
  cdd_c_error_t rc;
  if (!tok)
    return CDD_C_SUCCESS;
  if (tok->kind == CDD_TOKEN_EOF) {
    return emit_trivia(
        ctx, tok->leading_trivia); /* EOF token only has leading trivia */
  }

  rc = emit_trivia(ctx, tok->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = append_str(ctx, tok->start, tok->length);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return emit_trivia(ctx, tok->trailing_trivia);
}

/**
 * @brief Emits a CST node recursively.
 * @param[in,out] ctx The emit context.
 * @param[in,out] node The node to emit.
 * @return 0 on success, error code otherwise.
 */
static cdd_c_error_t emit_node(emit_ctx_t *ctx, cdd_cst_node_t *node) {
  size_t i;
  if (!node)
    return CDD_C_SUCCESS;

  for (i = 0; i < node->num_children; i++) {
    cdd_c_error_t rc = CDD_C_SUCCESS;
    cdd_cst_child_t *child = &node->children[i];
    if (child->kind == CDD_CST_CHILD_TOKEN) {
      rc = emit_token(ctx, child->val.token);
    } else if (child->kind == CDD_CST_CHILD_NODE) {
      rc = emit_node(ctx, child->val.node);
    }
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a Concrete Syntax Tree (CST) back into a newly allocated C
 * source string.
 *
 * @param[in] tree Parsed CST tree to serialize.
 * @param[out] out_str Output pointer receiving the newly allocated C source
 * string.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_cst_emit(cdd_cst_tree_t *tree, char **out_str) {
  emit_ctx_t ctx = {0};
  cdd_c_error_t rc;

  if (!tree || !out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = emit_node(&ctx, tree->root);
  if (rc != CDD_C_SUCCESS) {
    if (ctx.buf)
      C_CDD_FREE(ctx.buf);
    return rc;
  }

  if (!ctx.buf) {
    /* Empty file */
    ctx.buf = (char *)C_CDD_MALLOC(1);
    if (!ctx.buf)
      return CDD_C_ERROR_MEMORY;
    ctx.buf[0] = '\0';
  }

  *out_str = ctx.buf;
  return CDD_C_SUCCESS;
}
