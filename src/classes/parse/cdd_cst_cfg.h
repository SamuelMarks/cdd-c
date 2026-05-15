#ifndef CDD_CST_CFG_H
#define CDD_CST_CFG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_cst_node.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents the kind of a basic block.
 */
enum cdd_cst_cfg_block_kind_t {
  CDD_CST_CFG_BLOCK_ENTRY,
  CDD_CST_CFG_BLOCK_NORMAL,
  CDD_CST_CFG_BLOCK_CONDITION,
  CDD_CST_CFG_BLOCK_EXIT
};

typedef struct cdd_cst_cfg_edge_t cdd_cst_cfg_edge_t;
typedef struct cdd_cst_cfg_block_t cdd_cst_cfg_block_t;

/** @brief Struct definition */
struct cdd_cst_cfg_edge_t {
  /** @brief target field */
  cdd_cst_cfg_block_t *target;
  /** @brief condition_value field */
  int is_conditional;
  /** @brief condition_value field */
  int condition_value;      /* 1 for true branch, 0 for false branch */
  cdd_cst_cfg_edge_t *next; /**< next */
};

/** @brief Struct definition */
struct cdd_cst_cfg_block_t {
  /** @brief id field */
  int id;
  /** @brief statements field */
  enum cdd_cst_cfg_block_kind_t kind;
  /** @brief capacity field */
  cdd_cst_node_t **statements;
  /** @brief predecessors field */
  size_t num_statements;
  /** @brief capacity field */
  size_t capacity;                /**< capacity */
  cdd_cst_cfg_edge_t *successors; /**< successors */
  /** @brief entry_block field */
  cdd_cst_cfg_edge_t *predecessors;
  /** @brief blocks field */
};
/** @brief capacity field */

/** @brief Struct definition */
typedef struct cdd_cst_cfg_t cdd_cst_cfg_t;
/** @brief entry_block field */
/** @brief cdd_cst_cfg_t struct */
struct cdd_cst_cfg_t {
  /** @brief num_blocks field */
  cdd_cst_cfg_block_t *entry_block;
  cdd_cst_cfg_block_t *exit_block; /**< exit_block */
  cdd_cst_cfg_block_t **blocks;    /**< blocks */
  size_t num_blocks;               /**< num_blocks */
  size_t capacity;                 /**< capacity */
};

/**
 * @brief Constructs a Control Flow Graph (CFG) from a function definition node.
 * @param function_node The AST node representing the function.
 * @param out_cfg Pointer to store the constructed CFG.
 * @return 0 on success.
 */
C_CDD_EXPORT int cdd_cst_cfg_build(cdd_cst_node_t *function_node,
                                   cdd_cst_cfg_t **out_cfg);

/**
 * @brief Frees a CFG and all its blocks and edges.
 * @param cfg The CFG to free.
 */
C_CDD_EXPORT void cdd_cst_cfg_free(cdd_cst_cfg_t *cfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_CFG_H */
