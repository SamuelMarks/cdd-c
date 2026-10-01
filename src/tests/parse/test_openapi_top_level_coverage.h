/**
 * @file test_openapi_top_level_coverage.h
 * @brief Comprehensive 100% test coverage for openapi.c.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_TOP_LEVEL_COVERAGE_H
#define TEST_OPENAPI_TOP_LEVEL_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
#include "openapi/parse/openapi_test_helpers.h"
#include "openapi/parse/openapi_internal_components.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_fail_extensions_init;
extern C_CDD_EXPORT int g_cdd_fail_find_schema_by_anchor;
extern C_CDD_EXPORT int g_cdd_fail_find_schema_by_id;

TEST test_openapi_null_and_invalid_args(void) {
  struct OpenAPI_Spec spec;
  struct StructFields *sf = NULL;
  struct OpenAPI_SchemaRef sref;
  JSON_Value *jv_str;
  JSON_Value *jv_bool;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));
  memset(&sref, 0, sizeof(sref));

  /* Test NULL root and NULL out */
  rc = openapi_load_from_json(NULL, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = openapi_load_from_json(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  jv_str = json_parse_string("\"string_not_obj\"");
  ASSERT_NEQ(NULL, jv_str);
  rc = openapi_load_from_json(jv_str, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Test root is not an object and not boolean */
  rc = openapi_load_from_json(jv_str, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  json_value_free(jv_str);

  /* Boolean schema document */
  jv_bool = json_value_init_boolean(1);
  ASSERT_NEQ(NULL, jv_bool);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv_bool, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);
  json_value_free(jv_bool);

  /* Test empty retrieval_uri string */
  jv_str = json_parse_string("{\"openapi\": \"3.0.0\", \"info\": {\"title\": "
                             "\"T\", \"version\": \"1\"}, \"paths\": {}}");
  ASSERT_NEQ(NULL, jv_str);
  rc = openapi_load_from_json_with_context(jv_str, "", &spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);
  json_value_free(jv_str);

  /* openapi_spec_find_schema checks */
  rc = openapi_spec_find_schema(NULL, "User", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, sf);

  rc = openapi_spec_find_schema(&spec, NULL, &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, sf);

  rc = openapi_spec_find_schema(&spec, "User", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* openapi_spec_find_schema_by_id checks */
  rc = openapi_spec_find_schema_by_id(NULL, "http://id", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, sf);

  rc = openapi_spec_find_schema_by_id(&spec, NULL, &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, sf);

  rc = openapi_spec_find_schema_by_id(&spec, "http://id", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = openapi_spec_find_schema_by_id(&spec, "http://id", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, sf);

  /* openapi_spec_find_schema_by_anchor checks */
  rc = openapi_spec_find_schema_by_anchor(NULL, "#anchor", 0, &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, sf);

  rc = openapi_spec_find_schema_by_anchor(&spec, NULL, 0, &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, sf);

  rc = openapi_spec_find_schema_by_anchor(&spec, "#anchor", 0, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* openapi_spec_find_schema_for_ref checks */
  rc = openapi_spec_find_schema_for_ref(NULL, &sref, &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, sf);

  rc = openapi_spec_find_schema_for_ref(&spec, NULL, &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, sf);

  rc = openapi_spec_find_schema_for_ref(&spec, &sref, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_openapi_spec_find_schema_branches(void) {
  struct OpenAPI_Spec spec;
  struct StructFields fields[2];
  char *names[2];
  char model_name[] = "FoundModel";
  struct StructFields *found = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));
  memset(fields, 0, sizeof(fields));

  names[0] = NULL;
  names[1] = model_name;

  spec.n_defined_schemas = 2;
  spec.defined_schemas = fields;
  spec.defined_schema_names = names;

  /* Look for match with NULL entry at index 0 */
  rc = openapi_spec_find_schema(&spec, "FoundModel", &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(&fields[1], found);

  /* Look for item not present */
  found = NULL;
  rc = openapi_spec_find_schema(&spec, "NonExistent", &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  PASS();
}

TEST test_openapi_spec_find_schema_by_id_branches(void) {
  struct OpenAPI_Spec spec;
  struct StructFields fields[2];
  char *ids[2];
  char user_schema_id[] = "https://example.com/schemas/user";
  struct StructFields *found = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));
  memset(fields, 0, sizeof(fields));

  ids[0] = NULL;
  ids[1] = user_schema_id;

  spec.n_defined_schemas = 2;
  spec.defined_schemas = fields;
  spec.defined_schema_ids = ids;
  spec.document_uri = (char *)(size_t) "http://example.com/base/";

  /* Fragmented ref targeting subschema returns NULL */
  rc = openapi_spec_find_schema_by_id(
      &spec, "https://example.com/schemas/user#sub", &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* Ref with base_len == 0 ("#" or "") returns NULL */
  rc = openapi_spec_find_schema_by_id(&spec, "#", &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  rc = openapi_spec_find_schema_by_id(&spec, "", &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* Exact match with hash without fragment */
  rc = openapi_spec_find_schema_by_id(
      &spec, "https://example.com/schemas/user#", &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(&fields[1], found);

  /* Exact match without hash */
  found = NULL;
  rc = openapi_spec_find_schema_by_id(&spec, "https://example.com/schemas/user",
                                      &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(&fields[1], found);

  /* Non-matching id with same length to test strncmp != 0 */
  found = NULL;
  rc = openapi_spec_find_schema_by_id(&spec, "https://example.com/schemas/diff",
                                      &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* Non-matching id with different length to test strlen != base_len */
  found = NULL;
  rc = openapi_spec_find_schema_by_id(&spec, "https://example.com/short",
                                      &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* Test g_cdd_fail_find_schema_by_id > 1 branch */
  g_cdd_fail_find_schema_by_id = 2;
  rc = openapi_spec_find_schema_by_id(&spec, "https://example.com/schemas/user",
                                      &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = openapi_spec_find_schema_by_id(&spec, "https://example.com/schemas/user",
                                      &found);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_fail_find_schema_by_id = 0;

  PASS();
}

TEST test_openapi_spec_find_schema_by_anchor_branches(void) {
  struct OpenAPI_Spec spec;
  struct StructFields fields[2];
  char *anchors[2];
  char *dyn_anchors[2];
  char anchor_buf[] = "MyAnchor";
  char dyn_anchor_buf[] = "DynAnchor";
  struct StructFields *found = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));
  memset(fields, 0, sizeof(fields));

  anchors[0] = NULL;
  anchors[1] = anchor_buf;
  dyn_anchors[0] = dyn_anchor_buf;
  dyn_anchors[1] = NULL;

  spec.n_defined_schemas = 2;
  spec.defined_schemas = fields;

  /* No "#" in ref */
  rc = openapi_spec_find_schema_by_anchor(&spec, "no_hash", 0, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* Trailing "#" in ref */
  rc = openapi_spec_find_schema_by_anchor(&spec, "empty_hash#", 0, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* Anchor starting with "/" is a JSON pointer, not an anchor */
  rc = openapi_spec_find_schema_by_anchor(&spec, "#/components/schemas/Pet", 0,
                                          &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* anchors list is NULL */
  spec.defined_schema_anchors = NULL;
  rc = openapi_spec_find_schema_by_anchor(&spec, "#MyAnchor", 0, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  spec.defined_schema_dynamic_anchors = NULL;
  rc = openapi_spec_find_schema_by_anchor(&spec, "#DynAnchor", 1, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* Successful match on static anchor */
  spec.defined_schema_anchors = anchors;
  rc = openapi_spec_find_schema_by_anchor(&spec, "#MyAnchor", 0, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(&fields[1], found);

  /* Successful match on dynamic anchor */
  spec.defined_schema_dynamic_anchors = dyn_anchors;
  found = NULL;
  rc = openapi_spec_find_schema_by_anchor(&spec, "#DynAnchor", 1, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(&fields[0], found);

  /* Not found in list */
  found = NULL;
  rc = openapi_spec_find_schema_by_anchor(&spec, "#NotFound", 0, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  PASS();
}

TEST test_openapi_spec_find_schema_for_ref_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Spec target_spec;
  struct OpenAPI_DocRegistry reg;
  struct OpenAPI_SchemaRef ref;
  struct StructFields fields[3];
  char *anchors[3];
  char *dyn_anchors[3];
  char *ids[3];
  char static_name[] = "Static";
  char dyn_name[] = "Dyn";
  char id_name[] = "https://schema/user";
  char ref_name_buf[] = "User";
  char ext_ref_name_buf[] = "sub.json#User";
  char dyn_ref_buf[] = "#Dyn";
  char static_ref_buf[] = "#Static";
  char id_ref_buf[] = "https://schema/user";
  struct StructFields *found = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));
  memset(&target_spec, 0, sizeof(target_spec));
  memset(&ref, 0, sizeof(ref));
  memset(fields, 0, sizeof(fields));

  openapi_doc_registry_init(&reg);
  target_spec.retrieval_uri =
      (char *)(size_t) "http://example.com/base/sub.json";
  openapi_doc_registry_add(&reg, &target_spec);

  anchors[0] = static_name;
  anchors[1] = NULL;
  anchors[2] = NULL;

  dyn_anchors[0] = NULL;
  dyn_anchors[1] = dyn_name;
  dyn_anchors[2] = NULL;

  ids[0] = NULL;
  ids[1] = NULL;
  ids[2] = id_name;

  spec.n_defined_schemas = 3;
  spec.defined_schemas = fields;
  spec.defined_schema_anchors = anchors;
  spec.defined_schema_dynamic_anchors = dyn_anchors;
  spec.defined_schema_ids = ids;
  spec.document_uri = (char *)(size_t) "http://example.com/base/";
  spec.doc_registry = &reg;

  /* Test cleanups for allocated ref members */
  ref.n_all_of = 1;
  ref.all_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  ref.n_any_of = 1;
  ref.any_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  ref.n_one_of = 1;
  ref.one_of =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  ref.not_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  ref.if_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  ref.then_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));
  ref.else_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));

  /* ref has no ref or ref_name */
  rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* ref with ref_name without ref */
  memset(&ref, 0, sizeof(ref));
  ref.ref_name = ref_name_buf;
  rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* ref with ref_name AND external ref (triggering free(resolved.resolved_ref))
   */
  memset(&ref, 0, sizeof(ref));
  ref.ref_name = ref_name_buf;
  ref.ref = ext_ref_name_buf;
  rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* ref with ref_name AND local ref (resolved.resolved_ref is NULL) */
  {
    char local_ref_name[] = "#LocalUser";
    memset(&ref, 0, sizeof(ref));
    ref.ref_name = ref_name_buf;
    ref.ref = local_ref_name;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  /* resolve_ref_target failure on ref_name */
  memset(&ref, 0, sizeof(ref));
  ref.ref_name = ref_name_buf;
  ref.ref = ext_ref_name_buf;
  g_cdd_alloc_fail = 1;
  rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* resolve_ref_target failure on ref */
  memset(&ref, 0, sizeof(ref));
  ref.ref = ext_ref_name_buf;
  g_cdd_alloc_fail = 1;
  rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* ref with ref_is_dynamic and dynamic anchor found! */
  memset(&ref, 0, sizeof(ref));
  ref.ref = dyn_ref_buf;
  ref.ref_is_dynamic = 1;
  rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(&fields[1], found);

  /* ref with ref_is_dynamic == 0 and static anchor found! */
  memset(&ref, 0, sizeof(ref));
  ref.ref = static_ref_buf;
  ref.ref_is_dynamic = 0;
  found = NULL;
  rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(&fields[0], found);

  /* ref with ref_is_dynamic == 1, dynamic not found, fallback to static anchor
   * found! */
  memset(&ref, 0, sizeof(ref));
  ref.ref = static_ref_buf;
  ref.ref_is_dynamic = 1;
  found = NULL;
  rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(&fields[0], found);

  /* ref with ref_is_dynamic == 0, static not found, fallback to dynamic anchor
   * found! */
  memset(&ref, 0, sizeof(ref));
  ref.ref = dyn_ref_buf;
  ref.ref_is_dynamic = 0;
  found = NULL;
  rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(&fields[1], found);

  /* ref with neither anchor found, fallback to ID found! */
  memset(&ref, 0, sizeof(ref));
  ref.ref = id_ref_buf;
  ref.ref_is_dynamic = 0;
  found = NULL;
  rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(&fields[2], found);

  /* Test OOM error branches in anchor and id searches with non-matching anchor
   */
  {
    char missing_anchor[] = "sub.json#MissingAnchor";
    memset(&ref, 0, sizeof(ref));
    ref.ref = missing_anchor;

    ref.ref_is_dynamic = 1;
    g_cdd_fail_find_schema_by_anchor = 1;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_find_schema_by_anchor = 0;

    g_cdd_fail_find_schema_by_anchor = 2;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_find_schema_by_anchor = 0;

    ref.ref_is_dynamic = 0;
    g_cdd_fail_find_schema_by_anchor = 1;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_find_schema_by_anchor = 0;

    g_cdd_fail_find_schema_by_anchor = 2;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_find_schema_by_anchor = 0;

    g_cdd_fail_find_schema_by_id = 1;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_find_schema_by_id = 0;

    /* Normal execution where resolved.resolved_ref is freed on
     * success/not-found */
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, found);
  }

  /* Test with local anchor so resolved.resolved_ref is NULL when
   * errors/cleanups run */
  {
    char local_missing[] = "#LocalMissing";
    memset(&ref, 0, sizeof(ref));
    ref.ref = local_missing;

    ref.ref_is_dynamic = 1;
    g_cdd_fail_find_schema_by_anchor = 1;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_find_schema_by_anchor = 0;

    g_cdd_fail_find_schema_by_anchor = 2;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_find_schema_by_anchor = 0;

    ref.ref_is_dynamic = 0;
    g_cdd_fail_find_schema_by_anchor = 1;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_find_schema_by_anchor = 0;

    g_cdd_fail_find_schema_by_anchor = 2;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_find_schema_by_anchor = 0;

    g_cdd_fail_find_schema_by_id = 1;
    rc = openapi_spec_find_schema_for_ref(&spec, &ref, &found);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_find_schema_by_id = 0;
  }

  openapi_doc_registry_free(&reg);
  PASS();
}

TEST test_openapi_schema_document_extra_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_DocRegistry reg;
  JSON_Value *jv;
  cdd_c_error_t rc;

  openapi_doc_registry_init(&reg);

  /* Schema doc with empty $id and non-empty retrieval_uri */
  jv = json_parse_string("{\"$id\": \"\", \"type\": \"object\"}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json_with_context(jv, "http://example.com/schema.json",
                                           &spec, &reg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);

  /* Second load of identical document with same retrieval_uri triggers
   * duplicate in registry */
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json_with_context(jv, "http://example.com/schema.json",
                                           &spec, &reg);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  json_value_free(jv);

  /* Schema doc compute_document_uri OOM failure */
  jv = json_parse_string(
      "{\"$id\": \"http://example.com/id.json\", \"type\": \"object\"}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  g_cdd_strdup_fail = 1;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* Schema doc store_schema_root_json OOM failure (no $id and no retrieval_uri)
   */
  jv = json_parse_string("{\"type\": \"object\"}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  g_cdd_strdup_fail = 1;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  openapi_doc_registry_free(&reg);
  PASS();
}

TEST test_openapi_swagger_oom_and_specs(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv;
  cdd_c_error_t rc;

  /* Swagger with host, basePath, schemes, consumes, produces */
  const char *swag_json =
      "{\"swagger\": \"2.0\", \"info\": {\"title\": \"Swag\", \"version\": "
      "\"1.0\"},"
      " \"host\": \"api.example.com\", \"basePath\": \"/v1\","
      " \"schemes\": [\"https\", \"http\"],"
      " \"consumes\": [\"application/json\"],"
      " \"produces\": [\"application/json\"],"
      " \"paths\": {}}";

  jv = json_parse_string(swag_json);
  ASSERT_NEQ(NULL, jv);

  /* Normal success */
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);

  /* OOM on swagger_version */
  memset(&spec, 0, sizeof(spec));
  g_cdd_strdup_fail = 1;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* OOM on host */
  memset(&spec, 0, sizeof(spec));
  g_cdd_strdup_fail = 2;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* OOM on basePath */
  memset(&spec, 0, sizeof(spec));
  g_cdd_strdup_fail = 3;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* OOM on calloc for schemes */
  memset(&spec, 0, sizeof(spec));
  g_cdd_alloc_fail = 1;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* OOM on calloc for consumes */
  memset(&spec, 0, sizeof(spec));
  g_cdd_alloc_fail = 2;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* OOM on calloc for produces */
  memset(&spec, 0, sizeof(spec));
  g_cdd_alloc_fail = 3;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* OOM inside schemes loop */
  memset(&spec, 0, sizeof(spec));
  g_cdd_strdup_fail = 4;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* OOM inside consumes loop */
  memset(&spec, 0, sizeof(spec));
  g_cdd_strdup_fail = 6;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* OOM inside produces loop */
  memset(&spec, 0, sizeof(spec));
  g_cdd_strdup_fail = 7;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  json_value_free(jv);
  PASS();
}

TEST test_openapi_oauth2_and_extensions_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv;
  cdd_c_error_t rc;
  const char *oauth2_no_schemas;
  const char *self_and_dialect;
  const char *paths_ext;
  const char *webhooks_ext;
  const char *comps_ext;

  /* Spec with OAuth2 and no components.schemas */
  oauth2_no_schemas =
      "{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"T\", \"version\": "
      "\"1.0\"},"
      " \"paths\": {},"
      " \"components\": {\"securitySchemes\": {\"oauth\": {\"type\": "
      "\"oauth2\", \"flows\": {\"implicit\": {\"authorizationUrl\": "
      "\"https://auth.com\", \"scopes\": {}}}}}}}";
  jv = json_parse_string(oauth2_no_schemas);
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);

  /* OOM on json_value_init_object inside OAuth2 schemas */
  memset(&spec, 0, sizeof(spec));
  g_cdd_alloc_fail = 1;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;
  json_value_free(jv);

  /* Spec with self_uri and jsonSchemaDialect */
  self_and_dialect =
      "{\"openapi\": \"3.1.0\", \"$self\": \"http://example.com/self.json\", "
      "\"jsonSchemaDialect\": "
      "\"https://json-schema.org/draft/2020-12/schema\", \"info\": {\"title\": "
      "\"T\", \"version\": \"1.0\"}, \"paths\": {}}";
  jv = json_parse_string(self_and_dialect);
  ASSERT_NEQ(NULL, jv);

  /* OOM on $self */
  memset(&spec, 0, sizeof(spec));
  g_cdd_strdup_fail = 2;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  /* OOM on jsonSchemaDialect */
  memset(&spec, 0, sizeof(spec));
  g_cdd_strdup_fail = 4;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;

  json_value_free(jv);

  /* Root extensions failure */
  jv = json_parse_string(
      "{\"openapi\": \"3.1.0\", \"info\": {\"title\": \"T\", \"version\": "
      "\"1.0\"}, \"x-custom\": \"val\", \"paths\": {}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  g_cdd_fail_extensions_init = 1;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_extensions_init = 0;
  json_value_free(jv);

  /* Paths extensions failure */
  paths_ext =
      "{\"openapi\": \"3.1.0\", \"info\": {\"title\": \"T\", \"version\": "
      "\"1.0\"}, \"paths\": {\"x-paths-custom\": \"val\", \"/p\": {\"get\": "
      "{\"responses\": {\"200\": {\"description\": \"ok\"}}}}}}";
  jv = json_parse_string(paths_ext);
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  g_cdd_fail_extensions_init = 3;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_extensions_init = 0;
  json_value_free(jv);

  /* Webhooks extensions failure */
  webhooks_ext =
      "{\"openapi\": \"3.1.0\", \"info\": {\"title\": \"T\", \"version\": "
      "\"1.0\"}, \"webhooks\": {\"x-web-custom\": \"val\", \"hook\": "
      "{\"post\": {\"responses\": {\"200\": {\"description\": \"ok\"}}}}}}";
  jv = json_parse_string(webhooks_ext);
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  g_cdd_fail_extensions_init = 3;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_extensions_init = 0;
  json_value_free(jv);

  /* Components extensions failure */
  comps_ext =
      "{\"openapi\": \"3.1.0\", \"info\": {\"title\": \"T\", \"version\": "
      "\"1.0\"}, \"components\": {\"x-comp-custom\": \"val\"}}";
  jv = json_parse_string(comps_ext);
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  g_cdd_fail_extensions_init = 3;
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  g_cdd_fail_extensions_init = 0;
  json_value_free(jv);

  PASS();
}

TEST test_openapi_load_extensions_and_validation_errors(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_DocRegistry reg;
  JSON_Value *jv;
  cdd_c_error_t rc;

  /* No paths, webhooks, or components */
  jv = json_parse_string("{\"openapi\": \"3.1.0\", \"info\": {\"title\": "
                         "\"NoPaths\", \"version\": \"1.0\"}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  json_value_free(jv);

  /* Path template collision */
  jv = json_parse_string(
      "{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"T\", \"version\": "
      "\"1.0\"},"
      " \"paths\": {\"/users/{id}\": {\"get\": {\"responses\": {\"200\": "
      "{\"description\": \"ok\"}}}},"
      "            \"/users/{name}\": {\"get\": {\"responses\": {\"200\": "
      "{\"description\": \"ok\"}}}}}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Query string usage error in paths: multiple querystring params */
  jv = json_parse_string(
      "{\"openapi\": \"3.2.0\", \"info\": {\"title\": \"T\", \"version\": "
      "\"1.0\"},"
      " \"paths\": {\"/users\": {\"parameters\": ["
      "  {\"name\": \"qs1\", \"in\": \"querystring\", \"content\": "
      "{\"application/x-www-form-urlencoded\": {\"schema\": {\"type\": "
      "\"object\"}}}},"
      "  {\"name\": \"qs2\", \"in\": \"querystring\", \"content\": "
      "{\"application/x-www-form-urlencoded\": {\"schema\": {\"type\": "
      "\"object\"}}}}"
      " ], \"get\": {\"responses\": {\"200\": {\"description\": \"ok\"}}}}}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Query string usage in webhooks: multiple querystring params */
  jv = json_parse_string(
      "{\"openapi\": \"3.2.0\", \"info\": {\"title\": \"T\", \"version\": "
      "\"1.0\"},"
      " \"webhooks\": {\"webhook\": {\"parameters\": ["
      "  {\"name\": \"qs1\", \"in\": \"querystring\", \"content\": "
      "{\"application/x-www-form-urlencoded\": {\"schema\": {\"type\": "
      "\"object\"}}}},"
      "  {\"name\": \"qs2\", \"in\": \"querystring\", \"content\": "
      "{\"application/x-www-form-urlencoded\": {\"schema\": {\"type\": "
      "\"object\"}}}}"
      " ], \"post\": {\"responses\": {\"200\": {\"description\": \"ok\"}}}}}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Query string usage in component_path_items: multiple querystring params */
  jv = json_parse_string(
      "{\"openapi\": \"3.2.0\", \"info\": {\"title\": \"T\", \"version\": "
      "\"1.0\"},"
      " \"paths\": {\"/ok\": {\"get\": {\"responses\": {\"200\": "
      "{\"description\": \"ok\"}}}}},"
      " \"components\": {\"pathItems\": {\"item\": {\"parameters\": ["
      "  {\"name\": \"qs1\", \"in\": \"querystring\", \"content\": "
      "{\"application/x-www-form-urlencoded\": {\"schema\": {\"type\": "
      "\"object\"}}}},"
      "  {\"name\": \"qs2\", \"in\": \"querystring\", \"content\": "
      "{\"application/x-www-form-urlencoded\": {\"schema\": {\"type\": "
      "\"object\"}}}}"
      " ], \"get\": {\"responses\": {\"200\": {\"description\": \"ok\"}}}}}}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Query string violation in path callbacks */
  jv = json_parse_string(
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "p\":{\"get\":{\"callbacks\":{\"myCb\":{\"{$url}\":{\"parameters\":["
      "{\"name\":\"q1\",\"in\":\"querystring\",\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{}}}},"
      "{\"name\":\"q2\",\"in\":\"querystring\",\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{}}}}],"
      "\"post\":{}}}}}}}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Query string violation in webhook callbacks */
  jv = json_parse_string(
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"webhooks\":{\"hook\":{\"post\":{\"callbacks\":{\"myCb\":{\"{$url}\":{"
      "\"parameters\":["
      "{\"name\":\"q1\",\"in\":\"querystring\",\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{}}}},"
      "{\"name\":\"q2\",\"in\":\"querystring\",\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{}}}}],"
      "\"post\":{}}}}}}}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Query string violation in component path item callbacks */
  jv = json_parse_string(
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/ok\":{}},"
      "\"components\":{\"pathItems\":{\"item\":{\"get\":{\"callbacks\":{"
      "\"myCb\":{\"{$url}\":{\"parameters\":["
      "{\"name\":\"q1\",\"in\":\"querystring\",\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{}}}},"
      "{\"name\":\"q2\",\"in\":\"querystring\",\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{}}}}],"
      "\"post\":{}}}}}}}}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Query string violation in component callbacks */
  jv = json_parse_string(
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{\"/ok\":{}},"
      "\"components\":{\"callbacks\":{\"myCb\":{\"{$url}\":{\"parameters\":["
      "{\"name\":\"q1\",\"in\":\"querystring\",\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{}}}},"
      "{\"name\":\"q2\",\"in\":\"querystring\",\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{}}}}],"
      "\"post\":{}}}}}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Duplicate operationId validation failure */
  jv = json_parse_string(
      "{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"T\", \"version\": "
      "\"1.0\"},"
      " \"paths\": {\"/a\": {\"get\": {\"operationId\": \"dupOp\", "
      "\"responses\": {\"200\": {\"description\": \"ok\"}}}},"
      "            \"/b\": {\"post\": {\"operationId\": \"dupOp\", "
      "\"responses\": {\"200\": {\"description\": \"ok\"}}}}}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json(jv, &spec);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* Registry add failure with OpenAPI spec (duplicate self_uri in registry) */
  openapi_doc_registry_init(&reg);
  jv = json_parse_string(
      "{\"openapi\": \"3.0.0\", \"$self\": \"http://example.com/api.json\","
      " \"info\": {\"title\": \"RegTest\", \"version\": \"1.0\"},"
      " \"paths\": {}}");
  ASSERT_NEQ(NULL, jv);
  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json_with_context(jv, NULL, &spec, &reg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);

  memset(&spec, 0, sizeof(spec));
  rc = openapi_load_from_json_with_context(jv, NULL, &spec, &reg);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  json_value_free(jv);
  openapi_doc_registry_free(&reg);

  PASS();
}

#define OPENAPI_TOP_LEVEL_COVERAGE_TESTS()                                     \
  RUN_TEST(test_openapi_null_and_invalid_args);                                \
  RUN_TEST(test_openapi_spec_find_schema_branches);                            \
  RUN_TEST(test_openapi_spec_find_schema_by_id_branches);                      \
  RUN_TEST(test_openapi_spec_find_schema_by_anchor_branches);                  \
  RUN_TEST(test_openapi_spec_find_schema_for_ref_branches);                    \
  RUN_TEST(test_openapi_schema_document_extra_branches);                       \
  RUN_TEST(test_openapi_swagger_oom_and_specs);                                \
  RUN_TEST(test_openapi_oauth2_and_extensions_branches);                       \
  RUN_TEST(test_openapi_load_extensions_and_validation_errors)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_TOP_LEVEL_COVERAGE_H */
