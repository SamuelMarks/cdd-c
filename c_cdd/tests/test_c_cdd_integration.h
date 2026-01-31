#ifndef TEST_C_CDD_INTEGRATION_H
#define TEST_C_CDD_INTEGRATION_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "fs.h"

/* Integration of the full pipeline (Audit & Fix & Gen command simulation) */
#include "analysis.h"
#include "project_audit.h"
#include "refactor_orchestrator.h"
#include "rewriter_body.h"
#include "schema_codegen.h"
#include "tokenizer.h"

/**
 * @brief Test the tokenization, analysis, and rewriting pipeline on a single
 * string.
 *
 * Verifies that:
 * 1. Tokenizer parses valid C.
 * 2. Analysis finds the unchecked allocation.
 * 3. Rewriter successfully injects the safety check.
 */
TEST test_integration_full_pipeline(void) {
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

/**
 * @brief Test manual file I/O integration with the fix command logic.
 * Ensures `fix_code_main` can read, process, and write back a single file.
 */
TEST test_integration_fix_file_io(void) {
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

/**
 * @brief Test the recursive folder traversal of the fix command.
 * Verifies that multiple files in a directory tree are processed.
 */
TEST test_integration_recursive_fix(void) {
  char *sys_tmp = NULL;
  char *root = NULL;
  int rc;

  tempdir(&sys_tmp);
  asprintf(&root, "%s%sfix_rec_test_%d", sys_tmp, PATH_SEP, rand());
  makedir(root);

  {
    char *sub = NULL;
    char *f1 = NULL;
    char *f2 = NULL;
    char *content;
    size_t sz;

    asprintf(&sub, "%s%ssub", root, PATH_SEP);
    makedir(sub);

    /* File in root */
    asprintf(&f1, "%s%sa.c", root, PATH_SEP);
    /* Must provide variable 'p' for safety check to attach to */
    write_to_file(f1, "void a() { void *p = malloc(1); }");

    /* File in sub */
    asprintf(&f2, "%s%sb.c", sub, PATH_SEP);
    write_to_file(f2, "void b() { void *p = malloc(1); }");

    /* Call fix */
    {
      char *argv[2];
      argv[0] = root;
      argv[1] = "--in-place";
      rc = fix_code_main(2, argv);
      ASSERT_EQ(0, rc);
    }

    /* Verify f1 */
    rc = read_to_file(f1, "r", &content, &sz);
    ASSERT_EQ(0, rc);
    ASSERT(strstr(content, "ENOMEM") != NULL);
    free(content);

    /* Verify f2 */
    rc = read_to_file(f2, "r", &content, &sz);
    ASSERT_EQ(0, rc);
    ASSERT(strstr(content, "ENOMEM") != NULL);
    free(content);

    remove(f2);
    remove(f1);
    rmdir(sub);
    free(f2);
    free(f1);
    free(sub);
  }

  rmdir(root);
  free(root);
  free(sys_tmp);
  PASS();
}

/**
 * @brief Test the `--in-place` flag on a single file.
 */
TEST test_integration_fix_file_in_place(void) {
  const char *in_file = "inplace.c";
  /* Must provide variable 'p' for safety check to attach to */
  const char *content = "void f() { void *p = malloc(1); }";
  char *read_back = NULL;
  size_t sz;
  int rc;

  write_to_file(in_file, content);

  {
    char *argv[2];
    argv[0] = (char *)in_file;
    argv[1] = "--in-place";
    rc = fix_code_main(2, argv);
    ASSERT_EQ(0, rc);
  }

  rc = read_to_file(in_file, "r", &read_back, &sz);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(read_back, "ENOMEM") != NULL);

  free(read_back);
  remove(in_file);
  PASS();
}

/**
 * @brief Test that `fix` fails if a directory is passed without `--in-place`.
 */
TEST test_integration_fix_dir_error_no_flag(void) {
  char *sys_tmp = NULL;
  char *root = NULL;
  int rc;

  tempdir(&sys_tmp);
  asprintf(&root, "%s%sfix_err_test_%d", sys_tmp, PATH_SEP, rand());
  makedir(root);

  {
    char *argv[1];
    argv[0] = root;
    rc = fix_code_main(1, argv);
    ASSERT_EQ(EXIT_FAILURE, rc);
  }

  rmdir(root);
  free(root);
  free(sys_tmp);
  PASS();
}

/**
 * @brief End-to-End Simulation of the Audit -> Fix -> Verify workflow.
 *
 * Simulates a mini-project.
 * NOTE: The tool currently processes files individually (per-compilation-unit).
 * To verify call-site propagation (Callee -> Caller), both functions must be
 * in the same file for the Dependency Graph to connect them.
 *
 * Steps:
 * 1. Audit the project -> Expect violations.
 * 2. Run Repair (`fix`) in-place.
 * 3. Audit again -> Expect 0 violations.
 * 4. Code Inspection -> Verify signatures changed and error codes checking
 * injected.
 */
TEST test_end_to_end_project_lifecycle(void) {
  char *sys_tmp = NULL;
  char *project_root = NULL;
  char *src_c = NULL;
  int rc;

  /* 1. Setup Project Environment */
  tempdir(&sys_tmp);
  asprintf(&project_root, "%s%scdd_project_%d", sys_tmp, PATH_SEP, rand());
  /* Use makedirs to safely handle existence/parent issues */
  if (makedirs(project_root) != 0) {
    free(sys_tmp);
    free(project_root);
    FAILm("Failed to create project root");
  }

  asprintf(&src_c, "%s%ssrc.c", project_root, PATH_SEP);

  /*
     Single file containing callee and caller.
     Alloc returns raw ptr. Caller consumes it.
  */
  write_to_file(src_c,
                "#include <stdlib.h>\n"
                "char* make_data() { return malloc(10); }\n"
                "void process_data() { char *d = make_data(); *d = 1; }");

  /* 2. Initial Audit: Expect Violations */
  {
    struct AuditStats stats;
    audit_stats_init(&stats);
    rc = audit_project(project_root, &stats);
    ASSERT_EQ(0, rc);                          /* Audit tool ran successfully */
    ASSERT_EQ(1, stats.allocations_unchecked); /* malloc in make_data */
    /* Note: make_data is also a function_returning_alloc */
    ASSERT_EQ(1, stats.functions_returning_alloc);
    audit_stats_free(&stats);
  }

  /* 3. Execute Fix */
  {
    char *argv[2];
    argv[0] = project_root;
    argv[1] = "--in-place";
    rc = fix_code_main(2, argv);
    ASSERT_EQ(0, rc);
  }

  /* 4. Verification Audit: Expect Clean */
  {
    struct AuditStats stats;
    audit_stats_init(&stats);
    rc = audit_project(project_root, &stats);
    ASSERT_EQ(0, rc);
    /* Failure point previously: allocations_unchecked should be 0 */
    ASSERT_EQ(0, stats.allocations_unchecked);
    /* The newly injected check in the return rewrite should be counted as
     * checked */
    ASSERT_EQ(1, stats.allocations_checked);
    audit_stats_free(&stats);
  }

  /* 5. Source Code Inspection */
  {
    char *content = NULL;
    size_t sz;

    rc = read_to_file(src_c, "r", &content, &sz);
    ASSERT_EQ(0, rc);
    /* Check signature change - Relaxed check due to whitespace vagaries */
    /* Should be int make_data(..., ... *out) */
    ASSERT(strstr(content, "int make_data(") != NULL);
    ASSERT(strstr(content, "*out)") != NULL);

    /* Check safety injection */
    ASSERT(strstr(content, "return ENOMEM;") != NULL);

    /* Verify process_data Propagation */
    ASSERT(strstr(content, "make_data(&d)") != NULL);
    /* Check error propagation: `if (rc != 0) return rc` implies function
     * signature change to int */
    ASSERT(strstr(content, "int process_data()") != NULL);
    ASSERT(strstr(content, "int rc") != NULL);
    ASSERT(strstr(content, "return rc;") != NULL);
    free(content);
  }

  /* Teardown */
  remove(src_c);
  rmdir(project_root);
  free(src_c);
  free(project_root);
  free(sys_tmp);

  PASS();
}

TEST test_integration_schema2code_with_guards(void) {
  /*
   * Tests:
   *  ./cli schema2code integ_guard.json integ_guard_out \
   *    --guard-json=ENABLE_JSON --guard-utils=DATA_UTILS
   */
  const char *schema_file = "integ_guard.json";
  const char *base_name = "integ_guard_out";
  char *header_file = "integ_guard_out.h";
  char *source_file = "integ_guard_out.c";
  char *param1 = "--guard-json=ENABLE_JSON";
  char *param2 = "--guard-utils=DATA_UTILS";
  char *content;
  size_t sz;
  int rc;

  /* 1. Setup */
  write_to_file(schema_file,
                "{\"components\":{\"schemas\":{\"S\":{\"type\":\"object\"}}}}");

  /* 2. Run */
  {
    char *argv[4];
    argv[0] = (char *)schema_file;
    argv[1] = (char *)base_name;
    argv[2] = param1;
    argv[3] = param2;
    rc = schema2code_main(4, argv);
    ASSERT_EQ(0, rc);
  }

  /* 3. Verify Header */
  rc = read_to_file(header_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(content, "#ifdef ENABLE_JSON") != NULL);
  ASSERT(strstr(content, "int S_to_json(") != NULL);
  ASSERT(strstr(content, "#ifdef DATA_UTILS") != NULL);
  ASSERT(strstr(content, "void S_cleanup(") != NULL);
  free(content);

  /* 4. Verify Source */
  rc = read_to_file(source_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(content, "#ifdef ENABLE_JSON") != NULL);
  ASSERT(strstr(content, "#ifdef DATA_UTILS") != NULL);
  free(content);

  /* Cleanup */
  remove(schema_file);
  remove(header_file);
  remove(source_file);
  PASS();
}

SUITE(integration_suite) {
  RUN_TEST(test_integration_full_pipeline);
  RUN_TEST(test_integration_fix_file_io);
  RUN_TEST(test_integration_recursive_fix);
  RUN_TEST(test_integration_fix_file_in_place);
  RUN_TEST(test_integration_fix_dir_error_no_flag);
  RUN_TEST(test_end_to_end_project_lifecycle);
  RUN_TEST(test_integration_schema2code_with_guards);
}

#endif /* TEST_C_CDD_INTEGRATION_H */
