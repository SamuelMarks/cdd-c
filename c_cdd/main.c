/**
 * @file main.c
 * @brief Main entry point for the c_cdd CLI.
 * Dispatches commands and handles logic for auditing, refactoring, and code
 * generation.
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cddConfig.h"
#include "code2schema.h"
#include "generate_build_system.h"
#include "project_audit.h"
#include "refactor_orchestrator.h"
#include "schema2tests.h"
#include "schema_codegen.h"
#include "sync_code.h"

/**
 * @brief Helper to print human-readable error message based on error code.
 *
 * @param[in] rc The error code returned by a command function.
 * @param[in] command_name The name of the command that failed.
 */
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

/**
 * @brief Handler for the audit command.
 * Finds all .c files in the directory, checks for safe allocation usage,
 * and prints a JSON report.
 *
 * @param[in] argc Argument count (should be 1).
 * @param[in] argv Argument vector (argv[0] is directory).
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
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

/**
 * @brief Implementation of --help output.
 */
static void print_help(const char *prog_name) {
  printf("Usage: %s <command> [args]\n"
         "\n"
         "Commands:\n"
         "  audit <directory>\n"
         "      Scan a directory for memory safety issues and output a JSON "
         "report.\n"
         "  code2schema <header.h> <schema.json>\n"
         "      Convert C header structs/enums to JSON Schema.\n"
         "  fix <path> [--in-place] OR fix <input.c> <output.c>\n"
         "      Refactor C file(s) to inject error handling. Supports "
         "directories.\n"
         "  generate_build_system <build_system> <output_directory> <basename> "
         "[test_file]\n"
         "      Generate CMake files.\n"
         "  jsonschema2tests <schema.json> <header_to_test.h> <output-test.h>\n"
         "      Generate test suite from schema.\n"
         "  schema2code <schema.json> <basename>\n"
         "      Generate C implementation from JSON Schema.\n"
         "  sync_code <header.h> <impl.c>\n"
         "      Sync implementation file with header declarations.\n"
         "\n"
         "Options:\n"
         "  --version   Print version information.\n"
         "  --help      Print this help message.\n",
         prog_name);
}

/**
 * @brief Implementation of --version output.
 */
static void print_version(void) {
  printf("c_cdd_cli version %s\n", C_CDD_VERSION);
}

/**
 * @brief Main CLI dispatcher.
 * Parses arguments and invokes the subcommand.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int main(int argc, char **argv) {
  int rc = 0;
  const char *cmd;

  if (argc < 2) {
    print_help(argc > 0 ? argv[0] : "c_cdd_cli");
    return EXIT_FAILURE;
  }

  cmd = argv[1];

  /* Global Flags */
  if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0) {
    print_version();
    return EXIT_SUCCESS;
  }

  if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
    print_help(argv[0]);
    return EXIT_SUCCESS;
  }

  /* Subcommands */
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
    if (argc < 5 || argc > 6) {
      fprintf(stderr,
              "Usage: %s generate_build_system <build_system> "
              "<output_directory> <basename> "
              "[test_file]\n",
              argv[0]);
      return EXIT_FAILURE;
    }
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
              "Usage: %s jsonschema2tests <schema.json> <header_to_test.h> "
              "<output-test.h>\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    rc = jsonschema2tests_main(argc - 2, argv + 2);
  } else if (strcmp(cmd, "schema2code") == 0) {
    if (argc != 4) {
      fprintf(stderr, "Usage: %s schema2code <schema.json> <basename>\n",
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
