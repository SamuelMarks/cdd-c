#ifndef TEST_CDD_CST_PARSER_H
#define TEST_CDD_CST_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "classes/parse/cdd_cst_parser.h"
#include "classes/emit/cdd_cst_emit.h"
/* clang-format on */

TEST test_cdd_cst_roundtrip_basic(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#include <stdio.h>\n"
                     "/* Comment */\n"
                     "int main() {\n"
                     "  #ifdef _WIN32\n"
                     "    return 1;\n"
                     "  #else\n"
                     "    return 0;\n"
                     "  #endif\n"
                     "}\n"
                     "// end";
  char *out = NULL;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);

  ASSERT_EQ(0, rc);
  ASSERT(tree != NULL);
  ASSERT(tree->root != NULL);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);
  ASSERT(out != NULL);

  ASSERT_STR_EQ(code, out);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_cst_roundtrip_macros(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#define MACRO(x) (x + 1)\n"
                     "int var = MACRO(5);";
  char *out = NULL;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);

  ASSERT_EQ(0, rc);
  ASSERT(tree != NULL);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ(code, out);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

SUITE(cdd_cst_suite) {
  RUN_TEST(test_cdd_cst_roundtrip_basic);
  RUN_TEST(test_cdd_cst_roundtrip_macros);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_PARSER_H */
