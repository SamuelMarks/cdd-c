/**
 * @file test_openapi_loader.h
 * @brief Unit tests for OpenAPI Spec Loader.
 */

#ifndef TEST_OPENAPI_LOADER_H
#define TEST_OPENAPI_LOADER_H

#include <greatest.h>
#include <parson.h>
#include <stdlib.h>
#include <string.h>

#include "openapi_loader.h"

static int load_spec_str(const char *json_str, struct OpenAPI_Spec *spec) {
  JSON_Value *dyn = json_parse_string(json_str);
  int rc;
  if (!dyn)
    return -1;
  openapi_spec_init(spec);
  rc = openapi_load_from_json(dyn, spec);
  json_value_free(dyn);
  return rc;
}

TEST test_load_parameter_array(void) {
  const char *json =
      "{\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"integer\"}},"
      "\"style\":\"form\",\"explode\":true"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("tags", p->name);
    ASSERT_STR_EQ("array", p->type);
    ASSERT_EQ(1, p->is_array);
    ASSERT_STR_EQ("integer", p->items_type);
    ASSERT_EQ(OA_STYLE_FORM, p->style);
    ASSERT_EQ(1, p->explode);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_schema_parsing(void) {
  /* Test that schemas are loaded into the global registry */
  const char *json = "{\"components\":{\"schemas\":{"
                     "\"Login\":{\"type\":\"object\",\"properties\":{\"user\":{"
                     "\"type\":\"string\"}}}"
                     "}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_STR_EQ("Login", spec.defined_schema_names[0]);

  {
    const struct StructFields *sf = openapi_spec_find_schema(&spec, "Login");
    ASSERT(sf != NULL);
    ASSERT_EQ(1, sf->size);
    ASSERT_STR_EQ("user", sf->fields[0].name);
    ASSERT_STR_EQ("string", sf->fields[0].type);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_form_content_type(void) {
  const char *json = "{\"paths\":{\"/login\":{\"post\":{\"requestBody\":{"
                     "\"content\": {\"application/x-www-form-urlencoded\": { "
                     "\"schema\": {\"$ref\":\"#/components/schemas/Login\"}}}"
                     "}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("application/x-www-form-urlencoded",
                spec.paths[0].operations[0].req_body.content_type);
  ASSERT_STR_EQ("Login", spec.paths[0].operations[0].req_body.ref_name);

  openapi_spec_free(&spec);
  PASS();
}

SUITE(openapi_loader_suite) {
  RUN_TEST(test_load_parameter_array);
  RUN_TEST(test_load_schema_parsing);
  RUN_TEST(test_load_form_content_type);
}

#endif /* TEST_OPENAPI_LOADER_H */
