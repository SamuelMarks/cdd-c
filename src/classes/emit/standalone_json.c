#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/json.h"
#include "classes/emit/struct.h"

int write_struct_from_json_standalone_func(FILE *fp, const char *struct_name,
                                           const struct StructFields *sf) {
  size_t i;
  fprintf(fp,
          "int cdd_c_parse_oauth2_token(const char *json, struct %s **const "
          "out) {\n",
          struct_name);
  fprintf(fp, "  const char *p = json;\n");
  fprintf(fp, "  struct %s *ret = calloc(1, sizeof(*ret));\n", struct_name);
  fprintf(fp, "  if (!ret) return ENOMEM;\n");
  fprintf(fp, "  if (!json || !out) { free(ret); return EINVAL; }\n");
  fprintf(fp, "  while (*p) {\n");

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    fprintf(fp, "    if (strncmp(p, \"\\\"%s\\\"\", %d) == 0) {\n", n,
            (int)strlen(n) + 2);
    fprintf(fp, "      p += %d;\n", (int)strlen(n) + 2);
    fprintf(fp, "      while (*p == ' ' || *p == ':') p++;\n");
    if (strcmp(t, "string") == 0) {
      fprintf(fp, "      if (*p == '\"') {\n");
      fprintf(fp, "        const char *start = ++p;\n");
      fprintf(fp, "        while (*p && *p != '\"' && *p != '\\\\') p++;\n");
      fprintf(fp, "        if (*p != '\\\\') {\n");
      fprintf(fp, "          size_t len = p - start;\n");
      fprintf(fp, "          ret->%s = malloc(len + 1);\n", n);
      fprintf(fp, "          if (ret->%s) {\n", n);
      fprintf(fp, "            memcpy((char*)ret->%s, start, len);\n", n);
      fprintf(fp, "            ((char*)ret->%s)[len] = '\\0';\n", n);
      fprintf(fp, "          }\n");
      fprintf(fp, "          if (*p == '\"') p++;\n");
      fprintf(fp, "        }\n");
      fprintf(fp, "      }\n");
    } else if (strcmp(t, "integer") == 0) {
      fprintf(fp, "      ret->%s = atoi(p);\n", n);
      fprintf(fp, "      while ((*p >= '0' && *p <= '9') || *p == '-') p++;\n");
    }
    fprintf(fp, "      continue;\n");
    fprintf(fp, "    }\n");
  }

  fprintf(fp, "    p++;\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  *out = ret;\n");
  fprintf(fp, "  return 0;\n");
  fprintf(fp, "}\n");
  return 0;
}
