/**
 * @file main.c
 * @brief Main entry point for the c_cdd CLI.
 * Dispatches commands and handles logic for auditing and refactoring.
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analysis.h"
#include "code2schema.h"
#include "cst_parser.h"
#include "fs.h"
#include "generate_build_system.h"
#include "project_audit.h"
#include "refactor_orchestrator.h"
#include "rewriter_body.h"
#include "schema2tests.h"
#include "schema_codegen.h"
#include "sync_code.h"
#include "tokenizer.h"

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
    fprintf(
        stderr,
        "Usage: %s <command> [args]\n"
        "Commands:\n"
        "  code2schema <header.h> <schema.json>\n"
        "  fix <input.c> <output.c>\n"
        "  generate_build_system <build_system> <output_directory> <basename> "
        "[test_file]\n"
        "  jsonschema2tests <schema.json> <header_to_test.h> <output-test.h>\n"
        "  schema2code <schema.json> <basename>\n"
        "  sync_code <header.h> <impl.c>\n",
        argc > 0 ? argv[0] : "c_cdd_cli");
    return EXIT_FAILURE;
  }

  cmd = argv[1];

  if (strcmp(cmd, "generate_build_system") == 0) {
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
  } else if (strcmp(cmd, "fix") == 0) {
    if (argc != 4) {
      fprintf(stderr, "Usage: %s fix <input.c> <output.c>\n", argv[0]);
      return EXIT_FAILURE;
    }
    /* fix_code_main reads files from argv[0], argv[1].
       main passes argv+2 as args to subcommand? No, argv[2] and argv[3].
       Logic: argv[2] is input, argv[3] is output. */
    rc = fix_code_main(argc - 2, argv + 2);
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
