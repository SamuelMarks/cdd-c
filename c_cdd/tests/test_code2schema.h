#ifndef TEST_CODE2SCHEMA_H
#define TEST_CODE2SCHEMA_H

#include <code2schema.h>
#include <greatest.h>

TEST test_write_enum_functions(void) {
  struct EnumMembers em;
  enum_members_init(&em);
  enum_members_add(&em, "FOO");
  enum_members_add(&em, "BAR");
  enum_members_add(&em, "UNKNOWN");
  {
    FILE *tmp = tmpfile();
    ASSERT(tmp != NULL);
    write_enum_to_str_func(tmp, "MyEnum", &em);
    write_enum_from_str_func(tmp, "MyEnum", &em);
    fclose(tmp);
  }
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
  ASSERT_EQ(1, parse_struct_member_line("struct Point *p;", &sf));
  struct_fields_free(&sf);
  PASS();
}

/* Minimal struct fields for testing */
static struct StructFields test_struct_fields;

TEST test_write_struct_functions(void) {
  FILE *tmpf = tmpfile();

  if (!tmpf)
    FAILm("Failed to open tmpfile");

  struct_fields_init(&test_struct_fields);

  /* Add fields */
  struct_fields_add(&test_struct_fields, "str_field", "string", NULL);
  struct_fields_add(&test_struct_fields, "int_field", "integer", NULL);
  struct_fields_add(&test_struct_fields, "bool_field", "boolean", NULL);
  struct_fields_add(&test_struct_fields, "num_field", "number", NULL);
  struct_fields_add(&test_struct_fields, "obj_field", "object", "NestedType");
  struct_fields_add(&test_struct_fields, "enum_field", "enum", "MyEnum");

  /* Write various functions */
  write_struct_debug_func(tmpf, "TestStruct", &test_struct_fields);
  write_struct_deepcopy_func(tmpf, "TestStruct", &test_struct_fields);
  write_struct_default_func(tmpf, "TestStruct", &test_struct_fields);
  write_struct_display_func(tmpf, "TestStruct", &test_struct_fields);
  write_struct_eq_func(tmpf, "TestStruct", &test_struct_fields);
  write_struct_from_jsonObject_func(tmpf, "TestStruct", &test_struct_fields);
  write_struct_from_json_func(tmpf, "TestStruct");
  write_struct_to_json_func(tmpf, "TestStruct", &test_struct_fields);
  write_struct_cleanup_func(tmpf, "TestStruct", &test_struct_fields);

  fflush(tmpf);
  fseek(tmpf, 0, SEEK_SET);

  {
    enum { n = 5120 };
    char buf[n];
    size_t len = fread(buf, 1, n - 1, tmpf);
    buf[len] = 0;
    ASSERT(len > 0);

    /* Spot check function names */
    ASSERT(strstr(buf, "int TestStruct_debug") != NULL);
    ASSERT(strstr(buf, "int TestStruct_deepcopy") != NULL);
    ASSERT(strstr(buf, "int TestStruct_default") != NULL);
    ASSERT(strstr(buf, "int TestStruct_display") != NULL);
    ASSERT(strstr(buf, "int TestStruct_eq") != NULL);
    ASSERT(strstr(buf, "int TestStruct_from_jsonObject") != NULL);
    ASSERT(strstr(buf, "int TestStruct_from_json") != NULL);
    ASSERT(strstr(buf, "int TestStruct_to_json") != NULL);
    /*ASSERT(strstr(buf, "void TestStruct_cleanup") != NULL);*/

    /* Spot check presence of typed fields */
    ASSERT(strstr(buf, "ret->str_field = strdup") != NULL);
    ASSERT(strstr(buf, "ret->int_field = (int)json_object_get_number") != NULL);
    ASSERT(strstr(buf, "ret->bool_field = json_object_get_boolean") != NULL);
    ASSERT(strstr(buf, "ret->num_field = json_object_get_number") != NULL);

    /* Spot check nested struct and enum handling */
    ASSERT(strstr(buf, "int rc = NestedType_from_jsonObject") != NULL);
    ASSERT(strstr(buf, "int rc = MyEnum_from_str") != NULL);
  }

  struct_fields_free(&test_struct_fields);
  fclose(tmpf);

  PASS();
}

SUITE(code2schema_suite) {
  RUN_TEST(test_parse_struct_member_line);
  RUN_TEST(test_str_starts_with);
  RUN_TEST(test_struct_fields_manage);
  RUN_TEST(test_write_enum_functions);
  RUN_TEST(test_write_struct_functions);
}

#endif /* !TEST_CODE2SCHEMA_H */
