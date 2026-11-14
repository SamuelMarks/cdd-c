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

        g_cdd_last_alloc_err = write_to_file("test_gnu_file.h", "void foo();");

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
  char *argv_fix[4];
  /* void foo */

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
      g_cdd_last_alloc_err = cli_cst_transformer_main(3, argv_fix);
    }
    g_cdd_alloc_fail = 0;

    for (i = 1; i <= 500; i++) {
      g_cdd_cst_alloc_node_fail = i;
      g_cdd_last_alloc_err = cli_cst_transformer_main(3, argv_fix);
    }
    g_cdd_cst_alloc_node_fail = 0;

    for (i = 1; i <= 500; i++) {
      g_cdd_cst_alloc_node_fail = i;
      g_cdd_last_alloc_err = cli_cst_transformer_main(3, argv_fix);
    }
    g_cdd_cst_alloc_node_fail = 0;

    for (i = 1; i <= 500; i++) {
      g_cdd_cst_emit_realloc_fail = i;
      g_cdd_last_alloc_err = cli_cst_transformer_main(3, argv_fix);
    }
    g_cdd_cst_emit_realloc_fail = 0;
  }
#endif

  /* Output file open failure in fix mode */
  /* By creating a directory with the same name, fopen for write will fail */
  remove("test_cli_cst_file.h");
  g_cdd_last_alloc_err = makedir("test_cli_cst_file.h");
  g_cdd_last_alloc_err = write_to_file("test_cli_cst_file.h/foo",
                                       "void foo();"); /* just something */

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
 * @brief Tests `cdd_transform_c_lowering` and `cdd_transform_pipeline` APIs,
 * dialect profile sync, and per-pass error percolation.
 *
 * @return Greatest test result.
 */
TEST test_cli_cst_pipeline_api(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_tree_t empty_tree;
  cdd_transform_config_t cfg;
  char *out = NULL;
  int i;

  memset(&empty_tree, 0, sizeof(empty_tree));
  memset(&cfg, 0, sizeof(cfg));

  /* Null / invalid tree checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_c_lowering(NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_c_lowering(&empty_tree, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_pipeline(NULL, CDD_PASS_GNU, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_pipeline(&empty_tree, CDD_PASS_GNU, &cfg));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("void f(void) {}\n"), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_c_lowering(tree, NULL));
  cfg.msvc_ver = -1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_c_lowering(tree, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_pipeline(tree, CDD_PASS_C_LOWERING, &cfg));
  cfg.msvc_ver = 0;

  /* Default NULL config pipeline */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_transform_pipeline(
                tree,
                CDD_PASS_MACROS | CDD_PASS_GNU | CDD_PASS_C_LOWERING |
                    CDD_PASS_MSVC_PORT | CDD_PASS_SAFE_CRT |
                    CDD_PASS_ERROR_PERCOLATOR | CDD_PASS_EXTERN_C,
                NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "__cplusplus") != NULL);
  C_CDD_FREE(out);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));

  /* Profile & VLA mode combinations */
  {
    static const cdd_target_msvc_profile_t k_profs[] = {
        CDD_TARGET_PROFILE_DEFAULT,     CDD_TARGET_PROFILE_MSVC_LEGACY,
        CDD_TARGET_PROFILE_MSVC_MODERN, CDD_TARGET_PROFILE_WATCOM,
        CDD_TARGET_PROFILE_ISO_C89,     CDD_TARGET_PROFILE_ISO_C99};
    size_t p;
    for (p = 0; p < sizeof(k_profs) / sizeof(k_profs[0]); p++) {
      memset(&cfg, 0, sizeof(cfg));
      cfg.target_profile = k_profs[p];
      if (p == 1) {
        cfg.vla_mode = CDD_VLA_MODE_MALLOC;
      } else if (p == 2) {
        cfg.fallback_vla_to_malloc = 1;
      }
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str("int x;\n"), &tree));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_transform_pipeline(tree, CDD_PASS_NONE, &cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    }
  }

  /* Per-pass OOM error percolation in cdd_transform_pipeline */
  {
    static const struct {
      unsigned int mask;
      const char *src;
    } k_oom_cases[] = {
        {CDD_PASS_MACROS,
         "#define FOO(a) (a)\nconst char *f(void) { return FOO(1); }\n"},
        {CDD_PASS_GNU, "void f(int n) { int a[n]; }\n"},
        {CDD_PASS_MSVC_PORT, "#include <unistd.h>\nint f() { return 0; }\n"},
        {CDD_PASS_SAFE_CRT, "void f(char *d) { strcpy(d, \"hi\"); }\n"},
        {CDD_PASS_ERROR_PERCOLATOR, "void f(void) { void *p = malloc(4); }\n"},
        {CDD_PASS_EXTERN_C, "void f(void);\n"}};
    size_t c;
    memset(&cfg, 0, sizeof(cfg));
    for (c = 0; c < sizeof(k_oom_cases) / sizeof(k_oom_cases[0]); c++) {
      for (i = 1; i <= 35; i++) {
        tree = NULL;
        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(k_oom_cases[c].src), &tree));
        g_cdd_alloc_fail = i;
        if (cdd_transform_pipeline(tree, k_oom_cases[c].mask, &cfg) ==
            CDD_C_SUCCESS) {
          g_cdd_alloc_fail = 0;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
          break;
        }
        g_cdd_alloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      }
      g_cdd_alloc_fail = 0;
    }
  }

  PASS();
}

/**
 * @brief Tests `cli_cst_transform_pipeline_main` and
 * `cli_transform_dispatch_main` across out-of-tree `-o`, `--out-dir`,
 * `--root-dir`, `--flat`, `--stdin`,
 * `--stdout`, and option validation/error paths.
 *
 * @return Greatest test result.
 */
TEST test_cli_cst_out_of_tree_and_dispatch(void) {
  char *content = NULL;
  size_t sz = 0;

  /* 1. Help & empty arg checks */
  {
    char *a_help[1];
    char *a_disp_h[2];
    char *a_disp_h2[2];
    char *a_disp_v[2];
    char *a_disp_v2[2];
    char *a_disp_bad[2];

    a_help[0] = cdd_test_mut_str("--help");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(0, a_help));
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(1, a_help));
    a_help[0] = cdd_test_mut_str("-h");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(1, a_help));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_transform_dispatch_main(1, a_help));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_transform_dispatch_main(2, NULL));

    a_disp_h[0] = cdd_test_mut_str("cdd-c-transform");
    a_disp_h[1] = cdd_test_mut_str("--help");
    ASSERT_EQ(CDD_C_SUCCESS, cli_transform_dispatch_main(2, a_disp_h));
    a_disp_h2[0] = cdd_test_mut_str("cdd-c-transform");
    a_disp_h2[1] = cdd_test_mut_str("-h");
    ASSERT_EQ(CDD_C_SUCCESS, cli_transform_dispatch_main(2, a_disp_h2));

    a_disp_v[0] = cdd_test_mut_str("cdd-c-transform");
    a_disp_v[1] = cdd_test_mut_str("--version");
    ASSERT_EQ(CDD_C_SUCCESS, cli_transform_dispatch_main(2, a_disp_v));
    a_disp_v2[0] = cdd_test_mut_str("cdd-c-transform");
    a_disp_v2[1] = cdd_test_mut_str("-v");
    ASSERT_EQ(CDD_C_SUCCESS, cli_transform_dispatch_main(2, a_disp_v2));

    a_disp_bad[0] = cdd_test_mut_str("cdd-c-transform");
    a_disp_bad[1] = cdd_test_mut_str("bogus_cmd");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_transform_dispatch_main(2, a_disp_bad));
  }

  /* 2. Dispatch routes: transform, transformer, standardize-gnu, and '-' flag
   */
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_tf_in.c", "void f(void);\n"));
  {
    char *d_tf[6];
    char *d_tr[3];
    char *d_sg[3];
    char *d_dash[5];

    d_tf[0] = cdd_test_mut_str("cdd-c-transform");
    d_tf[1] = cdd_test_mut_str("transform");
    d_tf[2] = cdd_test_mut_str("--passes=extern_c");
    d_tf[3] = cdd_test_mut_str("test_tf_in.c");
    d_tf[4] = cdd_test_mut_str("-o");
    d_tf[5] = cdd_test_mut_str("test_tf_out.c");
    ASSERT_EQ(CDD_C_SUCCESS, cli_transform_dispatch_main(6, d_tf));
    ASSERT_EQ(CDD_C_SUCCESS,
              read_to_file("test_tf_out.c", "rb", &content, &sz));
    ASSERT(strstr(content, "__cplusplus") != NULL);
    C_CDD_FREE(content);
    remove("test_tf_out.c");

    /* Unchanged source still emits output file in out-of-tree -o mode */
    d_dash[0] = cdd_test_mut_str("cdd-c-transform");
    d_dash[1] = cdd_test_mut_str("--passes=none");
    d_dash[2] = cdd_test_mut_str("--output=test_tf_sub/out_nested.c");
    d_dash[3] = cdd_test_mut_str("test_tf_in.c");
    ASSERT_EQ(CDD_C_SUCCESS, cli_transform_dispatch_main(4, d_dash));
    ASSERT_EQ(CDD_C_SUCCESS,
              read_to_file("test_tf_sub/out_nested.c", "rb", &content, &sz));
    ASSERT(strstr(content, "void f(void);") != NULL);
    C_CDD_FREE(content);
    remove("test_tf_sub/out_nested.c");
#ifdef _WIN32
    _rmdir("test_tf_sub");
#else
    rmdir("test_tf_sub");
#endif

    d_tr[0] = cdd_test_mut_str("cdd-c-transform");
    d_tr[1] = cdd_test_mut_str("transformer");
    d_tr[2] = cdd_test_mut_str("macros");
    ASSERT_EQ(CDD_C_SUCCESS, cli_transform_dispatch_main(3, d_tr));

    d_sg[0] = cdd_test_mut_str("cdd-c-transform");
    d_sg[1] = cdd_test_mut_str("standardize-gnu");
    d_sg[2] = cdd_test_mut_str("--help");
    ASSERT_EQ(CDD_C_SUCCESS, cli_transform_dispatch_main(3, d_sg));
  }

  /* 3. --out-dir with --root-dir, relative subdir, --flat, and --dry-run */
  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_tf_src/sub"));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_to_file("test_tf_src/sub/mod.c", "void m(void);\n"));
  {
    char *a_od[8];
    a_od[0] = cdd_test_mut_str(
        "--passes=gnu,gnu_standardizer,c_lowering,msvc_port,msvc,safe_crt,"
        "macros,extern_c,error_percolator,percolate_errors,all,none,");
    a_od[1] = cdd_test_mut_str("--root-dir=test_tf_src/");
    a_od[2] = cdd_test_mut_str("--out-dir");
    a_od[3] = cdd_test_mut_str("test_tf_gen");
    a_od[4] = cdd_test_mut_str("--use-tabs");
    a_od[5] = cdd_test_mut_str("--indent-width=4");
    a_od[6] = cdd_test_mut_str("--with-auto-win-msvc");
    a_od[7] = cdd_test_mut_str("test_tf_src/sub/mod.c");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(8, a_od));
    ASSERT_EQ(CDD_C_SUCCESS,
              read_to_file("test_tf_gen/sub/mod.c", "rb", &content, &sz));
    C_CDD_FREE(content);
    remove("test_tf_gen/sub/mod.c");
#ifdef _WIN32
    _rmdir("test_tf_gen/sub");
    _rmdir("test_tf_gen");
#else
    rmdir("test_tf_gen/sub");
    rmdir("test_tf_gen");
#endif

    /* Relative path without root_dir */
    a_od[0] = cdd_test_mut_str("--passes=none");
    a_od[1] = cdd_test_mut_str("--out-dir=test_tf_gen2");
    a_od[2] = cdd_test_mut_str("test_tf_src/sub/mod.c");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(3, a_od));
    ASSERT_EQ(CDD_C_SUCCESS, read_to_file("test_tf_gen2/test_tf_src/sub/mod.c",
                                          "rb", &content, &sz));
    C_CDD_FREE(content);
    remove("test_tf_gen2/test_tf_src/sub/mod.c");
#ifdef _WIN32
    _rmdir("test_tf_gen2/test_tf_src/sub");
    _rmdir("test_tf_gen2/test_tf_src");
    _rmdir("test_tf_gen2");
#else
    rmdir("test_tf_gen2/test_tf_src/sub");
    rmdir("test_tf_gen2/test_tf_src");
    rmdir("test_tf_gen2");
#endif

    /* --flat + --dry-run + root_dir mismatch fallback */
    a_od[0] = cdd_test_mut_str("--passes=none");
    a_od[1] = cdd_test_mut_str("--out-dir=test_tf_flat");
    a_od[2] = cdd_test_mut_str("--root-dir=other_root");
    a_od[3] = cdd_test_mut_str("--flat");
    a_od[4] = cdd_test_mut_str("--dry-run");
    a_od[5] = cdd_test_mut_str("test_tf_src/sub/mod.c");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(6, a_od));

    /* Non-flat with "../", "/", "\\", "c:", and root_dir separator variants */
    a_od[0] = cdd_test_mut_str("--passes=none");
    a_od[1] = cdd_test_mut_str("--out-dir=.");
    a_od[2] = cdd_test_mut_str("--dry-run");
    a_od[3] = cdd_test_mut_str("test_tf_src/../test_tf_in.c");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(4, a_od));

    a_od[0] = cdd_test_mut_str("--passes=none");
    a_od[1] = cdd_test_mut_str("--out-dir=.");
    a_od[2] = cdd_test_mut_str("--root-dir=/");
    a_od[3] = cdd_test_mut_str("--dry-run");
    a_od[4] = cdd_test_mut_str("test_tf_in.c");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(5, a_od));

    a_od[0] = cdd_test_mut_str("--passes=none");
    a_od[1] = cdd_test_mut_str("--out-dir=.");
    a_od[2] = cdd_test_mut_str("--root-dir=test_tf\\");
    a_od[3] = cdd_test_mut_str("--dry-run");
    a_od[4] = cdd_test_mut_str("test_tf_in.c");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(5, a_od));

    a_od[0] = cdd_test_mut_str("--passes=none");
    a_od[1] = cdd_test_mut_str("--out-dir=.");
    a_od[2] = cdd_test_mut_str("--root-dir=test_tf_src");
    a_od[3] = cdd_test_mut_str("test_tf_src\\missing_mod.c");
    ASSERT_NEQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(4, a_od));

    a_od[0] = cdd_test_mut_str("--passes=none");
    a_od[1] = cdd_test_mut_str("--out-dir=.");
    a_od[2] = cdd_test_mut_str("/nonexistent_abs_tf.c");
    ASSERT_NEQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(3, a_od));

    a_od[0] = cdd_test_mut_str("--passes=none");
    a_od[1] = cdd_test_mut_str("--out-dir=.");
    a_od[2] = cdd_test_mut_str("\\nonexistent_unc_tf.c");
    ASSERT_NEQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(3, a_od));

    a_od[0] = cdd_test_mut_str("--passes=none");
    a_od[1] = cdd_test_mut_str("--out-dir=.");
    a_od[2] = cdd_test_mut_str("c:nonexistent_drv_tf.c");
    ASSERT_NEQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(3, a_od));

    ASSERT_EQ(CDD_C_SUCCESS, write_to_file("c_colon_tmp", "int z;\n"));
    /* Also test resolve_output_filepath on existing file with ../ and / */
    {
      char *p_slash[4];
      p_slash[0] = cdd_test_mut_str("--passes=none");
      p_slash[1] = cdd_test_mut_str("--out-dir=.");
      p_slash[2] = cdd_test_mut_str("--dry-run");
      p_slash[3] = cdd_test_mut_str("./test_tf_src/../c_colon_tmp");
      ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(4, p_slash));
    }
    remove("c_colon_tmp");
  }

  /* 4. All profile, VLA, Safe-CRT, msvc-ver, rename-main, and -I combinations
   */
  {
    char *a_cfg[15];
    a_cfg[0] = cdd_test_mut_str("--target-msvc=modern");
    a_cfg[1] = cdd_test_mut_str("--msvc-ver=0");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(2, a_cfg));

    a_cfg[0] = cdd_test_mut_str("--target-msvc=legacy");
    a_cfg[1] = cdd_test_mut_str("--target-msvc=modern");
    a_cfg[2] = cdd_test_mut_str("--msvc-ver=1500");
    a_cfg[3] = cdd_test_mut_str("--target-msvc=legacy");
    a_cfg[4] = cdd_test_mut_str("--target-msvc=modern");
    a_cfg[5] = cdd_test_mut_str("--target-msvc=watcom");
    a_cfg[6] = cdd_test_mut_str("--target-msvc=iso-c89");
    a_cfg[7] = cdd_test_mut_str("--target-msvc=iso-c99");
    a_cfg[8] = cdd_test_mut_str("--vla-mode=alloca");
    a_cfg[9] = cdd_test_mut_str("--vla-mode=malloc");
    a_cfg[10] = cdd_test_mut_str("--vla-mode=hybrid");
    a_cfg[11] = cdd_test_mut_str("--safe-crt=guarded");
    a_cfg[12] = cdd_test_mut_str("--safe-crt=msvc-only");
    a_cfg[13] = cdd_test_mut_str("--safe-crt=off");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(14, a_cfg)); /* no input files */

    a_cfg[0] = cdd_test_mut_str("-Iinc1");
    a_cfg[1] = cdd_test_mut_str("-I");
    a_cfg[2] = cdd_test_mut_str("inc2");
    a_cfg[3] = cdd_test_mut_str("--include-dir=inc3");
    a_cfg[4] = cdd_test_mut_str("--rename-main=my_entry");
    a_cfg[5] = cdd_test_mut_str("--fallback-malloc");
    a_cfg[6] = cdd_test_mut_str("--stdout");
    a_cfg[7] = cdd_test_mut_str("test_tf_in.c");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(8, a_cfg));

    /* OOM on append_include_dir */
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cli_cst_transform_pipeline_main(8, a_cfg));
    g_cdd_alloc_fail = 0;
  }

  /* 5. --stdin / "-" streaming */
#if !defined(_WIN32)
  {
    char *a_stdin[4];
    FILE *orig_in;
    int saved_fd = dup(fileno(stdin));
    orig_in = freopen("test_tf_in.c", "rb", stdin);
    ASSERT(orig_in != NULL);
    a_stdin[0] = cdd_test_mut_str("--passes=none");
    a_stdin[1] = cdd_test_mut_str("--stdin");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(2, a_stdin));

    /* "-" positional input + -o output */
    rewind(stdin);
    a_stdin[0] = cdd_test_mut_str("--passes=none");
    a_stdin[1] = cdd_test_mut_str("-");
    a_stdin[2] = cdd_test_mut_str("-o");
    a_stdin[3] = cdd_test_mut_str("test_stdin_o.c");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(4, a_stdin));
    remove("test_stdin_o.c");

    /* --stdin with --out-dir and explicit "-" */
    rewind(stdin);
    a_stdin[0] = cdd_test_mut_str("--passes=none");
    a_stdin[1] = cdd_test_mut_str("--stdin");
    a_stdin[2] = cdd_test_mut_str("--out-dir=.");
    a_stdin[3] = cdd_test_mut_str("--dry-run");
    ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(4, a_stdin));

    /* standardize-gnu with "-" positional input */
    rewind(stdin);
    a_stdin[0] = cdd_test_mut_str("--stdout");
    a_stdin[1] = cdd_test_mut_str("-");
    ASSERT_EQ(CDD_C_SUCCESS, cli_standardize_gnu_main(2, a_stdin));

    /* --stdin with virtual filepaths exercises resolve_output_filepath branches
     * (num_files == 1 with from_stdin, '\\' separator, prefix mismatch, '/',
     * '\\' UNC, and 'c:' drive paths) */
    {
      static const char *const k_vpaths[] = {
          "root\\v.c", "root_other/v.c", "/abs_v.c", "\\unc_v.c", "c:drv_v.c"};
      size_t vp;
      char *a_vp[6];
      for (vp = 0; vp < sizeof(k_vpaths) / sizeof(k_vpaths[0]); vp++) {
        rewind(stdin);
        a_vp[0] = cdd_test_mut_str("--passes=none");
        a_vp[1] = cdd_test_mut_str("--stdin");
        a_vp[2] = cdd_test_mut_str("--out-dir=.");
        a_vp[3] = cdd_test_mut_str("--root-dir=root");
        a_vp[4] = cdd_test_mut_str("--dry-run");
        a_vp[5] = cdd_test_mut_str(k_vpaths[vp]);
        ASSERT_EQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(6, a_vp));
      }
    }

    /* stdin OOM error */
    rewind(stdin);
    a_stdin[0] = cdd_test_mut_str("--passes=none");
    a_stdin[1] = cdd_test_mut_str("--stdin");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cli_cst_transform_pipeline_main(2, a_stdin));
    g_cdd_alloc_fail = 0;

    dup2(saved_fd, fileno(stdin));
    close(saved_fd);
  }
#endif

  /* 6. Invalid flag / option error paths */
  {
    char *a_err[4];

    /* Missing -o value */
    a_err[0] = cdd_test_mut_str("-o");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));

    /* Missing -I value */
    a_err[0] = cdd_test_mut_str("-I");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));

    /* Missing --output value & empty --output= */
    a_err[0] = cdd_test_mut_str("--output");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));
    a_err[0] = cdd_test_mut_str("--output=");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));

    /* Bad pass (empty token, overlong token, unknown name), profile, etc. */
    a_err[0] = cdd_test_mut_str("--passes=,gnu");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));
    a_err[0] =
        cdd_test_mut_str("--passes=012345678901234567890123456789012345");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));
    a_err[0] = cdd_test_mut_str("--passes=bad_pass");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));
    a_err[0] = cdd_test_mut_str("--target-msvc=bad_prof");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));
    a_err[0] = cdd_test_mut_str("--msvc-ver=0");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));
    a_err[0] = cdd_test_mut_str("--vla-mode=bad_vla");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));
    a_err[0] = cdd_test_mut_str("--safe-crt=bad_crt");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));
    a_err[0] = cdd_test_mut_str("--outputted");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(1, a_err));

    /* Multiple input files with -o */
    a_err[0] = cdd_test_mut_str("-o");
    a_err[1] = cdd_test_mut_str("out.c");
    a_err[2] = cdd_test_mut_str("test_tf_in.c");
    a_err[3] = cdd_test_mut_str("test_tf_src/sub/mod.c");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(4, a_err));

    /* Out-of-tree write failure (destination path is an existing directory) */
    a_err[0] = cdd_test_mut_str("--passes=none");
    a_err[1] = cdd_test_mut_str("-o");
    a_err[2] = cdd_test_mut_str("test_tf_src");
    a_err[3] = cdd_test_mut_str("test_tf_in.c");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cli_cst_transform_pipeline_main(4, a_err));

    /* Out-of-tree makedirs failure (parent path is a regular file!) */
    a_err[0] = cdd_test_mut_str("--passes=none");
    a_err[1] = cdd_test_mut_str("-o");
    a_err[2] = cdd_test_mut_str("test_tf_in.c/impossible/out.c");
    a_err[3] = cdd_test_mut_str("test_tf_in.c");
    ASSERT_NEQ(CDD_C_SUCCESS, cli_cst_transform_pipeline_main(4, a_err));

    /* OOM on resolve_output_filepath / write_out_of_tree_file */
    {
      int k;
      a_err[0] = cdd_test_mut_str("--passes=none");
      a_err[1] = cdd_test_mut_str("--out-dir=test_tf_oom_dir");
      a_err[2] = cdd_test_mut_str("--flat");
      a_err[3] = cdd_test_mut_str("test_tf_src/sub/mod.c");
      for (k = 1; k <= 30; k++) {
        g_cdd_alloc_fail = k;
        if (cli_cst_transform_pipeline_main(4, a_err) == CDD_C_SUCCESS) {
          g_cdd_alloc_fail = 0;
          break;
        }
      }
      g_cdd_alloc_fail = 0;
      remove("test_tf_oom_dir/mod.c");
#ifdef _WIN32
      _rmdir("test_tf_oom_dir");
#else
      rmdir("test_tf_oom_dir");
#endif
    }
  }

  remove("test_tf_src/sub/mod.c");
#ifdef _WIN32
  _rmdir("test_tf_src/sub");
  _rmdir("test_tf_src");
#else
  rmdir("test_tf_src/sub");
  rmdir("test_tf_src");
#endif
  remove("test_tf_in.c");
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
  RUN_TEST(test_cli_cst_pipeline_api);
  RUN_TEST(test_cli_cst_out_of_tree_and_dispatch);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_CST_H */
