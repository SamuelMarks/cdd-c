/**
 * @file main.c
 * @brief Main entry point for the c_cdd CLI.
 *
 * Updated to include the `openapi2client` command for generating API client
 * libraries.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cddConfig.h"
#include "code2schema.h"
#include "generate_build_system.h"
#include "openapi_client_gen.h"
#include "openapi_loader.h"
#include "project_audit.h"
#include "refactor_orchestrator.h"
#include "schema2tests.h"
#include "schema_codegen.h"
#include "sync_code.h"

static void print_error(int rc, const char *command_name) {
  if (rc == 0)
    return;

  fprintf(stderr, "Error executing '%s': ", command_name);
  switch (rc) {
  case ENOMEM:
    fputs("Out of memory.\n", stderr);
    break;
  case EINVAL:
    fputs("Invalid arguments.\n", stderr);
    break;
  case ENOENT:
    fputs("File or directory not found.\n", stderr);
    break;
  case EACCES:
    fputs("Permission denied.\n", stderr);
    break;
  case EIO:
    fputs("I/O error.\n", stderr);
    break;
  case EXIT_FAILURE:
    fputs("General failure.\n", stderr);
    break;
  default:
    fprintf(stderr, "Unknown error code %d (%s).\n", rc, strerror(rc));
    break;
  }
}

static int handle_audit(int argc, char **argv) {
  struct AuditStats stats;
  char *json = NULL;
  int rc;

  if (argc != 1) {
    fprintf(stderr, "Usage: audit <directory>\n");
    return EXIT_FAILURE;
  }

  audit_stats_init(&stats);
  rc = audit_project(argv[0], &stats);

  if (rc != 0) {
    audit_stats_free(&stats);
    return rc;
  }

  json = audit_print_json(&stats);
  if (json) {
    puts(json);
    free(json);
  } else {
    rc = ENOMEM;
  }

  audit_stats_free(&stats);
  return rc;
}

static int handle_openapi2client(int argc, char **argv) {
  const char *spec_file;
  const char *out_base;
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  JSON_Value *root_val;
  int rc;

  if (argc != 2) {
    fprintf(stderr, "Usage: openapi2client <spec.json> <out_basename>\n");
    return EXIT_FAILURE;
  }

  spec_file = argv[0];
  out_base = argv[1];

  /* 1. Load Spec */
  root_val = json_parse_file(spec_file);
  if (!root_val) {
    fprintf(stderr, "Failed to parse spec file: %s\n", spec_file);
    return EXIT_FAILURE;
  }

  openapi_spec_init(&spec);
  rc = openapi_load_from_json(root_val, &spec);
  json_value_free(root_val);

  if (rc != 0) {
    return rc;
  }

  /* 2. Configure Generator */
  memset(&config, 0, sizeof(config));
  config.filename_base = out_base;
  config.func_prefix = "api_"; /* Default prefix */

  /* 3. Execute */
  rc = openapi_client_generate(&spec, &config);

  openapi_spec_free(&spec);
  return rc;
}

static void print_help(const char *prog_name) {
  printf("Usage: %s <command> [args]\n\n", prog_name);
  puts("Commands:");
  puts("  audit <directory>");
  puts("      Scan directory for memory safety issues.");
  puts("  code2schema <header.h> <schema.json>");
  puts("      Convert C header to JSON Schema.");
  puts("  fix <path> [--in-place]");
  puts("      Refactor C file(s) for safety.");
  puts("  generate_build_system <sys> <dir> <name> [test]");
  puts("      Scaffold CMakeLists.txt.");
  puts("  jsonschema2tests <schema> <hdr> <out>");
  puts("      Generate tests from schema.");
  puts("  openapi2client <spec.json> <out_base>");
  puts("      Generate C API client from OpenAPI spec.");
  puts("  schema2code <schema> <base> [opts]");
  puts("      Generate C structs/enums from schema.");
  puts("  sync_code <hdr> <impl>");
  puts("      Sync implementation with header.");
  puts("\nOptions:");
  puts("  --version   Print version.");
  puts("  --help      Print this help.");
}

static void print_version(void) {
  printf("c_cdd_cli version %s\n", C_CDD_VERSION);
}

int main(int argc, char **argv) {
  int rc = 0;
  const char *cmd;

  if (argc < 2) {
    print_help(argc > 0 ? argv[0] : "c_cdd_cli");
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
    if (argc < 3) {
      fprintf(stderr, "Usage: %s audit <directory>\n", argv[0]);
      return EXIT_FAILURE;
    }
    rc = handle_audit(argc - 2, argv + 2);

  } else if (strcmp(cmd, "fix") == 0) {
    if (argc < 3) {
      fprintf(stderr, "Usage: %s fix <args...>\n", argv[0]);
      return EXIT_FAILURE;
    }
    rc = fix_code_main(argc - 2, argv + 2);

  } else if (strcmp(cmd, "generate_build_system") == 0) {
    rc = generate_build_system_main(argc - 2, argv + 2);

  } else if (strcmp(cmd, "code2schema") == 0) {
    if (argc != 4) {
      fprintf(stderr, "Usage: %s code2schema <header.h> <schema.json>\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    rc = code2schema_main(argc - 2, argv + 2);

  } else if (strcmp(cmd, "jsonschema2tests") == 0) {
    if (argc != 5) {
      fprintf(stderr,
              "Usage: %s jsonschema2tests <schema.json> <header.h> <out.h>\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    rc = jsonschema2tests_main(argc - 2, argv + 2);

  } else if (strcmp(cmd, "schema2code") == 0) {
    if (argc < 4) {
      fprintf(stderr,
              "Usage: %s schema2code <schema.json> <basename> [options]\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    rc = schema2code_main(argc - 2, argv + 2);

  } else if (strcmp(cmd, "sync_code") == 0) {
    if (argc != 4) {
      fprintf(stderr, "Usage: %s sync_code <header.h> <impl.c>\n", argv[0]);
      return EXIT_FAILURE;
    }
    rc = sync_code_main(argc - 2, argv + 2);

  } else if (strcmp(cmd, "openapi2client") == 0) {
    rc = handle_openapi2client(argc - 2, argv + 2);

  } else {
    fprintf(stderr, "Unknown command: %s\n", cmd);
    return EXIT_FAILURE;
  }

  if (rc != 0) {
    print_error(rc, cmd);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
