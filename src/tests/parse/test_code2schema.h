#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
/**
 * @file test_code2schema.h
 * @brief Unit tests for code to schema conversion.
 */

#ifndef TEST_CODE2SCHEMA_H
#define TEST_CODE2SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(_MSC_VER) && _MSC_VER < 1600
typedef signed __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef signed __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "msvc/stdint.h"
#else
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "c_cdd/msvc/stdint.h"
#else
#include <stdint.h>
#endif
#endif
#else
#include "msvc/stdint.h"
#endif /* _MSC_VER */
#else
#include "msvc/stdint.h"
#endif /* _MSC_VER */
#endif
/* clang-format off */
#include "classes/emit/schema.h"
#include "c_cdd_export.h"
#include <string.h>
#include <greatest.h>
#include "functions/parse/fs.h"
#include "classes/parse/code2schema.h"
#include "functions/emit/codegen.h"
#include <cdd_test_helpers/cdd_helpers.h>

/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_cdd_strdup_fail;

/* Updated test cases to reflect new return types (int vs void) */

/**
 * @brief Unit test test_write_enum_functions.
 * @return Greatest test result.
 */
TEST test_write_enum_functions(void) {

  struct EnumMembers em;
  FILE *tmp_fh;

  ASSERT_EQ(0, enum_members_init(&em));
  ASSERT_EQ(0, enum_members_add(&em, "FOO"));
  ASSERT_EQ(0, enum_members_add(&em, "BAR"));
  ASSERT_EQ(0, enum_members_add(&em, "UNKNOWN"));

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&tmp_fh, "tmp_enum_func.c", "w");
    if (err != 0 || tmp_fh == NULL)
      FAILm("Failed to open file for writing");
  }
#elif defined(_MSC_VER)
  fopen_s(&tmp_fh, "tmp_enum_func.c", "w");
  if (!tmp_fh)
    FAILm("Failed to open file for writing");
#else

#if defined(_MSC_VER)
  if (fopen_s(&tmp_fh, "tmp_enum_func.c", "w") != 0)
    tmp_fh = NULL;
#else
  tmp_fh = fopen("tmp_enum_func.c", "w");
#endif
  ASSERT(tmp_fh != NULL);

#endif

  ASSERT_EQ(0, write_enum_to_str_func(tmp_fh, "MyEnum", &em, NULL));
  ASSERT_EQ(0, write_enum_from_str_func(tmp_fh, "MyEnum", &em, NULL));
  fclose(tmp_fh);
  remove("tmp_enum_func.c");

  enum_members_free(&em);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_struct_fields_manage.
 * @return Greatest test result.
 */
TEST test_struct_fields_manage(void) {

  struct StructFields sf;

  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, struct_fields_add(&sf, "name", "string", NULL, NULL, NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "num", "integer", NULL, NULL, NULL));
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_str_starts_with.
 * @return Greatest test result.
 */
TEST test_str_starts_with(void) {
  int _ast_str_starts_with_0 = 0;
  int _ast_str_starts_with_1 = 0;
  ASSERT((str_starts_with("enum Color", "enum", &_ast_str_starts_with_0),

          _ast_str_starts_with_0));

  ASSERT(!(str_starts_with("structFoo", "enum", &_ast_str_starts_with_1),

           _ast_str_starts_with_1));

  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_parse_struct_member_line.
 * @return Greatest test result.
 */
TEST test_parse_struct_member_line(void) {

  struct StructFields sf;

  struct_fields_init(&sf);

  /* 0 means success in new API */

  ASSERT_EQ(0, parse_struct_member_line("const char *foo;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("int bar;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("double x;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("bool b;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("enum Color *e;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("struct Point * p;", &sf));
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_parse_struct_member_bitfield.
 * @return Greatest test result.
 */
TEST test_parse_struct_member_bitfield(void) {

  struct StructFields sf;

  struct_fields_init(&sf);

  /* int x : 3; */

  ASSERT_EQ(0, parse_struct_member_line("int x : 3;", &sf));
  ASSERT_EQ(1, sf.size);
  ASSERT_STR_EQ("x", sf.fields[0].name);
  ASSERT_STR_EQ("3", sf.fields[0].bit_width);
  ASSERT_STR_EQ("integer", sf.fields[0].type);

  /* Whitespace variation: int y:5; */

  ASSERT_EQ(0, parse_struct_member_line("int y:5;", &sf));
  ASSERT_STR_EQ("y", sf.fields[1].name);
  ASSERT_STR_EQ("5", sf.fields[1].bit_width);

  /* Type variation: unsigned int z : 1; */

  ASSERT_EQ(0, parse_struct_member_line("unsigned int z : 1;", &sf));
  ASSERT_STR_EQ("z", sf.fields[2].name);
  ASSERT_STR_EQ("1", sf.fields[2].bit_width);

  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_parse_struct_member_format_mapping.
 * @return Greatest test result.
 */
TEST test_parse_struct_member_format_mapping(void) {

  struct StructFields sf;
  struct StructField *field;
  struct StructField *arr_field;

  struct_fields_init(&sf);

  ASSERT_EQ(0, parse_struct_member_line("long id;", &sf));
  ASSERT_EQ(1, sf.size);
  field = &sf.fields[0];
  ASSERT_STR_EQ("id", field->name);
  ASSERT_STR_EQ("integer", field->type);
  ASSERT_STR_EQ("int64", field->format);

  ASSERT_EQ(0, parse_struct_member_line("long ids[];", &sf));
  ASSERT_EQ(2, sf.size);
  arr_field = &sf.fields[1];
  ASSERT_STR_EQ("ids", arr_field->name);
  ASSERT_STR_EQ("array", arr_field->type);
  ASSERT(arr_field->items_extra_json != NULL);
  ASSERT(strstr(arr_field->items_extra_json, "\"format\":\"int64\"") != NULL);

  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

/** @brief Test definition test_struct_fields. */
static struct StructFields test_struct_fields;

/**
 * @brief Unit test test_write_struct_functions.
 * @return Greatest test result.
 */
TEST test_write_struct_functions(void) {
  FILE *tmpf;
#if defined(_MSC_VER)
  if (((tmpf = cdd_test_tmpfile_global()) == NULL))
    tmpf = NULL;
#else
  tmpf = cdd_test_tmpfile_global();
#endif

  ASSERT(tmpf != NULL);

  struct_fields_init(&test_struct_fields);

  ASSERT_EQ(0, struct_fields_add(&test_struct_fields, "str_field", "string",

                                 NULL, NULL, NULL));

  ASSERT_EQ(0, struct_fields_add(&test_struct_fields, "int_field", "integer",

                                 NULL, NULL, NULL));

  ASSERT_EQ(0, write_struct_to_json_func(tmpf, "TestStruct",

                                         &test_struct_fields, NULL));

  fflush(tmpf);
  ASSERT_GT(ftell(tmpf), 0);

  struct_fields_free(&test_struct_fields);
  if (tmpf)
    fclose(tmpf);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_struct_fields_overflow.
 * @return Greatest test result.
 */
TEST test_struct_fields_overflow(void) {

  struct StructFields sf;
  unsigned char i;
  enum { n = 32 };

  ASSERT_EQ(0, struct_fields_init(&sf));
  for (i = 0; i < 50; ++i) {

    char name[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(name, sizeof(name), "f%d", i);
#else

    CDD_SNPRINTF(name, sizeof(name), "f%d", i);

#endif

    ASSERT_EQ(0, struct_fields_add(&sf, name, "string", NULL, NULL, NULL));
  }

  ASSERT_GT(sf.size, 16);
  struct_fields_free(&sf);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_enum_members_overflow.
 * @return Greatest test result.
 */
TEST test_enum_members_overflow(void) {

  struct EnumMembers em;
  unsigned char i;
  enum { n = 32 };

  ASSERT_EQ(0, enum_members_init(&em));
  for (i = 0; i < 50; ++i) {

    char name[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(name, sizeof(name), "E%d", i);
#else

    CDD_SNPRINTF(name, sizeof(name), "E%d", i);

#endif

    ASSERT_EQ(0, enum_members_add(&em, name));
  }

  ASSERT_GT(em.size, 16);
  enum_members_free(&em);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_trim_trailing.
 * @return Greatest test result.
 */
TEST test_trim_trailing(void) {

  enum { n = 32 };
  char a[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(a, n, "foo   \t;");
#elif defined(_MSC_VER)
  strcpy_s(a, sizeof(a), "foo   \t;");
#else

  strcpy(a, "foo   \t;");

#endif

  trim_trailing(a);
  ASSERT_STR_EQ("foo", a);
  g_fail_io_after = -1;
  PASS();
}

/* g_malloc_fail_at */
/* g_malloc_calls */
/* test_code2schema_mock_malloc */
/* mock_free */

/* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */

/* test_code2schema_oom */
/* test_code2schema_branches */
/**
 * @brief Unit test test_code2schema_main_bad_args.
 * @return Greatest test result.
 */
TEST test_code2schema_main_bad_args(void) {

  /* code2schema expects 2 args: in out */

  char *argv[1];

  /* Passing 1 args */

  argv[0] = cdd_test_mut_str("bad");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, code2schema_main(1, argv));
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_code2schema_parsing_details.
 * @return Greatest test result.
 */
TEST test_code2schema_parsing_details(void) {
  char *argv[2];
  const char *header_content = "enum Color {RED,GREEN=5,BLUE,};\n"
                               "struct Point {};\n"
                               "struct Line { struct Point p1; };\n";
  argv[0] = cdd_test_mut_str("test_details.h");
  argv[1] = cdd_test_mut_str("test_details.json");
  ASSERT_EQ(0, write_to_file(argv[0], header_content)); /* Write to in-file */

  ASSERT_EQ(CDD_C_SUCCESS,

            code2schema_main(2, argv)); /* Call with 2 args (in, out) */

  remove(argv[0]);
  remove(argv[1]);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_code2schema_parse_struct_and_enum.
 * @return Greatest test result.
 */
TEST test_code2schema_parse_struct_and_enum(void) {
  char *argv[2];
  const char *filename;
  char *json;
  int out_val;
  char trim_buf[32] = "hello   ";
  char trim_buf2[32] = "hello ; ";
  int rc;

  argv[0] = cdd_test_mut_str("test1.h");
  argv[1] = cdd_test_mut_str("test1.schema.json");
  filename = argv[0];
  json = argv[1];
  rc = write_to_file(filename,
                     "enum Colors { RED, GREEN = 5, BLUE };\n"
                     "struct Point { double x; double y; int used; };\n");

  ASSERT_EQ(0, rc);

  ASSERT_EQ(0, str_starts_with("hello", "hel", &out_val));
  ASSERT(out_val != 0);
  ASSERT_EQ(0, str_starts_with("hello", "helo", &out_val));
  ASSERT(out_val == 0);
  ASSERT_EQ(0, str_starts_with(NULL, "hel", &out_val));
  ASSERT(out_val == 0);
  ASSERT_EQ(0, str_starts_with("hello", NULL, &out_val));
  ASSERT(out_val == 0);

  trim_trailing(trim_buf);
  ASSERT_STR_EQ("hello", trim_buf);

  trim_trailing(trim_buf2);
  ASSERT_STR_EQ("hello", trim_buf2);

  trim_trailing(NULL);

  rc = code2schema_main(2, argv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  remove(filename);
  remove(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_code2schema_file_not_found.
 * @return Greatest test result.
 */
TEST test_code2schema_file_not_found(void) {
  char *argv[2];

  argv[0] = cdd_test_mut_str("no_such_file.h");
  argv[1] = cdd_test_mut_str("out.json");
  ASSERT_EQ(CDD_C_ERROR_IO, code2schema_main(2, argv));
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_codegen_enum_null_args.
 * @return Greatest test result.
 */
TEST test_codegen_enum_null_args(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  {
    struct EnumMembers em_valid;
    struct EnumMembers em_null_members;

    struct EnumMembers *em_null = NULL;

    ASSERT(tmp);
    memset(&em_null_members, 0, sizeof(em_null_members));

    enum_members_init(&em_valid);

    /* Check that the functions don't crash on NULL/invalid arguments and return
     * CDD_C_ERROR_INVALID_ARGUMENT */

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_enum_to_str_func(NULL, "E", &em_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_enum_to_str_func(tmp, NULL, &em_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_enum_to_str_func(tmp, "E", em_null, NULL));

    /* em_null_members.members is NULL so this triggers validation check */

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_enum_to_str_func(tmp, "E", &em_null_members, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_enum_from_str_func(NULL, "E", &em_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_enum_from_str_func(tmp, NULL, &em_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_enum_from_str_func(tmp, "E", em_null, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_enum_from_str_func(tmp, "E", &em_null_members, NULL));

    enum_members_free(&em_valid);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Unit test test_codegen_enum_with_unknown.
 * @return Greatest test result.
 */
TEST test_codegen_enum_with_unknown(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  {
    struct EnumMembers em;

    ASSERT(tmp);
    ASSERT_EQ(0, enum_members_init(&em));
    ASSERT_EQ(0, enum_members_add(&em, "A"));
    ASSERT_EQ(0, enum_members_add(&em, "UNKNOWN"));
    ASSERT_EQ(0, enum_members_add(&em, "B"));

    /* This tests that the generator functions handle "UNKNOWN" correctly */

    ASSERT_EQ(0, write_enum_to_str_func(tmp, "MyEnum", &em, NULL));
    fseek(tmp, 0, SEEK_END);
    ASSERT_GT(ftell(tmp), 0L);

    rewind(tmp);

    ASSERT_EQ(0, write_enum_from_str_func(tmp, "MyEnum", &em, NULL));
    fseek(tmp, 0, SEEK_END);
    ASSERT_GT(ftell(tmp), 0L);

    enum_members_free(&em);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Unit test test_codegen_all_field_types.
 * @return Greatest test result.
 */
TEST test_codegen_all_field_types(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  {
    struct StructFields sf;

    ASSERT(tmp);
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0,
              struct_fields_add(&sf, "f_string", "string", NULL, NULL, NULL));
    ASSERT_EQ(0,

              struct_fields_add(&sf, "f_integer", "integer", NULL, NULL, NULL));

    ASSERT_EQ(0,

              struct_fields_add(&sf, "f_boolean", "boolean", NULL, NULL, NULL));

    ASSERT_EQ(0,
              struct_fields_add(&sf, "f_number", "number", NULL, NULL, NULL));
    ASSERT_EQ(0,
              struct_fields_add(&sf, "f_enum", "enum", "MyEnum", NULL, NULL));
    ASSERT_EQ(

        0,
        struct_fields_add(&sf, "f_object", "object", "MyStruct", NULL, NULL));

    ASSERT_EQ(0, struct_fields_add(&sf, "f_unhandled", "unhandled_type", NULL,

                                   NULL, NULL));

    /* Call all generator functions with this comprehensive struct fields */

    ASSERT_EQ(0,
              write_struct_from_jsonObject_func(tmp, "TestStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_to_json_func(tmp, "TestStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_eq_func(tmp, "TestStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_cleanup_func(tmp, "TestStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_default_func(tmp, "TestStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "TestStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_display_func(tmp, "TestStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_debug_func(tmp, "TestStruct", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    ASSERT_GT(ftell(tmp), 0L);

    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Unit test test_codegen_empty_struct_and_enum.
 * @return Greatest test result.
 */
TEST test_codegen_empty_struct_and_enum(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  {
    struct EnumMembers em;
    struct StructFields sf;

    ASSERT(tmp);
    ASSERT_EQ(0, enum_members_init(&em));
    ASSERT_EQ(0, struct_fields_init(&sf));

    ASSERT_EQ(0, write_enum_to_str_func(tmp, "EmptyEnum", &em, NULL));
    ASSERT_EQ(0, write_enum_from_str_func(tmp, "EmptyEnum", &em, NULL));

    ASSERT_EQ(0,

              write_struct_from_jsonObject_func(tmp, "EmptyStruct", &sf, NULL));

    ASSERT_EQ(0, write_struct_to_json_func(tmp, "EmptyStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_eq_func(tmp, "EmptyStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_cleanup_func(tmp, "EmptyStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_default_func(tmp, "EmptyStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "EmptyStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_display_func(tmp, "EmptyStruct", &sf, NULL));
    ASSERT_EQ(0, write_struct_debug_func(tmp, "EmptyStruct", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    ASSERT_GT(ftell(tmp), 0L);

    enum_members_free(&em);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Unit test test_codegen_struct_null_args.
 * @return Greatest test result.
 */
TEST test_codegen_struct_null_args(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  {
    struct StructFields sf_valid;

    struct StructFields *sf_null = NULL;

    ASSERT(tmp);
    struct_fields_init(&sf_valid);
    struct_fields_add(&sf_valid, "field", "string", NULL, NULL, NULL);

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_from_jsonObject_func(NULL, "S", &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_from_jsonObject_func(tmp, NULL, &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_from_jsonObject_func(tmp, "S", sf_null, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_from_json_func(NULL, "S", NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_from_json_func(tmp, NULL, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_to_json_func(NULL, "S", &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_to_json_func(tmp, NULL, &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_to_json_func(tmp, "S", sf_null, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_eq_func(NULL, "S", &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_eq_func(tmp, NULL, &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_eq_func(tmp, "S", sf_null, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_cleanup_func(NULL, "S", &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_cleanup_func(tmp, NULL, &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_cleanup_func(tmp, "S", sf_null, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_default_func(NULL, "S", &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_default_func(tmp, NULL, &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_default_func(tmp, "S", sf_null, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_deepcopy_func(NULL, "S", &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_deepcopy_func(tmp, NULL, &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_deepcopy_func(tmp, "S", sf_null, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_display_func(NULL, "S", &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_display_func(tmp, NULL, &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_display_func(tmp, "S", sf_null, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_debug_func(NULL, "S", &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_debug_func(tmp, NULL, &sf_valid, NULL));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

              write_struct_debug_func(tmp, "S", sf_null, NULL));

    struct_fields_free(&sf_valid);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Tests hostile OpenAPI property names, keywords, UTF-8, collisions,
 * enums, and full roundtrip through codegen -> inspector -> sync_code ->
 * code2schema.
 *
 * @return Greatest test result.
 */
TEST test_hostile_openapi_property_and_schema_names(void) {
  char *out = NULL;
  JSON_Value *root_val = NULL;
  JSON_Object *schemas_obj = NULL;
  JSON_Object *hostile_obj = NULL;
  JSON_Object *enum_obj = NULL;
  struct StructFields sf;
  struct StructFields enum_sf;
  struct StructField *f = NULL;
  const char *hdr_file = "test_hostile_roundtrip.h";
  const char *impl_file = "test_hostile_roundtrip.c";
  const char *out_schema_file = "test_hostile_roundtrip.json";
  FILE *hf = NULL;
  char *sync_argv[2];
  char *c2s_argv[2];
  JSON_Value *rt_val = NULL;
  JSON_Object *rt_schemas = NULL;
  JSON_Object *rt_hostile = NULL;
  JSON_Object *rt_props = NULL;
  JSON_Object *rt_enum = NULL;
  JSON_Array *rt_enum_arr = NULL;

  /* 1. Direct sanitize_identifier edge cases */
  ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("", &out));
  ASSERT_STR_EQ("Variant", out);
  C_CDD_FREE(out);

  ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("   \t\n ", &out));
  ASSERT_STR_EQ("field_", out);
  C_CDD_FREE(out);

  ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("123-abc", &out));
  ASSERT_STR_EQ("_123_abc", out);
  C_CDD_FREE(out);

  ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("__reserved", &out));
  ASSERT(out[0] != '_' || out[1] != '_');
  C_CDD_FREE(out);

  ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("_Upper", &out));
  ASSERT(out[0] != '_' || !(out[1] >= 'A' && out[1] <= 'Z'));
  C_CDD_FREE(out);

  ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("default", &out));
  ASSERT_STR_EQ("default_", out);
  C_CDD_FREE(out);

  ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("class", &out));
  ASSERT_STR_EQ("class_", out);
  C_CDD_FREE(out);

  ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("nullptr", &out));
  ASSERT_STR_EQ("nullptr_", out);
  C_CDD_FREE(out);

  ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("caf\xc3\xa9", &out));
  ASSERT_STR_EQ("caf_xc3_xa9", out);
  C_CDD_FREE(out);

  ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("---", &out));
  ASSERT(out != NULL && out[0] != '\0');
  C_CDD_FREE(out);

  {
    int needs = 0;
    const char *long_id =
        "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz012345";
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_needs_identifier_sanitization("ok", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, c2s_needs_identifier_sanitization(NULL, &needs));
    ASSERT_EQ(1, needs);
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_needs_identifier_sanitization("_Reserved", &needs));
    ASSERT_EQ(1, needs);
    ASSERT_EQ(CDD_C_SUCCESS, c2s_needs_identifier_sanitization("a__b", &needs));
    ASSERT_EQ(1, needs);
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_needs_identifier_sanitization(long_id, &needs));
    ASSERT_EQ(1, needs);
    ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier(long_id, &out));
    ASSERT(strlen(out) <= 59);
    C_CDD_FREE(out);
  }

  /* 2. Hostile schema & enum JSON mapping */
  {
    char json_buf[2048];
    const char *p1 =
        "{\"Hostile-Schema\":{\"type\":\"object\",\"properties\":{"
        "\"0\":{\"type\":\"integer\"},\"123-abc\":{\"type\":\"string\"},"
        "\"first name\":{\"type\":\"string\"},\"   \":{\"type\":\"string\"},"
        "\"content-type\":{\"type\":\"string\"},\"io.k8s.pod\":{\"type\":"
        "\"string\"},"
        "\"application/"
        "json\":{\"type\":\"string\"},\"$ref_prop\":{\"type\":\"string\"},";
    const char *p2 =
        "\"@id\":{\"type\":\"string\"},\"xml:lang\":{\"type\":\"string\"},"
        "\"caf\\u00e9\":{\"type\":\"string\"},\"foo-bar\":{\"type\":\"string\"}"
        ","
        "\"foo_bar\":{\"type\":\"string\"},\"foo bar\":{\"type\":\"string\"},"
        "\"foo.bar\":{\"type\":\"string\"},\"default\":{\"type\":\"integer\"},";
    const char *p3 =
        "\"struct\":{\"type\":\"integer\"},\"class\":{\"type\":\"integer\"},"
        "\"operator\":{\"type\":\"integer\"},"
        "\"items\":{\"type\":\"array\",\"items\":{\"type\":\"integer\"}},"
        "\"n_items\":{\"type\":\"integer\"},\"a*/b\":{\"type\":\"string\"}}},"
        "\"Hostile-Enum\":{\"type\":\"string\",\"enum\":[\"in-progress\","
        "\"1st\","
        "\"true\",\"foo bar\",\"\",\"a-b\",\"a_b\",\"a.b\",\"UNKNOWN\"]}}";
    CDD_SNPRINTF(json_buf, sizeof(json_buf), "%s%s%s", p1, p2, p3);
    root_val = json_parse_string(json_buf);
  }
  ASSERT(root_val != NULL);
  schemas_obj = json_value_get_object(root_val);
  hostile_obj = json_object_get_object(schemas_obj, "Hostile-Schema");
  enum_obj = json_object_get_object(schemas_obj, "Hostile-Enum");
  ASSERT(hostile_obj != NULL);
  ASSERT(enum_obj != NULL);

  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            json_object_to_struct_fields_ex(hostile_obj, &sf, schemas_obj,
                                            "Hostile-Schema"));
  ASSERT_STR_EQ("Hostile-Schema", sf.serde_rename);

  /* Verify collision deduplication: foo_bar, foo_bar_2, foo_bar_3, foo_bar_4 */
  f = NULL;
  struct_fields_get(&sf, "foo_bar", &f);
  ASSERT(f != NULL);
  ASSERT_STR_EQ("foo-bar", f->serde_rename);

  f = NULL;
  struct_fields_get(&sf, "foo_bar_2", &f);
  ASSERT(f != NULL);
  ASSERT_STR_EQ("foo_bar", f->serde_rename);

  f = NULL;
  struct_fields_get(&sf, "foo_bar_3", &f);
  ASSERT(f != NULL);
  ASSERT_STR_EQ("foo bar", f->serde_rename);

  f = NULL;
  struct_fields_get(&sf, "foo_bar_4", &f);
  ASSERT(f != NULL);
  ASSERT_STR_EQ("foo.bar", f->serde_rename);

  /* Verify array n_items counter collision resolution */
  f = NULL;
  struct_fields_get(&sf, "n_items_2", &f);
  ASSERT(f != NULL);
  ASSERT_STR_EQ("n_items", f->serde_rename);

  /* Verify enum sanitization and deduplication */
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&enum_sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            json_object_to_struct_fields_ex(enum_obj, &enum_sf, schemas_obj,
                                            "Hostile-Enum"));
  ASSERT_EQ(1, enum_sf.is_enum);
  ASSERT_EQ(9, enum_sf.enum_members.size);
  ASSERT_STR_EQ("in_progress", enum_sf.enum_members.members[0]);
  ASSERT_STR_EQ("in-progress", enum_sf.enum_members.serde_names[0]);
  ASSERT_STR_EQ("_1st", enum_sf.enum_members.members[1]);
  ASSERT_STR_EQ("1st", enum_sf.enum_members.serde_names[1]);
  ASSERT_STR_EQ("true_", enum_sf.enum_members.members[2]);
  ASSERT_STR_EQ("true", enum_sf.enum_members.serde_names[2]);
  ASSERT_STR_EQ("Variant", enum_sf.enum_members.members[4]);
  ASSERT_STR_EQ("a_b", enum_sf.enum_members.members[5]);
  ASSERT_STR_EQ("a-b", enum_sf.enum_members.serde_names[5]);
  ASSERT_STR_EQ("a_b_2", enum_sf.enum_members.members[6]);
  ASSERT_STR_EQ("a_b", enum_sf.enum_members.serde_names[6]);
  ASSERT_STR_EQ("a_b_3", enum_sf.enum_members.members[7]);
  ASSERT_STR_EQ("a.b", enum_sf.enum_members.serde_names[7]);
  ASSERT_STR_EQ("UNKNOWN_VAL", enum_sf.enum_members.members[8]);
  ASSERT_STR_EQ("UNKNOWN", enum_sf.enum_members.serde_names[8]);

  /* 3. Emit header with write_struct_declaration_h and write_enum_declaration_h
   */
#if defined(_MSC_VER)
  if (fopen_s(&hf, hdr_file, "w") != 0)
    hf = NULL;
#else
  hf = fopen(hdr_file, "w");
#endif
  ASSERT(hf != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            write_enum_declaration_h(hf, "Hostile_Enum", &enum_sf, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_declaration_h(hf, "Hostile_Schema", &sf, NULL));
  fclose(hf);

  /* 4. Run sync_code_main on the generated header */
  sync_argv[0] = (char *)(size_t)hdr_file;
  sync_argv[1] = (char *)(size_t)impl_file;
  ASSERT_EQ(CDD_C_SUCCESS, sync_code_main(2, sync_argv));

  /* 5. Run code2schema_main on the generated header and verify 1:1 roundtrip */
  c2s_argv[0] = (char *)(size_t)hdr_file;
  c2s_argv[1] = (char *)(size_t)out_schema_file;
  ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, c2s_argv));

  rt_val = json_parse_file(out_schema_file);
  ASSERT(rt_val != NULL);
  rt_schemas = json_object_get_object(
      json_object_get_object(json_value_get_object(rt_val), "components"),
      "schemas");
  ASSERT(rt_schemas != NULL);
  rt_hostile = json_object_get_object(rt_schemas, "Hostile-Schema");
  ASSERT(rt_hostile != NULL);
  rt_props = json_object_get_object(rt_hostile, "properties");
  ASSERT(rt_props != NULL);
  ASSERT(json_object_get_object(rt_props, "0") != NULL);
  ASSERT(json_object_get_object(rt_props, "123-abc") != NULL);
  ASSERT(json_object_get_object(rt_props, "first name") != NULL);
  ASSERT(json_object_get_object(rt_props, "content-type") != NULL);
  ASSERT(json_object_get_object(rt_props, "io.k8s.pod") != NULL);
  ASSERT(json_object_get_object(rt_props, "application/json") != NULL);
  ASSERT(json_object_get_object(rt_props, "foo-bar") != NULL);
  ASSERT(json_object_get_object(rt_props, "foo_bar") != NULL);
  ASSERT(json_object_get_object(rt_props, "foo bar") != NULL);
  ASSERT(json_object_get_object(rt_props, "foo.bar") != NULL);
  ASSERT(json_object_get_object(rt_props, "default") != NULL);
  ASSERT(json_object_get_object(rt_props, "class") != NULL);
  ASSERT(json_object_get_object(rt_props, "items") != NULL);
  ASSERT(json_object_get_object(rt_props, "n_items") != NULL);
  ASSERT(json_object_get_object(rt_props, "a*/b") != NULL);

  rt_enum = json_object_get_object(rt_schemas, "Hostile-Enum");
  ASSERT(rt_enum != NULL);
  rt_enum_arr = json_object_get_array(rt_enum, "enum");
  ASSERT(rt_enum_arr != NULL);
  ASSERT_STR_EQ("in-progress", json_array_get_string(rt_enum_arr, 0));
  ASSERT_STR_EQ("1st", json_array_get_string(rt_enum_arr, 1));
  ASSERT_STR_EQ("true", json_array_get_string(rt_enum_arr, 2));
  ASSERT_STR_EQ("foo bar", json_array_get_string(rt_enum_arr, 3));

  json_value_free(rt_val);
  struct_fields_free(&sf);
  struct_fields_free(&enum_sf);
  json_value_free(root_val);
  remove(hdr_file);
  remove(impl_file);
  remove(out_schema_file);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the code2schema_suite test suite.
 */
SUITE(code2schema_suite) {
  RUN_TEST(test_write_enum_functions);
  RUN_TEST(test_struct_fields_manage);
  RUN_TEST(test_str_starts_with);
  RUN_TEST(test_parse_struct_member_line);
  RUN_TEST(test_parse_struct_member_bitfield);
  RUN_TEST(test_parse_struct_member_format_mapping);
  RUN_TEST(test_write_struct_functions);
  RUN_TEST(test_struct_fields_overflow);
  RUN_TEST(test_enum_members_overflow);
  RUN_TEST(test_trim_trailing);
  RUN_TEST(test_code2schema_main_bad_args);

  RUN_TEST(test_code2schema_file_not_found);

  RUN_TEST(test_code2schema_parsing_details);
  RUN_TEST(test_code2schema_parse_struct_and_enum);

  RUN_TEST(test_codegen_enum_null_args);
  RUN_TEST(test_codegen_enum_with_unknown);
  RUN_TEST(test_codegen_all_field_types);
  RUN_TEST(test_codegen_empty_struct_and_enum);
  RUN_TEST(test_codegen_struct_null_args);
  RUN_TEST(test_hostile_openapi_property_and_schema_names);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CODE2SCHEMA_H */
