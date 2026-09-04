#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
/**
 * @file test_codegen_build.h
 * @brief Unit tests for CMakeLists generator logic.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_BUILD_H
#define TEST_CODEGEN_BUILD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/build.h"
#include "functions/emit/build_system.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_fail_io_after;

/**
 * @brief test_cbuild_null_args
 * @return TEST
 */
TEST test_cbuild_null_args(void) {
  struct CodegenBuildConfig config;
  FILE *tmp = cdd_test_tmpfile_global();

  ASSERT(tmp);
  memset(&config, 0, sizeof(config));

  /* NULL config */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_build_generate(BUILD_SYS_CMAKE, tmp, NULL));

  /* NULL fp */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_build_generate(BUILD_SYS_CMAKE, NULL, &config));

  /* Missing project name */
  config.target_name = (char *)(size_t)"mylib";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_build_generate(BUILD_SYS_CMAKE, tmp, &config));

  /* Missing library target */
  config.project_name = (char *)(size_t)"MyProject";
  config.target_name = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_build_generate(BUILD_SYS_CMAKE, tmp, &config));

  if (tmp)
    fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_cbuild_basic_output
 * @return TEST
 */
TEST test_cbuild_basic_output(void) {
  FILE *tmp = cdd_test_tmpfile_global();
  struct CodegenBuildConfig config;
  const char *sources[] = {"client.c", "models.c"};
  long sz;
  char *content_str = NULL;
  (void)config;
  (void)sources;

  (void)sz;
  (void)content_str;

  ASSERT(tmp);
  memset(&config, 0, sizeof(config));
  config.project_name = (char *)(size_t)"PetStore";
  config.target_name = (char *)(size_t)"petstore_lib";
  config.src_files = sources;
  config.src_count = 2;
  config.build_shared_libs = 1;

  ASSERT_EQ(0, codegen_build_generate(BUILD_SYS_CMAKE, tmp, &config));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content_str = (char *)calloc(1, (size_t)sz + 1);
  ASSERT(content_str);
  if (fread(content_str, 1, (size_t)sz, tmp)) {
  }

  /* Verification */
  ASSERT(strstr(content_str, "project(PetStore C)"));
  ASSERT(strstr(content_str, "add_library(petstore_lib client.c models.c)"));
  ASSERT(strstr(content_str,
                "option(BUILD_SHARED_LIBS \"Build shared libs\" ON)"));

  /* Backend logic check */
  ASSERT(
      strstr(content_str,
             "target_compile_definitions(petstore_lib PRIVATE USE_WININET)"));
  ASSERT(strstr(content_str, "elseif(ANDROID)"));
  ASSERT(strstr(content_str, "find_library(log-lib log)"));
  ASSERT(strstr(content_str, "elseif(APPLE)"));
  ASSERT(strstr(content_str, "find_library(CFNETWORK_LIBRARY CFNetwork)"));
  ASSERT(strstr(content_str, "find_package(CURL REQUIRED)"));
  ASSERT(strstr(content_str,
                "target_link_libraries(petstore_lib PRIVATE CURL::libcurl)"));

  free(content_str);
  if (tmp)
    fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_cbuild_unsupported
 * @return TEST
 */

TEST test_cbuild_unsupported(void) {
  FILE *tmp;
  struct CodegenBuildConfig config;
  const char *sources[] = {"client.c", "models.c"};
  long sz;
  char *content_str = NULL;
  (void)config;
  (void)sources;
  (void)sz;
  (void)content_str;

  memset(&config, 0, sizeof(config));
  config.project_name = (char *)(size_t)"PetStore";
  config.target_name = (char *)(size_t)"petstore_lib";
  config.src_files = sources;
  config.src_count = 2;
  config.build_shared_libs = 1;

  /* Test branch where src_files is NOT NULL but src_count is 0 */
  config.src_files = sources;
  config.src_count = 0;
  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp);
  ASSERT_EQ(0, codegen_build_generate(BUILD_SYS_CMAKE, tmp, &config));
  if (tmp)
    fclose(tmp);
  config.src_count = 2; /* restore */

  /* Test branch where src_files is NULL but src_count is > 0 */
  config.src_files = NULL;
  config.src_count = 2;
  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp);
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_build_generate(BUILD_SYS_CMAKE, tmp, &config));
  if (tmp)
    fclose(tmp);

  /* Test type != BUILD_SYS_CMAKE */
  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            codegen_build_generate(BUILD_SYS_UNKNOWN, tmp, &config));
  if (tmp)
    fclose(tmp);

  /* Test config->src_files == NULL */
  memset(&config, 0, sizeof(config));
  config.project_name = (char *)(size_t)"PetStore";
  config.target_name = (char *)(size_t)"petstore_lib";
  config.src_files = NULL;
  config.src_count = 2;
  config.build_shared_libs = 1;
  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp);
  ASSERT_EQ(CDD_C_SUCCESS,
            codegen_build_generate(BUILD_SYS_CMAKE, tmp, &config));
  if (tmp)
    fclose(tmp);

  /* Test type != BUILD_SYS_CMAKE */
  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            codegen_build_generate(BUILD_SYS_UNKNOWN, tmp, &config));
  if (tmp)
    fclose(tmp);

  /* Test type == BUILD_SYS_MESON */
  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            codegen_build_generate(BUILD_SYS_MESON, tmp, &config));
  if (tmp)
    fclose(tmp);

  /* Test type == BUILD_SYS_MAKEFILE */
  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            codegen_build_generate(BUILD_SYS_MAKEFILE, tmp, &config));
  if (tmp)
    fclose(tmp);

  /* Test type == 999 (default) */
  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            codegen_build_generate((enum CodegenBuildSystem)999, tmp, &config));
  if (tmp)
    fclose(tmp);

  tmp = cdd_test_tmpfile_global();

  ASSERT(tmp);
  memset(&config, 0, sizeof(config));
  config.project_name = (char *)(size_t)"PetStore";
  config.target_name = (char *)(size_t)"petstore_lib";

  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            codegen_build_generate(BUILD_SYS_MESON, tmp, &config));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            codegen_build_generate(BUILD_SYS_MAKEFILE, tmp, &config));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM,
            codegen_build_generate(BUILD_SYS_UNKNOWN, tmp, &config));

  if (tmp)
    fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief codegen_build_suite
 */
TEST test_cbuild_io_failure(void) {
  FILE *tmp;
  struct CodegenBuildConfig config;
  const char *sources[] = {"client.c", "models.c"};
  long sz;
  char *content_str = NULL;
  int i;
  int rc;
  (void)config;
  (void)sources;
  (void)sz;
  (void)content_str;
  /* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */

  (void)rc;
  memset(&config, 0, sizeof(config));
  config.project_name = (char *)(size_t)"PetStore";
  config.target_name = (char *)(size_t)"petstore_lib";
  config.src_files = sources;
  config.src_count = 2;
  config.build_shared_libs = 1;

  for (i = 0; i <= 60; i++) {
    tmp = fopen("test_cbuild_dummy.txt", "w");
    ASSERT(tmp);

    g_fail_io_after = i;
    rc = codegen_build_generate(BUILD_SYS_CMAKE, tmp, &config);
    if (rc == CDD_C_SUCCESS) {
      if (tmp)
        fclose(tmp);
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
    if (tmp)
      fclose(tmp);
  }
  g_fail_io_after = -1;

  remove("test_cbuild_dummy.txt");
  PASS();
}

SUITE(codegen_build_suite) {
  RUN_TEST(test_cbuild_io_failure);
  RUN_TEST(test_cbuild_null_args);
  RUN_TEST(test_cbuild_basic_output);
  RUN_TEST(test_cbuild_unsupported);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_BUILD_H */
