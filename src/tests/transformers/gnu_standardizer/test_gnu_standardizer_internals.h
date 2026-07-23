#ifndef TEST_GNU_STANDARDIZER_INTERNALS_H
#define TEST_GNU_STANDARDIZER_INTERNALS_H

#include "c_cdd_export.h"
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include <greatest.h>

int g_force_gnu_alloc_fail = 0;
int g_force_strdup_fail = 0;
#define malloc(s) (g_force_gnu_alloc_fail ? NULL : malloc(s))
#define realloc(p, s) (g_force_gnu_alloc_fail ? NULL : realloc(p, s))
#define strdup(s) (g_force_strdup_fail ? NULL : strdup(s))

#define cdd_transform_gnu internal_cdd_transform_gnu

#include "transformers/gnu_standardizer/gnu_standardizer.c"

extern enum cdd_c_error append_int(char *p, int v, char **out_p);
extern void parse_hex_128_literal(const char *str, size_t len, uint64_t *high,
                                  uint64_t *low);

#undef cdd_transform_gnu

extern enum cdd_c_error cdd_transform_gnu(cdd_cst_tree_t *tree,
                                          const cdd_transform_config_t *config);

TEST test_gnu_internals(void) {
  cdd_cst_tree_t tree;
  memset(&tree, 0, sizeof(tree));

  /* Test pool_string_safe */
  ASSERT_EQ(NULL, pool_string_safe(NULL, "a"));
  ASSERT_EQ(NULL, pool_string_safe(&tree, NULL));

  g_force_gnu_alloc_fail = 1;
  ASSERT_EQ(NULL, pool_string_safe(&tree, "a"));
  g_force_gnu_alloc_fail = 0;

  tree.string_capacity = 0;
  tree.num_strings = 0;
  g_force_gnu_alloc_fail = 1;
  ASSERT_EQ(NULL, pool_string_safe(&tree, "a"));
  g_force_gnu_alloc_fail = 0;

  /* Test pool_string_safe_len */
  ASSERT_EQ(NULL, pool_string_safe_len(NULL, "a", 1));
  ASSERT_EQ(NULL, pool_string_safe_len(&tree, NULL, 1));

  g_force_gnu_alloc_fail = 1;
  ASSERT_EQ(NULL, pool_string_safe_len(&tree, "a", 1));
  g_force_gnu_alloc_fail = 0;

  tree.string_capacity = 0;
  tree.num_strings = 0;
  g_force_gnu_alloc_fail = 1;
  ASSERT_EQ(NULL, pool_string_safe_len(&tree, "a", 1));
  g_force_gnu_alloc_fail = 0;

  /* Test append_int */
  char buf[32];
  char *out_p = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, append_int(NULL, 0, &out_p));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, append_int(buf, 0, NULL));

  ASSERT_EQ(CDD_C_SUCCESS, append_int(buf, 0, &out_p));
  ASSERT_STR_EQ("0", buf);

  /* Test parse_hex_128_literal with hex */
  uint64_t high, low;
  parse_hex_128_literal("0xaA", 4, &high, &low);
  ASSERT_EQ(0, high);
  ASSERT_EQ(170, low);

  /* Test cdd_infer_type */
  ASSERT_STR_EQ("int", cdd_infer_type(NULL, 0));

  /* Test trampoline zero token */
  {
    cdd_cst_tree_t t;
    memset(&t, 0, sizeof(t));
    t.base_tokens = calloc(1, sizeof(struct TokenList));
    t.base_tokens->size = 3;
    t.base_tokens->tokens = calloc(3, sizeof(cdd_token_t));
    t.base_tokens->tokens[0].kind = CDD_TOKEN_IDENTIFIER;
    t.base_tokens->tokens[0].start = (const uint8_t *)"bar";
    t.base_tokens->tokens[0].length = 3;
    t.base_tokens->tokens[1].kind = CDD_TOKEN_OTHER;
    t.base_tokens->tokens[1].length = 0; /* Zero length token */
    t.base_tokens->tokens[2].kind = CDD_TOKEN_SEMICOLON;
    t.base_tokens->tokens[2].start = (const uint8_t *)";";
    t.base_tokens->tokens[2].length = 1;

    struct tramp_ctx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.name = (const uint8_t *)"bar";
    ctx.length = 3;

    cdd_cst_node_t root;
    memset(&root, 0, sizeof(root));
    root.kind = CDD_CST_IDENTIFIER;
    root.num_children = 1;
    root.children = calloc(1, sizeof(cdd_cst_child_t));
    root.children[0].kind = CDD_CST_CHILD_TOKEN;
    root.children[0].val.token = &t.base_tokens->tokens[0];

    tramp_visitor(&root, &ctx);
    ASSERT_EQ(1, ctx.is_tramp);

    free(root.children);
    free(t.base_tokens->tokens);
    free(t.base_tokens);
  }

  /* Test magic_visitor alloc fail */
  {
    cdd_cst_tree_t t;
    memset(&t, 0, sizeof(t));
    t.base_tokens = calloc(1, sizeof(struct TokenList));
    t.base_tokens->size = 1;
    t.base_tokens->tokens = calloc(1, sizeof(cdd_token_t));
    t.base_tokens->tokens[0].kind = CDD_TOKEN_IDENTIFIER;
    t.base_tokens->tokens[0].start = (const uint8_t *)"__func__";
    t.base_tokens->tokens[0].length = 8;

    struct magic_ctx mctx;
    memset(&mctx, 0, sizeof(mctx));
    mctx.tree = &t;
    mctx.func_name = (const uint8_t *)"test";
    mctx.func_len = 4;

    cdd_cst_node_t root;
    memset(&root, 0, sizeof(root));
    root.kind = CDD_CST_IDENTIFIER;
    root.num_children = 1;
    root.children = calloc(1, sizeof(cdd_cst_child_t));
    root.children[0].kind = CDD_CST_CHILD_TOKEN;
    root.children[0].val.token = &t.base_tokens->tokens[0];

    g_force_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, magic_visitor(&root, &mctx));
    g_force_strdup_fail = 0;

    free(root.children);
    free(t.base_tokens->tokens);
    free(t.base_tokens);
  }

  /* Test tramp_visitor early return */
  {
    struct tramp_ctx tctx;
    memset(&tctx, 0, sizeof(tctx));
    tctx.is_tramp = 1;
    ASSERT_EQ(CDD_C_SUCCESS, tramp_visitor(NULL, &tctx));
  }

  PASS();
}

SUITE(transformer_gnu_standardizer_internals_suite) {
  RUN_TEST(test_gnu_internals);
}

#undef malloc
#undef realloc
#undef strdup

#endif /* TEST_GNU_STANDARDIZER_INTERNALS_H */
