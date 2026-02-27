/**
 * @file rewriter_body.h
 * @brief Logic to inject error handling, rewrite function calls, and transform
 * returns in function bodies.
 * Supports Call-Site Rewriting for propagated transformations and Safety
 * Injection.
 * @author Samuel Marks
 */

#ifndef REWRITER_BODY_H
#define REWRITER_BODY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "functions/parse_analysis.h"
#include "functions/parse_tokenizer.h"
#include <c_cdd_export.h>

/**
 * @brief Enum describing how a function call has been refactored.
 */
enum RefactorType {
  REF_VOID_TO_INT,   /**< void func() -> int func() */
  REF_PTR_TO_INT_OUT /**< T* func() -> int func(T** out) */
};

/**
 * @brief Specification for a refactored function that needs its callsites
 * updated.
 */
struct RefactoredFunction {
  const char *name;                 /**< Name of the function */
  enum RefactorType type;           /**< How the signature was changed */
  const char *original_return_type; /**< C type string (e.g. "char *"), used for
                                       temps. NULL if void. */
};

/**
 * @brief Enum describing how the *current* function's signature is being
 * transformed.
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
struct SignatureTransform {
  enum TransformType type;
  const char *arg_name;
  const char *success_code;
  const char *error_code;
  const char
      *return_type; /**< Original return type (e.g. "char *") for temp vars */
};

/**
 * @brief Rewrite the body of a function (token stream) to inject checks, update
 * calls, and transform returns.
 *
 * @param[in] tokens The token stream of the function body.
 * @param[in] allocs List of unchecked allocation sites.
 * @param[in] funcs Array of function specs that have been refactored.
 * @param[in] func_count Number of functions in `funcs`.
 * @param[in] transform Spec defining how to rewrite return statements.
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
