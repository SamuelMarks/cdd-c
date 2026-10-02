/**
 * @file test_openapi_loader_refs.h
 * @brief Schema reference resolution, registries, and callback tests.
 */

#ifndef TEST_OPENAPI_LOADER_REFS_H
#define TEST_OPENAPI_LOADER_REFS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

TEST test_schema_id_ref_resolution(void) {
  struct StructFields *_ast_openapi_spec_find_schema_for_ref_9;

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  73,  68,  34,  44,  34,  118, 101, 114, 115,
      105, 111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  99,  111, 109, 112,
      111, 110, 101, 110, 116, 115, 34,  58,  123, 34,  115, 99,  104, 101, 109,
      97,  115, 34,  58,  123, 34,  70,  111, 111, 34,  58,  123, 34,  36,  105,
      100, 34,  58,  34,  104, 116, 116, 112, 115, 58,  47,  47,  101, 120, 97,
      109, 112, 108, 101, 46,  99,  111, 109, 47,  115, 99,  104, 101, 109, 97,
      115, 47,  102, 111, 111, 34,  44,  34,  116, 121, 112, 101, 34,  58,  34,
      111, 98,  106, 101, 99,  116, 34,  44,  34,  112, 114, 111, 112, 101, 114,
      116, 105, 101, 115, 34,  58,  123, 34,  105, 100, 34,  58,  123, 34,  116,
      121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125,
      125, 125, 125, 44,  34,  112, 97,  116, 104, 115, 34,  58,  123, 34,  47,
      102, 111, 111, 34,  58,  123, 34,  103, 101, 116, 34,  58,  123, 34,  111,
      112, 101, 114, 97,  116, 105, 111, 110, 73,  100, 34,  58,  34,  103, 101,
      116, 70,  111, 111, 34,  44,  34,  112, 97,  114, 97,  109, 101, 116, 101,
      114, 115, 34,  58,  91,  123, 34,  110, 97,  109, 101, 34,  58,  34,  102,
      34,  44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,
      34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  36,  114, 101, 102,
      34,  58,  34,  104, 116, 116, 112, 115, 58,  47,  47,  101, 120, 97,  109,
      112, 108, 101, 46,  99,  111, 109, 47,  115, 99,  104, 101, 109, 97,  115,
      47,  102, 111, 111, 34,  125, 125, 93,  44,  34,  114, 101, 115, 112, 111,
      110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,
      100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  111,
      107, 34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    const struct StructFields *sf =
        (openapi_spec_find_schema_for_ref(
             &spec, &p->schema, &_ast_openapi_spec_find_schema_for_ref_9),
         _ast_openapi_spec_find_schema_for_ref_9);
    ASSERT(sf != NULL);
    ASSERT(spec.defined_schema_ids != NULL);
    ASSERT_STR_EQ("https://example.com/schemas/foo",
                  spec.defined_schema_ids[0]);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_schema_anchor_ref_resolution(void) {
  struct StructFields *_ast_openapi_spec_find_schema_for_ref_10;

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  65,  110, 99,  104, 111, 114, 34,  44,  34,
      118, 101, 114, 115, 105, 111, 110, 34,  58,  34,  49,  34,  125, 44,  34,
      99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  123, 34,  115,
      99,  104, 101, 109, 97,  115, 34,  58,  123, 34,  70,  111, 111, 34,  58,
      123, 34,  36,  97,  110, 99,  104, 111, 114, 34,  58,  34,  70,  111, 111,
      65,  110, 99,  104, 111, 114, 34,  44,  34,  116, 121, 112, 101, 34,  58,
      34,  111, 98,  106, 101, 99,  116, 34,  44,  34,  112, 114, 111, 112, 101,
      114, 116, 105, 101, 115, 34,  58,  123, 34,  105, 100, 34,  58,  123, 34,
      116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125,
      125, 125, 125, 125, 44,  34,  112, 97,  116, 104, 115, 34,  58,  123, 34,
      47,  102, 111, 111, 34,  58,  123, 34,  103, 101, 116, 34,  58,  123, 34,
      111, 112, 101, 114, 97,  116, 105, 111, 110, 73,  100, 34,  58,  34,  103,
      101, 116, 70,  111, 111, 34,  44,  34,  112, 97,  114, 97,  109, 101, 116,
      101, 114, 115, 34,  58,  91,  123, 34,  110, 97,  109, 101, 34,  58,  34,
      102, 34,  44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,
      44,  34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  36,  114, 101,
      102, 34,  58,  34,  35,  70,  111, 111, 65,  110, 99,  104, 111, 114, 34,
      125, 125, 93,  44,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,
      58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114,
      105, 112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  125, 125, 125,
      125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    const struct StructFields *sf =
        (openapi_spec_find_schema_for_ref(
             &spec, &p->schema, &_ast_openapi_spec_find_schema_for_ref_10),
         _ast_openapi_spec_find_schema_for_ref_10);
    ASSERT(sf != NULL);
    ASSERT(spec.defined_schema_anchors != NULL);
    ASSERT_STR_EQ("FooAnchor", spec.defined_schema_anchors[0]);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_schema_dynamic_ref_resolution(void) {
  struct StructFields *_ast_openapi_spec_find_schema_for_ref_11;

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  68,  121, 110, 97,  109, 105, 99,  34,  44,
      34,  118, 101, 114, 115, 105, 111, 110, 34,  58,  34,  49,  34,  125, 44,
      34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  123, 34,
      115, 99,  104, 101, 109, 97,  115, 34,  58,  123, 34,  70,  111, 111, 34,
      58,  123, 34,  36,  100, 121, 110, 97,  109, 105, 99,  65,  110, 99,  104,
      111, 114, 34,  58,  34,  70,  111, 111, 68,  121, 110, 34,  44,  34,  116,
      121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116, 34,  44,  34,
      112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,  105,
      100, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114,
      105, 110, 103, 34,  125, 125, 125, 125, 125, 44,  34,  112, 97,  116, 104,
      115, 34,  58,  123, 34,  47,  102, 111, 111, 34,  58,  123, 34,  103, 101,
      116, 34,  58,  123, 34,  111, 112, 101, 114, 97,  116, 105, 111, 110, 73,
      100, 34,  58,  34,  103, 101, 116, 70,  111, 111, 34,  44,  34,  112, 97,
      114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,  123, 34,  110, 97,
      109, 101, 34,  58,  34,  102, 34,  44,  34,  105, 110, 34,  58,  34,  113,
      117, 101, 114, 121, 34,  44,  34,  115, 99,  104, 101, 109, 97,  34,  58,
      123, 34,  36,  100, 121, 110, 97,  109, 105, 99,  82,  101, 102, 34,  58,
      34,  35,  70,  111, 111, 68,  121, 110, 34,  125, 125, 93,  44,  34,  114,
      101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,
      34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110,
      34,  58,  34,  111, 107, 34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    const struct StructFields *sf =
        (openapi_spec_find_schema_for_ref(
             &spec, &p->schema, &_ast_openapi_spec_find_schema_for_ref_11),
         _ast_openapi_spec_find_schema_for_ref_11);
    ASSERT(sf != NULL);
    ASSERT(spec.defined_schema_dynamic_anchors != NULL);
    ASSERT_STR_EQ("FooDyn", spec.defined_schema_dynamic_anchors[0]);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_external_component_ref_registry_absolute(void) {

  const char shared[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  36,  115, 101, 108, 102, 34,  58,  34,  104, 116,
      116, 112, 115, 58,  47,  47,  101, 120, 97,  109, 112, 108, 101, 46,  99,
      111, 109, 47,  115, 104, 97,  114, 101, 100, 46,  106, 115, 111, 110, 34,
      44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105, 116, 108, 101,
      34,  58,  34,  83,  104, 97,  114, 101, 100, 34,  44,  34,  118, 101, 114,
      115, 105, 111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  99,  111, 109,
      112, 111, 110, 101, 110, 116, 115, 34,  58,  123, 34,  112, 97,  114, 97,
      109, 101, 116, 101, 114, 115, 34,  58,  123, 34,  80,  101, 116, 80,  97,
      114, 97,  109, 34,  58,  123, 34,  110, 97,  109, 101, 34,  58,  34,  112,
      101, 116, 34,  44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121,
      34,  44,  34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121,
      112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125,
      125, 125, 0};
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
  int rc = 0;
  rc += 0;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_doc_registry_init(&registry));

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
  g_fail_io_after = -1;
  PASS();
}

TEST test_external_component_ref_registry_relative(void) {

  const char shared[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  83,  104, 97,  114, 101, 100, 34,  44,  34,
      118, 101, 114, 115, 105, 111, 110, 34,  58,  34,  49,  34,  125, 44,  34,
      99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  123, 34,  112,
      97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  123, 34,  80,  101,
      116, 80,  97,  114, 97,  109, 34,  58,  123, 34,  110, 97,  109, 101, 34,
      58,  34,  112, 101, 116, 34,  44,  34,  105, 110, 34,  58,  34,  113, 117,
      101, 114, 121, 34,  44,  34,  115, 99,  104, 101, 109, 97,  34,  58,  123,
      34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,
      125, 125, 125, 125, 125, 0};
  const char root[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  82,  111, 111, 116, 34,  44,  34,  118, 101,
      114, 115, 105, 111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,
      116, 104, 115, 34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123,
      34,  103, 101, 116, 34,  58,  123, 34,  112, 97,  114, 97,  109, 101, 116,
      101, 114, 115, 34,  58,  91,  123, 34,  36,  114, 101, 102, 34,  58,  34,
      115, 104, 97,  114, 101, 100, 46,  106, 115, 111, 110, 35,  47,  99,  111,
      109, 112, 111, 110, 101, 110, 116, 115, 47,  112, 97,  114, 97,  109, 101,
      116, 101, 114, 115, 47,  80,  101, 116, 80,  97,  114, 97,  109, 34,  125,
      93,  44,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123,
      34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112,
      116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  125, 125, 125, 125, 125,
      125, 0};

  struct OpenAPI_DocRegistry registry;
  struct OpenAPI_Spec shared_spec;
  struct OpenAPI_Spec root_spec;
  int rc = 0;
  rc += 0;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_doc_registry_init(&registry));

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
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_query_verb_and_external_docs(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "search\":{\"query\":{\"operationId\":\"querySearch\",\"externalDocs\":{"
      "\"description\":\"Op "
      "docs\",\"url\":\"https://example.com/"
      "op\"},\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.paths[0].n_operations);
  ASSERT_EQ(OA_VERB_QUERY, spec.paths[0].operations[0].verb);
  ASSERT_STR_EQ("https://example.com/op",
                spec.paths[0].operations[0].external_docs.url);
  ASSERT_STR_EQ("Op docs",
                spec.paths[0].operations[0].external_docs.description);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_path_and_operation_servers(void) {

  const char *json =
      "{\"paths\":{\"/pets\":{\"servers\":[{\"url\":\"https://"
      "path.example.com\"}],\"get\":{\"operationId\":\"listPets\",\"servers\":["
      "{\"url\":\"https://"
      "op.example.com\",\"description\":\"Op\"}],\"responses\":{\"200\":{"
      "\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_webhooks(void) {

  const char *json = "{\"webhooks\":{\"petEvent\":{\"post\":{\"operationId\":"
                     "\"onPetEvent\",\"responses\":{\"200\":{\"description\":"
                     "\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_webhooks);
  ASSERT_STR_EQ("petEvent", spec.webhooks[0].route);
  ASSERT_EQ(1, spec.webhooks[0].n_operations);
  ASSERT_EQ(OA_VERB_POST, spec.webhooks[0].operations[0].verb);
  ASSERT_STR_EQ("onPetEvent", spec.webhooks[0].operations[0].operation_id);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_path_ref(void) {

  const char *json = "{\"paths\":{\"/foo\":{\"$ref\":\"#/components/pathItems/"
                     "Foo\"}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_STR_EQ("/foo", spec.paths[0].route);
  ASSERT_STR_EQ("#/components/pathItems/Foo", spec.paths[0].ref);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_component_parameter_ref(void) {

  const char *json =
      "{\"components\":{\"parameters\":{\"LimitParam\":{\"name\":\"limit\","
      "\"in\":\"query\",\"schema\":{\"type\":\"integer\"}}}},\"paths\":{\"/"
      "items\":{\"get\":{\"parameters\":[{\"$ref\":\"#/components/parameters/"
      "LimitParam\"}],\"responses\":{\"200\":{\"description\":\"OK\"}}}}},"
      "\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_component_response_and_headers(void) {

  const char json[] = {
      123, 34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  123,
      34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  78,
      111, 116, 70,  111, 117, 110, 100, 34,  58,  123, 34,  100, 101, 115, 99,
      114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  109, 105, 115, 115, 105,
      110, 103, 34,  44,  34,  104, 101, 97,  100, 101, 114, 115, 34,  58,  123,
      34,  88,  45,  84,  114, 97,  99,  101, 34,  58,  123, 34,  115, 99,  104,
      101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115,
      116, 114, 105, 110, 103, 34,  125, 125, 125, 125, 125, 44,  34,  104, 101,
      97,  100, 101, 114, 115, 34,  58,  123, 34,  82,  97,  116, 101, 76,  105,
      109, 105, 116, 34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  108, 105, 109, 105, 116, 34,  44,  34,  115,
      99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,
      34,  105, 110, 116, 101, 103, 101, 114, 34,  125, 125, 125, 125, 44,  34,
      112, 97,  116, 104, 115, 34,  58,  123, 34,  47,  120, 34,  58,  123, 34,
      103, 101, 116, 34,  58,  123, 34,  114, 101, 115, 112, 111, 110, 115, 101,
      115, 34,  58,  123, 34,  52,  48,  52,  34,  58,  123, 34,  36,  114, 101,
      102, 34,  58,  34,  35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116,
      115, 47,  114, 101, 115, 112, 111, 110, 115, 101, 115, 47,  78,  111, 116,
      70,  111, 117, 110, 100, 34,  125, 44,  34,  50,  48,  48,  34,  58,  123,
      34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,
      111, 107, 34,  44,  34,  104, 101, 97,  100, 101, 114, 115, 34,  58,  123,
      34,  88,  45,  82,  97,  116, 101, 34,  58,  123, 34,  36,  114, 101, 102,
      34,  58,  34,  35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115,
      47,  104, 101, 97,  100, 101, 114, 115, 47,  82,  97,  116, 101, 76,  105,
      109, 105, 116, 34,  125, 125, 125, 125, 125, 125, 125, 44,  34,  111, 112,
      101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,  46,  48,  34,  125,
      0};

  struct OpenAPI_Spec spec = {0};
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_additional_operations(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/copy\":{\"additionalOperations\":{"
      "\"COPY\":{\"operationId\":\"copyItem\","
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}"
      "}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].n_additional_operations);
  ASSERT_STR_EQ("COPY", spec.paths[0].additional_operations[0].method);
  ASSERT_EQ(1, spec.paths[0].additional_operations[0].is_additional);
  ASSERT_STR_EQ("copyItem",
                spec.paths[0].additional_operations[0].operation_id);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_component_media_type_ref(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"components\":{\"schemas\":{\"P\":{\"type\":\"object\"}},"
      "\"mediaTypes\":{\"M1\":{\"schema\":{\"$ref\":\"#/components/schemas/"
      "P\"},"
      "\"prefixEncoding\":[{\"style\":\"form\"}],"
      "\"itemEncoding\":{\"style\":\"form\"}},"
      "\"M2\":{\"schema\":{\"$ref\":\"#/components/schemas/P\"},"
      "\"encoding\":{\"f\":{\"style\":\"form\"}}}}},"
      "\"paths\":{\"/"
      "p\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\","
      "\"content\":{\"a/1\":{\"$ref\":\"#/components/mediaTypes/M1\"},"
      "\"a/2\":{\"$ref\":\"#/components/mediaTypes/M2\"}}}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(2, spec.n_component_media_types);
  ASSERT_STR_EQ("M1", spec.component_media_type_names[0]);
  ASSERT_STR_EQ("M2", spec.component_media_type_names[1]);

  {
    struct OpenAPI_Response *resp = &spec.paths[0].operations[0].responses[0];
    ASSERT_STR_EQ("#/components/mediaTypes/M1", resp->content_ref);
    ASSERT_STR_EQ("a/1", resp->content_type);
    ASSERT_STR_EQ("P", resp->schema.ref_name);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_schema_conditional_keywords(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "test\":{\"get\":{\"parameters\":[{\"name\":\"p\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\",\"not\":{\"type\":\"integer\"},"
      "\"if\":{\"maxLength\":10},\"then\":{\"minLength\":2},\"else\":{"
      "\"pattern\":\"^[a-z]+$\"}}}"
      "],\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].operations[0].n_parameters);
  ASSERT(spec.paths[0].operations[0].parameters[0].schema.not_schema != NULL);
  ASSERT(spec.paths[0].operations[0].parameters[0].schema.if_schema != NULL);
  ASSERT(spec.paths[0].operations[0].parameters[0].schema.then_schema != NULL);
  ASSERT(spec.paths[0].operations[0].parameters[0].schema.else_schema != NULL);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_path_item_ref_with_operation_security(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"components\":{\"securitySchemes\":{\"myKey\":{\"type\":\"apiKey\","
      "\"name\":\"k\",\"in\":\"header\"}},"
      "\"pathItems\":{\"MyPath\":{\"get\":{\"security\":[{\"myKey\":[]}],"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}}}},"
      "\"paths\":{\"/test\":{\"$ref\":\"#/components/pathItems/MyPath\"}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_EQ(1, spec.paths[0].n_operations);
  ASSERT_EQ(1, spec.paths[0].operations[0].n_security);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_component_path_items(void) {

  const char *json =
      "{\"components\":{\"pathItems\":{\"FooItem\":{\"summary\":\"foo\","
      "\"get\":{\"operationId\":\"getFoo\",\"responses\":{\"200\":{"
      "\"description\":\"ok\"}}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_component_path_items);
  ASSERT_STR_EQ("FooItem", spec.component_path_item_names[0]);
  ASSERT_EQ(1, spec.component_path_items[0].n_operations);
  ASSERT_STR_EQ("getFoo",
                spec.component_path_items[0].operations[0].operation_id);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_response_links_and_component_links(void) {

  const char json[] = {
      123, 34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  123,
      34,  108, 105, 110, 107, 115, 34,  58,  123, 34,  78,  101, 120, 116, 80,
      97,  103, 101, 34,  58,  123, 34,  111, 112, 101, 114, 97,  116, 105, 111,
      110, 73,  100, 34,  58,  34,  108, 105, 115, 116, 80,  101, 116, 115, 34,
      44,  34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  123,
      34,  108, 105, 109, 105, 116, 34,  58,  53,  48,  44,  34,  111, 102, 102,
      115, 101, 116, 34,  58,  34,  36,  114, 101, 115, 112, 111, 110, 115, 101,
      46,  98,  111, 100, 121, 35,  47,  111, 102, 102, 115, 101, 116, 34,  125,
      44,  34,  115, 101, 114, 118, 101, 114, 34,  58,  123, 34,  117, 114, 108,
      34,  58,  34,  104, 116, 116, 112, 115, 58,  47,  47,  97,  112, 105, 46,
      101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109, 34,  125, 125, 125,
      125, 44,  34,  112, 97,  116, 104, 115, 34,  58,  123, 34,  47,  112, 101,
      116, 115, 34,  58,  123, 34,  103, 101, 116, 34,  58,  123, 34,  114, 101,
      115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,
      58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,
      58,  34,  111, 107, 34,  44,  34,  108, 105, 110, 107, 115, 34,  58,  123,
      34,  110, 101, 120, 116, 34,  58,  123, 34,  36,  114, 101, 102, 34,  58,
      34,  35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  108,
      105, 110, 107, 115, 47,  78,  101, 120, 116, 80,  97,  103, 101, 34,  125,
      125, 125, 125, 125, 125, 125, 44,  34,  111, 112, 101, 110, 97,  112, 105,
      34,  58,  34,  51,  46,  50,  46,  48,  34,  125, 0};

  struct OpenAPI_Spec spec = {0};
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_callbacks_and_component_callbacks(void) {

  const char json[] = {
      123, 34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  123,
      34,  99,  97,  108, 108, 98,  97,  99,  107, 115, 34,  58,  123, 34,  79,
      110, 69,  118, 101, 110, 116, 34,  58,  123, 34,  123, 36,  114, 101, 113,
      117, 101, 115, 116, 46,  98,  111, 100, 121, 35,  47,  117, 114, 108, 125,
      34,  58,  123, 34,  112, 111, 115, 116, 34,  58,  123, 34,  114, 101, 115,
      112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,
      123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,
      34,  111, 107, 34,  125, 125, 125, 125, 125, 125, 125, 44,  34,  112, 97,
      116, 104, 115, 34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123,
      34,  103, 101, 116, 34,  58,  123, 34,  114, 101, 115, 112, 111, 110, 115,
      101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101,
      115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,
      125, 125, 44,  34,  99,  97,  108, 108, 98,  97,  99,  107, 115, 34,  58,
      123, 34,  111, 110, 69,  118, 101, 110, 116, 34,  58,  123, 34,  123, 36,
      114, 101, 113, 117, 101, 115, 116, 46,  98,  111, 100, 121, 35,  47,  117,
      114, 108, 125, 34,  58,  123, 34,  112, 111, 115, 116, 34,  58,  123, 34,
      114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,
      48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111,
      110, 34,  58,  34,  111, 107, 34,  125, 125, 125, 125, 125, 125, 125, 125,
      125, 44,  34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,
      50,  46,  48,  34,  125, 0};

  struct OpenAPI_Spec spec = {0};
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_path_item_ref_resolves_component(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  84,  34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  99,  111, 109, 112, 111,
      110, 101, 110, 116, 115, 34,  58,  123, 32,  32,  34,  112, 97,  116, 104,
      73,  116, 101, 109, 115, 34,  58,  123, 32,  32,  32,  32,  34,  80,  101,
      116, 115, 34,  58,  123, 32,  32,  32,  32,  32,  32,  34,  103, 101, 116,
      34,  58,  123, 32,  32,  32,  32,  32,  32,  32,  32,  34,  111, 112, 101,
      114, 97,  116, 105, 111, 110, 73,  100, 34,  58,  34,  108, 105, 115, 116,
      80,  101, 116, 115, 34,  44,  32,  32,  32,  32,  32,  32,  32,  32,  34,
      114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,
      48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111,
      110, 34,  58,  34,  111, 107, 34,  125, 125, 32,  32,  32,  32,  32,  32,
      125, 32,  32,  32,  32,  125, 32,  32,  125, 125, 44,  34,  112, 97,  116,
      104, 115, 34,  58,  123, 32,  32,  34,  47,  112, 101, 116, 115, 34,  58,
      123, 32,  32,  32,  32,  34,  36,  114, 101, 102, 34,  58,  34,  35,  47,
      99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  112, 97,  116, 104,
      73,  116, 101, 109, 115, 47,  80,  101, 116, 115, 34,  44,  32,  32,  32,
      32,  34,  115, 117, 109, 109, 97,  114, 121, 34,  58,  34,  80,  101, 116,
      115, 34,  32,  32,  125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_callback_ref_resolves_component(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  84,  34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  99,  111, 109, 112, 111,
      110, 101, 110, 116, 115, 34,  58,  123, 32,  32,  34,  99,  97,  108, 108,
      98,  97,  99,  107, 115, 34,  58,  123, 32,  32,  32,  32,  34,  78,  111,
      116, 105, 102, 121, 34,  58,  123, 32,  32,  32,  32,  32,  32,  34,  123,
      36,  114, 101, 113, 117, 101, 115, 116, 46,  98,  111, 100, 121, 35,  47,
      117, 114, 108, 125, 34,  58,  123, 32,  32,  32,  32,  32,  32,  32,  32,
      34,  112, 111, 115, 116, 34,  58,  123, 32,  32,  32,  32,  32,  32,  32,
      32,  32,  32,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,
      123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105,
      112, 116, 105, 111, 110, 34,  58,  34,  111, 107, 34,  125, 125, 32,  32,
      32,  32,  32,  32,  32,  32,  125, 32,  32,  32,  32,  32,  32,  125, 32,
      32,  32,  32,  125, 32,  32,  125, 125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 32,  32,  34,  47,  112, 101, 116, 115, 34,  58,  123, 32,
      32,  32,  32,  34,  103, 101, 116, 34,  58,  123, 32,  32,  32,  32,  32,
      32,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,
      50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  111, 107, 34,  125, 125, 44,  32,  32,  32,
      32,  32,  32,  34,  99,  97,  108, 108, 98,  97,  99,  107, 115, 34,  58,
      123, 32,  32,  32,  32,  32,  32,  32,  32,  34,  111, 110, 78,  111, 116,
      105, 102, 121, 34,  58,  123, 32,  32,  32,  32,  32,  32,  32,  32,  32,
      32,  34,  36,  114, 101, 102, 34,  58,  34,  35,  47,  99,  111, 109, 112,
      111, 110, 101, 110, 116, 115, 47,  99,  97,  108, 108, 98,  97,  99,  107,
      115, 47,  78,  111, 116, 105, 102, 121, 34,  44,  32,  32,  32,  32,  32,
      32,  32,  32,  32,  32,  34,  115, 117, 109, 109, 97,  114, 121, 34,  58,
      34,  79,  118, 101, 114, 114, 105, 100, 101, 34,  32,  32,  32,  32,  32,
      32,  32,  32,  125, 32,  32,  32,  32,  32,  32,  125, 32,  32,  32,  32,
      125, 32,  32,  125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
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
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_REFS_H */
