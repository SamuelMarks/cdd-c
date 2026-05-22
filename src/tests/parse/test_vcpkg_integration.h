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
#include <greatest.h>
#include <string.h>

#include "functions/parse/vcpkg_integration.h"
/* clang-format on */

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
  ASSERT_EQ(EINVAL, vcpkg_builder_init(NULL, "proj", NULL, NULL));
  ASSERT_EQ(EINVAL, vcpkg_builder_init(&builder, NULL, NULL, NULL));

  vcpkg_builder_init(&builder, "proj", NULL, NULL);
  ASSERT_EQ(EINVAL, vcpkg_builder_add_dep(NULL, "dep"));
  ASSERT_EQ(EINVAL, vcpkg_builder_add_dep(&builder, NULL));
  ASSERT_EQ(EINVAL, vcpkg_builder_scan_source(NULL, "#include <pthread.h>"));
  ASSERT_EQ(EINVAL, vcpkg_builder_scan_source(&builder, NULL));
  ASSERT_EQ(EINVAL, vcpkg_builder_generate(NULL, &json));
  ASSERT_EQ(EINVAL, vcpkg_builder_generate(&builder, NULL));

  vcpkg_builder_free(&builder);
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
  PASS();
}

/**
 * @brief Vcpkg integration test suite.
 */
SUITE(vcpkg_integration_suite) {
  RUN_TEST(test_vcpkg_builder_basic);
  RUN_TEST(test_vcpkg_builder_duplicate);
  RUN_TEST(test_vcpkg_builder_errors);
  RUN_TEST(test_vcpkg_builder_extras);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_VCPKG_INTEGRATION_H */
