/**
 * @file test_makefile_scraper.h
 * @brief Unit tests for Makefile/configure.ac scraper.
 */

#ifndef TEST_MAKEFILE_SCRAPER_H
#define TEST_MAKEFILE_SCRAPER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>

#include "functions/parse/makefile_scraper.h"
/* clang-format on */

/**
 * @brief Tests basic functionality of the Makefile scraper.
 *
 * @return The result of the test.
 */
TEST test_scrape_makefile_basic(void) {
  struct ExtractedBuildInfo info;
  char *cmake_str = NULL;
  const char *makefile = "CC=gcc\n"
                         "CFLAGS=-I./include -DDEBUG=1\n"
                         "SRCS=main.c util.c";

  build_info_init(&info);
  ASSERT_EQ(0, scrape_makefile(&info, makefile));

  ASSERT_EQ(2, info.source_files_n);
  ASSERT_STR_EQ("main.c", info.source_files[0]);
  ASSERT_STR_EQ("util.c", info.source_files[1]);

  ASSERT_EQ(1, info.include_dirs_n);
  ASSERT_STR_EQ("./include", info.include_dirs[0]);

  ASSERT_EQ(1, info.compile_defs_n);
  ASSERT_STR_EQ("DEBUG=1", info.compile_defs[0]);

  ASSERT_EQ(0, build_info_to_cmake(&info, "my_proj", &cmake_str));
  ASSERT(cmake_str != NULL);

  ASSERT(strstr(cmake_str, "project(my_proj C)") != NULL);
  ASSERT(strstr(cmake_str, "add_compile_definitions(\n  DEBUG=1\n)") != NULL);
  ASSERT(strstr(cmake_str, "add_executable(my_proj\n  main.c\n  util.c\n)") !=
         NULL);
  ASSERT(strstr(cmake_str,
                "target_include_directories(my_proj PRIVATE\n  ./include\n)") !=
         NULL);

  free(cmake_str);
  build_info_free(&info);
  PASS();
}

/**
 * @brief Tests error handling of the scraper APIs.
 *
 * @return The result of the test.
 */
TEST test_scrape_errors(void) {
  struct ExtractedBuildInfo info;
  char *cmake_str = NULL;

  build_info_init(&info);

  ASSERT_EQ(EINVAL, scrape_makefile(NULL, "Makefile"));
  ASSERT_EQ(EINVAL, scrape_makefile(&info, NULL));

  ASSERT_EQ(EINVAL, scrape_configure_ac(NULL, "configure.ac"));
  ASSERT_EQ(EINVAL, scrape_configure_ac(&info, NULL));

  ASSERT_EQ(EINVAL, build_info_to_cmake(NULL, "proj", &cmake_str));
  ASSERT_EQ(EINVAL, build_info_to_cmake(&info, NULL, &cmake_str));
  ASSERT_EQ(EINVAL, build_info_to_cmake(&info, "proj", NULL));

  build_info_free(&info);
  PASS();
}

TEST test_scrape_configure_ac_basic(void) {
  struct ExtractedBuildInfo info;
  char *cmake_str = NULL;
  const char *config = "AC_INIT([test], [1.0])\n"
                       "AC_CONFIG_SRCDIR([main.c])\n"
                       "CFLAGS=\"-Iinc -DTEST\"\n";

  build_info_init(&info);
  ASSERT_EQ(0, scrape_configure_ac(&info, config));

  ASSERT_EQ(1, info.source_files_n);
  ASSERT_STR_EQ("main.c", info.source_files[0]);

  ASSERT_EQ(1, info.include_dirs_n);
  ASSERT_STR_EQ("inc", info.include_dirs[0]);

  ASSERT_EQ(1, info.compile_defs_n);
  ASSERT_STR_EQ("TEST", info.compile_defs[0]);

  build_info_free(&info);
  PASS();
}

/**
 * @brief Makefile scraper test suite.
 */
SUITE(makefile_scraper_suite) {
  RUN_TEST(test_scrape_makefile_basic);
  RUN_TEST(test_scrape_errors);
  RUN_TEST(test_scrape_configure_ac_basic);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_MAKEFILE_SCRAPER_H */
