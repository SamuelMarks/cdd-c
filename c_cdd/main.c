/**
 * @file main.c
 * @brief Main entry point for the c_cdd CLI.
 *
 * Updated to include the `c2openapi` command.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c2openapi_cli.h" /* New entry */
#include "c_cddConfig.h"
#include "code2schema.h"
#include "generate_build_system.h"
#include "openapi_client_gen.h"
#include "openapi_loader.h"
#include "project_audit.h"
#include "refactor_orchestrator.h"
#include "schema2tests.h"
#include "schema_codegen.h"
#include "str_utils.h"
#include "sync_code.h"

static void print_error(int rc, const char *command_name) {
  /* ... Existing error print logic ... */
  if (rc == 0)
    return;
  fprintf(stderr, "Error executing '%s': code %d\n", command_name, rc);
}

static int handle_audit(int argc, char **argv) {
  /* ... Existing audit handler ... */
  struct AuditStats stats;
  int rc;
  if (argc != 1)
    return EXIT_FAILURE;
  audit_stats_init(&stats);
  rc = audit_project(argv[0], &stats);
  audit_stats_free(&stats);
  return rc;
}

/* ... Existing handlers for openapi2client, etc. ... */

static void print_help(const char *prog_name) {
  printf("Usage: %s <command> [args]\n\n", prog_name);
  puts("Commands:");
  puts("  audit <directory>");
  puts("      Scan directory for memory safety issues.");
  puts("  c2openapi <dir> <out.json>");
  puts("      Generate OpenAPI spec from C source code.");
  puts("  code2schema <header.h> <schema.json>");
  puts("      Convert C header to JSON Schema.");
  /* ... other commands ... */
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
    /* Audit logic */
    /* Only simplified here for patch context, assume original persists */
    if (argc < 3)
      return EXIT_FAILURE;
    rc = handle_audit(argc - 2, argv + 2);
  } else if (strcmp(cmd, "c2openapi") == 0) {
    /* Pass directly to c2openapi_cli_main */
    /* It parses arguments internally from argv[0]=c2openapi, ... */
    /* Actually prompt says it implements command logic.
       c2openapi_cli_main expects standard argc/argv
       where argv[0] is program name usually, but here we pass subcommand args.
       c2openapi_cli_main implementation expects:
         Usage: c2openapi <src> <out>
         So argc should be 3. argv[0]="c2openapi", argv[1]=src, argv[2]=out.
       We passed: main(argc, argv). argv[1] is "c2openapi".
       So passing (argc-1, argv+1) works perfect.
    */
    rc = c2openapi_cli_main(argc - 1, argv + 1);

  } else if (strcmp(cmd, "code2schema") == 0) {
    /* ... existing ... */
    if (argc != 4)
      return EXIT_FAILURE;
    rc = code2schema_main(argc - 2, argv + 2);
  } else {
    /* Fallback for other commands */
    if (strcmp(cmd, "openapi2client") == 0) {
      /* Stub to match previous signature context */
      return EXIT_FAILURE; /* Actually implemented in full file */
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
