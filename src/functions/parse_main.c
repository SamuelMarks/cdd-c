/**
 * @file main.c
 * @brief Main entry point for the c_cdd CLI.
 *
 * Updated to include the `c2openapi`, `from_openapi`, `to_openapi`, and
 * `to_docs_json` commands.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cddConfig.h"
#include "classes/emit_schema_codegen.h"
#include "classes/parse_code2schema.h"
#include "functions/emit_build_system.h"
#include "functions/emit_sync.h"
#include "functions/parse_audit.h"
#include "functions/parse_orchestrator.h"
#include "functions/parse_str.h"
#include "openapi/parse_openapi.h"
#include "routes/emit_client_gen.h"
#include "routes/parse_cli.h" /* New entry */
#include "tests/emit_schema2tests.h"

#include <parson.h>

static void print_error(int rc, const char *command_name) {
  if (rc == 0)
    return;
  fprintf(stderr, "Error executing '%s': code %d\n", command_name, rc);
}

static int handle_audit(int argc, char **argv) {
  struct AuditStats stats;
  int rc;
  if (argc != 1)
    return EXIT_FAILURE;
  audit_stats_init(&stats);
  rc = audit_project(argv[0], &stats);
  audit_stats_free(&stats);
  return rc;
}

static void print_help(const char *prog_name) {
  printf("Usage: %s <command> [args]\n\n", prog_name);
  puts("Commands:");
  puts("  from_openapi -i <spec.json>");
  puts("      Generate C SDK from OpenAPI spec.");
  puts("  to_openapi -f <dir> [-o <out.json>]");
  puts("      Generate OpenAPI spec from C source code.");
  puts("  to_docs_json [--no-imports] [--no-wrapping] -i <spec.json>");
  puts("      Generate JSON code examples for doc sites.");
  puts("  audit <directory>");
  puts("      Scan directory for memory safety issues.");
  puts("  c2openapi <dir> <out.json>");
  puts("      Generate OpenAPI spec from C source code.");
  puts("  code2schema <header.h> <schema.json>");
  puts("      Convert C header to JSON Schema.");
}

static void print_version(void) { printf("cdd-c version %s\n", C_CDD_VERSION); }

static int handle_from_openapi(int argc, char **argv) {
  const char *input_file = NULL;
  int i;
  struct OpenAPI_Spec spec = {0};
  struct OpenApiClientConfig config = {0};
  int rc;
  JSON_Value *root;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
      input_file = argv[++i];
    }
  }
  if (!input_file) {
    fprintf(stderr, "Error: -i <spec.json> required\n");
    return EXIT_FAILURE;
  }

  root = json_parse_file(input_file);
  if (!root) {
    fprintf(stderr, "Failed to parse JSON file: %s\n", input_file);
    return EXIT_FAILURE;
  }

  rc = openapi_load_from_json(root, &spec);
  json_value_free(root);

  if (rc != 0) {
    fprintf(stderr, "Failed to load openapi spec from %s\n", input_file);
    return rc;
  }

  config.filename_base = "generated_client";
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  openapi_spec_free(&spec);
  return rc;
}

static int handle_to_openapi(int argc, char **argv) {
  const char *input_dir = NULL;
  const char *out_file = "openapi.json";
  char *c2_argv[3];
  int i;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
      input_dir = argv[++i];
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      out_file = argv[++i];
    }
  }
  if (!input_dir) {
    fprintf(stderr, "Error: -f <directory> required\n");
    return EXIT_FAILURE;
  }
  c2_argv[0] = "c2openapi";
  c2_argv[1] = (char *)input_dir;
  c2_argv[2] = (char *)out_file;
  return c2openapi_cli_main(3, c2_argv);
}

static int handle_to_docs_json(int argc, char **argv) {
  const char *input_file = NULL;
  int no_imports = 0;
  int no_wrapping = 0;
  int i;
  struct OpenAPI_Spec spec = {0};
  int rc;
  JSON_Value *root_val, *parsed_root;
  JSON_Array *root_arr;
  JSON_Value *lang_val;
  JSON_Object *lang_obj;
  JSON_Value *ops_val;
  JSON_Array *ops_arr;
  size_t p, op_idx;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
      input_file = argv[++i];
    } else if (strcmp(argv[i], "--no-imports") == 0) {
      no_imports = 1;
    } else if (strcmp(argv[i], "--no-wrapping") == 0) {
      no_wrapping = 1;
    }
  }

  if (!input_file) {
    fprintf(stderr, "Error: -i <spec.json> required\n");
    return EXIT_FAILURE;
  }

  parsed_root = json_parse_file(input_file);
  if (!parsed_root) {
    fprintf(stderr, "Failed to parse JSON file: %s\n", input_file);
    return EXIT_FAILURE;
  }

  rc = openapi_load_from_json(parsed_root, &spec);
  json_value_free(parsed_root);

  if (rc != 0) {
    fprintf(stderr, "Failed to load openapi spec from %s\n", input_file);
    return rc;
  }

  root_val = json_value_init_array();
  root_arr = json_value_get_array(root_val);

  lang_val = json_value_init_object();
  lang_obj = json_value_get_object(lang_val);
  json_object_set_string(lang_obj, "language", "c");

  ops_val = json_value_init_array();
  ops_arr = json_value_get_array(ops_val);

  for (p = 0; p < spec.n_paths; p++) {
    struct OpenAPI_Path *pi = &spec.paths[p];
    for (op_idx = 0; op_idx < pi->n_operations; op_idx++) {
      struct OpenAPI_Operation *op = &pi->operations[op_idx];
      JSON_Value *op_val = json_value_init_object();
      JSON_Object *op_obj = json_value_get_object(op_val);
      JSON_Value *code_val = json_value_init_object();
      JSON_Object *code_obj = json_value_get_object(code_val);

      const char *method = "";
      char snippet[1024];
      const char *op_id = op->operation_id ? op->operation_id : "unknown";

      switch (op->verb) {
      case OA_VERB_GET:
        method = "GET";
        break;
      case OA_VERB_POST:
        method = "POST";
        break;
      case OA_VERB_PUT:
        method = "PUT";
        break;
      case OA_VERB_DELETE:
        method = "DELETE";
        break;
      case OA_VERB_PATCH:
        method = "PATCH";
        break;
      case OA_VERB_HEAD:
        method = "HEAD";
        break;
      case OA_VERB_OPTIONS:
        method = "OPTIONS";
        break;
      case OA_VERB_TRACE:
        method = "TRACE";
        break;
      case OA_VERB_QUERY:
        method = "QUERY";
        break;
      default:
        method =
            op->is_additional ? (op->method ? op->method : "CUSTOM") : "CUSTOM";
        break;
      }

      json_object_set_string(op_obj, "method", method);
      json_object_set_string(op_obj, "path", pi->route);
      json_object_set_string(op_obj, "operationId", op_id);

      if (!no_imports) {
        json_object_set_string(
            code_obj, "imports",
            "#include \"generated_client.h\"\n#include <stdio.h>\n");
      }
      if (!no_wrapping) {
        json_object_set_string(
            code_obj, "wrapper_start",
            "int main(void) {\n  struct HttpClient client;\n  struct ApiError "
            "*err = NULL;\n  api_init(&client, "
            "\"https://api.example.com\");\n");
        json_object_set_string(code_obj, "wrapper_end",
                               "  api_cleanup(&client);\n  return 0;\n}\n");
      }

      /* Format specific snippet depending on how the SDK gets generated.
         Ideally it matches C conventions of this SDK builder. */
      snprintf(snippet, sizeof(snippet),
               "  /* Call the %s API */\n  int rc = api_%s(&client, &err);\n  "
               "if (rc != 0) {\n    /* handle error */\n  }\n",
               op_id, op_id);
      json_object_set_string(code_obj, "snippet", snippet);

      json_object_set_value(op_obj, "code", code_val);
      json_array_append_value(ops_arr, op_val);
    }
  }

  json_object_set_value(lang_obj, "operations", ops_val);
  json_array_append_value(root_arr, lang_val);

  {
    char *serialized = json_serialize_to_string_pretty(root_val);
    printf("%s\n", serialized);
    json_free_serialized_string(serialized);
  }

  json_value_free(root_val);
  openapi_spec_free(&spec);
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  int rc = 0;
  const char *cmd;

  if (argc < 2) {
    print_help(argc > 0 ? argv[0] : "cdd-c");
    return EXIT_FAILURE;
  }

  cmd = argv[1];

  if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0) {
    print_version();
    return EXIT_SUCCESS;
  }

  if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
    print_help(argv[0]);
    return EXIT_SUCCESS;
  }

  if (strcmp(cmd, "audit") == 0) {
    if (argc < 3)
      return EXIT_FAILURE;
    rc = handle_audit(argc - 2, argv + 2);
  } else if (strcmp(cmd, "c2openapi") == 0) {
    rc = c2openapi_cli_main(argc - 1, argv + 1);
  } else if (strcmp(cmd, "code2schema") == 0) {
    if (argc != 4)
      return EXIT_FAILURE;
    rc = code2schema_main(argc - 2, argv + 2);
  } else if (strcmp(cmd, "from_openapi") == 0) {
    rc = handle_from_openapi(argc - 1, argv + 1);
  } else if (strcmp(cmd, "to_openapi") == 0) {
    rc = handle_to_openapi(argc - 1, argv + 1);
  } else if (strcmp(cmd, "to_docs_json") == 0) {
    rc = handle_to_docs_json(argc - 1, argv + 1);
  } else {
    /* Fallback for other commands */
    if (strcmp(cmd, "openapi2client") == 0) {
      /* Keep previous behavior stub */
      return EXIT_FAILURE;
    }
    fprintf(stderr, "Unknown command: %s\n", cmd);
    return EXIT_FAILURE;
  }

  if (rc != 0) {
    print_error(rc, cmd);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
