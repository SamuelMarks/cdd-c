/**
 * @file test_openapi2db.h
 * @brief Unit tests for OpenAPI to DB IR conversion.
 */

#ifndef TEST_OPENAPI2DB_H
#define TEST_OPENAPI2DB_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openapi/parse/openapi.h"
#include "openapi/parse/openapi2db.h"

TEST test_openapi_to_db_schema_basic(void) {
  struct OpenAPI_Spec spec;
  struct DatabaseSchema schema;
  int rc;
  
  const char *json = 
    "{\"openapi\": \"3.2.0\", \"info\": {\"title\": \"PetStore\"}, \"components\": {\"schemas\": {"
    "\"Pet\": {"
    "  \"type\": \"object\","
    "  \"required\": [\"name\"],"
    "  \"properties\": {"
    "    \"id\": {\"type\": \"integer\"},"
    "    \"name\": {\"type\": \"string\"}"
    "  }"
    "}"
    "}}}";
    
  JSON_Value *root = json_parse_string(json);
  ASSERT(root != NULL);
  
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(root, &spec);
  ASSERT_EQ(0, rc);
  
  db_schema_init(&schema);
  rc = openapi_to_db_schema(&spec, &schema);
  ASSERT_EQ(0, rc);
  
  ASSERT_STR_EQ("PetStore", schema.name);
  ASSERT_EQ(1, schema.n_tables);
  ASSERT_STR_EQ("Pet", schema.tables[0].name);
  ASSERT_EQ(2, schema.tables[0].n_columns);
  
  ASSERT_STR_EQ("id", schema.tables[0].columns[0].name);
  ASSERT_EQ(DB_COL_TYPE_INTEGER, schema.tables[0].columns[0].type);
  ASSERT_EQ(1, schema.tables[0].columns[0].is_primary_key);
  
  ASSERT_STR_EQ("name", schema.tables[0].columns[1].name);
  ASSERT_EQ(DB_COL_TYPE_VARCHAR, schema.tables[0].columns[1].type);
  ASSERT_EQ(0, schema.tables[0].columns[1].is_nullable); /* required */
  
  db_schema_free(&schema);
  openapi_spec_free(&spec);
  json_value_free(root);
  PASS();
}

SUITE(openapi2db_suite) {
  RUN_TEST(test_openapi_to_db_schema_basic);
}

#endif /* TEST_OPENAPI2DB_H */
