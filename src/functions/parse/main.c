/**
 * @file main.c
 * @brief Main entry point for the c_cdd CLI.
 *
 * Updated to include the `c2openapi`, `from_openapi`, `to_openapi`, and
 * `to_docs_json` commands.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/main.h"
#include "routes/parse/cli_cst.h"
#include "c_cddConfig.h"
#include "classes/emit/schema_codegen.h"
#include "classes/parse/code2schema.h"
#include "functions/emit/build_system.h"
#include "functions/emit/sync.h"
#include "functions/parse/audit.h"
#include "functions/parse/orchestrator.h"
#include "functions/parse/str.h"
#include "functions/parse/db_loader.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/cli_gen.h"
#include "routes/emit/client_gui_gen.h"
#include "routes/emit/client_gen.h"
#include "routes/emit/server_gen.h"
#include "routes/emit/serve_json_rpc.h"
#include "routes/parse/cli.h" /* New entry */
#include "tests/emit/schema2tests.h"

#include <parson.h>
/* clang-format on */
/* LCOV_EXCL_START */

static void print_version(void) {
  printf("cdd-c version %s\n", C_CDD_VERSION);
  printf("Database Driver Support:\n");

  printf("  - PostgreSQL: ");
#if defined(USE_LIBPQ_LINKED)
  printf("Linked (Packaged/Statically Compiled)\n");
#elif defined(USE_LIBPQ_DYNAMIC)
  if (check_libpq_available()) {
    printf("Found (Dynamically Loaded via system search path)\n");
  } else {
    printf("Not Found (Dynamic loading failed)\n");
  }
#else
  printf("Disabled (Not compiled)\n");
#endif

  printf("  - SQLite: ");
#if defined(USE_SQLITE_LINKED)
  printf("Linked (Packaged/Statically Compiled)\n");
#elif defined(USE_SQLITE_DYNAMIC)
  if (check_sqlite3_available()) {
    printf("Found (Dynamically Loaded via system search path)\n");
  } else {
    printf("Not Found (Dynamic loading failed)\n");
  }
#else
  printf("Disabled (Not compiled)\n");
#endif

  printf("  - MySQL/MariaDB: ");
#if defined(USE_MYSQL_LINKED)
  printf("Linked (Packaged/Statically Compiled)\n");
#elif defined(USE_MYSQL_DYNAMIC)
  if (check_mysql_available()) {
    printf("Found (Dynamically Loaded via system search path)\n");
  } else {
    printf("Not Found (Dynamic loading failed)\n");
  }
#else
  printf("Disabled (Not compiled)\n");
#endif
}

/**
 * @brief Helper to display error messages for failed commands.
 *
 * Only outputs an error message if the return code is non-zero.
 *
 * @param[in] rc The return code from the executed command
 * @param[in] command_name The name of the command that failed
 */
static void print_error(int rc, const char *command_name) {
  if (rc == 0)
    return;
  fprintf(stderr, "Error executing '%s': code %d\n", command_name, rc);
}

/**
 * @brief Audits a target project directory for common code issues.
 *
 * Runs static analysis to find potential memory leaks or rule violations
 * and prints a summary. Requires exactly one argument: the directory
 * path.
 *
 * @param[in] argc Argument count for the command (should be 1)
 * @param[in] argv Argument values containing the directory path
 * @return EXIT_SUCCESS or the error code from audit_project
 */
static enum cdd_c_error handle_audit(int argc, char **argv) {
  struct AuditStats stats;
  int rc;
  if (argc != 1)
    return CDD_C_ERROR_UNKNOWN;
  audit_stats_init(&stats);
  rc = audit_project(argv[0], &stats);
  audit_stats_free(&stats);
  return rc;
}

/**
 * @brief Displays CLI usage information and a list of available
 * commands.
 *
 * @param[in] prog_name The program executable name (usually argv[0])
 */
static void print_help(const char *prog_name) {
  printf("Usage: %s [OPTIONS] <COMMAND>\n\n", prog_name);
  puts("Commands:");
  puts("  from_openapi to_sdk -i <spec.json> [-o <dir>] [--no-github-actions] "
       "[--no-installable-package] [--tests]");
  puts("  from_openapi to_sdk --input-dir <specs_dir> [-o <dir>] "
       "[--no-github-actions] [--no-installable-package] [--tests]");
  puts("  from_openapi to_sdk_cli -i <spec.json> [-o <dir>] "
       "[--no-github-actions] [--no-installable-package] [--tests]");
  puts("  from_openapi to_sdk_cli --input-dir <specs_dir> [-o <dir>] "
       "[--no-github-actions] [--no-installable-package] [--tests]");
  puts("  from_openapi to_server -i <spec.json> [-o <dir>] "
       "[--no-github-actions] [--no-installable-package] [--tests]");
  puts("  from_openapi to_server --input-dir <specs_dir> [-o <dir>] "
       "[--no-github-actions] [--no-installable-package] [--tests]");
  puts("      Generate code from an OpenAPI specification.");
  puts("  to_openapi -i <dir> [-o <out.json>]");
  puts("      Generate an OpenAPI specification from source code.");
  puts("  to_docs_json [--no-imports] [--no-wrapping] -i|--input <spec.json> "
       "[-o <docs.json>]");
  puts("      Generate JSON documentation with code snippets for an OpenAPI "
       "specification.");
  puts("  bind [OPTIONS]");
  puts("      Generate SWIG-like FFI bindings for 40 languages.");
  puts("  serve_json_rpc [-p|--port <int>] [-l|--listen <address>]");
  puts("      Expose CLI interface as a JSON-RPC server.");
  puts("  mcp");
  puts("      Expose CLI interface as an MCP server via stdio.");
  puts("");
  puts("Language-Specific Commands:");
  puts("  audit <directory>");
  puts("      Scan directory for memory safety issues.");
  puts("  c2openapi <dir> <out.json>");
  puts("      Generate OpenAPI spec from C source code.");
  puts("  transformer <toolname> [--audit|--fix] [--dry-run] <files...>");
  puts("      Run syntax tree transformations.");
  puts("  code2schema <header.h> <schema.json>");
  puts("      Convert C header to JSON Schema.");
  puts("  generate_build_system <type> <out_dir> <name> [test_file]");
  puts("      Generate build system files.");
  puts("  schema2code <schema.json> <out_dir>");
  puts("      Generate C code from JSON schema.");
}

/**
 * @brief Parses and handles the `from_openapi` command.
 *
 * Loads the provided OpenAPI specification and acts as a router to the
 * correct sub-command: `to_sdk`, `to_sdk_cli`, or `to_server`.
 * Generates C bindings, structs, and implementations.
 *
 * @param[in] argc Argument count, stripped of the main program name
 * @param[in] argv Argument values for the command execution
 * @return EXIT_SUCCESS if code generation completes without error
 */
enum cdd_c_error from_openapi_cli_main(int argc, char **argv) {
  const char *input_file = NULL;
  const char *input_dir = NULL;
  const char *out_dir = NULL;
  int is_cli = 0;
  int is_server = 0;
  int i;
  struct OpenAPI_Spec spec = {0};

  struct OpenApiClientConfig config = {0};
  int rc = 0;
  JSON_Value *root;

  input_file = getenv("CDD_INPUT") ? getenv("CDD_INPUT") : getenv("INPUT_FILE");
  input_dir =
      getenv("CDD_INPUT_DIR") ? getenv("CDD_INPUT_DIR") : getenv("INPUT_DIR");
  out_dir = getenv("CDD_OUTPUT") ? getenv("CDD_OUTPUT") : getenv("OUT_DIR");

  if (argc > 1) {
    if (strcmp(argv[1], "to_sdk") == 0) {
      is_cli = 0;
      argc--;
      argv++;
    } else if (strcmp(argv[1], "to_sdk_cli") == 0) {
      is_cli = 1;
      argc--;
      argv++;
    } else if (strcmp(argv[1], "to_server") == 0) {
      is_server = 1;
      argc--;
      argv++;
    }
  }

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      puts("Usage: cdd-c from_openapi [to_sdk|to_sdk_cli|to_server] [args]");
      puts("");
      puts("Commands:");
      puts("  to_sdk         Generate a Client SDK from an OpenAPI "
           "specification.");
      puts("  to_sdk_cli     Generate a Client SDK CLI from an OpenAPI "
           "specification.");
      puts("  to_server      Generate a Server stub from an OpenAPI "
           "specification.");
      puts("");
      puts("Options:");
      puts("  -i, --input <spec.json>   Input OpenAPI spec file");
      puts("  --input-dir <specs_dir>   Input directory containing OpenAPI "
           "specs");
      puts("  -o, --output <dir>        Output directory");
      puts("  --no-github-actions       Do not generate GitHub Actions CI "
           "workflow");
      puts("  --no-installable-package  Do not generate build system files "
           "(e.g. CMakeLists.txt)");
      puts("  --tests                   Generate composable tests and mocks");
      return CDD_C_SUCCESS;
    } else if ((strcmp(argv[i], "-i") == 0 ||
                strcmp(argv[i], "--input") == 0) &&
               i + 1 < argc) {
      input_file = argv[++i];
    } else if (strcmp(argv[i], "--input-dir") == 0 && i + 1 < argc) {
      input_dir = argv[++i];
    } else if ((strcmp(argv[i], "-o") == 0 ||
                strcmp(argv[i], "--output") == 0) &&
               i + 1 < argc) {
      out_dir = argv[++i];
    } else if (strcmp(argv[i], "--no-github-actions") == 0) {
      config.no_github_actions = 1;
    } else if (strcmp(argv[i], "--no-installable-package") == 0) {
      config.no_installable_package = 1;
    } else if (strcmp(argv[i], "--tests") == 0) {
      config.create_tests_and_mocks = 1;
    }
  }

  if (!input_file && !input_dir) {
    fprintf(stderr,
            "Error: -i|--input <spec.json> or --input-dir <dir> required\n");
    return CDD_C_ERROR_UNKNOWN;
  }

  if (input_file) {
    root = json_parse_file(input_file);
    if (!root) {
      fprintf(stderr, "Failed to parse JSON file: %s\n", input_file);
      return CDD_C_ERROR_UNKNOWN;
    }

    rc = openapi_load_from_json(root, &spec);

    json_value_free(root);

    if (rc != 0) {
      fprintf(stderr, "Failed to load openapi spec from %s\n", input_file);
      return rc;
    }

    if (out_dir) {
      char *path = malloc(strlen(out_dir) + 32);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      sprintf_s(path, strlen(out_dir) + 32, "%s/generated_client", out_dir);
#else
      sprintf(path, "%s/generated_client", out_dir);
#endif
      config.filename_base = path;
    } else {
      config.filename_base = "generated_client";
    }
    config.func_prefix = "api_";

    rc = openapi_client_generate(&spec, &config);
    openapi_client_gui_generate(&spec, &config);
    if (is_cli) {
      openapi_cli_generate(&spec, &config);
    }
    if (is_server) {
      openapi_server_generate(&spec, &config);
    }

    /* Always generate ORM models for to_sdk and to_server */

    if (out_dir) {
      free((void *)config.filename_base);
    }
    openapi_spec_free(&spec);
  }

  return rc;
}

/**
 * @brief Handles the `to_openapi` command parsing C code to an OpenAPI
 * spec.
 *
 * Invokes the internal C-to-OpenAPI translation logic and writes the
 * result.
 *
 * @param[in] argc Argument count including command flags
 * @param[in] argv Argument values pointing to the source directory and
 * options
 * @return EXIT_SUCCESS if parsing and serialization succeed
 */
enum cdd_c_error to_openapi_cli_main(int argc, char **argv) {
  const char *input_dir =
      getenv("CDD_INPUT") ? getenv("CDD_INPUT") : getenv("INPUT_DIR");
  const char *out_file =
      getenv("CDD_OUTPUT")
          ? getenv("CDD_OUTPUT")
          : (getenv("OUT_FILE") ? getenv("OUT_FILE") : "openapi.json");
  char *c2_argv[3];
  int i;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      puts("Usage: cdd-c to_openapi [args]");
      puts("");
      puts("Options:");
      puts(
          "  -i, --input <dir>       Input directory containing C source code");
      puts("  -o, --output <out.json> Output OpenAPI spec file (default: "
           "openapi.json)");
      return CDD_C_SUCCESS;
    } else if ((strcmp(argv[i], "-i") == 0 ||
                strcmp(argv[i], "--input") == 0) &&
               i + 1 < argc) {
      input_dir = argv[++i];
    } else if ((strcmp(argv[i], "-o") == 0 ||
                strcmp(argv[i], "--output") == 0) &&
               i + 1 < argc) {
      out_file = argv[++i];
    }
  }
  if (!input_dir) {
    fprintf(stderr, "Error: -i <directory> required\n");
    return CDD_C_ERROR_UNKNOWN;
  }
  {
    char snapshot_path[1024];
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(snapshot_path, sizeof(snapshot_path), "%s/openapi.snapshot.json",
              input_dir);
#else
    sprintf(snapshot_path, "%s/openapi.snapshot.json", input_dir);
#endif
    f = fopen(snapshot_path, "r");
    if (f) {
      char *c2_argv_base[5];
      fclose(f);
      c2_argv_base[0] = (char *)"c2openapi";
      c2_argv_base[1] = (char *)"--base";
      c2_argv_base[2] = snapshot_path;
      c2_argv_base[3] = (char *)input_dir;
      c2_argv_base[4] = (char *)out_file;
      return c2openapi_cli_main(5, c2_argv_base);
    }
  }

  c2_argv[0] = (char *)"c2openapi";
  c2_argv[1] = (char *)input_dir;
  c2_argv[2] = (char *)out_file;
  return c2openapi_cli_main(3, c2_argv);
}

/** @brief main definition */
/**
 * @brief Main entry point dispatcher.
 *
 * This function routes execution to the specific sub-command requested by the
 * user, e.g., `audit`, `c2openapi`, `generate_build_system`. It handles
 * `--version` and
 * `--help` directly.
 *
 * @param[in] argc Passed straight from application main.
 * @param[in] argv Passed straight from application main.
 * @return Returns an exit code (0 for success, non-zero for failure).
 */
enum cdd_c_error cdd_main(int argc, char **argv);
/**
 * @brief Main entry point dispatcher execution logic.
 *
 * Implements the command routing and prints help or error outputs as needed.
 */
enum cdd_c_error cdd_main(int argc, char **argv) {
  int rc = 0;
  const char *cmd;

  if (argc < 2) {
    print_help(argc > 0 ? argv[0] : "cdd-c");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  cmd = argv[1];

  if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0) {
    print_version();
    return CDD_C_SUCCESS;
  }

  if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
    print_help(argv[0]);
    return CDD_C_SUCCESS;
  }

  if (strcmp(cmd, "audit") == 0) {
    if (argc < 3)
      return CDD_C_ERROR_UNKNOWN;
    rc = handle_audit(argc - 2, argv + 2);
  } else if (strcmp(cmd, "c2openapi") == 0) {
    rc = c2openapi_cli_main(argc - 1, argv + 1);
  } else if (strcmp(cmd, "transformer") == 0) {
    rc = cli_cst_transformer_main(argc - 2, argv + 2);
  } else if (strcmp(cmd, "code2schema") == 0) {
    if (argc != 4)
      return CDD_C_ERROR_UNKNOWN;
    rc = code2schema_main(argc - 2, argv + 2);
  } else if (strcmp(cmd, "from_openapi") == 0) {
    rc = from_openapi_cli_main(argc - 1, argv + 1);
  } else if (strcmp(cmd, "to_openapi") == 0) {
    rc = to_openapi_cli_main(argc - 1, argv + 1);
  } else if (strcmp(cmd, "to_docs_json") == 0) {
    rc = to_docs_json_cli_main(argc - 1, argv + 1);
  } else if (strcmp(cmd, "bind") == 0) {
    rc = generate_bindings_cli_main(argc - 1, argv + 1);
  } else if (strcmp(cmd, "generate_build_system") == 0) {
    rc = generate_build_system_main(argc - 2, argv + 2);
  } else if (strcmp(cmd, "schema2code") == 0) {
    rc = schema2code_main(argc - 2, argv + 2);
  } else if (strcmp(cmd, "serve_json_rpc") == 0) {
    rc = serve_json_rpc_main(argc - 1, argv + 1);
  } else if (strcmp(cmd, "mcp") == 0) {
    /* Expose Generator via MCP stdio */
    /* Register Tools: cdd_generate (Code Scaffold), cdd_inspect (Schema
     * Inspection), cdd_sync (Bidirectional Sync) */
    printf("Starting MCP server for cdd generator via stdio...\n");
    rc = serve_mcp_stdio_main(argc - 1, argv + 1);
  } else {
    /* Fallback for other commands */
    if (strcmp(cmd, "openapi2client") == 0) {
      /* Keep previous behavior stub */
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    fprintf(stderr, "Error: Unknown or incomplete command: %s\n", cmd);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (rc != 0) {
    print_error(rc, cmd);
    return rc;
  }

  return CDD_C_SUCCESS;
}

/* LCOV_EXCL_STOP */
