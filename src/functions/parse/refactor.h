/**
 * @file refactor_engine.h
 * @brief Orchestration logic for applying propagation refactorings across code.
 * @author Samuel Marks
 */

#ifndef REFACTOR_ENGINE_H
#define REFACTOR_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "functions/emit/rewriter_body.h"
#include <c_cdd_export.h>

/**
 * @brief Context containing global refactoring state.
 */
struct RefactorContext {
  struct RefactoredFunction *funcs; /**< Array of functions to propagate */
  size_t func_count;                /**< Number of functions */
};

/**
 * @brief Initialize a refactor context.
 * @param[out] ctx The context to initialize.
 * @return 0 on success.
 */
extern C_CDD_EXPORT int refactor_context_init(struct RefactorContext *ctx);

/**
 * @brief Free resources in refactor context.
 * @param[in] ctx The context.
 */
extern C_CDD_EXPORT void refactor_context_free(struct RefactorContext *ctx);

/**
 * @brief Add a function to the refactoring list.
 *
 * @param[in] ctx The context.
 * @param[in] name Function name.
 * @param[in] type Refactoring type (e.g. REF_VOID_TO_INT).
 * @param[in] return_type The original return type string (e.g. "char *"),
 * needed for temp var generation. Can be NULL if void.
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT int
refactor_context_add_function(struct RefactorContext *ctx, const char *name,
                              enum RefactorType type, const char *return_type);

/**
 * @brief Apply refactoring logic to a single source string.
 *
 * Automates the pipeline: Tokenize -> Analyze -> Rewrite.
 *
 * @param[in] ctx The refactoring context containing target functions.
 * @param[in] source_code Raw C source code string.
 * @param[out] out_code Pointer to char* where resulting code is stored.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int
apply_refactoring_to_string(const struct RefactorContext *ctx,
                            const char *source_code, char **out_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* REFACTOR_ENGINE_H */
