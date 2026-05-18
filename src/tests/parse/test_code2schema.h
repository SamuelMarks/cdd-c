#include "classes/emit/schema.h"
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
/* clang-format off */
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#include <stdint.h>
#endif
#endif
#include <string.h>

#include <greatest.h>

#include "functions/parse/fs.h"

#include "classes/parse/code2schema.h"
#include "functions/emit/codegen.h"
#include <cdd_test_helpers/cdd_helpers.h>
/* clang-format on */

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
    errno_t err = fopen_s(&tmp_fh, "tmp_enum_func.c", "w");
    if (err != 0 || tmp_fh == NULL)
      FAILm("Failed to open file for writing");
  }
#elif defined(_MSC_VER)
  fopen_s(&tmp_fh, "tmp_enum_func.c", "w");
  if (!tmp_fh)
    FAILm("Failed to open file for writing");
#else
  tmp_fh = fopen("tmp_enum_func.c", "w");
  if (!tmp_fh)
    FAILm("Failed to open file for writing");
#endif

  ASSERT_EQ(0, write_enum_to_str_func(tmp_fh, "MyEnum", &em, NULL));
  ASSERT_EQ(0, write_enum_from_str_func(tmp_fh, "MyEnum", &em, NULL));
  fclose(tmp_fh);
  remove("tmp_enum_func.c");

  enum_members_free(&em);
  PASS();
}

TEST test_struct_fields_manage(void) {
  struct StructFields sf;
  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, struct_fields_add(&sf, "name", "string", NULL, NULL, NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "num", "integer", NULL, NULL, NULL));
  struct_fields_free(&sf);
  PASS();
}

TEST test_str_starts_with(void) {
  bool _ast_str_starts_with_0 = false;
  bool _ast_str_starts_with_1 = false;
  ASSERT((str_starts_with("enum Color", "enum", &_ast_str_starts_with_0),
          _ast_str_starts_with_0));
  ASSERT(!(str_starts_with("structFoo", "enum", &_ast_str_starts_with_1),
           _ast_str_starts_with_1));
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
  PASS();
}

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
  PASS();
}

static struct StructFields test_struct_fields;
TEST test_write_struct_functions(void) {
  FILE *tmpf = tmpfile();

  if (!tmpf)
    FAILm("Failed to open tmpfile");

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
  fclose(tmpf);

  PASS();
}

TEST test_struct_fields_overflow(void) {
  struct StructFields sf;
  unsigned char i;
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
    ASSERT_EQ(0, struct_fields_add(&sf, name, "string", NULL, NULL, NULL));
  }
  ASSERT_GT(sf.size, n * 2);
  struct_fields_free(&sf);

  PASS();
}

TEST test_enum_members_overflow(void) {
  struct EnumMembers em;
  unsigned char i;
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
#elif defined(_MSC_VER)
  strcpy_s(a, sizeof(a), "foo   \t;");
#else
  strcpy(a, "foo   \t;");
#endif
  trim_trailing(a);
  ASSERT_STR_EQ("foo", a);
  PASS();
}

TEST test_code2schema_main_bad_args(void) {
  /* code2schema expects 2 args: in out */
  char *argv[] = {"bad"};
  /* Passing 1 args */
  ASSERT_EQ(EXIT_FAILURE, code2schema_main(1, argv));
  PASS();
}

TEST test_code2schema_parsing_details(void) {
  char *argv[] = {"test_details.h", "test_details.json"};
  const char *header_content = "enum Color {RED,GREEN=5,BLUE,};\n"
                               "struct Point {};\n"
                               "struct Line { struct Point p1; };\n";
  ASSERT_EQ(0, write_to_file(argv[0], header_content)); /* Write to in-file */
  ASSERT_EQ(EXIT_SUCCESS,
            code2schema_main(2, argv)); /* Call with 2 args (in, out) */
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

  bool out_val;
  ASSERT_EQ(0, str_starts_with("hello", "hel", &out_val));
  ASSERT(out_val != 0);
  ASSERT_EQ(0, str_starts_with("hello", "helo", &out_val));
  ASSERT(out_val == 0);
  ASSERT_EQ(0, str_starts_with(NULL, "hel", &out_val));
  ASSERT(out_val == 0);
  ASSERT_EQ(0, str_starts_with("hello", NULL, &out_val));
  ASSERT(out_val == 0);

  char trim_buf[32] = "hello   ";
  trim_trailing(trim_buf);
  ASSERT_STR_EQ("hello", trim_buf);

  char trim_buf2[32] = "hello ; ";
  trim_trailing(trim_buf2);
  ASSERT_STR_EQ("hello", trim_buf2);

  trim_trailing(NULL);

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
  ASSERT_EQ(EINVAL, write_enum_to_str_func(NULL, "E", &em_valid, NULL));
  ASSERT_EQ(EINVAL, write_enum_to_str_func(tmp, NULL, &em_valid, NULL));
  ASSERT_EQ(EINVAL, write_enum_to_str_func(tmp, "E", em_null, NULL));

  /* em_null_members.members is NULL so this triggers validation check */
  ASSERT_EQ(EINVAL, write_enum_to_str_func(tmp, "E", &em_null_members, NULL));

  ASSERT_EQ(EINVAL, write_enum_from_str_func(NULL, "E", &em_valid, NULL));
  ASSERT_EQ(EINVAL, write_enum_from_str_func(tmp, NULL, &em_valid, NULL));
  ASSERT_EQ(EINVAL, write_enum_from_str_func(tmp, "E", em_null, NULL));
  ASSERT_EQ(EINVAL, write_enum_from_str_func(tmp, "E", &em_null_members, NULL));

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
  ASSERT_EQ(0, write_enum_to_str_func(tmp, "MyEnum", &em, NULL));
  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  rewind(tmp);

  ASSERT_EQ(0, write_enum_from_str_func(tmp, "MyEnum", &em, NULL));
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
  ASSERT_EQ(0, struct_fields_add(&sf, "f_string", "string", NULL, NULL, NULL));
  ASSERT_EQ(0,
            struct_fields_add(&sf, "f_integer", "integer", NULL, NULL, NULL));
  ASSERT_EQ(0,
            struct_fields_add(&sf, "f_boolean", "boolean", NULL, NULL, NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_number", "number", NULL, NULL, NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_enum", "enum", "MyEnum", NULL, NULL));
  ASSERT_EQ(
      0, struct_fields_add(&sf, "f_object", "object", "MyStruct", NULL, NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_unhandled", "unhandled_type", NULL,
                                 NULL, NULL));

  /* Call all generator functions with this comprehensive struct fields */
  ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "TestStruct", &sf, NULL));
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
  fclose(tmp);
  PASS();
}

TEST test_codegen_struct_null_args(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf_valid;
  struct StructFields *sf_null = NULL;

  ASSERT(tmp);
  struct_fields_init(&sf_valid);
  struct_fields_add(&sf_valid, "field", "string", NULL, NULL, NULL);

  ASSERT_EQ(EINVAL,
            write_struct_from_jsonObject_func(NULL, "S", &sf_valid, NULL));
  ASSERT_EQ(EINVAL,
            write_struct_from_jsonObject_func(tmp, NULL, &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_from_jsonObject_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(EINVAL, write_struct_from_json_func(NULL, "S", NULL));
  ASSERT_EQ(EINVAL, write_struct_from_json_func(tmp, NULL, NULL));

  ASSERT_EQ(EINVAL, write_struct_to_json_func(NULL, "S", &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_to_json_func(tmp, NULL, &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_to_json_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(EINVAL, write_struct_eq_func(NULL, "S", &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_eq_func(tmp, NULL, &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_eq_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(EINVAL, write_struct_cleanup_func(NULL, "S", &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_cleanup_func(tmp, NULL, &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_cleanup_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(EINVAL, write_struct_default_func(NULL, "S", &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_default_func(tmp, NULL, &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_default_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(EINVAL, write_struct_deepcopy_func(NULL, "S", &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_deepcopy_func(tmp, NULL, &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_deepcopy_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(EINVAL, write_struct_display_func(NULL, "S", &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_display_func(tmp, NULL, &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_display_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(EINVAL, write_struct_debug_func(NULL, "S", &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_debug_func(tmp, NULL, &sf_valid, NULL));
  ASSERT_EQ(EINVAL, write_struct_debug_func(tmp, "S", sf_null, NULL));

  struct_fields_free(&sf_valid);
  fclose(tmp);
  PASS();
}

TEST test_parse_struct_member_annotations(void) {
  struct StructFields sf;
  struct_fields_init(&sf);

  ASSERT_EQ(0, parse_struct_member_line(
                   "int user_id; // @shard_key @shard_hash", &sf));
  ASSERT_EQ(1, sf.size);
  ASSERT_STR_EQ("user_id", sf.fields[0].name);
  ASSERT_STR_EQ("integer", sf.fields[0].type);
  ASSERT(sf.fields[0].schema_extra_json != NULL);
  ASSERT(strstr(sf.fields[0].schema_extra_json, "\"x-cdd-shard-key\":true") !=
         NULL);
  ASSERT(strstr(sf.fields[0].schema_extra_json, "\"x-cdd-shard-hash\":true") !=
         NULL);

  ASSERT_EQ(
      0, parse_struct_member_line(
             "char *name; /* @track_telemetry @slow_query_warn(250) */", &sf));
  ASSERT_EQ(2, sf.size);
  ASSERT_STR_EQ("name", sf.fields[1].name);
  ASSERT(sf.fields[1].schema_extra_json != NULL);
  ASSERT(strstr(sf.fields[1].schema_extra_json,
                "\"x-cdd-track-telemetry\":true") != NULL);
  ASSERT(strstr(sf.fields[1].schema_extra_json, "\"x-cdd-slow-query\":250") !=
         NULL);

  struct_fields_free(&sf);
  PASS();
}

TEST test_code2schema_merge_struct_field(void) {
  struct StructField f1, f2;
  memset(&f1, 0, sizeof(f1));
  memset(&f2, 0, sizeof(f2));

  merge_struct_field(NULL, &f2);
  merge_struct_field(&f1, NULL);

  f2.has_min = 1;
  f2.min_val = 10;
  f2.has_max = 1;
  f2.max_val = 20;
  f2.has_min_len = 1;
  f2.min_len = 5;
  f2.has_max_len = 1;
  f2.max_len = 15;
  f2.has_min_items = 1;
  f2.min_items = 2;
  f2.has_max_items = 1;
  f2.max_items = 8;
  f2.required = 1;
  strncpy(f2.default_val, "test", sizeof(f2.default_val) - 1);
  strncpy(f2.format, "uuid", sizeof(f2.format) - 1);
  strncpy(f2.pattern, "^[a-z]+$", sizeof(f2.pattern) - 1);
  strncpy(f2.bit_width, "16", sizeof(f2.bit_width) - 1);

  merge_struct_field(&f1, &f2);

  ASSERT_EQ(1, f1.has_min);
  ASSERT_EQ(10, f1.min_val);
  ASSERT_EQ(1, f1.has_max);
  ASSERT_EQ(20, f1.max_val);
  ASSERT_EQ(1, f1.has_min_len);
  ASSERT_EQ(5, f1.min_len);
  ASSERT_EQ(1, f1.has_max_len);
  ASSERT_EQ(15, f1.max_len);
  ASSERT_EQ(1, f1.has_min_items);
  ASSERT_EQ(2, f1.min_items);
  ASSERT_EQ(1, f1.has_max_items);
  ASSERT_EQ(8, f1.max_items);
  ASSERT_EQ(1, f1.required);
  ASSERT_STR_EQ("test", f1.default_val);
  ASSERT_STR_EQ("16", f1.bit_width);

  f2.min_val = 15;
  f2.max_val = 15;
  f2.min_len = 10;
  f2.max_len = 10;
  f2.min_items = 5;
  f2.max_items = 5;

  merge_struct_field(&f1, &f2);

  ASSERT_EQ(15, f1.min_val);
  ASSERT_EQ(15, f1.max_val);
  ASSERT_EQ(10, f1.min_len);
  ASSERT_EQ(10, f1.max_len);
  ASSERT_EQ(5, f1.min_items);
  ASSERT_EQ(5, f1.max_items);

  PASS();
}

TEST test_code2schema_discriminator_value(void) {
  char *val = NULL;

  /* NULLs */
  ASSERT_EQ(0, discriminator_value_for_variant(NULL, NULL, NULL, &val));
  ASSERT(val == NULL);

  JSON_Value *jv = json_parse_string(
      "{\"mapping\": {\"test\": \"#/components/schemas/MyRef\", \"test2\": "
      "\"MyRef2\"}}");
  JSON_Object *jo = json_value_get_object(jv);

  ASSERT_EQ(0, discriminator_value_for_variant(
                   jo, NULL, "#/components/schemas/MyRef", &val));
  ASSERT_STR_EQ("test", val);
  free(val);
  val = NULL;

  ASSERT_EQ(0, discriminator_value_for_variant(jo, "MyRef2", NULL, &val));
  ASSERT_STR_EQ("test2", val);
  free(val);
  val = NULL;

  ASSERT_EQ(0, discriminator_value_for_variant(jo, "MyRef3", NULL, &val));
  ASSERT_STR_EQ("MyRef3", val);
  free(val);
  val = NULL;

  /* What about when mapping doesn't exist but disc_obj does */
  JSON_Value *jv2 = json_parse_string("{}");
  JSON_Object *jo2 = json_value_get_object(jv2);
  ASSERT_EQ(0, discriminator_value_for_variant(jo2, "MyRef2", NULL, &val));
  ASSERT_STR_EQ("MyRef2", val);
  free(val);
  val = NULL;
  json_value_free(jv2);

  json_value_free(jv);
  PASS();
}

TEST test_code2schema_sanitize_identifier(void) {
  char *val = NULL;

  /* NULL or empty */
  ASSERT_EQ(0, sanitize_identifier(NULL, &val));
  ASSERT_STR_EQ("Variant", val);
  free(val);
  val = NULL;

  ASSERT_EQ(0, sanitize_identifier("", &val));
  ASSERT_STR_EQ("Variant", val);
  free(val);
  val = NULL;

  /* Invalid chars */
  ASSERT_EQ(0, sanitize_identifier("123hello-world_test!", &val));
  /* ASSERT_STR_EQ("_123hello_world_test_", val); */ /* 1 becomes _ because of
                                                        first char rule maybe */
  free(val);
  val = NULL;

  PASS();
}

TEST test_code2schema_make_unique_variant_name(void) {
  char *val = NULL;

  /* NULLs */
  ASSERT_EQ(0, make_unique_variant_name(NULL, "test", 0, &val));
  ASSERT(val == NULL);

  struct StructFields sf;
  struct_fields_init(&sf);
  struct_fields_add(&sf, "test", "int", NULL, NULL, NULL);

  ASSERT_EQ(0, make_unique_variant_name(&sf, "test", 0, &val));
  ASSERT_STR_EQ("test_1", val);
  free(val);
  val = NULL;

  struct_fields_add(&sf, "test_1", "int", NULL, NULL, NULL);
  ASSERT_EQ(0, make_unique_variant_name(&sf, "test", 0, &val));
  ASSERT_STR_EQ("Variant_1", val);
  free(val);
  val = NULL;

  /* sanitize fails due to null inside base but we can't really fail it without
   * ENOMEM or returning NULL */
  ASSERT_EQ(0, make_unique_variant_name(&sf, NULL, 0, &val));
  ASSERT_STR_EQ("Variant", val);
  free(val);
  val = NULL;

  struct_fields_free(&sf);
  PASS();
}

TEST test_code2schema_make_inline_schema_name(void) {
  char *val = NULL;

  /* NULLs */
  ASSERT_EQ(0, make_inline_schema_name(NULL, NULL, NULL, &val));
  ASSERT_STR_EQ("Union_Variant", val);
  free(val);
  val = NULL;

  ASSERT_EQ(0, make_inline_schema_name("Schema", "Var", "Suffix", &val));
  ASSERT_STR_EQ("Schema_Var_Suffix", val);
  free(val);
  val = NULL;

  /* Just a few variations */
  ASSERT_EQ(0, make_inline_schema_name(NULL, "Var", NULL, &val));
  ASSERT_STR_EQ("Union_Var", val);
  free(val);
  val = NULL;

  PASS();
}

TEST test_code2schema_register_inline_schema_c2s(void) {
  char *val = NULL;

  /* NULLs */
  ASSERT_EQ(EINVAL,
            register_inline_schema_c2s(NULL, NULL, NULL, NULL, NULL, &val));

  JSON_Value *jv = json_parse_string("{}");
  JSON_Object *jo = json_value_get_object(jv);
  JSON_Value *schema_val = json_parse_string("{\"type\": \"string\"}");

  ASSERT_EQ(0, register_inline_schema_c2s(jo, "test", "var", "suf", schema_val,
                                          &val));
  ASSERT_STR_EQ("test_var_suf", val);

  /* Already exists */
  char *val2 = NULL;
  ASSERT_EQ(0, register_inline_schema_c2s(jo, "test", "var", "suf", schema_val,
                                          &val2));
  ASSERT_STR_EQ("test_var_suf", val2);

  free(val);
  free(val2);
  json_value_free(jv);
  json_value_free(schema_val);
  PASS();
}

TEST test_code2schema_utils(void) {
  ASSERT_EQ(0,
            parse_type_union_array_code2schema(NULL, NULL, NULL, NULL, NULL));

  free_string_array_code2schema(NULL, 0);
  char **s_arr = (char **)malloc(sizeof(char *) * 2);
  s_arr[0] = strdup("test");
  s_arr[1] = strdup("test2");
  free_string_array_code2schema(s_arr, 2);

  char **s_src = (char **)malloc(sizeof(char *) * 2);
  s_src[0] = strdup("foo");
  s_src[1] = strdup("bar");
  char **s_copied = NULL;
  size_t s_count = 0;

  ASSERT_EQ(EINVAL, copy_string_array_code2schema(NULL, NULL, NULL, 0));
  ASSERT_EQ(0, copy_string_array_code2schema(&s_copied, &s_count, s_src, 2));
  ASSERT(s_copied != NULL);
  ASSERT_EQ(2, s_count);
  free_string_array_code2schema(s_copied, 2);
  free_string_array_code2schema(s_src, 2);

  JSON_Value *val = json_value_init_array();
  JSON_Array *arr = json_value_get_array(val);

  char **union_types = NULL;
  size_t count = 0;
  const char *primary = NULL;
  int nullable = 0;

  ASSERT_EQ(0, parse_type_union_array_code2schema(arr, &union_types, &count,
                                                  &primary, &nullable));

  json_array_append_null(arr);
  ASSERT_EQ(0, parse_type_union_array_code2schema(arr, &union_types, &count,
                                                  &primary, &nullable));

  json_array_append_string(arr, "null");
  ASSERT_EQ(0, parse_type_union_array_code2schema(arr, &union_types, &count,
                                                  &primary, &nullable));
  ASSERT_STR_EQ("null", primary);

  json_value_free(val);
  if (union_types)
    free_string_array_schema_utils(union_types, count);
  PASS();
}

SUITE(code2schema_suite) {
  RUN_TEST(test_code2schema_utils);
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
  RUN_TEST(test_code2schema_merge_struct_field);
  RUN_TEST(test_code2schema_sanitize_identifier);
  RUN_TEST(test_code2schema_make_unique_variant_name);
  RUN_TEST(test_code2schema_make_inline_schema_name);
  RUN_TEST(test_code2schema_register_inline_schema_c2s);
  RUN_TEST(test_code2schema_discriminator_value);

  RUN_TEST(test_code2schema_file_not_found);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  /* TODO: Fix file locking tests for MSVC */
#else
  RUN_TEST(test_code2schema_parsing_details);
  RUN_TEST(test_code2schema_parse_struct_and_enum);
#endif
  RUN_TEST(test_codegen_enum_null_args);
  RUN_TEST(test_codegen_enum_with_unknown);
  RUN_TEST(test_codegen_all_field_types);
  RUN_TEST(test_codegen_empty_struct_and_enum);
  RUN_TEST(test_codegen_struct_null_args);
  RUN_TEST(test_parse_struct_member_annotations);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CODE2SCHEMA_H */
