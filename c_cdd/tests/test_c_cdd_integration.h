#ifndef TEST_C_CDD_INTEGRATION_H
#define TEST_C_CDD_INTEGRATION_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "fs.h"

/* Integration of the full pipeline (Fix command simulation) */
#include "analysis.h"
#include "refactor_orchestrator.h"
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
  {
    /* Malloc safety injection */
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

TEST test_integration_fix_file_io(void) {
  /* Simulates the fs/main.c interaction for the 'fix' command logic */
  const char *in_file = "integ_in.c";
  const char *out_file = "integ_out.c";
  const char *content = "void f() { int *x = malloc(4); }";
  char *read_back = NULL;
  size_t sz;
  int rc;

  /* 1. Write Input */
  rc = write_to_file(in_file, content);
  ASSERT_EQ(0, rc);

  /* 2. Call Orchestrator Main (Fix Command) */
  {
    char *argv[2];
    argv[0] = (char *)in_file;
    argv[1] = (char *)out_file;
    rc = fix_code_main(2, argv);
    ASSERT_EQ(0, rc);
  }

  /* 3. Verify Output */
  rc = read_to_file(out_file, "r", &read_back, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(read_back, "return ENOMEM") != NULL);

  free(read_back);
  remove(in_file);
  remove(out_file);

  PASS();
}

SUITE(integration_suite) {
  RUN_TEST(test_integration_full_pipeline);
  RUN_TEST(test_integration_fix_file_io);
}

#endif /* TEST_C_CDD_INTEGRATION_H */
