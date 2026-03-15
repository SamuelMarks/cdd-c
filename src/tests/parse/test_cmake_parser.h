/**
 * @file test_cmake_parser.h
 * @brief Unit tests for CMake parser modifications.
 */

/* clang-format off */
#ifndef TEST_CMAKE_PARSER_H
#define TEST_CMAKE_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <greatest.h>
#include <string.h>

#include "functions/parse/cmake_parser.h"
/* clang-format on */

TEST test_cmake_modifier_basic(void) {
  struct CMakeModifier mod;
  char *diff_str = NULL;

  ASSERT_EQ(0, cmake_modifier_init(&mod, "CMakeLists.txt", "my_target"));
  ASSERT_EQ(0, cmake_modifier_add_compile_opt(&mod, "/W4"));
  ASSERT_EQ(0, cmake_modifier_add_compile_opt(&mod, "/WX"));
  ASSERT_EQ(0, cmake_modifier_add_link_lib(&mod, "ws2_32.lib"));

  ASSERT_EQ(0, cmake_modifier_apply_diff(&mod, &diff_str));
  ASSERT(diff_str != NULL);

  ASSERT(strstr(diff_str, "--- CMakeLists.txt
") != NULL);
  ASSERT(strstr(diff_str, "+++ CMakeLists.txt
") != NULL);
  ASSERT(strstr(diff_str, "+if(MSVC)
") != NULL);
  ASSERT(strstr(diff_str, "+    target_compile_options(my_target PRIVATE /W4 /WX)
") != NULL);
  ASSERT(strstr(diff_str, "+    target_link_libraries(my_target PRIVATE ws2_32.lib)
") != NULL);
  ASSERT(strstr(diff_str, "+endif()
") != NULL);

  free(diff_str);
  cmake_modifier_free(&mod);
  PASS();
}

TEST test_cmake_modifier_global(void) {
  struct CMakeModifier mod;
  char *diff_str = NULL;

  ASSERT_EQ(0, cmake_modifier_init(&mod, "CMakeLists.txt", NULL));
  ASSERT_EQ(0, cmake_modifier_add_compile_opt(&mod, "/W4"));

  ASSERT_EQ(0, cmake_modifier_apply_diff(&mod, &diff_str));
  ASSERT(diff_str != NULL);

  ASSERT(strstr(diff_str, "+    add_compile_options( /W4)
") != NULL);

  free(diff_str);
  cmake_modifier_free(&mod);
  PASS();
}

SUITE(cmake_parser_suite) {
  RUN_TEST(test_cmake_modifier_basic);
  RUN_TEST(test_cmake_modifier_global);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CMAKE_PARSER_H */
