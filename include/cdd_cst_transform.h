/**
 * @file cdd_cst_transform.h
 * @brief Concrete Syntax Tree (CST) source-to-source transformation passes.
 */
#ifndef CDD_CST_TRANSFORM_H
#define CDD_CST_TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "classes/parse/cdd_cst_node.h"
#include "cdd_c_error.h"
#include <stddef.h>
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef unsigned __int64 uint64_t;
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "c_cdd/msvc/stdint.h"
#else
#include <stdint.h>
#endif
#endif
#endif
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
 * @brief Checks if a CST node represents an ifdef __cplusplus guard.
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
 * @brief Safely pools a string duplicate into the CST tree's string pool.
 *
 * @param[in,out] tree Target CST tree structure.
 * @param[in] str Source string to duplicate and pool.
 * @param[out] out_pooled Output pointer receiving the pooled string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT or
 * CDD_C_ERROR_MEMORY on error.
 */
C_CDD_EXPORT cdd_c_error_t cdd_pool_string_safe(cdd_cst_tree_t *tree,
                                                const char *str,
                                                const char **out_pooled);

/**
 * @brief Safely pools a string of given length into the CST tree's string pool.
 *
 * @param[in,out] tree Target CST tree structure.
 * @param[in] str Source string buffer.
 * @param[in] len Length of source string to copy.
 * @param[out] out_pooled Output pointer receiving the pooled string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT or
 * CDD_C_ERROR_MEMORY on error.
 */
C_CDD_EXPORT cdd_c_error_t cdd_pool_string_safe_len(cdd_cst_tree_t *tree,
                                                    const char *str, size_t len,
                                                    const char **out_pooled);

/**
 * @brief Appends an integer as string to the buffer.
 *
 * @param[in,out] p Pointer to destination buffer.
 * @param[in] v Integer value to append.
 * @param[out] out_p Pointer receiving the updated buffer end pointer.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * pointers.
 */
C_CDD_EXPORT cdd_c_error_t cdd_append_int(char *p, int v, char **out_p);

/**
 * @brief Allocates memory for GNU standardizer transformations.
 * @param[in] sz Number of bytes to allocate.
 * @param[out] out_ptr Pointer to receive allocated memory.
 * @return CDD_C_SUCCESS on success or CDD_C_ERROR_MEMORY on failure.
 */
C_CDD_EXPORT cdd_c_error_t gnu_malloc(size_t sz, void **out_ptr);

/**
 * @brief Replaces a token child in the CST with a new token holding text.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] tok Existing token in CST to replace.
 * @param[in] kind Token kind for replacement token.
 * @param[in] text Replacement text.
 * @param[in] len Length of replacement text.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t replace_token_with_text(cdd_cst_tree_t *tree,
                                                   cdd_token_t *tok,
                                                   enum cdd_token_kind_t kind,
                                                   const char *text,
                                                   size_t len);

/**
 * @brief Parses a decimal 128-bit literal into high and low 64-bit words.
 *
 * @param[in] str Decimal literal string.
 * @param[in] len Length of literal string.
 * @param[out] out_high Pointer receiving the high 64 bits.
 * @param[out] out_low Pointer receiving the low 64 bits.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * pointers.
 */
C_CDD_EXPORT cdd_c_error_t cdd_parse_128_literal(const char *str, size_t len,
                                                 uint64_t *out_high,
                                                 uint64_t *out_low);

/**
 * @brief Parses a hexadecimal 128-bit literal into high and low 64-bit words.
 *
 * @param[in] str Hexadecimal literal string.
 * @param[in] len Length of literal string.
 * @param[out] out_high Pointer receiving the high 64 bits.
 * @param[out] out_low Pointer receiving the low 64 bits.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * pointers.
 */
C_CDD_EXPORT cdd_c_error_t cdd_parse_hex_128_literal(const char *str,
                                                     size_t len,
                                                     uint64_t *out_high,
                                                     uint64_t *out_low);

/**
 * @brief Infers the C type from a sequence of tokens.
 *
 * @param[in] tokens Token array.
 * @param[in] num_tokens Number of tokens in array.
 * @param[out] out_type Pointer receiving inferred type string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * out_type.
 */
C_CDD_EXPORT cdd_c_error_t cdd_infer_type(const cdd_token_t *tokens,
                                          size_t num_tokens,
                                          const char **out_type);

/** @brief Context for magic identifiers transformation */
struct magic_ctx {
  cdd_cst_tree_t *tree;     /**< The CST tree */
  const uint8_t *func_name; /**< Current function name */
  size_t func_len;          /**< Length of function name */
};

/** @brief Context for trampoline detection */
struct tramp_ctx {
  const uint8_t *name;       /**< Identifier name */
  size_t length;             /**< Length of name */
  int is_tramp;              /**< Set to 1 if trampoline detected */
  cdd_cst_node_t *func_node; /**< Function node */
};

/**
 * @brief CST node visitor for standardizing magic macro identifiers.
 *
 * @param[in,out] node CST node to inspect and transform.
 * @param[in,out] user_data Pointer to magic context.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_magic_visitor(cdd_cst_node_t *node,
                                             void *user_data);

/**
 * @brief CST node visitor to detect trampoline function calls.
 *
 * @param[in] node CST node to inspect.
 * @param[in,out] user_data Pointer to trampoline context.
 * @return CDD_C_SUCCESS on success, or error code on trampoline detection or
 * error.
 */
C_CDD_EXPORT cdd_c_error_t cdd_tramp_visitor(cdd_cst_node_t *node,
                                             void *user_data);

/**
 * @brief CST node visitor for validating inline assembly statements.
 *
 * @param[in] node CST node to inspect.
 * @param[in,out] user_data User data context pointer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_asm_visitor(cdd_cst_node_t *node,
                                           void *user_data);

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
