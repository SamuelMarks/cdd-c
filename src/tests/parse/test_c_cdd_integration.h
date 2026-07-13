/**
 * @file test_c_cdd_integration.h
 * @brief Integration tests for C CDD parsing.
 */

#ifndef TEST_C_CDD_INTEGRATION_H
#define TEST_C_CDD_INTEGRATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include "classes/emit/schema_codegen.h"
#include "functions/emit/rewriter_body.h"
#include "functions/parse/analysis.h"
#include "functions/parse/audit.h"
#include "functions/parse/orchestrator.h"
#include "functions/parse/tokenizer.h"
#include "classes/parse/cdd_cst_parser.h"
#include "openapi/parse/openapi.h"


/* Integration of the full pipeline (Audit & Fix & Gen command simulation) */
/* clang-format on */

/**
 * @brief Test the tokenization, analysis, and rewriting pipeline on a single
 * string.
 *
 * Verifies that:
 * 1. Tokenizer parses valid C.
 * 2. Analysis finds the unchecked allocation.
 * 3. Rewriter successfully injects the safety check.
 */
/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
TEST test_integration_full_pipeline(void) {
  const char *raw_source = "#include <stdlib.h>\n"
                           /* LCOV_EXCL_STOP */
                           /* LCOV_EXCL_STOP */
                           ""
                           "void foo(void) {\n"
                           "  char * p = (char *)malloc(100);\n"
                           "  *p = 0;\n"
                           "}\n";

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  struct TokenList *tokens = NULL;
  struct AllocationSiteList allocs = {0};
  char *final_output = NULL;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  int rc;

  /* 1. Tokenize */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  rc = tokenize(az_span_create_from_str((char *)raw_source), &tokens);
  ASSERT_EQ(0, rc);
  ASSERT(tokens != NULL);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* 2. Analyze */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  rc = find_allocations(tokens, &allocs);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  ASSERT_EQ(1, allocs.size); /* Should find 'p' */

  /* 3. Rewrite */
  /* Pass NULL/0 for funcs/count and transform, as we only test injection info
   * here */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  rc = rewrite_body(tokens, &allocs, NULL, 0, NULL, &final_output);
  ASSERT_EQ(0, rc);
  ASSERT(final_output != NULL);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* 4. Verify Content */
  {
    /* Malloc safety injection */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    const char *expected_snippet = "if (!p) { return CDD_C_ERROR_MEMORY; }";
    if (!strstr(final_output, expected_snippet)) {
      fprintf(stderr, "Output missing check:\n%s\n", final_output);
      FAIL();
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    }
  }

  /* Cleanup */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  free(final_output);
  allocation_site_list_free(&allocs);
  free_token_list(tokens);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Test manual file I/O integration with the fix command logic.
 * Ensures `fix_code_main` can read, process, and write back a single file.
 */
/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
TEST test_integration_fix_file_io(void) {
  const char *in_file = "integ_in.c";
  const char *out_file = "integ_out.c";
  const char *content = ""
                        /* LCOV_EXCL_STOP */
                        /* LCOV_EXCL_STOP */
                        "void f() { int * x = (int *)malloc(4); }";
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  char *read_back = NULL;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  size_t sz;
  int rc;

  /* 1. Write Input */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  rc = write_to_file(in_file, content);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* 2. Call Orchestrator Main (Fix Command) */
  {
    char *argv[2];
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    argv[0] = (char *)in_file;
    argv[1] = (char *)out_file;
    rc = fix_code_main(2, argv);
    ASSERT_EQ(0, rc);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* 3. Verify Output */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  rc = read_to_file(out_file, "r", &read_back, &sz);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  ASSERT(strstr(read_back, "return CDD_C_ERROR_MEMORY") != NULL);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  free(read_back);
  remove(in_file);
  remove(out_file);
  g_fail_io_after = -1;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  PASS();
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Test the recursive folder traversal of the fix command.
 * Verifies that multiple files in a directory tree are processed.
 */
/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
TEST test_integration_recursive_fix(void) {
  char *sys_tmp = NULL;
  char *root = NULL;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  int rc;

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  tempdir(&sys_tmp);
  asprintf(&root, "%s%sfix_rec_test_%d", sys_tmp, PATH_SEP, rand());
  makedirs(root);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  {
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    char *sub = NULL;
    char *f1 = NULL;
    char *f2 = NULL;
    char *content = NULL;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    size_t sz;

    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    asprintf(&sub, "%s%ssub", root, PATH_SEP);
    makedirs(sub);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */

    /* File in root */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    asprintf(&f1, "%s%sa.c", root, PATH_SEP);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* Must provide variable 'p' for safety check to attach to */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    write_to_file(f1, ""
                      /* LCOV_EXCL_STOP */
                      /* LCOV_EXCL_STOP */
                      "void a() { void * p = (void *)malloc(1); }");

    /* File in sub */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    asprintf(&f2, "%s%sb.c", sub, PATH_SEP);
    write_to_file(f2, ""
                      /* LCOV_EXCL_STOP */
                      /* LCOV_EXCL_STOP */
                      "void b() { void * p = (void *)malloc(1); }");

    /* Call fix */
    {
      char *argv[2];
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      argv[0] = root;
      argv[1] = "--in-place";
      rc = fix_code_main(2, argv);
      ASSERT_EQ(0, rc);
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    }

    /* Verify f1 */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    rc = read_to_file(f1, "r", &content, &sz);
    ASSERT_EQ(0, rc);
    ASSERT(strstr(content, "CDD_C_ERROR_MEMORY") != NULL);
    free(content);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */

    /* Verify f2 */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    rc = read_to_file(f2, "r", &content, &sz);
    ASSERT_EQ(0, rc);
    ASSERT(strstr(content, "CDD_C_ERROR_MEMORY") != NULL);
    free(content);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    remove(f2);
    remove(f1);
    rmdir(sub);
    free(f2);
    free(f1);
    free(sub);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  rmdir(root);
  free(root);
  free(sys_tmp);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Test the `--in-place` flag on a single file.
 */
/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
TEST test_integration_fix_file_in_place(void) {
  const char *in_file = "inplace.c";
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  /* Must provide variable 'p' for safety check to attach to */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  const char *content = ""
                        /* LCOV_EXCL_STOP */
                        /* LCOV_EXCL_STOP */
                        "void f() { void * p = (void *)malloc(1); }";
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  char *read_back = NULL;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  size_t sz;
  int rc;

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  write_to_file(in_file, content);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  {
    char *argv[2];
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    argv[0] = (char *)in_file;
    argv[1] = "--in-place";
    rc = fix_code_main(2, argv);
    ASSERT_EQ(0, rc);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  rc = read_to_file(in_file, "r", &read_back, &sz);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(read_back, "CDD_C_ERROR_MEMORY") != NULL);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  free(read_back);
  remove(in_file);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Test that `fix` fails if a directory is passed without `--in-place`.
 */
/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
TEST test_integration_fix_dir_error_no_flag(void) {
  char *sys_tmp = NULL;
  char *root = NULL;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  int rc;

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  tempdir(&sys_tmp);
  asprintf(&root, "%s%sfix_err_test_%d", sys_tmp, PATH_SEP, rand());
  makedir(root);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  {
    char *argv[1];
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    argv[0] = root;
    rc = fix_code_main(1, argv);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  rmdir(root);
  free(root);
  free(sys_tmp);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
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
/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
TEST test_end_to_end_project_lifecycle(void) {
  char *sys_tmp = NULL;
  char *project_root = NULL;
  char *src_c = NULL;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  int rc;

  /* 1. Setup Project Environment */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  tempdir(&sys_tmp);
  asprintf(&project_root, "%s%scdd_project_%d", sys_tmp, PATH_SEP, rand());
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  /* Use makedirs to safely handle existence/parent issues */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (makedirs(project_root) != 0) {
    free(sys_tmp);
    free(project_root);
    FAILm("Failed to create project root");
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  asprintf(&src_c, "%s%ssrc.c", project_root, PATH_SEP);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /*
     Single file containing callee and caller.
     Alloc returns raw ptr. Caller consumes it.
  */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  write_to_file(src_c,
                /* LCOV_EXCL_STOP */
                /* LCOV_EXCL_STOP */
                "#include <stdlib.h>\n"
                "char* make_data() { return malloc(10); }\n"
                ""
                "void process_data() { char *d = make_data(); *d = 1; }");

  /* 2. Initial Audit: Expect Violations */
  {
    struct AuditStats stats;
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    (void)audit_stats_init(&stats);
    rc = audit_project(project_root, &stats);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    ASSERT_EQ(0, rc);                          /* Audit tool ran successfully */
    ASSERT_EQ(1, stats.allocations_unchecked); /* malloc in make_data */
    /* Note: make_data is also a function_returning_alloc */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    ASSERT_EQ(1, stats.functions_returning_alloc);
    audit_stats_free(&stats);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* 3. Execute Fix */
  {
    char *argv[2];
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    argv[0] = project_root;
    argv[1] = "--in-place";
    rc = fix_code_main(2, argv);
    ASSERT_EQ(0, rc);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* 4. Verification Audit: Expect Clean */
  {
    struct AuditStats stats;
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    (void)audit_stats_init(&stats);
    rc = audit_project(project_root, &stats);
    ASSERT_EQ(0, rc);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* Failure point previously: allocations_unchecked should be 0 */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    ASSERT_EQ(0, stats.allocations_unchecked);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* The newly injected check in the return rewrite should be counted as
     * checked */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    ASSERT_EQ(1, stats.allocations_checked);
    audit_stats_free(&stats);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* 5. Source Code Inspection */
  {
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    char *content = NULL;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    size_t sz;

    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    rc = read_to_file(src_c, "r", &content, &sz);
    ASSERT_EQ(0, rc);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* Check signature change - Relaxed check due to whitespace vagaries */
    /* Should be int make_data(..., ... *out) */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    ASSERT(strstr(content, "int make_data(") != NULL);
    ASSERT(strstr(content, "*out)") != NULL);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */

    /* Check safety injection */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    ASSERT(strstr(content, "return CDD_C_ERROR_MEMORY;") != NULL);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */

    /* Verify process_data Propagation */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    ASSERT(strstr(content, "make_data(&d)") != NULL);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
    /* Check error propagation: `if (rc != 0) return rc` implies function
     * signature change to int */
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    ASSERT(strstr(content, "int process_data()") != NULL);
    ASSERT(strstr(content, "enum cdd_c_error rc") != NULL);
    ASSERT(strstr(content, "return rc;") != NULL);
    free(content);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* Teardown */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  remove(src_c);
  rmdir(project_root);
  free(src_c);
  free(project_root);
  free(sys_tmp);
  g_fail_io_after = -1;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  PASS();
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
TEST test_integration_schema2code_with_guards(void) {
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  /*
   * Tests:
   *  ./cli schema2code integ_guard.json integ_guard_out \
   *    --guard-json=ENABLE_JSON --guard-utils=DATA_UTILS
   */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  const char *schema_file = "integ_guard.json";
  const char *base_name = "integ_guard_out";
  char *header_file = "integ_guard_out.h";
  char *source_file = "integ_guard_out.c";
  char *param1 = "--guard-json=ENABLE_JSON";
  char *param2 = "--guard-utils=DATA_UTILS";
  char *content = NULL;
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  size_t sz;
  int rc;

  /* 1. Setup */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  write_to_file(schema_file,
                /* LCOV_EXCL_STOP */
                /* LCOV_EXCL_STOP */
                "{\"components\":{\"schemas\":{\"S\":{\"type\":\"object\"}}}}");

  /* 2. Run */
  {
    char *argv[4];
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    argv[0] = (char *)schema_file;
    argv[1] = (char *)base_name;
    argv[2] = param1;
    argv[3] = param2;
    rc = schema2code_main(4, argv);
    ASSERT_EQ(0, rc);
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_STOP */
  }

  /* 3. Verify Header */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  rc = read_to_file(header_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(content, "#ifdef ENABLE_JSON") != NULL);
  ASSERT(strstr(content, "enum cdd_c_error S_to_json(") != NULL);
  ASSERT(strstr(content, "#ifdef DATA_UTILS") != NULL);
  ASSERT(strstr(content, "enum cdd_c_error S_cleanup(") != NULL);
  free(content);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* 4. Verify Source */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  rc = read_to_file(source_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(content, "#ifdef ENABLE_JSON") != NULL);
  ASSERT(strstr(content, "#ifdef DATA_UTILS") != NULL);
  free(content);
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* Cleanup */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  remove(schema_file);
  remove(header_file);
  remove(source_file);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
/* LCOV_EXCL_START */
SUITE(integration_suite) {
  RUN_TEST(test_integration_full_pipeline);
  RUN_TEST(test_integration_fix_file_io);
  RUN_TEST(test_integration_recursive_fix);
  RUN_TEST(test_integration_fix_file_in_place);
  RUN_TEST(test_integration_fix_dir_error_no_flag);
  RUN_TEST(test_end_to_end_project_lifecycle);
  RUN_TEST(test_integration_schema2code_with_guards);
}
/* LCOV_EXCL_STOP */
/* LCOV_EXCL_STOP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C_CDD_INTEGRATION_H */
