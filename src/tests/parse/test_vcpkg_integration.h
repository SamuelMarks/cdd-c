/**
 * @file test_vcpkg_integration.h
 * @brief Unit tests for Vcpkg Integration Generator.
 */

#ifndef TEST_VCPKG_INTEGRATION_H
#define TEST_VCPKG_INTEGRATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>

#include "functions/parse/vcpkg_integration.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern int g_cdd_alloc_fail;

/**
 * @brief Tests basic functionality of the Vcpkg builder.
 *
 * @return The result of the test.
 */
TEST test_vcpkg_builder_basic(void) {
  struct VcpkgManifestBuilder builder;
  char *json = NULL;
  const char *src = "#include <stdio.h>\n"
                    "#ifndef _MSC_VER\n"
                    "#include <pthread.h>\n"
                    "#endif\n"
                    "#include \"local.h\"\n"
                    "#include <zlib.h>\n";

  ASSERT_EQ(0, vcpkg_builder_init(&builder, "my-proj", "1.0.0", "A test proj"));

  ASSERT_EQ(0, vcpkg_builder_scan_source(&builder, src));

  ASSERT_EQ(0, vcpkg_builder_generate(&builder, &json));
  ASSERT(json != NULL);

  ASSERT(strstr(json, "\"name\": \"my-proj\"") != NULL);
  ASSERT(strstr(json, "\"version-string\": \"1.0.0\"") != NULL);
  ASSERT(strstr(json, "\"pthreads\"") != NULL);
  ASSERT(strstr(json, "\"zlib\"") != NULL);
  ASSERT(strstr(json, "\"dirent\"") == NULL);

  free(json);
  vcpkg_builder_free(&builder);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests duplicate dependency handling in the Vcpkg builder.
 *
 * @return The result of the test.
 */
TEST test_vcpkg_builder_duplicate(void) {
  struct VcpkgManifestBuilder builder;
  const char *src = "#ifndef _MSC_VER\n"
                    "#include <pthread.h>\n"
                    "#endif\n"
                    "#ifndef _MSC_VER\n"
                    "#include <pthread.h>\n"
                    "#endif\n";

  ASSERT_EQ(0, vcpkg_builder_init(&builder, "proj", NULL, NULL));
  ASSERT_EQ(0, vcpkg_builder_scan_source(&builder, src));

  ASSERT_EQ(1, builder.deps_count);
  ASSERT_STR_EQ("pthreads", builder.deps[0].name);

  vcpkg_builder_free(&builder);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests error handling in the Vcpkg builder.
 *
 * @return The result of the test.
 */
TEST test_vcpkg_builder_errors(void) {
  struct VcpkgManifestBuilder builder;
  char *json = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            vcpkg_builder_init(NULL, "proj", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            vcpkg_builder_init(&builder, NULL, NULL, NULL));

  vcpkg_builder_init(&builder, "proj", NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, vcpkg_builder_add_dep(NULL, "dep"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            vcpkg_builder_add_dep(&builder, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            vcpkg_builder_scan_source(NULL, "#include <pthread.h>"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            vcpkg_builder_scan_source(&builder, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, vcpkg_builder_generate(NULL, &json));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            vcpkg_builder_generate(&builder, NULL));

  vcpkg_builder_free(NULL);

#ifdef CDD_BUILD_TESTS
  cdd_c_error_t test_vcpkg_my_strdup_errors(void);
  ASSERT_EQ(CDD_C_SUCCESS, test_vcpkg_my_strdup_errors());

  g_cdd_alloc_fail = 1; /* tokenize allocates tokens */
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            vcpkg_builder_scan_source(&builder, "#include <pthread.h>"));
  g_cdd_alloc_fail = 0;
#endif

  vcpkg_builder_free(&builder);
  g_fail_io_after = -1;

  /* Test abrupt token stream endings to hit edge case branches */
  vcpkg_builder_init(&builder, "proj", NULL, NULL);
  vcpkg_builder_scan_source(&builder, "#");
  vcpkg_builder_scan_source(&builder, "# ");
  vcpkg_builder_scan_source(&builder,
                            "# 123"); /* hit token != IDENTIFIER branch */
  vcpkg_builder_scan_source(&builder, "#include");
  vcpkg_builder_scan_source(&builder, "#include ");
  vcpkg_builder_scan_source(&builder, "#include <");
  vcpkg_builder_scan_source(&builder, "#include <a");

  /* Trigger reallocation where capacity > 0 */
  vcpkg_builder_add_dep(&builder, "d1");
  vcpkg_builder_add_dep(&builder, "d2");
  vcpkg_builder_add_dep(&builder, "d3");
  vcpkg_builder_add_dep(&builder, "d4");
  vcpkg_builder_add_dep(&builder, "d5");

  /* Trigger mock failure inside loop where name is NULL but count is high to
   * hit name==NULL free branch */
  builder.deps_count = 6;
  builder.deps[5].name = NULL;
  vcpkg_builder_free(&builder);

  /* Generate with 0 deps to hit builder->deps_count > 0 false branch */
  vcpkg_builder_init(&builder, "proj", NULL, NULL);
  vcpkg_builder_generate(&builder, &json);
  free(json);
  vcpkg_builder_free(&builder);

#ifdef CDD_BUILD_TESTS
  {
    int i;
    vcpkg_builder_init(&builder, "proj", NULL, NULL);
    for (i = 1; i < 50; i++) {
      g_cdd_alloc_fail = i;
      int rc = vcpkg_builder_scan_source(&builder, "#include <pthread.h>");
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }

    for (i = 1; i < 50; i++) {
      g_cdd_alloc_fail = i;
      int rc = vcpkg_builder_scan_source(&builder, "#include <dirent.h>");
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }

    for (i = 1; i < 50; i++) {
      g_cdd_alloc_fail = i;
      int rc = vcpkg_builder_scan_source(&builder, "#include <zlib.h>");
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }

    /* Generate OOM */
    vcpkg_builder_add_dep(&builder, "d1");
    for (i = 1; i < 20; i++) {
      g_cdd_alloc_fail = i;
      int rc = vcpkg_builder_generate(&builder, &json);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS) {
        free(json);
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    vcpkg_builder_free(&builder);
  }
#endif

  PASS();
}

TEST test_vcpkg_builder_extras(void) {
  struct VcpkgManifestBuilder builder;
  char *json = NULL;
  const char *src = "#include <dirent.h>\n#  include <stdio.h>\n";

  ASSERT_EQ(0, vcpkg_builder_init(&builder, "my-proj", "1.0.0", "A test proj"));
  ASSERT_EQ(0, vcpkg_builder_scan_source(&builder, src));
  ASSERT_EQ(0, vcpkg_builder_generate(&builder, &json));

  ASSERT(strstr(json, "\"dirent\"") != NULL);

  free(json);
  vcpkg_builder_free(&builder);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Vcpkg integration test suite.
 */

#ifdef CDD_BUILD_TESTS
/* extern int g_cdd_alloc_fail; (moved to global) */
#endif

TEST test_vcpkg_builder_oom(void) {
#ifdef CDD_BUILD_TESTS
  struct VcpkgManifestBuilder builder;
  char *json = NULL;
  const char *src = "#include <stdio.h>\n"
                    "#ifndef _MSC_VER\n"
                    "#include <pthread.h>\n"
                    "#endif\n"
                    "#include \"local.h\"\n"
                    "#include <zlib.h>\n";

  int i;
  for (i = 1; i < 20; i++) {
    g_cdd_alloc_fail = i;
    int rc = vcpkg_builder_init(&builder, "my-proj", "1.0.0", "A test proj");
    g_cdd_alloc_fail = 0;
    if (rc == CDD_C_SUCCESS) {
      vcpkg_builder_free(&builder);
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }

  for (i = 1; i < 5; i++) {
    vcpkg_builder_init(&builder, "my-proj", "1.0.0", "A test proj");
    g_cdd_alloc_fail = i;
    int rc = vcpkg_builder_add_dep(&builder, "pthreads");
    g_cdd_alloc_fail = 0;
    if (rc == CDD_C_SUCCESS) {
      vcpkg_builder_free(&builder);
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    vcpkg_builder_free(&builder);
  }

  vcpkg_builder_init(&builder, "my-proj", "1.0.0", "A test proj");
  vcpkg_builder_scan_source(&builder, src);
  g_cdd_alloc_fail = 1;
  int rc = vcpkg_builder_generate(&builder, &json);
  g_cdd_alloc_fail = 0;
  if (rc != CDD_C_ERROR_MEMORY) {
    printf("GENERATE RC: %d\n", rc);
  }
  vcpkg_builder_free(&builder);

#endif
  PASS();
}

SUITE(vcpkg_integration_suite) {
  RUN_TEST(test_vcpkg_builder_oom);
  RUN_TEST(test_vcpkg_builder_basic);
  RUN_TEST(test_vcpkg_builder_duplicate);
  RUN_TEST(test_vcpkg_builder_errors);
  RUN_TEST(test_vcpkg_builder_extras);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_VCPKG_INTEGRATION_H */
