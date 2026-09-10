#ifndef CDD_CST_TRANSFORM_H
#define CDD_CST_TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/parse/cdd_cst_node.h"
#include "cdd_c_error.h"
#include <stddef.h>
#include "c_cdd_export.h"
/* clang-format on */

/**
 * @brief Configuration for transformations.
 */
typedef struct cdd_transform_config_t {
  int use_tabs;     /**< 1 if tabs should be used for indentation */
  int indent_width; /**< Number of spaces (or tabs) per indentation level */
  int fallback_vla_to_malloc; /**< If 1, rewrite VLAs to use malloc/free instead
                                 of alloca */
  int target_c89;             /**< If 1, strictly target C89 semantics. */
  int target_c99;             /**< If 1, target C99 semantics. */
} cdd_transform_config_t;

/**
 * @brief Adds extern "C" wrapping to the parsed syntax tree.
 *
 * @param tree The CST tree.
 * @param config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_transform_extern_c(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);

/**
 * @brief Checks if a CST subtree contains C declarations.
 * @param[in] node Node to check.
 * @param[out] out_has_decl Pointer to int storing 1 if declarations found, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_tree_has_decl(cdd_cst_node_t *node,
                                             int *out_has_decl);

/**
 * @brief Checks if a CST node represents an #ifdef __cplusplus guard.
 * @param[in] dir Node to check.
 * @param[out] out_is_cpp Pointer to int storing 1 if guard found, 0 otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_check_node_is_cpp_guard(cdd_cst_node_t *dir,
                                                       int *out_is_cpp);

/**
 * @brief Ports POSIX/GNU specific syntax to MSVC equivalents.
 *
 * @param tree The CST tree.
 * @param config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_transform_msvc(cdd_cst_tree_t *tree, const cdd_transform_config_t *config);

/**
 * @brief Standardizes GNU-specific extensions (like __attribute__) into
 * standard C.
 *
 * @param tree The CST tree.
 * @param config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_transform_gnu(cdd_cst_tree_t *tree, const cdd_transform_config_t *config);

/**
 * @brief Checks if a token in a CST node represents a function call site.
 *
 * @param[in] node The CST node.
 * @param[in] ident_idx Index of the identifier token in node->children.
 * @param[out] out_is_call Pointer to int storing 1 if call, 0 otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_check_is_call(const cdd_cst_node_t *node,
                                             size_t ident_idx,
                                             int *out_is_call);

/**
 * @brief Checks if a token in a CST node represents a function definition.
 *
 * @param[in] node The CST node.
 * @param[in] ident_idx Index of the identifier token in node->children.
 * @param[out] out_is_def Pointer to int storing 1 if definition, 0 otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_check_is_function_def(const cdd_cst_node_t *node,
                                                     size_t ident_idx,
                                                     int *out_is_def);

/**
 * @brief Rewrites call sites in a CST node for functions whose signatures have
 * been modified.
 *
 * @param[in,out] tree The CST tree.
 * @param[in,out] node The current node to inspect and rewrite.
 * @param[in] modified_funcs Array of tokens representing modified function
 * names.
 * @param[in] num_modified Number of modified functions.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_rewrite_call_sites(cdd_cst_tree_t *tree,
                                                  cdd_cst_node_t *node,
                                                  cdd_token_t **modified_funcs,
                                                  size_t num_modified);

/**
 * @brief Percolates errors by rewriting function signatures and returning int.
 *
 * @param tree The CST tree.
 * @param config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_transform_percolate_errors(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);

/**
 * @brief Migrates standard C functions to MSVC Safe CRT functions.
 *
 * @param tree The CST tree.
 * @param config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_transform_safe_crt(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);

/**
 * @brief Expands function-like macros (and handles stringification/token
 * pasting) inside the AST.
 *
 * @param tree The CST tree.
 * @param config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_transform_macros(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_TRANSFORM_H */
