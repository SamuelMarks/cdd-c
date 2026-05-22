/**
 * @file refactor.c
 * @brief Implementation of refactoring orchestration.
 * @author Samuel Marks
 */

/* clang-format off */
#include "functions/parse/refactor.h"
#include "c_cdd/log.h"
#include "c_str_span.h"
#include "functions/parse/analysis.h"
#include "functions/parse/tokenizer.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
#include <errno.h>
#include "c_cdd/log.h"
#endif
/* clang-format on */

/**
 * @brief Initializes a refactor context.
 *
 * @param[out] ctx The context to initialize.
 * @return 0 on success, EINVAL if ctx is NULL.
 */
int refactor_context_init(struct RefactorContext *ctx) {
  if (!ctx)
    return EINVAL;
  ctx->funcs = NULL;
  ctx->func_count = 0;
  return 0;
}

/**
 * @brief Frees a refactor context.
 *
 * @param[in,out] ctx The context to free.
 */
void refactor_context_free(struct RefactorContext *ctx) {
  /* Note: func names and return types are shallow copies/refs in this design,
     owned by the caller (usually orchestrator). We only free the array. */
  if (!ctx)
    return;
  if (ctx->funcs) {
    free(ctx->funcs);
    ctx->funcs = NULL;
  }
  ctx->func_count = 0;
}

/**
 * @brief Adds a function to the refactor context.
 *
 * @param[in,out] ctx The context to append to.
 * @param[in] name The name of the function to refactor.
 * @param[in] type The refactor type.
 * @param[in] return_type The original return type of the function.
 * @return 0 on success, EINVAL if invalid arguments, ENOMEM if memory
 * allocation fails.
 */
int refactor_context_add_function(struct RefactorContext *ctx, const char *name,
                                  const enum RefactorType type,
                                  const char *return_type) {
  struct RefactoredFunction *new_alloc;
  if (!ctx || !name)
    return EINVAL;

  new_alloc = (struct RefactoredFunction *)realloc(
      ctx->funcs, (ctx->func_count + 1) * sizeof(struct RefactoredFunction));
  if (!new_alloc) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  ctx->funcs = new_alloc;
  ctx->funcs[ctx->func_count].name = name;
  ctx->funcs[ctx->func_count].type = type;
  ctx->funcs[ctx->func_count].original_return_type = return_type;
  ctx->func_count++;

  return 0;
}

/**
 * @brief Applies refactoring to a string of source code.
 *
 * @param[in] ctx The refactor context containing target functions.
 * @param[in] source_code The input C source code string.
 * @param[out] out_code The output refactored code (caller must free).
 * @return 0 on success, or an error code on failure.
 */
int apply_refactoring_to_string(const struct RefactorContext *ctx,
                                const char *source_code,
                                char **const out_code) {
  struct TokenList *tokens = NULL;
  struct AllocationSiteList allocs = {0};
  int rc;

  if (ctx == NULL || source_code == NULL || out_code == NULL)
    return EINVAL;

  /* 1. Tokenize */
  if ((rc = tokenize(az_span_create_from_str((char *)source_code), &tokens)) !=
      0) {
    return rc;
  }

  /* 2. Analyze Allocations */
  if ((rc = find_allocations(tokens, &allocs)) != 0) {
    free_token_list(tokens);
    return rc;
  }

  /* 3. Rewrite Body */
  rc = rewrite_body(tokens, &allocs, ctx ? ctx->funcs : NULL,
                    ctx ? ctx->func_count : 0, NULL, out_code);

  allocation_site_list_free(&allocs);
  free_token_list(tokens);

  return rc;
}
