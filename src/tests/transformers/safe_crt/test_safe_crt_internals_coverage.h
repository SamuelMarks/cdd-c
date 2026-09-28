/**
 * @file test_safe_crt_internals_coverage.h
 * @brief Direct internal coverage unit tests for the Safe CRT transformer.
 */

#ifndef TEST_CDD_TRANSFORM_SAFE_CRT_INTERNALS_COVERAGE_H
#define TEST_CDD_TRANSFORM_SAFE_CRT_INTERNALS_COVERAGE_H

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

TEST test_cdd_transform_safe_crt_direct_internals_coverage(void) {
  {
    /* infer_buffer_size Pattern 1 & 2 comprehensive branches */
    cdd_cst_tree_t tree_pat;
    cdd_cst_builder_t bld_pat;
    cdd_cst_node_t *root_node = NULL, *stmt_node = NULL,
                   *dummy_child_node = NULL;
    cdd_cst_child_t pat_children[12];
    cdd_token_t t_buf_id, t_lbrk_tok, t_type_id, t_assign_tok, t_malloc_tok,
        t_other_tok;
    cdd_token_t t_lpar, t_rpar, t_comma, t_arg;
    struct safe_crt_expr_test n_buf_dest;

    memset(&tree_pat, 0, sizeof(tree_pat));
    memset(&bld_pat, 0, sizeof(bld_pat));
    memset(pat_children, 0, sizeof(pat_children));
    memset(&t_buf_id, 0, sizeof(t_buf_id));
    memset(&t_lbrk_tok, 0, sizeof(t_lbrk_tok));
    memset(&t_type_id, 0, sizeof(t_type_id));
    memset(&t_assign_tok, 0, sizeof(t_assign_tok));
    memset(&t_malloc_tok, 0, sizeof(t_malloc_tok));
    memset(&t_other_tok, 0, sizeof(t_other_tok));
    memset(&t_lpar, 0, sizeof(t_lpar));
    memset(&t_rpar, 0, sizeof(t_rpar));
    memset(&t_comma, 0, sizeof(t_comma));
    memset(&t_arg, 0, sizeof(t_arg));
    memset(&n_buf_dest, 0, sizeof(n_buf_dest));

    t_lpar.kind = CDD_TOKEN_LPAREN;
    t_rpar.kind = CDD_TOKEN_RPAREN;
    t_comma.kind = CDD_TOKEN_COMMA;
    t_arg.kind = CDD_TOKEN_NUMBER;
    t_arg.start = (const uint8_t *)"1";
    t_arg.length = 1;

    t_buf_id.kind = CDD_TOKEN_IDENTIFIER;
    t_buf_id.start = (const uint8_t *)"buf";
    t_buf_id.length = 3;

    t_type_id.kind = CDD_TOKEN_IDENTIFIER;
    t_type_id.start = (const uint8_t *)"char";
    t_type_id.length = 4;

    t_lbrk_tok.kind = CDD_TOKEN_LBRACKET;
    t_assign_tok.kind = CDD_TOKEN_ASSIGN;
    t_other_tok.kind = CDD_TOKEN_NUMBER;

    t_malloc_tok.kind = CDD_TOKEN_IDENTIFIER;
    t_malloc_tok.start = (const uint8_t *)"malloc";
    t_malloc_tok.length = 6;

    n_buf_dest.type = 0;
    n_buf_dest.tok = &t_buf_id;

    if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &root_node) == 0 &&
        cdd_cst_alloc_node(CDD_CST_UNKNOWN, &stmt_node) == 0 &&
        cdd_cst_alloc_node(CDD_CST_EXPRESSION, &dummy_child_node) == 0) {
      tree_pat.root = root_node;
      cdd_cst_append_child_node(root_node, stmt_node);
      cdd_cst_builder_init(&bld_pat, &tree_pat, root_node);

      /* Case 1: j == 0 (t_buf_id is first child) -> j > 0 is false! */
      pat_children[0].kind = CDD_CST_CHILD_TOKEN;
      pat_children[0].val.token = &t_buf_id;
      pat_children[1].kind = CDD_CST_CHILD_TOKEN;
      pat_children[1].val.token = &t_type_id;
      stmt_node->children = pat_children;
      stmt_node->num_children = 2;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 2: j + 1 < stmt->num_children is false (j is last child) */
      pat_children[0].val.token = &t_type_id;
      pat_children[1].val.token = &t_buf_id;
      stmt_node->num_children = 2;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 3: children[j + 1].kind != CDD_CST_CHILD_TOKEN */
      pat_children[2].kind = CDD_CST_CHILD_NODE;
      pat_children[2].val.node = dummy_child_node;
      stmt_node->num_children = 3;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 4: children[j - 1].kind != CDD_CST_CHILD_TOKEN */
      pat_children[0].kind = CDD_CST_CHILD_NODE;
      pat_children[0].val.node = dummy_child_node;
      pat_children[2].kind = CDD_CST_CHILD_TOKEN;
      pat_children[2].val.token = &t_lbrk_tok;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 5: children[j - 1].val.token->kind != CDD_TOKEN_IDENTIFIER */
      pat_children[0].kind = CDD_CST_CHILD_TOKEN;
      pat_children[0].val.token = &t_other_tok;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 6: Pattern 2: children[j + 1] is node */
      pat_children[0].val.token = &t_buf_id;
      pat_children[1].kind = CDD_CST_CHILD_NODE;
      pat_children[1].val.node = dummy_child_node;
      pat_children[2].kind = CDD_CST_CHILD_TOKEN;
      pat_children[2].val.token = &t_other_tok;
      stmt_node->num_children = 3;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 7: Pattern 2: children[j + 1] is token, but not ASSIGN */
      pat_children[1].kind = CDD_CST_CHILD_TOKEN;
      pat_children[1].val.token = &t_other_tok;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 7b: Pattern 2: children[j + 1] is ASSIGN, children[j + 2] is node
       */
      pat_children[1].val.token = &t_assign_tok;
      pat_children[2].kind = CDD_CST_CHILD_NODE;
      pat_children[2].val.node = dummy_child_node;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 8: Pattern 2: children[j + 1] is ASSIGN, children[j + 2] is not
       * identifier */
      pat_children[2].kind = CDD_CST_CHILD_TOKEN;
      pat_children[2].val.token = &t_other_tok;
      pat_children[3].kind = CDD_CST_CHILD_TOKEN;
      pat_children[3].val.token = &t_other_tok;
      stmt_node->num_children = 4;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 9: m_tok->length < 6 */
      t_other_tok.kind = CDD_TOKEN_IDENTIFIER;
      t_other_tok.start = (const uint8_t *)"f";
      t_other_tok.length = 1;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 10: m_tok->length >= 6 but name is foobar */
      t_other_tok.start = (const uint8_t *)"foobar";
      t_other_tok.length = 6;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 11: m_expr with type != 1 */
      pat_children[2].val.token = &t_malloc_tok;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 12: calloc with num_args != 2 */
      t_malloc_tok.start = (const uint8_t *)"calloc";
      t_malloc_tok.length = 6;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 13: realloc with num_args != 2 */
      t_malloc_tok.start = (const uint8_t *)"realloc";
      t_malloc_tok.length = 7;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 14: Pattern 2: malloc with 2 args -> malloc(1, 2) */
      t_malloc_tok.start = (const uint8_t *)"malloc";
      t_malloc_tok.length = 6;
      pat_children[0].val.token = &t_buf_id;
      pat_children[1].kind = CDD_CST_CHILD_TOKEN;
      pat_children[1].val.token = &t_assign_tok;
      pat_children[2].kind = CDD_CST_CHILD_TOKEN;
      pat_children[2].val.token = &t_malloc_tok;
      pat_children[3].kind = CDD_CST_CHILD_TOKEN;
      pat_children[3].val.token = &t_lpar;
      pat_children[4].kind = CDD_CST_CHILD_TOKEN;
      pat_children[4].val.token = &t_arg;
      pat_children[5].kind = CDD_CST_CHILD_TOKEN;
      pat_children[5].val.token = &t_comma;
      pat_children[6].kind = CDD_CST_CHILD_TOKEN;
      pat_children[6].val.token = &t_arg;
      pat_children[7].kind = CDD_CST_CHILD_TOKEN;
      pat_children[7].val.token = &t_rpar;
      stmt_node->num_children = 8;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 15: Pattern 2: calloc with 3 args -> calloc(1, 2, 3) */
      t_malloc_tok.start = (const uint8_t *)"calloc";
      t_malloc_tok.length = 6;
      pat_children[7].val.token = &t_comma;
      pat_children[8].kind = CDD_CST_CHILD_TOKEN;
      pat_children[8].val.token = &t_arg;
      pat_children[9].kind = CDD_CST_CHILD_TOKEN;
      pat_children[9].val.token = &t_rpar;
      stmt_node->num_children = 10;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      /* Case 16: Pattern 2: realloc with 3 args -> realloc(1, 2, 3) */
      t_malloc_tok.start = (const uint8_t *)"realloc";
      t_malloc_tok.length = 7;
      emit_inferred_size(&bld_pat, (void *)&n_buf_dest);

      stmt_node->num_children = 0;
      stmt_node->children = NULL;
      free(dummy_child_node);
      free(stmt_node);
      free(root_node);
    }
  }

  {
    /* infer_buffer_size with NULL tok */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_null_tok;
    memset(&n_null_tok, 0, sizeof(n_null_tok));

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        n_null_tok.type = 0;
        n_null_tok.tok = NULL;
        emit_inferred_size(&dummy_bld, (void *)&n_null_tok);

        n_null_tok.type = 1;
        emit_inferred_size(&dummy_bld, (void *)&n_null_tok);

        /* node->type == 1 with tok->kind != CDD_TOKEN_IDENTIFIER */
        {
          cdd_token_t t_num;
          memset(&t_num, 0, sizeof(t_num));
          t_num.kind = CDD_TOKEN_NUMBER;
          t_num.start = (const uint8_t *)"1";
          t_num.length = 1;
          n_null_tok.tok = &t_num;
          emit_inferred_size(&dummy_bld, (void *)&n_null_tok);
        }
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* _putenv literal and non-literal branches in emit_ast_bld */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_putenv, n_str;
    cdd_token_t t_putenv, t_str;

    memset(&n_putenv, 0, sizeof(n_putenv));
    memset(&n_str, 0, sizeof(n_str));
    memset(&t_putenv, 0, sizeof(t_putenv));
    memset(&t_str, 0, sizeof(t_str));

    t_putenv.kind = CDD_TOKEN_IDENTIFIER;
    t_putenv.start = (const uint8_t *)"_putenv";
    t_putenv.length = 7;

    n_putenv.type = 1;
    n_putenv.tok = &t_putenv;
    n_putenv.num_args = 1;
    n_putenv.args[0] = &n_str;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* Case 0: str_node == NULL */
        n_putenv.args[0] = NULL;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);
        n_putenv.args[0] = &n_str;

        /* Case 1: str_node->type != 0 */
        n_str.type = 2;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 2: str_node->tok == NULL */
        n_str.type = 0;
        n_str.tok = NULL;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 3: str_node->tok->length == 2 */
        n_str.tok = &t_str;
        t_str.kind = CDD_TOKEN_IDENTIFIER;
        t_str.start = (const uint8_t *)"LL";
        t_str.length = 2;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 4: str_node->tok->start[0] != 'L' */
        t_str.start = (const uint8_t *)"M";
        t_str.length = 1;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 4b: str_node->tok->start[0] == 'L' but next is NULL */
        t_str.start = (const uint8_t *)"L";
        t_str.length = 1;
        n_str.next = NULL;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 4c: str_node->tok->start[0] == 'L', next has type != 0 */
        {
          struct safe_crt_expr_test n_next_str;
          memset(&n_next_str, 0, sizeof(n_next_str));
          n_str.next = &n_next_str;
          n_next_str.type = 2;
          emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

          /* Case 4d: str_node->tok->start[0] == 'L', next has type == 0 and tok
           * == NULL */
          n_next_str.type = 0;
          n_next_str.tok = NULL;
          emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);
          n_str.next = NULL;
        }

        /* Case 5: string starting with L\" */
        t_str.kind = CDD_TOKEN_STRING;
        t_str.start = (const uint8_t *)"L\"A=B\"";
        t_str.length = 6;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 6: string starting with = */
        t_str.start = (const uint8_t *)"\"=B\"";
        t_str.length = 4;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 7: _wputenv with L\"A=B\" */
        t_putenv.start = (const uint8_t *)"_wputenv";
        t_putenv.length = 8;
        t_str.start = (const uint8_t *)"L\"A=B\"";
        t_str.length = 6;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 8: string starting with ' */
        t_str.start = (const uint8_t *)"'A=B'";
        t_str.length = 5;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);

        /* Case 9: string starting with LX=Y */
        t_str.start = (const uint8_t *)"LX=Y";
        t_str.length = 4;
        emit_ast_bld_strip((void *)&n_putenv, &dummy_bld, 1);
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* sscanf format argument variations */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_scanf, n_arg0, n_fmt;
    cdd_token_t t_scanf_id;

    memset(&n_scanf, 0, sizeof(n_scanf));
    memset(&n_arg0, 0, sizeof(n_arg0));
    memset(&n_fmt, 0, sizeof(n_fmt));
    memset(&t_scanf_id, 0, sizeof(t_scanf_id));

    t_scanf_id.kind = CDD_TOKEN_IDENTIFIER;
    t_scanf_id.start = (const uint8_t *)"sscanf";
    t_scanf_id.length = 6;

    n_scanf.type = 1;
    n_scanf.tok = &t_scanf_id;
    n_scanf.num_args = 2;
    n_scanf.args[0] = &n_arg0;
    n_scanf.args[1] = NULL; /* args[1] is NULL */

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* args[format_idx] is NULL */
        emit_ast_bld_strip((void *)&n_scanf, &dummy_bld, 1);

        /* args[format_idx]->type != 0 */
        n_scanf.args[1] = &n_fmt;
        n_fmt.type = 2;
        emit_ast_bld_strip((void *)&n_scanf, &dummy_bld, 1);

        /* args[format_idx]->type == 0, tok == NULL */
        n_fmt.type = 0;
        n_fmt.tok = NULL;
        emit_ast_bld_strip((void *)&n_scanf, &dummy_bld, 1);
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* node->type == 2 and is_safe == 1 with clone_token failure */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_paren, n_inner;
    cdd_token_t t_paren;

    memset(&n_paren, 0, sizeof(n_paren));
    memset(&n_inner, 0, sizeof(n_inner));
    memset(&t_paren, 0, sizeof(t_paren));

    t_paren.kind = CDD_TOKEN_LPAREN;
    t_paren.start = (const uint8_t *)"(";
    t_paren.length = 1;

    n_paren.type = 2;
    n_paren.tok = &t_paren;
    n_paren.args[0] = &n_inner;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        g_safe_crt_malloc_fail = 1;
        emit_ast_bld_strip((void *)&n_paren, &dummy_bld, 0);
        g_safe_crt_malloc_fail = 0;
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* node->type == 3 branches */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_type3, n_lhs_t3, n_call_t3;
    cdd_token_t t_fopen;

    memset(&n_type3, 0, sizeof(n_type3));
    memset(&n_lhs_t3, 0, sizeof(n_lhs_t3));
    memset(&n_call_t3, 0, sizeof(n_call_t3));
    memset(&t_fopen, 0, sizeof(t_fopen));

    t_fopen.kind = CDD_TOKEN_IDENTIFIER;
    t_fopen.start = (const uint8_t *)"fopen";
    t_fopen.length = 5;

    n_type3.type = 3;
    n_type3.args[0] = &n_lhs_t3;
    n_type3.args[1] = &n_call_t3;

    n_call_t3.type = 1;
    n_call_t3.tok = &t_fopen;

    n_lhs_t3.type = 0;
    n_lhs_t3.tok = NULL;
    n_lhs_t3.next = &n_type3;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        emit_ast_bld_strip((void *)&n_type3, &dummy_bld, 1);
        emit_ast_bld_strip((void *)&n_type3, &dummy_bld, 0);

        /* call with name length >= 127 */
        {
          struct safe_crt_expr_test n_long_call;
          cdd_token_t t_long;
          char long_name[140];
          memset(long_name, 'a', 139);
          long_name[139] = '\0';
          memset(&t_long, 0, sizeof(t_long));
          t_long.kind = CDD_TOKEN_IDENTIFIER;
          t_long.start = (const uint8_t *)long_name;
          t_long.length = 139;
          memset(&n_long_call, 0, sizeof(n_long_call));
          n_long_call.type = 1;
          n_long_call.tok = &t_long;
          emit_ast_bld_strip((void *)&n_long_call, &dummy_bld, 0);
        }
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* clone_token fail in emit_inferred_size */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_char_arr;
    cdd_token_t t_char_arr;
    memset(&n_char_arr, 0, sizeof(n_char_arr));
    memset(&t_char_arr, 0, sizeof(t_char_arr));
    t_char_arr.kind = CDD_TOKEN_IDENTIFIER;
    t_char_arr.start = (const uint8_t *)"buf";
    t_char_arr.length = 3;
    n_char_arr.type = 0;
    n_char_arr.tok = &t_char_arr;

    if (cdd_cst_parse(az_span_create_from_str("void f() { char buf[10]; }"),
                      &dummy_tree) == 0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        /* sizeof (1st), ( (2nd), clone_token (3rd) */
        g_cdd_cst_alloc_token_fail = 3;
        emit_inferred_size(&dummy_bld, (void *)&n_char_arr);
        g_cdd_cst_alloc_token_fail = 0;
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* clone_token failure in emit_ast_bld */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_strcpy, n_paren, n_func, n_assign_test,
        n_lhs_var, n_lhs_type, n_call_fn;
    cdd_token_t t_scpy, t_paren_close, t_func_id, t_assign_op, t_var, t_fn;
    int k;

    memset(&n_strcpy, 0, sizeof(n_strcpy));
    memset(&n_paren, 0, sizeof(n_paren));
    memset(&n_func, 0, sizeof(n_func));
    memset(&n_assign_test, 0, sizeof(n_assign_test));
    memset(&n_lhs_var, 0, sizeof(n_lhs_var));
    memset(&n_lhs_type, 0, sizeof(n_lhs_type));
    memset(&n_call_fn, 0, sizeof(n_call_fn));

    memset(&t_scpy, 0, sizeof(t_scpy));
    memset(&t_paren_close, 0, sizeof(t_paren_close));
    memset(&t_func_id, 0, sizeof(t_func_id));
    memset(&t_assign_op, 0, sizeof(t_assign_op));
    memset(&t_var, 0, sizeof(t_var));
    memset(&t_fn, 0, sizeof(t_fn));

    t_scpy.kind = CDD_TOKEN_IDENTIFIER;
    t_scpy.start = (const uint8_t *)"strcpy";
    t_scpy.length = 6;

    t_paren_close.kind = CDD_TOKEN_RPAREN;
    t_paren_close.start = (const uint8_t *)")";
    t_paren_close.length = 1;

    t_func_id.kind = CDD_TOKEN_IDENTIFIER;
    t_func_id.start = (const uint8_t *)"getenv";
    t_func_id.length = 6;

    t_assign_op.kind = CDD_TOKEN_ASSIGN;
    t_var.kind = CDD_TOKEN_IDENTIFIER;
    t_var.start = (const uint8_t *)"f";
    t_var.length = 1;

    t_fn.kind = CDD_TOKEN_IDENTIFIER;
    t_fn.start = (const uint8_t *)"fopen";
    t_fn.length = 5;

    n_strcpy.type = 1;
    n_strcpy.tok = &t_scpy;
    n_strcpy.close_tok = &t_paren_close;

    n_paren.type = 2;
    n_paren.tok = &t_paren_close;
    n_paren.close_tok = &t_paren_close;

    n_func.type = 1;
    n_func.tok = &t_func_id;

    n_assign_test.type = 3;
    n_assign_test.tok = &t_assign_op;
    n_assign_test.args[0] = &n_lhs_type;
    n_assign_test.args[1] = &n_call_fn;
    n_lhs_type.type = 0;
    n_lhs_type.tok = &t_var;
    n_lhs_type.next = &n_lhs_var;
    n_lhs_var.type = 0;
    n_lhs_var.tok = &t_var;
    n_lhs_var.next = &n_assign_test;
    n_call_fn.type = 1;
    n_call_fn.tok = &t_fn;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* Fail on strcpy (is_safe = 1, line 984) */
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        g_cdd_cst_alloc_token_fail = 1;
        emit_ast_bld_strip((void *)&n_strcpy, &dummy_bld, 1);
        g_cdd_cst_alloc_token_fail = 0;

        /* Fail on close_tok (line 1470) */
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        g_cdd_cst_alloc_token_fail = 3;
        emit_ast_bld_strip((void *)&n_strcpy, &dummy_bld, 1);
        g_cdd_cst_alloc_token_fail = 0;

        /* Fail on type 2 close_tok (line 879) */
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        g_cdd_cst_alloc_token_fail = 2;
        emit_ast_bld_strip((void *)&n_paren, &dummy_bld, 0);
        g_cdd_cst_alloc_token_fail = 0;

        /* Fail on getenv (line 1012) */
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
        g_cdd_cst_alloc_token_fail = 1;
        emit_ast_bld_strip((void *)&n_func, &dummy_bld, 1);
        g_cdd_cst_alloc_token_fail = 0;

        /* Fail on node->type == 3 tokens */
        for (k = 1; k <= 10; k++) {
          cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);
          g_cdd_cst_alloc_token_fail = k;
          emit_ast_bld_strip((void *)&n_assign_test, &dummy_bld, 1);
          g_cdd_cst_alloc_token_fail = 0;
        }
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* clone_token with NULL out_tok */
    cdd_token_t t_clone;
    memset(&t_clone, 0, sizeof(t_clone));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, clone_token(NULL, &t_clone, NULL));
  }

  {
    /* emit_ast_bld_strip_ampersand branches */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_amp;
    cdd_token_t t_amp;

    memset(&n_amp, 0, sizeof(n_amp));
    memset(&t_amp, 0, sizeof(t_amp));
    t_amp.kind = CDD_TOKEN_OTHER;
    t_amp.start = (const uint8_t *)"&";
    t_amp.length = 1;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        /* node == NULL */
        emit_ast_bld_strip_ampersand(NULL, &dummy_bld, 0);

        /* node.type = 1 */
        n_amp.type = 1;
        emit_ast_bld_strip_ampersand(&n_amp, &dummy_bld, 0);

        /* node.type = 0, tok = NULL */
        n_amp.type = 0;
        n_amp.tok = NULL;
        emit_ast_bld_strip_ampersand(&n_amp, &dummy_bld, 0);

        /* node.type = 0, tok->length = 2 */
        n_amp.tok = &t_amp;
        t_amp.length = 2;
        emit_ast_bld_strip_ampersand(&n_amp, &dummy_bld, 0);
        t_amp.length = 1;

        /* node.type = 0, length = 1, start[0] = '&' */
        emit_ast_bld_strip_ampersand(&n_amp, &dummy_bld, 0);
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  {
    /* cdd_transform_safe_crt with empty statement and statement with node child
     */
    cdd_cst_tree_t *t_empty_stmt = NULL;
    cdd_cst_node_t *empty_node = NULL, *node_child_stmt = NULL,
                   *sub_node = NULL;
    cdd_transform_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &t_empty_stmt) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &empty_node) == 0 &&
          cdd_cst_alloc_node(CDD_CST_UNKNOWN, &node_child_stmt) == 0 &&
          cdd_cst_alloc_node(CDD_CST_EXPRESSION, &sub_node) == 0) {
        /* empty_node has 0 children */
        cdd_cst_append_child_node(t_empty_stmt->root, empty_node);

        /* node_child_stmt has a node child */
        cdd_cst_append_child_node(node_child_stmt, sub_node);
        cdd_cst_append_child_node(t_empty_stmt->root, node_child_stmt);

        cdd_transform_safe_crt(t_empty_stmt, &cfg);
      }
      cdd_cst_tree_free(t_empty_stmt);
    }
  }

  {
    /* node->close_tok failure in emit_ast_bld */
    cdd_cst_tree_t *dummy_tree = NULL;
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_builder_t dummy_bld;
    struct safe_crt_expr_test n_call_close;
    cdd_token_t t_call_id, t_close_id;

    memset(&n_call_close, 0, sizeof(n_call_close));
    memset(&t_call_id, 0, sizeof(t_call_id));
    memset(&t_close_id, 0, sizeof(t_close_id));

    t_call_id.kind = CDD_TOKEN_IDENTIFIER;
    t_call_id.start = (const uint8_t *)"printf";
    t_call_id.length = 6;

    t_close_id.kind = CDD_TOKEN_RPAREN;
    t_close_id.start = (const uint8_t *)")";
    t_close_id.length = 1;

    n_call_close.type = 1;
    n_call_close.tok = &t_call_id;
    n_call_close.close_tok = &t_close_id;

    if (cdd_cst_parse(az_span_create_from_str("void f() {}"), &dummy_tree) ==
        0) {
      if (cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node) == 0) {
        cdd_cst_builder_init(&dummy_bld, dummy_tree, dummy_node);

        g_safe_crt_malloc_fail = 2;
        emit_ast_bld_strip((void *)&n_call_close, &dummy_bld, 0);
        g_safe_crt_malloc_fail = 0;
      }
      cdd_cst_tree_free(dummy_tree);
    }
  }

  PASS();
}

SUITE(transformer_safe_crt_internals_coverage_suite) {
  RUN_TEST(test_cdd_transform_safe_crt_direct_internals_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_SAFE_CRT_INTERNALS_COVERAGE_H */
