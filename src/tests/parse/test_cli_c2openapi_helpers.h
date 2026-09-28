/**
 * @file test_cli_c2openapi_helpers.h
 * @brief Unit tests for c2openapi helpers.
 */

#ifndef TEST_CLI_C2OPENAPI_HELPERS_H
#define TEST_CLI_C2OPENAPI_HELPERS_H

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

TEST test_c2openapi_helpers_register_types_cases(void) {
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  struct StructFields sf;
  struct EnumMembers em;
  char *unions[] = {(char *)(size_t) "string", (char *)(size_t) "integer"};
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  /* NULL checks */
  rc = c2openapi_register_types(NULL, &types);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_register_types(&spec, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Struct type with field unions */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "data", "string", NULL, NULL, 0);
  sf.fields[0].n_type_union = 2;
  sf.fields[0].type_union = (char **)unions;
  sf.fields[0].n_items_type_union = 2;
  sf.fields[0].items_type_union = (char **)unions;

  /* Enum type */
  enum_members_init(&em);
  enum_members_add(&em, "RED");
  enum_members_add(&em, "GREEN");

  types.items =
      (struct TypeDefinition *)calloc(3, sizeof(struct TypeDefinition));
  types.size = 3;
  types.capacity = 3;

  types.items[0].kind = KIND_STRUCT;
  types.items[0].name = (char *)(size_t) "MyDataStruct";
  types.items[0].details.struct_fields = &sf;

  types.items[1].kind = KIND_ENUM;
  types.items[1].name = (char *)(size_t) "MyColorEnum";
  types.items[1].details.enum_members = &em;

  /* Duplicate type check */
  types.items[2].kind = KIND_STRUCT;
  types.items[2].name = (char *)(size_t) "MyDataStruct";
  types.items[2].details.struct_fields = &sf;

  rc = c2openapi_register_types(&spec, &types);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, spec.n_defined_schemas);
  ASSERT_STR_EQ("MyDataStruct", spec.defined_schema_names[0]);
  ASSERT_STR_EQ("MyColorEnum", spec.defined_schema_names[1]);

  sf.fields[0].n_type_union = 0;
  sf.fields[0].type_union = NULL;
  sf.fields[0].n_items_type_union = 0;
  sf.fields[0].items_type_union = NULL;
  struct_fields_free(&sf);
  enum_members_free(&em);
  free(types.items);
  types.items = NULL;
  types.size = 0;
  type_def_list_free(&types);
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

TEST test_to_docs_json_cli_main_all_verbs(void) {
  const char *spec_file = "test_docs_all_verbs.json";
  const char *invalid_spec = "test_docs_invalid_spec.json";
  char *argv1[5];
  char *argv2[3];
  cdd_c_error_t rc;

  /* Write spec containing operations with all HTTP verbs */
  {
    FILE *fp = fopen(spec_file, "w");
    if (fp) {
      fputs("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},\"paths\":{\"/r\":{",
            fp);
      fputs("\"get\":{\"operationId\":\"g\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"post\":{\"operationId\":\"po\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"put\":{\"operationId\":\"u\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"delete\":{\"operationId\":\"d\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"patch\":{\"operationId\":\"p\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"head\":{\"operationId\":\"h\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"options\":{\"operationId\":\"o\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"trace\":{\"operationId\":\"t\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}},",
            fp);
      fputs("\"query\":{\"operationId\":\"q\",\"responses\":{\"200\":{"
            "\"description\":\"ok\"}}}}}}",
            fp);
      fclose(fp);
    }
  }

  argv1[0] = (char *)(size_t) "to_docs_json";
  argv1[1] = (char *)(size_t) "-i";
  argv1[2] = (char *)(size_t)spec_file;
  argv1[3] = (char *)(size_t) "--no-imports";
  argv1[4] = (char *)(size_t) "--no-wrapping";

  rc = to_docs_json_cli_main(5, argv1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Without --no-imports and --no-wrapping */
  argv2[0] = (char *)(size_t) "to_docs_json";
  argv2[1] = (char *)(size_t) "-i";
  argv2[2] = (char *)(size_t)spec_file;
  rc = to_docs_json_cli_main(3, argv2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Invalid OpenAPI spec returning error from openapi_load_from_json */
  write_to_file(invalid_spec, "[1, 2, 3]");
  argv2[2] = (char *)(size_t)invalid_spec;
  rc = to_docs_json_cli_main(3, argv2);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);

  remove(spec_file);
  remove(invalid_spec);
  PASS();
}

TEST test_generate_bindings_cli_main_options(void) {
  const char *dummy_header = "test_cli_bind_dummy.h";
  const char *out_dir = "test_cli_bind_out";
  char *argv_full[14];
  char *argv_missing[5];
  cdd_c_error_t rc;

  write_to_file(dummy_header, "int my_api_add(int a, int b);\n");
  makedir(out_dir);

  /* Full options test with valid directory and header */
  argv_full[0] = (char *)(size_t) "bind";
  argv_full[1] = (char *)(size_t) "-i";
  argv_full[2] = (char *)(size_t)dummy_header;
  argv_full[3] = (char *)(size_t) "-o";
  argv_full[4] = (char *)(size_t)out_dir;
  argv_full[5] = (char *)(size_t) "-l";
  argv_full[6] = (char *)(size_t) "python";
  argv_full[7] = (char *)(size_t) "-n";
  argv_full[8] = (char *)(size_t) "my_lib";
  argv_full[9] = (char *)(size_t) "-m";
  argv_full[10] = (char *)(size_t) "my_mod";
  argv_full[11] = (char *)(size_t) "--skip-static";
  argv_full[12] = (char *)(size_t) "--opaque-pointers";
  argv_full[13] = (char *)(size_t) "--generate-tests";

  rc = generate_bindings_cli_main(14, argv_full);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Missing required parameters */
  argv_missing[0] = (char *)(size_t) "bind";
  argv_missing[1] = (char *)(size_t) "-i";
  argv_missing[2] = (char *)(size_t)dummy_header;
  argv_missing[3] = (char *)(size_t) "-o";
  argv_missing[4] = (char *)(size_t)out_dir;
  rc = generate_bindings_cli_main(5, argv_missing);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  remove(dummy_header);
  PASS();
}

TEST test_c2openapi_cli_main_edge_branches(void) {
  char *argv_base_missing[] = {(char *)(size_t) "c2openapi",
                               (char *)(size_t) "--base"};
  char *argv_self_missing[] = {(char *)(size_t) "c2openapi",
                               (char *)(size_t) "--self"};
  char *argv_dialect_missing[] = {(char *)(size_t) "c2openapi",
                                  (char *)(size_t) "--dialect"};
  char *argv_base_fail[] = {
      (char *)(size_t) "c2openapi", (char *)(size_t) "--base",
      (char *)(size_t) "nonexistent_base_123.json", (char *)(size_t) "src",
      (char *)(size_t) "out.json"};
  char *argv_walk_fail[] = {(char *)(size_t) "c2openapi",
                            (char *)(size_t) "nonexistent_src_dir_123",
                            (char *)(size_t) "out.json"};
  char *argv_write_fail[3];
  cdd_c_error_t rc;

  argv_write_fail[0] = (char *)(size_t) "c2openapi";
  argv_write_fail[1] = (char *)(size_t)get_mocks_dir();
  argv_write_fail[2] = (char *)(size_t) "/nonexistent_dir_9999/out.json";

  /* Missing option arguments */
  rc = c2openapi_cli_main(2, argv_base_missing);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  rc = c2openapi_cli_main(2, argv_self_missing);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  rc = c2openapi_cli_main(2, argv_dialect_missing);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  /* Base file load failure */
  rc = c2openapi_cli_main(5, argv_base_fail);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  /* Walk failure on nonexistent dir */
  rc = c2openapi_cli_main(3, argv_walk_fail);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  /* Write failure on invalid output path */
  rc = c2openapi_cli_main(3, argv_write_fail);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);

  PASS();
}

TEST test_c2openapi_helpers_tag_meta_and_collection(void) {
  struct OpenAPI_Spec spec;
  struct DocTagMeta meta;
  struct OpenAPI_Operation op;
  char *tag_names[] = {(char *)(size_t) "tagA", (char *)(size_t) "tagB"};
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  memset(&meta, 0, sizeof(meta));

  /* NULL checks */
  rc = c2openapi_spec_apply_tag_meta(NULL, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_spec_apply_tag_meta(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  meta.name = (char *)(size_t) "";
  rc = c2openapi_spec_apply_tag_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Full tag metadata */
  meta.name = (char *)(size_t) "MyTag";
  meta.summary = (char *)(size_t) "Tag Summary";
  meta.description = (char *)(size_t) "Tag Description";
  meta.parent = (char *)(size_t) "ParentTag";
  meta.kind = (char *)(size_t) "custom";
  meta.external_docs_url = (char *)(size_t) "http://example.com/docs";
  meta.external_docs_description = (char *)(size_t) "Docs description";

  rc = c2openapi_spec_apply_tag_meta(&spec, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* apply_doc_tag_meta */
  {
    struct DocMetadata doc_meta;
    doc_metadata_init(&doc_meta);
    rc = c2openapi_apply_doc_tag_meta(&spec, &doc_meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    doc_meta.n_tag_meta = 1;
    doc_meta.tag_meta = &meta;
    rc = c2openapi_apply_doc_tag_meta(&spec, &doc_meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    doc_meta.tag_meta = NULL;
    doc_meta.n_tag_meta = 0;
    doc_metadata_free(&doc_meta);
  }

  /* collect_tags_from_op */
  rc = c2openapi_collect_tags_from_op(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  memset(&op, 0, sizeof(op));
  op.tags = tag_names;
  op.n_tags = 2;
  rc = c2openapi_collect_tags_from_op(&spec, &op);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* collect_tags_from_paths with operations and additional_operations */
  {
    struct OpenAPI_Path paths[1];
    memset(paths, 0, sizeof(paths));
    paths[0].operations = &op;
    paths[0].n_operations = 1;
    paths[0].additional_operations = &op;
    paths[0].n_additional_operations = 1;
    rc = c2openapi_collect_tags_from_paths(&spec, paths, 1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  /* collect_spec_tags */
  rc = c2openapi_collect_spec_tags(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2openapi_collect_spec_tags(&spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

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

TEST test_c2openapi_helpers_scopes_and_flow_merge(void) {
  struct OpenAPI_SecurityScheme scheme;
  struct OpenAPI_OAuthFlow flow;
  struct DocOAuthFlow doc_flow;
  struct DocOAuthScope scopes[2];
  struct OpenAPI_OAuthFlow *found = NULL;
  cdd_c_error_t rc;

  memset(&scheme, 0, sizeof(scheme));
  memset(&flow, 0, sizeof(flow));
  memset(&doc_flow, 0, sizeof(doc_flow));
  memset(scopes, 0, sizeof(scopes));

  /* find_oauth_flow */
  rc = c2openapi_find_oauth_flow(&scheme, OA_OAUTH_FLOW_IMPLICIT, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, found);

  /* merge_scopes */
  scopes[0].name = (char *)(size_t) "read";
  scopes[0].description = (char *)(size_t) "Read scope";
  scopes[1].name = (char *)(size_t) "write";
  scopes[1].description = NULL;
  doc_flow.scopes = scopes;
  doc_flow.n_scopes = 2;

  rc = c2openapi_merge_scopes(&flow, &doc_flow);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, flow.n_scopes);

  /* Merge duplicate scope */
  rc = c2openapi_merge_scopes(&flow, &doc_flow);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, flow.n_scopes);

  /* merge_oauth_flow */
  doc_flow.authorization_url = (char *)(size_t) "http://example.com/auth";
  doc_flow.token_url = (char *)(size_t) "http://example.com/token";
  doc_flow.refresh_url = (char *)(size_t) "http://example.com/refresh";
  doc_flow.device_authorization_url = (char *)(size_t) "http://example.com/dev";
  rc = c2openapi_merge_oauth_flow(&flow, &doc_flow);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(flow.authorization_url != NULL);
  ASSERT(flow.token_url != NULL);
  ASSERT(flow.refresh_url != NULL);
  ASSERT(flow.device_authorization_url != NULL);

  /* Clean up flow */
  C_CDD_FREE(flow.authorization_url);
  C_CDD_FREE(flow.token_url);
  C_CDD_FREE(flow.refresh_url);
  C_CDD_FREE(flow.device_authorization_url);
  if (flow.scopes) {
    size_t s;
    for (s = 0; s < flow.n_scopes; ++s) {
      C_CDD_FREE(flow.scopes[s].name);
      if (flow.scopes[s].description)
        C_CDD_FREE(flow.scopes[s].description);
    }
    C_CDD_FREE(flow.scopes);
  }

  PASS();
}

TEST test_c2openapi_helpers_servers_and_security_append(void) {
  struct OpenAPI_Spec spec;
  struct DocMetadata meta;
  struct DocServer srv[1];
  struct DocServerVar vars[1];
  struct DocSecurityRequirement sec[1];
  char *sec_scopes[] = {(char *)(size_t) "read", (char *)(size_t) "write"};
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  doc_metadata_init(&meta);

  /* append_root_servers with NULL / empty */
  rc = c2openapi_append_root_servers(NULL, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_append_root_servers(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Server with name, description, variables */
  memset(srv, 0, sizeof(srv));
  memset(vars, 0, sizeof(vars));
  vars[0].name = (char *)(size_t) "port";
  vars[0].default_value = (char *)(size_t) "8080";
  srv[0].url = (char *)(size_t) "http://localhost:8080";
  srv[0].name = (char *)(size_t) "Local";
  srv[0].description = (char *)(size_t) "Local server";
  srv[0].variables = vars;
  srv[0].n_variables = 1;

  meta.servers = srv;
  meta.n_servers = 1;
  rc = c2openapi_append_root_servers(&spec, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, spec.n_servers);
  ASSERT_STR_EQ("Local", spec.servers[0].name);
  ASSERT_STR_EQ("Local server", spec.servers[0].description);

  /* append_root_security with NULL / empty */
  rc = c2openapi_append_root_security(NULL, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_append_root_security(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Security requirement with scopes */
  memset(sec, 0, sizeof(sec));
  sec[0].scheme = (char *)(size_t) "OAuth2";
  sec[0].scopes = sec_scopes;
  sec[0].n_scopes = 2;
  meta.security = sec;
  meta.n_security = 1;

  rc = c2openapi_append_root_security(&spec, &meta);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, spec.n_security);
  ASSERT_STR_EQ("OAuth2", spec.security[0].requirements[0].scheme);
  ASSERT_EQ(2, spec.security[0].requirements[0].n_scopes);

  meta.servers = NULL;
  meta.n_servers = 0;
  meta.security = NULL;
  meta.n_security = 0;
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

TEST test_c2openapi_helpers_sig_parsing_extra(void) {
  struct C2OpenAPI_ParsedSig sig;
  cdd_c_error_t rc;

  /* Function with unnamed/void argument */
  rc = c2openapi_parse_c_signature_string("void no_named_args(void)", &sig);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, sig.n_args);
  c2openapi_free_parsed_sig(&sig);

  PASS();
}

TEST test_c2openapi_helpers_security_schemes_all_types(void) {
  struct OpenAPI_Spec spec;
  struct DocSecurityScheme doc;
  struct DocOAuthFlow flow;
  cdd_c_error_t rc;

  openapi_spec_init(&spec);
  memset(&doc, 0, sizeof(doc));
  memset(&flow, 0, sizeof(flow));

  /* NULL checks */
  rc = c2openapi_spec_add_security_scheme(NULL, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = c2openapi_spec_add_security_scheme(&spec, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  doc.name = (char *)(size_t) "";
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Unknown type */
  doc.name = (char *)(size_t) "SecUnknown";
  doc.type = DOC_SEC_UNSET;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* OAuth2 with invalid flow */
  doc.name = (char *)(size_t) "SecOAuthInvalid";
  doc.type = DOC_SEC_OAUTH2;
  flow.type = DOC_OAUTH_FLOW_IMPLICIT; /* Missing auth URL */
  doc.flows = &flow;
  doc.n_flows = 1;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* API Key errors */
  doc.name = (char *)(size_t) "ApiKey1";
  doc.type = DOC_SEC_APIKEY;
  doc.param_name = NULL;
  doc.in = DOC_SEC_IN_HEADER;
  doc.flows = NULL;
  doc.n_flows = 0;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  doc.param_name = (char *)(size_t) "X-API-Key";
  doc.in = DOC_SEC_IN_UNSET;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Valid API Key */
  doc.in = DOC_SEC_IN_HEADER;
  doc.description = (char *)(size_t) "API Key auth";
  doc.deprecated_set = 1;
  doc.deprecated = 1;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Colliding deprecated value */
  doc.deprecated = 0;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  doc.deprecated = 1;

  /* Type collision */
  doc.type = DOC_SEC_HTTP;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* HTTP scheme errors and valid */
  doc.name = (char *)(size_t) "HttpSec";
  doc.type = DOC_SEC_HTTP;
  doc.scheme = NULL;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  doc.scheme = (char *)(size_t) "bearer";
  doc.bearer_format = (char *)(size_t) "JWT";
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* OpenID errors and valid */
  doc.name = (char *)(size_t) "OpenIdSec";
  doc.type = DOC_SEC_OPENID;
  doc.open_id_connect_url = NULL;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  doc.open_id_connect_url = (char *)(size_t) "http://example.com/openid";
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* OAuth2 valid flow and metadata url */
  doc.name = (char *)(size_t) "OAuthValid";
  doc.type = DOC_SEC_OAUTH2;
  doc.oauth2_metadata_url = (char *)(size_t) "http://example.com/oauth_meta";
  flow.type = DOC_OAUTH_FLOW_IMPLICIT;
  flow.authorization_url = (char *)(size_t) "http://example.com/auth";
  doc.flows = &flow;
  doc.n_flows = 1;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* OAuth2 with 0 flows -> error */
  doc.name = (char *)(size_t) "OAuthZeroFlows";
  doc.flows = NULL;
  doc.n_flows = 0;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Mutual TLS */
  doc.name = (char *)(size_t) "MtlsSec";
  doc.type = DOC_SEC_MUTUALTLS;
  rc = c2openapi_spec_add_security_scheme(&spec, &doc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* apply_doc_security_schemes */
  {
    struct DocMetadata meta;
    doc_metadata_init(&meta);
    rc = c2openapi_apply_doc_security_schemes(NULL, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = c2openapi_apply_doc_security_schemes(&spec, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    meta.security_schemes = &doc;
    meta.n_security_schemes = 1;
    rc = c2openapi_apply_doc_security_schemes(&spec, &meta);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    meta.security_schemes = NULL;
    meta.n_security_schemes = 0;
    doc_metadata_free(&meta);
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

#endif /* TEST_CLI_C2OPENAPI_HELPERS_H */
