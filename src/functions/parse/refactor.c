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
 */
enum cdd_c_error refactor_context_init(struct RefactorContext *ctx) {
  if (!ctx)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  ctx->funcs = NULL;
  ctx->func_count = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a refactor context.
 *
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
 * allocation fails.
 */
enum cdd_c_error refactor_context_add_function(struct RefactorContext *ctx,
                                               const char *name,
                                               const enum RefactorType type,
                                               const char *return_type) {
  struct RefactoredFunction *new_alloc;
  if (!ctx || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  new_alloc = (struct RefactoredFunction *)realloc(
      ctx->funcs, (ctx->func_count + 1) * sizeof(struct RefactoredFunction));
  if (!new_alloc) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }

  ctx->funcs = new_alloc;
  ctx->funcs[ctx->func_count].name = name;
  ctx->funcs[ctx->func_count].type = type;
  ctx->funcs[ctx->func_count].original_return_type = return_type;
  ctx->func_count++;

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies refactoring to a string of source code.
 *
 */
enum cdd_c_error apply_refactoring_to_string(const struct RefactorContext *ctx,
                                             const char *source_code,
                                             char **const out_code) {
  struct TokenList *tokens = NULL;
  struct AllocationSiteList allocs = {0};
  int rc;

  if (ctx == NULL || source_code == NULL || out_code == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Tokenize */
  if ((rc = tokenize(az_span_create_from_str((char *)source_code), &tokens)) !=
      0) {
    /* LCOV_EXCL_START */
    return rc;
    /* LCOV_EXCL_STOP */
  }

  /* 2. Analyze Allocations */
  if ((rc = find_allocations(tokens, &allocs)) != 0) {
    /* LCOV_EXCL_START */
    free_token_list(tokens);
    return rc;
    /* LCOV_EXCL_STOP */
  }

  /* 3. Rewrite Body */
  rc = rewrite_body(tokens, &allocs, ctx ? ctx->funcs : NULL,
                    ctx ? ctx->func_count : 0, NULL, out_code);

  allocation_site_list_free(&allocs);
  free_token_list(tokens);

  return rc;
}
