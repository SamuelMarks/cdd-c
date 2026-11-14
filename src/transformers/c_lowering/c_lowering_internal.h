/**
 * @file c_lowering_internal.h
 * @brief Internal declarations for C99/C11/C23 to MSVC/C89 CST lowering passes.
 */
#ifndef CDD_TRANSFORMERS_C_LOWERING_INTERNAL_H
#define CDD_TRANSFORMERS_C_LOWERING_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include "classes/parse/cdd_cst_scope.h"
#include "classes/parse/cdd_cst_semantic.h"
#include "classes/parse/cdd_cst_type_eval.h"
#include "transformers/gnu_standardizer/gnu_standardizer_internal.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Shared per-TU state for C99/C11/C23 -> MSVC/C89 CST lowering passes.
 */
struct c_lowering_ctx_t {
  cdd_cst_tree_t *tree;                 /**< Target CST tree */
  cdd_cst_scope_env_t *sem_env;         /**< Semantic scope & symbol table */
  const cdd_transform_config_t *config; /**< Active transformation config */
  int is_msvc;       /**< 1 on MSVC legacy or modern profiles */
  int is_legacy_c89; /**< 1 on MSVC legacy, Watcom, ISO C89, or target_c89 */
  int is_iso_c89;    /**< 1 on strict non-MSVC ISO C89 / Watcom */
  int cl_idx;        /**< Counter for hoisted compound-literal temporaries */
  int sa_idx;        /**< Counter for lowered `_Static_assert` typedefs */
  int for_idx;       /**< Counter for collision-renamed `for`-init vars */
  int vla_idx;       /**< Counter for hybrid VLA helper variables */
  int used_complex;  /**< Bitmask: 1 float complex, 2 double/ldouble complex */
  int used_ckd;      /**< 1 if C23 `<stdckdint.h>` `ckd_*` helpers are used */
  int used_atomic;   /**< 1 if C11 `_Atomic` declarations/ops are lowered */
  char atomic_vars[64][96];  /**< Lowered `_Atomic` variable identifiers */
  size_t num_atomic_vars;    /**< Number of entries in `atomic_vars` */
  char complex_vars[64][96]; /**< Lowered `_Complex` variable identifiers */
  int complex_is_float[64];  /**< 1 if corresponding complex var is float */
  size_t num_complex_vars;   /**< Number of entries in `complex_vars` */
};

/**
 * @brief Prepends a raw C snippet node at index 0 of `tree->root`.
 *
 * @param[in,out] tree CST tree.
 * @param[in] snippet Null-terminated C snippet string.
 * @return CDD_C_SUCCESS or error code.
 */
C_CDD_EXPORT cdd_c_error_t c_low_prepend_root(cdd_cst_tree_t *tree,
                                              const char *snippet);

/**
 * @brief Extracts space-separated token text from `toks[s..e_excl)`.
 *
 * @param[in] toks Token array.
 * @param[in] s Inclusive start index.
 * @param[in] e_excl Exclusive end index.
 * @param[out] buf Destination buffer.
 * @param[in] cap Buffer capacity.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t c_low_slice_text(const cdd_token_t *toks, size_t s,
                                            size_t e_excl, char *buf,
                                            size_t cap);

/**
 * @brief Replaces token span `toks[s..e_incl]` with `repl` on `toks[s]` and
 * wipes `toks[s+1..e_incl]`.
 *
 * @param[in,out] tree CST tree.
 * @param[in] s First token index.
 * @param[in] e_incl Last token index (inclusive).
 * @param[in] repl Null-terminated replacement string.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t c_low_replace_span(cdd_cst_tree_t *tree, size_t s, size_t e_incl,
                                 const char *repl);

/**
 * @brief Prepends `prefix` onto `tok` in `tree`.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] tok Target token.
 * @param[in] prefix Null-terminated prefix string.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t c_low_prepend_tok(cdd_cst_tree_t *tree, cdd_token_t *tok,
                                const char *prefix);

/**
 * @brief Appends `suffix` onto `tok` in `tree`.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] tok Target token.
 * @param[in] suffix Null-terminated suffix string.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t c_low_append_tok(cdd_cst_tree_t *tree, cdd_token_t *tok,
                               const char *suffix);

/**
 * @brief Inserts `ins` immediately after the opening `{` character of `tok`.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] tok Opening `{` token (`tok->length >= 1`).
 * @param[in] ins Null-terminated declaration text to insert after `{`.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t c_low_insert_after_lbrace(cdd_cst_tree_t *tree, cdd_token_t *tok,
                                        const char *ins);

/**
 * @brief Executes Phase 3.1 C99/C11/C23 lowering passes required on both Legacy
 * and Modern MSVC (C23 keywords/attributes/digits/stdckdint, array param
 * qualifiers, VLAs with hybrid/malloc/alloca + sizeof(vla), `_Complex`, and
 * `_Atomic`).
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t c_lower_modern_passes(struct c_lowering_ctx_t *ctx);

/**
 * @brief Executes Phase 3.2 Legacy MSVC / C89 lowering passes (compound
 * literals, designated initializers, mixed declarations & `for`-init hoisting,
 * legacy keywords, `ULL`->`UI64`, `//`->block comments, and `%zu`/`%lld`
 * printf/scanf format specifiers).
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t c_lower_legacy_passes(struct c_lowering_ctx_t *ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_TRANSFORMERS_C_LOWERING_INTERNAL_H */
