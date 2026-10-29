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
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
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
    root = json_parse_file(config->input);
    if (!root) {
      fprintf(stderr, "Failed to parse JSON file: %s\n", config->input);
      return CDD_C_ERROR_PARSE;
    }

    rc = openapi_load_from_json(root, &spec);
    json_value_free(root);

    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Failed to load openapi spec from %s\n", config->input);
      return rc;
    }

    if (config->output) {
      rc = fs_path_join(config->output, "generated_client", &alloc_path);
      if (rc != CDD_C_SUCCESS) {
        openapi_spec_free(&spec);
        return rc;
      }
      client_cfg.filename_base = alloc_path;
    } else {
      client_cfg.filename_base = "test_build_dir/generated_client";
    }
    client_cfg.func_prefix = "api_";

    if (is_server) {
      rc = openapi_server_generate(&spec, &client_cfg);
    } else {
      rc = openapi_client_generate(&spec, &client_cfg);
      if (rc == CDD_C_SUCCESS) {
        rc = openapi_client_gui_generate(&spec, &client_cfg);
      }
      if (rc == CDD_C_SUCCESS && is_cli) {
        rc = openapi_cli_generate(&spec, &client_cfg);
      }
    }

    if (alloc_path) {
      C_CDD_FREE(alloc_path);
    }
    openapi_spec_free(&spec);
  }

  return rc;
}

/**
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
  cdd_c_error_t rc;
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
    rc = fs_path_join(input_dir, "openapi.snapshot.json", &snapshot_path);
    if (rc == CDD_C_SUCCESS) {
#if defined(_MSC_VER)
      if (fopen_s(&f, snapshot_path, "r") != 0)
        f = NULL;
#else
      f = fopen(snapshot_path, "r");
#endif
      if (f) {
        fclose(f);
        run_cfg.base_file = snapshot_path;
        rc = c2openapi_generate(&run_cfg);
        C_CDD_FREE(snapshot_path);
        return rc;
      }
      C_CDD_FREE(snapshot_path);
    }
  }

  return c2openapi_generate(&run_cfg);
}

/**
 * @brief Build a structured documentation code JSON object.
 *
 * @param[in] snippet Core C SDK invocation snippet string.
 * @param[in] no_imports Non-zero to omit the imports field.
 * @param[in] no_wrapping Non-zero to omit wrapper_start and wrapper_end fields.
 * @param[out] out_val Pointer to receive the constructed JSON_Value object.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t cdd_docs_build_code_val(const char *snippet,
                                             int no_imports, int no_wrapping,
                                             JSON_Value **out_val) {
  JSON_Value *code_val = json_value_init_object();
  JSON_Object *code_obj = json_value_get_object(code_val);

  if (!no_imports) {
    json_object_set_string(
        code_obj, "imports",
        "#include \"generated_client.h\"\n#include <stdio.h>");
  }
  if (!no_wrapping) {
    json_object_set_string(
        code_obj, "wrapper_start",
        "int main(void) {\n  struct HttpClient client;\n  struct ApiError *err "
        "= NULL;\n  api_init(&client, \"https://api.example.com\");");
  }
  json_object_set_string(code_obj, "snippet", snippet);
  if (!no_wrapping) {
    json_object_set_string(
        code_obj, "wrapper_end",
        "  api_cleanup(&client);\n  return CDD_C_SUCCESS;\n}");
  }
  *out_val = code_val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Format the C SDK invocation snippet for an OpenAPI operation.
 *
 * @param[in] op OpenAPI operation structure.
 * @param[out] snippet Output buffer for the formatted C code snippet.
 * @param[in] snippet_sz Size of the snippet buffer in bytes.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t
cdd_docs_format_op_snippet(const struct OpenAPI_Operation *op, char *snippet,
                           size_t snippet_sz) {
  const char *op_id = op->operation_id ? op->operation_id : "unknown";
  char decls[2048];
  char call_args[1024];
  size_t k;

  decls[0] = '\0';
  call_args[0] = '\0';

  for (k = 0; k < op->n_parameters; k++) {
    const struct OpenAPI_Parameter *param = &op->parameters[k];
    const char *pname = param->name;
    const char *ptype = param->type;
    char decl_line[256];
    char arg_item[128];

    if (param->is_array) {
      CDD_SNPRINTF(decl_line, sizeof(decl_line),
                   "  const char *%s[] = {\"example\"};\n", pname);
      CDD_SNPRINTF(arg_item, sizeof(arg_item), ", %s, 1", pname);
    } else if (strcmp(ptype, "integer") == 0) {
      CDD_SNPRINTF(decl_line, sizeof(decl_line), "  int %s = 1;\n", pname);
      CDD_SNPRINTF(arg_item, sizeof(arg_item), ", %s", pname);
    } else if (strcmp(ptype, "number") == 0) {
      CDD_SNPRINTF(decl_line, sizeof(decl_line), "  double %s = 1.0;\n", pname);
      CDD_SNPRINTF(arg_item, sizeof(arg_item), ", %s", pname);
    } else {
      CDD_SNPRINTF(decl_line, sizeof(decl_line),
                   "  const char *%s = \"example\";\n", pname);
      CDD_SNPRINTF(arg_item, sizeof(arg_item), ", %s", pname);
    }
    CDD_STRCAT(decls, sizeof(decls), decl_line);
    CDD_STRCAT(call_args, sizeof(call_args), arg_item);
  }

  if (op->req_body.ref_name) {
    char decl_line[256];
    CDD_SNPRINTF(decl_line, sizeof(decl_line), "  struct %s req_body = {0};\n",
                 op->req_body.ref_name);
    CDD_STRCAT(decls, sizeof(decls), decl_line);
    CDD_STRCAT(call_args, sizeof(call_args), ", &req_body");
  } else if (op->req_body.content_type) {
    CDD_STRCAT(decls, sizeof(decls), "  const char *req_body = \"{}\";\n");
    CDD_STRCAT(call_args, sizeof(call_args), ", req_body");
  }

  CDD_SNPRINTF(snippet, snippet_sz,
               "%s  /* Call the %s API */\n  cdd_c_error_t rc = "
               "api_%s(&client%s, &err);\n  if (rc != CDD_C_SUCCESS) {\n    "
               "fprintf(stderr, \"Error: %%d\\n\", rc);\n  }",
               decls, op_id, op_id, call_args);
  return CDD_C_SUCCESS;
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
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t
cdd_docs_populate_path_map(JSON_Object *target_obj,
                           const struct OpenAPI_Path *paths, size_t n_paths,
                           int no_imports, int no_wrapping) {
  size_t p, op_idx;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (p = 0; p < n_paths; p++) {
    const struct OpenAPI_Path *pi = &paths[p];
    JSON_Value *path_val = json_value_init_object();
    JSON_Object *path_obj = json_value_get_object(path_val);
    json_object_set_value(target_obj, pi->route, path_val);

    for (op_idx = 0; op_idx < pi->n_operations; op_idx++) {
      const struct OpenAPI_Operation *op = &pi->operations[op_idx];
      const char *method = "";
      char snippet[4096];
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

      rc = cdd_docs_format_op_snippet(op, snippet, sizeof(snippet));
      rc = cdd_docs_build_code_val(snippet, no_imports, no_wrapping, &code_val);
      json_object_set_value(path_obj, method, code_val);
    }
  }
  return rc;
}

/**
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
  cdd_c_error_t rc = CDD_C_SUCCESS;
  JSON_Value *parsed_root;
  JSON_Value *root_val;
  JSON_Object *root_obj;
  JSON_Value *endpoints_val;
  JSON_Object *endpoints_obj;

  if (!config || !config->input)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  input_file = config->input;
  no_imports = (int)config->no_imports;
  no_wrapping = (int)config->no_wrapping;

  parsed_root = json_parse_file(input_file);
  if (!parsed_root)
    return CDD_C_ERROR_PARSE;

  memset(&az_spec, 0, sizeof(az_spec));
  if (arazzo_load_from_json_value(parsed_root, &az_spec) == CDD_C_SUCCESS) {
    size_t w;
    JSON_Value *workflows_val = json_value_init_object();
    JSON_Object *workflows_obj = json_value_get_object(workflows_val);

    json_value_free(parsed_root);
    root_val = json_value_init_object();
    root_obj = json_value_get_object(root_val);
    endpoints_val = json_value_init_object();
    json_object_set_value(root_obj, "endpoints", endpoints_val);

    for (w = 0; w < az_spec.n_workflows; w++) {
      const char *wf_id = az_spec.workflows[w].workflow_id;
      char snippet[1024];
      JSON_Value *code_val = NULL;

      CDD_SNPRINTF(
          snippet, sizeof(snippet),
          "  struct arazzo_%s_inputs inputs = {0};\n"
          "  struct arazzo_%s_outputs outputs = {0};\n"
          "  cdd_c_error_t rc = arazzo_run_workflow_%s(&clients, &inputs, "
          "&outputs);\n"
          "  if (rc != CDD_C_SUCCESS) {\n"
          "    fprintf(stderr, \"Error: %%d\\n\", rc);\n"
          "  }\n"
          "  arazzo_%s_outputs_free(&outputs);",
          wf_id, wf_id, wf_id, wf_id);
      rc = cdd_docs_build_code_val(snippet, no_imports, no_wrapping, &code_val);
      json_object_set_value(workflows_obj, wf_id, code_val);
    }
    json_object_set_value(root_obj, "workflows", workflows_val);
    arazzo_spec_free(&az_spec);
  } else {
    arazzo_spec_free(&az_spec);
    rc = openapi_load_from_json(parsed_root, &spec);
    json_value_free(parsed_root);
    if (rc != CDD_C_SUCCESS)
      return rc;

    root_val = json_value_init_object();
    root_obj = json_value_get_object(root_val);
    endpoints_val = json_value_init_object();
    endpoints_obj = json_value_get_object(endpoints_val);

    rc = cdd_docs_populate_path_map(endpoints_obj, spec.paths, spec.n_paths,
                                    no_imports, no_wrapping);
    json_object_set_value(root_obj, "endpoints", endpoints_val);

    if (spec.n_webhooks > 0) {
      JSON_Value *webhooks_val = json_value_init_object();
      JSON_Object *webhooks_obj = json_value_get_object(webhooks_val);
      rc = cdd_docs_populate_path_map(webhooks_obj, spec.webhooks,
                                      spec.n_webhooks, no_imports, no_wrapping);
      json_object_set_value(root_obj, "webhooks", webhooks_val);
    }
    openapi_spec_free(&spec);
  }

  {
    char *serialized = json_serialize_to_string_pretty(root_val);
    if (config->output) {
      int is_dir = 0;
      rc = fs_is_directory(config->output, &is_dir);
      if (is_dir) {
        char *out_file = NULL;
        rc = fs_path_join(config->output, "docs.json", &out_file);
        if (rc == CDD_C_SUCCESS) {
          rc = fs_write_to_file(out_file, serialized);
          C_CDD_FREE(out_file);
        }
      } else {
        rc = fs_write_to_file(config->output, serialized);
      }
    } else {
      printf("%s\n", serialized);
    }
    json_free_serialized_string(serialized);
  }

  json_value_free(root_val);
  return rc;
}

/**
 * @brief Start the JSON-RPC server exposing the CDD functionality.
 *
 * @param[in] config Configuration parameters for the JSON-RPC server.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
cdd_c_error_t cdd_serve_json_rpc(const cdd_serve_json_rpc_config_t *config) {
  return serve_json_rpc_run(config);
}

/**
 * @brief Checks if a comma-separated list of target languages contains `lang`.
 *
 * @param[in] langs Comma-separated target language string.
 * @param[in] lang Language name or alias to search for.
 * @param[out] out_result Pointer to receive 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t cdd_target_langs_contains(const char *langs,
                                               const char *lang,
                                               int *out_result) {
  const char *p = langs;
  size_t len = strlen(lang);
  *out_result = 0;
  while ((p = strstr(p, lang)) != NULL) {
    if ((p == langs || p[-1] == ',') && (p[len] == '\0' || p[len] == ',')) {
      *out_result = 1;
      break;
    }
    p += len;
  }
  return CDD_C_SUCCESS;
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
    {"delphi", "pascal", cdd_ffi_emit_delphi},
    {"ada", NULL, cdd_ffi_emit_ada},
    {"objc", "objective-c", cdd_ffi_emit_objc},
    {"crystal", NULL, cdd_ffi_emit_crystal},
    {"ir", "json", cdd_ffi_emit_ir_json}};

/**
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
  cdd_dsl_token_list_free(&tokens);
  if (rc != CDD_C_SUCCESS)
    return rc;

  /* If target_langs includes "dsl", we can emit it back */
  if (config->target_langs && strstr(config->target_langs, "dsl")) {
    rc = cdd_ffi_emit_dsl(ir, config);
  }

  cdd_ffi_ir_free(ir);
  C_CDD_FREE(ir);
  return rc;
}

/**
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
      !config->target_langs) {
    return CDD_C_ERROR_INVALID_ARGUMENT; /* EINVAL */
  }

  /* Read file to string */
  rc = read_to_file(config->input, "rb", &file_content, &file_size);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  /* Parse IR from JSON or extract from C */
  {
    size_t len = strlen(config->input);
    if (len > 5 && strcmp(config->input + len - 5, ".json") == 0) {
      rc = cdd_ffi_ir_from_json(file_content, &ir);
    } else {
      rc = cdd_ffi_ir_extract_exports(config->input, file_content, config, &ir);
    }
  }
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(file_content);
    return rc;
  }

  /* Sort IR dependencies */
  rc = cdd_ffi_ir_topological_sort(ir);
  if (rc != CDD_C_SUCCESS) {
    cdd_ffi_ir_free(ir);
    C_CDD_FREE(ir);
    C_CDD_FREE(file_content);
    return rc;
  }

  /* Dispatch to Emitters */
  {
    size_t i;
    for (i = 0; i < sizeof(EMITTERS) / sizeof(EMITTERS[0]); ++i) {
      int should_emit = 0;
      if (strcmp(config->target_langs, "all") == 0 ||
          strcmp(config->target_langs, "*") == 0) {
        should_emit = 1;
      } else {
        int out_result = 0;
        rc = cdd_target_langs_contains(config->target_langs, EMITTERS[i].name,
                                       &out_result);
        if (out_result) {
          should_emit = 1;
        } else if (EMITTERS[i].alias) {
          rc = cdd_target_langs_contains(config->target_langs,
                                         EMITTERS[i].alias, &out_result);
          if (out_result)
            should_emit = 1;
        }
      }
      if (should_emit) {
        rc = EMITTERS[i].emit(ir, config);
        if (rc != CDD_C_SUCCESS) {
          printf("Failed at %s, rc = %d\n", EMITTERS[i].name, rc);
          cdd_ffi_ir_free(ir);
          C_CDD_FREE(ir);
          C_CDD_FREE(file_content);
          return rc;
        }
      }
    }
  }

  /* Cleanup */
  cdd_ffi_ir_free(ir);
  C_CDD_FREE(ir);
  C_CDD_FREE(file_content);

  return CDD_C_SUCCESS;
}

/**
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
  struct OpenApiClientConfig client_cfg;
  JSON_Value *root = NULL;
  cdd_c_error_t rc;

  if (!config || !config->input) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  memset(&arazzo_spec, 0, sizeof(arazzo_spec));
  memset(&openapi_spec, 0, sizeof(openapi_spec));
  memset(&client_cfg, 0, sizeof(client_cfg));

  root = json_parse_file(config->input);
  if (!root) {
    return CDD_C_ERROR_IO;
  }
  rc = arazzo_load_from_json_value(root, &arazzo_spec);
  json_value_free(root);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }

  if (config->openapi_input) {
    JSON_Value *oa_root = json_parse_file(config->openapi_input);
    if (oa_root) {
      if (openapi_load_from_json(oa_root, &openapi_spec) == CDD_C_SUCCESS) {
        has_openapi = 1;
      }
      json_value_free(oa_root);
    }
  }

  client_cfg.filename_base =
      config->output ? config->output : "generated_arazzo";
  client_cfg.func_prefix = config->func_prefix ? config->func_prefix : "api_";

  if (config->subcommand == CDD_ARAZZO_SUBCOMMAND_TO_SDK_CLI) {
    rc = arazzo_cli_generate(&arazzo_spec, has_openapi ? &openapi_spec : NULL,
                             &client_cfg);
  } else if (config->subcommand == CDD_ARAZZO_SUBCOMMAND_TO_SERVER) {
    rc = arazzo_server_generate(
        &arazzo_spec, has_openapi ? &openapi_spec : NULL, &client_cfg);
  } else if (config->subcommand == CDD_ARAZZO_SUBCOMMAND_TO_SDK) {
    rc = arazzo_client_generate(
        &arazzo_spec, has_openapi ? &openapi_spec : NULL, &client_cfg);
  } else {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (has_openapi) {
    openapi_spec_free(&openapi_spec);
  }
  arazzo_spec_free(&arazzo_spec);
  return rc;
}

/**
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
    arazzo_spec_free(&spec);
    return rc;
  }
  if (config->self_uri && *config->self_uri) {
    C_CDD_FREE(spec.self_uri);
    spec.self_uri = NULL;
    rc = c_cdd_strdup(config->self_uri, &spec.self_uri);
    if (rc != CDD_C_SUCCESS) {
      arazzo_spec_free(&spec);
      return rc;
    }
  }
  rc = arazzo_write_spec_to_json(&spec, &json);
  arazzo_spec_free(&spec);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  rc = fs_write_to_file(out_path, json);
  C_CDD_FREE(json);
  return rc;
}
