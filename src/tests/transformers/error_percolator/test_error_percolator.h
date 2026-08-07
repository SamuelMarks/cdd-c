#ifndef CDD_TEST_TRANSFORMER_ERROR_PERCOLATOR_H
#define CDD_TEST_TRANSFORMER_ERROR_PERCOLATOR_H

/* clang-format off */
/* #include "cdd_cst.h" */
#include "cdd_cst_transform.h"
#include "greatest.h"
#include <errno.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

TEST test_cdd_transform_percolate_errors(void) {
  cdd_cst_tree_t *tree;
  const char *code = "void func_void() { }\n"
                     "int non_void_func() { return 1; }\n";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_percolate_errors(NULL, &config));

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  int rc = cdd_transform_percolate_errors(tree, &config);
  ASSERT(rc == 0 || rc == CDD_C_ERROR_PARSE);

  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_percolate_errors_complex(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "void func_void() { }\n"
      "int non_void_func() { return 1; }\n"
      "void * my_void_ptr_func() { return NULL; }\n"
      "void trailing_call() { my_void_ptr_func(); }\n"
      "void skip_ident_1() { int x; void; }\n"
      "void skip_ident_2() { int x; void x; }\n"
      "void skip_ident_3() { int x; void x(); }\n"
      "void skip_ident_4() { int x; void x(int y); }\n"
      "void skip_ident_5() { int x; void x(int y, ...); }\n"
      "void skip_ident_6() { int x; void * x(int y, ...); }\n"
      "void skip_ident_7() { int x; void * x(int y, ...) { return NULL; } }\n"
      "void skip_ident_8() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_9() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_10() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_11() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_12() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_13() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_14() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_15() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_16() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_17() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_18() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_19() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_20() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_21() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_22() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_23() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_24() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_25() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_26() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_27() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_28() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_29() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_30() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_31() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_32() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_33() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_34() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_35() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_36() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_37() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_38() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_39() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_40() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_41() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_42() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_43() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_44() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_45() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_46() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_47() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_48() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_49() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_50() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_51() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_52() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_53() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_54() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_55() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_56() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_57() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_58() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_59() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_60() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_61() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_62() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_63() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_64() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_65() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_66() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_67() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_68() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_69() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_70() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_71() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_72() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_73() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_74() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_75() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_76() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_77() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_78() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_79() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_80() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_81() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_82() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_83() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_84() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_85() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_86() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_87() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_88() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_89() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_90() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_91() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_92() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_93() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_94() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_95() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_96() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_97() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_98() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_99() { int x; void * x(int y, ...) { return NULL; } } "
      "void skip_ident_100() { int x; void * x(int y, ...) { return NULL; } } ";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_percolate_errors(NULL, &config));

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  int rc = cdd_transform_percolate_errors(tree, &config);
  ASSERT(rc == 0 || rc == CDD_C_ERROR_PARSE);

  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_percolate_errors_edge_cases(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "void func_void() { }\n"
      "int non_void_func() { return 1; }\n"
      "void * my_void_ptr_func() { return NULL; }\n"
      "void trailing_call() { my_void_ptr_func }\n"
      "void skip_ident_2() { int x; void x; }\n"
      "void alloc_complex() { struct X { int *p; } x; x.p = malloc(10); }\n"
      "void alloc_complex2() { int *arr[10]; arr[0] = calloc(1, 10); }\n"
      "void alloc_complex3() { struct X { int *p; } *x; x->p = realloc(NULL, "
      "10); }\n"
      "void alloc_strdup() { char *s = strdup(\"test\"); }\n";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_percolate_errors(NULL, &config));

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  int rc = cdd_transform_percolate_errors(tree, &config);
  ASSERT(rc == 0 || rc == CDD_C_ERROR_PARSE);

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_err_perc_fail;
#endif

TEST test_cdd_transform_percolate_errors_bld_fail(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  cdd_token_t *rbrace_tok_to_free = NULL;
  cdd_cst_node_t *unknown_node = NULL;
  cdd_token_t *rbrace_tok = NULL;
  cdd_cst_child_t *c = NULL;
  const char *code =
      "void foo() { void *p = malloc(1); void *p2 = malloc(1); return; }";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  g_err_perc_fail = 1;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  cdd_transform_percolate_errors(tree, &config);

  {
    cdd_cst_tree_t *t = NULL;
    cdd_cst_parse(
        az_span_create_from_str("CDD_VOID edge_void() { malloc(1); }"), &t);
    cdd_transform_percolate_errors(t, &config);
    cdd_cst_tree_free(t);
  }
  {
    cdd_cst_tree_t *t = NULL;
    cdd_cst_parse(az_span_create_from_str(
                      "int /* comment */ edge_void2() { malloc(1); }"),
                  &t);
    cdd_transform_percolate_errors(t, &config);
    cdd_cst_tree_free(t);
  }
  {
    cdd_cst_tree_t *t = NULL;
    cdd_cst_parse(
        az_span_create_from_str("void edge_void3() { malloc(1); malloc(1); }"),
        &t);
    cdd_transform_percolate_errors(t, &config);
    cdd_cst_tree_free(t);
  }
  {
    cdd_cst_tree_t *t = NULL;
    cdd_cst_parse(
        az_span_create_from_str("void edge_void4() { if (1) malloc(1); }"), &t);
    cdd_transform_percolate_errors(t, &config);
    cdd_cst_tree_free(t);
  }
  {
    cdd_cst_tree_t *t = NULL;
    cdd_cst_parse(
        az_span_create_from_str("void edge_void5() { if (1) { malloc(1); } }"),
        &t);
    cdd_transform_percolate_errors(t, &config);
    cdd_cst_tree_free(t);
  }
  cdd_cst_tree_free(tree);
  tree = NULL;

  g_err_perc_fail = 2; /* Custom trigger for cleanup node */

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));

  /* Mock an unknown block node manually to hit the unknown branch logic */
  unknown_node = calloc(1, sizeof(cdd_cst_node_t));
  rbrace_tok = calloc(1, sizeof(cdd_token_t));
  c = calloc(1, sizeof(cdd_cst_child_t));
  rbrace_tok_to_free = rbrace_tok;

  unknown_node->kind = CDD_CST_UNKNOWN;
  /* Add an RBRACE to it so it matches */
  rbrace_tok->kind = CDD_TOKEN_RBRACE;
  c->kind = CDD_CST_CHILD_TOKEN;
  c->val.token = rbrace_tok;
  unknown_node->children = c;
  unknown_node->num_children = 1;
  /* Add to end of first function definition in tree */
  if (tree->root && tree->root->num_children > 0 &&
      tree->root->children[0].kind == CDD_CST_CHILD_NODE) {
    cdd_cst_append_child_node(tree->root->children[0].val.node, unknown_node);
  }

  cdd_transform_percolate_errors(tree, &config);
  /* The unknown_node was appended. Does it get freed? No, append_child_node
   * just adds it to the children array. But cdd_cst_tree_free frees tokens. The
   * nodes must be freed. */

  cdd_cst_tree_free(tree);
  if (rbrace_tok_to_free)
    free(rbrace_tok_to_free);
  tree = NULL;

  g_err_perc_fail = 3; /* For return builder mock */
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  cdd_transform_percolate_errors(tree, &config);
  cdd_cst_tree_free(tree);
  tree = NULL;

  g_err_perc_fail = 4; /* For tmp_name malloc mock */
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  cdd_transform_percolate_errors(tree, &config);
  cdd_cst_tree_free(tree);
  tree = NULL;

  g_err_perc_fail = 0;
#endif
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_percolate_errors_oom(void) {
#ifdef CDD_BUILD_TESTS
  const char *code = "int my_func0() { return 0; }\n"
                     "int trailing_call0() { my_func0(); return 0; }\n"
                     "int my_func1() { return 0; }\n"
                     "int trailing_call1() { my_func1(); return 0; }\n"
                     "int my_func2() { return 0; }\n"
                     "int trailing_call2() { my_func2(); return 0; }\n"
                     "int my_func3() { return 0; }\n"
                     "int trailing_call3() { my_func3(); return 0; }\n"
                     "int my_func4() { return 0; }\n"
                     "int trailing_call4() { my_func4(); return 0; }\n"
                     "int my_func5() { return 0; }\n"
                     "int trailing_call5() { my_func5(); return 0; }\n"
                     "int my_func6() { return 0; }\n"
                     "int trailing_call6() { my_func6(); return 0; }\n"
                     "int my_func7() { return 0; }\n"
                     "int trailing_call7() { my_func7(); return 0; }\n"
                     "int my_func8() { return 0; }\n"
                     "int trailing_call8() { my_func8(); return 0; }\n"
                     "int my_func9() { return 0; }\n"
                     "int trailing_call9() { my_func9(); return 0; }\n"
                     "int my_func10() { return 0; }\n"
                     "int trailing_call10() { my_func10(); return 0; }\n"
                     "int my_func11() { return 0; }\n"
                     "int trailing_call11() { my_func11(); return 0; }\n"
                     "int my_func12() { return 0; }\n"
                     "int trailing_call12() { my_func12(); return 0; }\n"
                     "int my_func13() { return 0; }\n"
                     "int trailing_call13() { my_func13(); return 0; }\n"
                     "int my_func14() { return 0; }\n"
                     "int trailing_call14() { my_func14(); return 0; }\n"
                     "int my_func15() { return 0; }\n"
                     "int trailing_call15() { my_func15(); return 0; }\n"
                     "int my_func16() { return 0; }\n"
                     "int trailing_call16() { my_func16(); return 0; }\n"
                     "int my_func17() { return 0; }\n"
                     "int trailing_call17() { my_func17(); return 0; }\n"
                     "int my_func18() { return 0; }\n"
                     "int trailing_call18() { my_func18(); return 0; }\n"
                     "int my_func19() { return 0; }\n"
                     "int trailing_call19() { my_func19(); return 0; }\n"
                     "int my_func20() { return 0; }\n"
                     "int trailing_call20() { my_func20(); return 0; }\n"
                     "int my_func21() { return 0; }\n"
                     "int trailing_call21() { my_func21(); return 0; }\n"
                     "int my_func22() { return 0; }\n"
                     "int trailing_call22() { my_func22(); return 0; }\n"
                     "int my_func23() { return 0; }\n"
                     "int trailing_call23() { my_func23(); return 0; }\n"
                     "int my_func24() { return 0; }\n"
                     "int trailing_call24() { my_func24(); return 0; }\n"
                     "int my_func25() { return 0; }\n"
                     "int trailing_call25() { my_func25(); return 0; }\n"
                     "int my_func26() { return 0; }\n"
                     "int trailing_call26() { my_func26(); return 0; }\n"
                     "int my_func27() { return 0; }\n"
                     "int trailing_call27() { my_func27(); return 0; }\n"
                     "int my_func28() { return 0; }\n"
                     "int trailing_call28() { my_func28(); return 0; }\n"
                     "int my_func29() { return 0; }\n"
                     "int trailing_call29() { my_func29(); return 0; }\n"
                     "int my_func30() { return 0; }\n"
                     "int trailing_call30() { my_func30(); return 0; }\n"
                     "int my_func31() { return 0; }\n"
                     "int trailing_call31() { my_func31(); return 0; }\n"
                     "int my_func32() { return 0; }\n"
                     "int trailing_call32() { my_func32(); return 0; }\n"
                     "int my_func33() { return 0; }\n"
                     "int trailing_call33() { my_func33(); return 0; }\n";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};
  int i;
  for (i = 1; i < 2000; i++) {
    cdd_cst_tree_t *tree = NULL;
    int rc;
    rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
    ASSERT_EQ(0, rc);

    cdd_cst_query_result_t res_test;
    rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_FUNCTION_DEFINITION,
                                    &res_test);
    if (rc == 0) {
      printf("Found %zu functions at i=%d\n", res_test.size, i);
      /*
      for (size_t k = 0; k < res_test.size; k++) {
         printf("Func %zu\n", k);
      }
      */
      C_CDD_FREE(res_test.nodes);
    }

    g_cdd_alloc_fail = i;
    rc = cdd_transform_percolate_errors(tree, &config);
    if (rc == CDD_C_SUCCESS) {
      printf("OOM loop success at i=%d, alloc_fail remaining: %d, rc=%d\n", i,
             g_cdd_alloc_fail, rc);
      break;
    }
    printf("OOM loop i=%d failed with rc=%d\n", i, rc);
    g_cdd_alloc_fail = 0;

    cdd_cst_tree_free(tree);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }
  g_cdd_alloc_fail = 0;
#endif
  PASS();
}

/**
 * @brief Error percolator transformer test suite.
 */
SUITE(transformer_error_percolator_suite) {
  RUN_TEST(test_cdd_transform_percolate_errors);
  RUN_TEST(test_cdd_transform_percolate_errors_complex);
  RUN_TEST(test_cdd_transform_percolate_errors_edge_cases);
  RUN_TEST(test_cdd_transform_percolate_errors_bld_fail);
  RUN_TEST(test_cdd_transform_percolate_errors_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_TEST_TRANSFORMER_ERROR_PERCOLATOR_H */
