#ifndef TEST_C_CDD_INTEGRATION_H
#define TEST_C_CDD_INTEGRATION_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "fs.h"

/* Forward declare main from main.c?
   No, main is the entry point of the CLI exec.
   To test the logic inside main.c via unit tests, we usually expose
   the sub-functions or invoke the binary.
   Since we cannot easily invoke main() repeatedly due to global state or
   exit(), we will assume the logic implemented in fix_code_main
   matches the integration of components we test here.

   However, we *can* simulate the pipeline that main.c uses.
 */

#include "analysis.h"
#include "rewriter_body.h"
#include "tokenizer.h"

TEST test_integration_full_pipeline(void) {
  /* This tests the entire flow: Source -> Tokenize -> Analyze -> Rewrite ->
   * Source */

  const char *raw_source = "#include <stdlib.h>\n"
                           "void foo(void) {\n"
                           "  char *p = malloc(100);\n"
                           "  *p = 0;\n"
                           "}\n";

  struct TokenList *tokens = NULL;
  struct AllocationSiteList allocs = {0};
  char *final_output = NULL;
  int rc;

  /* 1. Tokenize */
  rc = tokenize(az_span_create_from_str((char *)raw_source), &tokens);
  ASSERT_EQ(0, rc);
  ASSERT(tokens != NULL);

  /* 2. Analyze */
  rc = find_allocations(tokens, &allocs);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, allocs.size); /* Should find 'p' */

  /* 3. Rewrite */
  /* Pass NULL/0 for funcs/count and transform, as we only test injection info
   * here */
  rc = rewrite_body(tokens, &allocs, NULL, 0, NULL, &final_output);
  ASSERT_EQ(0, rc);
  ASSERT(final_output != NULL);

  /* 4. Verify Content */
  /* We expect the injection of the check */
  {
    const char *expected_snippet = "if (!p) { return ENOMEM; }";
    if (!strstr(final_output, expected_snippet)) {
      fprintf(stderr, "Output missing check:\n%s\n", final_output);
      FAIL();
    }
  }

  /* Cleanup */
  free(final_output);
  allocation_site_list_free(&allocs);
  free_token_list(tokens);
  PASS();
}

TEST test_integration_file_io_wrapper(void) {
  /* Simulates the fs/main.c interaction */
  const char *in_file = "integ_in.c";
  const char *out_file = "integ_out.c";
  const char *content = "void f() { int *x = malloc(4); }";
  char *read_back = NULL;
  size_t sz;
  int rc;

  /* Write input */
  rc = write_to_file(in_file, content); /* cdd_helpers */
  ASSERT_EQ(0, rc);

  /* Read (simulating main.c step 1) */
  rc = read_to_file(in_file, "r", &read_back, &sz);
  ASSERT_EQ(0, rc);

  /* Validate */
  ASSERT_STR_EQ(content, read_back);
  free(read_back);

  /* Remove */
  remove(in_file);
  /* (Pretend logic ran) */
  /* Write (simulating main.c step 5) */
  rc = write_to_file(out_file, "Result");
  ASSERT_EQ(0, rc);
  remove(out_file);

  PASS();
}

SUITE(integration_suite) {
  RUN_TEST(test_integration_full_pipeline);
  RUN_TEST(test_integration_file_io_wrapper);
}

#endif /* TEST_C_CDD_INTEGRATION_H */
