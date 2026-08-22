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

/* Moved extern declarations for C89 compliance */
extern int g_fail_io_after;
extern int g_cdd_strdup_fail;
extern int g_cdd_alloc_fail;

/**
 * @brief test_gen_cmake_basic
 * @return TEST
 */
TEST test_gen_cmake_basic(void) {
  const char *out_file = "test_build_dir/CMakeLists.txt";
  const char *src_file = "test_build_dir/src/CMakeLists.txt";
  char *content = NULL;
  size_t sz;
  int rc;

  rc = generate_cmake_project("test_build_dir", "MyLib", 0);
  ASSERT_EQ(0, rc);

  rc = read_to_file(out_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "project(MyLib C)"));

  free(content);
  remove(out_file);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_gen_cmake_with_tests
 * @return TEST
 */
TEST test_gen_cmake_with_tests(void) {
  const char *out_file = "test_build_dir/CMakeLists.txt";
  const char *src_file = "test_build_dir/src/CMakeLists.txt";
  char *content = NULL;
  size_t sz;
  int rc;

  rc = generate_cmake_project("test_build_dir", "TestProj", 1);
  ASSERT_EQ(0, rc);

  rc = read_to_file(src_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "enable_testing()"));

  free(content);
  remove(out_file);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_gen_build_system_cli_args
 * @return TEST
 */
TEST test_gen_build_system_cli_args(void) {
  char arg0[] = "cmake";
  char arg1[] = "test_build_dir";
  char arg2[] = "CLIProj";
  char *argv[3];
  int rc;

  argv[0] = arg0;
  argv[1] = arg1;
  argv[2] = arg2;

  rc = generate_build_system_main(3, argv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Verify file creation */
  {
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    if (fopen_s(&f, "test_build_dir/src/CMakeLists.txt", "r") != 0)
      f = NULL;
#elif defined(_MSC_VER)
    fopen_s(&f, "test_build_dir/src/CMakeLists.txt", "r");
#else
#if defined(_MSC_VER)
    if (fopen_s(&f, "test_build_dir/src/CMakeLists.txt", "r") != 0)
      f = NULL;
#else
    f = fopen("test_build_dir/src/CMakeLists.txt", "r");
#endif
#endif
    ASSERT(f != NULL);
    fclose(f);
  }
  remove("test_build_dir/src/CMakeLists.txt");
  remove("test_build_dir/CMakeLists.txt");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_gen_build_system_bad_args
 * @return TEST
 */
TEST test_gen_build_system_bad_args(void) {
  char arg0_short[] = "cmake";
  char arg1_short[] = ".";
  char *argv_short[2];

  char arg0_bad[] = "ninja";
  char arg1_bad[] = ".";
  char arg2_bad[] = "Name";
  char *argv_bad[3];

  argv_short[0] = arg0_short;
  argv_short[1] = arg1_short;

  argv_bad[0] = arg0_bad;
  argv_bad[1] = arg1_bad;
  argv_bad[2] = arg2_bad;

  /* Missing name */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            generate_build_system_main(2, argv_short));

  /* Unsupported type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            generate_build_system_main(3, argv_bad));
  g_fail_io_after = -1;

  PASS();
}

TEST test_build_system_oom2(void) {
  int i;
  int rc;

#ifdef CDD_BUILD_TESTS
  /* extern int g_cdd_alloc_fail; (moved to global) */
  for (i = 1; i <= 20; i++) {
    g_cdd_alloc_fail = i;
    rc = generate_cmake_project("test_build_dir", "MyProject", 1);
    if (rc == CDD_C_SUCCESS)
      i = 999;
  }
  g_cdd_alloc_fail = 0;
#endif

  rc = generate_cmake_project("test_build_dir", "MyProject", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  remove("test_build_dir/src/CMakeLists.txt");
  remove("test_build_dir/CMakeLists.txt");
  PASS();
}

TEST test_gen_cmake_null_args(void) {
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            generate_cmake_project("out", NULL, 0));
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_cmake_null_outdir(void) {
  int rc;

  /* Backup existing CMakeLists.txt if any */
  rename("CMakeLists.txt", "CMakeLists.txt.bak");
  rename("src/CMakeLists.txt", "src/CMakeLists.txt.bak");

  rc = generate_cmake_project(NULL, "MyLib", 0);
  ASSERT_EQ(0, rc);

  remove("CMakeLists.txt");
  remove("src/CMakeLists.txt");

  /* Restore */
  rename("CMakeLists.txt.bak", "CMakeLists.txt");
  rename("src/CMakeLists.txt.bak", "src/CMakeLists.txt");

  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_cmake_bad_makedirs(void) {
  /* Passing an empty string to makedirs often fails or does nothing depending
     on OS, but we can try a definitely invalid path like a file that exists but
     is not a dir */
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_dummy_file_for_makedirs", "w") != 0)
    f = NULL;
#else
  f = fopen("test_dummy_file_for_makedirs", "w");
#endif
  if (f) {
    fclose(f);
    makedirs("test_dummy_dir_for_makedirs");
    FILE *f2;
#if defined(_MSC_VER)
    if (fopen_s(&f2, "test_dummy_dir_for_makedirs/src", "w") != 0)
      f2 = NULL;
#else
    f2 = fopen("test_dummy_dir_for_makedirs/src", "w");
#endif
    if (f2) {
      fclose(f2);
      ASSERT_NEQ(
          0, generate_cmake_project("test_dummy_dir_for_makedirs", "MyLib", 0));
      remove("test_dummy_dir_for_makedirs/src");
    }
    remove("test_dummy_dir_for_makedirs");
    ASSERT_NEQ(0, generate_cmake_project("test_dummy_file_for_makedirs/foo",
                                         "MyLib", 0));
    remove("test_dummy_file_for_makedirs");
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_system_io_failure(void) {
  int i;
  int rc;
  const char *out_file = "test_build_dir/CMakeLists.txt";
  const char *src_file = "test_build_dir/src/CMakeLists.txt";

#ifdef CDD_BUILD_TESTS
  /* extern int g_fail_io_after; (moved to global) */

  for (i = 0; i <= 400; i++) {
    g_fail_io_after = i;
    rc = generate_cmake_project("test_build_dir", "MyProject", 1);
  }
  while (rc != CDD_C_SUCCESS && ++i < 999)
    ;
  g_fail_io_after = -1;
#endif

  rc = generate_cmake_project("test_build_dir", "MyProject", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  remove(src_file);
  remove(out_file);
  PASS();
}

TEST test_gen_build_system_cli_args_tests(void) {
  char *argv[] = {"cmake", "test_build_dir_tests", "CLIProjWithTests", "test"};
  int rc = generate_build_system_main(4, argv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  remove("test_build_dir_tests/src/CMakeLists.txt");
  remove("test_build_dir_tests/CMakeLists.txt");
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_build_system_cli_args_fail(void) {
  char *argv[] = {"cmake", "test_dummy_file_for_makedirs/foo",
                  "CLIProjWithTests"};
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_dummy_file_for_makedirs", "w") != 0)
    f = NULL;
#else
  f = fopen("test_dummy_file_for_makedirs", "w");
#endif
  if (f) {
    fclose(f);
    ASSERT_EQ(CDD_C_ERROR_IO, generate_build_system_main(3, argv));
    remove("test_dummy_file_for_makedirs");
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_cmake_oom(void) {
#ifdef CDD_BUILD_TESTS
  /* extern int g_cdd_alloc_fail; (moved to global) */
  /* extern int g_cdd_strdup_fail; (moved to global) */
  int i;
  makedirs("test_build_dir_oom");
  for (i = 1; i <= 100; i++) {
    g_cdd_alloc_fail = i;
    int rc = generate_cmake_project("test_build_dir_oom", "Proj", 0);
    printf("i=%d, rc=%d, g_alloc=%d\n", i, rc, g_cdd_alloc_fail);
    if (rc == 0) {
      printf("BROKE AT %d\n", i);
      break;
    }
  }

  for (i = 1; i <= 100; i++) {
    g_cdd_alloc_fail = i;
    if (generate_cmake_project("test_build_dir_oom", "Proj", 1) == 0) {
      printf("BROKE TESTS AT %d\n", i);
      break;
    }
  }

  g_cdd_alloc_fail = 0;

  if (chdir("test_build_dir_oom")) {
  }
  for (i = 1; i <= 100; i++) {
    g_cdd_strdup_fail = i;
    if (generate_cmake_project(NULL, "Proj", 0) == 0)
      break;
  }
  g_cdd_strdup_fail = 0;

  for (i = 1; i <= 100; i++) {
    g_cdd_alloc_fail = i;
    if (generate_cmake_project(NULL, "Proj", 0) == 0)
      break;
  }
  g_cdd_alloc_fail = 0;
  remove("src/CMakeLists.txt");
  remove("CMakeLists.txt");
  if (chdir("..")) {
  }
#endif
  remove("test_build_dir_oom/src/CMakeLists.txt");
  remove("test_build_dir_oom/CMakeLists.txt");
  PASS();
}

TEST test_gen_cmake_readonly2(void) {
#ifndef _WIN32
  int rc;
  makedirs("test_build_dir_readonly2");
  chmod("test_build_dir_readonly2", 0444);
  rc = generate_cmake_project("test_build_dir_readonly2", "Proj", 0);
  chmod("test_build_dir_readonly2", 0755);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
#endif
  PASS();
}

TEST test_gen_cmake_readonly(void) {
#ifndef _WIN32
  int rc;
  makedirs("test_build_dir_readonly/src");
  chmod("test_build_dir_readonly/src", 0444);
  rc = generate_cmake_project("test_build_dir_readonly", "Proj", 1);
  chmod("test_build_dir_readonly/src", 0755);
  /* The first fopen is CMakeLists.txt in root, second is in src/CMakeLists.txt
   */
  /* If we make root readonly, it fails early. If we make src readonly, it fails
   * at line 588! */
  printf("RC = %d\n", rc);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
#endif
  PASS();
}

TEST test_gen_cmake_io_fail(void) {
  int i;
  makedirs("test_build_dir_io");
  i = 0;
  do {
    g_fail_io_after = i;
    g_io_calls = 0;
  } while (generate_cmake_project("test_build_dir_io", "Proj", 1) != 0 &&
           ++i <= 10);
  g_fail_io_after = -1;

  if (chdir("test_build_dir_io")) {
  }
  for (i = 0; i <= 10; i++) {
    g_fail_io_after = i;
    g_io_calls = 0;
    if (generate_cmake_project(NULL, "Proj", 1) == 0)
      break;
  }
  g_fail_io_after = -1;
  remove("src/CMakeLists.txt");
  remove("CMakeLists.txt");
  if (chdir("..")) {
  }
  remove("test_build_dir_io/src/CMakeLists.txt");
  remove("test_build_dir_io/CMakeLists.txt");
  PASS();
}

/**
 * @brief generate_build_system_suite
 */
SUITE(generate_build_system_suite) {
  RUN_TEST(test_build_system_oom2);
  RUN_TEST(test_build_system_io_failure);
  RUN_TEST(test_gen_cmake_basic);
  RUN_TEST(test_gen_cmake_with_tests);
  RUN_TEST(test_gen_build_system_cli_args);
  RUN_TEST(test_gen_build_system_bad_args);
  RUN_TEST(test_gen_cmake_null_args);
  RUN_TEST(test_gen_cmake_null_outdir);
  RUN_TEST(test_gen_cmake_bad_makedirs);
  RUN_TEST(test_gen_build_system_cli_args_tests);
  RUN_TEST(test_gen_build_system_cli_args_fail);
  RUN_TEST(test_gen_cmake_oom);
  RUN_TEST(test_gen_cmake_io_fail);
  RUN_TEST(test_gen_cmake_readonly);
  RUN_TEST(test_gen_cmake_readonly2);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_GENERATE_BUILD_SYSTEM_H */
