#ifndef CDD_CST_QUERY_H
#define CDD_CST_QUERY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_cst_node.h"
/* clang-format on */

/**
 * @brief Visitor callback function type.
 * @param node The current node being visited.
 * @param user_data User provided data pointer.
 * @return 0 to continue, non-zero to halt traversal.
 */
typedef int (*cdd_cst_visitor_fn)(cdd_cst_node_t *node, void *user_data);

/**
 * @brief Traverses the CST in pre-order.
 * @param root The root node to start traversal.
 * @param visitor The callback function.
 * @param user_data Passed to the callback.
 * @return 0 on success, or the non-zero value returned by the visitor.
 */
int cdd_cst_traverse_preorder(cdd_cst_node_t *root, cdd_cst_visitor_fn visitor,
                              void *user_data);

/**
 * @brief Traverses the CST in post-order.
 * @param root The root node to start traversal.
 * @param visitor The callback function.
 * @param user_data Passed to the callback.
 * @return 0 on success, or the non-zero value returned by the visitor.
 */
int cdd_cst_traverse_postorder(cdd_cst_node_t *root, cdd_cst_visitor_fn visitor,
                               void *user_data);

/**
 * @brief Result struct for node queries.
 */
typedef struct cdd_cst_query_result_t cdd_cst_query_result_t;
/** @brief Struct definition */
struct cdd_cst_query_result_t {
  /** @brief field */
  /** @brief field */
  cdd_cst_node_t **nodes;
  /** @brief field */
  /** @brief field */
  size_t size;
  /** @brief field */
  /** @brief field */
  size_t capacity;
};

/**
 * @brief Finds all nodes of a specific kind in the tree (pre-order search).
 * @param root The root node to search from.
 * @param kind The kind of node to find.
 * @param out_result Pointer to result struct to be populated. Caller must free
 * out_result->nodes.
 * @return 0 on success.
 */
int cdd_cst_find_nodes_by_type(cdd_cst_node_t *root,
                               enum cdd_cst_node_kind_t kind,
                               cdd_cst_query_result_t *out_result);

/**
 * @brief Finds all function call nodes matching a specific name.
 * @param root The root node to search from.
 * @param func_name The name of the function to find calls for.
 * @param out_result Pointer to result struct. Caller must free
 * out_result->nodes.
 * @return 0 on success.
 */
int cdd_cst_find_function_calls_named(cdd_cst_node_t *root,
                                      const char *func_name,
                                      cdd_cst_query_result_t *out_result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_QUERY_H */
