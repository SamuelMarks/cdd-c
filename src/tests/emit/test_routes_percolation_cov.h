/**
 * @file test_routes_percolation_cov.h
 * @brief Step-level fault injection and edge-case branch coverage tests for
 * error percolation across routes, emitters, and parser helpers.
 */

#ifndef TEST_ROUTES_PERCOLATION_COV_H
#define TEST_ROUTES_PERCOLATION_COV_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "c_cdd/code_writer.h"
#include "c_cdd/json_dom.h"
#include "c_cdd/memory.h"
#include "c_cdd/reflect.h"
#include "c_cdd/safe_crt.h"
#include "cdd_c_error.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/emit/cli_struct.h"
#include "classes/emit/enum.h"
#include "classes/emit/json.h"
#include "classes/emit/struct.h"
#include "classes/emit/types.h"
#include "classes/parse/inspector.h"
#include "functions/parse/fs.h"
#include "functions/parse/orchestrator.h"
#include "functions/parse/vcpkg_integration.h"
#include "functions/parse/vla_analyzer.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/cli_gen.h"
#include "routes/emit/client_gen.h"
#include "routes/emit/operation.h"
#include "routes/emit/url.h"
#include "routes/parse/sync.h"
/* clang-format on */

/** @brief Prototype for `client_gen_emit_url_utils_c1`. */
extern C_CDD_EXPORT cdd_c_error_t client_gen_emit_url_utils_c1(FILE *uc);
/** @brief Prototype for `client_gen_emit_url_utils_c2`. */
extern C_CDD_EXPORT cdd_c_error_t client_gen_emit_url_utils_c2(FILE *uc);
/** @brief Prototype for `client_gen_emit_url_utils_h`. */
extern C_CDD_EXPORT cdd_c_error_t
client_gen_emit_url_utils_h(const char *dir_name);

/**
 * @brief Failing allocator for JSON DOM OOM testing without `(void)` casts.
 * @param[in] sz Requested byte size.
 * @return Always NULL.
 */
static void *cov_null_malloc(size_t sz) { return sz ? NULL : NULL; }

/**
 * @brief Tests NULL-field VLA site cleanup, vcpkg scanner step failure, and
 * orchestrator `find_token_in_range` step failure.
 * @return Greatest test result.
 */
TEST test_vla_vcpkg_orchestrator_step_cov(void) {
  struct VLASiteList vlist;
  struct VcpkgManifestBuilder vb;
  int step;

  memset(&vlist, 0, sizeof(vlist));
  vlist.sites = (struct VLASite *)C_CDD_CALLOC(1, sizeof(struct VLASite));
  ASSERT(vlist.sites != NULL);
  vlist.count = 1;
  vlist.capacity = 1;
  ASSERT_EQ(CDD_C_SUCCESS, vla_site_list_free(&vlist));

  ASSERT_EQ(CDD_C_SUCCESS, vcpkg_builder_init(&vb, "pkg", "1.0.0", "desc"));
  for (step = 1; step <= 120; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = vcpkg_builder_scan_source(&vb, "#include <a.h>\n");
    if (rc == CDD_C_SUCCESS && g_cdd_step_fail > 0) {
      g_cdd_step_fail = 0;
      break;
    }
    g_cdd_step_fail = 0;
  }
  ASSERT_EQ(CDD_C_SUCCESS, vcpkg_builder_free(&vb));

  for (step = 1; step <= 200; ++step) {
    char *out_code = NULL;
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = orchestrate_fix("void f(void){}", &out_code);
    if (out_code) {
      C_CDD_FREE(out_code);
    }
    if (rc == CDD_C_SUCCESS && g_cdd_step_fail > 0) {
      g_cdd_step_fail = 0;
      break;
    }
    g_cdd_step_fail = 0;
  }

  PASS();
}

/**
 * @brief Tests `json_dom` getter step failures and `operation_body` error
 * percolation paths.
 * @return Greatest test result.
 */
TEST test_json_dom_and_operation_body_step_cov(void) {
  int step;
  int b_out = 0;
  double d_out = 0.0;
  const char *s_out = NULL;
  JSON_Value_Type t_out = JSONError;

  for (step = 1; step <= 3; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = cdd_json_value_get_type(NULL, &t_out);
    g_cdd_step_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    g_cdd_step_fail = step;
    rc = cdd_json_value_get_boolean(NULL, &b_out);
    g_cdd_step_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    g_cdd_step_fail = step;
    rc = cdd_json_value_get_number(NULL, &d_out);
    g_cdd_step_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    g_cdd_step_fail = step;
    rc = cdd_json_value_get_string(NULL, &s_out);
    g_cdd_step_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
  }

  for (step = 1; step <= 5; ++step) {
    struct OpenAPI_Any any_val;
    cdd_c_error_t rc;

    memset(&any_val, 0, sizeof(any_val));
    g_cdd_step_fail = step;
    rc = parse_example_any("\"hello\"", &any_val);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    ASSERT_EQ(CDD_C_SUCCESS, free_any_value_local(&any_val));

    memset(&any_val, 0, sizeof(any_val));
    g_cdd_step_fail = step;
    rc = parse_example_any("42", &any_val);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    ASSERT_EQ(CDD_C_SUCCESS, free_any_value_local(&any_val));

    memset(&any_val, 0, sizeof(any_val));
    g_cdd_step_fail = step;
    rc = parse_example_any("true", &any_val);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    ASSERT_EQ(CDD_C_SUCCESS, free_any_value_local(&any_val));
  }

  {
    JSON_Value *jobj = NULL;
    struct OpenAPI_Any any_val;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string("{\"k\":1}", &jobj));
    ASSERT(jobj != NULL);

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_set_allocation_functions(cov_null_malloc, free));
    memset(&any_val, 0, sizeof(any_val));
    ASSERT_EQ(CDD_C_ERROR_MEMORY, any_from_json_value(jobj, &any_val));
    memset(&any_val, 0, sizeof(any_val));
    ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_example_any("{\"k\":1}", &any_val));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_set_allocation_functions(NULL, NULL));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(jobj));
  }

  for (step = 1; step <= 25; ++step) {
    struct OpenAPI_Any any_val;
    cdd_c_error_t rc;
    memset(&any_val, 0, sizeof(any_val));
    g_cdd_alloc_fail = step;
    rc = parse_example_any("{\"k\":1}", &any_val);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, free_any_value_local(&any_val));
    if (rc == CDD_C_SUCCESS) {
      break;
    }
  }

  for (step = 1; step <= 6; ++step) {
    struct OpenAPI_Operation op;
    cdd_c_error_t rc;
    size_t m;

    memset(&op, 0, sizeof(op));
    op.req_body.content_type = cdd_test_mut_str("application/json");
    op.req_body.inline_type = cdd_test_mut_str("string");

    g_cdd_step_fail = step;
    rc = add_request_body_media_type(&op, "text/plain", 0);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);

    if (op.req_body_media_types) {
      for (m = 0; m < op.n_req_body_media_types; ++m) {
        if (op.req_body_media_types[m].name)
          C_CDD_FREE(op.req_body_media_types[m].name);
        if (op.req_body_media_types[m].schema.inline_type)
          C_CDD_FREE(op.req_body_media_types[m].schema.inline_type);
      }
      C_CDD_FREE(op.req_body_media_types);
    }
  }

  PASS();
}

/**
 * @brief Tests `cli_gen`, `code_writer`, `client_gen_url_utils_*`,
 * `client_gen_helpers`, `client_gen_preamble`, and `client_gen_operations`
 * step-failure percolation.
 * @return Greatest test result.
 */
TEST test_cli_and_client_gen_step_cov(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter path_param;
  struct OpenAPI_Parameter op_param;
  struct OpenAPI_Server srv;
  struct OpenAPI_ServerVariable svar;
  struct OpenApiClientConfig cfg;
  char *tag_arr[1];
  FILE *tmp_h = NULL;
  FILE *tmp_c = NULL;
  int step;

  memset(&spec, 0, sizeof(spec));
  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));
  memset(&path_param, 0, sizeof(path_param));
  memset(&op_param, 0, sizeof(op_param));
  memset(&srv, 0, sizeof(srv));
  memset(&svar, 0, sizeof(svar));
  memset(&cfg, 0, sizeof(cfg));

  svar.name = cdd_test_mut_str("host");
  svar.default_value = cdd_test_mut_str("example.com");
  srv.url = cdd_test_mut_str("https://{host}/v1");
  srv.variables = &svar;
  srv.n_variables = 1;

  path_param.name = cdd_test_mut_str("petId");
  path_param.in = OA_PARAM_IN_PATH;
  path_param.type = cdd_test_mut_str("string");

  op_param.name = cdd_test_mut_str("petId");
  op_param.in = OA_PARAM_IN_PATH;
  op_param.type = cdd_test_mut_str("string");

  tag_arr[0] = cdd_test_mut_str("pet");

  op.operation_id = cdd_test_mut_str("getPet");
  op.verb = OA_VERB_GET;
  op.parameters = &op_param;
  op.n_parameters = 1;
  op.tags = tag_arr;
  op.n_tags = 1;
  op.servers = &srv;
  op.n_servers = 1;

  path.route = cdd_test_mut_str("/pets/{petId}");
  path.parameters = &path_param;
  path.n_parameters = 1;
  path.operations = &op;
  path.n_operations = 1;

  spec.paths = &path;
  spec.n_paths = 1;
  spec.servers = &srv;
  spec.n_servers = 1;

  cfg.filename_base = "test_cli_cov_dir/test_cov_cli_tmp";
  cfg.func_prefix = "api_";
  cfg.no_github_actions = 1;
  cfg.no_installable_package = 1;

  /* 1. cli_gen + cdd_serde_transform_case */
  for (step = 1; step <= 6; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = openapi_cli_generate(&spec, &cfg);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
  }
  remove("test_cli_cov_dir/src/test_cov_cli_tmp_cli.c");
#ifdef _WIN32
  _rmdir("test_cli_cov_dir/src");
  _rmdir("test_cli_cov_dir");
#else
  rmdir("test_cli_cov_dir/src");
  rmdir("test_cli_cov_dir");
#endif

  /* 2. code_writer_init_stream + client_gen_url_utils_c1/c2/h */
  tmp_h = cdd_test_tmpfile_global();
  tmp_c = cdd_test_tmpfile_global();
  ASSERT(tmp_h != NULL);
  ASSERT(tmp_c != NULL);

  for (step = 1; step <= 3; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = client_gen_emit_url_utils_c1(tmp_c);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
  }

  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, client_gen_emit_url_utils_c2(tmp_c));
  g_cdd_step_fail = 0;

#ifdef _WIN32
  _mkdir("test_uu_cov_dir");
  _mkdir("test_uu_cov_dir/src");
#else
  mkdir("test_uu_cov_dir", 0755);
  mkdir("test_uu_cov_dir/src", 0755);
#endif
  g_cdd_step_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            client_gen_emit_url_utils_h("test_uu_cov_dir"));
  g_cdd_step_fail = 0;
  remove("test_uu_cov_dir/src/cdd_c_error.h");
  remove("test_uu_cov_dir/src/c_cdd_stdbool.h");
  remove("test_uu_cov_dir/src/url_utils.h");
#ifdef _WIN32
  _rmdir("test_uu_cov_dir/src");
  _rmdir("test_uu_cov_dir");
#else
  rmdir("test_uu_cov_dir/src");
  rmdir("test_uu_cov_dir");
#endif

  /* 3. client_gen_helpers + client_gen_preamble + client_gen_operations */
  for (step = 1; step <= 6; ++step) {
    char *rendered = NULL;
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = render_server_url_default(&srv, &rendered);
    g_cdd_step_fail = 0;
    if (rendered)
      C_CDD_FREE(rendered);
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
  }

  {
    char *lit = NULL;
    g_cdd_step_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              build_base_url_literal("https://example.com", &lit));
    g_cdd_step_fail = 0;
  }

  for (step = 1; step <= 7; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = write_lifecycle_funcs(tmp_h, tmp_c, "api_", &spec);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
  }

  for (step = 1; step <= 14; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = emit_operation(tmp_h, tmp_c, &path, &op, &spec, &cfg, "api_");
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
  }

  fclose(tmp_h);
  fclose(tmp_c);
  PASS();
}

/**
 * @brief Tests `url.c`, `url_path.c`, `url_query.c`, and
 * `url_serialization.c` step-failure percolation.
 * @return Greatest test result.
 */
TEST test_url_family_step_cov(void) {
  FILE *fp = NULL;
  struct OpenAPI_Parameter path_p;
  struct OpenAPI_Parameter qs_p;
  struct OpenAPI_Parameter q_params[3];
  struct OpenAPI_Operation op_qs;
  struct OpenAPI_Operation op_q;
  int step;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);

  memset(&path_p, 0, sizeof(path_p));
  path_p.name = cdd_test_mut_str("id");
  path_p.in = OA_PARAM_IN_PATH;
  path_p.type = cdd_test_mut_str("string");

  for (step = 1; step <= 6; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = codegen_url_write_builder(fp, "/users/{id}", &path_p, 1, NULL);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
  }

  memset(&qs_p, 0, sizeof(qs_p));
  qs_p.name = cdd_test_mut_str("qs");
  qs_p.in = OA_PARAM_IN_QUERYSTRING;
  qs_p.content_type = cdd_test_mut_str("text/plain");
  qs_p.type = cdd_test_mut_str("string");

  memset(&op_qs, 0, sizeof(op_qs));
  op_qs.parameters = &qs_p;
  op_qs.n_parameters = 1;

  for (step = 1; step <= 14; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = codegen_url_write_query_params(fp, &op_qs, 0);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
  }

  memset(q_params, 0, sizeof(q_params));
  q_params[0].name = cdd_test_mut_str("qa");
  q_params[0].in = OA_PARAM_IN_QUERY;
  q_params[0].content_type = cdd_test_mut_str("application/json");
  q_params[0].is_array = 1;
  q_params[0].items_type = cdd_test_mut_str("string");

  q_params[1].name = cdd_test_mut_str("qj");
  q_params[1].in = OA_PARAM_IN_QUERY;
  q_params[1].content_type = cdd_test_mut_str("application/json");
  q_params[1].type = cdd_test_mut_str("string");

  q_params[2].name = cdd_test_mut_str("qobj");
  q_params[2].in = OA_PARAM_IN_QUERY;
  q_params[2].type = cdd_test_mut_str("CustomObj");

  memset(&op_q, 0, sizeof(op_q));
  op_q.parameters = q_params;
  op_q.n_parameters = 3;

  for (step = 1; step <= 14; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = codegen_url_write_query_params(fp, &op_q, 0);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
  }

  fclose(fp);
  PASS();
}

/**
 * @brief Tests `sync.c` token_matches_string step failures and `parse_tokens`
 * allocation failure.
 * @return Greatest test result.
 */
TEST test_sync_step_and_oom_cov(void) {
  static const char *const k_src = "int api_getPet(void) {\n"
                                   "  url_query_init(&qp);\n"
                                   "  url_query_build(&qp, &q);\n"
                                   "  /* Header Parameter: X-Token */\n"
                                   "  { set_hdr(\"X-Token\"); }\n"
                                   "  asprintf(&url, \"%s/pets\", base);\n"
                                   "  return 0;\n"
                                   "}\n";
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter params[2];
  struct ApiSyncConfig sync_cfg;
  int step;

  memset(&spec, 0, sizeof(spec));
  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));
  memset(params, 0, sizeof(params));
  memset(&sync_cfg, 0, sizeof(sync_cfg));

  params[0].name = cdd_test_mut_str("q");
  params[0].in = OA_PARAM_IN_QUERY;
  params[0].type = cdd_test_mut_str("string");

  params[1].name = cdd_test_mut_str("X-Token");
  params[1].in = OA_PARAM_IN_HEADER;
  params[1].type = cdd_test_mut_str("string");

  op.operation_id = cdd_test_mut_str("getPet");
  op.verb = OA_VERB_GET;
  op.parameters = params;
  op.n_parameters = 2;

  path.route = cdd_test_mut_str("/pets");
  path.operations = &op;
  path.n_operations = 1;

  spec.paths = &path;
  spec.n_paths = 1;

  sync_cfg.func_prefix = "api_";
  sync_cfg.url_var_name = "url";

  {
    struct TokenList *tl = NULL;
    struct CstNodeList cst;
    memset(&cst, 0, sizeof(cst));
    ASSERT_EQ(CDD_C_SUCCESS, tokenize(az_span_create_from_str(k_src), &tl));
    ASSERT_EQ(CDD_C_SUCCESS, parse_tokens(tl, &cst));

    for (step = 1; step <= 200; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = cdd_test_sync_apply_updates("test_sync_cov_tmp.c", tl, &cst, &spec,
                                       &sync_cfg);
      if (rc == CDD_C_SUCCESS && g_cdd_step_fail > 0) {
        g_cdd_step_fail = 0;
        break;
      }
      g_cdd_step_fail = 0;
    }

    ASSERT_EQ(CDD_C_SUCCESS, free_cst_node_list(&cst));
    ASSERT_EQ(CDD_C_SUCCESS, free_token_list(tl));
  }

  for (step = 1; step <= 35; ++step) {
    cdd_c_error_t rc;
    ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_sync_cov_tmp.c", k_src));
    g_cdd_alloc_fail = step;
    rc = api_sync_file("test_sync_cov_tmp.c", &spec, &sync_cfg);
    g_cdd_alloc_fail = 0;
    if (rc == CDD_C_SUCCESS) {
      break;
    }
  }

  remove("test_sync_cov_tmp.c");
  PASS();
}

/**
 * @brief Tests step-failure and OOM error percolation across `enum.c`,
 * `standalone_json.c`, `types.c`, `struct_fields.c`, `cli_struct.c`,
 * `cli_struct_serialize.c`, and `json.c`.
 * @return Greatest test result.
 */
TEST test_classes_emit_step_cov(void) {
  FILE *fp = NULL;
  const char *out_tn = NULL;
  struct EnumMembers em;
  int step;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);

  /* 1. struct_fields.c + code2schema_names.c */
  for (step = 1; step <= 5; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = get_type_from_ref("#/components/schemas/123bad", &out_tn);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
  }
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            get_type_from_ref("#/components/schemas/123bad", &out_tn));
  g_cdd_alloc_fail = 0;

  /* 2. enum.c + cdd_serde.c:cdd_escape_json_c_str */
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_init(&em));
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_add(&em, "FOO"));
  ASSERT_EQ(CDD_C_SUCCESS, enum_member_add_alias(&em, 0, "bar"));

  for (step = 1; step <= 6; ++step) {
    cdd_c_error_t rc;
    g_cdd_step_fail = step;
    rc = write_enum_to_str_func(fp, "MyEnum", &em, NULL);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);

    g_cdd_step_fail = step;
    rc = write_enum_from_str_func(fp, "MyEnum", &em, NULL);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);

    g_cdd_step_fail = step;
    rc = write_enum_reflect_func(fp, "MyEnum", &em, NULL);
    g_cdd_step_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
  }
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_free(&em));

  /* 3. standalone_json.c */
  {
    struct StructFields sf;
    struct StructField flds[2];
    char *al0[1];
    char *al1[1];
    memset(&sf, 0, sizeof(sf));
    memset(flds, 0, sizeof(flds));
    al0[0] = cdd_test_mut_str("s_alt");
    al1[0] = cdd_test_mut_str("n_alt");

    CDD_STRCPY(flds[0].name, sizeof(flds[0].name), "s");
    CDD_STRCPY(flds[0].type, sizeof(flds[0].type), "string");
    CDD_STRCPY(flds[0].default_val, sizeof(flds[0].default_val), "abc");
    flds[0].serde_aliases = al0;
    flds[0].n_serde_aliases = 1;

    CDD_STRCPY(flds[1].name, sizeof(flds[1].name), "n");
    CDD_STRCPY(flds[1].type, sizeof(flds[1].type), "integer");
    flds[1].serde_aliases = al1;
    flds[1].n_serde_aliases = 1;

    sf.fields = flds;
    sf.size = 2;

    for (step = 1; step <= 12; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = write_struct_from_json_standalone_func(fp, "MySt", &sf);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);

      g_cdd_step_fail = step;
      rc = write_struct_to_json_standalone_func(fp, "MySt", &sf);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }
  }

  /* 4. types.c */
  {
    struct StructFields usf;
    struct StructField ufld;
    char *ual[1];
    memset(&usf, 0, sizeof(usf));
    memset(&ufld, 0, sizeof(ufld));
    ual[0] = cdd_test_mut_str("v1_alt");
    CDD_STRCPY(ufld.name, sizeof(ufld.name), "v1");
    CDD_STRCPY(ufld.type, sizeof(ufld.type), "object");
    CDD_STRCPY(ufld.ref, sizeof(ufld.ref), "#/components/schemas/Sub");
    ufld.serde_aliases = ual;
    ufld.n_serde_aliases = 1;

    usf.fields = &ufld;
    usf.size = 1;
    CDD_STRCPY(usf.serde_tag, sizeof(usf.serde_tag), "kind");
    CDD_STRCPY(usf.serde_content, sizeof(usf.serde_content), "data");

    for (step = 1; step <= 8; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = write_union_to_json_func(fp, "MyU", &usf, NULL);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);

      g_cdd_step_fail = step;
      rc = write_union_from_jsonObject_func(fp, "MyU", &usf, NULL);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }

    usf.serde_content[0] = '\0';
    for (step = 1; step <= 6; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = write_union_to_json_func(fp, "MyU", &usf, NULL);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }
  }

  /* 5. cli_struct.c + cli_struct_serialize.c + struct_reflect_access.c */
  {
    struct StructFields csf;
    struct StructField cflds[2];
    struct StructFields sub_sf;
    struct StructField sub_flds[2];
    struct TypeDefinition tdef;
    struct TypeDefList treg;
    struct CodegenCliConfig ccfg;
    char env_out[64];
    const char *cands[2];
    const char *best = NULL;

    memset(&csf, 0, sizeof(csf));
    memset(cflds, 0, sizeof(cflds));
    memset(&sub_sf, 0, sizeof(sub_sf));
    memset(sub_flds, 0, sizeof(sub_flds));
    memset(&tdef, 0, sizeof(tdef));
    memset(&treg, 0, sizeof(treg));
    memset(&ccfg, 0, sizeof(ccfg));

    CDD_STRCPY(cflds[0].name, sizeof(cflds[0].name), "optVal");
    CDD_STRCPY(cflds[0].type, sizeof(cflds[0].type), "string");
    CDD_STRCPY(cflds[0].cli_env, sizeof(cflds[0].cli_env), "1");
    for (step = 1; step <= 3; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = cdd_cli_resolve_field_env(&csf, &cflds[0], env_out, sizeof(env_out));
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }
    CDD_STRCPY(cflds[0].cli_env, sizeof(cflds[0].cli_env), "MY_ENV");
    for (step = 1; step <= 3; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = cdd_cli_resolve_field_env(&csf, &cflds[0], env_out, sizeof(env_out));
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }

    cands[0] = "--fao";
    cands[1] = "--bar";
    for (step = 1; step <= 5; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = cdd_cli_suggest_closest("--foo", cands, 2, &best);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }

    {
      int dummy_target = 0;
      struct cdd_field_info rfi;
      struct cdd_type_info rti;
      char *rargv[2];
      memset(&rfi, 0, sizeof(rfi));
      memset(&rti, 0, sizeof(rti));
      rfi.name = "flag";
      rfi.wire_name = "flag";
      rfi.kind = CDD_FIELD_KIND_BOOLEAN;
      rfi.size = sizeof(int);
      rti.name = "Dummy";
      rti.size = sizeof(int);
      rti.fields = &rfi;
      rti.n_fields = 1;
      rargv[0] = cdd_test_mut_str("prog");
      rargv[1] = cdd_test_mut_str("--flag");
      for (step = 1; step <= 3; ++step) {
        cdd_c_error_t rc;
        g_cdd_step_fail = step;
        rc = cdd_cli_parse_reflected_argv(2, rargv, 1, &rti, &dummy_target,
                                          NULL, NULL);
        g_cdd_step_fail = 0;
        ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
      }
    }

    CDD_STRCPY(cflds[1].name, sizeof(cflds[1].name), "cmd");
    CDD_STRCPY(cflds[1].type, sizeof(cflds[1].type), "object");
    CDD_STRCPY(cflds[1].ref, sizeof(cflds[1].ref), "#/components/schemas/SubU");
    cflds[1].cli_subcommand = 1;

    CDD_STRCPY(sub_flds[0].name, sizeof(sub_flds[0].name), "vOne");
    CDD_STRCPY(sub_flds[0].type, sizeof(sub_flds[0].type), "object");
    CDD_STRCPY(sub_flds[0].ref, sizeof(sub_flds[0].ref),
               "#/components/schemas/Cmd1");
    CDD_STRCPY(sub_flds[0].serde_rename, sizeof(sub_flds[0].serde_rename),
               "one");

    CDD_STRCPY(sub_flds[1].name, sizeof(sub_flds[1].name), "vTwo");
    CDD_STRCPY(sub_flds[1].type, sizeof(sub_flds[1].type), "object");
    CDD_STRCPY(sub_flds[1].ref, sizeof(sub_flds[1].ref),
               "#/components/schemas/Cmd2");

    sub_sf.fields = sub_flds;
    sub_sf.size = 2;

    tdef.kind = KIND_STRUCT;
    tdef.name = cdd_test_mut_str("SubU");
    tdef.details.struct_fields = &sub_sf;
    treg.items = &tdef;
    treg.size = 1;
    ccfg.type_registry = &treg;

    csf.fields = cflds;
    csf.size = 2;

    for (step = 1; step <= 6; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = write_struct_print_version_func(fp, "MyCli", &csf, &ccfg);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }
    for (step = 1; step <= 18; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = write_struct_to_cli_argv_func(fp, "MyCli", &csf, &ccfg);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }

    CDD_STRCPY(csf.cli_name, sizeof(csf.cli_name), "mycli");
    for (step = 1; step <= 6; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = write_struct_print_version_func(fp, "MyCli", &csf, &ccfg);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);

      g_cdd_step_fail = step;
      rc = write_struct_to_cli_argv_func(fp, "MyCli", &csf, &ccfg);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }

    CDD_STRCPY(csf.cli_bin_name, sizeof(csf.cli_bin_name), "mybin");
    for (step = 1; step <= 4; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = write_struct_to_cli_argv_func(fp, "MyCli", &csf, &ccfg);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }
  }

  /* 6. json.c */
  {
    struct StructFields jsf;
    struct StructField jflds[4];
    char *jal[1];
    memset(&jsf, 0, sizeof(jsf));
    memset(jflds, 0, sizeof(jflds));
    jal[0] = cdd_test_mut_str("s_alt");

    CDD_STRCPY(jflds[0].name, sizeof(jflds[0].name), "s");
    CDD_STRCPY(jflds[0].type, sizeof(jflds[0].type), "string");
    jflds[0].serde_aliases = jal;
    jflds[0].n_serde_aliases = 1;

    CDD_STRCPY(jflds[1].name, sizeof(jflds[1].name), "o");
    CDD_STRCPY(jflds[1].type, sizeof(jflds[1].type), "object");
    CDD_STRCPY(jflds[1].ref, sizeof(jflds[1].ref), "#/components/schemas/Sub");

    CDD_STRCPY(jflds[2].name, sizeof(jflds[2].name), "e");
    CDD_STRCPY(jflds[2].type, sizeof(jflds[2].type), "enum");
    CDD_STRCPY(jflds[2].ref, sizeof(jflds[2].ref),
               "#/components/schemas/MyEnum");

    CDD_STRCPY(jflds[3].name, sizeof(jflds[3].name), "arr");
    CDD_STRCPY(jflds[3].type, sizeof(jflds[3].type), "array");
    CDD_STRCPY(jflds[3].ref, sizeof(jflds[3].ref), "#/components/schemas/Sub");

    jsf.fields = jflds;
    jsf.size = 4;
    jsf.serde_deny_unknown_fields = 1;
    jsf.serde_rename_all = CDD_SERDE_CASE_CAMELCASE;

    for (step = 1; step <= 35; ++step) {
      cdd_c_error_t rc;
      g_cdd_step_fail = step;
      rc = write_struct_to_json_func(fp, "MyJ", &jsf, NULL);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);

      g_cdd_step_fail = step;
      rc = write_struct_to_jsonObject_func(fp, "MyJ", &jsf, NULL);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);

      g_cdd_step_fail = step;
      rc = write_struct_from_jsonObject_func(fp, "MyJ", &jsf, NULL);
      g_cdd_step_fail = 0;
      ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
    }
  }

  fclose(fp);
  PASS();
}

/**
 * @brief Suite covering step-level error percolation across routes and helpers.
 */
SUITE(routes_percolation_cov_suite) {
  RUN_TEST(test_vla_vcpkg_orchestrator_step_cov);
  RUN_TEST(test_json_dom_and_operation_body_step_cov);
  RUN_TEST(test_cli_and_client_gen_step_cov);
  RUN_TEST(test_url_family_step_cov);
  RUN_TEST(test_sync_step_and_oom_cov);
  RUN_TEST(test_classes_emit_step_cov);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_ROUTES_PERCOLATION_COV_H */
