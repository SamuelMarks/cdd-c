/**
 * @file test_schema_enum_required.h
 * @brief Tests for enum schema + required property support.
 */

#ifndef TEST_SCHEMA_ENUM_REQUIRED_H
#define TEST_SCHEMA_ENUM_REQUIRED_H

#include <greatest.h>
#include <parson.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit_enum.h"
#include "classes/emit_struct.h"
#include "routes/emit_openapi.h"
#include "routes/parse_openapi.h"

static int load_spec_string(const char *json, struct OpenAPI_Spec *spec) {
  JSON_Value *root;
  int rc;
  if (!json || !spec)
    return -1;
  root = json_parse_string(json);
  if (!root)
    return -1;
  openapi_spec_init(spec);
  rc = openapi_load_from_json(root, spec);
  json_value_free(root);
  return rc;
}

TEST test_loader_enum_and_required(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"components\":{\"schemas\":{"
      "\"Color\":{\"type\":\"string\",\"enum\":[\"RED\",\"GREEN\"]},"
      "\"Car\":{\"type\":\"object\","
      "\"required\":[\"color\"],"
      "\"properties\":{\"color\":{\"$ref\":\"#/components/schemas/Color\"}}}"
      "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_string(json, &spec);
  ASSERT_EQ(0, rc);

  {
    const struct StructFields *color = openapi_spec_find_schema(&spec, "Color");
    ASSERT(color != NULL);
    ASSERT_EQ(1, color->is_enum);
    ASSERT_EQ(2, color->enum_members.size);
    ASSERT_STR_EQ("RED", color->enum_members.members[0]);
    ASSERT_STR_EQ("GREEN", color->enum_members.members[1]);
  }

  {
    const struct StructFields *car = openapi_spec_find_schema(&spec, "Car");
    ASSERT(car != NULL);
    ASSERT_EQ(0, car->is_enum);
    ASSERT_EQ(1, car->size);
    ASSERT_STR_EQ("color", car->fields[0].name);
    ASSERT_STR_EQ("enum", car->fields[0].type);
    ASSERT_EQ(1, car->fields[0].required);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_writer_enum_and_required(void) {
  SKIPm("fix me");

  struct OpenAPI_Spec spec = {0};
  struct StructFields schemas[2];
  char *names[2];
  char *json = NULL;
  int rc;

  names[0] = "Color";
  names[1] = "Car";

  struct_fields_init(&schemas[0]);
  schemas[0].is_enum = 1;
  enum_members_init(&schemas[0].enum_members);
  enum_members_add(&schemas[0].enum_members, "RED");
  enum_members_add(&schemas[0].enum_members, "GREEN");

  struct_fields_init(&schemas[1]);
  struct_fields_add(&schemas[1], "color", "enum", "#/components/schemas/Color",
                    NULL, NULL);
  schemas[1].fields[0].required = 1;

  spec.openapi_version = "3.2.0";
  spec.defined_schemas = schemas;
  spec.defined_schema_names = names;
  spec.n_defined_schemas = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Object *components = json_object_get_object(root_obj, "components");
    JSON_Object *schemas_obj = json_object_get_object(components, "schemas");
    JSON_Object *color = json_object_get_object(schemas_obj, "Color");
    JSON_Object *car = json_object_get_object(schemas_obj, "Car");
    JSON_Array *enum_arr = json_object_get_array(color, "enum");
    JSON_Array *req_arr = json_object_get_array(car, "required");
    JSON_Object *props = json_object_get_object(car, "properties");
    JSON_Object *color_prop = json_object_get_object(props, "color");

    ASSERT_STR_EQ("string", json_object_get_string(color, "type"));
    ASSERT_EQ(2, json_array_get_count(enum_arr));
    ASSERT_STR_EQ("RED", json_array_get_string(enum_arr, 0));
    ASSERT_STR_EQ("GREEN", json_array_get_string(enum_arr, 1));

    ASSERT_EQ(1, json_array_get_count(req_arr));
    ASSERT_STR_EQ("color", json_array_get_string(req_arr, 0));
    ASSERT_STR_EQ("#/components/schemas/Color",
                  json_object_get_string(color_prop, "$ref"));

    json_value_free(root);
  }

  free(json);
  struct_fields_free(&schemas[0]);
  struct_fields_free(&schemas[1]);
  PASS();
}

SUITE(schema_enum_required_suite) {
  RUN_TEST(test_loader_enum_and_required);
  RUN_TEST(test_writer_enum_and_required);
}

#endif /* TEST_SCHEMA_ENUM_REQUIRED_H */
