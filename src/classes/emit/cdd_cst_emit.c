/* clang-format off */
#include "cdd_cst_emit.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct emit_ctx_t {
  char *buf;
  size_t size;
  size_t capacity;
} emit_ctx_t;

static int append_str(emit_ctx_t *ctx, const uint8_t *str, size_t len) {
  if (len == 0)
    return 0;
  if (ctx->size + len + 1 > ctx->capacity) {
    char *new_buf; size_t new_cap = ctx->capacity == 0 ? 1024 : ctx->capacity * 2;
    while (ctx->size + len + 1 > new_cap) {
      new_cap *= 2;
    }
    new_buf = (char *)realloc(ctx->buf, new_cap);
    if (!new_buf)
      return ENOMEM;
    ctx->buf = new_buf;
    ctx->capacity = new_cap;
  }
  memcpy(ctx->buf + ctx->size, str, len);
  ctx->size += len;
  ctx->buf[ctx->size] = '\0';
  return 0;
}

static int emit_trivia(emit_ctx_t *ctx, cdd_trivia_t *t) {
  while (t) {
    int rc = append_str(ctx, t->start, t->length);
    if (rc != 0)
      return rc;
    t = t->next;
  }
  return 0;
}

static int emit_token(emit_ctx_t *ctx, cdd_token_t *tok) {
  int rc;
  if (!tok)
    return 0;
  if (tok->kind == CDD_TOKEN_EOF) {
    return emit_trivia(
        ctx, tok->leading_trivia); /* EOF token only has leading trivia */
  }

  rc = emit_trivia(ctx, tok->leading_trivia);
  if (rc != 0)
    return rc;

  rc = append_str(ctx, tok->start, tok->length);
  if (rc != 0)
    return rc;

  return emit_trivia(ctx, tok->trailing_trivia);
}

static int emit_node(emit_ctx_t *ctx, cdd_cst_node_t *node) {
  size_t i;
  if (!node)
    return 0;

  for (i = 0; i < node->num_children; i++) {
    int rc = 0;
    cdd_cst_child_t *child = &node->children[i];
    if (child->kind == CDD_CST_CHILD_TOKEN) {
      rc = emit_token(ctx, child->val.token);
    } else if (child->kind == CDD_CST_CHILD_NODE) {
      rc = emit_node(ctx, child->val.node);
    }
    if (rc != 0)
      return rc;
  }
  return 0;
}

int cdd_cst_emit(cdd_cst_tree_t *tree, char **out_str) {
  emit_ctx_t ctx = {0};
  int rc;

  if (!tree || !out_str)
    return EINVAL;

  rc = emit_node(&ctx, tree->root);
  if (rc != 0) {
    if (ctx.buf)
      free(ctx.buf);
    return rc;
  }

  if (!ctx.buf) {
    /* Empty file */
    ctx.buf = (char *)malloc(1);
    if (!ctx.buf)
      return ENOMEM;
    ctx.buf[0] = '\0';
  }

  *out_str = ctx.buf;
  return 0;
}




