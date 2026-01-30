/**
 * @file main.c
 * @brief Main entry point for the c_cdd CLI.
 * Dispatches commands and handles error reporting based on return codes.
 * Contains the integration logic for allocator analysis and refactoring.
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
 * @brief Write a string to a file.
 * Helper function for CLI output generation.
 *
 * @param[in] filename Path to output file.
 * @param[in] content Null-terminated string content.
 * @return 0 on success, EIO/errno on failure.
 */
static int write_file_content(const char *filename, const char *content) {
  FILE *f = NULL;
  int rc = 0;

  if (!filename || !content)
    return EINVAL;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&f, filename, "w, ccs=UTF-8");
    if (err != 0 || f == NULL)
      return (int)err;
  }
#else
  f = fopen(filename, "w");
  if (!f)
    return errno ? errno : EIO;
#endif

  if (fputs(content, f) < 0)
    rc = EIO;

  if (fclose(f) != 0 && rc == 0)
    rc = errno ? errno : EIO;

  return rc;
}

/**
 * @brief Implementation of the refactor_safe_alloc command.
 * Pipeline: Read -> Tokenize -> Analyze Allocations -> Rewrite Body -> Write.
 *
 * @param[in] argc Argument count (expected 2: input, output).
 * @param[in] argv Argument vector.
 * @return 0 on success, non-zero error code on failure.
 */
static int refactor_safe_alloc_main(int argc, char **argv) {
  const char *input_file;
  const char *output_file;
  char *file_content = NULL;
  size_t file_size = 0;
  struct TokenList *tokens = NULL;
  struct AllocationSiteList sites = {0};
  char *rewritten_code = NULL;
  int rc;

  if (argc != 2) {
    fprintf(stderr,
            "Usage: refactor_safe_alloc <input_file.c> <output_file.c>\n");
    return EXIT_FAILURE;
  }

  input_file = argv[0];
  output_file = argv[1];

  /* 1. Read Input */
  rc = read_to_file(input_file, "r", &file_content, &file_size);
  if (rc != 0) {
    fprintf(stderr, "Failed to read input file: %s\n", input_file);
    return rc;
  }

  /* 2. Tokenize */
  /* Cast to generic char* span, size logic handles bounds */
  rc = tokenize(az_span_create_from_str(file_content), &tokens);
  if (rc != 0) {
    fprintf(stderr, "Failed to tokenize input.\n");
    free(file_content);
    return rc;
  }

  /* 3. Analyze Allocations */
  rc = find_allocations(tokens, &sites);
  if (rc != 0) {
    fprintf(stderr, "Analysis failed.\n");
    free_token_list(tokens);
    free(file_content);
    return rc;
  }

  /* 4. Rewrite Body (Inject Checks) */
  /* finding allocations works on the whole file stream, so we rewrite the whole
   * file */
  rc = rewrite_body(tokens, &sites, NULL, 0, &rewritten_code);
  if (rc != 0) {
    fprintf(stderr, "Rewriting failed.\n");
    allocation_site_list_free(&sites);
    free_token_list(tokens);
    free(file_content);
    return rc;
  }

  /* 5. Write Output */
  rc = write_file_content(output_file, rewritten_code);
  if (rc != 0) {
    fprintf(stderr, "Failed to write output file: %s\n", output_file);
  } else {
    printf("Refactored code written to %s\n", output_file);
  }

  /* Cleanup */
  free(rewritten_code);
  allocation_site_list_free(&sites);
  free_token_list(tokens);
  free(file_content);

  return rc;
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
        "  generate_build_system <build_system> <output_directory> <basename> "
        "[test_file]\n"
        "  jsonschema2tests <schema.json> <header_to_test.h> <output-test.h>\n"
        "  refactor_safe_alloc <input.c> <output.c>\n"
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
  } else if (strcmp(cmd, "refactor_safe_alloc") == 0) {
    if (argc != 4) {
      fprintf(stderr, "Usage: %s refactor_safe_alloc <input.c> <output.c>\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    rc = refactor_safe_alloc_main(argc - 2, argv + 2);
  } else {
    fprintf(stderr, "Unknown command: %s\n", cmd);
    fputs("Available commands: code2schema, generate_build_system, "
          "jsonschema2tests, refactor_safe_alloc, schema2code, sync_code\n",
          stderr);
    return EXIT_FAILURE;
  }

  if (rc != 0) {
    print_error(rc, cmd);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
