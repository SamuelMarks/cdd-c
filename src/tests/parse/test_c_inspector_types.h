/**
 * @file test_c_inspector_types.h
 * @brief Unit tests for C inspector type scanning logic.
 *
 * Verifies parsing of struct and enum definitions, including C23 specific
 * syntax.
 *
 * @author Samuel Marks
 */

#ifndef TEST_C_INSPECTOR_TYPES_H
#define TEST_C_INSPECTOR_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/parse/inspector.h"
#include "functions/parse/fs.h"
/* clang-format on */

/**
 * @brief test_scan_c23_enum_fixed_type
 * @return TEST
 */
TEST test_scan_c23_enum_fixed_type(void) {
  const char *filename = "test_c23_enum.h";
  const char *content = "enum E : long { A, B };";
  struct TypeDefList types;
  int rc;

  rc = write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  rc = type_def_list_init(&types);
  ASSERT_EQ(0, rc);

  rc = c_inspector_scan_file_types(filename, &types);
  ASSERT_EQ(0, rc);

  /* Should identify one enum named "E", stripping ": long" */
  ASSERT_EQ(1, types.size);
  ASSERT_EQ(KIND_ENUM, types.items[0].kind);
  ASSERT_STR_EQ("E", types.items[0].name);

  type_def_list_free(&types);
  remove(filename);
  PASS();
}

/**
 * @brief test_scan_c23_enum_fixed_type_whitespace
 * @return TEST
 */
TEST test_scan_c23_enum_fixed_type_whitespace(void) {
  const char *filename = "test_c23_enum_ws.h";
  const char *content = "enum  MyEnum  :  unsigned int  { X , Y };";
  struct TypeDefList types;
  int rc;

  rc = write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  type_def_list_init(&types);

  rc = c_inspector_scan_file_types(filename, &types);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, types.size);
  ASSERT_STR_EQ("MyEnum", types.items[0].name);

  type_def_list_free(&types);
  remove(filename);
  PASS();
}

/**
 * @brief test_scan_classic_enum
 * @return TEST
 */
TEST test_scan_classic_enum(void) {
  const char *filename = "test_classic.h";
  struct TypeDefList types;
  int rc;

  rc = write_to_file(filename, "enum Classic { ONE };");
  ASSERT_EQ(0, rc);

  type_def_list_init(&types);
  c_inspector_scan_file_types(filename, &types);

  ASSERT_EQ(1, types.size);
  ASSERT_STR_EQ("Classic", types.items[0].name);

  type_def_list_free(&types);
  remove(filename);
  PASS();
}

/**
 * @brief c_inspector_types_suite
 */

TEST test_inspector_nulls(void) {
  ASSERT_EQ(EINVAL, type_def_list_init(NULL));
  type_def_list_free(NULL);

  /* Create an empty struct/enum with NULL details to test free bounds */
  struct TypeDefList list = {0};
  type_def_list_init(&list);

  list.items = calloc(2, sizeof(struct TypeDefinition));
  list.size = 2;
  list.capacity = 2;

  list.items[0].kind = KIND_ENUM;
  list.items[0].details.enum_members = NULL;

  list.items[1].kind = KIND_STRUCT;
  list.items[1].details.struct_fields = NULL;

  type_def_list_free(&list);

  /* Create a struct with fields, one of which has NULL name */
  type_def_list_init(&list);
  list.items = calloc(1, sizeof(struct TypeDefinition));
  list.size = 1;
  list.capacity = 1;
  list.items[0].kind = KIND_STRUCT;
  list.items[0].details.struct_fields = calloc(1, sizeof(struct StructFields));
  list.items[0].details.struct_fields->fields =
      calloc(1, sizeof(struct StructField));
  list.items[0].details.struct_fields->size = 1;
  list.items[0].details.struct_fields->capacity = 1;
  type_def_list_free(&list);

  PASS();
}

SUITE(c_inspector_types_suite) {
  RUN_TEST(test_scan_c23_enum_fixed_type);
  RUN_TEST(test_scan_c23_enum_fixed_type_whitespace);
  RUN_TEST(test_scan_classic_enum);
  RUN_TEST(test_inspector_nulls);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C_INSPECTOR_TYPES_H */
