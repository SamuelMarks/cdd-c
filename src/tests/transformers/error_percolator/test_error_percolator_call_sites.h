#ifndef CDD_TEST_TRANSFORMER_ERROR_PERCOLATOR_CALL_SITES_H
#define CDD_TEST_TRANSFORMER_ERROR_PERCOLATOR_CALL_SITES_H

/* clang-format off */
#include "cdd_cst_transform.h"
#include "greatest.h"
#include "c_cdd/format_specifiers.h"
#include <errno.h>
/* clang-format on */

extern C_CDD_EXPORT int g_err_perc_fail;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

TEST test_cdd_check_is_call_unit(void) {
  int out_val;
  cdd_cst_node_t dummy_node;
  cdd_cst_child_t children[3];
  cdd_token_t tok_semi;
  cdd_token_t tok_lparen;

  out_val = 0;
  memset(&dummy_node, 0, sizeof(dummy_node));
  memset(children, 0, sizeof(children));
  memset(&tok_semi, 0, sizeof(tok_semi));
  memset(&tok_lparen, 0, sizeof(tok_lparen));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_check_is_call(NULL, 0, &out_val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_check_is_call(&dummy_node, 0, NULL));

  dummy_node.num_children = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_call(&dummy_node, 0, &out_val));
  ASSERT_EQ(0, out_val);

  children[0].kind = CDD_CST_CHILD_NODE;
  children[1].kind = CDD_CST_CHILD_NODE;
  dummy_node.children = children;
  dummy_node.num_children = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_call(&dummy_node, 0, &out_val));
  ASSERT_EQ(0, out_val);

  tok_semi.kind = CDD_TOKEN_SEMICOLON;
  children[1].kind = CDD_CST_CHILD_TOKEN;
  children[1].val.token = &tok_semi;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_call(&dummy_node, 0, &out_val));
  ASSERT_EQ(0, out_val);

  tok_lparen.kind = CDD_TOKEN_LPAREN;
  children[1].val.token = &tok_lparen;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_call(&dummy_node, 0, &out_val));
  ASSERT_EQ(1, out_val);

  PASS();
}

TEST test_cdd_check_is_function_def_unit(void) {
  int out_val;
  cdd_cst_node_t dummy_node;
  cdd_cst_child_t children[3];
  cdd_token_t tok_int;
  cdd_token_t tok_void;
  cdd_token_t tok_other;

  out_val = 0;
  memset(&dummy_node, 0, sizeof(dummy_node));
  memset(children, 0, sizeof(children));
  memset(&tok_int, 0, sizeof(tok_int));
  memset(&tok_void, 0, sizeof(tok_void));
  memset(&tok_other, 0, sizeof(tok_other));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_check_is_function_def(NULL, 0, &out_val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_check_is_function_def(&dummy_node, 0, NULL));

  dummy_node.kind = CDD_CST_FUNCTION_DEFINITION;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_function_def(&dummy_node, 0, &out_val));
  ASSERT_EQ(1, out_val);

  dummy_node.kind = CDD_CST_UNKNOWN;
  dummy_node.children = children;
  dummy_node.num_children = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_function_def(&dummy_node, 0, &out_val));
  ASSERT_EQ(0, out_val);

  children[0].kind = CDD_CST_CHILD_NODE;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_function_def(&dummy_node, 1, &out_val));
  ASSERT_EQ(0, out_val);

  tok_int.kind = CDD_TOKEN_KEYWORD_INT;
  children[0].kind = CDD_CST_CHILD_TOKEN;
  children[0].val.token = &tok_int;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_function_def(&dummy_node, 1, &out_val));
  ASSERT_EQ(1, out_val);

  tok_void.kind = CDD_TOKEN_IDENTIFIER;
  tok_void.start = (const uint8_t *)"void";
  tok_void.length = 4;
  children[0].val.token = &tok_void;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_function_def(&dummy_node, 1, &out_val));
  ASSERT_EQ(1, out_val);

  tok_other.kind = CDD_TOKEN_SEMICOLON;
  children[0].val.token = &tok_other;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_function_def(&dummy_node, 1, &out_val));
  ASSERT_EQ(0, out_val);

  /* Previous token is identifier of length != 4 */
  tok_other.kind = CDD_TOKEN_IDENTIFIER;
  tok_other.start = (const uint8_t *)"hello";
  tok_other.length = 5;
  children[0].val.token = &tok_other;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_function_def(&dummy_node, 1, &out_val));
  ASSERT_EQ(0, out_val);

  /* Previous token is identifier of length 4 but not "void" */
  tok_other.start = (const uint8_t *)"char";
  tok_other.length = 4;
  children[0].val.token = &tok_other;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_check_is_function_def(&dummy_node, 1, &out_val));
  ASSERT_EQ(0, out_val);

  PASS();
}

TEST test_cdd_rewrite_call_sites_unit(void) {
  cdd_cst_tree_t *tree;
  cdd_token_t tok_foo;
  cdd_token_t *mod_funcs[1];
  const char *code;
  int k;

  tree = NULL;
  code = "void bar(void) { foo(); }\n";
  memset(&tok_foo, 0, sizeof(tok_foo));
  tok_foo.kind = CDD_TOKEN_IDENTIFIER;
  tok_foo.start = (const uint8_t *)"foo";
  tok_foo.length = 3;
  mod_funcs[0] = &tok_foo;

  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code), &tree));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_rewrite_call_sites(NULL, tree->root, mod_funcs, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_rewrite_call_sites(tree, NULL, mod_funcs, 1));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_rewrite_call_sites(tree, tree->root, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 0));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));

  cdd_cst_tree_free(tree);

  {
    const char *code2 = "void bar(int x, int y) { foo(x, y); }\n";
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code2),
                               &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));
    cdd_cst_tree_free(tree);
  }

  /* Test non-call identifier reference */
  {
    const char *code_ref = "void bar(void) { void *p = foo; }\n";
    ASSERT_EQ(0, cdd_cst_parse(
                     az_span_create_from_str((char *)(size_t)code_ref), &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));
    cdd_cst_tree_free(tree);
  }

  /* Test prefix builder error failure */
  {
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code), &tree));
    g_err_perc_fail = 5;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);
  }

  /* Test call site rewrite when rparen_tok is absent */
  {
    cdd_cst_node_t dummy_parent;
    cdd_cst_child_t ch[2];
    cdd_token_t t_id;
    cdd_token_t t_lp;
    cdd_token_t *m[1];
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code), &tree));
    memset(&dummy_parent, 0, sizeof(dummy_parent));
    memset(ch, 0, sizeof(ch));
    memset(&t_id, 0, sizeof(t_id));
    memset(&t_lp, 0, sizeof(t_lp));
    t_id.kind = CDD_TOKEN_IDENTIFIER;
    t_id.start = (const uint8_t *)"foo";
    t_id.length = 3;
    t_lp.kind = CDD_TOKEN_LPAREN;
    ch[0].kind = CDD_CST_CHILD_TOKEN;
    ch[0].val.token = &t_id;
    ch[1].kind = CDD_CST_CHILD_TOKEN;
    ch[1].val.token = &t_lp;
    dummy_parent.children = ch;
    dummy_parent.num_children = 2;
    m[0] = &t_id;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_rewrite_call_sites(tree, &dummy_parent, m, 1));
    cdd_cst_tree_free(tree);
  }

  /* Test suffix builder error failure */
  {
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code), &tree));
    g_err_perc_fail = 6;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);
  }

  /* Test prefix splice_children failure */
  {
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code), &tree));
    g_err_perc_fail = 13;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);
  }

  /* Test suffix splice_children failure */
  {
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code), &tree));
    g_err_perc_fail = 14;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);
  }

  /* Test nested parentheses in call site and call without semicolon */
  {
    const char *code_nested = "void bar(void) { foo(bar()); }\n";
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_nested),
                         &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));
    cdd_cst_tree_free(tree);
  }

  /* Test call site with trailing comment and call site without trailing trivia
   */
  {
    const char *code_trivia =
        "void bar(void) { foo() /* comment */ ; /* semi */ }\n";
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_trivia),
                         &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));
    cdd_cst_tree_free(tree);

    ASSERT_EQ(0,
              cdd_cst_parse(az_span_create_from_str("void bar(void) { foo();}"),
                            &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));
    cdd_cst_tree_free(tree);
  }

  /* Test synthetic node with mixed token and node children */
  {
    cdd_cst_node_t *dummy_parent = NULL;
    cdd_token_t *t_id = NULL;
    cdd_token_t *t_lp = NULL;
    cdd_token_t *t_semi = NULL;
    cdd_token_t *t_rp = NULL;
    cdd_cst_node_t *dummy_child_node1 = NULL;
    cdd_cst_node_t *dummy_child_node2 = NULL;
    cdd_token_t *m[1];
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code), &tree));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_parent));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_child_node1));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_child_node2));

    cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, "foo", 3, &t_id);
    cdd_cst_create_token_len(tree, CDD_TOKEN_LPAREN, "(", 1, &t_lp);
    cdd_cst_create_token_len(tree, CDD_TOKEN_SEMICOLON, ";", 1, &t_semi);
    cdd_cst_create_token_len(tree, CDD_TOKEN_RPAREN, ")", 1, &t_rp);

    cdd_cst_append_child_token(dummy_parent, t_id);
    cdd_cst_append_child_token(dummy_parent, t_lp);
    cdd_cst_append_child_node(dummy_parent, dummy_child_node1);
    cdd_cst_append_child_token(dummy_parent, t_semi);
    cdd_cst_append_child_node(dummy_parent, dummy_child_node2);
    cdd_cst_append_child_token(dummy_parent, t_rp);

    m[0] = t_id;
    cdd_cst_free_node(tree->root);
    tree->root = dummy_parent;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_rewrite_call_sites(tree, tree->root, m, 1));
    cdd_cst_tree_free(tree);
  }

  /* Test new_cap >= 32 when string_capacity is already initialized */
  {
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code), &tree));
    tree->string_capacity = 32;
    tree->string_pool = (char **)C_CDD_CALLOC(32, sizeof(char *));
    tree->num_strings = 32;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1));
    cdd_cst_tree_free(tree);
  }

  for (k = 1; k <= 25; k++) {
    cdd_c_error_t rc = 0;
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code), &tree));
    g_cdd_alloc_fail = k;
    rc = cdd_rewrite_call_sites(tree, tree->root, mod_funcs, 1);
    g_cdd_alloc_fail = 0;
    cdd_cst_tree_free(tree);
    if (rc == CDD_C_SUCCESS) {
      break;
    }
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }
  g_cdd_alloc_fail = 0;

  PASS();
}

TEST test_cdd_transform_percolate_errors_comprehensive(void) {
  cdd_transform_config_t config;
  cdd_cst_tree_t *tree;
  cdd_cst_tree_t empty_tree;
  cdd_c_error_t rc = 0;

  memset(&config, 0, sizeof(config));
  config.indent_width = 2;
  tree = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_percolate_errors(NULL, &config));
  memset(&empty_tree, 0, sizeof(empty_tree));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_percolate_errors(&empty_tree, &config));

  {
    const char *code_already =
        "cdd_c_error_t f1(void) { return CDD_C_SUCCESS; }\n"
        "cdd_c_error f2(void) { return 0; }\n";
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_already),
                         &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  {
    const char *code_ret_void = "void empty_ret(void) { return; }\n";
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_ret_void),
                         &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  {
    const char *code_cdd_void = "CDD_VOID my_cdd_void(void) { return; }\n";
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_cdd_void),
                         &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  {
    const char *code_add = "int add(int a, int b) { return a + b; }\n";
    ASSERT_EQ(0, cdd_cst_parse(
                     az_span_create_from_str((char *)(size_t)code_add), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  {
    const char *code_ptr = "char *get_str(void) { return NULL; }\n";
    ASSERT_EQ(0, cdd_cst_parse(
                     az_span_create_from_str((char *)(size_t)code_ptr), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  {
    const char *code_allocs = "void *test_all_allocs(void) {\n"
                              "  void *p1 = malloc(10);\n"
                              "  void *p2 = calloc(1, 20);\n"
                              "  void *p3 = realloc(p1, 30);\n"
                              "  char *p4 = strdup(\"hello\");\n"
                              "  return;\n"
                              "}\n";
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_allocs),
                         &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  {
    const char *code_unassigned = "void bad_alloc(void) {\n"
                                  "  malloc(10);\n"
                                  "}\n";
    ASSERT_EQ(0, cdd_cst_parse(
                     az_span_create_from_str((char *)(size_t)code_unassigned),
                     &tree));
    rc = cdd_transform_percolate_errors(tree, &config);
    ASSERT_EQ(CDD_C_ERROR_PARSE, rc);
    cdd_cst_tree_free(tree);
  }

  {
    const char *code_call = "int callee(void) { return 42; }\n"
                            "void caller(void) { callee(); }\n";
    ASSERT_EQ(0,
              cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_call),
                            &tree));
    rc = cdd_transform_percolate_errors(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_tree_free(tree);
  }

  /* Test L-value break with keyword int */
  {
    const char *code_lval =
        "void test_lval(void) { int *p = malloc(10); return; }\n";
    ASSERT_EQ(0,
              cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_lval),
                            &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  /* Test g_err_perc_fail == 7 (modified_funcs failure) */
  {
    const char *code_f7 = "int f7(void) { return 1; }\n";
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_f7),
                               &tree));
    g_err_perc_fail = 7;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_transform_percolate_errors(tree, &config));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);
  }

  /* Test function body without braces (e.g. malformed or empty body) */
  {
    const char *code_nobrace = "int nobrace(void);\n";
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_nobrace),
                         &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  /* Test 11-char non-error return type and const return type */
  {
    const char *code_custom = "custom_type my_custom(void) { return 0; }\n"
                              "const int my_const(void) { return 1; }\n";
    ASSERT_EQ(
        0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_custom),
                         &tree));
    rc = cdd_transform_percolate_errors(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_tree_free(tree);
  }

  /* Test hooks 9, 10, 11, 12 for allocation and builder failures */
  {
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   "size_t my_sz(void) { return 0; }\n"),
                               &tree));
    g_err_perc_fail = 9;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);

    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   "char *my_s(void) { return 0; }\n"),
                               &tree));
    g_err_perc_fail = 10;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);

    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   "int my_bld(void) { return 0; }\n"),
                               &tree));
    g_err_perc_fail = 11;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);

    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   "int my_ret_bld(void) { return 0; }\n"),
                               &tree));
    g_err_perc_fail = 12;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);

    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   "int my_ret_spl(void) { return 0; }\n"),
                               &tree));
    g_err_perc_fail = 15;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_transform_percolate_errors(tree, &config));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);

    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   "int my_ret_spl2(void) { return 0; }\n"),
                               &tree));
    g_err_perc_fail = 16;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_transform_percolate_errors(tree, &config));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);
  }

  /* Test L-value token kinds: dot, arrow, bracket, star, and length 6/7
   * non-alloc identifiers */
  {
    const char *code_lvals = "void test_lval_variations(void) {\n"
                             "  obj.f = malloc(1);\n"
                             "  ptr->f = malloc(1);\n"
                             "  arr[0] = malloc(1);\n"
                             "  *p = malloc(1);\n"
                             "  buffer = malloc(1);\n"
                             "  message = realloc(p, 1);\n"
                             "  return (0);\n"
                             "}\n";
    ASSERT_EQ(0,
              cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_lvals),
                            &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);

    /* Test with tree string_capacity >= 32 during statement transformation */
    ASSERT_EQ(0,
              cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_lvals),
                            &tree));
    tree->string_capacity = 32;
    tree->string_pool = (char **)C_CDD_CALLOC(32, sizeof(char *));
    tree->num_strings = 32;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  /* Test return statement without semicolon */
  {
    const char *code_no_semi_ret = "void test_no_semi_ret(void) { return }\n";
    ASSERT_EQ(0, cdd_cst_parse(
                     az_span_create_from_str((char *)(size_t)code_no_semi_ret),
                     &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  /* Test AST edge cases with artificial CST nodes */
  {
    cdd_cst_node_t empty_func_node;
    cdd_cst_node_t *saved_node;
    ASSERT_EQ(0, cdd_cst_parse(
                     az_span_create_from_str("int ok_fn(void) { return 1; }\n"),
                     &tree));
    memset(&empty_func_node, 0, sizeof(empty_func_node));
    empty_func_node.kind = CDD_CST_FUNCTION_DEFINITION;
    saved_node = tree->root->children[0].val.node;
    tree->root->children[0].val.node = &empty_func_node;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    tree->root->children[0].val.node = saved_node;
    cdd_cst_tree_free(tree);
  }

  /* Test LPAREN at index 0 and RPAREN at index 0 */
  {
    cdd_cst_node_t dummy_fn;
    cdd_cst_child_t ch[2];
    cdd_token_t t_lp;
    cdd_token_t t_rp;
    cdd_cst_node_t *saved_node;
    ASSERT_EQ(0, cdd_cst_parse(
                     az_span_create_from_str("int ok_fn(void) { return 1; }\n"),
                     &tree));
    memset(&dummy_fn, 0, sizeof(dummy_fn));
    memset(ch, 0, sizeof(ch));
    memset(&t_lp, 0, sizeof(t_lp));
    memset(&t_rp, 0, sizeof(t_rp));
    dummy_fn.kind = CDD_CST_FUNCTION_DEFINITION;
    t_lp.kind = CDD_TOKEN_LPAREN;
    t_rp.kind = CDD_TOKEN_RPAREN;
    ch[0].kind = CDD_CST_CHILD_TOKEN;
    ch[0].val.token = &t_lp;
    ch[1].kind = CDD_CST_CHILD_TOKEN;
    ch[1].val.token = &t_rp;
    dummy_fn.children = ch;
    dummy_fn.num_children = 2;
    saved_node = tree->root->children[0].val.node;
    tree->root->children[0].val.node = &dummy_fn;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    ch[0].val.token = &t_rp;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    tree->root->children[0].val.node = saved_node;
    cdd_cst_tree_free(tree);
  }

  /* Test fake rparen not in tree */
  {
    cdd_token_t fake_rparen;
    ASSERT_EQ(0, cdd_cst_parse(
                     az_span_create_from_str("int ok_fn(void) { return 1; }\n"),
                     &tree));
    memset(&fake_rparen, 0, sizeof(fake_rparen));
    fake_rparen.kind = CDD_TOKEN_RPAREN;
    tree->root->children[0].val.node->children[3].val.token = &fake_rparen;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  /* Test return type default switch branch with OTHER token */
  {
    const char *code_def = "int ret_def(void) { return 1; }\n";
    cdd_cst_node_t *fn_node;
    ASSERT_EQ(0, cdd_cst_parse(
                     az_span_create_from_str((char *)(size_t)code_def), &tree));
    fn_node = tree->root->children[0].val.node;
    fn_node->children[0].val.token->kind = CDD_TOKEN_OTHER;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  /* Test non-RBRACE child token inside body_parent */
  {
    const char *code_m = "void test_m(void) { void *p = malloc(1); return; }\n";
    cdd_cst_node_t *body_node;
    cdd_token_t extra_tok;
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_m),
                               &tree));
    body_node =
        tree->root->children[0]
            .val.node
            ->children[tree->root->children[0].val.node->num_children - 1]
            .val.node;
    memset(&extra_tok, 0, sizeof(extra_tok));
    extra_tok.kind = CDD_TOKEN_SEMICOLON;
    cdd_cst_append_child_token(body_node, &extra_tok);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  /* Test func_name_tok == NULL and func_name_idx == 0 */
  {
    const char *code_edge_sigs = "foo_no_ret(void) { return 0; }\n";
    cdd_cst_node_t *fn_node;
    ASSERT_EQ(
        0, cdd_cst_parse(
               az_span_create_from_str((char *)(size_t)code_edge_sigs), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);

    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(
                                   "int f_node(void) { return 0; }\n"),
                               &tree));
    fn_node = tree->root->children[0].val.node;
    {
      cdd_cst_node_t *dummy1 = NULL;
      cdd_cst_node_t *dummy2 = NULL;
      cdd_token_t *saved_tok0 = fn_node->children[0].val.token;
      cdd_token_t *saved_tok1 = fn_node->children[1].val.token;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy1));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy2));
      fn_node->children[0].kind = CDD_CST_CHILD_NODE;
      fn_node->children[0].val.node = dummy1;
      fn_node->children[1].kind = CDD_CST_CHILD_NODE;
      fn_node->children[1].val.node = dummy2;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
      fn_node->children[0].kind = CDD_CST_CHILD_TOKEN;
      fn_node->children[0].val.token = saved_tok0;
      fn_node->children[1].kind = CDD_CST_CHILD_TOKEN;
      fn_node->children[1].val.token = saved_tok1;
      cdd_cst_free_node(dummy1);
      cdd_cst_free_node(dummy2);
    }
    cdd_cst_tree_free(tree);
  }

  /* Test char, uint64_t, and my_13_ident_t return types */
  {
    const char *code_types = "char my_char(void) { return 'a'; }\n"
                             "uint64_t my_u64(void) { return 0; }\n"
                             "my_13_ident_t f13(void) { return 0; }\n";
    ASSERT_EQ(0,
              cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_types),
                            &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  /* Test function with allocations where RBRACE is absent */
  {
    const char *code_no_rbrace =
        "void no_rbrace(void) { void *p = malloc(1); return; }\n";
    size_t ci;
    ASSERT_EQ(
        0, cdd_cst_parse(
               az_span_create_from_str((char *)(size_t)code_no_rbrace), &tree));
    for (ci = 0; ci < tree->base_tokens->size; ci++) {
      if (tree->base_tokens->tokens[ci].kind == CDD_TOKEN_RBRACE) {
        tree->base_tokens->tokens[ci].kind = CDD_TOKEN_OTHER;
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  {
    char buf[4096];
    size_t off;
    int fn_i;
    off = 0;
    for (fn_i = 0; fn_i < 35; fn_i++) {
#if defined(_MSC_VER)
      off += (size_t)sprintf_s(buf + off, sizeof(buf) - off,
                               "int fn_%d(void) { return %d; }\n", fn_i, fn_i);
#else
      off += (size_t)sprintf(buf + off, "int fn_%d(void) { return %d; }\n",
                             fn_i, fn_i);
#endif
    }
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(buf), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);

    /* Test g_err_perc_fail == 8 (modified_funcs realloc failure) */
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(buf), &tree));
    g_err_perc_fail = 8;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_transform_percolate_errors(tree, &config));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);
  }

  /* Test function with allocations where LBRACE is absent */
  {
    const char *code_no_lbrace =
        "void no_lbrace(void) { void *p = malloc(1); return; }\n";
    size_t ci;
    ASSERT_EQ(
        0, cdd_cst_parse(
               az_span_create_from_str((char *)(size_t)code_no_lbrace), &tree));
    for (ci = 0; ci < tree->base_tokens->size; ci++) {
      if (tree->base_tokens->tokens[ci].kind == CDD_TOKEN_LBRACE) {
        tree->base_tokens->tokens[ci].kind = CDD_TOKEN_OTHER;
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  /* Test function with allocations where RBRACE is absent */
  {
    const char *code_no_rbrace =
        "void no_rbrace(void) { void *p = malloc(1); return; }\n";
    size_t ci;
    ASSERT_EQ(
        0, cdd_cst_parse(
               az_span_create_from_str((char *)(size_t)code_no_rbrace), &tree));
    for (ci = 0; ci < tree->base_tokens->size; ci++) {
      if (tree->base_tokens->tokens[ci].kind == CDD_TOKEN_RBRACE) {
        tree->base_tokens->tokens[ci].kind = CDD_TOKEN_OTHER;
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_percolate_errors(tree, &config));
    cdd_cst_tree_free(tree);
  }

  {
    int k;
    const char *code_oom =
        "int simple_alloc(void) { void *p = malloc(16); return; }\n";
    for (k = 1; k <= 25; k++) {
      ASSERT_EQ(0,
                cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_oom),
                              &tree));
      g_cdd_alloc_fail = k;
      rc = cdd_transform_percolate_errors(tree, &config);
      g_cdd_alloc_fail = 0;
      cdd_cst_tree_free(tree);
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    g_cdd_alloc_fail = 0;
  }

  {
    int k;
    const char *code_call_oom = "int callee(void) { return 1; }\n"
                                "void caller(void) { callee(); }\n";
    for (k = 1; k <= 30; k++) {
      ASSERT_EQ(0, cdd_cst_parse(
                       az_span_create_from_str((char *)(size_t)code_call_oom),
                       &tree));
      g_cdd_alloc_fail = k;
      rc = cdd_transform_percolate_errors(tree, &config);
      g_cdd_alloc_fail = 0;
      cdd_cst_tree_free(tree);
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    g_cdd_alloc_fail = 0;
  }

  {
    const char *code_call_fail = "int callee(void) { return 1; }\n"
                                 "void caller(void) { callee(); }\n";
    ASSERT_EQ(
        0, cdd_cst_parse(
               az_span_create_from_str((char *)(size_t)code_call_fail), &tree));
    g_err_perc_fail = 22;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_transform_percolate_errors(tree, &config));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);

    ASSERT_EQ(
        0, cdd_cst_parse(
               az_span_create_from_str((char *)(size_t)code_call_fail), &tree));
    g_err_perc_fail = 23;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_transform_percolate_errors(tree, &config));
    g_err_perc_fail = 0;
    cdd_cst_tree_free(tree);
  }

  PASS();
}

/**
 * @brief Error percolator transformer test suite.
 */

/**
 * @brief Error percolator call sites test suite.
 */
SUITE(transformer_error_percolator_call_sites_suite) {
  RUN_TEST(test_cdd_check_is_call_unit);
  RUN_TEST(test_cdd_check_is_function_def_unit);
  RUN_TEST(test_cdd_rewrite_call_sites_unit);
  RUN_TEST(test_cdd_transform_percolate_errors_comprehensive);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_TEST_TRANSFORMER_ERROR_PERCOLATOR_CALL_SITES_H */
