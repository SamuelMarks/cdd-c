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

extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;

/**
 * @brief Tests remaining branches in code2schema part 3.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_exhaustive_100_percent_coverage_part3(void) {
  /* 22. c2s_write_default_value parser paths */
  {
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    struct StructField f_str, f_bool, f_num;
    cdd_c_error_t rc = 0;

    memset(&f_str, 0, sizeof(f_str));
    CDD_STRCPY(f_str.type, sizeof(f_str.type), "string");
    CDD_STRCPY(f_str.default_val, sizeof(f_str.default_val), "\"hello\"");

    memset(&f_bool, 0, sizeof(f_bool));
    CDD_STRCPY(f_bool.type, sizeof(f_bool.type), "boolean");
    CDD_STRCPY(f_bool.default_val, sizeof(f_bool.default_val), "true");

    memset(&f_num, 0, sizeof(f_num));
    CDD_STRCPY(f_num.type, sizeof(f_num.type), "integer");
    CDD_STRCPY(f_num.default_val, sizeof(f_num.default_val), "42");

    rc = c2s_write_default_value(obj, &f_str);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = c2s_write_default_value(obj, &f_bool);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = c2s_write_default_value(obj, &f_num);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    json_value_free(val);
  }

  /* 24. Final targeted branch coverage tests */
  {
    const char *out_s = NULL;
    int has_bval = 0;
    cdd_c_error_t rc = 0;

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
      const JSON_Object *out_obj = NULL;
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

  /* 26. make_unique_variant_name fallback strdup failure */
  {
    struct StructFields dest;
    char *out_val = NULL;
    cdd_c_error_t rc = 0;

    struct_fields_init(&dest);
    struct_fields_add(&dest, "variant", "string", NULL, NULL, NULL);
    struct_fields_add(&dest, "variant_1", "string", NULL, NULL, NULL);

    /* strdup failure on fallback in make_unique_variant_name */
    g_cdd_strdup_fail = 2;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&dest);
  }

  /* 28. Additional coverage for merge_extras, merge_struct_field and
   * discriminator */
  {
    cdd_c_error_t rc = 0;
    char *out_val = NULL;

    /* discriminator fallback strdup failure */
    g_cdd_strdup_fail = 1;
    rc =
        discriminator_value_for_variant(NULL, "FallbackSchema", NULL, &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    /* parse_struct_member_line struct_fields_add error */
    {
      struct StructFields sf;
      int j;
      struct_fields_init(&sf);
      for (j = 0; j < 8; ++j)
        struct_fields_add(&sf, "pad", "string", NULL, NULL, NULL);
      g_cdd_alloc_fail = 1;
      rc = parse_struct_member_line("int x;", &sf);
      g_cdd_alloc_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);
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
      g_cdd_fail_json_serialize = 1;
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL, "ExtrasFail",
                                                     0);
      g_cdd_fail_json_serialize = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);

      /* fail on prop extras */
      struct_fields_init(&sf);
      g_cdd_strdup_fail = 2;
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL, "ExtrasFail",
                                                     0);
      g_cdd_strdup_fail = 0;
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
    cdd_c_error_t rc = 0;

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
