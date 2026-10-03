#ifndef CDD_CST_PARSER_INTERNAL_H
#define CDD_CST_PARSER_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "cdd_cst_parser.h"
#include "cdd_lexer.h"
/* clang-format on */

/** @brief parser_state_t struct */
typedef struct parser_state_t {
  /** @brief pos field */
  cdd_token_list_t *list;
  /** @brief pos field */
  size_t pos;
  int err; /**< err */
} parser_state_t;

/**
 * @brief Allocate a new CST node.
 *
 * @param[in] kind Node kind.
 * @param[in] parent Parent node or NULL.
 * @param[out] out_node Allocated node.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t alloc_node(enum cdd_cst_node_kind_t kind, cdd_cst_node_t *parent,
                         cdd_cst_node_t **out_node);

/**
 * @brief Append a child token to a node.
 *
 * @param[in,out] node Target node.
 * @param[in] tok Token to append.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t append_child_token(cdd_cst_node_t *node, cdd_token_t *tok);

/**
 * @brief Append a child node to a node.
 *
 * @param[in,out] node Target node.
 * @param[in,out] child Child node to append.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t append_child_node(cdd_cst_node_t *node, cdd_cst_node_t *child);

/**
 * @brief Free a node and all of its descendants recursively.
 *
 * @param[in,out] node Node to free.
 */
void free_node(cdd_cst_node_t *node);

/**
 * @brief Peek next token from parser state without advancing.
 *
 * @param[in] s Parser state.
 * @param[out] out_tok Pointer to store token pointer.
 * @return CDD_C_SUCCESS or error code.
 */
C_CDD_EXPORT cdd_c_error_t peek(parser_state_t *s, cdd_token_t **out_tok);

/**
 * @brief Advance parser state and return current token.
 *
 * @param[in,out] s Parser state.
 * @param[out] out_tok Pointer to store token pointer.
 * @return CDD_C_SUCCESS or error code.
 */
C_CDD_EXPORT cdd_c_error_t advance(parser_state_t *s, cdd_token_t **out_tok);

/**
 * @brief Parse a block delimited by braces.
 *
 * @param[in,out] s Parser state.
 * @param[in] parent Parent node.
 * @param[out] out_node Parsed block node.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t parse_block(parser_state_t *s, cdd_cst_node_t *parent,
                          cdd_cst_node_t **out_node);

/**
 * @brief Parse a class, struct, access specifier, asm, or expression/statement.
 *
 * @param[in,out] s Parser state.
 * @param[in] parent Parent node.
 * @param[in] t Current token.
 * @param[out] out_node Parsed node.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t parse_class_or_statement(parser_state_t *s,
                                       cdd_cst_node_t *parent, cdd_token_t *t,
                                       cdd_cst_node_t **out_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_PARSER_INTERNAL_H */
