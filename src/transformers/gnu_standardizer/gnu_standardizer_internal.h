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

cdd_c_error_t gnu_malloc(size_t sz, void **out_ptr);
cdd_c_error_t gnu_alloc_node(enum cdd_cst_node_kind_t kind,
                             cdd_cst_node_t **out_node);
cdd_c_error_t gnu_find_node(cdd_cst_node_t *root, cdd_token_t *tok,
                            size_t *out_idx, cdd_cst_node_t **out_node);

cdd_c_error_t cdd_pool_string_safe(cdd_cst_tree_t *tree, const char *str,
                                   const char **out_pooled);
cdd_c_error_t cdd_pool_string_safe_len(cdd_cst_tree_t *tree, const char *str,
                                       size_t len, const char **out_pooled);

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT const char *pool_string_safe(cdd_cst_tree_t *tree,
                                          const char *str);
C_CDD_EXPORT const char *pool_string_safe_len(cdd_cst_tree_t *tree,
                                              const char *str, size_t len);
#else
const char *pool_string_safe(cdd_cst_tree_t *tree, const char *str);
const char *pool_string_safe_len(cdd_cst_tree_t *tree, const char *str,
                                 size_t len);
#endif

cdd_c_error_t cdd_append_int(char *p, int v, char **out_p);
cdd_c_error_t replace_token_with_text(cdd_cst_tree_t *tree, cdd_token_t *tok,
                                      enum cdd_token_kind_t kind,
                                      const char *text, size_t len);
cdd_c_error_t cdd_parse_128_literal(const char *str, size_t len,
                                    uint64_t *out_high, uint64_t *out_low);
cdd_c_error_t cdd_parse_hex_128_literal(const char *str, size_t len,
                                        uint64_t *out_high, uint64_t *out_low);

cdd_c_error_t cdd_magic_visitor(cdd_cst_node_t *node, void *user_data);
cdd_c_error_t cdd_tramp_visitor(cdd_cst_node_t *node, void *user_data);
cdd_c_error_t cdd_asm_visitor(cdd_cst_node_t *node, void *user_data);
cdd_c_error_t cdd_infer_type(const cdd_token_t *tokens, size_t num_tokens,
                             const char **out_type);

C_CDD_EXPORT cdd_c_error_t gnu_standardize_fn(cdd_cst_tree_t *tree);
cdd_c_error_t gnu_standardize_types(cdd_cst_tree_t *tree);
cdd_c_error_t gnu_standardize_expr(cdd_cst_tree_t *tree);
cdd_c_error_t gnu_standardize_unroll(cdd_cst_tree_t *tree,
                                     const cdd_transform_config_t *config);
cdd_c_error_t gnu_standardize_ranges(cdd_cst_tree_t *tree);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_TRANSFORMERS_GNU_STANDARDIZER_INTERNAL_H */
