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

/** @brief Test definition g_cdd_fail_json_set_value. */
extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
/** @brief Test definition g_cdd_fail_json_serialize. */
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;

/**
 * @brief Tests remaining branches in code2schema part 2.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_exhaustive_100_percent_coverage_part2(void) {
  /* 13. Additional edge cases for 100% coverage */
  {
    cdd_c_error_t rc = 0;
    char **out_arr = NULL;
    size_t out_cnt = 0;

    /* resolve_schema_ref_object NULL out */
    rc = resolve_schema_ref_object(NULL, "ref", NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* c2s_collect_property_names on object without properties */
    {
      JSON_Value *v = NULL;
      JSON_Object *o = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_parse_string("{\"type\":\"string\"}", &_cdd_jv_1);
        v = _cdd_jv_1;
      }
      o = json_value_get_object(v);
      rc = c2s_collect_property_names(o, &out_arr, &out_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(NULL, out_arr);
      ASSERT_EQ(0, out_cnt);
      g_cdd_last_alloc_err = json_value_free(v);
    }

    /* c2s_parse_union_and_write invalid */
    rc = c2s_parse_union_and_write(NULL, NULL, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* apply_union_to_struct_fields_ex with non-object element, empty object and
     * boolean */
    {
      struct StructFields dest;
      JSON_Value *val = NULL;
      JSON_Array *arr = NULL;
      JSON_Value *root_val = NULL;
      JSON_Object *root = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_parse_string(
            "[{\"type\":\"string\"}, 42, {}, {\"type\":\"boolean\"}]",
            &_cdd_jv_1);
        val = _cdd_jv_1;
      }
      arr = json_value_get_array(val);
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
        root_val = _cdd_jv_1;
      }
      root = json_value_get_object(root_val);

      rc = struct_fields_init(&dest);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnionEdges",
                                           0, NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      g_cdd_last_alloc_err = struct_fields_free(&dest);
      g_cdd_last_alloc_err = json_value_free(val);
      g_cdd_last_alloc_err = json_value_free(root_val);
    }

    /* apply_union_to_struct_fields_ex with allow_inline = 0 and array variant
     */
    {
      struct StructFields dest;
      JSON_Value *val = NULL;
      JSON_Array *arr = NULL;
      JSON_Value *root_val = NULL;
      JSON_Object *root = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_parse_string(
            "[{\"type\":\"array\",\"items\":{\"type\":\"string\"}}]",
            &_cdd_jv_1);
        val = _cdd_jv_1;
      }
      arr = json_value_get_array(val);
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
        root_val = _cdd_jv_1;
      }
      root = json_value_get_object(root_val);

      rc = struct_fields_init(&dest);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestArrNoInline",
                                           0, NULL, 0);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      g_cdd_last_alloc_err = struct_fields_free(&dest);
      g_cdd_last_alloc_err = json_value_free(val);
      g_cdd_last_alloc_err = json_value_free(root_val);
    }
  }

  /* 14. c2s_json_object_to_struct_fields_internal property error paths */
  {
    struct StructFields sf;
    JSON_Value *val_root = NULL;
    JSON_Object *root = NULL;
    JSON_Value *val_schema = NULL;
    JSON_Object *schema = NULL;
    cdd_c_error_t rc = 0;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"components\":{\"schemas\":{\"MyEnum\":{\"type\":"
          "\"string\",\"enum\":[\"A\"]}}}}",
          &_cdd_jv_1);
      val_root = _cdd_jv_1;
    }
    root = json_value_get_object(val_root);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"type\":\"object\",\"properties\":{"
          "  "
          "\"arr_ref\":{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/"
          "schemas/MyEnum\"}},"
          "  \"direct_ref\":{\"$ref\":\"#/components/schemas/MyEnum\"},"
          "  \"plain_str\":{\"type\":\"string\",\"x-extra\":1}"
          "}}",
          &_cdd_jv_1);
      val_schema = _cdd_jv_1;
    }
    schema = json_value_get_object(val_schema);

    /* ref_points_to_string_enum on array items failure */
    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_fail_str_after_last = 1;
    rc = c2s_json_object_to_struct_fields_internal(schema, &sf, root, "TestErr",
                                                   0);
    g_cdd_fail_str_after_last = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    /* ref_points_to_string_enum on direct ref failure */
    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_fail_str_after_last = 2;
    rc = c2s_json_object_to_struct_fields_internal(schema, &sf, root, "TestErr",
                                                   0);
    g_cdd_fail_str_after_last = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    /* struct_fields_add realloc failure on array, primitive, and ref */
    {
      int j;
      g_cdd_last_alloc_err = struct_fields_init(&sf);
      for (j = 0; j < 8; ++j)
        g_cdd_last_alloc_err =
            struct_fields_add(&sf, "pad", "string", NULL, NULL, NULL);
      g_cdd_alloc_fail = 1;
      rc = c2s_json_object_to_struct_fields_internal(schema, &sf, root,
                                                     "TestErr", 0);
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      g_cdd_last_alloc_err = struct_fields_free(&sf);
    }

    g_cdd_last_alloc_err = json_value_free(val_schema);
    g_cdd_last_alloc_err = json_value_free(val_root);
  }

  /* 15. c2s_merge_schema_extras_strings error branches */
  {
    char *dest = NULL;
    cdd_c_error_t rc = 0;

    g_cdd_last_alloc_err = c_cdd_strdup("{\"a\":1}", &dest);
    g_cdd_fail_json_serialize = 1;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_cdd_fail_json_serialize = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);

    g_cdd_last_alloc_err = c_cdd_strdup("{\"a\":1}", &dest);
    g_cdd_strdup_fail = 1;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);
  }

  /* 16. Bit-field parsing */
  {
    struct StructFields sf;
    cdd_c_error_t rc = 0;
    g_cdd_last_alloc_err = struct_fields_init(&sf);
    rc = parse_struct_member_line("int flag : 1;", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = struct_fields_free(&sf);
  }

  /* 17. parse_struct_member_line array format error & enum_members_add error */
  {
    struct StructFields sf;
    struct EnumMembers em;
    JSON_Value *arr_val;
    JSON_Array *arr;
    cdd_c_error_t rc = 0;

    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_strdup_fail = 1;
    rc = parse_struct_member_line("uint32_t arr[10];", &sf);
    g_cdd_strdup_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    g_cdd_last_alloc_err = enum_members_init(&em);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("[\"A\", \"B\"]", &_cdd_jv_1);
      arr_val = _cdd_jv_1;
    }
    arr = json_value_get_array(arr_val);
    g_cdd_strdup_fail = 1;
    rc = json_array_to_enum_members(arr, &em);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_last_alloc_err = json_value_free(arr_val);
    g_cdd_last_alloc_err = enum_members_free(&em);
  }

  /* 18. c2s_json_object_to_struct_fields_internal allOf / anyOf / oneOf error
   * percolation */
  {
    struct StructFields sf;
    JSON_Value *val_enum = NULL;
    JSON_Object *obj_enum = NULL;
    JSON_Value *val_allof = NULL;
    JSON_Object *obj_allof = NULL;
    JSON_Value *val_anyof = NULL;
    JSON_Object *obj_anyof = NULL;
    JSON_Value *val_oneof = NULL;
    JSON_Object *obj_oneof = NULL;
    cdd_c_error_t rc = 0;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"type\":\"string\",\"enum\":[\"A\",\"B\"]}", &_cdd_jv_1);
      val_enum = _cdd_jv_1;
    }
    obj_enum = json_value_get_object(val_enum);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"allOf\":[{\"type\":\"object\",\"properties\":{"
          "\"x\":{\"type\":\"int\"}}}]}",
          &_cdd_jv_1);
      val_allof = _cdd_jv_1;
    }
    obj_allof = json_value_get_object(val_allof);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"anyOf\":[{\"type\":\"object\",\"properties\":{"
          "\"y\":{\"type\":\"string\"}}}]}",
          &_cdd_jv_1);
      val_anyof = _cdd_jv_1;
    }
    obj_anyof = json_value_get_object(val_anyof);
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string(
          "{\"oneOf\":[{\"type\":\"object\",\"properties\":{"
          "\"z\":{\"type\":\"boolean\"}}}]}",
          &_cdd_jv_1);
      val_oneof = _cdd_jv_1;
    }
    obj_oneof = json_value_get_object(val_oneof);

    /* enum_members_init fail */
    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_alloc_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_enum, &sf, NULL,
                                                   "EnumTest", 1);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    /* json_array_to_enum_members fail */
    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_strdup_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_enum, &sf, NULL,
                                                   "EnumTest", 1);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    /* allOf fail with g_cdd_alloc_fail */
    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_alloc_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_allof, &sf, NULL,
                                                   "AllOfTest", 1);
    g_cdd_alloc_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    /* anyOf fallback fail with g_cdd_alloc_fail */
    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_alloc_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_anyof, &sf, NULL,
                                                   "AnyOfTest", 0);
    g_cdd_alloc_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    /* oneOf fallback fail with g_cdd_alloc_fail */
    g_cdd_last_alloc_err = struct_fields_init(&sf);
    g_cdd_alloc_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_oneof, &sf, NULL,
                                                   "OneOfTest", 0);
    g_cdd_alloc_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = struct_fields_free(&sf);

    g_cdd_last_alloc_err = json_value_free(val_enum);
    g_cdd_last_alloc_err = json_value_free(val_allof);
    g_cdd_last_alloc_err = json_value_free(val_anyof);
    g_cdd_last_alloc_err = json_value_free(val_oneof);
  }

  /* 19. Helpers error paths */
  {
    struct StructFields dest;
    char *out_val = NULL;
    char *out_name = NULL;
    cdd_c_error_t rc = 0;

    /* make_unique_variant_name strdup error */
    g_cdd_last_alloc_err = struct_fields_init(&dest);
    g_cdd_last_alloc_err =
        struct_fields_add(&dest, "variant", "string", NULL, NULL, NULL);
    g_cdd_strdup_fail = 1;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = struct_fields_free(&dest);

    /* register_inline_schema_c2s clone error */
    {
      JSON_Value *root_val = NULL;
      JSON_Object *root = NULL;
      JSON_Value *sch_val = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
        root_val = _cdd_jv_1;
      }
      root = json_value_get_object(root_val);
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_parse_string("{\"type\":\"string\"}", &_cdd_jv_1);
        sch_val = _cdd_jv_1;
      }
      g_cdd_fail_json_serialize = 1;
      rc = register_inline_schema_c2s(root, "Schema", "Variant", "Suffix",
                                      sch_val, &out_name);
      g_cdd_fail_json_serialize = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      g_cdd_last_alloc_err = json_value_free(sch_val);
      g_cdd_last_alloc_err = json_value_free(root_val);
    }

    /* discriminator_value_for_variant strdup error on ref_name */
    {
      JSON_Value *d_val = NULL;
      JSON_Object *d_obj = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_parse_string("{\"mapping\":{}}", &_cdd_jv_1);
        d_val = _cdd_jv_1;
      }
      d_obj = json_value_get_object(d_val);
      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(
          d_obj, NULL, "#/components/schemas/MyRef", &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      g_cdd_last_alloc_err = json_value_free(d_val);
    }
  }

  /* 21. Additional failure coverage tests */
  {
    /* c2s_collect_string_array strdup error */
    {
      char **out_arr = NULL;
      size_t out_cnt = 0;
      JSON_Value *v = NULL;
      JSON_Array *a = NULL;
      cdd_c_error_t rc = 0;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_parse_string("[\"item1\"]", &_cdd_jv_1);
        v = _cdd_jv_1;
      }
      a = json_value_get_array(v);
      g_cdd_strdup_fail = 1;
      rc = c2s_collect_string_array(a, &out_arr, &out_cnt);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      g_cdd_last_alloc_err = json_value_free(v);
    }

    /* c2s_collect_property_names strdup error */
    {
      char **out_arr = NULL;
      size_t out_cnt = 0;
      JSON_Value *v = NULL;
      JSON_Object *o = NULL;
      cdd_c_error_t rc = 0;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_parse_string(
            "{\"properties\":{\"prop1\":{\"type\":\"string\"} } }", &_cdd_jv_1);
        v = _cdd_jv_1;
      }
      o = json_value_get_object(v);
      g_cdd_strdup_fail = 1;
      rc = c2s_collect_property_names(o, &out_arr, &out_cnt);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      g_cdd_last_alloc_err = json_value_free(v);
    }

    /* merge_struct_fields struct_fields_add and strdup error */
    {
      struct StructFields src, dest;
      int j;
      cdd_c_error_t rc = 0;
      g_cdd_last_alloc_err = struct_fields_init(&src);
      g_cdd_last_alloc_err = struct_fields_init(&dest);
      for (j = 0; j < 8; ++j)
        g_cdd_last_alloc_err =
            struct_fields_add(&dest, "pad", "string", NULL, NULL, NULL);
      g_cdd_last_alloc_err =
          struct_fields_add(&src, "fld", "string", NULL, NULL, NULL);
      g_cdd_last_alloc_err =
          c_cdd_strdup("{\"x\":1}", &src.fields[0].schema_extra_json);

      /* struct_fields_add realloc failure in merge */
      g_cdd_alloc_fail = 1;
      rc = merge_struct_fields(&dest, &src);
      g_cdd_alloc_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      /* strdup failure on schema_extra_json in merge */
      g_cdd_strdup_fail = 1;
      rc = merge_struct_fields(&dest, &src);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      g_cdd_last_alloc_err = struct_fields_free(&src);
      g_cdd_last_alloc_err = struct_fields_free(&dest);
    }

    /* apply_union_to_struct_fields_ex variant struct_fields_add error */
    {
      struct StructFields dest;
      JSON_Value *val = NULL;
      JSON_Array *arr = NULL;
      JSON_Value *root_val = NULL;
      JSON_Object *root = NULL;
      cdd_c_error_t rc = 0;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_parse_string("[{\"type\":\"string\"}]", &_cdd_jv_1);
        val = _cdd_jv_1;
      }
      arr = json_value_get_array(val);
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
        root_val = _cdd_jv_1;
      }
      root = json_value_get_object(root_val);

      g_cdd_last_alloc_err = struct_fields_init(&dest);
      dest.capacity = 0;
      g_cdd_alloc_fail = 3;
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnionFldAdd",
                                           0, NULL, 1);
      g_cdd_alloc_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      g_cdd_last_alloc_err = struct_fields_free(&dest);
      g_cdd_last_alloc_err = json_value_free(val);
      g_cdd_last_alloc_err = json_value_free(root_val);
    }

    /* parse_struct_member_line error percolation in code2schema_main */
    {
      FILE *fp = fopen("test_err_main.h", "w");
      char *argv[3];
      cdd_c_error_t rc = 0;
      if (fp) {
        fprintf(fp, "struct BadStruct {\n  uint32_t bad_field[10];\n};\n");
        fclose(fp);

        argv[0] = C_CDD_STR_LIT("test_err_main.h");
        argv[1] = C_CDD_STR_LIT("test_err_main.json");
        argv[2] = NULL;

        g_cdd_strdup_fail = 1;
        rc = code2schema_main(2, argv);
        g_cdd_strdup_fail = 0;
        ASSERT_NEQ(CDD_C_SUCCESS, rc);

        remove("test_err_main.h");
        remove("test_err_main.json");
      }
    }

    /* c2s_needs_identifier_sanitization leading underscore branches and
     * json_array_to_enum_members OOM/NULL member branches */
    {
      int needs_san = 0;
      JSON_Value *ev = NULL;
      JSON_Array *ea = NULL;
      struct EnumMembers em;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_parse_string(
            "[\"foo-bar\", \"DUP\", \"DUP\"]", &_cdd_jv_1);
        ev = _cdd_jv_1;
      }
      ea = json_value_get_array(ev);

      ASSERT_EQ(CDD_C_SUCCESS,
                c2s_needs_identifier_sanitization("__dunder", &needs_san));
      ASSERT_EQ(1, needs_san);
      ASSERT_EQ(CDD_C_SUCCESS,
                c2s_needs_identifier_sanitization("_0digit", &needs_san));
      ASSERT_EQ(0, needs_san);
      ASSERT_EQ(CDD_C_SUCCESS,
                c2s_needs_identifier_sanitization("_lower", &needs_san));
      ASSERT_EQ(0, needs_san);
      ASSERT_EQ(CDD_C_SUCCESS,
                c2s_needs_identifier_sanitization("_Upper", &needs_san));
      ASSERT_EQ(1, needs_san);

      g_cdd_last_alloc_err = enum_members_init(&em);
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, json_array_to_enum_members(ea, &em));
      g_cdd_alloc_fail = 0;

      g_cdd_last_alloc_err = enum_members_add(&em, "EXISTING");
      C_CDD_FREE(em.members[0]);
      em.members[0] = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, json_array_to_enum_members(ea, &em));
      g_cdd_last_alloc_err = enum_members_free(&em);
      g_cdd_last_alloc_err = json_value_free(ev);
    }
  }
  PASS();
}
/**
 * @brief Executes the code2schema_internals_exhaustive_p2_suite test suite.
 */
SUITE(code2schema_internals_exhaustive_p2_suite) {
  RUN_TEST(test_code2schema_exhaustive_100_percent_coverage_part2);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P2_H */
