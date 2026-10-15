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
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cddConfig.h"
#include "c_cdd_export.h"
#include "cdd_api.h"
#include "c_cdd/memory.h"
#include "classes/emit/schema_codegen.h"
#include "classes/parse/code2schema.h"
#include "functions/emit/build_system.h"
#include "functions/emit/sync.h"
#include "functions/parse/audit.h"
#include "functions/parse/db_loader.h"
#include "functions/parse/main.h"
#include "functions/parse/orchestrator.h"
#include "functions/parse/str.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/cli_gen.h"
#include "routes/emit/client_gen.h"
#include "routes/emit/client_gui_gen.h"
#include "routes/emit/serve_json_rpc.h"
#include "routes/emit/server_gen.h"
#include "routes/parse/cli.h" /* New entry */
#include "routes/parse/cli_cst.h"
#include "tests/emit/schema2tests.h"

#include <parson.h>
/* clang-format on */

C_CDD_EXPORT cdd_c_error_t print_version(void);
C_CDD_EXPORT cdd_c_error_t handle_audit(int argc, char **argv);
C_CDD_EXPORT cdd_c_error_t print_help(const char *program_name);

/**
 * @brief Prints version and driver information to stdout.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t print_version(void) {
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
  return CDD_C_SUCCESS;
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
C_CDD_EXPORT cdd_c_error_t handle_audit(int argc, char **argv) {
  struct AuditStats stats;
  cdd_c_error_t rc;
  if (argc != 1)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = audit_stats_init(&stats);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = audit_project(argv[0], &stats);
  audit_stats_free(&stats);
  return rc;
}

/**
 * @brief Displays CLI usage information and a list of available
 * commands.
 *
 * @param[in] program_name The program executable name (usually argv[0])
 */
C_CDD_EXPORT cdd_c_error_t print_help(const char *program_name) {
  printf("Usage: %s [OPTIONS] <COMMAND>\n\n", program_name);
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
  puts("  standardize-gnu [OPTIONS] <files...>");
  puts("      Standardize GNU C extensions to ISO C.");
  puts("  transformer <toolname> [--audit|--fix] [--dry-run] <files...>");
  puts("      Run syntax tree transformations.");
  puts("  code2schema <header.h> <schema.json>");
  puts("      Convert C header to JSON Schema.");
  puts("  generate_build_system <type> <out_dir> <name> [test_file]");
  puts("      Generate build system files.");
  puts("  schema2code <schema.json> <out_dir>");
  puts("      Generate C code from JSON schema.");
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses and handles the `from_openapi` command.
 *
 * Loads the provided OpenAPI specification and acts as a router to the
 * correct sub-command: `to_sdk`, `to_sdk_cli`, or `to_server`.
 * Generates C bindings, structs, and implementations.
 */
C_CDD_EXPORT cdd_c_error_t from_openapi_cli_main(int argc, char **argv) {
  cdd_from_openapi_config_t api_cfg;
  int i;

  memset(&api_cfg, 0, sizeof(api_cfg));
  api_cfg.subcommand = "to_sdk";
  api_cfg.input =
      getenv("CDD_INPUT") ? getenv("CDD_INPUT") : getenv("INPUT_FILE");
  api_cfg.input_dir =
      getenv("CDD_INPUT_DIR") ? getenv("CDD_INPUT_DIR") : getenv("INPUT_DIR");
  api_cfg.output =
      getenv("CDD_OUTPUT") ? getenv("CDD_OUTPUT") : getenv("OUT_DIR");

  if (argc > 1) {
    if (strcmp(argv[1], "to_sdk") == 0) {
      api_cfg.subcommand = "to_sdk";
      argc--;
      argv++;
    } else if (strcmp(argv[1], "to_sdk_cli") == 0) {
      api_cfg.subcommand = "to_sdk_cli";
      argc--;
      argv++;
    } else if (strcmp(argv[1], "to_server") == 0) {
      api_cfg.subcommand = "to_server";
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
      api_cfg.input = argv[++i];
    } else if (strcmp(argv[i], "--input-dir") == 0 && i + 1 < argc) {
      api_cfg.input_dir = argv[++i];
    } else if ((strcmp(argv[i], "-o") == 0 ||
                strcmp(argv[i], "--output") == 0) &&
               i + 1 < argc) {
      api_cfg.output = argv[++i];
    } else if (strcmp(argv[i], "--no-github-actions") == 0) {
      api_cfg.no_github_actions = 1;
    } else if (strcmp(argv[i], "--no-installable-package") == 0) {
      api_cfg.no_installable_package = 1;
    } else if (strcmp(argv[i], "--tests") == 0) {
      api_cfg.tests = 1;
    }
  }

  return cdd_generate_from_openapi(&api_cfg);
}

/**
 * @brief Handles the `to_openapi` command parsing C code to an OpenAPI
 * spec.
 *
 * Invokes the internal C-to-OpenAPI translation logic and writes the
 * result.
 */
C_CDD_EXPORT cdd_c_error_t to_openapi_cli_main(int argc, char **argv) {
  cdd_to_openapi_config_t api_cfg;
  int i;

  memset(&api_cfg, 0, sizeof(api_cfg));
  api_cfg.input =
      getenv("CDD_INPUT") ? getenv("CDD_INPUT") : getenv("INPUT_DIR");
  api_cfg.output =
      getenv("CDD_OUTPUT")
          ? getenv("CDD_OUTPUT")
          : (getenv("OUT_FILE") ? getenv("OUT_FILE") : "openapi.json");

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
      api_cfg.input = argv[++i];
    } else if ((strcmp(argv[i], "-o") == 0 ||
                strcmp(argv[i], "--output") == 0) &&
               i + 1 < argc) {
      api_cfg.output = argv[++i];
    }
  }

  return cdd_generate_to_openapi(&api_cfg);
}

/**
 * @brief Main entry point dispatcher execution logic.
 *
 * Implements the command routing and prints help or error outputs as needed.
 */
C_CDD_EXPORT cdd_c_error_t cdd_main(int argc, char **argv) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  const char *cmd;

  if (argc < 2) {
    print_help(argc > 0 ? argv[0] : "cdd-c");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  cmd = argv[1];

  if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0) {
    return print_version();
  }

  if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
    return print_help(argv[0]);
  }

  if (strcmp(cmd, "audit") == 0) {
    if (argc < 3)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = handle_audit(argc - 2, argv + 2);
    printf("handle_audit returned %d\n", rc);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "c2openapi") == 0) {
    rc = c2openapi_cli_main(argc - 1, argv + 1);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "transformer") == 0) {
    rc = cli_cst_transformer_main(argc - 2, argv + 2);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "standardize-gnu") == 0) {
    rc = cli_standardize_gnu_main(argc - 2, argv + 2);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "code2schema") == 0) {
    if (argc != 4)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = code2schema_main(argc - 2, argv + 2);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "from_openapi") == 0) {
    rc = from_openapi_cli_main(argc - 1, argv + 1);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "to_openapi") == 0) {
    rc = to_openapi_cli_main(argc - 1, argv + 1);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "to_docs_json") == 0) {
    rc = to_docs_json_cli_main(argc - 1, argv + 1);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "bind") == 0) {
    rc = generate_bindings_cli_main(argc - 1, argv + 1);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "generate_build_system") == 0) {
    rc = generate_build_system_main(argc - 2, argv + 2);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "schema2code") == 0) {
    rc = schema2code_main(argc - 2, argv + 2);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "serve_json_rpc") == 0) {
    rc = serve_json_rpc_main(argc - 1, argv + 1);
    if (rc != CDD_C_SUCCESS)
      goto handle_err;
    return CDD_C_SUCCESS;
  } else if (strcmp(cmd, "mcp") == 0) {
    /* Expose Generator via MCP stdio */
    /* Register Tools: cdd_generate (Code Scaffold), cdd_inspect (Schema
     * Inspection), cdd_sync (Bidirectional Sync) */
    printf("Starting MCP server for cdd generator via stdio...\n");
    return serve_mcp_stdio_main(argc - 1, argv + 1);
  } else {
    /* Fallback for other commands */
    if (strcmp(cmd, "openapi2client") == 0) {
      /* Keep previous behavior stub */
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    fprintf(stderr, "Error: Unknown or incomplete command: %s\n", cmd);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

handle_err:
  fprintf(stderr, "Error executing '%s': code %d\n", cmd, rc);
  return rc;
}
