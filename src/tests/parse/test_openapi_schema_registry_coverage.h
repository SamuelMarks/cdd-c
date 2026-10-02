/**
 * @file test_openapi_schema_registry_coverage.h
 * @brief Comprehensive 100% test coverage for openapi_schema_registry.c.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_SCHEMA_REGISTRY_COVERAGE_H
#define TEST_OPENAPI_SCHEMA_REGISTRY_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
#include "openapi/parse/openapi_test_helpers.h"
/* clang-format on */

/**
 * @brief Tests apply_schema_ref_to_param and apply_schema_ref_to_header.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_registry_apply_schema_ref(void) {
  struct OpenAPI_Parameter p;
  struct OpenAPI_Header h;
  struct OpenAPI_SchemaRef sr;
  cdd_c_error_t rc = 0;

  memset(&p, 0, sizeof(p));
  memset(&h, 0, sizeof(h));
  memset(&sr, 0, sizeof(sr));

  /* 1. NULL checks */
  rc = cdd_test_apply_schema_ref_to_param(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_apply_schema_ref_to_param(&p, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_apply_schema_ref_to_param(NULL, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_test_apply_schema_ref_to_header(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_apply_schema_ref_to_header(&h, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_apply_schema_ref_to_header(NULL, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 2. Existing types freed */
  rc = c_cdd_strdup("old_type", &p.type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c_cdd_strdup("old_items", &p.items_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c_cdd_strdup("old_type", &h.type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c_cdd_strdup("old_items", &h.items_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  sr.inline_type = (char *)(size_t) "integer";
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("integer", p.type);
  ASSERT(p.items_type == NULL);
  ASSERT_EQ(0, p.is_array);

  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("integer", h.type);
  ASSERT(h.items_type == NULL);
  ASSERT_EQ(0, h.is_array);

  /* 3. Non-array with ref_name */
  sr.inline_type = NULL;
  sr.ref_name = (char *)(size_t) "UserRef";
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("UserRef", p.type);

  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("UserRef", h.type);

  /* 4. Array with inline_type */
  sr.is_array = 1;
  sr.inline_type = (char *)(size_t) "string";
  sr.ref_name = NULL;
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, p.is_array);
  ASSERT_STR_EQ("array", p.type);
  ASSERT_STR_EQ("string", p.items_type);

  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, h.is_array);
  ASSERT_STR_EQ("array", h.type);
  ASSERT_STR_EQ("string", h.items_type);

  /* 5. Array with ref_name */
  sr.inline_type = NULL;
  sr.ref_name = (char *)(size_t) "PetRef";
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("PetRef", p.items_type);

  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("PetRef", h.items_type);

  /* 6. Array with both inline_type and ref_name */
  sr.inline_type = (char *)(size_t) "object";
  sr.ref_name = (char *)(size_t) "OverrideRef";
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("OverrideRef", p.items_type);

  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("OverrideRef", h.items_type);

  /* 6b. Array with neither inline_type nor ref_name */
  sr.is_array = 1;
  sr.inline_type = NULL;
  sr.ref_name = NULL;
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, p.is_array);
  ASSERT_STR_EQ("array", p.type);
  ASSERT(p.items_type == NULL);

  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, h.is_array);
  ASSERT_STR_EQ("array", h.type);
  ASSERT(h.items_type == NULL);

  /* 7. OOM paths for param */
  sr.is_array = 1;
  sr.inline_type = (char *)(size_t) "object";
  sr.ref_name = (char *)(size_t) "OverrideRef";
  g_cdd_strdup_fail = 1;
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 2;
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 3;
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  sr.is_array = 0;
  sr.inline_type = (char *)(size_t) "int";
  sr.ref_name = NULL;
  g_cdd_strdup_fail = 1;
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  sr.inline_type = NULL;
  sr.ref_name = (char *)(size_t) "ref";
  g_cdd_strdup_fail = 1;
  rc = cdd_test_apply_schema_ref_to_param(&p, &sr);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* 8. OOM paths for header */
  sr.is_array = 1;
  sr.inline_type = (char *)(size_t) "object";
  sr.ref_name = (char *)(size_t) "OverrideRef";
  g_cdd_strdup_fail = 1;
  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 2;
  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 3;
  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  sr.is_array = 0;
  sr.inline_type = (char *)(size_t) "int";
  sr.ref_name = NULL;
  g_cdd_strdup_fail = 1;
  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  sr.inline_type = NULL;
  sr.ref_name = (char *)(size_t) "ref";
  g_cdd_strdup_fail = 1;
  rc = cdd_test_apply_schema_ref_to_header(&h, &sr);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* Cleanup */
  free(p.type);
  free(p.items_type);
  free(h.type);
  free(h.items_type);
  PASS();
}

/**
 * @brief Tests sanitize_component_name, make_unique_schema_name, and
 * schema_name_in_use.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_registry_names_and_sanitize(void) {
  struct OpenAPI_Spec spec;
  char *def_names[2];
  char *raw_names[2];
  char *out_name = NULL;
  cdd_c_error_t rc = 0;

  memset(&spec, 0, sizeof(spec));

  /* 1. sanitize_component_name NULL checks */
  rc = cdd_test_sanitize_component_name(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = cdd_test_sanitize_component_name(NULL, &out_name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("InlineSchema", out_name);
  free(out_name);
  out_name = NULL;

  rc = cdd_test_sanitize_component_name("", &out_name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("InlineSchema", out_name);
  free(out_name);
  out_name = NULL;

  /* 2. sanitize with special chars replaced by _ */
  rc = cdd_test_sanitize_component_name("Hello@World!123.test-ok_v", &out_name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Hello_World_123.test-ok_v", out_name);
  free(out_name);
  out_name = NULL;

  /* 3. sanitize with all invalid characters */
  rc = cdd_test_sanitize_component_name("@@@", &out_name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("___", out_name);
  free(out_name);
  out_name = NULL;

  /* 4. sanitize OOM on calloc */
  g_cdd_alloc_fail = 1;
  rc = cdd_test_sanitize_component_name("ValidName", &out_name);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  ASSERT(out_name == NULL);
  g_cdd_alloc_fail = 0;

  /* 5. sanitize OOM on fallback strdup */
  g_cdd_strdup_fail = 1;
  rc = cdd_test_sanitize_component_name(NULL, &out_name);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* 6. schema_name_in_use tests */
  rc = cdd_test_schema_name_in_use(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_schema_name_in_use(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_schema_name_in_use(&spec, "Test");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  def_names[0] = (char *)(size_t) "User";
  def_names[1] = (char *)(size_t) "Order";
  spec.defined_schema_names = def_names;
  spec.n_defined_schemas = 2;

  raw_names[0] = (char *)(size_t) "Raw1";
  raw_names[1] = (char *)(size_t) "Raw2";
  spec.raw_schema_names = raw_names;
  spec.n_raw_schemas = 2;

  rc = cdd_test_schema_name_in_use(&spec, "User");
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  rc = cdd_test_schema_name_in_use(&spec, "Order");
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  rc = cdd_test_schema_name_in_use(&spec, "Raw1");
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  rc = cdd_test_schema_name_in_use(&spec, "Raw2");
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  rc = cdd_test_schema_name_in_use(&spec, "NotFound");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* NULL arrays or NULL elements inside spec */
  {
    struct OpenAPI_Spec spec_nulls;
    char *null_arr[1];
    null_arr[0] = NULL;
    memset(&spec_nulls, 0, sizeof(spec_nulls));
    spec_nulls.n_defined_schemas = 1;
    spec_nulls.defined_schema_names = NULL;
    spec_nulls.n_raw_schemas = 1;
    spec_nulls.raw_schema_names = NULL;
    rc = cdd_test_schema_name_in_use(&spec_nulls, "Any");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_raw_schema_name_exists(&spec_nulls, "Any");
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    spec_nulls.defined_schema_names = null_arr;
    spec_nulls.raw_schema_names = null_arr;
    rc = cdd_test_schema_name_in_use(&spec_nulls, "Any");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_raw_schema_name_exists(&spec_nulls, "Any");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  /* 7. make_unique_schema_name tests */
  rc = cdd_test_make_unique_schema_name(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_make_unique_schema_name(&spec, NULL, &out_name);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = cdd_test_make_unique_schema_name(&spec, "NewModel", &out_name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("NewModel", out_name);
  free(out_name);
  out_name = NULL;

  /* Base in use -> generates NewModel_1 */
  rc = cdd_test_make_unique_schema_name(&spec, "User", &out_name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("User_1", out_name);
  free(out_name);
  out_name = NULL;

  /* OOM on make_unique_schema_name */
  g_cdd_strdup_fail = 1;
  rc = cdd_test_make_unique_schema_name(&spec, "NewModel", &out_name);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 1;
  rc = cdd_test_make_unique_schema_name(&spec, "User", &out_name);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* Exhausted attempts -> returns CDD_C_ERROR_NOT_FOUND */
  {
    char *exhausted_names[5];
    exhausted_names[0] = (char *)(size_t) "Busy";
    exhausted_names[1] = (char *)(size_t) "Busy_1";
    exhausted_names[2] = (char *)(size_t) "Busy_2";
    exhausted_names[3] = (char *)(size_t) "Busy_3";
    exhausted_names[4] = (char *)(size_t) "Busy_4";
    spec.defined_schema_names = exhausted_names;
    spec.n_defined_schemas = 5;
    out_name = NULL;
    rc = cdd_test_make_unique_schema_name(&spec, "Busy", &out_name);
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);
    ASSERT(out_name == NULL);
  }

  PASS();
}

/**
 * @brief Tests schema_type_array_includes and schema_object_is_object_like.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_registry_object_like(void) {
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  JSON_Array *ja = NULL;
  cdd_c_error_t rc = 0;

  /* 1. schema_type_array_includes NULL checks */
  rc = cdd_test_schema_type_array_includes(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_schema_type_array_includes(NULL, "object");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  jv = json_parse_string("[\"string\", 123, \"object\", null]");
  ja = json_value_get_array(jv);
  rc = cdd_test_schema_type_array_includes(ja, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_schema_type_array_includes(ja, "object");
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  rc = cdd_test_schema_type_array_includes(ja, "integer");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* 2. schema_object_is_object_like NULL check */
  rc = cdd_test_schema_object_is_object_like(NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 3. type == "object" */
  jv = json_parse_string("{\"type\": \"object\"}");
  jo = json_value_get_object(jv);
  rc = cdd_test_schema_object_is_object_like(jo);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  json_value_free(jv);

  /* 4. type array containing "object" */
  jv = json_parse_string("{\"type\": [\"null\", \"object\"]}");
  jo = json_value_get_object(jv);
  rc = cdd_test_schema_object_is_object_like(jo);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  json_value_free(jv);

  /* 5. properties */
  jv = json_parse_string("{\"properties\": {}}");
  jo = json_value_get_object(jv);
  rc = cdd_test_schema_object_is_object_like(jo);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  json_value_free(jv);

  /* 6. allOf / anyOf / oneOf */
  jv = json_parse_string("{\"allOf\": []}");
  jo = json_value_get_object(jv);
  rc = cdd_test_schema_object_is_object_like(jo);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  json_value_free(jv);

  jv = json_parse_string("{\"anyOf\": []}");
  jo = json_value_get_object(jv);
  rc = cdd_test_schema_object_is_object_like(jo);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  json_value_free(jv);

  jv = json_parse_string("{\"oneOf\": []}");
  jo = json_value_get_object(jv);
  rc = cdd_test_schema_object_is_object_like(jo);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  json_value_free(jv);

  /* 7. Scalar type (not object-like) */
  jv = json_parse_string("{\"type\": \"string\"}");
  jo = json_value_get_object(jv);
  rc = cdd_test_schema_object_is_object_like(jo);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  PASS();
}

/**
 * @brief Tests raw_schema_name_exists, append_raw_schema, and
 * append_defined_schema.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_registry_raw_and_defined_schemas(void) {
  struct OpenAPI_Spec spec;
  struct StructFields sf1;
  struct StructFields sf2;
  JSON_Value *jv = NULL;
  char *name = NULL;
  cdd_c_error_t rc = 0;

  memset(&spec, 0, sizeof(spec));
  rc = struct_fields_init(&sf1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = struct_fields_init(&sf2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 1. raw_schema_name_exists NULL checks */
  rc = cdd_test_raw_schema_name_exists(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_raw_schema_name_exists(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_raw_schema_name_exists(&spec, "Test");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 2. append_raw_schema NULL checks */
  jv = json_parse_string("{\"title\": \"RawSchema\"}");
  rc = cdd_test_append_raw_schema(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_append_raw_schema(&spec, NULL, jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_append_raw_schema(&spec, "RawSchema", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* 3. append_raw_schema success */
  rc = cdd_test_append_raw_schema(&spec, "RawSchema", jv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, spec.n_raw_schemas);
  ASSERT_STR_EQ("RawSchema", spec.raw_schema_names[0]);

  /* 4. append_raw_schema duplicate -> succeeds early */
  rc = cdd_test_append_raw_schema(&spec, "RawSchema", jv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, spec.n_raw_schemas);

  /* 5. append_raw_schema second item */
  rc = cdd_test_append_raw_schema(&spec, "RawSchema2", jv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, spec.n_raw_schemas);

  /* 6. append_raw_schema OOM */
  g_cdd_strdup_fail = 1;
  rc = cdd_test_append_raw_schema(&spec, "OOMSchema", jv);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  rc = cdd_test_append_raw_schema(&spec, "OOMSchema2", jv);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  g_cdd_alloc_fail = 1;
  rc = cdd_test_append_raw_schema(&spec, "OOMSchema3", jv);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  g_cdd_alloc_fail = 2;
  rc = cdd_test_append_raw_schema(&spec, "OOMSchema3b", jv);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  {
    extern C_CDD_EXPORT int g_cdd_fail_raw_schema_serialize;
    g_cdd_fail_raw_schema_serialize = 1;
    rc = cdd_test_append_raw_schema(&spec, "OOMSchema4", jv);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_raw_schema_serialize = 0;

    g_cdd_fail_raw_schema_serialize = 2;
    rc = cdd_test_append_raw_schema(&spec, "OOMSchema5", jv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_fail_raw_schema_serialize = 0;
  }

  json_value_free(jv);

  /* 7. append_defined_schema NULL checks */
  rc = cdd_test_append_defined_schema(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_append_defined_schema(&spec, NULL, &sf1);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c_cdd_strdup("DefSchema", &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_append_defined_schema(&spec, name, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* 8. append_defined_schema success */
  rc = cdd_test_append_defined_schema(&spec, name, &sf1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_STR_EQ("DefSchema", spec.defined_schema_names[0]);

  /* 9. append_defined_schema second item */
  rc = c_cdd_strdup("DefSchema2", &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_append_defined_schema(&spec, name, &sf2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, spec.n_defined_schemas);

  /* 10. append_defined_schema OOM */
  rc = c_cdd_strdup("DefSchema3", &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_alloc_fail = 1;
  rc = cdd_test_append_defined_schema(&spec, name, &sf2);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;
  free(name);

  /* 11. append_defined_schema with non-NULL ids, anchors, dynamic_anchors */
  {
    struct OpenAPI_Spec spec_extra;
    struct StructFields sf_extra1;
    struct StructFields sf_extra2;
    char *name_extra = NULL;
    char *id_extra = NULL;
    char *anchor_extra = NULL;
    char *dyn_anchor_extra = NULL;
    memset(&spec_extra, 0, sizeof(spec_extra));
    rc = struct_fields_init(&sf_extra1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = struct_fields_init(&sf_extra2);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = c_cdd_strdup("ExtraModel", &name_extra);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_append_defined_schema(&spec_extra, name_extra, &sf_extra1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = c_cdd_strdup("id1", &id_extra);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec_extra.defined_schema_ids[0] = id_extra;

    rc = c_cdd_strdup("anchor1", &anchor_extra);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec_extra.defined_schema_anchors[0] = anchor_extra;

    rc = c_cdd_strdup("dyn1", &dyn_anchor_extra);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec_extra.defined_schema_dynamic_anchors[0] = dyn_anchor_extra;

    rc = c_cdd_strdup("ExtraModel2", &name_extra);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_append_defined_schema(&spec_extra, name_extra, &sf_extra2);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    openapi_spec_free(&spec_extra);
  }

  /* 12. append_raw_schema with spec.raw_schema_names == NULL during realloc */
  {
    struct OpenAPI_Spec spec_raw;
    JSON_Value *jv_raw = json_parse_string("{\"type\": \"object\"}");
    memset(&spec_raw, 0, sizeof(spec_raw));
    spec_raw.n_raw_schemas = 1;
    spec_raw.raw_schema_names = NULL;
    spec_raw.raw_schema_json = NULL;
    rc = cdd_test_append_raw_schema(&spec_raw, "NewRaw", jv_raw);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    openapi_spec_free(&spec_raw);
    json_value_free(jv_raw);
  }

  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Tests register_inline_schema and assign_schema_ref_name.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_registry_register_inline(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_SchemaRef sr;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  char *out_name = NULL;
  cdd_c_error_t rc = 0;

  memset(&spec, 0, sizeof(spec));
  memset(&sr, 0, sizeof(sr));

  /* 1. assign_schema_ref_name NULL checks */
  rc = cdd_test_assign_schema_ref_name(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_assign_schema_ref_name(&sr, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_assign_schema_ref_name(NULL, "Name");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* 2. assign_schema_ref_name success */
  rc = cdd_test_assign_schema_ref_name(&sr, "FirstRef");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("FirstRef", sr.ref_name);

  /* 3. assign_schema_ref_name replacement */
  rc = cdd_test_assign_schema_ref_name(&sr, "SecondRef");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("SecondRef", sr.ref_name);

  /* 4. assign_schema_ref_name OOM */
  g_cdd_strdup_fail = 1;
  rc = cdd_test_assign_schema_ref_name(&sr, "ThirdRef");
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  {
    extern C_CDD_EXPORT int g_cdd_fail_assign_ref_name;
    g_cdd_fail_assign_ref_name = 1;
    rc = cdd_test_assign_schema_ref_name(&sr, "HookRef");
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_assign_ref_name = 0;

    g_cdd_fail_assign_ref_name = 2;
    rc = cdd_test_assign_schema_ref_name(&sr, "HookRef2");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_fail_assign_ref_name = 0;
  }
  free(sr.ref_name);
  sr.ref_name = NULL;

  /* 5. register_inline_schema NULL checks */
  jv = json_parse_string(
      "{\"type\": \"object\", \"properties\": {\"id\": {\"type\": "
      "\"integer\"}}}");
  jo = json_value_get_object(jv);

  rc = cdd_test_register_inline_schema(NULL, "Base", jo, jv, &out_name);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_register_inline_schema(&spec, "Base", NULL, jv, &out_name);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_register_inline_schema(&spec, "Base", jo, jv, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* 6. register_inline_schema success */
  rc = cdd_test_register_inline_schema(&spec, "UserPayload", jo, jv, &out_name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("UserPayload", out_name);
  ASSERT_EQ(1, spec.n_defined_schemas);

  /* 7. register duplicate inline schema creates unique suffix */
  out_name = NULL;
  rc = cdd_test_register_inline_schema(&spec, "UserPayload", jo, jv, &out_name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("UserPayload_1", out_name);
  ASSERT_EQ(2, spec.n_defined_schemas);

  /* 8. register inline schema with composition */
  json_value_free(jv);
  jv = json_parse_string("{\"allOf\": [{\"type\": \"object\"}]}");
  jo = json_value_get_object(jv);
  out_name = NULL;
  rc = cdd_test_register_inline_schema(&spec, "CompositeSchema", jo, jv,
                                       &out_name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("CompositeSchema", out_name);
  ASSERT(spec.n_raw_schemas >= 1);

  /* 8b. register inline schema with composition failure */
  {
    extern C_CDD_EXPORT int g_cdd_fail_raw_schema_serialize;
    g_cdd_fail_raw_schema_serialize = 1;
    out_name = NULL;
    rc = cdd_test_register_inline_schema(&spec, "CompositeFail", jo, jv,
                                         &out_name);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_raw_schema_serialize = 0;
  }

  /* 8c. register inline schema struct_fields_init failure */
  {
    extern C_CDD_EXPORT int g_struct_fields_init_fail;
    g_struct_fields_init_fail = 1;
    out_name = NULL;
    rc = cdd_test_register_inline_schema(&spec, "InitFail", jo, jv, &out_name);
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    g_struct_fields_init_fail = 0;
  }

  /* 8d. register inline schema with json_object_to_struct_fields_ex failure */
  {
    extern C_CDD_EXPORT int g_json_object_to_struct_fields_fail;
    g_json_object_to_struct_fields_fail = 1;
    out_name = NULL;
    rc = cdd_test_register_inline_schema(&spec, "BadSchema", jo, jv, &out_name);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_json_object_to_struct_fields_fail = 0;

    g_json_object_to_struct_fields_fail = 2;
    out_name = NULL;
    {
      JSON_Value *s_jv = json_parse_string("{\"type\": \"string\"}");
      JSON_Object *s_jo = json_value_get_object(s_jv);
      rc = cdd_test_register_inline_schema(&spec, "BadSchema2", s_jo, s_jv,
                                           &out_name);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      json_value_free(s_jv);
    }
    g_json_object_to_struct_fields_fail = 0;
  }

  /* 8e. register inline schema with composition and NULL schema_val */
  {
    out_name = NULL;
    rc = cdd_test_register_inline_schema(&spec, "CompNoVal", jo, NULL,
                                         &out_name);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("CompNoVal", out_name);
  }

  /* 9. register inline schema OOM */
  g_cdd_alloc_fail = 1;
  out_name = NULL;
  rc = cdd_test_register_inline_schema(&spec, "OOMFail", jo, jv, &out_name);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  g_cdd_alloc_fail = 0;

  g_cdd_strdup_fail = 1;
  out_name = NULL;
  rc = cdd_test_register_inline_schema(&spec, "OOMFail2", jo, jv, &out_name);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  g_cdd_strdup_fail = 0;

  json_value_free(jv);
  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Tests build_inline_request_name, build_inline_response_name, and
 * build_inline_param_name.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_registry_build_inline_names(void) {
  char *name = NULL;
  cdd_c_error_t rc = 0;

  /* 1. build_inline_request_name NULL check */
  rc = cdd_test_build_inline_request_name("op", 0, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* 2. build_inline_request_name variations */
  rc = cdd_test_build_inline_request_name("getUser", 0, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_getUser_Request", name);
  free(name);
  name = NULL;

  rc = cdd_test_build_inline_request_name("getUser", 1, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_getUser_Request_Item", name);
  free(name);
  name = NULL;

  rc = cdd_test_build_inline_request_name(NULL, 0, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_unnamed_Request", name);
  free(name);
  name = NULL;

  rc = cdd_test_build_inline_request_name("", 0, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_unnamed_Request", name);
  free(name);
  name = NULL;

  /* 3. build_inline_request_name OOM */
  g_cdd_alloc_fail = 1;
  rc = cdd_test_build_inline_request_name("op", 0, &name);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  ASSERT(name == NULL);
  g_cdd_alloc_fail = 0;

  /* 4. build_inline_response_name NULL check */
  rc = cdd_test_build_inline_response_name("op", "200", 0, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* 5. build_inline_response_name variations */
  rc = cdd_test_build_inline_response_name("getUser", "200", 0, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_getUser_Response_200", name);
  free(name);
  name = NULL;

  rc = cdd_test_build_inline_response_name("getUser", "200", 1, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_getUser_Response_200_Item", name);
  free(name);
  name = NULL;

  rc = cdd_test_build_inline_response_name(NULL, NULL, 0, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_unnamed_Response_default", name);
  free(name);
  name = NULL;

  rc = cdd_test_build_inline_response_name("", "", 1, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_unnamed_Response_default_Item", name);
  free(name);
  name = NULL;

  /* 6. build_inline_response_name OOM */
  g_cdd_alloc_fail = 1;
  rc = cdd_test_build_inline_response_name("op", "200", 0, &name);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  ASSERT(name == NULL);
  g_cdd_alloc_fail = 0;

  /* 7. build_inline_param_name NULL check */
  rc = cdd_test_build_inline_param_name("param", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* 8. build_inline_param_name variations */
  rc = cdd_test_build_inline_param_name("userId", &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_Querystring_userId", name);
  free(name);
  name = NULL;

  rc = cdd_test_build_inline_param_name(NULL, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_Querystring_param", name);
  free(name);
  name = NULL;

  rc = cdd_test_build_inline_param_name("", &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Inline_Querystring_param", name);
  free(name);
  name = NULL;

  /* 9. build_inline_param_name OOM */
  g_cdd_alloc_fail = 1;
  rc = cdd_test_build_inline_param_name("param", &name);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  ASSERT(name == NULL);
  g_cdd_alloc_fail = 0;

  PASS();
}

/**
 * @brief Registers schema registry tests in openapi suite.
 */
#define OPENAPI_SCHEMA_REGISTRY_TESTS()                                        \
  RUN_TEST(test_openapi_registry_apply_schema_ref);                            \
  RUN_TEST(test_openapi_registry_names_and_sanitize);                          \
  RUN_TEST(test_openapi_registry_object_like);                                 \
  RUN_TEST(test_openapi_registry_raw_and_defined_schemas);                     \
  RUN_TEST(test_openapi_registry_register_inline);                             \
  RUN_TEST(test_openapi_registry_build_inline_names)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_SCHEMA_REGISTRY_COVERAGE_H */
