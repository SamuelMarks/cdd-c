/**
 * @file macro_overlay.h
 * @brief Macro AST Overlay for Non-Destructive Rewriting.
 */

#ifndef C_CDD_MACRO_OVERLAY_H
#define C_CDD_MACRO_OVERLAY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "functions/parse/cst.h"
#include "functions/parse/tokenizer.h"
#include "c_cdd_export.h"

/**
 * @brief Represents a dual-view macro invocation.
 * The `invocation_node` points to the raw `#define` or macro usage in the CST.
 * The `expanded_ast` contains the parsed C tokens representing the semantic evaluation.
 */
struct MacroOverlayNode {
  const struct CstNode *invocation_node; /**< The raw textual CST node containing the macro call */
  struct CstNodeList *expanded_ast;      /**< The parsed semantic equivalent */
};

/**
 * @brief List of macro overlays for a file.
 */
struct MacroOverlayList {
  struct MacroOverlayNode *nodes;
  size_t size;
  size_t capacity;
};

/**
 * @brief Initialize a macro overlay list.
 */
C_CDD_EXPORT void macro_overlay_list_init(struct MacroOverlayList *list);

/**
 * @brief Free resources associated with a macro overlay list.
 */
C_CDD_EXPORT void macro_overlay_list_free(struct MacroOverlayList *list);

/**
 * @brief Scan a CST for macro invocations and build the overlay mappings.
 *
 * @param cst The root Concrete Syntax Tree to scan.
 * @param tokens The base token stream for resolving macro boundaries.
 * @param overlays Output overlay list to populate.
 * @return 0 on success, non-zero error code otherwise.
 */
C_CDD_EXPORT int cst_build_macro_overlay(const struct CstNodeList *cst,
                                         const struct TokenList *tokens,
                                         struct MacroOverlayList *overlays);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_MACRO_OVERLAY_H */
