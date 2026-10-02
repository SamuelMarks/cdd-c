/**
 * @file test_cli_c2openapi_oom.h
 * @brief Unit tests for c2openapi OOM and edge cases.
 */

#ifndef TEST_CLI_C2OPENAPI_OOM_H
#define TEST_CLI_C2OPENAPI_OOM_H

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

TEST test_c2openapi_helpers_global_meta_conflicts_full(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  cdd_c_error_t rc = 0;

  openapi_spec_init(&spec);
  doc_metadata_init(&meta);

  c_cdd_strdup("OrigTitle", &spec.info.title);
  meta.info_title = (char *)(size_t) "NewTitle";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.info_title = NULL;

  c_cdd_strdup("1.0", &spec.info.version);
  meta.info_version = (char *)(size_t) "2.0";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.info_version = NULL;

  c_cdd_strdup("Summary1", &spec.info.summary);
  meta.info_summary = (char *)(size_t) "Summary2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.info_summary = NULL;

  c_cdd_strdup("Desc1", &spec.info.description);
  meta.info_description = (char *)(size_t) "Desc2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.info_description = NULL;

  c_cdd_strdup("Terms1", &spec.info.terms_of_service);
  meta.terms_of_service = (char *)(size_t) "Terms2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.terms_of_service = NULL;

  c_cdd_strdup("Name1", &spec.info.contact.name);
  meta.contact_name = (char *)(size_t) "Name2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.contact_name = NULL;

  c_cdd_strdup("Url1", &spec.info.contact.url);
  meta.contact_url = (char *)(size_t) "Url2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.contact_url = NULL;

  c_cdd_strdup("Email1", &spec.info.contact.email);
  meta.contact_email = (char *)(size_t) "Email2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.contact_email = NULL;

  c_cdd_strdup("Dialect1", &spec.json_schema_dialect);
  meta.json_schema_dialect = (char *)(size_t) "Dialect2";
  rc = c2openapi_apply_doc_global_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  meta.json_schema_dialect = NULL;

  /* spec_add_security_scheme string conflicts */
  {
    struct DocSecurityScheme sec_doc;
    memset(&sec_doc, 0, sizeof(sec_doc));

    /* Description conflict */
    sec_doc.name = (char *)(size_t) "ConflictSec";
    sec_doc.type = DOC_SEC_HTTP;
    sec_doc.scheme = (char *)(size_t) "bearer";
    sec_doc.description = (char *)(size_t) "DescA";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.description = (char *)(size_t) "DescB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* HTTP scheme conflict */
    sec_doc.description = (char *)(size_t) "DescA";
    sec_doc.scheme = (char *)(size_t) "basic";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* Bearer format conflict */
    sec_doc.scheme = (char *)(size_t) "bearer";
    sec_doc.bearer_format = (char *)(size_t) "JWT";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.bearer_format = (char *)(size_t) "Opaque";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* OpenID URL conflict */
    sec_doc.name = (char *)(size_t) "OpenIdConflict";
    sec_doc.type = DOC_SEC_OPENID;
    sec_doc.scheme = NULL;
    sec_doc.bearer_format = NULL;
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlA";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

TEST test_c2openapi_oom_helpers_tag_and_global_meta(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  struct DocTagMeta tag_meta;
  int k;

  /* spec_add_tag OOM */
  for (k = 1; k <= 3; ++k) {
    openapi_spec_init(&spec);
    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_spec_add_tag(&spec, "tag_oom");
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* spec_apply_tag_meta OOM */
  for (k = 1; k <= 8; ++k) {
    openapi_spec_init(&spec);
    memset(&tag_meta, 0, sizeof(tag_meta));
    tag_meta.name = (char *)(size_t) "tag_oom";
    tag_meta.summary = (char *)(size_t) "summary";
    tag_meta.description = (char *)(size_t) "description";
    tag_meta.parent = (char *)(size_t) "parent";
    tag_meta.kind = (char *)(size_t) "kind";
    tag_meta.external_docs_url = (char *)(size_t) "http://url";
    tag_meta.external_docs_description = (char *)(size_t) "desc";

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_spec_apply_tag_meta(&spec, &tag_meta);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* apply_doc_global_meta OOM */
  for (k = 1; k <= 5; ++k) {
    openapi_spec_init(&spec);
    doc_metadata_init(&meta);
    meta.external_docs_url = (char *)(size_t) "http://url";
    meta.external_docs_description = (char *)(size_t) "desc";

    g_cdd_strdup_fail = k;
    c2openapi_apply_doc_global_meta(&spec, &meta);
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  PASS();
}

TEST test_c2openapi_oom_helpers_servers_and_security(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  int k;

  /* copy_doc_server_variables OOM */
  for (k = 1; k <= 6; ++k) {
    struct OpenAPI_Server srv;
    struct DocServer doc_srv;
    struct DocServerVar vars[1];
    char *enums[] = {(char *)(size_t) "8080"};
    memset(&srv, 0, sizeof(srv));
    memset(&doc_srv, 0, sizeof(doc_srv));
    memset(vars, 0, sizeof(vars));
    vars[0].name = (char *)(size_t) "port";
    vars[0].default_value = (char *)(size_t) "8080";
    vars[0].description = (char *)(size_t) "desc";
    vars[0].enum_values = enums;
    vars[0].n_enum_values = 1;
    doc_srv.variables = vars;
    doc_srv.n_variables = 1;

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_copy_doc_server_variables(&srv, &doc_srv);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    c2openapi_free_openapi_server_variables(&srv);
  }

  /* merge_scopes OOM */
  for (k = 1; k <= 4; ++k) {
    struct OpenAPI_OAuthFlow flow;
    struct DocOAuthFlow doc_flow;
    struct DocOAuthScope scopes[1];
    memset(&flow, 0, sizeof(flow));
    memset(&doc_flow, 0, sizeof(doc_flow));
    memset(scopes, 0, sizeof(scopes));
    scopes[0].name = (char *)(size_t) "read";
    scopes[0].description = (char *)(size_t) "desc";
    doc_flow.scopes = scopes;
    doc_flow.n_scopes = 1;

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_merge_scopes(&flow, &doc_flow);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    if (flow.scopes) {
      if (flow.scopes[0].name)
        C_CDD_FREE(flow.scopes[0].name);
      if (flow.scopes[0].description)
        C_CDD_FREE(flow.scopes[0].description);
      C_CDD_FREE(flow.scopes);
    }
  }

  /* add_oauth_flows OOM */
  for (k = 1; k <= 7; ++k) {
    struct OpenAPI_SecurityScheme scheme;
    struct DocSecurityScheme doc;
    struct DocOAuthFlow flow;
    struct DocOAuthScope scopes[1];
    memset(&scheme, 0, sizeof(scheme));
    memset(&doc, 0, sizeof(doc));
    memset(&flow, 0, sizeof(flow));
    memset(scopes, 0, sizeof(scopes));
    scopes[0].name = (char *)(size_t) "read";
    scopes[0].description = (char *)(size_t) "desc";
    flow.type = DOC_OAUTH_FLOW_IMPLICIT;
    flow.authorization_url = (char *)(size_t) "http://auth";
    flow.token_url = (char *)(size_t) "http://token";
    flow.refresh_url = (char *)(size_t) "http://refresh";
    flow.device_authorization_url = (char *)(size_t) "http://dev";
    flow.scopes = scopes;
    flow.n_scopes = 1;
    doc.flows = &flow;
    doc.n_flows = 1;

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_add_oauth_flows(&scheme, &doc);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    if (scheme.flows) {
      size_t f_idx;
      for (f_idx = 0; f_idx < scheme.n_flows; ++f_idx) {
        if (scheme.flows[f_idx].authorization_url)
          C_CDD_FREE(scheme.flows[f_idx].authorization_url);
        if (scheme.flows[f_idx].token_url)
          C_CDD_FREE(scheme.flows[f_idx].token_url);
        if (scheme.flows[f_idx].refresh_url)
          C_CDD_FREE(scheme.flows[f_idx].refresh_url);
        if (scheme.flows[f_idx].device_authorization_url)
          C_CDD_FREE(scheme.flows[f_idx].device_authorization_url);
        if (scheme.flows[f_idx].scopes) {
          size_t s_idx;
          for (s_idx = 0; s_idx < scheme.flows[f_idx].n_scopes; ++s_idx) {
            if (scheme.flows[f_idx].scopes[s_idx].name)
              C_CDD_FREE(scheme.flows[f_idx].scopes[s_idx].name);
            if (scheme.flows[f_idx].scopes[s_idx].description)
              C_CDD_FREE(scheme.flows[f_idx].scopes[s_idx].description);
          }
          C_CDD_FREE(scheme.flows[f_idx].scopes);
        }
      }
      C_CDD_FREE(scheme.flows);
    }
  }

  /* spec_add_security_scheme OOM */
  for (k = 1; k <= 5; ++k) {
    struct DocSecurityScheme doc;
    openapi_spec_init(&spec);
    memset(&doc, 0, sizeof(doc));
    doc.name = (char *)(size_t) "SecOOM";
    doc.type = DOC_SEC_APIKEY;
    doc.in = DOC_SEC_IN_HEADER;
    doc.param_name = (char *)(size_t) "key";

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_spec_add_security_scheme(&spec, &doc);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    openapi_spec_free(&spec);
  }

  /* append_root_security OOM */
  for (k = 1; k <= 5; ++k) {
    struct DocSecurityRequirement sec[1];
    char *sec_scopes[] = {(char *)(size_t) "read"};
    openapi_spec_init(&spec);
    doc_metadata_init(&meta);
    memset(sec, 0, sizeof(sec));
    sec[0].scheme = (char *)(size_t) "OAuth2";
    sec[0].scopes = sec_scopes;
    sec[0].n_scopes = 1;
    meta.security = sec;
    meta.n_security = 1;

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_append_root_security(&spec, &meta);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    meta.security = NULL;
    meta.n_security = 0;
    openapi_spec_free(&spec);
  }

  /* append_root_servers OOM */
  for (k = 1; k <= 6; ++k) {
    struct DocServer srv[1];
    openapi_spec_init(&spec);
    doc_metadata_init(&meta);
    memset(srv, 0, sizeof(srv));
    srv[0].url = (char *)(size_t) "http://srv";
    srv[0].name = (char *)(size_t) "name";
    srv[0].description = (char *)(size_t) "desc";
    meta.servers = srv;
    meta.n_servers = 1;

    g_cdd_alloc_fail = k;
    g_cdd_strdup_fail = k;
    c2openapi_append_root_servers(&spec, &meta);
    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    meta.servers = NULL;
    meta.n_servers = 0;
    openapi_spec_free(&spec);
  }

  PASS();
}

TEST test_c2openapi_oom_helpers_sig_and_file_process(void) {
  struct OpenAPI_Spec spec;
  const char *test_c = "test_c2openapi_oom_tmp.c";
  cdd_c_error_t rc = 0;
  int k;

  /* parse_c_signature_string OOM */
  for (k = 1; k <= 6; ++k) {
    struct C2OpenAPI_ParsedSig sig;
    g_cdd_alloc_fail = k;
    c2openapi_parse_c_signature_string("int foo(int a, char *b)", &sig);
    g_cdd_alloc_fail = 0;
  }

  write_to_file(test_c, "/**\n"
                        " * @route GET /items\n"
                        " */\n"
                        "void list_items(void) {}\n");

  /* process_file OOM */
  for (k = 1; k <= 6; ++k) {
    openapi_spec_init(&spec);
    g_cdd_alloc_fail = k;
    c2openapi_process_file(test_c, &spec);
    g_cdd_alloc_fail = 0;
    openapi_spec_free(&spec);
  }

  remove(test_c);

  /* walker_cb warning path */
  {
    openapi_spec_init(&spec);
    rc = c2openapi_walker_cb("nonexistent_walker_file_xyz.c", &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    openapi_spec_free(&spec);
  }

  PASS();
}

TEST test_c2openapi_remaining_edge_cases(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  struct C2OpenAPI_ParsedSig sig;
  cdd_c_error_t rc = 0;

  openapi_spec_init(&spec);
  doc_metadata_init(&meta);

  /* spec_find_tag with NULL _out_val */
  rc = c2openapi_spec_find_tag(&spec, "tag", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* free_parsed_sig with return_type */
  memset(&sig, 0, sizeof(sig));
  sig.return_type = (char *)(size_t)c_cdd_strdup_macro("int");
  c2openapi_free_parsed_sig(&sig);

  /* add_oauth_flows with unknown flow type */
  {
    struct OpenAPI_SecurityScheme scheme;
    struct DocSecurityScheme doc;
    struct DocOAuthFlow flow;
    memset(&scheme, 0, sizeof(scheme));
    memset(&doc, 0, sizeof(doc));
    memset(&flow, 0, sizeof(flow));
    flow.type = (enum DocOAuthFlowType)999;
    doc.flows = &flow;
    doc.n_flows = 1;
    rc = c2openapi_add_oauth_flows(&scheme, &doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }

  /* append_root_servers when copy_doc_server_variables fails with url and name
   * set */
  {
    struct DocServer srv[1];
    struct DocServerVar vars[1];
    memset(srv, 0, sizeof(srv));
    memset(vars, 0, sizeof(vars));
    srv[0].url = (char *)(size_t) "http://srv";
    srv[0].name = (char *)(size_t) "name";
    srv[0].description = (char *)(size_t) "desc";
    vars[0].name = NULL; /* causes copy_doc_server_variables to fail */
    srv[0].variables = vars;
    srv[0].n_variables = 1;
    meta.servers = srv;
    meta.n_servers = 1;
    rc = c2openapi_append_root_servers(&spec, &meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    meta.servers = NULL;
    meta.n_servers = 0;
  }

  /* apply_doc_global_meta license name/url/id conflicts and second
   * external_docs description */
  {
    spec.info.license.name = (char *)(size_t)c_cdd_strdup_macro("LicA");
    meta.license_name = (char *)(size_t) "LicB";
    rc = c2openapi_apply_doc_global_meta(&spec, &meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    C_CDD_FREE(spec.info.license.name);
    spec.info.license.name = (char *)(size_t)c_cdd_strdup_macro("LicA");
    meta.license_name = (char *)(size_t) "LicA";

    spec.info.license.url = (char *)(size_t)c_cdd_strdup_macro("UrlA");
    meta.license_url = (char *)(size_t) "UrlB";
    rc = c2openapi_apply_doc_global_meta(&spec, &meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    C_CDD_FREE(spec.info.license.url);
    spec.info.license.url = (char *)(size_t)c_cdd_strdup_macro("UrlA");
    meta.license_url = (char *)(size_t) "UrlA";

    C_CDD_FREE(spec.info.license.url);
    spec.info.license.url = NULL;
    meta.license_url = NULL;
    spec.info.license.identifier = (char *)(size_t)c_cdd_strdup_macro("IdA");
    meta.license_identifier = (char *)(size_t) "IdB";
    rc = c2openapi_apply_doc_global_meta(&spec, &meta);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    C_CDD_FREE(spec.info.license.identifier);
    spec.info.license.identifier = NULL;
    meta.license_identifier = NULL;

    /* external_docs description OOM */
    spec.external_docs.url = (char *)(size_t)c_cdd_strdup_macro("http://docs");
    meta.external_docs_url = (char *)(size_t) "http://docs";
    meta.external_docs_description = (char *)(size_t) "Added Desc";
    g_cdd_strdup_fail = 1;
    rc = c2openapi_apply_doc_global_meta(&spec, &meta);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_strdup_fail = 0;

    /* external_docs matching url, adding description */
    memset(&meta, 0, sizeof(meta));
    spec.external_docs.url = (char *)(size_t)c_cdd_strdup_macro("http://docs");
    meta.external_docs_url = (char *)(size_t) "http://docs";
    meta.external_docs_description = (char *)(size_t) "Added Desc";
    rc = c2openapi_apply_doc_global_meta(&spec, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("Added Desc", spec.external_docs.description);
  }

  /* process_file with comment separated by blank line */
  {
    const char *blank_file = "test_cli_blank_line.c";
    write_to_file(blank_file, "/**\n"
                              " * @route GET /spaced\n"
                              " */\n\n"
                              "void spaced_func(void) {}\n");
    rc = c2openapi_process_file(blank_file, &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    remove(blank_file);
  }

  /* parse_c_signature_string failing midway on args (trigger cleanup out->args
   * loop) */
  {
    g_cdd_alloc_fail = 4;
    rc = c2openapi_parse_c_signature_string("void multi(int a, int b, int c)",
                                            &sig);
    g_cdd_alloc_fail = 0;
  }

  /* generate_bindings_cli_main failing execution */
  {
    char *argv_fail[7];
    argv_fail[0] = (char *)(size_t) "bind";
    argv_fail[1] = (char *)(size_t) "-i";
    argv_fail[2] = (char *)(size_t) "nonexistent_dummy.h";
    argv_fail[3] = (char *)(size_t) "-o";
    argv_fail[4] = (char *)(size_t) "/nonexistent_dir_xyz";
    argv_fail[5] = (char *)(size_t) "-l";
    argv_fail[6] = (char *)(size_t) "python";
    rc = generate_bindings_cli_main(7, argv_fail);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  }

  /* c2openapi_cli_main with base spec that has self_uri and dialect, replaced
   * by options */
  {
    const char *base_tmp = "test_cli_base_overwrite.json";
    char *argv_replace[9];
    FILE *fp = fopen(base_tmp, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"$self\":\"http://old.com/"
            "self\",\"jsonSchemaDialect\":\"http://old.com/"
            "dialect\",\"info\":{\"title\":\"T\",\"version\":\"1\"},\"paths\":{"
            "}}",
            fp);
      fclose(fp);
    }

    argv_replace[0] = (char *)(size_t) "c2openapi";
    argv_replace[1] = (char *)(size_t) "--base";
    argv_replace[2] = (char *)(size_t)base_tmp;
    argv_replace[3] = (char *)(size_t) "--self";
    argv_replace[4] = (char *)(size_t) "http://new.com/self";
    argv_replace[5] = (char *)(size_t) "--dialect";
    argv_replace[6] = (char *)(size_t) "http://new.com/dialect";
    argv_replace[7] = (char *)(size_t)get_mocks_dir();
    argv_replace[8] = (char *)(size_t) "out_replace.json";

    rc = c2openapi_cli_main(9, argv_replace);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    remove(base_tmp);
    remove("out_replace.json");
  }

  /* spec_add_security_scheme string conflicts */
  {
    struct DocSecurityScheme sec_doc;
    memset(&sec_doc, 0, sizeof(sec_doc));

    /* Description conflict */
    sec_doc.name = (char *)(size_t) "ConflictSec";
    sec_doc.type = DOC_SEC_HTTP;
    sec_doc.scheme = (char *)(size_t) "bearer";
    sec_doc.description = (char *)(size_t) "DescA";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.description = (char *)(size_t) "DescB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* HTTP scheme conflict */
    sec_doc.description = (char *)(size_t) "DescA";
    sec_doc.scheme = (char *)(size_t) "basic";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* Bearer format conflict */
    sec_doc.scheme = (char *)(size_t) "bearer";
    sec_doc.bearer_format = (char *)(size_t) "JWT";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.bearer_format = (char *)(size_t) "Opaque";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* OpenID URL conflict */
    sec_doc.name = (char *)(size_t) "OpenIdConflict";
    sec_doc.type = DOC_SEC_OPENID;
    sec_doc.scheme = NULL;
    sec_doc.bearer_format = NULL;
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlA";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sec_doc.open_id_connect_url = (char *)(size_t) "http://urlB";
    rc = c2openapi_spec_add_security_scheme(&spec, &sec_doc);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  openapi_spec_free(&spec);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_C2OPENAPI_OOM_H */
