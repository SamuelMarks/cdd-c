/**
 * @file cst.h
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

/* clang-format off */
#include <stddef.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief High-level classification of CST Nodes.
 */
enum CstNodeKind {
  CST_NODE_STRUCT,            /**< Struct definition block */
  CST_NODE_ENUM,              /**< Enum definition block */
  CST_NODE_UNION,             /**< Union definition block */
  CST_NODE_FUNCTION,          /**< Function definition (signature + body) */
  CST_NODE_ATTRIBUTE,         /**< C23 Attribute block [[ ... ]] */
  CST_NODE_GCC_ATTRIBUTE,     /**< GCC __attribute__((...)) */
  CST_NODE_DECLSPEC,          /**< MSVC __declspec(...) */
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
 * @return 0 on success, EINVAL on invalid inputs, ENOMEM on allocation failure.
 */
extern C_CDD_EXPORT cdd_c_error_t parse_tokens(const struct TokenList *tokens,
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
extern C_CDD_EXPORT cdd_c_error_t cst_list_add(struct CstNodeList *list,
                                               enum CstNodeKind kind,
                                               const uint8_t *start,
                                               size_t length, size_t start_tok,
                                               size_t end_tok);

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
 * @param[out] out_node Pointer to store the found node, or NULL if not found.
 * @return 0 on success (whether found or not), EINVAL on bad params.
 */
extern C_CDD_EXPORT cdd_c_error_t cst_find_first(struct CstNodeList *list,
                                                 enum CstNodeKind kind,
                                                 struct CstNode **out_node);

#ifdef CDD_BUILD_TESTS
/**
 * @brief Test wrapper for skip_ws.
 * @param[in] tokens Token list.
 * @param[in] i Current index.
 * @param[in] limit Index limit.
 * @param[out] out_val Output index.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_cst_skip_ws(
    const struct TokenList *tokens, size_t i, size_t limit, size_t *out_val);

/**
 * @brief Test wrapper for skip_ws_back.
 * @param[in] tokens Token list.
 * @param[in] i Current index.
 * @param[out] out_val Output index.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_cst_skip_ws_back(
    const struct TokenList *tokens, size_t i, size_t *out_val);

/**
 * @brief Test wrapper for is_type_start.
 * @param[in] tok Token to check.
 * @param[out] out_is_type Output flag.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_cst_is_type_start(const struct Token *tok, int *out_is_type);

/**
 * @brief Test wrapper for match_function_definition.
 * @param[in] tokens Token list.
 * @param[in] start_idx Start index.
 * @param[in] limit Index limit.
 * @param[out] end_idx_out Output end index.
 * @param[out] out_is_match Output match flag.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_cst_match_function_definition(
    const struct TokenList *tokens, size_t start_idx, size_t limit,
    size_t *end_idx_out, int *out_is_match);

/**
 * @brief Test wrapper for consume_balanced_parens.
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] limit Index limit.
 * @param[out] out_val Output index.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_cst_consume_balanced_parens(
    const struct TokenList *tokens, size_t start, size_t limit,
    size_t *out_val);

/**
 * @brief Test wrapper for consume_attributes.
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] limit Index limit.
 * @param[out] out_val Output index.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_cst_consume_attributes(const struct TokenList *tokens, size_t start,
                                size_t limit, size_t *out_val);

/**
 * @brief Test wrapper for consume_static_assert.
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] limit Index limit.
 * @param[out] out_val Output index.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_cst_consume_static_assert(const struct TokenList *tokens, size_t start,
                                   size_t limit, size_t *out_val);

/**
 * @brief Test wrapper for consume_generic_selection.
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] limit Index limit.
 * @param[out] out_val Output index.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_cst_consume_generic_selection(
    const struct TokenList *tokens, size_t start, size_t limit,
    size_t *out_val);

/**
 * @brief Test wrapper for is_expression_brace.
 * @param[in] tokens Token list.
 * @param[in] brace_idx Index of brace.
 * @param[out] out_is_expr Output flag.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_cst_is_expression_brace(
    const struct TokenList *tokens, size_t brace_idx, int *out_is_expr);

/**
 * @brief Test wrapper for consume_balanced_braces.
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] limit Index limit.
 * @param[out] out_val Output index.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_cst_consume_balanced_braces(
    const struct TokenList *tokens, size_t start, size_t limit,
    size_t *out_val);

/**
 * @brief Test wrapper for parse_recursive.
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @param[in,out] out Output list.
 * @return CDD_C_SUCCESS or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_cst_parse_recursive(const struct TokenList *tokens, size_t start,
                             size_t end, struct CstNodeList *out);
#endif /* CDD_BUILD_TESTS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CST_PARSER_H */
