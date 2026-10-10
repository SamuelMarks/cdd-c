/* clang-format off */
#include "c_str_span.h"
#include "cdd_cst_transform.h"
#include "classes/emit/cdd_cst_emit.h"
#include "classes/parse/cdd_cst_parser.h"
#include <stdio.h>
/* clang-format on */
int main(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config;
  cdd_c_error_t rc;
  char *out = NULL;
  memset(&config, 0, sizeof(config));
  cdd_cst_parse(az_span_create_from_str(
                    "void f() { char *m = calloc(1, 20); strcpy(m, \"a\"); }"),
                &tree);
  rc = cdd_transform_safe_crt(tree, &config);
  printf("rc = %d\n", rc);
  cdd_cst_emit(tree, &out);
  printf("out: %s\n", out);
  free(out);
  cdd_cst_tree_free(tree);
  return 0;
}
