/**
 * @file test_code2schema_internals_exhaustive_p2.h
 * @brief Exhaustive branch coverage tests part 2 for code2schema.c.
 * @author Samuel Marks
 */

#ifndef TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P2_H
#define TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P2_H

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
 * @brief Tests remaining branches in code2schema part 2.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_exhaustive_100_percent_coverage_part2(void) {
  /* 13. Additional edge cases for 100% coverage */
  {
    cdd_c_error_t rc;
    char **out_arr = NULL;
    size_t out_cnt = 0;

    /* resolve_schema_ref_object NULL out */
    rc = resolve_schema_ref_object(NULL, "ref", NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* c2s_collect_property_names on object without properties */
    {
      JSON_Value *v = json_parse_string("{\"type\":\"string\"}");
      JSON_Object *o = json_value_get_object(v);
      rc = c2s_collect_property_names(o, &out_arr, &out_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(NULL, out_arr);
      ASSERT_EQ(0, out_cnt);
      json_value_free(v);
    }

    /* c2s_parse_union_and_write invalid */
    rc = c2s_parse_union_and_write(NULL, NULL, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* apply_union_to_struct_fields_ex with non-object element, empty object and
     * boolean */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "[{\"type\":\"string\"}, 42, {}, {\"type\":\"boolean\"}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);

      rc = struct_fields_init(&dest);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnionEdges",
                                           0, NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* apply_union_to_struct_fields_ex with allow_inline = 0 and array variant
     */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "[{\"type\":\"array\",\"items\":{\"type\":\"string\"}}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);

      rc = struct_fields_init(&dest);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestArrNoInline",
                                           0, NULL, 0);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }
  }

  /* 14. c2s_json_object_to_struct_fields_internal property error paths */
  {
    struct StructFields sf;
    JSON_Value *val_root =
        json_parse_string("{\"components\":{\"schemas\":{\"MyEnum\":{\"type\":"
                          "\"string\",\"enum\":[\"A\"]}}}}");
    JSON_Object *root = json_value_get_object(val_root);
    JSON_Value *val_schema = json_parse_string(
        "{\"type\":\"object\",\"properties\":{"
        "  "
        "\"arr_ref\":{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/"
        "schemas/MyEnum\"}},"
        "  \"direct_ref\":{\"$ref\":\"#/components/schemas/MyEnum\"},"
        "  \"plain_str\":{\"type\":\"string\",\"x-extra\":1}"
        "}}");
    JSON_Object *schema = json_value_get_object(val_schema);
    cdd_c_error_t rc;

    /* ref_points_to_string_enum on array items failure */
    struct_fields_init(&sf);
    g_cdd_fail_str_after_last = 1;
    rc = c2s_json_object_to_struct_fields_internal(schema, &sf, root, "TestErr",
                                                   0);
    g_cdd_fail_str_after_last = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    /* ref_points_to_string_enum on direct ref failure */
    struct_fields_init(&sf);
    g_cdd_fail_str_after_last = 2;
    rc = c2s_json_object_to_struct_fields_internal(schema, &sf, root, "TestErr",
                                                   0);
    g_cdd_fail_str_after_last = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    {
      int k;
      for (k = 1; k <= 3; ++k) {
        struct_fields_init(&sf);
        g_cdd_fail_struct_fields_add = k;
        rc = c2s_json_object_to_struct_fields_internal(schema, &sf, root,
                                                       "TestErr", 0);
        g_cdd_fail_struct_fields_add = 0;
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
        struct_fields_free(&sf);
      }
    }

    json_value_free(val_schema);
    json_value_free(val_root);
  }

  /* 15. c2s_merge_schema_extras_strings error branches */
  {
    char *dest = NULL;
    cdd_c_error_t rc;

    c_cdd_strdup("{\"a\":1}", &dest);
    g_c2s_helper_fail = 2;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);

    c_cdd_strdup("{\"a\":1}", &dest);
    g_cdd_strdup_fail = 1;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);
  }

  /* 16. Bit-field trim_trailing failure */
  {
    struct StructFields sf;
    cdd_c_error_t rc;
    struct_fields_init(&sf);
    g_c2s_helper_fail = 2;
    rc = parse_struct_member_line("int flag : 1;", &sf);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);
  }

  /* 17. parse_struct_member_line array format error & enum_members_add error */
  {
    struct StructFields sf;
    struct EnumMembers em;
    JSON_Value *arr_val;
    JSON_Array *arr;
    cdd_c_error_t rc;

    struct_fields_init(&sf);
    g_cdd_strdup_fail = 1;
    rc = parse_struct_member_line("uint32_t arr[10];", &sf);
    g_cdd_strdup_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    enum_members_init(&em);
    arr_val = json_parse_string("[\"A\", \"B\"]");
    arr = json_value_get_array(arr_val);
    g_enum_members_add_strdup_fail = 1;
    rc = json_array_to_enum_members(arr, &em);
    g_enum_members_add_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    json_value_free(arr_val);
    enum_members_free(&em);
  }

  /* 18. c2s_json_object_to_struct_fields_internal allOf / anyOf / oneOf error
   * percolation */
  {
    struct StructFields sf;
    JSON_Value *val_enum =
        json_parse_string("{\"type\":\"string\",\"enum\":[\"A\",\"B\"]}");
    JSON_Object *obj_enum = json_value_get_object(val_enum);
    JSON_Value *val_allof =
        json_parse_string("{\"allOf\":[{\"type\":\"object\",\"properties\":{"
                          "\"x\":{\"type\":\"int\"}}}]}");
    JSON_Object *obj_allof = json_value_get_object(val_allof);
    JSON_Value *val_anyof =
        json_parse_string("{\"anyOf\":[{\"type\":\"object\",\"properties\":{"
                          "\"y\":{\"type\":\"string\"}}}]}");
    JSON_Object *obj_anyof = json_value_get_object(val_anyof);
    JSON_Value *val_oneof =
        json_parse_string("{\"oneOf\":[{\"type\":\"object\",\"properties\":{"
                          "\"z\":{\"type\":\"boolean\"}}}]}");
    JSON_Object *obj_oneof = json_value_get_object(val_oneof);
    cdd_c_error_t rc;

    /* enum_members_init fail */
    struct_fields_init(&sf);
    g_enum_members_init_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_enum, &sf, NULL,
                                                   "EnumTest", 1);
    g_enum_members_init_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    struct_fields_free(&sf);

    /* json_array_to_enum_members fail */
    struct_fields_init(&sf);
    g_enum_members_add_strdup_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_enum, &sf, NULL,
                                                   "EnumTest", 1);
    g_enum_members_add_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    struct_fields_free(&sf);

    /* allOf fail with g_struct_fields_init_fail */
    struct_fields_init(&sf);
    g_struct_fields_init_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_allof, &sf, NULL,
                                                   "AllOfTest", 1);
    g_struct_fields_init_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    /* anyOf ex fail with g_c2s_helper_fail = 3 */
    struct_fields_init(&sf);
    g_c2s_helper_fail = 3;
    rc = c2s_json_object_to_struct_fields_internal(obj_anyof, &sf, NULL,
                                                   "AnyOfTest", 1);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    /* anyOf fallback fail with g_struct_fields_init_fail */
    struct_fields_init(&sf);
    g_struct_fields_init_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_anyof, &sf, NULL,
                                                   "AnyOfTest", 0);
    g_struct_fields_init_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    /* oneOf fallback fail with g_struct_fields_init_fail */
    struct_fields_init(&sf);
    g_struct_fields_init_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_oneof, &sf, NULL,
                                                   "OneOfTest", 0);
    g_struct_fields_init_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    json_value_free(val_enum);
    json_value_free(val_allof);
    json_value_free(val_anyof);
    json_value_free(val_oneof);
  }

  /* 19. Helpers error paths */
  {
    struct StructFields dest;
    char *out_val = NULL;
    char *out_name = NULL;
    cdd_c_error_t rc;

    /* make_unique_variant_name sanitize error */
    struct_fields_init(&dest);
    g_c2s_helper_fail = 1;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* make_unique_variant_name strdup error */
    struct_fields_add(&dest, "variant", "string", NULL, NULL, NULL);
    g_cdd_strdup_fail = 1;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&dest);

    /* make_inline_schema_name error */
    g_c2s_helper_fail = 1;
    rc = make_inline_schema_name("Schema", "Variant", "Suffix", &out_name);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* register_inline_schema_c2s clone error */
    {
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      JSON_Value *sch_val = json_parse_string("{\"type\":\"string\"}");
      g_c2s_helper_fail = 1;
      rc = register_inline_schema_c2s(root, "Schema", "Variant", "Suffix",
                                      sch_val, &out_name);
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(sch_val);
      json_value_free(root_val);
    }

    /* discriminator_value_for_variant strdup error on ref_name */
    {
      JSON_Value *d_val = json_parse_string("{\"mapping\":{}}");
      JSON_Object *d_obj = json_value_get_object(d_val);
      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(
          d_obj, NULL, "#/components/schemas/MyRef", &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(d_val);
    }
  }

  /* 20. c2s_parse_union_and_write edge cases */
  {
    FILE *fp;
    JSON_Value *root_val = json_value_init_object();
    JSON_Object *root = json_value_get_object(root_val);
    cdd_c_error_t rc;

    fp = tmpfile();
    if (fp) {
      fprintf(fp, "int a;\n}\n");
      rewind(fp);
      g_c2s_helper_fail = 1;
      rc = c2s_parse_union_and_write(fp, root, "MyUnion");
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      fclose(fp);
    }
    json_value_free(root_val);
  }

  /* 21. Additional failure coverage tests */
  {
    /* c2s_collect_string_array strdup error */
    {
      char **out_arr = NULL;
      size_t out_cnt = 0;
      JSON_Value *v = json_parse_string("[\"item1\"]");
      JSON_Array *a = json_value_get_array(v);
      cdd_c_error_t rc;
      g_cdd_strdup_fail = 1;
      rc = c2s_collect_string_array(a, &out_arr, &out_cnt);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(v);
    }

    /* c2s_collect_property_names strdup error */
    {
      char **out_arr = NULL;
      size_t out_cnt = 0;
      JSON_Value *v = json_parse_string(
          "{\"properties\":{\"prop1\":{\"type\":\"string\"} } }");
      JSON_Object *o = json_value_get_object(v);
      cdd_c_error_t rc;
      g_cdd_strdup_fail = 1;
      rc = c2s_collect_property_names(o, &out_arr, &out_cnt);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(v);
    }

    /* merge_struct_fields struct_fields_add and strdup error */
    {
      struct StructFields src, dest;
      cdd_c_error_t rc;
      struct_fields_init(&src);
      struct_fields_init(&dest);
      struct_fields_add(&src, "fld", "string", NULL, NULL, NULL);
      c_cdd_strdup("{\"x\":1}", &src.fields[0].schema_extra_json);

      /* struct_fields_add failure in merge */
      g_cdd_fail_struct_fields_add = 1;
      rc = merge_struct_fields(&dest, &src);
      g_cdd_fail_struct_fields_add = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      /* strdup failure on schema_extra_json in merge */
      g_cdd_strdup_fail = 1;
      rc = merge_struct_fields(&dest, &src);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&src);
      struct_fields_free(&dest);
    }

    /* apply_union_to_struct_fields_ex variant struct_fields_add error */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string("[{\"type\":\"string\"}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      cdd_c_error_t rc;

      struct_fields_init(&dest);
      g_cdd_fail_struct_fields_add = 1;
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnionFldAdd",
                                           0, NULL, 1);
      g_cdd_fail_struct_fields_add = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* parse_struct_member_line error percolation in code2schema_main */
    {
      FILE *fp = fopen("test_err_main.h", "w");
      char *argv[3];
      cdd_c_error_t rc;
      if (fp) {
        fprintf(fp, "struct BadStruct {\n  uint32_t bad_field[10];\n};\n");
        fclose(fp);

        argv[0] = C_CDD_STR_LIT("test_err_main.h");
        argv[1] = C_CDD_STR_LIT("test_err_main.json");
        argv[2] = NULL;

        g_c2s_helper_fail = 4;
        rc = code2schema_main(2, argv);
        g_c2s_helper_fail = 0;
        ASSERT_NEQ(CDD_C_SUCCESS, rc);

        remove("test_err_main.h");
        remove("test_err_main.json");
      }
    }
  }
  PASS();
}
SUITE(code2schema_internals_exhaustive_p2_suite) {
  RUN_TEST(test_code2schema_exhaustive_100_percent_coverage_part2);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P2_H */
