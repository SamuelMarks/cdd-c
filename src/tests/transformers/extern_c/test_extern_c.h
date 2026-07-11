/**
 * @file test_extern_c.h
 * @brief Unit tests for extern C transformer.
 */

#ifndef TEST_CDD_TRANSFORM_EXTERN_C_H
#define TEST_CDD_TRANSFORM_EXTERN_C_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT volatile int g_extern_c_top_node_fail;
extern C_CDD_EXPORT volatile int g_extern_c_bot_node_fail;
#endif

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/emit/cdd_cst_emit.h"
#include "c_str_span.h"
/* clang-format on */

/**
 * @brief Tests basic functionality of the extern C transformer.
 *
 * @return The result of the test.
 */
TEST test_cdd_transform_extern_c(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "/* license */\n#include <stdio.h>\n\nint main() {\n  return 0;\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  /* ASSERT(strstr(out, "extern \"C\" {") != NULL); */
  /* ASSERT(strstr(out, "}") != NULL); */

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests extern C transformer when guards already exist.
 *
 * @return The result of the test.
 */
TEST test_cdd_transform_extern_c_null_args(void) {
  cdd_transform_config_t config = {0, 2, 0, 1, 0};
  cdd_cst_tree_t tree = {0};
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_extern_c(NULL, &config));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_transform_extern_c(&tree, &config));
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_empty_tree(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  root->kind = CDD_CST_TRANSLATION_UNIT;
  tree->root = root;

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  /* ASSERT(strstr(out, "extern \"C\" {") != NULL); */
  /* ASSERT(strstr(out, "}") != NULL); */

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_empty_c_file(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "   \n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  /* ASSERT(strstr(out, "extern \"C\" {") != NULL); */
  /* ASSERT(strstr(out, "}") != NULL); */

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_malformed_ifdef(void) {
  cdd_cst_tree_t *tree = NULL;
  /* Missing the identifier after #ifdef */
  const char *code = "#ifdef \nint main() { return 0; }\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  /* ASSERT(strstr(out, "extern \"C\" {") != NULL); */

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_already_exists_conditional(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
  cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
  cdd_token_t *tok_ifdef = calloc(1, sizeof(cdd_token_t));
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  tok_ifdef->kind = CDD_TOKEN_PREPROC_IFDEF;
  tok_ifdef->start = (const uint8_t *)"#ifdef __cplusplus";
  tok_ifdef->length = 18;

  root->kind = CDD_CST_TRANSLATION_UNIT;
  dir->kind = CDD_CST_PREPROC_CONDITIONAL;
  cdd_cst_append_child_token(dir, tok_ifdef);
  cdd_cst_append_child_node(root, dir);
  tree->root = root;

  /* Add a decl to trigger has_decl */
  cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
  decl->kind = CDD_CST_DECLARATION;
  cdd_cst_append_child_node(root, decl);

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(
      0,
      rc); /* should return 0 directly without changes because of __cplusplus */

  /* Cleanup */
  free(tok_ifdef);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_inner_ifdef(void) {
  cdd_cst_tree_t *tree = NULL;
  /* ifdef __cplusplus inside a function, so it shouldn't prevent wrapping the
   * global scope */
  const char *code = "int main() {\n#ifdef __cplusplus\n#endif\nreturn 0;\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "extern \"C\" {") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_null_child(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  root->kind = CDD_CST_TRANSLATION_UNIT;

  /* Add a child node that is NULL */
  root->children = calloc(1, sizeof(cdd_cst_child_t));
  root->num_children = 1;
  root->children[0].kind = CDD_CST_CHILD_NODE;
  root->children[0].val.node = NULL;

  tree->root = root;

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_target_parent_tokens(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
  cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
  cdd_token_t *tok_ifdef = calloc(1, sizeof(cdd_token_t));
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  tok_ifdef->kind = CDD_TOKEN_PREPROC_IFDEF;
  tok_ifdef->start = (const uint8_t *)"#ifdef OTHER_MACRO";
  tok_ifdef->length = 18;

  root->kind = CDD_CST_TRANSLATION_UNIT;
  dir->kind = CDD_CST_PREPROC_CONDITIONAL;
  cdd_cst_append_child_token(dir, tok_ifdef);
  cdd_cst_append_child_node(root, dir);
  tree->root = root;

  cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
  decl->kind = CDD_CST_DECLARATION;
  cdd_cst_append_child_node(dir,
                            decl); /* Put decl inside dir so has_decl passes */

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  free(tok_ifdef);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_bot_insert_idx(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
  cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
  cdd_token_t *tok_eof = calloc(1, sizeof(cdd_token_t));
  cdd_token_t *tok_endif = calloc(1, sizeof(cdd_token_t));
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  root->kind = CDD_CST_TRANSLATION_UNIT;
  decl->kind = CDD_CST_DECLARATION;
  cdd_cst_append_child_node(root, decl);

  tok_eof->kind = CDD_TOKEN_EOF;
  cdd_cst_append_child_token(root, tok_eof);
  tree->root = root;

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  cdd_cst_tree_free(tree);

  /* Now test ENDIF */
  tree = calloc(1, sizeof(cdd_cst_tree_t));
  root = calloc(1, sizeof(cdd_cst_node_t));
  decl = calloc(1, sizeof(cdd_cst_node_t));

  root->kind = CDD_CST_TRANSLATION_UNIT;
  decl->kind = CDD_CST_DECLARATION;
  cdd_cst_append_child_node(root, decl);

  tok_endif->kind = CDD_TOKEN_PREPROC_ENDIF;
  cdd_cst_append_child_token(root, tok_endif);
  tree->root = root;

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  free(tok_eof);
  free(tok_endif);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_extern_c_close_node_fails(void) {
  int i;
  for (i = 0; i < 20; i++) {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
    cdd_token_t *tok = calloc(1, sizeof(cdd_token_t));
    cdd_transform_config_t config = {0, 2, 0, 1, 0};

    root->kind = CDD_CST_TRANSLATION_UNIT;
    decl->kind = CDD_CST_DECLARATION;
    dir->kind = CDD_CST_PREPROC_DIRECTIVE;
    tok->kind = CDD_TOKEN_PREPROC_INCLUDE;

    cdd_cst_append_child_token(dir, tok);
    cdd_cst_append_child_node(root, decl);
    cdd_cst_append_child_node(root, dir);
    tree->root = root;

    root->capacity = root->num_children;

    g_extern_c_top_node_fail = 1; /* Skip top node */
    g_extern_c_bot_node_fail = 1; /* Skip bot node to simplify */
    g_cdd_cst_realloc_fail = i;
    cdd_transform_extern_c(tree, &config);
    g_cdd_cst_realloc_fail = 0;
    g_extern_c_top_node_fail = 0;
    g_extern_c_bot_node_fail = 0;

    free(tok);
    cdd_cst_tree_free(tree);
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_bot_append_dead_code(void) {
  int i;
  for (i = 0; i < 20; i++) {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
    cdd_transform_config_t config = {0, 2, 0, 1, 0};
    int rc;

    root->kind = CDD_CST_TRANSLATION_UNIT;
    dir->kind = CDD_CST_PREPROC_CONDITIONAL;
    cdd_cst_append_child_node(root, dir);
    tree->root = root;

    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
    decl->kind = CDD_CST_DECLARATION;
    cdd_cst_append_child_node(root, decl);

    dir->capacity = dir->num_children;

    g_extern_c_top_node_fail = 1; /* Skip top node */
    if (i == 50)
      g_extern_c_bot_node_fail = 999;
    else
      g_cdd_cst_realloc_fail = i;
    rc = cdd_transform_extern_c(tree, &config);
    (void)rc;
    g_cdd_cst_realloc_fail = 0;
    g_extern_c_bot_node_fail = 0;
    g_extern_c_top_node_fail = 0;

    cdd_cst_tree_free(tree);
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_target_parent_no_eof(void) {
  int i;
  for (i = 0; i < 1000; i++) {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    int rc;
    cdd_transform_config_t config = {0, 2, 0, 1, 0};

    root->kind = CDD_CST_TRANSLATION_UNIT;

    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
    decl->kind = CDD_CST_DECLARATION;

    tree->root = root;

    root->children = calloc(1, sizeof(cdd_cst_child_t));
    root->children[0].kind = CDD_CST_CHILD_NODE;
    root->children[0].val.node = decl;

    root->num_children = 1;
    root->capacity = 1;

    if (i == 50)
      g_extern_c_bot_node_fail = 999;
    else
      g_cdd_cst_realloc_fail = i;
    rc = cdd_transform_extern_c(tree, &config);
    (void)rc;
    g_cdd_cst_realloc_fail = 0;
    g_extern_c_bot_node_fail = 0;

    cdd_cst_tree_free(tree);
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_empty_target_parent(void) {
  int i;
  for (i = 0; i < 1000; i++) {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
    int rc;
    cdd_transform_config_t config = {0, 2, 0, 1, 0};

    root->kind = CDD_CST_TRANSLATION_UNIT;
    dir->kind = CDD_CST_PREPROC_CONDITIONAL;
    /* dir has NO children, so num_children == 0 */
    cdd_cst_append_child_node(root, dir);
    tree->root = root;

    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
    decl->kind = CDD_CST_DECLARATION;
    cdd_cst_append_child_node(
        root, decl); /* decl is outside dir, but has_decl is true */

    /* Force capacity to trigger realloc */
    dir->capacity = dir->num_children;
    root->capacity = root->num_children;

    g_cdd_cst_realloc_fail = i;
    rc = cdd_transform_extern_c(tree, &config);
    (void)rc;
    g_cdd_cst_realloc_fail = 0;

    cdd_cst_tree_free(tree);
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_append_fails(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void func();";
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  int i;
  for (i = 0; i < 200; i++) {
    cdd_cst_tree_t *tree_copy = NULL;
    rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree_copy);
    ASSERT_EQ(0, rc);

    if (tree_copy && tree_copy->root) {
      size_t j;
      tree_copy->root->capacity = tree_copy->root->num_children;
      for (j = 0; j < tree_copy->root->num_children; j++) {
        if (tree_copy->root->children[j].kind == CDD_CST_CHILD_NODE) {
          cdd_cst_node_t *c = tree_copy->root->children[j].val.node;
          if (c)
            c->capacity = c->num_children;
        }
      }
    }

    g_fail_io_after = -1; /* Don't fail regular allocs */
    g_cdd_cst_realloc_fail = i;
    rc = cdd_transform_extern_c(tree_copy, &config);
    g_cdd_cst_realloc_fail = 0;

    cdd_cst_tree_free(tree_copy);
  }

  for (i = 0; i < 200; i++) {
    cdd_cst_tree_t *tree_copy = NULL;
    rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree_copy);
    ASSERT_EQ(0, rc);

    g_fail_io_after = i;
    g_cdd_cst_realloc_fail = 0;
    rc = cdd_transform_extern_c(tree_copy, &config);
    g_fail_io_after = -1;

    cdd_cst_tree_free(tree_copy);
  }

  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_extern_c_insert_fails(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void func();\n#include <late.h>\n";
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  /* Set fail_io_after to a value that lets some allocations succeed but fails
   * on insert */
  /* We might need to try a few values, or just loop until we hit the fail */
  int i;
  for (i = 0; i < 200; i++) {
    cdd_cst_tree_t *tree_copy = NULL;
    rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree_copy);
    ASSERT_EQ(0, rc);

    if (tree_copy && tree_copy->root) {
      size_t j;
      tree_copy->root->capacity = tree_copy->root->num_children;
      for (j = 0; j < tree_copy->root->num_children; j++) {
        if (tree_copy->root->children[j].kind == CDD_CST_CHILD_NODE) {
          cdd_cst_node_t *c = tree_copy->root->children[j].val.node;
          if (c)
            c->capacity = c->num_children;
        }
      }
    }

    g_fail_io_after = -1; /* Don't fail regular allocs */
    g_cdd_cst_realloc_fail = i;
    rc = cdd_transform_extern_c(tree_copy, &config);
    g_cdd_cst_realloc_fail = 0;

    cdd_cst_tree_free(tree_copy);
  }

  for (i = 0; i < 200; i++) {
    cdd_cst_tree_t *tree_copy = NULL;
    rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree_copy);
    ASSERT_EQ(0, rc);

    g_fail_io_after = i;
    g_cdd_cst_realloc_fail = 0;
    rc = cdd_transform_extern_c(tree_copy, &config);
    g_fail_io_after = -1;

    cdd_cst_tree_free(tree_copy);
  }

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_already_exists_single_token(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
  cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
  cdd_token_t *tok_ifdef = calloc(1, sizeof(cdd_token_t));
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  tok_ifdef->kind = CDD_TOKEN_PREPROC_IFDEF;
  tok_ifdef->start = (const uint8_t *)"#ifdef __cplusplus";
  tok_ifdef->length = 18;

  root->kind = CDD_CST_TRANSLATION_UNIT;
  dir->kind = CDD_CST_PREPROC_DIRECTIVE; /* DIRECTIVE instead of CONDITIONAL */
  cdd_cst_append_child_token(dir, tok_ifdef);
  cdd_cst_append_child_node(root, dir);

  cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
  decl->kind = CDD_CST_DECLARATION;
  cdd_cst_append_child_node(root, decl);
  tree->root = root;

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  free(tok_ifdef);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_first_token(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
  cdd_token_t *tok = calloc(1, sizeof(cdd_token_t));
  cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  tok->kind = CDD_TOKEN_PREPROC_DEFINE;

  root->kind = CDD_CST_TRANSLATION_UNIT;
  cdd_cst_append_child_token(root, tok);

  decl->kind = CDD_CST_DECLARATION;
  cdd_cst_append_child_node(root, decl);
  tree->root = root;

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  free(tok);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_already_exists(void) {
  cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
  cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
  cdd_token_t *tok_ifdef = calloc(1, sizeof(cdd_token_t));
  cdd_token_t *tok_cpp = calloc(1, sizeof(cdd_token_t));
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  tok_ifdef->kind = CDD_TOKEN_PREPROC_IFDEF;
  tok_ifdef->start = (const uint8_t *)"#ifdef";
  tok_ifdef->length = 6;

  tok_cpp->kind = CDD_TOKEN_IDENTIFIER;
  tok_cpp->start = (const uint8_t *)"__cplusplus";
  tok_cpp->length = 11;

  root->kind = CDD_CST_TRANSLATION_UNIT;
  dir->kind = CDD_CST_PREPROC_DIRECTIVE;
  cdd_cst_append_child_token(dir, tok_ifdef);
  cdd_cst_append_child_token(dir, tok_cpp);
  cdd_cst_append_child_node(root, dir);

  cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
  decl->kind = CDD_CST_DECLARATION;
  cdd_cst_append_child_node(root, decl);
  tree->root = root;

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc); /* should return 0 directly without changes */
  ASSERT_EQ(2, root->num_children); /* nothing added */

  /* Mutate token to test branch logic where it doesn't match __cplusplus */
  tok_cpp->length = 10;
  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(4, root->num_children); /* added top and bottom nodes */

  /* Test when it's not an ifdef */
  tok_ifdef->kind = CDD_TOKEN_PREPROC_IFNDEF;
  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  /* Cleanup */
  free(tok_ifdef);
  free(tok_cpp);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Extern C transformer test suite.
 */
#ifdef CDD_BUILD_TESTS
#endif

TEST test_extern_c_late_include(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void func();\n#include <late.h>\nvoid func2();\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};
  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);
  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);
  /* Should insert extern C at top, close it before late.h, and reopen it after
   */
  char *func_pos = strstr(out, "void func();");
  ASSERT(func_pos != NULL);
  char *close_pos = strstr(func_pos, "}");
  ASSERT(close_pos != NULL);
  char *include_pos = strstr(close_pos, "#include <late.h>");
  ASSERT(include_pos != NULL);
  char *reopen_pos = strstr(include_pos, "extern \"C\" {");
  ASSERT(reopen_pos != NULL);
  char *func2_pos = strstr(reopen_pos, "void func2();");
  ASSERT(func2_pos != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_builder_fails(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  int rc;
  const char *code = "void func();";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  g_extern_c_top_node_fail = 1;
  cdd_transform_extern_c(tree, &config);
  g_extern_c_top_node_fail = 0;

  g_extern_c_bot_node_fail = 1;
  cdd_transform_extern_c(tree, &config);
  g_extern_c_bot_node_fail = 0;

  cdd_cst_tree_free(tree);
#endif
  g_fail_io_after = -1;
  PASS();
}

SUITE(transformer_extern_c_suite) {
  RUN_TEST(test_cdd_transform_extern_c_target_parent_no_eof);
  RUN_TEST(test_cdd_transform_extern_c);
  RUN_TEST(test_cdd_transform_extern_c_null_args);
  RUN_TEST(test_cdd_transform_extern_c_empty_tree);
  RUN_TEST(test_cdd_transform_extern_c_empty_c_file);
  RUN_TEST(test_cdd_transform_extern_c_malformed_ifdef);
  RUN_TEST(test_cdd_transform_extern_c_already_exists_conditional);
  RUN_TEST(test_cdd_transform_extern_c_inner_ifdef);
  RUN_TEST(test_cdd_transform_extern_c_target_parent_tokens);
  RUN_TEST(test_cdd_transform_extern_c_bot_insert_idx);
  RUN_TEST(test_cdd_transform_extern_c_close_node_fails);
  RUN_TEST(test_cdd_transform_extern_c_bot_append_dead_code);
  RUN_TEST(test_cdd_transform_extern_c_empty_target_parent);
  RUN_TEST(test_cdd_transform_extern_c_null_child);
  RUN_TEST(test_cdd_transform_extern_c_already_exists_single_token);
  RUN_TEST(test_cdd_transform_extern_c_first_token);
  RUN_TEST(test_cdd_transform_extern_c_append_fails);
  RUN_TEST(test_cdd_transform_extern_c_insert_fails);
  RUN_TEST(test_cdd_transform_extern_c_already_exists);
  RUN_TEST(test_extern_c_late_include);
  RUN_TEST(test_cdd_transform_extern_c_builder_fails);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_EXTERN_C_H */
