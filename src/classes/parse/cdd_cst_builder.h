#ifndef CDD_CST_BUILDER_H
#define CDD_CST_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_cst_node.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Stateful builder for constructing Concrete Syntax Trees (CST).
 *
 * Encapsulates the target tree, the current insertion point (node),
 * and an internal error state to allow monadic/fluent-style building.
 */
typedef struct cdd_cst_builder_t cdd_cst_builder_t;

/**
 * @brief Struct definition for cdd_cst_builder_t
 */
struct cdd_cst_builder_t {
  /** @brief Pointer to the tree being built */
  cdd_cst_tree_t *tree;
  /** @brief Pointer to the current target node where new children will be
   * appended */
  cdd_cst_node_t *target_node;
  /** @brief Internal error state (0 on success, non-zero on error e.g., ENOMEM)
   */
  int error_state;
  /** @brief indent_level field */
  int indent_level;
};

/**
 * @brief Initializes a CST builder.
 *
 * @param builder The builder to initialize.
 * @param tree The CST tree.
 * @param target_node The initial node to append children to.
 * @return int 0 on success, or an error code (e.g., EINVAL).
 */
C_CDD_EXPORT int cdd_cst_builder_init(cdd_cst_builder_t *builder,
                                      cdd_cst_tree_t *tree,
                                      cdd_cst_node_t *target_node);

/**
 * @brief Frees resources associated with a CST builder (if any).
 *
 * @param builder The builder to free.
 * @return int 0 on success.
 */
C_CDD_EXPORT int cdd_cst_builder_free(cdd_cst_builder_t *builder);

/**
 * @brief Checks if the builder has encountered an error.
 *
 * @param builder The builder to check.
 * @return int 1 if an error occurred, 0 otherwise.
 */
C_CDD_EXPORT int cdd_cst_builder_has_error(const cdd_cst_builder_t *builder);

/**
 * @brief Sets the current insertion point for the builder.
 *
 * @param builder The builder to update.
 * @param node The new target node.
 * @return int 0 on success, or an error code if the builder has already failed.
 */
C_CDD_EXPORT int cdd_cst_builder_set_insert_point(cdd_cst_builder_t *builder,
                                                  cdd_cst_node_t *node);

/**
 * @brief Appends a token to the current target node.
 *
 * @param builder The builder
 * @param kind The token kind
 * @param text The token text
 * @return int 0 on success, or an error code
 */
C_CDD_EXPORT int cdd_cst_bld_token(cdd_cst_builder_t *builder,
                                   enum cdd_token_kind_t kind,
                                   const char *text);

/**
 * @brief Appends a space character.
 */
C_CDD_EXPORT int cdd_cst_bld_space(cdd_cst_builder_t *builder);

/**
 * @brief Appends a newline character.
 */
C_CDD_EXPORT int cdd_cst_bld_newline(cdd_cst_builder_t *builder);

/**
 * @brief Appends indentation spaces based on the depth level.
 *
 * @param builder The builder
 * @param depth_level The number of indentation levels (e.g. 1 level = 2 spaces)
 */
C_CDD_EXPORT int cdd_cst_bld_indent(cdd_cst_builder_t *builder,
                                    int depth_level);

/**
 * @brief Appends an identifier token.
 */
C_CDD_EXPORT int cdd_cst_bld_ident(cdd_cst_builder_t *builder,
                                   const char *text);

/**
 * @brief Appends a string literal token.
 */
C_CDD_EXPORT int cdd_cst_bld_string(cdd_cst_builder_t *builder,
                                    const char *text);

/**
 * @brief Appends an integer literal token.
 */
C_CDD_EXPORT int cdd_cst_bld_int(cdd_cst_builder_t *builder, int value);

/**
 * @brief Appends a punctuation token.
 */
C_CDD_EXPORT int cdd_cst_bld_punct(cdd_cst_builder_t *builder,
                                   const char *text);

/**
 * @brief Appends an #include directive.
 */
C_CDD_EXPORT int cdd_cst_bld_include(cdd_cst_builder_t *builder,
                                     const char *path, int is_system);

/**
 * @brief Appends an #ifndef macro_name directive.
 */
C_CDD_EXPORT int cdd_cst_bld_ifndef(cdd_cst_builder_t *builder,
                                    const char *macro_name);

/**
 * @brief Appends an #ifdef macro_name directive.
 */
C_CDD_EXPORT int cdd_cst_bld_ifdef(cdd_cst_builder_t *builder,
                                   const char *macro_name);

/**
 * @brief Appends an #else directive.
 */
C_CDD_EXPORT int cdd_cst_bld_else(cdd_cst_builder_t *builder);

/**
 * @brief Appends an #endif directive.
 */
C_CDD_EXPORT int cdd_cst_bld_endif(cdd_cst_builder_t *builder);

/**
 * @brief Appends extern "C" { block opening.
 */
C_CDD_EXPORT int cdd_cst_bld_extern_c_open(cdd_cst_builder_t *builder);

/**
 * @brief Appends extern "C" } block closing.
 */
C_CDD_EXPORT int cdd_cst_bld_extern_c_close(cdd_cst_builder_t *builder);

/**
 * @brief Appends a block opening brace { and increases internal indent state.
 */
C_CDD_EXPORT int cdd_cst_bld_block_open(cdd_cst_builder_t *builder);

/**
 * @brief Appends a block closing brace } and decreases internal indent state.
 */
C_CDD_EXPORT int cdd_cst_bld_block_close(cdd_cst_builder_t *builder);

/**
 * @brief Lexes a snippet of C code and appends its tokens to the builder's
 * current target.
 *
 * @param builder The CST builder.
 * @param snippet The C code snippet to lex.
 * @return int 0 on success, or an error code (e.g. ENOMEM).
 */
C_CDD_EXPORT int cdd_cst_bld_snippet(cdd_cst_builder_t *builder,
                                     const char *snippet);

/**
 * @brief Parses a restricted format string and builds CST components.
 *
 * Supports:
 *   %% - literal '%'
 *   %s - const char * (appended as identifier or literal depending on snippet
 * lexing) %d - int %n - cdd_cst_node_t * (injects an entire sub-tree)
 *
 * @param builder The CST builder.
 * @param format_string The format template.
 * @return int 0 on success, or an error code.
 */
C_CDD_EXPORT int cdd_cst_quote(cdd_cst_builder_t *builder,
                               const char *format_string, ...);

/**
 * @brief Appends a block comment (/ * ... * /) as trivia.
 */
C_CDD_EXPORT int cdd_cst_bld_block_comment(cdd_cst_builder_t *builder,
                                           const char *text);

/**
 * @brief Appends a line comment (// ...) as a trivia attached to the target
 * node.
 *
 * Note: If the last appended child is a token, the trivia will be attached as
 * trailing trivia. Otherwise, it will be attached to the target_node as leading
 * trivia.
 */
C_CDD_EXPORT int cdd_cst_bld_line_comment(cdd_cst_builder_t *builder,
                                          const char *text);

/**
 * @brief Extracts leading trivia from a source node, removing it from that
 * node.
 *
 * @param node The node to extract from.
 * @return The extracted trivia list (or NULL if none).
 */
C_CDD_EXPORT int cdd_cst_extract_leading_trivia(cdd_cst_node_t *node,
                                                cdd_trivia_t **out_trivia);

/**
 * @brief Extracts trailing trivia from a source node, removing it from that
 * node.
 *
 * @param node The node to extract from.
 * @return The extracted trivia list (or NULL if none).
 */
C_CDD_EXPORT int cdd_cst_extract_trailing_trivia(cdd_cst_node_t *node,
                                                 cdd_trivia_t **out_trivia);

/**
 * @brief Transfers all leading and trailing trivia from the source node to the
 * target node.
 *
 * @param source_node Node to remove trivia from.
 * @param target_node Node to attach trivia to.
 * @return 0 on success.
 */
C_CDD_EXPORT int cdd_cst_transfer_trivia(cdd_cst_node_t *source_node,
                                         cdd_cst_node_t *target_node);

/**
 * @brief Replaces a node preserving its trivia using the builder context.
 *
 * @param builder Builder context.
 * @param target_node Node to be replaced.
 * @param replacement_node New node to put in its place.
 * @return 0 on success.
 */
C_CDD_EXPORT int
cdd_cst_replace_node_preserve_trivia(cdd_cst_builder_t *builder,
                                     cdd_cst_node_t *target_node,
                                     cdd_cst_node_t *replacement_node);

/**
 * @brief Splices an array of nodes into a parent block, wrapping them in
 * children wrappers.
 *
 * @param builder Builder context.
 * @param parent Target block/node to inject into.
 * @param index Index to start splicing at.
 * @param new_nodes Array of nodes to inject.
 * @param count Number of nodes in new_nodes.
 * @return 0 on success.
 */
C_CDD_EXPORT int cdd_cst_splice_nodes(cdd_cst_builder_t *builder,
                                      cdd_cst_node_t *parent, size_t index,
                                      cdd_cst_node_t **new_nodes, size_t count);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_BUILDER_H */
