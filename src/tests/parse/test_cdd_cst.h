extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
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
#include "c_cdd_export.h"
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
  g_fail_io_after = -1;
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
  g_fail_io_after = -1;
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
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief CST test suite.
 */
/**
 * @brief test_cdd_cst_cpp_class
 * @return TEST
 */
TEST test_cdd_cst_cpp_class(void) {
  cdd_cst_tree_t *tree = NULL;
  int rc =
      cdd_cst_parse(az_span_create_from_str(
                        "class MyClass { public: int x; private: int y; };"),
                    &tree);
  ASSERT_EQ(0, rc);
  ASSERT(tree != NULL);
  ASSERT_EQ(CDD_CST_TRANSLATION_UNIT, tree->root->kind);
  ASSERT_EQ(1, tree->root->num_children); /* Class */
  ASSERT_EQ(CDD_CST_CHILD_NODE, tree->root->children[0].kind);
  ASSERT_EQ(CDD_CST_CLASS_DECLARATION, tree->root->children[0].val.node->kind);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test_cdd_cst_cpp_methods
 * @return TEST
 */
TEST test_cdd_cst_cpp_methods(void) {
  cdd_cst_tree_t *tree = NULL;
  int rc = cdd_cst_parse(
      az_span_create_from_str("class MyClass { \n"
                              "  MyClass() {} \n"
                              "  ~MyClass() {} \n"
                              "  MyClass& operator=(const MyClass& o) {} \n"
                              "};"),
      &tree);
  ASSERT_EQ(0, rc);
  ASSERT(tree != NULL);
  ASSERT_EQ(CDD_CST_TRANSLATION_UNIT, tree->root->kind);
  ASSERT_EQ(1, tree->root->num_children); /* Class */
  {
    cdd_cst_node_t *cls = tree->root->children[0].val.node;
    cdd_cst_node_t *blk = NULL;
    int has_ctor = 0, has_dtor = 0, has_op = 0;
    size_t i;
    for (i = 0; i < cls->num_children; i++) {
      if (cls->children[i].kind == CDD_CST_CHILD_NODE &&
          cls->children[i].val.node->kind == CDD_CST_BLOCK) {
        blk = cls->children[i].val.node;
        break;
      }
    }
    ASSERT(blk != NULL);
    for (i = 0; i < blk->num_children; i++) {
      if (blk->children[i].kind == CDD_CST_CHILD_NODE) {
        enum cdd_cst_node_kind_t k = blk->children[i].val.node->kind;
        printf("Found node kind %d in block\n", k);
        if (k == CDD_CST_CONSTRUCTOR)
          has_ctor = 1;
        if (k == CDD_CST_DESTRUCTOR)
          has_dtor = 1;
        if (k == CDD_CST_OPERATOR_OVERLOAD)
          has_op = 1;
      }
    }
    ASSERT(has_ctor);
    ASSERT(has_dtor);
    ASSERT(has_op);
  }

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_cpp_exceptions(void) {
  const char *code = "void func() noexcept {\n"
                     "  try {\n"
                     "    throw 1;\n"
                     "  } catch (int e) {\n"
                     "    throw;\n"
                     "  }\n"
                     "}\n";
  cdd_cst_tree_t *tree = NULL;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);
  ASSERT(tree != NULL);
  ASSERT_EQ(CDD_CST_TRANSLATION_UNIT, tree->root->kind);
  ASSERT_EQ(1, tree->root->num_children); /* Function definition */

  {
    cdd_cst_node_t *func = tree->root->children[0].val.node;
    int has_noexcept = 0;
    int has_try = 0;
    size_t i, j, k;

    for (i = 0; i < func->num_children; i++) {
      if (func->children[i].kind == CDD_CST_CHILD_NODE) {
        cdd_cst_node_t *child = func->children[i].val.node;
        if (child->kind == CDD_CST_NOEXCEPT_SPECIFIER)
          has_noexcept = 1;
        if (child->kind == CDD_CST_BLOCK) {
          for (j = 0; j < child->num_children; j++) {
            if (child->children[j].kind == CDD_CST_CHILD_NODE) {
              cdd_cst_node_t *stmt = child->children[j].val.node;
              if (stmt->kind == CDD_CST_TRY_BLOCK) {
                int has_throw = 0;
                int has_catch = 0;
                has_try = 1;
                for (k = 0; k < stmt->num_children; k++) {
                  if (stmt->children[k].kind == CDD_CST_CHILD_NODE) {
                    cdd_cst_node_t *try_child = stmt->children[k].val.node;
                    if (try_child->kind == CDD_CST_BLOCK) {
                      /* Inside try block, there should be a throw */
                      size_t m;
                      for (m = 0; m < try_child->num_children; m++) {
                        if (try_child->children[m].kind == CDD_CST_CHILD_NODE &&
                            try_child->children[m].val.node->kind ==
                                CDD_CST_THROW_EXPRESSION) {
                          has_throw = 1;
                        }
                      }
                    } else if (try_child->kind == CDD_CST_CATCH_BLOCK) {
                      has_catch = 1;
                    }
                  }
                }
                ASSERT(has_throw);
                ASSERT(has_catch);
              }
            }
          }
        }
      }
    }
    ASSERT(has_noexcept);
    ASSERT(has_try);
  }

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_cpp_namespace(void) {
  const char *code = "using namespace std;\n"
                     "namespace MyLib {\n"
                     "  class A {};\n"
                     "}\n";
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT(tree != NULL);
  ASSERT(tree->root != NULL);

  /* Check root children: USING_DIRECTIVE then NAMESPACE_DECLARATION */
  {
    cdd_cst_node_t *root = tree->root;
    size_t i;
    int using_found = 0;
    int ns_found = 0;
    for (i = 0; i < root->num_children; i++) {
      if (root->children[i].kind == CDD_CST_CHILD_NODE) {
        if (root->children[i].val.node->kind == CDD_CST_USING_DIRECTIVE) {
          using_found = 1;
        } else if (root->children[i].val.node->kind ==
                   CDD_CST_NAMESPACE_DECLARATION) {
          ns_found = 1;
        }
      }
    }
    ASSERT(using_found);
    ASSERT(ns_found);
  }

  {
    ASSERT_EQ(0, cdd_cst_emit(tree, &out));
    ASSERT(out != NULL);
    ASSERT_STR_EQ(code, out);
    free(out);
  }

  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_cst_cpp_template(void) {
  const char *code = "template <typename T, class U>\n"
                     "class Pair {\n"
                     "  T first;\n"
                     "  U second;\n"
                     "};\n";
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT(tree != NULL);
  ASSERT(tree->root != NULL);

  /* Check root children: TEMPLATE_DECLARATION */
  {
    cdd_cst_node_t *root = tree->root;
    size_t i;
    int template_found = 0;
    for (i = 0; i < root->num_children; i++) {
      if (root->children[i].kind == CDD_CST_CHILD_NODE) {
        if (root->children[i].val.node->kind == CDD_CST_TEMPLATE_DECLARATION) {
          template_found = 1;
        }
      }
    }
    ASSERT(template_found);
  }

  {
    ASSERT_EQ(0, cdd_cst_emit(tree, &out));
    ASSERT(out != NULL);
    ASSERT_STR_EQ(code, out);
    free(out);
  }

  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_cst_cpp_inheritance(void) {
  const char *code = "class C : public A, virtual private B {\n"
                     "};\n";
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT(tree != NULL);
  ASSERT(tree->root != NULL);

  /* Check root children: CLASS_DECLARATION */
  {
    cdd_cst_node_t *root = tree->root;
    cdd_cst_node_t *class_node = NULL;
    size_t i;
    for (i = 0; i < root->num_children; i++) {
      if (root->children[i].kind == CDD_CST_CHILD_NODE &&
          root->children[i].val.node->kind == CDD_CST_CLASS_DECLARATION) {
        class_node = root->children[i].val.node;
        break;
      }
    }
    ASSERT(class_node != NULL);

    {
      cdd_cst_node_t *base_list = NULL;
      for (i = 0; i < class_node->num_children; i++) {
        if (class_node->children[i].kind == CDD_CST_CHILD_NODE &&
            class_node->children[i].val.node->kind == CDD_CST_BASE_CLASS_LIST) {
          base_list = class_node->children[i].val.node;
          break;
        }
      }
      ASSERT(base_list != NULL);
      /* list should contain 2 base class specifiers */
      {
        int count = 0;
        for (i = 0; i < base_list->num_children; i++) {
          if (base_list->children[i].kind == CDD_CST_CHILD_NODE &&
              base_list->children[i].val.node->kind ==
                  CDD_CST_BASE_CLASS_SPECIFIER) {
            count++;
          }
        }
        ASSERT_EQ(2, count);
      }
    }
  }

  {
    ASSERT_EQ(0, cdd_cst_emit(tree, &out));
    ASSERT(out != NULL);
    ASSERT_STR_EQ(code, out);
    free(out);
  }

  cdd_cst_tree_free(tree);
  PASS();
}

SUITE(cdd_cst_suite) {
  RUN_TEST(test_cdd_cst_roundtrip_basic);
  RUN_TEST(test_cdd_cst_roundtrip_macros);
  RUN_TEST(test_cdd_cst_asm_statement);
  RUN_TEST(test_cdd_cst_cpp_class);
  RUN_TEST(test_cdd_cst_cpp_methods);
  RUN_TEST(test_cdd_cst_cpp_exceptions);
  RUN_TEST(test_cdd_cst_cpp_namespace);
  RUN_TEST(test_cdd_cst_cpp_template);
  RUN_TEST(test_cdd_cst_cpp_inheritance);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_PARSER_H */
