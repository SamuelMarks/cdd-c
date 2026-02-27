/**
 * @file refactor_engine.c
 * @brief Implementation of refactoring orchestration.
 * @author Samuel Marks
 */

#include "functions/parse_refactor.h"
#include "c_str_span.h"
#include "functions/parse_analysis.h"
#include "functions/parse_tokenizer.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
#include <sys/errno.h>
#endif

int refactor_context_init(struct RefactorContext *const ctx) {
  if (!ctx)
    return EINVAL;
  ctx->funcs = NULL;
  ctx->func_count = 0;
  return 0;
}

void refactor_context_free(struct RefactorContext *const ctx) {
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

int refactor_context_add_function(struct RefactorContext *const ctx,
                                  const char *const name,
                                  const enum RefactorType type,
                                  const char *const return_type) {
  struct RefactoredFunction *new_alloc;
  if (!ctx || !name)
    return EINVAL;

  new_alloc = (struct RefactoredFunction *)realloc(
      ctx->funcs, (ctx->func_count + 1) * sizeof(struct RefactoredFunction));
  if (!new_alloc)
    return ENOMEM;

  ctx->funcs = new_alloc;
  ctx->funcs[ctx->func_count].name = name;
  ctx->funcs[ctx->func_count].type = type;
  ctx->funcs[ctx->func_count].original_return_type = return_type;
  ctx->func_count++;

  return 0;
}

int apply_refactoring_to_string(const struct RefactorContext *const ctx,
                                const char *const source_code,
                                char **const out_code) {
  struct TokenList *tokens = NULL;
  struct AllocationSiteList allocs = {0};
  int rc;

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
