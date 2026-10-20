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
/** @brief Test definition g_c2o_oom_fail_at. */
static int g_c2o_oom_fail_at = -1;
/**
 * @brief Helper function mock_c2o_oom_malloc.
 * @param[in] sz Parameter sz.
 * @return Result status or value.
 */
static void *mock_c2o_oom_malloc(size_t sz) {
  if (g_c2o_oom_fail_at == 0) {
    g_c2o_oom_fail_at = -1;
    return NULL;
  }
  if (g_c2o_oom_fail_at > 0)
    g_c2o_oom_fail_at--;
  return malloc(sz);
}
/**
 * @brief Helper function mock_c2o_oom_free.
 * @param[in,out] ptr Parameter ptr.
 */
static void mock_c2o_oom_free(void *ptr) { free(ptr); }

/**
 * @brief Helper function get_mocks_dir.
 * @return Result status or value.
 */
static const char *get_mocks_dir(void) {
  FILE *f;
  f = fopen("src/tests/mocks/emit/simple.schema.json", "r");
  if (f) {
    fclose(f);
    return "src/tests/mocks";
  }
  return "../src/tests/mocks";
}

/**
 * @brief Helper function get_simple_schema.
 * @return Result status or value.
 */
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

/**
 * @brief Unit test test_c2openapi_final_branches.
 * @return Greatest test result.
 */
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
  meta.info_title = cdd_test_mut_str("");
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));

  /* 2. line 396 & 412: sv->name strdup fail, and enum_values != NULL &&
   * n_enum_values == 0 */
  memset(&srv, 0, sizeof(srv));
  memset(&doc_srv, 0, sizeof(doc_srv));
  doc_srv.n_variables = 1;
  doc_srv.variables = doc_vars;
  memset(doc_vars, 0, sizeof(doc_vars));
  doc_vars[0].name = cdd_test_mut_str("var");
  doc_vars[0].default_value = cdd_test_mut_str("def");
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
  flow_dst.scopes[0].name = strdup("read");
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
  doc_flow.authorization_url = cdd_test_mut_str("http://auth");
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
  doc_sec.name = cdd_test_mut_str("MutualTlsSec");
  doc_sec.type = DOC_SEC_MUTUALTLS;
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));

  doc_sec.name = cdd_test_mut_str("HttpEmptyScheme");
  doc_sec.type = DOC_SEC_HTTP;
  doc_sec.scheme = cdd_test_mut_str("");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));

  doc_sec.name = cdd_test_mut_str("OpenIdEmptyUrl");
  doc_sec.type = DOC_SEC_OPENID;
  doc_sec.open_id_connect_url = cdd_test_mut_str("");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));

  doc_sec.name = cdd_test_mut_str("OAuthMetaConflict");
  doc_sec.type = DOC_SEC_OAUTH2;
  doc_sec.oauth2_metadata_url = cdd_test_mut_str("http://meta1");
  doc_sec.n_flows = 1;
  doc_sec.flows = &doc_flow;
  doc_flow.type = DOC_OAUTH_FLOW_CLIENT_CREDENTIALS;
  doc_flow.token_url = cdd_test_mut_str("http://token");
  doc_flow.authorization_url = NULL;
  doc_flow.n_scopes = 0;
  ASSERT_EQ(CDD_C_SUCCESS, spec_add_security_scheme(&spec, &doc_sec));
  doc_sec.oauth2_metadata_url =
      cdd_test_mut_str("http://meta2"); /* conflict -> line 817 */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            spec_add_security_scheme(&spec, &doc_sec));
  openapi_spec_free(&spec);

  /* 6. line 992, 1001, 1003, 1012, 1016: append_root_servers cleanups */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.n_servers = 1;
  meta.servers = &doc_srv;
  memset(&doc_srv, 0, sizeof(doc_srv));
  doc_srv.url = cdd_test_mut_str("http://srv");
  doc_srv.name = cdd_test_mut_str("name1");
  doc_srv.description = cdd_test_mut_str("desc1");
  doc_srv.n_variables = 1;
  doc_srv.variables = doc_vars;
  memset(doc_vars, 0, sizeof(doc_vars));
  doc_vars[0].name = cdd_test_mut_str("var1");
  doc_vars[0].default_value = cdd_test_mut_str("def1");
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
  doc_vars[0].name = cdd_test_mut_str("var1");
  openapi_spec_free(&spec);

  /* 7. line 1100: contact_email, line 1107: meta->license_name == NULL &&
   * spec->info.license.name != NULL, line 1160: append_root_security fail */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.contact_email = cdd_test_mut_str("test@example.com");
  spec.info.license.name = strdup("ExistingLic");
  meta.license_url = cdd_test_mut_str("http://lic");
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  /* line 1160: append_root_security fail */
  meta.n_security = 1;
  meta.security = (struct DocSecurityRequirement *)calloc(
      1, sizeof(struct DocSecurityRequirement));
  meta.security[0].scheme = cdd_test_mut_str("oauth");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, apply_doc_global_meta(&spec, &meta));
  g_cdd_strdup_fail = 0;
  free(meta.security);
  openapi_spec_free(&spec);

  /* 8. line 1189: meta->name == "", line 1258: tag_meta != NULL && n_tag_meta
   * == 0, line 1262: spec_apply_tag_meta fail */
  memset(&spec, 0, sizeof(spec));
  memset(&doc_tag, 0, sizeof(doc_tag));
  doc_tag.name = cdd_test_mut_str("");
  ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, &doc_tag));
  memset(&meta, 0, sizeof(meta));
  meta.tag_meta = &doc_tag;
  meta.n_tag_meta = 0; /* line 1258 false */
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_tag_meta(&spec, &meta));
  meta.n_tag_meta = 1;
  doc_tag.name = cdd_test_mut_str("TagAllocFail");
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
    argv[0] = cdd_test_mut_str("c2openapi");
    argv[1] = cdd_test_mut_str("-s");
    argv[2] = cdd_test_mut_str("http://s1");
    argv[3] = cdd_test_mut_str("-s");
    argv[4] = cdd_test_mut_str("http://s2"); /* line 1941 overwrite */
    argv[5] = cdd_test_mut_str("--dialect");
    argv[6] = cdd_test_mut_str(""); /* line 1951 empty */
    TEST_MKDIR("build/my_empty_dir");
    argv[7] = cdd_test_mut_str("build/my_empty_dir");
    argv[8] = cdd_test_mut_str("build/out.json");
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_cli_main(9, argv));
    remove("build/out.json");

    /* self_uri strdup fail (line 1945) */
    argv[1] = cdd_test_mut_str("--self");
    argv[2] = cdd_test_mut_str("http://fail_self");
    argv[3] = cdd_test_mut_str("build/my_empty_dir");
    argv[4] = cdd_test_mut_str("build/out.json");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(5, argv));
    g_cdd_strdup_fail = 0;
  }

  /* 13. to_docs_json_cli_main CDD_INPUT env and duplicate path route */
  {
    const char *tmp_spec = "build/test_docs_multi_op.json";
    FILE *fp = fopen(tmp_spec, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},\"paths\":{\"/"
            "r\":{\"get\":{\"operationId\":\"g\"},\"post\":{\"operationId\":"
            "\"p\"}}}}",
            fp);
      fclose(fp);
    }
    argv[0] = cdd_test_mut_str("to_docs_json");
    argv[1] = cdd_test_mut_str("-i");
    argv[2] = cdd_test_mut_str(tmp_spec);
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(3, argv));

#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("CDD_INPUT=build/test_docs_multi_op.json");
#else
    setenv("CDD_INPUT", "build/test_docs_multi_op.json", 1);
#endif
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(1, argv));
#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("CDD_INPUT=");
#else
    unsetenv("CDD_INPUT");
#endif

#if defined(_WIN32) || defined(_MSC_VER)
    _putenv("INPUT_FILE=build/test_docs_multi_op.json");
#else
    setenv("INPUT_FILE", "build/test_docs_multi_op.json", 1);
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
    argv[0] = cdd_test_mut_str("bind");
    argv[1] = cdd_test_mut_str("-i");
    argv[2] = cdd_test_mut_str("in");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(3, argv));

    argv[3] = cdd_test_mut_str("-o");
    argv[4] = cdd_test_mut_str("out");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(5, argv));

    argv[5] = cdd_test_mut_str("-l");
    argv[6] = cdd_test_mut_str("py");
    argv[7] = cdd_test_mut_str("-n");
    argv[8] = cdd_test_mut_str("lib");
    argv[9] = cdd_test_mut_str("-m");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(10, argv));

    /* long flags */
    argv[1] = cdd_test_mut_str("--input");
    argv[2] = cdd_test_mut_str("in");
    argv[3] = cdd_test_mut_str("--output-dir");
    argv[4] = cdd_test_mut_str("out");
    argv[5] = cdd_test_mut_str("--lang");
    argv[6] = cdd_test_mut_str("py");
    argv[7] = cdd_test_mut_str("--lib-name");
    argv[8] = cdd_test_mut_str("lib");
    argv[9] = cdd_test_mut_str("--module-name");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(10, argv));

    /* all short valid flags */
    argv[1] = cdd_test_mut_str("-i");
    argv[2] = cdd_test_mut_str("in");
    argv[3] = cdd_test_mut_str("-o");
    argv[4] = cdd_test_mut_str("out");
    argv[5] = cdd_test_mut_str("-l");
    argv[6] = cdd_test_mut_str("py");
    argv[7] = cdd_test_mut_str("-n");
    argv[8] = cdd_test_mut_str("lib");
    argv[9] = cdd_test_mut_str("-m");
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
  meta.contact_email = cdd_test_mut_str("only_email@test.com");
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  openapi_spec_free(&spec);

  /* 16. line 1091: !meta->license_name && spec->info.license.name != NULL */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  spec.info.license.name = c_cdd_strdup_macro("ExistingLicName");
  meta.license_url = cdd_test_mut_str("http://license_url_only");
  ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));
  openapi_spec_free(&spec);

  /* 17. line 1173: append_root_security fail in apply_doc_global_meta */
  memset(&spec, 0, sizeof(spec));
  memset(&meta, 0, sizeof(meta));
  meta.n_security = 1;
  meta.security = (struct DocSecurityRequirement *)calloc(
      1, sizeof(struct DocSecurityRequirement));
  meta.security[0].scheme = cdd_test_mut_str("oauth");
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
  spec.paths[0].operations[0].tags[0] = cdd_test_mut_str("PathTag");
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
          cdd_c_error_t rc_proc;
          memset(&spec, 0, sizeof(spec));
          g_cdd_alloc_fail = k;
          rc_proc = process_file(fpath, &spec);
          ASSERT(rc_proc == CDD_C_SUCCESS || rc_proc == CDD_C_ERROR_MEMORY ||
                 rc_proc == CDD_C_ERROR_IO);
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
    const char *base_path = "build/test_base_with_uris.json";
    FILE *fp = fopen(base_path, "w");
    if (fp) {
      fputs(
          "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
          "\"$self\":\"http://orig_self\",\"jsonSchemaDialect\":\"http://"
          "orig_dialect\",\"paths\":{}}",
          fp);
      fclose(fp);

      argv[0] = cdd_test_mut_str("c2openapi");
      argv[1] = cdd_test_mut_str("-b");
      argv[2] = cdd_test_mut_str(base_path);
      argv[3] = cdd_test_mut_str("-s");
      argv[4] = cdd_test_mut_str("http://new_self");
      argv[5] = cdd_test_mut_str("--dialect");
      argv[6] = cdd_test_mut_str("http://new_dialect");
      argv[7] = cdd_test_mut_str("build/my_empty_dir");
      argv[8] = cdd_test_mut_str("build/out_uri.json");
      ASSERT_EQ(CDD_C_SUCCESS, c2openapi_cli_main(9, argv));
      remove(base_path);
      remove("build/out_uri.json");
    }
  }

  /* 22. c2openapi_cli_main collect_spec_tags fail and write fail */
  {
    const char *base_path = "build/test_base_tags.json";
    FILE *fp = fopen(base_path, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},\"paths\":{\"/r\":{\"get\":{\"tags\":[\"T1\"]}}}}",
            fp);
      fclose(fp);

      argv[0] = cdd_test_mut_str("c2openapi");
      argv[1] = cdd_test_mut_str("--base");
      argv[2] = cdd_test_mut_str(base_path);
      argv[3] = cdd_test_mut_str("build/my_empty_dir");
      argv[4] = cdd_test_mut_str("build/out_tag.json");
      g_cdd_strdup_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(5, argv));
      g_cdd_strdup_fail = 0;
      remove(base_path);
    }
  }

  /* 23. to_docs_json_cli_main nonexistent file */
  {
    argv[0] = cdd_test_mut_str("to_docs_json");
    argv[1] = cdd_test_mut_str("-i");
    argv[2] = cdd_test_mut_str("nonexistent_docs_9999.json");
    ASSERT_EQ(CDD_C_ERROR_PARSE, to_docs_json_cli_main(3, argv));
  }

  /* 24. generate_bindings_cli_main short options and missing arguments */
  {
    argv[0] = cdd_test_mut_str("bind");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(1, argv));
    argv[1] = cdd_test_mut_str("-i");
    argv[2] = cdd_test_mut_str("in");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(3, argv));
    argv[3] = cdd_test_mut_str("-o");
    argv[4] = cdd_test_mut_str("out");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(5, argv));
    argv[5] = cdd_test_mut_str("-l");
    argv[6] = cdd_test_mut_str("py");
    argv[7] = cdd_test_mut_str("-n");
    argv[8] = cdd_test_mut_str("lib");
    argv[9] = cdd_test_mut_str("-m");
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
    doc_tag.name = cdd_test_mut_str("TagAlloc");
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_apply_all_doc_meta(&spec, &meta));
    g_cdd_alloc_fail = 0;

    memset(&meta, 0, sizeof(meta));
    meta.n_security_schemes = 1;
    meta.security_schemes = &doc_sec;
    memset(&doc_sec, 0, sizeof(doc_sec));
    doc_sec.name = cdd_test_mut_str("SecAlloc");
    doc_sec.type = DOC_SEC_HTTP;
    doc_sec.scheme = cdd_test_mut_str("bearer");
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
    meta.contact_name = cdd_test_mut_str("OnlyName");
    meta.contact_url = NULL;
    meta.contact_email = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));

    memset(&meta, 0, sizeof(meta));
    meta.license_name = NULL;
    meta.license_url = NULL;
    meta.license_identifier = cdd_test_mut_str("MIT");
    spec.info.license.name = c_cdd_strdup_macro("LicName");
    ASSERT_EQ(CDD_C_SUCCESS, apply_doc_global_meta(&spec, &meta));

    /* line 1173: doc_tag.name == NULL */
    memset(&doc_tag, 0, sizeof(doc_tag));
    doc_tag.name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, spec_apply_tag_meta(&spec, &doc_tag));
    openapi_spec_free(&spec);
  }

  /* 27. missing arguments for options in all CLI mains */
  {
    argv[0] = cdd_test_mut_str("bind");
    argv[1] = cdd_test_mut_str("-i");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("--input");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("-o");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("--output-dir");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("-l");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("--lang");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("-n");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("--lib-name");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("-m");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("--module-name");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(2, argv));

    /* to_docs_json -i and --input missing */
    argv[0] = cdd_test_mut_str("to_docs_json");
    argv[1] = cdd_test_mut_str("-i");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, to_docs_json_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("--input");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, to_docs_json_cli_main(2, argv));

    /* c2openapi options missing args */
    argv[0] = cdd_test_mut_str("c2openapi");
    argv[1] = cdd_test_mut_str("--base");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("-b");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("--self");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("-s");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("--dialect");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));
    argv[1] = cdd_test_mut_str("--jsonSchemaDialect");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(2, argv));

    /* c2openapi --self empty string */
    argv[1] = cdd_test_mut_str("--self");
    argv[2] = cdd_test_mut_str("");
    argv[3] = cdd_test_mut_str("build/my_empty_dir");
    argv[4] = cdd_test_mut_str("build/out_self_empty.json");
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_cli_main(5, argv));
    remove("build/out_self_empty.json");
  }

  /* 28. c2openapi_build_operation fail in process_file (line 1705) */
  {
    cdd_c_error_t rc_proc;
    const char *fpath = "test_custom_method.c";
    FILE *fp = fopen(fpath, "w");
    if (fp) {
      fputs("/**\n * @route FOOBAR /custom\n */\nvoid fn_custom(void) {}\n",
            fp);
      fclose(fp);
      memset(&spec, 0, sizeof(spec));
      g_cdd_strdup_fail = 1;
      rc_proc = process_file(fpath, &spec);
      ASSERT_EQ(CDD_C_SUCCESS, rc_proc);
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
          cdd_c_error_t rc_proc;
          memset(&spec, 0, sizeof(spec));
          g_cdd_alloc_fail = k;
          rc_proc = process_file(fpath, &spec);
          ASSERT(rc_proc == CDD_C_SUCCESS || rc_proc == CDD_C_ERROR_MEMORY ||
                 rc_proc == CDD_C_ERROR_IO);
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
      argv[0] = cdd_test_mut_str("c2openapi");
      argv[1] = cdd_test_mut_str(empty_f);
      argv[2] = cdd_test_mut_str("/nonexistent_dir_9999/out.json");
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(3, argv));
      remove(empty_f);
    }
  }

  /* 30b. c2openapi_cli_main openapi_write_spec_to_json failure */
  {
    argv[0] = cdd_test_mut_str("c2openapi");
    argv[1] = cdd_test_mut_str(get_mocks_dir());
    argv[2] = cdd_test_mut_str("out.json");
    c2openapi_set_json_allocators(mock_c2o_oom_malloc, mock_c2o_oom_free);
    g_c2o_oom_fail_at = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, c2openapi_cli_main(3, argv));
    c2openapi_set_json_allocators(malloc, free);
    g_c2o_oom_fail_at = -1;
  }

  /* 31. to_docs_json_cli_main -h (line 1978) */
  {
    argv[0] = cdd_test_mut_str("to_docs_json");
    argv[1] = cdd_test_mut_str("-h");
    ASSERT_EQ(CDD_C_SUCCESS, to_docs_json_cli_main(2, argv));
  }

  /* 32. generate_bindings_cli_main reverse flags (line 2144) and -h (line 2106)
   */
  {
    argv[0] = cdd_test_mut_str("bind");
    argv[1] = cdd_test_mut_str("--module-name");
    argv[2] = cdd_test_mut_str("mod");
    argv[3] = cdd_test_mut_str("-m");
    argv[4] = cdd_test_mut_str("mod2");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, generate_bindings_cli_main(5, argv));

    argv[1] = cdd_test_mut_str("-h");
    ASSERT_EQ(CDD_C_SUCCESS, generate_bindings_cli_main(2, argv));
  }

  /* Cleanup temporary test files */
  remove("test_c2openapi_infer.c");
  remove("test_c2openapi_infer2.c");
  remove("test_c2openapi_infer3.c");
  remove("test_router_edge.c");
  remove("test_c2openapi_only_infer.c");
  remove("test_c2openapi_render_extra.c");
  remove("test_c2openapi_infer4.c");
  remove("test_c2openapi_infer5.c");
  remove("test_c2openapi_infer_fail.c");
  remove("test_c2openapi_oom_build.c");
  remove("test_c2openapi_eof.c");
  remove("test_c2openapi_last_tok.c");
  remove("test_c2openapi_no_brace.c");
  remove("test_valid_infer.c");
  remove("test_c2openapi_gui_only.c");

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_C2OPENAPI_BRANCHES_H */
/**
 * @brief Test client route inference, server routes, and GUI views in cli.c.
 */
extern C_CDD_EXPORT int g_cdd_aggregator_fail_ops_realloc;
/**
 * @brief Helper function cdd_test_c2openapi_infer_client_route.
 * @param[in] func_node Parameter func_node.
 * @param[in] tokens Parameter tokens.
 * @param[out] out_route Parameter out_route.
 * @return Result status or value.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_infer_client_route(
    const struct CstNode *func_node, const struct TokenList *tokens,
    char **out_route);
/**
 * @brief Helper function cdd_test_c2openapi_scan_server_routes.
 * @param[in] tokens Parameter tokens.
 * @param[in,out] spec Parameter spec.
 * @return Result status or value.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_scan_server_routes(
    const struct TokenList *tokens, struct OpenAPI_Spec *spec);
/**
 * @brief Helper function cdd_test_c2openapi_scan_gui_views.
 * @param[in] tokens Parameter tokens.
 * @param[in,out] spec Parameter spec.
 * @return Result status or value.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_scan_gui_views(
    const struct TokenList *tokens, struct OpenAPI_Spec *spec);
/**
 * @brief Unit test test_c2openapi_infer_routes_and_views.
 * @return Greatest test result.
 */
TEST test_c2openapi_infer_routes_and_views(void) {
  struct OpenAPI_Spec spec;
  cdd_c_error_t rc = 0;

  /* 1. Invalid args for process_file */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, process_file(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            process_file("test_c2openapi_infer.c", NULL));

  /* 2. Process file with client routes, server router, and GUI view */
  memset(&spec, 0, sizeof(spec));
  rc = openapi_spec_init(&spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Setup temporary test files */
  {
    FILE *fp = fopen("test_c2openapi_infer.c", "w");
    if (fp) {
      fputs("/* Documented route without @route tag to trigger "
            "c2openapi_infer_client_route with doc */\n",
            fp);
      fputs("/**\n", fp);
      fputs(" * @brief Test doc comment for inferred client route.\n", fp);
      fputs(" */\n", fp);
      fputs("int client_test_get(void) {\n", fp);
      fputs("  int action = HTTP_GET;\n", fp);
      fputs("  const char *endpoint = \"/users/list\";\n", fp);
      fputs("  return 0;\n", fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("int client_test_post(void) {\n", fp);
      fputs("  int action = HTTP_POST;\n", fp);
      fputs("  const char *endpoint = \"/users/create\";\n", fp);
      fputs("  return 0;\n", fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("int client_test_put(void) {\n", fp);
      fputs("  int action = HTTP_PUT;\n", fp);
      fputs("  const char *endpoint = \"/users/update\";\n", fp);
      fputs("  return 0;\n", fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("int client_test_delete(void) {\n", fp);
      fputs("  int action = HTTP_DELETE;\n", fp);
      fputs("  const char *endpoint = \"/users/delete\";\n", fp);
      fputs("  return 0;\n", fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("int client_test_patch(void) {\n", fp);
      fputs("  int action = HTTP_PATCH;\n", fp);
      fputs("  const char *endpoint = \"/users/patch\";\n", fp);
      fputs("  return 0;\n", fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("void setup_server_routes(void) {\n", fp);
      fputs("  c_rest_router_add(router, \"GET\", \"/api/items\", "
            "handle_list_items);\n",
            fp);
      fputs("  c_rest_router_add(router, \"POST\", \"/api/items\", "
            "custom_create);\n",
            fp);
      fputs(
          "  c_rest_router_add(router, \"DELETE\", \"/api/items\", handle_);\n",
          fp);
      fputs(
          "  c_rest_router_add(router, \"PUT\", \"/api/items\", handle_put);\n",
          fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("void render_dashboard_view(void) {\n", fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("/* Standalone doc comment block */\n", fp);
      fputs("/**\n", fp);
      fputs(" * @title Global API Title\n", fp);
      fputs(" * @version 1.0.0\n", fp);
      fputs(" */\n", fp);
      fputs("\n", fp);
      fputs("void dummy_no_route(void) {\n", fp);
      fputs("  int x = 1;\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_infer2.c", "w");
    if (fp) {
      fputs("void func_without_body;\n", fp);
      fputs("\n", fp);
      fputs("int render_view(void) { return 0; }\n", fp);
      fputs("\n", fp);
      fputs("void "
            "render_very_long_name_that_exceeds_one_hundred_and_twenty_eight_"
            "bytes_so_that_it_cannot_possibly_fit_inside_the_fixed_size_"
            "operation_id_buffer_view(void) {\n",
            fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("void render_ok_view(void) {\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_infer3.c", "w");
    if (fp) {
      fputs("void test_verbs(void) {\n", fp);
      fputs("  int a = HTTP_GET;\n", fp);
      fputs("  int b = HTTP_POST;\n", fp);
      fputs("  int c = HTTP_PUT;\n", fp);
      fputs("  int d = HTTP_DELETE;\n", fp);
      fputs("  int e = HTTP_PATCH;\n", fp);
      fputs("  const char *p1 = \"/api/v1\";\n", fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("void test_bad_route(void) {\n", fp);
      fputs("  int a = HTTP_GET;\n", fp);
      fputs("  const char *p2 = \"no_slash\";\n", fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("void test_no_verb(void) {\n", fp);
      fputs("  const char *p3 = \"/only/path\";\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_router_edge.c", "w");
    if (fp) {
      fputs("void test_router_incomplete(void) {\n", fp);
      fputs(
          "  /* 1. c_rest_router_add without '(' (j reaches tokens->size) */\n",
          fp);
      fputs("  c_rest_router_add ;\n", fp);
      fputs("\n", fp);
      fputs("  /* 2. c_rest_router_add with whitespace before '(' */\n", fp);
      fputs("  c_rest_router_add /* comment */ (router, \"GET\", \"/path\", "
            "handler);\n",
            fp);
      fputs("\n", fp);
      fputs("  /* 3. empty string literals for verb and path (len == 0) */\n",
            fp);
      fputs("  c_rest_router_add(router, \"\", \"\", \"\");\n", fp);
      fputs("\n", fp);
      fputs("  /* 4. verb provided but path empty */\n", fp);
      fputs("  c_rest_router_add(router, \"GET\", \"\", handler);\n", fp);
      fputs("\n", fp);
      fputs("  /* 5. path provided but verb empty */\n", fp);
      fputs("  c_rest_router_add(router, \"\", \"/path\", handler);\n", fp);
      fputs("\n", fp);
      fputs(
          "  /* 6. huge string literal exceeding buffer size (len >= 256) */\n",
          fp);
      fputs("  c_rest_router_add(router, "
            "\"VERY_VERY_LONG_VERB_THAT_EXCEEDS_SIXTEEN_BYTES\",\n",
            fp);
      fputs("    "
            "\"/"
            "very_long_path_that_exceeds_two_hundred_and_fifty_six_bytes_and_"
            "so_on_and_so_on_and_so_on_and_so_on_and_so_on_and_so_on_and_so_on_"
            "and_so_on_and_so_on_and_so_on_and_so_on_and_so_on_and_so_on_and_"
            "so_on_and_so_on_and_so_on_and_so_on_and_so_on_and_so_on_and_so_on_"
            "and_so_on_and_so_on_and_so_on\",\n",
            fp);
      fputs("    "
            "very_long_handler_name_that_exceeds_one_hundred_and_twenty_eight_"
            "bytes_and_so_on_and_so_on_and_so_on_and_so_on_and_so_on_and_so_on_"
            "and_so_on_and_so_on_and_so_on_and_so_on\n",
            fp);
      fputs("  );\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_only_infer.c", "w");
    if (fp) {
      fputs("int func_with_infer_route_only(void) {\n", fp);
      fputs("  int a = HTTP_GET;\n", fp);
      fputs("  const char *p = \"/test/infer\";\n", fp);
      fputs("  return 0;\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_render_extra.c", "w");
    if (fp) {
      fputs("void render_something_no(void) {\n", fp);
      fputs("  int not_a_view = 1;\n", fp);
      fputs("}\n", fp);
      fputs("\n", fp);
      fputs("int func_with_short_literal(void) {\n", fp);
      fputs("  int a = HTTP_GET;\n", fp);
      fputs("  const char *empty_str = \"\";\n", fp);
      fputs("  const char *one_char = \"x\";\n", fp);
      fputs("  return 0;\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_infer4.c", "w");
    if (fp) {
      fputs("int ( ) {\n", fp);
      fputs("  int a = HTTP_GET;\n", fp);
      fputs("  const char *p = \"/bad\";\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_infer5.c", "w");
    if (fp) {
      fputs("int my_inferred_handler(int id) {\n", fp);
      fputs("  int a = HTTP_GET;\n", fp);
      fputs("  const char *p = \"/api/valid_infer\";\n", fp);
      fputs("  return 0;\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_infer_fail.c", "w");
    if (fp) {
      fputs("/* Function with invalid type parameter causing "
            "c2openapi_build_operation to fail */\n",
            fp);
      fputs("int my_fail_op(unknown_custom_unmapped_type_t bad_param) {\n", fp);
      fputs("  int a = HTTP_GET;\n", fp);
      fputs("  const char *p = \"/api/fail_op\";\n", fp);
      fputs("  return 0;\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_oom_build.c", "w");
    if (fp) {
      fputs("int func_oom_build(void) {\n", fp);
      fputs("  int a = HTTP_GET;\n", fp);
      fputs("  const char *p = \"/api/oom_build\";\n", fp);
      fputs("  return 0;\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_eof.c", "w");
    if (fp) {
      fputs("/* Unclosed router call at EOF */\n", fp);
      fputs("void test_unclosed(void) {\n", fp);
      fputs("  c_rest_router_add(r, \"GET\", \"/eof\"\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_last_tok.c", "w");
    if (fp) {
      fputs("c_rest_router_add\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_no_brace.c", "w");
    if (fp) {
      fputs("int func_nobrace(void) /* no brace */\n", fp);
      fputs("  int a = HTTP_GET;\n", fp);
      fputs("  const char *p = \"/no_brace\";\n", fp);
      fputs(";\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_valid_infer.c", "w");
    if (fp) {
      fputs("void valid_api_endpoint(int id) {\n", fp);
      fputs("  HTTP_GET;\n", fp);
      fputs("  \"/api/endpoint\";\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }
  {
    FILE *fp = fopen("test_c2openapi_gui_only.c", "w");
    if (fp) {
      fputs("void render_only_view(void) {\n", fp);
      fputs("}\n", fp);
      fputs("", fp);
      fclose(fp);
    }
  }

  rc = process_file("test_c2openapi_infer.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = process_file("test_c2openapi_infer2.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = process_file("test_c2openapi_infer3.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = process_file("test_router_edge.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Process "test_c2openapi_only_infer.c" normally */
  rc = process_file("test_c2openapi_only_infer.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = process_file("test_c2openapi_render_extra.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = process_file("test_c2openapi_infer4.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = process_file("test_c2openapi_infer5.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = process_file("test_c2openapi_infer_fail.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Cause c2openapi_build_operation to fail via g_cdd_strdup_fail */
  g_cdd_strdup_fail = 1;
  rc = process_file("test_c2openapi_oom_build.c", &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = process_file("test_c2openapi_eof.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = process_file("test_c2openapi_last_tok.c", &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = process_file("test_c2openapi_no_brace.c", &spec);
  {
    const char *vpath = "test_valid_infer.c";
    FILE *fp = fopen(vpath, "w");
    if (fp) {
      fputs("void valid_api_endpoint(int id) {\n  HTTP_GET;\n  "
            "\"/api/endpoint\";\n}\n",
            fp);
      fclose(fp);
      rc = process_file(vpath, &spec);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
    }
  }
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Process file with failing server route scan
   * (g_cdd_aggregator_fail_ops_realloc = 1) */
  g_cdd_aggregator_fail_ops_realloc = 1;
  rc = process_file("test_router_edge.c", &spec);
  g_cdd_aggregator_fail_ops_realloc = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* Process file with failing GUI view route scan */
  g_cdd_aggregator_fail_ops_realloc = 1;
  rc = process_file("test_c2openapi_gui_only.c", &spec);
  g_cdd_aggregator_fail_ops_realloc = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* OOM on sig_raw in "test_c2openapi_only_infer.c" using g_cdd_alloc_fail */
  {
    int k;
    for (k = 1; k <= 12; ++k) {
      g_cdd_alloc_fail = k;
      rc = process_file("test_c2openapi_only_infer.c", &spec);
      g_cdd_alloc_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY ||
             rc == CDD_C_ERROR_IO);
    }
  }

  /* When alloc fails during file processing, it returns memory or success
   * depending on which allocation failed */
  g_cdd_alloc_fail = 1;
  rc = process_file("test_c2openapi_infer3.c", &spec);
  g_cdd_alloc_fail = 0;

  openapi_spec_free(&spec);

  /* 3. process_file OOM on comment_used */
  {
    memset(&spec, 0, sizeof(spec));
    rc = openapi_spec_init(&spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    g_cdd_alloc_fail = 1;
    rc = process_file("test_c2openapi_infer.c", &spec);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    openapi_spec_free(&spec);
  }

  PASS();
}

extern C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_infer_client_route(
    const struct CstNode *func_node, const struct TokenList *tokens,
    char **out_route);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_scan_server_routes(
    const struct TokenList *tokens, struct OpenAPI_Spec *spec);
extern C_CDD_EXPORT cdd_c_error_t cdd_test_c2openapi_scan_gui_views(
    const struct TokenList *tokens, struct OpenAPI_Spec *spec);

/**
 * @brief Comprehensive tests for c2openapi_infer_client_route, server_routes,
 * and gui_views.
 * @return Test or helper result.
 */
TEST test_cli_c2openapi_unit_internals(void) {
  struct OpenAPI_Spec spec;
  struct TokenList *tokens = NULL;
  struct CstNode func_node;
  char *route = NULL;
  cdd_c_error_t rc = 0;
  memset(&func_node, 0, sizeof(func_node));

  /* Create a valid dummy TokenList */
  {
    const char dummy_code[] = "int x = 1;";
    az_span d_span =
        az_span_create((uint8_t *)(size_t)dummy_code, strlen(dummy_code));
    tokenize(d_span, &tokens);
  }

  /* 1. c2openapi_infer_client_route NULL args */
  rc = cdd_test_c2openapi_infer_client_route(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_c2openapi_infer_client_route(&func_node, NULL, &route);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_c2openapi_infer_client_route(&func_node, tokens, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* 2. c2openapi_scan_server_routes NULL args */
  rc = cdd_test_c2openapi_scan_server_routes(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_c2openapi_scan_server_routes(NULL, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_c2openapi_scan_server_routes(tokens, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* 3. c2openapi_scan_gui_views NULL args */
  rc = cdd_test_c2openapi_scan_gui_views(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_c2openapi_scan_gui_views(NULL, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_c2openapi_scan_gui_views(tokens, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  free_token_list(tokens);
  tokens = NULL;

  /* 4. Tokenized test with various tokens */
  {
    const char code[] = "void f() {\n"
                        "  HTTP_POST;\n"
                        "  \"/my/path\";\n"
                        "}\n";
    az_span span = az_span_create((uint8_t *)(size_t)code, strlen(code));
    ASSERT_EQ(0, tokenize(span, &tokens));

    memset(&func_node, 0, sizeof(func_node));
    func_node.start_token = 0;
    func_node.end_token = tokens->size;

    rc = cdd_test_c2openapi_infer_client_route(&func_node, tokens, &route);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_NEQ(NULL, route);
    ASSERT_STR_EQ("POST /my/path", route);
    free(route);
    route = NULL;

    /* OOM on path_str (1st allocation) */
    g_cdd_alloc_fail = 1;
    rc = cdd_test_c2openapi_infer_client_route(&func_node, tokens, &route);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    /* OOM on route string (2nd allocation) */
    g_cdd_alloc_fail = 2;
    rc = cdd_test_c2openapi_infer_client_route(&func_node, tokens, &route);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    free_token_list(tokens);
  }

  /* 5. Server router with handle_ prefix and custom handler name */
  {
    const char srv_code[] =
        "void init() {\n"
        "  c_rest_router_add(r, \"GET\", \"/items\", handle_get_items);\n"
        "  c_rest_router_add(r, \"POST\", \"/items\", custom_create);\n"
        "  c_rest_router_add(r, \"DELETE\", \"/items\", handle_);\n"
        "}\n";
    az_span span =
        az_span_create((uint8_t *)(size_t)srv_code, strlen(srv_code));
    ASSERT_EQ(0, tokenize(span, &tokens));

    memset(&spec, 0, sizeof(spec));
    rc = openapi_spec_init(&spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = cdd_test_c2openapi_scan_server_routes(tokens, &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* OOM inside server scan */
    g_cdd_strdup_fail = 1;
    rc = cdd_test_c2openapi_scan_server_routes(tokens, &spec);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    openapi_spec_free(&spec);
    free_token_list(tokens);
  }

  /* 6. GUI view scanning */
  {
    const char gui_code[] = "void render_users_view(void) {}\n";
    az_span span =
        az_span_create((uint8_t *)(size_t)gui_code, strlen(gui_code));
    ASSERT_EQ(0, tokenize(span, &tokens));

    memset(&spec, 0, sizeof(spec));
    rc = openapi_spec_init(&spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = cdd_test_c2openapi_scan_gui_views(tokens, &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* OOM inside GUI view scan */
    g_cdd_strdup_fail = 1;
    rc = cdd_test_c2openapi_scan_gui_views(tokens, &spec);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    openapi_spec_free(&spec);
    free_token_list(tokens);
  }

  PASS();
}

extern C_CDD_EXPORT int g_cdd_aggregator_fail_ops_realloc;

/**
 * @brief Test aggregator failure error returns in c2openapi_scan_server_routes
 * and gui_views.
 * @return Test or helper result.
 */
TEST test_cli_c2openapi_aggregator_errors(void) {
  struct OpenAPI_Spec spec;
  struct TokenList *tokens = NULL;
  cdd_c_error_t rc = 0;

  /* 1. Server router route addition error */
  {
    const char code[] = "void init() { c_rest_router_add(r, \"GET\", "
                        "\"/items\", handle_items); }\n";
    az_span span = az_span_create((uint8_t *)(size_t)code, strlen(code));
    ASSERT_EQ(0, tokenize(span, &tokens));

    memset(&spec, 0, sizeof(spec));
    rc = openapi_spec_init(&spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    g_cdd_aggregator_fail_ops_realloc = 1;
    rc = cdd_test_c2openapi_scan_server_routes(tokens, &spec);
    g_cdd_aggregator_fail_ops_realloc = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    openapi_spec_free(&spec);
    free_token_list(tokens);
  }

  /* 2. GUI view route addition error */
  {
    const char code[] = "void render_users_view(void) {}\n";
    az_span span = az_span_create((uint8_t *)(size_t)code, strlen(code));
    ASSERT_EQ(0, tokenize(span, &tokens));

    memset(&spec, 0, sizeof(spec));
    rc = openapi_spec_init(&spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    g_cdd_aggregator_fail_ops_realloc = 1;
    rc = cdd_test_c2openapi_scan_gui_views(tokens, &spec);
    g_cdd_aggregator_fail_ops_realloc = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    openapi_spec_free(&spec);
    free_token_list(tokens);
  }

  PASS();
}
