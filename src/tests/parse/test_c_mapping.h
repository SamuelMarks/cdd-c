extern int g_fail_io_after;
extern int g_io_calls;
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

  c_mapping_init(&m);
  rc = c_mapping_map_type("int", "x", &m);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("integer", m.oa_type);
  ASSERT_STR_EQ("int32", m.oa_format);
  c_mapping_free(&m);

  /* Empty string */
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("", "x", &m));
  c_mapping_free(&m);

  /* void type (not pointer) */
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("void", "x", &m));
  c_mapping_free(&m);

  /* const volatile signed unsigned */
  c_mapping_init(&m);
  ASSERT_EQ(0,
            c_mapping_map_type("const volatile signed unsigned int", "x", &m));
  c_mapping_free(&m);

  /* array mapping */
  c_mapping_init(&m);
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
  c_mapping_init(&m);
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
  c_mapping_init(NULL);
  ASSERT_EQ(EINVAL, c_mapping_map_type("int", "x", NULL));
  c_mapping_free(NULL);

  /* long, short, float, size_t */
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("long int", "x", &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("integer", m.oa_type);
  ASSERT_STR_EQ("int64", m.oa_format);
  c_mapping_free(&m);

  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("short int", "x", &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("integer", m.oa_type);
  ASSERT_EQ(NULL, m.oa_format);
  c_mapping_free(&m);

  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("float", "x", &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("number", m.oa_type);
  ASSERT_STR_EQ("float", m.oa_format);
  c_mapping_free(&m);

  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("size_t", NULL, &m));
  ASSERT_EQ(OA_TYPE_PRIMITIVE, m.kind);
  ASSERT_STR_EQ("integer", m.oa_type);
  ASSERT_STR_EQ("int64", m.oa_format);
  c_mapping_free(&m);

  /* pointers and references */
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("struct MyStruct *", "x", &m));
  c_mapping_free(&m);

  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("int *", "x", &m));
  c_mapping_free(&m);

  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("const volatile int", "x", &m));
  c_mapping_free(&m);

#ifdef CDD_BUILD_TESTS
  {
    /* Simulate OOMs */
    extern int g_cdd_strdup_fail;
    int rc_oom;
    int i;
    g_cdd_strdup_fail = 1;
    rc_oom = c_mapping_map_type("int", "x", &m);
    printf("RC_OOM=%d ENOMEM=%d\n", rc_oom, ENOMEM);
    ASSERT_EQ(ENOMEM, rc_oom);
    g_cdd_strdup_fail = 0;
    c_mapping_free(&m);

    /* Trigger inner_type/inner_ref duplication OOM */
    for (i = 1; i < 10; i++) {
      g_cdd_strdup_fail = i;
      c_mapping_init(&m);
      (void)c_mapping_map_type("int *", "x[]", &m);
      g_cdd_strdup_fail = 0;
      c_mapping_free(&m);
    }
  }
#endif

  /* Empty string */
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("", "x", &m));
  c_mapping_free(&m);

  /* void type (not pointer) */
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("void", "x", &m));
  c_mapping_free(&m);

  /* const volatile signed unsigned */
  c_mapping_init(&m);
  ASSERT_EQ(0,
            c_mapping_map_type("const volatile signed unsigned int", "x", &m));
  c_mapping_free(&m);

  /* array mapping */
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("struct Item *", "ptr[]", &m));
  c_mapping_free(&m);

  /* Test end-of-string qualifiers */
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("const", "x", &m));
  c_mapping_free(&m);
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("volatile", "x", &m));
  c_mapping_free(&m);
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("signed", "x", &m));
  c_mapping_free(&m);
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("unsigned", "x", &m));
  c_mapping_free(&m);

  /* Simulate strdup OOM inside mapping */
#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_strdup_fail;

    g_cdd_strdup_fail = 1;
    ASSERT_EQ(ENOMEM, c_mapping_map_type("enum MyEnum", "x", &m));
    g_cdd_strdup_fail = 0;

    g_cdd_strdup_fail = 1;
    ASSERT_EQ(ENOMEM, c_mapping_map_type("struct MyStruct *", "x[]", &m));
    g_cdd_strdup_fail = 0;
    c_mapping_free(&m);

    g_cdd_strdup_fail = 2;
    ASSERT_EQ(ENOMEM, c_mapping_map_type("struct MyStruct *", "x[]", &m));
    g_cdd_strdup_fail = 0;
    c_mapping_free(&m);

    g_cdd_strdup_fail = 2;
    ASSERT_EQ(ENOMEM, c_mapping_map_type("int *", "x", &m));
    g_cdd_strdup_fail = 0;
    c_mapping_free(&m);

    g_cdd_strdup_fail = 3;
    ASSERT_EQ(ENOMEM, c_mapping_map_type("struct MyStruct *", "x[]", &m));
    g_cdd_strdup_fail = 0;
    c_mapping_free(&m);
  }
#endif

  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("short", "x", &m));
  c_mapping_free(&m);

  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("double", "x", &m));
  c_mapping_free(&m);

  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("_Bool", "x", &m));
  c_mapping_free(&m);

  /* Test spaces after struct */
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("struct   Foo", "x", &m));
  c_mapping_free(&m);

  /* Test empty struct name */
  c_mapping_init(&m);
  ASSERT_EQ(0, c_mapping_map_type("struct ", "x", &m));
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
