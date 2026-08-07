/**
 * @file test_cdd_cst_builder.h
 * @brief Unit tests for the CST builder.
 */

#ifndef TEST_CDD_CST_BUILDER_H
#define TEST_CDD_CST_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/emit/cdd_cst_emit.h"
#include "classes/parse/cdd_cst_factory.h"
/* clang-format on */

TEST test_cdd_cst_builder_basic(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  int rc;
  int out_has = -1;
  (void)out_has;
  char *out = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_builder_init(&b, tree, root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  out_has = -1;
  {
    cdd_trivia_t *trivia_ptr = NULL;
    cdd_cst_node_t *node_arr[1] = {NULL};
    rc = cdd_cst_extract_leading_trivia(NULL, &trivia_ptr);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_extract_trailing_trivia(NULL, &trivia_ptr);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_replace_node(NULL, root, root);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_replace_node(tree, NULL, root);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_replace_node(tree, root, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_splice_nodes(NULL, root, 0, node_arr, 0);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_splice_nodes(&b, NULL, 0, node_arr, 0);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_splice_nodes(&b, root, 0, NULL, 1);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }

  {
    int err = 0;
    rc = cdd_cst_builder_init(NULL, tree, root);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_builder_init(&b, NULL, root);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_builder_init(&b, tree, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_builder_has_error(&b, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_cst_builder_set_insert_point(NULL, root);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_builder_set_insert_point(&b, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_bld_snippet(NULL, "a");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_bld_snippet(&b, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_quote(NULL, "a");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_quote(&b, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_bld_line_comment(NULL, "a");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_bld_line_comment(&b, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_bld_block_comment(NULL, "a");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_bld_block_comment(&b, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_extract_leading_trivia(NULL, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_extract_leading_trivia(root, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_extract_trailing_trivia(NULL, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_extract_trailing_trivia(root, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_transfer_trivia(NULL, root);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_transfer_trivia(root, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_splice_nodes(NULL, root, 0, NULL, 0);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_splice_nodes(&b, NULL, 0, NULL, 0);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_splice_nodes(&b, root, 0, NULL, 1);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }

  ASSERT_EQ(0, cdd_cst_builder_has_error(&b, &out_has));
  ASSERT_EQ(0, out_has);

  rc = cdd_cst_bld_ident(&b, "int");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_space(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_ident(&b, "main");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_punct(&b, "(");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_punct(&b, ")");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_space(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_block_open(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_indent(&b, b.indent_level);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_ident(&b, "return");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_space(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_int(&b, 0);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_punct(&b, ";");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_block_close(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  fflush(stdout);
  rc = cdd_cst_emit(tree, &out);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(out, "int main()") != NULL);
  ASSERT(strstr(out, "return 0;") != NULL);

  free(out);
  rc = cdd_cst_builder_free(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  /* Manual tree free since we built it from scratch without a lexer list */

  cdd_cst_free_node_only(NULL);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_builder_macros(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  int rc;
  int out_has = -1;
  (void)out_has;
  char *out = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_builder_init(&b, tree, root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_include(&b, "stdio.h", 1);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_ifndef(&b, "TEST_MACRO");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_ifdef(&b, "TEST_MACRO2");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_else(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_endif(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_endif(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_extern_c_open(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_extern_c_close(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  free(out);
  cdd_cst_builder_free(&b);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_builder_quote(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  int rc;
  int out_has = -1;
  (void)out_has;
  char *out = NULL;
  cdd_cst_node_t *injected_node = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &injected_node);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_builder_init(&b, tree, root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_quote(&b, "int %s = %d; %% %n", "my_var", 42, injected_node);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(out, "my_var") != NULL);

  free(out);
  cdd_cst_builder_free(&b);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_builder_snippet(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  int rc;
  int out_has = -1;
  (void)out_has;
  char *out = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_builder_init(&b, tree, root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_snippet(&b, "void func() { return; }");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("void func() { return; }", out);

  free(out);
  cdd_cst_builder_free(&b);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_builder_comments(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  int rc;
  int out_has = -1;
  (void)out_has;
  char *out = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_builder_init(&b, tree, root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_block_comment(&b, " block ");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_line_comment(&b, " line");
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(out, "block") != NULL);
  ASSERT(strstr(out, "line") != NULL);

  free(out);
  cdd_cst_builder_free(&b);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_builder_errors(void) {
  cdd_cst_builder_t b;
  int rc;
  int out_has = -1;
  (void)out_has;

  rc = cdd_cst_builder_init(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = cdd_cst_builder_free(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  out_has = -1;
  rc = cdd_cst_builder_has_error(NULL, &out_has);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(1, out_has);

  b.error_state = CDD_C_ERROR_MEMORY;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_builder_has_error(&b, &out_has));
  ASSERT_EQ(1, out_has);

  rc = cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "test");
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  rc = cdd_cst_builder_set_insert_point(&b, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = cdd_cst_bld_space(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_builder_trivia_and_splice(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_node_t *target_node = NULL;
  cdd_cst_node_t *replacement_node = NULL;
  cdd_cst_node_t *spliced_node = NULL;
  cdd_cst_builder_t b;
  int rc;
  int out_has = -1;
  (void)out_has;
  cdd_trivia_t *lead;

  cdd_cst_tree_t *replacement_node_tree = NULL;
  cdd_cst_parse(az_span_create_from_str("/* L1 */ /* L2 */ int x; /* T1 */"),
                &tree);
  root = tree->root;
  (void)root;
  target_node = tree->root->children[0].val.node;
  cdd_cst_builder_init(&b, tree, tree->root);

  cdd_cst_parse(
      az_span_create_from_str("/* NL1 */ float y; /* NT1 */ /* NT2 */"),
      &replacement_node_tree);
  replacement_node = replacement_node_tree->root->children[0].val.node;

  rc = cdd_cst_extract_leading_trivia(target_node, &lead);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_extract_trailing_trivia(target_node, &lead);
  ASSERT_EQ(0, rc);

  /* This will transfer L1 L2 to NL1, and T1 to NT1 NT2 */
  rc = cdd_cst_transfer_trivia(target_node, replacement_node);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_replace_node_preserve_trivia(&b, target_node, replacement_node);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_alloc_node(CDD_CST_STATEMENT, &spliced_node);

  /* Also test the leak paths (lead without t_first) */
  {
    cdd_cst_node_t *empty_node;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &empty_node);
    cdd_cst_transfer_trivia(
        replacement_node,
        empty_node); /* replacement_node has all the trivia now */
    cdd_cst_free_node_only(empty_node);
  }

  {
    cdd_cst_node_t *nodes[1];
    nodes[0] = spliced_node;
    rc = cdd_cst_splice_nodes(&b, replacement_node, 0, nodes, 1);
    printf("rc = %d\n", rc);
    ASSERT_EQ(0, rc);
  }

  /* Error checks */
  b.error_state = CDD_C_ERROR_MEMORY;
  rc = cdd_cst_replace_node_preserve_trivia(&b, target_node, replacement_node);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  rc = cdd_cst_splice_nodes(&b, replacement_node, 0, NULL, 0);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  b.error_state = 0;

  rc = cdd_cst_extract_leading_trivia(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_cst_extract_leading_trivia(target_node, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_cst_extract_trailing_trivia(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_cst_extract_trailing_trivia(target_node, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_cst_transfer_trivia(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_cst_replace_node_preserve_trivia(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_cst_splice_nodes(NULL, NULL, 0, NULL, 1);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_cst_splice_nodes(&b, replacement_node, 0, NULL, 0);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  cdd_cst_builder_free(&b);

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_builder_extra(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  int rc;
  int out_has = -1;
  (void)out_has;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(*tree));
  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  /* Null checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_builder_init(NULL, NULL, NULL));
  out_has = -1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_builder_has_error(NULL, &out_has));
  ASSERT_EQ(1, out_has);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_builder_set_insert_point(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_bld_token(NULL, CDD_TOKEN_EOF, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_indent(NULL, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_snippet(NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_line_comment(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_bld_block_comment(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_ident(NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_punct(NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_string(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_space(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_newline(NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_extract_leading_trivia(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_extract_trailing_trivia(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_transfer_trivia(NULL, NULL));

  rc = cdd_cst_builder_init(&b, tree, root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  /* Test indent */
  rc = cdd_cst_bld_indent(&b, 2);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  /* Test insert point */
  rc = cdd_cst_builder_set_insert_point(&b, root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  /* Test error state */
  b.error_state = CDD_C_ERROR_MEMORY;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_builder_set_insert_point(&b, root));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_token(&b, CDD_TOKEN_EOF, "eof"));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_indent(&b, 1));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_snippet(&b, "snippet"));

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_builder_quote_errors(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  char buf[3000];

  tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  cdd_cst_builder_init(&b, tree, root);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_quote(NULL, "abc"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_quote(&b, NULL));

  b.error_state = CDD_C_ERROR_INVALID_ARGUMENT;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_quote(&b, "abc"));
  b.error_state = 0;

  /* buffer overflow */
  memset(buf, 'a', 2999);
  buf[2999] = '\0';
  cdd_cst_quote(&b, "123%s", buf);

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_builder_errors_extra(void) {
  cdd_cst_builder_t b;
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;

  cdd_cst_parse(az_span_create_from_str(""), &tree);
  if (tree->root)
    cdd_cst_free_node(tree->root);
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  cdd_cst_builder_init(&b, tree, root);

  /* NULL checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_bld_token(NULL, CDD_TOKEN_IDENTIFIER, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_int(NULL, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_punct(NULL, ";"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_block_open(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_block_close(NULL));

  /* Force error state */
  b.error_state = CDD_C_ERROR_MEMORY;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "a"));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_int(&b, 1));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_punct(&b, ";"));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_block_open(&b));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_block_close(&b));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_line_comment(&b, "test"));

  b.error_state = 0;
  ASSERT_EQ(0, cdd_cst_bld_line_comment(&b, "test2"));

  cdd_cst_bld_newline(&b);
  cdd_cst_bld_space(&b);

  cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "a");
  cdd_cst_bld_newline(&b);
  cdd_cst_bld_newline(&b); /* test trailing trivia append */
  cdd_cst_bld_newline(&b); /* test trailing trivia loop */

  /* Trigger error in snippet lexing or pool by mocking error_state */
  ASSERT_EQ(0, cdd_cst_bld_snippet(&b, "int z = 1;"));

  cdd_cst_tree_free(tree);

  /* leak root intentionally */
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
#endif

TEST test_cdd_cst_builder_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_builder_t b;
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
  cdd_cst_builder_init(&b, tree, root);

  g_cdd_cst_alloc_token_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "a"));
  g_cdd_cst_alloc_token_fail = 0;

  b.error_state = 0;

  /* Trigger realloc in track_synthesized */
  tree->synthesized_capacity = 0;
  tree->num_synthesized = 0;
  g_cdd_cst_realloc_fail = 1;
  {
    cdd_token_t *out_tok = NULL;
    ASSERT_EQ(
        CDD_C_ERROR_MEMORY,
        cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "test", &out_tok));
  }
  g_cdd_cst_realloc_fail = 0;
  g_cdd_cst_realloc_fail = 1;
  {
    cdd_cst_node_t *n1 = NULL;
    cdd_cst_node_t *n2 = NULL;
    cdd_token_t tok = {0};
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n1);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n2);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_append_child_node(n1, n2));
    g_cdd_cst_realloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_append_child_token(n1, &tok));
    cdd_cst_free_node_only(n1);
    cdd_cst_free_node_only(n2);
  }
  g_cdd_cst_realloc_fail = 0;

  /* To trigger append_child_token failure inside cdd_cst_bld_token */
  cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "success1");
  b.target_node->capacity = 0;
  b.target_node->num_children = 0;
  g_cdd_cst_realloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "trigger"));
  g_cdd_cst_realloc_fail = 0;
  b.error_state = 0;

  /* To trigger append_child_node failure inside cdd_cst_bld_block_open */
  cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "success2");
  b.target_node->capacity = 0;
  b.target_node->num_children = 0;
  g_cdd_cst_realloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_block_open(&b));
  g_cdd_cst_realloc_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_node_fail = 1;
  {
    cdd_cst_node_t *n1 = NULL;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_alloc_node(CDD_CST_STATEMENT, &n1));
  }
  g_cdd_cst_alloc_node_fail = 0;

  /* For trivia array OOM */
  g_cdd_cst_alloc_token_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_line_comment(&b, "test"));
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_token_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_block_open(&b));
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_token_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_block_close(&b));
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  b.indent_level = 2;
  g_cdd_cst_alloc_token_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_block_close(&b));
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  b.indent_level = 2;
  g_cdd_cst_alloc_token_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_block_close(&b));
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  b.indent_level = 2;
  g_cdd_cst_alloc_token_fail = 4;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_block_close(&b));
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  g_cdd_alloc_fail = 1;
  {
    cdd_c_error_t int_rc = cdd_cst_bld_int(&b, 10);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, int_rc);
  }
  g_cdd_alloc_fail = 0;
  b.error_state = 0;

  g_cdd_alloc_fail = 3;
  tree->string_capacity = tree->num_strings;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_cst_bld_block_comment(&b, "test_realloc_fail"));
  g_cdd_alloc_fail = 0;
  b.error_state = 0;

  {
    /* Test pool_string failure inside cdd_cst_bld_snippet trivia processing */
    int i;
    for (i = 1; i <= 20; ++i) {
      tree->string_capacity = tree->num_strings;
      g_cdd_alloc_fail = i;
      cdd_cst_bld_snippet(&b, "int /* test pool fail */ x;");
      g_cdd_alloc_fail = 0;
      b.error_state = 0;
    }
  }

  cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "foo");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_bld_block_comment(&b, "test1"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_bld_block_comment(&b, "test2"));

  {
    int i;
    for (i = 1; i < 20; i++) {
      b.error_state = 0;
      g_cdd_alloc_fail = i;
      cdd_cst_bld_snippet(&b, "int x;");
    }
  }
  g_cdd_alloc_fail = 0;
  b.error_state = 0;

  {
    int i;
    for (i = 1; i < 20; i++) {
      b.error_state = 0;
      g_cdd_alloc_fail = i;
      cdd_cst_quote(&b, "%s", "foo");
    }
  }
  g_cdd_alloc_fail = 0;
  b.error_state = 0;

  g_cdd_cst_realloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_quote(&b, "%n", root));
  g_cdd_cst_realloc_fail = 0;
  b.error_state = 0;

  {
    int i;
    for (i = 1; i < 20; i++) {
      b.error_state = 0;
      g_cdd_alloc_fail = i;
      cdd_cst_quote(&b, "int x;");
    }
  }
  g_cdd_alloc_fail = 0;
  b.error_state = 0;

  cdd_cst_tree_free(tree);
#endif
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_alloc_fail;
#endif

TEST test_cdd_cst_builder_punct_all(void) {
  cdd_cst_builder_t b;
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
  cdd_cst_builder_init(&b, tree, root);

  /* Test all punct types */
  cdd_cst_bld_punct(&b, "[");
  cdd_cst_bld_punct(&b, "]");
  cdd_cst_bld_punct(&b, "{");
  cdd_cst_bld_punct(&b, "}");
  cdd_cst_bld_punct(&b, "...");
  cdd_cst_bld_punct(&b, "->");
  cdd_cst_bld_punct(&b, ".");
  cdd_cst_bld_punct(&b, "+");
  cdd_cst_bld_punct(&b, "-");
  cdd_cst_bld_punct(&b, "*");
  cdd_cst_bld_punct(&b, "/");
  cdd_cst_bld_punct(&b, "%");
  cdd_cst_bld_punct(&b, "==");
  cdd_cst_bld_punct(&b, "!=");
  cdd_cst_bld_punct(&b, "<");
  cdd_cst_bld_punct(&b, ">");
  cdd_cst_bld_punct(&b, "<=");
  cdd_cst_bld_punct(&b, ">=");
  cdd_cst_bld_punct(&b, "&&");
  cdd_cst_bld_punct(&b, "||");
  cdd_cst_bld_punct(&b, "!");
  cdd_cst_bld_punct(&b, "&");
  cdd_cst_bld_punct(&b, "|");
  cdd_cst_bld_punct(&b, "^");
  cdd_cst_bld_punct(&b, "~");
  cdd_cst_bld_punct(&b, "<<");
  cdd_cst_bld_punct(&b, ">>");
  cdd_cst_bld_punct(&b, "=");
  cdd_cst_bld_punct(&b, "+=");
  cdd_cst_bld_punct(&b, "-=");
  cdd_cst_bld_punct(&b, "*=");
  cdd_cst_bld_punct(&b, "/=");
  cdd_cst_bld_punct(&b, "%=");
  cdd_cst_bld_punct(&b, "<<=");
  cdd_cst_bld_punct(&b, ">>=");
  cdd_cst_bld_punct(&b, "&=");
  cdd_cst_bld_punct(&b, "^=");
  cdd_cst_bld_punct(&b, "|=");
  cdd_cst_bld_punct(&b, "?");
  cdd_cst_bld_punct(&b, ":");
  cdd_cst_bld_punct(&b, ",");
  cdd_cst_bld_punct(&b, "#");
  cdd_cst_bld_punct(&b, "##");
  cdd_cst_bld_punct(&b, "++");
  cdd_cst_bld_punct(&b, "--");
  cdd_cst_bld_punct(&b, "other");

  /* Test all remaining builder functions for CDD_C_ERROR_INVALID_ARGUMENT and
   * error_state */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_include(NULL, "a", 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_ifndef(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_ifdef(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_line_comment(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_bld_block_comment(NULL, "a"));

  b.error_state = CDD_C_ERROR_MEMORY;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_include(&b, "a", 0));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_ifndef(&b, "a"));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_ifdef(&b, "a"));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_line_comment(&b, "a"));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_block_comment(&b, "a"));
  b.error_state = 0;

  cdd_cst_bld_include(&b, "a", 0);
  cdd_cst_bld_ifndef(&b, "a");
  cdd_cst_bld_ifdef(&b, "a");

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
    /* pool_string_safe OOM */
    g_cdd_cst_alloc_token_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_ident(&b, "a"));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    /* space OOM inside indent */
    g_cdd_cst_alloc_token_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_indent(&b, 1));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    /* include OOM coverage */
    g_cdd_cst_alloc_token_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_include(&b, "test1.h", 1));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    g_cdd_cst_alloc_token_fail = 3;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_include(&b, "test1.h", 1));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    g_cdd_cst_alloc_token_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_include(&b, "test2.h", 0));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    g_cdd_cst_alloc_token_fail = 3;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_bld_include(&b, "test2.h", 0));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;
  }
#endif

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_builder_exhaustive(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *node = NULL;
  cdd_cst_node_t *new_node = NULL;
  cdd_cst_builder_t b;
  int rc;

  cdd_cst_parse(az_span_create_from_str("int x;"), &tree);
  node = tree->root;
  cdd_cst_builder_init(&b, tree, node);

  /* pool_string expansion failure */
#ifdef CDD_BUILD_TESTS
  tree->string_capacity = 0;
  g_cdd_cst_alloc_token_fail = 1;
  {
    /* cdd_cst_bld_int calls pool_string directly before allocating token */
    rc = cdd_cst_bld_int(&b, 1234567);
    ASSERT(rc != 0);
  }
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;
#endif

  /* pool_string invalid argument coverage */
  {
    cdd_cst_tree_t *old_tree = b.tree;
    b.tree = NULL;
    rc = cdd_cst_bld_int(&b, 123);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    b.tree = old_tree;
    b.error_state = 0;
  }

  /* cdd_cst_bld_number failure */
#ifdef CDD_BUILD_TESTS
  g_cdd_cst_alloc_token_fail = 1;
  rc = cdd_cst_bld_int(&b, 123);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;
#endif

  /* cdd_cst_bld_string failure */
#ifdef CDD_BUILD_TESTS
  g_cdd_cst_alloc_token_fail = 1;
  rc = cdd_cst_bld_string(&b, "test");
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;
#endif

  /* cdd_cst_bld_else */
  rc = cdd_cst_bld_else(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  b.error_state = CDD_C_ERROR_UNKNOWN;
  rc = cdd_cst_bld_else(&b);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  b.error_state = 0;
  rc = cdd_cst_bld_else(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  /* cdd_cst_bld_endif */
  rc = cdd_cst_bld_endif(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  b.error_state = CDD_C_ERROR_UNKNOWN;
  rc = cdd_cst_bld_endif(&b);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  b.error_state = 0;
  rc = cdd_cst_bld_endif(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  /* cdd_cst_bld_extern_c_open */
  rc = cdd_cst_bld_extern_c_open(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  b.error_state = CDD_C_ERROR_UNKNOWN;
  rc = cdd_cst_bld_extern_c_open(&b);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  b.error_state = 0;
  rc = cdd_cst_bld_extern_c_open(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  /* cdd_cst_bld_extern_c_close */
  rc = cdd_cst_bld_extern_c_close(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  b.error_state = CDD_C_ERROR_UNKNOWN;
  rc = cdd_cst_bld_extern_c_close(&b);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  b.error_state = 0;
  rc = cdd_cst_bld_extern_c_close(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  /* snippet failure */
#ifdef CDD_BUILD_TESTS
  g_cdd_cst_alloc_token_fail = 5;
  rc = cdd_cst_bld_snippet(&b, "int y;");
  ASSERT(rc != 0);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_token_fail = 2;
  rc = cdd_cst_bld_snippet(&b, "int y;");
  ASSERT(rc != 0);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;
#endif

  /* format failure */
#ifdef CDD_BUILD_TESTS
  rc = cdd_cst_quote(&b, "int %s rest", "y");
  ASSERT_EQ(0, rc);

  g_cdd_cst_alloc_token_fail = 5;
  rc = cdd_cst_quote(&b, "int %s;", "y");
  ASSERT(rc != 0);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_token_fail = 5;
  rc = cdd_cst_quote(&b, "int %d;", 10);
  ASSERT(rc != 0);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  {
    cdd_cst_node_t *dummy;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &dummy);
    g_cdd_cst_realloc_fail = 1;
    rc = cdd_cst_quote(&b, "int %n;", dummy);
    ASSERT(rc != 0);
    g_cdd_cst_realloc_fail = 0;
    b.error_state = 0;
    cdd_cst_free_node_only(dummy);
  }
#endif

/* cdd_cst_bld_trivia */
#ifdef CDD_BUILD_TESTS
  {
    cdd_cst_tree_t *empty_tree = NULL;
    cdd_cst_node_t *empty_root = NULL;
    cdd_cst_builder_t empty_b;
    cdd_cst_parse(az_span_create_from_str(""), &empty_tree);
    cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &empty_root);
    cdd_cst_builder_init(&empty_b, empty_tree, empty_root);

    g_cdd_cst_alloc_token_fail = 1;
    rc = cdd_cst_bld_block_comment(&empty_b, "comment");
    ASSERT(rc != 0);
    g_cdd_cst_alloc_token_fail = 0;
    empty_b.error_state = 0;

    g_cdd_cst_alloc_token_fail = 2;
    rc = cdd_cst_bld_block_comment(&empty_b, "comment");
    ASSERT(rc != 0);
    g_cdd_cst_alloc_token_fail = 0;
    empty_b.error_state = 0;

    g_cdd_cst_alloc_token_fail = 3;
    rc = cdd_cst_bld_block_comment(&empty_b, "comment");
    ASSERT(rc != 0);
    g_cdd_cst_alloc_token_fail = 0;
    empty_b.error_state = 0;

    g_cdd_cst_alloc_token_fail = 4;
    rc = cdd_cst_bld_block_comment(&empty_b, "comment");
    ASSERT(rc != 0);
    g_cdd_cst_alloc_token_fail = 0;
    empty_b.error_state = 0;

    empty_b.tree = NULL;
    rc = cdd_cst_bld_block_comment(&empty_b, "comment");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    empty_b.tree = empty_tree;
    empty_b.error_state = 0;

    cdd_cst_free_node_only(empty_root);
    cdd_cst_tree_free(empty_tree);
  }
#endif

  cdd_cst_bld_newline(&b);
  cdd_cst_bld_line_comment(&b, "// comment");
  cdd_cst_bld_block_comment(&b, "/* comment */");

  {
    cdd_cst_node_t *parent_node;
    cdd_trivia_t *triv;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &parent_node);
    cdd_cst_append_child_node(parent_node, b.target_node);

    ASSERT_EQ(0, cdd_cst_extract_leading_trivia(parent_node, &triv));
    ASSERT_EQ(0, cdd_cst_extract_trailing_trivia(parent_node, &triv));

    cdd_cst_free_node_only(parent_node);
  }

  /* Replace node preserve trivia */
  cdd_cst_tree_free(tree);
  tree = NULL;
  cdd_cst_parse(az_span_create_from_str("int y;"), &tree);
  cdd_cst_builder_init(&b, tree, tree->root->children[0].val.node);
  node = tree->root->children[0].val.node;
  new_node = tree->root->children[0].val.node;

  rc = cdd_cst_replace_node_preserve_trivia(NULL, node, new_node);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_cst_replace_node_preserve_trivia(&b, NULL, new_node);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_cst_replace_node_preserve_trivia(&b, node, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = cdd_cst_replace_node_preserve_trivia(&b, node, new_node);
  printf("rc = %d\\n", rc);
  ASSERT_EQ(0, rc);

#ifdef CDD_BUILD_TESTS
  {
    cdd_cst_node_t *n1 = NULL;
    cdd_cst_node_t *n2 = NULL;
    cdd_cst_node_t *new_n2 = NULL;
    cdd_cst_tree_free(tree);
    tree = NULL;
    cdd_cst_parse(az_span_create_from_str("int y;"), &tree);
    cdd_cst_builder_init(&b, tree, tree->root);

    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n1);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n2);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &new_n2);
    cdd_cst_append_child_node(tree->root, n1);
    cdd_cst_append_child_node(n1, n2);

    g_cdd_alloc_fail = 2;
    g_cdd_cst_alloc_token_fail = 2;
    rc = cdd_cst_splice_nodes(&b, n1, 0, &new_n2, 1);
    printf("splice_nodes rc=%d\\n", rc);
    g_cdd_alloc_fail = 0;
    g_cdd_cst_alloc_token_fail = 0;
    ASSERT_EQ(0, rc);
  }
#endif

  b.error_state = CDD_C_ERROR_UNKNOWN;
  rc = cdd_cst_replace_node_preserve_trivia(&b, node, new_node);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  b.error_state = 0;

  /* Snippet test for trivia loops */
  {
    cdd_cst_node_t *n1 = NULL;
    cdd_cst_node_t *n2 = NULL;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n1);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n2);

    b.target_node = n1;
    cdd_cst_bld_snippet(
        &b,
        "/* l1a */ /* l1b */ /* l1c */ int x /* t1a */ /* t1b */ /* t1c */;");

    b.target_node = n2;
    cdd_cst_bld_snippet(
        &b,
        "/* l2a */ /* l2b */ /* l2c */ int y /* t2a */ /* t2b */ /* t2c */;");

    /* Empty snippet to hit CDD_TOKEN_EOF */
    cdd_cst_bld_snippet(&b, "/* just trivia */");

    {
      char *giant_trivia = (char *)malloc(4500);
      memset(giant_trivia, ' ', 4499);
      giant_trivia[0] = '/';
      giant_trivia[1] = '*';
      giant_trivia[4493] = '*';
      giant_trivia[4494] = '/';
      giant_trivia[4495] = 'i';
      giant_trivia[4496] = 'n';
      giant_trivia[4497] = 't';
      giant_trivia[4498] = ';';
      giant_trivia[4499] = '\0';
      cdd_cst_bld_snippet(&b, giant_trivia);

      memset(giant_trivia, ' ', 4499);
      giant_trivia[0] = 'i';
      giant_trivia[1] = 'n';
      giant_trivia[2] = 't';
      giant_trivia[3] = ' ';
      giant_trivia[4] = '/';
      giant_trivia[5] = '*';
      giant_trivia[4497] = '*';
      giant_trivia[4498] = '/';
      giant_trivia[4499] = '\0';
      cdd_cst_bld_snippet(&b, giant_trivia);
      free(giant_trivia);
    }

    rc = cdd_cst_transfer_trivia(n1, n2);
    ASSERT_EQ(0, rc);
  }

  {
    /* Test giant snippet format string to hit buffer limits */
    char *giant_format = (char *)malloc(3000);
    memset(giant_format, 'a', 2999);
    giant_format[2500] = '%';
    giant_format[2501] = '%';
    giant_format[2502] = '%';
    giant_format[2503] = 't';
    giant_format[2999] = '\0';
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &node);
    b.target_node = node;
    rc = cdd_cst_quote(&b, giant_format, "some_token");
    if (rc != 0)
      printf("cdd_cst_quote failed with %d\\n", rc);
    ASSERT_EQ(0, rc);
    free(giant_format);

    /* Test invalid format specifier %x */
    rc = cdd_cst_quote(&b, "invalid %x specifier");
    ASSERT_EQ(0, rc); /* Skips unknown specifier */
    b.error_state = 0;

    /* Test %n with NULL node */
    rc = cdd_cst_quote(&b, "null node %n", NULL);
    ASSERT_EQ(0, rc); /* It shouldn't set error_state if we just skip it, but
                         let's see */
    b.error_state = 0;

    /* Test format ending with % */
    rc = cdd_cst_quote(&b, "invalid format ending with %");
    ASSERT_EQ(0, rc);
    b.error_state = 0;
  }

  /* OOM test for cdd_cst_bld_block_comment */
  {
#ifdef CDD_BUILD_TESTS
    g_cdd_alloc_fail = 1;
    rc = cdd_cst_bld_block_comment(&b, "test");
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    b.error_state = 0;
#endif
  }

  {
    cdd_trivia_t *lead = NULL;
    rc = cdd_cst_extract_leading_trivia(NULL, &lead);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_cst_extract_trailing_trivia(NULL, &lead);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }

  {}

  {
    /* Test extract trivia from node with NO tokens */
    cdd_cst_node_t *empty_node;
    cdd_trivia_t *tr = NULL;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &empty_node);
    cdd_cst_extract_leading_trivia(empty_node, &tr);
    cdd_cst_extract_trailing_trivia(empty_node, &tr);
  }

  /* Test trailing trivia transfer where replacement already has trailing trivia
   */
  {
    cdd_cst_node_t *target_with_trail = NULL;
    cdd_cst_node_t *replacement_with_trail = NULL;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &target_with_trail);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &replacement_with_trail);

    b.target_node = target_with_trail;
    cdd_cst_bld_block_comment(&b, "lead1a");
    cdd_cst_bld_block_comment(&b, "lead1b");
    cdd_cst_bld_block_comment(&b, "lead1c");
    cdd_cst_bld_block_comment(&b, "lead1d");

    /* Test comment after a node */
    {
      cdd_cst_node_t *n1 = NULL;
      cdd_cst_node_t *n2 = NULL;
      cdd_cst_alloc_node(CDD_CST_STATEMENT, &n1);
      cdd_cst_alloc_node(CDD_CST_STATEMENT, &n2);

      b.target_node = n1;
      rc = cdd_cst_append_child_node(n1, n2);
      ASSERT_EQ(0, rc);
      rc = cdd_cst_bld_block_comment(&b, "comment_after_node");
      ASSERT_EQ(0, rc);

      /* Let's also do a block comment on a token with 3 existing trailing
       * trivias */
      cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "tok_for_trail");
      cdd_cst_bld_block_comment(&b, "trail1");
      cdd_cst_bld_block_comment(&b, "trail2");
      cdd_cst_bld_block_comment(&b, "trail3");
      cdd_cst_bld_block_comment(&b, "trail4");
    }
    cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "t1");
    /* Attach trailing trivia explicitly */
    {
      cdd_token_t *t =
          target_with_trail->children[target_with_trail->num_children - 1]
              .val.token;
      if (t) {
        cdd_trivia_t *tr1 = calloc(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *tr2 = calloc(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *tr3 = calloc(1, sizeof(cdd_trivia_t));
        tr1->next = tr2;
        tr2->next = tr3;
        t->trailing_trivia = tr1;
      }
    }

    b.target_node = replacement_with_trail;
    cdd_cst_bld_block_comment(&b, "lead2a");
    cdd_cst_bld_block_comment(&b, "lead2b");
    cdd_cst_bld_block_comment(&b, "lead2c");
    cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "r1");
    {
      cdd_token_t *t = replacement_with_trail
                           ->children[replacement_with_trail->num_children - 1]
                           .val.token;
      if (t) {
        cdd_trivia_t *tr1 = calloc(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *tr2 = calloc(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *tr3 = calloc(1, sizeof(cdd_trivia_t));
        tr1->next = tr2;
        tr2->next = tr3;
        t->trailing_trivia = tr1;
      }
    }

    /* We also need to add them to root so they have parents */
    cdd_cst_append_child_node(tree->root, target_with_trail);

    rc = cdd_cst_replace_node_preserve_trivia(&b, target_with_trail,
                                              replacement_with_trail);
    ASSERT_EQ(0, rc);
  }

  /* Test empty nodes in trivia extraction */
  {
    cdd_cst_node_t *parent_node = NULL;
    cdd_cst_node_t *empty_node1 = NULL;
    cdd_cst_node_t *empty_node2 = NULL;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &parent_node);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &empty_node1);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &empty_node2);

    cdd_cst_append_child_node(parent_node, empty_node1);
    b.target_node = parent_node;
    cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "mid");
    cdd_cst_append_child_node(parent_node, empty_node2);

    /* Extraction should skip empty_node1/2 and find mid */
    {
      cdd_trivia_t *lead = NULL;
      cdd_trivia_t *trail = NULL;
      cdd_cst_extract_leading_trivia(parent_node, &lead);
      cdd_cst_extract_trailing_trivia(parent_node, &trail);
    }
  }
  rc = cdd_cst_replace_node_preserve_trivia(&b, tree->root, new_node);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  b.error_state = 0;

  /* cdd_cst_bld_children */
#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_cst_alloc_token_fail;
    g_cdd_cst_alloc_token_fail = 1;
    rc = cdd_cst_splice_nodes(&b, node, 0, &new_node, 1);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    g_cdd_alloc_fail = 1;
    rc = cdd_cst_splice_nodes(&b, node, 0, &new_node, 1);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;
    b.error_state = 0;

    g_cdd_cst_realloc_fail = 1;
    rc = cdd_cst_splice_nodes(&b, node, 0, &new_node, 1);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_cst_realloc_fail = 0;
    b.error_state = 0;
  }
#endif

  b.error_state = CDD_C_ERROR_UNKNOWN;
  rc = cdd_cst_splice_nodes(&b, node, 0, &new_node, 1);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  b.error_state = 0;

  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_cst_builder_long_token(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *node = NULL;
  cdd_cst_builder_t b;
  char long_tok[2055];
  int i;
  for (i = 0; i < 2054; i++)
    long_tok[i] = 'a';
  long_tok[2054] = '\0';
  cdd_cst_parse(az_span_create_from_str(""), &tree);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &node);
  tree->root = node;
  cdd_cst_builder_init(&b, tree, node);
  ASSERT_EQ(0, cdd_cst_bld_snippet(&b, long_tok));
  cdd_cst_tree_free(tree);
  PASS();
}

SUITE(cdd_cst_builder_suite) {
  RUN_TEST(test_cdd_cst_builder_basic);
  RUN_TEST(test_cdd_cst_builder_extra);
  RUN_TEST(test_cdd_cst_builder_errors_extra);
  RUN_TEST(test_cdd_cst_builder_macros);
  RUN_TEST(test_cdd_cst_builder_quote);
  RUN_TEST(test_cdd_cst_builder_quote_errors);
  RUN_TEST(test_cdd_cst_builder_snippet);
  RUN_TEST(test_cdd_cst_builder_comments);
  RUN_TEST(test_cdd_cst_builder_errors);
  RUN_TEST(test_cdd_cst_builder_trivia_and_splice);
  RUN_TEST(test_cdd_cst_builder_extra);
  RUN_TEST(test_cdd_cst_builder_oom);
  RUN_TEST(test_cdd_cst_builder_punct_all);
  RUN_TEST(test_cdd_cst_builder_exhaustive);
  RUN_TEST(test_cdd_cst_builder_long_token);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_BUILDER_H */
