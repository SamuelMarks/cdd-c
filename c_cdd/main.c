#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code2schema.h"
#include "schema_codegen.h"
#include "sync_code.h"

/* main CLI dispatcher */
int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr,
            "Usage: %s <command> [args]\n"
            "Commands:\n"
            "  schema2code <schema.json> <basename>\n"
            "  code2schema <header.h> <schema.json>\n"
            "  sync_code <header.h> <impl.c>\n",
            argc > 0 ? argv[0] : "c_cdd_cli");
    return EXIT_FAILURE;
  } else if (strcmp(argv[1], "schema2code") == 0) {
    if (argc != 4) {
      fprintf(stderr, "Usage: %s schema2code <schema.json> <basename>\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    return schema2code_main(argc - 2, argv + 2);
  } else if (strcmp(argv[1], "code2schema") == 0) {
    /* code2schema expects exactly 2 arguments */
    if (argc != 4) {
      fprintf(stderr, "Usage: %s code2schema <header.h> <schema.json>\n",
              argv[0]);
      return EXIT_FAILURE;
    }
    return code2schema_main(argc - 2, argv + 2);
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
            "Available commands: schema2code, code2schema, sync_code\n");
    return EXIT_FAILURE;
  }
}
