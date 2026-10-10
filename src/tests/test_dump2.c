/* clang-format off */
#include "c_str_span.h"
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include <stdio.h>
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;

int main(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config;
  cdd_c_error_t rc;
  memset(&config, 0, sizeof(config));

  cdd_cst_parse(az_span_create_from_str(
                    "void f() { char *m = calloc(1, 20); strcpy(m, \"a\"); }"),
                &tree);
  g_cdd_cst_alloc_token_fail = 1;
  rc = cdd_transform_safe_crt(tree, &config);
  printf("rc = %d, token_fail_after = %d\n", rc, g_cdd_cst_alloc_token_fail);
  cdd_cst_tree_free(tree);
  return 0;
}
