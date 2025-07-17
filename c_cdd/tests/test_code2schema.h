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
    /* ASSERT(strstr(buf, "int rc = NestedType_from_jsonObject") != NULL);*/
    /* ASSERT(strstr(buf, "int rc = MyEnum_from_str") != NULL); */
  }

  struct_fields_free(&test_struct_fields);
  fclose(tmpf);

  PASS();
}

TEST test_struct_fields_overflow(void) {
  struct StructFields sf;
  int i;
  struct_fields_init(&sf);
  for (i = 0; i < 200; ++i) {
    char name[32];
    sprintf(name, "f%d", i);
    struct_fields_add(&sf, name, "string", NULL);
  }
  ASSERT_GT(sf.size, 64);
  struct_fields_free(&sf);
  PASS();
}

TEST test_enum_members_overflow(void) {
  struct EnumMembers em;
  int i;
  enum_members_init(&em);
  for (i = 0; i < 200; ++i) {
    char name[32];
    sprintf(name, "E%d", i);
    enum_members_add(&em, name);
  }
  ASSERT_GT(em.size, 64);
  enum_members_free(&em);
  PASS();
}

TEST test_trim_trailing(void) {
  char a[32] = "foo   \t;";
  trim_trailing(a);
  ASSERT_STR_EQ("foo", a);
  PASS();
}

TEST test_code2schema_main_bad_args(void) {
  char *argv[] = {"bad"};
  ASSERT_EQ(EXIT_FAILURE, code2schema_main(1, argv));
  PASS();
}

static int write_tmp_file(const char *const name, const char *const contents) {
  FILE *fh;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fh, name, "w");
    if (err != 0 || rootCmakeLists == NULL) {
      fprintf(stderr, "Failed to open %s for writing", name);
      free(fh);
      return EXIT_FAILURE;
    }
  }
#else
  fh = fopen(name, "w");
#endif
  fputs(contents, fh);
  return fclose(fh);
}

TEST test_code2schema_parse_struct_and_enum(void) {
  const char *const filename = "test1.h";
  int rc = write_tmp_file(filename,
                          "enum Colors { RED, GREEN = 5, BLUE };\n"
                          "struct Point { double x; double y; int used; };\n");
  const char *json = strdup("test1.schema.json");
  const char *argv[2] = {filename, NULL};
  argv[1] = json;
  ASSERT_EQ(0, rc);
  rc = code2schema_main(2, (char **)argv);
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

// Lines not matching anything (should be ignored)
TEST test_code2schema_unknown_lines(void) {
  const char *const filename = "test2.h";
  const int rc = write_tmp_file(
      filename, "// this is a comment\nvoid foo();\nstruct X { int a; };\n");
  char *json = strdup("test2.schema.json");
  const char *argv[] = {filename, NULL};
  argv[1] = json;
  ASSERT_EQ(0, rc);
  code2schema_main(2, (char **)argv);
  remove(filename);
  remove(json);
  PASS();
}

TEST test_code2schema_struct_member_edge_cases(void) {
  struct StructFields sf;
  struct_fields_init(&sf);
  ASSERT_EQ(0, parse_struct_member_line("unknown blah;", &sf));
  ASSERT_EQ(1, parse_struct_member_line("const char *mystr;", &sf));
  ASSERT_EQ(1, parse_struct_member_line("enum ABC *foo;", &sf));
  ASSERT_EQ(1, parse_struct_member_line("struct Y *z;", &sf));
  struct_fields_free(&sf);
  PASS();
}

TEST test_json_array_to_enum_members_null(void) {
  struct EnumMembers em;
  ASSERT_EQ(-1, json_array_to_enum_members(NULL, &em));
  ASSERT_EQ(-1,
            json_array_to_enum_members(
                (void *)0x123, NULL)); // invalid pointer, just to tick branch
  PASS();
}

TEST test_json_object_to_struct_fields_null(void) {
  struct StructFields sf = {0};
  ASSERT_EQ(-1, json_object_to_struct_fields(NULL, &sf));
  ASSERT_EQ(-1, json_object_to_struct_fields((void *)0x123, NULL));
  PASS();
}

TEST test_json_object_to_struct_fields_missing_properties(void) {
  JSON_Value *root = json_value_init_object();
  JSON_Object *obj = json_value_get_object(root);
  struct StructFields sf = {0};
  // object with no "properties"
  ASSERT_EQ(-2, json_object_to_struct_fields(obj, &sf));
  json_value_free(root);
  PASS();
}

TEST test_struct_fields_add_long_names(void) {
  struct StructFields sf;
  char bigname[80];
  struct_fields_init(&sf);
  memset(bigname, 'X', 79);
  bigname[79] = '\0';
  struct_fields_add(&sf, bigname, bigname, bigname);
  ASSERT_EQ(1, sf.size);
  struct_fields_free(&sf);
  PASS();
}

SUITE(code2schema_suite) {
  RUN_TEST(test_parse_struct_member_line);
  RUN_TEST(test_str_starts_with);
  RUN_TEST(test_struct_fields_manage);
  RUN_TEST(test_write_enum_functions);
  RUN_TEST(test_write_struct_functions);
  RUN_TEST(test_struct_fields_overflow);
  RUN_TEST(test_enum_members_overflow);
  RUN_TEST(test_trim_trailing);
  RUN_TEST(test_code2schema_main_bad_args);
  RUN_TEST(test_code2schema_parse_struct_and_enum);
  RUN_TEST(test_code2schema_file_not_found);
  RUN_TEST(test_code2schema_unknown_lines);
  RUN_TEST(test_code2schema_struct_member_edge_cases);
  RUN_TEST(test_json_array_to_enum_members_null);
  RUN_TEST(test_json_object_to_struct_fields_null);
  RUN_TEST(test_json_object_to_struct_fields_missing_properties);
  RUN_TEST(test_struct_fields_add_long_names);
}

#endif /* !TEST_CODE2SCHEMA_H */
