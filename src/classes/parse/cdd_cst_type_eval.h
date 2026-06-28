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
  CDD_CST_ABI_ILP32, /* int, long, ptr = 32-bit */
  CDD_CST_ABI_LP64,  /* int = 32-bit, long, ptr = 64-bit */
  CDD_CST_ABI_LLP64  /* int, long = 32-bit, long long, ptr = 64-bit */
};

/**
 * @brief Basic type size and alignment info.
 */
typedef struct cdd_cst_type_info_t cdd_cst_type_info_t;
/** @brief Struct definition */
struct cdd_cst_type_info_t {
  /** @brief field */
  /** @brief field */
  size_t size;
  /** @brief field */
  /** @brief field */
  size_t alignment;
};

/**
 * @brief Evaluates sizeof and _Alignof for primitive types (int, long, etc)
 * based on ABI.
 * @param type_name The primitive type name (e.g. "int", "long", "long long").
 * @param abi The ABI model to use.
 * @param out_info Pointer to receive size and alignment.
 * @return 0 on success, or ENOENT if type is unknown.
 */
C_CDD_EXPORT enum cdd_c_error
cdd_cst_eval_primitive_type(const char *type_name, enum cdd_cst_abi_model_t abi,
                            cdd_cst_type_info_t *out_info);

/**
 * @brief Evaluates sizeof a type node based on the given ABI and scope
 * environment.
 * @param env Scope environment for struct/union/typedef resolution.
 * @param type_node The node representing the type.
 * @param abi The target ABI model.
 * @param out_size Resulting size in bytes.
 * @return 0 on success.
 */
C_CDD_EXPORT enum cdd_c_error cdd_cst_eval_sizeof(cdd_cst_scope_env_t *env,
                                                  cdd_cst_node_t *type_node,
                                                  enum cdd_cst_abi_model_t abi,
                                                  size_t *out_size);

/**
 * @brief Evaluates _Alignof a type node based on the given ABI and scope
 * environment.
 * @param env Scope environment for struct/union/typedef resolution.
 * @param type_node The node representing the type.
 * @param abi The target ABI model.
 * @param out_align Resulting alignment in bytes.
 * @return 0 on success.
 */
C_CDD_EXPORT enum cdd_c_error cdd_cst_eval_alignof(cdd_cst_scope_env_t *env,
                                                   cdd_cst_node_t *type_node,
                                                   enum cdd_cst_abi_model_t abi,
                                                   size_t *out_align);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_TYPE_EVAL_H */
