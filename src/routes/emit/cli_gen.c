#include "cli_gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int openapi_cli_generate(const struct OpenAPI_Spec *spec,
                         const struct OpenApiClientConfig *config) {
  char path[1024];
  FILE *fp;
  size_t i, j;

  snprintf(path, sizeof(path), "%s_cli.c", config->filename_base);
  fp = fopen(path, "w");
  if (!fp)
    return -1;

  fprintf(fp, "#include <stdio.h>\n");
  fprintf(fp, "#include <stdlib.h>\n");
  fprintf(fp, "#include <string.h>\n");
  fprintf(fp, "#include \"%s.h\"\n\n", config->filename_base);

  fprintf(fp, "void print_cli_help(void) {\n");
  fprintf(fp, "  printf(\"Usage: cli <command> [args]\\n\\nCommands:\\n\");\n");
  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const char *opId = spec->paths[i].operations[j].operation_id;
      const char *desc = spec->paths[i].operations[j].summary;
      if (opId) {
        fprintf(fp, "  printf(\"  %%s\\t%%s\\n\", \"%s\", \"%s\");\n", opId,
                desc ? desc : "");
      }
    }
  }
  fprintf(fp, "}\n\n");

  fprintf(fp, "int main(int argc, char **argv) {\n");
  fprintf(fp, "  if (argc < 2 || strcmp(argv[1], \"--help\") == 0) {\n");
  fprintf(fp, "    print_cli_help();\n");
  fprintf(fp, "    return 0;\n");
  fprintf(fp, "  }\n");

  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const char *opId = spec->paths[i].operations[j].operation_id;
      if (opId) {
        fprintf(fp, "  if (strcmp(argv[1], \"%s\") == 0) {\n", opId);
        fprintf(fp, "    printf(\"Calling %s...\\n\");\n", opId);
        fprintf(fp, "    /* api_%s(...); */\n", opId);
        fprintf(fp, "    return 0;\n");
        fprintf(fp, "  }\n");
      }
    }
  }
  fprintf(fp, "  printf(\"Unknown command: %%s\\n\", argv[1]);\n");
  fprintf(fp, "  return 1;\n");
  fprintf(fp, "}\n");
  fclose(fp);
  return 0;
}
