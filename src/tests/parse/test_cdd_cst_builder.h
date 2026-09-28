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
  int rc;
  int out_has = -1;
  cdd_cst_builder_t b;
  (void)root;
  {
    char *out = NULL;

    tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
    (void)out_has;
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
      (void)err;
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
}

TEST test_cdd_cst_builder_macros(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  int rc;
  int out_has = -1;
  cdd_cst_builder_t b;
  (void)root;
  {
    char *out = NULL;

    tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
    (void)out_has;
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
}

TEST test_cdd_cst_builder_quote(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  int rc;
  int out_has = -1;
  cdd_cst_builder_t b;
  (void)root;
  {
    char *out = NULL;
    cdd_cst_node_t *injected_node = NULL;

    tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
    (void)out_has;
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
}

TEST test_cdd_cst_builder_snippet(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  int rc;
  int out_has = -1;
  cdd_cst_builder_t b;
  (void)root;
  {
    char *out = NULL;

    tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
    (void)out_has;
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
}

TEST test_cdd_cst_builder_comments(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  int rc;
  int out_has = -1;
  cdd_cst_builder_t b;
  (void)root;
  {
    char *out = NULL;

    tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
    (void)out_has;
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
}

TEST test_cdd_cst_builder_errors(void) {
  cdd_cst_builder_t b;
  int rc;
  int out_has = -1;

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
  cdd_cst_builder_t b;
  cdd_cst_node_t *target_node = NULL;
  cdd_cst_node_t *replacement_node = NULL;
  cdd_cst_node_t *spliced_node = NULL;
  int rc;
  int out_has = -1;
  (void)root;
  {
    cdd_trivia_t *lead;

    cdd_cst_tree_t *replacement_node_tree = NULL;
    (void)out_has;
    cdd_cst_parse(az_span_create_from_str(
                      (char *)(size_t) "/* L1 */ /* L2 */ int x; /* T1 */"),
                  &tree);
    root = tree->root;
    target_node = tree->root->children[0].val.node;
    cdd_cst_builder_init(&b, tree, tree->root);

    cdd_cst_parse(az_span_create_from_str((
                      char *)(size_t) "/* NL1 */ float y; /* NT1 */ /* NT2 */"),
                  &replacement_node_tree);
    replacement_node = replacement_node_tree->root->children[0].val.node;

    rc = cdd_cst_extract_leading_trivia(target_node, &lead);
    ASSERT_EQ(0, rc);
    while (lead) {
      cdd_trivia_t *nxt = lead->next;
      C_CDD_FREE(lead);
      lead = nxt;
    }

    rc = cdd_cst_extract_trailing_trivia(target_node, &lead);
    ASSERT_EQ(0, rc);
    while (lead) {
      cdd_trivia_t *nxt = lead->next;
      C_CDD_FREE(lead);
      lead = nxt;
    }

    /* This will transfer L1 L2 to NL1, and T1 to NT1 NT2 */
    rc = cdd_cst_transfer_trivia(target_node, replacement_node);
    ASSERT_EQ(0, rc);

    rc =
        cdd_cst_replace_node_preserve_trivia(&b, target_node, replacement_node);
    ASSERT_EQ(0, rc);

    rc = cdd_cst_alloc_node(CDD_CST_STATEMENT, &spliced_node);

    /* Also test the leak paths (lead without t_first) */
    {
      cdd_cst_node_t *empty_node = NULL;
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
    rc =
        cdd_cst_replace_node_preserve_trivia(&b, target_node, replacement_node);
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
    cdd_cst_free_node(target_node);

    cdd_cst_tree_free(tree);
    if (replacement_node_tree) {
      if (replacement_node_tree->root)
        replacement_node_tree->root->num_children = 0;
      cdd_cst_tree_free(replacement_node_tree);
    }
    g_fail_io_after = -1;
    PASS();
  }
}

TEST test_cdd_cst_builder_extra(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  int rc;
  int out_has = -1;
  cdd_cst_builder_t b;
  (void)root;

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
  char buf[3000];
  cdd_cst_builder_t b;
  (void)root;

  tree = (cdd_cst_tree_t *)calloc(1, (unsigned long)sizeof(cdd_cst_tree_t));
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
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
  (void)root;

  cdd_cst_parse(az_span_create_from_str((char *)(size_t) ""), &tree);
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
/* extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_cdd_cst_realloc_fail; (moved to global) */
#endif

SUITE(cdd_cst_builder_suite) {
  RUN_TEST(test_cdd_cst_builder_basic);
  RUN_TEST(test_cdd_cst_builder_macros);
  RUN_TEST(test_cdd_cst_builder_quote);
  RUN_TEST(test_cdd_cst_builder_snippet);
  RUN_TEST(test_cdd_cst_builder_comments);
  RUN_TEST(test_cdd_cst_builder_errors);
  RUN_TEST(test_cdd_cst_builder_trivia_and_splice);
  RUN_TEST(test_cdd_cst_builder_extra);
  RUN_TEST(test_cdd_cst_builder_quote_errors);
  RUN_TEST(test_cdd_cst_builder_errors_extra);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_BUILDER_H */
