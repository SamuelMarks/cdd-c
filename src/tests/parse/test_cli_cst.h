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
  char *argv[4];
  int rc;

  argv[0] = cdd_test_mut_str("extern_c");
  argv[1] = cdd_test_mut_str("--audit");
  argv[2] = cdd_test_mut_str("test_cli_cst_file.h");
  argv[3] = NULL;
  /* void foo */

  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_cli_cst_file.h", "void foo();"));

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
  char *argv[4];
  int rc;

  argv[0] = cdd_test_mut_str("extern_c");
  argv[1] = cdd_test_mut_str("--fix");
  argv[2] = cdd_test_mut_str("test_cli_cst_file.h");
  argv[3] = NULL;
  /* void foo */

  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_cli_cst_file.h", "void foo();"));

  /* Fix should succeed */
  rc = cli_cst_transformer_main(argc - 1, argv);
  ASSERT_EQ(0, rc);

  /* Audit on modified file */
  {
    char *argv_audit[4];

    argv_audit[0] = cdd_test_mut_str("extern_c");
    argv_audit[1] = cdd_test_mut_str("--audit");
    argv_audit[2] = cdd_test_mut_str("test_cli_cst_file.h");
    argv_audit[3] = NULL;
    rc = cli_cst_transformer_main(3, argv_audit);
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_NOT_FOUND);

    /* Fix again on the already fixed file. It should match exactly and hit the
     * no changes else block */
    rc = cli_cst_transformer_main(argc - 1, argv);
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_NOT_FOUND);

    remove("test_cli_cst_file.h");
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Tests extern C transformer in dry-run mode via CLI.
 *
 * @return The result of the test.
 */
TEST test_cli_cst_extern_c_dry_run(void) {
  int argc = 5;
  char *argv[5];
  int rc;

  argv[0] = cdd_test_mut_str("extern_c");
  argv[1] = cdd_test_mut_str("--fix");
  argv[2] = cdd_test_mut_str("--dry-run");
  argv[3] = cdd_test_mut_str("test_cli_cst_file.h");
  argv[4] = NULL;
  /* void foo */

  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_cli_cst_file.h", "void foo();"));

  rc = cli_cst_transformer_main(argc - 1, argv);
  ASSERT_EQ(0, rc);
  /* Test dry-run with no changes needed */
  /* content */
  /* void foo */
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_cli_cst_file.h", "void foo();"));
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
  char *argv_unknown[2];
  char *argv_help1[2];
  char *argv_help1b[2];
  char *argv_help2[3];
  char *argv_help2b[3];
  char *argv_nofix[3];
  char *argv_badfile[4];
  char *argv_msvc[3];
  char *argv_gnu[3];
  char *argv_percolate[3];
  char *argv_safe[3];

  argv_unknown[0] = cdd_test_mut_str("unknown_tool");
  argv_unknown[1] = NULL;
  argv_help1[0] = cdd_test_mut_str("--help");
  argv_help1[1] = NULL;
  argv_help1b[0] = cdd_test_mut_str("-h");
  argv_help1b[1] = NULL;
  argv_help2[0] = cdd_test_mut_str("extern_c");
  argv_help2[1] = cdd_test_mut_str("--help");
  argv_help2[2] = NULL;
  argv_help2b[0] = cdd_test_mut_str("extern_c");
  argv_help2b[1] = cdd_test_mut_str("-h");
  argv_help2b[2] = NULL;
  argv_nofix[0] = cdd_test_mut_str("extern_c");
  argv_nofix[1] = cdd_test_mut_str("file.h");
  argv_nofix[2] = NULL;
  argv_badfile[0] = cdd_test_mut_str("extern_c");
  argv_badfile[1] = cdd_test_mut_str("--fix");
  argv_badfile[2] = cdd_test_mut_str("does_not_exist_file.h");
  argv_badfile[3] = NULL;
  argv_msvc[0] = cdd_test_mut_str("msvc_port");
  argv_msvc[1] = cdd_test_mut_str("--help");
  argv_msvc[2] = NULL;
  argv_gnu[0] = cdd_test_mut_str("gnu_standardizer");
  argv_gnu[1] = cdd_test_mut_str("--help");
  argv_gnu[2] = NULL;
  argv_percolate[0] = cdd_test_mut_str("error_percolator");
  argv_percolate[1] = cdd_test_mut_str("--help");
  argv_percolate[2] = NULL;
  argv_safe[0] = cdd_test_mut_str("safe_crt");
  argv_safe[1] = cdd_test_mut_str("--help");
  argv_safe[2] = NULL;
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
  char *argv[8];
  int rc;

  argv[0] = cdd_test_mut_str("--target-c89");
  argv[1] = cdd_test_mut_str("--target-c99");
  argv[2] = cdd_test_mut_str("--fallback-alloca");
  argv[3] = cdd_test_mut_str("--audit");
  argv[4] = cdd_test_mut_str("--fix");
  argv[5] = cdd_test_mut_str("--dry-run");
  argv[6] = cdd_test_mut_str("test_gnu_file.h");
  argv[7] = NULL;
  /* void foo */

  /* Test no args */
  (void)argc;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cli_standardize_gnu_main(0, argv));

  /* Test help */
  {
    char *argv_help[1];
    char *argv_help2[1];

    argv_help[0] = cdd_test_mut_str("--help");
    argv_help2[0] = cdd_test_mut_str("-h");
    ASSERT_EQ(0, cli_standardize_gnu_main(1, argv_help));
    ASSERT_EQ(0, cli_standardize_gnu_main(1, argv_help2));

    /* Test no file specified or bad file */
    {
      char *argv_nofile[1];

      argv_nofile[0] = cdd_test_mut_str("--audit");
      ASSERT_EQ(0, cli_standardize_gnu_main(1, argv_nofile));

      /* Test missing audit/fix */
      {
        char *argv_missing[1];

        argv_missing[0] = cdd_test_mut_str("test_gnu_file.h");
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cli_standardize_gnu_main(1, argv_missing));

        write_to_file("test_gnu_file.h", "void foo();");

        /* Test valid args */
        rc = cli_standardize_gnu_main(7, argv);
        ASSERT_EQ(0, rc);

        {
          char *argv_fixonly[3];

          argv_fixonly[0] = cdd_test_mut_str("--fix");
          argv_fixonly[1] = cdd_test_mut_str("test_gnu_file.h");
          argv_fixonly[2] = NULL;
          ASSERT_EQ(0, cli_standardize_gnu_main(2, argv_fixonly));

          /* Test unknown flag */
          {
            char *argv_unknown_flag[4];

            argv_unknown_flag[0] = cdd_test_mut_str("--audit");
            argv_unknown_flag[1] = cdd_test_mut_str("--unknown-flag");
            argv_unknown_flag[2] = cdd_test_mut_str("test_gnu_file.h");
            argv_unknown_flag[3] = NULL;
            rc = cli_standardize_gnu_main(3, argv_unknown_flag);
            ASSERT_EQ(0, rc);

            remove("test_gnu_file.h");

            /* Test file not found */
            {
              char *argv_notfound[2];

              argv_notfound[0] = cdd_test_mut_str("--audit");
              argv_notfound[1] = cdd_test_mut_str("not_found.h");
              ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
                        cli_standardize_gnu_main(2, argv_notfound));

              g_fail_io_after = -1;
              PASS();
            }
          }
        }
      }
    }
  }
}

#ifdef CDD_BUILD_TESTS
#include <c_cdd_export.h>

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail;

/*  (moved to global) */
#endif

/**
 * @brief Tests error paths in process_file.
 *
 * @return The result of the test.
 */
TEST test_cli_cst_process_errors(void) {
  char *argv_audit[4];
  char *argv_fix[4];
  /* void foo */

  argv_audit[0] = cdd_test_mut_str("extern_c");
  argv_audit[1] = cdd_test_mut_str("--audit");
  argv_audit[2] = cdd_test_mut_str("test_cli_cst_file.h");
  argv_audit[3] = NULL;
  argv_fix[0] = cdd_test_mut_str("extern_c");
  argv_fix[1] = cdd_test_mut_str("--fix");
  argv_fix[2] = cdd_test_mut_str("test_cli_cst_file.h");
  argv_fix[3] = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_cli_cst_file.h", "void foo();"));

#ifdef CDD_BUILD_TESTS
  {
    /* extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail; (moved to global) */
    int i;
    for (i = 1; i <= 500; i++) {
      g_cdd_alloc_fail = i;
      cli_cst_transformer_main(3, argv_fix);
      (void)argv_audit;
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
  write_to_file("test_cli_cst_file.h/foo", "void foo();"); /* just something */

  /* The tool opens it for read first. But it's a directory, so read fails?
   * If read succeeds but write fails, we hit the write error.
   * On UNIX, fopen a dir for "rb" fails or succeeds? Usually fails.
   * If we want to simulate parse error, we can write invalid C, but our parser
   * is resilient and probably parses it anyway. Let's try read-only file for
   * write failure. */
  remove("test_cli_cst_file.h/foo");
#ifdef _WIN32
  _rmdir("test_cli_cst_file.h");
#else
  rmdir("test_cli_cst_file.h");
#endif

  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_cli_cst_file.h", "void foo();"));
#if (defined(__unix__) || defined(__APPLE__) || defined(__linux__) ||          \
     defined(__MACH__)) &&                                                     \
    !defined(__CYGWIN__)
  chmod("test_cli_cst_file.h", 0400); /* read-only */
  /* It needs to be fixed to attempt writing */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cli_cst_transformer_main(3, argv_fix));
  chmod("test_cli_cst_file.h", 0600); /* restore to delete */
#endif

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail;
    g_cdd_cst_emit_realloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cli_cst_transformer_main(3, argv_fix));
    g_cdd_cst_emit_realloc_fail = 0;
  }
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
