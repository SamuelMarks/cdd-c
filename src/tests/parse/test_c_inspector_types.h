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
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
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

TEST test_c_inspector_coverage(void) {
  struct TypeDefList types;
  int rc;
  int i;
  const char *filename = "test_inspector_oom.h";
  const char *content =
      "enum Color { RED, GREEN };\nstruct Point { int x; int y; };";

  write_to_file(filename, content);

  type_def_list_init(&types);

  /* IO Error - pass a file without read permissions */
  write_to_file("no_read.h", "int a;");
  chmod("no_read.h", 0000);
  rc = c_inspector_scan_file_types("no_read.h", &types);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  chmod("no_read.h", 0644);
  remove("no_read.h");

  /* Memory Error inside add_type_def string duplication */
  for (i = 0; i < 10; ++i) {
    type_def_list_init(&types);
    g_cdd_strdup_fail = i;
    rc = c_inspector_scan_file_types(filename, &types);
    if (rc == CDD_C_ERROR_MEMORY && g_cdd_strdup_fail != 0) {
      g_cdd_strdup_fail = 0;
      type_def_list_free(&types);
      break;
    }
    g_cdd_strdup_fail = 0;
    type_def_list_free(&types);
  }

  /* Memory Error when inserting ENUM or STRUCT */
  for (i = 0; i < 20; ++i) {
    type_def_list_init(&types);
    g_cdd_alloc_fail_countdown_countdown = i;
    rc = c_inspector_scan_file_types(filename, &types);
    if (rc == CDD_C_ERROR_MEMORY && g_cdd_alloc_fail_countdown_countdown != 0) {
      g_cdd_alloc_fail_countdown_countdown = 0;
      type_def_list_free(&types);
      break;
    }
    g_cdd_alloc_fail_countdown_countdown = 0;
    type_def_list_free(&types);
  }

  /* Memory Error inside list expansion (calloc in add_type_def array resize) */
  for (i = 0; i < 20; ++i) {
    type_def_list_init(&types);
    types.size = types.capacity; /* force realloc */
    g_cdd_alloc_fail_countdown_countdown = i;
    rc = c_inspector_scan_file_types(filename, &types);
    if (rc == CDD_C_ERROR_MEMORY && g_cdd_alloc_fail_countdown_countdown != 0) {
      g_cdd_alloc_fail_countdown_countdown = 0;
      types.size = 0; /* clean free */
      type_def_list_free(&types);
      break;
    }
    g_cdd_alloc_fail_countdown_countdown = 0;
    types.size = 0;
    type_def_list_free(&types);
  }

  remove(filename);
  PASS();
}

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
  struct TypeDefList list = {0};
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, type_def_list_init(NULL));
  type_def_list_free(NULL);

  /* Create an empty struct/enum with NULL details to test free bounds */
  type_def_list_init(&list);

  list.items = C_CDD_CALLOC(2, sizeof(struct TypeDefinition));
  list.size = 2;
  list.capacity = 2;

  list.items[0].kind = KIND_ENUM;
  list.items[0].details.enum_members = NULL;

  list.items[1].kind = KIND_STRUCT;
  list.items[1].details.struct_fields = NULL;

  type_def_list_free(&list);

  /* Create a struct with fields, one of which has NULL name */
  type_def_list_init(&list);
  list.items = C_CDD_CALLOC(1, sizeof(struct TypeDefinition));
  list.size = 1;
  list.capacity = 1;
  list.items[0].kind = KIND_STRUCT;
  list.items[0].details.struct_fields =
      C_CDD_CALLOC(1, sizeof(struct StructFields));
  list.items[0].details.struct_fields->fields =
      C_CDD_CALLOC(1, sizeof(struct StructField));
  list.items[0].details.struct_fields->size = 1;
  list.items[0].details.struct_fields->capacity = 1;
  type_def_list_free(&list);

  PASS();
}

TEST test_inspector_alloc_fail(void) {
  const char *filename = "test_alloc.h";
  struct TypeDefList types;
  int i, rc;
  extern int g_cdd_alloc_fail_countdown_countdown;
  extern C_CDD_EXPORT int g_cdd_strdup_fail;

  rc = write_to_file(filename, "struct Classic { int x; }; enum E { A };");
  ASSERT_EQ(0, rc);

  /* OOM loop for allocs */
  for (i = 1; i < 40; i++) {
    g_cdd_alloc_fail_countdown_countdown = i;
    type_def_list_init(&types);
    rc = c_inspector_scan_file_types(filename, &types);
    g_cdd_alloc_fail_countdown_countdown = 0;
    type_def_list_free(&types);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* OOM loop for strdups */
  for (i = 1; i < 40; i++) {
    g_cdd_strdup_fail = i;
    type_def_list_init(&types);
    rc = c_inspector_scan_file_types(filename, &types);
    g_cdd_strdup_fail = 0;
    type_def_list_free(&types);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  remove(filename);

  PASS();
}

TEST test_inspector_anonymous_struct(void) {
  const char *filename = "test_anon.h";
  struct TypeDefList types;
  int rc;

  rc = write_to_file(filename, "struct  { int x; }; enum { A };");
  ASSERT_EQ(0, rc);

  type_def_list_init(&types);
  c_inspector_scan_file_types(filename, &types);
  type_def_list_free(&types);

  remove(filename);

  PASS();
}

TEST test_inspector_invalid_args(void) {
  struct TypeDefList types;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_inspector_scan_file_types(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, func_sig_list_init(NULL));
  func_sig_list_free(NULL);

  /* IO Error (not found) */
  type_def_list_init(&types);
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            c_inspector_scan_file_types("unreadable.h", &types));
  type_def_list_free(&types);

  PASS();
}

TEST test_inspector_extract_signatures_coverage(void) {
  struct FuncSigList sigs;
  int i, rc;
  extern int g_cdd_alloc_fail_countdown_countdown;

  func_sig_list_init(&sigs);

  /* NULLs */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_inspector_extract_signatures(NULL, &sigs));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_inspector_extract_signatures("int a;", NULL));

  /* Valid signature */
  ASSERT_EQ(CDD_C_SUCCESS, c_inspector_extract_signatures(
                               "/* doc */\nint foo(int a, ...) {}", &sigs));
  ASSERT_EQ(1, sigs.size);
  ASSERT_STR_EQ("foo", sigs.items[0].name);
  ASSERT_EQ(1, sigs.items[0].is_variadic);
  func_sig_list_free(&sigs);

  /* Empty / syntax errors / parse_tokens fails */
  func_sig_list_init(&sigs);
  c_inspector_extract_signatures("struct A { int x; };", &sigs);
  c_inspector_extract_signatures("int foo (int a) {}",
                                 &sigs); /* whitespace before paren */
  c_inspector_extract_signatures("int () {}", &sigs); /* empty function name */
  c_inspector_extract_signatures("{", &sigs);         /* parse error */
  c_inspector_extract_signatures("\"", &sigs);        /* tokenize error */
  func_sig_list_free(&sigs);

  /* OOM loop */
  for (i = 1; i < 150; i++) {
    g_cdd_alloc_fail_countdown_countdown = i;
    g_cdd_strdup_fail = i;
    func_sig_list_init(&sigs);
    rc = c_inspector_extract_signatures("/* comment */\nint myfunc(int x) {}",
                                        &sigs);
    func_sig_list_free(&sigs);
    g_cdd_alloc_fail_countdown_countdown = 0;
    g_cdd_strdup_fail = 0;
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* name resolution branches */
  func_sig_list_init(&sigs);
  c_inspector_extract_signatures("int ()(int a) {}",
                                 &sigs); /* no identifier before paren */
  func_sig_list_free(&sigs);

  PASS();
}
SUITE(c_inspector_types_suite) {
  RUN_TEST(test_c_inspector_coverage);
  RUN_TEST(test_scan_c23_enum_fixed_type);
  RUN_TEST(test_scan_c23_enum_fixed_type_whitespace);
  RUN_TEST(test_scan_classic_enum);
  RUN_TEST(test_inspector_nulls);
  RUN_TEST(test_inspector_alloc_fail);
  RUN_TEST(test_inspector_anonymous_struct);
  RUN_TEST(test_inspector_invalid_args);
  RUN_TEST(test_inspector_extract_signatures_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C_INSPECTOR_TYPES_H */
