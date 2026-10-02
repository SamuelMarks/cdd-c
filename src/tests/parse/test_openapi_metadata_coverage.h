/**
 * @file test_openapi_metadata_coverage.h
 * @brief Comprehensive 100% test coverage for openapi_metadata.c.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_METADATA_COVERAGE_H
#define TEST_OPENAPI_METADATA_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
#include "openapi/parse/openapi_test_helpers.h"
#include "openapi/parse/openapi_internal_components.h"
/* clang-format on */

static void test_meta_free_external_docs(struct OpenAPI_ExternalDocs *d) {
  if (!d)
    return;
  if (d->description)
    free(d->description);
  if (d->url)
    free(d->url);
  if (d->extensions_json)
    free(d->extensions_json);
  memset(d, 0, sizeof(*d));
}

static void test_meta_free_discriminator(struct OpenAPI_Discriminator *d) {
  size_t i;
  if (!d)
    return;
  if (d->property_name)
    free(d->property_name);
  if (d->default_mapping)
    free(d->default_mapping);
  if (d->mapping) {
    for (i = 0; i < d->n_mapping; ++i) {
      if (d->mapping[i].value)
        free(d->mapping[i].value);
      if (d->mapping[i].schema)
        free(d->mapping[i].schema);
    }
    free(d->mapping);
  }
  if (d->extensions_json)
    free(d->extensions_json);
  memset(d, 0, sizeof(*d));
}

static void test_meta_free_xml(struct OpenAPI_Xml *x) {
  if (!x)
    return;
  if (x->name)
    free(x->name);
  if (x->namespace_uri)
    free(x->namespace_uri);
  if (x->prefix)
    free(x->prefix);
  if (x->extensions_json)
    free(x->extensions_json);
  memset(x, 0, sizeof(*x));
}

static void test_meta_free_server(struct OpenAPI_Server *s) {
  size_t v;
  if (!s)
    return;
  if (s->url)
    free(s->url);
  if (s->description)
    free(s->description);
  if (s->name)
    free(s->name);
  if (s->extensions_json)
    free(s->extensions_json);
  if (s->variables) {
    for (v = 0; v < s->n_variables; ++v) {
      struct OpenAPI_ServerVariable *var = &s->variables[v];
      if (var->name)
        free(var->name);
      if (var->default_value)
        free(var->default_value);
      if (var->description)
        free(var->description);
      if (var->extensions_json)
        free(var->extensions_json);
      if (var->enum_values) {
        size_t e;
        for (e = 0; e < var->n_enum_values; ++e) {
          if (var->enum_values[e])
            free(var->enum_values[e]);
        }
        free(var->enum_values);
      }
    }
    free(s->variables);
  }
  memset(s, 0, sizeof(*s));
}

extern C_CDD_EXPORT int g_cdd_strdup_fail;
extern C_CDD_EXPORT int g_cdd_fail_extensions_init;

/**
 * @brief Test parse_info branches and OOM scenarios.
 */
TEST test_openapi_metadata_parse_info_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* NULL root_obj and NULL out branches */
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_info(NULL, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_info(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  jv = json_parse_string("{\"not_info\": 123}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  rc = cdd_test_parse_info(jo, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  /* info missing */
  rc = cdd_test_parse_info(jo, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* parse_info with full fields */
  jv = json_parse_string(
      "{\n"
      "  \"info\": {\n"
      "    \"title\": \"My API\",\n"
      "    \"summary\": \"API Summary\",\n"
      "    \"description\": \"API Description\",\n"
      "    \"termsOfService\": \"http://example.com/terms\",\n"
      "    \"version\": \"1.0.0\",\n"
      "    \"x-ext\": \"custom\",\n"
      "    \"contact\": {\n"
      "      \"name\": \"Support\",\n"
      "      \"url\": \"http://example.com/contact\",\n"
      "      \"email\": \"support@example.com\",\n"
      "      \"x-contact\": 1\n"
      "    },\n"
      "    \"license\": {\n"
      "      \"name\": \"Apache 2.0\",\n"
      "      \"url\": \"http://example.com/license\",\n"
      "      \"x-lic\": 2\n"
      "    }\n"
      "  }\n"
      "}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);

  /* Test license with url */
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_info(jo, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("My API", spec.info.title);
  ASSERT_STR_EQ("API Summary", spec.info.summary);
  ASSERT_STR_EQ("API Description", spec.info.description);
  ASSERT_STR_EQ("http://example.com/terms", spec.info.terms_of_service);
  ASSERT_STR_EQ("1.0.0", spec.info.version);
  ASSERT_STR_EQ("Support", spec.info.contact.name);
  ASSERT_STR_EQ("http://example.com/contact", spec.info.contact.url);
  ASSERT_STR_EQ("support@example.com", spec.info.contact.email);
  ASSERT_STR_EQ("Apache 2.0", spec.info.license.name);
  ASSERT_STR_EQ("http://example.com/license", spec.info.license.url);
  openapi_spec_free(&spec);

  /* Test license with identifier instead of url */
  {
    JSON_Value *jv_lic =
        json_parse_string("{\"info\": {\"title\": \"T\", \"license\": "
                          "{\"name\": \"MIT\", \"identifier\": \"MIT\"}}}");
    JSON_Object *jo_lic = json_value_get_object(jv_lic);
    memset(&spec, 0, sizeof(spec));
    rc = cdd_test_parse_info(jo_lic, &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("MIT", spec.info.license.identifier);
    openapi_spec_free(&spec);
    json_value_free(jv_lic);
  }

  /* Test license validation errors: missing name, and both url + identifier */
  {
    JSON_Value *jv_err = json_parse_string("{\"info\": {\"license\": {}}}");
    JSON_Object *jo_err = json_value_get_object(jv_err);
    memset(&spec, 0, sizeof(spec));
    rc = cdd_test_parse_info(jo_err, &spec);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    json_value_free(jv_err);

    jv_err = json_parse_string(
        "{\"info\": {\"license\": {\"name\": \"MIT\", \"identifier\": \"MIT\", "
        "\"url\": \"http://example.com\"}}}");
    jo_err = json_value_get_object(jv_err);
    memset(&spec, 0, sizeof(spec));
    rc = cdd_test_parse_info(jo_err, &spec);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    json_value_free(jv_err);
  }

  /* Test OOM on each strdup in parse_info */
  {
    int fail_idx;
    for (fail_idx = 1; fail_idx <= 12; ++fail_idx) {
      g_cdd_strdup_fail = fail_idx;
      memset(&spec, 0, sizeof(spec));
      rc = cdd_test_parse_info(jo, &spec);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      openapi_spec_free(&spec);
    }
  }

  /* Test extensions error in info, contact, license */
  {
    g_cdd_fail_extensions_init = 1;
    memset(&spec, 0, sizeof(spec));
    rc = cdd_test_parse_info(jo, &spec);
    g_cdd_fail_extensions_init = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    openapi_spec_free(&spec);

    /* Fail at 2nd call (contact) */
    g_cdd_fail_extensions_init = 2;
    memset(&spec, 0, sizeof(spec));
    rc = cdd_test_parse_info(jo, &spec);
    g_cdd_fail_extensions_init = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    openapi_spec_free(&spec);

    /* Fail at 3rd call (license) */
    g_cdd_fail_extensions_init = 3;
    memset(&spec, 0, sizeof(spec));
    rc = cdd_test_parse_info(jo, &spec);
    g_cdd_fail_extensions_init = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    openapi_spec_free(&spec);
  }

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_external_docs branches and OOM.
 */
TEST test_openapi_metadata_parse_external_docs_branches(void) {
  struct OpenAPI_ExternalDocs docs;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* NULL obj or NULL out */
  memset(&docs, 0, sizeof(docs));
  rc = cdd_test_parse_external_docs(NULL, &docs);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_external_docs(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Missing / empty url */
  jv = json_parse_string("{\"description\": \"desc only\"}");
  jo = json_value_get_object(jv);
  memset(&docs, 0, sizeof(docs));
  rc = cdd_test_parse_external_docs(jo, &docs);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  json_value_free(jv);

  jv = json_parse_string("{\"url\": \"\"}");
  jo = json_value_get_object(jv);
  memset(&docs, 0, sizeof(docs));
  rc = cdd_test_parse_external_docs(jo, &docs);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  json_value_free(jv);

  /* Valid externalDocs with description, url, and extensions */
  jv = json_parse_string("{\"description\": \"docs\", \"url\": "
                         "\"http://example.com\", \"x-ext\": 1}");
  jo = json_value_get_object(jv);
  memset(&docs, 0, sizeof(docs));
  rc = cdd_test_parse_external_docs(jo, &docs);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("docs", docs.description);
  ASSERT_STR_EQ("http://example.com", docs.url);
  test_meta_free_external_docs(&docs);

  /* OOM branches on description and url */
  g_cdd_strdup_fail = 1;
  memset(&docs, 0, sizeof(docs));
  rc = cdd_test_parse_external_docs(jo, &docs);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_external_docs(&docs);

  g_cdd_strdup_fail = 2;
  memset(&docs, 0, sizeof(docs));
  rc = cdd_test_parse_external_docs(jo, &docs);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_external_docs(&docs);

  /* Fail extensions */
  g_cdd_fail_extensions_init = 1;
  memset(&docs, 0, sizeof(docs));
  rc = cdd_test_parse_external_docs(jo, &docs);
  g_cdd_fail_extensions_init = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_external_docs(&docs);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_discriminator_object branches and OOM.
 */
TEST test_openapi_metadata_parse_discriminator_branches(void) {
  struct OpenAPI_Discriminator disc;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* NULL obj or NULL out */
  memset(&disc, 0, sizeof(disc));
  rc = cdd_test_parse_discriminator_object(NULL, &disc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_discriminator_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Valid with propertyName, defaultMapping, mapping with x- extension to skip
   */
  jv = json_parse_string("{\n"
                         "  \"propertyName\": \"petType\",\n"
                         "  \"defaultMapping\": \"#/components/schemas/Pet\",\n"
                         "  \"mapping\": {\n"
                         "    \"x-ignore\": \"ignored\",\n"
                         "    \"dog\": \"#/components/schemas/Dog\",\n"
                         "    \"cat\": \"#/components/schemas/Cat\"\n"
                         "  },\n"
                         "  \"x-ext\": 1\n"
                         "}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);

  memset(&disc, 0, sizeof(disc));
  rc = cdd_test_parse_discriminator_object(jo, &disc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("petType", disc.property_name);
  ASSERT_STR_EQ("#/components/schemas/Pet", disc.default_mapping);
  ASSERT_EQ(2, disc.n_mapping);
  test_meta_free_discriminator(&disc);

  /* Test discriminator OOM for propertyName, defaultMapping, and mapping */
  {
    int fail_idx;
    for (fail_idx = 1; fail_idx <= 6; ++fail_idx) {
      g_cdd_strdup_fail = fail_idx;
      memset(&disc, 0, sizeof(disc));
      rc = cdd_test_parse_discriminator_object(jo, &disc);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      test_meta_free_discriminator(&disc);
    }
  }

  /* Extensions failure */
  g_cdd_fail_extensions_init = 1;
  memset(&disc, 0, sizeof(disc));
  rc = cdd_test_parse_discriminator_object(jo, &disc);
  g_cdd_fail_extensions_init = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_discriminator(&disc);

  /* Test mapping with non-string value */
  {
    JSON_Value *jv_nonstr = json_parse_string("{\"mapping\": {\"dog\": 123}}");
    JSON_Object *jo_nonstr = json_value_get_object(jv_nonstr);
    memset(&disc, 0, sizeof(disc));
    rc = cdd_test_parse_discriminator_object(jo_nonstr, &disc);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    test_meta_free_discriminator(&disc);
    json_value_free(jv_nonstr);
  }

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_xml_object branches and OOM.
 */
TEST test_openapi_metadata_parse_xml_branches(void) {
  struct OpenAPI_Xml xml;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* NULL obj or NULL out */
  memset(&xml, 0, sizeof(xml));
  rc = cdd_test_parse_xml_object(NULL, &xml);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_xml_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Valid XML object with attribute, wrapped, namespace, prefix, name, nodeType
   */
  jv = json_parse_string("{\n"
                         "  \"name\": \"item\",\n"
                         "  \"namespace\": \"http://example.com/schema\",\n"
                         "  \"prefix\": \"ex\",\n"
                         "  \"attribute\": true,\n"
                         "  \"wrapped\": false,\n"
                         "  \"nodeType\": \"element\",\n"
                         "  \"x-xml-ext\": true\n"
                         "}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);

  memset(&xml, 0, sizeof(xml));
  rc = cdd_test_parse_xml_object(jo, &xml);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("item", xml.name);
  ASSERT_STR_EQ("http://example.com/schema", xml.namespace_uri);
  ASSERT_STR_EQ("ex", xml.prefix);
  ASSERT_EQ(1, xml.attribute_set);
  ASSERT_EQ(1, xml.attribute);
  ASSERT_EQ(1, xml.wrapped_set);
  ASSERT_EQ(0, xml.wrapped);
  ASSERT_EQ(1, xml.node_type_set);
  test_meta_free_xml(&xml);

  /* OOM on name, namespace, prefix */
  g_cdd_strdup_fail = 1;
  memset(&xml, 0, sizeof(xml));
  rc = cdd_test_parse_xml_object(jo, &xml);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_xml(&xml);

  g_cdd_strdup_fail = 2;
  memset(&xml, 0, sizeof(xml));
  rc = cdd_test_parse_xml_object(jo, &xml);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_xml(&xml);

  g_cdd_strdup_fail = 3;
  memset(&xml, 0, sizeof(xml));
  rc = cdd_test_parse_xml_object(jo, &xml);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_xml(&xml);

  /* Fail extensions */
  g_cdd_fail_extensions_init = 1;
  memset(&xml, 0, sizeof(xml));
  rc = cdd_test_parse_xml_object(jo, &xml);
  g_cdd_fail_extensions_init = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_xml(&xml);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_tags and tag cycle detection branches.
 */
TEST test_openapi_metadata_parse_tags_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* NULL root_obj or NULL out */
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_tags(NULL, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_tags(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* tags missing */
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  rc = cdd_test_parse_tags(jo, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* tags empty */
  jv = json_parse_string("{\"tags\": []}");
  jo = json_value_get_object(jv);
  rc = cdd_test_parse_tags(jo, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* tags with invalid name (empty string) */
  jv = json_parse_string("{\"tags\": [{\"name\": \"\"}]}");
  jo = json_value_get_object(jv);
  rc = cdd_test_parse_tags(jo, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  json_value_free(jv);

  /* tags with duplicate name */
  jv = json_parse_string(
      "{\"tags\": [{\"name\": \"tag1\"}, {\"name\": \"tag1\"}]}");
  jo = json_value_get_object(jv);
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_tags(jo, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* Valid tags with all fields and externalDocs */
  jv = json_parse_string(
      "{\n"
      "  \"tags\": [\n"
      "    {\n"
      "      \"name\": \"parentTag\",\n"
      "      \"summary\": \"Parent summary\",\n"
      "      \"description\": \"Parent desc\",\n"
      "      \"kind\": \"group\"\n"
      "    },\n"
      "    {\n"
      "      \"name\": \"childTag\",\n"
      "      \"parent\": \"parentTag\",\n"
      "      \"externalDocs\": {\"url\": \"http://example.com\"},\n"
      "      \"x-tag-ext\": 1\n"
      "    }\n"
      "  ]\n"
      "}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);

  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_tags(jo, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, spec.n_tags);
  ASSERT_STR_EQ("parentTag", spec.tags[0].name);
  ASSERT_STR_EQ("childTag", spec.tags[1].name);

  /* Validate tag parents succeeds */
  rc = cdd_test_validate_tag_parents(&spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);

  /* Test tag cycle */
  {
    JSON_Value *jv_cycle =
        json_parse_string("{\n"
                          "  \"tags\": [\n"
                          "    {\"name\": \"A\", \"parent\": \"B\"},\n"
                          "    {\"name\": \"B\", \"parent\": \"A\"}\n"
                          "  ]\n"
                          "}");
    JSON_Object *jo_cycle = json_value_get_object(jv_cycle);
    memset(&spec, 0, sizeof(spec));
    rc = cdd_test_parse_tags(jo_cycle, &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_validate_tag_parents(&spec);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    openapi_spec_free(&spec);
    json_value_free(jv_cycle);
  }

  /* Test detect_tag_cycle edge cases: NULL spec, NULL state, idx >= n_tags,
   * state == 2 */
  {
    int state[2] = {0, 0};
    rc = cdd_test_detect_tag_cycle(NULL, 0, state);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_detect_tag_cycle(&spec, 0, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_detect_tag_cycle(&spec, 100, state);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    state[0] = 2;
    spec.n_tags = 1;
    rc = cdd_test_detect_tag_cycle(&spec, 0, state);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.n_tags = 0;
  }

  /* Test OOM on parse_tags fields */
  {
    int fail_idx;
    for (fail_idx = 1; fail_idx <= 6; ++fail_idx) {
      g_cdd_strdup_fail = fail_idx;
      memset(&spec, 0, sizeof(spec));
      rc = cdd_test_parse_tags(jo, &spec);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      openapi_spec_free(&spec);
    }
  }

  /* Fail externalDocs inside parse_tags */
  {
    JSON_Value *jv_bad_ext = json_parse_string(
        "{\"tags\": [{\"name\": \"T\", \"externalDocs\": {\"url\": \"\"}}]}");
    JSON_Object *jo_bad_ext = json_value_get_object(jv_bad_ext);
    memset(&spec, 0, sizeof(spec));
    rc = cdd_test_parse_tags(jo_bad_ext, &spec);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    openapi_spec_free(&spec);
    json_value_free(jv_bad_ext);
  }

  /* Fail extensions inside parse_tags */
  {
    g_cdd_fail_extensions_init = 1;
    memset(&spec, 0, sizeof(spec));
    rc = cdd_test_parse_tags(jo, &spec);
    g_cdd_fail_extensions_init = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    openapi_spec_free(&spec);
  }

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test server parsing, variables, and validation branches.
 */
TEST test_openapi_metadata_server_branches(void) {
  struct OpenAPI_Server srv;
  struct OpenAPI_Server *srvs = NULL;
  size_t srv_count = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* NULL checks */
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(NULL, &srv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_server_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_test_parse_servers_array(NULL, "servers", &srvs, &srv_count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_servers_array(NULL, NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Missing / empty servers array */
  jv = json_parse_string("{\"servers\": []}");
  jo = json_value_get_object(jv);
  rc = cdd_test_parse_servers_array(jo, "servers", &srvs, &srv_count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, srv_count);
  ASSERT_EQ(NULL, srvs);
  json_value_free(jv);

  /* Server object missing url or empty url */
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  json_value_free(jv);

  jv = json_parse_string("{\"url\": \"\"}");
  jo = json_value_get_object(jv);
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  json_value_free(jv);

  /* Server url with query or fragment */
  jv = json_parse_string("{\"url\": \"https://example.com/api?foo=bar\"}");
  jo = json_value_get_object(jv);
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  json_value_free(jv);

  /* Valid server object with variables, description, name, enum */
  jv = json_parse_string("{\n"
                         "  \"url\": \"https://{env}.example.com:{port}/v1\",\n"
                         "  \"name\": \"prod-server\",\n"
                         "  \"description\": \"Production\",\n"
                         "  \"variables\": {\n"
                         "    \"env\": {\n"
                         "      \"default\": \"api\",\n"
                         "      \"description\": \"Environment\",\n"
                         "      \"enum\": [\"api\", \"dev\"],\n"
                         "      \"x-var-ext\": 1\n"
                         "    },\n"
                         "    \"port\": {\n"
                         "      \"default\": \"8443\"\n"
                         "    }\n"
                         "  },\n"
                         "  \"x-srv-ext\": true\n"
                         "}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);

  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("https://{env}.example.com:{port}/v1", srv.url);
  ASSERT_STR_EQ("prod-server", srv.name);
  ASSERT_STR_EQ("Production", srv.description);
  ASSERT_EQ(2, srv.n_variables);
  test_meta_free_server(&srv);

  /* Duplicate server variable in url */
  {
    JSON_Value *jv_dup =
        json_parse_string("{\n"
                          "  \"url\": \"https://{env}.example.com/{env}\",\n"
                          "  \"variables\": {\"env\": {\"default\": \"api\"}}\n"
                          "}");
    JSON_Object *jo_dup = json_value_get_object(jv_dup);
    memset(&srv, 0, sizeof(srv));
    rc = cdd_test_parse_server_object(jo_dup, &srv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    test_meta_free_server(&srv);
    json_value_free(jv_dup);
  }

  /* Unmatched '{' in url */
  {
    char u1[] = "https://example.com/{unclosed";
    char u2[] = "https://example.com/{}";
    char u3[] = "https://example.com/foo}bar";
    struct OpenAPI_Server test_srv;
    memset(&test_srv, 0, sizeof(test_srv));
    test_srv.url = u1;
    rc = cdd_test_validate_server_url_variables(&test_srv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* Empty variable name '{}' */
    test_srv.url = u2;
    rc = cdd_test_validate_server_url_variables(&test_srv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* Stray '}' */
    test_srv.url = u3;
    rc = cdd_test_validate_server_url_variables(&test_srv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* NULL srv or NULL url */
    rc = cdd_test_validate_server_url_variables(NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    test_srv.url = NULL;
    rc = cdd_test_validate_server_url_variables(&test_srv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  /* Variable missing default, empty default, empty enum, default not in enum */
  {
    JSON_Value *jv_bad = json_parse_string(
        "{\"url\": \"https://{v}.com\", \"variables\": {\"v\": {}}}");
    JSON_Object *jo_bad = json_value_get_object(jv_bad);
    memset(&srv, 0, sizeof(srv));
    rc = cdd_test_parse_server_object(jo_bad, &srv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    test_meta_free_server(&srv);
    json_value_free(jv_bad);

    jv_bad = json_parse_string("{\"url\": \"https://{v}.com\", \"variables\": "
                               "{\"v\": {\"default\": \"\"}}}");
    jo_bad = json_value_get_object(jv_bad);
    memset(&srv, 0, sizeof(srv));
    rc = cdd_test_parse_server_object(jo_bad, &srv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    test_meta_free_server(&srv);
    json_value_free(jv_bad);

    jv_bad = json_parse_string("{\"url\": \"https://{v}.com\", \"variables\": "
                               "{\"v\": {\"default\": \"a\", \"enum\": []}}}");
    jo_bad = json_value_get_object(jv_bad);
    memset(&srv, 0, sizeof(srv));
    rc = cdd_test_parse_server_object(jo_bad, &srv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    test_meta_free_server(&srv);
    json_value_free(jv_bad);

    jv_bad = json_parse_string(
        "{\"url\": \"https://{v}.com\", \"variables\": {\"v\": {\"default\": "
        "\"a\", \"enum\": [\"b\", \"c\"]}}}");
    jo_bad = json_value_get_object(jv_bad);
    memset(&srv, 0, sizeof(srv));
    rc = cdd_test_parse_server_object(jo_bad, &srv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    test_meta_free_server(&srv);
    json_value_free(jv_bad);
  }

  /* Test parse_servers_array duplicate server names */
  {
    JSON_Value *jv_arr = json_parse_string(
        "{\n"
        "  \"servers\": [\n"
        "    {\"url\": \"https://a.com\", \"name\": \"serv1\"},\n"
        "    {\"url\": \"https://b.com\", \"name\": \"serv1\"}\n"
        "  ]\n"
        "}");
    JSON_Object *jo_arr = json_value_get_object(jv_arr);
    srvs = NULL;
    srv_count = 0;
    rc = cdd_test_parse_servers_array(jo_arr, "servers", &srvs, &srv_count);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    if (srvs) {
      openapi_free_servers_array(srvs, srv_count);
      srvs = NULL;
    }
    json_value_free(jv_arr);
  }

  /* Test parse_servers_array with nameless servers */
  {
    JSON_Value *jv_arr = json_parse_string(
        "{\n"
        "  \"servers\": [\n"
        "    {\"url\": \"https://a.com\"},\n"
        "    {\"url\": \"https://b.com\", \"name\": \"serv2\"}\n"
        "  ]\n"
        "}");
    JSON_Object *jo_arr = json_value_get_object(jv_arr);
    srvs = NULL;
    srv_count = 0;
    rc = cdd_test_parse_servers_array(jo_arr, "servers", &srvs, &srv_count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(2, srv_count);
    if (srvs) {
      openapi_free_servers_array(srvs, srv_count);
      srvs = NULL;
    }
    json_value_free(jv_arr);
  }

  /* OOM loops on server object strdups */
  {
    int fail_idx;
    for (fail_idx = 1; fail_idx <= 8; ++fail_idx) {
      g_cdd_strdup_fail = fail_idx;
      memset(&srv, 0, sizeof(srv));
      rc = cdd_test_parse_server_object(jo, &srv);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      test_meta_free_server(&srv);
    }
  }

  /* Extensions failure in server and server variable */
  g_cdd_fail_extensions_init = 1;
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  g_cdd_fail_extensions_init = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_server(&srv);

  g_cdd_fail_extensions_init = 2;
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  g_cdd_fail_extensions_init = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_server(&srv);

  json_value_free(jv);
  PASS();
}

extern C_CDD_EXPORT int g_cdd_alloc_fail;

/**
 * @brief Test additional branches and edge cases in openapi_metadata.c.
 */
TEST test_openapi_metadata_extra_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Xml xml;
  struct OpenAPI_Discriminator disc;
  struct OpenAPI_Server srv;
  struct OpenAPI_Server *srvs = NULL;
  size_t srv_count = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  size_t tag_idx = 0;
  cdd_c_error_t rc = 0;

  /* 1. tag_index_by_name branches */
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_tag_index_by_name(NULL, "tag", &tag_idx);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_tag_index_by_name(&spec, NULL, &tag_idx);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_tag_index_by_name(&spec, "tag", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_tag_index_by_name(&spec, "tag", &tag_idx);
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc);

  /* 2. server_variable_defined and server_variable_seen branches */
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_server_variable_defined(NULL, "var");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_server_variable_defined(&srv, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_server_variable_defined(&srv, "var");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = cdd_test_server_variable_seen(NULL, 0, "var");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_server_variable_seen(NULL, 0, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  {
    char *seen[2];
    char s1[] = "seen1";
    char s2[] = "seen2";
    seen[0] = s1;
    seen[1] = s2;
    rc = cdd_test_server_variable_seen(seen, 2, "not_seen");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_server_variable_seen(seen, 2, "seen1");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  }

  /* 3. validate_tag_parents branches */
  rc = cdd_test_validate_tag_parents(NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_validate_tag_parents(&spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Tag with parent without name */
  {
    struct OpenAPI_Tag tags[1];
    memset(tags, 0, sizeof(tags));
    spec.tags = tags;
    spec.n_tags = 1;
    rc = cdd_test_validate_tag_parents(&spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.tags = NULL;
    spec.n_tags = 0;
  }

  /* validate_tag_parents calloc OOM */
  {
    struct OpenAPI_Tag tags[1];
    memset(tags, 0, sizeof(tags));
    spec.tags = tags;
    spec.n_tags = 1;
    g_cdd_alloc_fail = 1;
    rc = cdd_test_validate_tag_parents(&spec);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    spec.tags = NULL;
    spec.n_tags = 0;
  }

  /* 4. parse_info without optional val branches and identifier OOM */
  jv =
      json_parse_string("{\"info\": {\"title\": \"T\", \"version\": \"1.0\"}}");
  jo = json_value_get_object(jv);
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_info(jo, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* Identifier OOM */
  jv = json_parse_string(
      "{\"info\": {\"title\": \"T\", \"version\": \"1.0\", \"license\": "
      "{\"name\": \"MIT\", \"identifier\": \"MIT\"} }}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 4;
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_info(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 5. parse_discriminator mapping calloc OOM */
  jv = json_parse_string("{\"mapping\": {\"k1\": \"#/v1\"}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  memset(&disc, 0, sizeof(disc));
  rc = cdd_test_parse_discriminator_object(jo, &disc);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  test_meta_free_discriminator(&disc);
  json_value_free(jv);

  /* Discriminator mapping without mapping_obj */
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  memset(&disc, 0, sizeof(disc));
  rc = cdd_test_parse_discriminator_object(jo, &disc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  test_meta_free_discriminator(&disc);
  json_value_free(jv);

  /* 6. parse_xml_object with NULL fields */
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  memset(&xml, 0, sizeof(xml));
  rc = cdd_test_parse_xml_object(jo, &xml);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  test_meta_free_xml(&xml);
  json_value_free(jv);

  /* 7. parse_tags calloc OOM */
  jv = json_parse_string("{\"tags\": [{\"name\": \"tag1\"}]}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_tags(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 8. validate_server_url_variables OOM on malloc and realloc */
  {
    char u_var[] = "https://{a}.{b}.{c}.{d}.{e}.example.com";
    char n0[] = "a";
    char n1[] = "b";
    char n2[] = "c";
    char n3[] = "d";
    char n4[] = "e";
    struct OpenAPI_Server s;
    struct OpenAPI_ServerVariable vars[5];
    memset(&s, 0, sizeof(s));
    memset(vars, 0, sizeof(vars));
    vars[0].name = n0;
    vars[1].name = n1;
    vars[2].name = n2;
    vars[3].name = n3;
    vars[4].name = n4;
    s.url = u_var;
    s.variables = vars;
    s.n_variables = 5;

    /* Fail malloc of name */
    g_cdd_alloc_fail = 1;
    rc = cdd_test_validate_server_url_variables(&s);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    /* Fail realloc of seen (when seen_count == seen_cap) */
    {
      int af;
      for (af = 1; af <= 8; ++af) {
        g_cdd_alloc_fail = af;
        rc = cdd_test_validate_server_url_variables(&s);
        if (g_cdd_alloc_fail != 0) {
          /* Did not consume alloc fail */
          g_cdd_alloc_fail = 0;
          break;
        }
        g_cdd_alloc_fail = 0;
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
    }

    /* Normal success of validate_server_url_variables with 5 variables */
    rc = cdd_test_validate_server_url_variables(&s);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  /* 9. parse_servers_array combinations */
  {
    /* parent has no key */
    jv = json_parse_string("{}");
    jo = json_value_get_object(jv);
    srvs = NULL;
    srv_count = 0;
    rc = cdd_test_parse_servers_array(jo, "servers", &srvs, &srv_count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    json_value_free(jv);

    /* server in array has empty name and another has valid name */
    jv = json_parse_string("{\"servers\": ["
                           "  {\"url\": \"https://a.com\", \"name\": \"\"},"
                           "  {\"url\": \"https://b.com\", \"name\": \"srvB\"},"
                           "  {\"url\": \"https://c.com\", \"name\": \"\"},"
                           "  {\"url\": \"https://d.com\", \"name\": \"srvD\"}"
                           "]}");
    jo = json_value_get_object(jv);
    srvs = NULL;
    srv_count = 0;
    rc = cdd_test_parse_servers_array(jo, "servers", &srvs, &srv_count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(4, srv_count);
    if (srvs) {
      openapi_free_servers_array(srvs, srv_count);
      srvs = NULL;
    }
    json_value_free(jv);

    /* server with null / non-object item in array */
    jv = json_parse_string("{\"servers\": [null, 123]}");
    jo = json_value_get_object(jv);
    srvs = NULL;
    srv_count = 0;
    rc = cdd_test_parse_servers_array(jo, "servers", &srvs, &srv_count);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    if (srvs) {
      openapi_free_servers_array(srvs, srv_count);
      srvs = NULL;
    }
    json_value_free(jv);
  }

  PASS();
}

/**
 * @brief Comprehensive branch coverage for openapi_metadata.c remaining
 * branches.
 */
TEST test_openapi_metadata_all_remaining_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Discriminator disc;
  struct OpenAPI_Server srv;
  struct OpenAPI_Server *srvs = NULL;
  size_t srv_count = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* Line 81: contact without url */
  jv = json_parse_string("{\"info\": {\"title\": \"T\", \"version\": \"1.0\", "
                         "\"contact\": {\"name\": \"Alice\"} }}");
  jo = json_value_get_object(jv);
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_info(jo, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* Line 116: license with empty name string */
  jv = json_parse_string("{\"info\": {\"title\": \"T\", \"version\": \"1.0\", "
                         "\"license\": {\"name\": \"\"} }}");
  jo = json_value_get_object(jv);
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_info(jo, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* Line 159, 197, 278, 342, 605: NULL out with non-NULL obj */
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  rc = cdd_test_parse_external_docs(jo, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_discriminator_object(jo, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_xml_object(jo, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_tags(jo, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_server_object(jo, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Line 222, 226, 236: discriminator mapping with only x- properties (used ==
   * 0) */
  jv = json_parse_string("{\"mapping\": {\"x-only\": \"#/schemas/Ignored\"}}");
  jo = json_value_get_object(jv);
  memset(&disc, 0, sizeof(disc));
  rc = cdd_test_parse_discriminator_object(jo, &disc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, disc.n_mapping);
  test_meta_free_discriminator(&disc);
  json_value_free(jv);

  /* Line 373: parse_tags with multiple tags with different names */
  jv = json_parse_string(
      "{\"tags\": [{\"name\": \"tagA\"}, {\"name\": \"tagB\"}]}");
  jo = json_value_get_object(jv);
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_tags(jo, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* Line 429: tag_index_by_name matching on 2nd tag */
  {
    struct OpenAPI_Tag two_tags[2];
    char t1[] = "t1";
    char t2[] = "t2";
    size_t out_idx = 0;
    memset(&spec, 0, sizeof(spec));
    memset(two_tags, 0, sizeof(two_tags));
    two_tags[0].name = t1;
    two_tags[1].name = t2;
    spec.tags = two_tags;
    spec.n_tags = 2;
    rc = cdd_test_tag_index_by_name(&spec, "t2", &out_idx);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, out_idx);
    spec.tags = NULL;
    spec.n_tags = 0;
  }

  /* Line 452: tag with empty parent string */
  {
    struct OpenAPI_Tag tag_empty_parent[1];
    char name_p[] = "tagA";
    char empty_parent[] = "";
    memset(&spec, 0, sizeof(spec));
    memset(tag_empty_parent, 0, sizeof(tag_empty_parent));
    tag_empty_parent[0].name = name_p;
    tag_empty_parent[0].parent = empty_parent;
    spec.tags = tag_empty_parent;
    spec.n_tags = 1;
    rc = cdd_test_validate_tag_parents(&spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.tags = NULL;
    spec.n_tags = 0;
  }

  /* Line 468: validate_tag_parents with tags != NULL but n_tags == 0 */
  {
    struct OpenAPI_Tag dummy_tag[1];
    memset(&spec, 0, sizeof(spec));
    spec.tags = dummy_tag;
    spec.n_tags = 0;
    rc = cdd_test_validate_tag_parents(&spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.tags = NULL;
  }

  /* Line 493: server_variable_defined matching on 2nd variable */
  {
    struct OpenAPI_Server test_srv;
    struct OpenAPI_ServerVariable two_vars[2];
    char v1[] = "v1";
    char v2[] = "v2";
    memset(&test_srv, 0, sizeof(test_srv));
    memset(two_vars, 0, sizeof(two_vars));
    two_vars[0].name = v1;
    two_vars[1].name = v2;
    test_srv.variables = two_vars;
    test_srv.n_variables = 2;
    rc = cdd_test_server_variable_defined(&test_srv, "v2");
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
  }

  /* Line 505: seen != NULL but name == NULL */
  {
    char *seen[1];
    char s[] = "s";
    seen[0] = s;
    rc = cdd_test_server_variable_seen(seen, 1, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  /* Line 508: server_variable_seen matching on 2nd element */
  {
    char *seen[2];
    char s1[] = "s1";
    char s2[] = "s2";
    seen[0] = s1;
    seen[1] = s2;
    rc = cdd_test_server_variable_seen(seen, 2, "s2");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  }

  /* Line 616: url without query or fragment */
  jv = json_parse_string("{\"url\": \"https://example.com/clean/path\"}");
  jo = json_value_get_object(jv);
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  test_meta_free_server(&srv);
  json_value_free(jv);

  /* Line 644: server with empty variables object */
  jv = json_parse_string(
      "{\"url\": \"https://example.com\", \"variables\": {}}");
  jo = json_value_get_object(jv);
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, srv.n_variables);
  test_meta_free_server(&srv);
  json_value_free(jv);

  /* Line 733: parse_servers_array combinations of NULL arguments */
  jv = json_parse_string("{\"servers\": []}");
  jo = json_value_get_object(jv);
  rc = cdd_test_parse_servers_array(jo, NULL, &srvs, &srv_count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_servers_array(jo, "servers", NULL, &srv_count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_test_parse_servers_array(jo, "servers", &srvs, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Line 772: servers array where second server has no name property */
  jv = json_parse_string("{\"servers\": ["
                         "  {\"url\": \"https://a.com\", \"name\": \"srvA\"},"
                         "  {\"url\": \"https://b.com\"},"
                         "  {\"url\": \"https://c.com\", \"name\": \"\"},"
                         "  {\"url\": \"https://d.com\", \"name\": \"srvD\"}"
                         "]}");
  jo = json_value_get_object(jv);
  srvs = NULL;
  srv_count = 0;
  rc = cdd_test_parse_servers_array(jo, "servers", &srvs, &srv_count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(4, srv_count);
  if (srvs) {
    openapi_free_servers_array(srvs, srv_count);
    srvs = NULL;
  }
  json_value_free(jv);

  PASS();
}

/**
 * @brief Tests for final missing branches in openapi_metadata.c.
 */
TEST test_openapi_metadata_final_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Server srv;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* Line 81: contact with url but without name */
  jv = json_parse_string("{\"info\": {\"title\": \"T\", \"version\": \"1.0\", "
                         "\"contact\": {\"url\": \"http://example.com\"} }}");
  jo = json_value_get_object(jv);
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_parse_info(jo, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, spec.info.contact.name);
  ASSERT_STR_EQ("http://example.com", spec.info.contact.url);
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* Line 660: variable that is NOT an object (e.g. an integer) */
  jv = json_parse_string(
      "{\"url\": \"https://example.com\", \"variables\": {\"var1\": 123}}");
  jo = json_value_get_object(jv);
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  test_meta_free_server(&srv);
  json_value_free(jv);

  /* Line 666: variable without default */
  jv = json_parse_string("{\"url\": \"https://example.com\", \"variables\": "
                         "{\"var1\": {\"description\": \"no default\"} }}");
  jo = json_value_get_object(jv);
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  test_meta_free_server(&srv);
  json_value_free(jv);

  /* Line 684: variable without enum */
  jv = json_parse_string("{\"url\": \"https://example.com\", \"variables\": "
                         "{\"var1\": {\"default\": \"v1\"} }}");
  jo = json_value_get_object(jv);
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  test_meta_free_server(&srv);
  json_value_free(jv);

  /* Line 691: enum array containing non-string value */
  jv = json_parse_string(
      "{\"url\": \"https://example.com\", \"variables\": {\"var1\": "
      "{\"default\": \"v1\", \"enum\": [123, \"v1\"]} }}");
  jo = json_value_get_object(jv);
  memset(&srv, 0, sizeof(srv));
  rc = cdd_test_parse_server_object(jo, &srv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  test_meta_free_server(&srv);
  json_value_free(jv);

  PASS();
}

/**
 * @brief Test metadata cleanup helpers with NULL arguments.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_metadata_cleanups(void) {
  test_meta_free_external_docs(NULL);
  test_meta_free_discriminator(NULL);
  test_meta_free_xml(NULL);
  test_meta_free_server(NULL);
  PASS();
}

#define OPENAPI_METADATA_COVERAGE_TESTS()                                      \
  RUN_TEST(test_openapi_metadata_parse_info_branches);                         \
  RUN_TEST(test_openapi_metadata_parse_external_docs_branches);                \
  RUN_TEST(test_openapi_metadata_parse_discriminator_branches);                \
  RUN_TEST(test_openapi_metadata_parse_xml_branches);                          \
  RUN_TEST(test_openapi_metadata_parse_tags_branches);                         \
  RUN_TEST(test_openapi_metadata_server_branches);                             \
  RUN_TEST(test_openapi_metadata_extra_branches);                              \
  RUN_TEST(test_openapi_metadata_all_remaining_branches);                      \
  RUN_TEST(test_openapi_metadata_final_branches);                              \
  RUN_TEST(test_openapi_metadata_cleanups)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_METADATA_COVERAGE_H */
