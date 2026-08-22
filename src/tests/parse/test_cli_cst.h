/**
 * @file test_cli_cst.h
 * @brief Unit tests for CST CLI transformer routing.
 */

#ifndef TEST_CLI_CST_H
#define TEST_CLI_CST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#if defined(__unix__) || defined(__APPLE__) || defined(__linux__) || defined(__MACH__)
#include <sys/stat.h>
#endif
#include "routes/parse/cli_cst.h"
#include "functions/parse/fs.h"
#include "cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

/**
 * @brief Tests extern C transformer in audit mode via CLI.
 *
 * @return The result of the test.
 */
TEST test_cli_cst_extern_c_audit(void) {
  int argc = 4;
  char *argv[] = {"extern_c", "--audit", "test_cli_cst_file.h", NULL};
  int rc;
  const char *content = "void foo();";

  write_to_file("test_cli_cst_file.h", content);

  /* Audit should fail because it needs extern "C" */
  rc = cli_cst_transformer_main(argc - 1, argv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  remove("test_cli_cst_file.h");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests extern C transformer in fix mode via CLI.
 *
 * @return The result of the test.
 */
TEST test_cli_cst_extern_c_fix(void) {
  int argc = 4;
  char *argv[] = {"extern_c", "--fix", "test_cli_cst_file.h", NULL};
  int rc;
  const char *content = "void foo();";

  write_to_file("test_cli_cst_file.h", content);

  /* Fix should succeed */
  rc = cli_cst_transformer_main(argc - 1, argv);
  ASSERT_EQ(0, rc);

  /* Audit should succeed on already fixed file */
  char *argv_audit[] = {"extern_c", "--audit", "test_cli_cst_file.h", NULL};
  rc = cli_cst_transformer_main(3, argv_audit);
  ASSERT_EQ(0, rc);

  /* Fix again on the already fixed file. It should match exactly and hit the no
   * changes else block */
  rc = cli_cst_transformer_main(argc - 1, argv);
  ASSERT_EQ(0, rc);

  remove("test_cli_cst_file.h");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests extern C transformer in dry-run mode via CLI.
 *
 * @return The result of the test.
 */
TEST test_cli_cst_extern_c_dry_run(void) {
  int argc = 5;
  char *argv[] = {"extern_c", "--fix", "--dry-run", "test_cli_cst_file.h",
                  NULL};
  int rc;
  const char *content = "void foo();";

  write_to_file("test_cli_cst_file.h", content);

  rc = cli_cst_transformer_main(argc - 1, argv);
  ASSERT_EQ(0, rc);
  /* Test dry-run with no changes needed */
  content = "#ifdef __cplusplus\nextern \"C\" {\n#endif\nvoid foo();\n#ifdef "
            "__cplusplus\n}\n#endif\n";
  write_to_file("test_cli_cst_file.h", content);
  rc = cli_cst_transformer_main(argc - 1, argv);
  ASSERT_EQ(0, rc);

  remove("test_cli_cst_file.h");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests error handling of the CST CLI router.
 *
 * @return The result of the test.
 */
TEST test_cli_cst_errors(void) {
  char *argv_no_args[] = {NULL};
  char *argv_unknown[] = {"unknown_tool", NULL};
  char *argv_help1[] = {"--help", NULL};
  char *argv_help1b[] = {"-h", NULL};
  char *argv_help2[] = {"extern_c", "--help", NULL};
  char *argv_help2b[] = {"extern_c", "-h", NULL};
  char *argv_nofix[] = {"extern_c", "file.h", NULL};
  char *argv_badfile[] = {"extern_c", "--fix", "does_not_exist_file.h", NULL};
  char *argv_msvc[] = {"msvc_port", "--help", NULL};
  char *argv_gnu[] = {"gnu_standardizer", "--help", NULL};
  char *argv_percolate[] = {"error_percolator", "--help", NULL};
  char *argv_safe[] = {"safe_crt", "--help", NULL};

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cli_cst_transformer_main(0, argv_no_args));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cli_cst_transformer_main(1, argv_unknown));
  ASSERT_EQ(0, cli_cst_transformer_main(1, argv_help1));
  ASSERT_EQ(0, cli_cst_transformer_main(1, argv_help1b));
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_help2));
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_help2b));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cli_cst_transformer_main(2, argv_nofix));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cli_cst_transformer_main(3, argv_badfile));

  /* Hit branches for other tools */
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_msvc));
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_gnu));
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_percolate));
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_safe));
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests standardize gnu via CLI.
 *
 * @return The result of the test.
 */
TEST test_cli_standardize_gnu(void) {
  int argc = 7;
  char *argv[] = {
      "--target-c89", "--target-c99", "--fallback-alloca", "--audit",
      "--fix",        "--dry-run",    "test_gnu_file.h",   NULL};
  int rc;
  const char *content = "void foo();";

  /* Test no args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cli_standardize_gnu_main(0, argv));

  /* Test help */
  char *argv_help[] = {"--help"};
  char *argv_help2[] = {"-h"};
  ASSERT_EQ(0, cli_standardize_gnu_main(1, argv_help));
  ASSERT_EQ(0, cli_standardize_gnu_main(1, argv_help2));

  /* Test no file specified or bad file */
  char *argv_nofile[] = {"--audit"};
  ASSERT_EQ(0, cli_standardize_gnu_main(1, argv_nofile));

  /* Test missing audit/fix */
  char *argv_missing[] = {"test_gnu_file.h"};
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cli_standardize_gnu_main(1, argv_missing));

  write_to_file("test_gnu_file.h", content);

  /* Test valid args */
  rc = cli_standardize_gnu_main(7, argv);
  ASSERT_EQ(0, rc);

  char *argv_fixonly[] = {"--fix", "test_gnu_file.h", NULL};
  ASSERT_EQ(0, cli_standardize_gnu_main(2, argv_fixonly));

  /* Test unknown flag */
  char *argv_unknown_flag[] = {"--audit", "--unknown-flag", "test_gnu_file.h",
                               NULL};
  rc = cli_standardize_gnu_main(3, argv_unknown_flag);
  ASSERT_EQ(0, rc);

  remove("test_gnu_file.h");

  /* Test file not found */
  char *argv_notfound[] = {"--audit", "not_found.h"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cli_standardize_gnu_main(2, argv_notfound));

  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
#include <c_cdd_export.h>

/* Moved extern declarations for C89 compliance */
extern int g_cdd_cst_emit_realloc_fail;
extern int g_cdd_alloc_fail;
/* extern int g_cdd_alloc_fail; (moved to global) */
#endif

/**
 * @brief Tests error paths in process_file.
 *
 * @return The result of the test.
 */
TEST test_cli_cst_process_errors(void) {
  char *argv_audit[] = {"extern_c", "--audit", "test_cli_cst_file.h", NULL};
  char *argv_fix[] = {"extern_c", "--fix", "test_cli_cst_file.h", NULL};
  const char *content = "void foo();";

  write_to_file("test_cli_cst_file.h", content);

#ifdef CDD_BUILD_TESTS
  {
    /* extern int g_cdd_cst_emit_realloc_fail; (moved to global) */
    int i;
    for (i = 1; i <= 500; i++) {
      g_cdd_alloc_fail = i;
      cli_cst_transformer_main(3, argv_fix);
    }
    g_cdd_alloc_fail = 0;

    for (i = 1; i <= 500; i++) {
      g_cdd_cst_alloc_node_fail = i;
      cli_cst_transformer_main(3, argv_fix);
    }
    g_cdd_cst_alloc_node_fail = 0;

    for (i = 1; i <= 500; i++) {
      g_cdd_cst_alloc_node_fail = i;
      cli_cst_transformer_main(3, argv_fix);
    }
    g_cdd_cst_alloc_node_fail = 0;

    for (i = 1; i <= 500; i++) {
      g_cdd_cst_emit_realloc_fail = i;
      cli_cst_transformer_main(3, argv_fix);
    }
    g_cdd_cst_emit_realloc_fail = 0;
  }
#endif

  /* Output file open failure in fix mode */
  /* By creating a directory with the same name, fopen for write will fail */
  remove("test_cli_cst_file.h");
  makedir("test_cli_cst_file.h");
  write_to_file("test_cli_cst_file.h/foo", content); /* just something */

  /* The tool opens it for read first. But it's a directory, so read fails?
   * If read succeeds but write fails, we hit the write error.
   * On UNIX, fopen a dir for "rb" fails or succeeds? Usually fails.
   * If we want to simulate parse error, we can write invalid C, but our parser
   * is resilient and probably parses it anyway. Let's try read-only file for
   * write failure. */
  remove("test_cli_cst_file.h/foo");
  remove("test_cli_cst_file.h");

  write_to_file("test_cli_cst_file.h", content);
#if defined(__unix__) || defined(__APPLE__) || defined(__linux__) ||           \
    defined(__MACH__)
  chmod("test_cli_cst_file.h", 0400); /* read-only */
  /* It needs to be fixed to attempt writing */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cli_cst_transformer_main(3, argv_fix));
  chmod("test_cli_cst_file.h", 0600); /* restore to delete */
#endif

  remove("test_cli_cst_file.h");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief CLI CST router test suite.
 */
SUITE(cli_cst_suite) {
  RUN_TEST(test_cli_cst_extern_c_audit);
  RUN_TEST(test_cli_cst_extern_c_fix);
  RUN_TEST(test_cli_cst_extern_c_dry_run);
  RUN_TEST(test_cli_cst_errors);
  RUN_TEST(test_cli_standardize_gnu);
  RUN_TEST(test_cli_cst_process_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_CST_H */
