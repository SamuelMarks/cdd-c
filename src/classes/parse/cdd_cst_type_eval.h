/**
 * @file cdd_cst_type_eval.h
 * @brief Concrete Syntax Tree (CST) type eval implementation and utilities.
 */
#ifndef CDD_CST_TYPE_EVAL_H
#define CDD_CST_TYPE_EVAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "cdd_cst_node.h"
#include "cdd_cst_scope.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents standard ABI models.
 */
enum cdd_cst_abi_model_t {
  CDD_CST_ABI_ILP32, /**< int, long, ptr = 32-bit */
  CDD_CST_ABI_LP64,  /**< int = 32-bit, long, ptr = 64-bit */
  CDD_CST_ABI_LLP64  /**< int, long = 32-bit, long long, ptr = 64-bit */
};

/**
 * @brief Qualifier/decay normalization modes for CST expression type
 * evaluation.
 */
enum cdd_cst_type_eval_mode_t {
  CDD_CST_TYPE_MODE_EXACT = 0,  /**< Preserve qualifiers and array bounds */
  CDD_CST_TYPE_MODE_UNQUAL = 1, /**< Strip top-level cvr qualifiers */
  CDD_CST_TYPE_MODE_RVALUE = 2  /**< Strip top-level cvr & decay arrays to * */
};

/**
 * @brief Basic type size and alignment info.
 */
typedef struct cdd_cst_type_info_t cdd_cst_type_info_t;
/** @brief Struct definition */
struct cdd_cst_type_info_t {
  size_t size;      /**< Byte size of evaluated type */
  size_t alignment; /**< Byte alignment of evaluated type */
};

/**
 * @brief Evaluates sizeof and _Alignof for primitive types (int, long, etc)
 * based on ABI.
 * @param[in] type_name The primitive type name (e.g. "int", "long", "long
 * long").
 * @param[in] abi The ABI model to use.
 * @param[out] out_info Pointer to receive size and alignment.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_NOT_FOUND if unknown.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_cst_eval_primitive_type(const char *type_name, enum cdd_cst_abi_model_t abi,
                            cdd_cst_type_info_t *out_info);

/**
 * @brief Evaluates sizeof a type node based on the given ABI and scope
 * environment.
 * @param[in,out] env Scope environment for struct/union/typedef resolution.
 * @param[in,out] type_node The node representing the type.
 * @param[in] abi The target ABI model.
 * @param[out] out_size Resulting size in bytes.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_eval_sizeof(cdd_cst_scope_env_t *env,
                                               cdd_cst_node_t *type_node,
                                               enum cdd_cst_abi_model_t abi,
                                               size_t *out_size);

/**
 * @brief Evaluates _Alignof a type node based on the given ABI and scope
 * environment.
 * @param[in,out] env Scope environment for struct/union/typedef resolution.
 * @param[in,out] type_node The node representing the type.
 * @param[in] abi The target ABI model.
 * @param[out] out_align Resulting alignment in bytes.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_eval_alignof(cdd_cst_scope_env_t *env,
                                                cdd_cst_node_t *type_node,
                                                enum cdd_cst_abi_model_t abi,
                                                size_t *out_align);

/**
 * @brief Evaluates the canonical C type of a token-pointer expression or type
 * name within a lexical scope.
 * @param[in] env Optional semantic scope environment.
 * @param[in] scope Optional active lexical scope (defaults to
 * `env->global_scope`).
 * @param[in] tokens Array of token pointers forming the expression or type.
 * @param[in] num_tokens Number of entries in `tokens`.
 * @param[in] mode Qualifier/decay mode (`cdd_cst_type_eval_mode_t`).
 * @param[out] out_type Destination buffer receiving canonical C type string.
 * @param[in] out_cap Byte capacity of `out_type`.
 * @param[out] out_is_type_expr Optional flag set to 1 if `tokens` is a type
 * name, 0 if value expression.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_eval_token_ptrs_type(
    const cdd_cst_scope_env_t *env, const cdd_cst_scope_t *scope,
    const cdd_token_t *const *tokens, size_t num_tokens, int mode,
    char *out_type, size_t out_cap, int *out_is_type_expr);

/**
 * @brief Evaluates the canonical C type of a contiguous token slice within a
 * lexical scope.
 * @param[in] env Optional semantic scope environment.
 * @param[in] scope Optional active lexical scope.
 * @param[in] tokens Contiguous array of tokens.
 * @param[in] num_tokens Number of tokens in `tokens`.
 * @param[in] mode Qualifier/decay mode (`cdd_cst_type_eval_mode_t`).
 * @param[out] out_type Destination buffer receiving canonical C type string.
 * @param[in] out_cap Byte capacity of `out_type`.
 * @param[out] out_is_type_expr Optional flag set to 1 if `tokens` is a type
 * name, 0 if value expression.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_eval_tokens_type(
    const cdd_cst_scope_env_t *env, const cdd_cst_scope_t *scope,
    const cdd_token_t *tokens, size_t num_tokens, int mode, char *out_type,
    size_t out_cap, int *out_is_type_expr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_TYPE_EVAL_H */
