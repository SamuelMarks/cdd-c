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
  ASSERT_EQ(1, rc);

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
  char *argv_help2[] = {"extern_c", "--help", NULL};
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
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_help2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cli_cst_transformer_main(2, argv_nofix));
  ASSERT_EQ(1, cli_cst_transformer_main(3, argv_badfile));

  /* Hit branches for other tools */
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_msvc));
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_gnu));
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_percolate));
  ASSERT_EQ(0, cli_cst_transformer_main(2, argv_safe));
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
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_CST_H */
