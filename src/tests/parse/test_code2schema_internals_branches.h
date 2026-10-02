/**
 * @file test_code2schema_internals_branches.h
 * @brief Branch and edge case tests for code2schema.c.
 * @author Samuel Marks
 */

#ifndef TEST_CODE2SCHEMA_INTERNALS_BRANCHES_H
#define TEST_CODE2SCHEMA_INTERNALS_BRANCHES_H

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
 * @brief Tests c2s_json_object_to_struct_fields_internal comprehensively.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_json_object_to_struct_fields_full(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Value *schema_val;
  JSON_Object *schema_obj;
  struct StructFields sf;
  cdd_c_error_t rc = 0;

  root_val = json_parse_string(
      "{\"StatusEnum\":{\"type\":\"string\",\"enum\":[\"OK\",\"FAIL\"]},"
      "\"SubObj\":{\"type\":\"object\",\"properties\":{\"tag\":{\"type\":"
      "\"string\"}}}}");
  root = json_value_get_object(root_val);

  schema_val = json_value_init_object();
  schema_obj = json_value_get_object(schema_val);
  json_object_set_string(schema_obj, "type", "object");
  {
    JSON_Value *req_v = json_parse_string("[\"field_req\"]");
    json_object_set_value(schema_obj, "required", req_v);
  }
  {
    JSON_Value *p1 = json_parse_string(
        "{\"field_req\":{\"type\":\"string\"},"
        "\"field_int\":{\"type\":\"integer\",\"default\":42,\"minimum\":0,"
        "\"exclusiveMinimum\":false,\"maximum\":100,\"exclusiveMaximum\":true},"
        "\"field_float\":{\"type\":\"number\",\"default\":3.14,\"minimum\":0.5,"
        "\"maximum\":99.9},"
        "\"field_bool\":{\"type\":\"boolean\",\"default\":true}}");
    JSON_Value *p2 = json_parse_string(
        "{\"field_str\":{\"type\":\"string\",\"minLength\":1,\"maxLength\":50,"
        "\"pattern\":\"^[a-z]+$\",\"format\":\"email\",\"deprecated\":true,"
        "\"readOnly\":true,\"writeOnly\":false},"
        "\"field_enum_ref\":{\"$ref\":\"#/components/schemas/StatusEnum\"},"
        "\"field_obj_ref\":{\"$ref\":\"#/components/schemas/SubObj\"}}");
    JSON_Value *p3 = json_parse_string(
        "{\"field_arr_enum\":{\"type\":\"array\",\"items\":{\"$ref\":\"#/"
        "components/schemas/StatusEnum\"}},"
        "\"field_arr_union\":{\"type\":\"array\",\"items\":{\"type\":["
        "\"string\",\"null\"]}},"
        "\"field_arr_obj\":{\"type\":\"array\",\"items\":{\"type\":\"object\"},"
        "\"minItems\":1,\"maxItems\":5,\"uniqueItems\":true}}");
    JSON_Object *props_obj;
    JSON_Value *props_val = json_value_init_object();
    props_obj = json_value_get_object(props_val);
    c2s_merge_schema_extras_object(props_obj, json_serialize_to_string(p1));
    c2s_merge_schema_extras_object(props_obj, json_serialize_to_string(p2));
    c2s_merge_schema_extras_object(props_obj, json_serialize_to_string(p3));
    json_object_set_value(schema_obj, "properties", props_val);
    json_value_free(p1);
    json_value_free(p2);
    json_value_free(p3);
  }

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = c2s_json_object_to_struct_fields_internal(schema_obj, &sf, root,
                                                 "FullModel", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(sf.size >= 10);

  struct_fields_free(&sf);
  json_value_free(schema_val);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests apply_union_to_struct_fields_ex with discriminator and inline
 * promotion.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_apply_union_ex_full(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Value *union_val;
  JSON_Array *union_arr;
  JSON_Value *schema_val;
  JSON_Object *schema_obj;
  struct StructFields sf;
  cdd_c_error_t rc = 0;

  root_val = json_value_init_object();
  root = json_value_get_object(root_val);

  schema_val =
      json_parse_string("{\"discriminator\":{\"propertyName\":\"kind\","
                        "\"mapping\":{\"cat\":\"CatModel\"}}}");
  schema_obj = json_value_get_object(schema_val);

  union_val = json_parse_string(
      "["
      "  "
      "{\"title\":\"CatModel\",\"type\":\"object\",\"properties\":{\"meow\":{"
      "\"type\":\"boolean\"}},\"required\":[\"meow\"]},"
      "  {\"type\":\"array\",\"items\":{\"type\":\"string\"}}"
      "]");
  union_arr = json_value_get_array(union_val);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "PetUnion", 0,
                                       schema_obj, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, sf.is_union);
  ASSERT_EQ(2, sf.n_union_variants);
  ASSERT_STR_EQ("kind", sf.union_discriminator);

  struct_fields_free(&sf);
  json_value_free(schema_val);
  json_value_free(union_val);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests code2schema_main CLI with error paths and rich C headers.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_main_full_suite(void) {
  char *argv[3];
  char test_h[256];
  char test_json[256];
  FILE *f;
  cdd_c_error_t rc = 0;

  /* Wrong argc */
  rc = code2schema_main(1, NULL);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  /* Nonexistent input file */
  argv[0] = (char *)(size_t) "code2schema";
  argv[1] = (char *)(size_t) "/nonexistent/missing_file.h";
  rc = code2schema_main(2, argv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  /* Create temporary C header file */
  CDD_SNPRINTF(test_h, sizeof(test_h), "c2s_tmp_%u.h", (unsigned)rand());
  CDD_SNPRINTF(test_json, sizeof(test_json), "c2s_tmp_%u.json",
               (unsigned)rand());

  f = fopen(test_h, "w");
  ASSERT(f != NULL);
  fputs("/* C Header with struct, union, enum, and nested structs */\n"
        "union Shape {\n"
        "  int circle_radius;\n"
        "  char* label;\n"
        "};\n\n"
        "enum Color {\n"
        "  RED,\n"
        "  GREEN,\n"
        "  BLUE\n"
        "};\n\n"
        "struct UserProfile {\n"
        "  int id;\n"
        "  char* username;\n"
        "  struct {\n"
        "    char* street;\n"
        "    int zip;\n"
        "  } address;\n"
        "  size_t n_roles;\n"
        "  char* roles;\n"
        "};\n\n"
        "enum { ANON_A, ANON_B };\n\n"
        "enum Broken { VAL1\n",
        f);
  fclose(f);

  argv[0] = test_h;
  argv[1] = test_json;
  rc = code2schema_main(2, argv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Unwritable output path */
  argv[1] = (char *)(size_t) "/proc/nonexistent_dir/out.json";
  rc = code2schema_main(2, argv);
  ASSERT(rc != CDD_C_SUCCESS);

  remove(test_h);
  remove(test_json);
  PASS();
}

/**
 * @brief Tests c2s_union_array_items_supported branches.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_union_array_items_branches(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Value *val;
  JSON_Object *obj;
  int supported = 0;
  cdd_c_error_t rc = 0;

  root_val = json_parse_string(
      "{\"StatusEnum\":{\"type\":\"string\",\"enum\":[\"A\",\"B\"]}}");
  root = json_value_get_object(root_val);

  /* Items with $ref to enum */
  val = json_parse_string(
      "{\"items\":{\"$ref\":\"#/components/schemas/StatusEnum\"}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, root, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, supported);
  json_value_free(val);

  /* Items with type union array */
  val = json_parse_string("{\"items\":{\"type\":[\"string\",\"null\"]}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, root, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, supported);
  json_value_free(val);

  /* Items with anonymous properties */
  val = json_parse_string(
      "{\"items\":{\"properties\":{\"name\":{\"type\":\"string\"}}}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, root, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, supported);
  json_value_free(val);

  /* Items with empty object (no type, ref, or properties) */
  val = json_parse_string("{\"items\":{}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, root, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, supported);
  json_value_free(val);

  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests c2s_merge_schema_extras_strings with invalid JSON and key
 * collisions.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_merge_schema_extras_strings_branches(void) {
  char *dest = NULL;
  cdd_c_error_t rc = 0;

  /* Dest is not a JSON object */
  dest = (char *)malloc(32);
  CDD_STRCPY(dest, 32, "not_json");
  rc = c2s_merge_schema_extras_strings(&dest, "{\"a\":1}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free(dest);
  dest = NULL;

  /* Dest is a JSON array, not object */
  dest = (char *)malloc(32);
  CDD_STRCPY(dest, 32, "[1, 2]");
  rc = c2s_merge_schema_extras_strings(&dest, "{\"a\":1}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free(dest);
  dest = NULL;

  /* Key overwrite branch: both have key "key" */
  dest = (char *)malloc(64);
  CDD_STRCPY(dest, 64, "{\"key\":\"old_val\",\"other\":1}");
  rc = c2s_merge_schema_extras_strings(&dest, "{\"key\":\"new_val\"}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(strstr(dest, "new_val") != NULL);
  C_CDD_FREE(dest);
  dest = NULL;

  PASS();
}

/**
 * @brief Tests merge_struct_fields when src is an enum or has type unions.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_merge_struct_fields_enums_and_extras(void) {
  struct StructFields src_sf;
  struct StructFields dst_sf;
  char *u1[1];
  char *u2[1];
  cdd_c_error_t rc = 0;

  u1[0] = (char *)(size_t) "string";
  u2[0] = (char *)(size_t) "null";

  /* src is_enum */
  rc = struct_fields_init(&src_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = struct_fields_init(&dst_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  src_sf.is_enum = 1;
  rc = enum_members_init(&src_sf.enum_members);
  ASSERT_EQ(0, rc);
  rc = enum_members_add(&src_sf.enum_members, "VAL");
  ASSERT_EQ(0, rc);

  rc = merge_struct_fields(&dst_sf, &src_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  struct_fields_free(&src_sf);
  struct_fields_free(&dst_sf);

  /* src field has type_union and items_type_union and items_extra_json */
  rc = struct_fields_init(&src_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = struct_fields_init(&dst_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = struct_fields_add(&src_sf, "rich_field", "array", "Item", NULL, NULL);
  ASSERT_EQ(0, rc);
  c_cdd_strdup("{\"x-item\":true}", &src_sf.fields[0].items_extra_json);
  copy_string_array_code2schema(&src_sf.fields[0].type_union,
                                &src_sf.fields[0].n_type_union, u1, 1);
  copy_string_array_code2schema(&src_sf.fields[0].items_type_union,
                                &src_sf.fields[0].n_items_type_union, u2, 1);

  rc = merge_struct_fields(&dst_sf, &src_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, dst_sf.size);
  ASSERT(dst_sf.fields[0].items_extra_json != NULL);
  ASSERT(dst_sf.fields[0].type_union != NULL);
  ASSERT(dst_sf.fields[0].items_type_union != NULL);

  struct_fields_free(&src_sf);
  struct_fields_free(&dst_sf);
  PASS();
}

/**
 * @brief Tests apply_union_to_struct_fields_ex with all variant types and edge
 * cases.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_apply_union_variants_all_types(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Value *union_val;
  JSON_Array *union_arr;
  struct StructFields sf;
  cdd_c_error_t rc = 0;

  root_val =
      json_parse_string("{\"MyEnum\":{\"type\":\"string\",\"enum\":[\"X\"]}}");
  root = json_value_get_object(root_val);

  /* Variants covering: number, boolean, null, object with title, array with
   * enum items */
  union_val = json_parse_string(
      "["
      "  {\"type\":\"number\"},"
      "  {\"type\":\"boolean\"},"
      "  {\"type\":\"null\"},"
      "  "
      "{\"title\":\"MyObj\",\"type\":\"object\",\"properties\":{\"p\":{"
      "\"type\":\"integer\"}}},"
      "  "
      "{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/schemas/"
      "MyEnum\"}},"
      "  {\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]}},"
      "  "
      "{\"type\":\"array\",\"items\":{\"properties\":{\"sub\":{\"type\":"
      "\"string\"}}}}"
      "]");
  union_arr = json_value_get_array(union_val);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "AllUnion", 0,
                                       NULL, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, sf.is_union);
  ASSERT_EQ(7, sf.n_union_variants);

  /* Already has fields -> returns success early */
  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "AllUnion", 0,
                                       NULL, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  struct_fields_free(&sf);
  json_value_free(union_val);

  /* Empty union array -> returns success early */
  union_val = json_parse_string("[]");
  union_arr = json_value_get_array(union_val);
  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "EmptyUnion", 0,
                                       NULL, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  struct_fields_free(&sf);
  json_value_free(union_val);

  /* Array variant when allow_inline is 0 */
  union_val = json_parse_string(
      "[{\"type\":\"array\",\"items\":{\"type\":\"string\"}}]");
  union_arr = json_value_get_array(union_val);
  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "ArrayNoInline", 0,
                                       NULL, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  struct_fields_free(&sf);
  json_value_free(union_val);

  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests json_array_to_enum_members with non-string elements.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_json_array_to_enum_members_nonstrings(void) {
  JSON_Value *val = json_parse_string("[123, \"VALID\", true, null]");
  JSON_Array *arr = json_value_get_array(val);
  struct EnumMembers em;
  cdd_c_error_t rc = 0;

  rc = enum_members_init(&em);
  ASSERT_EQ(0, rc);

  rc = json_array_to_enum_members(arr, &em);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, em.size);
  ASSERT_STR_EQ("VALID", em.members[0]);

  enum_members_free(&em);
  json_value_free(val);
  PASS();
}

/**
 * @brief Tests out-of-memory error paths across code2schema functions using
 * g_cdd_alloc_fail.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_alloc_failures(void) {
  char *src_arr[2];
  char **dst_arr = NULL;
  size_t dst_cnt = 0;
  JSON_Value *val;
  JSON_Array *arr;
  JSON_Object *obj;
  char *str_out = NULL;
  struct StructFields sf1;
  struct StructFields sf2;
  cdd_c_error_t rc = 0;

  src_arr[0] = (char *)(size_t) "alpha";
  src_arr[1] = (char *)(size_t) "beta";

  /* copy_string_array_code2schema OOM */
  g_cdd_alloc_fail = 1;
  rc = copy_string_array_code2schema(&dst_arr, &dst_cnt, src_arr, 2);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  g_cdd_strdup_fail = 1;
  rc = copy_string_array_code2schema(&dst_arr, &dst_cnt, src_arr, 2);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* parse_type_union_array_code2schema OOM */
  val = json_parse_string("[\"string\",\"integer\"]");
  arr = json_value_get_array(val);

  g_cdd_alloc_fail = 1;
  rc = parse_type_union_array_code2schema(arr, &dst_arr, &dst_cnt, NULL, NULL);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  g_cdd_strdup_fail = 1;
  rc = parse_type_union_array_code2schema(arr, &dst_arr, &dst_cnt, NULL, NULL);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  json_value_free(val);

  /* c2s_collect_string_array OOM */
  val = json_parse_string("[\"one\",\"two\"]");
  arr = json_value_get_array(val);

  g_cdd_alloc_fail = 1;
  rc = c2s_collect_string_array(arr, &dst_arr, &dst_cnt);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  g_cdd_strdup_fail = 1;
  rc = c2s_collect_string_array(arr, &dst_arr, &dst_cnt);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  json_value_free(val);

  /* c2s_collect_property_names OOM */
  val = json_parse_string("{\"properties\":{\"a\":{},\"b\":{}}}");
  obj = json_value_get_object(val);

  g_cdd_alloc_fail = 1;
  rc = c2s_collect_property_names(obj, &dst_arr, &dst_cnt);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  g_cdd_strdup_fail = 1;
  rc = c2s_collect_property_names(obj, &dst_arr, &dst_cnt);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  json_value_free(val);

  /* sanitize_identifier OOM */
  g_cdd_alloc_fail = 1;
  rc = sanitize_identifier("my_var", &str_out);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str_out);

  g_cdd_strdup_fail = 1;
  rc = sanitize_identifier(NULL, &str_out);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* c2s_merge_schema_extras_strings OOM */
  str_out = NULL;
  g_cdd_strdup_fail = 1;
  rc = c2s_merge_schema_extras_strings(&str_out, "{\"a\":1}");
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* merge_struct_fields OOM */
  struct_fields_init(&sf1);
  struct_fields_init(&sf2);
  struct_fields_add(&sf1, "src_f", "string", NULL, NULL, NULL);
  c_cdd_strdup("{\"extra\":1}", &sf1.schema_extra_json);

  g_cdd_strdup_fail = 1;
  rc = merge_struct_fields(&sf2, &sf1);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  struct_fields_free(&sf1);
  struct_fields_free(&sf2);

  /* apply_union_to_struct_fields_ex OOM */
  val = json_parse_string("[{\"type\":\"string\"}]");
  arr = json_value_get_array(val);
  struct_fields_init(&sf1);

  g_cdd_alloc_fail = 1;
  rc = apply_union_to_struct_fields_ex(arr, &sf1, NULL, "UnionOOM", 0, NULL, 0);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  struct_fields_free(&sf1);
  json_value_free(val);

  PASS();
}

/**
 * @brief Tests ref_points_to_string_enum and resolve_schema_ref_object empty
 * ref paths.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_empty_refs_and_defaults(void) {
  JSON_Value *root_val = json_value_init_object();
  JSON_Object *root = json_value_get_object(root_val);
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  JSON_Object *res = NULL;
  struct StructField f;
  char *name = NULL;
  int is_enum = 0;
  int b = 0;
  cdd_c_error_t rc = 0;

  /* Empty and slash refs */
  rc = ref_points_to_string_enum(root, "", &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);

  rc = ref_points_to_string_enum(root, "/", &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);

  rc = resolve_schema_ref_object(root, "", &res);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, res);

  rc = resolve_schema_ref_object(root, "/", &res);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, res);

  /* Test str_after_last failure */
  {
    extern C_CDD_EXPORT int g_cdd_fail_str_after_last;
    g_cdd_fail_str_after_last = 1;
    rc = ref_points_to_string_enum(root, "#/components/schemas/Status",
                                   &is_enum);
    g_cdd_fail_str_after_last = 0;
    ASSERT(rc != CDD_C_SUCCESS);

    g_cdd_fail_str_after_last = 1;
    rc = resolve_schema_ref_object(root, "#/components/schemas/Status", &res);
    g_cdd_fail_str_after_last = 0;
    ASSERT(rc != CDD_C_SUCCESS);
  }

  /* str_starts_with error branch */
  {
    extern C_CDD_EXPORT int g_cdd_fail_str_starts_with;
    g_cdd_fail_str_starts_with = 1;
    rc = str_starts_with("abc", "a", &b);
    g_cdd_fail_str_starts_with = 0;
    ASSERT(rc != CDD_C_SUCCESS);
  }

  /* make_inline_schema_name with NULL args */
  rc = make_inline_schema_name(NULL, NULL, NULL, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Union_Variant", name);
  C_CDD_FREE(name);
  name = NULL;

  /* c2s_write_default_value for object type */
  memset(&f, 0, sizeof(f));
  CDD_STRCPY(f.type, sizeof(f.type), "object");
  CDD_STRCPY(f.default_val, sizeof(f.default_val), "{}");
  rc = c2s_write_default_value(obj, &f);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* c2s_write_array_constraints for non-array */
  CDD_STRCPY(f.type, sizeof(f.type), "string");
  rc = c2s_write_array_constraints(obj, &f);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* c2s_write_type_union with NULL type and NULL union */
  rc = c2s_write_type_union(obj, NULL, NULL, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  json_value_free(val);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests c2s_json_object_to_struct_fields_internal with union array and
 * empty properties.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_json_object_fields_more_branches(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct StructFields sf;
  cdd_c_error_t rc = 0;

  /* Schema with boolean exclusiveMinimum and field without type/ref (empty
   * property) */
  val = json_parse_string("{\"type\":\"object\","
                          "\"properties\":{"
                          "  \"empty_p\":{},"
                          "  "
                          "\"num_bool_ex\":{\"type\":\"number\",\"minimum\":5."
                          "0,\"exclusiveMinimum\":true},"
                          "  "
                          "\"arr_union_type\":{\"type\":[\"array\",\"null\"],"
                          "\"items\":{\"type\":\"string\"}}"
                          "}}");
  obj = json_value_get_object(val);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = c2s_json_object_to_struct_fields_internal(obj, &sf, NULL, "MoreBranches",
                                                 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, sf.size);
  ASSERT_EQ(1, sf.fields[0].exclusive_min);

  struct_fields_free(&sf);
  json_value_free(val);
  PASS();
}

SUITE(code2schema_internals_branches_suite) {
  RUN_TEST(test_code2schema_json_object_to_struct_fields_full);
  RUN_TEST(test_code2schema_apply_union_ex_full);
  RUN_TEST(test_code2schema_main_full_suite);
  RUN_TEST(test_code2schema_c2s_union_array_items_branches);
  RUN_TEST(test_code2schema_merge_schema_extras_strings_branches);
  RUN_TEST(test_code2schema_merge_struct_fields_enums_and_extras);
  RUN_TEST(test_code2schema_apply_union_variants_all_types);
  RUN_TEST(test_code2schema_json_array_to_enum_members_nonstrings);
  RUN_TEST(test_code2schema_alloc_failures);
  RUN_TEST(test_code2schema_empty_refs_and_defaults);
  RUN_TEST(test_code2schema_json_object_fields_more_branches);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_BRANCHES_H */
