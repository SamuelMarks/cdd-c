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

TEST test_load_operation_tags(void) {
  const char *json = "{\"paths\":{\"/tagged\":{\"get\":{"
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
      "{\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
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
      "{\"paths\":{\"/q\":{\"get\":{\"parameters\":[{"
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

TEST test_load_querystring_parameter(void) {
  const char *json =
      "{\"paths\":{\"/search\":{\"get\":{\"parameters\":[{"
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

TEST test_load_parameter_content_any(void) {
  const char *json =
      "{\"paths\":{\"/h\":{\"get\":{\"parameters\":[{"
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

TEST test_load_path_level_parameters(void) {
  const char *json =
      "{\"paths\":{\"/pets\":{\"summary\":\"Pets\",\"description\":\"All pets\","
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
  const char *json =
      "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://{env}.example.com\","
      "\"variables\":{\"env\":{\"default\":\"prod\",\"enum\":[\"prod\",\"staging\"],"
      "\"description\":\"Environment\"}}}]}";

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

TEST test_load_openapi_version_and_servers(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://api.example.com\","
      "\"description\":\"Prod\",\"name\":\"prod\"}]}";

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
      "\"ApiKeyAuth\":{\"type\":\"apiKey\",\"in\":\"header\",\"name\":\"X-Api\"},"
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
      "\"name\":\"X-Api-Key\"}"
      "}}}";

  struct OpenAPI_Spec spec;
  const struct OpenAPI_SecurityScheme *bearer;
  const struct OpenAPI_SecurityScheme *apikey;
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(2, spec.n_security_schemes);

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

TEST test_load_info_metadata(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{"
      "\"title\":\"Example API\",\"summary\":\"Short\",\"description\":\"Long\","
      "\"termsOfService\":\"https://example.com/terms\","
      "\"version\":\"2.1.0\","
      "\"contact\":{\"name\":\"API Support\",\"url\":\"https://example.com\","
      "\"email\":\"support@example.com\"},"
      "\"license\":{\"name\":\"Apache 2.0\",\"identifier\":\"Apache-2.0\","
      "\"url\":\"https://www.apache.org/licenses/LICENSE-2.0.html\"}"
      "}}";

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
  ASSERT_STR_EQ("https://www.apache.org/licenses/LICENSE-2.0.html",
                spec.info.license.url);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_operation_metadata(void) {
  const char *json =
      "{\"paths\":{\"/meta\":{\"get\":{"
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
  ASSERT_STR_EQ("text/plain", spec.paths[0].operations[0].responses[0].content_type);
  ASSERT_STR_EQ("Message", spec.paths[0].operations[0].responses[0].schema.ref_name);

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
      "\"jsonSchemaDialect\":\"https://spec.openapis.org/oas/3.1/dialect/base\","
      "\"externalDocs\":{\"description\":\"Root docs\",\"url\":\"https://example.com/docs\"},"
      "\"tags\":[{"
      "\"name\":\"pets\",\"summary\":\"Pets\",\"description\":\"Pet ops\","
      "\"parent\":\"animals\",\"kind\":\"nav\","
      "\"externalDocs\":{\"description\":\"Tag docs\",\"url\":\"https://example.com/tags/pets\"}"
      "}]"
      "}";

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
  ASSERT_STR_EQ("https://example.com/tags/pets", spec.tags[0].external_docs.url);
  ASSERT_STR_EQ("Tag docs", spec.tags[0].external_docs.description);

  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_query_verb_and_external_docs(void) {
  const char *json =
      "{\"paths\":{\"/search\":{\"query\":{"
      "\"operationId\":\"querySearch\","
      "\"externalDocs\":{\"description\":\"Op docs\",\"url\":\"https://example.com/op\"}"
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
  const char *json =
      "{"
      "\"paths\":{"
      "  \"/pets\":{"
      "    \"servers\":[{\"url\":\"https://path.example.com\"}],"
      "    \"get\":{"
      "      \"operationId\":\"listPets\","
      "      \"servers\":[{\"url\":\"https://op.example.com\",\"description\":\"Op\"}],"
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
  const char *json =
      "{"
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
  const char *json =
      "{"
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

SUITE(openapi_loader_suite) {
  RUN_TEST(test_load_parameter_array);
  RUN_TEST(test_load_parameter_metadata);
  RUN_TEST(test_load_allow_empty_value);
  RUN_TEST(test_load_querystring_parameter);
  RUN_TEST(test_load_parameter_content_any);
  RUN_TEST(test_load_path_level_parameters);
  RUN_TEST(test_load_server_variables);
  RUN_TEST(test_load_schema_parsing);
  RUN_TEST(test_load_form_content_type);
  RUN_TEST(test_load_operation_tags);
  RUN_TEST(test_load_openapi_version_and_servers);
  RUN_TEST(test_load_security_requirements);
  RUN_TEST(test_load_security_schemes);
  RUN_TEST(test_load_request_body_metadata_and_response_description);
  RUN_TEST(test_load_info_metadata);
  RUN_TEST(test_load_operation_metadata);
  RUN_TEST(test_load_root_metadata_and_tags);
  RUN_TEST(test_load_response_content_type);
  RUN_TEST(test_load_inline_response_schema_primitive);
  RUN_TEST(test_load_inline_response_schema_array);
  RUN_TEST(test_load_options_trace_verbs);
  RUN_TEST(test_load_query_verb_and_external_docs);
  RUN_TEST(test_load_path_and_operation_servers);
  RUN_TEST(test_load_webhooks);
  RUN_TEST(test_load_path_ref);
}

#endif /* TEST_OPENAPI_LOADER_H */
