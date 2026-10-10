/**
 * @file test_safe_crt_internals.h
 * @brief Direct internal unit tests for the Safe CRT transformer.
 */

#ifndef TEST_CDD_TRANSFORM_SAFE_CRT_INTERNALS_H
#define TEST_CDD_TRANSFORM_SAFE_CRT_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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

extern C_CDD_EXPORT int g_safe_crt_malloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
extern C_CDD_EXPORT int g_cdd_query_err_fail;

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT cdd_c_error_t arena_alloc(size_t len, void **out_ptr);
extern C_CDD_EXPORT cdd_c_error_t parse_expr_ast(void *stmt, size_t *idx,
                                                 int stop_at_comma,
                                                 void *out_ast);
extern C_CDD_EXPORT cdd_c_error_t find_and_mark_fopen(void *head,
                                                      int *out_found);
extern C_CDD_EXPORT cdd_c_error_t check_unsupported_calls(void *head);
extern C_CDD_EXPORT cdd_c_error_t check_needs_transform(void *head);
extern C_CDD_EXPORT cdd_c_error_t expr_is_null_or_zero(void *node);
extern C_CDD_EXPORT cdd_c_error_t clone_trivia(void *head, void **out_trivia);
extern C_CDD_EXPORT cdd_c_error_t clone_token(void *tree, void *tok,
                                              void **out_tok);
extern C_CDD_EXPORT void emit_inferred_size(cdd_cst_builder_t *bld, void *dest);
extern C_CDD_EXPORT const char *safe_crt_pool_string_safe(void *tree,
                                                          const char *str);
#ifndef SAFE_CRT_EXPR_TEST_DEF
#define SAFE_CRT_EXPR_TEST_DEF
struct safe_crt_expr_test {
  int type;
  cdd_token_t *tok;
  cdd_token_t *close_tok;
  void *args[16];
  size_t num_args;
  void *next;
};
#endif
extern C_CDD_EXPORT cdd_c_error_t emit_ast_bld_strip(void *node,
                                                     cdd_cst_builder_t *bld,
                                                     int is_msc);
extern C_CDD_EXPORT cdd_c_error_t
emit_ast_bld_strip_ampersand(void *node, cdd_cst_builder_t *bld, int is_msc);
extern C_CDD_EXPORT void get_indent_string(cdd_token_t *tok, char *out_indent);
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
#endif

TEST test_cdd_transform_safe_crt_direct_internals(void) {
  void *ptr = NULL;
  size_t idx = 0;
  int found = 0;

  /* arena_alloc */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arena_alloc(10, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, arena_alloc(10, &ptr));
  ASSERT(ptr != NULL);

  /* parse_expr_ast */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_expr_ast(NULL, &idx, 0, NULL));

  /* find_and_mark_fopen */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, find_and_mark_fopen(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, find_and_mark_fopen(NULL, &found));
  ASSERT_EQ(0, found);

  /* check_unsupported_calls */
  ASSERT_EQ(CDD_C_SUCCESS, check_unsupported_calls(NULL));

  /* check_needs_transform */
  ASSERT_EQ(CDD_C_SUCCESS, check_needs_transform(NULL));
  {
    /* emit_inferred_size with unknown token (name remains NULL) and with
     * cdd_query_err_fail */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test unknown_node;
    cdd_token_t t_num;
    memset(&unknown_node, 0, sizeof(unknown_node));
    memset(&t_num, 0, sizeof(t_num));
    t_num.kind = CDD_TOKEN_STRING;
    t_num.start = (const uint8_t *)"\"hello\"";
    t_num.length = 7;
    unknown_node.tok = &t_num;

    if (cdd_cst_parse(az_span_create_from_str("void f() { int x; }"),
                      &dummy_tree) == 0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        emit_inferred_size(&dummy_bld, (void *)&unknown_node);

        /* now with identifier and g_cdd_query_err_fail */
        t_num.kind = CDD_TOKEN_IDENTIFIER;
        t_num.start = (const uint8_t *)"x";
        t_num.length = 1;
        g_cdd_query_err_fail = 1;
        emit_inferred_size(&dummy_bld, (void *)&unknown_node);
        g_cdd_query_err_fail = 0;
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }
  {
    /* check_needs_transform with type 2 node wrapping type 3 */
    struct safe_crt_expr_test n2, n3;
    memset(&n2, 0, sizeof(n2));
    memset(&n3, 0, sizeof(n3));
    n3.type = 3;
    n2.type = 2;
    n2.args[0] = &n3;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, check_needs_transform(&n2));
  }
  {
    /* emit_ast_bld directly on call node with NULL close_tok */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test call_ast;
    memset(&call_ast, 0, sizeof(call_ast));
    call_ast.type = 1; /* type = 1 (call) */
    {
      cdd_token_t t;
      memset(&t, 0, sizeof(t));
      t.kind = CDD_TOKEN_IDENTIFIER;
      t.start = (const uint8_t *)"strlen";
      t.length = 6;
      call_ast.tok = &t;
      /* close_tok remains NULL */
      if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
          0) {
        if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
          cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
          emit_ast_bld_strip((void *)&call_ast, &dummy_bld, 0);
        }
        cdd_cst_tree_free(dummy_tree);
      }
    }
  }
  {
    /* check_needs_transform with type 3 node */
    int n3[32];
    memset(n3, 0, sizeof(n3));
    n3[0] = 3; /* type = 3 */
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, check_needs_transform(n3));
  }

  /* expr_is_null_or_zero */
  ASSERT_EQ(CDD_C_SUCCESS, expr_is_null_or_zero(NULL));

  /* clone_trivia */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, clone_trivia(NULL, NULL));

  /* clone_token */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, clone_token(NULL, NULL, NULL));

  {
    /* emit_ast_bld_strip with leading trivia on first token */
    cdd_cst_tree_t *t = NULL;
    cdd_cst_node_t *n = NULL;
    cdd_cst_builder_t b;
    size_t s_idx = 0;
    void *ast = NULL;
    if (cdd_cst_parse(az_span_create_from_str("/*hello*/ foo;"), &t) == 0) {
      if (t->root && t->root->num_children > 0) {
        parse_expr_ast(t->root->children[0].val.node, &s_idx, 0, (void *)&ast);
        if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &n) == 0) {
          cdd_cst_builder_init(&b, t, n);
          emit_ast_bld_strip(ast, &b, 0);
        }
      }
      cdd_cst_tree_free(t);
    }
  }

  {
    /* safe_crt_pool_string_safe tests */
    cdd_cst_tree_t *dummy_tree = NULL;
    ASSERT_EQ(NULL, safe_crt_pool_string_safe(NULL, "test"));
    ASSERT_EQ(NULL, safe_crt_pool_string_safe((void *)1, NULL));
    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      g_safe_crt_malloc_fail = 1;
      ASSERT_EQ(NULL, safe_crt_pool_string_safe(dummy_tree, "dup_fail"));
      g_safe_crt_malloc_fail = 2;
      /* strdup succeeds, then pool realloc fails because new_cap becomes 0 */
      ASSERT_EQ(NULL, safe_crt_pool_string_safe(dummy_tree, "pool_fail"));
      g_safe_crt_malloc_fail = 3;
      safe_crt_pool_string_safe(dummy_tree, "pool_fail3");
      g_safe_crt_malloc_fail = 0;
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* clone_trivia with multiple elements and failure on second element */
    cdd_trivia_t t1, t2;
    cdd_trivia_t *out_triv = NULL;
    memset(&t1, 0, sizeof(t1));
    memset(&t2, 0, sizeof(t2));
    t1.next = &t2;
    g_safe_crt_malloc_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, clone_trivia(&t1, (void **)&out_triv));
    g_safe_crt_malloc_fail = 0;
  }
  {
    /* clone_token with failed token creation */
    cdd_token_t tok_in;
    cdd_token_t *tok_out = NULL;
    memset(&tok_in, 0, sizeof(tok_in));
    /* NULL tree causes cdd_cst_create_token_len to fail */
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              clone_token(NULL, &tok_in, (void **)&tok_out));
  }

  {
    /* emit_inferred_size with invalid size expression */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    emit_inferred_size(NULL, NULL);
    memset(&dummy_bld, 0, sizeof(dummy_bld));
    emit_inferred_size(&dummy_bld, NULL);
    emit_ast_bld_strip(NULL, &dummy_bld, 0);

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        emit_inferred_size(&dummy_bld, NULL);
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* expr_is_null_or_zero variants */
    struct safe_crt_expr_test n_test, n_next;
    cdd_token_t t_zero, t_one, t_null;
    memset(&n_test, 0, sizeof(n_test));
    memset(&n_next, 0, sizeof(n_next));
    memset(&t_zero, 0, sizeof(t_zero));
    memset(&t_one, 0, sizeof(t_one));
    memset(&t_null, 0, sizeof(t_null));

    t_zero.kind = CDD_TOKEN_NUMBER;
    t_zero.start = (const uint8_t *)"0";
    t_zero.length = 1;

    t_one.kind = CDD_TOKEN_NUMBER;
    t_one.start = (const uint8_t *)"1";
    t_one.length = 1;

    t_null.kind = CDD_TOKEN_IDENTIFIER;
    t_null.start = (const uint8_t *)"NULL";
    t_null.length = 4;

    n_test.type = 1;
    ASSERT_EQ(CDD_C_SUCCESS, expr_is_null_or_zero((void *)&n_test));

    n_test.type = 0;
    n_test.tok = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, expr_is_null_or_zero((void *)&n_test));

    n_test.tok = &t_zero;
    n_test.next = &n_next;
    ASSERT_EQ(CDD_C_SUCCESS, expr_is_null_or_zero((void *)&n_test));

    n_test.next = NULL;
    n_test.tok = &t_one;
    ASSERT_EQ(CDD_C_SUCCESS, expr_is_null_or_zero((void *)&n_test));

    n_test.tok = &t_null;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, expr_is_null_or_zero((void *)&n_test));

    n_test.tok = &t_zero;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, expr_is_null_or_zero((void *)&n_test));
  }

  {
    /* check_unsupported_calls with long identifier */
    struct safe_crt_expr_test long_call;
    cdd_token_t t_long;
    char long_name[140];
    memset(long_name, 'a', 139);
    long_name[139] = '\0';
    memset(&t_long, 0, sizeof(t_long));
    t_long.kind = CDD_TOKEN_IDENTIFIER;
    t_long.start = (const uint8_t *)long_name;
    t_long.length = 139;
    memset(&long_call, 0, sizeof(long_call));
    long_call.type = 1;
    long_call.tok = &t_long;
    ASSERT_EQ(CDD_C_SUCCESS, check_unsupported_calls((void *)&long_call));
  }

  {
    /* infer_buffer_size edge cases */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n1, n2, n3, n_call_test;
    cdd_token_t t_amp, t_buf, t_lbrk, t_star, t_not_ident, t_plus, t_call_name;

    memset(&n1, 0, sizeof(n1));
    memset(&n2, 0, sizeof(n2));
    memset(&n3, 0, sizeof(n3));
    memset(&n_call_test, 0, sizeof(n_call_test));
    memset(&t_amp, 0, sizeof(t_amp));
    memset(&t_buf, 0, sizeof(t_buf));
    memset(&t_lbrk, 0, sizeof(t_lbrk));
    memset(&t_star, 0, sizeof(t_star));
    memset(&t_not_ident, 0, sizeof(t_not_ident));
    memset(&t_plus, 0, sizeof(t_plus));
    memset(&t_call_name, 0, sizeof(t_call_name));

    t_amp.kind = CDD_TOKEN_OTHER;
    t_amp.start = (const uint8_t *)"&";
    t_amp.length = 1;

    t_star.kind = CDD_TOKEN_STAR;
    t_star.start = (const uint8_t *)"*";
    t_star.length = 1;

    t_buf.kind = CDD_TOKEN_IDENTIFIER;
    t_buf.start = (const uint8_t *)"buf";
    t_buf.length = 3;

    t_not_ident.kind = CDD_TOKEN_NUMBER;
    t_not_ident.start = (const uint8_t *)"1";
    t_not_ident.length = 1;

    t_lbrk.kind = CDD_TOKEN_LBRACKET;
    t_lbrk.start = (const uint8_t *)"[";
    t_lbrk.length = 1;

    t_plus.kind = CDD_TOKEN_PLUS;
    t_plus.start = (const uint8_t *)"+";
    t_plus.length = 1;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* length != 1 */
        t_amp.length = 2;
        n1.tok = &t_amp;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        t_amp.length = 1;

        /* start[0] != '&' */
        n1.tok = &t_star;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        n1.tok = &t_amp;

        /* n1.next == NULL */
        n1.next = NULL;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        /* n1.next->type != 0 */
        n1.next = &n2;
        n2.type = 1;
        n2.tok = &t_buf;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        n2.type = 0;

        /* n2.tok->kind != CDD_TOKEN_IDENTIFIER */
        n2.tok = &t_not_ident;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        n2.tok = &t_buf;

        /* n2.next == NULL */
        n2.next = NULL;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        /* n2.next->type != 2 */
        n2.next = &n3;
        n3.type = 0;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        n3.type = 2;

        /* n3.tok->kind != CDD_TOKEN_LBRACKET */
        n3.tok = &t_star;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        /* buf + expr sub-conditions */
        n1.tok = &t_buf;
        n1.next = NULL;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        n1.next = &n2;
        n2.type = 1;
        n2.tok = &t_buf;
        emit_inferred_size(&dummy_bld, (void *)&n1);
        n2.type = 0;

        n2.tok = &t_star;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        n2.tok = &t_plus;
        n2.next = NULL;
        emit_inferred_size(&dummy_bld, (void *)&n1);

        /* malloc/calloc/realloc sub-conditions */
        n_call_test.type = 1;
        t_call_name.kind = CDD_TOKEN_IDENTIFIER;
        n_call_test.tok = &t_call_name;

        n_call_test.num_args = 1;
        t_call_name.start = (const uint8_t *)"f";
        t_call_name.length = 1;
        emit_inferred_size(&dummy_bld, (void *)&n_call_test);

        t_call_name.start = (const uint8_t *)"malloX";
        t_call_name.length = 6;
        emit_inferred_size(&dummy_bld, (void *)&n_call_test);

        n_call_test.num_args = 2;
        t_call_name.start = (const uint8_t *)"f";
        t_call_name.length = 1;
        emit_inferred_size(&dummy_bld, (void *)&n_call_test);

        t_call_name.start = (const uint8_t *)"calloX";
        t_call_name.length = 6;
        emit_inferred_size(&dummy_bld, (void *)&n_call_test);

        t_call_name.start = (const uint8_t *)"realloX";
        t_call_name.length = 7;
        emit_inferred_size(&dummy_bld, (void *)&n_call_test);
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* parse_expr_ast edge cases: unclosed and brace */
    cdd_cst_tree_t *t_brace = NULL;
    size_t s_idx = 0;
    void *ast_out = NULL;
    cdd_cst_node_t stmt;
    cdd_cst_child_t children[3];
    cdd_token_t tok_rbrace, tok_lbrace, tok_ident, tok_lparen;
    size_t test_idx = 0;

    memset(&stmt, 0, sizeof(stmt));
    memset(children, 0, sizeof(children));
    memset(&tok_rbrace, 0, sizeof(tok_rbrace));
    memset(&tok_lbrace, 0, sizeof(tok_lbrace));
    memset(&tok_ident, 0, sizeof(tok_ident));
    memset(&tok_lparen, 0, sizeof(tok_lparen));

    tok_rbrace.kind = CDD_TOKEN_RBRACE;
    tok_lbrace.kind = CDD_TOKEN_LBRACE;
    tok_ident.kind = CDD_TOKEN_IDENTIFIER;
    tok_lparen.kind = CDD_TOKEN_LPAREN;

    children[0].kind = CDD_CST_CHILD_TOKEN;
    children[0].val.token = &tok_rbrace;
    stmt.children = children;
    stmt.num_children = 1;

    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &ast_out);

    children[0].val.token = &tok_lbrace;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &ast_out);

    children[0].val.token = &tok_ident;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &ast_out);

    children[0].val.token = &tok_ident;
    children[1].kind = CDD_CST_CHILD_TOKEN;
    children[1].val.token = &tok_lparen;
    stmt.num_children = 2;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &ast_out);

    if (cdd_cst_parse(az_span_create_from_str("void f() { { int x; } }"),
                      &t_brace) == 0) {
      if (t_brace->root && t_brace->root->num_children > 0) {
        parse_expr_ast(t_brace->root->children[0].val.node, &s_idx, 0,
                       &ast_out);
      }
      cdd_cst_tree_free(t_brace);
    }
  }

  {
    /* check_needs_transform branch coverage */
    struct safe_crt_expr_test n_chk;
    cdd_token_t t_chk;
    const char *names[] = {"vprintf", "_vsnprintf", "vscanf", "vfscanf",
                           "vsscanf", "strlen",     "tmpfile"};
    size_t k;
    memset(&n_chk, 0, sizeof(n_chk));
    memset(&t_chk, 0, sizeof(t_chk));
    t_chk.kind = CDD_TOKEN_IDENTIFIER;
    n_chk.tok = &t_chk;

    /* type = 5 */
    n_chk.type = 5;
    t_chk.start = (const uint8_t *)"strcpy";
    t_chk.length = 6;
    check_needs_transform((void *)&n_chk);

    /* length >= 127 */
    {
      char long_name[130];
      memset(long_name, 'a', 130);
      t_chk.start = (const uint8_t *)long_name;
      t_chk.length = 130;
      check_needs_transform((void *)&n_chk);
    }

    n_chk.type = 1;
    for (k = 0; k < sizeof(names) / sizeof(names[0]); k++) {
      t_chk.start = (const uint8_t *)names[k];
      t_chk.length = strlen(names[k]);
      check_needs_transform((void *)&n_chk);
    }
  }

  {
    /* get_indent_string branch coverage */
    char out_ind[64];
    cdd_token_t tok_ind;
    cdd_trivia_t triv_ind;
    char long_spaces[70];
    memset(long_spaces, ' ', 68);
    long_spaces[68] = '\0';
    memset(&tok_ind, 0, sizeof(tok_ind));
    memset(&triv_ind, 0, sizeof(triv_ind));

    /* tok == NULL */
    get_indent_string(NULL, out_ind);

    /* last_ws->length >= 63 */
    triv_ind.kind = TRIVIA_WHITESPACE;
    triv_ind.start = (const uint8_t *)long_spaces;
    triv_ind.length = 68;
    tok_ind.leading_trivia = &triv_ind;
    get_indent_string(&tok_ind, out_ind);
  }

  {
    /* transform with NULL root */
    cdd_cst_tree_t empty_tree;
    cdd_transform_config_t cfg;
    memset(&empty_tree, 0, sizeof(empty_tree));
    memset(&cfg, 0, sizeof(cfg));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_transform_safe_crt(&empty_tree, &cfg));
  }

  {
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_emit;
    cdd_token_t t_emit;
    const char *emit_names[] = {"vfprintf", "vscanf", "_stricmp"};
    size_t k;

    memset(&n_emit, 0, sizeof(n_emit));
    memset(&t_emit, 0, sizeof(t_emit));
    t_emit.kind = CDD_TOKEN_IDENTIFIER;
    n_emit.type = 1;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* node->tok == NULL */
        n_emit.tok = NULL;
        emit_ast_bld_strip((void *)&n_emit, &dummy_bld, 0);

        n_emit.tok = &t_emit;
        for (k = 0; k < sizeof(emit_names) / sizeof(emit_names[0]); k++) {
          t_emit.start = (const uint8_t *)emit_names[k];
          t_emit.length = strlen(emit_names[k]);
          emit_ast_bld_strip((void *)&n_emit, &dummy_bld, 1);
        }
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    cdd_cst_node_t stmt;
    cdd_cst_child_t children[4];
    cdd_token_t t_ident, t_lpar, t_comma, t_rpar, t_semi;
    size_t test_idx = 0;
    void *out_expr = NULL;

    memset(&stmt, 0, sizeof(stmt));
    memset(children, 0, sizeof(children));
    memset(&t_ident, 0, sizeof(t_ident));
    memset(&t_lpar, 0, sizeof(t_lpar));
    memset(&t_comma, 0, sizeof(t_comma));
    memset(&t_rpar, 0, sizeof(t_rpar));
    memset(&t_semi, 0, sizeof(t_semi));

    t_ident.kind = CDD_TOKEN_IDENTIFIER;
    t_lpar.kind = CDD_TOKEN_LPAREN;
    t_comma.kind = CDD_TOKEN_COMMA;
    t_rpar.kind = CDD_TOKEN_RPAREN;
    t_semi.kind = CDD_TOKEN_SEMICOLON;

    /* foo(,) */
    children[0].kind = CDD_CST_CHILD_TOKEN;
    children[0].val.token = &t_ident;
    children[1].kind = CDD_CST_CHILD_TOKEN;
    children[1].val.token = &t_lpar;
    children[2].kind = CDD_CST_CHILD_TOKEN;
    children[2].val.token = &t_comma;
    children[3].kind = CDD_CST_CHILD_TOKEN;
    children[3].val.token = &t_rpar;
    stmt.children = children;
    stmt.num_children = 4;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &out_expr);

    /* foo(;) */
    children[2].val.token = &t_semi;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &out_expr);

    /* foo(a) ending at num_children */
    stmt.num_children = 3;
    children[2].val.token = &t_ident;
    test_idx = 0;
    parse_expr_ast(&stmt, &test_idx, 0, &out_expr);
  }

  {
    /* find_and_mark_fopen with long call name and lhs == curr */
    struct safe_crt_expr_test n_assign, n_call, n_semi;
    cdd_token_t t_assign, t_long_call, t_semi;
    int found_test = 0;
    char long_call_name[20];
    memset(long_call_name, 'x', 19);
    long_call_name[19] = '\0';

    memset(&n_assign, 0, sizeof(n_assign));
    memset(&n_call, 0, sizeof(n_call));
    memset(&n_semi, 0, sizeof(n_semi));
    memset(&t_assign, 0, sizeof(t_assign));
    memset(&t_long_call, 0, sizeof(t_long_call));
    memset(&t_semi, 0, sizeof(t_semi));

    t_semi.kind = CDD_TOKEN_SEMICOLON;
    t_assign.kind = CDD_TOKEN_ASSIGN;
    t_long_call.kind = CDD_TOKEN_IDENTIFIER;
    t_long_call.start = (const uint8_t *)long_call_name;
    t_long_call.length = 19;

    n_assign.type = 0;
    n_assign.tok = &t_assign;
    n_assign.next = &n_call;

    n_call.type = 1;
    n_call.tok = &t_long_call;

    find_and_mark_fopen((void *)&n_assign, &found_test);

    /* fopen with semicolon before assign so lhs_start is NULL and t is NULL */
    n_semi.type = 0;
    n_semi.tok = &t_semi;
    n_semi.next = &n_assign;
    t_long_call.start = (const uint8_t *)"fopen";
    t_long_call.length = 5;
    find_and_mark_fopen((void *)&n_semi, &found_test);
  }

  {
    /* infer_buffer_size with NULL current_tree->root and NULL dest */
    cdd_cst_tree_t tree_null_root;
    cdd_cst_builder_t bld_null_root;
    cdd_cst_node_t dummy_tgt;
    struct safe_crt_expr_test n_dest;
    cdd_token_t t_dest;
    memset(&tree_null_root, 0, sizeof(tree_null_root));
    memset(&bld_null_root, 0, sizeof(bld_null_root));
    memset(&dummy_tgt, 0, sizeof(dummy_tgt));
    memset(&n_dest, 0, sizeof(n_dest));
    memset(&t_dest, 0, sizeof(t_dest));

    t_dest.kind = CDD_TOKEN_IDENTIFIER;
    t_dest.start = (const uint8_t *)"buf";
    t_dest.length = 3;
    n_dest.type = 0;
    n_dest.tok = &t_dest;

    cdd_cst_builder_init(&bld_null_root, &tree_null_root, &dummy_tgt);

    /* current_tree->root is NULL */
    emit_inferred_size(&bld_null_root, (void *)&n_dest);

    /* dest is NULL */
    tree_null_root.root = &dummy_tgt;
    emit_inferred_size(&bld_null_root, NULL);
  }
  PASS();
}

struct my_expr_t {
  int type;
  cdd_token_t *tok;
  cdd_token_t *close_tok;
  void *args[16];
  size_t num_args;
  void *next;
};

TEST test_cdd_transform_safe_crt_emit_oom(void) {
  struct my_expr_t node;
  struct my_expr_t arg;
  cdd_cst_builder_t bld;
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *target = NULL;
  cdd_c_error_t rc;
  int j;
  (void)rc;

  memset(&node, 0, sizeof(node));
  memset(&arg, 0, sizeof(arg));
  memset(&bld, 0, sizeof(bld));

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str("void f() {}"), &tree));
  cdd_cst_alloc_node(CDD_CST_UNKNOWN, &target);
  cdd_cst_builder_init(&bld, tree, target);

  node.type = 1; /* type=1 is function call */
  node.tok = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
  node.tok->kind = CDD_TOKEN_IDENTIFIER;
  node.tok->start = (const uint8_t *)"_ecvt";
  node.tok->length = 5;
  node.close_tok = NULL;
  node.num_args = 1;
  node.args[0] = &arg;

  arg.type = 0;
  arg.tok = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
  arg.tok->kind = CDD_TOKEN_IDENTIFIER;
  arg.tok->start = (const uint8_t *)"d";
  arg.tok->length = 1;

  for (j = 1; j <= 5; j++) {
    extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
    g_cdd_cst_alloc_token_fail = j;
    emit_ast_bld_strip(&node, &bld, 0); /* 0 means is_msc = false */
    g_cdd_cst_alloc_token_fail = 0;
  }

  free(arg.tok);
  free(node.tok);
  cdd_cst_tree_free(tree);

  PASS();
}

SUITE(transformer_safe_crt_internals_suite) {
  RUN_TEST(test_cdd_transform_safe_crt_emit_oom);
  RUN_TEST(test_cdd_transform_safe_crt_direct_internals);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_SAFE_CRT_INTERNALS_H */
