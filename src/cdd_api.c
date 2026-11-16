/**
 * @file cdd_api.c
 * @brief Core cdd api definitions and implementation.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_api.h"
#include "c_cdd/memory.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_scope.h"
#include "classes/parse/cdd_cst_semantic.h"
#include "dsl/cdd_dsl_parser.h"
#include "dsl/cdd_ffi_emit_dsl.h"
#include "functions/ffi/cdd_ffi_emit_ada.h"
#include "functions/ffi/cdd_ffi_emit_clojure.h"
#include "functions/ffi/cdd_ffi_emit_cobol.h"
#include "functions/ffi/cdd_ffi_emit_common_lisp.h"
#include "functions/ffi/cdd_ffi_emit_cpp.h"
#include "functions/ffi/cdd_ffi_emit_crystal.h"
#include "functions/ffi/cdd_ffi_emit_csharp.h"
#include "functions/ffi/cdd_ffi_emit_d.h"
#include "functions/ffi/cdd_ffi_emit_dart.h"
#include "functions/ffi/cdd_ffi_emit_delphi.h"
#include "functions/ffi/cdd_ffi_emit_elixir.h"
#include "functions/ffi/cdd_ffi_emit_erlang.h"
#include "functions/ffi/cdd_ffi_emit_fortran.h"
#include "functions/ffi/cdd_ffi_emit_fsharp.h"
#include "functions/ffi/cdd_ffi_emit_go.h"
#include "functions/ffi/cdd_ffi_emit_groovy.h"
#include "functions/ffi/cdd_ffi_emit_haskell.h"
#include "functions/ffi/cdd_ffi_emit_java.h"
#include "functions/ffi/cdd_ffi_emit_julia.h"
#include "functions/ffi/cdd_ffi_emit_kotlin.h"
#include "functions/ffi/cdd_ffi_emit_lua.h"
#include "functions/ffi/cdd_ffi_emit_matlab.h"
#include "functions/ffi/cdd_ffi_emit_napi.h"
#include "functions/ffi/cdd_ffi_emit_nim.h"
#include "functions/ffi/cdd_ffi_emit_objc.h"
#include "functions/ffi/cdd_ffi_emit_ocaml.h"
#include "functions/ffi/cdd_ffi_emit_odin.h"
#include "functions/ffi/cdd_ffi_emit_perl.h"
#include "functions/ffi/cdd_ffi_emit_php.h"
#include "functions/ffi/cdd_ffi_emit_python.h"
#include "functions/ffi/cdd_ffi_emit_r.h"
#include "functions/ffi/cdd_ffi_emit_racket.h"
#include "functions/ffi/cdd_ffi_emit_ruby.h"
#include "functions/ffi/cdd_ffi_emit_rust.h"
#include "functions/ffi/cdd_ffi_emit_scala.h"
#include "functions/ffi/cdd_ffi_emit_scheme.h"
#include "functions/ffi/cdd_ffi_emit_swift.h"
#include "functions/ffi/cdd_ffi_emit_tcl.h"
#include "functions/ffi/cdd_ffi_emit_typescript.h"
#include "functions/ffi/cdd_ffi_emit_vlang.h"
#include "functions/ffi/cdd_ffi_emit_webassembly.h"
#include "functions/ffi/cdd_ffi_emit_zig.h"
#include "functions/ffi/cdd_ffi_emit_ir_json.h"
#include "functions/ffi/cdd_ffi_ir_extractor.h"
#include "arazzo/emit/arazzo.h"
#include "arazzo/parse/arazzo.h"
#include "arazzo/parse/c2arazzo.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/arazzo_cli_gen.h"
#include "routes/emit/arazzo_client_gen.h"
#include "routes/emit/arazzo_server_gen.h"
#include "routes/emit/cli_gen.h"
#include "routes/emit/client_gen.h"
#include "routes/emit/client_gui_gen.h"
#include "routes/emit/serve_json_rpc.h"
#include "routes/emit/server_gen.h"
#include "routes/parse/cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Runs the selected OpenAPI target code generator(s) (`server`, `sdk`,
 * `gui`, and optional `cli`).
 *
 * @param[in] spec Loaded OpenAPI specification.
 * @param[in] client_cfg Output path and generation options.
 * @param[in] is_server Non-zero to emit server stubs instead of client SDK.
 * @param[in] is_cli Non-zero to emit CLI client alongside SDK and GUI.
 * @return `CDD_C_SUCCESS` on success, or generator error code.
 */
static cdd_c_error_t
run_openapi_target_generators(const struct OpenAPI_Spec *spec,
                              const struct OpenApiClientConfig *client_cfg,
                              int is_server, int is_cli) {
  cdd_c_error_t rc;
  if (is_server) {
    return openapi_server_generate(spec, client_cfg);
  }
  rc = openapi_client_generate(spec, client_cfg);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = openapi_client_gui_generate(spec, client_cfg);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_cli) {
    return openapi_cli_generate(spec, client_cfg);
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Generate C code (SDK, CLI, or Server) from an OpenAPI specification.
 *
 * @param[in] config Configuration parameters for OpenAPI-to-C generation.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
cdd_c_error_t
cdd_generate_from_openapi(const cdd_from_openapi_config_t *config) {
  int is_cli;
  int is_server;
  struct OpenAPI_Spec spec = {0};
  struct OpenApiClientConfig client_cfg = {0};
  cdd_c_error_t rc = CDD_C_SUCCESS;
  JSON_Value *root;

  if (!config || (!config->input && !config->input_dir)) {
    fprintf(stderr,
            "Error: -i|--input <spec.json> or --input-dir <dir> required\n");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  is_cli = (config->subcommand == CDD_OPENAPI_SUBCOMMAND_TO_SDK_CLI);
  is_server = (config->subcommand == CDD_OPENAPI_SUBCOMMAND_TO_SERVER);

  client_cfg.no_github_actions = (int)config->no_github_actions;
  client_cfg.no_installable_package = (int)config->no_installable_package;
  client_cfg.create_tests_and_mocks = (int)config->tests;

  if (config->input) {
    char *alloc_path = NULL;
    rc = cdd_json_parse_file(config->input, &root);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Failed to parse JSON file: %s\n", config->input);
      return cdd_c_keep_err(CDD_C_ERROR_PARSE, rc);
    }

    rc = openapi_load_from_json(root, &spec);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Failed to load openapi spec from %s\n", config->input);
      return cdd_c_keep_err(rc, cdd_json_value_free(root));
    }

    if (config->output) {
      rc = fs_path_join(config->output, "generated_client", &alloc_path);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(cdd_c_keep_err(rc, cdd_json_value_free(root)),
                              openapi_spec_free(&spec));
      }
      client_cfg.filename_base = alloc_path;
    } else {
      client_cfg.filename_base = "test_build_dir/generated_client";
    }
    client_cfg.func_prefix = "api_";

    rc = run_openapi_target_generators(&spec, &client_cfg, is_server, is_cli);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(alloc_path);
      return cdd_c_keep_err(cdd_c_keep_err(rc, cdd_json_value_free(root)),
                            openapi_spec_free(&spec));
    }
    C_CDD_FREE(alloc_path);
    return cdd_c_keep_err(cdd_json_value_free(root), openapi_spec_free(&spec));
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Generate an OpenAPI specification from existing C source code.
 *
 * @param[in] config Configuration parameters for C-to-OpenAPI extraction.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
cdd_c_error_t cdd_generate_to_openapi(const cdd_to_openapi_config_t *config) {
  const char *input_dir;
  const char *out_file;
  char *snapshot_path = NULL;
  FILE *f = NULL;
  cdd_c_error_t rc_join;
  cdd_to_openapi_config_t run_cfg;

  if (!config || !config->input) {
    fprintf(stderr, "Error: -i <directory> required\n");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  input_dir = config->input;
  out_file = config->output ? config->output : "openapi.json";

  memset(&run_cfg, 0, sizeof(run_cfg));
  run_cfg.input = input_dir;
  run_cfg.output = out_file;
  run_cfg.base_file = config->base_file;
  run_cfg.self_uri = config->self_uri;
  run_cfg.dialect_uri = config->dialect_uri;

  if (!run_cfg.base_file) {
    rc_join = fs_path_join(input_dir, "openapi.snapshot.json", &snapshot_path);
    if (rc_join == CDD_C_SUCCESS) {
#if defined(_MSC_VER)
      if (fopen_s(&f, snapshot_path, "r") != 0)
        f = NULL;
#else
      f = fopen(snapshot_path, "r");
#endif
      if (f) {
        cdd_c_error_t gen_rc;
        fclose(f);
        run_cfg.base_file = snapshot_path;
        gen_rc = c2openapi_generate(&run_cfg);
        if (gen_rc != CDD_C_SUCCESS) {
          C_CDD_FREE(snapshot_path);
          return gen_rc;
        }
        C_CDD_FREE(snapshot_path);
        return rc_join;
      }
      C_CDD_FREE(snapshot_path);
    }
  }

  return c2openapi_generate(&run_cfg);
}

/**
 * @brief Allocates a `JSON_Value` object and retrieves its inner `JSON_Object`.
 *
 * @param[out] out_val Output `JSON_Value` pointer.
 * @param[out] out_obj Output `JSON_Object` pointer.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
 */
static cdd_c_error_t init_json_object_pair(JSON_Value **out_val,
                                           JSON_Object **out_obj) {
  cdd_c_error_t rc = cdd_json_value_init_object(out_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_json_value_get_object(*out_val, out_obj);
}

/**
 * @brief Sets `key` on `obj` to newly created `val`, freeing `val` on failure.
 *
 * @param[in,out] obj Target `JSON_Object`.
 * @param[in] key Property name.
 * @param[in] val Newly created `JSON_Value` whose ownership is transferred.
 * @return `CDD_C_SUCCESS` on success, or error code on OOM.
 */
static cdd_c_error_t set_json_object_owned(JSON_Object *obj, const char *key,
                                           JSON_Value *val) {
  cdd_c_error_t rc = cdd_json_object_set_value(obj, key, val);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(val));
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Build a structured documentation code JSON object.
 *
 * @param[in] snippet Core C SDK invocation snippet string.
 * @param[in] no_imports Non-zero to omit the imports field.
 * @param[in] no_wrapping Non-zero to omit wrapper_start and wrapper_end fields.
 * @param[out] out_val Pointer to receive the constructed JSON_Value object.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
static cdd_c_error_t cdd_docs_build_code_val(const char *snippet,
                                             int no_imports, int no_wrapping,
                                             JSON_Value **out_val) {
  JSON_Value *code_val = NULL;
  JSON_Object *code_obj = NULL;
  cdd_c_error_t rc;

  rc = init_json_object_pair(&code_val, &code_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (!no_imports) {
    rc = cdd_json_object_set_string(
        code_obj, "imports",
        "#include \"generated_client.h\"\n#include <stdio.h>");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (!no_wrapping) {
    rc = cdd_json_object_set_string(
        code_obj, "wrapper_start",
        "int main(void) {\n  struct HttpClient client;\n  struct ApiError *err "
        "= NULL;\n  cdd_c_error_t rc;");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  rc = cdd_json_object_set_string(code_obj, "snippet", snippet);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (!no_wrapping) {
    rc = cdd_json_object_set_string(
        code_obj, "wrapper_end",
        "  api_cleanup(&client);\n  return (int)CDD_C_SUCCESS;\n}");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  *out_val = code_val;
  return CDD_C_SUCCESS;

cleanup: { return cdd_c_keep_err(rc, cdd_json_value_free(code_val)); }
}

/**
 * @brief Format the C SDK invocation snippet for an OpenAPI operation and
 * build its documentation JSON object.
 *
 * @param[in] op OpenAPI operation structure.
 * @param[in] no_imports Non-zero to omit the imports field.
 * @param[in] no_wrapping Non-zero to omit the 2-space leading indentation and
 * wrapper fields.
 * @param[out] out_val Output pointer receiving the constructed code JSON_Value.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
static cdd_c_error_t
cdd_docs_build_op_code_val(const struct OpenAPI_Operation *op, int no_imports,
                           int no_wrapping, JSON_Value **out_val) {
  const char *op_id = op->operation_id ? op->operation_id : "unknown";
  const char *ind = no_wrapping ? "" : "  ";
  char snippet[4096];
  char decls[2048];
  char call_args[1024];
  size_t k;

  decls[0] = '\0';
  call_args[0] = '\0';

  if (no_wrapping) {
    CDD_STRCAT(decls, sizeof(decls), "cdd_c_error_t rc;\n");
  }

  for (k = 0; k < op->n_parameters; k++) {
    const struct OpenAPI_Parameter *param = &op->parameters[k];
    const char *pname = param->name;
    const char *ptype = param->type;
    char decl_line[256];
    char arg_item[128];

    if (param->is_array) {
      CDD_SNPRINTF(decl_line, sizeof(decl_line),
                   "%sconst char *%s[] = {\"example\"};\n", ind, pname);
      CDD_SNPRINTF(arg_item, sizeof(arg_item), ", %s, 1", pname);
    } else if (strcmp(ptype, "integer") == 0) {
      CDD_SNPRINTF(decl_line, sizeof(decl_line), "%sint %s = 1;\n", ind, pname);
      CDD_SNPRINTF(arg_item, sizeof(arg_item), ", %s", pname);
    } else if (strcmp(ptype, "number") == 0) {
      CDD_SNPRINTF(decl_line, sizeof(decl_line), "%sdouble %s = 1.0;\n", ind,
                   pname);
      CDD_SNPRINTF(arg_item, sizeof(arg_item), ", %s", pname);
    } else {
      CDD_SNPRINTF(decl_line, sizeof(decl_line),
                   "%sconst char *%s = \"example\";\n", ind, pname);
      CDD_SNPRINTF(arg_item, sizeof(arg_item), ", %s", pname);
    }
    CDD_STRCAT(decls, sizeof(decls), decl_line);
    CDD_STRCAT(call_args, sizeof(call_args), arg_item);
  }

  if (op->req_body.ref_name) {
    char decl_line[256];
    CDD_SNPRINTF(decl_line, sizeof(decl_line), "%sstruct %s req_body = {0};\n",
                 ind, op->req_body.ref_name);
    CDD_STRCAT(decls, sizeof(decls), decl_line);
    CDD_STRCAT(call_args, sizeof(call_args), ", &req_body");
  } else if (op->req_body.content_type) {
    char decl_line[256];
    CDD_SNPRINTF(decl_line, sizeof(decl_line),
                 "%sconst char *req_body = \"{}\";\n", ind);
    CDD_STRCAT(decls, sizeof(decls), decl_line);
    CDD_STRCAT(call_args, sizeof(call_args), ", req_body");
  }

  if (no_wrapping) {
    CDD_SNPRINTF(snippet, sizeof(snippet),
                 "%s/* Call the %s API */\nrc = "
                 "api_%s(&client%s, &err);\nif (rc != CDD_C_SUCCESS) {\n  "
                 "fprintf(stderr, \"Error: %%d\\n\", rc);\n}",
                 decls, op_id, op_id, call_args);
  } else {
    CDD_SNPRINTF(snippet, sizeof(snippet),
                 "%s%sapi_init(&client, \"https://api.example.com\");\n"
                 "%s/* Call the %s API */\n%src = "
                 "api_%s(&client%s, &err);\n%sif (rc != CDD_C_SUCCESS) {\n%s  "
                 "fprintf(stderr, \"Error: %%d\\n\", rc);\n%s}",
                 decls, ind, ind, op_id, ind, op_id, call_args, ind, ind, ind);
  }
  return cdd_docs_build_code_val(snippet, no_imports, no_wrapping, out_val);
}

/**
 * @brief Populate a documentation JSON route map from an array of OpenAPI
 * paths.
 *
 * @param[in,out] target_obj Target JSON_Object (`endpoints` or `webhooks`).
 * @param[in] paths Array of OpenAPI_Path items.
 * @param[in] n_paths Number of path items.
 * @param[in] no_imports Non-zero to omit imports.
 * @param[in] no_wrapping Non-zero to omit main() wrapper.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
static cdd_c_error_t
cdd_docs_populate_path_map(JSON_Object *target_obj,
                           const struct OpenAPI_Path *paths, size_t n_paths,
                           int no_imports, int no_wrapping) {
  size_t p, op_idx;
  cdd_c_error_t rc;

  for (p = 0; p < n_paths; p++) {
    const struct OpenAPI_Path *pi = &paths[p];
    JSON_Value *path_val = NULL;
    JSON_Object *path_obj = NULL;
    rc = init_json_object_pair(&path_val, &path_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = set_json_object_owned(target_obj, pi->route, path_val);
    if (rc != CDD_C_SUCCESS)
      return rc;

    for (op_idx = 0; op_idx < pi->n_operations; op_idx++) {
      const struct OpenAPI_Operation *op = &pi->operations[op_idx];
      const char *method = "";
      JSON_Value *code_val = NULL;

      switch (op->verb) {
      case OA_VERB_GET:
        method = "get";
        break;
      case OA_VERB_POST:
        method = "post";
        break;
      case OA_VERB_PUT:
        method = "put";
        break;
      case OA_VERB_DELETE:
        method = "delete";
        break;
      case OA_VERB_PATCH:
        method = "patch";
        break;
      case OA_VERB_HEAD:
        method = "head";
        break;
      case OA_VERB_OPTIONS:
        method = "options";
        break;
      case OA_VERB_TRACE:
        method = "trace";
        break;
      default:
        method = "custom";
        break;
      }

      rc = cdd_docs_build_op_code_val(op, no_imports, no_wrapping, &code_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = set_json_object_owned(path_obj, method, code_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Builds the root documentation `JSON_Value` for a loaded Arazzo spec.
 *
 * @param[in] az_spec Loaded Arazzo specification.
 * @param[in] no_imports Non-zero to omit imports.
 * @param[in] no_wrapping Non-zero to omit `main()` wrapper.
 * @param[out] out_root_val Output pointer receiving the root `JSON_Value`.
 * @return `CDD_C_SUCCESS` on success, or error code on OOM.
 */
static cdd_c_error_t build_arazzo_docs_root(const struct Arazzo_Spec *az_spec,
                                            int no_imports, int no_wrapping,
                                            JSON_Value **out_root_val) {
  size_t w;
  const char *ind = no_wrapping ? "" : "  ";
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Value *endpoints_val = NULL;
  JSON_Value *workflows_val = NULL;
  JSON_Object *workflows_obj = NULL;
  cdd_c_error_t rc;

  rc = init_json_object_pair(&root_val, &root_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_value_init_object(&endpoints_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = set_json_object_owned(root_obj, "endpoints", endpoints_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = init_json_object_pair(&workflows_val, &workflows_obj);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = set_json_object_owned(root_obj, "workflows", workflows_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (w = 0; w < az_spec->n_workflows; w++) {
    const char *wf_id = az_spec->workflows[w].workflow_id;
    char snippet[1024];
    JSON_Value *code_val = NULL;

    if (no_wrapping) {
      CDD_SNPRINTF(snippet, sizeof(snippet),
                   "cdd_c_error_t rc;\n"
                   "struct arazzo_%s_inputs inputs = {0};\n"
                   "struct arazzo_%s_outputs outputs = {0};\n"
                   "rc = arazzo_run_workflow_%s(&clients, &inputs, "
                   "&outputs);\n"
                   "if (rc != CDD_C_SUCCESS) {\n"
                   "  fprintf(stderr, \"Error: %%d\\n\", rc);\n"
                   "}\n"
                   "arazzo_%s_outputs_free(&outputs);",
                   wf_id, wf_id, wf_id, wf_id);
    } else {
      CDD_SNPRINTF(snippet, sizeof(snippet),
                   "%sstruct arazzo_%s_inputs inputs = {0};\n"
                   "%sstruct arazzo_%s_outputs outputs = {0};\n"
                   "%sapi_init(&client, \"https://api.example.com\");\n"
                   "%src = arazzo_run_workflow_%s(&clients, &inputs, "
                   "&outputs);\n"
                   "%sif (rc != CDD_C_SUCCESS) {\n"
                   "%s  fprintf(stderr, \"Error: %%d\\n\", rc);\n"
                   "%s}\n"
                   "%sarazzo_%s_outputs_free(&outputs);",
                   ind, wf_id, ind, wf_id, ind, ind, wf_id, ind, ind, ind, ind,
                   wf_id);
    }
    rc = cdd_docs_build_code_val(snippet, no_imports, no_wrapping, &code_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = set_json_object_owned(workflows_obj, wf_id, code_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  *out_root_val = root_val;
  return CDD_C_SUCCESS;

cleanup: { return cdd_c_keep_err(rc, cdd_json_value_free(root_val)); }
}

/**
 * @brief Builds the root documentation `JSON_Value` for a loaded OpenAPI spec.
 *
 * @param[in] spec Loaded OpenAPI specification.
 * @param[in] no_imports Non-zero to omit imports.
 * @param[in] no_wrapping Non-zero to omit `main()` wrapper.
 * @param[out] out_root_val Output pointer receiving the root `JSON_Value`.
 * @return `CDD_C_SUCCESS` on success, or error code on OOM.
 */
static cdd_c_error_t build_openapi_docs_root(const struct OpenAPI_Spec *spec,
                                             int no_imports, int no_wrapping,
                                             JSON_Value **out_root_val) {
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Value *endpoints_val = NULL;
  JSON_Object *endpoints_obj = NULL;
  cdd_c_error_t rc;

  rc = init_json_object_pair(&root_val, &root_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = init_json_object_pair(&endpoints_val, &endpoints_obj);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = set_json_object_owned(root_obj, "endpoints", endpoints_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_docs_populate_path_map(endpoints_obj, spec->paths, spec->n_paths,
                                  no_imports, no_wrapping);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (spec->n_webhooks > 0) {
    JSON_Value *webhooks_val = NULL;
    JSON_Object *webhooks_obj = NULL;
    rc = init_json_object_pair(&webhooks_val, &webhooks_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = set_json_object_owned(root_obj, "webhooks", webhooks_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_docs_populate_path_map(webhooks_obj, spec->webhooks,
                                    spec->n_webhooks, no_imports, no_wrapping);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  *out_root_val = root_val;
  return CDD_C_SUCCESS;

cleanup: { return cdd_c_keep_err(rc, cdd_json_value_free(root_val)); }
}

/**
 * @brief Writes serialized documentation JSON once `is_dir` has been resolved.
 *
 * @param[in] output_target Non-NULL output file or directory path.
 * @param[in] serialized Null-terminated pretty-printed JSON string.
 * @param[in] is_dir_ptr Pointer to directory flag populated by caller.
 * @param[in] dir_rc Status from `fs_is_directory`.
 * @return `CDD_C_SUCCESS` on success, or error code on OOM/I/O failure.
 */
static cdd_c_error_t write_docs_json_target_resolved(const char *output_target,
                                                     const char *serialized,
                                                     const int *is_dir_ptr,
                                                     cdd_c_error_t dir_rc) {
  if (*is_dir_ptr) {
    char *out_file = NULL;
    cdd_c_error_t rc = fs_path_join(output_target, "docs.json", &out_file);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = fs_write_to_file(out_file, serialized);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(out_file);
      return rc;
    }
    C_CDD_FREE(out_file);
    return dir_rc;
  }
  return cdd_c_keep_err(fs_write_to_file(output_target, serialized), dir_rc);
}

/**
 * @brief Writes serialized documentation JSON to `output_target` (directory or
 * file path) or `stdout` if `output_target` is `NULL`.
 *
 * @param[in] output_target Optional output file or directory path.
 * @param[in] serialized Null-terminated pretty-printed JSON string.
 * @return `CDD_C_SUCCESS` on success, or error code on OOM/I/O failure.
 */
static cdd_c_error_t write_docs_json_output(const char *output_target,
                                            const char *serialized) {
  int is_dir = 0;

  if (!output_target) {
    printf("%s\n", serialized);
    return CDD_C_SUCCESS;
  }
  return write_docs_json_target_resolved(
      output_target, serialized, &is_dir,
      fs_is_directory(output_target, &is_dir));
}

/**
 * @brief Serializes `root_val` as pretty-printed JSON and writes it to
 * `output_target` (or `stdout`).
 *
 * @param[in] root_val Constructed documentation root `JSON_Value`.
 * @param[in] output_target Optional output file/directory path.
 * @return `CDD_C_SUCCESS` on success, or error code on OOM/I/O failure.
 */
static cdd_c_error_t serialize_and_write_docs_json(JSON_Value *root_val,
                                                   const char *output_target) {
  char *serialized = NULL;
  cdd_c_error_t rc = cdd_json_serialize_to_string_pretty(root_val, &serialized);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_docs_json_output(output_target, serialized);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_free_serialized_string(serialized));
  }
  return cdd_json_free_serialized_string(serialized);
}

/*
 * @brief Generate documentation JSON containing C code snippets for an OpenAPI
 * or Arazzo specification.
 *
 * @param[in] config Configuration parameters for documentation JSON generation.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
cdd_c_error_t cdd_generate_docs_json(const cdd_docs_json_config_t *config) {
  const char *input_file;
  int no_imports;
  int no_wrapping;
  struct OpenAPI_Spec spec = {0};
  struct Arazzo_Spec az_spec;
  cdd_c_error_t rc;
  cdd_c_error_t az_rc;
  JSON_Value *parsed_root = NULL;
  JSON_Value *root_val = NULL;

  if (!config || !config->input)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  input_file = config->input;
  no_imports = (int)config->no_imports;
  no_wrapping = (int)config->no_wrapping;

  rc = cdd_json_parse_file(input_file, &parsed_root);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(CDD_C_ERROR_PARSE, rc);
  }

  memset(&az_spec, 0, sizeof(az_spec));
  az_rc = arazzo_load_from_json_value(parsed_root, &az_spec);
  if (az_rc == CDD_C_SUCCESS) {
    rc = build_arazzo_docs_root(&az_spec, no_imports, no_wrapping, &root_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(
          cdd_c_keep_err(rc, cdd_json_value_free(parsed_root)),
          arazzo_spec_free(&az_spec));
    }
    rc = serialize_and_write_docs_json(root_val, config->output);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(
          cdd_c_keep_err(cdd_c_keep_err(rc, cdd_json_value_free(parsed_root)),
                         cdd_json_value_free(root_val)),
          arazzo_spec_free(&az_spec));
    }
    return cdd_c_keep_err(
        cdd_c_keep_err(cdd_c_keep_err(az_rc, cdd_json_value_free(parsed_root)),
                       cdd_json_value_free(root_val)),
        arazzo_spec_free(&az_spec));
  }

  rc = openapi_load_from_json(parsed_root, &spec);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(cdd_c_keep_err(rc, cdd_json_value_free(parsed_root)),
                          arazzo_spec_free(&az_spec));
  }
  rc = build_openapi_docs_root(&spec, no_imports, no_wrapping, &root_val);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(
        cdd_c_keep_err(cdd_c_keep_err(rc, cdd_json_value_free(parsed_root)),
                       openapi_spec_free(&spec)),
        arazzo_spec_free(&az_spec));
  }
  rc = serialize_and_write_docs_json(root_val, config->output);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(
        cdd_c_keep_err(
            cdd_c_keep_err(cdd_c_keep_err(rc, cdd_json_value_free(parsed_root)),
                           cdd_json_value_free(root_val)),
            openapi_spec_free(&spec)),
        arazzo_spec_free(&az_spec));
  }
  return cdd_c_keep_err(
      cdd_c_keep_err(cdd_json_value_free(parsed_root),
                     cdd_json_value_free(root_val)),
      cdd_c_keep_err(openapi_spec_free(&spec), arazzo_spec_free(&az_spec)));
}

/*
 * @brief Start the JSON-RPC server exposing the CDD functionality.
 *
 * @param[in] config Configuration parameters for the JSON-RPC server.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
cdd_c_error_t cdd_serve_json_rpc(const cdd_serve_json_rpc_config_t *config) {
  return serve_json_rpc_run(config);
}

/** @brief Function pointer type for FFI target language emitters */
typedef enum cdd_c_error (*ffi_emitter_fn_t)(
    cdd_ffi_ir_t *, const cdd_generate_bindings_config_t *);

/**
 * @brief Mapping entry from target language name/alias to its FFI emitter.
 */
struct FfiEmitterEntry {
  const char *name;      /**< Primary target language identifier */
  const char *alias;     /**< Optional secondary alias identifier */
  ffi_emitter_fn_t emit; /**< Emitter callback function */
};

/** @brief Static registry of supported FFI target language emitters */
static const struct FfiEmitterEntry EMITTERS[] = {
    {"python", NULL, cdd_ffi_emit_python},
    {"rust", NULL, cdd_ffi_emit_rust},
    {"csharp", NULL, cdd_ffi_emit_csharp},
    {"typescript", NULL, cdd_ffi_emit_typescript},
    {"napi", NULL, cdd_ffi_emit_napi},
    {"java", NULL, cdd_ffi_emit_java},
    {"cpp", NULL, cdd_ffi_emit_cpp},
    {"go", NULL, cdd_ffi_emit_go},
    {"swift", NULL, cdd_ffi_emit_swift},
    {"dart", NULL, cdd_ffi_emit_dart},
    {"ruby", NULL, cdd_ffi_emit_ruby},
    {"kotlin", NULL, cdd_ffi_emit_kotlin},
    {"php", NULL, cdd_ffi_emit_php},
    {"lua", NULL, cdd_ffi_emit_lua},
    {"zig", NULL, cdd_ffi_emit_zig},
    {"odin", NULL, cdd_ffi_emit_odin},
    {"julia", NULL, cdd_ffi_emit_julia},
    {"r", NULL, cdd_ffi_emit_r},
    {"matlab", NULL, cdd_ffi_emit_matlab},
    {"haskell", NULL, cdd_ffi_emit_haskell},
    {"ocaml", NULL, cdd_ffi_emit_ocaml},
    {"elixir", NULL, cdd_ffi_emit_elixir},
    {"erlang", NULL, cdd_ffi_emit_erlang},
    {"common_lisp", NULL, cdd_ffi_emit_common_lisp},
    {"racket", NULL, cdd_ffi_emit_racket},
    {"scheme", NULL, cdd_ffi_emit_scheme},
    {"scala", NULL, cdd_ffi_emit_scala},
    {"fsharp", NULL, cdd_ffi_emit_fsharp},
    {"clojure", NULL, cdd_ffi_emit_clojure},
    {"groovy", NULL, cdd_ffi_emit_groovy},
    {"webassembly", "wasm", cdd_ffi_emit_webassembly},
    {"nim", NULL, cdd_ffi_emit_nim},
    {"vlang", NULL, cdd_ffi_emit_vlang},
    {"dlang", "d", cdd_ffi_emit_d},
    {"perl", NULL, cdd_ffi_emit_perl},
    {"tcl", NULL, cdd_ffi_emit_tcl},
    {"fortran", NULL, cdd_ffi_emit_fortran},
    {"cobol", "cbl", cdd_ffi_emit_cobol},
    {"delphi", "pascal", cdd_ffi_emit_delphi},
    {"ada", NULL, cdd_ffi_emit_ada},
    {"objc", "objective-c", cdd_ffi_emit_objc},
    {"crystal", NULL, cdd_ffi_emit_crystal},
    {"ir", "json", cdd_ffi_emit_ir_json}};

/**
 * @brief Topologically sorts `ir` and computes target ABI struct/union layouts.
 *
 * @param[in,out] ir FFI IR to prepare for code emission.
 * @return CDD_C_SUCCESS on success, or error code on OOM/invalid IR.
 */
static NO_DISCARD cdd_c_error_t prepare_ffi_ir_for_emission(cdd_ffi_ir_t *ir) {
  cdd_ffi_abi_profile_t relaxed_abi;
  cdd_c_error_t rc;

  rc = cdd_ffi_ir_topological_sort(ir);
  if (rc != CDD_C_SUCCESS)
    return rc;

  memset(&relaxed_abi, 0, sizeof(relaxed_abi));
  relaxed_abi.kind = CDD_FFI_ABI_HOST;
  relaxed_abi.allow_unresolved_refs = 1;
  return cdd_ffi_ir_compute_layout(ir, &relaxed_abi);
}

/**
 * @brief Frees a heap-allocated `cdd_ffi_ir_t` and its contents while
 * propagating `err`.
 *
 * @param[in,out] ir Heap-allocated FFI IR pointer to free.
 * @param[in] err Primary status code to preserve.
 * @return `err` if non-zero, otherwise `cdd_ffi_ir_free` status.
 */
static NO_DISCARD cdd_c_error_t free_boxed_ffi_ir_err(cdd_ffi_ir_t *ir,
                                                      cdd_c_error_t err) {
  cdd_ffi_ir_t ir_copy = *ir;
  C_CDD_FREE(ir);
  return cdd_c_keep_err(err, cdd_ffi_ir_free(&ir_copy));
}

/**
 * @brief Parses an in-memory `.json`, `.dsl`, or C/C++ header buffer into
 * `*out_ir`.
 *
 * @param[in] config Binding generation configuration.
 * @param[in] file_content Null-terminated source buffer.
 * @param[out] out_ir Output pointer receiving the allocated `cdd_ffi_ir_t`.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static NO_DISCARD cdd_c_error_t
load_ffi_ir_from_buffer(const cdd_generate_bindings_config_t *config,
                        const char *file_content, cdd_ffi_ir_t **out_ir) {
  size_t len = strlen(config->input);
  if (len > 5 && strcmp(config->input + len - 5, ".json") == 0) {
    return cdd_ffi_ir_from_json(file_content, out_ir);
  }
  if (len > 4 && strcmp(config->input + len - 4, ".dsl") == 0) {
    cdd_dsl_token_list_t tokens;
    cdd_c_error_t rc = cdd_dsl_tokenize(file_content, &tokens);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_dsl_parse(&tokens, out_ir);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_dsl_token_list_free(&tokens));
    }
    return cdd_dsl_token_list_free(&tokens);
  }
  return cdd_ffi_ir_extract_exports(config->input, file_content, config,
                                    out_ir);
}

/**
 * @brief Dispatches comma-separated `config->target_langs` tokens across
 * `EMITTERS[]` and `"dsl"`, rejecting unknown tokens with
 * `CDD_C_ERROR_NOT_FOUND`.
 *
 * @param[in,out] ir Prepared FFI IR structure.
 * @param[in] config Binding generation configuration (with non-empty
 * `target_langs`).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on empty
 * token, CDD_C_ERROR_NOT_FOUND on unknown language, or emitter error code.
 */
static NO_DISCARD cdd_c_error_t dispatch_ffi_emitters(
    cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  const char *cur = config->target_langs;
  cdd_c_error_t rc;

  for (;;) {
    const char *comma;
    const char *tok_s;
    const char *tok_e;
    size_t tok_len;
    char tok_buf[64];

    comma = strchr(cur, ',');
    tok_s = cur;
    tok_e = comma ? comma : (cur + strlen(cur));
    while (tok_s < tok_e && (*tok_s == ' ' || *tok_s == '\t'))
      tok_s++;
    while (tok_e > tok_s && (tok_e[-1] == ' ' || tok_e[-1] == '\t'))
      tok_e--;
    tok_len = (size_t)(tok_e - tok_s);
    if (tok_len == 0 || tok_len >= sizeof(tok_buf))
      return CDD_C_ERROR_INVALID_ARGUMENT;

    memcpy(tok_buf, tok_s, tok_len);
    tok_buf[tok_len] = '\0';

    if (strcmp(tok_buf, "all") == 0 || strcmp(tok_buf, "*") == 0) {
      size_t i;
      for (i = 0; i < sizeof(EMITTERS) / sizeof(EMITTERS[0]); ++i) {
        rc = EMITTERS[i].emit(ir, config);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (strcmp(tok_buf, "dsl") == 0) {
      rc = cdd_ffi_emit_dsl(ir, config);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      size_t i;
      int matched = 0;
      for (i = 0; i < sizeof(EMITTERS) / sizeof(EMITTERS[0]); ++i) {
        if (strcmp(tok_buf, EMITTERS[i].name) == 0 ||
            (EMITTERS[i].alias && strcmp(tok_buf, EMITTERS[i].alias) == 0)) {
          matched = 1;
          rc = EMITTERS[i].emit(ir, config);
          if (rc != CDD_C_SUCCESS)
            return rc;
          break;
        }
      }
      if (!matched)
        return CDD_C_ERROR_NOT_FOUND;
    }

    if (!comma)
      break;
    cur = comma + 1;
    if (*cur == '\0')
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Generate FFI bindings directly from an IR DSL string.
 *
 * @param[in] source Input DSL source text.
 * @param[in] config Binding generation configuration parameters.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_generate_bindings_from_dsl(
    const char *source, const cdd_generate_bindings_config_t *config) {
  cdd_dsl_token_list_t tokens;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;

  if (!source || !config)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_dsl_tokenize(source, &tokens);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = cdd_dsl_parse(&tokens, &ir);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_dsl_token_list_free(&tokens));
  }

  rc = prepare_ffi_ir_for_emission(ir);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(free_boxed_ffi_ir_err(ir, rc),
                          cdd_dsl_token_list_free(&tokens));
  }
  if (config->target_langs && config->target_langs[0] != '\0') {
    rc = dispatch_ffi_emitters(ir, config);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(free_boxed_ffi_ir_err(ir, rc),
                            cdd_dsl_token_list_free(&tokens));
    }
  }
  return cdd_c_keep_err(free_boxed_ffi_ir_err(ir, CDD_C_SUCCESS),
                        cdd_dsl_token_list_free(&tokens));
}

/*
 * @brief Generate SWIG-like FFI bindings for multiple target languages.
 *
 * @param[in] config Binding generation configuration parameters.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_generate_bindings(const cdd_generate_bindings_config_t *config) {
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;
  char *file_content = NULL;
  size_t file_size = 0;

  if (!config || !config->input || !config->output_dir ||
      !config->target_langs || config->target_langs[0] == '\0') {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = read_to_file(config->input, "rb", &file_content, &file_size);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  rc = load_ffi_ir_from_buffer(config, file_content, &ir);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(file_content);
    return rc;
  }
  C_CDD_FREE(file_content);

  rc = prepare_ffi_ir_for_emission(ir);
  if (rc != CDD_C_SUCCESS) {
    return free_boxed_ffi_ir_err(ir, rc);
  }
  return free_boxed_ffi_ir_err(ir, dispatch_ffi_emitters(ir, config));
}

/**
 * @brief Optionally loads an auxiliary OpenAPI specification when readable and
 * valid.
 *
 * @param[in] openapi_input Path to the optional OpenAPI JSON file.
 * @param[out] openapi_spec Target `OpenAPI_Spec` structure.
 * @param[out] out_has_openapi Set to 1 if loaded successfully, 0 otherwise.
 * @return `CDD_C_SUCCESS`.
 */
static cdd_c_error_t
try_load_optional_openapi_spec(const char *openapi_input,
                               struct OpenAPI_Spec *openapi_spec,
                               int *out_has_openapi) {
  JSON_Value *oa_root = NULL;
  cdd_c_error_t rc = cdd_json_parse_file(openapi_input, &oa_root);
  if (rc == CDD_C_SUCCESS) {
    cdd_c_error_t load_rc = openapi_load_from_json(oa_root, openapi_spec);
    if (load_rc == CDD_C_SUCCESS) {
      *out_has_openapi = 1;
      return cdd_c_keep_err(load_rc, cdd_json_value_free(oa_root));
    }
    return cdd_c_keep_err(rc, cdd_json_value_free(oa_root));
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Dispatches the selected Arazzo code generator (`to_sdk_cli`,
 * `to_server`, or `to_sdk`).
 *
 * @param[in] subcommand Requested Arazzo generation subcommand.
 * @param[in] arazzo_spec Loaded Arazzo specification.
 * @param[in] openapi_spec Optional companion OpenAPI specification (or `NULL`).
 * @param[in] client_cfg Output path and function-prefix configuration.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t
dispatch_arazzo_generator(cdd_arazzo_subcommand_t subcommand,
                          const struct Arazzo_Spec *arazzo_spec,
                          const struct OpenAPI_Spec *openapi_spec,
                          const struct OpenApiClientConfig *client_cfg) {
  if (subcommand == CDD_ARAZZO_SUBCOMMAND_TO_SDK_CLI) {
    return arazzo_cli_generate(arazzo_spec, openapi_spec, client_cfg);
  }
  if (subcommand == CDD_ARAZZO_SUBCOMMAND_TO_SERVER) {
    return arazzo_server_generate(arazzo_spec, openapi_spec, client_cfg);
  }
  if (subcommand == CDD_ARAZZO_SUBCOMMAND_TO_SDK) {
    return arazzo_client_generate(arazzo_spec, openapi_spec, client_cfg);
  }
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Runs the requested Arazzo generator and frees loaded specs.
 *
 * @param[in] config Arazzo generation configuration.
 * @param[in,out] arazzo_spec Loaded Arazzo specification.
 * @param[in,out] openapi_spec Auxiliary OpenAPI specification buffer.
 * @param[in] has_openapi_ptr Pointer to `has_openapi` flag populated by caller.
 * @param[in,out] root Parsed Arazzo JSON root value.
 * @param[in] opt_rc Status from optional OpenAPI loader.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t finish_generate_from_arazzo(
    const cdd_from_arazzo_config_t *config, struct Arazzo_Spec *arazzo_spec,
    struct OpenAPI_Spec *openapi_spec, const int *has_openapi_ptr,
    JSON_Value *root, cdd_c_error_t opt_rc) {
  struct OpenApiClientConfig client_cfg;
  memset(&client_cfg, 0, sizeof(client_cfg));
  client_cfg.filename_base =
      config->output ? config->output : "generated_arazzo";
  client_cfg.func_prefix = config->func_prefix ? config->func_prefix : "api_";

  if (*has_openapi_ptr) {
    cdd_c_error_t gen_rc = dispatch_arazzo_generator(
        config->subcommand, arazzo_spec, openapi_spec, &client_cfg);
    if (gen_rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(
          cdd_c_keep_err(cdd_c_keep_err(cdd_c_keep_err(gen_rc, opt_rc),
                                        cdd_json_value_free(root)),
                         openapi_spec_free(openapi_spec)),
          arazzo_spec_free(arazzo_spec));
    }
    return cdd_c_keep_err(
        cdd_c_keep_err(cdd_c_keep_err(opt_rc, cdd_json_value_free(root)),
                       openapi_spec_free(openapi_spec)),
        arazzo_spec_free(arazzo_spec));
  }
  {
    cdd_c_error_t gen_rc = dispatch_arazzo_generator(
        config->subcommand, arazzo_spec, NULL, &client_cfg);
    if (gen_rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(cdd_c_keep_err(cdd_c_keep_err(gen_rc, opt_rc),
                                           cdd_json_value_free(root)),
                            arazzo_spec_free(arazzo_spec));
    }
    return cdd_c_keep_err(cdd_c_keep_err(opt_rc, cdd_json_value_free(root)),
                          arazzo_spec_free(arazzo_spec));
  }
}

/*
 * @brief Generate C SDK workflows, CLI workflow runner, or Backend Server
 * orchestrator from an Arazzo v1.1.0 specification.
 *
 * @param[in] config Configuration parameters for Arazzo-to-C generation.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t cdd_generate_from_arazzo(const cdd_from_arazzo_config_t *config) {
  struct Arazzo_Spec arazzo_spec;
  struct OpenAPI_Spec openapi_spec;
  int has_openapi = 0;
  JSON_Value *root = NULL;
  cdd_c_error_t rc;

  if (!config || !config->input) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  memset(&arazzo_spec, 0, sizeof(arazzo_spec));
  memset(&openapi_spec, 0, sizeof(openapi_spec));

  rc = cdd_json_parse_file(config->input, &root);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(CDD_C_ERROR_IO, rc);
  }
  rc = arazzo_load_from_json_value(root, &arazzo_spec);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root));
  }

  if (config->openapi_input) {
    return finish_generate_from_arazzo(
        config, &arazzo_spec, &openapi_spec, &has_openapi, root,
        try_load_optional_openapi_spec(config->openapi_input, &openapi_spec,
                                       &has_openapi));
  }
  return finish_generate_from_arazzo(config, &arazzo_spec, &openapi_spec,
                                     &has_openapi, root, CDD_C_SUCCESS);
}

/*
 * @brief Extract an Arazzo v1.1.0 specification from annotated C source/header
 * files.
 *
 * @param[in] config Configuration parameters for C-to-Arazzo extraction.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t cdd_generate_to_arazzo(const cdd_to_arazzo_config_t *config) {
  struct Arazzo_Spec spec;
  char *json = NULL;
  const char *out_path;
  cdd_c_error_t rc;

  if (!config || !config->input) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  out_path = config->output ? config->output : "arazzo.json";
  memset(&spec, 0, sizeof(spec));
  rc = arazzo_spec_init(&spec);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  rc = c2arazzo_parse_dir(config->input, &spec);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, arazzo_spec_free(&spec));
  }
  if (config->self_uri && *config->self_uri) {
    C_CDD_FREE(spec.self_uri);
    spec.self_uri = NULL;
    rc = c_cdd_strdup(config->self_uri, &spec.self_uri);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, arazzo_spec_free(&spec));
    }
  }
  rc = arazzo_write_spec_to_json(&spec, &json);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, arazzo_spec_free(&spec));
  }
  rc = fs_write_to_file(out_path, json);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(json);
    return cdd_c_keep_err(rc, arazzo_spec_free(&spec));
  }
  C_CDD_FREE(json);
  return arazzo_spec_free(&spec);
}

/** @brief Reflected fields for `cdd_from_openapi_config_t`. */
static const struct cdd_field_info g_cdd_from_openapi_config_fields[] = {
    {"input",
     "input",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_from_openapi_config_t, input),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "i",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"input_dir",
     "input-dir",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_from_openapi_config_t, input_dir),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"output",
     "output",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_from_openapi_config_t, output),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     "o",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"no_github_actions",
     "no-github-actions",
     "c_cdd_bool_t",
     NULL,
     CDD_FIELD_KIND_BOOLEAN,
     offsetof(cdd_from_openapi_config_t, no_github_actions),
     sizeof(c_cdd_bool_t),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"no_installable_package",
     "no-installable-package",
     "c_cdd_bool_t",
     NULL,
     CDD_FIELD_KIND_BOOLEAN,
     offsetof(cdd_from_openapi_config_t, no_installable_package),
     sizeof(c_cdd_bool_t),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"tests",
     "tests",
     "c_cdd_bool_t",
     NULL,
     CDD_FIELD_KIND_BOOLEAN,
     offsetof(cdd_from_openapi_config_t, tests),
     sizeof(c_cdd_bool_t),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info cdd_from_openapi_config_type_info = {
    "cdd_from_openapi_config_t",
    sizeof(cdd_from_openapi_config_t),
    0,
    g_cdd_from_openapi_config_fields,
    sizeof(g_cdd_from_openapi_config_fields) /
        sizeof(g_cdd_from_openapi_config_fields[0]),
    0};

/** @brief Deserialization aliases for `dialect_uri`. */
static const char *const g_cdd_dialect_aliases[] = {"jsonSchemaDialect"};

/** @brief Reflected fields for `cdd_to_openapi_config_t`. */
static const struct cdd_field_info g_cdd_to_openapi_config_fields[] = {
    {"input",
     "input",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_to_openapi_config_t, input),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "i",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"output",
     "output",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_to_openapi_config_t, output),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "o",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"base_file",
     "base",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_to_openapi_config_t, base_file),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "b",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"self_uri",
     "self",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_to_openapi_config_t, self_uri),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "s",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"dialect_uri",
     "dialect",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_to_openapi_config_t, dialect_uri),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     g_cdd_dialect_aliases,
     1,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info cdd_to_openapi_config_type_info = {
    "cdd_to_openapi_config_t",
    sizeof(cdd_to_openapi_config_t),
    0,
    g_cdd_to_openapi_config_fields,
    sizeof(g_cdd_to_openapi_config_fields) /
        sizeof(g_cdd_to_openapi_config_fields[0]),
    0};

/** @brief Reflected fields for `cdd_docs_json_config_t`. */
static const struct cdd_field_info g_cdd_docs_json_config_fields[] = {
    {"input",
     "input",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_docs_json_config_t, input),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "i",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"output",
     "output",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_docs_json_config_t, output),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     "o",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"no_imports",
     "no-imports",
     "c_cdd_bool_t",
     NULL,
     CDD_FIELD_KIND_BOOLEAN,
     offsetof(cdd_docs_json_config_t, no_imports),
     sizeof(c_cdd_bool_t),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"no_wrapping",
     "no-wrapping",
     "c_cdd_bool_t",
     NULL,
     CDD_FIELD_KIND_BOOLEAN,
     offsetof(cdd_docs_json_config_t, no_wrapping),
     sizeof(c_cdd_bool_t),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info cdd_docs_json_config_type_info = {
    "cdd_docs_json_config_t",
    sizeof(cdd_docs_json_config_t),
    0,
    g_cdd_docs_json_config_fields,
    sizeof(g_cdd_docs_json_config_fields) /
        sizeof(g_cdd_docs_json_config_fields[0]),
    0};

/** @brief Reflected fields for `cdd_serve_json_rpc_config_t`. */
static const struct cdd_field_info g_cdd_serve_json_rpc_config_fields[] = {
    {"port",
     "port",
     "int",
     NULL,
     CDD_FIELD_KIND_INTEGER,
     offsetof(cdd_serve_json_rpc_config_t, port),
     sizeof(int),
     0,
     0,
     1,
     0,
     "p",
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"bind_address",
     "listen",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_serve_json_rpc_config_t, bind_address),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     "l",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"max_requests",
     "max-requests",
     "size_t",
     NULL,
     CDD_FIELD_KIND_INTEGER,
     offsetof(cdd_serve_json_rpc_config_t, max_requests),
     sizeof(size_t),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info cdd_serve_json_rpc_config_type_info = {
    "cdd_serve_json_rpc_config_t",
    sizeof(cdd_serve_json_rpc_config_t),
    0,
    g_cdd_serve_json_rpc_config_fields,
    sizeof(g_cdd_serve_json_rpc_config_fields) /
        sizeof(g_cdd_serve_json_rpc_config_fields[0]),
    0};

/** @brief Reflected fields for `cdd_generate_bindings_config_t`. */
static const struct cdd_field_info g_cdd_generate_bindings_config_fields[] = {
    {"input",
     "input",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_generate_bindings_config_t, input),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "i",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"output_dir",
     "output-dir",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_generate_bindings_config_t, output_dir),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "o",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"target_langs",
     "lang",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_generate_bindings_config_t, target_langs),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "l",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"library_name",
     "lib-name",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_generate_bindings_config_t, library_name),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "n",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"module_name",
     "module-name",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_generate_bindings_config_t, module_name),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     "m",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"skip_static",
     "skip-static",
     "c_cdd_bool_t",
     NULL,
     CDD_FIELD_KIND_BOOLEAN,
     offsetof(cdd_generate_bindings_config_t, skip_static),
     sizeof(c_cdd_bool_t),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"opaque_pointers",
     "opaque-pointers",
     "c_cdd_bool_t",
     NULL,
     CDD_FIELD_KIND_BOOLEAN,
     offsetof(cdd_generate_bindings_config_t, opaque_pointers),
     sizeof(c_cdd_bool_t),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"generate_tests",
     "generate-tests",
     "c_cdd_bool_t",
     NULL,
     CDD_FIELD_KIND_BOOLEAN,
     offsetof(cdd_generate_bindings_config_t, generate_tests),
     sizeof(c_cdd_bool_t),
     0,
     0,
     0,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"recursive_includes",
     "recursive",
     "c_cdd_bool_t",
     NULL,
     CDD_FIELD_KIND_BOOLEAN,
     offsetof(cdd_generate_bindings_config_t, recursive_includes),
     sizeof(c_cdd_bool_t),
     0,
     0,
     0,
     0,
     "r",
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info cdd_generate_bindings_config_type_info = {
    "cdd_generate_bindings_config_t",
    sizeof(cdd_generate_bindings_config_t),
    0,
    g_cdd_generate_bindings_config_fields,
    sizeof(g_cdd_generate_bindings_config_fields) /
        sizeof(g_cdd_generate_bindings_config_fields[0]),
    0};

/** @brief Reflected fields for `cdd_from_arazzo_config_t`. */
static const struct cdd_field_info g_cdd_from_arazzo_config_fields[] = {
    {"input",
     "input",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_from_arazzo_config_t, input),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "i",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"openapi_input",
     "openapi",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_from_arazzo_config_t, openapi_input),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"output",
     "output",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_from_arazzo_config_t, output),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "o",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"func_prefix",
     "prefix",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_from_arazzo_config_t, func_prefix),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info cdd_from_arazzo_config_type_info = {
    "cdd_from_arazzo_config_t",
    sizeof(cdd_from_arazzo_config_t),
    0,
    g_cdd_from_arazzo_config_fields,
    sizeof(g_cdd_from_arazzo_config_fields) /
        sizeof(g_cdd_from_arazzo_config_fields[0]),
    0};

/** @brief Reflected fields for `cdd_to_arazzo_config_t`. */
static const struct cdd_field_info g_cdd_to_arazzo_config_fields[] = {
    {"input",
     "input",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_to_arazzo_config_t, input),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "i",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"output",
     "output",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_to_arazzo_config_t, output),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     "o",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"self_uri",
     "self",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(cdd_to_arazzo_config_t, self_uri),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     "s",
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info cdd_to_arazzo_config_type_info = {
    "cdd_to_arazzo_config_t",
    sizeof(cdd_to_arazzo_config_t),
    0,
    g_cdd_to_arazzo_config_fields,
    sizeof(g_cdd_to_arazzo_config_fields) /
        sizeof(g_cdd_to_arazzo_config_fields[0]),
    0};
