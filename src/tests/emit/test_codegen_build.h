extern int g_fail_io_after;
extern int g_io_calls;
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
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/build.h"
#include "functions/emit/build_system.h"
/* clang-format on */

/**
 * @brief test_cbuild_null_args
 * @return TEST
 */
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
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_cbuild_basic_output
 * @return TEST
 */
TEST test_cbuild_basic_output(void) {
  FILE *tmp = tmpfile();
  struct CodegenBuildConfig config;
  const char *sources[] = {"client.c", "models.c"};
  long sz;
  char *content = NULL;

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
  ASSERT(strstr(content, "elseif(ANDROID)"));
  ASSERT(strstr(content, "find_library(log-lib log)"));
  ASSERT(strstr(content, "elseif(APPLE)"));
  ASSERT(strstr(content, "find_library(CFNETWORK_LIBRARY CFNetwork)"));
  ASSERT(strstr(content, "find_package(CURL REQUIRED)"));
  ASSERT(strstr(content,
                "target_link_libraries(petstore_lib PRIVATE CURL::libcurl)"));

  free(content);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_cbuild_unsupported
 * @return TEST
 */
TEST test_cbuild_unsupported(void) {
  FILE *tmp = tmpfile();
  struct CodegenBuildConfig config;

  ASSERT(tmp);
  memset(&config, 0, sizeof(config));
  config.project_name = "PetStore";
  config.target_name = "petstore_lib";

#if defined(ENOTSUP) && (!defined(_MSC_VER) || _MSC_VER >= 1900)
  ASSERT_EQ(ENOTSUP, codegen_build_generate(BUILD_SYS_MESON, tmp, &config));
  ASSERT_EQ(ENOTSUP, codegen_build_generate(BUILD_SYS_MAKEFILE, tmp, &config));
  ASSERT_EQ(ENOTSUP, codegen_build_generate(BUILD_SYS_UNKNOWN, tmp, &config));
#else
  ASSERT_EQ(EINVAL, codegen_build_generate(BUILD_SYS_MESON, tmp, &config));
  ASSERT_EQ(EINVAL, codegen_build_generate(BUILD_SYS_MAKEFILE, tmp, &config));
  ASSERT_EQ(EINVAL, codegen_build_generate(BUILD_SYS_UNKNOWN, tmp, &config));
#endif

  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_system_oom(void) {
  /* Using generate_cmake_project directly */
#ifdef CDD_BUILD_TESTS
  extern int g_cdd_fprintf_fail;
  int i;
  int rc;
  for (i = 1; i < 200; i++) {
    g_cdd_fprintf_fail = i;

    rc = generate_cmake_project("test_build_sys_out", "Proj", 1);
    g_cdd_fprintf_fail = 0;
    if (rc == 0)
      break;
  }
  remove("test_build_sys_out/CMakeLists.txt");

#endif

  ASSERT_EQ(0, generate_cmake_project("test_build_sys_out", "Proj", 0));
  remove("test_build_sys_out/CMakeLists.txt");

#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_fail_alloc;
    int rc_fp2;
    int rc_fp;
    g_cdd_fail_alloc = 1111;
    ASSERT_EQ(ENOMEM, generate_cmake_project("test_build_sys_out", "Proj", 0));
    g_cdd_fail_alloc = 0;

    g_cdd_fail_alloc = 2222;
    ASSERT_EQ(ENOMEM, generate_cmake_project(NULL, "Proj", 0));
    g_cdd_fail_alloc = 0;

    ASSERT_EQ(0, generate_cmake_project(NULL, "Proj", 0));
    remove("CMakeLists.txt");

    /* Test fopen failure */

    g_cdd_fail_alloc = 4444;
    ASSERT(generate_cmake_project("test_build_sys_out", "Proj", 0) != 0);
    g_cdd_fail_alloc = 0;

    g_cdd_fail_alloc = 3334;
    rc_fp2 = generate_cmake_project(NULL, "Proj", 0);
    g_cdd_fail_alloc = 0;
    ASSERT(rc_fp2 != 0);

    g_cdd_fail_alloc = 4445;
    ASSERT(generate_cmake_project("test_build_sys_out", "Proj", 0) != 0);
    g_cdd_fail_alloc = 0;

    g_cdd_fail_alloc = 3333;
    rc_fp = generate_cmake_project(NULL, "Proj", 0);
    g_cdd_fail_alloc = 0;
    ASSERT(rc_fp != 0);
  }
#endif
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief codegen_build_suite
 */
SUITE(codegen_build_suite) {
  RUN_TEST(test_cbuild_null_args);
  RUN_TEST(test_cbuild_basic_output);
  RUN_TEST(test_cbuild_unsupported);
  RUN_TEST(test_build_system_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_BUILD_H */
