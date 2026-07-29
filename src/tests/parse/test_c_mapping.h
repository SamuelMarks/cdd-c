/**
 * @file test_c_mapping.h
 * @brief Unit tests for the C to OpenAPI Type Mapper.
 *
 * Verifies that basic C types, pointers, arrays, and structs are correctly
 * categorized and mapped to their OpenAPI equivalents.
 *
 * @author Samuel Marks
 */

#ifndef TEST_C_MAPPING_H
#define TEST_C_MAPPING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse/mapping.h"
/* clang-format on */

/**
 * @brief test_mapping_int
 * @return TEST
 */
TEST test_mapping_int(void) {
  struct OpenApiTypeMapping m;
  int rc;

  (void)c_mapping_init(&m);
  rc = c_mapping_map_type("int", "x", &m);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("integer", m.oa_type);
  ASSERT_STR_EQ("int32", m.oa_format);
  c_mapping_free(&m);

  /* Empty string */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("", "x", &m));
  c_mapping_free(&m);

  /* void type (not pointer) */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("void", "x", &m));
  c_mapping_free(&m);

  /* const volatile signed unsigned */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0,
            c_mapping_map_type("const volatile signed unsigned int", "x", &m));
  c_mapping_free(&m);

  /* array mapping */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("struct Item *", "ptr[]", &m));
  c_mapping_free(&m);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test_mapping_string
 * @return TEST
 */
TEST test_mapping_string(void) {
  struct OpenApiTypeMapping m;
  int rc;

  rc = c_mapping_map_type("char *", "str", &m);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("string", m.oa_type);
  ASSERT_EQ(NULL, m.oa_format);
  c_mapping_free(&m);

  rc = c_mapping_map_type("const char *", "s", &m);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("string", m.oa_type);
  c_mapping_free(&m);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test_mapping_struct_ref
 * @return TEST
 */
TEST test_mapping_struct_ref(void) {
  struct OpenApiTypeMapping m;
  int rc;

  rc = c_mapping_map_type("struct User", "u", &m);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_TYPE_OBJECT, m.kind);
  ASSERT_STR_EQ("User", m.ref_name);
  ASSERT_EQ(NULL, m.oa_type);
  c_mapping_free(&m);

  rc = c_mapping_map_type("struct Item *", "ptr", &m);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_TYPE_OBJECT, m.kind);
  ASSERT_STR_EQ("Item", m.ref_name);
  c_mapping_free(&m);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test_mapping_array
 * @return TEST
 */
TEST test_mapping_array(void) {
  struct OpenApiTypeMapping m;
  int rc;

  rc = c_mapping_map_type("int", "ids[]", &m);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_TYPE_ARRAY, m.kind);
  /* The "type" field indicates item type */
  ASSERT_STR_EQ("integer", m.oa_type);
  c_mapping_free(&m);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test_mapping_bool
 * @return TEST
 */
TEST test_mapping_bool(void) {
  struct OpenApiTypeMapping m;
  int rc;

  rc = c_mapping_map_type("bool", "flag", &m);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("boolean", m.oa_type);
  c_mapping_free(&m);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_mapping_long
 * @return TEST
 */
TEST test_mapping_long(void) {
  struct OpenApiTypeMapping m;
  int rc;
  rc = c_mapping_map_type("unsigned long long", "big", &m);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("integer", m.oa_type);
  ASSERT_STR_EQ("int64", m.oa_format);
  c_mapping_free(&m);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_mapping_void_ptr
 * @return TEST
 */
TEST test_mapping_void_ptr(void) {
  struct OpenApiTypeMapping m;
  int rc;
  (void)c_mapping_init(&m);
  rc = c_mapping_map_type("void *", "data", &m);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("string", m.oa_type);
  ASSERT_STR_EQ("binary", m.oa_format);
  c_mapping_free(&m);
  g_fail_io_after = -1;
  PASS();
}

TEST test_mapping_coverage(void) {
  struct OpenApiTypeMapping m = {0};

  /* NULL tests */
  (void)c_mapping_init(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_mapping_map_type("int", "x", NULL));
  c_mapping_free(NULL);

  /* long, short, float, size_t */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("long int", "x", &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("integer", m.oa_type);
  ASSERT_STR_EQ("int64", m.oa_format);
  c_mapping_free(&m);

  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("short int", "x", &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("integer", m.oa_type);
  ASSERT_EQ(NULL, m.oa_format);
  c_mapping_free(&m);

  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("float", "x", &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("number", m.oa_type);
  ASSERT_STR_EQ("float", m.oa_format);
  c_mapping_free(&m);

  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("size_t", NULL, &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("integer", m.oa_type);
  ASSERT_STR_EQ("int64", m.oa_format);
  c_mapping_free(&m);

  /* pointers and references */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("struct MyStruct *", "x", &m));
  c_mapping_free(&m);

  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("int *", "x", &m));
  c_mapping_free(&m);

  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("const volatile int", "x", &m));
  c_mapping_free(&m);

#ifdef CDD_BUILD_TESTS
  {
    /* Simulate OOMs */
    extern C_CDD_EXPORT int g_cdd_strdup_fail;
    int rc_oom;
    int i;
    g_cdd_strdup_fail = 1;
    rc_oom = c_mapping_map_type("int", "x", &m);
    printf("RC_OOM=%d CDD_C_ERROR_MEMORY=%d\n", rc_oom, CDD_C_ERROR_MEMORY);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_oom);
    g_cdd_strdup_fail = 0;
    c_mapping_free(&m);

    /* Trigger inner_type/inner_ref duplication OOM */
    for (i = 1; i < 10; i++) {
      g_cdd_strdup_fail = i;
      (void)c_mapping_init(&m);
      (void)c_mapping_map_type("int *", "x[]", &m);
      g_cdd_strdup_fail = 0;
      c_mapping_free(&m);
    }
  }
#endif

  /* Empty string */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("", "x", &m));
  c_mapping_free(&m);

  /* void type (not pointer) */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("void", "x", &m));
  c_mapping_free(&m);

  /* const volatile signed unsigned */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0,
            c_mapping_map_type("const volatile signed unsigned int", "x", &m));
  c_mapping_free(&m);

  /* array mapping */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("struct Item *", "ptr[]", &m));
  c_mapping_free(&m);

  /* Test end-of-string qualifiers */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("const", "x", &m));
  c_mapping_free(&m);
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("volatile", "x", &m));
  c_mapping_free(&m);
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("signed", "x", &m));
  c_mapping_free(&m);
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("unsigned", "x", &m));
  c_mapping_free(&m);

  /* Simulate strdup OOM inside mapping */
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_strdup_fail;

    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c_mapping_map_type("enum MyEnum", "x", &m));
    g_cdd_strdup_fail = 0;

    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              c_mapping_map_type("struct MyStruct *", "x[]", &m));
    g_cdd_strdup_fail = 0;
    c_mapping_free(&m);

    g_cdd_strdup_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              c_mapping_map_type("struct MyStruct *", "x[]", &m));
    g_cdd_strdup_fail = 0;
    c_mapping_free(&m);

    g_cdd_strdup_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c_mapping_map_type("int *", "x", &m));
    g_cdd_strdup_fail = 0;
    c_mapping_free(&m);

    g_cdd_strdup_fail = 3;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              c_mapping_map_type("struct MyStruct *", "x[]", &m));
    g_cdd_strdup_fail = 0;
    c_mapping_free(&m);
  }
#endif

  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("short", "x", &m));
  c_mapping_free(&m);

  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("double", "x", &m));
  c_mapping_free(&m);

  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("_Bool", "x", &m));
  c_mapping_free(&m);

  /* Test spaces after struct */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("struct   Foo", "x", &m));
  c_mapping_free(&m);

  /* Test empty struct name */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("struct ", "x", &m));
  c_mapping_free(&m);

  /* Test templates */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("std::vector<int>", "x", &m));
  ASSERT_EQ(OA_TYPE_OBJECT, m.kind);
  ASSERT_STR_EQ("std::vector<int>", m.ref_name);
  c_mapping_free(&m);

  /* Test generic T */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("T", "x", &m));
  ASSERT_EQ(OA_TYPE_OBJECT, m.kind);
  ASSERT_STR_EQ("T", m.ref_name);
  c_mapping_free(&m);

  /* Test multiple leading spaces */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("   int", "x", &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  c_mapping_free(&m);

  /* Test template without > to hit fallback string */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("template<", "x", &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("string", m.oa_type);
  c_mapping_free(&m);

  /* Test single char to hit the false branch of is_ptr || is_array */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("char", "x", &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("string", m.oa_type);
  c_mapping_free(&m);

  /* Test char array to hit is_ptr == false but is_array == true */
  (void)c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("char", "x[]", &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("string", m.oa_type);
  c_mapping_free(&m);

  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief c_mapping_suite
 */
SUITE(c_mapping_suite) {
  RUN_TEST(test_mapping_int);
  RUN_TEST(test_mapping_string);
  RUN_TEST(test_mapping_struct_ref);
  RUN_TEST(test_mapping_array);
  RUN_TEST(test_mapping_bool);
  RUN_TEST(test_mapping_long);
  RUN_TEST(test_mapping_void_ptr);
  RUN_TEST(test_mapping_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C_MAPPING_H */
