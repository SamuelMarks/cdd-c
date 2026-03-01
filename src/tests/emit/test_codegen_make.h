/**
 * @file test_codegen_make.h
 * @brief Unit tests for CMake generator.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_MAKE_H
#define TEST_CODEGEN_MAKE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "functions/emit/make.h"

TEST test_make_simple(void) {
  FILE *tmp = tmpfile();
  struct MakeConfig cfg;
  char *content;
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
  ASSERT(strstr(content, "parson::parson"));

  free(content);
  fclose(tmp);
  PASS();
}

TEST test_make_extra_sources(void) {
  FILE *tmp = tmpfile();
  struct MakeConfig cfg;
  char *content;
  char *extras[] = {"a.c", "b.c"};
  long sz;

  ASSERT(tmp);
  memset(&cfg, 0, sizeof(cfg));
  cfg.project_name = "w_extras";
  cfg.extra_sources = extras;
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

SUITE(codegen_make_suite) {
  RUN_TEST(test_make_simple);
  RUN_TEST(test_make_extra_sources);
  RUN_TEST(test_make_invalid);
}

#endif /* TEST_CODEGEN_MAKE_H */
