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
/* extern C_CDD_EXPORT int g_cdd_cst_realloc_fail; (moved to global) */
extern C_CDD_EXPORT volatile int g_extern_c_top_node_fail;
extern C_CDD_EXPORT volatile int g_extern_c_bot_node_fail;
extern C_CDD_EXPORT volatile int g_extern_c_helper_fail;
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

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_query_err_fail;
extern C_CDD_EXPORT cdd_c_error_t
cdd_check_node_is_cpp_guard(cdd_cst_node_t *dir, int *out_is_cpp);
extern C_CDD_EXPORT cdd_c_error_t cdd_tree_has_decl(cdd_cst_node_t *node,
                                                    int *out_has_decl);

/**
 * @brief Tests basic functionality of the extern C transformer.
 *
 * @return The result of the test.
 */
TEST test_cdd_transform_extern_c(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      (char *)(size_t)(size_t) "/* license */\n#include <stdio.h>\n\nint "
                               "main() {\n  return 0;\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
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
  (void)rc;
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
  const char *code = (char *)(size_t)(size_t) "   \n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
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
  const char *code =
      (char *)(size_t)(size_t) "#ifdef \nint main() { return 0; }\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
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
  (void)rc;
  cdd_cst_append_child_token(dir, tok_ifdef);
  cdd_cst_append_child_node(root, dir);
  tree->root = root;

  /* Add a decl to trigger has_decl */
  {
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
    decl->kind = CDD_CST_DECLARATION;
    cdd_cst_append_child_node(root, decl);

    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(0, rc); /* should return 0 directly without changes because of
                         __cplusplus */

    /* Cleanup */
    free(tok_ifdef);
    cdd_cst_tree_free(tree);
    g_fail_io_after = -1;
    PASS();
  }
}

TEST test_cdd_transform_extern_c_inner_ifdef(void) {
  cdd_cst_tree_t *tree = NULL;
  /* ifdef __cplusplus inside a function, so it shouldn't prevent wrapping the
   * global scope */
  const char *code =
      (char *)(size_t)(size_t) "int main() {\n#ifdef "
                               "__cplusplus\n#endif\nreturn 0;\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
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
  (void)rc;
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
  (void)rc;
  cdd_cst_append_child_token(dir, tok_ifdef);
  cdd_cst_append_child_node(root, dir);
  tree->root = root;

  {
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
    decl->kind = CDD_CST_DECLARATION;
    cdd_cst_append_child_node(
        dir, decl); /* Put decl inside dir so has_decl passes */

    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(0, rc);

    free(tok_ifdef);
    cdd_cst_tree_free(tree);
    g_fail_io_after = -1;
    PASS();
  }
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
  (void)rc;
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
  for (i = 0; i < 50; i++) {
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
  for (i = 0; i < 50; i++) {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
    cdd_transform_config_t config = {0, 2, 0, 1, 0};
    int rc;

    root->kind = CDD_CST_TRANSLATION_UNIT;
    dir->kind = CDD_CST_PREPROC_CONDITIONAL;
    (void)rc;
    cdd_cst_append_child_node(root, dir);
    tree->root = root;

    {
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
      g_cdd_cst_realloc_fail = 0;
      g_extern_c_bot_node_fail = 0;
      g_extern_c_top_node_fail = 0;

      cdd_cst_tree_free(tree);
    }
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_target_parent_no_eof(void) {
  int i;
  for (i = 0; i < 50; i++) {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    int rc;
    cdd_transform_config_t config = {0, 2, 0, 1, 0};

    root->kind = CDD_CST_TRANSLATION_UNIT;

    {
      cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
      decl->kind = CDD_CST_DECLARATION;

      tree->root = root;

      root->children = calloc(1, sizeof(cdd_cst_child_t));
      root->children[0].kind = CDD_CST_CHILD_NODE;
      root->children[0].val.node = decl;

      root->num_children = 1;
      root->capacity = 1;

      (void)rc;
      if (i == 50)
        g_extern_c_bot_node_fail = 999;
      else
        g_cdd_cst_realloc_fail = i;
      rc = cdd_transform_extern_c(tree, &config);
      g_cdd_cst_realloc_fail = 0;
      g_extern_c_bot_node_fail = 0;

      cdd_cst_tree_free(tree);
    }
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_empty_target_parent(void) {
  int i;
  for (i = 0; i < 50; i++) {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
    int rc;
    cdd_transform_config_t config = {0, 2, 0, 1, 0};

    root->kind = CDD_CST_TRANSLATION_UNIT;
    dir->kind = CDD_CST_PREPROC_CONDITIONAL;
    /* dir has NO children, so num_children == 0 */
    (void)rc;
    cdd_cst_append_child_node(root, dir);
    tree->root = root;

    {
      cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
      decl->kind = CDD_CST_DECLARATION;
      cdd_cst_append_child_node(
          root, decl); /* decl is outside dir, but has_decl is true */

      /* Force capacity to trigger realloc */
      dir->capacity = dir->num_children;
      root->capacity = root->num_children;

      g_cdd_cst_realloc_fail = i;
      rc = cdd_transform_extern_c(tree, &config);
      g_cdd_cst_realloc_fail = 0;

      cdd_cst_tree_free(tree);
    }
  }
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_extern_c_append_fails(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = (char *)(size_t)(size_t) "void func();";
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
  ASSERT_EQ(0, rc);

  {
    int i;
    for (i = 0; i < 50; i++) {
      cdd_cst_tree_t *tree_copy = NULL;
      rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                         &tree_copy);
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

    for (i = 0; i < 50; i++) {
      cdd_cst_tree_t *tree_copy = NULL;
      rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                         &tree_copy);
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
}

TEST test_cdd_transform_extern_c_insert_fails(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      (char *)(size_t)(size_t) "void func();\n#include <late.h>\n";
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
  ASSERT_EQ(0, rc);

  /* Set fail_io_after to a value that lets some allocations succeed but fails
   * on insert */
  /* We might need to try a few values, or just loop until we hit the fail */
  {
    int i;
    for (i = 0; i < 50; i++) {
      cdd_cst_tree_t *tree_copy = NULL;
      rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                         &tree_copy);
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

    for (i = 0; i < 50; i++) {
      cdd_cst_tree_t *tree_copy = NULL;
      rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                         &tree_copy);
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
  (void)rc;
  cdd_cst_append_child_token(dir, tok_ifdef);
  cdd_cst_append_child_node(root, dir);

  {
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
  (void)rc;
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
  (void)rc;
  cdd_cst_append_child_token(dir, tok_ifdef);
  cdd_cst_append_child_token(dir, tok_cpp);
  cdd_cst_append_child_node(root, dir);

  {
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
}

/**
 * @brief Extern C transformer test suite.
 */
#ifdef CDD_BUILD_TESTS
#endif

TEST test_extern_c_late_include(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = (char *)(size_t)(size_t) "void func();\n#include "
                                              "<late.h>\nvoid func2();\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};
  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
  ASSERT_EQ(0, rc);
  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);
  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);
  /* Should insert extern C at top, close it before late.h, and reopen it after
   */
  {
    char *func_pos = strstr(out, "void func();");
    ASSERT(func_pos != NULL);
    {
      char *close_pos = strstr(func_pos, "}");
      ASSERT(close_pos != NULL);
      {
        char *include_pos = strstr(close_pos, "#include <late.h>");
        ASSERT(include_pos != NULL);
        {
          char *reopen_pos = strstr(include_pos, "extern \"C\" {");
          ASSERT(reopen_pos != NULL);
          {
            char *func2_pos = strstr(reopen_pos, "void func2();");
            ASSERT(func2_pos != NULL);
            free(out);
            cdd_cst_tree_free(tree);
            g_fail_io_after = -1;
            PASS();
          }
        }
      }
    }
  }
}

TEST test_cdd_transform_extern_c_builder_fails(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  int rc;
  const char *code = (char *)(size_t)(size_t) "void func();";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
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

TEST test_extern_c_bot_node_insert_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  const char *code = (char *)(size_t)(size_t) "void func();"; /* This has EOF */
  int rc;
  cdd_token_t *eof_tok = NULL;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
  ASSERT_EQ(0, rc);

  if (tree && tree->root) {
    /* Manually append an EOF token so insert_child_node_at is used */
    eof_tok = calloc(1, sizeof(cdd_token_t));
    if (eof_tok) {
      eof_tok->kind = CDD_TOKEN_EOF;

      {
        cdd_cst_child_t ch;
        ch.kind = CDD_CST_CHILD_TOKEN;
        ch.val.token = eof_tok;

        if (tree->root->num_children >= tree->root->capacity) {
          cdd_cst_child_t *new_arr =
              realloc(tree->root->children,
                      (tree->root->capacity + 2) * sizeof(cdd_cst_child_t));
          if (new_arr) {
            tree->root->children = new_arr;
            tree->root->capacity += 2;
          }
        }
        if (tree->root->num_children < tree->root->capacity) {
          tree->root->children[tree->root->num_children++] = ch;
        }
      }
    }
  }

  g_fail_io_after = 12345;
  rc = cdd_transform_extern_c(tree, &config);
  g_fail_io_after = -1;

  cdd_cst_tree_free(tree);
  if (eof_tok)
    free(eof_tok);
#endif
  PASS();
}

TEST test_extern_c_bot_node_append_oom(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  const char *code = (char *)(size_t)(size_t) "void func();";
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
  ASSERT_EQ(0, rc);

  g_fail_io_after = 12346;
  rc = cdd_transform_extern_c(tree, &config);
  g_fail_io_after = -1;

  cdd_cst_tree_free(tree);
#endif
  PASS();
}

TEST test_cdd_transform_extern_c_helper_fails(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#ifdef __cplusplus\n#endif\n#include <stdio.h>\nint "
                     "main() { return 0; }\n";
  int rc;
  int final_rc = 0;
  cdd_transform_config_t config = {0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_NEQ(NULL, tree);

#ifdef CDD_BUILD_TESTS
  g_extern_c_helper_fail = -1;
  rc = cdd_transform_extern_c(tree, &config);
  g_extern_c_helper_fail = 0;
  if (rc != CDD_C_ERROR_MEMORY)
    final_rc |= 1;

  g_extern_c_helper_fail = 1;
  rc = cdd_transform_extern_c(tree, &config);
  g_extern_c_helper_fail = 0;
  if (rc != CDD_C_ERROR_MEMORY)
    final_rc |= 2;

  g_extern_c_helper_fail = 2;
  rc = cdd_transform_extern_c(tree, &config);
  g_extern_c_helper_fail = 0;
  if (rc != CDD_C_ERROR_MEMORY)
    final_rc |= 4;
#endif

  cdd_cst_tree_free(tree);

#ifdef CDD_BUILD_TESTS
  {
    cdd_cst_tree_t *tree2 = NULL;
    const char *code2 = (char *)(size_t)(size_t) "#include <stdio.h>\nint "
                                                 "main() { return 0; }\n";
    rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code2),
                       &tree2);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    g_extern_c_helper_fail = -1;
    rc = cdd_transform_extern_c(tree2, &config);
    g_extern_c_helper_fail = 0;
    if (rc != CDD_C_ERROR_MEMORY)
      final_rc |= 16;

    cdd_cst_tree_free(tree2);
  }
#endif

  ASSERT_EQ(0, final_rc);
  PASS();
}

TEST test_extern_c_top_node_oom(void) {
#ifdef CDD_BUILD_TESTS
  const char *code = (char *)(size_t)(size_t) "int main() { return 0; }\n";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};
  int i;
  for (i = 1; i < 50; i++) {
    cdd_cst_tree_t *tree = NULL;
    int rc;
    rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                       &tree);
    (void)rc;
    ASSERT_EQ(0, rc);

    g_cdd_alloc_fail = i;
    rc = cdd_transform_extern_c(tree, &config);
    g_cdd_alloc_fail = 0;
    cdd_cst_tree_free(tree);

    if (rc == CDD_C_SUCCESS)
      break;
  }
#endif
  PASS();
}

TEST test_extern_c_extra_coverage2(void) {
  const char *code = "class MyClass { \n#ifdef __cplusplus\n#endif\n };\n"
                     "int main() { \n#ifdef __cplusplus\n#endif\n"
                     " int x; \n#ifdef __cplusplus\n#endif\n x = 5; "
                     " { \n#ifdef __cplusplus\n#endif\n } return 0; }\n";
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config = {0};
  int rc;
  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_extern_c_extra_coverage3(void) {
  const char *code = "#if 1\n#endif\n"
                     "#ifdef OTHER\n#endif\n"
                     "#include <stdio.h>\n"
                     "/* test */\n"
                     "int z;\n";
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config = {0};
  int rc;
  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  cdd_cst_tree_free(tree);

  {
    const char *code2 = (char *)(size_t)(size_t) "int main() { return 0; }\n";
    rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code2),
                       &tree);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_tree_free(tree);
    PASS();
  }
}

TEST test_extern_c_extra_coverage4(void) {
  cdd_transform_config_t config = {0};
  int rc;

  /* 1. Empty directive (num_children == 0) and directive with CHILD_NODE */
  {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *dir1 = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *dir2 = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *subnode = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));

    root->kind = CDD_CST_TRANSLATION_UNIT;
    dir1->kind = CDD_CST_PREPROC_DIRECTIVE; /* num_children == 0 */
    dir2->kind = CDD_CST_PREPROC_DIRECTIVE;
    subnode->kind = CDD_CST_UNKNOWN;
    cdd_cst_append_child_node(dir2, subnode); /* children[0] is CHILD_NODE */

    decl->kind = CDD_CST_DECLARATION;

    cdd_cst_append_child_node(root, dir1);
    cdd_cst_append_child_node(root, dir2);
    cdd_cst_append_child_node(root, decl);
    tree->root = root;

    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_tree_free(tree);
  }

  /* 2. Directive with child[1] being CHILD_NODE, or non-identifier, or length
   * != 11, or identifier != __cplusplus */
  {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *dir = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *subnode = calloc(1, sizeof(cdd_cst_node_t));
    cdd_token_t *tok_ifdef = calloc(1, sizeof(cdd_token_t));
    cdd_token_t *tok_num = calloc(1, sizeof(cdd_token_t));
    cdd_token_t *tok_ident10 = calloc(1, sizeof(cdd_token_t));
    cdd_token_t *tok_ident11_mismatch = calloc(1, sizeof(cdd_token_t));
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));

    tok_ifdef->kind = CDD_TOKEN_PREPROC_IFDEF;
    tok_ifdef->start = (const uint8_t *)"#ifdef";
    tok_ifdef->length = 6;

    tok_num->kind = CDD_TOKEN_NUMBER;
    tok_num->start = (const uint8_t *)"123";
    tok_num->length = 3;

    tok_ident10->kind = CDD_TOKEN_IDENTIFIER;
    tok_ident10->start = (const uint8_t *)"abcdefghij";
    tok_ident10->length = 10;

    tok_ident11_mismatch->kind = CDD_TOKEN_IDENTIFIER;
    tok_ident11_mismatch->start = (const uint8_t *)"abcdefghijk";
    tok_ident11_mismatch->length = 11;

    root->kind = CDD_CST_TRANSLATION_UNIT;
    dir->kind = CDD_CST_PREPROC_DIRECTIVE;
    subnode->kind = CDD_CST_UNKNOWN;

    cdd_cst_append_child_token(dir, tok_ifdef);
    cdd_cst_append_child_node(dir, subnode);
    cdd_cst_append_child_token(dir, tok_num);
    cdd_cst_append_child_token(dir, tok_ident10);
    cdd_cst_append_child_token(dir, tok_ident11_mismatch);

    decl->kind = CDD_CST_DECLARATION;

    cdd_cst_append_child_node(root, dir);
    cdd_cst_append_child_node(root, decl);
    tree->root = root;

    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    free(tok_ifdef);
    free(tok_num);
    free(tok_ident10);
    free(tok_ident11_mismatch);
    cdd_cst_tree_free(tree);
  }

  PASS();
}

TEST test_extern_c_extra_coverage5(void) {
  cdd_transform_config_t config = {0};
  int rc;

  /* 1. Late include OOMs */
  {
    cdd_cst_tree_t *tree = NULL;
    const char *code = "void func();\n#include <late.h>\nvoid func2();\n";
    int i;
    for (i = 1; i <= 6; i++) {
      rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                         &tree);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      g_cdd_alloc_fail = i;
      (void)cdd_transform_extern_c(tree, &config);
      g_cdd_alloc_fail = 0;
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }

  /* 2. bot_node append fail with 12346 */
  {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));

    root->kind = CDD_CST_TRANSLATION_UNIT;
    decl->kind = CDD_CST_DECLARATION;
    cdd_cst_append_child_node(root, decl);
    tree->root = root;

    g_fail_io_after = 12346;
    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_fail_io_after = -1;

    cdd_cst_tree_free(tree);
  }

  /* 3. Unknown child node with non-comment token */
  {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *unk = calloc(1, sizeof(cdd_cst_node_t));
    cdd_token_t *tok = calloc(1, sizeof(cdd_token_t));
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));

    tok->kind = CDD_TOKEN_IDENTIFIER;
    tok->start = (const uint8_t *)"x";
    tok->length = 1;

    root->kind = CDD_CST_TRANSLATION_UNIT;
    unk->kind = CDD_CST_UNKNOWN;
    cdd_cst_append_child_token(unk, tok);

    decl->kind = CDD_CST_DECLARATION;

    cdd_cst_append_child_node(root, unk);
    cdd_cst_append_child_node(root, decl);
    tree->root = root;

    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    free(tok);
    cdd_cst_tree_free(tree);
  }

  /* 4. Target parent child is token other than ifdef/ifndef/define/pragma/eof
   */
  {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_token_t *tok_semi = calloc(1, sizeof(cdd_token_t));
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));

    tok_semi->kind = CDD_TOKEN_SEMICOLON;
    tok_semi->start = (const uint8_t *)";";
    tok_semi->length = 1;

    root->kind = CDD_CST_TRANSLATION_UNIT;
    cdd_cst_append_child_token(root, tok_semi);

    decl->kind = CDD_CST_DECLARATION;
    cdd_cst_append_child_node(root, decl);
    tree->root = root;

    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    free(tok_semi);
    cdd_cst_tree_free(tree);
  }

  /* 5. Last child is token with kind other than EOF/ENDIF to hit default: break
   * in line 444 */
  {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
    cdd_token_t *tok_semi = calloc(1, sizeof(cdd_token_t));

    tok_semi->kind = CDD_TOKEN_SEMICOLON;
    tok_semi->start = (const uint8_t *)";";
    tok_semi->length = 1;

    root->kind = CDD_CST_TRANSLATION_UNIT;
    decl->kind = CDD_CST_DECLARATION;
    cdd_cst_append_child_node(root, decl);
    cdd_cst_append_child_token(root, tok_semi);
    tree->root = root;

    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    free(tok_semi);
    cdd_cst_tree_free(tree);
  }

  /* 6. g_cdd_query_err_fail tests for CONDITIONAL and DIRECTIVE */
  {
    cdd_cst_tree_t *tree = NULL;
    const char *code1 = "#ifdef FOO\n#endif\nvoid f();\n";
    const char *code2 = "#include <stdio.h>\nvoid f();\n";

    rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code1),
                       &tree);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_query_err_fail = 1;
    rc = cdd_transform_extern_c(tree, &config);
    g_cdd_query_err_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    cdd_cst_tree_free(tree);

    rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code2),
                       &tree);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_query_err_fail = 1;
    rc = cdd_transform_extern_c(tree, &config);
    g_cdd_query_err_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    cdd_cst_tree_free(tree);
  }

  PASS();
}

TEST test_extern_c_helpers_direct(void) {
  int out_val = 0;
  cdd_cst_node_t node;
  cdd_cst_child_t ch;
  cdd_token_t tok;
  int rc;

  memset(&node, 0, sizeof(node));
  memset(&ch, 0, sizeof(ch));
  memset(&tok, 0, sizeof(tok));

  /* cdd_check_node_is_cpp_guard NULL dir */
  rc = cdd_check_node_is_cpp_guard(NULL, &out_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, out_val);

  /* cdd_tree_has_decl NULL node */
  rc = cdd_tree_has_decl(NULL, &out_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, out_val);

  /* cdd_tree_has_decl with child node whose children[0] is CHILD_NODE */
  {
    cdd_cst_node_t child_node;
    cdd_cst_node_t grandchild;
    cdd_cst_child_t gc_ch;

    memset(&child_node, 0, sizeof(child_node));
    memset(&grandchild, 0, sizeof(grandchild));
    memset(&gc_ch, 0, sizeof(gc_ch));

    node.num_children = 1;
    node.children = &ch;
    ch.kind = CDD_CST_CHILD_NODE;
    ch.val.node = &child_node;

    child_node.kind = CDD_CST_UNKNOWN;
    child_node.num_children = 1;
    child_node.children = &gc_ch;
    gc_ch.kind = CDD_CST_CHILD_NODE;
    gc_ch.val.node = &grandchild;

    rc = cdd_tree_has_decl(&node, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, out_val);

    /* Now make grandchild a token with CDD_TOKEN_OTHER */
    gc_ch.kind = CDD_CST_CHILD_TOKEN;
    gc_ch.val.token = &tok;
    tok.kind = CDD_TOKEN_OTHER;
    rc = cdd_tree_has_decl(&node, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, out_val);

    /* Now make grandchild a token with CDD_TOKEN_EOF */
    tok.kind = CDD_TOKEN_EOF;
    rc = cdd_tree_has_decl(&node, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, out_val);

    /* Now make grandchild a non-other token */
    tok.kind = CDD_TOKEN_IDENTIFIER;
    rc = cdd_tree_has_decl(&node, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, out_val);
  }

  /* Test directive inside function for CDD_CST_PREPROC_DIRECTIVE loop */
  {
    cdd_cst_tree_t *tree = NULL;
    const char *code = "int main() { #pragma once\n return 0; }\n";
    cdd_transform_config_t config = {0};
    rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                       &tree);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_tree_free(tree);
  }

  /* Test first_token_of_file when children[0] has num_children == 0 */
  {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *empty_node = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
    cdd_transform_config_t config = {0};

    root->kind = CDD_CST_TRANSLATION_UNIT;
    empty_node->kind = CDD_CST_UNKNOWN;
    decl->kind = CDD_CST_DECLARATION;

    cdd_cst_append_child_node(root, empty_node);
    cdd_cst_append_child_node(root, decl);
    tree->root = root;

    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_tree_free(tree);
  }

  /* Test target_parent child token EOF */
  {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_token_t *tok_eof = calloc(1, sizeof(cdd_token_t));
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
    cdd_transform_config_t config = {0};

    root->kind = CDD_CST_TRANSLATION_UNIT;
    tok_eof->kind = CDD_TOKEN_EOF;
    decl->kind = CDD_CST_DECLARATION;

    cdd_cst_append_child_token(root, tok_eof);
    cdd_cst_append_child_node(root, decl);
    tree->root = root;

    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    free(tok_eof);
    cdd_cst_tree_free(tree);
  }

  /* Test child in UNKNOWN whose children[0] is CHILD_NODE */
  {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *unk = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *sub = calloc(1, sizeof(cdd_cst_node_t));
    cdd_cst_node_t *decl = calloc(1, sizeof(cdd_cst_node_t));
    cdd_transform_config_t config = {0};

    root->kind = CDD_CST_TRANSLATION_UNIT;
    unk->kind = CDD_CST_UNKNOWN;
    sub->kind = CDD_CST_UNKNOWN;
    decl->kind = CDD_CST_DECLARATION;

    cdd_cst_append_child_node(unk, sub);
    cdd_cst_append_child_node(root, unk);
    cdd_cst_append_child_node(root, decl);
    tree->root = root;

    rc = cdd_transform_extern_c(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_tree_free(tree);
  }

  PASS();
}

SUITE(transformer_extern_c_suite) {
  RUN_TEST(test_extern_c_helpers_direct);
  RUN_TEST(test_extern_c_extra_coverage5);
  RUN_TEST(test_extern_c_extra_coverage4);
  RUN_TEST(test_extern_c_extra_coverage3);
  RUN_TEST(test_extern_c_extra_coverage2);
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
  RUN_TEST(test_extern_c_bot_node_insert_oom);
  RUN_TEST(test_extern_c_bot_node_append_oom);
  RUN_TEST(test_cdd_transform_extern_c_helper_fails);
  RUN_TEST(test_extern_c_top_node_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_EXTERN_C_H */
