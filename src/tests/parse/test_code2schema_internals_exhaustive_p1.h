/**
 * @file test_code2schema_internals_exhaustive_p1.h
 * @brief Exhaustive branch coverage tests part 1 for code2schema.c.
 * @author Samuel Marks
 */

#ifndef TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P1_H
#define TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P1_H

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
 * @brief Tests remaining branches in code2schema part 1.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_exhaustive_100_percent_coverage_part1(void) {
  /* 1. sanitize_identifier empty string */
  {
    char *out = NULL;
    cdd_c_error_t rc = 0;
    rc = sanitize_identifier("", &out);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("Variant", out);
    C_CDD_FREE(out);
  }

  /* 2. discriminator_value_for_variant all branches */
  {
    JSON_Value *val =
        json_parse_string("{\"mapping\":{\"k1\":\"#/components/schemas/"
                          "A\",\"k2\":\"B\",\"k3\":\"C\",\"k4\":123}}");
    JSON_Object *obj = json_value_get_object(val);
    char *out_val = NULL;
    cdd_c_error_t rc = 0;

    /* Fallback on NULL disc_obj */
    rc = discriminator_value_for_variant(NULL, "Hint", NULL, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("Hint", out_val);
    C_CDD_FREE(out_val);

    /* Matching ref */
    rc = discriminator_value_for_variant(obj, "Hint", "#/components/schemas/A",
                                         &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("k1", out_val);
    C_CDD_FREE(out_val);

    /* Matching ref_name */
    rc = discriminator_value_for_variant(obj, "Hint", "#/components/schemas/B",
                                         &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("k2", out_val);
    C_CDD_FREE(out_val);

    /* Matching variant_name */
    rc = discriminator_value_for_variant(obj, "C", NULL, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("k3", out_val);
    C_CDD_FREE(out_val);

    /* Fallback variant_name */
    rc = discriminator_value_for_variant(obj, "SchemaX", NULL, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("SchemaX", out_val);
    C_CDD_FREE(out_val);

    /* Fallback ref_name */
    rc = discriminator_value_for_variant(obj, NULL, "#/components/schemas/RefX",
                                         &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("RefX", out_val);
    C_CDD_FREE(out_val);

    /* Fallback NULL */
    rc = discriminator_value_for_variant(obj, NULL, NULL, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, out_val);

    json_value_free(val);
  }

  /* 3. merge_struct_fields complete branches */
  {
    struct StructFields src;
    struct StructFields dest;
    cdd_c_error_t rc = 0;
    char *types1[2];
    char *types2[2];

    rc = struct_fields_init(&src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = struct_fields_init(&dest);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = struct_fields_add(&src, "field1", "string", NULL, NULL, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    c_cdd_strdup("{\"x-custom\":1}", &src.fields[0].schema_extra_json);
    c_cdd_strdup("{\"x-item-custom\":2}", &src.fields[0].items_extra_json);

    types1[0] = C_CDD_STR_LIT("string");
    types1[1] = C_CDD_STR_LIT("null");
    copy_string_array_code2schema(&src.fields[0].type_union,
                                  &src.fields[0].n_type_union, types1, 2);
    types2[0] = C_CDD_STR_LIT("integer");
    types2[1] = C_CDD_STR_LIT("null");
    copy_string_array_code2schema(&src.fields[0].items_type_union,
                                  &src.fields[0].n_items_type_union, types2, 2);

    /* First merge into empty dest (field added) */
    rc = merge_struct_fields(&dest, &src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, dest.size);

    /* Second merge with matching field (field merged via merge_struct_field) */
    rc = merge_struct_fields(&dest, &src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&src);
    struct_fields_free(&dest);
  }

  /* 4. apply_union_to_struct_fields_ex variant types */
  {
    struct StructFields dest;
    JSON_Value *val = json_parse_string(
        "[42, {\"type\":\"null\"}, {\"type\":\"custom_unknown\"}, "
        "{\"type\":\"array\",\"items\":{\"properties\":{\"sub\":{\"type\":"
        "\"string\"}}}}, "
        "{\"type\":\"array\",\"items\":{\"type\":\"unknown_items\"}}, "
        "{}]");
    JSON_Array *arr = json_value_get_array(val);
    JSON_Value *root_val = json_value_init_object();
    JSON_Object *root = json_value_get_object(root_val);
    cdd_c_error_t rc = 0;

    rc = struct_fields_init(&dest);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Test with allow_inline = 1 */
    rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnion", 0, NULL,
                                         1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&dest);

    /* Test with allow_inline = 0 */
    rc = struct_fields_init(&dest);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnion", 0, NULL,
                                         0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&dest);
    json_value_free(val);
    json_value_free(root_val);
  }

  /* 5. write_struct_to_json_schema all field attributes */
  {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(root);
    char *arr_types[2];
    cdd_c_error_t rc = 0;

    rc = struct_fields_init(&sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = struct_fields_add(&sf, "attr_field", "string", NULL, NULL, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sf.fields[0].deprecated_set = 1;
    sf.fields[0].deprecated = 1;
    sf.fields[0].read_only_set = 1;
    sf.fields[0].read_only = 1;
    sf.fields[0].write_only_set = 1;
    sf.fields[0].write_only = 1;
    CDD_STRCPY(sf.fields[0].description, sizeof(sf.fields[0].description),
               "Description text");
    CDD_STRCPY(sf.fields[0].format, sizeof(sf.fields[0].format), "date-time");
    c_cdd_strdup("{\"x-fld\":1}", &sf.fields[0].schema_extra_json);

    /* Array field with items_type_union and $ref */
    rc = struct_fields_add(&sf, "arr_ref_field", "array", "MyCustomModel", NULL,
                           NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    arr_types[0] = C_CDD_STR_LIT("string");
    arr_types[1] = C_CDD_STR_LIT("null");
    copy_string_array_code2schema(&sf.fields[1].items_type_union,
                                  &sf.fields[1].n_items_type_union, arr_types,
                                  2);
    c_cdd_strdup("{\"x-item\":2}", &sf.fields[1].items_extra_json);

    /* Enum field with direct ref */
    rc = struct_fields_add(&sf, "enum_field", "enum",
                           "#/components/schemas/MyEnum", NULL, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    c_cdd_strdup("{\"x-struct\":3}", &sf.schema_extra_json);

    rc = write_struct_to_json_schema(schemas_obj, "FullStruct", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&sf);
    json_value_free(root);
  }

  /* 6. parse_struct_member_line formats & code2schema_main nested structs */
  {
    struct StructFields sf;
    cdd_c_error_t rc = 0;
    FILE *fp;
    char out_path[256];
    char *argv[4];

    rc = struct_fields_init(&sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = parse_struct_member_line("uint32_t numbers[10];", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, sf.size);
    ASSERT(sf.fields[0].items_extra_json != NULL);
    ASSERT(strstr(sf.fields[0].items_extra_json, "int32") != NULL);

    rc = parse_struct_member_line("long int big_nums[5];", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(2, sf.size);
    ASSERT(sf.fields[1].items_extra_json != NULL);
    ASSERT(strstr(sf.fields[1].items_extra_json, "int64") != NULL);

    struct_fields_free(&sf);

    /* Test code2schema_main with nested struct, union pointer member, and
     * leading spaces */
    fp = fopen("test_nested_code2schema.h", "w");
    ASSERT(fp != NULL);
    fprintf(fp, "   \n");
    fprintf(fp, "   struct Outer {\n");
    fprintf(fp, "     struct {\n");
    fprintf(fp, "       int nested_val;\n");
    fprintf(fp, "     } inner;\n");
    fprintf(fp, "     int plain_val;\n");
    fprintf(fp, "   };\n");
    fprintf(fp, "   union PtrUnion {\n");
    fprintf(fp, "     char *ptr_name;\n");
    fprintf(fp, "     int id;\n");
    fprintf(fp, "   };\n");
    fclose(fp);

    CDD_SNPRINTF(out_path, sizeof(out_path), "test_nested_code2schema.json");
    argv[0] = C_CDD_STR_LIT("test_nested_code2schema.h");
    argv[1] = out_path;
    argv[2] = NULL;

    rc = code2schema_main(2, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    remove("test_nested_code2schema.h");
    remove(out_path);
  }

  /* 7. json_array_to_enum_members NULL args */
  {
    struct EnumMembers em;
    JSON_Value *val = json_parse_string("[\"A\", \"B\"]");
    JSON_Array *arr = json_value_get_array(val);
    cdd_c_error_t rc = 0;

    enum_members_init(&em);

    rc = json_array_to_enum_members(NULL, &em);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    rc = json_array_to_enum_members(arr, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    enum_members_free(&em);
    json_value_free(val);
  }

  /* 8. c2s_merge_schema_extras_object & strings edge cases */
  {
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    char *dest = NULL;
    cdd_c_error_t rc = 0;

    /* non-object json strings */
    rc = c2s_merge_schema_extras_object(obj, "[]");
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = c2s_merge_schema_extras_object(obj, "123");
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* dest is valid json, src is invalid */
    c_cdd_strdup("{\"x\":1}", &dest);
    rc = c2s_merge_schema_extras_strings(&dest, "invalid_json");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    C_CDD_FREE(dest);

    json_value_free(val);
  }

  /* 9. Union array in items */
  {
    struct StructFields sf;
    JSON_Value *val = json_parse_string(
        "{\"type\":\"object\",\"properties\":{\"tags\":{\"type\":\"array\","
        "\"items\":{\"type\":[\"string\",\"null\"]}}}}");
    JSON_Object *obj = json_value_get_object(val);
    cdd_c_error_t rc = 0;

    rc = struct_fields_init(&sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = c2s_json_object_to_struct_fields_internal(obj, &sf, NULL,
                                                   "UnionArrItems", 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, sf.size);
    ASSERT_STR_EQ("array", sf.fields[0].type);
    ASSERT_STR_EQ("string", sf.fields[0].ref);
    ASSERT(sf.fields[0].items_type_union != NULL);
    ASSERT_EQ(2, sf.fields[0].n_items_type_union);

    struct_fields_free(&sf);
    json_value_free(val);
  }

  /* 10. Null checks in helpers */
  {
    int is_prim = 0;
    char **props_out = NULL;
    size_t props_cnt = 0;
    cdd_c_error_t rc = 0;

    rc = ref_points_to_string_enum(NULL, "#/components/schemas/Test", &is_prim);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_prim);

    rc = c2s_collect_property_names(NULL, &props_out, &props_cnt);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, props_out);
    ASSERT_EQ(0, props_cnt);
  }

  /* 11. Helper error and null arguments */
  {
    char *out_val = NULL;
    char **out_arr = NULL;
    size_t out_cnt = 0;
    cdd_c_error_t rc = 0;

    /* sanitize_identifier invalid / OOM */
    rc = sanitize_identifier(NULL, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("Variant", out_val);
    C_CDD_FREE(out_val);
    rc = sanitize_identifier("foo", NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_alloc_fail = 1;
    rc = sanitize_identifier("foo", &out_val);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, out_val);
    g_cdd_strdup_fail = 1;
    rc = sanitize_identifier("", &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    /* make_unique_variant_name invalid */
    rc = make_unique_variant_name(NULL, "base", 0, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, out_val);

    /* make_inline_schema_name invalid */
    rc = make_inline_schema_name("Schema", "Variant", "Suffix", NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* c2s_collect_string_array OOM */
    {
      JSON_Value *arr_val = json_parse_string("[\"A\", \"B\"]");
      JSON_Array *arr = json_value_get_array(arr_val);
      g_cdd_alloc_fail = 1;
      rc = c2s_collect_string_array(arr, &out_arr, &out_cnt);
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      json_value_free(arr_val);
    }

    /* c2s_collect_property_names invalid & OOM */
    rc = c2s_collect_property_names(NULL, NULL, &out_cnt);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = c2s_collect_property_names(NULL, &out_arr, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    {
      JSON_Value *p_val =
          json_parse_string("{\"properties\":{\"x\":{\"type\":\"int\"}}}");
      JSON_Object *p_obj = json_value_get_object(p_val);
      g_cdd_alloc_fail = 1;
      rc = c2s_collect_property_names(p_obj, &out_arr, &out_cnt);
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      json_value_free(p_val);
    }

    /* discriminator_value_for_variant invalid & failure */
    rc = discriminator_value_for_variant(NULL, "Hint", NULL, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    {
      JSON_Value *d_val = json_parse_string(
          "{\"mapping\":{\"k1\":\"#/schemas/A\",\"k2\":\"B\",\"k3\":\"C\"}}");
      JSON_Object *d_obj = json_value_get_object(d_val);
      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(d_obj, "Hint", "#/schemas/A",
                                           &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(d_obj, "Hint", "#/schemas/B",
                                           &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(d_obj, "C", NULL, &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(d_obj, "FallbackS", NULL, &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(d_obj, NULL, "#/schemas/FallbackR",
                                           &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

      json_value_free(d_val);
    }
  }

  /* 12. c2s_merge_schema_extras_strings exhaustive tests */
  {
    char *dest = NULL;
    cdd_c_error_t rc = 0;

    /* dest_obj NULL */
    c_cdd_strdup("[]", &dest);
    rc = c2s_merge_schema_extras_strings(&dest, "{}");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    C_CDD_FREE(dest);

    /* src_obj NULL */
    c_cdd_strdup("{}", &dest);
    rc = c2s_merge_schema_extras_strings(&dest, "[]");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    C_CDD_FREE(dest);

    /* existing key replacement */
    c_cdd_strdup("{\"key\":1,\"other\":2}", &dest);
    rc = c2s_merge_schema_extras_strings(&dest, "{\"key\":3}");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(strstr(dest, "\"key\"") != NULL);
    C_CDD_FREE(dest);

    /* OOM on c_cdd_strdup */
    c_cdd_strdup("{\"a\":1}", &dest);
    g_cdd_strdup_fail = 1;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);
  }
  PASS();
}
SUITE(code2schema_internals_exhaustive_p1_suite) {
  RUN_TEST(test_code2schema_exhaustive_100_percent_coverage_part1);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_EXHAUSTIVE_P1_H */
