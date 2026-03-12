/**
 * @file test_makefile_scraper.h
 * @brief Unit tests for Makefile/configure.ac scraper.
 */

#ifndef TEST_MAKEFILE_SCRAPER_H
#define TEST_MAKEFILE_SCRAPER_H

/* clang-format off */
#include <greatest.h>
#include <string.h>

#include "functions/parse/makefile_scraper.h"
/* clang-format on */

TEST test_scrape_makefile_basic(void) {
  struct ExtractedBuildInfo info;
  char *cmake_str = NULL;
  const char *makefile = "CC=gcc
      CFLAGS = -I./ include - DDEBUG = 1 SRCS = main.c util.c ";

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
  ASSERT(strstr(cmake_str, "add_compile_definitions(
  DEBUG=1
)") != NULL);
  ASSERT(strstr(cmake_str, "add_executable(my_proj
  main.c
  util.c
)") != NULL);
  ASSERT(strstr(cmake_str, "target_include_directories(my_proj PRIVATE
  ./include
)") != NULL);

  free(cmake_str);
  build_info_free(&info);
  PASS();
}

SUITE(makefile_scraper_suite) { RUN_TEST(test_scrape_makefile_basic); }

#endif /* TEST_MAKEFILE_SCRAPER_H */
