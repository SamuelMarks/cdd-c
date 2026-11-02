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
    ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"\\\"data\\\": "
                           "\\\"%s\\\"\", obj->data)"));
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

    /* Check parson usage */
    ASSERT(strstr(content,
                  "ret->id = (int)json_object_get_number(jsonObject, \"id\")"));
    ASSERT(strstr(content, "json_object_get_string(jsonObject, \"data\")"));
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
    ASSERT(strstr(content, "json_object_get_array(jsonObject, \"tags\")"));
    ASSERT(strstr(content, "json_array_get_count(arr)"));
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
    ASSERT(strstr(content, "json_parse_string(json_str)"));
    ASSERT(strstr(content, "json_value_get_array(val)"));
    ASSERT(
        strstr(content,
               "Data_from_jsonObject(json_array_get_object(arr, i), &tmp[i])"));

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

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "Data_parse_json(char *json"));
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
  ASSERT(strstr(content, "\"\\\"userName\\\": \\\"%s\\\"\", obj->user_name"));
  ASSERT(strstr(content, "if (obj->auth_token != NULL)"));
  ASSERT(
      strstr(content, "\"\\\"AUTH-TOKEN\\\": \\\"%s\\\"\", obj->auth_token"));
  ASSERT(strstr(content, "obj->internal_ptr") == NULL);
  ASSERT(strstr(content, "obj->secret_pin") == NULL);
  ASSERT(strstr(content, "\"\\\"tag-val\\\": \\\"%s\\\"\", obj->computed_tag"));
  /* Flattened MetaInfo in to_json */
  ASSERT(strstr(content, "rc = MetaInfo_to_json(obj->meta, &s);"));
  ASSERT(strstr(content, "c89stringutils_jasprintf(json, \"%s\", s + 1);"));
  /* Custom with codec */
  ASSERT(strstr(content, "rc = ts_codec_to_json(obj->custom_ts, &s);"));

  /* Verify from_jsonObject output */
  ASSERT(strstr(content, "json_object_get_count(jsonObject)"));
  ASSERT(strstr(content, "strcmp(_jn, \"userName\") != 0"));
  ASSERT(strstr(content, "strcmp(_jn, \"username\") != 0"));
  ASSERT(
      strstr(content,
             "if (!s) s = json_object_get_string(jsonObject, \"username\");"));
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

  root_val = json_parse_file(schema_file);
  ASSERT(root_val != NULL);
  root_obj = json_value_get_object(root_val);
  comps_obj = json_object_get_object(root_obj, "components");
  schemas_obj = json_object_get_object(comps_obj, "schemas");
  payload_schema = json_object_get_object(schemas_obj, "Payload");
  ASSERT(payload_schema != NULL);
  ASSERT_STR_EQ("kebab-case",
                json_object_get_string(payload_schema, "x-cdd-rename-all"));
  ASSERT_EQ(
      1, json_object_get_boolean(payload_schema, "x-cdd-deny-unknown-fields"));

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
  ASSERT_STR_EQ("a\\\"b\\\\c\\n\\r\\t\\b\\f\\x01", esc_out);
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
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_JSON_H */
