#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "0.0.1"

extern int yyparse();
extern FILE *yyin;

void print_help() {
  printf("cdd-c CLI (Code-Driven Development)\n\n");
  printf("Usage: cdd-c <command> [options]\n\n");
  printf("Commands:\n");
  printf("  --help                            Show this help message\n");
  printf("  --version                         Show version information\n");
  printf("  to_openapi -f <path> -o <spec>    Parse code and generate OpenAPI "
         "spec\n");
  printf("  serve_json_rpc                    Start JSON-RPC server\n");
  printf("    --port <port>                   (default 8082)\n");
  printf("    --listen <ip>                   (default 0.0.0.0)\n");
  printf("  to_docs_json                      Generate docs.json from spec\n");
  printf("    -i <spec>                       Input OpenAPI spec\n");
  printf("    -o <docs.json>                  Output JSON file\n");
  printf("    --no-imports                    Omit imports from output\n");
  printf("    --no-wrapping                   Omit wrappers from output\n");
  printf("  from_openapi to_sdk_cli           Generate CLI SDK from OpenAPI "
         "spec\n");
  printf("    -i <spec> | --input-dir <dir>   Input spec or directory\n");
  printf("    -o <dir>                        Output directory (default: "
         "current dir)\n");
  printf(
      "    --no-github-actions             Do not generate GitHub Actions\n");
  printf("    --no-installable-package        Do not generate installable "
         "package\n");
  printf(
      "  from_openapi to_sdk               Generate SDK from OpenAPI spec\n");
  printf("    -i <spec> | --input-dir <dir>   Input spec or directory\n");
  printf("    -o <dir>                        Output directory (default: "
         "current dir)\n");
  printf("  from_openapi to_server            Generate server stub from "
         "OpenAPI spec\n");
  printf("    -i <spec> | --input-dir <dir>   Input spec or directory\n");
  printf("    -o <dir>                        Output directory (default: "
         "current dir)\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_help();
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
    print_help();
    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
    printf("%s\n", VERSION);
    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "to_openapi") == 0) {
    printf("to_openapi not yet implemented fully\n");
    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "serve_json_rpc") == 0) {
    int port = 8082;
    const char *listen_addr = "0.0.0.0";
    for (int i = 2; i < argc; i++) {
      if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
        port = atoi(argv[++i]);
      } else if (strcmp(argv[i], "--listen") == 0 && i + 1 < argc) {
        listen_addr = argv[++i];
      }
    }
    printf("Starting JSON-RPC server on %s:%d\n", listen_addr, port);
    // Stub for JSON RPC server
    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "to_docs_json") == 0) {
    printf("to_docs_json not yet implemented fully\n");
    return EXIT_SUCCESS;
  }

  if (strcmp(argv[1], "from_openapi") == 0) {
    if (argc >= 3) {
      if (strcmp(argv[2], "to_sdk_cli") == 0) {
        printf("from_openapi to_sdk_cli not yet implemented fully\n");
      } else if (strcmp(argv[2], "to_sdk") == 0) {
        printf("from_openapi to_sdk not yet implemented fully\n");
      } else if (strcmp(argv[2], "to_server") == 0) {
        printf("from_openapi to_server not yet implemented fully\n");
      } else {
        printf("Unknown from_openapi command: %s\n", argv[2]);
      }
    } else {
      printf("Missing sub-command for from_openapi\n");
    }
    return EXIT_SUCCESS;
  }

  // Default parser fallback for old CLI
  yyin = fopen(argv[1], "r");
  if (!yyin) {
    printf("couldn't open file for reading\n");
    return EXIT_FAILURE;
  }
  return yyparse();
}
