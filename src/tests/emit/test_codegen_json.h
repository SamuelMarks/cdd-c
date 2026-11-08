#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
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

#include "classes/emit/enum.h"
#include "classes/emit/form.h"
#include "classes/emit/json.h"
#include "classes/emit/struct.h" /* For struct_fields_init etc */
#include "classes/parse/cdd_serde.h"
#include "classes/parse/code2schema.h"
#include "classes/parse/inspector.h"
#include "c_cdd/memory.h"
#include "functions/emit/codegen.h"
#include "functions/parse/fs.h"
/* clang-format on */

/* Common Setup */
/**
 * @brief Helper function setup_json_fields.
 * @param[in,out] sf Parameter sf.
 */
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
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    long sz;
    char *content = NULL;

    ASSERT(tmp != NULL);
    setup_json_fields(&sf);
    printf("\nid flags: %d %d %d %d\n", sf.fields[0].has_min,
           sf.fields[0].has_max, sf.fields[0].exclusive_min,
           sf.fields[0].exclusive_max);

    ASSERT_EQ(0, write_struct_to_json_func(tmp, "Data", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    /* Check structure */
    ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"{\");"));
    ASSERT(
        strstr(content,
               "c89stringutils_jasprintf(json, \"\\\"id\\\": %d\", obj->id)"));
    ASSERT(strstr(content, "_cdd_json_escape_str(obj->data, &_esc)"));
    ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"\\\"data\\\": "
                           "\\\"%s\\\"\", _esc)"));
    ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"}\");"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_json_from_plain
 * @return TEST
 */
TEST test_json_from_plain(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    long sz;
    char *content = NULL;

    ASSERT(tmp != NULL);
    setup_json_fields(&sf);
    printf("\nid flags: %d %d %d %d\n", sf.fields[0].has_min,
           sf.fields[0].has_max, sf.fields[0].exclusive_min,
           sf.fields[0].exclusive_max);

    ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "Data", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    /* Check cdd_json usage */
    ASSERT(strstr(content, "cdd_json_object_get_number(jsonObject, \"id\", "
                           "&_num); ret->id = (int)_num;"));
    ASSERT(strstr(content,
                  "cdd_json_object_get_string(jsonObject, \"data\", &s)"));
    ASSERT(strstr(content, "strdup(s)"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_json_recursive_obj
 * @return TEST
 */
TEST test_json_recursive_obj(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    char *content = NULL;
    long sz;

    ASSERT(tmp != NULL);
    struct_fields_init(&sf);
    struct_fields_add(&sf, "child", "object", "ChildType", NULL, NULL);

    ASSERT_EQ(0, write_struct_to_json_func(tmp, "Parent", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    /* Check recursive call pattern */
    ASSERT(strstr(content, "rc = ChildType_to_json(obj->child, &s);"));
    ASSERT(strstr(content,
                  "c89stringutils_jasprintf(json, \"\\\"child\\\": %s\", s);"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_json_array_logic
 * @return TEST
 */
TEST test_json_array_logic(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    char *content = NULL;
    long sz;

    ASSERT(tmp != NULL);
    struct_fields_init(&sf);
    /* Array of strings */
    struct_fields_add(&sf, "tags", "array", "string", NULL, NULL);

    ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "Post", &sf, NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    /* Check array loop extraction */
    ASSERT(strstr(content,
                  "cdd_json_object_get_array(jsonObject, \"tags\", &arr)"));
    ASSERT(strstr(content, "cdd_json_array_get_count(arr, &ret->n_tags)"));
    ASSERT(strstr(content, "calloc(ret->n_tags, sizeof(char *))"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_json_guards
 * @return TEST
 */
TEST test_json_guards(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    struct CodegenJsonConfig config;
    char *content = NULL;
    long sz;

    ASSERT(tmp != NULL);
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
#if defined(_MSC_VER)
    strcpy_s(sf.fields[sf.size - 1].pattern,
             sizeof(sf.fields[sf.size - 1].pattern), "^exact$");
#else
    strcpy(sf.fields[sf.size - 1].pattern, "^exact$");
#endif
    struct_fields_add(&sf, "pat_prefix", "string", NULL, NULL, NULL);
#if defined(_MSC_VER)
    strcpy_s(sf.fields[sf.size - 1].pattern,
             sizeof(sf.fields[sf.size - 1].pattern), "^prefix");
#else
    strcpy(sf.fields[sf.size - 1].pattern, "^prefix");
#endif
    struct_fields_add(&sf, "pat_suffix", "string", NULL, NULL, NULL);
#if defined(_MSC_VER)
    strcpy_s(sf.fields[sf.size - 1].pattern,
             sizeof(sf.fields[sf.size - 1].pattern), "suffix$");
#else
    strcpy(sf.fields[sf.size - 1].pattern, "suffix$");
#endif
    struct_fields_add(&sf, "pat_contains", "string", NULL, NULL, NULL);
#if defined(_MSC_VER)
    strcpy_s(sf.fields[sf.size - 1].pattern,
             sizeof(sf.fields[sf.size - 1].pattern), "contains");
#else
    strcpy(sf.fields[sf.size - 1].pattern, "contains");
#endif

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

    config.guard_macro = cdd_test_mut_str("JSON_ENABLED");

    ASSERT_EQ(0, write_struct_from_json_func(tmp, "Data", &config));
    ASSERT_EQ(0, write_struct_array_from_json_func(tmp, "Data", &config));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "#ifdef JSON_ENABLED"));
    ASSERT(strstr(content, "#endif /* JSON_ENABLED */"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_struct_array_from_json
 * @return TEST
 */
TEST test_struct_array_from_json(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    char *content = NULL;
    long sz;

    ASSERT(tmp != NULL);
    ASSERT_EQ(0, write_struct_array_from_json_func(tmp, "Data", NULL));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content,
                  "cdd_c_error_t Data_array_from_json(const char *json_str, "
                  "struct Data ***out, size_t *out_len)"));
    ASSERT(strstr(content, "cdd_json_parse_string(json_str, &val)"));
    ASSERT(strstr(content, "cdd_json_value_get_array(val, &arr)"));
    ASSERT(strstr(content, "cdd_json_array_get_object(arr, i, &_elem_obj)"));
    ASSERT(strstr(content, "Data_from_jsonObject(_elem_obj, &tmp[i])"));

    free(content);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief test_json_null_args
 * @return TEST
 */
TEST test_json_null_args(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
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
    FILE *readonly_f;
#if defined(_MSC_VER)
    if (((readonly_f = cdd_test_tmpfile_global()) == NULL))
      readonly_f = NULL;
#else
    readonly_f = cdd_test_tmpfile_global();
#endif
    {
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
#if defined(_MSC_VER)
      strcpy_s(sf.fields[sf.size - 1].pattern,
               sizeof(sf.fields[sf.size - 1].pattern), "^exact$");
#else
      strcpy(sf.fields[sf.size - 1].pattern, "^exact$");
#endif
      struct_fields_add(&sf, "pat_prefix", "string", NULL, NULL, NULL);
#if defined(_MSC_VER)
      strcpy_s(sf.fields[sf.size - 1].pattern,
               sizeof(sf.fields[sf.size - 1].pattern), "^prefix");
#else
      strcpy(sf.fields[sf.size - 1].pattern, "^prefix");
#endif
      struct_fields_add(&sf, "pat_suffix", "string", NULL, NULL, NULL);
#if defined(_MSC_VER)
      strcpy_s(sf.fields[sf.size - 1].pattern,
               sizeof(sf.fields[sf.size - 1].pattern), "suffix$");
#else
      strcpy(sf.fields[sf.size - 1].pattern, "suffix$");
#endif
      struct_fields_add(&sf, "pat_contains", "string", NULL, NULL, NULL);
#if defined(_MSC_VER)
      strcpy_s(sf.fields[sf.size - 1].pattern,
               sizeof(sf.fields[sf.size - 1].pattern), "contains");
#else
      strcpy(sf.fields[sf.size - 1].pattern, "contains");
#endif

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

      config.guard_macro = cdd_test_mut_str("JSON_ENABLED");

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
  }

  if (tmp)
    fclose(tmp);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_standalone_json_func
 * @return TEST
 */
TEST test_standalone_json_func(void) {
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    struct StructFields sf;
    char *content = NULL;
    long sz;

    ASSERT(tmp != NULL);
    setup_json_fields(&sf);
    printf("\nid flags: %d %d %d %d\n", sf.fields[0].has_min,
           sf.fields[0].has_max, sf.fields[0].exclusive_min,
           sf.fields[0].exclusive_max);

    ASSERT_EQ(0, write_struct_from_json_standalone_func(tmp, "Data", &sf));
    ASSERT_EQ(0, write_struct_to_json_standalone_func(tmp, "Data", &sf));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "Data_parse_json(const char *json"));
    ASSERT(strstr(content, "Data_to_json_standalone(const struct Data *obj"));
    ASSERT(strstr(content, "(strcmp(key, \"id\") == 0)"));

    free(content);
    struct_fields_free(&sf);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Tests case conversion rules, string rewriting, and error handling in
 * cdd_serde.
 * @return TEST
 */
TEST test_serde_case_and_replace_engine(void) {
  enum CddSerdeCaseRule rule = CDD_SERDE_CASE_NONE;
  const char *rule_str = NULL;
  char out[128];

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_parse_case_rule(NULL, &rule));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_parse_case_rule("camelCase", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_parse_case_rule("unknown_rule", &rule));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_case_rule_to_str(CDD_SERDE_CASE_NONE, NULL));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_serde_transform_case(NULL, CDD_SERDE_CASE_NONE, out, sizeof(out)));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_transform_case("abc", CDD_SERDE_CASE_NONE, NULL, 10));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_replace_str(NULL, "_", "-", out, sizeof(out)));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("lowercase", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_LOWERCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_case_rule_to_str(rule, &rule_str));
  ASSERT_STR_EQ("lowercase", rule_str);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_transform_case("User_ID", rule, out, sizeof(out)));
  ASSERT_STR_EQ("user_id", out);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("UPPERCASE", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_UPPERCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_case_rule_to_str(rule, &rule_str));
  ASSERT_STR_EQ("UPPERCASE", rule_str);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_transform_case("user_id", rule, out, sizeof(out)));
  ASSERT_STR_EQ("USER_ID", out);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("PascalCase", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_PASCALCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_case_rule_to_str(rule, &rule_str));
  ASSERT_STR_EQ("PascalCase", rule_str);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_transform_case("user_profile_id", rule,
                                                    out, sizeof(out)));
  ASSERT_STR_EQ("UserProfileId", out);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("camelCase", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_CAMELCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_case_rule_to_str(rule, &rule_str));
  ASSERT_STR_EQ("camelCase", rule_str);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_transform_case("user_profile_id", rule,
                                                    out, sizeof(out)));
  ASSERT_STR_EQ("userProfileId", out);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("snake_case", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_SNAKECASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_case_rule_to_str(rule, &rule_str));
  ASSERT_STR_EQ("snake_case", rule_str);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_transform_case("HTTPResponseCode", rule,
                                                    out, sizeof(out)));
  ASSERT_STR_EQ("http_response_code", out);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_case_rule("SCREAMING_SNAKE_CASE", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_SCREAMING_SNAKECASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_case_rule_to_str(rule, &rule_str));
  ASSERT_STR_EQ("SCREAMING_SNAKE_CASE", rule_str);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_transform_case("httpResponseCode", rule,
                                                    out, sizeof(out)));
  ASSERT_STR_EQ("HTTP_RESPONSE_CODE", out);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("kebab-case", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_KEBABCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_case_rule_to_str(rule, &rule_str));
  ASSERT_STR_EQ("kebab-case", rule_str);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_transform_case("user_first_name", rule,
                                                    out, sizeof(out)));
  ASSERT_STR_EQ("user-first-name", out);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_case_rule("SCREAMING-KEBAB-CASE", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_SCREAMING_KEBABCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_case_rule_to_str(rule, &rule_str));
  ASSERT_STR_EQ("SCREAMING-KEBAB-CASE", rule_str);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_transform_case("user_first_name", rule,
                                                    out, sizeof(out)));
  ASSERT_STR_EQ("USER-FIRST-NAME", out);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("Train-Case", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_TRAINCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_case_rule_to_str(rule, &rule_str));
  ASSERT_STR_EQ("Train-Case", rule_str);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_transform_case("content_type_header", rule,
                                                    out, sizeof(out)));
  ASSERT_STR_EQ("Content-Type-Header", out);

  /* Replace '_' with '-' and '-' with '_' */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_replace_str("my_field_name", "_", "-", out, sizeof(out)));
  ASSERT_STR_EQ("my-field-name", out);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_replace_str("my-field-name", "-", "_", out, sizeof(out)));
  ASSERT_STR_EQ("my_field_name", out);

  PASS();
}

/**
 * @brief Tests C89 reflection comment parsing and code generation for to_json,
 * from_jsonObject, standalone_json, enum, and form.
 * @return TEST
 */
TEST test_serde_comment_parsing_and_codegen(void) {
  FILE *tmp;
  struct StructFields sf;
  struct EnumMembers em;
  char *content = NULL;
  long sz;

  struct_fields_init(&sf);
  /* Container comment: camelCase + deny_unknown_fields */
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_serde_parse_container_comment(
          "/* @cdd rename_all = \"camelCase\", deny_unknown_fields */", &sf));
  ASSERT_EQ(CDD_SERDE_CASE_CAMELCASE, sf.serde_rename_all);
  ASSERT_EQ(1, sf.serde_deny_unknown_fields);

  /* 0: user_name -> camelCase "userName" with alias "username" */
  struct_fields_add(&sf, "user_name", "string", NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* @cdd alias = \"username\", alias = \"u_name\" */",
                &sf.fields[0]));

  /* 1: auth_token -> field override replace "_" -> "-" and UPPERCASE ->
   * "AUTH-TOKEN" */
  struct_fields_add(&sf, "auth_token", "string", NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_field_comment(
                               "/* @cdd uppercase, replace = (\"_\" -> \"-\"), "
                               "skip_serializing_if = \"null\" */",
                               &sf.fields[1]));

  /* 2: internal_ptr -> @cdd skip */
  struct_fields_add(&sf, "internal_ptr", "string", NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment("/* @cdd skip */", &sf.fields[2]));

  /* 3: secret_pin -> @cdd skip_serializing, default = "999" */
  struct_fields_add(&sf, "secret_pin", "integer", NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_field_comment(
                               "/* @cdd skip_serializing, default = \"999\" */",
                               &sf.fields[3]));

  /* 4: computed_tag -> @cdd skip_deserializing, default = "auto", rename =
   * "tag-val" */
  struct_fields_add(&sf, "computed_tag", "string", NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* @cdd skip_deserializing, default = \"auto\", rename = "
                "\"tag-val\" */",
                &sf.fields[4]));

  /* 5: meta -> @cdd flatten */
  struct_fields_add(&sf, "meta", "object", "MetaInfo", NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment("/* @cdd flatten */", &sf.fields[5]));

  /* 6: custom_ts -> @cdd with = "timestamp_codec" */
  struct_fields_add(&sf, "custom_ts", "integer", NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment("/* @cdd with = \"ts_codec\" */",
                                          &sf.fields[6]));

  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_to_json_func(tmp, "Account", &sf, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_from_jsonObject_func(tmp, "Account", &sf, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_from_json_standalone_func(tmp, "Account", &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_to_form_urlencoded_func(tmp, "Account", &sf));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  if (fread(content, 1, (size_t)sz, tmp)) {
  }

  /* Verify to_json output */
  ASSERT(strstr(content, "_cdd_json_escape_str(obj->user_name, &_esc)"));
  ASSERT(strstr(content, "\"\\\"userName\\\": \\\"%s\\\"\", _esc"));
  ASSERT(strstr(content, "if (obj->auth_token != NULL)"));
  ASSERT(strstr(content, "_cdd_json_escape_str(obj->auth_token, &_esc)"));
  ASSERT(strstr(content, "\"\\\"AUTH-TOKEN\\\": \\\"%s\\\"\", _esc"));
  ASSERT(strstr(content, "obj->internal_ptr") == NULL);
  ASSERT(strstr(content, "obj->secret_pin") == NULL);
  ASSERT(strstr(content, "_cdd_json_escape_str(obj->computed_tag, &_esc)"));
  ASSERT(strstr(content, "\"\\\"tag-val\\\": \\\"%s\\\"\", _esc"));
  /* Flattened MetaInfo in to_json */
  ASSERT(strstr(content, "rc = MetaInfo_to_json(obj->meta, &s);"));
  ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"%s\", s + 1);"));
  /* Custom with codec */
  ASSERT(strstr(content, "rc = ts_codec_to_json(obj->custom_ts, &s);"));

  /* Verify from_jsonObject output */
  ASSERT(strstr(content, "cdd_json_object_get_count(jsonObject, &_cnt)"));
  ASSERT(strstr(content, "strcmp(_jn, \"userName\") != 0"));
  ASSERT(strstr(content, "strcmp(_jn, \"username\") != 0"));
  ASSERT(strstr(
      content,
      "if (!s) cdd_json_object_get_string(jsonObject, \"username\", &s);"));
  ASSERT(strstr(content, "ret->computed_tag = strdup(\"auto\");"));
  /* Flattened MetaInfo in from_jsonObject passes parent jsonObject directly! */
  ASSERT(strstr(content,
                "rc = MetaInfo_from_jsonObject(jsonObject, &ret->meta);"));
  ASSERT(strstr(content, "rc = ts_codec_from_jsonObject(jsonObject, "
                         "\"customTs\", &ret->custom_ts);"));

  free(content);
  fclose(tmp);
  struct_fields_free(&sf);

  /* Test Enum reflection */
  enum_members_init(&em);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_enum_comment("/* @cdd kebabcase */", &em));
  enum_members_add_ex(&em, " IN_PROGRESS ", NULL, 0);
  enum_members_add_ex(&em, "COMPLETED_OK", "done_ok", 0);
  enum_members_add_ex(&em, "INTERNAL_ONLY", NULL, 1);

  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp != NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_enum_to_str_func(tmp, "Status", &em, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, write_enum_from_str_func(tmp, "Status", &em, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  if (fread(content, 1, (size_t)sz, tmp)) {
  }

  ASSERT(strstr(content, "\"in-progress\""));
  ASSERT(strstr(content, "\"done_ok\""));
  ASSERT(strstr(content, "INTERNAL_ONLY") == NULL);

  free(content);
  fclose(tmp);
  enum_members_free(&em);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests end-to-end C header -> inspector & code2schema -> schema2code
 * roundtrip with C89 reflection comments.
 * @return TEST
 */
TEST test_serde_inspector_and_schema_roundtrip(void) {
  char *hdr_file = cdd_test_mut_str("test_serde_reflect.h");
  char *schema_file = cdd_test_mut_str("test_serde_schema.json");
  const char *hdr_src =
      "/* @cdd rename_all = \"kebab-case\", deny_unknown_fields */\n"
      "struct Payload {\n"
      "  /* @cdd rename = \"payload_id\", alias = \"id\" */\n"
      "  int payload_id;\n"
      "  /* @cdd uppercase, replace = (\"_\" -> \"-\") */\n"
      "  char *api_key;\n"
      "  /* @cdd skip */\n"
      "  char *local_cache;\n"
      "  /* @cdd flatten */\n"
      "  struct ExtraMeta *extra;\n"
      "};\n";
  struct TypeDefList types;
  char *c2s_argv[2];
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Object *comps_obj = NULL;
  JSON_Object *schemas_obj = NULL;
  JSON_Object *payload_schema = NULL;
  struct StructFields roundtrip_sf;
  FILE *h_tmp = NULL;
  char *emitted_h = NULL;
  long h_sz;

  ASSERT_EQ(CDD_C_SUCCESS, write_to_file(hdr_file, hdr_src));

  /* 1. Verify c_inspector_scan_file_types parses container and field comments
   */
  ASSERT_EQ(CDD_C_SUCCESS, type_def_list_init(&types));
  ASSERT_EQ(CDD_C_SUCCESS, c_inspector_scan_file_types(hdr_file, &types));
  ASSERT_EQ(1, types.size);
  ASSERT_STR_EQ("Payload", types.items[0].name);
  ASSERT_EQ(CDD_SERDE_CASE_KEBABCASE,
            types.items[0].details.struct_fields->serde_rename_all);
  ASSERT_EQ(1, types.items[0].details.struct_fields->serde_deny_unknown_fields);
  ASSERT_EQ(4, types.items[0].details.struct_fields->size);
  ASSERT_STR_EQ("payload_id",
                types.items[0].details.struct_fields->fields[0].serde_rename);
  ASSERT_EQ(1, types.items[0].details.struct_fields->fields[0].n_serde_aliases);
  ASSERT_EQ(CDD_SERDE_CASE_UPPERCASE,
            types.items[0].details.struct_fields->fields[1].serde_case);
  ASSERT_STR_EQ(
      "_", types.items[0].details.struct_fields->fields[1].serde_replace_from);
  ASSERT_STR_EQ(
      "-", types.items[0].details.struct_fields->fields[1].serde_replace_to);
  ASSERT_EQ(1, types.items[0].details.struct_fields->fields[2].serde_skip);
  ASSERT_EQ(1, types.items[0].details.struct_fields->fields[3].serde_flatten);
  type_def_list_free(&types);

  /* 2. Run code2schema_main on hdr_file and verify schema extensions */
  c2s_argv[0] = hdr_file;
  c2s_argv[1] = schema_file;
  ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, c2s_argv));

  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_parse_file(schema_file, &_cdd_jv_1);
    root_val = _cdd_jv_1;
  }
  ASSERT(root_val != NULL);
  root_obj = json_value_get_object(root_val);
  {
    JSON_Object *_cdd_jo_1 = NULL;
    cdd_json_object_get_object(root_obj, "components", &_cdd_jo_1);
    comps_obj = _cdd_jo_1;
  }
  {
    JSON_Object *_cdd_jo_1 = NULL;
    cdd_json_object_get_object(comps_obj, "schemas", &_cdd_jo_1);
    schemas_obj = _cdd_jo_1;
  }
  {
    JSON_Object *_cdd_jo_1 = NULL;
    cdd_json_object_get_object(schemas_obj, "Payload", &_cdd_jo_1);
    payload_schema = _cdd_jo_1;
  }
  ASSERT(payload_schema != NULL);
  {
    const char *_cdd_js_1 = NULL;
    cdd_json_object_get_string(payload_schema, "x-cdd-rename-all", &_cdd_js_1);
    ASSERT_STR_EQ("kebab-case", _cdd_js_1);
  }
  {
    int _cdd_jb_1 = -1;
    if (cdd_json_object_get_boolean(payload_schema, "x-cdd-deny-unknown-fields",
                                    &_cdd_jb_1) != CDD_C_SUCCESS)
      _cdd_jb_1 = -1;
    ASSERT_EQ(1, _cdd_jb_1);
  }

  /* 3. Convert JSON Schema back to StructFields via code2schema_mapper */
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&roundtrip_sf));
  ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(
                               payload_schema, &roundtrip_sf, schemas_obj));
  ASSERT_EQ(CDD_SERDE_CASE_KEBABCASE, roundtrip_sf.serde_rename_all);
  ASSERT_EQ(1, roundtrip_sf.serde_deny_unknown_fields);
  ASSERT_STR_EQ("api_key", roundtrip_sf.fields[1].name);
  ASSERT_EQ(CDD_SERDE_CASE_UPPERCASE, roundtrip_sf.fields[1].serde_case);
  ASSERT_EQ(1, roundtrip_sf.fields[2].serde_skip);
  ASSERT_EQ(1, roundtrip_sf.fields[3].serde_flatten);

  /* 4. Emit C header declaration and verify @cdd comments */
  h_tmp = cdd_test_tmpfile_global();
  ASSERT(h_tmp != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_declaration_h(h_tmp, "Payload", &roundtrip_sf, NULL));
  fseek(h_tmp, 0, SEEK_END);
  h_sz = ftell(h_tmp);
  rewind(h_tmp);
  emitted_h = (char *)calloc(1, (size_t)h_sz + 1);
  if (fread(emitted_h, 1, (size_t)h_sz, h_tmp)) {
  }
  ASSERT(strstr(emitted_h,
                "/* @cdd rename_all = \"kebab-case\", deny_unknown_fields */"));
  ASSERT(strstr(emitted_h, "flatten"));

  free(emitted_h);
  fclose(h_tmp);
  struct_fields_free(&roundtrip_sf);
  json_value_free(root_val);
  remove(hdr_file);
  remove(schema_file);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests enum aliases, \@cdd other fallback, union
 * internal/adjacent/untagged sum-type tagging, and complete struct deepcopy
 * codegen.
 *
 * @return Greatest test result.
 */
TEST test_serde_sum_types_enum_other_and_deepcopy(void) {
  struct EnumMembers em;
  struct StructFields usf;
  struct StructFields dsf;
  FILE *tmp = NULL;
  long sz;
  char *content = NULL;
  char esc_out[128];

  /* 1. Escape helpers */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_escape_json_c_str(NULL, esc_out, sizeof(esc_out)));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_escape_comment_str(NULL, esc_out, sizeof(esc_out)));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_escape_json_c_str("a\"b\\c\n\r\t\b\f\x01",
                                                 esc_out, sizeof(esc_out)));
  ASSERT_STR_EQ("a\\\"b\\\\c\\n\\r\\t\\b\\f\\u0001", esc_out);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_escape_comment_str("a*/b\"c", esc_out, sizeof(esc_out)));
  ASSERT_STR_EQ("a*\\/b\\\"c", esc_out);

  /* 2. Enum aliases and @cdd other */
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_init(&em));
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_add_ex(&em, "ACTIVE", "active", 0));
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_add_alias(&em, 0, "enabled"));
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_add_alias(&em, 0, "on"));
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_add_ex(&em, "FALLBACK", "fallback", 0));
  em.serde_other_idx = 1;

  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp != NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_enum_from_str_func(tmp, "State", &em, NULL));
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  if (fread(content, 1, (size_t)sz, tmp)) {
  }
  ASSERT(strstr(content, "strcmp(str, \"active\") == 0 || strcmp(str, "
                         "\"enabled\") == 0 || strcmp(str, \"on\") == 0"));
  ASSERT(strstr(content, "else *val = State_FALLBACK;"));
  free(content);
  fclose(tmp);
  enum_members_free(&em);

  /* 3. Adjacent tagging union (tag = "t", content = "c") */
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&usf));
  usf.is_union = 1;
  CDD_STRCPY(usf.serde_tag, sizeof(usf.serde_tag), "t");
  CDD_STRCPY(usf.serde_content, sizeof(usf.serde_content), "c");
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&usf, "msg", "object", "MsgPayload", NULL, NULL));
  CDD_STRCPY(usf.fields[0].serde_rename, sizeof(usf.fields[0].serde_rename),
             "message_event");
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_field_add_alias(&usf.fields[0], "legacy_msg"));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&usf, "code", "integer", NULL, NULL, NULL));

  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp != NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_union_to_json_func(tmp, "Event", &usf, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_union_from_jsonObject_func(tmp, "Event", &usf, NULL));
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  if (fread(content, 1, (size_t)sz, tmp)) {
  }
  ASSERT(strstr(content, "{\\\"t\\\":\\\"message_event\\\",\\\"c\\\":"));
  ASSERT(strstr(content, "strcmp(disc, \"message_event\") == 0 || strcmp(disc, "
                         "\"legacy_msg\") == 0"));
  ASSERT(strstr(
      content,
      "MsgPayload_from_jsonObject(json_object_get_object(jsonObject, \"c\")"));
  free(content);
  fclose(tmp);

  /* 4. Untagged union */
  usf.serde_untagged = 1;
  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            write_union_from_jsonObject_func(tmp, "Event", &usf, NULL));
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  if (fread(content, 1, (size_t)sz, tmp)) {
  }
  ASSERT(strstr(content,
                "if (MsgPayload_from_jsonObject(jsonObject, &ret->data.msg) == "
                "CDD_C_SUCCESS)"));
  free(content);
  fclose(tmp);
  struct_fields_free(&usf);

  /* 5. Complete deepcopy generation */
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&dsf));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&dsf, "title", "string", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&dsf, "child", "object",
                                             "ChildNode", NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&dsf, "nums", "array", "integer", NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&dsf, "tags", "array", "string", NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&dsf, "nodes", "array", "ChildNode", NULL, NULL));

  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp != NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_struct_deepcopy_func(tmp, "Tree", &dsf, NULL));
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  if (fread(content, 1, (size_t)sz, tmp)) {
  }
  ASSERT(strstr(content, "(*dest)->child = NULL;"));
  ASSERT(strstr(content, "ChildNode_deepcopy(src->child, &(*dest)->child)"));
  ASSERT(strstr(content,
                "ChildNode_deepcopy(src->nodes[_i], &(*dest)->nodes[_i])"));
  free(content);
  fclose(tmp);
  struct_fields_free(&dsf);

  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Exhaustive tests for all functions and branches in cdd_serde.c.
 * @return greatest_test_res
 */
TEST test_cdd_serde_exhaustive(void) {
  enum CddSerdeCaseRule rule;
  enum CddCliAction act;
  enum CddCliValueHint hint;
  const char *s = NULL;
  char buf[128];
  struct StructFields sf;
  struct EnumMembers em;

  /* 1. cdd_serde_parse_case_rule & cdd_serde_case_rule_to_str */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_parse_case_rule(NULL, &rule));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_parse_case_rule("none", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("none", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_NONE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_NONE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("snakecase", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_SNAKECASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_case_rule("screaming_snakecase", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_SCREAMING_SNAKECASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("kebab_case", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_KEBABCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_case_rule("screaming_kebabcase", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_SCREAMING_KEBABCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_case_rule("screaming_kebab_case", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_SCREAMING_KEBABCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("traincase", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_TRAINCASE, rule);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_case_rule("train_case", &rule));
  ASSERT_EQ(CDD_SERDE_CASE_TRAINCASE, rule);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_parse_case_rule("unknown_rule", &rule));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_case_rule_to_str(CDD_SERDE_CASE_NONE, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_case_rule_to_str(CDD_SERDE_CASE_NONE, &s));
  ASSERT_STR_EQ("none", s);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_case_rule_to_str((enum CddSerdeCaseRule)999, &s));
  ASSERT_STR_EQ("none", s);

  /* 2. cdd_cli_parse_action & cdd_cli_action_to_str */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cli_parse_action(NULL, &act));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cli_parse_action("auto", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("auto", &act));
  ASSERT_EQ(CDD_CLI_ACTION_AUTO, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("", &act));
  ASSERT_EQ(CDD_CLI_ACTION_AUTO, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("set", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("set_true", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET_TRUE, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("settrue", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET_TRUE, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("true", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET_TRUE, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("set_false", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET_FALSE, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("setfalse", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET_FALSE, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("false", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET_FALSE, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("count", &act));
  ASSERT_EQ(CDD_CLI_ACTION_COUNT, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("append", &act));
  ASSERT_EQ(CDD_CLI_ACTION_APPEND, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("help", &act));
  ASSERT_EQ(CDD_CLI_ACTION_HELP, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("version", &act));
  ASSERT_EQ(CDD_CLI_ACTION_VERSION, act);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_parse_action("invalid_act", &act));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_action_to_str(CDD_CLI_ACTION_AUTO, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(CDD_CLI_ACTION_SET, &s));
  ASSERT_STR_EQ("set", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(CDD_CLI_ACTION_SET_TRUE, &s));
  ASSERT_STR_EQ("set_true", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(CDD_CLI_ACTION_SET_FALSE, &s));
  ASSERT_STR_EQ("set_false", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(CDD_CLI_ACTION_COUNT, &s));
  ASSERT_STR_EQ("count", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(CDD_CLI_ACTION_APPEND, &s));
  ASSERT_STR_EQ("append", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(CDD_CLI_ACTION_HELP, &s));
  ASSERT_STR_EQ("help", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(CDD_CLI_ACTION_VERSION, &s));
  ASSERT_STR_EQ("version", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(CDD_CLI_ACTION_AUTO, &s));
  ASSERT_STR_EQ("auto", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str((enum CddCliAction)999, &s));
  ASSERT_STR_EQ("auto", s);

  /* 3. cdd_cli_parse_value_hint & cdd_cli_value_hint_to_str */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_parse_value_hint(NULL, &hint));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_parse_value_hint("none", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("none", &hint));
  ASSERT_EQ(CDD_CLI_HINT_NONE, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("", &hint));
  ASSERT_EQ(CDD_CLI_HINT_NONE, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("file", &hint));
  ASSERT_EQ(CDD_CLI_HINT_FILE_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("file_path", &hint));
  ASSERT_EQ(CDD_CLI_HINT_FILE_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("filepath", &hint));
  ASSERT_EQ(CDD_CLI_HINT_FILE_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("dir", &hint));
  ASSERT_EQ(CDD_CLI_HINT_DIR_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("dir_path", &hint));
  ASSERT_EQ(CDD_CLI_HINT_DIR_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("dirpath", &hint));
  ASSERT_EQ(CDD_CLI_HINT_DIR_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("path", &hint));
  ASSERT_EQ(CDD_CLI_HINT_ANY_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("any_path", &hint));
  ASSERT_EQ(CDD_CLI_HINT_ANY_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("anypath", &hint));
  ASSERT_EQ(CDD_CLI_HINT_ANY_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("cmd", &hint));
  ASSERT_EQ(CDD_CLI_HINT_EXECUTABLE, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("executable", &hint));
  ASSERT_EQ(CDD_CLI_HINT_EXECUTABLE, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("command", &hint));
  ASSERT_EQ(CDD_CLI_HINT_EXECUTABLE, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("url", &hint));
  ASSERT_EQ(CDD_CLI_HINT_URL, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("hostname", &hint));
  ASSERT_EQ(CDD_CLI_HINT_HOSTNAME, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("host", &hint));
  ASSERT_EQ(CDD_CLI_HINT_HOSTNAME, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("username", &hint));
  ASSERT_EQ(CDD_CLI_HINT_USERNAME, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("user", &hint));
  ASSERT_EQ(CDD_CLI_HINT_USERNAME, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("email", &hint));
  ASSERT_EQ(CDD_CLI_HINT_EMAIL, hint);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_parse_value_hint("invalid_hint", &hint));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_value_hint_to_str(CDD_CLI_HINT_NONE, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cli_value_hint_to_str(CDD_CLI_HINT_FILE_PATH, &s));
  ASSERT_STR_EQ("file", s);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cli_value_hint_to_str(CDD_CLI_HINT_DIR_PATH, &s));
  ASSERT_STR_EQ("dir", s);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cli_value_hint_to_str(CDD_CLI_HINT_ANY_PATH, &s));
  ASSERT_STR_EQ("path", s);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cli_value_hint_to_str(CDD_CLI_HINT_EXECUTABLE, &s));
  ASSERT_STR_EQ("executable", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(CDD_CLI_HINT_URL, &s));
  ASSERT_STR_EQ("url", s);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cli_value_hint_to_str(CDD_CLI_HINT_HOSTNAME, &s));
  ASSERT_STR_EQ("hostname", s);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cli_value_hint_to_str(CDD_CLI_HINT_USERNAME, &s));
  ASSERT_STR_EQ("username", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(CDD_CLI_HINT_EMAIL, &s));
  ASSERT_STR_EQ("email", s);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(CDD_CLI_HINT_NONE, &s));
  ASSERT_STR_EQ("none", s);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cli_value_hint_to_str((enum CddCliValueHint)999, &s));
  ASSERT_STR_EQ("none", s);

  /* 4. cdd_serde_transform_case & cdd_serde_replace_str bounds */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_transform_case(NULL, CDD_SERDE_CASE_NONE, buf, 10));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_transform_case("a", CDD_SERDE_CASE_NONE, NULL, 10));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_transform_case("a", CDD_SERDE_CASE_NONE, buf, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_transform_case("abc", CDD_SERDE_CASE_NONE,
                                                    buf, sizeof(buf)));
  ASSERT_STR_EQ("abc", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_transform_case("___", CDD_SERDE_CASE_SNAKECASE, buf,
                                     sizeof(buf)));
  ASSERT_STR_EQ("___", buf);
  /* Small buffer truncation in transform_case */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_transform_case(
                               "a_b_c", CDD_SERDE_CASE_SNAKECASE, buf, 3));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_transform_case(
                               "a_b_c", CDD_SERDE_CASE_KEBABCASE, buf, 3));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_transform_case("abc", CDD_SERDE_CASE_LOWERCASE, buf, 2));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_transform_case("abc", CDD_SERDE_CASE_UPPERCASE, buf, 2));
  /* >32 words and >64 char word in serde_split_words */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_transform_case(
                "w1_w2_w3_w4_w5_w6_w7_w8_w9_w10_w11_w12_w13_w14_w15_w16_w17_"
                "w18_w19_w20_w21_w22_w23_w24_w25_w26_w27_w28_w29_w30_w31_w32_"
                "w33_w34_abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
                "abcdefghijklmnopqrstuvwxyz",
                CDD_SERDE_CASE_SNAKECASE, buf, sizeof(buf)));
  /* digit-to-upper boundary in serde_split_words */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_transform_case("v1Beta2", CDD_SERDE_CASE_SNAKECASE, buf,
                                     sizeof(buf)));
  ASSERT_STR_EQ("v1_beta2", buf);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_replace_str(NULL, "a", "b", buf, 10));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_replace_str("x", "a", "b", NULL, 10));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_replace_str("x", "a", "b", buf, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_replace_str("abc", NULL, "b", buf, sizeof(buf)));
  ASSERT_STR_EQ("abc", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_replace_str("abc", "", "b", buf, sizeof(buf)));
  ASSERT_STR_EQ("abc", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_replace_str("abc", "a", NULL, buf, sizeof(buf)));
  ASSERT_STR_EQ("abc", buf);
  /* Small buffer truncation in replace_str */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_replace_str("prefix_match_suffix", "match",
                                                 "longrep", buf, 10));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_replace_str("prefix_match_suffix", "match", "x", buf, 5));

  /* 5. cdd_serde_resolve_field_names & cdd_serde_resolve_enum_member_name */
  struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_resolve_field_names(NULL, 0, buf, sizeof(buf), NULL, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_resolve_field_names(&sf, 0, buf, sizeof(buf), NULL, 0));
  struct_fields_add(&sf, "my_field", "string", NULL, NULL, NULL);
  CDD_STRCPY(sf.serde_prefix, sizeof(sf.serde_prefix), "pre_");
  CDD_STRCPY(sf.serde_suffix, sizeof(sf.serde_suffix), "_suf");
  sf.serde_rename_all_ser = CDD_SERDE_CASE_UPPERCASE;
  sf.serde_rename_all_de = CDD_SERDE_CASE_LOWERCASE;
  CDD_STRCPY(sf.serde_replace_from, sizeof(sf.serde_replace_from), "_");
  CDD_STRCPY(sf.serde_replace_to, sizeof(sf.serde_replace_to), "-");
  {
    char s_buf[128], d_buf[128];
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_serde_resolve_field_names(&sf, 0, s_buf, sizeof(s_buf), d_buf,
                                            sizeof(d_buf)));
    ASSERT_STR_EQ("PRE-MY-FIELD-SUF", s_buf);
    ASSERT_STR_EQ("pre-my-field-suf", d_buf);
    /* serde_copy_str failure when ser_sz == 0 or de_sz == 0 */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_serde_resolve_field_names(&sf, 0, s_buf, 0, NULL, 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_serde_resolve_field_names(&sf, 0, NULL, 0, d_buf, 0));

    CDD_STRCPY(sf.fields[0].serde_rename_ser,
               sizeof(sf.fields[0].serde_rename_ser), "explicit_ser");
    CDD_STRCPY(sf.fields[0].serde_rename_de,
               sizeof(sf.fields[0].serde_rename_de), "explicit_de");
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_serde_resolve_field_names(&sf, 0, s_buf, sizeof(s_buf), d_buf,
                                            sizeof(d_buf)));
    ASSERT_STR_EQ("explicit_ser", s_buf);
    ASSERT_STR_EQ("explicit_de", d_buf);
  }
  struct_fields_free(&sf);

  enum_members_init(&em);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_resolve_enum_member_name(NULL, 0, buf, sizeof(buf)));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_resolve_enum_member_name(&em, 0, buf, sizeof(buf)));
  enum_members_add(&em, "MY_VAR");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_resolve_enum_member_name(&em, 0, NULL, 10));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_resolve_enum_member_name(&em, 0, buf, 0));
  CDD_STRCPY(em.serde_replace_from, sizeof(em.serde_replace_from), "_");
  CDD_STRCPY(em.serde_replace_to, sizeof(em.serde_replace_to), "-");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_resolve_enum_member_name(&em, 0, buf, sizeof(buf)));
  ASSERT_STR_EQ("MY-VAR", buf);
  em.members[0][0] = '\0';
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_resolve_enum_member_name(&em, 0, buf, sizeof(buf)));
  enum_members_free(&em);

  /* 6. Container, enum, field, and variant comment parsers */
  struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_parse_container_comment("/* @cdd default */", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_container_comment(NULL, &sf));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_container_comment("", &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment(
                "/*!< @brief \v\n"
                " * @version "
                "12345678901234567890123456789012345678901234567890123456789012"
                "34567890\n */",
                &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment(
                "/*!< @summary Sum text \t \n"
                " * @description Desc text \n"
                " * @version 1.2.3 \n"
                " * @infoVersion 2.0.0 \n"
                " * @author Jane \n"
                " * @contact jane@example.com \n"
                " * #[serde(rename = (\"R\\n\\r\\t\\\\x\"), replace_all = "
                "(\"_\" -> \"-\"), rename_all_serialize = \"camelCase\", "
                "rename_all_deserialize = \"snake_case\", prefix = \"p_\", "
                "suffix = \"_s\", tag = \"t\", content = \"c\", default, "
                "untagged)] */",
                &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment(
                "/* #[command(name = 'app\\'s', bin_name = \"bin\", version = "
                "\"0.2\", author = \"A\", about = \"Ab\", long_about = \"L1\", "
                "long_about = \"L2\", before_help = \"B1\", before_help = "
                "\"B2\", after_help = \"A1\", after_help = \"A2\", "
                "override_usage = \"U\", cli_rename_all = \"kebab-case\", "
                "rename_all = \"snake_case\", rename_all_env = \"UPPERCASE\", "
                "env_prefix = \"EP_\", alias = \"al1\", visible_alias = "
                "\"al2\")] */",
                &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment(
                "/* #[command(arg_required_else_help = false, "
                "subcommand_required = no, allow_external_subcommands = off, "
                "disable_help_flag = 0, disable_version_flag = true, "
                "disable_help_subcommand = 1, propagate_version = true, "
                "allow_windows_slash_flags = true, allow_response_files = "
                "true, case_insensitive = true)] */",
                &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment(
                "/* #[clap(version, author, about, long_about, "
                "arg_required_else_help, subcommand_required, "
                "allow_external_subcommands, disable_help_flag, "
                "disable_version_flag, disable_help_subcommand, "
                "propagate_version, allow_windows_slash_flags, "
                "allow_response_files, ignore_case, kebab-case, +)]\n"
                " * #[structopt(cli(name = \"so\"))]\n */",
                &sf));
  struct_fields_free(&sf);

  /* Plain docstring fallback, long_about when NULL, and ':' delimiter in
   * container comment */
  struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment(
                "/*!<\n * @cli(version, long_about: \"LA_init\", "
                "rename(val))\n * Plain brief line\r\n */",
                &sf));
  ASSERT_STR_EQ("Plain brief line", sf.cli_about);
  ASSERT_STR_EQ("0.1.0", sf.cli_version);
  struct_fields_free(&sf);

  /* Enum comment parser */
  enum_members_init(&em);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_parse_enum_comment("/* @cdd untagged */", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_enum_comment(NULL, &em));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_enum_comment("", &em));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_enum_comment(
                "/* @cdd cli(rename_all = \"kebab-case\", case_insensitive), "
                "replace_all = (\"A\" -> \"B\"), tag = \"t\", content = \"c\", "
                "untagged */",
                &em));
  ASSERT_EQ(CDD_SERDE_CASE_KEBABCASE, em.cli_rename_all);
  ASSERT_EQ(1, em.cli_case_insensitive);
  ASSERT_STR_EQ("A", em.serde_replace_from);
  ASSERT_STR_EQ("B", em.serde_replace_to);
  ASSERT_STR_EQ("t", em.serde_tag);
  ASSERT_STR_EQ("c", em.serde_content);
  ASSERT_EQ(1, em.serde_untagged);
  enum_members_free(&em);

  /* Field comment parser */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "__my_opt", "string", NULL, NULL, "existing_desc");
  struct_fields_add(&sf, "", "string", NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_serde_parse_field_comment("/* @cdd skip */", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_field_comment(NULL, &sf.fields[0]));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_field_comment("", &sf.fields[0]));
  /* First call without @brief or docstring so field->description[0] != '\0' &&
   * field->cli_help[0] == '\0' is hit */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* @details Field detail\n"
                " * #[arg(required_if_eq = (\"other\" -> \"val\"), "
                "rename_serialize = \"rs\", rename_deserialize = \"rd\", "
                "case = \"snake_case\", short = false, short = true, short = "
                "'x', short_alias = 'y', visible_short_alias = 'y', "
                "long = false, long = true, long = \"custom-long\")] */",
                &sf.fields[0]));
  /* Also test when field->description[0] != '\0' and field->cli_help[0] != '\0'
   * without dox_brief */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_field_comment(
                               "/* @details Second detail */", &sf.fields[0]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* #[arg(negatable = true, positional = false, index = 0, "
                "last = true, trailing_var_arg = true, value_name = \"VN\", "
                "help = \"H\", long_help = \"LH1\", long_help = \"LH2\", "
                "help_heading = \"HH\", env = true, env = false, env = "
                "\"MY_ENV\", hide_env = true, hide_env_values = true, "
                "default_missing_value = \"dmv\", hide_default_value = "
                "true)] */",
                &sf.fields[0]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* #[arg(action = \"count\", value_hint = \"file\", "
                "value_delimiter = ';', num_args = \"2\", num_args = \"..=3\", "
                "num_args = \"1..4\", num_args = \"1..\", num_args = \"..=\", "
                "require_equals = true, allow_hyphen_values = true, "
                "global = true, hide = true, exclusive = true, subcommand = "
                "true, flatten = true, required = true)] */",
                &sf.fields[0]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* #[arg(value_parser = \"vp\", group = \"grp\", "
                "conflicts_with = \"c1\", requires = \"r1\", "
                "required_unless = \"ru1\", skip_serializing_if = \"empty\", "
                "skip_serializing_if = \"is_zero\", skip_serializing_if = "
                "\"fn_sk\", default_fn = \"dfn\", serialize_with = \"sw\", "
                "deserialize_with = \"dw\", +)] */",
                &sf.fields[0]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* @cdd default, short, long, negatable, positional, last, "
                "trailing_var_arg, env, hide_env, hide_env_values, "
                "hide_default_value, value_delimiter, require_equals, "
                "allow_hyphen_values, global, hidden, exclusive, subcommand, "
                "required, count */",
                &sf.fields[0]));
  /* Test synonym keys, whitespace around paren-wrapped values, and invalid
   * action/hint/case_rule */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* @serde( rename = ( \"val\" ) , replace_all = ( \"a\" : "
                "\"b\" ) ,\n * required_if = ( \"f\" , \"v\" ) , serialize = "
                "\"s\" , deserialize = \"d\" , rename_all = \"bad\" , "
                "short = 0 , short = \"\" , short_alias = \"\" , long = 0 , "
                "long = \"\" , negate_with_no = 1 , env = \"\" , env = 0 ) */",
                &sf.fields[0]));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* @serde( action = \"bad\" , value_hint = \"bad\" , "
                "delimiter = \"\" , allow_negative_numbers = 1 , hidden = 1 , "
                "required_unless_present = \"u2\" , "
                "skip_serializing_if = \"nullptr\" , "
                "skip_serializing_if = \"is_empty\" , "
                "skip_serializing_if = \"zero\" , "
                "skip_serializing_if = \"\" , default_value = \"dv\" , "
                "default_value_t = \"dvt\" , unknown_key = \"uk\" ) */",
                &sf.fields[0]));
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_serde_parse_field_comment(
          "/* @cdd cli(default_value, default_value_t, negate_with_no, "
          "delimiter, allow_negative_numbers, hide, none, unknown_bare), "
          "clap(short_alias = 'a'), structopt(short_alias = 'b'), "
          "command(short_alias = 'c'), arg(short_alias = 'd'), "
          "rewrite = (\"x\" -> \"y\"), visible_alias: \"va\", "
          "cli, short\n * @cdd (\"\n * @cdd rename = (\n * @cdd rename = \n"
          " * @cdd replace = \n */",
          &sf.fields[0]));
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_serde_parse_field_comment(
          "/* @cdd to_cli(short_alias = 'a'), from_cli(short_alias = 'a'), "
          "to_json(rename = \"tj\"), from_json(rename = \"fj\") */",
          &sf.fields[0]));
  /* Test long = true when cli_long is already populated */
  CDD_STRCPY(sf.fields[0].cli_long, sizeof(sf.fields[0].cli_long), "already");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_field_comment(
                               "/* @cdd long = true */", &sf.fields[0]));
  /* Fill short_aliases buffer to test capacity check */
  {
    const char *als =
        "/* @cdd short_alias='d', short_alias='e', short_alias='f', "
        "short_alias='g', short_alias='h', short_alias='i', short_alias='j', "
        "short_alias='k', short_alias='l', short_alias='m', short_alias='n', "
        "short_alias='o', short_alias='p', short_alias='q' */";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_field_comment(als, &sf.fields[0]));
  }
  /* Test empty field name with env = true, env bare, long = true, short = true
   */
  sf.fields[1].description[0] = '\0';
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_serde_parse_field_comment(
          "/* @brief Brief text\n"
          " * @cdd cli(short = true, long = true, env = true, help = \"H2\", "
          "last = false, trailing_var_arg = false, num_args = \"0..0\")\n"
          " * @cdd env, short, long\n"
          " */",
          &sf.fields[1]));
  /* Test @brief when description and cli_help are already populated */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_field_comment(
                               "/* @brief Ignored brief */", &sf.fields[1]));
  struct_fields_free(&sf);

  /* Additional container comment branches (synonyms, invalid rules, unclosed
   * quotes/directives, plain docstrings) */
  struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment(
                "/** @briefFirstNoSpace\n"
                " * #[unknown_attr(foo)]\n"
                " * #[serde(rewrite_all = (\"a\" -> \"b\"), rewrite = (\"c\" "
                "-> \"d\"), replace = (\"e\" -> \"f\"), usage = \"u\", "
                "case = \"bad\", cli_rename_all = \"bad\", "
                "rename_all_env = \"bad\", ignore_case = 1, "
                "serialize = \"bad\", deserialize = \"bad\", "
                "unknown_kv = \"v\")]\n"
                " * @cli(none, unknown_bare)\n"
                " * @cdd(rename = \"unclosed\n"
                " */",
                &sf));
  /* Container comment when cli_about, cli_version, cli_author, cli_long_about
   * are already set */
  CDD_STRCPY(sf.cli_about, sizeof(sf.cli_about), "PreAbout");
  CDD_STRCPY(sf.cli_version, sizeof(sf.cli_version), "PreVer");
  CDD_STRCPY(sf.cli_author, sizeof(sf.cli_author), "PreAuth");
  sf.cli_long_about = strdup("PreLongAbout");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment(
                "/** @brief B\n * @details D\n * @version V\n * @author A\n"
                " * @cdd version */",
                &sf));
  /* Unclosed block comment and #[serde( at EOF */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_container_comment(
                               "/** #[serde(rename = \"a\\\")]", &sf));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_container_comment("/**   */", &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment("/** @cdd rename = \"a\\", &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment("/** @cdd ) */", &sf));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_container_comment(
                               "/** @cdd a = 1 @cdd b = 2 */", &sf));
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_serde_parse_container_comment(
          "/** @brief\n * #[serde(replace = (\"a\" -> \"b\")\n"
          " * @cdd cli, to_cli(name = \"tc\"), from_cli(version = \"1\"), "
          "to_json(camelCase), from_json(snake_case), command, clap, "
          "structopt, rename = \n"
          " * @cdd replace = \"a\" -> \"b\" */",
          &sf));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_container_comment("/**   ", &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment("/** @cdd x = 1", &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment("/** @cdd x = 1 */", &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment("/** @brief:Brief", &sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment("/** @brief\tBrief", &sf));
  struct_fields_free(&sf);

  /* Suffix-only and long prefix/suffix truncation in serde_resolve_single_name
   */
  struct_fields_init(&sf);
  struct_fields_add(&sf,
                    "very_long_field_name_012345678901234567890123456789012345",
                    "string", NULL, NULL, NULL);
  CDD_STRCPY(sf.serde_suffix, sizeof(sf.serde_suffix),
             "_very_long_suffix_01234567890123456789012345678901234567890123");
  {
    char s_buf[128];
    ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_resolve_field_names(
                                 &sf, 0, s_buf, sizeof(s_buf), NULL, 0));
    CDD_STRCPY(
        sf.serde_prefix, sizeof(sf.serde_prefix),
        "very_long_prefix_01234567890123456789012345678901234567890123_");
    ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_resolve_field_names(
                                 &sf, 0, s_buf, sizeof(s_buf), NULL, 0));
  }
  struct_fields_free(&sf);

  /* serde_split_words with '-', '.', '\t', and UPPER->UPPER non-boundary */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_transform_case("a-b.c\td_ABC", CDD_SERDE_CASE_SNAKECASE,
                                     buf, sizeof(buf)));

  /* Enum variant comment parser */
  enum_members_init(&em);
  enum_members_add(&em, "V1");
  {
    char ren[64], rf[64], rt[64];
    int sk = 0, oth = 0;
    enum CddSerdeCaseRule vcase = CDD_SERDE_CASE_NONE;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_serde_parse_enum_variant_comment(
                  NULL, NULL, 0, &sk, &vcase, rf, sizeof(rf), rt, sizeof(rt)));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_serde_parse_enum_variant_comment(
                  NULL, ren, 0, &sk, &vcase, rf, sizeof(rf), rt, sizeof(rt)));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_serde_parse_enum_variant_comment(NULL, ren, sizeof(ren), NULL,
                                                   &vcase, rf, sizeof(rf), rt,
                                                   sizeof(rt)));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_serde_parse_enum_variant_comment(NULL, ren, sizeof(ren), &sk,
                                                   NULL, rf, sizeof(rf), rt,
                                                   sizeof(rt)));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_serde_parse_enum_variant_comment(NULL, ren, sizeof(ren), &sk,
                                                   &vcase, NULL, sizeof(rf), rt,
                                                   sizeof(rt)));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_serde_parse_enum_variant_comment(
                  NULL, ren, sizeof(ren), &sk, &vcase, rf, 0, rt, sizeof(rt)));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_serde_parse_enum_variant_comment(NULL, ren, sizeof(ren), &sk,
                                                   &vcase, rf, sizeof(rf), NULL,
                                                   sizeof(rt)));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_serde_parse_enum_variant_comment(
                  NULL, ren, sizeof(ren), &sk, &vcase, rf, sizeof(rf), rt, 0));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_enum_variant_comment(
                                 NULL, ren, sizeof(ren), &sk, &vcase, rf,
                                 sizeof(rf), rt, sizeof(rt)));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_serde_parse_enum_variant_comment(
                                 "", ren, sizeof(ren), &sk, &vcase, rf,
                                 sizeof(rf), rt, sizeof(rt)));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_serde_parse_enum_variant_comment_ex(
                  "/* #[serde(other)] @cdd + other, rename = \"v_one\", skip, "
                  "replace = (\"a\" -> \"b\"), alias = \"v1_al\", help = "
                  "\"V help\", hidden, conflicts_with = \"c\", requires = "
                  "\"r\", required_unless = \"u\", long_help = \"lh\" */",
                  ren, sizeof(ren), &sk, &vcase, rf, sizeof(rf), rt, sizeof(rt),
                  &oth, &em, 0));
    ASSERT_STR_EQ("v_one", ren);
    ASSERT_EQ(1, sk);
    ASSERT_EQ(1, oth);
    ASSERT_STR_EQ("a", rf);
    ASSERT_STR_EQ("b", rt);

    /* Variant with hidden only (cli_help empty), skip_serializing only,
     * skip_deserializing only */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_serde_parse_enum_variant_comment_ex(
                  "/* @cdd hidden, skip_serializing */", ren, sizeof(ren), &sk,
                  &vcase, rf, sizeof(rf), rt, sizeof(rt), NULL, &em, 0));
    ASSERT_EQ(1, sk);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_serde_parse_enum_variant_comment_ex(
                  "/* @cdd skip_deserializing */", ren, sizeof(ren), &sk,
                  &vcase, rf, sizeof(rf), rt, sizeof(rt), NULL, NULL, 0));
    ASSERT_EQ(1, sk);
    {
      struct StructField empty_f;
      const char *long_doc =
          "/* @cddx @cdd_not_a_tag @from_form(rename = \"ff\") @to_form(rename "
          "= \"tf\") [[serde::rename = \"sr\"]] #[serde(rename = a[0], "
          "to_form(rename = \"tf2\"), from_form(rename = \"ff2\"), "
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
          " = 1, replace = "
          "(\"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
          "bbb\" -> "
          "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
          "))] @cli(case = snake_case, kebab-case) */";
      memset(&empty_f, 0, sizeof(empty_f));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_serde_parse_field_comment(long_doc, &empty_f));
    }
  }
  enum_members_free(&em);

  /* Escape helpers edge cases */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_escape_json_c_str("a", NULL, 10));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_escape_json_c_str("a", buf, 0));
  /* Control char when pos + 6 >= out_sz, and normal truncation */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_escape_json_c_str("\x01", buf, 6));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_escape_json_c_str("abcdef", buf, 3));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_escape_comment_str("a", NULL, 10));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_escape_comment_str("a", buf, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_escape_comment_str("a\\b*c\nd\re\t", buf, sizeof(buf)));
  ASSERT_STR_EQ("a\\\\b*c\\nd\\re\\t", buf);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_escape_comment_str("abcdef", buf, 3));

#ifdef CDD_BUILD_TESTS
  /* OOM tests for container, enum, field, and variant comment parsers */
  {
    int k;
    for (k = 1; k <= 12; ++k) {
      struct StructFields sf_oom;
      struct EnumMembers em_oom;
      char ren[64], rf[64], rt[64];
      int sk = 0, oth = 0;
      enum CddSerdeCaseRule vcase = CDD_SERDE_CASE_NONE;

      struct_fields_init(&sf_oom);
      struct_fields_add(&sf_oom, "f", "string", NULL, NULL, NULL);
      g_cdd_strdup_fail = k;
      cdd_serde_parse_container_comment(
          "/* @details D\n * @cdd long_about = \"L\", before_help = \"B\", "
          "after_help = \"A\", alias = \"al\" */",
          &sf_oom);
      g_cdd_strdup_fail = k;
      cdd_serde_parse_field_comment(
          "/* @cdd long_help = \"LH\", alias = \"a\", conflicts_with = \"c\", "
          "requires = \"r\", required_unless = \"u\" */",
          &sf_oom.fields[0]);
      g_cdd_strdup_fail = 0;
      struct_fields_free(&sf_oom);

      enum_members_init(&em_oom);
      enum_members_add(&em_oom, "V1");
      g_cdd_strdup_fail = k;
      cdd_serde_parse_enum_comment("/* @details D */", &em_oom);
      g_cdd_strdup_fail = k;
      cdd_serde_parse_enum_variant_comment_ex(
          "/* @details D\n * @cdd alias = \"a\", conflicts_with = \"c\", "
          "requires = \"r\", required_unless = \"u\", help = \"h\" */",
          ren, sizeof(ren), &sk, &vcase, rf, sizeof(rf), rt, sizeof(rt), &oth,
          &em_oom, 0);
      g_cdd_strdup_fail = 0;
      enum_members_free(&em_oom);
    }
    /* Test enum_member_set_cli_meta failure when member_idx is out of bounds */
    {
      struct EnumMembers em_oob;
      char ren[64], rf[64], rt[64];
      int sk = 0;
      enum CddSerdeCaseRule vcase = CDD_SERDE_CASE_NONE;
      enum_members_init(&em_oob);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_serde_parse_enum_variant_comment_ex(
                    "/* @cdd help = \"h\" */", ren, sizeof(ren), &sk, &vcase,
                    rf, sizeof(rf), rt, sizeof(rt), NULL, &em_oob, 99));
      enum_members_free(&em_oob);
    }
  }
#endif

  PASS();
}

/**
 * @brief Verifies that write_struct_to_json_func and cdd_json_escape_str
 * properly escape quotes, backslashes, and control characters in string fields
 * and string arrays, including OOM branches.
 * @return greatest_test_res
 */
TEST test_json_string_escaping_and_oom(void) {
  struct StructFields sf;
  FILE *tmp = NULL;
  char *content = NULL;
  long sz = 0;
  char *esc = NULL;

  /* 1. Direct cdd_json_escape_str unit tests */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_escape_str(NULL, &esc));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_escape_str("abc", NULL));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_escape_str(
                               "hello \"world\" \\ \b\f\n\r\t\x01\x1f!", &esc));
  ASSERT(esc != NULL);
  ASSERT_STR_EQ("hello \\\"world\\\" \\\\ \\b\\f\\n\\r\\t\\u0001\\u001f!", esc);
  C_CDD_FREE(esc);
  esc = NULL;

#ifdef CDD_BUILD_TESTS
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_json_escape_str("hello \"world\"", &esc));
  g_cdd_alloc_fail = 0;
#endif

  /* 2. Verify write_struct_to_json_func emits escaping for string fields
   * (pointer and fixed array) and string array elements */
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "msg", "string", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "fixed_msg", "string", NULL, NULL, NULL));
  sf.fields[1].fixed_array_len = 16;
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "lines", "array", "string", NULL, NULL));

  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_to_json_func(tmp, "EscModel", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  ASSERT(content != NULL);
  if (fread(content, 1, (size_t)sz, tmp)) {
  }

  ASSERT(strstr(content, "_cdd_json_escape_str(const char *s, char **out)"));
  ASSERT(strstr(content, "_cdd_json_escape_str(obj->msg, &_esc)"));
  ASSERT(strstr(content, "_cdd_json_escape_str(obj->fixed_msg, &_esc)"));
  ASSERT(strstr(content, "_cdd_json_escape_str(obj->lines[i], &_esc)"));
  ASSERT(strstr(content, "else if (c < 0x20)"));

  free(content);
  fclose(tmp);
  struct_fields_free(&sf);
  PASS();
}

/**
 * @brief Verifies 64-bit and 128-bit integer field codegen in
 * write_struct_to_json_func and write_struct_from_jsonObject_func.
 * @return greatest_test_res
 */
TEST test_json_int64_and_int128_codegen(void) {
  struct StructFields sf;
  FILE *tmp = NULL;
  char *content = NULL;
  long sz = 0;
  int k;

  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "id64", "int64_t", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "id64_alt", "int64", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "id64_fmt", "integer", NULL, NULL, NULL));
  CDD_STRCPY(sf.fields[sf.size - 1].format,
             sizeof(sf.fields[sf.size - 1].format), "int64");
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "u64", "uint64_t", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "u64_alt", "uint64", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "u64_fmt", "integer", NULL, NULL, NULL));
  CDD_STRCPY(sf.fields[sf.size - 1].format,
             sizeof(sf.fields[sf.size - 1].format), "uint64");
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "i128", "cdd_int128_t", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "i128_alt", "int128", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "u128", "cdd_uint128_t", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "u128_alt", "uint128", NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "fix_empty", "string", NULL, NULL, NULL));
  sf.fields[sf.size - 1].fixed_array_len = 8;
  sf.fields[sf.size - 1].serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "ro_bf_def", "integer", NULL, NULL, "4"));
  sf.fields[sf.size - 1].read_only = 1;
  CDD_STRCPY(sf.fields[sf.size - 1].serde_default_fn,
             sizeof(sf.fields[sf.size - 1].serde_default_fn), "def_bf_fn");
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "ro_fix_str", "string", NULL, "abc", NULL));
  sf.fields[sf.size - 1].read_only = 1;
  sf.fields[sf.size - 1].fixed_array_len = 8;
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "bf_de_with", "integer", NULL, NULL, "4"));
  CDD_STRCPY(sf.fields[sf.size - 1].serde_deserialize_with,
             sizeof(sf.fields[sf.size - 1].serde_deserialize_with),
             "custom_bf_de");
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "bf_int_def", "integer", NULL, NULL, "4"));
  CDD_STRCPY(sf.fields[sf.size - 1].serde_default_fn,
             sizeof(sf.fields[sf.size - 1].serde_default_fn), "def_bf_int");
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "bf_bool", "boolean", NULL, NULL, "1"));
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "bf_bool_def", "boolean", NULL, NULL, "1"));
  CDD_STRCPY(sf.fields[sf.size - 1].serde_default_fn,
             sizeof(sf.fields[sf.size - 1].serde_default_fn), "def_bf_bool");
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "str_def_fn", "string", NULL, NULL, NULL));
  CDD_STRCPY(sf.fields[sf.size - 1].serde_default_fn,
             sizeof(sf.fields[sf.size - 1].serde_default_fn), "def_str_fn");
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "bf_enum", "enum",
                                             "#/definitions/E", NULL, "2"));
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_bool_fix", "array",
                                             "boolean", NULL, NULL));
  sf.fields[sf.size - 1].fixed_array_len = 2;
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_num_fix", "array",
                                             "number", NULL, NULL));
  sf.fields[sf.size - 1].fixed_array_len = 2;
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_str_fix", "array",
                                             "string", NULL, NULL));
  sf.fields[sf.size - 1].fixed_array_len = 2;
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_sub_fix", "array",
                                             "#/definitions/Sub", NULL, NULL));
  sf.fields[sf.size - 1].fixed_array_len = 2;
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_bool_dyn", "array",
                                             "boolean", NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "arr_num_dyn", "array",
                                             "number", NULL, NULL));

  tmp = cdd_test_tmpfile_global();
  ASSERT(tmp != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_to_json_func(tmp, "BigNums", &sf, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_from_jsonObject_func(tmp, "BigNums", &sf, NULL));

  for (k = 0; k < 120; ++k) {
    cdd_c_error_t rc_io;
    g_fail_io_after = k;
    g_io_calls = 0;
    rc_io = write_struct_to_json_func(tmp, "BigNums", &sf, NULL);
    g_fail_io_after = -1;
    if (rc_io == CDD_C_SUCCESS)
      break;
  }
  for (k = 0; k < 120; ++k) {
    cdd_c_error_t rc_io;
    g_fail_io_after = k;
    g_io_calls = 0;
    rc_io = write_struct_from_jsonObject_func(tmp, "BigNums", &sf, NULL);
    g_fail_io_after = -1;
    if (rc_io == CDD_C_SUCCESS)
      break;
  }
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_str_after_last;
    g_cdd_fail_str_after_last = 2;
    ASSERT_NEQ(CDD_C_SUCCESS,
               write_struct_from_jsonObject_func(tmp, "BigNums", &sf, NULL));
    g_cdd_fail_str_after_last = 0;
  }
#endif

  rewind(tmp);
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_to_json_func(tmp, "BigNums", &sf, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_from_jsonObject_func(tmp, "BigNums", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, (size_t)sz + 1);
  ASSERT(content != NULL);
  if (fread(content, 1, (size_t)sz, tmp)) {
  }

  ASSERT(strstr(content, "cdd_json_value_init_int128(obj->i128, &_v128)"));
  ASSERT(strstr(content, "cdd_json_value_init_uint128(obj->u128, &_v128)"));
  ASSERT(strstr(content,
                "cdd_json_object_get_int64(jsonObject, \"id64\", &ret->id64)"));
  ASSERT(strstr(content,
                "cdd_json_object_get_uint64(jsonObject, \"u64\", &ret->u64)"));
  ASSERT(strstr(
      content, "cdd_json_object_get_int128(jsonObject, \"i128\", &ret->i128)"));
  ASSERT(
      strstr(content,
             "cdd_json_object_get_uint128(jsonObject, \"u128\", &ret->u128)"));

  free(content);
  fclose(tmp);
  struct_fields_free(&sf);
  PASS();
}

/**
 * @brief codegen_json_suite
 */

SUITE(codegen_json_suite) {
  RUN_TEST(test_json_to_plain);
  RUN_TEST(test_json_from_plain);
  RUN_TEST(test_json_recursive_obj);
  RUN_TEST(test_json_array_logic);
  RUN_TEST(test_json_guards);
  RUN_TEST(test_struct_array_from_json);
  RUN_TEST(test_json_null_args);
  RUN_TEST(test_standalone_json_func);
  RUN_TEST(test_serde_case_and_replace_engine);
  RUN_TEST(test_serde_comment_parsing_and_codegen);
  RUN_TEST(test_serde_inspector_and_schema_roundtrip);
  RUN_TEST(test_serde_sum_types_enum_other_and_deepcopy);
  RUN_TEST(test_cdd_serde_exhaustive);
  RUN_TEST(test_json_string_escaping_and_oom);
  RUN_TEST(test_json_int64_and_int128_codegen);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_JSON_H */
