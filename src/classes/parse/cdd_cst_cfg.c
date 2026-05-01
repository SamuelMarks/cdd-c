/* clang-format off */
#include "cdd_cst_cfg.h"
#include <errno.h>
#include <stdlib.h>
#include "c_cdd/log.h"
/* clang-format on */

static int alloc_block(cdd_cst_cfg_t *cfg, enum cdd_cst_cfg_block_kind_t kind,
                       cdd_cst_cfg_block_t **out_block) {
  cdd_cst_cfg_block_t *block;
  if (!cfg || !out_block)
    return EINVAL;
  block = (cdd_cst_cfg_block_t *)calloc(1, sizeof(cdd_cst_cfg_block_t));
  if (!block) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  block->id = (int)cfg->num_blocks;
  block->kind = kind;

  if (cfg->num_blocks >= cfg->capacity) {
    size_t new_cap = cfg->capacity == 0 ? 16 : cfg->capacity * 2;
    cdd_cst_cfg_block_t **new_arr = (cdd_cst_cfg_block_t **)realloc(
        cfg->blocks, new_cap * sizeof(cdd_cst_cfg_block_t *));
    if (!new_arr) {
      free(block);
      return ENOMEM;
    }
    cfg->blocks = new_arr;
    cfg->capacity = new_cap;
  }
  cfg->blocks[cfg->num_blocks++] = block;
  *out_block = block;
  return 0;
}

static int add_edge(cdd_cst_cfg_block_t *from, cdd_cst_cfg_block_t *to,
                    int is_cond, int cond_val) {
  cdd_cst_cfg_edge_t *edge =
      (cdd_cst_cfg_edge_t *)calloc(1, sizeof(cdd_cst_cfg_edge_t));
  cdd_cst_cfg_edge_t *back_edge =
      (cdd_cst_cfg_edge_t *)calloc(1, sizeof(cdd_cst_cfg_edge_t));

  if (!edge || !back_edge) {
    if (edge)
      free(edge);
    if (back_edge)
      free(back_edge);
    return ENOMEM;
  }

  edge->target = to;
  edge->is_conditional = is_cond;
  edge->condition_value = cond_val;
  edge->next = from->successors;
  from->successors = edge;

  back_edge->target = from;
  back_edge->next = to->predecessors;
  to->predecessors = back_edge;

  return 0;
}

static int build_block(cdd_cst_cfg_t *cfg, cdd_cst_cfg_block_t *curr_block,
                       cdd_cst_node_t *stmt) {
  /* This is a skeletal stub that linearly links everything into one block
     and handles simple returns by routing them to the exit block. */
  int rc;
  size_t i;
  int is_return = 0;

  if (!stmt)
    return 0;

  /* Check for return token inside the node */
  for (i = 0; i < stmt->num_children; i++) {
    if (stmt->children[i].kind == CDD_CST_CHILD_TOKEN) {
      if (stmt->children[i].val.token->kind == CDD_TOKEN_KEYWORD_RETURN) {
        is_return = 1;
        break;
      }
    }
  }

  if (curr_block->num_statements >= curr_block->capacity) {
    size_t new_cap = curr_block->capacity == 0 ? 8 : curr_block->capacity * 2;
    cdd_cst_node_t **new_arr = (cdd_cst_node_t **)realloc(
        curr_block->statements, new_cap * sizeof(cdd_cst_node_t *));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return ENOMEM;
    }
    curr_block->statements = new_arr;
    curr_block->capacity = new_cap;
  }

  curr_block->statements[curr_block->num_statements++] = stmt;

  if (is_return) {
    rc = add_edge(curr_block, cfg->exit_block, 0, 0);
    if (rc != 0)
      return rc;
  }

  return 0;
}

static int walk_function_body(cdd_cst_cfg_t *cfg,
                              cdd_cst_node_t *function_node) {
  cdd_cst_cfg_block_t *current;
  size_t i, j;
  int rc;

  alloc_block(cfg, CDD_CST_CFG_BLOCK_NORMAL, &current);
  if (!current) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  rc = add_edge(cfg->entry_block, current, 0, 0);
  if (rc != 0)
    return rc;

  for (i = 0; i < function_node->num_children; i++) {
    if (function_node->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *block_node = function_node->children[i].val.node;
      if (block_node->kind == CDD_CST_BLOCK) {
        for (j = 0; j < block_node->num_children; j++) {
          if (block_node->children[j].kind == CDD_CST_CHILD_NODE) {
            rc = build_block(cfg, current, block_node->children[j].val.node);
            if (rc != 0)
              return rc;
          }
        }
      }
    }
  }

  /* Add edge to exit if current block doesn't unconditionally return */
  if (current->successors == NULL) {
    rc = add_edge(current, cfg->exit_block, 0, 0);
    if (rc != 0)
      return rc;
  }

  return 0;
}

int cdd_cst_cfg_build(cdd_cst_node_t *function_node, cdd_cst_cfg_t **out_cfg) {
  cdd_cst_cfg_t *cfg;
  int rc;

  if (!function_node || !out_cfg)
    return EINVAL;

  cfg = (cdd_cst_cfg_t *)calloc(1, sizeof(cdd_cst_cfg_t));
  if (!cfg) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  alloc_block(cfg, CDD_CST_CFG_BLOCK_ENTRY, &cfg->entry_block);
  alloc_block(cfg, CDD_CST_CFG_BLOCK_EXIT, &cfg->exit_block);

  if (!cfg->entry_block || !cfg->exit_block) {
    cdd_cst_cfg_free(cfg);
    return ENOMEM;
  }

  rc = walk_function_body(cfg, function_node);
  if (rc != 0) {
    cdd_cst_cfg_free(cfg);
    return rc;
  }

  *out_cfg = cfg;
  return 0;
}

void cdd_cst_cfg_free(cdd_cst_cfg_t *cfg) {
  size_t i;
  if (!cfg)
    return;

  for (i = 0; i < cfg->num_blocks; i++) {
    cdd_cst_cfg_block_t *block = cfg->blocks[i];
    if (block) {
      cdd_cst_cfg_edge_t *edge = block->successors;
      while (edge) {
        cdd_cst_cfg_edge_t *next = edge->next;
        free(edge);
        edge = next;
      }
      edge = block->predecessors;
      while (edge) {
        cdd_cst_cfg_edge_t *next = edge->next;
        free(edge);
        edge = next;
      }
      if (block->statements)
        free(block->statements);
      free(block);
    }
  }

  if (cfg->blocks)
    free(cfg->blocks);
  free(cfg);
}
