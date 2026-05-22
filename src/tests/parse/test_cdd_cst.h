/**
 * @file test_cdd_cst.h
 * @brief Unit tests for CST parsing and emitting roundtrips.
 */

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

/**
 * @brief Tests basic roundtrip of CST parsing and emitting.
 *
 * @return The result of the test.
 */
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

/**
 * @brief Tests roundtrip of macros with CST.
 *
 * @return The result of the test.
 */
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

/**
 * @brief Tests CST parsing of asm statements.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_asm_statement(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void func() {\n"
                     "  __asm__ volatile (\"nop\" : : : \"memory\");\n"
                     "}";
  char *out = NULL;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);

  ASSERT_EQ(0, rc);
  ASSERT(tree != NULL);
  ASSERT(tree->root != NULL);

  /* Validate that CDD_CST_ASM_STATEMENT is present */
  {
    cdd_cst_node_t *func = tree->root->children[0].val.node;
    cdd_cst_node_t *block = func->children[func->num_children - 1].val.node;
    cdd_cst_node_t *asm_stmt = NULL;
    size_t i;
    for (i = 0; i < block->num_children; i++) {
      if (block->children[i].kind == CDD_CST_CHILD_NODE &&
          block->children[i].val.node->kind == CDD_CST_ASM_STATEMENT) {
        asm_stmt = block->children[i].val.node;
        break;
      }
    }
    ASSERT(asm_stmt != NULL);
  }

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);
  ASSERT(out != NULL);

  ASSERT_STR_EQ(code, out);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief CST test suite.
 */
SUITE(cdd_cst_suite) {
  RUN_TEST(test_cdd_cst_roundtrip_basic);
  RUN_TEST(test_cdd_cst_roundtrip_macros);
  RUN_TEST(test_cdd_cst_asm_statement);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_PARSER_H */
