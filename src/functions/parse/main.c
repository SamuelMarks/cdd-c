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
#include "classes/emit/schema_codegen.h"
#include "classes/parse/code2schema.h"
#include "functions/emit/build_system.h"
#include "functions/emit/sync.h"
#include "functions/parse/audit.h"
#include "functions/parse/orchestrator.h"
#include "functions/parse/str.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/cli_gen.h"
#include "routes/emit/client_gen.h"
#include "routes/emit/server_json_rpc.h"
#include "routes/parse/cli.h" /* New entry */
#include "tests/emit/schema2tests.h"

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
  puts("  from_openapi to_sdk -i <spec.json> [-o <dir>]");
  puts("  from_openapi to_sdk --input-dir <specs_dir> [-o <dir>]");
  puts("  from_openapi to_sdk_cli -i <spec.json> [-o <dir>]");
  puts("  from_openapi to_sdk_cli --input-dir <specs_dir> [-o <dir>]");
  puts("      Generate C SDK and optionally CLI from OpenAPI spec.");
  puts("  to_openapi -f <dir> [-o <out.json>]");
  puts("      Generate OpenAPI spec from C source code.");
  puts("  to_docs_json [--no-imports] [--no-wrapping] -i|--input <spec.json>");
  puts("      Generate JSON code examples for doc sites.");
  puts("  audit <directory>");
  puts("      Scan directory for memory safety issues.");
  puts("  c2openapi <dir> <out.json>");
  puts("      Generate OpenAPI spec from C source code.");
  puts("  code2schema <header.h> <schema.json>");
  puts("      Convert C header to JSON Schema.");
  puts("  generate_build_system <type> <out_dir> <name> [test_file]");
  puts("      Generate build system files.");
  puts("  schema2code <schema.json> <out_dir>");
  puts("      Generate C code from JSON schema.");
  puts("  jsonschema2tests <schema.json> <header_to_test.h> <out.h>");
  puts("      Generate C tests from JSON schema.");
}

static void print_version(void) { printf("cdd-c version %s\n", C_CDD_VERSION); }

static int handle_from_openapi(int argc, char **argv) {
  const char *input_file = NULL;
  const char *input_dir = NULL;
  const char *out_dir = NULL;
  int is_cli = 0;
  int i;
  struct OpenAPI_Spec spec = {0};

  struct OpenApiClientConfig config = {0};
  int rc = 0;
  JSON_Value *root;

  input_file = getenv("CDD_INPUT_FILE") ? getenv("CDD_INPUT_FILE")
                                        : getenv("INPUT_FILE");
  input_dir =
      getenv("CDD_INPUT_DIR") ? getenv("CDD_INPUT_DIR") : getenv("INPUT_DIR");
  out_dir = getenv("CDD_OUT_DIR") ? getenv("CDD_OUT_DIR") : getenv("OUT_DIR");

  if (argc > 1) {
    if (strcmp(argv[1], "to_sdk") == 0) {
      is_cli = 0;
      argc--;
      argv++;
    } else if (strcmp(argv[1], "to_sdk_cli") == 0) {
      is_cli = 1;
      argc--;
      argv++;
    }
  }

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
      input_file = argv[++i];
    } else if (strcmp(argv[i], "--input-dir") == 0 && i + 1 < argc) {
      input_dir = argv[++i];
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      out_dir = argv[++i];
    }
  }

  if (!input_file && !input_dir) {
    fprintf(stderr, "Error: -i <spec.json> or --input-dir <dir> required\n");
    return EXIT_FAILURE;
  }

  if (input_file) {
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

    if (out_dir) {
      char *path = malloc(strlen(out_dir) + 32);
      sprintf(path, "%s/generated_client", out_dir);
      config.filename_base = path;
    } else {
      config.filename_base = "generated_client";
    }
    config.func_prefix = "api_";

    rc = openapi_client_generate(&spec, &config);
    if (is_cli) {
      openapi_cli_generate(&spec, &config);
    }
    if (out_dir) {
      free((void *)config.filename_base);
    }
    openapi_spec_free(&spec);
  }

  return rc;
}

static int handle_to_openapi(int argc, char **argv) {
  const char *input_dir =
      getenv("CDD_INPUT_DIR") ? getenv("CDD_INPUT_DIR") : getenv("INPUT_DIR");
  const char *out_file =
      getenv("CDD_OUT_FILE")
          ? getenv("CDD_OUT_FILE")
          : (getenv("OUT_FILE") ? getenv("OUT_FILE") : "openapi.json");
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
    rc = to_docs_json_cli_main(argc - 1, argv + 1);
  } else if (strcmp(cmd, "generate_build_system") == 0) {
    rc = generate_build_system_main(argc - 2, argv + 2);
  } else if (strcmp(cmd, "schema2code") == 0) {
    rc = schema2code_main(argc - 2, argv + 2);
  } else if (strcmp(cmd, "server_json_rpc") == 0) {
    rc = server_json_rpc_main(argc - 1, argv + 1);
  } else if (strcmp(cmd, "jsonschema2tests") == 0) {
    rc = jsonschema2tests_main(argc - 2, argv + 2);
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
