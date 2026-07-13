/**
 * @file test_generate_build_system.h
 * @brief Unit tests for build system generation logic.
 *
 * Verifies that CMakeLists.txt content is generated correctly for different
 * configurations.
 *
 * @author Samuel Marks
 */

#ifndef TEST_GENERATE_BUILD_SYSTEM_H
#define TEST_GENERATE_BUILD_SYSTEM_H

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
#include "functions/emit/build_system.h"
#include "functions/parse/fs.h"
/* clang-format on */

/**
 * @brief test_gen_cmake_basic
 * @return TEST
 */
/* LCOV_EXCL_START */
TEST test_gen_cmake_basic(void) {
  const char *out_file = "test_build_dir/CMakeLists.txt";
  const char *src_file = "test_build_dir/src/CMakeLists.txt";
  char *content = NULL;
  /* LCOV_EXCL_STOP */
  size_t sz;
  int rc;

  /* LCOV_EXCL_START */
  rc = generate_cmake_project("test_build_dir", "MyLib", 0);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = read_to_file(out_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT(strstr(content, "project(MyLib C)"));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free(content);
  remove(out_file);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/**
 * @brief test_gen_cmake_with_tests
 * @return TEST
 */
/* LCOV_EXCL_START */
TEST test_gen_cmake_with_tests(void) {
  const char *out_file = "test_build_dir/CMakeLists.txt";
  const char *src_file = "test_build_dir/src/CMakeLists.txt";
  char *content = NULL;
  /* LCOV_EXCL_STOP */
  size_t sz;
  int rc;

  /* LCOV_EXCL_START */
  rc = generate_cmake_project("test_build_dir", "TestProj", 1);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = read_to_file(src_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  ASSERT(strstr(content, "enable_testing()"));
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free(content);
  remove(out_file);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/**
 * @brief test_gen_build_system_cli_args
 * @return TEST
 */
/* LCOV_EXCL_START */
TEST test_gen_build_system_cli_args(void) {
  char arg0[] = "cmake";
  char arg1[] = "test_build_dir";
  char arg2[] = "CLIProj";
  /* LCOV_EXCL_STOP */
  char *argv[3];
  int rc;

  /* LCOV_EXCL_START */
  argv[0] = arg0;
  argv[1] = arg1;
  argv[2] = arg2;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = generate_build_system_main(3, argv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  /* LCOV_EXCL_STOP */

  /* Verify file creation */
  {
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    if (fopen_s(&f, "test_build_dir/src/CMakeLists.txt", "r") != 0)
      f = NULL;
#elif defined(_MSC_VER)
    fopen_s(&f, "test_build_dir/src/CMakeLists.txt", "r");
#else
    /* LCOV_EXCL_START */
    f = fopen("test_build_dir/src/CMakeLists.txt", "r");
/* LCOV_EXCL_STOP */
#endif
    /* LCOV_EXCL_START */
    ASSERT(f != NULL);
    fclose(f);
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  remove("test_build_dir/src/CMakeLists.txt");
  remove("test_build_dir/CMakeLists.txt");
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/**
 * @brief test_gen_build_system_bad_args
 * @return TEST
 */
/* LCOV_EXCL_START */
TEST test_gen_build_system_bad_args(void) {
  char arg0_short[] = "cmake";
  char arg1_short[] = ".";
  /* LCOV_EXCL_STOP */
  char *argv_short[2];

  /* LCOV_EXCL_START */
  char arg0_bad[] = "ninja";
  char arg1_bad[] = ".";
  char arg2_bad[] = "Name";
  /* LCOV_EXCL_STOP */
  char *argv_bad[3];

  /* LCOV_EXCL_START */
  argv_short[0] = arg0_short;
  argv_short[1] = arg1_short;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  argv_bad[0] = arg0_bad;
  argv_bad[1] = arg1_bad;
  argv_bad[2] = arg2_bad;
  /* LCOV_EXCL_STOP */

  /* Missing name */
  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            /* LCOV_EXCL_STOP */
            generate_build_system_main(2, argv_short));

  /* Unsupported type */
  /* LCOV_EXCL_START */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            /* LCOV_EXCL_STOP */
            generate_build_system_main(3, argv_bad));
  /* LCOV_EXCL_START */
  g_fail_io_after = -1;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_gen_cmake_null_args(void) {
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            /* LCOV_EXCL_STOP */
            generate_cmake_project("out", NULL, 0));
  /* LCOV_EXCL_START */
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_gen_cmake_null_outdir(void) {
  /* LCOV_EXCL_STOP */
  int rc;

  /* Backup existing CMakeLists.txt if any */
  /* LCOV_EXCL_START */
  rename("CMakeLists.txt", "CMakeLists.txt.bak");
  rename("src/CMakeLists.txt", "src/CMakeLists.txt.bak");
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  rc = generate_cmake_project(NULL, "MyLib", 0);
  ASSERT_EQ(0, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  remove("CMakeLists.txt");
  remove("src/CMakeLists.txt");
  /* LCOV_EXCL_STOP */

  /* Restore */
  /* LCOV_EXCL_START */
  rename("CMakeLists.txt.bak", "CMakeLists.txt");
  rename("src/CMakeLists.txt.bak", "src/CMakeLists.txt");
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_gen_cmake_bad_makedirs(void) {
  /* LCOV_EXCL_STOP */
  /* Passing an empty string to makedirs often fails or does nothing depending
     on OS, but we can try a definitely invalid path like a file that exists but
     is not a dir */
  /* LCOV_EXCL_START */
  FILE *f = fopen("test_dummy_file_for_makedirs", "w");
  if (f) {
    fclose(f);
    ASSERT_NEQ(0, generate_cmake_project("test_dummy_file_for_makedirs/foo",
                                         /* LCOV_EXCL_STOP */
                                         "MyLib", 0));
    /* LCOV_EXCL_START */
    remove("test_dummy_file_for_makedirs");
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_gen_build_system_cli_args_tests(void) {
  char *argv[] = {"cmake", "test_build_dir_tests", "CLIProjWithTests", "test"};
  int rc = generate_build_system_main(4, argv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  remove("test_build_dir_tests/src/CMakeLists.txt");
  remove("test_build_dir_tests/CMakeLists.txt");
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_gen_build_system_cli_args_fail(void) {
  char *argv[] = {"cmake", "test_dummy_file_for_makedirs/foo",
                  /* LCOV_EXCL_STOP */
                  "CLIProjWithTests"};
  /* LCOV_EXCL_START */
  FILE *f = fopen("test_dummy_file_for_makedirs", "w");
  if (f) {
    fclose(f);
    ASSERT_EQ(CDD_C_ERROR_IO, generate_build_system_main(3, argv));
    remove("test_dummy_file_for_makedirs");
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/**
 * @brief generate_build_system_suite
 */
/* LCOV_EXCL_START */
SUITE(generate_build_system_suite) {
  RUN_TEST(test_gen_cmake_basic);
  RUN_TEST(test_gen_cmake_with_tests);
  RUN_TEST(test_gen_build_system_cli_args);
  RUN_TEST(test_gen_build_system_bad_args);
  RUN_TEST(test_gen_cmake_null_args);
  RUN_TEST(test_gen_cmake_null_outdir);
  RUN_TEST(test_gen_cmake_bad_makedirs);
  RUN_TEST(test_gen_build_system_cli_args_tests);
  RUN_TEST(test_gen_build_system_cli_args_fail);
}
/* LCOV_EXCL_STOP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_GENERATE_BUILD_SYSTEM_H */
