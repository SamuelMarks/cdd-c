extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_cmake_parser.h
 * @brief Unit tests for CMake parser modifications.
 */

#ifndef TEST_CMAKE_PARSER_H
#define TEST_CMAKE_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>

#include "functions/parse/cmake_parser.h"
/* clang-format on */

/**
 * @brief Tests basic functionality of the CMake modifier.
 *
 * @return The result of the test.
 */
TEST test_cmake_modifier_basic(void) {
  struct CMakeModifier mod;
  char *diff_str = NULL;
  FILE *f;
  FILE *f2;
  extern int g_cdd_fail_alloc;
  int i;
  int rc;
  (void)f;
  (void)f2;
  (void)i;
  (void)rc;

  ASSERT_EQ(0, cmake_modifier_init(&mod, "CMakeLists.txt", "my_target"));
  ASSERT_EQ(0, cmake_modifier_add_compile_opt(&mod, "/W4"));
  ASSERT_EQ(0, cmake_modifier_add_compile_opt(&mod, "/WX"));
  ASSERT_EQ(0, cmake_modifier_add_link_lib(&mod, "ws2_32.lib"));

  ASSERT_EQ(0, cmake_modifier_apply_diff(&mod, &diff_str));
  ASSERT(diff_str != NULL);
  /* printf("\n--- CMAKE DIFF ---\n%s\n------------------\n", diff_str); */

  ASSERT(strstr(diff_str, "--- CMakeLists.txt\n") != NULL);
  ASSERT(strstr(diff_str, "+++ CMakeLists.txt\n") != NULL);
  ASSERT(strstr(diff_str, "+if(MSVC)\n") != NULL);
  ASSERT(strstr(diff_str,
                "+    target_compile_options(my_target PRIVATE /W4 /WX)\n") !=
         NULL);
  ASSERT(strstr(diff_str,
                "+    target_link_libraries(my_target PRIVATE ws2_32.lib)\n") !=
         NULL);
  ASSERT(strstr(diff_str, "+endif()\n") != NULL);

  free(diff_str);
  cmake_modifier_free(&mod);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests global settings modifications with the CMake modifier.
 *
 * @return The result of the test.
 */
TEST test_cmake_modifier_global(void) {
  struct CMakeModifier mod;
  char *diff_str = NULL;
  FILE *f;
  FILE *f2;
  extern int g_cdd_fail_alloc;
  int i;
  int rc;
  (void)f;
  (void)f2;
  (void)i;
  (void)rc;

  ASSERT_EQ(0, cmake_modifier_init(&mod, "CMakeLists.txt", NULL));
  ASSERT_EQ(0, cmake_modifier_add_compile_opt(&mod, "/W4"));
  ASSERT_EQ(0, cmake_modifier_add_link_lib(&mod, "libm.a"));

  ASSERT_EQ(0, cmake_modifier_apply_diff(&mod, &diff_str));
  ASSERT(diff_str != NULL);

  ASSERT(strstr(diff_str, "+    add_compile_options( /W4)\n") != NULL);

  free(diff_str);
  cmake_modifier_free(&mod);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests error handling of the CMake modifier APIs.
 *
 * @return The result of the test.
 */
TEST test_cmake_modifier_errors(void) {
  struct CMakeModifier mod;
  ASSERT_EQ(EINVAL, cmake_modifier_init(NULL, "CMakeLists.txt", "my_target"));
  ASSERT_EQ(EINVAL, cmake_modifier_init(&mod, NULL, "my_target"));

  cmake_modifier_init(&mod, "CMakeLists.txt", "my_target");
  ASSERT_EQ(EINVAL, cmake_modifier_add_compile_opt(NULL, "/W4"));
  ASSERT_EQ(EINVAL, cmake_modifier_add_compile_opt(&mod, NULL));
  ASSERT_EQ(EINVAL, cmake_modifier_add_link_lib(NULL, "ws2_32.lib"));
  ASSERT_EQ(EINVAL, cmake_modifier_add_link_lib(&mod, NULL));

  cmake_modifier_free(&mod);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cmake_parser_oom(void) {

#ifdef CDD_BUILD_TESTS
  struct CMakeModifier mod;
  char *diff_str = NULL;
  FILE *f;
  FILE *f2;
  extern int g_cdd_fail_alloc;
  int i;
  int rc;
  (void)f;
  (void)f2;
  (void)i;
  (void)rc;

  makedirs("test_cmake_dir");
  f = fopen("test_cmake_dir/CMakeLists.txt", "w");
  if (f) {
    fprintf(
        f,
        "project(test)\nadd_library(test a.c)\ntarget_include_directories(test "
        "PUBLIC inc)\ntarget_link_libraries(test PRIVATE cfs)\n");
    fclose(f);
  }

  for (i = 1; i < 20; i++) {
    g_cdd_fail_alloc = i;
    rc = cmake_modifier_init(&mod, "test_cmake_dir/CMakeLists.txt", "test");
    g_cdd_fail_alloc = 0;
    if (rc == 0)
      cmake_modifier_free(&mod);
  }

  for (i = 1; i < 20; i++) {
    cmake_modifier_init(&mod, "test_cmake_dir/CMakeLists.txt", "test");
    g_cdd_fail_alloc = i;
    (void)cmake_modifier_add_compile_opt(&mod, "/W4");
    g_cdd_fail_alloc = 0;
    cmake_modifier_free(&mod);
  }

  for (i = 1; i < 20; i++) {
    cmake_modifier_init(&mod, "test_cmake_dir/CMakeLists.txt", "test");
    g_cdd_fail_alloc = i;
    (void)cmake_modifier_add_link_lib(&mod, "ws2_32.lib");
    g_cdd_fail_alloc = 0;
    cmake_modifier_free(&mod);
  }

  for (i = 1; i < 200; i++) {
    cmake_modifier_init(&mod, "test_cmake_dir/CMakeLists.txt", "test");
    cmake_modifier_add_compile_opt(&mod, "/W4");
    cmake_modifier_add_link_lib(&mod, "ws2_32.lib");

    g_cdd_fail_alloc = i;
    (void)cmake_modifier_apply_diff(&mod, &diff_str);
    g_cdd_fail_alloc = 0;
    if (diff_str) {
      free(diff_str);
      diff_str = NULL;
    }
    cmake_modifier_free(&mod);
  }

  /* Trigger src[len-1] != '
' */
  f2 = fopen("test_cmake_dir/CMakeLists2.txt", "w");
  if (f2) {
    fprintf(f2, "project(test) ");
    fclose(f2);
  }
  cmake_modifier_init(&mod, "test_cmake_dir/CMakeLists2.txt", "test");
  cmake_modifier_apply_diff(&mod, &diff_str);
  if (diff_str)
    free(diff_str);
  cmake_modifier_free(&mod);

  /* Test free NULL */
  cmake_modifier_free(NULL);

  /* Test fopen failure */
  ASSERT_EQ(
      0, cmake_modifier_init(&mod, "test_cmake_dir/non_existent.txt", "test"));
  ASSERT_EQ(0, cmake_modifier_apply_diff(&mod, &diff_str));
  cmake_modifier_free(&mod);

  remove("test_cmake_dir/CMakeLists2.txt");

  remove("test_cmake_dir/CMakeLists.txt");
  remove("test_cmake_dir");
#endif
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief CMake parser test suite.
 */
SUITE(cmake_parser_suite) {
  RUN_TEST(test_cmake_modifier_basic);
  RUN_TEST(test_cmake_modifier_global);
  RUN_TEST(test_cmake_modifier_errors);
  RUN_TEST(test_cmake_parser_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CMAKE_PARSER_H */
