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
#include "c_cdd/memory.h"
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

  tree =
      (cdd_cst_tree_t *)C_CDD_CALLOC(1, (unsigned long)sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_builder_init(&b, tree, root);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  out_has = -1;
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

  C_CDD_FREE(out);
  rc = cdd_cst_builder_free(&b);
  printf("rc = %d\n", rc);
  ASSERT_EQ(0, rc);

  /* Manual tree free since we built it from scratch without a lexer list */

  cdd_cst_free_node_only(NULL);
  cdd_cst_tree_free(tree);

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

  tree =
      (cdd_cst_tree_t *)C_CDD_CALLOC(1, (unsigned long)sizeof(cdd_cst_tree_t));
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

  C_CDD_FREE(out);
  cdd_cst_builder_free(&b);
  cdd_cst_tree_free(tree);

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

  tree =
      (cdd_cst_tree_t *)C_CDD_CALLOC(1, (unsigned long)sizeof(cdd_cst_tree_t));
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

  C_CDD_FREE(out);
  cdd_cst_builder_free(&b);
  cdd_cst_tree_free(tree);

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

  tree =
      (cdd_cst_tree_t *)C_CDD_CALLOC(1, (unsigned long)sizeof(cdd_cst_tree_t));
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

  C_CDD_FREE(out);
  cdd_cst_builder_free(&b);
  cdd_cst_tree_free(tree);

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

  tree =
      (cdd_cst_tree_t *)C_CDD_CALLOC(1, (unsigned long)sizeof(cdd_cst_tree_t));
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

  C_CDD_FREE(out);
  cdd_cst_builder_free(&b);
  cdd_cst_tree_free(tree);

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

  PASS();
}

TEST test_cdd_cst_builder_extra(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  int rc;
  int out_has = -1;
  (void)out_has;

  tree = (cdd_cst_tree_t *)C_CDD_CALLOC(1, sizeof(*tree));
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

  {
    int i;
    for (i = 1; i < 20; i++) {
      g_cdd_cst_alloc_token_fail = i;
      rc = cdd_cst_bld_snippet(&b, "int x = 1;");
      if (rc == CDD_C_SUCCESS) {
        break;
      }
      b.error_state = CDD_C_SUCCESS;
    }
    g_cdd_cst_alloc_token_fail = 0;
  }

  {
    /* Append multiple block comments to cover the while loop in trivia
     * appending */
    cdd_cst_bld_block_comment(&b, "1");
    cdd_cst_bld_block_comment(&b, "2");
    cdd_cst_bld_block_comment(&b, "3");
  }

  {
    cdd_cst_tree_t *tree2 = NULL;
    cdd_cst_parse(az_span_create_from_str("/* 1 */ /* 2 */ /* 3 */ int x;"),
                  &tree2);
    if (tree2 && tree2->root) {
      cdd_cst_node_t *node2 = NULL;
      cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &node2);
      cdd_cst_builder_t b2;
      cdd_cst_builder_init(&b2, tree2, node2);
      cdd_cst_bld_token(&b2, CDD_TOKEN_IDENTIFIER, "test");

      cdd_cst_transfer_trivia(tree2->root, node2);
      cdd_cst_tree_free(tree2);
    }
  }

  {
    cdd_cst_node_t *node2 = NULL;
    cdd_cst_node_t *empty1 = NULL;
    cdd_cst_node_t *empty2 = NULL;
    cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &node2);
    cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &empty1);
    cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &empty2);
    if (node2 && empty1 && empty2) {
      cdd_cst_builder_t b2;
      cdd_cst_builder_init(&b2, tree, node2);

      cdd_cst_append_child_node(node2, empty1);
      cdd_cst_append_child_node(node2, empty2);

      /* Source node setup */
      cdd_cst_bld_token(&b2, CDD_TOKEN_IDENTIFIER, "test");
      cdd_cst_bld_block_comment(&b2, " src_trail");

      cdd_cst_append_child_node(node2, empty1); /* now empty1 is at the end */
      cdd_cst_append_child_node(node2, empty2); /* empty2 is at the end */

      /* Target node setup (fresh node) */
      cdd_cst_node_t *target_node = NULL;
      cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &target_node);
      cdd_cst_builder_t b3;
      cdd_cst_builder_init(&b3, tree, target_node);
      cdd_cst_bld_token(&b3, CDD_TOKEN_IDENTIFIER, "target");

      /* Transfer from node2 to target_node.
         target_node has tokens with NO trailing trivia. node2 has trailing
         trivia. This will trigger the t_last->trailing_trivia == NULL path!
      */
      cdd_cst_transfer_trivia(node2, target_node);

      /* Transfer AGAIN. Now target_node HAS trailing trivia (from first
         transfer). This will trigger the t_last->trailing_trivia != NULL path!
         And since node2 still has its trailing trivia (transfer doesn't clear
         source), wait, transfer_trivia MOVES trivia! So node2 no longer has it!
         So we must add trivia to node2 again! */
      cdd_cst_bld_block_comment(&b2, " src_trail_2");
      cdd_cst_transfer_trivia(node2, target_node);

      /* And to trigger the while loop for tail->next, target_node must have TWO
         trivias! We just transferred one, so now it has TWO! Let's transfer a
         THIRD time! */
      cdd_cst_bld_block_comment(&b2, " src_trail_3");
      cdd_cst_transfer_trivia(node2, target_node);

      {
        /* Direct manipulation to cover missing branches */
        cdd_cst_node_t *source = NULL;
        cdd_cst_node_t *target = NULL;
        cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &source);
        cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &target);

        if (source && target) {
          cdd_token_t t_src = {0};
          cdd_token_t t_tgt = {0};
          cdd_cst_child_t src_child = {0};
          cdd_cst_child_t tgt_child = {0};
          cdd_cst_child_t empty_child = {0};
          cdd_trivia_t t1 = {0}, t2 = {0}, t3 = {0};
          cdd_trivia_t l1 = {0}, l2 = {0}, l3 = {0};

          t1.next = &t2;
          l1.next = &l2;
          t_tgt.trailing_trivia = &t1;
          t_tgt.leading_trivia = &l1;

          t_src.trailing_trivia = &t3;
          t_src.leading_trivia = &l3;

          empty_child.kind = CDD_CST_CHILD_NODE;
          empty_child.val.node = empty1;

          src_child.kind = CDD_CST_CHILD_TOKEN;
          src_child.val.token = &t_src;
          source->children =
              (cdd_cst_child_t *)C_CDD_MALLOC(sizeof(cdd_cst_child_t) * 2);
          source->children[0] =
              empty_child; /* empty child at the start too, so both directions
                              hit the recursion miss */
          source->children[1] = empty_child;
          source->num_children = 2;
          source->capacity = 2;

          tgt_child.kind = CDD_CST_CHILD_TOKEN;
          tgt_child.val.token = &t_tgt;
          target->children =
              (cdd_cst_child_t *)C_CDD_MALLOC(sizeof(cdd_cst_child_t) * 2);
          target->children[0] = empty_child; /* empty child at the start */
          target->children[1] = empty_child;
          target->num_children = 2;
          target->capacity = 2;

          cdd_cst_transfer_trivia(source, target);
          cdd_cst_transfer_trivia(target, source);

          source->children[0] = src_child;
          target->children[1] = tgt_child;
          cdd_cst_transfer_trivia(source, target);
          cdd_cst_transfer_trivia(target, source);

          /* Clear tokens to hit NOT_FOUND branches in recursive find token
           * calls */
          source->children[0] = empty_child;
          source->children[1] = empty_child;
          target->children[0] = empty_child;
          target->children[1] = empty_child;

          cdd_cst_transfer_trivia(source, target);
          cdd_cst_transfer_trivia(target, source);

          /* target needs to hit the find_last_token backwards search. It goes
             backwards. So it checks children[1] then children[0]. If
             children[1] is a node with no tokens, it continues to children[0].
           */
          target->children[1] = empty_child; /* no tokens */
          target->children[0] = src_child;   /* has tokens */
          target->num_children = 2;
          source->children[1] = empty_child; /* no tokens */
          source->children[0] = src_child;   /* has tokens */
          source->num_children = 2;
          cdd_cst_transfer_trivia(source, target);

          source->num_children = 0; /* loop never entered */
          cdd_cst_transfer_trivia(source, target);
          cdd_cst_transfer_trivia(target, source);

          {
            cdd_trivia_t *out_l = NULL;
            cdd_trivia_t *out_t = NULL;
            cdd_cst_extract_leading_trivia(source, &out_l);
            cdd_cst_extract_trailing_trivia(source, &out_t);
          }

          /* Cover edge case: node with children that are ALL empty nodes */
          source->num_children = 2;
          source->children[0] = empty_child;
          source->children[1] = empty_child;
          target->num_children = 2;
          target->children[0] = empty_child;
          target->children[1] = empty_child;
          cdd_cst_transfer_trivia(source, target);
          cdd_cst_transfer_trivia(target, source);

          {
            cdd_token_t *t = NULL;
            find_first_token_mutate(source, &t);
            find_last_token_mutate(source, &t);
          }
        }
        C_CDD_FREE(source->children);
        C_CDD_FREE(source);
        C_CDD_FREE(target->children);
        C_CDD_FREE(target);
      }

      cdd_cst_replace_node_preserve_trivia(
          &b2, empty1, empty2); /* also cover replace_node error branch by
                                   triggering error state */

      {
        cdd_cst_node_t *orphan_node = NULL;
        cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &orphan_node);
        cdd_cst_replace_node_preserve_trivia(
            &b2, orphan_node,
            empty2); /* orphan_node not in tree, fails replace_node */
        C_CDD_FREE(orphan_node);
      }

      b2.error_state = 0;
      {
        cdd_cst_node_t *nodes_to_insert[] = {empty1};
        cdd_cst_splice_nodes(&b2, empty2, 100, nodes_to_insert,
                             1); /* out of bounds, fails splice_children */
      }
    }
  }

  cdd_cst_tree_free(tree);

  PASS();
}

TEST test_cdd_cst_builder_quote_errors(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  char buf[3000];

  tree =
      (cdd_cst_tree_t *)C_CDD_CALLOC(1, (unsigned long)sizeof(cdd_cst_tree_t));
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

  tree =
      (cdd_cst_tree_t *)C_CDD_CALLOC(1, (unsigned long)sizeof(cdd_cst_tree_t));
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

  cdd_cst_tree_free(tree);
#endif

  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
#endif

TEST test_cdd_cst_builder_punct_all(void) {
  cdd_cst_builder_t b;
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;

  tree =
      (cdd_cst_tree_t *)C_CDD_CALLOC(1, (unsigned long)sizeof(cdd_cst_tree_t));
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
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
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
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
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
  node = tree->root->children[0].val.node;
  new_node = tree->root->children[0].val.node;

  rc = cdd_cst_replace_node_preserve_trivia(&b, node, new_node);
  printf("rc = %d\\n", rc);
  ASSERT_EQ(0, rc);
  b.error_state = CDD_C_ERROR_UNKNOWN;
  rc = cdd_cst_replace_node_preserve_trivia(&b, node, new_node);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
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

TEST test_cdd_cst_builder_coverage_final(void) {
  cdd_cst_builder_t b;
  cdd_cst_tree_t *tree =
      (cdd_cst_tree_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = NULL;

  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
  cdd_cst_builder_init(&b, tree, root);

#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
  b.indent_level = 2;
  g_cdd_cst_alloc_token_fail = 1;
  cdd_cst_bld_block_close(&b);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_token_fail = 2;
  cdd_cst_bld_block_close(&b);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  {
    char super_long[4100];
    memset(super_long, ' ', 2050);
    super_long[2050] = '\0';
    strcat(super_long, "int x;");

    g_cdd_cst_alloc_token_fail = 1;
    cdd_cst_bld_snippet(&b, super_long);
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    memset(super_long, 'A', 4000);
    super_long[0] = '/';
    super_long[1] = '*';
    super_long[3998] = '*';
    super_long[3999] = '/';
    super_long[4000] = '\0';

    g_cdd_cst_alloc_token_fail = 1;
    cdd_cst_bld_snippet(&b, super_long);
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;
  }
#endif

  cdd_cst_bld_block_comment(&b, NULL);
  cdd_cst_replace_node_preserve_trivia(&b, NULL, root);
  cdd_cst_replace_node_preserve_trivia(&b, root, NULL);

  {
    cdd_cst_node_t *n2 = NULL;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n2);
    cdd_cst_splice_nodes(&b, NULL, 0, &n2, 1);
    cdd_cst_splice_nodes(&b, root, 0, NULL, 1);
    cdd_cst_free_node_only(n2);
  }

#ifdef CDD_BUILD_TESTS
  {
    cdd_cst_node_t *n2 = NULL;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n2);
    g_cdd_cst_alloc_token_fail = 1;
    cdd_cst_splice_nodes(&b, root, 0, &n2, 1);
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;
    cdd_cst_free_node_only(n2);
  }
#endif

  /* trigger get_last_token and get_first_token on nested nodes */
  {
    cdd_cst_node_t *outer = NULL;
    cdd_cst_node_t *inner = NULL;
    cdd_token_t tok;
    cdd_trivia_t *triv = NULL;
    memset(&tok, 0, sizeof(tok));
    tok.kind = CDD_TOKEN_IDENTIFIER;
    tok.start = (const uint8_t *)"test";
    tok.length = 4;

    cdd_cst_alloc_node(CDD_CST_STATEMENT, &outer);
    cdd_cst_alloc_node(CDD_CST_EXPRESSION, &inner);
    cdd_cst_append_child_node(outer, inner);
    cdd_cst_append_child_token(inner, &tok);

    cdd_cst_extract_leading_trivia(outer, &triv);
    cdd_cst_extract_trailing_trivia(outer, &triv);

    C_CDD_FREE(inner->children);
    C_CDD_FREE(inner);
    C_CDD_FREE(outer->children);
    C_CDD_FREE(outer);
  }

  cdd_cst_builder_free(&b);
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
  RUN_TEST(test_cdd_cst_builder_coverage_final);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_BUILDER_H */
