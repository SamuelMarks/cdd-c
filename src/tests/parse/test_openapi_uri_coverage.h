/**
 * @file test_openapi_uri_coverage.h
 * @brief Comprehensive 100% test coverage for openapi_uri.c.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_URI_COVERAGE_H
#define TEST_OPENAPI_URI_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
#include "openapi/parse/openapi_test_helpers.h"
#include "openapi/parse/openapi_internal_components.h"
/* clang-format on */

#ifndef RESOLVED_REF_TARGET_DEF
#define RESOLVED_REF_TARGET_DEF
struct ResolvedRefTarget {
  const struct OpenAPI_Spec *spec;
  const char *ref;
  char *resolved_ref;
};
#endif

extern C_CDD_EXPORT int g_cdd_strdup_fail;
extern C_CDD_EXPORT int g_cdd_alloc_fail;

/**
 * @brief Test basic URI helper functions in openapi_uri.c.
 */
TEST test_openapi_uri_basics(void) {
  char *str = NULL;
  size_t ulen = 0;
  cdd_c_error_t rc = 0;

  /* 1. json_pointer_unescape */
  rc = cdd_test_json_pointer_unescape(NULL, &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str);

  rc = cdd_test_json_pointer_unescape("a~0b~1c~2d", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("a~b/c~2d", str);
  free(str);
  str = NULL;

  /* json_pointer_unescape OOM */
  g_cdd_alloc_fail = 1;
  rc = cdd_test_json_pointer_unescape("test", &str);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str);

  /* 2. uri_base_len */
  rc = cdd_test_uri_base_len(NULL, &ulen);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, ulen);

  rc = cdd_test_uri_base_len("http://example.com/api#fragment", &ulen);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(strlen("http://example.com/api"), ulen);

  /* 3. uri_scheme_len */
  rc = cdd_test_uri_scheme_len(NULL, 0, &ulen);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, ulen);

  rc = cdd_test_uri_scheme_len("http://example.com", 0, &ulen);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, ulen);

  rc = cdd_test_uri_scheme_len("http://example.com",
                               strlen("http://example.com"), &ulen);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(4, ulen);

  /* uri_scheme_len stopping on slash, query, hash */
  rc = cdd_test_uri_scheme_len("/path/test", 10, &ulen);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, ulen);

  rc = cdd_test_uri_scheme_len("?query", 6, &ulen);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, ulen);

  rc = cdd_test_uri_scheme_len("#hash", 5, &ulen);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, ulen);

  /* 4. dup_substr */
  rc = cdd_test_dup_substr(NULL, 10, &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str);

  rc = cdd_test_dup_substr("hello world", 5, &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("hello", str);
  free(str);
  str = NULL;

  g_cdd_alloc_fail = 1;
  rc = cdd_test_dup_substr("hello world", 5, &str);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str);

  PASS();
}

/**
 * @brief Test normalize_path branches and edge cases.
 */
TEST test_openapi_uri_normalize_path_branches(void) {
  char *str = NULL;
  cdd_c_error_t rc = 0;

  /* NULL path */
  rc = cdd_test_normalize_path(NULL, &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str);

  /* Empty string */
  rc = cdd_test_normalize_path("", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("", str);
  free(str);
  str = NULL;

  /* Absolute root "/" */
  rc = cdd_test_normalize_path("/", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("/", str);
  free(str);
  str = NULL;

  /* Absolute "//" */
  rc = cdd_test_normalize_path("//", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("/", str);
  free(str);
  str = NULL;

  /* Absolute "/." */
  rc = cdd_test_normalize_path("/.", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("/", str);
  free(str);
  str = NULL;

  /* Absolute "/.." */
  rc = cdd_test_normalize_path("/..", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("/", str);
  free(str);
  str = NULL;

  /* Trailing slash with single segment */
  rc = cdd_test_normalize_path("/a/", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("/a/", str);
  free(str);
  str = NULL;

  /* Relative path with trailing slash */
  rc = cdd_test_normalize_path("a/b/", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("a/b/", str);
  free(str);
  str = NULL;

  /* Relative path with leading "../" */
  rc = cdd_test_normalize_path("../../a", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("../../a", str);
  free(str);
  str = NULL;

  /* Normalize path OOM conditions */
  g_cdd_alloc_fail = 1;
  rc = cdd_test_normalize_path("a/b/c", &str);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str);

  PASS();
}

/**
 * @brief Test resolve_uri_reference branches.
 */
TEST test_openapi_uri_resolve_reference_branches(void) {
  char *str = NULL;
  cdd_c_error_t rc = 0;

  /* NULL ref */
  rc = cdd_test_resolve_uri_reference("http://example.com", NULL, &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str);

  /* Protocol-relative ref "//cdn.example.com/lib.js" */
  rc = cdd_test_resolve_uri_reference("https://example.com/api",
                                      "//cdn.example.com/lib.js", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("https://cdn.example.com/lib.js", str);
  free(str);
  str = NULL;

  /* Protocol-relative ref with base without scheme */
  rc = cdd_test_resolve_uri_reference("no_scheme_base",
                                      "//cdn.example.com/lib.js", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("//cdn.example.com/lib.js", str);
  free(str);
  str = NULL;

  /* Base URI with scheme authority and no path */
  rc = cdd_test_resolve_uri_reference("https://example.com", "api/v1", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("https://example.com/api/v1", str);
  free(str);
  str = NULL;

  /* Base URI with root directory */
  rc = cdd_test_resolve_uri_reference("https://example.com/", "api/v1", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("https://example.com/api/v1", str);
  free(str);
  str = NULL;

  /* Absolute path ref on base URI */
  rc = cdd_test_resolve_uri_reference("https://example.com/api/v1", "/root/doc",
                                      &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("https://example.com/root/doc", str);
  free(str);
  str = NULL;

  PASS();
}

/**
 * @brief Test compute_document_uri branches.
 */
TEST test_openapi_uri_compute_document_uri_branches(void) {
  char *str = NULL;
  cdd_c_error_t rc = 0;

  /* Both NULL or empty */
  rc = cdd_test_compute_document_uri(NULL, NULL, &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str);

  rc = cdd_test_compute_document_uri("", "", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str);

  /* Only self_uri */
  rc = cdd_test_compute_document_uri("https://example.com/schema.json", NULL,
                                     &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("https://example.com/schema.json", str);
  free(str);
  str = NULL;

  /* Only retrieval_uri */
  rc = cdd_test_compute_document_uri(
      NULL, "https://example.com/retrieved.json#frag", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("https://example.com/retrieved.json", str);
  free(str);
  str = NULL;

  /* Both self_uri and retrieval_uri */
  rc = cdd_test_compute_document_uri("sub/schema.json#frag",
                                     "https://example.com/api/root.json", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("https://example.com/api/sub/schema.json", str);
  free(str);
  str = NULL;

  /* OOM branches */
  g_cdd_strdup_fail = 1;
  rc = cdd_test_compute_document_uri("https://example.com", NULL, &str);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  PASS();
}

/**
 * @brief Test schema document detection, storage, and ref resolution.
 */
TEST test_openapi_uri_schema_and_ref_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_DocRegistry reg;
  struct OpenAPI_DocRegistryEntry entry;
  struct ResolvedRefTarget tgt;
  char *name = NULL;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* 1. root_has_openapi_fields */
  rc = cdd_test_root_has_openapi_fields(NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  jv = json_parse_string("{\"servers\": []}");
  jo = json_value_get_object(jv);
  rc = cdd_test_root_has_openapi_fields(jo);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  jv = json_parse_string("{\"webhooks\": {}}");
  jo = json_value_get_object(jv);
  rc = cdd_test_root_has_openapi_fields(jo);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  jv = json_parse_string("{\"tags\": []}");
  jo = json_value_get_object(jv);
  rc = cdd_test_root_has_openapi_fields(jo);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  jv = json_parse_string("{\"security\": []}");
  jo = json_value_get_object(jv);
  rc = cdd_test_root_has_openapi_fields(jo);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  jv = json_parse_string("{\"externalDocs\": {}}");
  jo = json_value_get_object(jv);
  rc = cdd_test_root_has_openapi_fields(jo);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  jv = json_parse_string("{\"$self\": \"uri\"}");
  jo = json_value_get_object(jv);
  rc = cdd_test_root_has_openapi_fields(jo);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* 2. root_is_schema_document */
  rc = cdd_test_root_is_schema_document(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  jv = json_parse_string("true");
  rc = cdd_test_root_is_schema_document(jv, json_value_get_object(jv));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  json_value_free(jv);

  jv = json_parse_string("123");
  rc = cdd_test_root_is_schema_document(jv, json_value_get_object(jv));
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* 3. store_schema_root_json */
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_store_schema_root_json(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  jv = json_parse_string("{\"type\": \"string\"}");
  rc = cdd_test_store_schema_root_json(&spec, jv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  /* second call when already stored */
  rc = cdd_test_store_schema_root_json(&spec, jv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 4. resolve_ref_target */
  memset(&spec, 0, sizeof(spec));
  rc = cdd_test_resolve_ref_target(NULL, "#/components/schemas/Pet", &tgt);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Local ref #/frag without base */
  rc = cdd_test_resolve_ref_target(&spec, "#/frag", &tgt);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Target in registry */
  {
    char b_uri[] = "https://example.com/models.json";
    char d_uri[] = "https://example.com/api.json";
    memset(&reg, 0, sizeof(reg));
    memset(&entry, 0, sizeof(entry));
    entry.base_uri = b_uri;
    entry.spec = &spec;
    reg.entries = &entry;
    reg.count = 1;
    spec.doc_registry = &reg;
    spec.document_uri = d_uri;

    rc = cdd_test_resolve_ref_target(&spec, "models.json#/Pet", &tgt);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(&spec, tgt.spec);
    if (tgt.resolved_ref)
      free(tgt.resolved_ref);
    spec.doc_registry = NULL;
    spec.document_uri = NULL;
  }

  /* 5. ref_base_matches_self */
  rc = cdd_test_ref_base_matches_self(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  {
    const char ex_frag[] = "https://example.com#frag";
    rc = cdd_test_ref_base_matches_self(&spec, ex_frag, ex_frag);
  }
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  rc = cdd_test_ref_base_matches_self(NULL, "https://example.com/api#frag",
                                      "#frag");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Relative document_uri match */
  {
    char doc_rel[] = "./models.json";
    spec.document_uri = doc_rel;
    rc = cdd_test_ref_base_matches_self(&spec, "dir/models.json#frag",
                                        strchr("dir/models.json#frag", '#'));
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    spec.document_uri = NULL;
  }

  /* Relative self_uri match */
  {
    char self_rel[] = "./common.json";
    spec.self_uri = self_rel;
    rc = cdd_test_ref_base_matches_self(&spec, "sub/common.json#frag",
                                        strchr("sub/common.json#frag", '#'));
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    spec.self_uri = NULL;
  }

  /* 6. ref_name_from_prefix */
  rc = cdd_test_ref_name_from_prefix(NULL, NULL, NULL, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, name);

  /* Matching prefix */
  rc = cdd_test_ref_name_from_prefix(&spec, "#/components/schemas/Dog",
                                     "#/components/schemas/", &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Dog", name);

  /* Prefix with empty name */
  rc = cdd_test_ref_name_from_prefix(&spec, "#/components/schemas/",
                                     "#/components/schemas/", &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, name);

  /* Prefix with slash in name */
  rc = cdd_test_ref_name_from_prefix(&spec, "#/components/schemas/Dog/Sub",
                                     "#/components/schemas/", &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, name);

  /* External ref with hash prefix */
  {
    char s_uri[] = "models.json";
    spec.self_uri = s_uri;
    rc = cdd_test_ref_name_from_prefix(&spec,
                                       "models.json#/components/schemas/Cat",
                                       "#/components/schemas/", &name);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("Cat", name);

    /* External ref with hash prefix but empty name */
    rc =
        cdd_test_ref_name_from_prefix(&spec, "models.json#/components/schemas/",
                                      "#/components/schemas/", &name);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, name);

    /* External ref with slash in name */
    rc = cdd_test_ref_name_from_prefix(&spec,
                                       "models.json#/components/schemas/A/B",
                                       "#/components/schemas/", &name);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, name);
    spec.self_uri = NULL;
  }

  PASS();
}

extern C_CDD_EXPORT int g_cdd_fail_json_serialize;

/**
 * @brief More targeted branch coverage tests for openapi_uri.c.
 */
TEST test_openapi_uri_advanced_branches(void) {
  struct OpenAPI_Spec spec;
  char *str = NULL;
  cdd_c_error_t rc = 0;

  /* 1. json_pointer_unescape with '~' at end of string */
  rc = cdd_test_json_pointer_unescape("abc~", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("abc~", str);
  free(str);
  str = NULL;

  /* 2. normalize_path:
     - relative path with .. growth requiring realloc of segments
     - absolute path requiring root "/" OOM
     - path with multiple trailing slashes and absolute /
  */
  {
    /* relative path with multiple .. requiring capacity expansion (cap 4 -> 8)
     */
    rc = cdd_test_normalize_path("../../../../../a", &str);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("../../../../../a", str);
    free(str);
    str = NULL;

    /* normalize_path OOM on root "/" when count == 0 and absolute == 1 */
    g_cdd_strdup_fail = 1;
    rc = cdd_test_normalize_path("/", &str);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, str);

    /* normalize_path OOM on empty "" when count == 0 and absolute == 0 */
    g_cdd_strdup_fail = 1;
    rc = cdd_test_normalize_path(".", &str);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, str);

    /* normalize_path with trailing slash on absolute path with 1 segment: "/a/"
     */
    rc = cdd_test_normalize_path("/a/", &str);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("/a/", str);
    free(str);
    str = NULL;
  }

  /* 3. resolve_uri_reference:
     - base_uri without path or trailing slash, but prefix_len > 0 and
     base_dir_len == 0
  */
  {
    /* e.g. "https://example.com" with "sub" where base_dir_len == 0 and
     * prefix_len > 0 -> base_dir = "/" */
    rc = cdd_test_resolve_uri_reference("https://example.com", "sub", &str);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("https://example.com/sub", str);
    free(str);
    str = NULL;

    /* protocol relative ref OOM on malloc */
    g_cdd_alloc_fail = 1;
    rc = cdd_test_resolve_uri_reference("https://example.com/api",
                                        "//cdn.com/lib", &str);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, str);
  }

  /* 4. store_schema_root_json OOM on serialize */
  {
    JSON_Value *jv = json_parse_string("{\"type\": \"object\"}");
    memset(&spec, 0, sizeof(spec));
    g_cdd_fail_json_serialize = 1;
    rc = cdd_test_store_schema_root_json(&spec, jv);
    g_cdd_fail_json_serialize = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    json_value_free(jv);
  }

  /* 5. ref_base_matches_self combinations:
     - document_uri relative match branches:
       - rel == "/"
       - base_len == rel_len
       - slash before match
     - self_uri relative match branches:
       - self_base[0] == '/'
       - base_len == self_len
       - self_base reduction to empty
  */
  {
    char doc_exact[] = "models.json";
    char doc_slash[] = "/models.json";
    char self_exact[] = "common.json";
    char self_slash[] = "/common.json";
    char self_dots[] = "././";

    memset(&spec, 0, sizeof(spec));

    /* base_len == rel_len */
    spec.document_uri = doc_exact;
    {
      const char mf[] = "models.json#frag";
      rc = cdd_test_ref_base_matches_self(&spec, mf, strchr(mf, '#'));
    }
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

    /* rel[0] == '/' */
    spec.document_uri = doc_slash;
    rc = cdd_test_ref_base_matches_self(&spec, "dir/models.json#frag",
                                        strchr("dir/models.json#frag", '#'));
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    spec.document_uri = NULL;

    /* self_base[0] == '/' */
    spec.self_uri = self_slash;
    rc = cdd_test_ref_base_matches_self(&spec, "dir/common.json#frag",
                                        strchr("dir/common.json#frag", '#'));
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

    /* base_len == self_len */
    spec.self_uri = self_exact;
    {
      const char cf[] = "common.json#frag";
      rc = cdd_test_ref_base_matches_self(&spec, cf, strchr(cf, '#'));
    }
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

    /* self_len == 0 after stripping ./ */
    spec.self_uri = self_dots;
    rc = cdd_test_ref_base_matches_self(&spec, "something#frag",
                                        strchr("something#frag", '#'));
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.self_uri = NULL;
  }

  PASS();
}

/**
 * @brief Test final missing branches in openapi_uri.c.
 */
TEST test_openapi_uri_final_edges(void) {
  struct OpenAPI_Spec spec;
  char *str = NULL;
  cdd_c_error_t rc = 0;

  /* 1. normalize_path:
     - path with multiple slashes "a//b"
     - realloc failure in segments
  */
  rc = cdd_test_normalize_path("a//b", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("a/b", str);
  free(str);
  str = NULL;

  /* 2. ref_base_matches_self with slash before match:
     - ref = "prefix/models.json#frag" and base_uri = "models.json"
       base_len (18) > rel_len (11), ref[18 - 11 - 1] = ref[6] = '/' -> MATCH!
  */
  {
    char doc[] = "models.json";
    char self_doc[] = "common.json";
    memset(&spec, 0, sizeof(spec));

    spec.document_uri = doc;
    rc = cdd_test_ref_base_matches_self(&spec, "prefix/models.json#frag",
                                        strchr("prefix/models.json#frag", '#'));
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    spec.document_uri = NULL;

    spec.self_uri = self_doc;
    rc = cdd_test_ref_base_matches_self(&spec, "prefix/common.json#frag",
                                        strchr("prefix/common.json#frag", '#'));
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    spec.self_uri = NULL;
  }

  /* 3. resolve_uri_reference where base_uri has scheme but prefix_len > 0 and
   * base_path has no trailing slash */
  rc = cdd_test_resolve_uri_reference("http://example.com/api", "v1", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("http://example.com/v1", str);
  free(str);
  str = NULL;

  PASS();
}

/**
 * @brief Test realloc failure in normalize_path and edge cases.
 */
TEST test_openapi_uri_exhaust_branches(void) {
  struct OpenAPI_Spec spec;
  struct ResolvedRefTarget tgt;
  char *str = NULL;
  cdd_c_error_t rc = 0;

  /* 1. normalize_path realloc failure on '..' when absolute == 0 */
  {
    /* Path with 5 components requiring realloc from cap 4 to 8 */
    g_cdd_alloc_fail = 2;
    rc = cdd_test_normalize_path("a/b/c/d/e", &str);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, str);

    /* realloc failure when pushing '..' */
    g_cdd_alloc_fail = 2;
    rc = cdd_test_normalize_path("../../../../..", &str);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, str);
  }

  /* 2. resolve_ref_target:
     - spec->document_uri is non-null but resolve_uri_reference returns NULL
     (e.g. OOM)
     - resolved_base != base_part
  */
  {
    char doc_uri[] = "https://example.com/dir/api.json";
    memset(&spec, 0, sizeof(spec));
    spec.document_uri = doc_uri;

    /* OOM on dup_substr */
    g_cdd_alloc_fail = 1;
    rc = cdd_test_resolve_ref_target(&spec, "models.json#/Pet", &tgt);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    /* OOM on resolve_uri_reference (falls back to base_part) */
    g_cdd_alloc_fail = 2;
    rc = cdd_test_resolve_ref_target(&spec, "models.json#/Pet", &tgt);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(tgt.resolved_ref);

    /* Normal resolution where resolved_base != ref base */
    rc = cdd_test_resolve_ref_target(&spec, "models.json#/Pet", &tgt);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_NEQ(NULL, tgt.resolved_ref);
    free(tgt.resolved_ref);
    spec.document_uri = NULL;
  }

  /* 3. root_is_schema_document with openapi and swagger properties */
  {
    JSON_Value *jv_oa = json_parse_string("{\"openapi\": \"3.0.0\"}");
    JSON_Value *jv_sw = json_parse_string("{\"swagger\": \"2.0\"}");
    JSON_Value *jv_info = json_parse_string("{\"info\": {}}");

    rc = cdd_test_root_is_schema_document(jv_oa, json_value_get_object(jv_oa));
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = cdd_test_root_is_schema_document(jv_sw, json_value_get_object(jv_sw));
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = cdd_test_root_is_schema_document(jv_info,
                                          json_value_get_object(jv_info));
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    json_value_free(jv_oa);
    json_value_free(jv_sw);
    json_value_free(jv_info);
  }

  /* 4. ref_name_from_prefix NULL arguments */
  {
    char *out_name = NULL;
    rc = cdd_test_ref_name_from_prefix(&spec, NULL, "#/prefix/", &out_name);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, out_name);

    rc = cdd_test_ref_name_from_prefix(&spec, "#/something", NULL, &out_name);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, out_name);
  }

  PASS();
}

/**
 * @brief Test exact non-matching suffix paths for ref_base_matches_self.
 */
TEST test_openapi_uri_non_matching_relative(void) {
  struct OpenAPI_Spec spec;
  cdd_c_error_t rc = 0;

  /* document_uri with non-matching suffix:
     base_len >= rel_len, but strncmp != 0
  */
  {
    char doc[] = "models.json";
    char self_doc[] = "common.json";
    memset(&spec, 0, sizeof(spec));

    spec.document_uri = doc;
    rc = cdd_test_ref_base_matches_self(
        &spec, "prefix/different.json#frag",
        strchr("prefix/different.json#frag", '#'));
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.document_uri = NULL;

    spec.self_uri = self_doc;
    rc = cdd_test_ref_base_matches_self(
        &spec, "prefix/different.json#frag",
        strchr("prefix/different.json#frag", '#'));
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.self_uri = NULL;
  }

  PASS();
}

/**
 * @brief Test components check, self_uri with hash, and opaque base in
 * openapi_uri.c.
 */
TEST test_openapi_uri_final_unhit_branches(void) {
  struct OpenAPI_Spec spec;
  char *str = NULL;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* 1. root_has_openapi_fields with "components" */
  jv = json_parse_string("{\"components\": {}}");
  jo = json_value_get_object(jv);
  rc = cdd_test_root_has_openapi_fields(jo);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* 2. ref_base_matches_self with spec->self_uri containing '#' */
  {
    char self_with_hash[] = "models.json#MySelf";
    memset(&spec, 0, sizeof(spec));
    spec.self_uri = self_with_hash;
    {
      const char mf[] = "models.json#frag";
      rc = cdd_test_ref_base_matches_self(&spec, mf, strchr(mf, '#'));
    }
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    spec.self_uri = NULL;
  }

  /* 3. resolve_uri_reference where base_dir_len == 0 (no prefix and no path) */
  rc = cdd_test_resolve_uri_reference("urn:isbn", "12345", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("12345", str);
  free(str);
  str = NULL;

  PASS();
}

/**
 * @brief Test all 10 remaining branch combinations for openapi_uri.c.
 */
TEST test_openapi_uri_100_percent_final(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_DocRegistry reg;
  struct OpenAPI_DocRegistryEntry entries[2];
  struct ResolvedRefTarget tgt;
  char *str = NULL;
  cdd_c_error_t rc = 0;

  /* 1. normalize_path: 5 regular segments to grow cap from 4 to 8 */
  rc = cdd_test_normalize_path("a/b/c/d/e", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("a/b/c/d/e", str);
  free(str);
  str = NULL;

  /* 2. resolve_uri_reference: ref_len < 2, e.g. "/" */
  rc = cdd_test_resolve_uri_reference("https://example.com/api", "/", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("https://example.com/", str);
  free(str);
  str = NULL;

  /* 3. compute_document_uri: self_uri non-empty and retrieval_uri empty string
   * "" */
  rc = cdd_test_compute_document_uri("https://example.com/self", "", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("https://example.com/self", str);
  free(str);
  str = NULL;

  /* 4. store_schema_root_json: spec == NULL vs root == NULL */
  {
    JSON_Value *jv = json_parse_string("123");
    memset(&spec, 0, sizeof(spec));
    rc = cdd_test_store_schema_root_json(NULL, jv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_store_schema_root_json(&spec, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    json_value_free(jv);
  }

  /* 5. resolve_ref_target:
     - spec->document_uri is empty ""
     - registry entry with base_uri == NULL
     - resolved_base == base_part (resolved_len == base_len and strncmp == 0)
  */
  {
    char empty_doc[] = "";
    char mod_base[] = "https://example.com/models.json";
    memset(&spec, 0, sizeof(spec));
    spec.document_uri = empty_doc;
    rc = cdd_test_resolve_ref_target(&spec, "models.json#/Pet", &tgt);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(tgt.resolved_ref);

    /* Registry with entry having base_uri == NULL */
    memset(&reg, 0, sizeof(reg));
    memset(entries, 0, sizeof(entries));
    entries[0].base_uri = NULL;
    entries[0].spec = &spec;
    entries[1].base_uri = mod_base;
    entries[1].spec = &spec;
    reg.entries = entries;
    reg.count = 2;
    spec.doc_registry = &reg;
    spec.document_uri = mod_base;

    rc = cdd_test_resolve_ref_target(
        &spec, "https://example.com/models.json#/Pet", &tgt);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(&spec, tgt.spec);
    if (tgt.resolved_ref)
      free(tgt.resolved_ref);
    spec.doc_registry = NULL;
    spec.document_uri = NULL;
  }

  /* 6. ref_base_matches_self:
     - spec->document_uri is empty ""
     - spec->self_uri is empty ""
  */
  {
    char empty_str[] = "";
    memset(&spec, 0, sizeof(spec));
    spec.document_uri = empty_str;
    rc = cdd_test_ref_base_matches_self(&spec, "something#frag",
                                        strchr("something#frag", '#'));
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    spec.document_uri = NULL;
    spec.self_uri = empty_str;
    rc = cdd_test_ref_base_matches_self(&spec, "something#frag",
                                        strchr("something#frag", '#'));
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.self_uri = NULL;
  }

  PASS();
}

/**
 * @brief Test precise branch coverage targets in openapi_uri.c.
 */
TEST test_openapi_uri_branch_finishing(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_DocRegistry reg;
  struct OpenAPI_DocRegistryEntry entry;
  struct ResolvedRefTarget tgt;
  char *str = NULL;
  cdd_c_error_t rc = 0;

  /* 1. base_uri with scheme where scheme_len + 2 >= base_len (e.g. "http:" or
   * "http:/") */
  rc = cdd_test_resolve_uri_reference("http:", "path", &str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("path", str);
  free(str);
  str = NULL;

  /* 2. resolve_ref_target where resolved_len == base_len but strncmp != 0 */
  {
    char doc_uri[] = "https://example.com/dir/file.json";
    char reg_base[] = "https://example.com/dir/abcd.json";
    memset(&spec, 0, sizeof(spec));
    memset(&reg, 0, sizeof(reg));
    memset(&entry, 0, sizeof(entry));
    entry.base_uri = reg_base;
    entry.spec = &spec;
    reg.entries = &entry;
    reg.count = 1;
    spec.doc_registry = &reg;
    spec.document_uri = doc_uri;

    /* ref is "wxyz.json#/Pet" (len 9), resolved is
       "https://example.com/dir/wxyz.json" (different len), so let's make
       base_part same len as resolved: If doc_uri is NULL, resolved_base is
       base_part. Let's test resolve_ref_target with relative doc_uri:
    */
    rc = cdd_test_resolve_ref_target(&spec, "abcd.json#/Pet", &tgt);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    if (tgt.resolved_ref)
      free(tgt.resolved_ref);
    spec.doc_registry = NULL;
    spec.document_uri = NULL;
  }

  /* 3. ref_base_matches_self with ref != NULL and hash == NULL */
  rc = cdd_test_ref_base_matches_self(&spec, "no_hash_ref", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 4. ref_base_matches_self where uri_len == base_len but strncmp != 0 */
  {
    char doc[] = "models.json";
    memset(&spec, 0, sizeof(spec));
    spec.document_uri = doc;
    /* "common.json" has same len (11) as "models.json" */
    {
      const char cf[] = "common.json#frag";
      rc = cdd_test_ref_base_matches_self(&spec, cf, strchr(cf, '#'));
    }
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.document_uri = NULL;
  }

  /* 5. ref_base_matches_self where base_len < rel_len */
  {
    char doc[] = "prefix/models.json";
    memset(&spec, 0, sizeof(spec));
    spec.document_uri = doc;
    /* base_len ("m.json", 6) < rel_len (18) */
    rc = cdd_test_ref_base_matches_self(&spec, "m.json#frag",
                                        strchr("m.json#frag", '#'));
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.document_uri = NULL;

    /* And for self_uri with base_len < self_len */
    spec.self_uri = doc;
    rc = cdd_test_ref_base_matches_self(&spec, "m.json#frag",
                                        strchr("m.json#frag", '#'));
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.self_uri = NULL;
  }

  /* Edge case: suffix matches but not preceded by slash, not equal length, not
   * absolute */
  {
    char doc[] = "models.json";
    char self_dot_doc[] = "./common.json";
    memset(&spec, 0, sizeof(spec));

    spec.document_uri = doc;
    /* 'badmodels.json' ends with 'models.json' but char before is 'd' != '/' */
    rc = cdd_test_ref_base_matches_self(&spec, "badmodels.json#frag",
                                        strchr("badmodels.json#frag", '#'));
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.document_uri = NULL;

    spec.self_uri = self_dot_doc;
    rc = cdd_test_ref_base_matches_self(&spec, "badcommon.json#frag",
                                        strchr("badcommon.json#frag", '#'));
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.self_uri = NULL;
  }

  /* Test base_len == self_len after ./ stripping */
  {
    char self_dot[] = "./common.json";
    memset(&spec, 0, sizeof(spec));
    spec.self_uri = self_dot;
    {
      const char cf[] = "common.json#frag";
      rc = cdd_test_ref_base_matches_self(&spec, cf, strchr(cf, '#'));
    }
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    spec.self_uri = NULL;
  }

  /* 6. ref_base_matches_self where base_len == self_len but strncmp != 0 */
  {
    char self_doc[] = "models.json";
    memset(&spec, 0, sizeof(spec));
    spec.self_uri = self_doc;
    {
      const char cf[] = "common.json#frag";
      rc = cdd_test_ref_base_matches_self(&spec, cf, strchr(cf, '#'));
    }
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    spec.self_uri = NULL;
  }

  PASS();
}

#define OPENAPI_URI_COVERAGE_TESTS()                                           \
  RUN_TEST(test_openapi_uri_basics);                                           \
  RUN_TEST(test_openapi_uri_normalize_path_branches);                          \
  RUN_TEST(test_openapi_uri_resolve_reference_branches);                       \
  RUN_TEST(test_openapi_uri_compute_document_uri_branches);                    \
  RUN_TEST(test_openapi_uri_schema_and_ref_branches);                          \
  RUN_TEST(test_openapi_uri_advanced_branches);                                \
  RUN_TEST(test_openapi_uri_final_edges);                                      \
  RUN_TEST(test_openapi_uri_exhaust_branches);                                 \
  RUN_TEST(test_openapi_uri_non_matching_relative);                            \
  RUN_TEST(test_openapi_uri_final_unhit_branches);                             \
  RUN_TEST(test_openapi_uri_100_percent_final);                                \
  RUN_TEST(test_openapi_uri_branch_finishing)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_URI_COVERAGE_H */
