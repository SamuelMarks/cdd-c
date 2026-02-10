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
#include "fs.h"
#include "generate_build_system.h"

TEST test_gen_cmake_basic(void) {
  const char *out_file = "CMakeLists.txt";
  char *content;
  size_t sz;
  int rc;

  rc = generate_cmake_project(".", "MyLib", 0);
  ASSERT_EQ(0, rc);

  rc = read_to_file(out_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "project(MyLib C)"));
  /* Verify Logic for WinHTTP on Windows */
  ASSERT(strstr(content, "if (WIN32)"));
  ASSERT(strstr(content, "target_link_libraries(MyLib PRIVATE winhttp)"));
  /* Verify Logic for Curl on Unix */
  ASSERT(strstr(content, "else ()"));
  ASSERT(strstr(content, "find_package(CURL REQUIRED)"));
  ASSERT(strstr(content, "target_link_libraries(MyLib PRIVATE CURL::libcurl)"));

  free(content);
  remove(out_file);
  PASS();
}

TEST test_gen_cmake_with_tests(void) {
  const char *out_file = "CMakeLists.txt";
  char *content;
  size_t sz;
  int rc;

  rc = generate_cmake_project(".", "TestProj", 1);
  ASSERT_EQ(0, rc);

  rc = read_to_file(out_file, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "enable_testing()"));

  free(content);
  remove(out_file);
  PASS();
}

TEST test_gen_build_system_cli_args(void) {
  char *argv[] = {"cmake", ".", "CLIProj"};
  int rc;

  rc = generate_build_system_main(3, argv);
  ASSERT_EQ(EXIT_SUCCESS, rc);

  /* Verify file creation */
  {
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    if (fopen_s(&f, "CMakeLists.txt", "r") != 0)
      f = NULL;
#else
    f = fopen("CMakeLists.txt", "r");
#endif
    ASSERT(f != NULL);
    fclose(f);
  }
  remove("CMakeLists.txt");
  PASS();
}

TEST test_gen_build_system_bad_args(void) {
  char *argv_short[] = {"cmake", "."};
  char *argv_bad[] = {"ninja", ".", "Name"};

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
