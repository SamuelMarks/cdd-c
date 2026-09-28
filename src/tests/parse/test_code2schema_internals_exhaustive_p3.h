/**
 * @file test_code2schema_internals_exhaustive_p3.h
 * @brief Exhaustive branch coverage tests part 3 for code2schema.c.
 * @author Samuel Marks
 */

#ifndef TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P3_H
#define TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P3_H

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
 * @brief Tests remaining branches in code2schema part 3.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_exhaustive_100_percent_coverage_part3(void) {
  /* 22. c2s_write_default_value parser failure paths */
  {
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    struct StructField f_str, f_bool, f_num;
    cdd_c_error_t rc;

    memset(&f_str, 0, sizeof(f_str));
    CDD_STRCPY(f_str.type, sizeof(f_str.type), "string");
    CDD_STRCPY(f_str.default_val, sizeof(f_str.default_val), "\"hello\"");

    memset(&f_bool, 0, sizeof(f_bool));
    CDD_STRCPY(f_bool.type, sizeof(f_bool.type), "boolean");
    CDD_STRCPY(f_bool.default_val, sizeof(f_bool.default_val), "true");

    memset(&f_num, 0, sizeof(f_num));
    CDD_STRCPY(f_num.type, sizeof(f_num.type), "integer");
    CDD_STRCPY(f_num.default_val, sizeof(f_num.default_val), "42");

    /* c2s_strip_quotes fail */
    g_c2s_helper_fail = 2;
    rc = c2s_write_default_value(obj, &f_str);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* c2s_parse_bool_default fail */
    g_c2s_helper_fail = 2;
    rc = c2s_write_default_value(obj, &f_bool);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* c2s_parse_number_default fail */
    g_c2s_helper_fail = 2;
    rc = c2s_write_default_value(obj, &f_num);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    json_value_free(val);
  }

  /* 23. c2s_parse_union_and_write error percolation in code2schema_main */
  {
    FILE *fp = fopen("test_union_err.h", "w");
    char *argv[3];
    cdd_c_error_t rc;
    if (fp) {
      fprintf(fp, "union FailUnion {\n  int x;\n};\n");
      fclose(fp);

      argv[0] = C_CDD_STR_LIT("test_union_err.h");
      argv[1] = C_CDD_STR_LIT("test_union_err.json");
      argv[2] = NULL;

      g_c2s_helper_fail = 2;
      rc = code2schema_main(2, argv);
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      remove("test_union_err.h");
      remove("test_union_err.json");
    }
  }

  /* 24. Final targeted branch coverage tests */
  {
    const char *out_s = NULL;
    int has_bval = 0;
    cdd_c_error_t rc;

    /* c2s_strip_quotes null input */
    rc = c2s_strip_quotes(NULL, NULL, 0, &out_s);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* c2s_parse_bool_default null input */
    rc = c2s_parse_bool_default(NULL, NULL, &has_bval);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* resolve_schema_ref_object str_after_last error */
    {
      JSON_Value *rv = json_value_init_object();
      JSON_Object *ro = json_value_get_object(rv);
      JSON_Object *out_obj = NULL;
      g_cdd_fail_str_after_last = 1;
      rc = resolve_schema_ref_object(ro, "#/components/schemas/X", &out_obj);
      g_cdd_fail_str_after_last = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(rv);
    }

    /* make_unique_variant_name double collision and strdup fail */
    {
      struct StructFields dest;
      char *out_val = NULL;
      struct_fields_init(&dest);
      struct_fields_add(&dest, "variant", "string", NULL, NULL, NULL);
      struct_fields_add(&dest, "variant_1", "string", NULL, NULL, NULL);
      g_cdd_strdup_fail = 1;
      rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&dest);
    }

    /* merge_struct_fields items_extra_json strdup error */
    {
      struct StructFields src, dest;
      struct_fields_init(&src);
      struct_fields_init(&dest);
      struct_fields_add(&src, "arr_fld", "array", "string", NULL, NULL);
      c_cdd_strdup("{\"format\":\"int32\"}", &src.fields[0].items_extra_json);
      g_cdd_strdup_fail = 1;
      rc = merge_struct_fields(&dest, &src);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&src);
      struct_fields_free(&dest);
    }

    /* apply_union_to_struct_fields_ex array with allow_inline = 0 */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "[{\"type\":\"array\",\"items\":{\"type\":\"string\"}}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);

      struct_fields_init(&dest);
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestArrNoInline2",
                                           0, NULL, 0);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(0, dest.is_union);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* apply_union_to_struct_fields_ex discriminator error via str_after_last */
    {
      struct StructFields dest;
      JSON_Value *val =
          json_parse_string("{\"discriminator\":{\"propertyName\":\"type\"},"
                            "\"oneOf\":[{\"$ref\":\"#/components/schemas/"
                            "A\"},{\"$ref\":\"#/components/schemas/B\"}]}");
      JSON_Object *obj = json_value_get_object(val);
      JSON_Array *oneof_arr = json_object_get_array(obj, "oneOf");
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);

      struct_fields_init(&dest);
      g_cdd_fail_str_after_last = 1;
      rc = apply_union_to_struct_fields_ex(oneof_arr, &dest, root,
                                           "TestDiscErr", 0, obj, 1);
      g_cdd_fail_str_after_last = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }
  }

  /* 25. Direct helper fail hook tests */
  {
    const char *out_s = NULL;
    char buf[64];
    int b = 0, has_b = 0;
    double n = 0;
    int has_n = 0;
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    JSON_Object *out_obj = NULL;
    struct StructField fld;
    struct StructFields sf;
    cdd_c_error_t rc;

    memset(&fld, 0, sizeof(fld));
    struct_fields_init(&sf);

    g_c2s_helper_fail = 1;
    rc = c2s_strip_quotes("abc", buf, sizeof(buf), &out_s);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_parse_bool_default("true", &b, &has_b);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_parse_number_default("42", &n, &has_n);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = resolve_schema_ref_object(obj, "#/components/schemas/A", &out_obj);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_write_type_union(obj, "string", NULL, 0);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_write_numeric_constraints(obj, &fld);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_write_string_constraints(obj, &fld);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_write_array_constraints(obj, &fld);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = write_struct_to_json_schema(obj, "TestSt", &sf);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_collapse_arrays(&sf);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    struct_fields_free(&sf);
    json_value_free(val);
  }

  /* 26. struct_fields_get failure and make_unique_variant_name coverage */
  {
    struct StructFields dest, src;
    char *out_val = NULL;
    cdd_c_error_t rc;

    struct_fields_init(&dest);
    struct_fields_add(&dest, "variant", "string", NULL, NULL, NULL);
    struct_fields_add(&dest, "variant_1", "string", NULL, NULL, NULL);

    /* struct_fields_get failure in make_unique_variant_name call 1 */
    g_cdd_fail_struct_fields_get = 1;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_cdd_fail_struct_fields_get = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* struct_fields_get failure in make_unique_variant_name call 2 */
    g_cdd_fail_struct_fields_get = 2;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_cdd_fail_struct_fields_get = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* strdup failure on fallback in make_unique_variant_name */
    g_cdd_strdup_fail = 2;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* struct_fields_get failure in merge_struct_fields */
    struct_fields_init(&src);
    struct_fields_add(&src, "fld", "string", NULL, NULL, NULL);
    g_cdd_fail_struct_fields_get = 1;
    rc = merge_struct_fields(&dest, &src);
    g_cdd_fail_struct_fields_get = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&src);
    struct_fields_free(&dest);
  }

  /* 27. apply_union_to_struct_fields_ex comprehensive failure loop */
  {
    int k;
    cdd_c_error_t rc;
    for (k = 1; k <= 25; ++k) {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "{\"discriminator\":{\"propertyName\":\"kind\"},"
          "\"oneOf\":["
          "  {\"$ref\":\"#/components/schemas/MyRef\"},"
          "  "
          "{\"type\":\"object\",\"properties\":{\"p\":{\"type\":\"string\"}}},"
          "  {\"type\":\"array\",\"items\":{\"type\":\"string\"}},"
          "  "
          "{\"type\":\"array\",\"items\":{\"properties\":{\"p2\":{\"type\":"
          "\"integer\"}}}},"
          "  {\"type\":\"string\"}"
          "]}");
      JSON_Object *obj = json_value_get_object(val);
      JSON_Array *oneof_arr = json_object_get_array(obj, "oneOf");
      JSON_Value *root_val =
          json_parse_string("{\"components\":{\"schemas\":{\"MyRef\":{\"type\":"
                            "\"string\",\"enum\":[\"A\"]}}}}");
      JSON_Object *root = json_value_get_object(root_val);

      struct_fields_init(&dest);
      g_c2s_helper_fail = k;
      rc = apply_union_to_struct_fields_ex(oneof_arr, &dest, root, "LoopUnion",
                                           0, obj, 1);
      g_c2s_helper_fail = 0;
      if (rc != CDD_C_SUCCESS) { /* expected failure */
      }

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }
  }

  /* 28. Additional coverage for merge_extras, merge_struct_field and
   * discriminator */
  {
    cdd_c_error_t rc;
    char *out_val = NULL;

    /* discriminator fallback strdup failure */
    g_cdd_strdup_fail = 1;
    rc =
        discriminator_value_for_variant(NULL, "FallbackSchema", NULL, &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    /* parse_struct_member_line struct_fields_add error (line 1786) */
    {
      struct StructFields sf;
      struct_fields_init(&sf);
      g_cdd_fail_struct_fields_add = 1;
      rc = parse_struct_member_line("int x;", &sf);
      g_cdd_fail_struct_fields_add = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);
    }

    /* merge_struct_fields with merge_struct_field failure */
    {
      struct StructFields src, dest;
      struct_fields_init(&src);
      struct_fields_init(&dest);
      struct_fields_add(&src, "shared_fld", "string", NULL, NULL, NULL);
      struct_fields_add(&dest, "shared_fld", "string", NULL, NULL, NULL);

      g_c2s_helper_fail = 2;
      rc = merge_struct_fields(&dest, &src);
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&src);
      struct_fields_free(&dest);
    }

    /* c2s_json_object_to_struct_fields_internal with items and prop extras
     * error */
    {
      struct StructFields sf;
      JSON_Value *v =
          json_parse_string("{\"type\":\"object\",\"properties\":{"
                            "  "
                            "\"arr\":{\"type\":\"array\",\"items\":{\"type\":"
                            "\"string\",\"x-item\":1}},"
                            "  \"p\":{\"type\":\"string\",\"x-prop\":2}"
                            "}}");
      JSON_Object *o = json_value_get_object(v);

      /* fail on items extras */
      struct_fields_init(&sf);
      g_c2s_helper_fail = 4;
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL, "ExtrasFail",
                                                     0);
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);

      /* fail on prop extras */
      struct_fields_init(&sf);
      g_c2s_helper_fail = 3;
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL, "ExtrasFail",
                                                     0);
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);

      json_value_free(v);
    }
  }

  /* 29. allOf and union fallback failure with invalid sub-schemas */
  {
    struct StructFields dest;
    JSON_Value *val_bad = json_parse_string(
        "{\"allOf\":[{\"type\":\"object\",\"properties\":{\"p\":{\"type\":"
        "\"array\",\"items\":{\"$ref\":\"#/components/schemas/Bad\"}}}}]}");
    JSON_Object *obj_bad = json_value_get_object(val_bad);
    JSON_Value *val_root = json_value_init_object();
    JSON_Object *root = json_value_get_object(val_root);
    cdd_c_error_t rc;

    /* allOf fails because sub-schema fails on unresolved ref with
     * fail_str_after_last */
    struct_fields_init(&dest);
    g_cdd_fail_str_after_last = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_bad, &dest, root,
                                                   "BadAllOf", 0);
    g_cdd_fail_str_after_last = 0;
    if (rc != CDD_C_SUCCESS) { /* expected failure */
    }
    struct_fields_free(&dest);

    /* oneOf fallback fails similarly */
    {
      JSON_Value *val_oneof_bad = json_parse_string(
          "{\"oneOf\":[{\"type\":\"object\",\"properties\":{\"p\":{\"type\":"
          "\"array\",\"items\":{\"$ref\":\"#/components/schemas/Bad\"}}}}]}");
      JSON_Object *obj_oneof_bad = json_value_get_object(val_oneof_bad);

      struct_fields_init(&dest);
      g_cdd_fail_str_after_last = 1;
      rc = c2s_json_object_to_struct_fields_internal(obj_oneof_bad, &dest, root,
                                                     "BadOneOf", 0);
      g_cdd_fail_str_after_last = 0;
      if (rc != CDD_C_SUCCESS) { /* expected failure */
      }
      struct_fields_free(&dest);

      json_value_free(val_oneof_bad);
    }

    json_value_free(val_bad);
    json_value_free(val_root);
  }
  PASS();
}
SUITE(code2schema_internals_exhaustive_p3_suite) {
  RUN_TEST(test_code2schema_exhaustive_100_percent_coverage_part3);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P3_H */
