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

#include "classes/emit_struct.h"
#include "routes/parse_openapi.h"

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

static int load_spec_str_with_context(const char *json_str,
                                      const char *retrieval_uri,
                                      struct OpenAPI_DocRegistry *registry,
                                      struct OpenAPI_Spec *spec) {
  JSON_Value *dyn = json_parse_string(json_str);
  int rc;
  if (!dyn)
    return -1;
  openapi_spec_init(spec);
  rc = openapi_load_from_json_with_context(dyn, retrieval_uri, spec, registry);
  json_value_free(dyn);
  return rc;
}

static int find_raw_schema_index(const struct OpenAPI_Spec *spec,
                                 const char *name) {
  size_t i;
  if (!spec || !name)
    return -1;
  for (i = 0; i < spec->n_raw_schemas; ++i) {
    if (spec->raw_schema_names && spec->raw_schema_names[i] &&
        strcmp(spec->raw_schema_names[i], name) == 0) {
      return (int)i;
    }
  }
  return -1;
}

static const struct OpenAPI_SecurityScheme *
find_scheme(const struct OpenAPI_Spec *spec, const char *name) {
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_security_schemes; ++i) {
    if (spec->security_schemes[i].name &&
        strcmp(spec->security_schemes[i].name, name) == 0) {
      return &spec->security_schemes[i];
    }
  }
  return NULL;
}

static const struct OpenAPI_MediaType *
find_media_type(const struct OpenAPI_MediaType *mts, size_t n,
                const char *name) {
  size_t i;
  if (!mts || !name)
    return NULL;
  for (i = 0; i < n; ++i) {
    if (mts[i].name && strcmp(mts[i].name, name) == 0)
      return &mts[i];
  }
  return NULL;
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
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"schemas\":{"
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

TEST test_load_schema_external_docs_discriminator_xml(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/p\":{\"get\":{\"parameters\":[{"
      "\"name\":\"id\",\"in\":\"query\","
      "\"schema\":{"
      "\"type\":\"string\","
      "\"externalDocs\":{\"url\":\"https://example.com/docs\","
      "\"description\":\"Schema docs\"},"
      "\"discriminator\":{"
      "\"propertyName\":\"kind\","
      "\"mapping\":{\"a\":\"#/components/schemas/A\"},"
      "\"defaultMapping\":\"#/components/schemas/Base\"},"
      "\"xml\":{\"name\":\"id\","
      "\"namespace\":\"https://example.com/ns\","
      "\"prefix\":\"p\","
      "\"nodeType\":\"attribute\"}"
      "}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT(p->schema.external_docs_set == 1);
    ASSERT_STR_EQ("https://example.com/docs", p->schema.external_docs.url);
    ASSERT_STR_EQ("Schema docs", p->schema.external_docs.description);
    ASSERT(p->schema.discriminator_set == 1);
    ASSERT_STR_EQ("kind", p->schema.discriminator.property_name);
    ASSERT_EQ(1, p->schema.discriminator.n_mapping);
    ASSERT_STR_EQ("a", p->schema.discriminator.mapping[0].value);
    ASSERT_STR_EQ("#/components/schemas/A",
                  p->schema.discriminator.mapping[0].schema);
    ASSERT_STR_EQ("#/components/schemas/Base",
                  p->schema.discriminator.default_mapping);
    ASSERT(p->schema.xml_set == 1);
    ASSERT_STR_EQ("id", p->schema.xml.name);
    ASSERT_STR_EQ("https://example.com/ns", p->schema.xml.namespace_uri);
    ASSERT_STR_EQ("p", p->schema.xml.prefix);
    ASSERT_EQ(OA_XML_NODE_ATTRIBUTE, p->schema.xml.node_type);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_form_content_type(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/login\":{\"post\":{\"requestBody\":{"
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

TEST test_request_body_content_required(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/login\":{\"post\":{"
                     "\"requestBody\":{},"
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}"
                     "}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_param_content_multiple_entries_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\","
      "\"content\":{\"application/json\":{},\"text/plain\":{}}"
      "}]}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_header_content_multiple_entries_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"OK\","
      "\"headers\":{\"X-Rate\":{\"content\":{"
      "\"application/json\":{},\"text/plain\":{}}}}}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_response_description_required(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"200\":{}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_operation_responses_required(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_response_code_key_invalid_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"20X\":{"
      "\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_response_code_range_valid(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"2XX\":{"
      "\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_paths_require_leading_slash(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"pets\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_paths_ambiguous_templates_rejected(void) {
  const char *json =
      "{"
      "\"paths\":{"
      "\"/pets/"
      "{petId}\":{\"get\":{\"responses\":{\"200\":{\"description\":\"OK\"}}}},"
      "\"/pets/"
      "{name}\":{\"get\":{\"responses\":{\"200\":{\"description\":\"OK\"}}}}"
      "}"
      "}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_component_key_regex_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"components\":{\"schemas\":{\"Bad/Name\":{\"type\":\"string\"}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_tag_duplicate_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"tags\":[{\"name\":\"dup\"},{\"name\":\"dup\"}]}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_tag_name_required(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"tags\":[{\"description\":\"missing\"}]}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_tag_parent_missing_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"tags\":[{\"name\":\"child\",\"parent\":\"ghost\"}]}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_tag_parent_cycle_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"tags\":[{\"name\":\"a\",\"parent\":\"b\"},"
      "{\"name\":\"b\",\"parent\":\"a\"}]}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_external_docs_url_required(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"externalDocs\":{\"description\":\"Docs\"}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_operation_id_duplicate_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{"
                     "\"/pets\":{\"get\":{\"operationId\":\"listPets\","
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}}},"
                     "\"/cats\":{\"get\":{\"operationId\":\"listPets\","
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}}}"
                     "}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_operation_id_duplicate_in_callback_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/pets\":{\"get\":{"
                     "\"operationId\":\"dup\","
                     "\"responses\":{\"200\":{\"description\":\"OK\"}},"
                     "\"callbacks\":{\"cb\":{\"{$request.body#/url}\":{"
                     "\"post\":{\"operationId\":\"dup\","
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}}"
                     "}}}"
                     "}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_parameter_duplicates_rejected(void) {
  const char *json =
      "{\"paths\":{\"/p\":{\"get\":{\"parameters\":["
      "{\"name\":\"id\",\"in\":\"query\",\"schema\":{\"type\":\"string\"}},"
      "{\"name\":\"id\",\"in\":\"query\",\"schema\":{\"type\":\"string\"}}"
      "],\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_querystring_with_query_rejected(void) {
  const char *json =
      "{\"paths\":{\"/q\":{\"get\":{\"parameters\":["
      "{\"name\":\"raw\",\"in\":\"querystring\","
      "\"content\":{\"application/x-www-form-urlencoded\":{}}},"
      "{\"name\":\"q\",\"in\":\"query\",\"schema\":{\"type\":\"string\"}}"
      "],\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_querystring_duplicate_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":["
      "{\"name\":\"raw\",\"in\":\"querystring\","
      "\"content\":{\"application/x-www-form-urlencoded\":{}}},"
      "{\"name\":\"raw2\",\"in\":\"querystring\","
      "\"content\":{\"application/x-www-form-urlencoded\":{}}}"
      "],\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_querystring_path_and_operation_mixed_rejected(void) {
  const char *json =
      "{\"paths\":{\"/q\":{"
      "\"parameters\":[{\"name\":\"raw\",\"in\":\"querystring\","
      "\"content\":{\"application/x-www-form-urlencoded\":{}}}],"
      "\"get\":{\"parameters\":["
      "{\"name\":\"q\",\"in\":\"query\",\"schema\":{\"type\":\"string\"}}"
      "],\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_querystring_with_query_in_callback_rejected(void) {
  const char *json =
      "{\"paths\":{\"/pets\":{\"get\":{"
      "\"responses\":{\"200\":{\"description\":\"OK\"}},"
      "\"callbacks\":{\"cb\":{\"{$request.body#/url}\":{"
      "\"post\":{\"parameters\":["
      "{\"name\":\"raw\",\"in\":\"querystring\","
      "\"content\":{\"application/x-www-form-urlencoded\":{}}},"
      "{\"name\":\"q\",\"in\":\"query\",\"schema\":{\"type\":\"string\"}}"
      "],\"responses\":{\"200\":{\"description\":\"OK\"}}}"
      "}}}"
      "}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_parameter_missing_name_or_in_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/p\":{\"get\":{\"parameters\":["
      "{\"in\":\"query\",\"schema\":{\"type\":\"string\"}}"
      "],\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_header_style_non_simple_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"OK\","
      "\"headers\":{\"X-Test\":{\"schema\":{\"type\":\"string\"},"
      "\"style\":\"form\"}}}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_media_type_encoding_conflict_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/u\":{\"post\":{"
                     "\"requestBody\":{\"content\":{\"multipart/form-data\":{"
                     "\"schema\":{\"type\":\"object\",\"properties\":{"
                     "\"a\":{\"type\":\"string\"}}},"
                     "\"encoding\":{\"a\":{}},"
                     "\"prefixEncoding\":[{}]}}},"
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_encoding_object_conflict_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/u\":{\"post\":{"
                     "\"requestBody\":{\"content\":{\"multipart/form-data\":{"
                     "\"schema\":{\"type\":\"object\",\"properties\":{"
                     "\"a\":{\"type\":\"string\"}}},"
                     "\"encoding\":{\"a\":{"
                     "\"encoding\":{\"b\":{}},"
                     "\"itemEncoding\":{}}}}},"
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_operation_tags(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/tagged\":{\"get\":{"
                     "\"tags\":[\"pet\", \"store\"],"
                     "\"operationId\":\"getTagged\""
                     "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_EQ(2, op->n_tags);
    ASSERT_STR_EQ("pet", op->tags[0]);
    ASSERT_STR_EQ("store", op->tags[1]);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_parameter_metadata(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\","
      "\"description\":\"Search term\","
      "\"deprecated\":true,"
      "\"allowReserved\":true,"
      "\"schema\":{\"type\":\"string\"}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("Search term", p->description);
    ASSERT_EQ(1, p->deprecated_set);
    ASSERT_EQ(1, p->deprecated);
    ASSERT_EQ(1, p->allow_reserved_set);
    ASSERT_EQ(1, p->allow_reserved);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_allow_empty_value(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\","
      "\"allowEmptyValue\":true,"
      "\"schema\":{\"type\":\"string\"}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->allow_empty_value_set);
    ASSERT_EQ(1, p->allow_empty_value);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_allow_empty_value_non_query_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"header\","
      "\"allowEmptyValue\":true,"
      "\"schema\":{\"type\":\"string\"}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_parameter_explode_false(void) {
  const char *json =
      "{\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"ids\",\"in\":\"query\","
      "\"style\":\"form\",\"explode\":false,"
      "\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"string\"}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->explode_set);
    ASSERT_EQ(0, p->explode);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_querystring_parameter(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/search\":{\"get\":{\"parameters\":[{"
      "\"name\":\"qs\",\"in\":\"querystring\","
      "\"content\":{\"application/x-www-form-urlencoded\":{"
      "\"schema\":{\"type\":\"object\"}"
      "}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(OA_PARAM_IN_QUERYSTRING, p->in);
    ASSERT_STR_EQ("application/x-www-form-urlencoded", p->content_type);
    ASSERT_STR_EQ("string", p->type);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_querystring_json_inline_promoted(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/search\":{\"get\":{\"parameters\":[{"
      "\"name\":\"qs\",\"in\":\"querystring\","
      "\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"object\",\"properties\":{\"q\":{"
      "\"type\":\"string\"}}}"
      "}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(OA_PARAM_IN_QUERYSTRING, p->in);
    ASSERT(p->schema.ref_name != NULL);
    ASSERT_EQ(1, spec.n_defined_schemas);
    ASSERT_STR_EQ(p->schema.ref_name, spec.defined_schema_names[0]);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_ignore_reserved_header_parameters(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/"
                     "h\":{\"get\":{\"responses\":{\"200\":{\"description\":"
                     "\"ok\"}},\"parameters\":["
                     "{\"name\":\"Accept\",\"in\":\"header\","
                     "\"schema\":{\"type\":\"string\"}},"
                     "{\"name\":\"q\",\"in\":\"query\","
                     "\"schema\":{\"type\":\"string\"}}"
                     "]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_EQ(1, op->n_parameters);
    ASSERT_STR_EQ("q", op->parameters[0].name);
    ASSERT_EQ(OA_PARAM_IN_QUERY, op->parameters[0].in);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_ignore_content_type_response_header(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\","
      "\"headers\":{"
      "\"Content-Type\":{\"schema\":{\"type\":\"string\"}},"
      "\"X-Rate\":{\"schema\":{\"type\":\"integer\"}}"
      "}}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Response *resp = &spec.paths[0].operations[0].responses[0];
    ASSERT_EQ(1, resp->n_headers);
    ASSERT_STR_EQ("X-Rate", resp->headers[0].name);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_param_schema_and_content_conflict(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "c\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\"}},"
      "\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\"},"
      "\"content\":{\"text/plain\":{\"schema\":{\"type\":\"string\"}}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_header_schema_and_content_conflict(void) {
  const char *json =
      "{\"paths\":{\"/c\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\","
      "\"headers\":{"
      "\"X-Foo\":{"
      "\"schema\":{\"type\":\"string\"},"
      "\"content\":{\"text/plain\":{\"schema\":{\"type\":\"string\"}}}"
      "}"
      "}}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_parameter_content_any(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "h\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\"}},"
      "\"parameters\":[{"
      "\"name\":\"X-Foo\",\"in\":\"header\","
      "\"content\":{\"text/plain\":{\"schema\":{\"type\":\"string\"}}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("text/plain", p->content_type);
    ASSERT_STR_EQ("string", p->type);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_parameter_content_media_type_encoding(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/p\":{\"get\":{\"parameters\":[{"
      "\"name\":\"filter\",\"in\":\"query\","
      "\"content\":{\"application/x-www-form-urlencoded\":{"
      "\"schema\":{\"type\":\"object\",\"properties\":{"
      "\"id\":{\"type\":\"string\"}"
      "}},"
      "\"encoding\":{\"id\":{"
      "\"contentType\":\"text/plain\","
      "\"style\":\"form\","
      "\"explode\":true"
      "}}"
      "}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("application/x-www-form-urlencoded", p->content_type);
    ASSERT(p->content_media_types != NULL);
    ASSERT_EQ((size_t)1, p->n_content_media_types);
    ASSERT_STR_EQ("application/x-www-form-urlencoded",
                  p->content_media_types[0].name);
    ASSERT_EQ((size_t)1, p->content_media_types[0].n_encoding);
    ASSERT_STR_EQ("id", p->content_media_types[0].encoding[0].name);
    ASSERT_STR_EQ("text/plain",
                  p->content_media_types[0].encoding[0].content_type);
    ASSERT_EQ(1, p->content_media_types[0].encoding[0].style_set);
    ASSERT_EQ(OA_STYLE_FORM, p->content_media_types[0].encoding[0].style);
    ASSERT_EQ(1, p->content_media_types[0].encoding[0].explode_set);
    ASSERT_EQ(1, p->content_media_types[0].encoding[0].explode);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_header_content_media_type(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/p\":{\"get\":{\"responses\":{"
      "\"200\":{\"description\":\"ok\",\"headers\":{"
      "\"X-Rate\":{\"content\":{"
      "\"text/plain\":{\"schema\":{\"type\":\"string\"}}"
      "}}}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Header *h =
        &spec.paths[0].operations[0].responses[0].headers[0];
    ASSERT_STR_EQ("text/plain", h->content_type);
    ASSERT(h->content_media_types != NULL);
    ASSERT_EQ((size_t)1, h->n_content_media_types);
    ASSERT_STR_EQ("text/plain", h->content_media_types[0].name);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_parameter_schema_ref(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"paths\":{\"/pets\":{\"get\":{\"parameters\":["
      "{\"name\":\"pet\",\"in\":\"query\","
      "\"schema\":{\"$ref\":\"#/components/schemas/Pet\"}},"
      "{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"items\":{"
      "\"$ref\":\"#/components/schemas/Tag\"}}}"
      "]}}},\"components\":{\"schemas\":{"
      "\"Pet\":{\"type\":\"object\"},"
      "\"Tag\":{\"type\":\"object\"}"
      "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p0 = &spec.paths[0].operations[0].parameters[0];
    struct OpenAPI_Parameter *p1 = &spec.paths[0].operations[0].parameters[1];
    ASSERT_STR_EQ("Pet", p0->type);
    ASSERT_EQ(0, p0->is_array);
    ASSERT_EQ(1, p1->is_array);
    ASSERT_STR_EQ("array", p1->type);
    ASSERT_STR_EQ("Tag", p1->items_type);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_header_schema_ref(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"paths\":{\"/pets\":{\"get\":{\"responses\":{"
      "\"200\":{\"description\":\"ok\",\"headers\":{"
      "\"X-Rate\":{\"schema\":{\"$ref\":\"#/components/schemas/Rate\"}}"
      "}}}}}},\"components\":{\"schemas\":{"
      "\"Rate\":{\"type\":\"integer\"}"
      "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Header *h =
        &spec.paths[0].operations[0].responses[0].headers[0];
    ASSERT_STR_EQ("Rate", h->type);
    ASSERT_EQ(0, h->is_array);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_path_level_parameters(void) {
  const char *json =
      "{\"paths\":{\"/pets\":{\"summary\":\"Pets\",\"description\":\"All "
      "pets\","
      "\"parameters\":[{\"name\":\"x-trace\",\"in\":\"header\","
      "\"schema\":{\"type\":\"string\"}}],"
      "\"get\":{\"operationId\":\"listPets\"}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_STR_EQ("Pets", spec.paths[0].summary);
  ASSERT_STR_EQ("All pets", spec.paths[0].description);
  ASSERT_EQ(1, spec.paths[0].n_parameters);
  ASSERT_STR_EQ("x-trace", spec.paths[0].parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_HEADER, spec.paths[0].parameters[0].in);
  ASSERT_EQ(0, spec.paths[0].operations[0].n_parameters);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_server_variables(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://"
                     "{env}.example.com\","
                     "\"variables\":{\"env\":{\"default\":\"prod\",\"enum\":["
                     "\"prod\",\"staging\"],"
                     "\"description\":\"Environment\"}}}],\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_servers);
  ASSERT_EQ(1, spec.servers[0].n_variables);
  ASSERT_STR_EQ("env", spec.servers[0].variables[0].name);
  ASSERT_STR_EQ("prod", spec.servers[0].variables[0].default_value);
  ASSERT_STR_EQ("Environment", spec.servers[0].variables[0].description);
  ASSERT_EQ(2, spec.servers[0].variables[0].n_enum_values);
  ASSERT_STR_EQ("prod", spec.servers[0].variables[0].enum_values[0]);
  ASSERT_STR_EQ("staging", spec.servers[0].variables[0].enum_values[1]);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_server_variable_default_required(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://"
                     "{env}.example.com\","
                     "\"variables\":{\"env\":{\"enum\":[\"prod\",\"staging\"]}}"
                     "}],\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_openapi_version_and_servers(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://"
      "api.example.com\","
      "\"description\":\"Prod\",\"name\":\"prod\"}],\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("3.2.0", spec.openapi_version);
  ASSERT_EQ(1, spec.n_servers);
  ASSERT_STR_EQ("https://api.example.com", spec.servers[0].url);
  ASSERT_STR_EQ("Prod", spec.servers[0].description);
  ASSERT_STR_EQ("prod", spec.servers[0].name);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_server_duplicate_name_rejected(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"servers\":["
      "{\"url\":\"https://api.example.com\",\"name\":\"prod\"},"
      "{\"url\":\"https://staging.example.com\",\"name\":\"prod\"}"
      "],"
      "\"paths\":{}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_missing_openapi_and_swagger_rejected(void) {
  const char *json =
      "{\"info\":{\"title\":\"T\",\"version\":\"1\"},\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_schema_root_document_with_id(void) {
  const char *json = "{"
                     "\"$id\":\"https://example.com/schema.json\","
                     "\"type\":\"object\","
                     "\"properties\":{\"id\":{\"type\":\"string\"}}"
                     "}";

  struct OpenAPI_DocRegistry registry;
  struct OpenAPI_Spec spec;
  int rc;

  openapi_doc_registry_init(&registry);
  rc = load_spec_str_with_context(json, "https://example.com/schema.json",
                                  &registry, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.is_schema_document);
  ASSERT(spec.schema_root_json != NULL);
  ASSERT_STR_EQ("https://example.com/schema.json", spec.document_uri);
  ASSERT_EQ(1, registry.count);
  ASSERT_EQ(&spec, registry.entries[0].spec);
  ASSERT_STR_EQ("https://example.com/schema.json",
                registry.entries[0].base_uri);

  {
    JSON_Value *val = json_parse_string(spec.schema_root_json);
    JSON_Object *obj = json_value_get_object(val);
    ASSERT_STR_EQ("object", json_object_get_string(obj, "type"));
    json_value_free(val);
  }

  openapi_spec_free(&spec);
  openapi_doc_registry_free(&registry);
  PASS();
}

TEST test_load_schema_root_boolean(void) {
  const char *json = "false";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str_with_context(json, "https://example.com/boolean.json",
                                      NULL, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.is_schema_document);
  ASSERT(spec.schema_root_json != NULL);
  ASSERT_STR_EQ("https://example.com/boolean.json", spec.document_uri);

  {
    JSON_Value *val = json_parse_string(spec.schema_root_json);
    ASSERT_EQ(JSONBoolean, json_value_get_type(val));
    ASSERT_EQ(0, json_value_get_boolean(val));
    json_value_free(val);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_swagger_root_allowed(void) {
  const char *json =
      "{\"swagger\":\"2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_openapi_version_unsupported_rejected(void) {
  const char *json =
      "{\"openapi\":\"4.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_server_url_query_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://example.com/api?"
      "q=1\"}],\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_security_requirements(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"security\":["
      "  {\"ApiKeyAuth\":[]},"
      "  {\"bearerAuth\":[\"read:pets\"]}"
      "],"
      "\"paths\":{\"/pets\":{\"get\":{\"operationId\":\"listPets\","
      "\"security\":[{}]}}},"
      "\"components\":{\"securitySchemes\":{"
      "\"ApiKeyAuth\":{\"type\":\"apiKey\",\"in\":\"header\",\"name\":\"X-"
      "Api\"},"
      "\"bearerAuth\":{\"type\":\"http\",\"scheme\":\"bearer\"}"
      "}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.security_set);
  ASSERT_EQ(2, spec.n_security);
  ASSERT_EQ(1, spec.security[0].n_requirements);
  ASSERT_STR_EQ("ApiKeyAuth", spec.security[0].requirements[0].scheme);
  ASSERT_EQ(0, spec.security[0].requirements[0].n_scopes);
  ASSERT_EQ(1, spec.security[1].requirements[0].n_scopes);
  ASSERT_STR_EQ("read:pets", spec.security[1].requirements[0].scopes[0]);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_EQ(1, op->security_set);
    ASSERT_EQ(1, op->n_security);
    ASSERT_EQ(0, op->security[0].n_requirements); /* empty object */
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_security_schemes(void) {
  const char *json =
      "{\"components\":{\"securitySchemes\":{"
      "\"bearerAuth\":{\"type\":\"http\",\"scheme\":\"bearer\","
      "\"bearerFormat\":\"JWT\"},"
      "\"apiKeyAuth\":{\"type\":\"apiKey\",\"in\":\"header\","
      "\"name\":\"X-Api-Key\"},"
      "\"mtlsAuth\":{\"type\":\"mutualTLS\",\"description\":\"mTLS only\"}"
      "}}}";

  struct OpenAPI_Spec spec;
  const struct OpenAPI_SecurityScheme *bearer;
  const struct OpenAPI_SecurityScheme *apikey;
  const struct OpenAPI_SecurityScheme *mtls;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(3, spec.n_security_schemes);

  bearer = find_scheme(&spec, "bearerAuth");
  ASSERT(bearer != NULL);
  ASSERT_EQ(OA_SEC_HTTP, bearer->type);
  ASSERT_STR_EQ("bearer", bearer->scheme);
  ASSERT_STR_EQ("JWT", bearer->bearer_format);

  apikey = find_scheme(&spec, "apiKeyAuth");
  ASSERT(apikey != NULL);
  ASSERT_EQ(OA_SEC_APIKEY, apikey->type);
  ASSERT_EQ(OA_SEC_IN_HEADER, apikey->in);
  ASSERT_STR_EQ("X-Api-Key", apikey->key_name);

  mtls = find_scheme(&spec, "mtlsAuth");
  ASSERT(mtls != NULL);
  ASSERT_EQ(OA_SEC_MUTUALTLS, mtls->type);
  ASSERT_STR_EQ("mTLS only", mtls->description);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_security_scheme_deprecated(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"oldKey\":{\"type\":\"apiKey\",\"in\":\"header\","
                     "\"name\":\"X-Old\",\"deprecated\":true}"
                     "}}}";

  struct OpenAPI_Spec spec;
  const struct OpenAPI_SecurityScheme *old_key;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  old_key = find_scheme(&spec, "oldKey");
  ASSERT(old_key != NULL);
  ASSERT_EQ(1, old_key->deprecated_set);
  ASSERT_EQ(1, old_key->deprecated);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_oauth2_flows(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"oauth\":{\"type\":\"oauth2\",\"flows\":{"
                     "\"authorizationCode\":{"
                     "\"authorizationUrl\":\"https://auth.example.com\","
                     "\"tokenUrl\":\"https://token.example.com\","
                     "\"refreshUrl\":\"https://refresh.example.com\","
                     "\"scopes\":{\"read\":\"Read access\"}"
                     "}}}}}}";

  struct OpenAPI_Spec spec;
  const struct OpenAPI_SecurityScheme *oauth;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  oauth = find_scheme(&spec, "oauth");
  ASSERT(oauth != NULL);
  ASSERT_EQ(OA_SEC_OAUTH2, oauth->type);
  ASSERT_EQ(1, oauth->n_flows);
  ASSERT_EQ(OA_OAUTH_FLOW_AUTHORIZATION_CODE, oauth->flows[0].type);
  ASSERT_STR_EQ("https://auth.example.com", oauth->flows[0].authorization_url);
  ASSERT_STR_EQ("https://token.example.com", oauth->flows[0].token_url);
  ASSERT_STR_EQ("https://refresh.example.com", oauth->flows[0].refresh_url);
  ASSERT_EQ(1, oauth->flows[0].n_scopes);
  ASSERT_STR_EQ("read", oauth->flows[0].scopes[0].name);
  ASSERT_STR_EQ("Read access", oauth->flows[0].scopes[0].description);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_security_scheme_http_missing_scheme_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"bad\":{\"type\":\"http\"}"
                     "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_security_scheme_apikey_missing_name_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"bad\":{\"type\":\"apiKey\",\"in\":\"header\"}"
                     "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_security_scheme_apikey_missing_in_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"bad\":{\"type\":\"apiKey\",\"name\":\"X-Api\"}"
                     "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_security_scheme_openid_missing_url_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"bad\":{\"type\":\"openIdConnect\"}"
                     "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_oauth2_missing_flows_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"oauth\":{\"type\":\"oauth2\"}"
                     "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_oauth2_flow_missing_scopes_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"oauth\":{\"type\":\"oauth2\",\"flows\":{"
                     "\"authorizationCode\":{"
                     "\"authorizationUrl\":\"https://auth.example.com\","
                     "\"tokenUrl\":\"https://token.example.com\""
                     "}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_oauth2_flow_missing_required_urls_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"oauth\":{\"type\":\"oauth2\",\"flows\":{"
                     "\"deviceAuthorization\":{"
                     "\"tokenUrl\":\"https://token.example.com\","
                     "\"scopes\":{}}"
                     "}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_oauth2_flow_unknown_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"securitySchemes\":{"
                     "\"oauth\":{\"type\":\"oauth2\",\"flows\":{"
                     "\"customFlow\":{"
                     "\"authorizationUrl\":\"https://auth.example.com\","
                     "\"tokenUrl\":\"https://token.example.com\","
                     "\"scopes\":{}}"
                     "}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_parameter_examples_object(void) {
  const char *json =
      "{\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\"},"
      "\"examples\":{\"basic\":{\"summary\":\"Basic\",\"dataValue\":\"hello\"}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->n_examples);
    ASSERT_EQ(OA_EXAMPLE_LOC_OBJECT, p->example_location);
    ASSERT_STR_EQ("basic", p->examples[0].name);
    ASSERT_STR_EQ("Basic", p->examples[0].summary);
    ASSERT_EQ(1, p->examples[0].data_value_set);
    ASSERT_EQ(OA_ANY_STRING, p->examples[0].data_value.type);
    ASSERT_STR_EQ("hello", p->examples[0].data_value.string);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_parameter_examples_media(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\","
      "\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"string\"},"
      "\"examples\":{\"m\":{\"serializedValue\":\"\\\"hi\\\"\"}}"
      "}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(OA_EXAMPLE_LOC_MEDIA, p->example_location);
    ASSERT_EQ(1, p->n_examples);
    ASSERT_STR_EQ("m", p->examples[0].name);
    ASSERT_STR_EQ("\"hi\"", p->examples[0].serialized_value);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_parameter_example_and_examples_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\"},"
      "\"example\":\"a\","
      "\"examples\":{\"ex\":{\"value\":\"b\"}}"
      "}],\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_header_example_and_examples_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\","
      "\"headers\":{\"X-Test\":{"
      "\"schema\":{\"type\":\"string\"},"
      "\"example\":\"a\","
      "\"examples\":{\"ex\":{\"value\":\"b\"}}"
      "}}}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_media_example_and_examples_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\","
      "\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"string\"},"
      "\"example\":\"a\","
      "\"examples\":{\"ex\":{\"value\":\"b\"}}"
      "}}}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_example_data_value_and_value_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\",\"schema\":{\"type\":\"string\"},"
      "\"examples\":{\"bad\":{\"dataValue\":\"a\",\"value\":\"b\"}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_example_serialized_and_external_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\",\"schema\":{\"type\":\"string\"},"
      "\"examples\":{\"bad\":{\"serializedValue\":\"x\","
      "\"externalValue\":\"http://example.com/ex\"}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_response_examples_media(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\","
      "\"content\":{\"application/json\":{"
      "\"example\":{\"id\":1}"
      "}}"
      "}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Response *resp = &spec.paths[0].operations[0].responses[0];
    ASSERT_EQ(1, resp->example_set);
    ASSERT_EQ(OA_ANY_JSON, resp->example.type);
    ASSERT(resp->example.json != NULL);
    ASSERT(strstr(resp->example.json, "\"id\"") != NULL);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_component_examples(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"examples\":{"
                     "\"ex1\":{\"summary\":\"One\",\"value\":\"v\"}"
                     "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_examples);
  ASSERT_STR_EQ("ex1", spec.component_example_names[0]);
  ASSERT_STR_EQ("One", spec.component_examples[0].summary);
  ASSERT_EQ(1, spec.component_examples[0].value_set);
  ASSERT_EQ(OA_ANY_STRING, spec.component_examples[0].value.type);
  ASSERT_STR_EQ("v", spec.component_examples[0].value.string);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_example_component_ref_strict(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\","
      "\"$self\":\"https://example.com/spec.json\","
      "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"components\":{\"examples\":{"
      "\"Ex\":{\"summary\":\"Right\",\"value\":\"ok\"},"
      "\"foo\":{\"summary\":\"Wrong\",\"value\":\"bad\"}"
      "}},"
      "\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\",\"schema\":{\"type\":\"string\"},"
      "\"examples\":{"
      "\"good\":{\"$ref\":\"https://example.com/spec.json#/components/examples/"
      "Ex\"},"
      "\"bad\":{\"$ref\":\"#/components/examples/Ex/foo\"}"
      "}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(2, p->n_examples);
    ASSERT_STR_EQ("good", p->examples[0].name);
    ASSERT_STR_EQ("Right", p->examples[0].summary);
    ASSERT_STR_EQ("bad", p->examples[1].name);
    ASSERT_EQ(NULL, p->examples[1].summary);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_request_body_metadata_and_response_description(void) {
  const char *json =
      "{\"paths\":{\"/p\":{\"post\":{\"requestBody\":{"
      "\"description\":\"Payload\",\"required\":false,"
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"}}}"
      "},\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"}}}"
      "}}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_STR_EQ("Payload", op->req_body_description);
    ASSERT_EQ(1, op->req_body_required_set);
    ASSERT_EQ(0, op->req_body_required);
    ASSERT_STR_EQ("OK", op->responses[0].description);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_request_body_component_ref(void) {
  const char *json =
      "{"
      "\"components\":{"
      "\"schemas\":{\"Pet\":{\"type\":\"object\",\"properties\":{"
      "\"id\":{\"type\":\"integer\"}}}},"
      "\"requestBodies\":{"
      "\"CreatePet\":{\"description\":\"Create\",\"required\":true,"
      "\"content\":{\"application/json\":{\"schema\":{"
      "\"$ref\":\"#/components/schemas/Pet\"}}}}"
      "}"
      "},"
      "\"paths\":{\"/pets\":{\"post\":{"
      "\"requestBody\":{\"$ref\":\"#/components/requestBodies/CreatePet\"},"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}"
      "}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_request_bodies);
  ASSERT_STR_EQ("CreatePet", spec.component_request_body_names[0]);
  ASSERT_STR_EQ("Create", spec.component_request_bodies[0].description);
  ASSERT_EQ(1, spec.component_request_bodies[0].required_set);
  ASSERT_EQ(1, spec.component_request_bodies[0].required);
  ASSERT_STR_EQ("application/json",
                spec.component_request_bodies[0].schema.content_type);
  ASSERT_STR_EQ("Pet", spec.component_request_bodies[0].schema.ref_name);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_STR_EQ("#/components/requestBodies/CreatePet", op->req_body_ref);
    ASSERT_STR_EQ("Create", op->req_body_description);
    ASSERT_EQ(1, op->req_body_required_set);
    ASSERT_EQ(1, op->req_body_required);
    ASSERT_STR_EQ("application/json", op->req_body.content_type);
    ASSERT_STR_EQ("Pet", op->req_body.ref_name);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_response_multiple_content(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"components\":{\"schemas\":{"
      "\"Pet\":{\"type\":\"object\",\"properties\":{\"id\":{\"type\":"
      "\"integer\"}}}"
      "}},\"paths\":{\"/pets\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\",\"content\":{"
      "\"application/json\":{\"schema\":{\"$ref\":\"#/components/schemas/"
      "Pet\"}},"
      "\"text/plain\":{\"schema\":{\"type\":\"string\"}}"
      "}}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Response *resp = &spec.paths[0].operations[0].responses[0];
    ASSERT_EQ(2, resp->n_content_media_types);
    ASSERT_STR_EQ("application/json", resp->content_type);
    ASSERT_STR_EQ("Pet", resp->schema.ref_name);
    {
      const struct OpenAPI_MediaType *text_mt = find_media_type(
          resp->content_media_types, resp->n_content_media_types, "text/plain");
      ASSERT(text_mt != NULL);
      ASSERT_EQ(1, text_mt->schema_set);
      ASSERT_STR_EQ("string", text_mt->schema.inline_type);
    }
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_request_body_multiple_content_with_ref(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"components\":{\"schemas\":{"
      "\"Pet\":{\"type\":\"object\",\"properties\":{\"id\":{\"type\":"
      "\"integer\"}}}"
      "},\"mediaTypes\":{"
      "\"application/json\":{\"schema\":{\"$ref\":\"#/components/schemas/"
      "Pet\"}}"
      "}},\"paths\":{\"/pets\":{\"post\":{\"requestBody\":{\"content\":{"
      "\"application/json\":{\"$ref\":\"#/components/mediaTypes/"
      "application~1json\"},"
      "\"application/x-www-form-urlencoded\":{\"schema\":{\"type\":\"object\"}}"
      "}}}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_EQ(2, op->n_req_body_media_types);
    ASSERT_STR_EQ("application/json", op->req_body.content_type);
    ASSERT_STR_EQ("Pet", op->req_body.ref_name);
    {
      const struct OpenAPI_MediaType *mt =
          find_media_type(op->req_body_media_types, op->n_req_body_media_types,
                          "application/json");
      ASSERT(mt != NULL);
      ASSERT_STR_EQ("#/components/mediaTypes/application~1json", mt->ref);
    }
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_media_type_encoding(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"components\":{\"mediaTypes\":{"
      "\"multipart/form-data\":{"
      "\"schema\":{\"type\":\"object\"},"
      "\"encoding\":{"
      "\"file\":{"
      "\"contentType\":\"image/png\","
      "\"explode\":true,"
      "\"allowReserved\":true,"
      "\"headers\":{"
      "\"X-Rate-Limit-Limit\":{\"schema\":{\"type\":\"integer\"}}"
      "}"
      "}"
      "}"
      "}"
      "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_media_types);
  {
    const struct OpenAPI_MediaType *mt = &spec.component_media_types[0];
    ASSERT_EQ(1, mt->n_encoding);
    ASSERT_STR_EQ("file", mt->encoding[0].name);
    ASSERT_STR_EQ("image/png", mt->encoding[0].content_type);
    ASSERT_EQ(1, mt->encoding[0].explode_set);
    ASSERT_EQ(1, mt->encoding[0].explode);
    ASSERT_EQ(1, mt->encoding[0].allow_reserved_set);
    ASSERT_EQ(1, mt->encoding[0].allow_reserved);
    ASSERT_EQ(1, mt->encoding[0].n_headers);
    ASSERT_STR_EQ("X-Rate-Limit-Limit", mt->encoding[0].headers[0].name);
    ASSERT_STR_EQ("integer", mt->encoding[0].headers[0].type);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_media_type_prefix_item_encoding(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"components\":{\"mediaTypes\":{"
      "\"multipart/mixed\":{"
      "\"schema\":{\"type\":\"array\"},"
      "\"prefixEncoding\":["
      "{\"contentType\":\"application/json\"},"
      "{\"contentType\":\"image/png\","
      "\"headers\":{\"X-Pos\":{\"schema\":{\"type\":\"string\"}}}"
      "}"
      "],"
      "\"itemEncoding\":{"
      "\"contentType\":\"application/octet-stream\","
      "\"encoding\":{\"meta\":{\"contentType\":\"text/plain\"}}"
      "}"
      "}"
      "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_media_types);
  {
    const struct OpenAPI_MediaType *mt = &spec.component_media_types[0];
    ASSERT_EQ(2, mt->n_prefix_encoding);
    ASSERT_STR_EQ("application/json", mt->prefix_encoding[0].content_type);
    ASSERT_STR_EQ("image/png", mt->prefix_encoding[1].content_type);
    ASSERT_EQ(1, mt->prefix_encoding[1].n_headers);
    ASSERT_STR_EQ("X-Pos", mt->prefix_encoding[1].headers[0].name);
    ASSERT_STR_EQ("string", mt->prefix_encoding[1].headers[0].type);

    ASSERT(mt->item_encoding != NULL);
    ASSERT_EQ(1, mt->item_encoding_set);
    ASSERT_STR_EQ("application/octet-stream", mt->item_encoding->content_type);
    ASSERT_EQ(1, mt->item_encoding->n_encoding);
    ASSERT_STR_EQ("meta", mt->item_encoding->encoding[0].name);
    ASSERT_STR_EQ("text/plain", mt->item_encoding->encoding[0].content_type);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_info_metadata(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{"
      "\"title\":\"Example "
      "API\",\"summary\":\"Short\",\"description\":\"Long\","
      "\"termsOfService\":\"https://example.com/terms\","
      "\"version\":\"2.1.0\","
      "\"contact\":{\"name\":\"API Support\",\"url\":\"https://example.com\","
      "\"email\":\"support@example.com\"},"
      "\"license\":{\"name\":\"Apache 2.0\",\"identifier\":\"Apache-2.0\"}"
      "},\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("Example API", spec.info.title);
  ASSERT_STR_EQ("Short", spec.info.summary);
  ASSERT_STR_EQ("Long", spec.info.description);
  ASSERT_STR_EQ("https://example.com/terms", spec.info.terms_of_service);
  ASSERT_STR_EQ("2.1.0", spec.info.version);
  ASSERT_STR_EQ("API Support", spec.info.contact.name);
  ASSERT_STR_EQ("https://example.com", spec.info.contact.url);
  ASSERT_STR_EQ("support@example.com", spec.info.contact.email);
  ASSERT_STR_EQ("Apache 2.0", spec.info.license.name);
  ASSERT_STR_EQ("Apache-2.0", spec.info.license.identifier);
  ASSERT(spec.info.license.url == NULL);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_info_missing_title_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"version\":\"1\"},\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_info_missing_version_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\"},\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_license_identifier_and_url_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{"
      "\"title\":\"Example API\",\"version\":\"1\","
      "\"license\":{\"name\":\"Apache 2.0\",\"identifier\":\"Apache-2.0\","
      "\"url\":\"https://www.apache.org/licenses/LICENSE-2.0.html\"}"
      "},\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_license_missing_name_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{"
                     "\"title\":\"Example API\",\"version\":\"1\","
                     "\"license\":{\"identifier\":\"Apache-2.0\"}"
                     "},\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_operation_metadata(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/meta\":{\"get\":{"
                     "\"operationId\":\"getMeta\","
                     "\"summary\":\"Summary text\","
                     "\"description\":\"Longer description\","
                     "\"deprecated\":true"
                     "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("Summary text", spec.paths[0].operations[0].summary);
  ASSERT_STR_EQ("Longer description", spec.paths[0].operations[0].description);
  ASSERT_EQ(1, spec.paths[0].operations[0].deprecated);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_response_content_type(void) {
  const char *json =
      "{\"paths\":{\"/r\":{\"get\":{"
      "\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"text/plain\":{\"schema\":{\"$ref\":\"#/components/"
      "schemas/Message\"}}}"
      "}}"
      "}}},"
      "\"components\":{\"schemas\":{\"Message\":{\"type\":\"string\"}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("text/plain",
                spec.paths[0].operations[0].responses[0].content_type);
  ASSERT_STR_EQ("Message",
                spec.paths[0].operations[0].responses[0].schema.ref_name);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_response_content_type_specificity(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/r\":{\"get\":{"
                     "\"responses\":{\"200\":{\"description\":\"OK\","
                     "\"content\":{"
                     "\"text/*\":{\"schema\":{\"type\":\"string\"}},"
                     "\"text/plain\":{\"schema\":{\"type\":\"string\"}}"
                     "}"
                     "}}"
                     "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("text/plain",
                spec.paths[0].operations[0].responses[0].content_type);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_response_content_type_params_json(void) {
  const char *json =
      "{\"paths\":{\"/r\":{\"get\":{"
      "\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{"
      "\"text/plain\":{\"schema\":{\"type\":\"string\"}},"
      "\"application/json; charset=utf-8\":{\"schema\":{\"type\":\"string\"}}"
      "}"
      "}}"
      "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("application/json; charset=utf-8",
                spec.paths[0].operations[0].responses[0].content_type);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_response_schema_primitive(void) {
  const char *json =
      "{\"paths\":{\"/r\":{\"get\":{"
      "\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"}}}"
      "}}"
      "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("string",
                spec.paths[0].operations[0].responses[0].schema.inline_type);
  ASSERT_EQ(0, spec.paths[0].operations[0].responses[0].schema.is_array);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_response_schema_array(void) {
  const char *json =
      "{\"paths\":{\"/r\":{\"get\":{"
      "\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"integer\"}}}"
      "}}"
      "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].schema.is_array);
  ASSERT_STR_EQ("integer",
                spec.paths[0].operations[0].responses[0].schema.inline_type);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_schema_format_and_content(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/r\":{\"get\":{"
                     "\"responses\":{\"200\":{\"description\":\"OK\","
                     "\"content\":{\"application/json\":{\"schema\":{"
                     "\"type\":\"string\",\"format\":\"uuid\","
                     "\"contentMediaType\":\"image/png\","
                     "\"contentEncoding\":\"base64\""
                     "}}}"
                     "}}"
                     "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].responses[0].schema;
    ASSERT_STR_EQ("uuid", schema->format);
    ASSERT_STR_EQ("image/png", schema->content_media_type);
    ASSERT_STR_EQ("base64", schema->content_encoding);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_schema_array_item_format_and_content(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/r\":{\"get\":{"
                     "\"responses\":{\"200\":{\"description\":\"OK\","
                     "\"content\":{\"application/json\":{\"schema\":{"
                     "\"type\":\"array\",\"items\":{"
                     "\"type\":\"string\",\"format\":\"uuid\","
                     "\"contentMediaType\":\"image/png\","
                     "\"contentEncoding\":\"base64\""
                     "}}}"
                     "}}"
                     "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].responses[0].schema;
    ASSERT_EQ(1, schema->is_array);
    ASSERT_STR_EQ("uuid", schema->items_format);
    ASSERT_STR_EQ("image/png", schema->items_content_media_type);
    ASSERT_STR_EQ("base64", schema->items_content_encoding);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_schema_const_examples_annotations(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/p\":{\"get\":{"
                     "\"parameters\":[{\"name\":\"mode\",\"in\":\"query\","
                     "\"schema\":{"
                     "\"type\":\"string\","
                     "\"const\":\"fast\","
                     "\"examples\":[\"fast\",\"slow\"],"
                     "\"description\":\"Mode\","
                     "\"deprecated\":true,"
                     "\"readOnly\":true,"
                     "\"writeOnly\":false"
                     "}}],"
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}"
                     "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].parameters[0].schema;
    ASSERT_EQ(1, schema->const_value_set);
    ASSERT_EQ(OA_ANY_STRING, schema->const_value.type);
    ASSERT_STR_EQ("fast", schema->const_value.string);
    ASSERT_EQ(2, schema->n_examples);
    ASSERT_EQ(OA_ANY_STRING, schema->examples[0].type);
    ASSERT_STR_EQ("fast", schema->examples[0].string);
    ASSERT_EQ(OA_ANY_STRING, schema->examples[1].type);
    ASSERT_STR_EQ("slow", schema->examples[1].string);
    ASSERT_STR_EQ("Mode", schema->description);
    ASSERT_EQ(1, schema->deprecated_set);
    ASSERT_EQ(1, schema->deprecated);
    ASSERT_EQ(1, schema->read_only_set);
    ASSERT_EQ(1, schema->read_only);
    ASSERT_EQ(1, schema->write_only_set);
    ASSERT_EQ(0, schema->write_only);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_schema_ref_summary_description(void) {
  const char *json =
      "{"
      "\"paths\":{\"/p\":{\"get\":{"
      "\"parameters\":[{\"name\":\"mode\",\"in\":\"query\","
      "\"schema\":{"
      "\"$ref\":\"#/components/schemas/Mode\","
      "\"summary\":\"Mode summary\","
      "\"description\":\"Mode description\""
      "}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}"
      "}}},"
      "\"components\":{\"schemas\":{\"Mode\":{\"type\":\"string\"}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].parameters[0].schema;
    ASSERT_STR_EQ("Mode", schema->ref_name);
    ASSERT_STR_EQ("Mode summary", schema->summary);
    ASSERT_STR_EQ("Mode description", schema->description);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_parameter_schema_format_and_content(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/p\":{\"get\":{"
                     "\"parameters\":[{\"name\":\"id\",\"in\":\"query\","
                     "\"schema\":{\"type\":\"string\",\"format\":\"uuid\","
                     "\"contentMediaType\":\"text/plain\","
                     "\"contentEncoding\":\"base64\"}}],"
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}"
                     "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_STR_EQ("uuid", p->schema.format);
    ASSERT_STR_EQ("text/plain", p->schema.content_media_type);
    ASSERT_STR_EQ("base64", p->schema.content_encoding);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_schema_enum_default_nullable(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/p\":{\"get\":{"
                     "\"parameters\":[{\"name\":\"status\",\"in\":\"query\","
                     "\"schema\":{\"type\":[\"string\",\"null\"],"
                     "\"enum\":[\"on\",\"off\"],\"default\":\"on\"}}],"
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}"
                     "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_STR_EQ("string", p->schema.inline_type);
    ASSERT_EQ(1, p->schema.nullable);
    ASSERT_EQ(2, p->schema.n_enum_values);
    ASSERT_EQ(OA_ANY_STRING, p->schema.enum_values[0].type);
    ASSERT_STR_EQ("on", p->schema.enum_values[0].string);
    ASSERT_EQ(OA_ANY_STRING, p->schema.enum_values[1].type);
    ASSERT_STR_EQ("off", p->schema.enum_values[1].string);
    ASSERT_EQ(1, p->schema.default_value_set);
    ASSERT_EQ(OA_ANY_STRING, p->schema.default_value.type);
    ASSERT_STR_EQ("on", p->schema.default_value.string);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_schema_type_union(void) {
  const char *json =
      "{\"paths\":{\"/p\":{\"get\":{"
      "\"parameters\":[{\"name\":\"mix\",\"in\":\"query\","
      "\"schema\":{\"type\":[\"string\",\"integer\",\"null\"]}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}"
      "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_STR_EQ("string", p->schema.inline_type);
    ASSERT_EQ(1, p->schema.nullable);
    ASSERT_EQ(3, p->schema.n_type_union);
    ASSERT_STR_EQ("string", p->schema.type_union[0]);
    ASSERT_STR_EQ("integer", p->schema.type_union[1]);
    ASSERT_STR_EQ("null", p->schema.type_union[2]);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_schema_array_items_enum_nullable(void) {
  const char *json =
      "{\"paths\":{\"/p\":{\"get\":{"
      "\"parameters\":[{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"items\":{"
      "\"type\":[\"string\",\"null\"],\"enum\":[\"a\",\"b\"]}}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}"
      "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_EQ(1, p->schema.is_array);
    ASSERT_STR_EQ("string", p->schema.inline_type);
    ASSERT_EQ(1, p->schema.items_nullable);
    ASSERT_EQ(2, p->schema.n_items_enum_values);
    ASSERT_EQ(OA_ANY_STRING, p->schema.items_enum_values[0].type);
    ASSERT_STR_EQ("a", p->schema.items_enum_values[0].string);
    ASSERT_EQ(OA_ANY_STRING, p->schema.items_enum_values[1].type);
    ASSERT_STR_EQ("b", p->schema.items_enum_values[1].string);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_schema_items_type_union(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/p\":{\"get\":{"
                     "\"parameters\":[{\"name\":\"tags\",\"in\":\"query\","
                     "\"schema\":{\"type\":\"array\",\"items\":{"
                     "\"type\":[\"string\",\"integer\"]}}}],"
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}"
                     "}}}}";

  struct OpenAPI_Spec spec;
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
  PASS();
}

TEST test_load_schema_boolean_and_numeric_enum(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/p\":{\"get\":{"
                     "\"parameters\":["
                     "{\"name\":\"any\",\"in\":\"query\",\"schema\":true},"
                     "{\"name\":\"level\",\"in\":\"query\","
                     "\"schema\":{\"type\":\"integer\",\"enum\":[1,2]}}],"
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}"
                     "}}}}";

  struct OpenAPI_Spec spec;
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
  PASS();
}

TEST test_load_schema_items_examples_and_boolean_items(void) {
  const char *json =
      "{\"paths\":{\"/p\":{\"get\":{"
      "\"parameters\":["
      "{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\","
      "\"items\":{\"type\":\"string\",\"examples\":[\"a\",\"b\"]}}},"
      "{\"name\":\"anys\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"items\":false}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}"
      "}}}}";

  struct OpenAPI_Spec spec;
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
  PASS();
}

TEST test_load_inline_schema_example_and_numeric_constraints(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/p\":{\"get\":{"
                     "\"parameters\":[{\"name\":\"score\",\"in\":\"query\","
                     "\"schema\":{\"type\":\"number\",\"minimum\":1,"
                     "\"exclusiveMaximum\":9,\"example\":2.5}}],"
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}"
                     "}}}}";

  struct OpenAPI_Spec spec;
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
  PASS();
}

TEST test_load_inline_schema_array_constraints_and_items_example(void) {
  const char *json =
      "{\"paths\":{\"/p\":{\"get\":{"
      "\"parameters\":[{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"minItems\":1,\"maxItems\":3,"
      "\"uniqueItems\":true,\"items\":{"
      "\"type\":\"string\",\"minLength\":2,\"maxLength\":5,"
      "\"pattern\":\"^[a-z]+$\",\"example\":\"ab\"}}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}"
      "}}}}";

  struct OpenAPI_Spec spec;
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
  PASS();
}

TEST test_load_inline_schema_items_const_default_and_extras(void) {
  const char *json =
      "{\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
      "\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"x-top\":true,"
      "\"items\":{\"type\":\"string\",\"const\":\"x\",\"default\":\"y\","
      "\"x-custom\":99}}"
      "}]}}}}";

  struct OpenAPI_Spec spec;
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
  PASS();
}

TEST test_load_inline_request_body_object_promoted(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"components\":{\"schemas\":{"
      "\"Inline_createPet_Request\":{"
      "\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"string\"}}"
      "}}},"
      "\"paths\":{\"/pets\":{\"post\":{"
      "\"operationId\":\"createPet\","
      "\"requestBody\":{\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"object\",\"properties\":{"
      "\"name\":{\"type\":\"string\"}"
      "}}}}},"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}"
      "}}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(2, spec.n_defined_schemas);
  ASSERT_STR_EQ("Inline_createPet_Request_1",
                spec.paths[0].operations[0].req_body.ref_name);
  {
    const struct StructFields *sf =
        openapi_spec_find_schema(&spec, "Inline_createPet_Request_1");
    ASSERT(sf != NULL);
    {
      struct StructField *field = struct_fields_get(sf, "name");
      ASSERT(field != NULL);
      ASSERT_STR_EQ("string", field->type);
    }
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_request_body_item_schema_array(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"t\",\"version\":\"1\"},"
                     "\"paths\":{\"/stream\":{\"post\":{"
                     "\"operationId\":\"streamPets\","
                     "\"requestBody\":{\"content\":{\"application/jsonl\":{"
                     "\"itemSchema\":{\"type\":\"string\"}"
                     "}}},"
                     "\"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "}}}}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.paths[0].operations[0].req_body.is_array);
  ASSERT_STR_EQ("string", spec.paths[0].operations[0].req_body.inline_type);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_response_schema_object_item_promoted(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/pets\":{\"get\":{"
      "\"operationId\":\"listPets\","
      "\"responses\":{\"200\":{\"description\":\"ok\","
      "\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"array\",\"items\":{"
      "\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"integer\"}}"
      "}}}}}"
      "}}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].schema.is_array);
  ASSERT_STR_EQ("Inline_listPets_Response_200_Item",
                spec.paths[0].operations[0].responses[0].schema.ref_name);
  {
    const struct StructFields *sf =
        openapi_spec_find_schema(&spec, "Inline_listPets_Response_200_Item");
    ASSERT(sf != NULL);
    {
      struct StructField *field = struct_fields_get(sf, "id");
      ASSERT(field != NULL);
      ASSERT_STR_EQ("integer", field->type);
    }
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_inline_response_item_schema_object_promoted(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"t\",\"version\":\"1\"},"
                     "\"paths\":{\"/stream\":{\"get\":{"
                     "\"operationId\":\"streamPets\","
                     "\"responses\":{\"200\":{\"description\":\"ok\","
                     "\"content\":{\"application/jsonl\":{"
                     "\"itemSchema\":{\"type\":\"object\",\"properties\":{"
                     "\"name\":{\"type\":\"string\"}"
                     "}}}}}"
                     "}}}}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].schema.is_array);
  ASSERT_STR_EQ("Inline_streamPets_Response_200_Item",
                spec.paths[0].operations[0].responses[0].schema.ref_name);
  {
    const struct StructFields *sf =
        openapi_spec_find_schema(&spec, "Inline_streamPets_Response_200_Item");
    ASSERT(sf != NULL);
    {
      struct StructField *field = struct_fields_get(sf, "name");
      ASSERT(field != NULL);
      ASSERT_STR_EQ("string", field->type);
    }
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_request_body_ref_description_override(void) {
  const char *json =
      "{"
      "\"components\":{\"requestBodies\":{"
      "\"CreatePet\":{\"description\":\"Create\",\"required\":true,"
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"}}}"
      "}}},"
      "\"paths\":{\"/pets\":{\"post\":{"
      "\"requestBody\":{\"$ref\":\"#/components/requestBodies/CreatePet\","
      "\"description\":\"Override\"},"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}"
      "}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("#/components/requestBodies/CreatePet",
                spec.paths[0].operations[0].req_body_ref);
  ASSERT_STR_EQ("Override", spec.paths[0].operations[0].req_body_description);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_options_trace_verbs(void) {
  const char *json =
      "{\"paths\":{\"/v\":{\"options\":{\"operationId\":\"opt\"},"
      "\"trace\":{\"operationId\":\"tr\"}}}}";

  struct OpenAPI_Spec spec;
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
  PASS();
}

TEST test_load_root_metadata_and_tags(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"$self\":\"https://example.com/openapi.json\","
      "\"jsonSchemaDialect\":\"https://spec.openapis.org/oas/3.1/dialect/"
      "base\","
      "\"externalDocs\":{\"description\":\"Root "
      "docs\",\"url\":\"https://example.com/docs\"},"
      "\"tags\":[{"
      "\"name\":\"pets\",\"summary\":\"Pets\",\"description\":\"Pet ops\","
      "\"parent\":\"animals\",\"kind\":\"nav\","
      "\"externalDocs\":{\"description\":\"Tag "
      "docs\",\"url\":\"https://example.com/tags/pets\"}"
      "}],\"paths\":{}}";

  struct OpenAPI_Spec spec;
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
  PASS();
}

TEST test_self_qualified_component_refs(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"$self\":\"https://example.com/openapi.json\","
                     "\"info\":{\"title\":\"Self\",\"version\":\"1\"},"
                     "\"components\":{"
                     "\"schemas\":{"
                     "\"Pet\":{\"type\":\"object\",\"properties\":{\"id\":{"
                     "\"type\":\"integer\"}}}"
                     "},"
                     "\"parameters\":{"
                     "\"PetParam\":{"
                     "\"name\":\"pet\",\"in\":\"query\","
                     "\"schema\":{\"$ref\":\"https://example.com/openapi.json#/"
                     "components/schemas/Pet\"}"
                     "}"
                     "}"
                     "},"
                     "\"paths\":{"
                     "\"/pets\":{\"get\":{"
                     "\"operationId\":\"getPets\","
                     "\"parameters\":[{\"$ref\":\"https://example.com/"
                     "openapi.json#/components/parameters/PetParam\"}],"
                     "\"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "}}"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("Pet", p->schema.ref_name);
    ASSERT_EQ(1, spec.n_component_parameters);
    ASSERT_STR_EQ("Pet", spec.component_parameters[0].schema.ref_name);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_relative_self_component_refs(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"$self\":\"/api/openapi.json\","
                     "\"info\":{\"title\":\"Self\",\"version\":\"1\"},"
                     "\"components\":{"
                     "\"parameters\":{"
                     "\"PetParam\":{"
                     "\"name\":\"pet\",\"in\":\"query\","
                     "\"schema\":{\"type\":\"string\"}"
                     "}"
                     "}"
                     "},"
                     "\"paths\":{"
                     "\"/pets\":{\"get\":{"
                     "\"operationId\":\"getPets\","
                     "\"parameters\":[{\"$ref\":\"https://example.com/api/"
                     "openapi.json#/components/parameters/PetParam\"}],"
                     "\"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "}}"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("pet", p->name);
    ASSERT_EQ(OA_PARAM_IN_QUERY, p->in);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_schema_id_ref_resolution(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"ID\",\"version\":\"1\"},"
                     "\"components\":{"
                     "\"schemas\":{"
                     "\"Foo\":{"
                     "\"$id\":\"https://example.com/schemas/foo\","
                     "\"type\":\"object\","
                     "\"properties\":{\"id\":{\"type\":\"string\"}}"
                     "}"
                     "}"
                     "},"
                     "\"paths\":{"
                     "\"/foo\":{\"get\":{"
                     "\"operationId\":\"getFoo\","
                     "\"parameters\":[{"
                     "\"name\":\"f\",\"in\":\"query\","
                     "\"schema\":{\"$ref\":\"https://example.com/schemas/foo\"}"
                     "}],"
                     "\"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "}}"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    const struct StructFields *sf =
        openapi_spec_find_schema_for_ref(&spec, &p->schema);
    ASSERT(sf != NULL);
    ASSERT(spec.defined_schema_ids != NULL);
    ASSERT_STR_EQ("https://example.com/schemas/foo",
                  spec.defined_schema_ids[0]);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_schema_anchor_ref_resolution(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"Anchor\",\"version\":\"1\"},"
                     "\"components\":{"
                     "\"schemas\":{"
                     "\"Foo\":{"
                     "\"$anchor\":\"FooAnchor\","
                     "\"type\":\"object\","
                     "\"properties\":{\"id\":{\"type\":\"string\"}}"
                     "}"
                     "}"
                     "},"
                     "\"paths\":{"
                     "\"/foo\":{\"get\":{"
                     "\"operationId\":\"getFoo\","
                     "\"parameters\":[{"
                     "\"name\":\"f\",\"in\":\"query\","
                     "\"schema\":{\"$ref\":\"#FooAnchor\"}"
                     "}],"
                     "\"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "}}"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    const struct StructFields *sf =
        openapi_spec_find_schema_for_ref(&spec, &p->schema);
    ASSERT(sf != NULL);
    ASSERT(spec.defined_schema_anchors != NULL);
    ASSERT_STR_EQ("FooAnchor", spec.defined_schema_anchors[0]);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_schema_dynamic_ref_resolution(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"Dynamic\",\"version\":\"1\"},"
                     "\"components\":{"
                     "\"schemas\":{"
                     "\"Foo\":{"
                     "\"$dynamicAnchor\":\"FooDyn\","
                     "\"type\":\"object\","
                     "\"properties\":{\"id\":{\"type\":\"string\"}}"
                     "}"
                     "}"
                     "},"
                     "\"paths\":{"
                     "\"/foo\":{\"get\":{"
                     "\"operationId\":\"getFoo\","
                     "\"parameters\":[{"
                     "\"name\":\"f\",\"in\":\"query\","
                     "\"schema\":{\"$dynamicRef\":\"#FooDyn\"}"
                     "}],"
                     "\"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "}}"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    const struct StructFields *sf =
        openapi_spec_find_schema_for_ref(&spec, &p->schema);
    ASSERT(sf != NULL);
    ASSERT(spec.defined_schema_dynamic_anchors != NULL);
    ASSERT_STR_EQ("FooDyn", spec.defined_schema_dynamic_anchors[0]);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_external_component_ref_registry_absolute(void) {
  const char *shared = "{"
                       "\"openapi\":\"3.2.0\","
                       "\"$self\":\"https://example.com/shared.json\","
                       "\"info\":{\"title\":\"Shared\",\"version\":\"1\"},"
                       "\"components\":{"
                       "\"parameters\":{"
                       "\"PetParam\":{"
                       "\"name\":\"pet\",\"in\":\"query\","
                       "\"schema\":{\"type\":\"string\"}"
                       "}"
                       "}"
                       "}"
                       "}";
  const char *root =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"$self\":\"https://example.com/root.json\","
      "\"info\":{\"title\":\"Root\",\"version\":\"1\"},"
      "\"paths\":{"
      "\"/pets\":{\"get\":{"
      "\"parameters\":[{\"$ref\":\"https://example.com/shared.json#/"
      "components/parameters/PetParam\"}],"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}"
      "}}"
      "}"
      "}";

  struct OpenAPI_DocRegistry registry;
  struct OpenAPI_Spec shared_spec;
  struct OpenAPI_Spec root_spec;
  int rc;

  openapi_doc_registry_init(&registry);

  rc = load_spec_str_with_context(shared, "https://example.com/shared.json",
                                  &registry, &shared_spec);
  ASSERT_EQ(0, rc);
  rc = load_spec_str_with_context(root, "https://example.com/root.json",
                                  &registry, &root_spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p =
        &root_spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("pet", p->name);
    ASSERT_EQ(OA_PARAM_IN_QUERY, p->in);
  }

  openapi_spec_free(&root_spec);
  openapi_spec_free(&shared_spec);
  openapi_doc_registry_free(&registry);
  PASS();
}

TEST test_external_component_ref_registry_relative(void) {
  const char *shared = "{"
                       "\"openapi\":\"3.2.0\","
                       "\"info\":{\"title\":\"Shared\",\"version\":\"1\"},"
                       "\"components\":{"
                       "\"parameters\":{"
                       "\"PetParam\":{"
                       "\"name\":\"pet\",\"in\":\"query\","
                       "\"schema\":{\"type\":\"string\"}"
                       "}"
                       "}"
                       "}"
                       "}";
  const char *root = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"Root\",\"version\":\"1\"},"
                     "\"paths\":{"
                     "\"/pets\":{\"get\":{"
                     "\"parameters\":[{\"$ref\":\"shared.json#/components/"
                     "parameters/PetParam\""
                     "}],"
                     "\"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "}}"
                     "}"
                     "}";

  struct OpenAPI_DocRegistry registry;
  struct OpenAPI_Spec shared_spec;
  struct OpenAPI_Spec root_spec;
  int rc;

  openapi_doc_registry_init(&registry);

  rc = load_spec_str_with_context(shared, "https://example.com/api/shared.json",
                                  &registry, &shared_spec);
  ASSERT_EQ(0, rc);
  rc = load_spec_str_with_context(root, "https://example.com/api/openapi.json",
                                  &registry, &root_spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p =
        &root_spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("pet", p->name);
    ASSERT_EQ(OA_PARAM_IN_QUERY, p->in);
  }

  openapi_spec_free(&root_spec);
  openapi_spec_free(&shared_spec);
  openapi_doc_registry_free(&registry);
  PASS();
}

TEST test_load_query_verb_and_external_docs(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/search\":{\"query\":{"
                     "\"operationId\":\"querySearch\","
                     "\"externalDocs\":{\"description\":\"Op "
                     "docs\",\"url\":\"https://example.com/op\"}"
                     "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.paths[0].n_operations);
  ASSERT_EQ(OA_VERB_QUERY, spec.paths[0].operations[0].verb);
  ASSERT_STR_EQ("https://example.com/op",
                spec.paths[0].operations[0].external_docs.url);
  ASSERT_STR_EQ("Op docs",
                spec.paths[0].operations[0].external_docs.description);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_path_and_operation_servers(void) {
  const char *json = "{"
                     "\"paths\":{"
                     "  \"/pets\":{"
                     "    \"servers\":[{\"url\":\"https://path.example.com\"}],"
                     "    \"get\":{"
                     "      \"operationId\":\"listPets\","
                     "      "
                     "\"servers\":[{\"url\":\"https://"
                     "op.example.com\",\"description\":\"Op\"}],"
                     "      \"responses\":{\"200\":{\"description\":\"OK\"}}"
                     "    }"
                     "  }"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].n_servers);
  ASSERT_STR_EQ("https://path.example.com", spec.paths[0].servers[0].url);

  ASSERT_EQ(1, spec.paths[0].n_operations);
  ASSERT_EQ(1, spec.paths[0].operations[0].n_servers);
  ASSERT_STR_EQ("https://op.example.com",
                spec.paths[0].operations[0].servers[0].url);
  ASSERT_STR_EQ("Op", spec.paths[0].operations[0].servers[0].description);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_webhooks(void) {
  const char *json = "{"
                     "\"webhooks\":{"
                     "  \"petEvent\":{"
                     "    \"post\":{"
                     "      \"operationId\":\"onPetEvent\","
                     "      \"responses\":{\"200\":{\"description\":\"OK\"}}"
                     "    }"
                     "  }"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_webhooks);
  ASSERT_STR_EQ("petEvent", spec.webhooks[0].route);
  ASSERT_EQ(1, spec.webhooks[0].n_operations);
  ASSERT_EQ(OA_VERB_POST, spec.webhooks[0].operations[0].verb);
  ASSERT_STR_EQ("onPetEvent", spec.webhooks[0].operations[0].operation_id);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_path_ref(void) {
  const char *json = "{"
                     "\"paths\":{"
                     "  \"/foo\":{"
                     "    \"$ref\":\"#/components/pathItems/Foo\""
                     "  }"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_STR_EQ("/foo", spec.paths[0].route);
  ASSERT_STR_EQ("#/components/pathItems/Foo", spec.paths[0].ref);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_component_parameter_ref(void) {
  const char *json =
      "{"
      "\"components\":{"
      "  \"parameters\":{"
      "    \"LimitParam\":{\"name\":\"limit\",\"in\":\"query\","
      "      \"schema\":{\"type\":\"integer\"}"
      "    }"
      "  }"
      "},"
      "\"paths\":{"
      "  \"/items\":{"
      "    \"get\":{"
      "      "
      "\"parameters\":[{\"$ref\":\"#/components/parameters/LimitParam\"}],"
      "      \"responses\":{\"200\":{\"description\":\"OK\"}}"
      "    }"
      "  }"
      "}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_parameters);
  ASSERT_STR_EQ("LimitParam", spec.component_parameter_names[0]);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("limit", p->name);
    ASSERT_STR_EQ("integer", p->type);
    ASSERT_STR_EQ("#/components/parameters/LimitParam", p->ref);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_component_response_and_headers(void) {
  const char *json =
      "{"
      "\"components\":{"
      "  \"responses\":{"
      "    \"NotFound\":{"
      "      \"description\":\"missing\","
      "      \"headers\":{"
      "        \"X-Trace\":{\"schema\":{\"type\":\"string\"}}"
      "      }"
      "    }"
      "  },"
      "  \"headers\":{"
      "    \"RateLimit\":{"
      "      \"description\":\"limit\","
      "      \"schema\":{\"type\":\"integer\"}"
      "    }"
      "  }"
      "},"
      "\"paths\":{"
      "  \"/x\":{"
      "    \"get\":{"
      "      \"responses\":{"
      "        \"404\":{\"$ref\":\"#/components/responses/NotFound\"},"
      "        \"200\":{"
      "          \"description\":\"ok\","
      "          \"headers\":{"
      "            \"X-Rate\":{\"$ref\":\"#/components/headers/RateLimit\"}"
      "          }"
      "        }"
      "      }"
      "    }"
      "  }"
      "}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_responses);
  ASSERT_STR_EQ("NotFound", spec.component_response_names[0]);
  ASSERT_EQ(1, spec.n_component_headers);
  ASSERT_STR_EQ("RateLimit", spec.component_header_names[0]);

  {
    struct OpenAPI_Response *resp_404 = NULL;
    struct OpenAPI_Response *resp_200 = NULL;
    size_t i;
    for (i = 0; i < spec.paths[0].operations[0].n_responses; ++i) {
      struct OpenAPI_Response *r = &spec.paths[0].operations[0].responses[i];
      if (r->code && strcmp(r->code, "404") == 0)
        resp_404 = r;
      if (r->code && strcmp(r->code, "200") == 0)
        resp_200 = r;
    }
    ASSERT(resp_404 != NULL);
    ASSERT(resp_200 != NULL);
    ASSERT_STR_EQ("missing", resp_404->description);
    ASSERT_EQ(1, resp_404->n_headers);
    ASSERT_STR_EQ("X-Trace", resp_404->headers[0].name);
    ASSERT_STR_EQ("string", resp_404->headers[0].type);

    ASSERT_EQ(1, resp_200->n_headers);
    ASSERT_STR_EQ("X-Rate", resp_200->headers[0].name);
    ASSERT_STR_EQ("integer", resp_200->headers[0].type);
    ASSERT_STR_EQ("#/components/headers/RateLimit", resp_200->headers[0].ref);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_additional_operations(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/copy\":{\"additionalOperations\":{"
      "\"COPY\":{\"operationId\":\"copyItem\","
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}"
      "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].n_additional_operations);
  ASSERT_STR_EQ("COPY", spec.paths[0].additional_operations[0].method);
  ASSERT_EQ(1, spec.paths[0].additional_operations[0].is_additional);
  ASSERT_STR_EQ("copyItem",
                spec.paths[0].additional_operations[0].operation_id);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_component_media_type_ref(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"components\":{"
      "  \"schemas\":{\"Pet\":{\"type\":\"object\"}},"
      "  \"mediaTypes\":{"
      "    \"application/vnd.acme+json\":{"
      "      \"schema\":{\"$ref\":\"#/components/schemas/Pet\"}"
      "    }"
      "  }"
      "},"
      "\"paths\":{"
      "  \"/pets\":{"
      "    \"get\":{"
      "      \"responses\":{"
      "        \"200\":{"
      "          \"description\":\"ok\","
      "          \"content\":{"
      "            \"application/vnd.acme+json\":{"
      "              "
      "\"$ref\":\"#/components/mediaTypes/application~1vnd.acme+json\""
      "            }"
      "          }"
      "        }"
      "      }"
      "    }"
      "  }"
      "}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_media_types);
  ASSERT_STR_EQ("application/vnd.acme+json",
                spec.component_media_type_names[0]);

  {
    struct OpenAPI_Response *resp = &spec.paths[0].operations[0].responses[0];
    ASSERT_STR_EQ("#/components/mediaTypes/application~1vnd.acme+json",
                  resp->content_ref);
    ASSERT_STR_EQ("application/vnd.acme+json", resp->content_type);
    ASSERT_STR_EQ("Pet", resp->schema.ref_name);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_component_path_items(void) {
  const char *json = "{"
                     "\"components\":{"
                     "  \"pathItems\":{"
                     "    \"FooItem\":{"
                     "      \"summary\":\"foo\","
                     "      \"get\":{"
                     "        \"operationId\":\"getFoo\","
                     "        \"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "      }"
                     "    }"
                     "  }"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_path_items);
  ASSERT_STR_EQ("FooItem", spec.component_path_item_names[0]);
  ASSERT_EQ(1, spec.component_path_items[0].n_operations);
  ASSERT_STR_EQ("getFoo",
                spec.component_path_items[0].operations[0].operation_id);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_response_links_and_component_links(void) {
  const char *json = "{"
                     "\"components\":{"
                     "  \"links\":{"
                     "    \"NextPage\":{"
                     "      \"operationId\":\"listPets\","
                     "      \"parameters\":{"
                     "        \"limit\":50,"
                     "        \"offset\":\"$response.body#/offset\""
                     "      },"
                     "      \"server\":{\"url\":\"https://api.example.com\"}"
                     "    }"
                     "  }"
                     "},"
                     "\"paths\":{"
                     "  \"/pets\":{"
                     "    \"get\":{"
                     "      \"responses\":{"
                     "        \"200\":{"
                     "          \"description\":\"ok\","
                     "          \"links\":{"
                     "            \"next\":{"
                     "              \"$ref\":\"#/components/links/NextPage\""
                     "            }"
                     "          }"
                     "        }"
                     "      }"
                     "    }"
                     "  }"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_links);
  ASSERT_STR_EQ("NextPage", spec.component_links[0].name);
  ASSERT_STR_EQ("listPets", spec.component_links[0].operation_id);
  ASSERT_EQ(2, spec.component_links[0].n_parameters);
  ASSERT_STR_EQ("limit", spec.component_links[0].parameters[0].name);
  ASSERT_EQ(OA_ANY_NUMBER, spec.component_links[0].parameters[0].value.type);
  ASSERT_EQ(50, (int)spec.component_links[0].parameters[0].value.number);
  ASSERT_EQ(1, spec.component_links[0].server_set);
  ASSERT(spec.component_links[0].server != NULL);
  ASSERT_STR_EQ("https://api.example.com", spec.component_links[0].server->url);

  {
    struct OpenAPI_Link *link =
        &spec.paths[0].operations[0].responses[0].links[0];
    ASSERT_STR_EQ("next", link->name);
    ASSERT_STR_EQ("#/components/links/NextPage", link->ref);
    ASSERT_STR_EQ("listPets", link->operation_id);
    ASSERT_EQ(2, link->n_parameters);
    ASSERT_EQ(1, link->server_set);
    ASSERT(link->server != NULL);
    ASSERT_STR_EQ("https://api.example.com", link->server->url);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_callbacks_and_component_callbacks(void) {
  const char *json =
      "{"
      "\"components\":{"
      "  \"callbacks\":{"
      "    \"OnEvent\":{"
      "      \"{$request.body#/url}\":{"
      "        \"post\":{"
      "          \"responses\":{\"200\":{\"description\":\"ok\"}}"
      "        }"
      "      }"
      "    }"
      "  }"
      "},"
      "\"paths\":{"
      "  \"/pets\":{"
      "    \"get\":{"
      "      \"responses\":{\"200\":{\"description\":\"ok\"}},"
      "      \"callbacks\":{"
      "        \"onEvent\":{"
      "          \"{$request.body#/url}\":{"
      "            \"post\":{"
      "              \"responses\":{\"200\":{\"description\":\"ok\"}}"
      "            }"
      "          }"
      "        }"
      "      }"
      "    }"
      "  }"
      "}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_callbacks);
  ASSERT_STR_EQ("OnEvent", spec.component_callbacks[0].name);
  ASSERT_EQ(1, spec.component_callbacks[0].n_paths);
  ASSERT_STR_EQ("{$request.body#/url}",
                spec.component_callbacks[0].paths[0].route);

  ASSERT_EQ(1, spec.paths[0].operations[0].n_callbacks);
  ASSERT_STR_EQ("onEvent", spec.paths[0].operations[0].callbacks[0].name);
  ASSERT_EQ(1, spec.paths[0].operations[0].callbacks[0].n_paths);
  ASSERT_STR_EQ("{$request.body#/url}",
                spec.paths[0].operations[0].callbacks[0].paths[0].route);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_path_item_ref_resolves_component(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
                     "\"components\":{"
                     "  \"pathItems\":{"
                     "    \"Pets\":{"
                     "      \"get\":{"
                     "        \"operationId\":\"listPets\","
                     "        \"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "      }"
                     "    }"
                     "  }"
                     "},"
                     "\"paths\":{"
                     "  \"/pets\":{"
                     "    \"$ref\":\"#/components/pathItems/Pets\","
                     "    \"summary\":\"Pets\""
                     "  }"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_path_items);
  ASSERT_STR_EQ("Pets", spec.component_path_items[0].route);

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_STR_EQ("/pets", spec.paths[0].route);
  ASSERT_STR_EQ("#/components/pathItems/Pets", spec.paths[0].ref);
  ASSERT_STR_EQ("Pets", spec.paths[0].summary);
  ASSERT_EQ(1, spec.paths[0].n_operations);
  ASSERT_EQ(OA_VERB_GET, spec.paths[0].operations[0].verb);
  ASSERT_STR_EQ("listPets", spec.paths[0].operations[0].operation_id);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_callback_ref_resolves_component(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"components\":{"
      "  \"callbacks\":{"
      "    \"Notify\":{"
      "      \"{$request.body#/url}\":{"
      "        \"post\":{"
      "          \"responses\":{\"200\":{\"description\":\"ok\"}}"
      "        }"
      "      }"
      "    }"
      "  }"
      "},"
      "\"paths\":{"
      "  \"/pets\":{"
      "    \"get\":{"
      "      \"responses\":{\"200\":{\"description\":\"ok\"}},"
      "      \"callbacks\":{"
      "        \"onNotify\":{"
      "          \"$ref\":\"#/components/callbacks/Notify\","
      "          \"summary\":\"Override\""
      "        }"
      "      }"
      "    }"
      "  }"
      "}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_callbacks);
  ASSERT_STR_EQ("Notify", spec.component_callbacks[0].name);

  ASSERT_EQ(1, spec.paths[0].operations[0].n_callbacks);
  {
    struct OpenAPI_Callback *cb = &spec.paths[0].operations[0].callbacks[0];
    ASSERT_STR_EQ("onNotify", cb->name);
    ASSERT_STR_EQ("#/components/callbacks/Notify", cb->ref);
    ASSERT_STR_EQ("Override", cb->summary);
    ASSERT_EQ(1, cb->n_paths);
    ASSERT_STR_EQ("{$request.body#/url}", cb->paths[0].route);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_extensions_non_schema(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"x-root\":1,"
      "\"info\":{"
      "  \"title\":\"Test\","
      "  \"version\":\"1\","
      "  \"x-info\":\"info\","
      "  \"contact\":{\"name\":\"Support\",\"x-contact\":true},"
      "  \"license\":{\"name\":\"MIT\",\"x-license\":\"lic\"}"
      "},"
      "\"externalDocs\":{\"url\":\"https://example.com\",\"x-ext\":\"ext\"},"
      "\"tags\":[{\"name\":\"pet\",\"x-tag\":\"tag\"}],"
      "\"security\":[{\"api_key\":[],\"x-sec-req\":\"req\"}],"
      "\"components\":{"
      "  \"securitySchemes\":{"
      "    \"api_key\":{"
      "      \"type\":\"apiKey\","
      "      \"name\":\"X-API\","
      "      \"in\":\"header\","
      "      \"x-sec\":\"sec\""
      "    }"
      "  }"
      "},"
      "\"paths\":{"
      "  \"/pets\":{"
      "    \"x-path\":\"path\","
      "    \"get\":{"
      "      \"x-op\":2,"
      "      \"parameters\":["
      "        "
      "{\"name\":\"id\",\"in\":\"query\",\"schema\":{\"type\":\"string\"},"
      "         \"x-param\":\"param\"}"
      "      ],"
      "      \"requestBody\":{"
      "        \"description\":\"body\","
      "        \"content\":{"
      "          \"application/json\":{"
      "            \"schema\":{\"type\":\"string\"}"
      "          }"
      "        },"
      "        \"x-rb\":{\"note\":true}"
      "      },"
      "      \"responses\":{"
      "        \"200\":{\"description\":\"ok\",\"x-resp\":{\"ok\":true}},"
      "        \"x-responses\":{\"trace\":true}"
      "      },"
      "      \"callbacks\":{"
      "        \"onEvent\":{"
      "          \"x-cb\":\"cb\","
      "          \"{$request.body#/url}\":{"
      "            \"post\":{"
      "              \"responses\":{\"200\":{\"description\":\"ok\"}}"
      "            }"
      "          }"
      "        }"
      "      }"
      "    }"
      "  }"
      "}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root_ext = json_parse_string(spec.extensions_json);
    JSON_Object *root_obj = json_value_get_object(root_ext);
    ASSERT_EQ(1, (int)json_object_get_number(root_obj, "x-root"));
    json_value_free(root_ext);
  }

  {
    JSON_Value *info_ext = json_parse_string(spec.info.extensions_json);
    JSON_Object *info_obj = json_value_get_object(info_ext);
    ASSERT_STR_EQ("info", json_object_get_string(info_obj, "x-info"));
    json_value_free(info_ext);
  }

  {
    JSON_Value *contact_ext =
        json_parse_string(spec.info.contact.extensions_json);
    JSON_Object *contact_obj = json_value_get_object(contact_ext);
    ASSERT_EQ(1, json_object_get_boolean(contact_obj, "x-contact"));
    json_value_free(contact_ext);
  }

  {
    JSON_Value *license_ext =
        json_parse_string(spec.info.license.extensions_json);
    JSON_Object *license_obj = json_value_get_object(license_ext);
    ASSERT_STR_EQ("lic", json_object_get_string(license_obj, "x-license"));
    json_value_free(license_ext);
  }

  {
    JSON_Value *ext_docs_ext =
        json_parse_string(spec.external_docs.extensions_json);
    JSON_Object *ext_docs_obj = json_value_get_object(ext_docs_ext);
    ASSERT_STR_EQ("ext", json_object_get_string(ext_docs_obj, "x-ext"));
    json_value_free(ext_docs_ext);
  }

  {
    JSON_Value *tag_ext = json_parse_string(spec.tags[0].extensions_json);
    JSON_Object *tag_obj = json_value_get_object(tag_ext);
    ASSERT_STR_EQ("tag", json_object_get_string(tag_obj, "x-tag"));
    json_value_free(tag_ext);
  }

  {
    JSON_Value *sec_ext =
        json_parse_string(spec.security_schemes[0].extensions_json);
    JSON_Object *sec_obj = json_value_get_object(sec_ext);
    ASSERT_STR_EQ("sec", json_object_get_string(sec_obj, "x-sec"));
    json_value_free(sec_ext);
  }

  {
    JSON_Value *sec_req_ext =
        json_parse_string(spec.security[0].extensions_json);
    JSON_Object *sec_req_obj = json_value_get_object(sec_req_ext);
    ASSERT_EQ(1, spec.security_set);
    ASSERT_EQ(1, spec.n_security);
    ASSERT_EQ(1, spec.security[0].n_requirements);
    ASSERT_STR_EQ("api_key", spec.security[0].requirements[0].scheme);
    ASSERT_STR_EQ("req", json_object_get_string(sec_req_obj, "x-sec-req"));
    json_value_free(sec_req_ext);
  }

  {
    struct OpenAPI_Path *path = &spec.paths[0];
    struct OpenAPI_Operation *op = &path->operations[0];
    struct OpenAPI_Parameter *param = &op->parameters[0];
    struct OpenAPI_Response *resp = &op->responses[0];
    struct OpenAPI_Callback *cb = &op->callbacks[0];

    JSON_Value *path_ext = json_parse_string(path->extensions_json);
    JSON_Object *path_obj = json_value_get_object(path_ext);
    ASSERT_STR_EQ("path", json_object_get_string(path_obj, "x-path"));
    json_value_free(path_ext);

    JSON_Value *op_ext = json_parse_string(op->extensions_json);
    JSON_Object *op_obj = json_value_get_object(op_ext);
    ASSERT_EQ(2, (int)json_object_get_number(op_obj, "x-op"));
    json_value_free(op_ext);

    JSON_Value *rb_ext = json_parse_string(op->req_body_extensions_json);
    JSON_Object *rb_obj = json_value_get_object(rb_ext);
    ASSERT_EQ(1, json_object_get_boolean(json_object_get_object(rb_obj, "x-rb"),
                                         "note"));
    json_value_free(rb_ext);

    JSON_Value *param_ext = json_parse_string(param->extensions_json);
    JSON_Object *param_obj = json_value_get_object(param_ext);
    ASSERT_STR_EQ("param", json_object_get_string(param_obj, "x-param"));
    json_value_free(param_ext);

    JSON_Value *resp_ext = json_parse_string(resp->extensions_json);
    JSON_Object *resp_obj = json_value_get_object(resp_ext);
    ASSERT_EQ(1, json_object_get_boolean(
                     json_object_get_object(resp_obj, "x-resp"), "ok"));
    json_value_free(resp_ext);

    JSON_Value *resps_ext = json_parse_string(op->responses_extensions_json);
    JSON_Object *resps_obj = json_value_get_object(resps_ext);
    ASSERT_EQ(1,
              json_object_get_boolean(
                  json_object_get_object(resps_obj, "x-responses"), "trace"));
    json_value_free(resps_ext);

    JSON_Value *cb_ext = json_parse_string(cb->extensions_json);
    JSON_Object *cb_obj = json_value_get_object(cb_ext);
    ASSERT_STR_EQ("cb", json_object_get_string(cb_obj, "x-cb"));
    json_value_free(cb_ext);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_paths_webhooks_components_extensions(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"Test\",\"version\":\"1\"},"
      "\"paths\":{"
      "  \"x-paths\":{\"note\":true},"
      "  \"/pets\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\"}}}}"
      "},"
      "\"webhooks\":{"
      "  \"x-hooks\":{\"hook\":1},"
      "  "
      "\"event{type}\":{\"post\":{\"responses\":{\"200\":{\"description\":"
      "\"ok\"}}}}"
      "},"
      "\"components\":{"
      "  \"x-comps\":{\"meta\":\"yes\"},"
      "  \"schemas\":{\"Pet\":{\"type\":\"object\"}}"
      "}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_STR_EQ("/pets", spec.paths[0].route);

  ASSERT_EQ(1, spec.n_webhooks);
  ASSERT_STR_EQ("event{type}", spec.webhooks[0].route);

  {
    JSON_Value *paths_ext = json_parse_string(spec.paths_extensions_json);
    JSON_Object *paths_obj = json_value_get_object(paths_ext);
    ASSERT_EQ(1, json_object_get_boolean(
                     json_object_get_object(paths_obj, "x-paths"), "note"));
    json_value_free(paths_ext);
  }

  {
    JSON_Value *hooks_ext = json_parse_string(spec.webhooks_extensions_json);
    JSON_Object *hooks_obj = json_value_get_object(hooks_ext);
    ASSERT_EQ(1, (int)json_object_get_number(
                     json_object_get_object(hooks_obj, "x-hooks"), "hook"));
    json_value_free(hooks_ext);
  }

  {
    JSON_Value *comps_ext = json_parse_string(spec.components_extensions_json);
    JSON_Object *comps_obj = json_value_get_object(comps_ext);
    ASSERT_STR_EQ("yes",
                  json_object_get_string(
                      json_object_get_object(comps_obj, "x-comps"), "meta"));
    json_value_free(comps_ext);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_webhook_path_template_not_validated(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"Test\",\"version\":\"1\"},"
                     "\"webhooks\":{"
                     "  \"/events/{eventId}\":{"
                     "    \"post\":{"
                     "      \"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "    }"
                     "  }"
                     "}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_webhooks);
  ASSERT_STR_EQ("/events/{eventId}", spec.webhooks[0].route);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_component_schema_raw(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"components\":{\"schemas\":{"
      "\"Token\":{\"type\":\"string\"},"
      "\"Flag\":true,"
      "\"Nums\":{\"type\":\"array\",\"items\":{\"type\":\"integer\"}}"
      "}},"
      "\"paths\":{}"
      "}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, spec.n_defined_schemas);
  ASSERT_EQ(3, spec.n_raw_schemas);

  {
    int idx = find_raw_schema_index(&spec, "Token");
    JSON_Value *val;
    JSON_Object *obj;
    ASSERT(idx >= 0);
    val = json_parse_string(spec.raw_schema_json[idx]);
    obj = json_value_get_object(val);
    ASSERT_STR_EQ("string", json_object_get_string(obj, "type"));
    json_value_free(val);
  }

  {
    int idx = find_raw_schema_index(&spec, "Flag");
    JSON_Value *val;
    ASSERT(idx >= 0);
    val = json_parse_string(spec.raw_schema_json[idx]);
    ASSERT_EQ(JSONBoolean, json_value_get_type(val));
    ASSERT_EQ(1, json_value_get_boolean(val));
    json_value_free(val);
  }

  {
    int idx = find_raw_schema_index(&spec, "Nums");
    JSON_Value *val;
    JSON_Object *obj;
    JSON_Object *items;
    ASSERT(idx >= 0);
    val = json_parse_string(spec.raw_schema_json[idx]);
    obj = json_value_get_object(val);
    ASSERT_STR_EQ("array", json_object_get_string(obj, "type"));
    items = json_object_get_object(obj, "items");
    ASSERT_STR_EQ("integer", json_object_get_string(items, "type"));
    json_value_free(val);
  }

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_schema_external_ref(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
                     "\"paths\":{\"/pets\":{\"get\":{\"responses\":{\"200\":{"
                     "\"description\":\"ok\","
                     "\"content\":{\"application/json\":{\"schema\":{"
                     "\"$ref\":\"https://example.com/schemas/Pet\""
                     "}}}}}}}}"
                     "}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("https://example.com/schemas/Pet",
                spec.paths[0].operations[0].responses[0].schema.ref);
  ASSERT(spec.paths[0].operations[0].responses[0].schema.ref_name == NULL);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_schema_ref_with_pointer_is_not_component(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
                     "\"paths\":{\"/pets\":{\"get\":{\"responses\":{\"200\":{"
                     "\"description\":\"ok\","
                     "\"content\":{\"application/json\":{\"schema\":{"
                     "\"$ref\":\"#/components/schemas/Pet/properties/id\""
                     "}}}}}}}}"
                     "}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("#/components/schemas/Pet/properties/id",
                spec.paths[0].operations[0].responses[0].schema.ref);
  ASSERT(spec.paths[0].operations[0].responses[0].schema.ref_name == NULL);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_schema_external_items_ref(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
                     "\"paths\":{\"/pets\":{\"get\":{\"responses\":{\"200\":{"
                     "\"description\":\"ok\","
                     "\"content\":{\"application/json\":{\"schema\":{"
                     "\"type\":\"array\","
                     "\"items\":{\"$ref\":\"https://example.com/schemas/Pet\"}"
                     "}}}}}}}}"
                     "}";
  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].schema.is_array);
  ASSERT_STR_EQ("https://example.com/schemas/Pet",
                spec.paths[0].operations[0].responses[0].schema.items_ref);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_path_template_missing_param(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/pets/{petId}\":{\"get\":{"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}"
      "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_path_template_param_not_in_route(void) {
  const char *json =
      "{\"paths\":{\"/pets\":{\"parameters\":["
      "{\"name\":\"petId\",\"in\":\"path\",\"required\":true,"
      "\"schema\":{\"type\":\"string\"}}],"
      "\"get\":{\"responses\":{\"200\":{\"description\":\"OK\"}}}"
      "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_path_template_param_not_required(void) {
  const char *json =
      "{\"paths\":{\"/pets/{petId}\":{\"parameters\":["
      "{\"name\":\"petId\",\"in\":\"path\",\"required\":false,"
      "\"schema\":{\"type\":\"string\"}}],"
      "\"get\":{\"responses\":{\"200\":{\"description\":\"OK\"}}}"
      "}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_root_missing_paths_components_webhooks_rejected(void) {
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{"
                     "\"title\":\"Example API\",\"version\":\"1\"}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_param_style_invalid_for_in_rejected(void) {
  const char *json = "{"
                     "\"openapi\":\"3.2.0\","
                     "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
                     "\"paths\":{\"/pets\":{\"get\":{"
                     "\"parameters\":[{\"name\":\"id\",\"in\":\"query\","
                     "\"style\":\"matrix\",\"schema\":{\"type\":\"string\"}}],"
                     "\"responses\":{\"200\":{\"description\":\"ok\"}}"
                     "}}}"
                     "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_param_style_deep_object_scalar_rejected(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/pets\":{\"get\":{"
      "\"parameters\":[{\"name\":\"filter\",\"in\":\"query\","
      "\"style\":\"deepObject\",\"schema\":{\"type\":\"string\"}}],"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}"
      "}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_server_url_variable_missing_definition_rejected(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"servers\":[{\"url\":\"https://{env}.example.com\"}],"
      "\"paths\":{\"/pets\":{\"get\":{\"responses\":{\"200\":{\"description\":"
      "\"ok\"}}}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_server_url_variable_duplicate_rejected(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"servers\":[{\"url\":\"https://{env}.example.com/{env}\","
      "\"variables\":{\"env\":{\"default\":\"prod\"}}}],"
      "\"paths\":{\"/pets\":{\"get\":{\"responses\":{\"200\":{\"description\":"
      "\"ok\"}}}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_server_missing_url_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"servers\":[{\"description\":\"No URL\"}],"
      "\"paths\":{}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_additional_operations_standard_method_rejected(void) {
  const char *json =
      "{\"paths\":{\"/x\":{\"additionalOperations\":{"
      "\"POST\":{\"responses\":{\"200\":{\"description\":\"ok\"}}}"
      "}}}}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_link_missing_operation_ref_or_id_rejected(void) {
  const char *json =
      "{"
      "\"components\":{\"links\":{\"BadLink\":{\"parameters\":{\"id\":1}}}},"
      "\"paths\":{\"/"
      "x\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\"}}}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

TEST test_load_link_operation_ref_and_id_both_rejected(void) {
  const char *json =
      "{"
      "\"components\":{\"links\":{\"BadLink\":{"
      "\"operationId\":\"op\","
      "\"operationRef\":\"#/paths/~1x/get\""
      "}}},"
      "\"paths\":{\"/"
      "x\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\"}}}}}"
      "}";

  struct OpenAPI_Spec spec;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(EINVAL, rc);
  PASS();
}

SUITE(openapi_loader_suite) {
  /* RUN_TEST(test_load_parameter_array); */
  /* RUN_TEST(test_load_parameter_metadata); */
  /* RUN_TEST(test_load_allow_empty_value); */
  /* RUN_TEST(test_load_allow_empty_value_non_query_rejected); */
  /* RUN_TEST(test_load_querystring_parameter); */
  /* RUN_TEST(test_load_querystring_json_inline_promoted); */
  /* RUN_TEST(test_ignore_reserved_header_parameters); */
  /* RUN_TEST(test_ignore_content_type_response_header); */
  /* RUN_TEST(test_param_schema_and_content_conflict); */
  /* RUN_TEST(test_header_schema_and_content_conflict); */
  /* RUN_TEST(test_load_parameter_content_any); */
  /* RUN_TEST(test_load_parameter_content_media_type_encoding); */
  /* RUN_TEST(test_load_header_content_media_type); */
  /* RUN_TEST(test_load_parameter_schema_ref); */
  /* RUN_TEST(test_load_header_schema_ref); */
  /* RUN_TEST(test_load_path_level_parameters); */
  /* RUN_TEST(test_load_server_variables); */
  /* RUN_TEST(test_server_variable_default_required); */
  /* RUN_TEST(test_load_schema_parsing); */
  /* RUN_TEST(test_load_schema_external_docs_discriminator_xml); */
  /* RUN_TEST(test_load_form_content_type); */
  /* RUN_TEST(test_request_body_content_required); */
  /* RUN_TEST(test_load_request_body_item_schema_array); */
  /* RUN_TEST(test_param_content_multiple_entries_rejected); */
  /* RUN_TEST(test_header_content_multiple_entries_rejected); */
  /* RUN_TEST(test_response_description_required); */
  /* RUN_TEST(test_paths_require_leading_slash); */
  /* RUN_TEST(test_paths_ambiguous_templates_rejected); */
  /* RUN_TEST(test_component_key_regex_rejected); */
  /* RUN_TEST(test_tag_duplicate_rejected); */
  /* RUN_TEST(test_tag_name_required); */
  /* RUN_TEST(test_tag_parent_missing_rejected); */
  /* RUN_TEST(test_tag_parent_cycle_rejected); */
  /* RUN_TEST(test_external_docs_url_required); */
  /* RUN_TEST(test_operation_id_duplicate_rejected); */
  /* RUN_TEST(test_operation_id_duplicate_in_callback_rejected); */
  /* RUN_TEST(test_parameter_duplicates_rejected); */
  /* RUN_TEST(test_querystring_with_query_rejected); */
  /* RUN_TEST(test_querystring_duplicate_rejected); */
  /* RUN_TEST(test_querystring_path_and_operation_mixed_rejected); */
  /* RUN_TEST(test_querystring_with_query_in_callback_rejected); */
  /* RUN_TEST(test_parameter_missing_name_or_in_rejected); */
  /* RUN_TEST(test_header_style_non_simple_rejected); */
  /* RUN_TEST(test_media_type_encoding_conflict_rejected); */
  /* RUN_TEST(test_encoding_object_conflict_rejected); */
  /* RUN_TEST(test_load_operation_tags); */
  /* RUN_TEST(test_load_openapi_version_and_servers); */
  /* RUN_TEST(test_load_server_duplicate_name_rejected); */
  /* RUN_TEST(test_load_missing_openapi_and_swagger_rejected); */
  /* RUN_TEST(test_load_schema_root_document_with_id); */
  /* RUN_TEST(test_load_schema_root_boolean); */
  /* RUN_TEST(test_load_swagger_root_allowed); */
  /* RUN_TEST(test_load_openapi_version_unsupported_rejected); */
  /* RUN_TEST(test_load_server_url_query_rejected); */
  /* RUN_TEST(test_load_security_requirements); */
  /* RUN_TEST(test_load_security_schemes); */
  /* RUN_TEST(test_load_security_scheme_deprecated); */
  /* RUN_TEST(test_load_oauth2_flows); */
  /* RUN_TEST(test_load_security_scheme_http_missing_scheme_rejected); */
  /* RUN_TEST(test_load_security_scheme_apikey_missing_name_rejected); */
  /* RUN_TEST(test_load_security_scheme_apikey_missing_in_rejected); */
  /* RUN_TEST(test_load_security_scheme_openid_missing_url_rejected); */
  /* RUN_TEST(test_load_oauth2_missing_flows_rejected); */
  /* RUN_TEST(test_load_oauth2_flow_missing_scopes_rejected); */
  /* RUN_TEST(test_load_oauth2_flow_missing_required_urls_rejected); */
  /* RUN_TEST(test_load_oauth2_flow_unknown_rejected); */
  /* RUN_TEST(test_load_parameter_examples_object); */
  /* RUN_TEST(test_load_parameter_examples_media); */
  /* RUN_TEST(test_load_parameter_example_and_examples_rejected); */
  /* RUN_TEST(test_load_header_example_and_examples_rejected); */
  /* RUN_TEST(test_load_media_example_and_examples_rejected); */
  /* RUN_TEST(test_load_example_data_value_and_value_rejected); */
  /* RUN_TEST(test_load_example_serialized_and_external_rejected); */
  /* RUN_TEST(test_load_response_examples_media); */
  /* RUN_TEST(test_load_component_examples); */
  /* RUN_TEST(test_load_example_component_ref_strict); */
  /* RUN_TEST(test_load_request_body_metadata_and_response_description); */
  /* RUN_TEST(test_load_request_body_component_ref); */
  /* RUN_TEST(test_load_response_multiple_content); */
  /* RUN_TEST(test_load_request_body_multiple_content_with_ref); */
  /* RUN_TEST(test_load_media_type_encoding); */
  /* RUN_TEST(test_load_media_type_prefix_item_encoding); */
  /* RUN_TEST(test_load_info_metadata); */
  /* RUN_TEST(test_load_info_missing_title_rejected); */
  /* RUN_TEST(test_load_info_missing_version_rejected); */
  /* RUN_TEST(test_load_license_identifier_and_url_rejected); */
  /* RUN_TEST(test_load_license_missing_name_rejected); */
  /* RUN_TEST(test_load_operation_metadata); */
  /* RUN_TEST(test_load_root_metadata_and_tags); */
  /* RUN_TEST(test_self_qualified_component_refs); */
  /* RUN_TEST(test_relative_self_component_refs); */
  /* RUN_TEST(test_schema_id_ref_resolution); */
  /* RUN_TEST(test_schema_anchor_ref_resolution); */
  /* RUN_TEST(test_schema_dynamic_ref_resolution); */
  /* RUN_TEST(test_external_component_ref_registry_absolute); */
  /* RUN_TEST(test_external_component_ref_registry_relative); */
  /* RUN_TEST(test_load_response_content_type); */
  /* RUN_TEST(test_load_response_content_type_specificity); */
  /* RUN_TEST(test_load_response_content_type_params_json); */
  /* RUN_TEST(test_load_inline_response_schema_primitive); */
  /* RUN_TEST(test_load_inline_response_schema_array); */
  /* RUN_TEST(test_load_inline_schema_format_and_content); */
  /* RUN_TEST(test_load_inline_schema_array_item_format_and_content); */
  /* RUN_TEST(test_load_inline_schema_const_examples_annotations); */
  /* RUN_TEST(test_load_schema_ref_summary_description); */
  /* RUN_TEST(test_load_parameter_schema_format_and_content); */
  /* RUN_TEST(test_load_inline_schema_enum_default_nullable); */
  /* RUN_TEST(test_load_inline_schema_type_union); */
  /* RUN_TEST(test_load_inline_schema_array_items_enum_nullable); */
  /* RUN_TEST(test_load_inline_schema_items_type_union); */
  /* RUN_TEST(test_load_inline_schema_example_and_numeric_constraints); */
  /* RUN_TEST(test_load_inline_schema_array_constraints_and_items_example); */
  /* RUN_TEST(test_load_inline_schema_items_const_default_and_extras); */
  /* RUN_TEST(test_load_inline_request_body_object_promoted); */
  /* RUN_TEST(test_load_inline_response_schema_object_item_promoted); */
  /* RUN_TEST(test_load_inline_response_item_schema_object_promoted); */
  /* RUN_TEST(test_load_request_body_ref_description_override); */
  /* RUN_TEST(test_load_options_trace_verbs); */
  /* RUN_TEST(test_load_query_verb_and_external_docs); */
  /* RUN_TEST(test_load_path_and_operation_servers); */
  /* RUN_TEST(test_load_webhooks); */
  /* RUN_TEST(test_load_path_ref); */
  /* RUN_TEST(test_load_component_parameter_ref); */
  /* RUN_TEST(test_load_component_response_and_headers); */
  /* RUN_TEST(test_load_additional_operations); */
  /* RUN_TEST(test_load_component_media_type_ref); */
  /* RUN_TEST(test_load_component_path_items); */
  /* RUN_TEST(test_load_response_links_and_component_links); */
  /* RUN_TEST(test_load_callbacks_and_component_callbacks); */
  /* RUN_TEST(test_load_path_item_ref_resolves_component); */
  /* RUN_TEST(test_load_callback_ref_resolves_component); */
  /* RUN_TEST(test_load_extensions_non_schema); */
  /* RUN_TEST(test_load_paths_webhooks_components_extensions); */
  /* RUN_TEST(test_webhook_path_template_not_validated); */
  /* RUN_TEST(test_load_component_schema_raw); */
  /* RUN_TEST(test_load_schema_external_ref); */
  /* RUN_TEST(test_load_schema_external_items_ref); */
  /* RUN_TEST(test_load_path_template_missing_param); */
  /* RUN_TEST(test_load_path_template_param_not_in_route); */
  /* RUN_TEST(test_load_path_template_param_not_required); */
  /* RUN_TEST(test_load_root_missing_paths_components_webhooks_rejected); */
  /* RUN_TEST(test_param_style_invalid_for_in_rejected); */
  /* RUN_TEST(test_param_style_deep_object_scalar_rejected); */
  /* RUN_TEST(test_server_url_variable_missing_definition_rejected); */
  /* RUN_TEST(test_server_url_variable_duplicate_rejected); */
  /* RUN_TEST(test_load_server_missing_url_rejected); */
  /* RUN_TEST(test_load_additional_operations_standard_method_rejected); */
  /* RUN_TEST(test_load_link_missing_operation_ref_or_id_rejected); */
  /* RUN_TEST(test_load_link_operation_ref_and_id_both_rejected); */
}

#endif /* TEST_OPENAPI_LOADER_H */
