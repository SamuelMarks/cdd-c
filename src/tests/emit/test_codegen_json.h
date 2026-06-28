/**
 * @file test_codegen_json.h
 * @brief Unit tests for JSON Serialization generator.
 *
 * Verifies that the correct `parson` calls and `jasprintf` patterns
 * are emitted for various struct layouts.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_JSON_H
#define TEST_CODEGEN_JSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/json.h"
#include "classes/emit/struct.h" /* For struct_fields_init etc */
/* clang-format on */

/* Common Setup */
static void setup_json_fields(struct StructFields *sf) {
  struct_fields_init(sf);
  struct_fields_add(sf, "id", "integer", NULL, "0", NULL);
  struct_fields_add(sf, "data", "string", NULL, NULL, NULL);
}

/**
 * @brief test_json_to_plain
 * @return TEST
 */
TEST test_json_to_plain(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  long sz;
  char *content = NULL;

  ASSERT(tmp);
  setup_json_fields(&sf);
  printf("\nid flags: %d %d %d %d\n", sf.fields[0].has_min,
         sf.fields[0].has_max, sf.fields[0].exclusive_min,
         sf.fields[0].exclusive_max);

  ASSERT_EQ(0, write_struct_to_json_func(tmp, "Data", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  /* Check structure */
  ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"{\");"));
  ASSERT(strstr(content,
                "c89stringutils_jasprintf(json, \"\\\"id\\\": %d\", obj->id)"));
  ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"\\\"data\\\": "
                         "\\\"%s\\\"\", obj->data)"));
  ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"}\");"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_json_from_plain
 * @return TEST
 */
TEST test_json_from_plain(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  long sz;
  char *content = NULL;

  ASSERT(tmp);
  setup_json_fields(&sf);
  printf("\nid flags: %d %d %d %d\n", sf.fields[0].has_min,
         sf.fields[0].has_max, sf.fields[0].exclusive_min,
         sf.fields[0].exclusive_max);

  ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "Data", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  /* Check parson usage */
  ASSERT(strstr(content,
                "ret->id = (int)json_object_get_number(jsonObject, \"id\")"));
  ASSERT(strstr(content, "json_object_get_string(jsonObject, \"data\")"));
  ASSERT(strstr(content, "strdup(s)"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_json_recursive_obj
 * @return TEST
 */
TEST test_json_recursive_obj(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "child", "object", "ChildType", NULL, NULL);

  ASSERT_EQ(0, write_struct_to_json_func(tmp, "Parent", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  /* Check recursive call pattern */
  ASSERT(strstr(content, "rc = ChildType_to_json(obj->child, &s);"));
  ASSERT(strstr(content,
                "c89stringutils_jasprintf(json, \"\\\"child\\\": %s\", s);"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_json_array_logic
 * @return TEST
 */
TEST test_json_array_logic(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  /* Array of strings */
  struct_fields_add(&sf, "tags", "array", "string", NULL, NULL);

  ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "Post", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  /* Check array loop extraction */
  ASSERT(strstr(content, "json_object_get_array(jsonObject, \"tags\")"));
  ASSERT(strstr(content, "json_array_get_count(arr)"));
  ASSERT(strstr(content, "calloc(ret->n_tags, sizeof(char*))"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_json_guards
 * @return TEST
 */
TEST test_json_guards(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  struct CodegenJsonConfig config;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  setup_json_fields(&sf);
  printf("\nid flags: %d %d %d %d\n", sf.fields[0].has_min,
         sf.fields[0].has_max, sf.fields[0].exclusive_min,
         sf.fields[0].exclusive_max);
  /* Number with min/max */
  struct_fields_add(&sf, "num_bounded", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 100.0;
  sf.fields[sf.size - 1].exclusive_min = 1;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Integer with min/max exclusive */
  struct_fields_add(&sf, "int_bounded", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 10.0;
  sf.fields[sf.size - 1].exclusive_min = 1;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Number with inclusive min/max */
  struct_fields_add(&sf, "num_bounded_inc", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 100.0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 0;

  /* String with regex patterns */
  struct_fields_add(&sf, "pat_exact", "string", NULL, NULL, NULL);
  strcpy(sf.fields[sf.size - 1].pattern, "^exact$");
  struct_fields_add(&sf, "pat_prefix", "string", NULL, NULL, NULL);
  strcpy(sf.fields[sf.size - 1].pattern, "^prefix");
  struct_fields_add(&sf, "pat_suffix", "string", NULL, NULL, NULL);
  strcpy(sf.fields[sf.size - 1].pattern, "suffix$");
  struct_fields_add(&sf, "pat_contains", "string", NULL, NULL, NULL);
  strcpy(sf.fields[sf.size - 1].pattern, "contains");

  /* Integer with inclusive min/max */
  struct_fields_add(&sf, "int_bounded_inc", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 100.0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 0;

  /* Number with only max */
  struct_fields_add(&sf, "num_max_only", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 10.0;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Number with only exclusive min (bizarre but technically possible) */
  struct_fields_add(&sf, "num_ex_min_only", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 1;

  /* Number with only exclusive max */
  struct_fields_add(&sf, "num_ex_max_only", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Integer with only max */
  struct_fields_add(&sf, "int_max_only", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 10.0;
  sf.fields[sf.size - 1].exclusive_max = 0;

  /* Integer with only exclusive min */
  struct_fields_add(&sf, "int_ex_min_only", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 1;

  /* Integer with only exclusive max */
  struct_fields_add(&sf, "int_ex_max_only", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 1;

  config.guard_macro = "JSON_ENABLED";

  ASSERT_EQ(0, write_struct_from_json_func(tmp, "Data", &config));
  ASSERT_EQ(0, write_struct_array_from_json_func(tmp, "Data", &config));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "#ifdef JSON_ENABLED"));
  ASSERT(strstr(content, "#endif /* JSON_ENABLED */"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_struct_array_from_json
 * @return TEST
 */
TEST test_struct_array_from_json(void) {
  FILE *tmp = tmpfile();
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(0, write_struct_array_from_json_func(tmp, "Data", NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "int Data_array_from_json(const char *json_str, "
                         "struct Data ***out, size_t *out_len)"));
  ASSERT(strstr(content, "json_parse_string(json_str)"));
  ASSERT(strstr(content, "json_value_get_array(val)"));
  ASSERT(strstr(
      content, "Data_from_jsonObject(json_array_get_object(arr, i), &tmp[i])"));

  free(content);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_json_null_args
 * @return TEST
 */
TEST test_json_null_args(void) {
  FILE *tmp = tmpfile();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_to_json_func(NULL, "S", NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_from_json_func(NULL, "S", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_array_from_json_func(NULL, "S", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_array_from_json_func(tmp, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_from_jsonObject_func(NULL, "S", NULL, NULL));

  {
    FILE *readonly_f = tmpfile();
    struct StructFields sf;
    struct CodegenJsonConfig config;

    setup_json_fields(&sf);
    printf("\nid flags: %d %d %d %d\n", sf.fields[0].has_min,
           sf.fields[0].has_max, sf.fields[0].exclusive_min,
           sf.fields[0].exclusive_max);

    /* Number with min/max */
    struct_fields_add(&sf, "num_bounded", "number", NULL, NULL, NULL);
    sf.fields[sf.size - 1].has_min = 1;
    sf.fields[sf.size - 1].min_val = 0.0;
    sf.fields[sf.size - 1].has_max = 1;
    sf.fields[sf.size - 1].max_val = 100.0;
    sf.fields[sf.size - 1].exclusive_min = 1;
    sf.fields[sf.size - 1].exclusive_max = 1;

    /* Integer with min/max exclusive */
    struct_fields_add(&sf, "int_bounded", "integer", NULL, NULL, NULL);
    sf.fields[sf.size - 1].has_min = 1;
    sf.fields[sf.size - 1].min_val = 0.0;
    sf.fields[sf.size - 1].has_max = 1;
    sf.fields[sf.size - 1].max_val = 10.0;
    sf.fields[sf.size - 1].exclusive_min = 1;
    sf.fields[sf.size - 1].exclusive_max = 1;

    /* Number with inclusive min/max */
    struct_fields_add(&sf, "num_bounded_inc", "number", NULL, NULL, NULL);
    sf.fields[sf.size - 1].has_min = 1;
    sf.fields[sf.size - 1].min_val = 0.0;
    sf.fields[sf.size - 1].has_max = 1;
    sf.fields[sf.size - 1].max_val = 100.0;
    sf.fields[sf.size - 1].exclusive_min = 0;
    sf.fields[sf.size - 1].exclusive_max = 0;

    /* String with regex patterns */
    struct_fields_add(&sf, "pat_exact", "string", NULL, NULL, NULL);
    strcpy(sf.fields[sf.size - 1].pattern, "^exact$");
    struct_fields_add(&sf, "pat_prefix", "string", NULL, NULL, NULL);
    strcpy(sf.fields[sf.size - 1].pattern, "^prefix");
    struct_fields_add(&sf, "pat_suffix", "string", NULL, NULL, NULL);
    strcpy(sf.fields[sf.size - 1].pattern, "suffix$");
    struct_fields_add(&sf, "pat_contains", "string", NULL, NULL, NULL);
    strcpy(sf.fields[sf.size - 1].pattern, "contains");

    /* Integer with inclusive min/max */
    struct_fields_add(&sf, "int_bounded_inc", "integer", NULL, NULL, NULL);
    sf.fields[sf.size - 1].has_min = 1;
    sf.fields[sf.size - 1].min_val = 0.0;
    sf.fields[sf.size - 1].has_max = 1;
    sf.fields[sf.size - 1].max_val = 100.0;
    sf.fields[sf.size - 1].exclusive_min = 0;
    sf.fields[sf.size - 1].exclusive_max = 0;

    /* Number with only max */
    struct_fields_add(&sf, "num_max_only", "number", NULL, NULL, NULL);
    sf.fields[sf.size - 1].has_min = 0;
    sf.fields[sf.size - 1].has_max = 1;
    sf.fields[sf.size - 1].max_val = 10.0;
    sf.fields[sf.size - 1].exclusive_max = 1;

    /* Number with only exclusive min (bizarre but technically possible) */
    struct_fields_add(&sf, "num_ex_min_only", "number", NULL, NULL, NULL);
    sf.fields[sf.size - 1].has_min = 0;
    sf.fields[sf.size - 1].has_max = 0;
    sf.fields[sf.size - 1].exclusive_min = 1;

    /* Number with only exclusive max */
    struct_fields_add(&sf, "num_ex_max_only", "number", NULL, NULL, NULL);
    sf.fields[sf.size - 1].has_min = 0;
    sf.fields[sf.size - 1].has_max = 0;
    sf.fields[sf.size - 1].exclusive_min = 0;
    sf.fields[sf.size - 1].exclusive_max = 1;

    /* Integer with only max */
    struct_fields_add(&sf, "int_max_only", "integer", NULL, NULL, NULL);
    sf.fields[sf.size - 1].has_min = 0;
    sf.fields[sf.size - 1].has_max = 1;
    sf.fields[sf.size - 1].max_val = 10.0;
    sf.fields[sf.size - 1].exclusive_max = 0;

    /* Integer with only exclusive min */
    struct_fields_add(&sf, "int_ex_min_only", "integer", NULL, NULL, NULL);
    sf.fields[sf.size - 1].has_min = 0;
    sf.fields[sf.size - 1].has_max = 0;
    sf.fields[sf.size - 1].exclusive_min = 1;

    /* Integer with only exclusive max */
    struct_fields_add(&sf, "int_ex_max_only", "integer", NULL, NULL, NULL);
    sf.fields[sf.size - 1].has_min = 0;
    sf.fields[sf.size - 1].has_max = 0;
    sf.fields[sf.size - 1].exclusive_min = 0;
    sf.fields[sf.size - 1].exclusive_max = 1;

    config.guard_macro = "JSON_ENABLED";

    if (readonly_f) {
      g_fail_io_after = 0;
      g_io_calls = 0;
      g_fail_io_after = 0;
      g_io_calls = 0;
      ASSERT_EQ(CDD_C_ERROR_IO,
                write_struct_to_json_func(readonly_f, "S", &sf, NULL));
      g_fail_io_after = 0;
      g_io_calls = 0;
      g_fail_io_after = 0;
      g_io_calls = 0;
      ASSERT_EQ(CDD_C_ERROR_IO,
                write_struct_from_json_func(readonly_f, "S", &config));
      g_fail_io_after = 0;
      g_io_calls = 0;
      g_fail_io_after = 0;
      g_io_calls = 0;
      ASSERT_EQ(CDD_C_ERROR_IO,
                write_struct_array_from_json_func(readonly_f, "S", &config));
      g_fail_io_after = 0;
      g_io_calls = 0;
      g_fail_io_after = 0;
      g_io_calls = 0;
      ASSERT_EQ(CDD_C_ERROR_IO, write_struct_from_jsonObject_func(
                                    readonly_f, "S", &sf, &config));
      fclose(readonly_f);
    }
    struct_fields_free(&sf);
  }

  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_standalone_json_func
 * @return TEST
 */
TEST test_standalone_json_func(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content = NULL;
  long sz;

  ASSERT(tmp);
  setup_json_fields(&sf);
  printf("\nid flags: %d %d %d %d\n", sf.fields[0].has_min,
         sf.fields[0].has_max, sf.fields[0].exclusive_min,
         sf.fields[0].exclusive_max);

  ASSERT_EQ(0, write_struct_from_json_standalone_func(tmp, "Data", &sf));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "Data_parse_json(char *json"));
  ASSERT(strstr(content, "(strcmp(key, \"id\") == 0)"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief codegen_json_suite
 */
#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
#endif

TEST test_json_exhaustive_io(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  int rc;
  struct StructFields sf;
  struct CodegenJsonConfig config;

  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, "0", NULL);
  struct_fields_add(&sf, "data", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "arr", "array", "string", NULL, NULL);
  struct_fields_add(&sf, "obj", "object", "Obj", NULL, NULL);
  struct_fields_add(&sf, "enm", "enum", "Enum", NULL, NULL);
  struct_fields_add(&sf, "num", "number", NULL, NULL, NULL);
  struct_fields_add(&sf, "b", "boolean", NULL, NULL, NULL);
  struct_fields_add(&sf, "arr_int", "array", "integer", NULL, NULL);
  struct_fields_add(&sf, "arr_obj", "array", "Obj", NULL, NULL);

  /* Optional fields */
  struct_fields_add(&sf, "opt_str", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].required = 0;

  /* Min/Max lengths */
  struct_fields_add(&sf, "len_str", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min_len = 1;
  sf.fields[sf.size - 1].min_len = 5;
  sf.fields[sf.size - 1].has_max_len = 1;
  sf.fields[sf.size - 1].max_len = 10;

  /* Min/Max items */
  struct_fields_add(&sf, "len_arr", "array", "string", NULL, NULL);
  sf.fields[sf.size - 1].has_min_items = 1;
  sf.fields[sf.size - 1].min_items = 1;
  sf.fields[sf.size - 1].has_max_items = 1;
  sf.fields[sf.size - 1].max_items = 3;

  /* Write-only and Read-only */
  struct_fields_add(&sf, "w_only", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].write_only = 1;
  struct_fields_add(&sf, "r_only", "string", NULL, NULL, NULL);
  sf.fields[sf.size - 1].read_only = 1;

  /* Number with min/max */
  struct_fields_add(&sf, "num_bounded", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 100.0;
  sf.fields[sf.size - 1].exclusive_min = 1;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Integer with min/max exclusive */
  struct_fields_add(&sf, "int_bounded", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 10.0;
  sf.fields[sf.size - 1].exclusive_min = 1;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Number with inclusive min/max */
  struct_fields_add(&sf, "num_bounded_inc", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 100.0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 0;

  /* String with regex patterns */
  struct_fields_add(&sf, "pat_exact", "string", NULL, NULL, NULL);
  strcpy(sf.fields[sf.size - 1].pattern, "^exact$");
  struct_fields_add(&sf, "pat_prefix", "string", NULL, NULL, NULL);
  strcpy(sf.fields[sf.size - 1].pattern, "^prefix");
  struct_fields_add(&sf, "pat_suffix", "string", NULL, NULL, NULL);
  strcpy(sf.fields[sf.size - 1].pattern, "suffix$");
  struct_fields_add(&sf, "pat_contains", "string", NULL, NULL, NULL);
  strcpy(sf.fields[sf.size - 1].pattern, "contains");

  /* Integer with inclusive min/max */
  struct_fields_add(&sf, "int_bounded_inc", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 1;
  sf.fields[sf.size - 1].min_val = 0.0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 100.0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 0;

  /* Number with only max */
  struct_fields_add(&sf, "num_max_only", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 10.0;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Number with only exclusive min (bizarre but technically possible) */
  struct_fields_add(&sf, "num_ex_min_only", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 1;

  /* Number with only exclusive max */
  struct_fields_add(&sf, "num_ex_max_only", "number", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 1;

  /* Integer with only max */
  struct_fields_add(&sf, "int_max_only", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 1;
  sf.fields[sf.size - 1].max_val = 10.0;
  sf.fields[sf.size - 1].exclusive_max = 0;

  /* Integer with only exclusive min */
  struct_fields_add(&sf, "int_ex_min_only", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 1;

  /* Integer with only exclusive max */
  struct_fields_add(&sf, "int_ex_max_only", "integer", NULL, NULL, NULL);
  sf.fields[sf.size - 1].has_min = 0;
  sf.fields[sf.size - 1].has_max = 0;
  sf.fields[sf.size - 1].exclusive_min = 0;
  sf.fields[sf.size - 1].exclusive_max = 1;

  config.guard_macro = "JSON_ENABLED";

  for (i = 0; i < 500; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_to_json_func(tmp, "Data", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 500; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_from_json_func(tmp, "Data", &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 500; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_array_from_json_func(tmp, "Data", &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  for (i = 0; i < 500; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_from_jsonObject_func(tmp, "Data", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  config.guard_macro = NULL;
  for (i = 0; i < 500; ++i) {
    FILE *tmp = tmpfile();
    g_fail_io_after = i;
    g_io_calls = 0;
    rc = write_struct_to_json_func(tmp, "Data", &sf, &config);
    fclose(tmp);
    if (rc == 0)
      break;
    g_fail_io_after = 0;
    g_io_calls = 0;
    g_fail_io_after = 0;
    g_io_calls = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  g_fail_io_after = -1;
  struct_fields_free(&sf);
#endif
  g_fail_io_after = -1;
  PASS();
}

TEST test_codegen_json_extra(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  struct CodegenJsonConfig config;

  ASSERT(tmp);
  struct_fields_init(&sf);

  /* Add fields for missing coverage:
     - exclusive_max, exclusive_min
     - max_len without min_len
     - min_items, max_items
   */
  {
    struct StructField *f;
    ASSERT_EQ(0, struct_fields_add(&sf, "v1", "number", NULL, NULL, NULL));
    f = &sf.fields[sf.size - 1];
    f->has_min = 1;
    f->min_val = 0;
    f->exclusive_min = 1;
    f->has_max = 1;
    f->max_val = 10;
    f->exclusive_max = 1;
  }
  {
    struct StructField *f;
    ASSERT_EQ(0, struct_fields_add(&sf, "v2", "string", NULL, NULL, NULL));
    f = &sf.fields[sf.size - 1];
    f->has_max_len = 1;
    f->max_len = 5;
  }
  {
    struct StructField *f;
    ASSERT_EQ(0, struct_fields_add(&sf, "v3", "array", "string", NULL, NULL));
    f = &sf.fields[sf.size - 1];
    f->has_min_items = 1;
    f->min_items = 1;
    f->has_max_items = 1;
    f->max_items = 5;
  }
  {
    struct StructField *f;
    ASSERT_EQ(0, struct_fields_add(&sf, "v4", "string", NULL, NULL, NULL));
    f = &sf.fields[sf.size - 1];
    f->has_min_len = 1;
    f->min_len = 1;
  }
  {
    struct StructField *f;
    ASSERT_EQ(0, struct_fields_add(&sf, "v5", "array", "string", NULL, NULL));
    f = &sf.fields[sf.size - 1];
    f->has_min_items = 0;
    f->has_max_items = 1;
    f->max_items = 10;
  }

  memset(&config, 0, sizeof(config));

  /* config == NULL tests */
  ASSERT_EQ(0, write_struct_from_json_func(tmp, "Extra", NULL));
  ASSERT_EQ(0, write_struct_to_json_func(tmp, "Extra", &sf, NULL));
  ASSERT_EQ(0, write_struct_array_from_json_func(tmp, "Extra", NULL));
  ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "Extra", &sf, NULL));

  ASSERT_EQ(0, write_struct_from_json_func(tmp, "Extra", &config));
  ASSERT_EQ(0, write_struct_to_json_func(tmp, "Extra", &sf, &config));
  ASSERT_EQ(0, write_struct_array_from_json_func(tmp, "Extra", &config));
  ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "Extra", &sf, &config));

  struct_fields_free(&sf);
  fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

SUITE(codegen_json_suite) {
  RUN_TEST(test_json_to_plain);
  RUN_TEST(test_json_from_plain);
  RUN_TEST(test_json_recursive_obj);
  RUN_TEST(test_json_array_logic);
  RUN_TEST(test_json_guards);
  RUN_TEST(test_struct_array_from_json);
  RUN_TEST(test_json_null_args);
  RUN_TEST(test_standalone_json_func);
  RUN_TEST(test_json_exhaustive_io);
  RUN_TEST(test_codegen_json_extra);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_JSON_H */
