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

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/emit/build_system.h"
#include "functions/parse/fs.h"

TEST test_gen_cmake_basic(void) {
  const char *out_file = "test_build_dir/CMakeLists.txt";
  char *content;
  size_t sz;
  int rc;

  rc = generate_cmake_project("test_build_dir", "MyLib", 0);
  ASSERT_EQ(0, rc);

  rc = read_to_file(out_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "project(MyLib C)"));

  free(content);
  remove(out_file);
  PASS();
}

TEST test_gen_cmake_with_tests(void) {
  const char *out_file = "test_build_dir/CMakeLists.txt";
  char *content;
  size_t sz;
  int rc;

  rc = generate_cmake_project("test_build_dir", "TestProj", 1);
  ASSERT_EQ(0, rc);

  rc = read_to_file(out_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "enable_testing()"));

  free(content);
  remove(out_file);
  PASS();
}

TEST test_gen_build_system_cli_args(void) {
  char arg0[] = "cmake";
  char arg1[] = "test_build_dir";
  char arg2[] = "CLIProj";
  char *argv[] = {arg0, arg1, arg2};
  int rc;

  rc = generate_build_system_main(3, argv);
  ASSERT_EQ(EXIT_SUCCESS, rc);

  /* Verify file creation */
  {
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    if (fopen_s(&f, "test_build_dir/CMakeLists.txt", "r") != 0)
      f = NULL;
#else
#if defined(_MSC_VER)
    fopen_s(&f, "test_build_dir/CMakeLists.txt", "r");
#else
    f = fopen("test_build_dir/CMakeLists.txt", "r");
#endif
    ASSERT(f != NULL);
    fclose(f);
  }
  remove("test_build_dir/CMakeLists.txt");
  PASS();
}

TEST test_gen_build_system_bad_args(void) {
  char arg0_short[] = "cmake";
  char arg1_short[] = ".";
  char *argv_short[] = {arg0_short, arg1_short};

  char arg0_bad[] = "ninja";
  char arg1_bad[] = ".";
  char arg2_bad[] = "Name";
  char *argv_bad[] = {arg0_bad, arg1_bad, arg2_bad};

  /* Missing name */
  ASSERT_EQ(EXIT_FAILURE, generate_build_system_main(2, argv_short));

  /* Unsupported type */
  ASSERT_EQ(EXIT_FAILURE, generate_build_system_main(3, argv_bad));

  PASS();
}

SUITE(generate_build_system_suite) {
  RUN_TEST(test_gen_cmake_basic);
  RUN_TEST(test_gen_cmake_with_tests);
  RUN_TEST(test_gen_build_system_cli_args);
  RUN_TEST(test_gen_build_system_bad_args);
}

#endif /* TEST_GENERATE_BUILD_SYSTEM_H */
