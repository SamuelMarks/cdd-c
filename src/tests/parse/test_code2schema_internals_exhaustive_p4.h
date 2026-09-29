/**
 * @file test_code2schema_internals_exhaustive_p4.h
 * @brief Exhaustive branch coverage tests part 4 for code2schema.c.
 * @author Samuel Marks
 */

#ifndef TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P4_H
#define TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P4_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_c_error.h"
#include "classes/emit/struct.h"
#include "classes/parse/code2schema.h"
#include <greatest.h>
#include <parson.h>
/* clang-format on */

extern C_CDD_EXPORT volatile int g_c2s_helper_fail;
extern C_CDD_EXPORT volatile int g_cdd_fail_c2s_collect_schema_extras;
extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
extern C_CDD_EXPORT volatile int g_cdd_fail_struct_fields_get;
extern C_CDD_EXPORT volatile int g_cdd_fail_struct_fields_add;
extern C_CDD_EXPORT int g_struct_fields_init_fail;
extern C_CDD_EXPORT int g_enum_members_init_fail;
extern C_CDD_EXPORT int g_enum_members_add_strdup_fail;

/**
 * @brief Tests remaining branches in code2schema part 4.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_exhaustive_100_percent_coverage_part4(void) {
  /* 30. Exact branch coverage for apply_union_to_struct_fields_ex */
  {
    /* 1. sub == NULL in loop 2 (42 in array) */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string("[{\"type\":\"string\"}, 42]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      cdd_c_error_t rc;

      struct_fields_init(&dest);
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestSubNull", 0,
                                           NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, dest.is_union);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* 2. name_hint fallback (object with properties but no type/title) */
    {
      struct StructFields dest;
      JSON_Value *val =
          json_parse_string("[{\"properties\":{\"x\":{\"type\":\"string\"}}}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      cdd_c_error_t rc;

      struct_fields_init(&dest);
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "FallbackHint", 0,
                                           NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, dest.is_union);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* 3. default branch in switch(jtype) (boolean variant) */
    {
      struct StructFields dest;
      JSON_Value *val =
          json_parse_string("[{\"type\":\"boolean\"}, {\"type\":\"string\"}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      cdd_c_error_t rc;

      struct_fields_init(&dest);
      rc = apply_union_to_struct_fields_ex(arr, &dest, root,
                                           "TestDefaultSwitch", 0, NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, dest.is_union);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* 4. unsupported array items (!supported in loop 1) */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "[{\"type\":\"array\",\"items\":{\"type\":\"array\"}}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      cdd_c_error_t rc;

      struct_fields_init(&dest);
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnsupported",
                                           0, NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(0, dest.is_union);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }
  }

  /* 31. Direct tests for remaining missing error branches */
  {
    char *dest = NULL;
    char *out_val = NULL;
    struct StructFields sf;
    cdd_c_error_t rc;

    /* discriminator line 4228: str_after_last failure when ref is present */
    {
      JSON_Value *d_val = json_parse_string("{\"mapping\":{}}");
      JSON_Object *d_obj = json_value_get_object(d_val);
      g_cdd_fail_str_after_last = 1;
      rc = discriminator_value_for_variant(d_obj, "MySchema",
                                           "#/components/schemas/A", &out_val);
      g_cdd_fail_str_after_last = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(d_val);
    }

    /* c2s_merge_schema_extras_strings: json_set_value error */
    c_cdd_strdup("{\"a\":1}", &dest);
    g_cdd_fail_json_set_value = 1;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_cdd_fail_json_set_value = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);

    /* c2s_merge_schema_extras_strings: json_serialize error */
    c_cdd_strdup("{\"a\":1}", &dest);
    g_cdd_fail_json_serialize = 1;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_cdd_fail_json_serialize = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);

    /* c2s_json_object_to_struct_fields_internal: items extras error (line 2185)
     */
    {
      JSON_Value *v = json_parse_string("{\"type\":\"object\",\"properties\":{"
                                        "  "
                                        "\"arr\":{\"type\":\"array\",\"items\":"
                                        "{\"type\":\"string\",\"x-item\":1}}"
                                        "}}");
      JSON_Object *o = json_value_get_object(v);

      struct_fields_init(&sf);
      g_cdd_fail_c2s_collect_schema_extras = 2;
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL,
                                                     "ExtrasFail2", 0);
      g_cdd_fail_c2s_collect_schema_extras = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      struct_fields_free(&sf);
      json_value_free(v);
    }

    /* c2s_json_object_to_struct_fields_internal: prop extras error (line 2358)
     */
    {
      JSON_Value *v =
          json_parse_string("{\"type\":\"object\",\"properties\":{"
                            "  \"p\":{\"type\":\"string\",\"x-prop\":2}"
                            "}}");
      JSON_Object *o = json_value_get_object(v);

      struct_fields_init(&sf);
      g_cdd_fail_c2s_collect_schema_extras = 2;
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL,
                                                     "ExtrasFail3", 0);
      g_cdd_fail_c2s_collect_schema_extras = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      struct_fields_free(&sf);
      json_value_free(v);
    }
  }

  /* 32. 100% line completion tests */
  {
    cdd_c_error_t rc;

    /* 1. c2s_clone_json_value g_cdd_fail_json_serialize */
    {
      JSON_Value *v = json_parse_string("{\"a\":1}");
      JSON_Value *c = NULL;
      g_cdd_fail_json_serialize = 1;
      rc = c2s_clone_json_value(v, &c);
      g_cdd_fail_json_serialize = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      json_value_free(v);
    }

    /* 2. c2s_merge_schema_extras_strings json_serialize failure with empty
     * objects */
    {
      char *dest = NULL;
      c_cdd_strdup("{}", &dest);
      g_cdd_fail_json_serialize = 1;
      rc = c2s_merge_schema_extras_strings(&dest, "{}");
      g_cdd_fail_json_serialize = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      C_CDD_FREE(dest);
    }

    /* 3. copy_string_array_code2schema direct fail hook */
    {
      char *src_arr[1];
      char **out_arr = NULL;
      size_t out_cnt = 0;
      src_arr[0] = C_CDD_STR_LIT("a");
      g_c2s_helper_fail = 1;
      rc = copy_string_array_code2schema(&out_arr, &out_cnt, src_arr, 1);
      g_c2s_helper_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }

    /* 4. parse_type_union_array_code2schema direct fail hook */
    {
      JSON_Value *v = json_parse_string("[\"string\"]");
      JSON_Array *arr = json_value_get_array(v);
      char **out_arr = NULL;
      size_t out_cnt = 0;
      const char *primary = NULL;
      g_c2s_helper_fail = 1;
      rc = parse_type_union_array_code2schema(arr, &out_arr, &out_cnt, &primary,
                                              NULL);
      g_c2s_helper_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      json_value_free(v);
    }

    /* 5. required_name_in_list direct and internal failure */
    {
      JSON_Value *av = json_parse_string("[\"fld\"]");
      JSON_Array *arr = json_value_get_array(av);
      int in_list = 0;
      g_c2s_helper_fail = 1;
      rc = required_name_in_list(arr, "fld", &in_list);
      g_c2s_helper_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      json_value_free(av);

      /* internal percolation in c2s_json_object_to_struct_fields_internal (Line
       * 2261) */
      {
        struct StructFields sf;
        JSON_Value *v = json_parse_string(
            "{\"type\":\"object\",\"required\":[\"fld\"],\"properties\":{"
            "\"fld\":{\"type\":\"string\"} } }");
        JSON_Object *o = json_value_get_object(v);
        int k;

        for (k = 1; k <= 5; ++k) {
          struct_fields_init(&sf);
          g_c2s_helper_fail = k;
          rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL,
                                                         "ReqFail", 0);
          g_c2s_helper_fail = 0;
          struct_fields_free(&sf);
        }
        json_value_free(v);
      }
    }

    /* 6. merge_struct_fields struct_fields_get call 2 (Line 4483) */
    {
      struct StructFields src, dest;
      struct_fields_init(&src);
      struct_fields_init(&dest);
      struct_fields_add(&src, "fld1", "string", NULL, NULL, NULL);
      g_cdd_fail_struct_fields_get = 2;
      rc = merge_struct_fields(&dest, &src);
      g_cdd_fail_struct_fields_get = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&src);
      struct_fields_free(&dest);
    }

    /* 7. merge_struct_fields items_type_union copy failure (Line 4521) */
    {
      struct StructFields src, dest;
      char *arr_types[2];
      arr_types[0] = C_CDD_STR_LIT("string");
      arr_types[1] = C_CDD_STR_LIT("null");
      struct_fields_init(&src);
      struct_fields_init(&dest);
      struct_fields_add(&src, "fld2", "array", "string", NULL, NULL);
      copy_string_array_code2schema(&src.fields[0].items_type_union,
                                    &src.fields[0].n_items_type_union,
                                    arr_types, 2);
      g_c2s_helper_fail = 2;
      rc = merge_struct_fields(&dest, &src);
      g_c2s_helper_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      struct_fields_free(&src);
      struct_fields_free(&dest);
    }

    /* 8. apply_allof_to_struct_fields merge failure (Line 4764) */
    {
      struct StructFields sf;
      JSON_Value *allof_val =
          json_parse_string("[{\"type\":\"object\",\"properties\":{\"f1\":{"
                            "\"type\":\"string\"}}}]");
      JSON_Array *allof_arr = json_value_get_array(allof_val);
      struct_fields_init(&sf);
      g_cdd_fail_struct_fields_add = 2;
      rc = apply_allof_to_struct_fields(allof_arr, &sf, NULL);
      g_cdd_fail_struct_fields_add = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);
      json_value_free(allof_val);
    }

    /* 9. apply_union_to_struct_fields_ex variant error paths */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "{\"discriminator\":{\"propertyName\":\"type\"},"
          "\"oneOf\":["
          "  {\"$ref\":\"#/components/schemas/RefA\"},"
          "  "
          "{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/schemas/"
          "EnumItem\"}},"
          "  {\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]}}"
          "]}");
      JSON_Object *obj = json_value_get_object(val);
      JSON_Array *oneof_arr = json_object_get_array(obj, "oneOf");
      JSON_Value *root_val = json_parse_string(
          "{\"RefA\":{\"type\":\"object\",\"properties\":{\"a\":{\"type\":"
          "\"string\"}}},"
          "\"EnumItem\":{\"type\":\"string\",\"enum\":[\"E1\"]}}");
      JSON_Object *root = json_value_get_object(root_val);
      int k;

      for (k = 1; k <= 15; ++k) {
        struct_fields_init(&dest);
        g_c2s_helper_fail = k;
        rc = apply_union_to_struct_fields_ex(oneof_arr, &dest, root,
                                             "UnionExErr", 0, obj, 1);
        g_c2s_helper_fail = 0;
        struct_fields_free(&dest);
      }

      for (k = 1; k <= 10; ++k) {
        struct_fields_init(&dest);
        g_cdd_fail_str_after_last = k;
        rc = apply_union_to_struct_fields_ex(oneof_arr, &dest, root,
                                             "UnionExErr", 0, obj, 1);
        g_cdd_fail_str_after_last = 0;
        struct_fields_free(&dest);
      }

      json_value_free(val);
      json_value_free(root_val);
    }
  }

  /* 33. Comprehensive branch coverage for parameter validations and C parsing
   */
  {
    cdd_c_error_t rc;
    char *src_arr[2];
    char **out_arr = NULL;
    size_t out_cnt = 0;
    struct StructFields sf;

    src_arr[0] = C_CDD_STR_LIT("val1");
    src_arr[1] = NULL;

    /* copy_string_array_code2schema branch permutations */
    rc = copy_string_array_code2schema(NULL, &out_cnt, src_arr, 1);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = copy_string_array_code2schema(&out_arr, NULL, src_arr, 1);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = copy_string_array_code2schema(&out_arr, &out_cnt, NULL, 1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = copy_string_array_code2schema(&out_arr, &out_cnt, src_arr, 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = copy_string_array_code2schema(&out_arr, &out_cnt, src_arr, 2);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free_string_array_code2schema(out_arr, out_cnt);

    /* parse_type_union_array_code2schema branch permutations */
    {
      JSON_Value *v = json_parse_string("[\"null\"]");
      JSON_Array *arr = json_value_get_array(v);
      int is_null = 0;
      rc = parse_type_union_array_code2schema(arr, &out_arr, &out_cnt, NULL,
                                              &is_null);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, is_null);
      free_string_array_code2schema(out_arr, out_cnt);
      json_value_free(v);
    }

    /* parse_struct_member_line branch permutations */
    struct_fields_init(&sf);
    rc = parse_struct_member_line(NULL, &sf);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = parse_struct_member_line("int a;", NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* int* p without space before pointer */
    rc = parse_struct_member_line("int* p;", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Flexible array member */
    rc = parse_struct_member_line("int fam[];", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Flexible array member of strings */
    rc = parse_struct_member_line("char fam_str[][32];", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Shard key, hash, track telemetry, and slow query */
    rc = parse_struct_member_line("int id; // @shard_key @shard_hash "
                                  "@track_telemetry @slow_query_warn(100)",
                                  &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&sf);

    /* c2s_collect_schema_extras NULL checks */
    {
      char *out_j = NULL;
      JSON_Value *v = json_parse_string("{\"a\":1}");
      JSON_Object *o = json_value_get_object(v);
      rc = c2s_collect_schema_extras(NULL, NULL, 0, &out_j);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      rc = c2s_collect_schema_extras(o, NULL, 0, NULL);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      json_value_free(v);
    }

    /* c2s_json_object_to_struct_fields_internal NULL checks */
    struct_fields_init(&sf);
    rc = c2s_json_object_to_struct_fields_internal(NULL, &sf, NULL, "A", 0);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    {
      JSON_Value *v = json_parse_string("{\"type\":\"object\"}");
      JSON_Object *o = json_value_get_object(v);
      rc = c2s_json_object_to_struct_fields_internal(o, NULL, NULL, "A", 0);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      json_value_free(v);
    }
    struct_fields_free(&sf);
  }

  /* 34. Exhaustive branch coverage for remaining untaken conditions */
  {
    cdd_c_error_t rc;

    /* str_starts_with with NULL _out_val */
    rc = str_starts_with("abc", "a", NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* parse_type_union_array_code2schema with NULL out pointers */
    {
      JSON_Value *v = json_parse_string("[\"string\", \"null\"]");
      JSON_Array *arr = json_value_get_array(v);
      rc = parse_type_union_array_code2schema(arr, NULL, NULL, NULL, NULL);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      json_value_free(v);
    }

    /* c2s_merge_schema_extras_object with existing key collision */
    {
      JSON_Value *val = json_parse_string("{\"x-existing\":1}");
      JSON_Object *target = json_value_get_object(val);
      rc = c2s_merge_schema_extras_object(target,
                                          "{\"x-existing\":2,\"x-new\":3}");
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      json_value_free(val);
    }

    /* c2s_merge_schema_extras_strings with empty src_json */
    {
      char *dest = NULL;
      c_cdd_strdup("{}", &dest);
      rc = c2s_merge_schema_extras_strings(&dest, "");
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      C_CDD_FREE(dest);
    }

    /* parse_struct_member_line variations */
    {
      struct StructFields sf;
      struct_fields_init(&sf);

      /* Bit-width with spaces after colon */
      rc = parse_struct_member_line("int width :   5;", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      /* Array of objects */
      rc = parse_struct_member_line("struct SubObj items[5];", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      /* Array of primitives without format */
      rc = parse_struct_member_line("int plain_items[5];", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      /* Individual annotations */
      rc = parse_struct_member_line("int a; // @shard_key", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = parse_struct_member_line("int b; // @shard_hash", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = parse_struct_member_line("int c; // @track_telemetry", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = parse_struct_member_line("int d; // @slow_query_warn(50)", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      /* FAM variations */
      rc = parse_struct_member_line("int fam_int[];", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = parse_struct_member_line("char fam_char[];", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = parse_struct_member_line("struct SubObj fam_obj[];", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&sf);
    }

    /* c2s_json_object_to_struct_fields_internal root == NULL variations */
    {
      struct StructFields sf;
      JSON_Value *v = json_parse_string(
          "{\"type\":\"object\",\"properties\":{"
          "  "
          "\"arr_ref\":{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/"
          "schemas/Item\"}},"
          "  "
          "\"arr_anon\":{\"type\":\"array\",\"items\":{\"properties\":{\"sub\":"
          "{\"type\":\"string\"}}}},"
          "  \"dir_ref\":{\"$ref\":\"#/components/schemas/Item\"},"
          "  \"int_prop\":{\"type\":\"integer\",\"minimum\":1}"
          "}}");
      JSON_Object *o = json_value_get_object(v);

      struct_fields_init(&sf);
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL,
                                                     "NullRootTest", 0);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);

      json_value_free(v);
    }
  }

  PASS();
}
/**
 * @brief Exhaustive branch testing for merge_struct_field.
 */
TEST test_code2schema_merge_struct_field_exhaustive(void) {
  struct StructField dest, src;
  cdd_c_error_t rc;

  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));

  /* Invalid args */
  rc = merge_struct_field(NULL, &src);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = merge_struct_field(&dest, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Default val, desc, format, pattern when dest empty vs dest non-empty */
  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));
  CDD_STRCPY(src.default_val, sizeof(src.default_val), "def1");
  CDD_STRCPY(src.pattern, sizeof(src.pattern), "pat1");
  CDD_STRCPY(src.bit_width, sizeof(src.bit_width), "4");
  src.required = 1;
  src.unique_items = 1;
  src.is_flexible_array = 1;

  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("def1", dest.default_val);
  ASSERT_STR_EQ("pat1", dest.pattern);
  ASSERT_STR_EQ("4", dest.bit_width);
  ASSERT_EQ(1, dest.required);
  ASSERT_EQ(1, dest.unique_items);
  ASSERT_EQ(1, dest.is_flexible_array);

  /* Already non-empty dest (should not overwrite) */
  CDD_STRCPY(src.default_val, sizeof(src.default_val), "def2");
  CDD_STRCPY(src.pattern, sizeof(src.pattern), "pat2");
  CDD_STRCPY(src.bit_width, sizeof(src.bit_width), "8");
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("def1", dest.default_val);
  ASSERT_STR_EQ("pat1", dest.pattern);
  ASSERT_STR_EQ("4", dest.bit_width);

  /* Min / Max comparisons */
  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));
  dest.has_min = 1;
  dest.min_val = 10.0;
  dest.exclusive_min = 0;
  src.has_min = 1;
  src.min_val = 5.0; /* smaller min, not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(10.0, dest.min_val);

  src.min_val = 20.0; /* larger min, taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(20.0, dest.min_val);

  /* equal min, exclusive takes precedence */
  dest.has_min = 1;
  dest.min_val = 20.0;
  dest.exclusive_min = 0;
  src.has_min = 1;
  src.min_val = 20.0;
  src.exclusive_min = 1;
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(1, dest.exclusive_min);

  /* equal min, non-exclusive does not overwrite exclusive */
  src.exclusive_min = 0;
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(1, dest.exclusive_min);

  /* Max comparisons */
  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));
  dest.has_max = 1;
  dest.max_val = 50.0;
  dest.exclusive_max = 0;
  src.has_max = 1;
  src.max_val = 100.0; /* larger max, not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(50.0, dest.max_val);

  src.max_val = 25.0; /* smaller max, taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(25.0, dest.max_val);

  /* equal max, exclusive takes precedence */
  dest.has_max = 1;
  dest.max_val = 25.0;
  dest.exclusive_max = 0;
  src.has_max = 1;
  src.max_val = 25.0;
  src.exclusive_max = 1;
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(1, dest.exclusive_max);

  src.exclusive_max = 0;
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(1, dest.exclusive_max);

  /* Length comparisons */
  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));
  dest.has_min_len = 1;
  dest.min_len = 5;
  src.has_min_len = 1;
  src.min_len = 2; /* not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(5, dest.min_len);
  src.min_len = 10; /* taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(10, dest.min_len);

  dest.has_max_len = 1;
  dest.max_len = 20;
  src.has_max_len = 1;
  src.max_len = 50; /* not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(20, dest.max_len);
  src.max_len = 15; /* taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(15, dest.max_len);

  /* Items comparisons */
  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));
  dest.has_min_items = 1;
  dest.min_items = 3;
  src.has_min_items = 1;
  src.min_items = 1; /* not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(3, dest.min_items);
  src.min_items = 6; /* taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(6, dest.min_items);

  dest.has_max_items = 1;
  dest.max_items = 12;
  src.has_max_items = 1;
  src.max_items = 20; /* not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(12, dest.max_items);
  src.max_items = 8; /* taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(8, dest.max_items);

  /* Test merge_struct_field schema_extra_json, items_extra_json, type_union
   * failure branches */
  {
    memset(&dest, 0, sizeof(dest));
    memset(&src, 0, sizeof(src));
    src.schema_extra_json = (char *)(size_t) "{bad_json";
    dest.schema_extra_json = (char *)(size_t) "{bad_json_dest";
    rc = merge_struct_field(&dest, &src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    dest.schema_extra_json = NULL;
    src.schema_extra_json = NULL;
    dest.items_extra_json = (char *)(size_t) "{\"a\":1}";
    src.items_extra_json = (char *)(size_t) "{\"b\":2}";
    g_cdd_fail_json_serialize = 1;
    rc = merge_struct_field(&dest, &src);
    g_cdd_fail_json_serialize = 0;
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    dest.items_extra_json = NULL;
    src.items_extra_json = NULL;

    /* type_union copy failure via OOM */
    {
      char *u[] = {(char *)(size_t) "str"};
      src.type_union = u;
      src.n_type_union = 1;
      dest.type_union = NULL;
      dest.n_type_union = 0;
      g_cdd_alloc_fail = 1;
      rc = merge_struct_field(&dest, &src);
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      src.type_union = NULL;
      src.n_type_union = 0;
    }

    /* items_type_union copy failure via OOM */
    {
      char *iu[] = {(char *)(size_t) "str"};
      src.items_type_union = iu;
      src.n_items_type_union = 1;
      dest.items_type_union = NULL;
      dest.n_items_type_union = 0;
      g_cdd_alloc_fail = 1;
      rc = merge_struct_field(&dest, &src);
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      src.items_type_union = NULL;
      src.n_items_type_union = 0;
    }
  }

  PASS();
}
SUITE(code2schema_internals_exhaustive_p4_suite) {
  RUN_TEST(test_code2schema_exhaustive_100_percent_coverage_part4);
  RUN_TEST(test_code2schema_merge_struct_field_exhaustive);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P4_H */
