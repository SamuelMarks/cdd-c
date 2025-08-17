#ifndef TEST_CODE2SCHEMA_H
#define TEST_CODE2SCHEMA_H

#include <stdint.h>
#include <string.h>

#include <greatest.h>

#include "fs.h"

#include <code2schema.h>
#include <codegen.h>
#include <cdd_test_helpers/cdd_helpers.h>

TEST test_write_enum_functions(void) {
  struct EnumMembers em;
  FILE *tmp_fh;

  enum_members_init(&em);
  enum_members_add(&em, "FOO");
  enum_members_add(&em, "BAR");
  enum_members_add(&em, "UNKNOWN");

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = tmpfile_s(&tmp_fh);
    if (err != 0 || tmp_fh == NULL)
      FAILm("Failed to open file for writing");
  }
#else
  tmp = tmpfile();
  if (!tmp) FAILm("Failed to open file for writing");
#endif

  write_enum_to_str_func(tmp_fh, "MyEnum", &em);
  write_enum_from_str_func(tmp_fh, "MyEnum", &em);
  fclose(tmp_fh);

  enum_members_free(&em);
  PASS();
}

TEST test_struct_fields_manage(void) {
  struct StructFields sf;
  struct_fields_init(&sf);
  struct_fields_add(&sf, "name", "string", NULL);
  struct_fields_add(&sf, "num", "integer", NULL);
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
  ASSERT_EQ(1, parse_struct_member_line("const char *foo;", &sf));
  ASSERT_EQ(1, parse_struct_member_line("int bar;", &sf));
  ASSERT_EQ(1, parse_struct_member_line("double x;", &sf));
  ASSERT_EQ(1, parse_struct_member_line("bool b;", &sf));
  ASSERT_EQ(1, parse_struct_member_line("enum Color *e;", &sf));
  ASSERT_EQ(1, parse_struct_member_line("struct Point * p;", &sf));
  struct_fields_free(&sf);
  PASS();
}

static struct StructFields test_struct_fields;
TEST test_write_struct_functions(void) {
  FILE *tmpf = tmpfile();

  if (!tmpf)
    FAILm("Failed to open tmpfile");

  struct_fields_init(&test_struct_fields);

  struct_fields_add(&test_struct_fields, "str_field", "string", NULL);
  struct_fields_add(&test_struct_fields, "int_field", "integer", NULL);

  write_struct_to_json_func(tmpf, "TestStruct", &test_struct_fields);
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
  struct_fields_init(&sf);
  for (i = 0; i < 200; ++i) {
    char name[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(name, sizeof(name), "f%d", i);
#else
    sprintf(name, "f%d", i);
#endif
    struct_fields_add(&sf, name, "string", NULL);
  }
  ASSERT_GT(sf.size, n * 2);
  struct_fields_free(&sf);

  PASS();
}

TEST test_enum_members_overflow(void) {
  struct EnumMembers em;
  uint8_t i;
  enum { n = 32 };
  enum_members_init(&em);
  for (i = 0; i < 200; ++i) {
    char name[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(name, sizeof(name), "E%d", i);
#else
    sprintf(name, "E%d", i);
#endif
    enum_members_add(&em, name);
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
  ASSERT_EQ(0, code2schema_main(2, argv));
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
  ASSERT_EQ(0, rc);
  remove(filename);
  remove(json);
  PASS();
}

TEST test_code2schema_file_not_found(void) {
  char *argv[] = {"no_such_file.h", "out.json"};
  ASSERT_EQ(-1, code2schema_main(2, argv));
  PASS();
}

TEST test_code2schema_output_fail(void) {
  char *argv[] = {"test_out_fail.h", "a_dir"};
  const char *filename = argv[0];
  write_to_file(filename, "struct S{};");
  makedir("a_dir");
  ASSERT_EQ(-1, code2schema_main(2, argv));
  remove(filename);
  rmdir("a_dir");
  PASS();
}

TEST test_code2schema_complex_header(void) {
  char *argv[] = {"complex_header.h", "complex_header.json"};
  const char *filename = argv[0];
  const char *json_out = argv[1];
  write_to_file(filename, "//comment\n\n"
                          "enum E1{A,B,C,};\n"
                          "struct S1 {\n  int x;\n};\n\n"
                          "struct S2{const char *name;};\n"
                          "enum {\n  ANON_A, ANON_B\n};\n"
                          "struct { int anon_field; };\n");

  ASSERT_EQ(0, code2schema_main(2, argv));
  remove(filename);
  remove(json_out);
  PASS();
}

TEST test_code2schema_unterminated_defs(void) {
  char *argv[] = {"unterminated.h", "unterminated.json"};
  const char *filename = argv[0];
  const char *json_out = argv[1];
  write_to_file(filename, "struct S { int x; \n enum E { A, B");

  ASSERT_EQ(0, code2schema_main(2, argv));

  remove(filename);
  remove(json_out);
  PASS();
}

TEST test_parse_struct_member_line_ignore(void) {
  struct StructFields sf;
  struct_fields_init(&sf);
  /* Should be ignored, not a pointer and not a simple type */
  ASSERT_EQ(0, parse_struct_member_line("char foo[10];", &sf));
  ASSERT_EQ(0, sf.size);
  struct_fields_free(&sf);
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

  /* Check that the functions don't crash on NULL/invalid arguments */
  write_enum_to_str_func(NULL, "E", &em_valid);
  write_enum_to_str_func(tmp, NULL, &em_valid);
  write_enum_to_str_func(tmp, "E", em_null);
  write_enum_to_str_func(tmp, "E", &em_null_members);

  write_enum_from_str_func(NULL, "E", &em_valid);
  write_enum_from_str_func(tmp, NULL, &em_valid);
  write_enum_from_str_func(tmp, "E", em_null);
  write_enum_from_str_func(tmp, "E", &em_null_members);

  enum_members_free(&em_valid);
  fclose(tmp);
  PASS();
}

TEST test_codegen_enum_with_unknown(void) {
  FILE *tmp = tmpfile();
  struct EnumMembers em;

  ASSERT(tmp);
  enum_members_init(&em);
  enum_members_add(&em, "A");
  enum_members_add(&em, "UNKNOWN");
  enum_members_add(&em, "B");

  /* This tests that the generator functions handle "UNKNOWN" correctly */
  write_enum_to_str_func(tmp, "MyEnum", &em);
  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  rewind(tmp);

  write_enum_from_str_func(tmp, "MyEnum", &em);
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
  struct_fields_init(&sf);
  struct_fields_add(&sf, "f_string", "string", NULL);
  struct_fields_add(&sf, "f_integer", "integer", NULL);
  struct_fields_add(&sf, "f_boolean", "boolean", NULL);
  struct_fields_add(&sf, "f_number", "number", NULL);
  struct_fields_add(&sf, "f_enum", "enum", "MyEnum");
  struct_fields_add(&sf, "f_object", "object", "MyStruct");
  struct_fields_add(&sf, "f_unhandled", "unhandled_type", NULL);

  /* Call all generator functions with this comprehensive struct fields */
  write_struct_from_jsonObject_func(tmp, "TestStruct", &sf);
  write_struct_to_json_func(tmp, "TestStruct", &sf);
  write_struct_eq_func(tmp, "TestStruct", &sf);
  write_struct_cleanup_func(tmp, "TestStruct", &sf);
  write_struct_default_func(tmp, "TestStruct", &sf);
  write_struct_deepcopy_func(tmp, "TestStruct", &sf);
  write_struct_display_func(tmp, "TestStruct", &sf);
  write_struct_debug_func(tmp, "TestStruct", &sf);

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
  enum_members_init(&em);
  struct_fields_init(&sf);

  write_enum_to_str_func(tmp, "EmptyEnum", &em);
  write_enum_from_str_func(tmp, "EmptyEnum", &em);

  write_struct_from_jsonObject_func(tmp, "EmptyStruct", &sf);
  write_struct_to_json_func(tmp, "EmptyStruct", &sf);
  write_struct_eq_func(tmp, "EmptyStruct", &sf);
  write_struct_cleanup_func(tmp, "EmptyStruct", &sf);
  write_struct_default_func(tmp, "EmptyStruct", &sf);
  write_struct_deepcopy_func(tmp, "EmptyStruct", &sf);
  write_struct_display_func(tmp, "EmptyStruct", &sf);
  write_struct_debug_func(tmp, "EmptyStruct", &sf);

  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  enum_members_free(&em);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_json_converters_error_paths(void) {
  struct EnumMembers em;
  struct StructFields sf;
  JSON_Value *val_arr, *val_obj;
  JSON_Array *j_arr;

  /* Test json_array_to_enum_members */
  ASSERT_EQ(-1, json_array_to_enum_members(NULL, &em));
  ASSERT_EQ(-1, json_array_to_enum_members((void *)1, NULL));
  val_arr = json_parse_string("[\"A\", null, \"B\"]");
  j_arr = json_value_get_array(val_arr);
  ASSERT_EQ(0, json_array_to_enum_members(j_arr, &em));
  ASSERT_EQ(2, em.size); /* NULL should be skipped */
  enum_members_free(&em);
  json_value_free(val_arr);

  /* Test json_object_to_struct_fields error paths */
  ASSERT_EQ(-1, json_object_to_struct_fields(NULL, &sf, NULL));
  ASSERT_EQ(-1, json_object_to_struct_fields((void *)1, NULL, NULL));
  val_obj = json_parse_string(
      "{}"); /* No 'properties', should be handled as valid empty object */
  ASSERT_EQ(0, json_object_to_struct_fields(json_value_get_object(val_obj), &sf,
                                            NULL));
  json_value_free(val_obj);

  val_obj = json_parse_string(
      "{\"properties\": {\"field1\": 123}}"); /* prop not object */
  ASSERT_EQ(0, json_object_to_struct_fields(json_value_get_object(val_obj), &sf,
                                            NULL));
  ASSERT_EQ(0, sf.size); /* Should skip bad property */
  struct_fields_free(&sf);
  json_value_free(val_obj);

  val_obj = json_parse_string(
      "{\"properties\": {\"field1\": {\"type\":\"string\"}, \"field2\": "
      "{\"$ref\":\"#/foo\"}}}");
  ASSERT_EQ(0, json_object_to_struct_fields(json_value_get_object(val_obj), &sf,
                                            NULL));
  ASSERT_EQ(2, sf.size);
  ASSERT_STR_EQ(sf.fields[0].type, "string");
  ASSERT_STR_EQ(sf.fields[0].ref, "");
  ASSERT_STR_EQ(sf.fields[1].type,
                "object"); /* fallback when schemas_obj is NULL */
  ASSERT_STR_EQ(sf.fields[1].ref, "#/foo");
  struct_fields_free(&sf);
  json_value_free(val_obj);

  PASS();
}

TEST test_struct_fields_free_null(void) {
  struct StructFields sf;
  memset(&sf, 0, sizeof(sf));
  /* Should not crash */
  struct_fields_free(&sf);
  PASS();
}

TEST test_code2schema_messy_header(void) {
  char *argv[] = {"messy_header.h", "messy_header.json"};
  const char *header_content =
      "enum E_Messy { A,, B, };\n" /* empty item */
      "struct S_Messy {\n"
      "  int field1;\n"
      "  some_unsupported_type field2;\n" /* should be ignored */
      "};\n";

  ASSERT_EQ(0, write_to_file(argv[0], header_content));
  ASSERT_EQ(0, code2schema_main(2, argv));

  remove(argv[0]);
  remove(argv[1]);

  PASS();
}

TEST test_codegen_enum_with_null_member(void) {
  FILE *tmp = tmpfile();
  struct EnumMembers em;

  ASSERT(tmp);
  enum_members_init(&em);
  enum_members_add(&em, "A");

  /* Manually add a NULL to test the null-check inside the loop */
  if (em.size >= em.capacity) {
    em.capacity *= 2;
    em.members = (char **)realloc(em.members, em.capacity * sizeof(char *));
    ASSERT(em.members != NULL);
  }
  em.members[em.size++] = NULL;

  enum_members_add(&em, "B");

  /* This tests that the generator functions handle a NULL member correctly */
  write_enum_to_str_func(tmp, "MyEnum", &em);
  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  rewind(tmp);

  write_enum_from_str_func(tmp, "MyEnum", &em);
  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  enum_members_free(&em);
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

  write_struct_from_jsonObject_func(NULL, "S", &sf_valid);
  write_struct_from_jsonObject_func(tmp, NULL, &sf_valid);
  write_struct_from_jsonObject_func(tmp, "S", sf_null);

  write_struct_from_json_func(NULL, "S");
  write_struct_from_json_func(tmp, NULL);

  write_struct_to_json_func(NULL, "S", &sf_valid);
  write_struct_to_json_func(tmp, NULL, &sf_valid);
  write_struct_to_json_func(tmp, "S", sf_null);

  write_struct_eq_func(NULL, "S", &sf_valid);
  write_struct_eq_func(tmp, NULL, &sf_valid);
  write_struct_eq_func(tmp, "S", sf_null);

  write_struct_cleanup_func(NULL, "S", &sf_valid);
  write_struct_cleanup_func(tmp, NULL, &sf_valid);
  write_struct_cleanup_func(tmp, "S", sf_null);

  write_struct_default_func(NULL, "S", &sf_valid);
  write_struct_default_func(tmp, NULL, &sf_valid);
  write_struct_default_func(tmp, "S", sf_null);

  write_struct_deepcopy_func(NULL, "S", &sf_valid);
  write_struct_deepcopy_func(tmp, NULL, &sf_valid);
  write_struct_deepcopy_func(tmp, "S", sf_null);

  write_struct_display_func(NULL, "S", &sf_valid);
  write_struct_display_func(tmp, NULL, &sf_valid);
  write_struct_display_func(tmp, "S", sf_null);

  write_struct_debug_func(NULL, "S", &sf_valid);
  write_struct_debug_func(tmp, NULL, &sf_valid);
  write_struct_debug_func(tmp, "S", sf_null);

  struct_fields_free(&sf_valid);
  fclose(tmp);
  PASS();
}

TEST test_code2schema_with_enum_field(void) {
  char *argv[] = {"test_enum_field.h", "test_enum_field.json"};
  const char *header_content = "enum MyEnum { V1, V2 };\n"
                               "struct MyStruct { enum MyEnum *e_field; };\n";
  int err;

  ASSERT_EQ(0, write_to_file(argv[0], header_content));
  err = code2schema_main(2, argv);
  ASSERT_EQ(0, err);

  remove(argv[0]);
  remove(argv[1]);

  PASS();
}

TEST test_code2schema_single_line_defs(void) {
  char *argv[] = {"oneline.h", "oneline.json"};
  const char *header_content =
      "enum E {A, B}; struct S {int x; const char* s;};";
  int err;

  ASSERT_EQ(0, write_to_file(argv[0], header_content));
  err = code2schema_main(2, argv);
  ASSERT_EQ(0, err);

  remove(argv[0]);
  remove(argv[1]);
  PASS();
}

TEST test_code2schema_forward_declarations(void) {
  char *argv[] = {"fwd.h", "fwd.json"};
  const char *header_content = "struct MyStruct;\n"
                               "enum MyEnum;\n"
                               "struct RealStruct { int x; };\n";
  int err;

  ASSERT_EQ(0, write_to_file(argv[0], header_content));
  err = code2schema_main(2, argv);
  ASSERT_EQ(0, err);

  remove(argv[0]);
  remove(argv[1]);

  PASS();
}

TEST test_parse_struct_member_unhandled_line(void) {
  struct StructFields sf;
  struct_fields_init(&sf);
  /* This should not be parsed as a field */
  ASSERT_EQ(0, parse_struct_member_line("struct Other s;", &sf));
  ASSERT_EQ(0, sf.size);
  /* This should not be parsed either */
  ASSERT_EQ(0, parse_struct_member_line("void* ptr;", &sf));
  ASSERT_EQ(0, sf.size);
  /* nor this */
  ASSERT_EQ(0, parse_struct_member_line("char *name;", &sf));
  ASSERT_EQ(0, sf.size);
  struct_fields_free(&sf);
  PASS();
}

TEST test_parse_struct_member_line_no_space_after_ptr(void) {
  struct StructFields sf;
  struct_fields_init(&sf);
  ASSERT_EQ(1, parse_struct_member_line("struct Point*p;", &sf));
  ASSERT_EQ(1, sf.size);
  ASSERT_STR_EQ("p", sf.fields[0].name);
  ASSERT_STR_EQ("object", sf.fields[0].type);
  ASSERT_STR_EQ("Point", sf.fields[0].ref);
  struct_fields_free(&sf);
  PASS();
}

TEST test_json_object_to_struct_fields_with_ref_resolution(void) {
  const char *schema_str = "{"
                           "\"properties\": {\"my_enum_field\": {\"$ref\": "
                           "\"#/components/schemas/MyEnum\"}},"
                           "\"components\": {\"schemas\": {\"MyEnum\": "
                           "{\"type\": \"string\", \"enum\": [\"A\"]}}}"
                           "}";
  JSON_Value *root_val = json_parse_string(schema_str);
  JSON_Object *root_obj, *components, *schemas;
  struct StructFields sf;
  ASSERT(root_val);
  root_obj = json_value_get_object(root_val);
  components = json_object_get_object(root_obj, "components");
  schemas = json_object_get_object(components, "schemas");

  ASSERT_EQ(0, json_object_to_struct_fields(root_obj, &sf, schemas));
  ASSERT_EQ(1, sf.size);
  ASSERT_STR_EQ("enum", sf.fields[0].type);

  struct_fields_free(&sf);
  json_value_free(root_val);
  PASS();
}

TEST test_get_type_from_ref_no_slash_or_null(void) {
  const char *ref_no_slash = "MyType";
  const char *type;

  type = get_type_from_ref(ref_no_slash);
  ASSERT_STR_EQ("MyType", type);

  type = get_type_from_ref(NULL);
  ASSERT_STR_EQ("", type);

  PASS();
}

TEST test_json_object_to_struct_fields_ref_no_type(void) {
  const char *schema_str = "{"
                           "\"properties\": {\"my_ref_field\": {\"$ref\": "
                           "\"#/components/schemas/MyRef\"}},"
                           "\"components\": {\"schemas\": {\"MyRef\": "
                           "{\"enum\": [\"A\"]}}}" /* No 'type' field */
                           "}";
  JSON_Value *root_val = json_parse_string(schema_str);
  JSON_Object *root_obj, *components, *schemas;
  struct StructFields sf;
  ASSERT(root_val);
  root_obj = json_value_get_object(root_val);
  components = json_object_get_object(root_obj, "components");
  schemas = json_object_get_object(components, "schemas");

  ASSERT_EQ(0, json_object_to_struct_fields(root_obj, &sf, schemas));
  ASSERT_EQ(1, sf.size);
  /* It should default to object because the ref'd schema is not an enum */
  ASSERT_STR_EQ("object", sf.fields[0].type);

  struct_fields_free(&sf);
  json_value_free(root_val);
  PASS();
}

TEST test_json_object_to_struct_fields_ref_to_object(void) {
  const char *schema_str = "{"
                           "\"properties\": {\"my_obj_field\": {\"$ref\": "
                           "\"#/c/s/MyObj\"}},"
                           "\"c\": {\"s\": {\"MyObj\": {\"type\": \"object\"}}}"
                           "}";
  JSON_Value *root_val = json_parse_string(schema_str);
  JSON_Object *root_obj, *c, *s;
  struct StructFields sf;
  ASSERT(root_val);
  root_obj = json_value_get_object(root_val);
  c = json_object_get_object(root_obj, "c");
  s = json_object_get_object(c, "s");

  ASSERT_EQ(0, json_object_to_struct_fields(root_obj, &sf, s));
  ASSERT_EQ(1, sf.size);
  ASSERT_STR_EQ("object", sf.fields[0].type);

  struct_fields_free(&sf);
  json_value_free(root_val);
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
  /* TODO: Get them to work on MSVC */
#else
  RUN_TEST(test_code2schema_parsing_details);
  RUN_TEST(test_code2schema_parse_struct_and_enum);
  RUN_TEST(test_code2schema_output_fail);
  RUN_TEST(test_code2schema_complex_header);
  RUN_TEST(test_code2schema_unterminated_defs);
  RUN_TEST(test_code2schema_with_enum_field);
  RUN_TEST(test_code2schema_single_line_defs);
  RUN_TEST(test_code2schema_forward_declarations);
#endif
  RUN_TEST(test_codegen_enum_null_args);
  RUN_TEST(test_codegen_enum_with_unknown);
  RUN_TEST(test_codegen_all_field_types);
  RUN_TEST(test_codegen_empty_struct_and_enum);
  RUN_TEST(test_json_converters_error_paths);
  RUN_TEST(test_struct_fields_free_null);
  RUN_TEST(test_codegen_enum_with_null_member);
  RUN_TEST(test_codegen_struct_null_args);
  RUN_TEST(test_parse_struct_member_unhandled_line);
  RUN_TEST(test_parse_struct_member_line_no_space_after_ptr);
  RUN_TEST(test_json_object_to_struct_fields_with_ref_resolution);
  RUN_TEST(test_json_object_to_struct_fields_ref_no_type);
  RUN_TEST(test_json_object_to_struct_fields_ref_to_object);
  RUN_TEST(test_get_type_from_ref_no_slash_or_null);
  RUN_TEST(test_parse_struct_member_line_ignore);
}

#endif /* !TEST_CODE2SCHEMA_H */
