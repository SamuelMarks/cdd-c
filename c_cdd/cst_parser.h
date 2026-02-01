/**
 * @file cst_parser.h
 * @brief Concrete Syntax Tree (CST) Parser.
 *
 * Groups linear tokens into semantic blocks (Functions, Structs, Enums).
 * Enriched CST nodes now contain direct token indices to allow O(1)
 * lookups into the token stream.
 *
 * Supports C99/C11/C23 constructs:
 * - Compound Literals `(struct S){ ... }`
 * - Designated Initializers `.x = 1`
 * - Static Assertions inside blocks.
 * - C23 Attributes.
 * - C11 _Generic selections.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CST_PARSER_H
#define C_CDD_CST_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

#include "c_cdd_export.h"
#include "tokenizer.h"

/**
 * @brief High-level classification of CST Nodes.
 */
enum CstNodeKind {
  CST_NODE_STRUCT,            /**< Struct definition block */
  CST_NODE_ENUM,              /**< Enum definition block */
  CST_NODE_UNION,             /**< Union definition block */
  CST_NODE_FUNCTION,          /**< Function definition (signature + body) */
  CST_NODE_ATTRIBUTE,         /**< C23 Attribute block [[ ... ]] */
  CST_NODE_STATIC_ASSERT,     /**< Static assertion declaration */
  CST_NODE_GENERIC_SELECTION, /**< C11 _Generic(expr, assoc-list) */
  CST_NODE_COMMENT,           /**< Comment block (preserved for rewriting) */
  CST_NODE_MACRO,             /**< Preprocessor macro */
  CST_NODE_WHITESPACE,        /**< Whitespace block */
  CST_NODE_OTHER,  /**< Unclassified sentence (e.g. variables, expressions) */
  CST_NODE_UNKNOWN /**< Error sentinel */
};

/**
 * @brief A node in the CST.
 * Represents a logical grouping of tokens.
 */
struct CstNode {
  enum CstNodeKind kind; /**< Type of the node */
  const uint8_t
      *start;    /**< Byte pointer to start in source (for debugging/legacy) */
  size_t length; /**< Length in bytes */

  /* --- Navigation Metadata --- */
  size_t start_token; /**< Index of first token in TokenList */
  size_t end_token;   /**< Index of token AFTER the last token (Exclusive) */
};

/**
 * @brief Dynamic list of CST nodes.
 */
struct CstNodeList {
  struct CstNode *nodes; /**< Array of nodes */
  size_t size;           /**< Number of nodes */
  size_t capacity;       /**< Capacity */
};

/**
 * @brief Parse a token stream into CST nodes.
 *
 * Populates a CstNodeList by recursively identifying block structures.
 * Handles compound literals by consuming braces that appear in expression
 * contexts (e.g. assignments, returns) into the `CST_NODE_OTHER` node, rather
 * than breaking them into new block nodes.
 *
 * @param[in] tokens The token stream.
 * @param[out] out Destination structure (managed by caller, internal array
 * alloc'd).
 * @return 0 on success, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT int parse_tokens(const struct TokenList *tokens,
                                     struct CstNodeList *out);

/**
 * @brief Add a node manually (exposed for testing/manual construction).
 *
 * @param[in,out] list The list to append to.
 * @param[in] kind Node classification.
 * @param[in] start Byte pointer start.
 * @param[in] length Byte length.
 * @param[in] start_tok Token start index.
 * @param[in] end_tok Token end index (exclusive).
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT int cst_list_add(struct CstNodeList *list,
                                     enum CstNodeKind kind,
                                     const uint8_t *start, size_t length,
                                     size_t start_tok, size_t end_tok);

/**
 * @brief Free internal memory of CST list.
 * Does not free the `struct CstNodeList` pointer itself.
 *
 * @param[in] list The list to clean.
 */
extern C_CDD_EXPORT void free_cst_node_list(struct CstNodeList *list);

/**
 * @brief Find the first node of a specific kind in the list.
 *
 * @param[in] list The list to search.
 * @param[in] kind The kind to search for.
 * @return Pointer to the found node, or NULL if not found.
 */
extern C_CDD_EXPORT struct CstNode *cst_find_first(struct CstNodeList *list,
                                                   enum CstNodeKind kind);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CST_PARSER_H */
