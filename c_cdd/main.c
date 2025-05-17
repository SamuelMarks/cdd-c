#include <stdio.h>

#include "code2schema.h"
#include "generate_build_system.h"
#include "schema2tests.h"
#include "schema_codegen.h"
#include "sync_code.h"

/* main CLI dispatcher */
int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr,
            "Usage: %s <command> [args]\n"
            "Commands:\n"
            "  code2schema <header.h> <schema.json>\n"
            "  generate_build_system <build_system> <basename> [test_file]\n"
            "  jsonschema2tests <schema.json> <test.c>\n"
            "  schema2code <schema.json> <basename>\n"
            "  sync_code <header.h> <impl.c>\n",
            argc > 0 ? argv[0] : "c_cdd_cli");
    return EXIT_FAILURE;
  } else if (strcmp(argv[1], "generate_build_system") == 0) {
    if (argc < 4 || argc > 5) {
      fprintf(stderr,
              "Usage: %s generate_build_system <build_system> <basename> "
              "[test_file]\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    return generate_build_system_main(argc - 2, argv + 2);
  } else if (strcmp(argv[1], "code2schema") == 0) {
    /* code2schema expects exactly 2 arguments */
    if (argc != 4) {
      fprintf(stderr, "Usage: %s code2schema <header.h> <schema.json>\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    return code2schema_main(argc - 2, argv + 2);
  } else if (strcmp(argv[1], "jsonschema2tests") == 0) {
    if (argc != 4) {
      fprintf(stderr, "Usage: %s jsonschema2tests <schema.json> <test.c>\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    return jsonschema2tests_main(argc - 2, argv + 2);
  } else if (strcmp(argv[1], "schema2code") == 0) {
    if (argc != 4) {
      fprintf(stderr, "Usage: %s schema2code <schema.json> <basename>\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    return schema2code_main(argc - 2, argv + 2);
  } else if (strcmp(argv[1], "sync_code") == 0) {
    /* sync_code expects exactly 2 arguments */
    if (argc != 4) {
      fprintf(stderr, "Usage: %s sync_code <header.h> <impl.c>\n", argv[0]);
      return EXIT_FAILURE;
    }
    return sync_code_main(argc - 2, argv + 2);
  } else {
    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    fprintf(stderr,
            "Available commands: code2schema, schema2code, sync_code\n");
    return EXIT_FAILURE;
  }
}
