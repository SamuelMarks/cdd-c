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

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse_mapping.h"

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
  PASS();
}

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

  PASS();
}

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

  PASS();
}

TEST test_mapping_array(void) {
  struct OpenApiTypeMapping m;
  int rc;

  rc = c_mapping_map_type("int", "ids[]", &m);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_TYPE_ARRAY, m.kind);
  /* The "type" field indicates item type */
  ASSERT_STR_EQ("integer", m.oa_type);
  c_mapping_free(&m);

  PASS();
}

TEST test_mapping_bool(void) {
  struct OpenApiTypeMapping m;
  int rc;

  rc = c_mapping_map_type("bool", "flag", &m);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("boolean", m.oa_type);
  c_mapping_free(&m);
  PASS();
}

TEST test_mapping_long(void) {
  struct OpenApiTypeMapping m;
  int rc;
  rc = c_mapping_map_type("unsigned long long", "big", &m);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("integer", m.oa_type);
  ASSERT_STR_EQ("int64", m.oa_format);
  c_mapping_free(&m);
  PASS();
}

TEST test_mapping_void_ptr(void) {
  struct OpenApiTypeMapping m;
  int rc;
  /* void* -> string (binary) */
  rc = c_mapping_map_type("void *", "data", &m);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("string", m.oa_type);
  ASSERT_STR_EQ("binary", m.oa_format);
  c_mapping_free(&m);
  PASS();
}

SUITE(c_mapping_suite) {
  RUN_TEST(test_mapping_int);
  RUN_TEST(test_mapping_string);
  RUN_TEST(test_mapping_struct_ref);
  RUN_TEST(test_mapping_array);
  RUN_TEST(test_mapping_bool);
  RUN_TEST(test_mapping_long);
  RUN_TEST(test_mapping_void_ptr);
}

#endif /* TEST_C_MAPPING_H */
