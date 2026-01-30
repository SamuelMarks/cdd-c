/**
 * @file rewriter_body.h
 * @brief Logic to inject error handling and rewrite function calls in function
 * bodies.
 * @author Samuel Marks
 */

#ifndef REWRITER_BODY_H
#define REWRITER_BODY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "analysis.h"
#include "tokenizer.h"
#include <c_cdd_export.h>

/**
 * @brief Enum describing how a function has been refactored.
 */
enum RefactorType {
  REF_VOID_TO_INT,   /**< void func() -> int func() */
  REF_PTR_TO_INT_OUT /**< T* func() -> int func(T** out) */
};

/**
 * @brief Specification for a refactored function that needs its callsites
 * updated.
 */
struct C_CDD_EXPORT RefactoredFunction {
  const char *name;       /**< Name of the function */
  enum RefactorType type; /**< How the signature was changed */
};

/**
 * @brief Rewrite the body of a function (token stream) to inject checks and
 * update calls.
 *
 * Operations performed:
 * 1. At sites specified in `allocs` (unchecked allocations), inject `if (!ptr)
 * { return ENOMEM; }`.
 * 2. Identify calls to functions listed in `funcs`.
 *    - For `REF_VOID_TO_INT`: Wrap call `func(args);` -> `if (func(args) != 0)
 * return EIO;`
 *    - For `REF_PTR_TO_INT_OUT`: Transform `var = func(args);` -> `if
 * (func(args, &var) != 0) return EIO;`
 *
 * @param[in] tokens The token stream of the function body.
 * @param[in] allocs List of unchecked allocation sites (from analysis).
 * @param[in] funcs Array of function specs that have been refactored.
 * @param[in] func_count Number of functions in `funcs`.
 * @param[out] out_code Pointer to char* where the allocated result string is
 * stored.
 * @return 0 on success, ENOMEM or EINVAL on error.
 */
extern C_CDD_EXPORT int rewrite_body(const struct TokenList *tokens,
                                     const struct AllocationSiteList *allocs,
                                     const struct RefactoredFunction *funcs,
                                     size_t func_count, char **out_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* REWRITER_BODY_H */
