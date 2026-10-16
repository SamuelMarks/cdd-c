/**
 * @file gnu_standardizer_internal.h
 * @brief Internal declarations and prototypes for GNU standardizer transformer
 * passes.
 */

#ifndef CDD_TRANSFORMERS_GNU_STANDARDIZER_INTERNAL_H
#define CDD_TRANSFORMERS_GNU_STANDARDIZER_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/safe_crt.h"
#include "c_cdd/format_specifiers.h"
#include "c_str_span.h"
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_mutate.h"

#ifdef _MSC_VER
#ifndef strdup
#define strdup _strdup
#endif
#endif
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include "classes/parse/numeric.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
/** @brief MAX_LOCAL_LABELS */
#define MAX_LOCAL_LABELS 4
/** @brief MAX_VLAS */
#define MAX_VLAS 4
/** @brief MAX_CLEANUPS */
#define MAX_CLEANUPS 4
#else
/** @brief MAX_LOCAL_LABELS */
#define MAX_LOCAL_LABELS 256
/** @brief MAX_VLAS */
#define MAX_VLAS 256
/** @brief MAX_CLEANUPS */
#define MAX_CLEANUPS 256
#endif

#ifdef _MSC_VER
/** @brief ULL_HEX_FMT */
#define ULL_HEX_FMT "%I64x"
#else
/** @brief ULL_HEX_FMT */
#define ULL_HEX_FMT "%llx"
#endif

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_gnu_standardizer_fail;
extern C_CDD_EXPORT volatile int g_gnu_alloc_fail;
extern C_CDD_EXPORT volatile int g_gnu_replace_fail;
extern C_CDD_EXPORT volatile int g_gnu_bld_fail;
extern C_CDD_EXPORT volatile int g_gnu_malloc_fail;
extern C_CDD_EXPORT volatile int g_gnu_find_fail;
#endif

/**
 * @brief Allocates memory for GNU standardizer transformations.
 * @param[in] sz Number of bytes to allocate.
 * @param[out] out_ptr Pointer to receive allocated memory.
 * @return CDD_C_SUCCESS on success or CDD_C_ERROR_MEMORY on failure.
 */
C_CDD_EXPORT cdd_c_error_t gnu_malloc(size_t sz, void **out_ptr);

/**
 * @brief Allocates a CST node of the specified kind.
 * @param[in] kind CST node kind.
 * @param[out] out_node Pointer to receive allocated CST node.
 * @return CDD_C_SUCCESS on success or CDD_C_ERROR_MEMORY on failure.
 */
C_CDD_EXPORT cdd_c_error_t gnu_alloc_node(enum cdd_cst_node_kind_t kind,
                                          cdd_cst_node_t **out_node);

/**
 * @brief Finds the parent CST node and child index containing a given token.
 * @param[in] root Root CST node to search.
 * @param[in] tok Target token to locate.
 * @param[out] out_idx Child index within the found parent node.
 * @param[out] out_node Found parent CST node.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t gnu_find_node(cdd_cst_node_t *root, cdd_token_t *tok,
                                         size_t *out_idx,
                                         cdd_cst_node_t **out_node);

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

/**
 * @brief Standardizes GNU function attributes and nested functions in a CST
 * tree.
 * @param[in,out] tree Target CST tree.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t gnu_standardize_fn(cdd_cst_tree_t *tree);

/**
 * @brief Standardizes GNU type extensions (__int128, typeof, __auto_type) in a
 * CST tree.
 * @param[in,out] tree Target CST tree.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t gnu_standardize_types(cdd_cst_tree_t *tree);

/**
 * @brief Standardizes GNU expression extensions (statement expressions, ?:
 * elision) in a CST tree.
 * @param[in,out] tree Target CST tree.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t gnu_standardize_expr(cdd_cst_tree_t *tree);

/**
 * @brief Unrolls loops annotated with \#pragma GCC unroll in a CST tree.
 * @param[in,out] tree Target CST tree.
 * @param[in] config Transformer configuration.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t gnu_standardize_unroll(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config);

/**
 * @brief Expands GNU case range statements (case A ... B:) in a CST tree.
 * @param[in,out] tree Target CST tree.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t gnu_standardize_ranges(cdd_cst_tree_t *tree);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_TRANSFORMERS_GNU_STANDARDIZER_INTERNAL_H */
