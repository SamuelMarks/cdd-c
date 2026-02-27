/**
 * @file test_codegen_build.h
 * @brief Unit tests for CMakeLists generator logic.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_BUILD_H
#define TEST_CODEGEN_BUILD_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit_build.h"

TEST test_cbuild_null_args(void) {
  struct CodegenBuildConfig config;
  FILE *tmp = tmpfile();

  ASSERT(tmp);
  memset(&config, 0, sizeof(config));

  /* NULL config */
  ASSERT_EQ(EINVAL, codegen_build_generate(BUILD_SYS_CMAKE, tmp, NULL));

  /* NULL fp */
  ASSERT_EQ(EINVAL, codegen_build_generate(BUILD_SYS_CMAKE, NULL, &config));

  /* Missing project name */
  config.target_name = "mylib";
  ASSERT_EQ(EINVAL, codegen_build_generate(BUILD_SYS_CMAKE, tmp, &config));

  /* Missing library target */
  config.project_name = "MyProject";
  config.target_name = NULL;
  ASSERT_EQ(EINVAL, codegen_build_generate(BUILD_SYS_CMAKE, tmp, &config));

  fclose(tmp);
  PASS();
}

TEST test_cbuild_basic_output(void) {
  FILE *tmp = tmpfile();
  struct CodegenBuildConfig config;
  const char *sources[] = {"client.c", "models.c"};
  long sz;
  char *content;

  ASSERT(tmp);
  memset(&config, 0, sizeof(config));
  config.project_name = "PetStore";
  config.target_name = "petstore_lib";
  config.src_files = sources;
  config.src_count = 2;
  config.build_shared_libs = 1;

  ASSERT_EQ(0, codegen_build_generate(BUILD_SYS_CMAKE, tmp, &config));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  ASSERT(content);
  fread(content, 1, sz, tmp);

  /* Verification */
  ASSERT(strstr(content, "project(PetStore C)"));
  ASSERT(strstr(content, "add_library(petstore_lib client.c models.c)"));
  ASSERT(strstr(content, "option(BUILD_SHARED_LIBS \"Build shared libs\" ON)"));

  /* Backend logic check */
  ASSERT(strstr(
      content, "target_compile_definitions(petstore_lib PRIVATE USE_WININET)"));
  ASSERT(strstr(content, "find_package(CURL REQUIRED)"));
  ASSERT(strstr(content,
                "target_link_libraries(petstore_lib PRIVATE CURL::libcurl)"));

  free(content);
  fclose(tmp);
  PASS();
}

TEST test_cbuild_unsupported(void) {
  struct CodegenBuildConfig config = {"P", "L", NULL, 0, 0};
  FILE *tmp = tmpfile();
  ASSERT(tmp);

  ASSERT_EQ(ENOTSUP, codegen_build_generate(BUILD_SYS_MESON, tmp, &config));
  ASSERT_EQ(ENOTSUP, codegen_build_generate(BUILD_SYS_UNKNOWN, tmp, &config));

  fclose(tmp);
  PASS();
}

SUITE(codegen_build_suite) {
  RUN_TEST(test_cbuild_null_args);
  RUN_TEST(test_cbuild_basic_output);
  RUN_TEST(test_cbuild_unsupported);
}

#endif /* TEST_CODEGEN_BUILD_H */
