#ifndef TEST_CDD_TRANSFORM_SAFE_CRT_H
#define TEST_CDD_TRANSFORM_SAFE_CRT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/emit/cdd_cst_emit.h"
#include "c_str_span.h"
/* clang-format on */

TEST test_cdd_transform_safe_crt(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void foo() {\n"
                     "  char dest[10];\n"
                     "  strcpy(dest, \"abc\");\n"
                     "  strncpy(dest, \"def\", 3);\n"
                     "  sprintf(dest, \"%d\", 1);\n"
                     "  FILE *f;\n"
                     "  f = fopen(\"a.txt\", \"r\");\n"
                     "  strcat(dest, \"x\");\n"
                     "  strncat(dest, \"y\", 1);\n"
                     "  memcpy(dest, \"z\", 1);\n"
                     "  memmove(dest, \"w\", 1);\n"
                     "}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_safe_crt(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "strcpy_s(dest, sizeof(dest), \"abc\");") != NULL);
  ASSERT(strstr(out, "strncpy_s(dest, 3 + 1, \"def\", 3);") != NULL);
  ASSERT(strstr(out, "sprintf_s(dest, sizeof(dest), \"%d\", 1);") != NULL);
  ASSERT(strstr(out, "fopen_s(&f") != NULL);
  ASSERT(strstr(out, "strcat_s(dest, sizeof(dest), \"x\");") != NULL);
  ASSERT(strstr(out, "strncat_s(dest, 1 + 1, \"y\", 1);") != NULL);
  ASSERT(strstr(out, "memcpy_s(dest, 1, \"z\", 1);") != NULL);
  ASSERT(strstr(out, "memmove_s(dest, 1, \"w\", 1);") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

SUITE(transformer_safe_crt_suite) { RUN_TEST(test_cdd_transform_safe_crt); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_SAFE_CRT_H */
