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
TEST test_scan_c23_enum_fixed_type(void) {
  const char *filename = "test_c23_enum.h";
  const char *content = "enum E : long { A, B };";
  struct TypeDefList types;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif

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
  g_fail_io_after = -1;
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
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif

  rc = write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  type_def_list_init(&types);

  rc = c_inspector_scan_file_types(filename, &types);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, types.size);
  ASSERT_STR_EQ("MyEnum", types.items[0].name);

  type_def_list_free(&types);
  remove(filename);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_scan_classic_enum
 * @return TEST
 */
TEST test_scan_classic_enum(void) {
  const char *filename = "test_classic.h";
  struct TypeDefList types;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif

  rc = write_to_file(filename, "enum Classic { ONE };");
  ASSERT_EQ(0, rc);

  type_def_list_init(&types);
  c_inspector_scan_file_types(filename, &types);

  ASSERT_EQ(1, types.size);
  ASSERT_STR_EQ("Classic", types.items[0].name);

  type_def_list_free(&types);
  remove(filename);
  g_fail_io_after = -1;
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
  /* 3. Hit capacity logic and k < end_idx without LBRACE */
  {
    struct FuncSigList sig_list;
    func_sig_list_init(&sig_list);
    c_inspector_extract_signatures(
        "void f1() {} void f2() {} void f3() {} void f4() {} void f5() {} "
        "void f6() {} void f7() {} void f8() {} void f9() {} void f10() {}"
        "void proto(int a);",
        &sig_list);
    func_sig_list_free(&sig_list);
  }

  /* 4. Hit n > start_idx false condition */
  {
    struct FuncSigList sig_list;
    func_sig_list_init(&sig_list);
    c_inspector_extract_signatures("(){}", &sig_list);
    func_sig_list_free(&sig_list);
  }

  /* 5. Another attempt for n > start_idx */
  {
    struct FuncSigList sig_list;
    func_sig_list_init(&sig_list);
    c_inspector_extract_signatures(" ( ) {}", &sig_list);
    func_sig_list_free(&sig_list);
  }

  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test_inspector_oom
 * @return TEST
 */
TEST test_inspector_oom(void) {
  const char *filename = "test_oom.h";
  const char *content = "enum E { A, B }; struct S { int x; };";
  struct TypeDefList types;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_alloc_fail;

  (void)rc;

  rc = write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  for (i = 1; i < 20; ++i) {
    g_cdd_alloc_fail = i;
    type_def_list_init(&types);
    rc = c_inspector_scan_file_types(filename, &types);
    g_cdd_alloc_fail = 0;
    type_def_list_free(&types);
    /* remove early break to force all limits */ /* Success means we passed the
                                                    allocations */
  }

  remove(filename);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_inspector_extract_sig_oom
 * @return TEST
 */
TEST test_inspector_extract_sig_oom(void) {
  const char *code = "/* doc */ void foo(int a, ...) { }";
  struct FuncSigList out;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_alloc_fail;

  (void)rc;

  for (i = 1; i < 40; ++i) {
    g_cdd_alloc_fail = i;
    func_sig_list_init(&out);
    rc = c_inspector_extract_signatures(code, &out);
    g_cdd_alloc_fail = 0;
    func_sig_list_free(&out);
    /* remove early break to force all limits */
  }

  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_inspector_extract_sig_nulls
 * @return TEST
 */
TEST test_inspector_extract_sig_nulls(void) {
  struct FuncSigList out;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, func_sig_list_init(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_inspector_extract_signatures(NULL, &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_inspector_extract_signatures("void foo(void){}", NULL));

  func_sig_list_init(&out);

  /* Missing name logic */
  ASSERT_EQ(CDD_C_SUCCESS,
            c_inspector_extract_signatures("void (void){}", &out));
  func_sig_list_free(&out);

  /* Variadic coverage */
  func_sig_list_init(&out);
  ASSERT_EQ(CDD_C_SUCCESS,
            c_inspector_extract_signatures("void bar(int a, ...) {}", &out));
  func_sig_list_free(&out);

  func_sig_list_free(NULL);

  /* File IO branch coverage */
  {
    struct TypeDefList types;
    type_def_list_init(&types);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c_inspector_scan_file_types(NULL, &types));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c_inspector_scan_file_types("f.c", NULL));

    /* Missing file */
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              c_inspector_scan_file_types("does_not_exist_999.h", &types));

    type_def_list_free(&types);
  }

  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_inspector_struct_empty_fields
 * @return TEST
 */
TEST test_inspector_struct_empty_fields(void) {
  const char *filename = "test_empty_struct.h";
  const char *content = "struct Empty { };";
  struct TypeDefList types;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif

  rc = write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  type_def_list_init(&types);
  rc = c_inspector_scan_file_types(filename, &types);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  type_def_list_free(&types);

  remove(filename);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_inspector_extract_sig_oom_2
 * @return TEST
 */
TEST test_inspector_extract_sig_oom_2(void) {
  struct FuncSigList out;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_alloc_fail;

  (void)rc;

  for (i = 1; i < 20; ++i) {
    g_cdd_alloc_fail = i;
    func_sig_list_init(&out);
    rc = c_inspector_extract_signatures(
        "void my_func() { /* body */ } void /* doc */ other(int a) {}", &out);
    g_cdd_alloc_fail = 0;
    func_sig_list_free(&out);
    /* remove early break to force all limits */
  }

  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_inspector_extract_sig_oom_3
 * @return TEST
 */
TEST test_inspector_extract_sig_oom_3(void) {
  struct FuncSigList out;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_alloc_fail;

  (void)rc;

  for (i = 1; i < 20; ++i) {
    g_cdd_alloc_fail = i;
    func_sig_list_init(&out);
    rc = c_inspector_extract_signatures(
        "void my_func() { /* body */ } void other(int a) {}", &out);
    g_cdd_alloc_fail = 0;
    func_sig_list_free(&out);
    /* remove early break to force all limits */
  }

  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_inspector_list_frees_with_partial_alloc
 * @return TEST
 */
TEST test_inspector_list_frees_with_partial_alloc(void) {
  /* Provide empty spans for func sigs */
  struct FuncSigList out;
  func_sig_list_init(&out);
  c_inspector_extract_signatures("void() {}", &out);
  func_sig_list_free(&out);
  PASS();
}

/**
 * @brief test_inspector_extract_sig_oom_4
 * @return TEST
 */
TEST test_inspector_extract_sig_oom_4(void) {
  struct FuncSigList out;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_alloc_fail;

  (void)rc;

  for (i = 1; i < 30; ++i) {
    g_cdd_alloc_fail = i;
    func_sig_list_init(&out);
    rc = c_inspector_extract_signatures(
        "/* doc1 */ void my_func() {} /* doc2 */ void other(int a) {}", &out);
    g_cdd_alloc_fail = 0;
    func_sig_list_free(&out);
    /* remove early break to force all limits */
  }

  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_inspector_struct_fields_oom_2
 * @return TEST
 */
TEST test_inspector_struct_fields_oom_2(void) {
  const char *filename = "test_oom_struct2.h";
  const char *content = "struct Empty { }; struct NotEmpty { int a; };";
  struct TypeDefList types;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_alloc_fail;

  (void)rc;

  rc = write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  for (i = 1; i < 30; ++i) {
    g_cdd_alloc_fail = i;
    type_def_list_init(&types);
    rc = c_inspector_scan_file_types(filename, &types);
    g_cdd_alloc_fail = 0;
    type_def_list_free(&types);
    /* remove early break to force all limits */
  }

  remove(filename);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_inspector_struct_empty_fields_oom_3
 * @return TEST
 */
TEST test_inspector_struct_empty_fields_oom_3(void) {
  const char *filename = "test_oom_struct3.h";
  const char *content = "struct Empty { };";
  struct TypeDefList types;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_alloc_fail;

  (void)rc;

  rc = write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  for (i = 1; i < 30; ++i) {
    g_cdd_alloc_fail = i;
    type_def_list_init(&types);
    rc = c_inspector_scan_file_types(filename, &types);
    g_cdd_alloc_fail = 0;
    type_def_list_free(&types);
    /* remove early break to force all limits */
  }

  remove(filename);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_inspector_struct_empty_fields_no_name_oom
 * @return TEST
 */
TEST test_inspector_struct_empty_fields_no_name_oom(void) {
  const char *filename = "test_oom_struct4.h";
  const char *content = "struct { }; struct { int a; }; enum { }; enum { A };";
  struct TypeDefList types;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_alloc_fail;

  (void)rc;

  rc = write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  for (i = 1; i < 30; ++i) {
    g_cdd_alloc_fail = i;
    type_def_list_init(&types);
    rc = c_inspector_scan_file_types(filename, &types);
    g_cdd_alloc_fail = 0;
    type_def_list_free(&types);
    /* remove early break to force all limits */
  }

  remove(filename);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_inspector_extract_sig_oom_tokenize
 * @return TEST
 */
TEST test_inspector_extract_sig_oom_tokenize(void) {
  struct FuncSigList out;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_alloc_fail;

  (void)rc;

  for (i = 1; i < 40; ++i) {
    g_cdd_alloc_fail = i;
    func_sig_list_init(&out);
    rc = c_inspector_extract_signatures("void foo(int a, ...) {}", &out);
    g_cdd_alloc_fail = 0;
    func_sig_list_free(&out);
  }

  PASS();
}

/**
 * @brief test_inspector_extract_sig_oom_parse_tokens
 * @return TEST
 */
TEST test_inspector_extract_sig_oom_parse_tokens(void) {
  struct FuncSigList out;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_alloc_fail;

  (void)rc;

  for (i = 40; i < 80; ++i) {
    g_cdd_alloc_fail = i;
    func_sig_list_init(&out);
    rc = c_inspector_extract_signatures("void foo(int a, ...) {}", &out);
    g_cdd_alloc_fail = 0;
    func_sig_list_free(&out);
  }

  PASS();
}

/**
 * @brief test_inspector_extract_sig_fail_tokenizer
 * @return TEST
 */
TEST test_inspector_extract_sig_fail_tokenizer(void) {
  struct FuncSigList out;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  func_sig_list_init(&out);
  rc = c_inspector_extract_signatures("int a = \xff\xff;", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, out.size);
  func_sig_list_free(&out);
  PASS();
}

/**
 * @brief test_inspector_extract_sig_fail_parser
 * @return TEST
 */
TEST test_inspector_extract_sig_fail_parser(void) {
  struct FuncSigList out;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  func_sig_list_init(&out);
  rc = c_inspector_extract_signatures("int f( { }", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, out.size);
  func_sig_list_free(&out);
  PASS();
}

/**
 * @brief test_inspector_extract_sig_fail_parser_err
 * @return TEST
 */
TEST test_inspector_extract_sig_fail_parser_err(void) {
  struct FuncSigList out;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  func_sig_list_init(&out);
  rc = c_inspector_extract_signatures("int f( { }", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, out.size);
  func_sig_list_free(&out);

  func_sig_list_init(&out);
  /* Covers: while (n > start_idx) exit */
  rc = c_inspector_extract_signatures("int () { }", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, out.size);
  func_sig_list_free(&out);

  PASS();
}

/**
 * @brief test_inspector_strdup_oom
 * @return TEST
 */
TEST test_inspector_strdup_oom(void) {
  const char *filename = "test_oom_strdup.h";
  const char *content = "enum MyEnum { A, B };";
  struct TypeDefList types;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  int i;
  extern int g_cdd_strdup_fail;

  rc = write_to_file(filename, content);
  ASSERT_EQ(0, rc);

  for (i = 1; i < 10; ++i) {
    g_cdd_strdup_fail = i;
    type_def_list_init(&types);
    rc = c_inspector_scan_file_types(filename, &types);
    g_cdd_strdup_fail = 0;
    type_def_list_free(&types);
    /* break removed to force all strdup fails */
  }

  remove(filename);
  PASS();
}

#ifndef _WIN32
#include <sys/stat.h>
#endif

/**
 * @brief test_inspector_io_error_perms
 * @return TEST
 */
TEST test_inspector_io_error_perms(void) {
  struct TypeDefList types;
#ifdef _MSC_VER
  int rc;
#else
  int rc __attribute__((unused));
#endif
  const char *filename = "test_no_read.h";

  write_to_file(filename, "");
#ifndef _WIN32
  chmod(filename, 0200);
#endif

  type_def_list_init(&types);
  rc = c_inspector_scan_file_types(filename, &types);
#ifndef _WIN32
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
#endif
  type_def_list_free(&types);

#ifndef _WIN32
  chmod(filename, 0644);
#endif
  remove(filename);

  PASS();
}
TEST test_inspector_branch_coverage(void) {
  const char *filename = "test_branch_cov.h";
  struct TypeDefList types;
  extern int g_enum_members_init_fail;
  extern int g_struct_fields_init_fail;
  extern int g_cdd_strdup_fail;

  /* 1. All-whitespace name: "struct    {" */
  write_to_file(filename, "struct    { int a; };");
  type_def_list_init(&types);
  c_inspector_scan_file_types(filename, &types);
  type_def_list_free(&types);

  /* 2. enum_members_init failure */
  write_to_file(filename, "enum X { A };");
  type_def_list_init(&types);
  g_enum_members_init_fail = 1;
  c_inspector_scan_file_types(filename, &types);
  g_enum_members_init_fail = 0;
  type_def_list_free(&types);

  /* 3. struct_fields_init failure */
  write_to_file(filename, "struct Y { int a; };");
  type_def_list_init(&types);
  g_struct_fields_init_fail = 1;
  c_inspector_scan_file_types(filename, &types);
  g_struct_fields_init_fail = 0;
  type_def_list_free(&types);

  /* 4. strdup failure inside struct field processing (line 277) */
  write_to_file(filename, "struct Z { int a; };");
  type_def_list_init(&types);
  g_cdd_strdup_fail = 1;
  c_inspector_scan_file_types(filename, &types);
  g_cdd_strdup_fail = 0;
  type_def_list_free(&types);

  /* 5. strdup failure inside enum field processing */
  write_to_file(filename, "enum W { A, B };");
  type_def_list_init(&types);
  g_cdd_strdup_fail = 1;
  c_inspector_scan_file_types(filename, &types);
  g_cdd_strdup_fail = 0;
  type_def_list_free(&types);

  /* 6. Zero capacity list */
  {
    struct FuncSigList zout = {0};
    c_inspector_extract_signatures("int f() {}", &zout);
    func_sig_list_free(&zout);
  }

  remove(filename);
  PASS();
}

SUITE(c_inspector_types_suite) {
  RUN_TEST(test_scan_c23_enum_fixed_type);
  RUN_TEST(test_scan_c23_enum_fixed_type_whitespace);
  RUN_TEST(test_scan_classic_enum);
  RUN_TEST(test_inspector_nulls);
  RUN_TEST(test_inspector_oom);
  RUN_TEST(test_inspector_extract_sig_oom);
  RUN_TEST(test_inspector_extract_sig_nulls);
  RUN_TEST(test_inspector_struct_empty_fields);
  RUN_TEST(test_inspector_extract_sig_oom_2);
  RUN_TEST(test_inspector_extract_sig_oom_3);
  RUN_TEST(test_inspector_list_frees_with_partial_alloc);
  RUN_TEST(test_inspector_extract_sig_oom_4);
  RUN_TEST(test_inspector_struct_fields_oom_2);
  RUN_TEST(test_inspector_struct_empty_fields_oom_3);
  RUN_TEST(test_inspector_struct_empty_fields_no_name_oom);
  RUN_TEST(test_inspector_extract_sig_oom_tokenize);
  RUN_TEST(test_inspector_extract_sig_oom_parse_tokens);
  RUN_TEST(test_inspector_extract_sig_fail_tokenizer);
  RUN_TEST(test_inspector_extract_sig_fail_parser);
  RUN_TEST(test_inspector_extract_sig_fail_parser_err);
  RUN_TEST(test_inspector_strdup_oom);
  RUN_TEST(test_inspector_io_error_perms);
  RUN_TEST(test_inspector_branch_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C_INSPECTOR_TYPES_H */
