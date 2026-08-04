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

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_fail_alloc_refactor_add = 0;
#endif

/**
 * @brief Initializes a refactor context.
 *
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
cdd_c_error_t refactor_context_add_function(struct RefactorContext *ctx,
                                            const char *name,
                                            const enum RefactorType type,
                                            const char *return_type) {
  struct RefactoredFunction *new_alloc;
  if (!ctx || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  new_alloc = (struct RefactoredFunction *)realloc(
      ctx->funcs, (ctx->func_count + 1) * sizeof(struct RefactoredFunction));
#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_fail_alloc_refactor_add;
    if (g_cdd_fail_alloc_refactor_add) {
      free(new_alloc);
      new_alloc = NULL;
    }
  }
#endif
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
 */
cdd_c_error_t apply_refactoring_to_string(const struct RefactorContext *ctx,
                                          const char *source_code,
                                          char **const out_code) {
  struct TokenList *tokens = NULL;
  struct AllocationSiteList allocs = {0};
  int rc;

  if (ctx == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (source_code == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (out_code == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;

    /* 1. Tokenize */
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_audit_fail_tokenize;
    if (g_cdd_audit_fail_tokenize)
      rc = 1;
    else
      rc = tokenize(az_span_create_from_str((char *)source_code), &tokens);
  }
#else
  rc = tokenize(az_span_create_from_str((char *)source_code), &tokens);
#endif
  if (rc != 0) {
    return rc;
  }

  /* 2. Analyze Allocations */
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_audit_fail_find;
    if (g_cdd_audit_fail_find)
      rc = 1;
    else
      rc = find_allocations(tokens, &allocs);
  }
#else
  rc = find_allocations(tokens, &allocs);
#endif
  if (rc != 0) {
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
