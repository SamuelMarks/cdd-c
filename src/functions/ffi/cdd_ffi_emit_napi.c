#include "c_cdd/safe_crt.h"
/* clang-format off */
#include "cdd_ffi_emit_napi.h"
#include <stdio.h>
#include "c_cdd/format_specifiers.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int emit_napi_c(cdd_ffi_ir_t *ir,
                       const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s_napi.c", config->output_dir,
               lib_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s_napi.c", config->output_dir,
               lib_name);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "/* Auto-generated Node.js N-API bindings for %s */\n\n",
          lib_name);
  fprintf(f, "#include <node_api.h>\n");
  fprintf(f, "#include <stdlib.h>\n");
  fprintf(f, "#include <string.h>\n ");
  fprintf(f, "#include \"%s\"\n\n", config->input);

  fprintf(f, "/* Helper for checking N-API calls */\n");
  fprintf(
      f,
      "#define NAPI_CALL(env, call)                                      \\\n");
  fprintf(
      f,
      "  do {                                                            \\\n");
  fprintf(
      f,
      "    napi_status status = (call);                                  \\\n");
  fprintf(
      f,
      "    if (status != napi_ok) {                                      \\\n");
  fprintf(
      f,
      "      const napi_extended_error_info* error_info = NULL;          \\\n");
  fprintf(
      f,
      "      napi_get_last_error_info((env), &error_info);               \\\n");
  fprintf(
      f,
      "      int is_pending;                                             \\\n");
  fprintf(
      f,
      "      napi_is_exception_pending((env), &is_pending);              \\\n");
  fprintf(
      f,
      "      if (!is_pending) {                                          \\\n");
  fprintf(
      f,
      "        const char* message = (error_info->error_message == NULL) \\\n");
  fprintf(f, "            ? \"empty error message\"                            "
             "   \\\n");
  fprintf(
      f,
      "            : error_info->error_message;                          \\\n");
  fprintf(
      f,
      "        napi_throw_error((env), NULL, message);                   \\\n");
  fprintf(
      f,
      "      }                                                           \\\n");
  fprintf(
      f,
      "      return NULL;                                                \\\n");
  fprintf(
      f,
      "    }                                                             \\\n");
  fprintf(f, "  } while(0)\n\n");

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (node->requires_gil_release) {
        /* Generate async worker for Node.js */
        fprintf(f, "/* Async worker for %s */\n", node->name);
        fprintf(f, "typedef struct {\n");
        fprintf(f, "  napi_async_work work;\n");
        fprintf(f, "  napi_deferred deferred;\n");
        fprintf(f, "  /* Add args and result here */\n");
        fprintf(f, "} %s_worker_data;\n\n", node->name);

        fprintf(f, "static void execute_%s(napi_env env, void* data) {\n",
                node->name);
        fprintf(f, "  %s_worker_data* worker_data = (%s_worker_data*)data;\n",
                node->name, node->name);
        fprintf(f, "  /* Call C function: %s(); */\n", node->name);
        fprintf(f, "}\n\n");

        fprintf(f,
                "static void complete_%s(napi_env env, napi_status status, "
                "void* data) {\n",
                node->name);
        fprintf(f, "  %s_worker_data* worker_data = (%s_worker_data*)data;\n",
                node->name, node->name);
        fprintf(f, "  napi_value result;\n");
        fprintf(f, "  napi_get_undefined(env, &result);\n");
        fprintf(
            f,
            "  napi_resolve_deferred(env, worker_data->deferred, result);\n");
        fprintf(f, "  napi_delete_async_work(env, worker_data->work);\n");
        fprintf(f, "  free(worker_data);\n");
        fprintf(f, "}\n\n");
      }

      fprintf(f,
              "static napi_value wrap_%s(napi_env env, napi_callback_info "
              "info) {\n",
              node->name);

      if (node->requires_gil_release) {
        fprintf(f, "  napi_value promise;\n");
        fprintf(f, "  napi_value resource_name;\n");
        fprintf(f,
                "  %s_worker_data* worker_data = (%s_worker_data*)calloc(1, "
                "sizeof(%s_worker_data));\n",
                node->name, node->name, node->name);
        fprintf(
            f,
            "  napi_create_promise(env, &worker_data->deferred, &promise);\n");
        fprintf(f,
                "  napi_create_string_utf8(env, \"%s_work\", NAPI_AUTO_LENGTH, "
                "&resource_name);\n",
                node->name);
        fprintf(f,
                "  napi_create_async_work(env, NULL, resource_name, "
                "execute_%s, complete_%s, worker_data, &worker_data->work);\n",
                node->name, node->name);
        fprintf(f, "  napi_queue_async_work(env, worker_data->work);\n");
        fprintf(f, "  return promise;\n");
        fprintf(f, "}\n\n");
        continue;
      }

      if (node->fields_count > 0) {
        fprintf(f, "  size_t argc = %" CDD_PRIz ";\n", node->fields_count);
        fprintf(f, "  napi_value argv[%" CDD_PRIz "];\n", node->fields_count);
        fprintf(f, "  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, "
                   "NULL, NULL));\n");
      } else {
        fprintf(f, "  NAPI_CALL(env, napi_get_cb_info(env, info, NULL, NULL, "
                   "NULL, NULL));\n");
      }

      for (j = 0; j < node->fields_count; j++) {
        fprintf(f, "  /* Mapping argument %" CDD_PRIz ": %s */\n", j,
                node->fields[j].name ? node->fields[j].name : "arg");
        /* In a full implementation we'd check type.kind and map to
         * napi_get_value_int32 etc */
        /* For now we stub the argument parsing to satisfy C89 and allow
         * compiling */
      }

      fprintf(f, "\n  /* %s(...) */\n", node->name);
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f,
                "  /* Call the C function and map return to N-API value */\n");
        fprintf(f, "  napi_value result;\n");
        fprintf(f, "  NAPI_CALL(env, napi_get_undefined(env, &result));\n");
        fprintf(f, "  return result;\n");
      } else {
        fprintf(f, "  napi_value result;\n");
        fprintf(f, "  NAPI_CALL(env, napi_get_undefined(env, &result));\n");
        fprintf(f, "  return result;\n");
      }
      fprintf(f, "}\n\n");
    }
  }

  /* Init function */
  fprintf(f, "static napi_value Init(napi_env env, napi_value exports) {\n");
  fprintf(f, "  napi_property_descriptor desc[] = {\n");
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "    { \"%s\", 0, wrap_%s, 0, 0, 0, napi_default, 0 },\n",
              node->name, node->name);
    }
  }
  fprintf(f, "  };\n");

  fprintf(f, "  size_t prop_count = sizeof(desc) / sizeof(desc[0]);\n");
  fprintf(f, "  if (prop_count > 0) {\n");
  fprintf(f, "    NAPI_CALL(env, napi_define_properties(env, exports, "
             "prop_count, desc));\n");
  fprintf(f, "  }\n");
  fprintf(f, "  return exports;\n");
  fprintf(f, "}\n\n");

  fprintf(f, "NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)\n");

  fclose(f);

  if (config->generate_tests) {
    char filepath_test[1024];
    FILE *ft = NULL;
#if defined(_MSC_VER)
    CDD_SNPRINTF(filepath_test, sizeof(filepath_test), "%s\\test_%s.js",
                 config->output_dir,
                 config->library_name ? config->library_name : "mylib");
    fopen_s(&ft, filepath_test, "w");
#else
    CDD_SNPRINTF(filepath_test, sizeof(filepath_test), "%s/test_%s.js",
                 config->output_dir,
                 config->library_name ? config->library_name : "mylib");
    ft = fopen(filepath_test, "w");
#endif
    if (ft) {
      fprintf(ft, "// Auto-generated tests for %s\n",
              config->library_name ? config->library_name : "mylib");
      fprintf(ft, "const lib = require('./build/Release/%s.node');\n\n",
              config->library_name ? config->library_name : "mylib");
      for (i = 0; i < ir->nodes_count; i++) {
        cdd_ffi_ir_node_t *node = &ir->nodes[i];
        if (node->kind == CDD_FFI_NODE_FUNCTION) {
          int out_count = 0;
          for (j = 0; j < node->fields_count; j++) {
            if (node->fields[j].intent == CDD_FFI_INTENT_OUT ||
                node->fields[j].intent == CDD_FFI_INTENT_INOUT)
              out_count++;
          }
          if (out_count > 0) {
            fprintf(ft, "// Multiple return values test placeholder\n");
            fprintf(ft, "// const res = lib.%s(...);\n", node->name);
            fprintf(ft, "// if (!Array.isArray(res)) throw new Error('Expected "
                        "array');\n\n");
          }
        }
      }
      fclose(ft);
    }
  }

  return 0;
}

static int emit_binding_gyp(const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  const char *lib_name = config->library_name ? config->library_name : "mylib";

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\binding.gyp",
               config->output_dir);
  if (fopen_s(&f, filepath, "w") != 0) {
    return 1;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/binding.gyp",
               config->output_dir);
  f = fopen(filepath, "w");
  if (!f) {
    return 1;
  }
#endif

  fprintf(f, "{\n");
  fprintf(f, "  \"targets\": [\n");
  fprintf(f, "    {\n");
  fprintf(f, "      \"target_name\": \"%s\",\n", lib_name);
  fprintf(f, "      \"sources\": [ \"%s_napi.c\" ]\n", lib_name);
  fprintf(f, "    }\n");
  fprintf(f, "  ]\n");
  fprintf(f, "}\n");

  fclose(f);
  return 0;
}

int cdd_ffi_emit_napi(cdd_ffi_ir_t *ir,
                      const cdd_generate_bindings_config_t *config) {
  int rc;
  if (!ir || !config || !config->output_dir) {
    return 1;
  }

  rc = emit_napi_c(ir, config);
  if (rc != 0)
    return rc;

  return emit_binding_gyp(config);
}
