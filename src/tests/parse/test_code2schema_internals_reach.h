/**
 * @file test_code2schema_internals_reach.h
 * @brief Targeted branch coverage reach tests for code2schema.c.
 * @author Samuel Marks
 */

#ifndef TEST_CODE2SCHEMA_INTERNALS_REACH_H
#define TEST_CODE2SCHEMA_INTERNALS_REACH_H

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

/** @brief Test definition g_cdd_fail_json_set_value. */
extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
/** @brief Test definition g_cdd_fail_json_serialize. */
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;

/**
 * @brief Targeted tests to reach 100% branch coverage in code2schema.c.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_reach_100_percent_coverage(void) {
  /* 1. copy_string_array_code2schema OOM */
  {
    char **dst = NULL;
    size_t dst_count = 0;
    char *src_arr[1];
    src_arr[0] = cdd_test_mut_str("item");
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_string_array_code2schema(&dst, &dst_count, src_arr, 1));
    g_cdd_last_alloc_err = free_string_array_code2schema(dst, dst_count);
    dst = NULL;
    dst_count = 0;
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              copy_string_array_code2schema(&dst, &dst_count, src_arr, 1));
    g_cdd_alloc_fail = 0;
  }

  /* 2. parse_type_union_array_code2schema */
  {
    JSON_Value *arr_v = NULL;
    char **u_types = NULL;
    size_t n_u = 0;
    const char *prim = NULL;
    int nullable = 0;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("[\"string\", \"integer\"]", &_cdd_jv_1);
      arr_v = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS, parse_type_union_array_code2schema(
                                 json_value_get_array(arr_v), &u_types, &n_u,
                                 &prim, &nullable));
    g_cdd_last_alloc_err = free_string_array_code2schema(u_types, n_u);
    g_cdd_last_alloc_err = json_value_free(arr_v);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string("[\"null\"]", &_cdd_jv_1);
      arr_v = _cdd_jv_1;
    }
    u_types = NULL;
    n_u = 0;
    prim = NULL;
    nullable = 0;
    ASSERT_EQ(CDD_C_SUCCESS, parse_type_union_array_code2schema(
                                 json_value_get_array(arr_v), &u_types, &n_u,
                                 &prim, &nullable));
    g_cdd_last_alloc_err = free_string_array_code2schema(u_types, n_u);
    g_cdd_last_alloc_err = json_value_free(arr_v);
  }

  /* 3. g_cdd_fail_json_set_value in c2s_merge_schema_extras_strings */
  {
    char *extras = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, c_cdd_strdup("{\"orig\":1}", &extras));
    g_cdd_fail_json_set_value = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              c2s_merge_schema_extras_strings(&extras, "{\"a\":1}"));
    g_cdd_fail_json_set_value = 0;
    C_CDD_FREE(extras);
  }

  /* 4. parse_struct_member_line bitfields */
  {
    struct StructFields sf;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int a :", &sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int b :   ", &sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int c : ;", &sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int d : (1+2)", &sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int e : MY_FLAG", &sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int f : +1", &sf));
    g_cdd_last_alloc_err = struct_fields_free(&sf);
  }

  /* 5. Pointer with * in raw type */
  {
    struct StructFields sf;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("char * *ptr;", &sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int * *pp;", &sf));
    g_cdd_last_alloc_err = struct_fields_free(&sf);
  }

  /* 6. FAM variations */
  {
    struct StructFields sf;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("char buf[];", &sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("char name[0];", &sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int items[];", &sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_struct_member_line("uint32_t counts[];", &sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_struct_member_line("/* @shard_key */ int id;", &sf));
    g_cdd_last_alloc_err = struct_fields_free(&sf);
  }

  /* 7. json_object_to_struct_fields properties */
  {
    JSON_Value *schema;
    struct StructFields sf;

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"properties\": {\"x\": {\"default\": 10}}}", &_cdd_jv_1);
      schema = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(
                                 json_value_get_object(schema), &sf, NULL));
    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(schema);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"properties\": {\"arr\": {\"type\": \"array\"}}}", &_cdd_jv_1);
      schema = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(
                                 json_value_get_object(schema), &sf, NULL));
    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(schema);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"properties\": {\"n\": {\"type\": \"number\", "
          "\"exclusiveMaximum\": false}}}",
          &_cdd_jv_1);
      schema = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(
                                 json_value_get_object(schema), &sf, NULL));
    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(schema);
  }

  /* 8. c2s_strip_quotes */
  {
    const char *out_s = NULL;
    char buf[16];
    ASSERT_EQ(CDD_C_SUCCESS, c2s_strip_quotes("\"abc\"", buf, 0, &out_s));
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_strip_quotes("\"abc", buf, sizeof(buf), &out_s));
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_strip_quotes("\"abc\"", NULL, sizeof(buf), &out_s));
  }

  /* 9. c2s_parse_bool_default and c2s_parse_number_default */
  {
    int has_val = 0;
    int has_nval = 0;
    double nval = 0;
    ASSERT_EQ(CDD_C_SUCCESS, c2s_parse_bool_default("true", NULL, &has_val));
    ASSERT_EQ(CDD_C_SUCCESS, c2s_parse_number_default("123", NULL, &has_nval));
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_parse_number_default("3.14", &nval, &has_nval));
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_parse_number_default("not_a_num", &nval, &has_nval));
  }

  /* c2s_collapse_arrays loop exit */
  {
    struct StructFields sf_coll;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_coll));
    ASSERT_EQ(0, struct_fields_add(&sf_coll, "n_foo", "integer", "", NULL, ""));
    ASSERT_EQ(0, struct_fields_add(&sf_coll, "bar", "string", "", NULL, ""));
    ASSERT_EQ(CDD_C_SUCCESS, c2s_collapse_arrays(&sf_coll));
    g_cdd_last_alloc_err = struct_fields_free(&sf_coll);
  }

  /* 10. ref_points_to_string_enum */
  {
    JSON_Value *root_v = NULL;
    int pts = 0;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root_v = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS, ref_points_to_string_enum(
                                 json_value_get_object(root_v), NULL, &pts));
    ASSERT_EQ(CDD_C_SUCCESS,
              ref_points_to_string_enum(json_value_get_object(root_v),
                                        "#/components/", &pts));
    g_cdd_last_alloc_err = json_value_free(root_v);
  }

  /* 11. required_name_in_list */
  {
    JSON_Value *req_v = NULL;
    int in_l = 0;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string("[\"a\", 123]", &_cdd_jv_1);
      req_v = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              required_name_in_list(json_value_get_array(req_v), NULL, &in_l));
    ASSERT_EQ(CDD_C_SUCCESS,
              required_name_in_list(json_value_get_array(req_v), "b", &in_l));
    g_cdd_last_alloc_err = json_value_free(req_v);
  }

  /* 12. resolve_schema_ref_object */
  {
    JSON_Value *root_v = NULL;
    const JSON_Object *out_obj = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root_v = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              resolve_schema_ref_object(json_value_get_object(root_v), NULL,
                                        &out_obj));
    ASSERT_EQ(CDD_C_SUCCESS,
              resolve_schema_ref_object(json_value_get_object(root_v),
                                        "#/components/", &out_obj));
    g_cdd_last_alloc_err = json_value_free(root_v);
  }

  /* 13. c2s_collect_string_array with non-string */
  {
    JSON_Value *arr_v = NULL;
    char **arr_out = NULL;
    size_t cnt = 0;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("[123, \"valid\"]", &_cdd_jv_1);
      arr_v = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS, c2s_collect_string_array(
                                 json_value_get_array(arr_v), &arr_out, &cnt));
    g_cdd_last_alloc_err = free_string_array_code2schema(arr_out, cnt);
    g_cdd_last_alloc_err = json_value_free(arr_v);
  }

  /* 14. sanitize_identifier with char > 'z' */
  {
    char *san = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, sanitize_identifier("hello{world|test}~", &san));
    C_CDD_FREE(san);
  }

  /* 15. make_inline_schema_name empty strings */
  {
    char *iname = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, make_inline_schema_name("", "", "", &iname));
    C_CDD_FREE(iname);
  }

  /* 16. discriminator_value_for_variant non-string value */
  {
    JSON_Value *sch = NULL;
    char *disc_val = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"mapping\": {\"key\": 123}}", &_cdd_jv_1);
      sch = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              discriminator_value_for_variant(json_value_get_object(sch),
                                              "schema", "ref", &disc_val));
    C_CDD_FREE(disc_val);
    g_cdd_last_alloc_err = json_value_free(sch);
  }

  /* 17. merge_struct_field exclusive bounds */
  {
    struct StructField dest, src;
    struct StructFields dest_sfs, src_sfs;
    memset(&dest, 0, sizeof(dest));
    memset(&src, 0, sizeof(src));
    memset(&dest_sfs, 0, sizeof(dest_sfs));
    memset(&src_sfs, 0, sizeof(src_sfs));
    dest.has_min = 1;
    dest.min_val = 5;
    dest.exclusive_min = 1;
    src.has_min = 1;
    src.min_val = 5;
    src.exclusive_min = 1;
    dest.has_max = 1;
    dest.max_val = 10;
    dest.exclusive_max = 1;
    src.has_max = 1;
    src.max_val = 10;
    src.exclusive_max = 1;
    ASSERT_EQ(CDD_C_SUCCESS, merge_struct_field(&dest, &src));
    ASSERT_EQ(CDD_C_SUCCESS, merge_struct_fields(&dest_sfs, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, merge_struct_fields(NULL, &src_sfs));
  }

  /* 18. apply_allof_to_struct_fields & fallback */
  {
    JSON_Value *allof_v = NULL;
    JSON_Value *allof_ref = NULL;
    struct StructFields sf;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string("[{}]", &_cdd_jv_1);
      allof_v = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "[{\"$ref\": \"#/components/schemas/Foo\"}]", &_cdd_jv_1);
      allof_ref = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS, apply_allof_to_struct_fields(
                                 json_value_get_array(allof_v), NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_allof_to_struct_fields(
                                 json_value_get_array(allof_ref), &sf, NULL));
    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(allof_ref);
    g_cdd_last_alloc_err = json_value_free(allof_v);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string("[{}]", &_cdd_jv_1);
      allof_v = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "[{\"$ref\": \"#/components/schemas/Foo\"}]", &_cdd_jv_1);
      allof_ref = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_fallback(
                                 json_value_get_array(allof_v), NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_fallback(
                                 json_value_get_array(allof_ref), &sf, NULL));
    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(allof_ref);
    g_cdd_last_alloc_err = json_value_free(allof_v);
  }

  /* 19. apply_union_to_struct_fields_ex branches */
  {
    JSON_Value *u_v = NULL;
    JSON_Value *u_ref = NULL;
    JSON_Value *sch_disc = NULL;
    JSON_Value *sch_disc_empty = NULL;
    JSON_Value *u_no_type = NULL;
    JSON_Value *arr_no_items = NULL;
    JSON_Value *arr_items_ref = NULL;
    JSON_Value *arr_items_obj = NULL;
    JSON_Value *arr_inline_obj = NULL;
    JSON_Value *u_obj = NULL;
    JSON_Value *root_val = NULL;
    struct StructFields sf;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("[{\"type\": \"string\"}]", &_cdd_jv_1);
      u_v = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "[{\"$ref\": \"#/components/schemas/Foo\"}]", &_cdd_jv_1);
      u_ref = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"discriminator\": {\"propertyName\": \"\"}}", &_cdd_jv_1);
      sch_disc = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("{\"discriminator\": {}}", &_cdd_jv_1);
      sch_disc_empty = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("[{\"description\": \"desc\"}]", &_cdd_jv_1);
      u_no_type = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("[{\"type\": \"array\"}]", &_cdd_jv_1);
      arr_no_items = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "[{\"type\": \"array\", \"items\": {\"$ref\": \"#/Foo\"}}]",
          &_cdd_jv_1);
      arr_items_ref = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "[{\"type\": \"array\", \"items\": {\"type\": \"object\"}}]",
          &_cdd_jv_1);
      arr_items_obj = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "[{\"type\": \"array\", \"items\": {\"type\": \"object\", "
          "\"properties\": {\"x\": {\"type\": \"integer\"}}}}]",
          &_cdd_jv_1);
      arr_inline_obj = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "[{\"type\": \"object\", \"properties\": {\"a\": {\"type\": "
          "\"string\"}}}]",
          &_cdd_jv_1);
      u_obj = _cdd_jv_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"components\": {\"schemas\": {\"Foo\": {\"type\": \"string\"}}}}",
          &_cdd_jv_1);
      root_val = _cdd_jv_1;
    }

    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_v), NULL,
                                              NULL, NULL, 0, NULL, 0));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_ref), &sf,
                                              NULL, NULL, 0, NULL, 0));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_ref), &sf,
                                              json_value_get_object(root_val),
                                              NULL, 0, NULL, 0));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(
                                 json_value_get_array(u_v), &sf, NULL, NULL, 0,
                                 json_value_get_object(sch_disc), 0));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(
                                 json_value_get_array(u_v), &sf, NULL, NULL, 0,
                                 json_value_get_object(sch_disc_empty), 0));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_no_type),
                                              &sf, NULL, NULL, 0, NULL, 0));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_obj), &sf,
                                              NULL, NULL, 0, NULL, 1));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_obj), &sf,
                                              json_value_get_object(root_val),
                                              NULL, 0, NULL, 1));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_obj), &sf,
                                              json_value_get_object(root_val),
                                              NULL, 0, NULL, 0));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(
                                 json_value_get_array(arr_no_items), &sf, NULL,
                                 NULL, 0, NULL, 1));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(
                                 json_value_get_array(arr_items_obj), &sf, NULL,
                                 NULL, 0, NULL, 1));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(
                                 json_value_get_array(arr_items_ref), &sf, NULL,
                                 NULL, 0, NULL, 1));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(
                  json_value_get_array(arr_inline_obj), &sf,
                  json_value_get_object(root_val), NULL, 0, NULL, 1));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(
                  json_value_get_array(arr_inline_obj), &sf,
                  json_value_get_object(root_val), NULL, 0, NULL, 0));
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    g_cdd_last_alloc_err = json_value_free(root_val);
    g_cdd_last_alloc_err = json_value_free(u_obj);
    g_cdd_last_alloc_err = json_value_free(arr_inline_obj);
    g_cdd_last_alloc_err = json_value_free(arr_items_obj);
    g_cdd_last_alloc_err = json_value_free(arr_items_ref);
    g_cdd_last_alloc_err = json_value_free(arr_no_items);
    g_cdd_last_alloc_err = json_value_free(u_no_type);
    g_cdd_last_alloc_err = json_value_free(sch_disc_empty);
    g_cdd_last_alloc_err = json_value_free(sch_disc);
    g_cdd_last_alloc_err = json_value_free(u_ref);
    g_cdd_last_alloc_err = json_value_free(u_v);
  }

  /* 20. default value and constraint writers */
  {
    JSON_Value *pobj_v = NULL;
    JSON_Object *pobj = NULL;
    struct StructField f;
    char *tun[1];
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      pobj_v = _cdd_jv_1;
    }
    pobj = json_value_get_object(pobj_v);
    tun[0] = NULL;
    memset(&f, 0, sizeof(f));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_write_default_value(pobj, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_write_numeric_constraints(pobj, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_write_string_constraints(pobj, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_write_array_constraints(pobj, NULL));

    ASSERT_EQ(CDD_C_SUCCESS, c2s_write_default_value(pobj, &f));
    ASSERT_EQ(CDD_C_SUCCESS, c2s_write_numeric_constraints(pobj, &f));
    ASSERT_EQ(CDD_C_SUCCESS, c2s_write_string_constraints(pobj, &f));
    ASSERT_EQ(CDD_C_SUCCESS, c2s_write_array_constraints(pobj, &f));

    CDD_STRCPY(f.type, sizeof(f.type), "boolean");
    CDD_STRCPY(f.default_val, sizeof(f.default_val), "invalid");
    ASSERT_EQ(CDD_C_SUCCESS, c2s_write_default_value(pobj, &f));

    CDD_STRCPY(f.type, sizeof(f.type), "integer");
    CDD_STRCPY(f.default_val, sizeof(f.default_val), "invalid");
    ASSERT_EQ(CDD_C_SUCCESS, c2s_write_default_value(pobj, &f));

    ASSERT_EQ(CDD_C_SUCCESS, c2s_write_type_union(pobj, "string", tun, 1));
    g_cdd_last_alloc_err = json_value_free(pobj_v);
  }

  /* 21. write_struct_to_json_schema invalid and primitive array ref */
  {
    JSON_Value *sch_v = NULL;
    struct StructFields sf;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      sch_v = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        write_struct_to_json_schema(json_value_get_object(sch_v), NULL, &sf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              write_struct_to_json_schema(json_value_get_object(sch_v), "Test",
                                          NULL));

    ASSERT_EQ(0, struct_fields_add(&sf, "arr_b", "array", "boolean", NULL, ""));
    ASSERT_EQ(0, struct_fields_add(&sf, "arr_n", "array", "number", NULL, ""));
    ASSERT_EQ(0, struct_fields_add(&sf, "arr_e", "array", "", NULL, ""));
    sf.fields[0].required = 1;
    sf.fields[1].required = 1;
    ASSERT_EQ(CDD_C_SUCCESS, write_struct_to_json_schema(
                                 json_value_get_object(sch_v), "Test", &sf));
    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(sch_v);
  }

  /* 22. c2s_parse_union_and_write invalid arguments and non-field line */
  {
    FILE *fp;
    JSON_Value *sch_v;
    fp = fopen("cdd_test_tmp_union.txt", "w");
    fputs("int a;\n}\n", fp);
    fclose(fp);
    fp = fopen("cdd_test_tmp_union.txt", "r");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_parse_union_and_write(fp, NULL, "U"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_parse_union_and_write(fp, (JSON_Object *)1, NULL));
    fclose(fp);

    fp = fopen("cdd_test_tmp_union.txt", "w");
    fputs("single_word\nint a;\n}\n", fp);
    fclose(fp);
    fp = fopen("cdd_test_tmp_union.txt", "r");
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      sch_v = _cdd_jv_1;
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_parse_union_and_write(fp, json_value_get_object(sch_v), "U"));
    fclose(fp);
    g_cdd_last_alloc_err = json_value_free(sch_v);
    remove("cdd_test_tmp_union.txt");
  }

  /* 23. code2schema_main invalid args and header parsing */
  {
    FILE *fhdr;
    char *av1[2];
    char *av2[2];
    char *argv_full[2];

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, code2schema_main(2, NULL));
    av1[0] = NULL;
    av1[1] = cdd_test_mut_str("out.json");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, code2schema_main(2, av1));

    av2[0] = cdd_test_mut_str("in.h");
    av2[1] = NULL;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, code2schema_main(2, av2));

    fhdr = fopen("cdd_test_tmp_hdr.h", "w");
    fputs("/* comments */\n", fhdr);
    fputs("union NoBrace;\n", fhdr);
    fputs("union {\n  int a;\n};\n", fhdr);
    fputs("union ValidUnion {\n  int a;\n};\n", fhdr);
    fputs("struct NoBrace;\n", fhdr);
    fputs("struct {\n  int a;\n};\n", fhdr);
    fputs("struct StructWithEmptyLines {\n\n  int a;\n\n};\n", fhdr);
    fputs("struct StructWithNestedNoSemi {\n  struct {\n\n    int x;\n\n  } "
          "nested\n  int y;\n};\n",
          fhdr);
    fputs("struct StructWithNestedCompact {\n  struct{\n    int x;\n  } n2;\n  "
          "int req1;\n  int req2;\n};\n",
          fhdr);
    fputs("enum NoBrace;\n", fhdr);
    fputs("enum EnumWithTrailingComma {\n  VAL_A = 1,\n  ,\n  VAL_B,\n};\n",
          fhdr);
    fclose(fhdr);

    argv_full[0] = cdd_test_mut_str("cdd_test_tmp_hdr.h");
    argv_full[1] = cdd_test_mut_str("cdd_test_tmp_out.json");
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, argv_full));

    remove("cdd_test_tmp_hdr.h");
    remove("cdd_test_tmp_out.json");
  }

  /* 27. name_hint fallback with schema_name NULL and object variant */
  {
    JSON_Value *val = NULL;
    JSON_Array *arr = NULL;
    JSON_Value *root_val = NULL;
    JSON_Object *root = NULL;
    struct StructFields dest;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "[{\"properties\":{\"x\":{\"type\":\"string\"}}}]", &_cdd_jv_1);
      val = _cdd_jv_1;
    }
    arr = json_value_get_array(val);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root_val = _cdd_jv_1;
    }
    root = json_value_get_object(root_val);
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&dest));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(arr, &dest, root,
                                                             NULL, 0, NULL, 1));
    g_cdd_last_alloc_err = struct_fields_free(&dest);
    g_cdd_last_alloc_err = json_value_free(root_val);
    g_cdd_last_alloc_err = json_value_free(val);
  }

  /* 29. unclosed nested struct in code2schema_main */
  {
    FILE *fhdr = fopen("cdd_test_tmp_unclosed.h", "w");
    char *av_unclosed[2];
    fputs("struct Unclosed {\n  struct {\n    int x;\n", fhdr);
    fclose(fhdr);
    av_unclosed[0] = cdd_test_mut_str("cdd_test_tmp_unclosed.h");
    av_unclosed[1] = cdd_test_mut_str("cdd_test_tmp_out.json");
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av_unclosed));
    remove("cdd_test_tmp_unclosed.h");
    remove("cdd_test_tmp_out.json");
  }

  /* 30. struct_fields_init failure inside nested struct */
  {
    FILE *fhdr = fopen("cdd_test_tmp_nested_fail.h", "w");
    char *av_nested[2];
    fputs("struct S {\n  struct {\n    int x;\n  } n;\n};\n", fhdr);
    fclose(fhdr);
    av_nested[0] = cdd_test_mut_str("cdd_test_tmp_nested_fail.h");
    av_nested[1] = cdd_test_mut_str("cdd_test_tmp_out.json");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, code2schema_main(2, av_nested));
    g_cdd_alloc_fail = 5;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, code2schema_main(2, av_nested));
    g_cdd_alloc_fail = 0;
    remove("cdd_test_tmp_nested_fail.h");
    remove("cdd_test_tmp_out.json");
  }

  /* 31. struct_fields_init failure on top-level struct in code2schema_main */
  {
    FILE *fhdr = fopen("cdd_test_tmp_struct_fail.h", "w");
    char *av[2];
    fputs("struct S {\n  int a;\n};\n", fhdr);
    fclose(fhdr);
    av[0] = cdd_test_mut_str("cdd_test_tmp_struct_fail.h");
    av[1] = cdd_test_mut_str("cdd_test_tmp_out.json");
    g_cdd_alloc_fail = 4;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, code2schema_main(2, av));
    g_cdd_alloc_fail = 0;
    remove("cdd_test_tmp_struct_fail.h");
    remove("cdd_test_tmp_out.json");
  }

  /* 32. enum in code2schema_main */
  {
    FILE *fhdr = fopen("cdd_test_tmp_enum_fail.h", "w");
    char *av[2];
    fputs("enum E {\n  A = 1,\n  B\n};\n", fhdr);
    fclose(fhdr);
    av[0] = cdd_test_mut_str("cdd_test_tmp_enum_fail.h");
    av[1] = cdd_test_mut_str("cdd_test_tmp_out.json");
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av));
    remove("cdd_test_tmp_enum_fail.h");
    remove("cdd_test_tmp_out.json");
  }

  /* 33. Trailing comments, anonymous blocks, unclosed blocks, and nested OOM */
  {
    FILE *fhdr = fopen("cdd_test_tmp_edges.h", "w");
    char *av[2];
    fputs("union;\nstruct;\nenum;\n"
          "union { struct { int x; } s; };\n"
          "struct { int x; };\n"
          "enum { A };\n"
          "enum E { 123, A = 1 };\n"
          "struct S {\n"
          "  ;\n"
          "  int a; /** @brief inline doc */\n"
          "  int b;\n"
          "  /** @brief next line doc */\n"
          "  struct { ; int y; } n;\n"
          "};\n",
          fhdr);
    fclose(fhdr);
    av[0] = cdd_test_mut_str("cdd_test_tmp_edges.h");
    av[1] = cdd_test_mut_str("cdd_test_tmp_out.json");
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av));

    /* OOM on parse_struct_member_line inside nested struct */
    fhdr = fopen("cdd_test_tmp_edges.h", "w");
    fputs("struct S { struct { int y; } n; };\n", fhdr);
    fclose(fhdr);
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, code2schema_main(2, av));
    g_cdd_strdup_fail = 0;

    /* Unclosed struct at EOF with trailing whitespace */
    fhdr = fopen("cdd_test_tmp_edges.h", "w");
    fputs("struct S { int a;   ", fhdr);
    fclose(fhdr);
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av));

    /* Unclosed nested struct at EOF */
    fhdr = fopen("cdd_test_tmp_edges.h", "w");
    fputs("struct S { struct { int a;   ", fhdr);
    fclose(fhdr);
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av));

    /* Unclosed enum at EOF with trailing whitespace */
    fhdr = fopen("cdd_test_tmp_edges.h", "w");
    fputs("enum E { A,   ", fhdr);
    fclose(fhdr);
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av));

    /* Unclosed union at EOF with trailing whitespace */
    fhdr = fopen("cdd_test_tmp_edges.h", "w");
    fputs("union U { int a;   ", fhdr);
    fclose(fhdr);
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av));

    /* c2s_parse_union_and_write OOM on read_from_fh and tokenize */
    fhdr = fopen("cdd_test_tmp_edges.h", "w");
    fputs("int a;\n};\n", fhdr);
    fclose(fhdr);
    {
      JSON_Value *sv = NULL;
      JSON_Object *so = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
        sv = _cdd_jv_1;
      }
      so = json_value_get_object(sv);
      fhdr = fopen("cdd_test_tmp_edges.h", "r");
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, c2s_parse_union_and_write(fhdr, so, "U"));
      g_cdd_alloc_fail = 0;
      fclose(fhdr);

      fhdr = fopen("cdd_test_tmp_edges.h", "r");
      g_cdd_alloc_fail = 2;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, c2s_parse_union_and_write(fhdr, so, "U"));
      g_cdd_alloc_fail = 0;
      fclose(fhdr);
      g_cdd_last_alloc_err = json_value_free(sv);
    }

    remove("cdd_test_tmp_edges.h");
    remove("cdd_test_tmp_out.json");
  }

  /* 34. mapper, unions, and writer remaining error branches */
  {
    struct StructFields sf;
    JSON_Value *v_root = NULL;
    JSON_Object *o_root = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"MyObj\":{\"type\":\"object\",\"properties\":{"
          "\"x\":{\"type\":\"integer\"}}},"
          "\"MyEnum\":{\"type\":\"string\",\"enum\":[\"A\"]}}",
          &_cdd_jv_1);
      v_root = _cdd_jv_1;
    }
    o_root = json_value_get_object(v_root);

    /* anyOf OOM (lines 301-306 of code2schema_mapper.c) */
    {
      JSON_Value *v_any1 = NULL;
      JSON_Value *v_any2 = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_parse_string(
            "{\"anyOf\":[{\"$ref\":\"#/components/schemas/MyObj\"}]}",
            &_cdd_jv_1);
        v_any1 = _cdd_jv_1;
      }
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_parse_string(
            "{\"anyOf\":[{\"type\":\"object\",\"properties\":{\"x\":{\"type\":"
            "\"integer\"}}}]}",
            &_cdd_jv_1);
        v_any2 = _cdd_jv_1;
      }
      g_cdd_last_alloc_err = struct_fields_init(&sf);
      g_cdd_alloc_fail = 1;
      ASSERT_NEQ(CDD_C_SUCCESS, c2s_json_object_to_struct_fields_internal(
                                    json_value_get_object(v_any1), &sf, o_root,
                                    "AnyFallback", 0));
      g_cdd_last_alloc_err = struct_fields_free(&sf);
      g_cdd_last_alloc_err = struct_fields_init(&sf);
      g_cdd_alloc_fail = 1;
      ASSERT_NEQ(CDD_C_SUCCESS, c2s_json_object_to_struct_fields_internal(
                                    json_value_get_object(v_any2), &sf, o_root,
                                    "AnyFallback", 0));
      g_cdd_alloc_fail = 0;
      g_cdd_last_alloc_err = struct_fields_free(&sf);
      g_cdd_last_alloc_err = json_value_free(v_any1);
      g_cdd_last_alloc_err = json_value_free(v_any2);
    }

    /* direct $ref struct_fields_add OOM (lines 255-256 of code2schema_mapper.c)
     */
    {
      int j;
      JSON_Value *v_ref = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_parse_string(
            "{\"type\":\"object\",\"properties\":{\"direct_ref\":{\"$ref\":\"#/"
            "components/schemas/MyEnum\"}}}",
            &_cdd_jv_1);
        v_ref = _cdd_jv_1;
      }
      g_cdd_last_alloc_err = struct_fields_init(&sf);
      for (j = 0; j < 8; ++j)
        g_cdd_last_alloc_err =
            struct_fields_add(&sf, "pad", "string", NULL, NULL, NULL);
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                c2s_json_object_to_struct_fields_internal(
                    json_value_get_object(v_ref), &sf, o_root, "RefOOM", 0));
      g_cdd_alloc_fail = 0;
      g_cdd_last_alloc_err = struct_fields_free(&sf);
      g_cdd_last_alloc_err = json_value_free(v_ref);
    }

    /* union array variant items type union OOM (lines 305-307 of
     * code2schema_unions.c) */
    {
      int j;
      JSON_Value *v_uarr = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_parse_string("{\"oneOf\":[{\"type\":\"array\",\"items\":{"
                                  "\"type\":[\"string\",\"null\"]}},"
                                  "{\"type\":\"integer\"}]}",
                                  &_cdd_jv_1);
        v_uarr = _cdd_jv_1;
      }
      for (j = 1; j < 10; ++j) {
        g_cdd_last_alloc_err = struct_fields_init(&sf);
        g_cdd_strdup_fail = j;
        g_cdd_last_alloc_err = c2s_json_object_to_struct_fields_internal(
            json_value_get_object(v_uarr), &sf, o_root, "UnionArrOOM", 1);
        g_cdd_strdup_fail = 0;
        g_cdd_last_alloc_err = struct_fields_free(&sf);
      }
      for (j = 1; j < 10; ++j) {
        g_cdd_last_alloc_err = struct_fields_init(&sf);
        g_cdd_alloc_fail = j;
        g_cdd_last_alloc_err = c2s_json_object_to_struct_fields_internal(
            json_value_get_object(v_uarr), &sf, o_root, "UnionArrOOM", 1);
        g_cdd_alloc_fail = 0;
        g_cdd_last_alloc_err = struct_fields_free(&sf);
      }
      g_cdd_last_alloc_err = json_value_free(v_uarr);
    }

    g_cdd_last_alloc_err = json_value_free(v_root);
  }

  /* 35. code2schema.c full branch and comment directive coverage */
  {
    FILE *fhdr = fopen("cdd_test_tmp_c2s_full.h", "w");
    char *av[2];
    int k;
    av[0] = cdd_test_mut_str("cdd_test_tmp_c2s_full.h");
    av[1] = cdd_test_mut_str("cdd_test_tmp_c2s_full.json");

    /* Long comment (>1024 chars) + rich struct/nested/enum comments */
    fprintf(fhdr, "/* ");
    for (k = 0; k < 1020; ++k)
      fputc('a', fhdr);
    fprintf(fhdr, " */ /* second */\n");
    fputs("union UBefore { int a; };\n"
          "/** @details Container details\n"
          " * @cdd(alias = \"cmd_alias\", rename_all = \"camelCase\") */\n"
          "struct "
          "VeryLongIdentifierNameThatExceedsSixtyThreeCharactersForTokenCopy123"
          "45 {\n"
          "  int x;   /** @brief trailing after spaces */\n"
          "  int y;/** @brief trailing immediate */\n"
          "  struct NotAnon *ptr;\n"
          "};\n",
          fhdr);
    fputs("struct RichStruct {\n"
          "  int f0; int f1; int f2; int f3; int f4; int f5; int f6; int f7;\n"
          "  /** @details Nested field details */\n"
          "  struct {\n"
          "    /** @details Inner field details */\n"
          "    int inner_val;\n"
          "    singleword_inner;\n"
          "  } nested_obj\n"
          "  /** @details Top field details */\n"
          "  int top_val;\n"
          "  int no_lead_val;\n"
          "  /** @details Ignored singleword cmt */\n"
          "  singleword_top;\n"
          "  struct {\n"
          "    int x;\n"
          "  };\n"
          "};\n",
          fhdr);
    fputs("/** @details Enum details\n"
          " * @cdd(alias = \"enum_alias\", rename_all = \"snake_case\") */\n"
          "enum RichEnum {\n"
          "  RichEnum,\n"
          "  RichEnum_,\n"
          "  RichEnum_VAL_ONE /* inline cmt */ = 1, /* trailing cmt */\n"
          "  /** @details Variant two details\n"
          "   * @cdd(rename = \"explicit_two\", other) */\n"
          "  RichEnum_VAL_TWO = 2,\n",
          fhdr);
    fputs("  /** @cdd(case = \"lowercase\", replace(\"_\", \"-\")) */\n"
          "  RichEnum_VAL_THREE,\n"
          "  /** @cdd(replace(\"_\", \"-\"), alias = \"four_alias\") */\n"
          "  RichEnum_VAL_FOUR,\n"
          "  /** @cdd(case = \"UPPERCASE\") */\n"
          "  OTHER_PREFIX_FIVE\n"
          "};\n"
          "struct UnclosedAtEof {\n"
          "  int trailing_eof",
          fhdr);
    fclose(fhdr);
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av));

    for (k = 1; k <= 60; ++k) {
      g_cdd_strdup_fail = k;
      g_cdd_last_alloc_err = code2schema_main(2, av);
      g_cdd_strdup_fail = 0;
    }
    for (k = 1; k <= 60; ++k) {
      g_cdd_alloc_fail = k;
      g_cdd_last_alloc_err = code2schema_main(2, av);
      g_cdd_alloc_fail = 0;
    }

    /* Enum with >1024 chars inline/trailing comments */
    fhdr = fopen("cdd_test_tmp_c2s_full.h", "w");
    fprintf(fhdr, "enum LongCmtEnum {\n  V1 /* ");
    for (k = 0; k < 1050; ++k)
      fputc('x', fhdr);
    fprintf(fhdr, " */ = 1, /* ");
    for (k = 0; k < 1050; ++k)
      fputc('y', fhdr);
    fprintf(fhdr, " */\n  V2,/** @brief no space after comma */\n};\n"
                  "enum UnclosedEnumCmt {\n  /* cmt */");
    fclose(fhdr);
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av));

    fhdr = fopen("cdd_test_tmp_c2s_full.h", "w");
    fputs("struct SimpleTopFieldCmt {\n"
          "  /** @details Field long help\n"
          "   * @cdd(alias = \"f_alias\") */\n"
          "  int x;\n"
          "};\n",
          fhdr);
    fclose(fhdr);
    for (k = 1; k <= 15; ++k) {
      g_cdd_strdup_fail = k;
      g_cdd_last_alloc_err = code2schema_main(2, av);
      g_cdd_strdup_fail = 0;
    }
    for (k = 1; k <= 15; ++k) {
      g_cdd_alloc_fail = k;
      g_cdd_last_alloc_err = code2schema_main(2, av);
      g_cdd_alloc_fail = 0;
    }

    remove("cdd_test_tmp_c2s_full.h");
    remove("cdd_test_tmp_c2s_full.json");
  }

  /* 36. code2schema_writer.c full branch coverage */
  {
    JSON_Value *root_v = NULL;
    JSON_Object *schemas_obj = NULL;
    struct StructFields sf;
    char *aliases[2];
    char *conflicts[2];
    char *requires_arr[2];
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      root_v = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(root_v);

    /* Enum with rename == struct_name, rename != struct_name, replace, other,
     * NULL/UNKNOWN/skip members */
    g_cdd_last_alloc_err = struct_fields_init(&sf);
    sf.is_enum = 1;
    g_cdd_last_alloc_err = enum_members_init(&sf.enum_members);
    CDD_STRCPY(sf.serde_rename, sizeof(sf.serde_rename), "SameEnum");
    sf.serde_rename_all = CDD_SERDE_CASE_SNAKECASE;
    sf.enum_members.serde_rename_all = CDD_SERDE_CASE_NONE;
    CDD_STRCPY(sf.serde_replace_from, sizeof(sf.serde_replace_from), "_");
    CDD_STRCPY(sf.serde_replace_to, sizeof(sf.serde_replace_to), "-");
    g_cdd_last_alloc_err =
        enum_members_add_ex(&sf.enum_members, "UNKNOWN", NULL, 0);
    g_cdd_last_alloc_err =
        enum_members_add_ex(&sf.enum_members, "SKIP_ME", NULL, 1);
    g_cdd_last_alloc_err =
        enum_members_add_ex(&sf.enum_members, "VAL_A", "custom_a", 0);
    sf.enum_members.members[sf.enum_members.size] = NULL;
    sf.enum_members.size++;
    sf.enum_members.serde_other_idx = (int)(sf.enum_members.size - 1);
    ASSERT_EQ(CDD_C_SUCCESS,
              write_struct_to_json_schema(schemas_obj, "SameEnum", &sf));
    sf.enum_members.serde_other_idx = 99;
    CDD_STRCPY(sf.serde_rename, sizeof(sf.serde_rename), "DiffEnum");
    CDD_STRCPY(sf.enum_members.serde_replace_from,
               sizeof(sf.enum_members.serde_replace_from), "A");
    CDD_STRCPY(sf.enum_members.serde_replace_to,
               sizeof(sf.enum_members.serde_replace_to), "B");
    free(sf.enum_members.serde_skip);
    sf.enum_members.serde_skip = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              write_struct_to_json_schema(schemas_obj, "OrigEnum", &sf));
    sf.enum_members.size--;
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    /* Object with all container and field serde/cli attributes */
    g_cdd_last_alloc_err = struct_fields_init(&sf);
    CDD_STRCPY(sf.serde_rename, sizeof(sf.serde_rename), "RenamedObj");
    sf.serde_rename_all = CDD_SERDE_CASE_CAMELCASE;
    sf.serde_rename_all_ser = CDD_SERDE_CASE_SNAKECASE;
    sf.serde_rename_all_de = CDD_SERDE_CASE_PASCALCASE;
    CDD_STRCPY(sf.serde_replace_from, sizeof(sf.serde_replace_from), "old");
    CDD_STRCPY(sf.serde_replace_to, sizeof(sf.serde_replace_to), "new");
    CDD_STRCPY(sf.serde_prefix, sizeof(sf.serde_prefix), "pre_");
    CDD_STRCPY(sf.serde_suffix, sizeof(sf.serde_suffix), "_suf");
    sf.serde_deny_unknown_fields = 1;
    sf.serde_default = 1;
    CDD_STRCPY(sf.serde_tag, sizeof(sf.serde_tag), "t");
    CDD_STRCPY(sf.serde_content, sizeof(sf.serde_content), "c");
    sf.serde_untagged = 1;
    sf.is_cli_command = 1;
    CDD_STRCPY(sf.cli_name, sizeof(sf.cli_name), "mycli");
    CDD_STRCPY(sf.cli_version, sizeof(sf.cli_version), "1.0");
    CDD_STRCPY(sf.cli_about, sizeof(sf.cli_about), "about");
    CDD_STRCPY(sf.cli_author, sizeof(sf.cli_author), "author");
    sf.cli_rename_all = CDD_SERDE_CASE_KEBABCASE;
    CDD_STRCPY(sf.cli_env_prefix, sizeof(sf.cli_env_prefix), "APP_");

    /* Field 0: skip_if null, aliases, conflicts, requires, skip=1 */
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f0", "string", NULL, NULL, "4");
    CDD_STRCPY(sf.fields[0].serde_rename, sizeof(sf.fields[0].serde_rename),
               "f0_ren");
    CDD_STRCPY(sf.fields[0].serde_rename_ser,
               sizeof(sf.fields[0].serde_rename_ser), "f0_ser");
    CDD_STRCPY(sf.fields[0].serde_rename_de,
               sizeof(sf.fields[0].serde_rename_de), "f0_de");
    sf.fields[0].serde_case = CDD_SERDE_CASE_UPPERCASE;
    CDD_STRCPY(sf.fields[0].serde_replace_from,
               sizeof(sf.fields[0].serde_replace_from), "a");
    CDD_STRCPY(sf.fields[0].serde_replace_to,
               sizeof(sf.fields[0].serde_replace_to), "b");
    aliases[0] = cdd_test_mut_str("al1");
    aliases[1] = NULL;
    sf.fields[0].serde_aliases = aliases;
    sf.fields[0].n_serde_aliases = 2;
    sf.fields[0].serde_skip = 1;
    sf.fields[0].serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;
    sf.fields[0].serde_flatten = 1;
    sf.fields[0].serde_has_default = 1;
    CDD_STRCPY(sf.fields[0].serde_default_fn,
               sizeof(sf.fields[0].serde_default_fn), "def_fn");
    CDD_STRCPY(sf.fields[0].serde_with, sizeof(sf.fields[0].serde_with),
               "mod_w");
    CDD_STRCPY(sf.fields[0].serde_serialize_with,
               sizeof(sf.fields[0].serde_serialize_with), "ser_w");
    CDD_STRCPY(sf.fields[0].serde_deserialize_with,
               sizeof(sf.fields[0].serde_deserialize_with), "de_w");
    sf.fields[0].cli_short = 'f';
    CDD_STRCPY(sf.fields[0].cli_long, sizeof(sf.fields[0].cli_long), "flag0");
    sf.fields[0].cli_negatable = 1;
    sf.fields[0].cli_positional = 1;
    sf.fields[0].cli_index = 0;
    CDD_STRCPY(sf.fields[0].cli_env, sizeof(sf.fields[0].cli_env), "F0_ENV");
    CDD_STRCPY(sf.fields[0].cli_value_name, sizeof(sf.fields[0].cli_value_name),
               "VAL0");
    CDD_STRCPY(sf.fields[0].cli_help, sizeof(sf.fields[0].cli_help), "help0");
    sf.fields[0].cli_action = CDD_CLI_ACTION_COUNT;
    sf.fields[0].cli_value_delimiter = ',';
    sf.fields[0].cli_subcommand = 1;
    conflicts[0] = cdd_test_mut_str("other_f");
    conflicts[1] = NULL;
    sf.fields[0].cli_conflicts_with = conflicts;
    sf.fields[0].n_cli_conflicts_with = 2;
    requires_arr[0] = cdd_test_mut_str("req_f");
    requires_arr[1] = NULL;
    sf.fields[0].cli_requires = requires_arr;
    sf.fields[0].n_cli_requires = 2;
    sf.fields[0].required = 1;

    /* Field 1: skip_serializing=1, skip_deserializing=1, skip_if=empty */
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f1", "string", NULL, NULL, NULL);
    sf.fields[1].serde_skip_serializing = 1;
    sf.fields[1].serde_skip_deserializing = 1;
    sf.fields[1].serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
    sf.fields[1].n_serde_aliases = 1;
    sf.fields[1].serde_aliases = NULL;
    sf.fields[1].n_cli_conflicts_with = 1;
    sf.fields[1].cli_conflicts_with = NULL;
    sf.fields[1].n_cli_requires = 1;
    sf.fields[1].cli_requires = NULL;
    sf.fields[1].required = 1;

    /* Field 2: skip_if=zero */
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f2", "integer", NULL, NULL, NULL);
    sf.fields[2].serde_skip_if_kind = CDD_SERDE_SKIP_IF_ZERO;

    /* Field 3: skip_if=custom with fn name */
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f3", "integer", NULL, NULL, NULL);
    sf.fields[3].serde_skip_if_kind = CDD_SERDE_SKIP_IF_FUNC;
    CDD_STRCPY(sf.fields[3].serde_skip_if_fn,
               sizeof(sf.fields[3].serde_skip_if_fn), "my_pred");

    /* Field 4: skip_if=custom with empty fn name */
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f4", "integer", NULL, NULL, NULL);
    sf.fields[4].serde_skip_if_kind = CDD_SERDE_SKIP_IF_FUNC;
    sf.fields[4].has_min = 1;
    sf.fields[4].exclusive_min = 1;
    sf.fields[4].has_max = 1;
    sf.fields[4].exclusive_max = 1;

    /* Field 5: array of number, array of CustomRef, and empty field name */
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f5", "array", "number", NULL, NULL);
    sf.fields[5].has_min_items = 1;
    sf.fields[5].has_max_items = 1;
    sf.fields[5].unique_items = 1;
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f6", "array", "CustomRef", NULL, NULL);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f7", "string", NULL, "\"hello\"", NULL);
    sf.fields[7].name[0] = '\0';
    sf.fields[7].has_min_len = 1;
    sf.fields[7].has_max_len = 1;
    CDD_STRCPY(sf.fields[7].pattern, sizeof(sf.fields[7].pattern), "^[a-z]+$");
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f8", "boolean", NULL, "invalid_bool", NULL);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f8b", "boolean", NULL, "true", NULL);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f9", "number", NULL, "3.14", NULL);
    sf.fields[sf.size - 1].has_min = 1;
    sf.fields[sf.size - 1].has_max = 1;
    g_cdd_last_alloc_err = struct_fields_add(
        &sf, "f10", "object", "#/components/schemas/DirectRef", NULL, NULL);
    g_cdd_last_alloc_err =
        struct_fields_add(&sf, "f11", "object", NULL, NULL, NULL);
    {
      JSON_Value *pv = NULL;
      struct StructField empty_f;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
        pv = _cdd_jv_1;
      }
      memset(&empty_f, 0, sizeof(empty_f));
      g_cdd_last_alloc_err = c2s_write_array_constraints(NULL, &empty_f);
      g_cdd_last_alloc_err =
          c2s_write_array_constraints(json_value_get_object(pv), NULL);
      g_cdd_last_alloc_err =
          c2s_write_array_constraints(json_value_get_object(pv), &empty_f);
      CDD_STRCPY(empty_f.type, sizeof(empty_f.type), "string");
      g_cdd_last_alloc_err =
          c2s_write_array_constraints(json_value_get_object(pv), &empty_f);
      g_cdd_last_alloc_err = json_value_free(pv);
    }

    ASSERT_EQ(CDD_C_SUCCESS,
              write_struct_to_json_schema(schemas_obj, "RichObj", &sf));

    sf.fields[0].serde_aliases = NULL;
    sf.fields[0].n_serde_aliases = 0;
    sf.fields[0].cli_conflicts_with = NULL;
    sf.fields[0].n_cli_conflicts_with = 0;
    sf.fields[0].cli_requires = NULL;
    sf.fields[0].n_cli_requires = 0;
    sf.fields[1].n_serde_aliases = 0;
    sf.fields[1].n_cli_conflicts_with = 0;
    sf.fields[1].n_cli_requires = 0;
    g_cdd_last_alloc_err = struct_fields_free(&sf);
    g_cdd_last_alloc_err = json_value_free(root_v);
  }

  /* 37. code2schema_mapper.c full branch coverage */
  {
    const char *enum_jsons[] = {
        ("{\"type\":\"string\",\"enum\":[\"a-b\",\"c\"],"
         "\"x-cdd-rename-all\":\"invalid_case\","
         "\"x-cdd-enum-c-names\":[\"C_ZERO\",null,\"C_ONE\",\"EXTRA\"],"
         "\"x-cdd-enum-names\":[\"w_zero\",\"\",\"w_three\"],"
         "\"x-cdd-enum-other\":\"c\"}"),
        ("{\"type\":\"string\",\"enum\":[\"a\",\"b\"],"
         "\"x-cdd-enum-c-names\":[\"A_1\",\"B_1\"],"
         "\"x-cdd-enum-names\":[\"a_w\",\"b_w\"]}"),
        ("{\"type\":\"string\",\"enum\":[\"x\",\"y\"],"
         "\"x-cdd-enum-c-names\":[\"\",\"Y_1\"],"
         "\"x-cdd-enum-other\":\"\"}"),
        ("{\"type\":\"string\",\"enum\":[\"x\",\"y\"],"
         "\"x-cdd-enum-other\":\"NOT_FOUND\"}")};
    size_t ej;
    for (ej = 0; ej < sizeof(enum_jsons) / sizeof(enum_jsons[0]); ++ej) {
      JSON_Value *ev = NULL;
      JSON_Object *eo = NULL;
      struct StructFields esf;
      int k;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_parse_string(enum_jsons[ej], &_cdd_jv_1);
        ev = _cdd_jv_1;
      }
      eo = json_value_get_object(ev);
      g_cdd_last_alloc_err = struct_fields_init(&esf);
      ASSERT_EQ(CDD_C_SUCCESS,
                json_object_to_struct_fields_ex(eo, &esf, NULL, "my-enum"));
      g_cdd_last_alloc_err = struct_fields_free(&esf);
      for (k = 1; k <= 10; ++k) {
        g_cdd_strdup_fail = k;
        g_cdd_last_alloc_err = struct_fields_init(&esf);
        g_cdd_last_alloc_err =
            json_object_to_struct_fields_ex(eo, &esf, NULL, "my-enum");
        g_cdd_strdup_fail = 0;
        g_cdd_last_alloc_err = struct_fields_free(&esf);
      }
      for (k = 1; k <= 10; ++k) {
        g_cdd_alloc_fail = k;
        g_cdd_last_alloc_err = struct_fields_init(&esf);
        g_cdd_last_alloc_err =
            json_object_to_struct_fields_ex(eo, &esf, NULL, "my-enum");
        g_cdd_alloc_fail = 0;
        g_cdd_last_alloc_err = struct_fields_free(&esf);
      }
      g_cdd_last_alloc_err = json_value_free(ev);
    }

    {
      const char *obj_parts[] = {
          "{\"type\":\"object\","
          "\"x-cdd-rename\":\"ObjRen\","
          "\"x-cdd-rename-all\":\"camelCase\","
          "\"x-cdd-rename-all-serialize\":\"snake_case\","
          "\"x-cdd-rename-all-deserialize\":\"PascalCase\","
          "\"x-cdd-replace-from\":\"_\","
          "\"x-cdd-replace-to\":\"-\","
          "\"x-cdd-prefix\":\"p_\","
          "\"x-cdd-suffix\":\"_s\","
          "\"x-cdd-deny-unknown-fields\":true,"
          "\"x-cdd-default\":true,"
          "\"x-cdd-tag\":\"t\","
          "\"x-cdd-content\":\"c\","
          "\"x-cdd-untagged\":true}",
          "{\"type\":\"object\","
          "\"x-cdd-cli-command\":true,"
          "\"x-cdd-cli-name\":\"cn\","
          "\"x-cdd-cli-version\":\"1.0\","
          "\"x-cdd-cli-about\":\"ab\","
          "\"x-cdd-cli-author\":\"au\","
          "\"x-cdd-cli-rename-all\":\"kebab-case\","
          "\"x-cdd-cli-env-prefix\":\"EP_\"}",
          "{\"type\":\"object\",\"properties\":{"
          "\"a-b\":{\"type\":\"string\",\"x-cdd-c-name\":\"\","
          "\"x-cdd-rename\":\"r\",\"x-cdd-rename-serialize\":\"rs\","
          "\"x-cdd-rename-deserialize\":\"rd\",\"x-cdd-case\":\"lowercase\","
          "\"x-cdd-replace-from\":\"x\",\"x-cdd-replace-to\":\"y\","
          "\"x-cdd-aliases\":[\"al1\",123],"
          "\"x-cdd-skip\":true,\"x-cdd-skip-serializing\":true,"
          "\"x-cdd-skip-deserializing\":true,"
          "\"x-cdd-skip-serializing-if\":\"null\"}}}",
          "{\"type\":\"object\",\"properties\":{"
          "\"a\":{\"type\":\"string\","
          "\"x-cdd-flatten\":true,\"x-cdd-default\":true,"
          "\"x-cdd-default-fn\":\"dfn\",\"x-cdd-with\":\"w\","
          "\"x-cdd-serialize-with\":\"sw\",\"x-cdd-deserialize-with\":\"dw\","
          "\"x-cdd-cli-short\":\"f\",\"x-cdd-cli-long\":\"flag\","
          "\"x-cdd-cli-negatable\":true,\"x-cdd-cli-positional\":true,"
          "\"x-cdd-cli-index\":0,\"x-cdd-cli-env\":\"ENV\"}}}",
          "{\"type\":\"object\",\"properties\":{"
          "\"a\":{\"type\":\"string\","
          "\"x-cdd-cli-value-name\":\"VN\",\"x-cdd-cli-help\":\"H\","
          "\"x-cdd-cli-action\":\"count\",\"x-cdd-cli-delimiter\":\",\","
          "\"x-cdd-cli-subcommand\":true,"
          "\"x-cdd-cli-conflicts-with\":[\"c1\",123],"
          "\"x-cdd-cli-requires\":[\"r1\",123]}}}",
          "{\"type\":\"object\","
          "\"x-cdd-rename-all\":\"bad_case\","
          "\"x-cdd-rename-all-serialize\":\"bad_case\","
          "\"x-cdd-rename-all-deserialize\":\"bad_case\","
          "\"x-cdd-deny-unknown-fields\":false,"
          "\"x-cdd-default\":false,"
          "\"x-cdd-untagged\":false,"
          "\"x-cdd-cli-command\":false,"
          "\"x-cdd-cli-rename-all\":\"bad_case\"}",
          "{\"type\":\"object\",\"properties\":{"
          "\"f_empty\":{\"type\":\"string\",\"x-cdd-c-name\":\"dup_nm\","
          "\"x-cdd-case\":\"bad_case\",\"x-cdd-skip\":false,"
          "\"x-cdd-skip-serializing\":false,\"x-cdd-skip-deserializing\":false,"
          "\"x-cdd-skip-serializing-if\":\"empty\","
          "\"x-cdd-flatten\":false,\"x-cdd-default\":false,"
          "\"x-cdd-cli-short\":\"\",\"x-cdd-cli-long\":\"\","
          "\"x-cdd-cli-negatable\":false,\"x-cdd-cli-positional\":false,"
          "\"x-cdd-cli-action\":\"bad_act\",\"x-cdd-cli-delimiter\":\"\","
          "\"x-cdd-cli-subcommand\":false}}}",
          "{\"type\":\"object\",\"properties\":{"
          "\"f1\":{\"type\":\"string\",\"x-cdd-c-name\":\"dup_nm\"},"
          "\"f_zero\":{\"type\":\"integer\",\"x-cdd-c-name\":\"dup_nm\","
          "\"x-cdd-skip-serializing-if\":\"zero\"},"
          "\"f_custom\":{\"type\":\"integer\",\"x-cdd-c-name\":\"dup_nm\","
          "\"x-cdd-skip-serializing-if\":\"my_fn\"},"
          "\"f_blank_if\":{\"type\":\"integer\","
          "\"x-cdd-skip-serializing-if\":\"\"}}}",
          "{\"type\":\"object\",\"properties\":{"
          "\"arr\":{\"type\":\"array\",\"items\":{\"type\":\"string\"}},"
          "\"n_arr\":{\"type\":\"integer\"},"
          "\"n_arr_2\":{\"type\":\"integer\"},"
          "\"arr2\":{\"type\":\"array\",\"items\":{\"type\":\"string\"}},"
          "\"n_arr2\":{\"type\":\"integer\",\"x-cdd-rename\":\"already_ren\"}"
          "}}"};
      size_t oj;
      for (oj = 0; oj < sizeof(obj_parts) / sizeof(obj_parts[0]); ++oj) {
        JSON_Value *ov = NULL;
        JSON_Object *oo = NULL;
        struct StructFields osf;
        int k;
        {
          JSON_Value *_cdd_jv_1 = NULL;
          g_cdd_last_alloc_err =
              cdd_json_parse_string(obj_parts[oj], &_cdd_jv_1);
          ov = _cdd_jv_1;
        }
        oo = json_value_get_object(ov);
        ASSERT(oo != NULL);
        g_cdd_last_alloc_err = struct_fields_init(&osf);
        ASSERT_EQ(CDD_C_SUCCESS,
                  json_object_to_struct_fields_ex(oo, &osf, NULL, "my-obj"));
        g_cdd_last_alloc_err = struct_fields_free(&osf);
        for (k = 1; k <= 25; ++k) {
          g_cdd_strdup_fail = k;
          g_cdd_last_alloc_err = struct_fields_init(&osf);
          g_cdd_last_alloc_err =
              json_object_to_struct_fields_ex(oo, &osf, NULL, "my-obj");
          g_cdd_strdup_fail = 0;
          g_cdd_last_alloc_err = struct_fields_free(&osf);
        }
        for (k = 1; k <= 25; ++k) {
          g_cdd_alloc_fail = k;
          g_cdd_last_alloc_err = struct_fields_init(&osf);
          g_cdd_last_alloc_err =
              json_object_to_struct_fields_ex(oo, &osf, NULL, "my-obj");
          g_cdd_alloc_fail = 0;
          g_cdd_last_alloc_err = struct_fields_free(&osf);
        }
        g_cdd_last_alloc_err = json_value_free(ov);
      }

      /* Remaining branches in code2schema_mapper.c */
      {
        struct StructFields msf;
        JSON_Value *mv;
        JSON_Object *mo;
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  c2s_json_object_to_struct_fields_internal(NULL, &msf, NULL,
                                                            NULL, 0));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  c2s_json_object_to_struct_fields_internal(
                      (const JSON_Object *)1, NULL, NULL, NULL, 0));

        g_json_object_to_struct_fields_fail = 2;
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  json_object_to_struct_fields_ex(NULL, NULL, NULL, NULL));
        g_json_object_to_struct_fields_fail = 1;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  json_object_to_struct_fields_ex(NULL, NULL, NULL, NULL));
        g_json_object_to_struct_fields_fail = 2;
        ASSERT_EQ(
            CDD_C_ERROR_INVALID_ARGUMENT,
            json_object_to_struct_fields_ex_codegen(NULL, NULL, NULL, NULL));
        g_json_object_to_struct_fields_fail = 1;
        ASSERT_EQ(CDD_C_ERROR_MEMORY, json_object_to_struct_fields_ex_codegen(
                                          NULL, NULL, NULL, NULL));
        g_json_object_to_struct_fields_fail = 0;

        /* allOf error, type union error, array item union, exclusiveMin/Max,
         * array constraints */
        {
          JSON_Value *_cdd_jv_1 = NULL;
          g_cdd_last_alloc_err = cdd_json_parse_string(
              "{\"allOf\":[{\"x\":1,\"properties\":{\"x\":{"
              "\"type\":\"string\"}}}]}",
              &_cdd_jv_1);
          mv = _cdd_jv_1;
        }
        mo = json_value_get_object(mv);
        memset(&msf, 0, sizeof(msf));
        g_cdd_alloc_fail = 1;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  json_object_to_struct_fields(mo, &msf, NULL));
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        memset(&msf, 0, sizeof(msf));
        g_cdd_alloc_fail = 2;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  json_object_to_struct_fields(mo, &msf, NULL));
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        g_cdd_alloc_fail = 0;
        g_cdd_last_alloc_err = struct_fields_init(&msf);
        g_cdd_strdup_fail = 2;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  json_object_to_struct_fields(mo, &msf, NULL));
        g_cdd_strdup_fail = 0;
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        g_cdd_last_alloc_err = json_value_free(mv);

        {
          JSON_Value *_cdd_jv_1 = NULL;
          g_cdd_last_alloc_err = cdd_json_parse_string(
              "{\"properties\":{\"r\":{\"$ref\":\"#/R\"},\"a\":{\"type\":"
              "\"array\",\"items\":{\"type\":\"string\",\"x\":1}}}}",
              &_cdd_jv_1);
          mv = _cdd_jv_1;
        }
        mo = json_value_get_object(mv);
        g_cdd_last_alloc_err = struct_fields_init(&msf);
        ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(mo, &msf, NULL));
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        g_cdd_last_alloc_err = struct_fields_init(&msf);
        g_cdd_strdup_fail = 1;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  json_object_to_struct_fields(mo, &msf, NULL));
        g_cdd_strdup_fail = 0;
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        g_cdd_last_alloc_err = json_value_free(mv);

        {
          JSON_Value *_cdd_jv_1 = NULL;
          g_cdd_last_alloc_err = cdd_json_parse_string(
              "{\"properties\":{"
              "\"u\":{\"type\":[\"number\",\"null\"],\"default\":1.5},"
              "\"d\":{\"type\":\"number\",\"exclusiveMinimum\":0,"
              "\"exclusiveMaximum\":9},"
              "\"e1\":{\"type\":\"number\",\"exclusiveMinimum\":true,"
              "\"exclusiveMaximum\":true},"
              "\"e2\":{\"type\":\"number\",\"exclusiveMinimum\":false,"
              "\"exclusiveMaximum\":false},"
              "\"au\":{\"type\":[\"array\",\"null\"],"
              "\"items\":{\"type\":[\"integer\",\"null\"],\"x\":1}},"
              "\"a1\":{\"type\":\"array\",\"minItems\":1,\"maxItems\":5,"
              "\"uniqueItems\":false,\"items\":{}},"
              "\"a2\":{\"type\":\"array\"}"
              "}}",
              &_cdd_jv_1);
          mv = _cdd_jv_1;
        }
        mo = json_value_get_object(mv);
        ASSERT(mo != NULL);
        memset(&msf, 0, sizeof(msf));
        g_cdd_alloc_fail = 1;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  json_object_to_struct_fields(mo, &msf, NULL));
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        memset(&msf, 0, sizeof(msf));
        g_cdd_alloc_fail = 2;
        ASSERT_EQ(CDD_C_ERROR_MEMORY,
                  json_object_to_struct_fields(mo, &msf, NULL));
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        g_cdd_alloc_fail = 0;
        g_cdd_last_alloc_err = struct_fields_init(&msf);
        ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(mo, &msf, NULL));
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        {
          int k;
          for (k = 1; k <= 15; ++k) {
            g_cdd_alloc_fail = k;
            g_cdd_last_alloc_err = struct_fields_init(&msf);
            g_cdd_last_alloc_err = json_object_to_struct_fields(mo, &msf, NULL);
            g_cdd_alloc_fail = 0;
            g_cdd_last_alloc_err = struct_fields_free(&msf);
          }
          for (k = 1; k <= 15; ++k) {
            g_cdd_strdup_fail = k;
            g_cdd_last_alloc_err = struct_fields_init(&msf);
            g_cdd_last_alloc_err = json_object_to_struct_fields(mo, &msf, NULL);
            g_cdd_strdup_fail = 0;
            g_cdd_last_alloc_err = struct_fields_free(&msf);
          }
          for (k = 1; k <= 15; ++k) {
            int p;
            g_cdd_last_alloc_err = struct_fields_init(&msf);
            for (p = 0; p < 8; ++p)
              g_cdd_last_alloc_err =
                  struct_fields_add(&msf, "pad", "string", NULL, NULL, NULL);
            g_cdd_alloc_fail = k;
            g_cdd_last_alloc_err = json_object_to_struct_fields(mo, &msf, NULL);
            g_cdd_alloc_fail = 0;
            g_cdd_last_alloc_err = struct_fields_free(&msf);
          }
        }
        g_cdd_last_alloc_err = json_value_free(mv);

        {
          JSON_Value *_cdd_jv_1 = NULL;
          g_cdd_last_alloc_err = cdd_json_parse_string(
              "{\"properties\":{\"i\":{\"type\":\"integer\",\"default\":42}}}",
              &_cdd_jv_1);
          mv = _cdd_jv_1;
        }
        mo = json_value_get_object(mv);
        ASSERT(mo != NULL);
        g_cdd_last_alloc_err = struct_fields_init(&msf);
        ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(mo, &msf, NULL));
        ASSERT_EQ(1, msf.size);
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        g_cdd_last_alloc_err = json_value_free(mv);

        {
          JSON_Value *_cdd_jv_1 = NULL;
          g_cdd_last_alloc_err = cdd_json_parse_string(
              "{\"properties\":{\"b\":{\"type\":\"boolean\","
              "\"default\":true}}}",
              &_cdd_jv_1);
          mv = _cdd_jv_1;
        }
        mo = json_value_get_object(mv);
        ASSERT(mo != NULL);
        g_cdd_last_alloc_err = struct_fields_init(&msf);
        ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(mo, &msf, NULL));
        ASSERT_EQ(1, msf.size);
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        g_cdd_last_alloc_err = json_value_free(mv);

        {
          JSON_Value *_cdd_jv_1 = NULL;
          g_cdd_last_alloc_err = cdd_json_parse_string(
              "{\"anyOf\":[{\"type\":\"string\"},{\"type\":\"integer\"}]}",
              &_cdd_jv_1);
          mv = _cdd_jv_1;
        }
        mo = json_value_get_object(mv);
        ASSERT(mo != NULL);
        g_cdd_last_alloc_err = struct_fields_init(&msf);
        ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(mo, &msf, NULL));
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        g_cdd_last_alloc_err = json_value_free(mv);

        {
          JSON_Value *_cdd_jv_1 = NULL;
          g_cdd_last_alloc_err = cdd_json_parse_string(
              "{\"properties\":{\"s\":{\"type\":\"string\",\"default\":\"hi\","
              "\"description\":\"d\"},"
              "\"ar\":{\"type\":\"array\",\"items\":{\"$ref\":\"#/R\"}}}}",
              &_cdd_jv_1);
          mv = _cdd_jv_1;
        }
        mo = json_value_get_object(mv);
        ASSERT(mo != NULL);
        g_cdd_last_alloc_err = struct_fields_init(&msf);
        ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(mo, &msf, NULL));
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        g_cdd_last_alloc_err = json_value_free(mv);

        {
          JSON_Value *_cdd_jv_1 = NULL;
          g_cdd_last_alloc_err = cdd_json_parse_string(
              "{\"properties\":{\"a\":{\"type\":\"array\",\"minItems\":1,"
              "\"maxItems\":5,\"uniqueItems\":true,\"items\":{\"type\":"
              "\"string\""
              "}}}}",
              &_cdd_jv_1);
          mv = _cdd_jv_1;
        }
        mo = json_value_get_object(mv);
        ASSERT(mo != NULL);
        g_cdd_last_alloc_err = struct_fields_init(&msf);
        ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(mo, &msf, NULL));
        ASSERT_EQ(1, msf.size);
        g_cdd_last_alloc_err = struct_fields_free(&msf);
        g_cdd_last_alloc_err = json_value_free(mv);
      }
    }
  }

  /* 38. code2schema_parser.c and code2schema_merge.c full branch coverage */
  {
    struct StructFields psf;
    char long_line[1100];
    int k;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_struct_member_line(NULL, &psf));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_struct_member_line("int a;", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2s_collapse_arrays(NULL));

    g_cdd_last_alloc_err = struct_fields_init(&psf);
    /* Block and line comments with ORM tags */
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_struct_member_line(
                  "/* @shard_hash @track_telemetry @slow_query_warn(10) */ "
                  "int*p1;",
                  &psf));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_struct_member_line("/* @track_telemetry */ int tt;", &psf));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_struct_member_line("/* @slow_query_warn */ int sq;", &psf));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_struct_member_line(
                  "int l1; // @shard_key @shard_hash @track_telemetry "
                  "@slow_query_warn(20)",
                  &psf));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_struct_member_line("int l2; // @slow_query_warn", &psf));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_struct_member_line("int l3; // normal cmt", &psf));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_struct_member_line("struct S arr[10];", &psf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("struct S fam[];", &psf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line(";;;", &psf));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_struct_member_line(
                  "struct "
                  "VeryLongTypeNameThatExceedsSixtyThreeCharactersLimit12345678"
                  "9012345 vlong;",
                  &psf));
    memcpy(long_line, "/* ", 3);
    for (k = 3; k < 1030; ++k)
      long_line[k] = 'a';
    memcpy(long_line + 1030, " */ int x;", 11);
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line(long_line, &psf));
    /* Bitfield starting with 'a' and 'Z' and '_' */
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int b1 : a;", &psf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int b2 : Z;", &psf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int b3 : _;", &psf));
    /* OOM on cdd_serde_parse_field_comment */
    g_cdd_strdup_fail = 3;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              parse_struct_member_line("/** @details D */ int f_oom;", &psf));
    g_cdd_strdup_fail = 0;
    g_cdd_last_alloc_err = struct_fields_free(&psf);

    /* OOM on struct_fields_add in parse_struct_member_line */
    memset(&psf, 0, sizeof(psf));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_struct_member_line("int f_oom;", &psf));
    g_cdd_alloc_fail = 0;

    /* c2s_parse_union_tokens_and_write with comment, float, custom type, and
     * unterminated stmt */
    {
      struct TokenList *utl = NULL;
      size_t uidx = 0;
      JSON_Value *uv = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
        uv = _cdd_jv_1;
      }
      ASSERT_EQ(CDD_C_SUCCESS,
                tokenize(az_span_create_from_str(
                             "; /* c */ float /* c */ f; CustomType c; "
                             "single_ident; 123; int unterm"),
                         &utl));
      ASSERT_EQ(CDD_C_SUCCESS, c2s_parse_union_tokens_and_write(
                                   utl, &uidx, json_value_get_object(uv), "U"));
      g_cdd_last_alloc_err = free_token_list(utl);
      g_cdd_last_alloc_err = json_value_free(uv);
    }

    /* c2s_collapse_arrays with object and already-array fields */
    {
      struct StructFields osf;
      g_cdd_last_alloc_err = struct_fields_init(&osf);
      g_cdd_last_alloc_err =
          struct_fields_add(&osf, "n_objs", "integer", NULL, NULL, NULL);
      g_cdd_last_alloc_err =
          struct_fields_add(&osf, "objs", "object", "MyObj", NULL, NULL);
      g_cdd_last_alloc_err =
          struct_fields_add(&osf, "n_arrs", "integer", NULL, NULL, NULL);
      g_cdd_last_alloc_err =
          struct_fields_add(&osf, "arrs", "array", "string", NULL, NULL);
      ASSERT_EQ(CDD_C_SUCCESS, c2s_collapse_arrays(&osf));
      g_cdd_last_alloc_err = struct_fields_free(&osf);
    }

    /* c2s_collapse_arrays exhaustive branches (both empty af and populated af)
     */
    {
      int mode;
      for (mode = 0; mode < 2; ++mode) {
        struct StructFields csf;
        struct StructField *nf, *af;
        g_cdd_last_alloc_err = struct_fields_init(&csf);
        g_cdd_last_alloc_err =
            struct_fields_add(&csf, "n_items", "integer", NULL, NULL, NULL);
        g_cdd_last_alloc_err =
            struct_fields_add(&csf, "items", "string", NULL, NULL, NULL);
        nf = &csf.fields[0];
        af = &csf.fields[1];

        CDD_STRCPY(nf->serde_rename, sizeof(nf->serde_rename), "ren");
        CDD_STRCPY(nf->serde_rename_ser, sizeof(nf->serde_rename_ser), "rser");
        CDD_STRCPY(nf->serde_rename_de, sizeof(nf->serde_rename_de), "rde");
        nf->serde_case = CDD_SERDE_CASE_LOWERCASE;
        CDD_STRCPY(nf->serde_replace_from, sizeof(nf->serde_replace_from), "a");
        CDD_STRCPY(nf->serde_replace_to, sizeof(nf->serde_replace_to), "b");
        nf->serde_skip = 1;
        nf->serde_skip_serializing = 1;
        nf->serde_skip_deserializing = 1;
        nf->serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;
        CDD_STRCPY(nf->serde_skip_if_fn, sizeof(nf->serde_skip_if_fn), "fn");
        nf->serde_flatten = 1;
        nf->serde_has_default = 1;
        CDD_STRCPY(nf->serde_default_fn, sizeof(nf->serde_default_fn), "dfn");
        CDD_STRCPY(nf->serde_with, sizeof(nf->serde_with), "w");
        CDD_STRCPY(nf->serde_serialize_with, sizeof(nf->serde_serialize_with),
                   "sw");
        CDD_STRCPY(nf->serde_deserialize_with,
                   sizeof(nf->serde_deserialize_with), "dw");
        g_cdd_last_alloc_err = struct_field_add_alias(nf, "al1");
        nf->serde_aliases =
            (char **)realloc(nf->serde_aliases, 2 * sizeof(char *));
        nf->serde_aliases[1] = NULL;
        nf->n_serde_aliases = 2;
        nf->cli_short = 's';
        CDD_STRCPY(nf->cli_short_aliases, sizeof(nf->cli_short_aliases), "ab");
        CDD_STRCPY(nf->cli_long, sizeof(nf->cli_long), "long");
        nf->cli_has_long = 1;
        nf->cli_negatable = 1;
        nf->cli_positional = 1;
        nf->cli_index = 0;
        nf->cli_last = 1;
        nf->cli_trailing_var_arg = 1;
        CDD_STRCPY(nf->cli_value_name, sizeof(nf->cli_value_name), "VN");
        CDD_STRCPY(nf->cli_help, sizeof(nf->cli_help), "H");
        g_cdd_last_alloc_err = c_cdd_strdup("LH", &nf->cli_long_help);
        CDD_STRCPY(nf->cli_help_heading, sizeof(nf->cli_help_heading), "HH");
        CDD_STRCPY(nf->cli_env, sizeof(nf->cli_env), "ENV");
        nf->cli_hide_env = 1;
        nf->cli_hide_env_values = 1;
        CDD_STRCPY(nf->cli_default_missing_val,
                   sizeof(nf->cli_default_missing_val), "DMV");
        nf->cli_hide_default_value = 1;
        nf->cli_action = CDD_CLI_ACTION_COUNT;
        nf->cli_value_hint = CDD_CLI_HINT_FILE_PATH;
        nf->cli_value_delimiter = ',';
        nf->cli_num_args_min = 1;
        nf->cli_num_args_max = 2;
        nf->cli_require_equals = 1;
        nf->cli_allow_hyphen_values = 1;
        nf->cli_global = 1;
        nf->cli_hidden = 1;
        nf->cli_exclusive = 1;
        nf->cli_subcommand = 1;
        CDD_STRCPY(nf->cli_value_parser, sizeof(nf->cli_value_parser), "vp");
        CDD_STRCPY(nf->cli_group, sizeof(nf->cli_group), "grp");
        CDD_STRCPY(nf->cli_required_if_field, sizeof(nf->cli_required_if_field),
                   "rif");
        CDD_STRCPY(nf->cli_required_if_val, sizeof(nf->cli_required_if_val),
                   "riv");
        g_cdd_last_alloc_err = struct_field_add_cli_conflict(nf, "c1");
        nf->cli_conflicts_with =
            (char **)realloc(nf->cli_conflicts_with, 2 * sizeof(char *));
        nf->cli_conflicts_with[1] = NULL;
        nf->n_cli_conflicts_with = 2;
        g_cdd_last_alloc_err = struct_field_add_cli_requires(nf, "r1");
        nf->cli_requires =
            (char **)realloc(nf->cli_requires, 2 * sizeof(char *));
        nf->cli_requires[1] = NULL;
        nf->n_cli_requires = 2;
        g_cdd_last_alloc_err = struct_field_add_cli_required_unless(nf, "ru1");
        nf->cli_required_unless =
            (char **)realloc(nf->cli_required_unless, 2 * sizeof(char *));
        nf->cli_required_unless[1] = NULL;
        nf->n_cli_required_unless = 2;
        g_cdd_last_alloc_err =
            c_cdd_strdup("{\"a\":1}", &nf->schema_extra_json);
        g_cdd_last_alloc_err = c_cdd_strdup("{\"b\":1}", &nf->items_extra_json);

        if (mode == 1) {
          CDD_STRCPY(af->serde_rename, sizeof(af->serde_rename), "existing");
          CDD_STRCPY(af->serde_rename_ser, sizeof(af->serde_rename_ser),
                     "existing");
          CDD_STRCPY(af->serde_rename_de, sizeof(af->serde_rename_de),
                     "existing");
          af->serde_case = CDD_SERDE_CASE_UPPERCASE;
          CDD_STRCPY(af->serde_replace_from, sizeof(af->serde_replace_from),
                     "x");
          af->serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
          CDD_STRCPY(af->serde_default_fn, sizeof(af->serde_default_fn), "ex");
          CDD_STRCPY(af->serde_with, sizeof(af->serde_with), "ex");
          CDD_STRCPY(af->serde_serialize_with, sizeof(af->serde_serialize_with),
                     "ex");
          CDD_STRCPY(af->serde_deserialize_with,
                     sizeof(af->serde_deserialize_with), "ex");
          af->cli_short = 'x';
          CDD_STRCPY(af->cli_short_aliases, sizeof(af->cli_short_aliases), "x");
          CDD_STRCPY(af->cli_long, sizeof(af->cli_long), "ex");
          af->cli_has_long = 1;
          af->cli_index = 1;
          CDD_STRCPY(af->cli_value_name, sizeof(af->cli_value_name), "ex");
          CDD_STRCPY(af->cli_help, sizeof(af->cli_help), "ex");
          g_cdd_last_alloc_err = c_cdd_strdup("ex", &af->cli_long_help);
          CDD_STRCPY(af->cli_help_heading, sizeof(af->cli_help_heading), "ex");
          CDD_STRCPY(af->cli_env, sizeof(af->cli_env), "ex");
          CDD_STRCPY(af->cli_default_missing_val,
                     sizeof(af->cli_default_missing_val), "ex");
          af->cli_action = CDD_CLI_ACTION_SET_TRUE;
          af->cli_value_hint = CDD_CLI_HINT_DIR_PATH;
          af->cli_value_delimiter = ':';
          af->cli_num_args_min = 2;
          af->cli_num_args_max = 3;
          CDD_STRCPY(af->cli_value_parser, sizeof(af->cli_value_parser), "ex");
          CDD_STRCPY(af->cli_group, sizeof(af->cli_group), "ex");
          CDD_STRCPY(af->cli_required_if_field,
                     sizeof(af->cli_required_if_field), "ex");
        }

        ASSERT_EQ(CDD_C_SUCCESS, c2s_collapse_arrays(&csf));
        g_cdd_last_alloc_err = struct_fields_free(&csf);
      }

      /* OOM on alias, conflict, requires, required_unless in
       * c2s_collapse_arrays
       */
      for (k = 1; k <= 4; ++k) {
        struct StructFields csf;
        struct StructField *nf;
        g_cdd_last_alloc_err = struct_fields_init(&csf);
        g_cdd_last_alloc_err =
            struct_fields_add(&csf, "n_items", "integer", NULL, NULL, NULL);
        g_cdd_last_alloc_err =
            struct_fields_add(&csf, "items", "string", NULL, NULL, NULL);
        nf = &csf.fields[0];
        g_cdd_last_alloc_err = struct_field_add_alias(nf, "al1");
        g_cdd_last_alloc_err = struct_field_add_cli_conflict(nf, "c1");
        g_cdd_last_alloc_err = struct_field_add_cli_requires(nf, "r1");
        g_cdd_last_alloc_err = struct_field_add_cli_required_unless(nf, "ru1");
        g_cdd_alloc_fail = k;
        ASSERT_EQ(CDD_C_ERROR_MEMORY, c2s_collapse_arrays(&csf));
        g_cdd_alloc_fail = 0;
        g_cdd_last_alloc_err = struct_fields_free(&csf);
      }

      /* merge_struct_field and merge_struct_fields exhaustive coverage */
      {
        int m_mode;
        for (m_mode = 0; m_mode < 3; ++m_mode) {
          struct StructFields src_sf, dst_sf;
          struct StructField *sf_f;
          g_cdd_last_alloc_err = struct_fields_init(&src_sf);
          g_cdd_last_alloc_err = struct_fields_init(&dst_sf);

          CDD_STRCPY(src_sf.serde_rename, sizeof(src_sf.serde_rename), "SR");
          src_sf.serde_rename_all = CDD_SERDE_CASE_CAMELCASE;
          src_sf.serde_rename_all_ser = CDD_SERDE_CASE_SNAKECASE;
          src_sf.serde_rename_all_de = CDD_SERDE_CASE_PASCALCASE;
          CDD_STRCPY(src_sf.serde_replace_from,
                     sizeof(src_sf.serde_replace_from), "a");
          CDD_STRCPY(src_sf.serde_replace_to, sizeof(src_sf.serde_replace_to),
                     "b");
          CDD_STRCPY(src_sf.serde_prefix, sizeof(src_sf.serde_prefix), "p");
          CDD_STRCPY(src_sf.serde_suffix, sizeof(src_sf.serde_suffix), "s");
          src_sf.serde_deny_unknown_fields = 1;
          src_sf.serde_default = 1;
          CDD_STRCPY(src_sf.serde_tag, sizeof(src_sf.serde_tag), "t");
          CDD_STRCPY(src_sf.serde_content, sizeof(src_sf.serde_content), "c");
          src_sf.serde_untagged = 1;
          src_sf.is_cli_command = 1;
          CDD_STRCPY(src_sf.cli_name, sizeof(src_sf.cli_name), "cn");
          CDD_STRCPY(src_sf.cli_bin_name, sizeof(src_sf.cli_bin_name), "cb");
          CDD_STRCPY(src_sf.cli_version, sizeof(src_sf.cli_version), "1.0");
          CDD_STRCPY(src_sf.cli_author, sizeof(src_sf.cli_author), "au");
          CDD_STRCPY(src_sf.cli_about, sizeof(src_sf.cli_about), "ab");
          g_cdd_last_alloc_err = c_cdd_strdup("la", &src_sf.cli_long_about);
          g_cdd_last_alloc_err = c_cdd_strdup("bh", &src_sf.cli_before_help);
          g_cdd_last_alloc_err = c_cdd_strdup("ah", &src_sf.cli_after_help);
          CDD_STRCPY(src_sf.cli_usage_override,
                     sizeof(src_sf.cli_usage_override), "uo");
          src_sf.cli_rename_all = CDD_SERDE_CASE_KEBABCASE;
          src_sf.cli_rename_all_env = CDD_SERDE_CASE_UPPERCASE;
          CDD_STRCPY(src_sf.cli_env_prefix, sizeof(src_sf.cli_env_prefix),
                     "EP");
          src_sf.cli_arg_required_else_help = 1;
          src_sf.cli_subcommand_required = 1;
          src_sf.cli_allow_external_subcommands = 1;
          src_sf.cli_disable_help_flag = 1;
          src_sf.cli_disable_version_flag = 1;
          src_sf.cli_disable_help_subcommand = 1;
          src_sf.cli_propagate_version = 1;
          src_sf.cli_allow_windows_slash_flags = 1;
          src_sf.cli_allow_response_files = 1;
          src_sf.cli_case_insensitive_flags = 1;
          g_cdd_last_alloc_err = struct_fields_add_cli_alias(&src_sf, "ca1");
          src_sf.cli_aliases =
              (char **)realloc(src_sf.cli_aliases, 2 * sizeof(char *));
          src_sf.cli_aliases[1] = NULL;
          src_sf.n_cli_aliases = 2;

          g_cdd_last_alloc_err =
              struct_fields_add(&src_sf, "f1", "string", "MyRef", "def", "8");
          sf_f = &src_sf.fields[0];
          sf_f->required = 1;
          sf_f->has_min = 1;
          sf_f->min_val = 10;
          sf_f->exclusive_min = 1;
          sf_f->has_max = 1;
          sf_f->max_val = 20;
          sf_f->exclusive_max = 1;
          sf_f->has_min_len = 1;
          sf_f->min_len = 5;
          sf_f->has_max_len = 1;
          sf_f->max_len = 15;
          sf_f->has_min_items = 1;
          sf_f->min_items = 2;
          sf_f->has_max_items = 1;
          sf_f->max_items = 8;
          sf_f->unique_items = 1;
          CDD_STRCPY(sf_f->pattern, sizeof(sf_f->pattern), "^a");
          sf_f->is_flexible_array = 1;
          g_cdd_last_alloc_err =
              c_cdd_strdup("{\"x\":1}", &sf_f->schema_extra_json);
          g_cdd_last_alloc_err =
              c_cdd_strdup("{\"y\":1}", &sf_f->items_extra_json);
          sf_f->type_union = (char **)calloc(1, sizeof(char *));
          g_cdd_last_alloc_err = c_cdd_strdup("string", &sf_f->type_union[0]);
          sf_f->n_type_union = 1;
          sf_f->items_type_union = (char **)calloc(1, sizeof(char *));
          g_cdd_last_alloc_err =
              c_cdd_strdup("integer", &sf_f->items_type_union[0]);
          sf_f->n_items_type_union = 1;
          CDD_STRCPY(sf_f->serde_rename, sizeof(sf_f->serde_rename), "ren");
          CDD_STRCPY(sf_f->serde_rename_ser, sizeof(sf_f->serde_rename_ser),
                     "rser");
          CDD_STRCPY(sf_f->serde_rename_de, sizeof(sf_f->serde_rename_de),
                     "rde");
          sf_f->serde_case = CDD_SERDE_CASE_LOWERCASE;
          CDD_STRCPY(sf_f->serde_replace_from, sizeof(sf_f->serde_replace_from),
                     "a");
          CDD_STRCPY(sf_f->serde_replace_to, sizeof(sf_f->serde_replace_to),
                     "b");
          sf_f->serde_skip = 1;
          sf_f->serde_skip_serializing = 1;
          sf_f->serde_skip_deserializing = 1;
          sf_f->serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;
          CDD_STRCPY(sf_f->serde_skip_if_fn, sizeof(sf_f->serde_skip_if_fn),
                     "fn");
          sf_f->serde_flatten = 1;
          sf_f->serde_has_default = 1;
          CDD_STRCPY(sf_f->serde_default_fn, sizeof(sf_f->serde_default_fn),
                     "dfn");
          CDD_STRCPY(sf_f->serde_with, sizeof(sf_f->serde_with), "w");
          CDD_STRCPY(sf_f->serde_serialize_with,
                     sizeof(sf_f->serde_serialize_with), "sw");
          CDD_STRCPY(sf_f->serde_deserialize_with,
                     sizeof(sf_f->serde_deserialize_with), "dw");
          g_cdd_last_alloc_err = struct_field_add_alias(sf_f, "al1");
          sf_f->serde_aliases =
              (char **)realloc(sf_f->serde_aliases, 2 * sizeof(char *));
          sf_f->serde_aliases[1] = NULL;
          sf_f->n_serde_aliases = 2;
          sf_f->cli_short = 's';
          CDD_STRCPY(sf_f->cli_short_aliases, sizeof(sf_f->cli_short_aliases),
                     "ab");
          CDD_STRCPY(sf_f->cli_long, sizeof(sf_f->cli_long), "long");
          sf_f->cli_has_long = 1;
          sf_f->cli_negatable = 1;
          sf_f->cli_positional = 1;
          sf_f->cli_index = 0;
          sf_f->cli_last = 1;
          sf_f->cli_trailing_var_arg = 1;
          CDD_STRCPY(sf_f->cli_value_name, sizeof(sf_f->cli_value_name), "VN");
          CDD_STRCPY(sf_f->cli_help, sizeof(sf_f->cli_help), "H");
          g_cdd_last_alloc_err = c_cdd_strdup("LH", &sf_f->cli_long_help);
          CDD_STRCPY(sf_f->cli_help_heading, sizeof(sf_f->cli_help_heading),
                     "HH");
          CDD_STRCPY(sf_f->cli_env, sizeof(sf_f->cli_env), "ENV");
          sf_f->cli_hide_env = 1;
          sf_f->cli_hide_env_values = 1;
          CDD_STRCPY(sf_f->cli_default_missing_val,
                     sizeof(sf_f->cli_default_missing_val), "DMV");
          sf_f->cli_hide_default_value = 1;
          sf_f->cli_action = CDD_CLI_ACTION_COUNT;
          sf_f->cli_value_hint = CDD_CLI_HINT_FILE_PATH;
          sf_f->cli_value_delimiter = ',';
          sf_f->cli_num_args_min = 1;
          sf_f->cli_num_args_max = 2;
          sf_f->cli_require_equals = 1;
          sf_f->cli_allow_hyphen_values = 1;
          sf_f->cli_global = 1;
          sf_f->cli_hidden = 1;
          sf_f->cli_exclusive = 1;
          sf_f->cli_subcommand = 1;
          CDD_STRCPY(sf_f->cli_value_parser, sizeof(sf_f->cli_value_parser),
                     "vp");
          CDD_STRCPY(sf_f->cli_group, sizeof(sf_f->cli_group), "grp");
          CDD_STRCPY(sf_f->cli_required_if_field,
                     sizeof(sf_f->cli_required_if_field), "rif");
          CDD_STRCPY(sf_f->cli_required_if_val,
                     sizeof(sf_f->cli_required_if_val), "riv");
          g_cdd_last_alloc_err = struct_field_add_cli_conflict(sf_f, "c1");
          sf_f->cli_conflicts_with =
              (char **)realloc(sf_f->cli_conflicts_with, 2 * sizeof(char *));
          sf_f->cli_conflicts_with[1] = NULL;
          sf_f->n_cli_conflicts_with = 2;
          g_cdd_last_alloc_err = struct_field_add_cli_requires(sf_f, "r1");
          sf_f->cli_requires =
              (char **)realloc(sf_f->cli_requires, 2 * sizeof(char *));
          sf_f->cli_requires[1] = NULL;
          sf_f->n_cli_requires = 2;
          g_cdd_last_alloc_err =
              struct_field_add_cli_required_unless(sf_f, "ru1");
          sf_f->cli_required_unless =
              (char **)realloc(sf_f->cli_required_unless, 2 * sizeof(char *));
          sf_f->cli_required_unless[1] = NULL;
          sf_f->n_cli_required_unless = 2;

          if (m_mode == 1) {
            /* dst_sf has f1 empty, so merge_struct_field copies into empty f1
             */
            g_cdd_last_alloc_err =
                struct_fields_add(&dst_sf, "f1", "string", NULL, NULL, NULL);
          } else if (m_mode == 2) {
            /* dst_sf already has all attributes populated */
            ASSERT_EQ(CDD_C_SUCCESS, merge_struct_fields(&dst_sf, &src_sf));
          }

          ASSERT_EQ(CDD_C_SUCCESS, merge_struct_fields(&dst_sf, &src_sf));
          if (m_mode == 0) {
            for (k = 1; k <= 20; ++k) {
              struct StructFields oom_dst;
              g_cdd_last_alloc_err = struct_fields_init(&oom_dst);
              g_cdd_strdup_fail = k;
              g_cdd_last_alloc_err = merge_struct_fields(&oom_dst, &src_sf);
              g_cdd_strdup_fail = 0;
              g_cdd_last_alloc_err = struct_fields_free(&oom_dst);
            }
            for (k = 1; k <= 20; ++k) {
              struct StructFields oom_dst;
              g_cdd_last_alloc_err = struct_fields_init(&oom_dst);
              g_cdd_alloc_fail = k;
              g_cdd_last_alloc_err = merge_struct_fields(&oom_dst, &src_sf);
              g_cdd_alloc_fail = 0;
              g_cdd_last_alloc_err = struct_fields_free(&oom_dst);
            }
          }
          g_cdd_last_alloc_err = struct_fields_free(&src_sf);
          g_cdd_last_alloc_err = struct_fields_free(&dst_sf);
        }

        /* merge_struct_field NULL and constraint comparison branches */
        {
          struct StructField df, sf_m;
          memset(&df, 0, sizeof(df));
          memset(&sf_m, 0, sizeof(sf_m));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    merge_struct_field(NULL, &sf_m));
          ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                    merge_struct_field(&df, NULL));
          /* 1. Equal min/max where src is exclusive and dest is not */
          df.has_min = 1;
          df.min_val = 10;
          df.exclusive_min = 0;
          sf_m.has_min = 1;
          sf_m.min_val = 10;
          sf_m.exclusive_min = 1;
          df.has_max = 1;
          df.max_val = 20;
          df.exclusive_max = 0;
          sf_m.has_max = 1;
          sf_m.max_val = 20;
          sf_m.exclusive_max = 1;
          ASSERT_EQ(CDD_C_SUCCESS, merge_struct_field(&df, &sf_m));
          /* Repeat when df.exclusive_min/max is already 1 */
          ASSERT_EQ(CDD_C_SUCCESS, merge_struct_field(&df, &sf_m));
          /* Repeat when sf_m.exclusive_min/max is 0 */
          sf_m.exclusive_min = 0;
          sf_m.exclusive_max = 0;
          ASSERT_EQ(CDD_C_SUCCESS, merge_struct_field(&df, &sf_m));

          /* 2. Stricter src min/max/len/items (> min, < max) */
          sf_m.min_val = 15;
          sf_m.max_val = 18;
          df.has_min_len = 1;
          df.min_len = 2;
          sf_m.has_min_len = 1;
          sf_m.min_len = 5;
          df.has_max_len = 1;
          df.max_len = 10;
          sf_m.has_max_len = 1;
          sf_m.max_len = 8;
          df.has_min_items = 1;
          df.min_items = 1;
          sf_m.has_min_items = 1;
          sf_m.min_items = 3;
          df.has_max_items = 1;
          df.max_items = 10;
          sf_m.has_max_items = 1;
          sf_m.max_items = 6;
          ASSERT_EQ(CDD_C_SUCCESS, merge_struct_field(&df, &sf_m));

          /* 3. Looser src min/max/len/items (< min, > max) */
          sf_m.min_val = 5;
          sf_m.max_val = 25;
          sf_m.min_len = 1;
          sf_m.max_len = 20;
          sf_m.min_items = 1;
          sf_m.max_items = 20;
          ASSERT_EQ(CDD_C_SUCCESS, merge_struct_field(&df, &sf_m));
        }

        /* apply_allof_to_struct_fields edge cases */
        {
          struct StructFields asf;
          JSON_Value *av = NULL;
          JSON_Value *rv = NULL;
          {
            JSON_Value *_cdd_jv_1 = NULL;
            g_cdd_last_alloc_err = cdd_json_parse_string(
                "[null,{\"$ref\":\"#/Missing\"},{\"$ref\":\"\"},"
                "{\"properties\":{\"x\":{\"type\":\"string\"}}}]",
                &_cdd_jv_1);
            av = _cdd_jv_1;
          }
          {
            JSON_Value *_cdd_jv_1 = NULL;
            g_cdd_last_alloc_err = cdd_json_parse_string(
                "{\"Valid\":{\"properties\":{\"y\":{\"type\":\"integer\"}}}}",
                &_cdd_jv_1);
            rv = _cdd_jv_1;
          }
          g_cdd_last_alloc_err = struct_fields_init(&asf);
          ASSERT_EQ(CDD_C_SUCCESS,
                    apply_allof_to_struct_fields(NULL, &asf, NULL));
          ASSERT_EQ(CDD_C_SUCCESS, apply_allof_to_struct_fields(
                                       json_value_get_array(av), NULL, NULL));
          ASSERT_EQ(CDD_C_SUCCESS,
                    apply_allof_to_struct_fields(json_value_get_array(av), &asf,
                                                 json_value_get_object(rv)));
          g_cdd_alloc_fail = 1;
          ASSERT_EQ(CDD_C_ERROR_MEMORY,
                    apply_allof_to_struct_fields(json_value_get_array(av), &asf,
                                                 NULL));
          g_cdd_alloc_fail = 0;
          g_json_object_to_struct_fields_fail = 1;
          ASSERT_EQ(CDD_C_ERROR_MEMORY,
                    apply_allof_to_struct_fields(json_value_get_array(av), &asf,
                                                 NULL));
          g_json_object_to_struct_fields_fail = 0;
          g_cdd_last_alloc_err = struct_fields_free(&asf);
          g_cdd_last_alloc_err = json_value_free(av);
          g_cdd_last_alloc_err = json_value_free(rv);
        }

        /* Round-trip all extended CLI, Enum, C-type, and C23/GNU attribute
         * reflection extensions */
        {
          struct StructFields rt_sf, rt_out, rt_esf, rt_eout;
          struct StructField *rf;
          JSON_Value *schemas_v = NULL;
          JSON_Object *schemas_o = NULL;
          g_cdd_last_alloc_err = cdd_json_value_init_object(&schemas_v);
          schemas_o = json_value_get_object(schemas_v);

          g_cdd_last_alloc_err = struct_fields_init(&rt_esf);
          rt_esf.is_enum = 1;
          g_cdd_last_alloc_err =
              enum_members_add_ex(&rt_esf.enum_members, "VAL_A", "val_a", 0);
          g_cdd_last_alloc_err =
              enum_members_add_ex(&rt_esf.enum_members, "VAL_B", "val_b", 0);
          g_cdd_last_alloc_err =
              enum_members_add_ex(&rt_esf.enum_members, "VAL_C", "val_c", 0);
          g_cdd_last_alloc_err =
              enum_member_add_alias(&rt_esf.enum_members, 0, "alias_a");
          g_cdd_last_alloc_err =
              enum_member_set_cli_meta(&rt_esf.enum_members, 0, "Help A", 0);
          ASSERT_EQ(CDD_C_SUCCESS, write_struct_to_json_schema(
                                       schemas_o, "DescOnlyEnum", &rt_esf));
          g_cdd_last_alloc_err = struct_fields_init(&rt_eout);
          {
            JSON_Object *_cdd_jo_1 = NULL;
            g_cdd_last_alloc_err = cdd_json_object_get_object(
                schemas_o, "DescOnlyEnum", &_cdd_jo_1);
            ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(
                                         _cdd_jo_1, &rt_eout, schemas_o));
          }
          g_cdd_last_alloc_err = struct_fields_free(&rt_eout);

          g_cdd_last_alloc_err =
              enum_member_set_cli_meta(&rt_esf.enum_members, 0, "Help A", 1);
          g_cdd_last_alloc_err =
              enum_member_set_cli_meta(&rt_esf.enum_members, 1, NULL, 1);
          ASSERT_EQ(CDD_C_SUCCESS, write_struct_to_json_schema(
                                       schemas_o, "RichEnum", &rt_esf));
          g_cdd_last_alloc_err = struct_fields_init(&rt_eout);
          {
            JSON_Object *_cdd_jo_1 = NULL;
            g_cdd_last_alloc_err =
                cdd_json_object_get_object(schemas_o, "RichEnum", &_cdd_jo_1);
            ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(
                                         _cdd_jo_1, &rt_eout, schemas_o));
          }
          ASSERT_EQ(1, rt_eout.enum_members.n_serde_aliases[0]);
          ASSERT_STR_EQ("alias_a", rt_eout.enum_members.serde_aliases[0][0]);
          ASSERT_STR_EQ("Help A", rt_eout.enum_members.cli_descriptions[0]);
          ASSERT_EQ(1, rt_eout.enum_members.cli_hidden[0]);

          g_cdd_last_alloc_err = struct_fields_init(&rt_sf);
          rt_sf.is_cli_command = 1;
          CDD_STRCPY(rt_sf.cli_bin_name, sizeof(rt_sf.cli_bin_name), "my-bin");
          g_cdd_last_alloc_err =
              c_cdd_strdup("Long about text", &rt_sf.cli_long_about);
          g_cdd_last_alloc_err =
              c_cdd_strdup("Before help", &rt_sf.cli_before_help);
          g_cdd_last_alloc_err =
              c_cdd_strdup("After help", &rt_sf.cli_after_help);
          CDD_STRCPY(rt_sf.cli_usage_override, sizeof(rt_sf.cli_usage_override),
                     "my-bin [ARGS]");
          rt_sf.cli_rename_all_env = CDD_SERDE_CASE_SCREAMING_SNAKECASE;
          rt_sf.cli_arg_required_else_help = 1;
          rt_sf.cli_subcommand_required = 1;
          rt_sf.cli_allow_external_subcommands = 1;
          rt_sf.cli_disable_help_flag = 1;
          rt_sf.cli_disable_version_flag = 1;
          rt_sf.cli_disable_help_subcommand = 1;
          rt_sf.cli_propagate_version = 1;
          rt_sf.cli_allow_windows_slash_flags = 1;
          rt_sf.cli_allow_response_files = 1;
          rt_sf.cli_case_insensitive_flags = 1;
          g_cdd_last_alloc_err = struct_fields_add_cli_alias(&rt_sf, "mb");

          g_cdd_last_alloc_err =
              struct_fields_add(&rt_sf, "arg1", "string", NULL, NULL, NULL);
          rf = &rt_sf.fields[0];
          CDD_STRCPY(rf->cli_short_aliases, sizeof(rf->cli_short_aliases),
                     "ab");
          rf->cli_last = 1;
          rf->cli_trailing_var_arg = 1;
          g_cdd_last_alloc_err =
              c_cdd_strdup("Long field help", &rf->cli_long_help);
          CDD_STRCPY(rf->cli_help_heading, sizeof(rf->cli_help_heading), "ADV");
          rf->cli_hide_env = 1;
          rf->cli_hide_env_values = 1;
          CDD_STRCPY(rf->cli_default_missing_val,
                     sizeof(rf->cli_default_missing_val), "auto");
          rf->cli_hide_default_value = 1;
          rf->cli_value_hint = CDD_CLI_HINT_FILE_PATH;
          rf->cli_num_args_min = 1;
          rf->cli_num_args_max = 3;
          rf->cli_require_equals = 1;
          rf->cli_allow_hyphen_values = 1;
          rf->cli_global = 1;
          rf->cli_hidden = 1;
          rf->cli_exclusive = 1;
          CDD_STRCPY(rf->cli_value_parser, sizeof(rf->cli_value_parser),
                     "parse_custom");
          CDD_STRCPY(rf->cli_group, sizeof(rf->cli_group), "grp1");
          CDD_STRCPY(rf->cli_required_if_field,
                     sizeof(rf->cli_required_if_field), "mode");
          CDD_STRCPY(rf->cli_required_if_val, sizeof(rf->cli_required_if_val),
                     "strict");
          g_cdd_last_alloc_err =
              struct_field_add_cli_required_unless(rf, "fallback");

          /* Test C23 [[cdd(...)]] / [[serde(...)]] / [[cli(...)]] attributes */
          g_cdd_last_alloc_err = cdd_serde_parse_field_comment(
              "[[cdd(cdd::c_type = \"int64_t\", cdd::nullable_ptr = true, "
              "cdd::fixed_array_len = 16)]] "
              "[[serde(serde::rename = \"arg_one\", nullable_ptr)]] "
              "[[cli(cli::hide)]] [[other]]",
              rf);
          ASSERT_STR_EQ("int64_t", rf->c_type);
          ASSERT_EQ(1, rf->is_nullable_ptr);
          ASSERT_EQ(16, rf->fixed_array_len);

          g_cdd_last_alloc_err =
              struct_fields_add(&rt_sf, "earr", "array", "string", NULL, NULL);
          rt_sf.fields[1].is_enum_array = 1;
          CDD_STRCPY(rt_sf.fields[1].enum_array_ref,
                     sizeof(rt_sf.fields[1].enum_array_ref), "RichEnum");

          {
            struct StructFields sub_sf;
            g_cdd_last_alloc_err = struct_fields_init(&sub_sf);
            g_cdd_last_alloc_err = struct_fields_add(
                &sub_sf, "sub_prop", "string", NULL, NULL, NULL);
            ASSERT_EQ(CDD_C_SUCCESS, write_struct_to_json_schema(
                                         schemas_o, "FlatSubObj", &sub_sf));
            g_cdd_last_alloc_err = struct_fields_free(&sub_sf);
          }
          g_cdd_last_alloc_err =
              struct_fields_add(&rt_sf, "flat_sub", "object",
                                "#/components/schemas/FlatSubObj", NULL, NULL);
          rt_sf.fields[2].serde_flatten = 1;
          g_cdd_last_alloc_err =
              struct_fields_add(&rt_sf, "flat_missing", "object",
                                "#/components/schemas/MissingObj", NULL, NULL);
          rt_sf.fields[3].serde_flatten = 1;

          ASSERT_EQ(CDD_C_SUCCESS,
                    write_struct_to_json_schema(schemas_o, "RichCmd", &rt_sf));
          g_cdd_last_alloc_err = struct_fields_init(&rt_out);
          {
            JSON_Object *_cdd_jo_1 = NULL;
            g_cdd_last_alloc_err =
                cdd_json_object_get_object(schemas_o, "RichCmd", &_cdd_jo_1);
            ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(
                                         _cdd_jo_1, &rt_out, schemas_o));
          }
          ASSERT_STR_EQ("my-bin", rt_out.cli_bin_name);
          ASSERT_EQ(1, rt_out.n_cli_aliases);
          ASSERT_STR_EQ("int64_t", rt_out.fields[0].c_type);
          ASSERT_EQ(1, rt_out.fields[0].is_nullable_ptr);
          ASSERT_EQ(16, rt_out.fields[0].fixed_array_len);
          ASSERT_EQ(1, rt_out.fields[1].is_enum_array);
          ASSERT_STR_EQ("RichEnum", rt_out.fields[1].enum_array_ref);

          g_cdd_last_alloc_err = struct_fields_free(&rt_sf);
          g_cdd_last_alloc_err = struct_fields_free(&rt_out);
          g_cdd_last_alloc_err = struct_fields_free(&rt_esf);
          g_cdd_last_alloc_err = struct_fields_free(&rt_eout);
          g_cdd_last_alloc_err = json_value_free(schemas_v);
        }
      }
    }
  }

  PASS();
}

/**
 * @brief Executes the code2schema_internals_reach_suite test suite.
 */
SUITE(code2schema_internals_reach_suite) {
  RUN_TEST(test_code2schema_reach_100_percent_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_REACH_H */
