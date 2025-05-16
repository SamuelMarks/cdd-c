#ifndef TEST_CODEGEN_H
#define TEST_CODEGEN_H

#ifndef _MSC_VER
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "code2schema.h"
#include "codegen.h"

/* Minimal enum members for testing */
static struct EnumMembers test_enum_members;

TEST test_write_enum_to_and_from_str_func(void) {
  FILE *tmpf = tmpfile();

  if (!tmpf)
    FAILm("Failed to open tmpfile");

  /* Setup enum members */
  enum_members_init(&test_enum_members);
  enum_members_add(&test_enum_members, "FOO");
  enum_members_add(&test_enum_members, "BAR");
  enum_members_add(&test_enum_members, "BAZ");

  write_enum_to_str_func(tmpf, "TestEnum", &test_enum_members);
  write_enum_from_str_func(tmpf, "TestEnum", &test_enum_members);

  /* Flush and rewind */
  fflush(tmpf);
  fseek(tmpf, 0, SEEK_SET);

  /* Check content non-empty */
  {
    char buf[1024];
    size_t len = fread(buf, 1, sizeof(buf) - 1, tmpf);
    buf[len] = 0;

    ASSERT(len > 0);
    ASSERT(strstr(buf, "int TestEnum_to_str") != NULL);
    ASSERT(strstr(buf, "int TestEnum_from_str") != NULL);

    /* Spot check enum members */
    ASSERT(strstr(buf, "case FOO") != NULL);
    ASSERT(strstr(buf, "case BAR") != NULL);
    ASSERT(strstr(buf, "case BAZ") != NULL);
  }

  enum_members_free(&test_enum_members);
  fclose(tmpf);

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

SUITE(codegen_suite) {
  RUN_TEST(test_write_enum_to_and_from_str_func);
  RUN_TEST(test_write_struct_functions);
}

#endif /* !TEST_CODEGEN_H */
