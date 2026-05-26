extern int g_fail_io_after;
extern int g_io_calls;
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
  char *out = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_builder_init(&b, tree, root);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(0, cdd_cst_builder_has_error(&b));

  rc = cdd_cst_bld_ident(&b, "int");
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_space(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_ident(&b, "main");
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_punct(&b, "(");
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_punct(&b, ")");
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_space(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_block_open(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_indent(&b, b.indent_level);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_ident(&b, "return");
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_space(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_int(&b, 0);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_punct(&b, ";");
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_block_close(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(out, "int main()") != NULL);
  ASSERT(strstr(out, "return 0;") != NULL);

  free(out);
  rc = cdd_cst_builder_free(&b);
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
  char *out = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_builder_init(&b, tree, root);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_include(&b, "stdio.h", 1);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_ifndef(&b, "TEST_MACRO");
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_ifdef(&b, "TEST_MACRO2");
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_else(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_endif(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_endif(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_extern_c_open(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_extern_c_close(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
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
  char *out = NULL;
  cdd_cst_node_t *injected_node = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &injected_node);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_builder_init(&b, tree, root);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_quote(&b, "int %s = %d; %% %n", "my_var", 42, injected_node);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
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
  char *out = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_builder_init(&b, tree, root);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_snippet(&b, "void func() { return; }");
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
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
  char *out = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  ASSERT(tree != NULL);

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  ASSERT_EQ(0, rc);
  tree->root = root;

  rc = cdd_cst_builder_init(&b, tree, root);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_bld_block_comment(&b, " block ");
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_line_comment(&b, " line");
  ASSERT_EQ(0, rc);
  rc = cdd_cst_bld_newline(&b);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
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

  rc = cdd_cst_builder_init(NULL, NULL, NULL);
  ASSERT_EQ(EINVAL, rc);

  rc = cdd_cst_builder_free(NULL);
  ASSERT_EQ(EINVAL, rc);

  rc = cdd_cst_builder_has_error(NULL);
  ASSERT_EQ(1, rc);

  b.error_state = 1;
  ASSERT_EQ(1, cdd_cst_builder_has_error(&b));

  rc = cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "test");
  ASSERT_EQ(1, rc); /* Returns error_state */

  rc = cdd_cst_builder_set_insert_point(&b, NULL);
  ASSERT_EQ(EINVAL, rc);

  rc = cdd_cst_bld_space(NULL);
  ASSERT_EQ(EINVAL, rc);
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
  cdd_trivia_t *lead;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));

  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  rc = cdd_cst_alloc_node(CDD_CST_STATEMENT, &target_node);
  rc = cdd_cst_alloc_node(CDD_CST_STATEMENT, &replacement_node);
  rc = cdd_cst_alloc_node(CDD_CST_STATEMENT, &spliced_node);

  rc = cdd_cst_builder_init(&b, tree, root);
  rc = cdd_cst_append_child_node(root, target_node);

  rc = cdd_cst_extract_leading_trivia(target_node, &lead);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_extract_trailing_trivia(target_node, &lead);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_transfer_trivia(target_node, replacement_node);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_replace_node_preserve_trivia(&b, target_node, replacement_node);
  ASSERT_EQ(0, rc);

  {
    cdd_cst_node_t *nodes[1];
    nodes[0] = spliced_node;
    rc = cdd_cst_splice_nodes(&b, replacement_node, 0, nodes, 1);
    ASSERT_EQ(0, rc);
  }

  /* Error checks */
  rc = cdd_cst_extract_leading_trivia(NULL, NULL);
  ASSERT_EQ(EINVAL, rc);
  rc = cdd_cst_extract_trailing_trivia(NULL, NULL);
  ASSERT_EQ(EINVAL, rc);
  rc = cdd_cst_transfer_trivia(NULL, NULL);
  ASSERT_EQ(EINVAL, rc);
  rc = cdd_cst_replace_node_preserve_trivia(NULL, NULL, NULL);
  ASSERT_EQ(EINVAL, rc);
  rc = cdd_cst_splice_nodes(NULL, NULL, 0, NULL, 1);
  ASSERT_EQ(EINVAL, rc);
  rc = cdd_cst_splice_nodes(&b, replacement_node, 0, NULL, 0);
  ASSERT_EQ(0, rc);

  cdd_cst_builder_free(&b);
  cdd_cst_free_node_only(target_node);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_builder_extra(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  int rc;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(*tree));
  rc = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  /* Null checks */
  ASSERT_EQ(EINVAL, cdd_cst_builder_init(NULL, NULL, NULL));
  ASSERT_EQ(1, cdd_cst_builder_has_error(NULL));

  ASSERT_EQ(EINVAL, cdd_cst_builder_set_insert_point(NULL, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_bld_token(NULL, CDD_TOKEN_EOF, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_bld_indent(NULL, 1));
  ASSERT_EQ(EINVAL, cdd_cst_bld_snippet(NULL, NULL));

  ASSERT_EQ(EINVAL, cdd_cst_bld_line_comment(NULL, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_bld_block_comment(NULL, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_bld_ident(NULL, NULL));

  ASSERT_EQ(EINVAL, cdd_cst_bld_punct(NULL, NULL));

  ASSERT_EQ(EINVAL, cdd_cst_bld_string(NULL, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_bld_space(NULL));
  ASSERT_EQ(EINVAL, cdd_cst_bld_newline(NULL));

  ASSERT_EQ(EINVAL, cdd_cst_extract_leading_trivia(NULL, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_extract_trailing_trivia(NULL, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_transfer_trivia(NULL, NULL));

  rc = cdd_cst_builder_init(&b, tree, root);
  ASSERT_EQ(0, rc);

  /* Test indent */
  rc = cdd_cst_bld_indent(&b, 2);
  ASSERT_EQ(0, rc);

  /* Test insert point */
  rc = cdd_cst_builder_set_insert_point(&b, root);
  ASSERT_EQ(0, rc);

  /* Test error state */
  b.error_state = ENOMEM;
  ASSERT_EQ(ENOMEM, cdd_cst_builder_set_insert_point(&b, root));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_token(&b, CDD_TOKEN_EOF, "eof"));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_indent(&b, 1));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_snippet(&b, "snippet"));

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_builder_quote_errors(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_builder_t b;
  char buf[3000];

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  cdd_cst_builder_init(&b, tree, root);

  ASSERT_EQ(EINVAL, cdd_cst_quote(NULL, "abc"));
  ASSERT_EQ(EINVAL, cdd_cst_quote(&b, NULL));

  b.error_state = EINVAL;
  ASSERT_EQ(EINVAL, cdd_cst_quote(&b, "abc"));
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
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;

  cdd_cst_builder_init(&b, tree, root);

  /* NULL checks */
  ASSERT_EQ(EINVAL, cdd_cst_bld_token(NULL, CDD_TOKEN_IDENTIFIER, "a"));
  ASSERT_EQ(EINVAL, cdd_cst_bld_int(NULL, 1));
  ASSERT_EQ(EINVAL, cdd_cst_bld_punct(NULL, ";"));
  ASSERT_EQ(EINVAL, cdd_cst_bld_block_open(NULL));
  ASSERT_EQ(EINVAL, cdd_cst_bld_block_close(NULL));

  /* Force error state */
  b.error_state = ENOMEM;
  ASSERT_EQ(ENOMEM, cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "a"));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_int(&b, 1));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_punct(&b, ";"));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_block_open(&b));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_block_close(&b));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_line_comment(&b, "test"));

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
extern int g_cdd_cst_alloc_token_fail;
extern int g_cdd_cst_realloc_fail;
#endif

TEST test_cdd_cst_builder_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_builder_t b;
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  int rc;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
  tree->root = root;
  cdd_cst_builder_init(&b, tree, root);

  g_cdd_cst_alloc_token_fail = 1;
  ASSERT_EQ(ENOMEM, cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "a"));
  g_cdd_cst_alloc_token_fail = 0;

  b.error_state = 0;

  /* Trigger realloc in track_synthesized */
  tree->synthesized_capacity = 0;
  tree->num_synthesized = 0;
  g_cdd_cst_realloc_fail = 1;
  {
    cdd_token_t *out_tok = NULL;
    ASSERT_EQ(ENOMEM, cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "test",
                                           &out_tok));
  }
  g_cdd_cst_realloc_fail = 0;
  g_cdd_cst_realloc_fail = 1;
  {
    cdd_cst_node_t *n1 = NULL;
    cdd_cst_node_t *n2 = NULL;
    cdd_token_t tok = {0};
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n1);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n2);
    ASSERT_EQ(ENOMEM, cdd_cst_append_child_node(n1, n2));
    ASSERT_EQ(ENOMEM, cdd_cst_append_child_token(n1, &tok));
    cdd_cst_free_node_only(n1);
    cdd_cst_free_node_only(n2);
  }
  g_cdd_cst_realloc_fail = 0;

  /* To trigger append_child_token failure inside cdd_cst_bld_token */
  cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "success1");
  b.target_node->capacity = 0;
  b.target_node->num_children = 0;
  g_cdd_cst_realloc_fail = 1;
  ASSERT_EQ(ENOMEM, cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "trigger"));
  g_cdd_cst_realloc_fail = 0;
  b.error_state = 0;

  /* To trigger append_child_node failure inside cdd_cst_bld_block_open */
  cdd_cst_bld_token(&b, CDD_TOKEN_IDENTIFIER, "success2");
  b.target_node->capacity = 0;
  b.target_node->num_children = 0;
  g_cdd_cst_realloc_fail = 1;
  ASSERT_EQ(ENOMEM, cdd_cst_bld_block_open(&b));
  g_cdd_cst_realloc_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_node_fail = 1;
  {
    cdd_cst_node_t *n1 = NULL;
    ASSERT_EQ(ENOMEM, cdd_cst_alloc_node(CDD_CST_STATEMENT, &n1));
  }
  g_cdd_cst_alloc_node_fail = 0;

  /* For trivia array OOM */
  g_cdd_cst_alloc_token_fail = 1;
  ASSERT_EQ(ENOMEM, cdd_cst_bld_line_comment(&b, "test"));
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  cdd_cst_tree_free(tree);
#endif
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern int g_cdd_cst_alloc_token_fail;
extern int g_cdd_cst_realloc_fail;
#endif

TEST test_cdd_cst_builder_punct_all(void) {
  cdd_cst_builder_t b;
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
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

  /* Test all remaining builder functions for EINVAL and error_state */
  ASSERT_EQ(EINVAL, cdd_cst_bld_include(NULL, "a", 0));
  ASSERT_EQ(EINVAL, cdd_cst_bld_ifndef(NULL, "a"));
  ASSERT_EQ(EINVAL, cdd_cst_bld_ifdef(NULL, "a"));
  ASSERT_EQ(EINVAL, cdd_cst_bld_line_comment(NULL, "a"));
  ASSERT_EQ(EINVAL, cdd_cst_bld_block_comment(NULL, "a"));

  b.error_state = ENOMEM;
  ASSERT_EQ(ENOMEM, cdd_cst_bld_include(&b, "a", 0));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_ifndef(&b, "a"));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_ifdef(&b, "a"));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_line_comment(&b, "a"));
  ASSERT_EQ(ENOMEM, cdd_cst_bld_block_comment(&b, "a"));
  b.error_state = 0;

  cdd_cst_bld_include(&b, "a", 0);
  cdd_cst_bld_ifndef(&b, "a");
  cdd_cst_bld_ifdef(&b, "a");

#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_cst_alloc_token_fail;
    /* pool_string_safe OOM */
    g_cdd_cst_alloc_token_fail = 1;
    ASSERT_EQ(ENOMEM, cdd_cst_bld_ident(&b, "a"));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    /* space OOM inside indent */
    g_cdd_cst_alloc_token_fail = 1;
    ASSERT_EQ(ENOMEM, cdd_cst_bld_indent(&b, 1));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    /* include OOM coverage */
    g_cdd_cst_alloc_token_fail = 1;
    ASSERT_EQ(ENOMEM, cdd_cst_bld_include(&b, "test1.h", 1));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    g_cdd_cst_alloc_token_fail = 1;
    ASSERT_EQ(ENOMEM, cdd_cst_bld_include(&b, "test2.h", 0));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;
  }
#endif

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
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
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_BUILDER_H */
