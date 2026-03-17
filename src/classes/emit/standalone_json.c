#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/json.h"
#include "classes/emit/struct.h"

int write_struct_from_json_standalone_func(FILE *fp, const char *struct_name,
                                           const struct StructFields *sf) {
  (void)sf;
  fprintf(fp,
          "int cdd_c_parse_oauth2_token(const char *json, struct %s **const "
          "out) {\n",
          struct_name);
  fprintf(fp, "  return %s_from_json(json, out);\n", struct_name);
  fprintf(fp, "}\n");
  return 0;
}
