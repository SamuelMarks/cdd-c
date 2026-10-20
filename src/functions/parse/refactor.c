/**
 * @file refactor.c
 * @brief Implementation of refactoring orchestration.
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "functions/parse/refactor.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_str_span.h"
#include "functions/parse/analysis.h"
#include "functions/parse/tokenizer.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Initializes a refactor context.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * NULL.
 */
cdd_c_error_t refactor_context_init(struct RefactorContext *ctx) {
  if (!ctx)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  ctx->funcs = NULL;
  ctx->func_count = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a refactor context.
 */
void refactor_context_free(struct RefactorContext *ctx) {
  /* Note: func names and return types are shallow copies/refs in this design,
     owned by the caller (usually orchestrator). We only free the array. */
  if (!ctx)
    return;
  if (ctx->funcs) {
    C_CDD_FREE(ctx->funcs);
    ctx->funcs = NULL;
  }
  ctx->func_count = 0;
}

/**
 * @brief Adds a function to the refactor context.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` if allocation
 * fails.
 */
cdd_c_error_t refactor_context_add_function(struct RefactorContext *ctx,
                                            const char *name,
                                            const enum RefactorType type,
                                            const char *return_type) {
  struct RefactoredFunction *new_alloc;
  if (!ctx || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  new_alloc = (struct RefactoredFunction *)C_CDD_REALLOC(
      ctx->funcs, (ctx->func_count + 1) * sizeof(struct RefactoredFunction));
  if (!new_alloc) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
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
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t apply_refactoring_to_string(const struct RefactorContext *ctx,
                                          const char *source_code,
                                          char **out_code) {
  struct TokenList *tokens = NULL;
  struct AllocationSiteList allocs = {0};
  cdd_c_error_t rc;

  if (ctx == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (source_code == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (out_code == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Tokenize */
  rc = tokenize(az_span_create_from_str(source_code), &tokens);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  /* 2. Analyze Allocations */
  rc = find_allocations(tokens, &allocs);
  if (rc != CDD_C_SUCCESS) {
    free_token_list(tokens);
    return rc;
  }

  /* 3. Rewrite Body */
  rc = rewrite_body(tokens, &allocs, ctx->funcs, ctx->func_count, NULL,
                    out_code);

  allocation_site_list_free(&allocs);
  free_token_list(tokens);

  return rc;
}
