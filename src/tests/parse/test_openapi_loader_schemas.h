/**
 * @file test_openapi_loader_schemas.h
 * @brief Type unions, array items, and numeric constraint tests.
 */

#ifndef TEST_OPENAPI_LOADER_SCHEMAS_H
#define TEST_OPENAPI_LOADER_SCHEMAS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

TEST test_load_inline_schema_items_type_union(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  116, 97,  103, 115, 34,  44,
      34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  115,
      99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,
      34,  97,  114, 114, 97,  121, 34,  44,  34,  105, 116, 101, 109, 115, 34,
      58,  123, 34,  116, 121, 112, 101, 34,  58,  91,  34,  115, 116, 114, 105,
      110, 103, 34,  44,  34,  105, 110, 116, 101, 103, 101, 114, 34,  93,  125,
      125, 125, 93,  44,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,
      58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114,
      105, 112, 116, 105, 111, 110, 34,  58,  34,  79,  75,  34,  125, 125, 125,
      125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_EQ(1, p->schema.is_array);
    ASSERT_STR_EQ("string", p->schema.inline_type);
    ASSERT_EQ(2, p->schema.n_items_type_union);
    ASSERT_STR_EQ("string", p->schema.items_type_union[0]);
    ASSERT_STR_EQ("integer", p->schema.items_type_union[1]);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_schema_boolean_and_numeric_enum(void) {
  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  97,  110, 121, 34,  44,  34,
      105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  115, 99,
      104, 101, 109, 97,  34,  58,  116, 114, 117, 101, 125, 44,  123, 34,  110,
      97,  109, 101, 34,  58,  34,  108, 101, 118, 101, 108, 34,  44,  34,  105,
      110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  115, 99,  104,
      101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  105,
      110, 116, 101, 103, 101, 114, 34,  44,  34,  101, 110, 117, 109, 34,  58,
      91,  49,  44,  50,  93,  125, 125, 93,  44,  34,  114, 101, 115, 112, 111,
      110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,
      100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  79,
      75,  34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *any_p =
        &spec.paths[0].operations[0].parameters[0];
    struct OpenAPI_Parameter *lvl_p =
        &spec.paths[0].operations[0].parameters[1];
    ASSERT_EQ(1, any_p->schema_set);
    ASSERT_EQ(1, any_p->schema.schema_is_boolean);
    ASSERT_EQ(1, any_p->schema.schema_boolean_value);

    ASSERT_EQ(1, lvl_p->schema_set);
    ASSERT_EQ(2, lvl_p->schema.n_enum_values);
    ASSERT_EQ(OA_ANY_NUMBER, lvl_p->schema.enum_values[0].type);
    ASSERT_EQ(1.0, lvl_p->schema.enum_values[0].number);
    ASSERT_EQ(OA_ANY_NUMBER, lvl_p->schema.enum_values[1].type);
    ASSERT_EQ(2.0, lvl_p->schema.enum_values[1].number);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_schema_items_examples_and_boolean_items(void) {
  const char *json =
      "{\"paths\":{\"/"
      "p\":{\"get\":{\"parameters\":[{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"string\","
      "\"examples\":[\"a\",\"b\"]}}},{\"name\":\"anys\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"items\":false}}],\"responses\":{"
      "\"200\":{\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *tags = &spec.paths[0].operations[0].parameters[0];
    struct OpenAPI_Parameter *anys = &spec.paths[0].operations[0].parameters[1];

    ASSERT_EQ(1, tags->schema_set);
    ASSERT_EQ(1, tags->schema.is_array);
    ASSERT_EQ(2, tags->schema.n_items_examples);
    ASSERT_EQ(OA_ANY_STRING, tags->schema.items_examples[0].type);
    ASSERT_STR_EQ("a", tags->schema.items_examples[0].string);
    ASSERT_EQ(OA_ANY_STRING, tags->schema.items_examples[1].type);
    ASSERT_STR_EQ("b", tags->schema.items_examples[1].string);

    ASSERT_EQ(1, anys->schema_set);
    ASSERT_EQ(1, anys->schema.is_array);
    ASSERT_EQ(1, anys->schema.items_schema_is_boolean);
    ASSERT_EQ(0, anys->schema.items_schema_boolean_value);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_schema_example_and_numeric_constraints(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  115, 99,  111, 114, 101, 34,
      44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,
      115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,
      58,  34,  110, 117, 109, 98,  101, 114, 34,  44,  34,  109, 105, 110, 105,
      109, 117, 109, 34,  58,  49,  44,  34,  101, 120, 99,  108, 117, 115, 105,
      118, 101, 77,  97,  120, 105, 109, 117, 109, 34,  58,  57,  44,  34,  101,
      120, 97,  109, 112, 108, 101, 34,  58,  50,  46,  53,  125, 125, 93,  44,
      34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,
      48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105,
      111, 110, 34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].parameters[0].schema;
    ASSERT_EQ(1, schema->has_min);
    ASSERT_EQ(1.0, schema->min_val);
    ASSERT_EQ(1, schema->has_max);
    ASSERT_EQ(9.0, schema->max_val);
    ASSERT_EQ(1, schema->exclusive_max);
    ASSERT_EQ(1, schema->example_set);
    ASSERT_EQ(OA_ANY_NUMBER, schema->example.type);
    ASSERT_EQ(2.5, schema->example.number);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_schema_array_constraints_and_items_example(void) {

  const char json[] = {
      123, 34,  112, 97,  116, 104, 115, 34,  58,  123, 34,  47,  112, 34,  58,
      123, 34,  103, 101, 116, 34,  58,  123, 34,  112, 97,  114, 97,  109, 101,
      116, 101, 114, 115, 34,  58,  91,  123, 34,  110, 97,  109, 101, 34,  58,
      34,  116, 97,  103, 115, 34,  44,  34,  105, 110, 34,  58,  34,  113, 117,
      101, 114, 121, 34,  44,  34,  115, 99,  104, 101, 109, 97,  34,  58,  123,
      34,  116, 121, 112, 101, 34,  58,  34,  97,  114, 114, 97,  121, 34,  44,
      34,  109, 105, 110, 73,  116, 101, 109, 115, 34,  58,  49,  44,  34,  109,
      97,  120, 73,  116, 101, 109, 115, 34,  58,  51,  44,  34,  117, 110, 105,
      113, 117, 101, 73,  116, 101, 109, 115, 34,  58,  116, 114, 117, 101, 44,
      34,  105, 116, 101, 109, 115, 34,  58,  123, 34,  116, 121, 112, 101, 34,
      58,  34,  115, 116, 114, 105, 110, 103, 34,  44,  34,  109, 105, 110, 76,
      101, 110, 103, 116, 104, 34,  58,  50,  44,  34,  109, 97,  120, 76,  101,
      110, 103, 116, 104, 34,  58,  53,  44,  34,  112, 97,  116, 116, 101, 114,
      110, 34,  58,  34,  94,  91,  97,  45,  122, 93,  43,  36,  34,  44,  34,
      101, 120, 97,  109, 112, 108, 101, 34,  58,  34,  97,  98,  34,  125, 125,
      125, 93,  44,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,
      123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105,
      112, 116, 105, 111, 110, 34,  58,  34,  79,  75,  34,  125, 125, 125, 125,
      125, 44,  34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,
      50,  46,  48,  34,  125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].parameters[0].schema;
    ASSERT_EQ(1, schema->has_min_items);
    ASSERT_EQ(1u, schema->min_items);
    ASSERT_EQ(1, schema->has_max_items);
    ASSERT_EQ(3u, schema->max_items);
    ASSERT_EQ(1, schema->unique_items);
    ASSERT_EQ(1, schema->items_has_min_len);
    ASSERT_EQ(2u, schema->items_min_len);
    ASSERT_EQ(1, schema->items_has_max_len);
    ASSERT_EQ(5u, schema->items_max_len);
    ASSERT_STR_EQ("^[a-z]+$", schema->items_pattern);
    ASSERT_EQ(1, schema->items_example_set);
    ASSERT_EQ(OA_ANY_STRING, schema->items_example.type);
    ASSERT_STR_EQ("ab", schema->items_example.string);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_schema_items_const_default_and_extras(void) {

  const char *json =
      "{\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"x-top\":true,\"items\":{\"type\":"
      "\"string\",\"const\":\"x\",\"default\":\"y\",\"x-custom\":99}}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}}}},\"openapi\":\"3.2."
      "0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    struct OpenAPI_SchemaRef *schema = &p->schema;
    ASSERT_EQ(1, schema->is_array);
    ASSERT_EQ(1, schema->items_const_value_set);
    ASSERT_EQ(OA_ANY_STRING, schema->items_const_value.type);
    ASSERT_STR_EQ("x", schema->items_const_value.string);
    ASSERT_EQ(1, schema->items_default_value_set);
    ASSERT_EQ(OA_ANY_STRING, schema->items_default_value.type);
    ASSERT_STR_EQ("y", schema->items_default_value.string);
    ASSERT(schema->schema_extra_json != NULL);
    ASSERT(strstr(schema->schema_extra_json, "\"x-top\"") != NULL);
    ASSERT(schema->items_extra_json != NULL);
    ASSERT(strstr(schema->items_extra_json, "\"x-custom\"") != NULL);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_request_body_object_promoted(void) {
  struct StructFields *_ast_openapi_spec_find_schema_3;
  struct StructField *_ast_struct_fields_get_4 = NULL;

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  99,  111, 109, 112, 111,
      110, 101, 110, 116, 115, 34,  58,  123, 34,  115, 99,  104, 101, 109, 97,
      115, 34,  58,  123, 34,  73,  110, 108, 105, 110, 101, 95,  99,  114, 101,
      97,  116, 101, 80,  101, 116, 95,  82,  101, 113, 117, 101, 115, 116, 34,
      58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,
      116, 34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,
      58,  123, 34,  105, 100, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,
      34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 125, 125, 44,  34,
      112, 97,  116, 104, 115, 34,  58,  123, 34,  47,  112, 101, 116, 115, 34,
      58,  123, 34,  112, 111, 115, 116, 34,  58,  123, 34,  111, 112, 101, 114,
      97,  116, 105, 111, 110, 73,  100, 34,  58,  34,  99,  114, 101, 97,  116,
      101, 80,  101, 116, 34,  44,  34,  114, 101, 113, 117, 101, 115, 116, 66,
      111, 100, 121, 34,  58,  123, 34,  99,  111, 110, 116, 101, 110, 116, 34,
      58,  123, 34,  97,  112, 112, 108, 105, 99,  97,  116, 105, 111, 110, 47,
      106, 115, 111, 110, 34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  34,
      58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,
      116, 34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,
      58,  123, 34,  110, 97,  109, 101, 34,  58,  123, 34,  116, 121, 112, 101,
      34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 125, 125,
      125, 44,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123,
      34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112,
      116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  125, 125, 125, 125, 125,
      125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(2, spec.n_defined_schemas);
  ASSERT_STR_EQ("Inline_createPet_Request_1",
                spec.paths[0].operations[0].req_body.ref_name);
  {
    const struct StructFields *sf =
        (openapi_spec_find_schema(&spec, "Inline_createPet_Request_1",
                                  &_ast_openapi_spec_find_schema_3),
         _ast_openapi_spec_find_schema_3);
    ASSERT(sf != NULL);
    {
      struct StructField *field =
          (struct_fields_get(sf, "name", &_ast_struct_fields_get_4),
           _ast_struct_fields_get_4);
      ASSERT(field != NULL);
      ASSERT_STR_EQ("string", field->type);
    }
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_request_body_item_schema_array(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  115, 116, 114, 101, 97,  109, 34,  58,  123, 34,
      112, 111, 115, 116, 34,  58,  123, 34,  111, 112, 101, 114, 97,  116, 105,
      111, 110, 73,  100, 34,  58,  34,  115, 116, 114, 101, 97,  109, 80,  101,
      116, 115, 34,  44,  34,  114, 101, 113, 117, 101, 115, 116, 66,  111, 100,
      121, 34,  58,  123, 34,  99,  111, 110, 116, 101, 110, 116, 34,  58,  123,
      34,  97,  112, 112, 108, 105, 99,  97,  116, 105, 111, 110, 47,  106, 115,
      111, 110, 108, 34,  58,  123, 34,  105, 116, 101, 109, 83,  99,  104, 101,
      109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116,
      114, 105, 110, 103, 34,  125, 125, 125, 125, 44,  34,  114, 101, 115, 112,
      111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123,
      34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,
      111, 107, 34,  125, 125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.paths[0].operations[0].req_body.is_array);
  ASSERT_STR_EQ("string", spec.paths[0].operations[0].req_body.inline_type);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_response_schema_object_item_promoted(void) {
  struct StructFields *_ast_openapi_spec_find_schema_5;
  struct StructField *_ast_struct_fields_get_6 = NULL;

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  103, 101,
      116, 34,  58,  123, 34,  111, 112, 101, 114, 97,  116, 105, 111, 110, 73,
      100, 34,  58,  34,  108, 105, 115, 116, 80,  101, 116, 115, 34,  44,  34,
      114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,
      48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111,
      110, 34,  58,  34,  111, 107, 34,  44,  34,  99,  111, 110, 116, 101, 110,
      116, 34,  58,  123, 34,  97,  112, 112, 108, 105, 99,  97,  116, 105, 111,
      110, 47,  106, 115, 111, 110, 34,  58,  123, 34,  115, 99,  104, 101, 109,
      97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  97,  114, 114,
      97,  121, 34,  44,  34,  105, 116, 101, 109, 115, 34,  58,  123, 34,  116,
      121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116, 34,  44,  34,
      112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,  105,
      100, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  105, 110, 116,
      101, 103, 101, 114, 34,  125, 125, 125, 125, 125, 125, 125, 125, 125, 125,
      125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].schema.is_array);
  ASSERT_STR_EQ("Inline_listPets_Response_200_Item",
                spec.paths[0].operations[0].responses[0].schema.ref_name);
  {
    const struct StructFields *sf =
        (openapi_spec_find_schema(&spec, "Inline_listPets_Response_200_Item",
                                  &_ast_openapi_spec_find_schema_5),
         _ast_openapi_spec_find_schema_5);
    ASSERT(sf != NULL);
    {
      struct StructField *field =
          (struct_fields_get(sf, "id", &_ast_struct_fields_get_6),
           _ast_struct_fields_get_6);
      ASSERT(field != NULL);
      ASSERT_STR_EQ("integer", field->type);
    }
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_inline_response_item_schema_object_promoted(void) {
  struct StructFields *_ast_openapi_spec_find_schema_7;
  struct StructField *_ast_struct_fields_get_8 = NULL;

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  115, 116, 114, 101, 97,  109, 34,  58,  123, 34,
      103, 101, 116, 34,  58,  123, 34,  111, 112, 101, 114, 97,  116, 105, 111,
      110, 73,  100, 34,  58,  34,  115, 116, 114, 101, 97,  109, 80,  101, 116,
      115, 34,  44,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,
      123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105,
      112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  44,  34,  99,  111,
      110, 116, 101, 110, 116, 34,  58,  123, 34,  97,  112, 112, 108, 105, 99,
      97,  116, 105, 111, 110, 47,  106, 115, 111, 110, 108, 34,  58,  123, 34,
      105, 116, 101, 109, 83,  99,  104, 101, 109, 97,  34,  58,  123, 34,  116,
      121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116, 34,  44,  34,
      112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,  110,
      97,  109, 101, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115,
      116, 114, 105, 110, 103, 34,  125, 125, 125, 125, 125, 125, 125, 125, 125,
      125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].schema.is_array);
  ASSERT_STR_EQ("Inline_streamPets_Response_200_Item",
                spec.paths[0].operations[0].responses[0].schema.ref_name);
  {
    const struct StructFields *sf =
        (openapi_spec_find_schema(&spec, "Inline_streamPets_Response_200_Item",
                                  &_ast_openapi_spec_find_schema_7),
         _ast_openapi_spec_find_schema_7);
    ASSERT(sf != NULL);
    {
      struct StructField *field =
          (struct_fields_get(sf, "name", &_ast_struct_fields_get_8),
           _ast_struct_fields_get_8);
      ASSERT(field != NULL);
      ASSERT_STR_EQ("string", field->type);
    }
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_request_body_ref_description_override(void) {

  const char json[] = {
      123, 34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  123,
      34,  114, 101, 113, 117, 101, 115, 116, 66,  111, 100, 105, 101, 115, 34,
      58,  123, 34,  67,  114, 101, 97,  116, 101, 80,  101, 116, 34,  58,  123,
      34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,
      67,  114, 101, 97,  116, 101, 34,  44,  34,  114, 101, 113, 117, 105, 114,
      101, 100, 34,  58,  116, 114, 117, 101, 44,  34,  99,  111, 110, 116, 101,
      110, 116, 34,  58,  123, 34,  97,  112, 112, 108, 105, 99,  97,  116, 105,
      111, 110, 47,  106, 115, 111, 110, 34,  58,  123, 34,  115, 99,  104, 101,
      109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116,
      114, 105, 110, 103, 34,  125, 125, 125, 125, 125, 125, 44,  34,  112, 97,
      116, 104, 115, 34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123,
      34,  112, 111, 115, 116, 34,  58,  123, 34,  114, 101, 113, 117, 101, 115,
      116, 66,  111, 100, 121, 34,  58,  123, 34,  36,  114, 101, 102, 34,  58,
      34,  35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  114,
      101, 113, 117, 101, 115, 116, 66,  111, 100, 105, 101, 115, 47,  67,  114,
      101, 97,  116, 101, 80,  101, 116, 34,  44,  34,  100, 101, 115, 99,  114,
      105, 112, 116, 105, 111, 110, 34,  58,  34,  79,  118, 101, 114, 114, 105,
      100, 101, 34,  125, 44,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115,
      34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,
      114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  79,  75,  34,  125, 125,
      125, 125, 125, 44,  34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,
      51,  46,  50,  46,  48,  34,  125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("#/components/requestBodies/CreatePet",
                spec.paths[0].operations[0].req_body_ref);
  ASSERT_STR_EQ("Override", spec.paths[0].operations[0].req_body_description);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_options_trace_verbs(void) {

  const char *json = "{\"paths\":{\"/"
                     "v\":{\"options\":{\"operationId\":\"opt\",\"responses\":{"
                     "\"200\":{\"description\":\"OK\"}}},\"trace\":{"
                     "\"operationId\":\"tr\",\"responses\":{\"200\":{"
                     "\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    size_t i;
    int saw_options = 0;
    int saw_trace = 0;
    ASSERT_EQ(2, spec.paths[0].n_operations);
    for (i = 0; i < spec.paths[0].n_operations; ++i) {
      if (spec.paths[0].operations[i].verb == OA_VERB_OPTIONS)
        saw_options = 1;
      if (spec.paths[0].operations[i].verb == OA_VERB_TRACE)
        saw_trace = 1;
    }
    ASSERT_EQ(1, saw_options);
    ASSERT_EQ(1, saw_trace);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_root_metadata_and_tags(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"Test\",\"version\":\"1\"},"
      "\"$self\":\"https://example.com/openapi.json\","
      "\"jsonSchemaDialect\":\"https://spec.openapis.org/oas/3.1/dialect/"
      "base\","
      "\"externalDocs\":{\"description\":\"Root "
      "docs\",\"url\":\"https://example.com/docs\"},"
      "\"tags\":[{\"name\":\"pets\",\"summary\":\"Pets\",\"description\":\"Pet "
      "ops\","
      "\"parent\":\"animals\",\"kind\":\"nav\","
      "\"externalDocs\":{\"description\":\"Tag "
      "docs\",\"url\":\"https://example.com/tags/pets\"}}],"
      "\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("https://example.com/openapi.json", spec.self_uri);
  ASSERT_STR_EQ("https://spec.openapis.org/oas/3.1/dialect/base",
                spec.json_schema_dialect);
  ASSERT_STR_EQ("https://example.com/docs", spec.external_docs.url);
  ASSERT_STR_EQ("Root docs", spec.external_docs.description);
  ASSERT_EQ(1, spec.n_tags);
  ASSERT_STR_EQ("pets", spec.tags[0].name);
  ASSERT_STR_EQ("Pets", spec.tags[0].summary);
  ASSERT_STR_EQ("Pet ops", spec.tags[0].description);
  ASSERT_STR_EQ("animals", spec.tags[0].parent);
  ASSERT_STR_EQ("nav", spec.tags[0].kind);
  ASSERT_STR_EQ("https://example.com/tags/pets",
                spec.tags[0].external_docs.url);
  ASSERT_STR_EQ("Tag docs", spec.tags[0].external_docs.description);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_self_qualified_component_refs(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  36,  115, 101, 108, 102, 34,  58,  34,  104, 116,
      116, 112, 115, 58,  47,  47,  101, 120, 97,  109, 112, 108, 101, 46,  99,
      111, 109, 47,  111, 112, 101, 110, 97,  112, 105, 46,  106, 115, 111, 110,
      34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105, 116, 108,
      101, 34,  58,  34,  83,  101, 108, 102, 34,  44,  34,  118, 101, 114, 115,
      105, 111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  99,  111, 109, 112,
      111, 110, 101, 110, 116, 115, 34,  58,  123, 34,  115, 99,  104, 101, 109,
      97,  115, 34,  58,  123, 34,  80,  101, 116, 34,  58,  123, 34,  116, 121,
      112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116, 34,  44,  34,  112,
      114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,  105, 100,
      34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  105, 110, 116, 101,
      103, 101, 114, 34,  125, 125, 125, 125, 44,  34,  112, 97,  114, 97,  109,
      101, 116, 101, 114, 115, 34,  58,  123, 34,  80,  101, 116, 80,  97,  114,
      97,  109, 34,  58,  123, 34,  110, 97,  109, 101, 34,  58,  34,  112, 101,
      116, 34,  44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,
      44,  34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  36,  114, 101,
      102, 34,  58,  34,  104, 116, 116, 112, 115, 58,  47,  47,  101, 120, 97,
      109, 112, 108, 101, 46,  99,  111, 109, 47,  111, 112, 101, 110, 97,  112,
      105, 46,  106, 115, 111, 110, 35,  47,  99,  111, 109, 112, 111, 110, 101,
      110, 116, 115, 47,  115, 99,  104, 101, 109, 97,  115, 47,  80,  101, 116,
      34,  125, 125, 125, 125, 44,  34,  112, 97,  116, 104, 115, 34,  58,  123,
      34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  111, 112, 101, 114, 97,  116, 105, 111, 110, 73,  100, 34,  58,
      34,  103, 101, 116, 80,  101, 116, 115, 34,  44,  34,  112, 97,  114, 97,
      109, 101, 116, 101, 114, 115, 34,  58,  91,  123, 34,  36,  114, 101, 102,
      34,  58,  34,  104, 116, 116, 112, 115, 58,  47,  47,  101, 120, 97,  109,
      112, 108, 101, 46,  99,  111, 109, 47,  111, 112, 101, 110, 97,  112, 105,
      46,  106, 115, 111, 110, 35,  47,  99,  111, 109, 112, 111, 110, 101, 110,
      116, 115, 47,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 47,  80,
      101, 116, 80,  97,  114, 97,  109, 34,  125, 93,  44,  34,  114, 101, 115,
      112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,
      123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,
      34,  111, 107, 34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("Pet", p->schema.ref_name);
    ASSERT_EQ(1, spec.n_component_parameters);
    ASSERT_STR_EQ("Pet", spec.component_parameters[0].schema.ref_name);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_relative_self_component_refs(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  36,  115, 101, 108, 102, 34,  58,  34,  47,  97,
      112, 105, 47,  111, 112, 101, 110, 97,  112, 105, 46,  106, 115, 111, 110,
      34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105, 116, 108,
      101, 34,  58,  34,  83,  101, 108, 102, 34,  44,  34,  118, 101, 114, 115,
      105, 111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  99,  111, 109, 112,
      111, 110, 101, 110, 116, 115, 34,  58,  123, 34,  112, 97,  114, 97,  109,
      101, 116, 101, 114, 115, 34,  58,  123, 34,  80,  101, 116, 80,  97,  114,
      97,  109, 34,  58,  123, 34,  110, 97,  109, 101, 34,  58,  34,  112, 101,
      116, 34,  44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,
      44,  34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 125,
      44,  34,  112, 97,  116, 104, 115, 34,  58,  123, 34,  47,  112, 101, 116,
      115, 34,  58,  123, 34,  103, 101, 116, 34,  58,  123, 34,  111, 112, 101,
      114, 97,  116, 105, 111, 110, 73,  100, 34,  58,  34,  103, 101, 116, 80,
      101, 116, 115, 34,  44,  34,  112, 97,  114, 97,  109, 101, 116, 101, 114,
      115, 34,  58,  91,  123, 34,  36,  114, 101, 102, 34,  58,  34,  104, 116,
      116, 112, 115, 58,  47,  47,  101, 120, 97,  109, 112, 108, 101, 46,  99,
      111, 109, 47,  97,  112, 105, 47,  111, 112, 101, 110, 97,  112, 105, 46,
      106, 115, 111, 110, 35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116,
      115, 47,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 47,  80,  101,
      116, 80,  97,  114, 97,  109, 34,  125, 93,  44,  34,  114, 101, 115, 112,
      111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123,
      34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,
      111, 107, 34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("pet", p->name);
    ASSERT_EQ(OA_PARAM_IN_QUERY, p->in);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_SCHEMAS_H */
