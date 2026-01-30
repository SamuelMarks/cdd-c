/**
 * @file cst_parser.h
 * @brief Concrete Syntax Tree parser.
 * Groups tokens into meaningful high-level nodes like structs, enums, etc.
 * @author Samuel Marks
 */

#ifndef CST_PARSER_H
#define CST_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>
#include <stddef.h>

#include "tokenizer.h"

/**
 * @brief Type of a CST Node.
 */
enum C_CDD_EXPORT CstNodeKind1 {
  CST_NODE_STRUCT,     /**< Represents a `struct` definition */
  CST_NODE_ENUM,       /**< Represents an `enum` definition */
  CST_NODE_UNION,      /**< Represents a `union` definition */
  CST_NODE_FUNCTION,   /**< Represents a function definition */
  CST_NODE_COMMENT,    /**< Represents a comment block */
  CST_NODE_MACRO,      /**< Represents a preprocessor macro */
  CST_NODE_WHITESPACE, /**< Represents whitespace */
  CST_NODE_OTHER,      /**< Any other sequence (e.g. variable decl) */
  CST_NODE_UNKNOWN     /**< Unclassified */
};

/**
 * @brief A node in the Concrete Syntax Tree.
 */
struct C_CDD_EXPORT CstNode1 {
  enum CstNodeKind1 kind; /**< Kind of node */
  const uint8_t *start;   /**< Pointer to start in source buffer */
  size_t length;          /**< Total length covered by this node */
};

/**
 * @brief Dynamic list of CST nodes.
 */
struct C_CDD_EXPORT CstNodeList {
  struct CstNode1 *nodes; /**< Array of nodes */
  size_t size;            /**< Number of active nodes */
  size_t capacity;        /**< Allocated capacity */
};

/**
 * @brief Add a node to the list.
 * Exposed for testing and custom usage.
 *
 * @param[in,out] list The list to append to.
 * @param[in] kind The classification of node.
 * @param[in] start Pointer to start of text.
 * @param[in] length Length of text.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on invalid args.
 */
extern C_CDD_EXPORT int add_node(struct CstNodeList *list,
                                 enum CstNodeKind1 kind, const uint8_t *start,
                                 size_t length);

/**
 * @brief Parse a list of tokens into CST nodes.
 * Allocates memory in `out` which must be freed by `free_cst_node_list`.
 * This function does not free the input tokens.
 *
 * @param[in] tokens The list of tokens to parse.
 * @param[out] out The destination CST node list.
 * @return 0 on success, error code (e.g. ENOMEM) on failure.
 */
extern C_CDD_EXPORT int parse_tokens(const struct TokenList *tokens,
                                     struct CstNodeList *out);

/**
 * @brief Release memory associated with the CST node list.
 * Frees the internal `nodes` array but not the `struct CstNodeList` container
 * itself (assumed to be managed by caller or stack-allocated).
 *
 * @param[in] list Pointer to the list structure.
 */
extern C_CDD_EXPORT void free_cst_node_list(struct CstNodeList *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CST_PARSER_H */
