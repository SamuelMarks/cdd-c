#ifndef TEST_CODE2SCHEMA_H
#define TEST_CODE2SCHEMA_H

#include <stdint.h>
#include <string.h>

#include <greatest.h>

#include "fs.h"

#include <cdd_test_helpers/cdd_helpers.h>
#include <code2schema.h>
#include <codegen.h>

/* Updated test cases to reflect new return types (int vs void) */

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
    errno_t err = fopen_s(&tmp_fh, "tmp_enum_func.c", "w, ccs=UTF-8");
    if (err != 0 || tmp_fh == NULL)
      FAILm("Failed to open file for writing");
  }
#else
  tmp_fh = fopen("tmp_enum_func.c", "w");
  if (!tmp_fh)
    FAILm("Failed to open file for writing");
#endif

  ASSERT_EQ(0, write_enum_to_str_func(tmp_fh, "MyEnum", &em));
  ASSERT_EQ(0, write_enum_from_str_func(tmp_fh, "MyEnum", &em));
  fclose(tmp_fh);
  remove("tmp_enum_func.c");

  enum_members_free(&em);
  PASS();
}

TEST test_struct_fields_manage(void) {
  struct StructFields sf;
  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, struct_fields_add(&sf, "name", "string", NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "num", "integer", NULL));
  struct_fields_free(&sf);
  PASS();
}

TEST test_str_starts_with(void) {
  ASSERT(str_starts_with("enum Color", "enum"));
  ASSERT(!str_starts_with("structFoo", "enum"));
  PASS();
}

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
  PASS();
}

static struct StructFields test_struct_fields;
TEST test_write_struct_functions(void) {
  FILE *tmpf = tmpfile();

  if (!tmpf)
    FAILm("Failed to open tmpfile");

  struct_fields_init(&test_struct_fields);

  ASSERT_EQ(
      0, struct_fields_add(&test_struct_fields, "str_field", "string", NULL));
  ASSERT_EQ(
      0, struct_fields_add(&test_struct_fields, "int_field", "integer", NULL));

  ASSERT_EQ(0,
            write_struct_to_json_func(tmpf, "TestStruct", &test_struct_fields));
  fflush(tmpf);
  ASSERT_GT(ftell(tmpf), 0);

  struct_fields_free(&test_struct_fields);
  fclose(tmpf);

  PASS();
}

TEST test_struct_fields_overflow(void) {
  struct StructFields sf;
  uint8_t i;
  enum { n = 32 };
  ASSERT_EQ(0, struct_fields_init(&sf));
  for (i = 0; i < 200; ++i) {
    char name[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(name, sizeof(name), "f%d", i);
#else
    sprintf(name, "f%d", i);
#endif
    ASSERT_EQ(0, struct_fields_add(&sf, name, "string", NULL));
  }
  ASSERT_GT(sf.size, n * 2);
  struct_fields_free(&sf);

  PASS();
}

TEST test_enum_members_overflow(void) {
  struct EnumMembers em;
  uint8_t i;
  enum { n = 32 };
  ASSERT_EQ(0, enum_members_init(&em));
  for (i = 0; i < 200; ++i) {
    char name[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(name, sizeof(name), "E%d", i);
#else
    sprintf(name, "E%d", i);
#endif
    ASSERT_EQ(0, enum_members_add(&em, name));
  }
  ASSERT_GT(em.size, n * 2);
  enum_members_free(&em);
  PASS();
}

TEST test_trim_trailing(void) {
  enum { n = 32 };
  char a[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(a, n, "foo   \t;");
#else
  strcpy(a, "foo   \t;");
#endif
  trim_trailing(a);
  ASSERT_STR_EQ("foo", a);
  PASS();
}

TEST test_code2schema_main_bad_args(void) {
  char *argv[] = {"bad"};
  ASSERT_EQ(EXIT_FAILURE, code2schema_main(1, argv));
  PASS();
}

TEST test_code2schema_parsing_details(void) {
  char *argv[] = {"test_details.h", "test_details.json"};
  const char *header_content = "enum Color {RED,GREEN=5,BLUE,};\n"
                               "struct Point {};\n"
                               "struct Line { struct Point p1; };\n";
  ASSERT_EQ(0, write_to_file(argv[0], header_content));
  ASSERT_EQ(EXIT_SUCCESS, code2schema_main(2, argv));
  remove(argv[0]);
  remove(argv[1]);
  PASS();
}

TEST test_code2schema_parse_struct_and_enum(void) {
  char *argv[] = {"test1.h", "test1.schema.json"};
  const char *const filename = argv[0];
  char *json = argv[1];
  int rc = write_to_file(filename,
                         "enum Colors { RED, GREEN = 5, BLUE };\n"
                         "struct Point { double x; double y; int used; };\n");
  ASSERT_EQ(0, rc);
  rc = code2schema_main(2, argv);
  ASSERT_EQ(EXIT_SUCCESS, rc);
  remove(filename);
  remove(json);
  PASS();
}

TEST test_code2schema_file_not_found(void) {
  char *argv[] = {"no_such_file.h", "out.json"};
  ASSERT_EQ(EXIT_FAILURE, code2schema_main(2, argv));
  PASS();
}

TEST test_codegen_enum_null_args(void) {
  FILE *tmp = tmpfile();
  struct EnumMembers em_valid;
  struct EnumMembers em_null_members;
  struct EnumMembers *em_null = NULL;

  ASSERT(tmp);
  memset(&em_null_members, 0, sizeof(em_null_members));

  enum_members_init(&em_valid);

  /* Check that the functions don't crash on NULL/invalid arguments and return
   * EINVAL */
  ASSERT_EQ(EINVAL, write_enum_to_str_func(NULL, "E", &em_valid));
  ASSERT_EQ(EINVAL, write_enum_to_str_func(tmp, NULL, &em_valid));
  ASSERT_EQ(EINVAL, write_enum_to_str_func(tmp, "E", em_null));

  /* em_null_members.members is NULL so this triggers validation check */
  ASSERT_EQ(EINVAL, write_enum_to_str_func(tmp, "E", &em_null_members));

  ASSERT_EQ(EINVAL, write_enum_from_str_func(NULL, "E", &em_valid));
  ASSERT_EQ(EINVAL, write_enum_from_str_func(tmp, NULL, &em_valid));
  ASSERT_EQ(EINVAL, write_enum_from_str_func(tmp, "E", em_null));
  ASSERT_EQ(EINVAL, write_enum_from_str_func(tmp, "E", &em_null_members));

  enum_members_free(&em_valid);
  fclose(tmp);
  PASS();
}

TEST test_codegen_enum_with_unknown(void) {
  FILE *tmp = tmpfile();
  struct EnumMembers em;

  ASSERT(tmp);
  ASSERT_EQ(0, enum_members_init(&em));
  ASSERT_EQ(0, enum_members_add(&em, "A"));
  ASSERT_EQ(0, enum_members_add(&em, "UNKNOWN"));
  ASSERT_EQ(0, enum_members_add(&em, "B"));

  /* This tests that the generator functions handle "UNKNOWN" correctly */
  ASSERT_EQ(0, write_enum_to_str_func(tmp, "MyEnum", &em));
  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  rewind(tmp);

  ASSERT_EQ(0, write_enum_from_str_func(tmp, "MyEnum", &em));
  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  enum_members_free(&em);
  fclose(tmp);
  PASS();
}

TEST test_codegen_all_field_types(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;

  ASSERT(tmp);
  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_string", "string", NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_integer", "integer", NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_boolean", "boolean", NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_number", "number", NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_enum", "enum", "MyEnum"));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_object", "object", "MyStruct"));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_unhandled", "unhandled_type", NULL));

  /* Call all generator functions with this comprehensive struct fields */
  ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "TestStruct", &sf));
  ASSERT_EQ(0, write_struct_to_json_func(tmp, "TestStruct", &sf));
  ASSERT_EQ(0, write_struct_eq_func(tmp, "TestStruct", &sf));
  ASSERT_EQ(0, write_struct_cleanup_func(tmp, "TestStruct", &sf));
  ASSERT_EQ(0, write_struct_default_func(tmp, "TestStruct", &sf));
  ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "TestStruct", &sf));
  ASSERT_EQ(0, write_struct_display_func(tmp, "TestStruct", &sf));
  ASSERT_EQ(0, write_struct_debug_func(tmp, "TestStruct", &sf));

  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_codegen_empty_struct_and_enum(void) {
  FILE *tmp = tmpfile();
  struct EnumMembers em;
  struct StructFields sf;

  ASSERT(tmp);
  ASSERT_EQ(0, enum_members_init(&em));
  ASSERT_EQ(0, struct_fields_init(&sf));

  ASSERT_EQ(0, write_enum_to_str_func(tmp, "EmptyEnum", &em));
  ASSERT_EQ(0, write_enum_from_str_func(tmp, "EmptyEnum", &em));

  ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "EmptyStruct", &sf));
  ASSERT_EQ(0, write_struct_to_json_func(tmp, "EmptyStruct", &sf));
  ASSERT_EQ(0, write_struct_eq_func(tmp, "EmptyStruct", &sf));
  ASSERT_EQ(0, write_struct_cleanup_func(tmp, "EmptyStruct", &sf));
  ASSERT_EQ(0, write_struct_default_func(tmp, "EmptyStruct", &sf));
  ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "EmptyStruct", &sf));
  ASSERT_EQ(0, write_struct_display_func(tmp, "EmptyStruct", &sf));
  ASSERT_EQ(0, write_struct_debug_func(tmp, "EmptyStruct", &sf));

  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  enum_members_free(&em);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_codegen_struct_null_args(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf_valid;
  struct StructFields *sf_null = NULL;

  ASSERT(tmp);
  struct_fields_init(&sf_valid);
  struct_fields_add(&sf_valid, "field", "string", NULL);

  ASSERT_EQ(EINVAL, write_struct_from_jsonObject_func(NULL, "S", &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_from_jsonObject_func(tmp, NULL, &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_from_jsonObject_func(tmp, "S", sf_null));

  ASSERT_EQ(EINVAL, write_struct_from_json_func(NULL, "S"));
  ASSERT_EQ(EINVAL, write_struct_from_json_func(tmp, NULL));

  ASSERT_EQ(EINVAL, write_struct_to_json_func(NULL, "S", &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_to_json_func(tmp, NULL, &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_to_json_func(tmp, "S", sf_null));

  ASSERT_EQ(EINVAL, write_struct_eq_func(NULL, "S", &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_eq_func(tmp, NULL, &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_eq_func(tmp, "S", sf_null));

  ASSERT_EQ(EINVAL, write_struct_cleanup_func(NULL, "S", &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_cleanup_func(tmp, NULL, &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_cleanup_func(tmp, "S", sf_null));

  ASSERT_EQ(EINVAL, write_struct_default_func(NULL, "S", &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_default_func(tmp, NULL, &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_default_func(tmp, "S", sf_null));

  ASSERT_EQ(EINVAL, write_struct_deepcopy_func(NULL, "S", &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_deepcopy_func(tmp, NULL, &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_deepcopy_func(tmp, "S", sf_null));

  ASSERT_EQ(EINVAL, write_struct_display_func(NULL, "S", &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_display_func(tmp, NULL, &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_display_func(tmp, "S", sf_null));

  ASSERT_EQ(EINVAL, write_struct_debug_func(NULL, "S", &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_debug_func(tmp, NULL, &sf_valid));
  ASSERT_EQ(EINVAL, write_struct_debug_func(tmp, "S", sf_null));

  struct_fields_free(&sf_valid);
  fclose(tmp);
  PASS();
}

SUITE(code2schema_suite) {
  RUN_TEST(test_write_enum_functions);
  RUN_TEST(test_struct_fields_manage);
  RUN_TEST(test_str_starts_with);
  RUN_TEST(test_parse_struct_member_line);
  RUN_TEST(test_write_struct_functions);
  RUN_TEST(test_struct_fields_overflow);
  RUN_TEST(test_enum_members_overflow);
  RUN_TEST(test_trim_trailing);
  RUN_TEST(test_code2schema_main_bad_args);
  RUN_TEST(test_code2schema_file_not_found);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#else
  RUN_TEST(test_code2schema_parsing_details);
  RUN_TEST(test_code2schema_parse_struct_and_enum);
#endif
  RUN_TEST(test_codegen_enum_null_args);
  RUN_TEST(test_codegen_enum_with_unknown);
  RUN_TEST(test_codegen_all_field_types);
  RUN_TEST(test_codegen_empty_struct_and_enum);
  RUN_TEST(test_codegen_struct_null_args);
}

#endif /* !TEST_CODE2SCHEMA_H */
