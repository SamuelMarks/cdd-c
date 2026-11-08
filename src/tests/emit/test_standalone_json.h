/**
 * @file test_standalone_json.h
 * @brief Unit tests and mocks in test_standalone_json.h.
 */
#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
#ifndef TEST_STANDALONE_JSON_H
#define TEST_STANDALONE_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "c_cdd_export.h"
#include "classes/emit/json.h"
#include "classes/emit/struct.h"
#include <greatest.h>
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Unit test test_standalone_json_gen.
 * @return Greatest test result.
 */
TEST test_standalone_json_gen(void) {
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

    ASSERT(tmp);
    ASSERT_EQ(0, struct_fields_init(&sf));

    /* Invalid args */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_json_standalone_func(NULL, "MyStruct", &sf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_json_standalone_func(tmp, NULL, &sf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_from_json_standalone_func(tmp, "MyStruct", NULL));

    /* empty struct */
    ASSERT_EQ(0, write_struct_from_json_standalone_func(tmp, "MyStruct", &sf));

    /* with string and primitive */
    {
      struct StructField f1 = {0};
      struct StructField f2 = {0};
      struct StructField f3 = {0};
      struct StructField f4 = {0};
      struct StructField f5 = {0};

#if defined(_MSC_VER)
      strcpy_s(f1.name, sizeof(f1.name), "my_str");
      strcpy_s(f1.type, sizeof(f1.type), "string");
      strcpy_s(f1.default_val, sizeof(f1.default_val), "def_s");
#else
      strcpy(f1.name, "my_str");
      strcpy(f1.type, "string");
      strcpy(f1.default_val, "def_s");
#endif
      f1.required = 1;
      f1.has_min_len = 1;
      f1.min_len = 1;
      f1.has_max_len = 1;
      f1.max_len = 10;
      sf.fields[sf.size++] = f1;

      {
        struct StructField f1_2 = {0};
#if defined(_MSC_VER)
        strcpy_s(f1_2.name, sizeof(f1_2.name), "my_str2");
        strcpy_s(f1_2.type, sizeof(f1_2.type), "string");
#else
        strcpy(f1_2.name, "my_str2");
        strcpy(f1_2.type, "string");
#endif
        f1_2.required = 0;
        sf.fields[sf.size++] = f1_2;
      }

#if defined(_MSC_VER)
      strcpy_s(f2.name, sizeof(f2.name), "my_int");
      strcpy_s(f2.type, sizeof(f2.type), "integer");
      strcpy_s(f2.default_val, sizeof(f2.default_val), "42");
#else
      strcpy(f2.name, "my_int");
      strcpy(f2.type, "integer");
      strcpy(f2.default_val, "42");
#endif
      f2.required = 1;
      f2.has_min = 1;
      f2.min_val = 0.0;
      f2.has_max = 1;
      f2.max_val = 100.0;
      sf.fields[sf.size++] = f2;

#if defined(_MSC_VER)
      strcpy_s(f3.name, sizeof(f3.name), "my_bool");
      strcpy_s(f3.type, sizeof(f3.type), "boolean");
      strcpy_s(f3.default_val, sizeof(f3.default_val), "true");
#else
      strcpy(f3.name, "my_bool");
      strcpy(f3.type, "boolean");
      strcpy(f3.default_val, "true");
#endif
      f3.required = 1;
      sf.fields[sf.size++] = f3;

      {
        struct StructField f_opt_bool = {0};
#if defined(_MSC_VER)
        strcpy_s(f_opt_bool.name, sizeof(f_opt_bool.name), "opt_bool");
        strcpy_s(f_opt_bool.type, sizeof(f_opt_bool.type), "boolean");
        strcpy_s(f_opt_bool.default_val, sizeof(f_opt_bool.default_val),
                 "false");
#else
        strcpy(f_opt_bool.name, "opt_bool");
        strcpy(f_opt_bool.type, "boolean");
        strcpy(f_opt_bool.default_val, "false");
#endif
        f_opt_bool.required = 0;
        sf.fields[sf.size++] = f_opt_bool;
      }

#if defined(_MSC_VER)
      strcpy_s(f4.name, sizeof(f4.name), "my_num");
      strcpy_s(f4.type, sizeof(f4.type), "number");
      strcpy_s(f4.default_val, sizeof(f4.default_val), "3.14");
#else
      strcpy(f4.name, "my_num");
      strcpy(f4.type, "number");
      strcpy(f4.default_val, "3.14");
#endif
      f4.required = 1;
      f4.has_min = 1;
      f4.exclusive_min = 1;
      f4.min_val = 0.0;
      f4.has_max = 1;
      f4.exclusive_max = 1;
      f4.max_val = 99.0;
      sf.fields[sf.size++] = f4;

#if defined(_MSC_VER)
      strcpy_s(f5.name, sizeof(f5.name), "my_arr");
      strcpy_s(f5.type, sizeof(f5.type), "array");
#else
      strcpy(f5.name, "my_arr");
      strcpy(f5.type, "array");
#endif
      sf.fields[sf.size++] = f5;
    }

    ASSERT_EQ(0, write_struct_from_json_standalone_func(tmp, "MyStruct", &sf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_to_json_standalone_func(NULL, "MyStruct", &sf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_to_json_standalone_func(tmp, NULL, &sf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_to_json_standalone_func(tmp, "MyStruct", NULL));
    ASSERT_EQ(0, write_struct_to_json_standalone_func(tmp, "MyStruct", &sf));

    fseek(tmp, 0, SEEK_END);
    sz = ftell(tmp);
    rewind(tmp);
    content = (char *)calloc(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

    ASSERT(strstr(content, "MyStruct_parse_json"));
    ASSERT(strstr(content, "MyStruct_to_json_standalone"));
    ASSERT(strstr(content, "_cdd_json_buf_append_escaped"));
    ASSERT(strstr(content, "_cdd_json_parse_hex4"));
    ASSERT(strstr(content, "MyStruct_cleanup(ret)"));
    ASSERT(strstr(content, "my_str"));
    ASSERT(strstr(content, "my_int"));

    free(content);
    struct_fields_free(&sf);

    /* Test 1D and 2D arrays of all types */
    {
      struct StructFields sf_arr;
      int k;
      ASSERT_EQ(0, struct_fields_init(&sf_arr));
      ASSERT_EQ(0, struct_fields_add(&sf_arr, "a_int", "array", "integer", NULL,
                                     NULL));
      sf_arr.fields[sf_arr.size - 1].has_min_items = 1;
      sf_arr.fields[sf_arr.size - 1].min_items = 1;
      sf_arr.fields[sf_arr.size - 1].has_max_items = 1;
      sf_arr.fields[sf_arr.size - 1].max_items = 5;
      ASSERT_EQ(0, struct_fields_add(&sf_arr, "a_num", "array", "number", NULL,
                                     NULL));
      ASSERT_EQ(0, struct_fields_add(&sf_arr, "a_bool", "array", "boolean",
                                     NULL, NULL));
      ASSERT_EQ(0, struct_fields_add(&sf_arr, "a_str", "array", "string", NULL,
                                     NULL));
      ASSERT_EQ(0, struct_fields_add(&sf_arr, "a_enum", "array",
                                     "#/definitions/Role", NULL, NULL));
      sf_arr.fields[sf_arr.size - 1].is_enum_array = 1;
      ASSERT_EQ(0, struct_fields_add(&sf_arr, "a_obj", "array",
                                     "#/definitions/Child", NULL, NULL));

      ASSERT_EQ(0,
                write_struct_from_json_standalone_func(tmp, "ArrS", &sf_arr));
      ASSERT_EQ(0, write_struct_to_json_standalone_func(tmp, "ArrS", &sf_arr));
      for (k = 0; k < 30; ++k) {
        g_fail_io_after = k;
        g_io_calls = 0;
        write_struct_to_json_standalone_func(tmp, "ArrS", &sf_arr);
        g_fail_io_after = -1;
      }
      struct_fields_free(&sf_arr);
    }

    {
      struct StructFields sf_rich;
      int k;
      ASSERT_EQ(0, struct_fields_init(&sf_rich));
      ASSERT_EQ(0, struct_fields_add(&sf_rich, "role", "enum",
                                     "#/definitions/Role", NULL, NULL));
      ASSERT_EQ(0, struct_fields_add(&sf_rich, "child", "object",
                                     "#/definitions/Child", NULL, NULL));
      ASSERT_EQ(0, struct_fields_add(&sf_rich, "fixed_s", "string", NULL, NULL,
                                     NULL));
      sf_rich.fields[sf_rich.size - 1].fixed_array_len = 8;
      ASSERT_EQ(
          0, struct_fields_add(&sf_rich, "wo_s", "string", NULL, NULL, NULL));
      sf_rich.fields[sf_rich.size - 1].write_only = 1;
      ASSERT_EQ(
          0, struct_fields_add(&sf_rich, "ex_int", "integer", NULL, "0", NULL));
      sf_rich.fields[sf_rich.size - 1].has_min = 1;
      sf_rich.fields[sf_rich.size - 1].exclusive_min = 1;
      sf_rich.fields[sf_rich.size - 1].min_val = 1.0;
      sf_rich.fields[sf_rich.size - 1].has_max = 1;
      sf_rich.fields[sf_rich.size - 1].exclusive_max = 1;
      sf_rich.fields[sf_rich.size - 1].max_val = 10.0;
      ASSERT_EQ(
          0, struct_fields_add(&sf_rich, "b_one", "boolean", NULL, "1", NULL));
      ASSERT_EQ(0,
                write_struct_from_json_standalone_func(tmp, "RichS", &sf_rich));
      ASSERT_EQ(0,
                write_struct_to_json_standalone_func(tmp, "RichS", &sf_rich));
      for (k = 0; k < 25; ++k) {
        g_fail_io_after = k;
        g_io_calls = 0;
        write_struct_to_json_standalone_func(tmp, "RichS", &sf_rich);
        g_fail_io_after = -1;
      }
      struct_fields_free(&sf_rich);
    }

    {
      struct StructFields sf_bool;
      struct StructField fb = {0};
      ASSERT_EQ(0, struct_fields_init(&sf_bool));
#if defined(_MSC_VER)
      strcpy_s(fb.name, sizeof(fb.name), "only_bool");
      strcpy_s(fb.type, sizeof(fb.type), "boolean");
#else
      strcpy(fb.name, "only_bool");
      strcpy(fb.type, "boolean");
#endif
      fb.required = 1;
      sf_bool.fields[sf_bool.size++] = fb;
      ASSERT_EQ(
          0, write_struct_from_json_standalone_func(tmp, "BoolOnly", &sf_bool));
      struct_fields_free(&sf_bool);
    }

    {
      struct StructFields sf_edge;
      struct StructField f_ro = {0};
      struct StructField f_str_al = {0};
      struct StructField f_int_al = {0};
      char s_al_buf[] = "s_alias";
      char i_al_buf[] = "i_alias";
      char *str_aliases[2];
      char *int_aliases[2];

      ASSERT_EQ(0, struct_fields_init(&sf_edge));
#if defined(_MSC_VER)
      strcpy_s(f_ro.name, sizeof(f_ro.name), "ro_field");
      strcpy_s(f_ro.type, sizeof(f_ro.type), "string");
      strcpy_s(f_str_al.name, sizeof(f_str_al.name), "str_al");
      strcpy_s(f_str_al.type, sizeof(f_str_al.type), "string");
      strcpy_s(f_int_al.name, sizeof(f_int_al.name), "int_al");
      strcpy_s(f_int_al.type, sizeof(f_int_al.type), "integer");
#else
      strcpy(f_ro.name, "ro_field");
      strcpy(f_ro.type, "string");
      strcpy(f_str_al.name, "str_al");
      strcpy(f_str_al.type, "string");
      strcpy(f_int_al.name, "int_al");
      strcpy(f_int_al.type, "integer");
#endif
      f_ro.read_only = 1;
      str_aliases[0] = s_al_buf;
      str_aliases[1] = NULL;
      f_str_al.serde_aliases = str_aliases;
      f_str_al.n_serde_aliases = 2;

      int_aliases[0] = i_al_buf;
      int_aliases[1] = NULL;
      f_int_al.serde_aliases = int_aliases;
      f_int_al.n_serde_aliases = 2;

      sf_edge.fields[sf_edge.size++] = f_ro;
      sf_edge.fields[sf_edge.size++] = f_str_al;
      sf_edge.fields[sf_edge.size++] = f_int_al;

      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_from_json_standalone_func(tmp, "EdgeS", &sf_edge));

      sf_edge.fields[1].name[0] = '\0';
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_struct_from_json_standalone_func(tmp, "EdgeS", &sf_edge));
      sf_edge.fields[1].name[0] = 's';

      sf_edge.fields[2].name[0] = '\0';
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_struct_from_json_standalone_func(tmp, "EdgeS", &sf_edge));

      sf_edge.fields[1].serde_aliases = NULL;
      sf_edge.fields[1].n_serde_aliases = 0;
      sf_edge.fields[2].serde_aliases = NULL;
      sf_edge.fields[2].n_serde_aliases = 0;
      struct_fields_free(&sf_edge);
    }

    {
      struct StructFields sf_cov;
      int k;
      ASSERT_EQ(0, struct_fields_init(&sf_cov));
      struct_fields_add(&sf_cov, "skip_f", "string", NULL, NULL, NULL);
      sf_cov.fields[sf_cov.size - 1].serde_skip = 1;
      struct_fields_add(&sf_cov, "skip_de", "string", NULL, NULL, NULL);
      sf_cov.fields[sf_cov.size - 1].serde_skip_deserializing = 1;
      struct_fields_add(&sf_cov, "skip_ser", "string", NULL, NULL, NULL);
      sf_cov.fields[sf_cov.size - 1].serde_skip_serializing = 1;
      struct_fields_add(&sf_cov, "custom_t", "custom", NULL, NULL, NULL);
      struct_fields_add(&sf_cov, "obj_first", "object", "#/definitions/Sub",
                        NULL, NULL);
      struct_fields_add(&sf_cov, "fix_str_def", "string", NULL, "hi", NULL);
      sf_cov.fields[sf_cov.size - 1].fixed_array_len = 8;
      struct_fields_add(&sf_cov, "enum_def", "enum", "#/definitions/E", "V1",
                        NULL);
      struct_fields_add(&sf_cov, "num_excl", "number", NULL, NULL, NULL);
      sf_cov.fields[sf_cov.size - 1].has_min = 1;
      sf_cov.fields[sf_cov.size - 1].exclusive_min = 1;
      sf_cov.fields[sf_cov.size - 1].min_val = 0.0;
      sf_cov.fields[sf_cov.size - 1].has_max = 1;
      sf_cov.fields[sf_cov.size - 1].exclusive_max = 1;
      sf_cov.fields[sf_cov.size - 1].max_val = 100.0;
      struct_fields_add(&sf_cov, "num_incl", "number", NULL, NULL, NULL);
      sf_cov.fields[sf_cov.size - 1].has_min = 1;
      sf_cov.fields[sf_cov.size - 1].min_val = 0.0;
      sf_cov.fields[sf_cov.size - 1].has_max = 1;
      sf_cov.fields[sf_cov.size - 1].max_val = 100.0;
      struct_fields_add(&sf_cov, "num_none", "number", NULL, NULL, NULL);
      struct_fields_add(&sf_cov, "int_f", "integer", NULL, NULL, NULL);
      struct_fields_add(&sf_cov, "bool_f", "boolean", NULL, NULL, NULL);
      struct_fields_add(&sf_cov, "enum_arr_explicit", "array",
                        "#/definitions/E", NULL, NULL);
      sf_cov.fields[sf_cov.size - 1].is_enum_array = 1;
      CDD_STRCPY(sf_cov.fields[sf_cov.size - 1].enum_array_ref,
                 sizeof(sf_cov.fields[sf_cov.size - 1].enum_array_ref),
                 "#/definitions/E");
      struct_fields_add(&sf_cov, "empty_arr", "array", NULL, NULL, NULL);

      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_from_json_standalone_func(tmp, "CovS", &sf_cov));
      ASSERT_EQ(CDD_C_SUCCESS,
                write_struct_to_json_standalone_func(tmp, "CovS", &sf_cov));
      for (k = 0; k < 40; ++k) {
        cdd_c_error_t rc_io;
        g_fail_io_after = k;
        g_io_calls = 0;
        rc_io = write_struct_to_json_standalone_func(tmp, "CovS", &sf_cov);
        g_fail_io_after = -1;
        if (rc_io == CDD_C_SUCCESS)
          break;
      }
      sf_cov.fields[4].name[0] = '\0';
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                write_struct_to_json_standalone_func(tmp, "CovS", &sf_cov));
      sf_cov.fields[4].name[0] = 'o';
#ifdef CDD_BUILD_TESTS
      {
        extern C_CDD_EXPORT int g_cdd_fail_str_after_last;
        struct StructFields sf_sub_arr;
        ASSERT_EQ(0, struct_fields_init(&sf_sub_arr));
        struct_fields_add(&sf_sub_arr, "subs", "array", "#/definitions/Sub",
                          NULL, NULL);
        g_cdd_fail_str_after_last = 1;
        ASSERT_NEQ(CDD_C_SUCCESS, write_struct_from_json_standalone_func(
                                      tmp, "CovS", &sf_cov));
        g_cdd_fail_str_after_last = 2;
        ASSERT_NEQ(CDD_C_SUCCESS, write_struct_from_json_standalone_func(
                                      tmp, "CovS", &sf_cov));
        g_cdd_fail_str_after_last = 3;
        ASSERT_NEQ(CDD_C_SUCCESS, write_struct_from_json_standalone_func(
                                      tmp, "CovS", &sf_cov));
        g_cdd_fail_str_after_last = 1;
        ASSERT_NEQ(CDD_C_SUCCESS, write_struct_from_json_standalone_func(
                                      tmp, "SubArrS", &sf_sub_arr));
        g_cdd_fail_str_after_last = 1;
        ASSERT_NEQ(CDD_C_SUCCESS,
                   write_struct_to_json_standalone_func(tmp, "CovS", &sf_cov));
        g_cdd_fail_str_after_last = 2;
        ASSERT_NEQ(CDD_C_SUCCESS,
                   write_struct_to_json_standalone_func(tmp, "CovS", &sf_cov));
        g_cdd_fail_str_after_last = 3;
        ASSERT_NEQ(CDD_C_SUCCESS,
                   write_struct_to_json_standalone_func(tmp, "CovS", &sf_cov));
        g_cdd_fail_str_after_last = 1;
        ASSERT_NEQ(CDD_C_SUCCESS, write_struct_to_json_standalone_func(
                                      tmp, "SubArrS", &sf_sub_arr));
        g_cdd_fail_str_after_last = 0;
        struct_fields_free(&sf_sub_arr);
      }
#endif
      struct_fields_free(&sf_cov);
    }

    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Unit test test_json_dom_comprehensive.
 * @return Greatest test result.
 */
TEST test_json_dom_comprehensive(void) {
  JSON_Value *v = NULL;
  JSON_Value *copy = NULL;
  JSON_Object *o = NULL;
  JSON_Array *a = NULL;
  char *s = NULL;
  const char *cs = NULL;
  char bom_json[32];

  /* Null argument checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_parse_string(NULL, &v));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_parse_string("{}", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_parse_file(NULL, &v));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_parse_file("x.json", NULL));
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_json_parse_file("no_such_file_999.json", &v));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_init_object(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_init_array(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_init_string("a", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_init_string(NULL, &v));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_init_number(1.0, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_init_boolean(1, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_init_null(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_deep_copy(NULL, &v));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_deep_copy(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_serialize_to_string(NULL, &s));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_serialize_to_string(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_serialize_to_string_pretty(NULL, &s));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_serialize_to_string_pretty(v, NULL));
  ASSERT_EQ(JSONFailure, json_serialize_to_file_pretty(NULL, "x.json"));

  /* BOM + Unicode escape sequences (1, 2, 3, and 4 byte UTF-8) */
  bom_json[0] = '\xEF';
  bom_json[1] = '\xBB';
  bom_json[2] = '\xBF';
  bom_json[3] = '\"';
  bom_json[4] = '\\';
  bom_json[5] = 'u';
  bom_json[6] = '0';
  bom_json[7] = '0';
  bom_json[8] = '4';
  bom_json[9] = '1';
  bom_json[10] = '\\';
  bom_json[11] = 'u';
  bom_json[12] = '0';
  bom_json[13] = '0';
  bom_json[14] = 'e';
  bom_json[15] = '9';
  bom_json[16] = '\\';
  bom_json[17] = 'u';
  bom_json[18] = '2';
  bom_json[19] = '0';
  bom_json[20] = 'A';
  bom_json[21] = 'C';
  bom_json[22] = '\\';
  bom_json[23] = 'u';
  bom_json[24] = 'D';
  bom_json[25] = '8';
  bom_json[26] = '3';
  bom_json[27] = 'D';
  bom_json[28] = '\\';
  bom_json[29] = 'u';
  bom_json[30] = 'D';
  bom_json[31] = 'E';
  {
    char full_bom[40];
    memcpy(full_bom, bom_json, 32);
    full_bom[32] = '0';
    full_bom[33] = '0';
    full_bom[34] = '\"';
    full_bom[35] = '\0';
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string(full_bom, &v));
    ASSERT(v != NULL);
    json_value_free(v);
  }

  /* Invalid escapes, unpaired surrogates, leading zeros, trailing garbage */
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\uD800\"", &v));
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\uDC00\"", &v));
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\uZZZZ\"", &v));
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\x\"", &v));
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("01", &v));
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("-a", &v));
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("{} extra", &v));
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("true false", &v));
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("123 abc", &v));
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("[1,2] []", &v));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{}  \n\t\r", &v));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

  /* P1.3 & P1.4: Embedded NUL bytes in strings and object keys */
  {
    size_t slen = 0;
    const char raw_nul[5] = {'a', '\0', 'b', '\0', 'c'};
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_value_init_string_with_len(raw_nul, 5, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_string_len(v, &slen));
    ASSERT_EQ((size_t)5, slen);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string(v, &s));
    ASSERT_STR_EQ("\"a\\u0000b\\u0000c\"", s);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(s));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string(
                  "{\"k\\u00001\":\"v\\u00001\",\"k\\u00002\":\"v2\"}", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_get_count(v->object_val, &slen));
    ASSERT_EQ((size_t)2, slen);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_deep_copy(v, &copy));
    {
      int eq = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_equals(v, copy, &eq));
      ASSERT_EQ(1, eq);
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(copy));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* P1.5: Parse error diagnostics (line, column, byte_offset) */
  {
    struct cdd_json_parse_error perr;
    ASSERT_EQ(
        CDD_C_ERROR_PARSE,
        cdd_json_parse_string_ex("{\n  \"a\": 1,\n  \"b\": @\n}", &v, &perr));
    ASSERT_EQ((size_t)3, perr.line);
    ASSERT_EQ((size_t)8, perr.column);
    ASSERT(perr.byte_offset > 0);
  }

  /* P1.6: Comments, array_remove/clear, equals, validate, dotremove, buffer */
  {
    int eq = 0;
    int has = 0;
    size_t req_sz = 0;
    char sbuf[128];
    JSON_Value *schema = NULL;
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string_with_comments(
            "// top comment\n{\n /* block \n comment */ \"x\": [1, 2, 3]\n}",
            &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"x\":[0]}", &schema));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_validate(schema, v));
    ASSERT_EQ(JSONSuccess, json_validate(schema, v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(schema));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_array_remove(v->object_val->values[0]->array_val, 1));
    ASSERT_EQ((size_t)2,
              json_array_get_count(v->object_val->values[0]->array_val));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_array_clear(v->object_val->values[0]->array_val));
    ASSERT_EQ((size_t)0,
              json_array_get_count(v->object_val->values[0]->array_val));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_dotset_string(
                                 v->object_val, "nested.key", "val"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_dothas_value(v->object_val, "nested.key", &has));
    ASSERT_EQ(1, has);
    ASSERT_EQ(CDD_JSON_TRUE,
              json_object_dothas_value(v->object_val, "nested.key"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_dotremove(v->object_val, "nested.key"));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_dothas_value(v->object_val, "nested.key", &has));
    ASSERT_EQ(0, has);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialization_size(v, 0, &req_sz));
    ASSERT(req_sz > 2 && req_sz < sizeof(sbuf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_buffer(v, sbuf, 2, 0));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_serialize_to_buffer(v, sbuf, sizeof(sbuf), 0));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_equals(v, v, &eq));
    ASSERT_EQ(1, eq);
    ASSERT_EQ(CDD_JSON_TRUE, json_value_equals(v, v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* P2.1 & P2.2: 64-bit integer precision round-trip */
  {
    int64_t i64_in = (((int64_t)1) << 53) + (int64_t)1;
    int64_t i64_out = 0;
    uint64_t u64_in = (uint64_t)-1;
    uint64_t u64_out = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int64(i64_in, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int64(v, &i64_out));
    ASSERT_EQ(i64_in, i64_out);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string(v, &s));
    ASSERT_STR_EQ("9007199254740993", s);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(s));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_uint64(u64_in, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint64(v, &u64_out));
    ASSERT_EQ(u64_in, u64_out);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string(v, &s));
    ASSERT_STR_EQ("18446744073709551615", s);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(s));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("{\"big\":9007199254740993}", &v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_get_int64(v->object_val, "big", &i64_out));
    ASSERT_EQ(i64_in, i64_out);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_int64(v->object_val, "neg", -i64_in));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_get_int64(v->object_val, "neg", &i64_out));
    ASSERT_EQ(-i64_in, i64_out);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* P2.3: 128-bit integer precision round-trip, object/array helpers & hash
   * index */
  {
    cdd_int128_t i128_in;
    cdd_int128_t i128_out;
    cdd_uint128_t u128_in;
    cdd_uint128_t u128_out;
    int64_t i64_tmp = 0;
    uint64_t u64_tmp = 0;
    int idx;
    char kbuf[32];

    /* 2^100 + 7 = high: 1<<36, low: 7 */
    i128_in.high = ((int64_t)1) << 36;
    i128_in.low = 7u;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int128(i128_in, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128_out));
    ASSERT_EQ(i128_in.high, i128_out.high);
    ASSERT_EQ(i128_in.low, i128_out.low);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint128(v, &u128_out));
    ASSERT_EQ((uint64_t)i128_in.high, u128_out.high);
    ASSERT_EQ(i128_in.low, u128_out.low);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_get_int64(v, &i64_tmp));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_get_uint64(v, &u64_tmp));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string(v, &s));
    ASSERT_STR_EQ("1267650600228229401496703205383", s);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(s));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Negative 128-bit integer: -(2^100 + 7) */
    i128_in.high = ~(((int64_t)1) << 36);
    i128_in.low = (~((uint64_t)7u)) + 1u;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int128(i128_in, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128_out));
    ASSERT_EQ(i128_in.high, i128_out.high);
    ASSERT_EQ(i128_in.low, i128_out.low);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_get_uint128(v, &u128_out));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string(v, &s));
    ASSERT_STR_EQ("-1267650600228229401496703205383", s);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(s));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Max unsigned 128-bit integer: 2^128 - 1 */
    u128_in.high = (uint64_t)-1;
    u128_in.low = (uint64_t)-1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_uint128(u128_in, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint128(v, &u128_out));
    ASSERT_EQ(u128_in.high, u128_out.high);
    ASSERT_EQ(u128_in.low, u128_out.low);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_get_int128(v, &i128_out));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string(v, &s));
    ASSERT_STR_EQ("340282366920938463463374607431768211455", s);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(s));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Parse 128-bit numbers from JSON and test object/array helpers */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string(
                  "{\"u128\":340282366920938463463374607431768211455,"
                  "\"i128\":-1267650600228229401496703205383,"
                  "\"u64\":18446744073709551615}",
                  &v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_get_uint128(v->object_val, "u128", &u128_out));
    ASSERT_EQ(u128_in.high, u128_out.high);
    ASSERT_EQ(u128_in.low, u128_out.low);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_get_int128(v->object_val, "i128", &i128_out));
    ASSERT_EQ(i128_in.high, i128_out.high);
    ASSERT_EQ(i128_in.low, i128_out.low);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_get_uint64(v->object_val, "u64", &u64_tmp));
    ASSERT_EQ((uint64_t)-1, u64_tmp);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_uint64(v->object_val, "u64_2", 12345u));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_int128(v->object_val, "i128_2", i128_in));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_uint128(v->object_val, "u128_2", u128_in));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Array 64/128-bit helpers */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_int64(v->array_val, -42));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_uint64(v->array_val, 42u));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_array_append_int128(v->array_val, i128_in));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_array_append_uint128(v->array_val, u128_in));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_array_get_int64(v->array_val, 0, &i64_tmp));
    ASSERT_EQ((int64_t)-42, i64_tmp);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_array_get_uint64(v->array_val, 1, &u64_tmp));
    ASSERT_EQ((uint64_t)42u, u64_tmp);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_array_get_int128(v->array_val, 2, &i128_out));
    ASSERT_EQ(i128_in.high, i128_out.high);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_array_get_uint128(v->array_val, 3, &u128_out));
    ASSERT_EQ(u128_in.high, u128_out.high);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Hash index exercise (> 32 keys, overwrite, lookup, remove) */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&v));
    for (idx = 0; idx < 40; ++idx) {
#if defined(_MSC_VER)
      sprintf_s(kbuf, sizeof(kbuf), "key_%d", idx);
#else
      sprintf(kbuf, "key_%d", idx);
#endif
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_int64(v->object_val, kbuf, (int64_t)idx));
    }
    /* Overwrite via hash table */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_int64(v->object_val, "key_15", 999));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_get_int64(v->object_val, "key_15", &i64_tmp));
    ASSERT_EQ((int64_t)999, i64_tmp);
    ASSERT_EQ(
        CDD_C_ERROR_NOT_FOUND,
        cdd_json_object_get_int64(v->object_val, "missing_key", &i64_tmp));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_remove(v->object_val, "key_15"));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_object_get_int64(v->object_val, "key_15", &i64_tmp));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Trailing commas in cdd_json_parse_string_with_comments */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string_with_comments(
                  "{\"arr\": [1, 2, ], \"obj\": {\"k\": \"v,\", }, }", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* RFC 9535 JSONPath nodelist query & recursive descent */
    {
      JSON_Value *jp_doc = NULL;
      JSON_Value *nodelist = NULL;
      const JSON_Value *first_match = NULL;
      size_t nl_cnt = 0;
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_parse_string(
              "{\"store\":{\"book\":[{\"author\":\"A\",\"price\":10},"
              "{\"author\":\"B\",\"price\":20}],\"bicycle\":{\"price\":30}}}",
              &jp_doc));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_jsonpath_get(jp_doc, "$..price", &first_match));
      ASSERT(first_match != NULL);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_jsonpath_get(jp_doc, "$..*", &first_match));
      ASSERT(first_match != NULL);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_jsonpath_query(jp_doc, "$..price", &nodelist));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nodelist->array_val, &nl_cnt));
      ASSERT_EQ(3u, nl_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_jsonpath_query(jp_doc, "$.store.book[*].author", &nodelist));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nodelist->array_val, &nl_cnt));
      ASSERT_EQ(2u, nl_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_jsonpath_query(jp_doc, "$.store['book','bicycle']",
                                        &nodelist));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nodelist->array_val, &nl_cnt));
      ASSERT_EQ(2u, nl_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_query(
                                   jp_doc, "$.store.book[0,1]", &nodelist));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nodelist->array_val, &nl_cnt));
      ASSERT_EQ(2u, nl_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_query(
                                   jp_doc, "$.store.book[:2:1]", &nodelist));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nodelist->array_val, &nl_cnt));
      ASSERT_EQ(2u, nl_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_query(
                                   jp_doc, "$.store.book[0:2:1]", &nodelist));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nodelist->array_val, &nl_cnt));
      ASSERT_EQ(2u, nl_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

      /* RFC 9535 compound filters (&&, ||, !) and functions (length, count,
       * match, search) */
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_jsonpath_get(
              jp_doc,
              "$.store.book[?(@.price >= 10 && !(@.author == 'A'))].author",
              &first_match));
      ASSERT(first_match != NULL && first_match->type == JSONString);
      ASSERT_STR_EQ("B", first_match->string_val);

      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_jsonpath_get(
              jp_doc,
              "$.store.book[?(length(@.author) == 1 && match(@.author, 'B'))]",
              &first_match));
      ASSERT(first_match != NULL);

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_jsonpath_query(
                    jp_doc,
                    "$.store.book[?(search(@.author, 'A') || @.price == 20)]",
                    &nodelist));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nodelist->array_val, &nl_cnt));
      ASSERT_EQ(2u, nl_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(jp_doc));
    }

    /* RFC 9535 Extended JSONPath: nested filter paths, bracketed filter keys,
     * escaped quotes, root $ comparisons, value(), and I-Regexp match/search */
    {
      JSON_Value *ext_doc = NULL;
      JSON_Value *nodelist = NULL;
      const JSON_Value *first_match = NULL;
      size_t nl_cnt = 0;

      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_parse_string(
              "{\"max_price\":15,\"a'b\":\"quoted_val\",\"items\":["
              "{\"meta\":{\"code\":\"AB-12\",\"cost\":10},\"tags\":[\"x\"]},"
              "{\"meta\":{\"code\":\"CD-99\",\"cost\":20},\"tags\":[\"y\"]}]}",
              &ext_doc));

      /* Escaped quote in bracketed selector */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_jsonpath_get(ext_doc, "$['a\\'b']", &first_match));
      ASSERT(first_match != NULL && first_match->type == JSONString);
      ASSERT_STR_EQ("quoted_val", first_match->string_val);

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_jsonpath_query(ext_doc, "$['a\\'b']", &nodelist));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nodelist->array_val, &nl_cnt));
      ASSERT_EQ(1u, nl_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

      /* Nested filter path @.meta.cost <= $.max_price and @['meta']['cost'] */
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_jsonpath_query(
              ext_doc, "$.items[?(@.meta.cost <= $.max_price)]", &nodelist));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nodelist->array_val, &nl_cnt));
      ASSERT_EQ(1u, nl_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_jsonpath_get(
                    ext_doc,
                    "$.items[?(@['meta']['cost'] == 20 && @.tags[0] == 'y')]",
                    &first_match));
      ASSERT(first_match != NULL);

      /* value() function extension and I-Regexp match()/search() */
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(
                                   ext_doc,
                                   "$.items[?(value(@.meta.code) == 'AB-12' && "
                                   "match(@.meta.code, '^[A-Z]+-\\\\d+$') && "
                                   "search(@.meta.code, '12|99'))]",
                                   &first_match));
      ASSERT(first_match != NULL);

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(ext_doc));
    }

    /* RFC 8785 UTF-16 code unit key sorting & canonical number formatting,
     * cdd_json_array_insert_value, cdd_json_serialize_to_fp, and NDJSON */
    {
      JSON_Value *jcs_doc = NULL;
      JSON_Value *nd_arr = NULL;
      JSON_Value *nd_line = NULL;
      JSON_Value *ins_v = NULL;
      char *jcs_str = NULL;
      char *nd_str = NULL;
      size_t cnt = 0;

      /* U+10000 (\uD800\uDC00 -> F0 90 80 80) has UTF-16 unit 0xD800, which
       * must sort BEFORE U+E000 (\uE000 -> EE 80 80, UTF-16 unit 0xE000) */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string(
                    "{\"\\uE000\":1.00,\"\\uD800\\uDC00\":1e2,\"zero\":-0.0}",
                    &jcs_doc));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_serialize_to_string_canonical(jcs_doc, &jcs_str));
      ASSERT_STR_EQ("{\"zero\":0,\"\xF0\x90\x80\x80\":100,\"\xEE\x80\x80\":1}",
                    jcs_str);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(jcs_str));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(jcs_doc));

      /* NDJSON parse & serialize + cdd_json_array_insert_value */
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_ndjson_parse_line(NULL, &nd_line));
      ASSERT_EQ(CDD_C_ERROR_PARSE,
                cdd_json_ndjson_parse_line("  \r\n", &nd_line));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_ndjson_parse_line("{\"id\":1}\r\n", &nd_line));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nd_line));

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_ndjson_parse_string(
                                   "{\"a\":1}\n\n{\"b\":2}\r\n", &nd_arr));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nd_arr->array_val, &cnt));
      ASSERT_EQ(2u, cnt);

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int64(42, &ins_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_insert_value(nd_arr->array_val, 1, ins_v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_array_get_count(nd_arr->array_val, &cnt));
      ASSERT_EQ(3u, cnt);

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_ndjson_serialize_array(nd_arr->array_val, &nd_str));
      ASSERT_STR_EQ("{\"a\":1}\n42\n{\"b\":2}\n", nd_str);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(nd_str));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nd_arr));
    }

    /* Phase 2: cdd_json_parse_buffer, cdd_json_parse_string_with_options,
     * cdd_json_serialize_to_string_canonical,
     * cdd_json_pointer_escape/set/remove
     */
    {
      struct cdd_json_parse_options popts;
      char esc_tok[32];
      JSON_Value *set_val = NULL;
      const JSON_Value *got_val = NULL;

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_parse_buffer(NULL, 2, &v));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_parse_buffer("{}", 2, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_buffer("{\"k\":1}trailing", 7, &v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

      memset(&popts, 0, sizeof(popts));
      popts.reject_duplicate_keys = 1;
      ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string_with_options(
                                       "{\"a\":1,\"a\":2}", &popts, &v));
      popts.max_depth = 2;
      ASSERT_EQ(CDD_C_ERROR_PARSE,
                cdd_json_parse_string_with_options("[[[1]]]", &popts, &v));
      popts.max_depth = 0;
      popts.allow_comments = 1;
      popts.allow_trailing_commas = 1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string_with_options(
                                   "/* c */ {\"b\":2, \"a\":1, }", &popts, &v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string_canonical(v, &s));
      ASSERT_STR_EQ("{\"a\":1,\"b\":2}", s);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(s));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_pointer_escape(NULL, esc_tok, sizeof(esc_tok)));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_pointer_escape("a/b~c", NULL, sizeof(esc_tok)));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_pointer_escape("a/b~c", esc_tok, 4));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_pointer_escape("a/b~c", esc_tok, sizeof(esc_tok)));
      ASSERT_STR_EQ("a~1b~0c", esc_tok);

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string(
                                   "{\"obj\":{\"x\":1},\"arr\":[10,20]}", &v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int64(99, &set_val));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_set(v, "/obj/y", set_val));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_get(v, "/obj/y", &got_val));
      ASSERT(got_val != NULL && got_val->int64_val == 99);

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int64(30, &set_val));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_set(v, "/arr/1", set_val));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_remove(v, "/obj/x"));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_json_pointer_get(v, "/obj/x", &got_val));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_remove(v, "/arr/0"));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    }
  }

  /* P3.1 & P3.4: RFC 6901 JSON Pointer, RFC 7396 Merge Patch, RFC 6902 Patch */
  {
    const JSON_Value *found = NULL;
    JSON_Value *patch = NULL;
    JSON_Value *patched = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("{\"a\":{\"b/c\":[\"x\",\"y\"]}}", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_get(v, "#/a/b~1c/1", &found));
    ASSERT(found != NULL && found->type == JSONString);
    ASSERT_STR_EQ("y", found->string_val);

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("{\"a\":null,\"d\":42}", &patch));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_merge_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_object_get_value(patched->object_val, "a", &copy));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patched));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string(
            "[{\"op\":\"test\",\"path\":\"/a/b~1c/0\",\"value\":\"x\"},"
            "{\"op\":\"add\",\"path\":\"/a/b~1c/-\",\"value\":\"z\"},"
            "{\"op\":\"replace\",\"path\":\"/a/b~1c/0\",\"value\":\"w\"},"
            "{\"op\":\"copy\",\"from\":\"/a/b~1c/2\",\"path\":\"/copied\"},"
            "{\"op\":\"move\",\"from\":\"/copied\",\"path\":\"/moved\"},"
            "{\"op\":\"remove\",\"path\":\"/a/b~1c/1\"}]",
            &patch));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_get(patched, "/moved", &found));
    ASSERT_STR_EQ("z", found->string_val);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* Object & Array mutators, dotset/dotget, clear, replace */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_value_init_object(&_cdd_jv_1);
    v = _cdd_jv_1;
  }
  o = json_value_get_object(v);
  ASSERT_EQ(JSONSuccess, json_object_dotset_string(o, "a.b.c", "hello"));
  ASSERT_EQ(JSONSuccess, json_object_dotset_number(o, "a.b.n", 12.5));
  ASSERT_EQ(JSONSuccess, json_object_dotset_boolean(o, "a.b.flag", 1));
  ASSERT_EQ(JSONSuccess, json_object_dotset_null(o, "a.b.nil"));
  {
    JSON_Value *_cdd_jv_1 = NULL;
    const char *_cdd_js_1 = NULL;
    if (cdd_json_object_dotget_value(o, "a.b.c", &_cdd_jv_1) == CDD_C_SUCCESS &&
        _cdd_jv_1 && _cdd_jv_1->type == JSONString)
      _cdd_js_1 = _cdd_jv_1->string_val;
    ASSERT_STR_EQ("hello", _cdd_js_1);
  }
  {
    JSON_Value *_cdd_jv_1 = NULL;
    JSON_Object *_cdd_jo_1 = NULL;
    if (cdd_json_object_dotget_value(o, "a.b", &_cdd_jv_1) == CDD_C_SUCCESS &&
        _cdd_jv_1 && _cdd_jv_1->type == JSONObject)
      _cdd_jo_1 = _cdd_jv_1->object_val;
    ASSERT(_cdd_jo_1 != NULL);
  }
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_object_get_string(o, "a", &cs));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_object_get_array(o, "a", &a));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_deep_copy(v, &copy));
  json_value_free(copy);
  ASSERT_EQ(JSONSuccess, json_object_clear(o));
  ASSERT_EQ(0u, json_object_get_count(o));
  json_value_free(v);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    cdd_json_value_init_array(&_cdd_jv_1);
    v = _cdd_jv_1;
  }
  a = json_value_get_array(v);
  ASSERT_EQ(JSONSuccess, json_array_append_string(a, "s0"));
  ASSERT_EQ(JSONSuccess, json_array_append_number(a, 1.0));
  ASSERT_EQ(JSONSuccess, json_array_append_boolean(a, 0));
  ASSERT_EQ(JSONSuccess, json_array_append_null(a));
  ASSERT_EQ(JSONSuccess, json_array_replace_string(a, 0, "s1"));
  ASSERT_EQ(JSONSuccess, json_array_replace_number(a, 1, 2.0));
  ASSERT_EQ(JSONSuccess, json_array_replace_boolean(a, 2, 1));
  ASSERT_EQ(JSONSuccess, json_array_replace_null(a, 3));
  ASSERT_EQ(JSONFailure, json_array_replace_null(a, 99));
  ASSERT_EQ(CDD_JSON_TRUE, json_array_get_boolean(a, 2));
  ASSERT_EQ(CDD_JSON_ERR_BOOL, json_array_get_boolean(a, 99));
  json_value_free(v);

  PASS();
}

static int g_custom_fail_at = -1;
static int g_custom_alloc_cnt = 0;

/**
 * @brief Custom test allocator that fails when g_custom_alloc_cnt ==
 * g_custom_fail_at.
 */
static void *test_custom_malloc(size_t sz) {
  if (g_custom_fail_at >= 0 && g_custom_alloc_cnt++ == g_custom_fail_at) {
    return NULL;
  }
  return malloc(sz);
}

/**
 * @brief Custom test deallocator.
 */
static void test_custom_free(void *p) { free(p); }

/**
 * @brief Exhaustive coverage test for json_dom.c (128-bit math, accessors,
 * mutators, parser, serializer, JSON Pointer, JSONPath, I-Regexp, Merge Patch,
 * JSON Patch, and OOM paths).
 * @return Greatest test result.
 */
TEST test_json_dom_exhaustive_edges(void) {
  JSON_Value *v = NULL;
  JSON_Value *v2 = NULL;
  JSON_Object *o = NULL;
  JSON_Array *a = NULL;
  char *s = NULL;
  const char *cs = NULL;
  size_t len = 0;
  double d = 0.0;
  int bval = 0;
  int64_t i64 = 0;
  uint64_t u64 = 0;
  cdd_int128_t i128;
  cdd_uint128_t u128;
  JSON_Value_Type vtype;

  /* 1. Custom allocation functions & ctx_dealloc coverage */
  ASSERT_EQ(JSONSuccess, json_set_allocation_functions(test_custom_malloc,
                                                       test_custom_free));
  g_custom_fail_at = -1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_json_parse_string("{\"k\":\"v\",\"arr\":[1,2]}", &v));
  {
    const JSON_Value *found = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_get(v, "/k", &found));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(v, "$.k", &found));
  }
  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string(v, &s));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(s));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  ASSERT_EQ(JSONSuccess, json_set_allocation_functions(malloc, free));
  ASSERT_EQ(JSONSuccess, json_set_allocation_functions(NULL, NULL));

  /* 2. Init & getter edge cases, NaN/Inf, 64/128-bit conversions */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_init_string(NULL, &v));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_init_string("x", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_init_string_with_len(NULL, 1, &v));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_init_string_with_len("x", 1, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_string_with_len("", 0, &v));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_get_string_len(NULL, &len));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_get_string_len(v, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_string_len(v, &len));
  ASSERT_EQ(0u, len);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_get_type(NULL, &vtype));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_type(v, NULL));
  ASSERT_EQ(JSONError, json_value_get_type(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_get_boolean(NULL, &bval));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_boolean(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_boolean(v, &bval));
  ASSERT_EQ(CDD_JSON_ERR_BOOL, json_value_get_boolean(NULL));
  ASSERT_EQ(CDD_JSON_ERR_BOOL, json_value_get_boolean(v));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_number(NULL, &d));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_number(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_object(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_array(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_array(v, &a));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_string(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_string(NULL, &cs));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_int64(NULL, &i64));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_int64(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_int64(v, &i64));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_get_uint64(NULL, &u64));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_uint64(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_uint64(v, &u64));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_get_int128(NULL, &i128));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_int128(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_int128(v, &i128));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_value_get_uint128(NULL, &u128));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_uint128(v, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_uint128(v, &u128));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

  /* NaN and Infinity in init_number */
  {
    double zero_d = 0.0;
    double nan_d = zero_d / zero_d;
    double inf_d = 1.0 / zero_d;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_init_number(nan_d, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_init_number(inf_d, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_init_int64(0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_init_uint64(0, NULL));
    i128.low = 0;
    i128.high = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_init_int128(i128, NULL));
    u128.low = 0;
    u128.high = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_init_uint128(u128, NULL));
  }

  /* 128-bit & 64-bit cross-kind conversions and carry/borrow branches */
  {
    /* init_uint128 with 0 */
    u128.low = 0u;
    u128.high = 0u;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_uint128(u128, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int64(v, &i64));
    ASSERT_EQ(0, i64);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint64(v, &u64));
    ASSERT_EQ(0u, u64);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint128(v, &u128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* init_uint128 with high=0, low > INT64_MAX (num_kind == 2) */
    u128.low = (((uint64_t)1u) << 63) + 5u;
    u128.high = 0u;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_uint128(u128, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_int64(v, &i64));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint64(v, &u64));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint128(v, &u128));
    /* Artificially set uint64_val <= INT64_MAX with num_kind==2 to cover branch
     */
    v->uint64_val = 42u;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int64(v, &i64));
    ASSERT_EQ(42, i64);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* init_int128 with small positive (num_kind == 1), small negative (num_kind
     * == 1), high==0 && low > INT64_MAX (num_kind == 2), and lo==0 negative
     * (hi++ carry) */
    i128.low = 123u;
    i128.high = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int128(i128, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint128(v, &u128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    i128.low = (uint64_t)(-50);
    i128.high = -1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int128(i128, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_uint64(v, &u64));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_get_uint128(v, &u128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    i128.low = (((uint64_t)1u) << 63) + 9u;
    i128.high = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int128(i128, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Negative 128-bit with low == 0 -> triggers ++lo == 0u => hi++ in
     * i128_to_double */
    i128.low = 0u;
    i128.high = -2;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int128(i128, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_int64(v, &i64));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_uint64(v, &u64));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* init_uint128 with high > 0 but bit 63 == 0 (num_kind == 4) -> get_int128
     * succeeds */
    u128.low = 7u;
    u128.high = 3u;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_uint128(u128, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_int64(v, &i64));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_uint64(v, &u64));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128));
    ASSERT_EQ(3, i128.high);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* num_kind == 0 (floating point number_val) -> get_int64, get_uint64,
     * get_int128, get_uint128 */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_number(12345.0, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int64(v, &i64));
    ASSERT_EQ(12345, i64);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint64(v, &u64));
    ASSERT_EQ(12345u, u64);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128));
    ASSERT_EQ(12345u, i128.low);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint128(v, &u128));
    ASSERT_EQ(12345u, u128.low);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_number(-12345.0, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_get_uint64(v, &u64));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_get_uint128(v, &u128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128));
    ASSERT_EQ(-1, i128.high);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* 3. Parser number edge cases: 128-bit carry/overflow, large exponents, hex
   * escapes */
  {
    /* 38-digit number that triggers u128_add carry and mid2 < mid1 in u128_mul
     */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string("340282366920938463463374607431768211455", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint128(v, &u128));
    ASSERT_EQ(~((uint64_t)0u), u128.low);
    ASSERT_EQ(~((uint64_t)0u), u128.high);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* 128-bit overflow on high > max_div10, low > max_div10, and digit > 5 */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string("340282366920938463463374607431768211456", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string("340282366920938463463374607431768211460", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string("9999999999999999999999999999999999999999", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Negative 128-bit boundaries: -(2^63), -(2^63 + 1), -(2^127), -(2^127 + 1)
     */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("-9223372036854775808", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int64(v, &i64));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("-9223372036854775809", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string("-170141183460469231731687303715884105728", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string("-170141183460469231731687303715884105729", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Positive > 2^64 but < 2^127 (num_kind == 3) */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("18446744073709551616", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_int128(v, &i128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_get_uint128(v, &u128));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Fractional & exponent edge cases: 0.000123, >19 fractional digits, e-400,
     * e+400 */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("0.0001234567890123456789012345e-400", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("1e50000", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("-", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("01", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("1.", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("1e", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("1e+", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("tru", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("fals", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("nul", &v));

    /* UTF-8 escapes: 1-byte (\u0041), 2-byte (\u00e9), 3-byte (\u20ac),
     * 4-byte surrogate pair (\uD83D\uDE00), and invalid surrogates/escapes */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string(
            "\"\\b\\f\\n\\r\\t\\/\\\\\\\"\\u0041\\u00e9\\u20AC\\uD83D\\uDE00\"",
            &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\u123G\"", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\uD800\"", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE,
              cdd_json_parse_string("\"\\uD800\\u0041\"", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\uDC00\"", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\x\"", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\x01\"", &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\", &v));
  }

  /* 4. Object & Array hash table rebuilds (>16 keys), getters, mutators */
  {
    int ki;
    char kname[16];
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&v));
    o = json_value_get_object(v);
    for (ki = 0; ki < 20; ++ki) {
#if defined(_MSC_VER)
      sprintf_s(kname, sizeof(kname), "key_%d", ki);
#else
      sprintf(kname, "key_%d", ki);
#endif
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_int64(o, kname, ki));
    }
    /* Overwrite key in hash table */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_int64(o, "key_5", 500));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_get_int64(o, "key_5", &i64));
    ASSERT_EQ(500, i64);
    /* Remove key and trigger rebuild_hash with existing hash_buckets */
    ASSERT_EQ(JSONSuccess, json_object_remove(o, "key_3"));
    ASSERT_EQ(JSONFailure, json_object_remove(o, "nonexistent"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_object_remove(NULL, "a"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_object_remove(o, NULL));

    /* Clear object with hash_buckets */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_object_clear(NULL));
    ASSERT_EQ(JSONFailure, json_object_clear(NULL));
    ASSERT_EQ(JSONSuccess, json_object_clear(o));

    /* Object getters & mutators invalid args and types */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_value(o, "k", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_value(o, NULL, &v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_string(o, "k", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_object(o, "k", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_array(o, "k", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_boolean(o, "k", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_number(o, "k", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_int64(o, "k", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_uint64(o, "k", NULL));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_object_get_int64(o, "missing", &i64));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_object_get_uint64(o, "missing", &u64));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_int128(o, "k", NULL));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_object_get_int128(o, "missing", &i128));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_uint128(o, "k", NULL));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_object_get_uint128(o, "missing", &u128));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_object_get_count(o, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_name(NULL, 0, &cs));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_name(o, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_name(o, 99, &cs));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_value_at(NULL, 0, &v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_value_at(o, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_get_value_at(o, 99, &v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dotget_value(o, "a.b", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dotget_value(NULL, "a.b", &v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dotget_value(o, NULL, &v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dothas_value(NULL, "a.b", &bval));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dothas_value(o, NULL, &bval));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dothas_value(o, "a.b", NULL));
    ASSERT_EQ(CDD_JSON_FALSE, json_object_dothas_value(NULL, "a.b"));
    ASSERT_EQ(CDD_JSON_ERR_BOOL, json_object_get_boolean(o, "missing"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_boolean(o, "flag", 1));
    ASSERT_EQ(CDD_JSON_TRUE, json_object_get_boolean(o, "flag"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_has_value(o, NULL, &bval));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_has_value(o, "flag", NULL));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_json_object_has_value_of_type(NULL, "flag", JSONBoolean, &bval));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_has_value_of_type(o, NULL, JSONBoolean, &bval));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_has_value_of_type(o, "flag", JSONBoolean, NULL));
    ASSERT_EQ(CDD_JSON_FALSE,
              json_object_has_value_of_type(o, "missing", JSONBoolean));
    ASSERT_EQ(CDD_JSON_TRUE,
              json_object_has_value_of_type(o, "flag", JSONBoolean));

    /* dotset / dotget / dotremove when intermediate key is non-object */
    ASSERT_EQ(JSONSuccess, json_object_dotset_string(o, "flag.sub", "val"));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_object_dotget_value(o, "flag.sub.deep", &v2));
    ASSERT_EQ(JSONFailure, json_object_dotremove(NULL, "flag.sub"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_object_dotremove(o, NULL));
    ASSERT_EQ(JSONFailure, json_object_dotremove(o, "flag.sub.deep"));
    ASSERT_EQ(JSONFailure, json_object_dotremove(o, "missing.sub"));
    ASSERT_EQ(JSONSuccess, json_object_dotremove(o, "flag.sub"));

    /* parent != NULL rejection */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_get_value(o, "flag", &v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_value(o, "dup_parent", v2));
    ASSERT_EQ(JSONFailure, json_object_set_value(NULL, "k", v2));
    ASSERT_EQ(JSONFailure, json_object_set_string(NULL, "k", "v"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_string(o, NULL, "v"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_string(o, "k", NULL));
    ASSERT_EQ(JSONFailure, json_object_set_number(NULL, "k", 1.0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_number(o, NULL, 1.0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_int64(NULL, "k", 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_int64(o, NULL, 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_uint64(NULL, "k", 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_uint64(o, NULL, 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_int128(NULL, "k", i128));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_int128(o, NULL, i128));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_uint128(NULL, "k", u128));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_uint128(o, NULL, u128));
    ASSERT_EQ(JSONFailure, json_object_set_boolean(NULL, "k", 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_set_boolean(o, NULL, 1));
    ASSERT_EQ(JSONFailure, json_object_set_null(NULL, "k"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_object_set_null(o, NULL));
    ASSERT_EQ(JSONFailure, json_object_dotset_value(NULL, "a.b", v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dotset_value(o, NULL, v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dotset_value(o, "a.b", NULL));
    ASSERT_EQ(JSONFailure, json_object_dotset_string(NULL, "a.b", "s"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dotset_string(o, NULL, "s"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dotset_string(o, "a.b", NULL));
    ASSERT_EQ(JSONFailure, json_object_dotset_number(NULL, "a.b", 1.0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dotset_number(o, NULL, 1.0));
    ASSERT_EQ(JSONFailure, json_object_dotset_boolean(NULL, "a.b", 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dotset_boolean(o, NULL, 1));
    ASSERT_EQ(JSONFailure, json_object_dotset_null(NULL, "a.b"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_object_dotset_null(o, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* 5. Array getters, mutators, resize (>16 items), replace, remove, clear */
  {
    int ai;
    JSON_Array *sub_a = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&v));
    a = json_value_get_array(v);
    for (ai = 0; ai < 20; ++ai) {
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_int64(a, ai));
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_value(a, v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_array(a, 20, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_array(a, 99, &sub_a));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_array(a, 0, &sub_a));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_get_array(a, 20, &sub_a));
    ASSERT(sub_a == v2->array_val);

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_number(a, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_number(a, 99, &d));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_get_number(a, 1, &d));
    ASSERT_EQ(1.0, d);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_replace_boolean(a, 0, 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_boolean(a, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_boolean(a, 99, &bval));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_get_boolean(a, 0, &bval));
    ASSERT_EQ(1, bval);

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_array_get_count(a, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_value(NULL, 0, &v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_value(a, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_value(a, 99, &v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_string(a, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_string(a, 99, &cs));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_object(a, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_object(a, 99, &o));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_int64(a, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_int64(a, 99, &i64));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_uint64(a, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_uint64(a, 99, &u64));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_int128(a, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_int128(a, 99, &i128));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_uint128(a, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_get_uint128(a, 99, &u128));

    ASSERT_EQ(JSONFailure, json_array_append_value(a, v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_append_int64(NULL, 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_append_uint64(NULL, 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_append_int128(NULL, i128));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_append_uint128(NULL, u128));
    ASSERT_EQ(JSONFailure, json_array_append_string(NULL, "s"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_append_string(a, NULL));
    ASSERT_EQ(JSONFailure, json_array_append_number(NULL, 1.0));
    ASSERT_EQ(JSONFailure, json_array_append_boolean(NULL, 1));
    ASSERT_EQ(JSONFailure, json_array_append_null(NULL));

    ASSERT_EQ(JSONFailure, json_array_replace_value(NULL, 0, v2));
    ASSERT_EQ(JSONFailure, json_array_replace_value(a, 99, v2));
    ASSERT_EQ(JSONFailure, json_array_replace_value(a, 0, NULL));
    ASSERT_EQ(JSONFailure, json_array_replace_value(a, 0, v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_null(&v2));
    ASSERT_EQ(JSONSuccess, json_array_replace_value(a, 0, v2));
    ASSERT_EQ(JSONFailure, json_array_replace_string(NULL, 0, "s"));
    ASSERT_EQ(JSONFailure, json_array_replace_string(a, 0, NULL));
    ASSERT_EQ(JSONFailure, json_array_replace_string(a, 99, "s"));
    ASSERT_EQ(JSONFailure, json_array_replace_number(NULL, 0, 1.0));
    ASSERT_EQ(JSONFailure, json_array_replace_number(a, 99, 1.0));
    ASSERT_EQ(JSONFailure, json_array_replace_boolean(NULL, 0, 1));
    ASSERT_EQ(JSONFailure, json_array_replace_boolean(a, 99, 1));
    ASSERT_EQ(JSONFailure, json_array_replace_null(NULL, 0));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_insert_value(NULL, 0, v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_insert_value(a, 99, v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_insert_value(a, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_array_insert_value(a, 0, v2));

    ASSERT_EQ(JSONFailure, json_array_remove(NULL, 0));
    ASSERT_EQ(JSONFailure, json_array_remove(a, 99));
    ASSERT_EQ(JSONSuccess, json_array_remove(a, 0));
    ASSERT_EQ(JSONFailure, json_array_clear(NULL));
    ASSERT_EQ(JSONSuccess, json_array_clear(a));
    /* Insert into empty array with existing capacity */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_null(&v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_insert_value(a, 0, v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Insert into newly initialized empty array (capacity == 0) */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&v));
    a = json_value_get_array(v);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_null(&v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_insert_value(a, 0, v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* 6. Equals, Validate, Deep Copy edge cases (depth limits, raw_num_str,
   * JSONError) */
  {
    JSON_Value dummy_err;
    memset(&dummy_err, 0, sizeof(dummy_err));
    dummy_err.type = JSONError;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_equals(NULL, NULL, NULL));
    ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_value_equals(&dummy_err, &dummy_err, &bval));
    ASSERT_EQ(1, bval);
    {
      JSON_Value dummy_err2;
      memset(&dummy_err2, 0, sizeof(dummy_err2));
      dummy_err2.type = JSONError;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_value_equals(&dummy_err, &dummy_err2, &bval));
      ASSERT_EQ(0, bval);
    }
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_deep_copy(&dummy_err, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_value_deep_copy(NULL, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_value_deep_copy(&dummy_err, NULL));

    /* Validate array schema with empty schema vs non-empty schema, object
     * schema mismatch */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_validate(NULL, &dummy_err));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_validate(&dummy_err, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("[]", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("[1,2]", &v2));
    ASSERT_EQ(JSONSuccess, json_validate(v, v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("[\"str\"]", &v));
    ASSERT_EQ(JSONFailure, json_validate(v, v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v2));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"a\":1}", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"a\":\"wrong\"}", &v2));
    ASSERT_EQ(JSONFailure, json_validate(v, v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"b\":1}", &v2));
    ASSERT_EQ(JSONFailure, json_validate(v, v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v2));
  }

  /* 7. Parse options, BOM, comments, trailing commas, parse_file,
   * serialize_to_file */
  {
    struct cdd_json_parse_options popts;
    struct cdd_json_parse_error perr;
    /* UTF-8 BOM in parse_string_ex and parse_string_with_options */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string_ex("\xEF\xBB\xBF{\"a\":1,}", &v, &perr));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(CDD_C_ERROR_PARSE,
              cdd_json_parse_string_ex("{\n\"a\": 1\n} extra", &v, &perr));
    ASSERT_EQ(3u, perr.line);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_parse_string_ex(NULL, &v, &perr));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_parse_string_ex("{}", NULL, &perr));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_parse_string_with_options(NULL, NULL, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_parse_string_with_options("{}", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string_with_options("{}", NULL, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    memset(&popts, 0, sizeof(popts));
    popts.allow_comments = 1;
    popts.allow_trailing_commas = 0;
    /* Trailing comma rejected when allow_trailing_commas == 0 */
    ASSERT_EQ(CDD_C_ERROR_PARSE,
              cdd_json_parse_string_with_options("{\"a\":1,}", &popts, &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE,
              cdd_json_parse_string_with_options("[1,]", &popts, &v));
    /* Comments inside and outside strings, multiline block comments, unclosed
     * block comment */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string_with_options(
                                 "\xEF\xBB\xBF// line comment\n/* multi\nline "
                                 "*/{\"s\":\"a\\\"//b/*c*/\"}\n",
                                 &popts, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(CDD_C_ERROR_PARSE,
              cdd_json_parse_string_with_options("/* unclosed", &popts, &v));
    ASSERT_EQ(CDD_C_ERROR_PARSE,
              cdd_json_parse_string_with_options("{} trailing", &popts, &v));

    popts.allow_comments = 0;
    popts.allow_trailing_commas = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string_with_options(
                                 "\xEF\xBB\xBF[1, 2, ]", &popts, &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* 8. Canonical JCS (RFC 8785) UTF-16 sorting & canonical numbers & serializer
   * edges */
  {
    char buf[64];
    /* Keys with 1-byte, 2-byte (\u00e9), 3-byte (\u20ac), 4-byte
     * (\uD83D\uDE00), prefix keys ("ab" vs "a"), and invalid UTF-8 byte (0xFF)
     */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&v));
    o = json_value_get_object(v);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_int64(o, "ab", -42));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_uint64(o, "a", 42u));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_number(o, "\xC3\xA9", 1.2345678901234568));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_number(o, "\xE2\x82\xAC", 1e-7));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_number(o, "\xF0\x9F\x98\x80", 1e25));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_set_string(o, "\xFF", "\b\f\n\r\t\"\\/\x05"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string_canonical(v, &s));
    ASSERT(s != NULL);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(s));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_string_canonical(NULL, &s));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_string_canonical(v, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_string(NULL, &s));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_string(v, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_string_pretty(NULL, &s));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_string_pretty(v, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialization_size(NULL, 0, &len));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialization_size(v, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_buffer(NULL, buf, sizeof(buf), 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_buffer(v, NULL, sizeof(buf), 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_buffer(v, buf, 0, 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_buffer(v, buf, 4, 1));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_fp(NULL, stdout, 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_fp(v, NULL, 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_file_pretty(NULL, "x.json"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_serialize_to_file_pretty(v, NULL));
    ASSERT_EQ(CDD_C_ERROR_IO, cdd_json_serialize_to_file_pretty(
                                  v, "/nonexistent_dir_xyz/a.json"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_parse_file(NULL, &v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_parse_file("x.json", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* serialize_to_buffer success (compact and pretty) */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("[true,false,null]", &v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_serialize_to_buffer(v, buf, sizeof(buf), 0));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_serialize_to_buffer(v, buf, sizeof(buf), 1));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* 9. NDJSON edge cases */
  {
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_ndjson_parse_line("{}", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_ndjson_parse_string(NULL, &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_ndjson_parse_string("{}", NULL));
    ASSERT_EQ(CDD_C_ERROR_PARSE,
              cdd_json_ndjson_parse_string("{\"a\":1}\n{bad}\n", &v));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_ndjson_serialize_array(NULL, &s));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_ndjson_serialize_array(a, NULL));
  }

  /* 10. JSON Pointer (RFC 6901) edge cases */
  {
    const JSON_Value *found = NULL;
    char esc_buf[16];
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_escape("a/b", esc_buf, 2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_escape("abc", esc_buf, 2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_get(NULL, "", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_get(v, NULL, &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_pointer_get(v, "", NULL));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("{\"a\":[10,20],\"b\":5}", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_get(v, "", &found));
    ASSERT(found == v);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_get(v, "#", &found));
    ASSERT(found == v);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_get(v, "no_slash", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_get(v, "/a~", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_get(v, "/a~2", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_pointer_get(v, "/missing", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_pointer_get(v, "/a/", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_pointer_get(v, "/a/01", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_pointer_get(v, "/a/xy", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_pointer_get(v, "/a/99", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_pointer_get(v, "/b/sub", &found));

    /* pointer_set & pointer_remove edge cases */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int64(7, &v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_set(NULL, "/x", v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_pointer_set(v, NULL, v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_set(v, "/x", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_pointer_set(v, "", v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_pointer_set(v, "bad", v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_set(v, "/a/~2", v2));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_pointer_set(v, "/missing/x", v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_set(v, "/b/x", v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_pointer_set(v, "/a/", v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_set(v, "/a/01", v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_set(v, "/a/xy", v2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_set(v, "/a/99", v2));
    /* append via /a/- and insert at index */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_set(v, "#/a/-", v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int64(8, &v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_set(v, "/a/3", v2));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_remove(NULL, "/b"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_pointer_remove(v, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_pointer_remove(v, ""));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_pointer_remove(v, "bad"));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_pointer_remove(v, "/missing/x"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_pointer_remove(v, "/b/x"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_pointer_remove(v, "/a/"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_pointer_remove(v, "/a/01"));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_pointer_remove(v, "/a/xy"));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_pointer_remove(v, "/a/99"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_pointer_remove(v, "#/b"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* 11. RFC 9535 JSONPath & I-Regexp comprehensive branch coverage */
  {
    const JSON_Value *found = NULL;
    JSON_Value *nodelist = NULL;
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string(
            "{\"arr\":[{\"k\":\"v1\\r\\n\\t\",\"n\":5,\"b\":true,\"nil\":null,"
            "\"sub\":[10,20,30],\"obj\":{\"p\":1}},"
            "{\"k\":\"v2_9\",\"n\":15,\"b\":false,\"nil\":1,"
            "\"sub\":[],\"obj\":{}}],\"prim\":42,\"empty_obj\":{},\"empty_"
            "arr\":[]}",
            &v));

    /* Invalid arguments */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(NULL, "$.arr", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, NULL, &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "arr", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, "$.arr", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(NULL, "$.arr", &nodelist));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, NULL, &nodelist));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, "arr", &nodelist));

    /* Wildcard .*, ..*, [*], ..key on objects, arrays, empty, primitives */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(v, "$.*", &found));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(v, "$.arr.*", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.prim.*", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.empty_obj.*", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.empty_arr.*", &found));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(v, "$..*", &found));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(v, "$.arr..*", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.prim..*", &found));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(v, "$..p", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$..missing_key", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.prim.sub", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.missing", &found));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(v, "$[*]", &found));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(v, "$.arr[*]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.prim[*]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.empty_arr[*]", &found));

    /* Quoted bracket selectors & escapes */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_jsonpath_get(v, "$[\"arr\"][0]['k']", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$['unclosed]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$['a\\']", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$['arr'bad]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.prim['k']", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$['missing']", &found));

    /* Array index & slice selectors in jsonpath_get and jsonpath_query */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(v, "$.arr[-1]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.arr[-99]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.arr[99]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.prim[0]", &found));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_jsonpath_get(v, "$.arr[0].sub[0:2:1]", &found));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_jsonpath_get(v, "$.arr[0].sub[-5:10:]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.arr[0].sub[2:1]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.prim[0:1]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[0:x]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[0:1:x]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[0:1:0]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[0:1extra]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[@]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$!bad", &found));

    /* Filter expressions: length/count/value, operators == != <= < >= >,
     * types (string, number, bool, null, $root), unary !, parenthesized, &&, ||
     */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_jsonpath_query(
            v,
            "$.arr[?(!!(@.n > 1 && @.n >= 5 && @.n < 10 && @.n <= 5 && "
            "@.n != 99 && @.b == true && @.b != false && @.nil == null && "
            "@.k != null && @.k == 'v1\\r\\n\\t' && @.k != 'other'))]",
            &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_jsonpath_query(
                  v,
                  "$.arr[?(count(@.sub) >= 3 && length(@.obj) <= 1 && "
                  "length(@.obj) < 2 && length(@.obj) > 0 && length(@.k) != 99 "
                  "&& length(@.n) == 0 || length(@.k) == 'bad_rhs')]",
                  &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

    /* $ root comparisons with string, bool, number, and sub-array index
     * @.sub[-1] */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_jsonpath_query(
            v,
            "$.arr[?(@.sub[-1] == 30 && @.sub[99] == 1 || "
            "@.n == $.arr[0].n || @.k == $.arr[0].k || @.b == $.arr[0].b)]",
            &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

    /* I-Regexp character classes, ranges, escapes (\d \D \s \S \w \W \n \r \t
     * \-), quantifiers (*, +, ?), anchors (^, $), alternation (|) */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_jsonpath_query(
            v,
            "$.arr[?(match(@.k, '^[^0-9][0-9a-z\\\\-]+\\\\s*$') || "
            "search(@.k, '\\\\D+\\\\d?\\\\w*\\\\W?\\\\S+\\\\n\\\\r\\\\t') || "
            "search(@.k, '') || match(@.k, '[]a-z]+'))]",
            &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));

    /* Invalid I-Regexp patterns & filter syntax errors */
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_json_jsonpath_get(v, "$.arr[?(match(@.k, '[unclosed'))]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(match(@.k, '\\\\'))]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(length(@) == 1)]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(length(@.k == 1)]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(match(@) )]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(match(@.k) )]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(match(@.k, 123))]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(match(@.k, 'a' )]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
              cdd_json_jsonpath_get(v, "$.arr[?(@.fn() == 1)]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(@['k' == 'v')]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(@[0 == 1)]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(@[- == 1)]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(@[bad] == 1)]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
              cdd_json_jsonpath_get(v, "$.arr[?(unknown == 1)]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
              cdd_json_jsonpath_get(v, "$.arr[?(@.n ~= 1)]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?(@.n == -)]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
              cdd_json_jsonpath_get(v, "$.arr[?(@.n == @bad)]", &found));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_get(v, "$.arr[?((@.n == 1)]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
              cdd_json_jsonpath_get(v, "$.arr[?(@.n == 1)extra]", &found));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_json_jsonpath_get(v, "$.prim[?(@.n == 1)]", &found));

    /* jsonpath_query union selectors, negative slices, wildcards */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_query(v, "$.*", &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_query(v, "$..*", &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_query(v, "$[*]", &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_jsonpath_query(v, "$.arr[0].sub[:2:1]", &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_jsonpath_query(v, "$.arr[0].sub[:-1:]", &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_jsonpath_query(v, "$.arr[0].sub[-10:10:2]", &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_jsonpath_query(v, "$.arr[0].sub[0:-1:]", &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_jsonpath_query(v, "$.arr[-1, 0]", &nodelist));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nodelist));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, "$.", &nodelist));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, "$.arr[0:1:0]", &nodelist));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, "$.arr[:1:0]", &nodelist));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, "$.arr[0:x]", &nodelist));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, "$.arr[0:1:x]", &nodelist));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, "$.arr[:x]", &nodelist));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, "$.arr[0 extra]", &nodelist));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_jsonpath_query(v, "$!bad", &nodelist));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* 12. RFC 7396 Merge Patch & RFC 6902 JSON Patch edge cases */
  {
    JSON_Value *patch = NULL;
    JSON_Value *patched = NULL;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_merge_patch(v, patch, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_merge_patch(v, NULL, &patched));

    /* Merge patch when patch is non-object, or target is NULL/non-object, or
     * nested object */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("42", &patch));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_merge_patch(NULL, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("{\"a\":{\"x\":1},\"b\":null}", &patch));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_merge_patch(NULL, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patched));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("{\"a\":{\"y\":2},\"c\":3}", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_merge_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    /* JSON Patch (RFC 6902) root replacement, self-move, array
     * replace/copy/move, errors */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, NULL, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(NULL, v, &patched));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, v, &patched));

    /* Root add/replace and self move */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string(
            "[{\"op\":\"add\",\"path\":\"\",\"value\":{\"arr\":[1,2],\"k\":1}},"
            "{\"op\":\"replace\",\"path\":\"\",\"value\":{\"arr\":[10,20],"
            "\"k\":2}},"
            "{\"op\":\"move\",\"from\":\"/k\",\"path\":\"/k\"},"
            "{\"op\":\"move\",\"from\":\"/arr/0\",\"path\":\"/arr/1\"}]",
            &patch));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    /* Patch failure cases: non-object op, missing op/path, test mismatch, bad
     * remove/replace/move/copy, unknown op */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("[123]", &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("[{\"op\":\"add\"}]", &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string(
                  "[{\"op\":\"test\",\"path\":\"/c\",\"value\":999}]", &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string("[{\"op\":\"test\",\"path\":\"/c\"}]", &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("[{\"op\":\"remove\",\"path\":\"/nope\"}]",
                                    &patch));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string(
                                 "[{\"op\":\"add\",\"path\":\"/x\"}]", &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("[{\"op\":\"replace\",\"path\":\"/x\"}]",
                                    &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string(
            "[{\"op\":\"replace\",\"path\":\"/nope\",\"value\":1}]", &patch));
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string("[{\"op\":\"move\",\"path\":\"/x\"}]", &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string(
            "[{\"op\":\"move\",\"from\":\"/nope\",\"path\":\"/x\"}]", &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string(
                  "[{\"op\":\"move\",\"from\":\"\",\"path\":\"/x\"}]", &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string("[{\"op\":\"copy\",\"path\":\"/x\"}]", &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_json_parse_string(
            "[{\"op\":\"copy\",\"from\":\"/nope\",\"path\":\"/x\"}]", &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("[{\"op\":\"unknown\",\"path\":\"/c\"}]",
                                    &patch));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_json_apply_patch(v, patch, &patched));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(patch));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
  }

  /* 13. OOM sweep across parsing, serialization, deep copy, JSONPath, Patch */
  {
    int fi;
    ASSERT_EQ(JSONSuccess, json_set_allocation_functions(test_custom_malloc,
                                                         test_custom_free));
    for (fi = 0; fi < 45; ++fi) {
      JSON_Value *oom_v = NULL;
      JSON_Value *oom_v2 = NULL;
      char *oom_s = NULL;
      g_custom_alloc_cnt = 0;
      g_custom_fail_at = fi;
      if (cdd_json_parse_string(
              "{\"k1\":\"v1\",\"k2\":[1,2,3],\"k3\":{\"sub\":true},"
              "\"k4\":12345678901234567890}",
              &oom_v) == CDD_C_SUCCESS) {
        if (cdd_json_value_deep_copy(oom_v, &oom_v2) == CDD_C_SUCCESS) {
          cdd_json_value_free(oom_v2);
        }
        if (cdd_json_serialize_to_string_pretty(oom_v, &oom_s) ==
            CDD_C_SUCCESS) {
          cdd_json_free_serialized_string(oom_s);
        }
        if (cdd_json_serialize_to_string_canonical(oom_v, &oom_s) ==
            CDD_C_SUCCESS) {
          cdd_json_free_serialized_string(oom_s);
        }
        if (cdd_json_jsonpath_query(oom_v, "$..*", &oom_v2) == CDD_C_SUCCESS) {
          cdd_json_value_free(oom_v2);
        }
        cdd_json_value_free(oom_v);
      }
    }
    /* Targeted OOM and edge sweep for remaining branches */
    for (fi = 0; fi < 15; ++fi) {
      JSON_Value *tv = NULL;
      JSON_Value *tv2 = NULL;
      JSON_Object *to = NULL;
      JSON_Array *ta = NULL;
      char *ts = NULL;
      struct cdd_json_parse_options popts;
      memset(&popts, 0, sizeof(popts));
      popts.allow_comments = 1;
      g_custom_alloc_cnt = 0;
      g_custom_fail_at = fi;
      if (cdd_json_value_init_int64(1, &tv) == CDD_C_SUCCESS)
        cdd_json_value_free(tv);
      g_custom_alloc_cnt = 0;
      if (cdd_json_value_init_uint64(1u, &tv) == CDD_C_SUCCESS)
        cdd_json_value_free(tv);
      g_custom_alloc_cnt = 0;
      i128.low = 1u;
      i128.high = 5;
      if (cdd_json_value_init_int128(i128, &tv) == CDD_C_SUCCESS)
        cdd_json_value_free(tv);
      g_custom_alloc_cnt = 0;
      u128.low = 1u;
      u128.high = 5u;
      if (cdd_json_value_init_uint128(u128, &tv) == CDD_C_SUCCESS)
        cdd_json_value_free(tv);
      g_custom_alloc_cnt = 0;
      if (cdd_json_parse_string_with_options("//c\n{}", &popts, &tv) ==
          CDD_C_SUCCESS)
        cdd_json_value_free(tv);
      g_custom_alloc_cnt = 0;
      if (cdd_json_ndjson_parse_string("{\"a\":1}\n{\"b\":2}\n", &tv) ==
          CDD_C_SUCCESS) {
        if (cdd_json_ndjson_serialize_array(tv->array_val, &ts) ==
            CDD_C_SUCCESS)
          cdd_json_free_serialized_string(ts);
        cdd_json_value_free(tv);
      }

      /* Mutators with OOM at step fi */
      g_custom_fail_at = -1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&tv));
      to = tv->object_val;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&tv2));
      ta = tv2->array_val;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_int64(ta, 1));

      g_custom_alloc_cnt = 0;
      g_custom_fail_at = fi;
      cdd_json_object_set_string(to, "s", "v");
      g_custom_alloc_cnt = 0;
      cdd_json_object_set_number(to, "n", 1.0);
      g_custom_alloc_cnt = 0;
      cdd_json_object_set_int64(to, "i", 1);
      g_custom_alloc_cnt = 0;
      cdd_json_object_set_uint64(to, "u", 1u);
      g_custom_alloc_cnt = 0;
      cdd_json_object_set_int128(to, "i128", i128);
      g_custom_alloc_cnt = 0;
      cdd_json_object_set_uint128(to, "u128", u128);
      g_custom_alloc_cnt = 0;
      cdd_json_object_set_boolean(to, "b", 1);
      g_custom_alloc_cnt = 0;
      cdd_json_object_set_null(to, "nil");
      g_custom_alloc_cnt = 0;
      cdd_json_object_dotset_string(to, "d.s", "v");
      g_custom_alloc_cnt = 0;
      cdd_json_object_dotset_number(to, "d.n", 1.0);
      g_custom_alloc_cnt = 0;
      cdd_json_object_dotset_boolean(to, "d.b", 1);
      g_custom_alloc_cnt = 0;
      cdd_json_object_dotset_null(to, "d.nil");

      g_custom_alloc_cnt = 0;
      cdd_json_array_append_int64(ta, 1);
      g_custom_alloc_cnt = 0;
      cdd_json_array_append_uint64(ta, 1u);
      g_custom_alloc_cnt = 0;
      cdd_json_array_append_int128(ta, i128);
      g_custom_alloc_cnt = 0;
      cdd_json_array_append_uint128(ta, u128);
      g_custom_alloc_cnt = 0;
      cdd_json_array_append_string(ta, "v");
      g_custom_alloc_cnt = 0;
      cdd_json_array_append_number(ta, 1.0);
      g_custom_alloc_cnt = 0;
      cdd_json_array_append_boolean(ta, 1);
      g_custom_alloc_cnt = 0;
      cdd_json_array_append_null(ta);
      g_custom_alloc_cnt = 0;
      cdd_json_array_replace_string(ta, 0, "v");
      g_custom_alloc_cnt = 0;
      cdd_json_array_replace_number(ta, 0, 1.0);
      g_custom_alloc_cnt = 0;
      cdd_json_array_replace_boolean(ta, 0, 1);
      g_custom_alloc_cnt = 0;
      cdd_json_array_replace_null(ta, 0);

      g_custom_fail_at = -1;
      cdd_json_value_free(tv2);
      cdd_json_value_free(tv);
    }

    /* OOM on merge_patch, apply_patch, pointer_set, jsonpath_get */
    {
      JSON_Value *base_doc = NULL;
      JSON_Value *mp_doc = NULL;
      JSON_Value *jp_doc = NULL;
      g_custom_fail_at = -1;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string(
                    "{\"a\":{\"x\":1},\"arr\":[{\"k\":\"v\"},2],\"b\":2}",
                    &base_doc));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string("{\"a\":{\"y\":2},\"c\":3}", &mp_doc));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string(
                    "[{\"op\":\"add\",\"path\":\"/a/z\",\"value\":1},"
                    "{\"op\":\"replace\",\"path\":\"/a/x\",\"value\":9},"
                    "{\"op\":\"copy\",\"from\":\"/a/x\",\"path\":\"/b\"},"
                    "{\"op\":\"move\",\"from\":\"/b\",\"path\":\"/d\"}]",
                    &jp_doc));
      for (fi = 0; fi < 25; ++fi) {
        JSON_Value *out_doc = NULL;
        const JSON_Value *found = NULL;
        g_custom_alloc_cnt = 0;
        g_custom_fail_at = fi;
        if (cdd_json_merge_patch(base_doc, mp_doc, &out_doc) == CDD_C_SUCCESS)
          cdd_json_value_free(out_doc);
        g_custom_alloc_cnt = 0;
        if (cdd_json_apply_patch(base_doc, jp_doc, &out_doc) == CDD_C_SUCCESS)
          cdd_json_value_free(out_doc);
        g_custom_alloc_cnt = 0;
        cdd_json_jsonpath_get(base_doc, "$.arr[?(@.k == 'v')].k", &found);
        g_custom_alloc_cnt = 0;
        if (cdd_json_jsonpath_query(base_doc,
                                    "$.arr[?(match(@.k, '^v$'))]['k']",
                                    &out_doc) == CDD_C_SUCCESS)
          cdd_json_value_free(out_doc);
      }
      g_custom_fail_at = -1;
      cdd_json_value_free(jp_doc);
      cdd_json_value_free(mp_doc);
      cdd_json_value_free(base_doc);
    }

    g_custom_fail_at = -1;
    ASSERT_EQ(JSONSuccess, json_set_allocation_functions(NULL, NULL));

    /* Additional non-OOM branch coverage:
     * - json_free_serialized_string(NULL) and
     * cdd_json_free_serialized_string(NULL)
     * - json_object_has_value
     * - cdd_json_value_equals on Null, Boolean, Array, Object, raw_num_str, max
     * depth
     * - cdd_json_validate with Null schema
     * - NDJSON empty array, whitespace lines, single line without trailing \n
     * - JSON Pointer ~0 / ~1 escape buffer boundary
     * - I-Regexp dot (.), \b \f in jp_quoted, [\a-\z], [abc], ^ at non-zero
     * pos, * / + / ? match
     * - JSONPath filter existence (@.k), $.arr.*, $[*] on object
     * - JSON Patch replace existing object key, replace array elem,
     * remove/replace on primitive parent
     */
    ASSERT_EQ(CDD_C_SUCCESS, json_free_serialized_string(NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(NULL));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string(
                  "{\"a\":null,\"b\":true,\"arr\":[1,2],\"obj\":{\"x\":1},"
                  "\"big\":340282366920938463463374607431768211455}",
                  &v));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string(
                  "{\"a\":null,\"b\":true,\"arr\":[1,2],\"obj\":{\"x\":1},"
                  "\"big\":340282366920938463463374607431768211455}",
                  &v2));
    ASSERT_EQ(CDD_JSON_TRUE, json_value_equals(v, v2));
    o = v->object_val;
    ASSERT_EQ(CDD_JSON_TRUE, json_object_has_value(o, "a"));
    ASSERT_EQ(CDD_JSON_FALSE, json_object_has_value(o, "nope"));
    /* Validate with JSONNull schema */
    {
      JSON_Value *null_schema = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_null(&null_schema));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_validate(null_schema, v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(null_schema));
    }
    /* Modify v2 to test unequal array count, unequal array elem, unequal object
     * count, missing key, unequal val */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_object_get_array(v2->object_val, "arr", &a));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_array_append_int64(a, 3));
    ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(v, v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v2));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string(
                  "{\"a\":null,\"b\":true,\"arr\":[1,99],\"obj\":{\"x\":1},"
                  "\"big\":340282366920938463463374607431768211454}",
                  &v2));
    ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(v, v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v2));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"a\":null}", &v2));
    ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(v, v2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v2));

    /* Serialization buffer > 256 bytes growth */
    {
      char long_str[300];
      memset(long_str, 'A', sizeof(long_str) - 1);
      long_str[sizeof(long_str) - 1] = '\0';
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_string(v->object_val, "long", long_str));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string(v, &s));
      ASSERT_EQ(CDD_C_SUCCESS, json_free_serialized_string(s));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialization_size(v, 0, &len));
      ASSERT(len > 256u);
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* NDJSON empty line, whitespace line, no trailing newline, empty array
     * serialize */
    ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_ndjson_parse_line("\r\n", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_ndjson_parse_string(
                                 "  \t \n{\"a\":1}\n   \n{\"b\":2}", &v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_ndjson_serialize_array(v->array_val, &s));
    ASSERT_STR_EQ("", s);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(s));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));

    /* Pointer escape ~ overflow & ~0 / ~1 unescape */
    {
      char ebuf[3];
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_pointer_escape("~", ebuf, 2));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_pointer_escape(NULL, ebuf, 3));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_pointer_escape("a", NULL, 3));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_pointer_escape("a", ebuf, 0));
    }

    /* I-Regexp dot (.), \b \f, [\a-\z], [abc], ^ at pos > 0, + / ? quantifiers,
     * filter existence (@.k), value(@.k), $.arr.*, $[*] in jsonpath_query */
    {
      JSON_Value *nl = NULL;
      const JSON_Value *fnd = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string(
                    "{\"arr\":[{\"k\":\"abc\\b\\f\",\"n\":1},{\"k\":\"xyz\"}],"
                    "\"mat\":[[1,2],[3,4]]}",
                    &v));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_query(v, "$.mat.*", &nl));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_jsonpath_query(
              v,
              "$.arr[?(@.n && value(@.n) && @.k == 'abc\\b\\f' || "
              "search(@.k, '^xyz') || match(@.k, '.+[\\\\a-\\\\z][xyz]?'))]",
              &nl));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                cdd_json_jsonpath_get(v, "$.arr[?(@.n == 999)]", &fnd));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    }

    /* JSON Patch replace existing object key, replace array item, invalid
     * parent/index */
    {
      JSON_Value *pdoc = NULL;
      JSON_Value *pres = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string("{\"k\":1,\"arr\":[10,20]}", &v));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string(
                    "[{\"op\":\"replace\",\"path\":\"/k\",\"value\":2},"
                    "{\"op\":\"replace\",\"path\":\"/arr/0\",\"value\":99},"
                    "{\"op\":\"remove\",\"path\":\"/arr/1\"}]",
                    &pdoc));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_apply_patch(v, pdoc, &pres));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(pres));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(pdoc));

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string(
                    "[{\"op\":\"replace\",\"path\":\"/arr/bad\",\"value\":1}]",
                    &pdoc));
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_json_apply_patch(v, pdoc, &pres));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(pdoc));

      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_parse_string(
              "[{\"op\":\"replace\",\"path\":\"/k/sub\",\"value\":1}]", &pdoc));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_apply_patch(v, pdoc, &pres));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(pdoc));

      /* Add at non-slash path, leading zero index, out-of-range index,
       * primitive parent */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string(
                    "[{\"op\":\"add\",\"path\":\"bad\",\"value\":1}]", &pdoc));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_apply_patch(v, pdoc, &pres));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(pdoc));

      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_parse_string(
              "[{\"op\":\"add\",\"path\":\"/arr/01\",\"value\":1}]", &pdoc));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_apply_patch(v, pdoc, &pres));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(pdoc));

      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_parse_string(
              "[{\"op\":\"add\",\"path\":\"/arr/99\",\"value\":1}]", &pdoc));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_apply_patch(v, pdoc, &pres));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(pdoc));

      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_parse_string(
              "[{\"op\":\"add\",\"path\":\"/k/sub\",\"value\":1}]", &pdoc));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_apply_patch(v, pdoc, &pres));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(pdoc));

      /* Patch missing path, test missing target, apply_patch NULL out */
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_json_apply_patch(v, v, NULL));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_parse_string("[{\"op\":\"remove\"}]", &pdoc));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_apply_patch(v, pdoc, &pres));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(pdoc));

      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_parse_string(
              "[{\"op\":\"test\",\"path\":\"/missing\",\"value\":1}]", &pdoc));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_apply_patch(v, pdoc, &pres));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(pdoc));

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(v));
    }

    /* 14. Remaining parser, equals, depth limit, UTF-16 surrogate & filter
     * edges */
    {
      JSON_Value *va = NULL;
      JSON_Value *vb = NULL;
      JSON_Value dummy_err;
      char *tok = NULL;
      char *ser_out = NULL;
      size_t sz_out = 0;
      char sbuf[16];
      memset(&dummy_err, 0, sizeof(dummy_err));
      dummy_err.type = JSONError;

      /* Serialize JSONError to trigger error path in cdd_json_ser_value, size,
       * buffer, file_pretty */
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_serialize_to_string(&dummy_err, &ser_out));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_serialization_size(&dummy_err, 0, &sz_out));
      ASSERT_EQ(
          CDD_C_ERROR_INVALID_ARGUMENT,
          cdd_json_serialize_to_buffer(&dummy_err, sbuf, sizeof(sbuf), 0));
      {
        const char *tmp_fn = "tmp_json_err_test.json";
        ASSERT_EQ(JSONFailure,
                  json_serialize_to_file_pretty(&dummy_err, tmp_fn));
        remove(tmp_fn);
      }

      /* cdd_json_pointer_unescape invalid arguments */
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_pointer_unescape("a", 1, NULL));
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_json_pointer_unescape(NULL, 1, &tok));

      /* Unclosed string, missing colon in object, invalid separator in
       * object/array */
      ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"unclosed", &va));
      ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("{\"k\" 1}", &va));
      ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("{\"k\":1 2}", &va));
      ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("[1 2]", &va));

      /* Equals with different types, empty strings, unequal strings, missing
       * key in b, unequal child in b */
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int64(1, &va));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_string("", &vb));
      ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(va, vb));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_string("", &va));
      ASSERT_EQ(CDD_JSON_TRUE, json_value_equals(va, vb));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_string("diff", &vb));
      ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(va, vb));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"k1\":1}", &va));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"k2\":1}", &vb));
      ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(va, vb));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"k1\":2}", &vb));
      ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(va, vb));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));

      /* Surrogate pair comparison in canonical JCS (two different 4-byte UTF-8
       * keys) */
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&va));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_int64(
                                   va->object_val, "\xF0\x9F\x98\x81", 1));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_int64(
                                   va->object_val, "\xF0\x9F\x98\x80", 2));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_int64(va->object_val, "z", 3));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_object_set_int64(va->object_val, "za", 4));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_json_serialize_to_string_canonical(va, &ser_out));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(ser_out));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));

      /* Large > 512 byte single string to trigger while (ncap < b->len + n + 1)
       * ncap *= 2 */
      {
        char big_s[600];
        memset(big_s, 'B', sizeof(big_s) - 1);
        big_s[sizeof(big_s) - 1] = '\0';
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_string(big_s, &va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_serialize_to_string(va, &ser_out));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(ser_out));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
      }

      /* Deep nesting > CDD_JSON_MAX_DEPTH (512) for equals, deep_copy, and
       * serialize */
      {
        int di;
        JSON_Value *root_a = NULL;
        JSON_Value *cur_a = NULL;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&root_a));
        cur_a = root_a;
        for (di = 0; di < 2052; ++di) {
          JSON_Value *next_a = NULL;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&next_a));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_array_append_value(cur_a->array_val, next_a));
          cur_a = next_a;
        }
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_array_append_int64(cur_a->array_val, 1));
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_serialize_to_string(root_a, &ser_out));
        ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_value_deep_copy(root_a, &vb));
        {
          JSON_Value *root_b = NULL;
          JSON_Value *cur_b = NULL;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&root_b));
          cur_b = root_b;
          for (di = 0; di < 2052; ++di) {
            JSON_Value *next_b = NULL;
            ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&next_b));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_json_array_append_value(cur_b->array_val, next_b));
            cur_b = next_b;
          }
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_array_append_int64(cur_b->array_val, 1));
          ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(root_a, root_b));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(root_b));
        }
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(root_a));
      }

      /* Filter and I-Regexp remaining branches:
       * - \r, \t, and literal escape in cdd_json_iregexp_atom
       * - trailing backslash inside character class `[a\]`
       * - filter index on non-array `@.k[0]`
       * - match/search with unsupported path `match(@.fn(), 'a')` and unclosed
       * string `match(@.k, 'a)`
       * - `$` root path returning NULL (`$.missing`) or null value (`$.nil`),
       * and invalid `$` path (`$.`)
       * - `||` in existence check `@.k || @.n`
       * - compound filter error on RHS `@.k == 'a' && bad`
       */
      {
        const JSON_Value *fnd = NULL;
        JSON_Value *nl = NULL;
        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_json_parse_string(
                "{\"arr\":[{\"k\":\"\\r\\t-\",\"n\":1}],\"nil\":null}", &va));
        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_json_jsonpath_query(
                va,
                "$.arr[?(match(@.k, '^\\\\r\\\\t\\\\-$') || @.k[0] == 1 || "
                "@.k || @.n == $.nil || @.n == $.missing)]",
                &nl));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));

        ASSERT_EQ(
            CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_jsonpath_get(va, "$.arr[?(match(@.k, '[a\\\\'))]", &fnd));
        ASSERT_EQ(
            CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_jsonpath_get(va, "$.arr[?(match(@[bad], 'a'))]", &fnd));
        ASSERT_EQ(
            CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_jsonpath_get(va, "$.arr[?(length(@[bad]) == 1)]", &fnd));
        ASSERT_EQ(
            CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_jsonpath_get(va, "$.arr[?(match(@.k, 'unclosed))]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
                  cdd_json_jsonpath_get(va, "$.arr[?(@.n == $.)]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
                  cdd_json_jsonpath_get(va, "$.arr[?(@.n == 1 && bad)]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_query(va, "$.arr[bad]", &nl));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
      }

      /* 15. Final branch sweep:
       * - cdd_json_value_get_string_len on non-string
       * - cdd_json_value_init_int128 with high==-1, low < 2^63 (-(2^63 + 5))
       * - json_value_get_string on non-string
       * - cdd_json_object_set_value(obj, NULL, v)
       * - cdd_json_value_equals(a, NULL, &bval)
       * - cdd_json_value_equals on object where nested value is > MAX_DEPTH
       * - surrogate escapes with invalid 2nd backslash/u/hex or low < 0xDC00 or
       * low > 0xDFFF
       * - exponent with >5 digits (1e000001) and uppercase 'E' (1E+2, 1E)
       * - partial BOM (\xEF\x00, \xEF\xBB\x00) in parse_string_ex &
       * parse_string_with_options
       * - comment edge cases: escaped char at end of string, single `/` at end
       * or followed by non-comment
       * - parse_file OOM and serialize_to_fp write error (read-only FILE*)
       * - canonical number with int64 > 2^53 or < -2^53, truncated UTF-8 in JCS
       * keys
       * - canonical object sort OOM, ndjson_serialize_array OOM on 2nd element
       * - I-Regexp: empty `[]`, `\n` / `\r` against `.`, `\d` on `/` and `:`,
       * `\w` / `\W` on `@`, `[`, `` ` ``, `{`, `/`, `:`, `$a`
       * - Filter: tab whitespace `\t`, identifier with `A-Z`, `0-9`, `_`, `-`,
       * double-quoted `["k"]`, `$["k"]`, `$[0]`, `length(@.k) == "s"` (freeing
       * rhs_str_owned), `match(@.n, 'a')` on non-string/missing field,
       * `search(@.k, 'z')` non-matching
       * - JSONPath: `$..*` on empty array, `$..k` inside array, `$[:1]`,
       * `$[::2]`, `$[1:1]` in jsonpath_get, `$.a` on non-object in
       * jsonpath_query, `[?(@.a)]` on non-array in jsonpath_query, union
       * `["k"]` on non-object or missing key, union `[0]` / `[0:1]` on
       * non-array, union `[1, 2]` with comma
       * - Merge patch on non-object target, OOM during merge_patch set, OOM
       * during patch_resolve_parent strndup, pointer_remove OOM
       */
      {
        const JSON_Value *fnd = NULL;
        JSON_Value *nl = NULL;
        struct cdd_json_parse_error perr;
        struct cdd_json_parse_options popts;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int64(1, &va));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_value_get_string_len(va, &sz_out));
        ASSERT_EQ(NULL, json_value_get_string(va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_equals(va, NULL, &bval));
        ASSERT_EQ(0, bval);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&vb));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_object_set_value(vb->object_val, NULL, va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));

        /* int128 with high == -1 and bit 63 of low == 0 */
        i128.low = 5u;
        i128.high = -1;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int128(i128, &va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));

        /* Surrogate pair failure branches */
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_parse_string("\"\\uD800x\"", &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_parse_string("\"\\uD800\\x\"", &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_parse_string("\"\\uD800\\uZZZZ\"", &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_parse_string("\"\\uD800\\uE000\"", &va));

        /* Exponent branches: uppercase E, E without digits, >5 exponent digits
         */
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("1E2", &va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
        ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("1E", &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("1e/", &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("1e:", &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("1./", &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("1.:", &va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("1e000001", &va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));

        /* Partial BOM & comment edge cases */
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_parse_string_ex("\xEF\x01", &va, &perr));
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_parse_string_ex("\xEF\xBB\x01", &va, &perr));
        memset(&popts, 0, sizeof(popts));
        popts.allow_comments = 1;
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_parse_string_with_options("\xEF\x01", &popts, &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string_with_options(
                                         "\xEF\xBB\x01", &popts, &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_parse_string_with_options("\"\\", &popts, &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_parse_string_with_options("/", &popts, &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE,
                  cdd_json_parse_string_with_options("/x", &popts, &va));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_parse_string_with_options("{}//eof", &popts, &va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));

        /* parse_file OOM and serialize_to_fp write failure (read-only stream)
         */
        {
          const char *tmp_fn = "tmp_json_file_oom.json";
          FILE *rfp = NULL;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"a\":1}", &va));
          ASSERT_EQ(JSONSuccess, json_serialize_to_file_pretty(va, tmp_fn));
          ASSERT_EQ(JSONSuccess, json_set_allocation_functions(
                                     test_custom_malloc, test_custom_free));
          g_custom_alloc_cnt = 0;
          g_custom_fail_at = 0;
          ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_json_parse_file(tmp_fn, &vb));
          g_custom_fail_at = -1;
          ASSERT_EQ(JSONSuccess, json_set_allocation_functions(NULL, NULL));
#if defined(_MSC_VER)
          fopen_s(&rfp, tmp_fn, "rb");
#else
          rfp = fopen(tmp_fn, "rb");
#endif
          if (rfp) {
            /* Writing to a "rb" stream fails fwrite/fflush or triggers error */
            cdd_json_serialize_to_fp(va, rfp, 0);
            fclose(rfp);
          }
          remove(tmp_fn);
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
        }

        /* Canonical JCS: int64 outside +/-2^53, negative exponent with leading
         * 0, truncated UTF-8 in key, OOM in order alloc */
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&va));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_object_set_int64(va->object_val, "big_pos",
                                            (((int64_t)1) << 55)));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_object_set_int64(va->object_val, "big_neg",
                                            -(((int64_t)1) << 55)));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_object_set_number(va->object_val, "\xC3", 1e-9));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_number(va->object_val,
                                                            "\xE2\x82", 1e100));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_object_set_number(
                                     va->object_val, "\xF0\x9F\x98", 1.5));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_serialize_to_string_canonical(va, &ser_out));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(ser_out));
        ASSERT_EQ(JSONSuccess, json_set_allocation_functions(test_custom_malloc,
                                                             test_custom_free));
        g_custom_alloc_cnt = 0;
        g_custom_fail_at = 0;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  cdd_json_serialize_to_string_canonical(va, &ser_out));
        g_custom_fail_at = -1;
        ASSERT_EQ(JSONSuccess, json_set_allocation_functions(NULL, NULL));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));

        /* I-Regexp & Filter comprehensive character & operator coverage */
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_parse_string(
                      "{\"arr\":["
                      "{\"K_1-a\":\"a0_/"
                      ":@[`{\\n\\r\",\"n\":5,\"b\":true,\"nil\":null},"
                      "{\"K_1-a\":\"B\",\"n\":10,\"b\":false,\"nil\":1}"
                      "],\"obj\":{\"x\":1},\"empty_a\":[]}",
                      &va));
        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_json_jsonpath_query(
                va,
                "$.arr[?(\t@['K_1-a'] != 'z' && @[\"K_1-a\"] == "
                "$.arr[0]['K_1-a'] "
                "|| length(@.K_1-a) > 0 || length(@.K_1-a) == 'str' "
                "|| match(@.n, 'a') || search(@['K_1-a'], 'nomatch') "
                "|| match(@['K_1-a'], '[]a-z].*\\\\d.*\\\\w.*\\\\W.*') "
                "|| search(@['K_1-a'], 'a$') || (@.b) || (@.b && @.n == 5))]",
                &nl));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));

        /* JSONPath remaining branches in get and query */
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_json_jsonpath_get(va, "$.empty_a..*", &fnd));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(va, "$..x", &fnd));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(va, "$.arr[:1]", &fnd));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_jsonpath_get(va, "$.arr[::2]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
                  cdd_json_jsonpath_get(va, "$.arr[1:1]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_get(va, "$.arr[0:/]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_get(va, "$.arr[0:a]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_get(va, "$.arr[0:1:/]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_get(va, "$.arr[0:1:a]", &fnd));

        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_jsonpath_query(
                      va, "$.arr[0, 1, 99, -99]['K_1-a', \"missing\"]", &nl));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_jsonpath_query(va, "$.obj.missing", &nl));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_jsonpath_query(va, "$.obj[?(@.x == 1)]", &nl));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_jsonpath_query(va, "$.obj[0, 0:1]", &nl));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_jsonpath_query(va, "$.arr['k']", &nl));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_query(va, "$.arr[0:/]", &nl));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_query(va, "$.arr[0:a]", &nl));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_query(va, "$.arr[0:1:/]", &nl));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_query(va, "$.arr[0:1:a]", &nl));

        /* Merge patch with non-object target */
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_int64(5, &vb));
        {
          JSON_Value *mp_out = NULL;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_merge_patch(vb, va, &mp_out));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(mp_out));
        }
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));

        /* 16. Precision branch hits for remaining conditions */
        /* Deep object > CDD_JSON_MAX_DEPTH in equals */
        {
          int di;
          JSON_Value *ro_a = NULL, *co_a = NULL;
          JSON_Value *ro_b = NULL, *co_b = NULL;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&ro_a));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&ro_b));
          co_a = ro_a;
          co_b = ro_b;
          for (di = 0; di < 2052; ++di) {
            JSON_Value *na = NULL, *nb = NULL;
            ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&na));
            ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&nb));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_json_object_set_value(co_a->object_val, "k", na));
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_json_object_set_value(co_b->object_val, "k", nb));
            co_a = na;
            co_b = nb;
          }
          ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(ro_a, ro_b));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(ro_a));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(ro_b));
        }

        /* Hex escapes below '0' (\u000/), between '9' and 'A' (\u000:), between
         * 'F' and 'a' (\u000G), and trailing comma in object with
         * allow_trailing_commas=1 */
        ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\u000/\"", &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\u000:\"", &va));
        ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_json_parse_string("\"\\u000G\"", &va));
        popts.allow_comments = 0;
        popts.allow_trailing_commas = 1;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string_with_options(
                                     "{\"a\":1,}", &popts, &va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));

        /* 128-bit exact boundary: u128 == u128_max_div10 with digit == 4 (<5)
         * vs digit == 6 (>5), and negative 128-bit with u128.high == 2^63 and
         * u128.low == 1 */
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_parse_string(
                      "340282366920938463463374607431768211454", &va));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_parse_string(
                      "340282366920938463463374607431768211454", &vb));
        /* Compare number with raw_num_str vs number without raw_num_str */
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_number(1.0, &vb));
        ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(va, vb));
        ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(vb, va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_parse_string(
                      "-170141183460469231731687303715884105729", &va));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));

        /* NDJSON OOM on 2nd element in cdd_json_ndjson_serialize_array */
        {
          char long_s[300];
          memset(long_s, 'X', sizeof(long_s) - 1);
          long_s[sizeof(long_s) - 1] = '\0';
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_array(&va));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_array_append_int64(va->array_val, 1));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_array_append_string(va->array_val, long_s));
          ASSERT_EQ(JSONSuccess, json_set_allocation_functions(
                                     test_custom_malloc, test_custom_free));
          g_custom_alloc_cnt = 0;
          g_custom_fail_at = 1;
          ASSERT_EQ(CDD_C_ERROR_MEMORY,
                    cdd_json_ndjson_serialize_array(va->array_val, &ser_out));
          g_custom_fail_at = -1;
          ASSERT_EQ(JSONSuccess, json_set_allocation_functions(NULL, NULL));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
        }

        /* Filter & JSONPath edge cases:
         * - `@./` (char < '0' in property name)
         * - `@[0` with char < '0' (`@[/]`)
         * - `match(@.k, 'a'x)` (missing `)` after quoted pattern)
         * - `@a` (starts with `@` but not `.` or `[`)
         * - `$a` on RHS (starts with `$` but not `.` or `[`)
         * - `$[0]` on RHS
         * - `$..*` on primitive root in jsonpath_get
         * - `$.a:` in jsonpath_get (has_start=0 and not `:`)
         * - `[?(...)]extra` in jsonpath_query
         * - union `[:, 0:1]` in jsonpath_query
         * - OOM in jsonpath_query init_array, key alloc, filter quote alloc,
         * union quote alloc
         * - OOM in pointer_remove
         */
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_parse_string(
                      "{\"arr\":[{\"k\":\" "
                      "\\t\\n\\raZ09_/:@[`{\",\"n\":1}],\"prim\":1}",
                      &va));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_jsonpath_query(
                      va,
                      "$.arr[?(match(@.k, '^\\\\s+\\\\w+\\\\W+$') || "
                      "@.k == $[0] || @.n == 1)]",
                      &nl));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_json_jsonpath_query(va, "$.arr[:1, 0:1:1]", &nl));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));

        ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
                  cdd_json_jsonpath_get(va, "$.arr[?(@. == 1)]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_get(va, "$.arr[?(@[/] == 1)]", &fnd));
        ASSERT_EQ(
            CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_json_jsonpath_get(va, "$.arr[?(match(@.k, 'a'x))]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
                  cdd_json_jsonpath_get(va, "$.arr[?(@a == 1)]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
                  cdd_json_jsonpath_get(va, "$.arr[?(@.n == $a)]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_json_jsonpath_get(va, "$.arr[0x]", &fnd));
        ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
                  cdd_json_jsonpath_query(va, "$.arr[?(@.n == 1)extra]", &nl));

        /* OOM in jsonpath_query and pointer_remove */
        {
          int qi;
          ASSERT_EQ(JSONSuccess, json_set_allocation_functions(
                                     test_custom_malloc, test_custom_free));
          for (qi = 0; qi < 12; ++qi) {
            g_custom_alloc_cnt = 0;
            g_custom_fail_at = qi;
            if (cdd_json_jsonpath_query(va, "$.arr[?(@['k'] == 'a')].k", &nl) ==
                CDD_C_SUCCESS)
              cdd_json_value_free(nl);
            g_custom_alloc_cnt = 0;
            if (cdd_json_jsonpath_query(va, "$.arr['k']", &nl) == CDD_C_SUCCESS)
              cdd_json_value_free(nl);
            g_custom_alloc_cnt = 0;
            cdd_json_pointer_remove(va, "/arr/0/k");
          }
          g_custom_fail_at = -1;
          ASSERT_EQ(JSONSuccess, json_set_allocation_functions(NULL, NULL));
        }
        ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));

        /* 17. Final 40 untaken branch completions */
        {
          JSON_Value *n1 = NULL;
          JSON_Value *n2 = NULL;
          /* 1) json_value_get_string(NULL) */
          ASSERT_EQ(NULL, json_value_get_string(NULL));
          /* 2) dotget_value where intermediate key is missing */
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_object(&va));
          ASSERT_EQ(
              CDD_C_ERROR_NOT_FOUND,
              cdd_json_object_dotget_value(va->object_val, "missing.sub", &vb));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
          /* 3) json_array_get_boolean(NULL, 0) */
          ASSERT_EQ(CDD_JSON_ERR_BOOL, json_array_get_boolean(NULL, 0));
          /* 4) memcmp != 0 on equal-length strings in cdd_json_value_equals */
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_string("abc", &n1));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_string("abd", &n2));
          ASSERT_EQ(CDD_JSON_FALSE, json_value_equals(n1, n2));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(n1));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(n2));
          /* 5) hex escape > 'f' (\u000g) */
          ASSERT_EQ(CDD_C_ERROR_PARSE,
                    cdd_json_parse_string("\"\\u000g\"", &n1));
          /* 6) 128-bit positive with u128 == u128_max_div10 and digit == 5, and
           * negative with high > max_neg_hi */
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_parse_string(
                        "340282366920938463463374607431768211455", &n1));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(n1));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_parse_string(
                        "-200000000000000000000000000000000000000", &n1));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(n1));
          /* 7) exponent followed by char < '0' (1e2,) and >5 digits (1e100000)
           */
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_parse_string("[1e2, 1e100000]", &n1));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(n1));
          /* 8) block comment containing '*' not followed by '/' */
          popts.allow_comments = 1;
          popts.allow_trailing_commas = 0;
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string_with_options(
                                       "/* a * b */{}", &popts, &n1));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(n1));
          /* 9) canonical number with e* where exponent is 2 digits vs 3 digits,
           * and e followed by neither + nor - */
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_init_number(1e-12, &n1));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_serialize_to_string_canonical(n1, &ser_out));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_free_serialized_string(ser_out));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(n1));
          /* 10) NDJSON OOM on first element */
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("[1]", &n1));
          ASSERT_EQ(JSONSuccess, json_set_allocation_functions(
                                     test_custom_malloc, test_custom_free));
          g_custom_alloc_cnt = 0;
          g_custom_fail_at = 0;
          ASSERT_EQ(CDD_C_ERROR_MEMORY,
                    cdd_json_ndjson_serialize_array(n1->array_val, &ser_out));
          g_custom_fail_at = -1;
          ASSERT_EQ(JSONSuccess, json_set_allocation_functions(NULL, NULL));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(n1));

          /* 11) I-Regexp & Filter remaining branches:
           * - `$` not at end of pattern (`$a`)
           * - `match(@.k, "pat")` with double quotes
           * - `search(@.missing, 'a')` where fval == NULL
           * - `value(@.k)` followed by `]` (`[?value(@.k)]`)
           * - RHS string with double quotes `@.k == "v"`
           * - RHS comparison where fval is wrong type (`@.n == 'str'`, `@.k ==
           * 5`, `@.n == true`)
           * - Slice in union `[0:1:, 0::]` and `[-1:1]` in jsonpath_get
           * - pointer_set with parent != NULL
           * - apply_patch where op_val has op but no path (`[{"op":"add"}]`
           * already tested, test `[{"path":"/a"}]`)
           */
          ASSERT_EQ(
              CDD_C_SUCCESS,
              cdd_json_parse_string(
                  "{\"arr\":[{\"k\":\"v\",\"n\":1}],\"obj\":{\"a\":1}}", &va));
          ASSERT_EQ(
              CDD_C_SUCCESS,
              cdd_json_jsonpath_query(
                  va,
                  "$.arr[?(match(@.k, \"^v$\") || search(@.missing, 'a') || "
                  "match(@.k, '$a') || @.k == \"v\" || @.n == 's' || "
                  "@.k == 1 || @.n == true)]",
                  &nl));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_jsonpath_query(va, "$.arr[?value(@.k)]", &nl));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_jsonpath_query(
                        va, "$.arr[0:1:, 0::, :1:1, :-1:1, :1]", &nl));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(nl));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_jsonpath_get(va, "$.arr[-1:1]", &fnd));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_json_jsonpath_get(va, "$.arr[:x]", &fnd));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_json_jsonpath_query(va, "$.arr[0:1, bad]", &nl));

          /* OOM on quoted key inside filter `@['k']` and OOM on
           * nodelist_append_sticky and merge_patch */
          ASSERT_EQ(JSONSuccess, json_set_allocation_functions(
                                     test_custom_malloc, test_custom_free));
          g_custom_alloc_cnt = 0;
          g_custom_fail_at = 0;
          ASSERT_EQ(CDD_C_ERROR_MEMORY,
                    cdd_json_jsonpath_get(va, "$.arr[?(@['k'] == 'v')]", &fnd));
          {
            int mi;
            JSON_Value *mp_patch = NULL;
            g_custom_fail_at = -1;
            ASSERT_EQ(CDD_C_SUCCESS,
                      cdd_json_parse_string(
                          "{\"k1\":1,\"k2\":2,\"k3\":3,\"k4\":4,\"k5\":5,"
                          "\"k6\":6,\"k7\":7,\"k8\":8,\"k9\":{\"s\":9}}",
                          &mp_patch));
            for (mi = 0; mi < 35; ++mi) {
              JSON_Value *mp_res = NULL;
              g_custom_alloc_cnt = 0;
              g_custom_fail_at = mi;
              if (cdd_json_merge_patch(va, mp_patch, &mp_res) == CDD_C_SUCCESS)
                cdd_json_value_free(mp_res);
              g_custom_alloc_cnt = 0;
              if (cdd_json_jsonpath_query(va, "$..*", &mp_res) == CDD_C_SUCCESS)
                cdd_json_value_free(mp_res);
            }
            g_custom_fail_at = -1;
            ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(mp_patch));
          }
          g_custom_fail_at = -1;
          ASSERT_EQ(JSONSuccess, json_set_allocation_functions(NULL, NULL));

          /* pointer_set with val->parent != NULL */
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_object_get_value(va->object_val, "obj", &vb));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_json_pointer_set(va, "/x", vb));

          /* apply_patch where op is missing (`[{"path":"/a"}]`) */
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_parse_string("[{\"path\":\"/a\"}]", &vb));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_json_apply_patch(va, vb, &n1));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));

          /* Final 7 branches:
           * - cdd_json_value_get_string(va, &cs) where va is non-string
           * - parse_string_with_options("[1,2]", &popts, &vb) with
           * allow_trailing_commas=0
           * - 340282366920938463463374607431768211450 (u128 == max_div10, digit
           * == 0)
           * - trailing backslash in jp_quoted "$['\\x00" ("$['\\")
           * - unclosed quote inside filter "$.arr[?(@['k)]"
           * - invalid filter inside jsonpath_query "$.arr[?(bad)]"
           * - unclosed quote in union selector "$.arr['k]" in jsonpath_query
           */
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_json_value_get_string(va, &cs));
          popts.allow_comments = 0;
          popts.allow_trailing_commas = 0;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_parse_string_with_options("[1,2]", &popts, &vb));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_parse_string(
                        "340282366920938463463374607431768211450", &vb));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_json_jsonpath_get(va, "$['\\", &fnd));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_json_jsonpath_get(va, "$.arr[?(@['k)]", &fnd));
          ASSERT_EQ(CDD_C_ERROR_NOT_SUPPORTED,
                    cdd_json_jsonpath_query(va, "$.arr[?(bad)]", &nl));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    cdd_json_jsonpath_query(va, "$.arr['k]", &nl));

          /* 1) [1,2,] with opts == NULL (cdd_json_parse_string)
           * 2) 340282366920938463463374607431768211450 (39 digits:
           * u128==max_div10, digit==0) 3) OOM on cdd_json_apply_patch with
           * single add op failing on 2nd allocation (v_copy)
           */
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("[1,2,]", &vb));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_json_parse_string(
                        "340282366920938463463374607431768211450", &vb));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(vb));
          {
            JSON_Value *add_patch = NULL;
            int pi;
            ASSERT_EQ(
                CDD_C_SUCCESS,
                cdd_json_parse_string(
                    "[{\"op\":\"add\",\"path\":\"/z\",\"value\":{\"x\":1}}]",
                    &add_patch));
            ASSERT_EQ(JSONSuccess, json_set_allocation_functions(
                                       test_custom_malloc, test_custom_free));
            for (pi = 0; pi < 40; ++pi) {
              g_custom_alloc_cnt = 0;
              g_custom_fail_at = pi;
              if (cdd_json_apply_patch(va, add_patch, &vb) == CDD_C_SUCCESS)
                cdd_json_value_free(vb);
            }
            g_custom_fail_at = -1;
            ASSERT_EQ(JSONSuccess, json_set_allocation_functions(NULL, NULL));
            ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(add_patch));
          }

          ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(va));
        }
      }
    }
  }

  PASS();
}

/**
 * @brief Executes the standalone_json_suite test suite.
 */
SUITE(standalone_json_suite) {
  RUN_TEST(test_standalone_json_gen);
  RUN_TEST(test_json_dom_comprehensive);
  RUN_TEST(test_json_dom_exhaustive_edges);
}

#ifdef __cplusplus
}
#endif

#endif
