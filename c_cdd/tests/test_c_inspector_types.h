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

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_inspector.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "fs.h"

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

SUITE(c_inspector_types_suite) {
  RUN_TEST(test_scan_c23_enum_fixed_type);
  RUN_TEST(test_scan_c23_enum_fixed_type_whitespace);
  RUN_TEST(test_scan_classic_enum);
}

#endif /* TEST_C_INSPECTOR_TYPES_H */
