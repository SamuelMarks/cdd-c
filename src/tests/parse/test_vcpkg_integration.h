/**
 * @file test_vcpkg_integration.h
 * @brief Unit tests for Vcpkg Integration Generator.
 */

#ifndef TEST_VCPKG_INTEGRATION_H
#define TEST_VCPKG_INTEGRATION_H

/* clang-format off */
#include <greatest.h>
#include <string.h>

#include "functions/parse/vcpkg_integration.h"

TEST test_vcpkg_builder_basic(void) {
  struct VcpkgManifestBuilder builder;
  char *json = NULL;
  const char *src = "#include <stdio.h>
#ifndef _MSC_VER
#include <pthread.h>
#endif
#include \"local.h\"
#include <zlib.h>
                    ";

      ASSERT_EQ(
          0, vcpkg_builder_init(&builder, "my-proj", "1.0.0", "A test proj"));

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

TEST test_vcpkg_builder_duplicate(void) {
  struct VcpkgManifestBuilder builder;
  const char *src = "#ifndef _MSC_VER
#include <pthread.h>
#endif
#ifndef _MSC_VER
#include <pthread.h>
#endif
                /* clang-format on */
                ";

      ASSERT_EQ(0, vcpkg_builder_init(&builder, "proj", NULL, NULL));
  ASSERT_EQ(0, vcpkg_builder_scan_source(&builder, src));

  ASSERT_EQ(1, builder.deps_count);
  ASSERT_STR_EQ("pthreads", builder.deps[0].name);

  vcpkg_builder_free(&builder);
  PASS();
}

SUITE(vcpkg_integration_suite) {
  RUN_TEST(test_vcpkg_builder_basic);
  RUN_TEST(test_vcpkg_builder_duplicate);
}

#endif /* TEST_VCPKG_INTEGRATION_H */
