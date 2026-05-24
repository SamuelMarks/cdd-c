/**
 * @file test_codegen_make.h
 * @brief Unit tests for CMake generator.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_MAKE_H
#define TEST_CODEGEN_MAKE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "functions/emit/make.h"
/* clang-format on */

/**
 * @brief Tests basic make file generation.
 * @return TEST
 */
TEST test_make_simple(void) {
  FILE *tmp = tmpfile();
  struct MakeConfig cfg;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  memset(&cfg, 0, sizeof(cfg));
  cfg.project_name = "test_client";

  ASSERT_EQ(0, codegen_make_generate(tmp, &cfg));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "project(test_client"));
  ASSERT(strstr(content, "find_package(CURL REQUIRED)"));
  ASSERT(strstr(content, "add_library(test_client"));
  ASSERT(strstr(content, "parson"));

  free(content);
  fclose(tmp);
  PASS();
}

/**
 * @brief Tests make file generation with extra sources.
 * @return TEST
 */
TEST test_make_extra_sources(void) {
  FILE *tmp = tmpfile();
  struct MakeConfig cfg;
  char *content = NULL;
  char *extras[] = {"a.c", "b.c"};
  long sz;

  ASSERT(tmp);
  memset(&cfg, 0, sizeof(cfg));
  cfg.project_name = "w_extras";
  cfg.extra_sources = (char **)extras;
  cfg.extra_source_count = 2;

  ASSERT_EQ(0, codegen_make_generate(tmp, &cfg));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "\"a.c\""));
  ASSERT(strstr(content, "\"b.c\""));

  free(content);
  fclose(tmp);
  PASS();
}

/**
 * @brief Tests make file generator with invalid arguments.
 * @return TEST
 */
TEST test_make_invalid(void) {
  struct MakeConfig cfg = {0};
  FILE *tmp = tmpfile();
  ASSERT(tmp);

  ASSERT_EQ(EINVAL, codegen_make_generate(NULL, &cfg));
  ASSERT_EQ(EINVAL, codegen_make_generate(tmp, NULL));

  ASSERT_EQ(EINVAL, codegen_make_generate(tmp, &cfg)); /* No name */

  fclose(tmp);
  PASS();
}

#ifdef _WIN32
#define DEV_NULL "nul"
#else
#define DEV_NULL "/dev/null"
#endif

TEST test_make_io_failure(void) {
  struct MakeConfig cfg = {0};
  FILE *f;
  cfg.project_name = "test_io";
  f = fopen(DEV_NULL, "r");
  ASSERT(f);
  ASSERT_EQ(EIO, codegen_make_generate(f, &cfg));
  fclose(f);
  PASS();
}

/**
 * @brief Suite for codegen make
 */

TEST test_make_oom(void) {
  struct MakeConfig config = {0};
  config.project_name = "proj";
  const char *srcs[] = {"a.c", "b.c"};
  config.extra_sources = (char **)srcs;
  config.extra_source_count = 2;

  FILE *fp = fopen("test_make_out.txt", "w");
  ASSERT(fp);

#ifdef CDD_BUILD_TESTS
  extern int g_cdd_fprintf_fail;
  int i;
  for (i = 1; i < 200; i++) {
    g_cdd_fprintf_fail = i;
    int rc = codegen_make_generate(fp, &config);
    g_cdd_fprintf_fail = 0;
    if (rc == 0)
      break;
  }
#endif

  fclose(fp);

  fp = fopen("test_make_out.txt", "w");
  struct MakeConfig config2 = {0};
  config2.project_name = "proj";
  config2.min_cmake_version = "3.20";
  ASSERT_EQ(0, codegen_make_generate(fp, &config2));
  fclose(fp);

  remove("test_make_out.txt");

  fp = fopen("test_make_out.txt", "w");
  struct MakeConfig config3 = {0};
  config3.project_name = "proj";
  const char *srcs2[] = {NULL};
  config3.extra_sources = (char **)srcs2;
  config3.extra_source_count = 1;
  ASSERT_EQ(0, codegen_make_generate(fp, &config3));
  fclose(fp);
  remove("test_make_out.txt");

  PASS();
}

SUITE(codegen_make_suite) {
  RUN_TEST(test_make_oom);
  RUN_TEST(test_make_simple);
  RUN_TEST(test_make_extra_sources);
  RUN_TEST(test_make_invalid);
  RUN_TEST(test_make_io_failure);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_MAKE_H */
