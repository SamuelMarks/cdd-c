#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
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
#include "c_cdd_export.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "functions/emit/make.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_cdd_fprintf_fail;

/**
 * @brief Tests basic make file generation.
 * @return TEST
 */
TEST test_make_simple(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
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
    if (fread(content, 1, sz, tmp)) {
    }

    ASSERT(strstr(content, "project(test_client"));
    ASSERT(strstr(content, "find_package(CURL REQUIRED)"));
    ASSERT(strstr(content, "add_library(test_client"));
    ASSERT(strstr(content, "parson"));

    free(content);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Tests make file generation with extra sources.
 * @return TEST
 */
TEST test_make_extra_sources(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
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
    if (fread(content, 1, sz, tmp)) {
    }

    ASSERT(strstr(content, "\"a.c\""));
    ASSERT(strstr(content, "\"b.c\""));

    free(content);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Tests make file generator with invalid arguments.
 * @return TEST
 */
TEST test_make_invalid(void) {
  struct MakeConfig cfg = {0};
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  ASSERT(tmp);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, codegen_make_generate(NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, codegen_make_generate(tmp, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_make_generate(tmp, &cfg)); /* No name */

  if (tmp)
    fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

#ifdef _WIN32
#else
#endif

TEST test_make_io_failure(void) {
  struct MakeConfig cfg = {0};
  FILE *f;
  cfg.project_name = "test_io";
#if defined(_MSC_VER)
  if (((f = cdd_test_tmpfile_global()) == NULL))
    f = NULL;
#else
  f = cdd_test_tmpfile_global();
#endif
  g_fail_io_after = 0;
  g_io_calls = 0;
  ASSERT(f);
  g_fail_io_after = 0;
  g_io_calls = 0;
  g_fail_io_after = 1;
  g_io_calls = 0;
  ASSERT_EQ(CDD_C_ERROR_IO, codegen_make_generate(f, &cfg));
  if (f)
    fclose(f);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Suite for codegen make
 */

TEST test_make_oom(void) {
  struct MakeConfig config = {0};
  const char *srcs[] = {"a.c", "b.c"};
  FILE *fp;
  struct MakeConfig config2 = {0};
  struct MakeConfig config3 = {0};
  const char *srcs2[] = {NULL};
#ifdef CDD_BUILD_TESTS
  /* extern C_CDD_EXPORT int g_cdd_fprintf_fail; (moved to global) */
  int i;
  int rc;
  (void)rc;
#endif

  config.project_name = "proj";
  config.extra_sources = (char **)srcs;
  config.extra_source_count = 2;

#if defined(_MSC_VER)
  if (fopen_s(&fp, "test_make_out.txt", "w") != 0)
    fp = NULL;
#else
  fp = fopen("test_make_out.txt", "w");
#endif
  ASSERT(fp);

#ifdef CDD_BUILD_TESTS
  for (i = 1; i < 50; i++) {
    g_cdd_fprintf_fail = i;
    rc = codegen_make_generate(fp, &config);
    g_cdd_fprintf_fail = 0;
    if (rc == 0)
      break;
  }
#endif

  if (fp)
    fclose(fp);

#if defined(_MSC_VER)
  if (fopen_s(&fp, "test_make_out.txt", "w") != 0)
    fp = NULL;
#else
  fp = fopen("test_make_out.txt", "w");
#endif
  config2.project_name = "proj";
  config2.min_cmake_version = "3.20";
  ASSERT_EQ(0, codegen_make_generate(fp, &config2));
  if (fp)
    fclose(fp);

  remove("test_make_out.txt");

#if defined(_MSC_VER)
  if (fopen_s(&fp, "test_make_out.txt", "w") != 0)
    fp = NULL;
#else
  fp = fopen("test_make_out.txt", "w");
#endif
  config3.project_name = "proj";
  config3.extra_sources = (char **)srcs2;
  config3.extra_source_count = 1;
  ASSERT_EQ(0, codegen_make_generate(fp, &config3));
  if (fp)
    fclose(fp);
  remove("test_make_out.txt");
  g_fail_io_after = -1;

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
