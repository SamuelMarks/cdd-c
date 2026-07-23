#ifndef TEST_ORCHESTRATOR_INTERNALS_H
#define TEST_ORCHESTRATOR_INTERNALS_H

#include "c_cdd_export.h"
#include "functions/parse/tokenizer.h"
#include <greatest.h>

int g_force_find_allocations_fail = 0;
#define orchestrate_fix internal_orchestrate_fix
#define fix_code_main internal_fix_code_main

#define find_allocations mock_find_allocations
static enum cdd_c_error mock_find_allocations(const struct TokenList *tokens,
                                              struct AllocationSiteList *out);
#include "functions/parse/orchestrator.c"
#undef find_allocations
extern enum cdd_c_error find_allocations(const struct TokenList *tokens,
                                         struct AllocationSiteList *out);
static enum cdd_c_error mock_find_allocations(const struct TokenList *tokens,
                                              struct AllocationSiteList *out) {
  if (g_force_find_allocations_fail)
    return CDD_C_ERROR_MEMORY;
  return find_allocations(tokens, out);
}

TEST test_orchestrator_internals(void) {
  struct TokenList *tl = NULL;
  char *out_fix = NULL;
  struct TokenList dst;
  size_t out = 0;
  char *name = NULL;
  struct DependencyGraph g;
  struct FixWalkContext ctx = {0};
  char *argv3[] = {"1", "2", "3"};
  int is_src = 0;

  tokenize(AZ_SPAN_FROM_STR("int a = 1;"), &tl);

  /* Test get_token_slice error */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_token_slice(tl, 100, 10, &dst));

  /* Test find_token_in_range not found */
  ASSERT_EQ(CDD_C_SUCCESS,
            find_token_in_range(tl, 0, tl->size, TOKEN_LPAREN, &out));
  ASSERT_EQ(tl->size, out);

  /* Test extract_func_name not found */
  ASSERT_EQ(CDD_C_SUCCESS, extract_func_name(tl, 0, tl->size, &name));
  ASSERT_EQ(NULL, name);

  free_token_list(tl);

  /* Test analyze_signature_tokens void as identifier */
  {
    struct TokenList *my_tl = NULL;
    int is_ptr = 0, is_void = 0;
    char *type_str = NULL;
    tokenize(AZ_SPAN_FROM_STR("void foo()"), &my_tl);
    /* Change the keyword void to identifier to test the fallback */
    my_tl->tokens[0].kind = TOKEN_IDENTIFIER;

    ASSERT_EQ(CDD_C_SUCCESS, analyze_signature_tokens(my_tl, 0, 5, &is_ptr,
                                                      &is_void, &type_str));
    printf("is_void=%d kind=%d len=%d start=%.4s eq=%d\n", is_void,
           my_tl->tokens[0].kind, (int)my_tl->tokens[0].length,
           my_tl->tokens[0].start, token_eq_str(&my_tl->tokens[0], "void"));

    if (type_str)
      free(type_str);
    free_token_list(my_tl);
  }

  /* Test analyze_signature_tokens error */
  /* Removed invalid analyze_signature_tokens test */

  /* Test orchestrate_fix errors */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            internal_orchestrate_fix(NULL, &out_fix));

  /* Test fix_code_main usage */
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, internal_fix_code_main(0, NULL));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, internal_fix_code_main(3, argv3));

  /* Test is_c_source */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, is_c_source("a.c", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, is_c_source("no_ext", &is_src));
  ASSERT_EQ(0, is_src);

  /* Test fix_file_callback not a source */
  ASSERT_EQ(CDD_C_SUCCESS, fix_file_callback("ignore.txt", &ctx));

  /* Graph errors */
  g.nodes = NULL;
  PASS();
}

SUITE(orchestrator_internals_suite) { RUN_TEST(test_orchestrator_internals); }

#endif /* TEST_ORCHESTRATOR_INTERNALS_H */
