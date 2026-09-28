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
 * @brief Targeted tests to reach 100% branch coverage in code2schema.c.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_reach_100_percent_coverage(void) {
  /* 1. Helper fail counters decrementing to non-zero and then zero */
  {
    char **dst = NULL;
    size_t dst_count = 0;
    char *src_arr[1];
    src_arr[0] = (char *)(size_t) "item";
    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS,
              copy_string_array_code2schema(&dst, &dst_count, src_arr, 1));
    free_string_array_code2schema(dst, dst_count);
    dst = NULL;
    dst_count = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              copy_string_array_code2schema(&dst, &dst_count, src_arr, 1));
    g_c2s_helper_fail = 0;
  }
  {
    const char *out_str = NULL;
    char buf[32];
    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_strip_quotes("\"hello\"", buf, sizeof(buf), &out_str));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_strip_quotes("\"hello\"", buf, sizeof(buf), &out_str));
    g_c2s_helper_fail = 0;
  }
  {
    enum UnionVariantJsonType utype;
    JSON_Value *jv = json_value_init_object();
    g_c2s_helper_fail = 3;
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_detect_union_json_type(json_value_get_object(jv), &utype));
    g_c2s_helper_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_detect_union_json_type(json_value_get_object(jv), &utype));
    json_value_free(jv);
    g_c2s_helper_fail = 0;
  }
  {
    JSON_Value *req_jv = json_value_init_array();
    int in_list = 0;
    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS, required_name_in_list(json_value_get_array(req_jv),
                                                   "a", &in_list));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        required_name_in_list(json_value_get_array(req_jv), "a", &in_list));
    json_value_free(req_jv);
    g_c2s_helper_fail = 0;
  }
  {
    struct StructField dest_f, src_f;
    memset(&dest_f, 0, sizeof(dest_f));
    memset(&src_f, 0, sizeof(src_f));
    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS, merge_struct_field(&dest_f, &src_f));
    g_c2s_helper_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, merge_struct_field(&dest_f, &src_f));
    g_c2s_helper_fail = 0;
  }
  {
    FILE *fp = fopen("cdd_test_tmp_union.txt", "w");
    JSON_Value *schemas_v = json_value_init_object();
    fputs("int a;\n}\n", fp);
    fclose(fp);
    fp = fopen("cdd_test_tmp_union.txt", "r");
    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_parse_union_and_write(fp, json_value_get_object(schemas_v),
                                        "MyUnion"));
    fclose(fp);
    fp = fopen("cdd_test_tmp_union.txt", "r");
    g_c2s_helper_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_parse_union_and_write(fp, json_value_get_object(schemas_v),
                                        "MyUnion"));
    fclose(fp);
    json_value_free(schemas_v);
    remove("cdd_test_tmp_union.txt");
    g_c2s_helper_fail = 0;
  }

  /* 2. parse_type_union_array_code2schema */
  {
    JSON_Value *arr_v = json_parse_string("[\"string\", \"integer\"]");
    char **u_types = NULL;
    size_t n_u = 0;
    const char *prim = NULL;
    int nullable = 0;
    ASSERT_EQ(CDD_C_SUCCESS, parse_type_union_array_code2schema(
                                 json_value_get_array(arr_v), &u_types, &n_u,
                                 &prim, &nullable));
    free_string_array_code2schema(u_types, n_u);
    json_value_free(arr_v);

    arr_v = json_parse_string("[\"null\"]");
    u_types = NULL;
    n_u = 0;
    prim = NULL;
    nullable = 0;
    ASSERT_EQ(CDD_C_SUCCESS, parse_type_union_array_code2schema(
                                 json_value_get_array(arr_v), &u_types, &n_u,
                                 &prim, &nullable));
    free_string_array_code2schema(u_types, n_u);
    json_value_free(arr_v);
  }

  /* 3. g_cdd_fail_json_set_value in c2s_merge_schema_extras_strings */
  {
    char *extras = NULL;
    c_cdd_strdup("{\"orig\":1}", &extras);
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
    struct_fields_free(&sf);
  }

  /* 4b. parse_struct_member_line helper fail */
  {
    struct StructFields sf_hf;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_hf));
    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int a;", &sf_hf));
    g_c2s_helper_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_struct_member_line("int a;", &sf_hf));
    g_c2s_helper_fail = 0;
    struct_fields_free(&sf_hf);
  }

  /* 5. Pointer with * in raw type */
  {
    struct StructFields sf;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("char * *ptr;", &sf));
    ASSERT_EQ(CDD_C_SUCCESS, parse_struct_member_line("int * *pp;", &sf));
    struct_fields_free(&sf);
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
    struct_fields_free(&sf);
  }

  /* 7. json_object_to_struct_fields properties */
  {
    JSON_Value *schema;
    struct StructFields sf;

    schema = json_parse_string("{\"properties\": {\"x\": {\"default\": 10}}}");
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(
                                 json_value_get_object(schema), &sf, NULL));
    struct_fields_free(&sf);
    json_value_free(schema);

    schema =
        json_parse_string("{\"properties\": {\"arr\": {\"type\": \"array\"}}}");
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(
                                 json_value_get_object(schema), &sf, NULL));
    struct_fields_free(&sf);
    json_value_free(schema);

    schema =
        json_parse_string("{\"properties\": {\"n\": {\"type\": \"number\", "
                          "\"exclusiveMaximum\": false}}}");
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields(
                                 json_value_get_object(schema), &sf, NULL));
    struct_fields_free(&sf);
    json_value_free(schema);
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
    int b = 0;
    double nval = 0;
    ASSERT_EQ(CDD_C_SUCCESS, c2s_parse_bool_default("true", NULL, &has_val));
    ASSERT_EQ(CDD_C_SUCCESS, c2s_parse_number_default("123", NULL, &has_nval));
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_parse_number_default("3.14", &nval, &has_nval));
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_parse_number_default("not_a_num", &nval, &has_nval));
    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS, c2s_parse_bool_default("true", &b, &has_val));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_parse_bool_default("true", &b, &has_val));
    g_c2s_helper_fail = 0;
  }

  /* c2s_collapse_arrays loop exit */
  {
    struct StructFields sf_coll;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_coll));
    ASSERT_EQ(0, struct_fields_add(&sf_coll, "n_foo", "integer", "", NULL, ""));
    ASSERT_EQ(0, struct_fields_add(&sf_coll, "bar", "string", "", NULL, ""));
    ASSERT_EQ(CDD_C_SUCCESS, c2s_collapse_arrays(&sf_coll));
    struct_fields_free(&sf_coll);
  }

  /* 10. ref_points_to_string_enum */
  {
    JSON_Value *root_v = json_value_init_object();
    int pts = 0;
    ASSERT_EQ(CDD_C_SUCCESS, ref_points_to_string_enum(
                                 json_value_get_object(root_v), NULL, &pts));
    ASSERT_EQ(CDD_C_SUCCESS,
              ref_points_to_string_enum(json_value_get_object(root_v),
                                        "#/components/", &pts));
    json_value_free(root_v);
  }

  /* 11. required_name_in_list */
  {
    JSON_Value *req_v = json_parse_string("[\"a\", 123]");
    int in_l = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              required_name_in_list(json_value_get_array(req_v), NULL, &in_l));
    ASSERT_EQ(CDD_C_SUCCESS,
              required_name_in_list(json_value_get_array(req_v), "b", &in_l));
    json_value_free(req_v);
  }

  /* 12. resolve_schema_ref_object */
  {
    JSON_Value *root_v = json_value_init_object();
    JSON_Object *out_obj = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              resolve_schema_ref_object(json_value_get_object(root_v), NULL,
                                        &out_obj));
    ASSERT_EQ(CDD_C_SUCCESS,
              resolve_schema_ref_object(json_value_get_object(root_v),
                                        "#/components/", &out_obj));
    json_value_free(root_v);
  }

  /* 13. c2s_collect_string_array with non-string */
  {
    JSON_Value *arr_v = json_parse_string("[123, \"valid\"]");
    char **arr_out = NULL;
    size_t cnt = 0;
    ASSERT_EQ(CDD_C_SUCCESS, c2s_collect_string_array(
                                 json_value_get_array(arr_v), &arr_out, &cnt));
    free_string_array_code2schema(arr_out, cnt);
    json_value_free(arr_v);
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
    JSON_Value *sch = json_parse_string("{\"mapping\": {\"key\": 123}}");
    char *disc_val = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              discriminator_value_for_variant(json_value_get_object(sch),
                                              "schema", "ref", &disc_val));
    C_CDD_FREE(disc_val);
    json_value_free(sch);
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
    JSON_Value *allof_v = json_parse_string("[{}]");
    JSON_Value *allof_ref =
        json_parse_string("[{\"$ref\": \"#/components/schemas/Foo\"}]");
    struct StructFields sf;
    ASSERT_EQ(CDD_C_SUCCESS, apply_allof_to_struct_fields(
                                 json_value_get_array(allof_v), NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_allof_to_struct_fields(
                                 json_value_get_array(allof_ref), &sf, NULL));
    struct_fields_free(&sf);
    json_value_free(allof_ref);
    json_value_free(allof_v);

    allof_v = json_parse_string("[{}]");
    allof_ref = json_parse_string("[{\"$ref\": \"#/components/schemas/Foo\"}]");
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_fallback(
                                 json_value_get_array(allof_v), NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_fallback(
                                 json_value_get_array(allof_ref), &sf, NULL));
    struct_fields_free(&sf);
    json_value_free(allof_ref);
    json_value_free(allof_v);
  }

  /* 18b. Error percolation with helper fail */
  {
    JSON_Value *allof_ref =
        json_parse_string("[{\"$ref\": \"#/components/schemas/Foo\"}]");
    JSON_Value *root_v = json_value_init_object();
    enum UnionVariantJsonType ut;
    struct StructFields sf_err;

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_err));
    g_c2s_helper_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              apply_allof_to_struct_fields(json_value_get_array(allof_ref),
                                           &sf_err,
                                           json_value_get_object(root_v)));
    g_c2s_helper_fail = 0;

    g_c2s_helper_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              apply_union_to_struct_fields_fallback(
                  json_value_get_array(allof_ref), &sf_err,
                  json_value_get_object(root_v)));
    g_c2s_helper_fail = 0;

    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_detect_union_json_type(json_value_get_object(root_v), &ut));
    g_c2s_helper_fail = 0;

    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              json_object_to_struct_fields(json_value_get_object(root_v),
                                           &sf_err, NULL));
    g_c2s_helper_fail = 0;

    struct_fields_free(&sf_err);
    json_value_free(root_v);
    json_value_free(allof_ref);
  }

  /* 19. apply_union_to_struct_fields_ex branches */
  {
    JSON_Value *u_v = json_parse_string("[{\"type\": \"string\"}]");
    JSON_Value *u_ref =
        json_parse_string("[{\"$ref\": \"#/components/schemas/Foo\"}]");
    JSON_Value *sch_disc =
        json_parse_string("{\"discriminator\": {\"propertyName\": \"\"}}");
    JSON_Value *sch_disc_empty = json_parse_string("{\"discriminator\": {}}");
    JSON_Value *u_no_type = json_parse_string("[{\"description\": \"desc\"}]");
    JSON_Value *arr_no_items = json_parse_string("[{\"type\": \"array\"}]");
    JSON_Value *arr_items_ref = json_parse_string(
        "[{\"type\": \"array\", \"items\": {\"$ref\": \"#/Foo\"}}]");
    JSON_Value *arr_items_obj = json_parse_string(
        "[{\"type\": \"array\", \"items\": {\"type\": \"object\"}}]");
    JSON_Value *arr_inline_obj = json_parse_string(
        "[{\"type\": \"array\", \"items\": {\"type\": \"object\", "
        "\"properties\": {\"x\": {\"type\": \"integer\"}}}}]");
    JSON_Value *u_obj = json_parse_string(
        "[{\"type\": \"object\", \"properties\": {\"a\": {\"type\": "
        "\"string\"}}}]");
    JSON_Value *root_val = json_parse_string(
        "{\"components\": {\"schemas\": {\"Foo\": {\"type\": \"string\"}}}}");
    struct StructFields sf;

    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_v), NULL,
                                              NULL, NULL, 0, NULL, 0));
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_ref), &sf,
                                              NULL, NULL, 0, NULL, 0));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_ref), &sf,
                                              json_value_get_object(root_val),
                                              NULL, 0, NULL, 0));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(
                                 json_value_get_array(u_v), &sf, NULL, NULL, 0,
                                 json_value_get_object(sch_disc), 0));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(
                                 json_value_get_array(u_v), &sf, NULL, NULL, 0,
                                 json_value_get_object(sch_disc_empty), 0));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_no_type),
                                              &sf, NULL, NULL, 0, NULL, 0));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_obj), &sf,
                                              NULL, NULL, 0, NULL, 1));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_obj), &sf,
                                              json_value_get_object(root_val),
                                              NULL, 0, NULL, 1));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(json_value_get_array(u_obj), &sf,
                                              json_value_get_object(root_val),
                                              NULL, 0, NULL, 0));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(
                                 json_value_get_array(arr_no_items), &sf, NULL,
                                 NULL, 0, NULL, 1));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(
                                 json_value_get_array(arr_items_obj), &sf, NULL,
                                 NULL, 0, NULL, 1));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(
                                 json_value_get_array(arr_items_ref), &sf, NULL,
                                 NULL, 0, NULL, 1));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(
                  json_value_get_array(arr_inline_obj), &sf,
                  json_value_get_object(root_val), NULL, 0, NULL, 1));
    struct_fields_free(&sf);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_union_to_struct_fields_ex(
                  json_value_get_array(arr_inline_obj), &sf,
                  json_value_get_object(root_val), NULL, 0, NULL, 0));
    struct_fields_free(&sf);

    json_value_free(root_val);
    json_value_free(u_obj);
    json_value_free(arr_inline_obj);
    json_value_free(arr_items_obj);
    json_value_free(arr_items_ref);
    json_value_free(arr_no_items);
    json_value_free(u_no_type);
    json_value_free(sch_disc_empty);
    json_value_free(sch_disc);
    json_value_free(u_ref);
    json_value_free(u_v);
  }

  /* 20. default value and constraint writers */
  {
    JSON_Value *pobj_v = json_value_init_object();
    JSON_Object *pobj = json_value_get_object(pobj_v);
    struct StructField f;
    char *tun[1];
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
    json_value_free(pobj_v);
  }

  /* 21. write_struct_to_json_schema invalid and primitive array ref */
  {
    JSON_Value *sch_v = json_value_init_object();
    struct StructFields sf;
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
    struct_fields_free(&sf);
    json_value_free(sch_v);
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
    sch_v = json_value_init_object();
    ASSERT_EQ(CDD_C_SUCCESS,
              c2s_parse_union_and_write(fp, json_value_get_object(sch_v), "U"));
    fclose(fp);
    json_value_free(sch_v);
    remove("cdd_test_tmp_union.txt");
  }

  /* 23. code2schema_main invalid args and header parsing */
  {
    FILE *fhdr;
    char *av1[2];
    char *av2[2];
    char *argv_full[2];

    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, code2schema_main(2, NULL));
    av1[0] = NULL;
    av1[1] = (char *)(size_t) "out.json";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, code2schema_main(2, av1));

    av2[0] = (char *)(size_t) "in.h";
    av2[1] = NULL;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, code2schema_main(2, av2));

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

    argv_full[0] = (char *)(size_t) "cdd_test_tmp_hdr.h";
    argv_full[1] = (char *)(size_t) "cdd_test_tmp_out.json";
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, argv_full));

    remove("cdd_test_tmp_hdr.h");
    remove("cdd_test_tmp_out.json");
  }

  /* 24. c2s_openapi_type_is_primitive helper fail */
  {
    int is_prim = 0;
    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS, c2s_openapi_type_is_primitive("string", &is_prim));
    g_c2s_helper_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2s_openapi_type_is_primitive("string", &is_prim));
    g_c2s_helper_fail = 0;
  }

  /* 25. anyOf error in json_object_to_struct_fields */
  {
    JSON_Value *any_v =
        json_parse_string("{\"anyOf\": [{\"type\": \"string\"}]}");
    struct StructFields sf_any;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_any));
    g_c2s_helper_fail = 4;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              json_object_to_struct_fields(json_value_get_object(any_v),
                                           &sf_any, NULL));
    g_c2s_helper_fail = 0;
    struct_fields_free(&sf_any);
    json_value_free(any_v);
  }

  /* 26. required_name_in_list error in json_object_to_struct_fields */
  {
    JSON_Value *req_fail_v = json_parse_string(
        "{\"properties\": {\"x\": {\"type\": \"string\"}}, \"required\": "
        "[\"x\"]}");
    struct StructFields sf_req;
    cdd_c_error_t rc_sub26;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_req));
    g_c2s_helper_fail = 4;
    rc_sub26 = json_object_to_struct_fields(json_value_get_object(req_fail_v),
                                            &sf_req, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc_sub26);
    g_c2s_helper_fail = 0;
    struct_fields_free(&sf_req);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_req));
    g_c2s_helper_fail = 5;
    rc_sub26 = json_object_to_struct_fields(json_value_get_object(req_fail_v),
                                            &sf_req, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc_sub26);
    g_c2s_helper_fail = 0;
    struct_fields_free(&sf_req);
    json_value_free(req_fail_v);
  }

  /* 27. name_hint fallback with schema_name NULL and object variant */
  {
    JSON_Value *val =
        json_parse_string("[{\"properties\":{\"x\":{\"type\":\"string\"}}}]");
    JSON_Array *arr = json_value_get_array(val);
    JSON_Value *root_val = json_value_init_object();
    JSON_Object *root = json_value_get_object(root_val);
    struct StructFields dest;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&dest));
    ASSERT_EQ(CDD_C_SUCCESS, apply_union_to_struct_fields_ex(arr, &dest, root,
                                                             NULL, 0, NULL, 1));
    struct_fields_free(&dest);
    json_value_free(root_val);
    json_value_free(val);
  }

  /* 28. parse_type_union_array_code2schema failure inside */
  /* apply_union_to_struct_fields_ex */
  {
    JSON_Value *arr_u_fail = json_parse_string(
        "[{\"type\": \"array\", \"items\": {\"type\": [\"string\", "
        "\"null\"]}}]");
    struct StructFields sf_fail;
    cdd_c_error_t rc_sub28;
    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_fail));
    g_c2s_helper_fail = 2;
    rc_sub28 = apply_union_to_struct_fields_ex(
        json_value_get_array(arr_u_fail), &sf_fail, NULL, NULL, 0, NULL, 1);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc_sub28);
    g_c2s_helper_fail = 0;
    struct_fields_free(&sf_fail);

    ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_fail));
    g_c2s_helper_fail = 9;
    rc_sub28 = apply_union_to_struct_fields_ex(
        json_value_get_array(arr_u_fail), &sf_fail, NULL, NULL, 0, NULL, 1);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc_sub28);
    g_c2s_helper_fail = 0;
    struct_fields_free(&sf_fail);
    json_value_free(arr_u_fail);
  }

  /* 29. unclosed nested struct in code2schema_main */
  {
    FILE *fhdr = fopen("cdd_test_tmp_unclosed.h", "w");
    char *av_unclosed[2];
    fputs("struct Unclosed {\n  struct {\n    int x;\n", fhdr);
    fclose(fhdr);
    av_unclosed[0] = (char *)(size_t) "cdd_test_tmp_unclosed.h";
    av_unclosed[1] = (char *)(size_t) "cdd_test_tmp_out.json";
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
    av_nested[0] = (char *)(size_t) "cdd_test_tmp_nested_fail.h";
    av_nested[1] = (char *)(size_t) "cdd_test_tmp_out.json";
    g_struct_fields_init_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, code2schema_main(2, av_nested));
    g_struct_fields_init_fail = 0;
    remove("cdd_test_tmp_nested_fail.h");
    remove("cdd_test_tmp_out.json");
  }

  /* 31. struct_fields_init failure on top-level struct in code2schema_main */
  {
    FILE *fhdr = fopen("cdd_test_tmp_struct_fail.h", "w");
    char *av[2];
    fputs("struct S {\n  int a;\n};\n", fhdr);
    fclose(fhdr);
    av[0] = (char *)(size_t) "cdd_test_tmp_struct_fail.h";
    av[1] = (char *)(size_t) "cdd_test_tmp_out.json";
    g_struct_fields_init_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av));
    g_struct_fields_init_fail = 0;
    remove("cdd_test_tmp_struct_fail.h");
    remove("cdd_test_tmp_out.json");
  }

  /* 32. c2s_read_line failure inside enum in code2schema_main */
  {
    FILE *fhdr = fopen("cdd_test_tmp_enum_fail.h", "w");
    char *av[2];
    fputs("enum E {\n  A = 1,\n  B\n};\n", fhdr);
    fclose(fhdr);
    av[0] = (char *)(size_t) "cdd_test_tmp_enum_fail.h";
    av[1] = (char *)(size_t) "cdd_test_tmp_out.json";
    g_c2s_helper_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, av));
    g_c2s_helper_fail = 0;
    remove("cdd_test_tmp_enum_fail.h");
    remove("cdd_test_tmp_out.json");
  }

  PASS();
}

SUITE(code2schema_internals_reach_suite) {
  RUN_TEST(test_code2schema_reach_100_percent_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_REACH_H */
