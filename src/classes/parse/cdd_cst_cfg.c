/**
 * @file cdd_cst_cfg.c
 * @brief Concrete Syntax Tree (CST) cfg implementation and utilities.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_cfg.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include <errno.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @brief Allocates a new basic block and registers it in the CFG.
 *
 * @param[in,out] cfg Target Control Flow Graph.
 * @param[in] kind Kind of basic block (entry, normal, exit).
 * @param[out] out_block Pointer to receive the allocated basic block.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static cdd_c_error_t alloc_block(cdd_cst_cfg_t *cfg,
                                 enum cdd_cst_cfg_block_kind_t kind,
                                 cdd_cst_cfg_block_t **out_block) {
  cdd_cst_cfg_block_t *block =
      (cdd_cst_cfg_block_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_cfg_block_t));
  if (!block) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }

  block->id = (int)cfg->num_blocks;
  block->kind = kind;

  if (cfg->num_blocks >= cfg->capacity) {
    size_t new_cap = cfg->capacity == 0 ? 2 : cfg->capacity * 2;
    cdd_cst_cfg_block_t **new_arr = (cdd_cst_cfg_block_t **)C_CDD_REALLOC(
        cfg->blocks, new_cap * sizeof(cdd_cst_cfg_block_t *));
    if (!new_arr) {
      C_CDD_FREE(block);
      return CDD_C_ERROR_MEMORY;
    }
    cfg->blocks = new_arr;
    cfg->capacity = new_cap;
  }
  cfg->blocks[cfg->num_blocks++] = block;
  *out_block = block;
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds a directed edge (and reverse predecessor edge) between two CFG
 * basic blocks.
 *
 * @param[in,out] from Source basic block.
 * @param[in,out] to Destination basic block.
 * @param[in] is_cond Non-zero if the transition is conditional.
 * @param[in] cond_val Branch condition value (e.g., 1 for true, 0 for false).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static cdd_c_error_t add_edge(cdd_cst_cfg_block_t *from,
                              cdd_cst_cfg_block_t *to, int is_cond,
                              int cond_val) {
  cdd_cst_cfg_edge_t *edge =
      (cdd_cst_cfg_edge_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_cfg_edge_t));
  cdd_cst_cfg_edge_t *back_edge =
      (cdd_cst_cfg_edge_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_cfg_edge_t));

  if (!edge || !back_edge) {
    if (edge)
      C_CDD_FREE(edge);
    if (back_edge)
      C_CDD_FREE(back_edge);
    return CDD_C_ERROR_MEMORY;
  }

  edge->target = to;
  edge->is_conditional = is_cond;
  edge->condition_value = cond_val;
  edge->next = from->successors;
  from->successors = edge;

  back_edge->target = from;
  back_edge->next = to->predecessors;
  to->predecessors = back_edge;

  return CDD_C_SUCCESS;
}

static cdd_c_error_t walk_stmt(cdd_cst_cfg_t *cfg, cdd_cst_node_t *stmt,
                               cdd_cst_cfg_block_t **curr_block_ptr,
                               cdd_cst_cfg_block_t *break_target,
                               cdd_cst_cfg_block_t *continue_target);

/**
 * @brief Appends a statement node to a basic block and links return statements
 * to the exit block.
 *
 * @param[in,out] cfg Target Control Flow Graph.
 * @param[in,out] curr_block Current basic block receiving the statement.
 * @param[in] stmt CST statement node to append.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static cdd_c_error_t build_block(cdd_cst_cfg_t *cfg,
                                 cdd_cst_cfg_block_t *curr_block,
                                 cdd_cst_node_t *stmt) {
  cdd_c_error_t rc;
  size_t i;
  int is_return = 0;

  for (i = 0; i < stmt->num_children; i++) {
    if (stmt->children[i].kind == CDD_CST_CHILD_TOKEN &&
        stmt->children[i].val.token->kind == CDD_TOKEN_KEYWORD_RETURN) {
      is_return = 1;
      break;
    }
  }

  if (curr_block->num_statements >= curr_block->capacity) {
    size_t new_cap = curr_block->capacity == 0 ? 8 : curr_block->capacity * 2;
    cdd_cst_node_t **new_arr = (cdd_cst_node_t **)C_CDD_REALLOC(
        curr_block->statements, new_cap * sizeof(cdd_cst_node_t *));
    if (!new_arr)
      return CDD_C_ERROR_MEMORY;
    curr_block->statements = new_arr;
    curr_block->capacity = new_cap;
  }
  curr_block->statements[curr_block->num_statements++] = stmt;

  if (is_return) {
    rc = add_edge(curr_block, cfg->exit_block, 0, 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Recursively traverses a CST statement or block to construct CFG basic
 * blocks and edges.
 *
 * @param[in,out] cfg Target Control Flow Graph.
 * @param[in] stmt Statement or block CST node to process.
 * @param[in,out] curr_block_ptr Pointer to the active basic block pointer.
 * @param[in] break_target Target block for `break` statements (if any).
 * @param[in] continue_target Target block for `continue` statements (if any).
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t walk_stmt(cdd_cst_cfg_t *cfg, cdd_cst_node_t *stmt,
                               cdd_cst_cfg_block_t **curr_block_ptr,
                               cdd_cst_cfg_block_t *break_target,
                               cdd_cst_cfg_block_t *continue_target) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  cdd_cst_cfg_block_t *curr = *curr_block_ptr;
  if (!curr)
    return CDD_C_SUCCESS;

  if (stmt->kind == CDD_CST_BLOCK) {
    size_t i;
    for (i = 0; i < stmt->num_children; i++) {
      if (stmt->children[i].kind == CDD_CST_CHILD_NODE) {
        rc = walk_stmt(cfg, stmt->children[i].val.node, &curr, break_target,
                       continue_target);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    *curr_block_ptr = curr;
    return CDD_C_SUCCESS;
  }

  if (stmt->kind == CDD_CST_STATEMENT) {
    cdd_cst_cfg_block_t *if_body = NULL, *else_body = NULL, *merge_block = NULL;
    size_t i;
    cdd_cst_node_t *then_stmt = NULL, *else_stmt = NULL;

    for (i = 0; i < stmt->num_children; i++) {
      if (stmt->children[i].kind == CDD_CST_CHILD_NODE) {
        if (!then_stmt)
          then_stmt = stmt->children[i].val.node;
        else if (!else_stmt)
          else_stmt = stmt->children[i].val.node;
      }
    }

    rc = alloc_block(cfg, CDD_CST_CFG_BLOCK_NORMAL, &if_body);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = alloc_block(cfg, CDD_CST_CFG_BLOCK_NORMAL, &merge_block);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (else_stmt) {
      rc = alloc_block(cfg, CDD_CST_CFG_BLOCK_NORMAL, &else_body);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = add_edge(curr, else_body, 1, 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      rc = add_edge(curr, merge_block, 1, 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = add_edge(curr, if_body, 1, 1);
    if (rc != CDD_C_SUCCESS)
      return rc;

    curr = if_body;
    if (then_stmt) {
      rc = walk_stmt(cfg, then_stmt, &curr, break_target, continue_target);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (curr) {
      rc = add_edge(curr, merge_block, 0, 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }

    if (else_stmt) {
      curr = else_body;
      rc = walk_stmt(cfg, else_stmt, &curr, break_target, continue_target);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (curr) {
        rc = add_edge(curr, merge_block, 0, 0);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
    *curr_block_ptr = merge_block;
    return CDD_C_SUCCESS;
  }

  /* Fallback: just append stmt to current block */
  rc = build_block(cfg, curr, stmt);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (curr->successors)
    *curr_block_ptr = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Walks the body of a CST function definition to populate the CFG.
 *
 * @param[in,out] cfg Target Control Flow Graph.
 * @param[in] function_node CST function definition node.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t walk_function_body(cdd_cst_cfg_t *cfg,
                                        cdd_cst_node_t *function_node) {
  cdd_cst_cfg_block_t *current = NULL;
  cdd_c_error_t rc;
  size_t i;

  rc = alloc_block(cfg, CDD_CST_CFG_BLOCK_NORMAL, &current);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = add_edge(cfg->entry_block, current, 0, 0);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < function_node->num_children; i++) {
    if (function_node->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *block_node = function_node->children[i].val.node;
      if (block_node->kind == CDD_CST_BLOCK) {
        rc = walk_stmt(cfg, block_node, &current, NULL, NULL);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  if (current) {
    rc = add_edge(current, cfg->exit_block, 0, 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Constructs a Control Flow Graph (CFG) for a CST function definition
 * node.
 *
 * @param[in] function_node CST function definition node to analyze.
 * @param[out] out_cfg Output pointer receiving the newly constructed CFG.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_cfg_build(cdd_cst_node_t *function_node,
                                cdd_cst_cfg_t **out_cfg) {
  cdd_cst_cfg_t *cfg;
  cdd_c_error_t rc;

  if (!function_node || !out_cfg)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  cfg = (cdd_cst_cfg_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_cfg_t));
  if (!cfg) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }

  rc = alloc_block(cfg, CDD_CST_CFG_BLOCK_ENTRY, &cfg->entry_block);
  if (rc != CDD_C_SUCCESS) {
    cdd_cst_cfg_free(cfg);
    return rc;
  }
  rc = alloc_block(cfg, CDD_CST_CFG_BLOCK_EXIT, &cfg->exit_block);
  if (rc != CDD_C_SUCCESS) {
    cdd_cst_cfg_free(cfg);
    return rc;
  }

  rc = walk_function_body(cfg, function_node);
  if (rc != CDD_C_SUCCESS) {
    cdd_cst_cfg_free(cfg);
    return rc;
  }

  *out_cfg = cfg;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a Control Flow Graph (CFG) and all of its blocks and edges.
 *
 * @param[in,out] cfg Pointer to the CFG to free.
 */
void cdd_cst_cfg_free(cdd_cst_cfg_t *cfg) {
  size_t i;
  if (!cfg)
    return;

  for (i = 0; i < cfg->num_blocks; i++) {
    cdd_cst_cfg_block_t *block = cfg->blocks[i];
    cdd_cst_cfg_edge_t *edge = block->successors;
    while (edge) {
      cdd_cst_cfg_edge_t *next = edge->next;
      C_CDD_FREE(edge);
      edge = next;
    }
    edge = block->predecessors;
    while (edge) {
      cdd_cst_cfg_edge_t *next = edge->next;
      C_CDD_FREE(edge);
      edge = next;
    }
    if (block->statements)
      C_CDD_FREE(block->statements);
    C_CDD_FREE(block);
  }

  if (cfg->blocks)
    C_CDD_FREE(cfg->blocks);
  C_CDD_FREE(cfg);
}
