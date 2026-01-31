/**
 * @file rewriter_body.h
 * @brief Logic to inject error handling, rewrite function calls, and transform
 * returns in function bodies.
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
 * @brief Enum describing how a function call has been refactored (for call-site
 * rewriting).
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
 * @brief Enum describing how the *current* function's signature is being
 * transformed (for return rewriting).
 */
enum TransformType {
  TRANSFORM_NONE,          /**< No change to signature return type */
  TRANSFORM_VOID_TO_INT,   /**< void f() -> int f() */
  TRANSFORM_RET_PTR_TO_ARG /**< T* f() -> int f(T** out) */
};

/**
 * @brief Configuration for transforming the current function's return
 * statements.
 */
struct C_CDD_EXPORT SignatureTransform {
  enum TransformType
      type; /**< The type of transformation applied to the function signature */
  const char *arg_name; /**< Name of the output argument (e.g. "out"), used if
                           RE_PTR_TO_ARG */
  const char
      *success_code;      /**< Integer string to return on success (e.g. "0") */
  const char *error_code; /**< Integer string to return on failure (e.g.
                             "ENOMEM"). Optional (can be NULL). */
};

/**
 * @brief Rewrite the body of a function (token stream) to inject checks, update
 * calls, and transform returns.
 *
 * Operations performed:
 * 1. Allocator Checks: At sites specified in `allocs`, inject `if (!ptr) {
 * return ENOMEM; }`.
 * 2. Call-site Updates: Identify calls to functions in `funcs` and rewrite them
 * to handle error codes.
 * 3. Return Transformation: Rewrite `return`-statements based on `transform`.
 *    - VOID_TO_INT: `return;` -> `return <success_code>;`
 *    - PTR_TO_ARG: `return <expr>;` -> `{ *<arg_name> = <expr>; return
 * <success_code>; }` (Special case: if <expr> is "NULL" and error_code is set,
 * -> `return <error_code>;`)
 *
 * @param[in] tokens The token stream of the function body.
 * @param[in] allocs List of unchecked allocation sites (optional, can be NULL).
 * @param[in] funcs Array of function specs that have been refactored (optional,
 * can be NULL).
 * @param[in] func_count Number of functions in `funcs`.
 * @param[in] transform Spec defining how to rewrite return statements
 * (optional, can be NULL).
 * @param[out] out_code Pointer to char* where the allocated result string is
 * stored.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on invalid args.
 */
extern C_CDD_EXPORT int rewrite_body(const struct TokenList *tokens,
                                     const struct AllocationSiteList *allocs,
                                     const struct RefactoredFunction *funcs,
                                     size_t func_count,
                                     const struct SignatureTransform *transform,
                                     char **out_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* REWRITER_BODY_H */
