/**
 * @file test_cli_c2openapi_branches.h
 * @brief Unit tests for c2openapi final branches.
 */

#ifndef TEST_CLI_C2OPENAPI_BRANCHES_H
#define TEST_CLI_C2OPENAPI_BRANCHES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_test_helpers/cdd_helpers.h"
#include "c_cdd/memory.h"
#include "routes/parse/cli.h"
#include <greatest.h>
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifndef CDD_TEST_CLI_C2OPENAPI_HELPERS_DEFINED
#define CDD_TEST_CLI_C2OPENAPI_HELPERS_DEFINED
static int g_c2o_oom_fail_at = -1;
static void *mock_c2o_oom_malloc(size_t sz) {
  if (g_c2o_oom_fail_at == 0) {
    g_c2o_oom_fail_at = -1;
    return NULL;
  }
  if (g_c2o_oom_fail_at > 0)
    g_c2o_oom_fail_at--;
  return malloc(sz);
}
static void mock_c2o_oom_free(void *ptr) { free(ptr); }

static const char *get_mocks_dir(void) {
  FILE *f;
  f = fopen("src/tests/mocks/emit/simple.schema.json", "r");
  if (f) {
    fclose(f);
    return "src/tests/mocks";
  }
  return "../src/tests/mocks";
}

static const char *get_simple_schema(void) {
  FILE *f;
  f = fopen("src/tests/mocks/emit/simple.schema.json", "r");
  if (f) {
    fclose(f);
    return "src/tests/mocks/emit/simple.schema.json";
  }
  return "../src/tests/mocks/emit/simple.schema.json";
}
#endif /* CDD_TEST_CLI_C2OPENAPI_HELPERS_DEFINED */

TEST test_c2openapi_final_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Server srv;
  struct DocServer doc_srv;
  struct DocServerVar doc_vars[1];
  char *enum_vals[1];
  struct OpenAPI_OAuthFlow flow_dst;
  struct DocOAuthFlow doc_flow;
  struct DocOAuthScope doc_scopes[2];
  struct OpenAPI_SecurityScheme scheme;
  struct DocSecurityScheme doc_sec;
  struct DocMetadata meta;
  struct DocTagMeta doc_tag;
  struct OpenAPI_Operation op;
  char *argv[10];

  /* 1. line 303: empty string in apply_doc_global_meta */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.info_title = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));

  /* 2. line 396 & 412: sv->name strdup fail, and enum_values != NULL &&
   * n_enum_values == 0 */
  memset(&srv, 0, sizeof(srv));
  memset(&doc_srv, 0, sizeof(doc_srv));
  doc_srv.n_variables = 1;
  doc_srv.variables = doc_vars;
  memset(doc_vars, 0, sizeof(doc_vars));
  doc_vars[0].name = (char *)(size_t) "var";
  doc_vars[0].default_value = (char *)(size_t) "def";
  doc_vars[0].enum_values = enum_vals;
  doc_vars[0].n_enum_values = 0; /* line 412 false branch */
  ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables(&srv, &doc_srv));
  free_openapi_server_variables(&srv);

  g_cdd_strdup_fail = 1; /* line 396 */
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables(&srv, &doc_srv));
  g_cdd_strdup_fail = 0;

  /* 3. line 472: merge_scopes with scope name == NULL matching non-null */
  memset(&flow_dst, 0, sizeof(flow_dst));
  flow_dst.n_scopes = 1;
  flow_dst.scopes =
      (struct OpenAPI_OAuthScope *)calloc(1, sizeof(struct OpenAPI_OAuthScope));
  flow_dst.scopes[0].name = (char *)(size_t)strdup("read");
  memset(&doc_flow, 0, sizeof(doc_flow));
  doc_flow.n_scopes = 1;
  doc_flow.scopes = doc_scopes;
  memset(doc_scopes, 0, sizeof(doc_scopes));
  doc_scopes[0].name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, merge_scopes(&flow_dst, &doc_flow));
  free(flow_dst.scopes[0].name);
  free(flow_dst.scopes[1].name);
  free(flow_dst.scopes);

  /* 4. line 657, 678, 688: add_oauth_flows */
  memset(&scheme, 0, sizeof(scheme));
  memset(&doc_sec, 0, sizeof(doc_sec));
  doc_sec.n_flows = 1;
  doc_sec.flows = &doc_flow;
  memset(&doc_flow, 0, sizeof(doc_flow));
  doc_flow.type = DOC_OAUTH_FLOW_IMPLICIT;
  doc_flow.authorization_url = (char *)(size_t) "http://auth";
  doc_flow.scopes = doc_scopes;
  doc_flow.n_scopes = 0; /* line 678 false */
  g_cdd_strdup_fail = 1; /* line 657 */
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_oauth_flows(&scheme, &doc_sec));
  g_cdd_strdup_fail = 0;
  doc_flow.n_scopes = 1;
  doc_scopes[0].name = NULL; /* line 688 name ? name : "" */
  ASSERT_EQ(CDD_C_SUCCESS, add_oauth_flows(&scheme, &doc_sec));
  if (scheme.flows) {
    if (scheme.flows[0].authorization_url)
      free(scheme.flows[0].authorization_url);
    if (scheme.flows[0].scopes) {
      if (scheme.flows[0].scopes[0].name)
        free(scheme.flows[0].scopes[0].name);
      free(scheme.flows[0].scopes);
    }
    free(scheme.flows);
    scheme.flows = NULL;
    scheme.n_flows = 0;
  }

  /* 5. line 774: OA_SEC_MUTUALTLS, line 789: doc->scheme = "", line 804: openid
   * url = "", line 817: oauth2_metadata_url fail */
  memset(&spec, 0, sizeof(spec));
  memset(&doc_sec, 0, sizeof(doc_sec));
  doc_sec.name = (char *)(size_t) "MutualTlsSec";
  doc_sec.type = DOC_SEC_MUTUALTLS;
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));

  doc_sec.name = (char *)(size_t) "HttpEmptyScheme";
  doc_sec.type = DOC_SEC_HTTP;
  doc_sec.scheme = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));

  doc_sec.name = (char *)(size_t) "OpenIdEmptyUrl";
  doc_sec.type = DOC_SEC_OPENID;
  doc_sec.open_id_connect_url = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));

  doc_sec.name = (char *)(size_t) "OAuthMetaConflict";
  doc_sec.type = DOC_SEC_OAUTH2;
  doc_sec.oauth2_metadata_url = (char *)(size_t) "http://meta1";
  doc_sec.n_flows = 1;
  doc_sec.flows = &doc_flow;
  doc_flow.type = DOC_OAUTH_FLOW_CLIENT_CREDENTIALS;
  doc_flow.token_url = (char *)(size_t) "http://token";
  doc_flow.authorization_url = NULL;
  doc_flow.n_scopes = 0;
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));
  doc_sec.oauth2_metadata_url =
      (char *)(size_t) "http://meta2"; /* conflict -> line 817 */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));
  openapi_spec_free(&spec);

  /* 6. line 992, 1001, 1003, 1012, 1016: append_root_servers cleanups */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.n_servers = 1;
  meta.servers = &doc_srv;
  memset(&doc_srv, 0, sizeof(doc_srv));
  doc_srv.url = (char *)(size_t) "http://srv";
  doc_srv.name = (char *)(size_t) "name1";
  doc_srv.description = (char *)(size_t) "desc1";
  doc_srv.n_variables = 1;
  doc_srv.variables = doc_vars;
  memset(doc_vars, 0, sizeof(doc_vars));
  doc_vars[0].name = (char *)(size_t) "var1";
  doc_vars[0].default_value = (char *)(size_t) "def1";
  /* line 992: name strdup fail with url present */
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_servers(&spec, &meta));
  g_cdd_strdup_fail = 0;
  /* line 1001, 1003: description strdup fail with url and name present */
  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, append_root_servers(&spec, &meta));
  g_cdd_strdup_fail = 0;
  /* line 1012, 1016: copy_doc_server_variables fail with url, name, desc
   * present */
  doc_vars[0].name = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, append_root_servers(&spec, &meta));
  doc_vars[0].name = (char *)(size_t) "var1";
  openapi_spec_free(&spec);

  /* 7. line 1100: contact_email, line 1107: meta->license_name == NULL &&
   * spec->info.license.name != NULL, line 1160: append_root_security fail */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.contact_email = (char *)(size_t) "test@example.com";
  spec.info.license.name = (char *)(size_t)strdup("ExistingLic");
  meta.license_url = (char *)(size_t) "http://lic";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  /* line 1160: append_root_security fail */
  meta.n_security = 1;
  meta.security = (struct DocSecurityRequirement *)calloc(
      1, sizeof(struct DocSecurityRequirement));
  meta.security[0].scheme = (char *)(size_t) "oauth";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, apply_doc_global_meta(&spec, &meta));
  g_cdd_strdup_fail = 0;
  free(meta.security);
  openapi_spec_free(&spec);

  /* 8. line 1189: meta->name == "", line 1258: tag_meta != NULL && n_tag_meta
   * == 0, line 1262: spec_apply_tag_meta fail */
  memset(&spec, 0, sizeof(spec));
  memset(&doc_tag, 0, sizeof(doc_tag));
  doc_tag.name = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, &doc_tag));
  memset(&meta, 0, sizeof(meta));
  meta.tag_meta = &doc_tag;
  meta.n_tag_meta = 0; /* line 1258 false */
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_tag_meta(&spec, &meta));
  meta.n_tag_meta = 1;
  doc_tag.name = (char *)(size_t) "TagAllocFail";
  g_cdd_alloc_fail = 1; /* line 1262 */
  ASSERT_EQ(CDD_C_ERROR_MEMORY, apply_doc_tag_meta(&spec, &meta));
  g_cdd_alloc_fail = 0;
  openapi_spec_free(&spec);

  /* 9. line 1291: op->tags == NULL */
  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));
  op.tags = NULL;
  op.n_tags = 0;
  ASSERT_EQ(CDD_C_SUCCESS, collect_tags_from_op(&spec, &op));

  /* 10. process_file edge cases: line 1681, 1685, 1697, 1707, 1716, 1743 */
  {
    const char *fpath = "test_func_no_route.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      /* Function without route (line 1681 false), function at index 0 without
       * doc (line 1716) */
      fputs("void no_route_first(void) {}\n/**\n * @summary just comment\n "
            "*/\nvoid no_route_second(void) {}\n",
            fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      ASSERT_EQ(CDD_C_SUCCESS, process_file(fpath, &spec));
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* Webhook operation (line 1707) and sig parsing error (line 1697) */
  {
    const char *fpath = "test_webhook_and_bad_sig.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @route POST /hook\n * @webhook\n */\nvoid on_webhook(int "
            "x) {}\n/**\n * @route GET /badsig\n */\n123bad(void) {}\n",
            fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      ASSERT_EQ(CDD_C_SUCCESS, process_file(fpath, &spec));
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* 11. walker_cb line 1806 and 1816 */
  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, walker_cb(NULL, &spec));
  {
    const char *fpath = "test_walker_oom.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("void fn(void) {}\n", fp);
      fclose(fp);
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, walker_cb(fpath, &spec));
      g_cdd_alloc_fail = 0;
      remove(fpath);
    }
  }

  /* 12. c2openapi_cli_main line 1928 (-s), 1941, 1945 (self_uri fail), 1951
   * (*dialect_uri == "") */
  {
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t) "-s";
    argv[2] = (char *)(size_t) "http://s1";
    argv[3] = (char *)(size_t) "-s";
    argv[4] = (char *)(size_t) "http://s2"; /* line 1941 overwrite */
    argv[5] = (char *)(size_t) "--dialect";
    argv[6] = (char *)(size_t) ""; /* line 1951 empty */
    TEST_MKDIR("my_empty_dir");
    argv[7] = (char *)(size_t) "my_empty_dir";
    argv[8] = (char *)(size_t) "out.json";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_cli_main(9, argv));
    remove("out.json");

    /* self_uri strdup fail (line 1945) */
    argv[1] = (char *)(size_t) "--self";
    argv[2] = (char *)(size_t) "http://fail_self";
    argv[3] = (char *)(size_t) "my_empty_dir";
    argv[4] = (char *)(size_t) "out.json";
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(5, argv));
    g_cdd_strdup_fail = 0;
  }

  /* 13. to_docs_json_cli_main CDD_INPUT env and duplicate path route */
  {
    const char *tmp_spec = "test_docs_multi_op.json";
    FILE *fp = fopen(tmp_spec, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},\"paths\":{\"/"
            "r\":{\"get\":{\"operationId\":\"g\"},\"post\":{\"operationId\":"
            "\"p\"}}}}",
            fp);
      fclose(fp);
    }
    argv[0] = (char *)(size_t) "to_docs_json";
    argv[1] = (char *)(size_t) "-i";
    argv[2] = (char *)(size_t)tmp_spec;
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(3, argv));

#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("CDD_INPUT=test_docs_multi_op.json");
#else
    setenv("CDD_INPUT", "test_docs_multi_op.json", 1);
#endif
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(1, argv));
#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("CDD_INPUT=");
#else
    unsetenv("CDD_INPUT");
#endif

#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("INPUT_FILE=test_docs_multi_op.json");
#else
    setenv("INPUT_FILE", "test_docs_multi_op.json", 1);
#endif
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(1, argv));
#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("INPUT_FILE=");
#else
    unsetenv("INPUT_FILE");
#endif
    remove(tmp_spec);
  }

  /* 14. generate_bindings_cli_main short/long flags and missing checks */
  {
    struct TypeDefList types;
    argv[0] = (char *)(size_t) "bind";
    argv[1] = (char *)(size_t) "-i";
    argv[2] = (char *)(size_t) "in";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(3, argv));

    argv[3] = (char *)(size_t) "-o";
    argv[4] = (char *)(size_t) "out";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(5, argv));

    argv[5] = (char *)(size_t) "-l";
    argv[6] = (char *)(size_t) "py";
    argv[7] = (char *)(size_t) "-n";
    argv[8] = (char *)(size_t) "lib";
    argv[9] = (char *)(size_t) "-m";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(10, argv));

    /* long flags */
    argv[1] = (char *)(size_t) "--input";
    argv[2] = (char *)(size_t) "in";
    argv[3] = (char *)(size_t) "--output-dir";
    argv[4] = (char *)(size_t) "out";
    argv[5] = (char *)(size_t) "--lang";
    argv[6] = (char *)(size_t) "py";
    argv[7] = (char *)(size_t) "--lib-name";
    argv[8] = (char *)(size_t) "lib";
    argv[9] = (char *)(size_t) "--module-name";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(10, argv));

    /* all short valid flags */
    argv[1] = (char *)(size_t) "-i";
    argv[2] = (char *)(size_t) "in";
    argv[3] = (char *)(size_t) "-o";
    argv[4] = (char *)(size_t) "out";
    argv[5] = (char *)(size_t) "-l";
    argv[6] = (char *)(size_t) "py";
    argv[7] = (char *)(size_t) "-n";
    argv[8] = (char *)(size_t) "lib";
    argv[9] = (char *)(size_t) "-m";
    /* valid 7 args */
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(7, argv));

    memset(&types, 0, sizeof(types));
    types.items =
        (struct TypeDefinition *)calloc(1, sizeof(struct TypeDefinition));
    types.size = 1;
    types.capacity = 1;
    types.items[0].kind = KIND_ENUM;
    types.items[0].name = NULL;
    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_register_types(&spec, &types));
    free(types.items);
    openapi_spec_free(&spec);
  }

  /* 15. line 1084: contact_email without contact_name and contact_url */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.contact_email = (char *)(size_t) "only_email@test.com";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  openapi_spec_free(&spec);

  /* 16. line 1091: !meta->license_name && spec->info.license.name != NULL */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  spec.info.license.name =
      (char *)(size_t)c_cdd_strdup_macro("ExistingLicName");
  meta.license_url = (char *)(size_t) "http://license_url_only";
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  openapi_spec_free(&spec);

  /* 17. line 1173: append_root_security fail in apply_doc_global_meta */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.n_security = 1;
  meta.security = (struct DocSecurityRequirement *)calloc(
      1, sizeof(struct DocSecurityRequirement));
  meta.security[0].scheme = (char *)(size_t) "oauth";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, apply_doc_global_meta(&spec, &meta));
  g_cdd_strdup_fail = 0;
  free(meta.security);
  openapi_spec_free(&spec);

  /* 18. line 1385 and 1388: collect_spec_tags with paths and webhooks failure
   */
  memset(&spec, 0, sizeof(spec));
  spec.n_paths = 1;
  spec.paths = (struct OpenAPI_Path *)calloc(1, sizeof(struct OpenAPI_Path));
  spec.paths[0].n_operations = 1;
  spec.paths[0].operations =
      (struct OpenAPI_Operation *)calloc(1, sizeof(struct OpenAPI_Operation));
  spec.paths[0].operations[0].n_tags = 1;
  spec.paths[0].operations[0].tags = (char **)calloc(1, sizeof(char *));
  spec.paths[0].operations[0].tags[0] = (char *)(size_t) "PathTag";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, collect_spec_tags(&spec));
  g_cdd_strdup_fail = 0;

  spec.n_webhooks = 1;
  spec.webhooks = spec.paths;
  spec.n_paths = 0;
  spec.paths = NULL;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, collect_spec_tags(&spec));
  g_cdd_strdup_fail = 0;
  free(spec.webhooks[0].operations[0].tags);
  free(spec.webhooks[0].operations);
  free(spec.webhooks);
  spec.webhooks = NULL;
  spec.n_webhooks = 0;

  /* 19. process_file alloc fail loop to hit all allocation branches (sig_raw,
   * doc_text, comment_used) */
  {
    const char *fpath = "test_alloc_loop.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @route GET /alloc\n */\nvoid fn_alloc(int x) {}\n/**\n * "
            "Standalone comment\n */\n",
            fp);
      fclose(fp);
      {
        int k;
        for (k = 1; k <= 15; ++k) {
          memset(&spec, 0, sizeof(spec));
          g_cdd_alloc_fail = k;
          if (process_file(fpath, &spec) != CDD_C_SUCCESS) {
          }
          g_cdd_alloc_fail = 0;
          openapi_spec_free(&spec);
        }
      }
      remove(fpath);
    }
  }

  /* 20. process_file with bad C signature: int (int a) {} */
  {
    const char *fpath = "test_bad_csig.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @route GET /bad_csig\n */\nint (int a) {}\n", fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      ASSERT_EQ(CDD_C_SUCCESS, process_file(fpath, &spec));
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* 21. c2openapi_cli_main -s, --self overwrite, --dialect overwrite */
  {
    const char *base_path = "test_base_with_uris.json";
    FILE *fp = fopen(base_path, "w");
    if (fp) {
      fputs(
          "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
          "\"$self\":\"http://orig_self\",\"jsonSchemaDialect\":\"http://"
          "orig_dialect\",\"paths\":{}}",
          fp);
      fclose(fp);

      argv[0] = (char *)(size_t) "c2openapi";
      argv[1] = (char *)(size_t) "-b";
      argv[2] = (char *)(size_t)base_path;
      argv[3] = (char *)(size_t) "-s";
      argv[4] = (char *)(size_t) "http://new_self";
      argv[5] = (char *)(size_t) "--dialect";
      argv[6] = (char *)(size_t) "http://new_dialect";
      argv[7] = (char *)(size_t) "my_empty_dir";
      argv[8] = (char *)(size_t) "out_uri.json";
      ASSERT_EQ(CDD_C_SUCCESS, c2openapi_cli_main(9, argv));
      remove(base_path);
      remove("out_uri.json");
    }
  }

  /* 22. c2openapi_cli_main collect_spec_tags fail and write fail */
  {
    const char *base_path = "test_base_tags.json";
    FILE *fp = fopen(base_path, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},\"paths\":{\"/r\":{\"get\":{\"tags\":[\"T1\"]}}}}",
            fp);
      fclose(fp);

      argv[0] = (char *)(size_t) "c2openapi";
      argv[1] = (char *)(size_t) "--base";
      argv[2] = (char *)(size_t)base_path;
      argv[3] = (char *)(size_t) "my_empty_dir";
      argv[4] = (char *)(size_t) "out_tag.json";
      g_cdd_strdup_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(5, argv));
      g_cdd_strdup_fail = 0;
      remove(base_path);
    }
  }

  /* 23. to_docs_json_cli_main nonexistent file */
  {
    argv[0] = (char *)(size_t) "to_docs_json";
    argv[1] = (char *)(size_t) "-i";
    argv[2] = (char *)(size_t) "nonexistent_docs_9999.json";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, to_docs_json_cli_main(3, argv));
  }

  /* 24. generate_bindings_cli_main short options and missing arguments */
  {
    argv[0] = (char *)(size_t) "bind";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(1, argv));
    argv[1] = (char *)(size_t) "-i";
    argv[2] = (char *)(size_t) "in";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(3, argv));
    argv[3] = (char *)(size_t) "-o";
    argv[4] = (char *)(size_t) "out";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(5, argv));
    argv[5] = (char *)(size_t) "-l";
    argv[6] = (char *)(size_t) "py";
    argv[7] = (char *)(size_t) "-n";
    argv[8] = (char *)(size_t) "lib";
    argv[9] = (char *)(size_t) "-m";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(10, argv));
    /* All required options present (-i in -o out -l py) -> tests line 2184
     * false branch */
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(7, argv));
  }

  /* 25. apply_all_doc_meta tag and sec scheme failures */
  {
    memset(&spec, 0, sizeof(spec));
    memset(&meta, 0, sizeof(meta));
    meta.n_tag_meta = 1;
    meta.tag_meta = &doc_tag;
    memset(&doc_tag, 0, sizeof(doc_tag));
    doc_tag.name = (char *)(size_t) "TagAlloc";
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_apply_all_doc_meta(&spec, &meta));
    g_cdd_alloc_fail = 0;

    memset(&meta, 0, sizeof(meta));
    meta.n_security_schemes = 1;
    meta.security_schemes = &doc_sec;
    memset(&doc_sec, 0, sizeof(doc_sec));
    doc_sec.name = (char *)(size_t) "SecAlloc";
    doc_sec.type = DOC_SEC_HTTP;
    doc_sec.scheme = (char *)(size_t) "bearer";
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_apply_all_doc_meta(&spec, &meta));
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* 26. contact_email == NULL (line 1084) and license_identifier only (line
   * 1091) */
  {
    memset(&spec, 0, sizeof(spec));
    memset(&meta, 0, sizeof(meta));
    meta.contact_name = (char *)(size_t) "OnlyName";
    meta.contact_url = NULL;
    meta.contact_email = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));

    memset(&meta, 0, sizeof(meta));
    meta.license_name = NULL;
    meta.license_url = NULL;
    meta.license_identifier = (char *)(size_t) "MIT";
    spec.info.license.name = (char *)(size_t)c_cdd_strdup_macro("LicName");
    ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));

    /* line 1173: doc_tag.name == NULL */
    memset(&doc_tag, 0, sizeof(doc_tag));
    doc_tag.name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, &doc_tag));
    openapi_spec_free(&spec);
  }

  /* 27. missing arguments for options in all CLI mains */
  {
    argv[0] = (char *)(size_t) "bind";
    argv[1] = (char *)(size_t) "-i";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--input";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-o";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--output-dir";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-l";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--lang";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-n";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--lib-name";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-m";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--module-name";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));

    /* to_docs_json -i and --input missing */
    argv[0] = (char *)(size_t) "to_docs_json";
    argv[1] = (char *)(size_t) "-i";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, to_docs_json_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--input";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, to_docs_json_cli_main(2, argv));

    /* c2openapi options missing args */
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t) "--base";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-b";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--self";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = (char *)(size_t) "-s";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--dialect";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = (char *)(size_t) "--jsonSchemaDialect";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));

    /* c2openapi --self empty string */
    argv[1] = (char *)(size_t) "--self";
    argv[2] = (char *)(size_t) "";
    argv[3] = (char *)(size_t) "my_empty_dir";
    argv[4] = (char *)(size_t) "out_self_empty.json";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_cli_main(5, argv));
    remove("out_self_empty.json");
  }

  /* 28. c2openapi_build_operation fail in process_file (line 1705) */
  {
    const char *fpath = "test_custom_method.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @route FOOBAR /custom\n */\nvoid fn_custom(void) {}\n",
            fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      g_cdd_strdup_fail = 1;
      if (process_file(fpath, &spec) != CDD_C_SUCCESS) {
      }
      g_cdd_strdup_fail = 0;
      openapi_spec_free(&spec);
      remove(fpath);
    }
  }

  /* 29. Standalone doc_text alloc fail (line 1731) */
  {
    const char *fpath = "test_standalone_alloc.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @infoTitle StandaloneAlloc\n */\n", fp);
      fclose(fp);
      {
        int k;
        for (k = 1; k <= 8; ++k) {
          memset(&spec, 0, sizeof(spec));
          g_cdd_alloc_fail = k;
          if (process_file(fpath, &spec) != CDD_C_SUCCESS) {
          }
          g_cdd_alloc_fail = 0;
          openapi_spec_free(&spec);
        }
      }
      remove(fpath);
    }
  }

  /* 30. c2openapi_cli_main write fail (line 1938) */
  {
    const char *empty_f = "test_empty_for_walk.txt";
    FILE *fp = fopen(empty_f, "w");
    if (fp) {
      fclose(fp);
      argv[0] = (char *)(size_t) "c2openapi";
      argv[1] = (char *)(size_t)empty_f;
      argv[2] = (char *)(size_t) "/nonexistent_dir_9999/out.json";
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(3, argv));
      remove(empty_f);
    }
  }

  /* 30b. c2openapi_cli_main openapi_write_spec_to_json failure */
  {
    argv[0] = (char *)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)get_mocks_dir();
    argv[2] = (char *)(size_t) "out.json";
    c2openapi_set_json_allocators(mock_c2o_oom_malloc, mock_c2o_oom_free);
    g_c2o_oom_fail_at = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(3, argv));
    c2openapi_set_json_allocators(malloc, free);
    g_c2o_oom_fail_at = -1;
  }

  /* 31. to_docs_json_cli_main -h (line 1978) */
  {
    argv[0] = (char *)(size_t) "to_docs_json";
    argv[1] = (char *)(size_t) "-h";
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(2, argv));
  }

  /* 32. generate_bindings_cli_main reverse flags (line 2144) and -h (line 2106)
   */
  {
    argv[0] = (char *)(size_t) "bind";
    argv[1] = (char *)(size_t) "--module-name";
    argv[2] = (char *)(size_t) "mod";
    argv[3] = (char *)(size_t) "-m";
    argv[4] = (char *)(size_t) "mod2";
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(5, argv));

    argv[1] = (char *)(size_t) "-h";
    ASSERT_EQ(CDD_C_SUCCESS, generate_bindings_cli_main(2, argv));
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_C2OPENAPI_BRANCHES_H */
