#ifndef TEST_ORCHESTRATOR_INTERNALS_H
#define TEST_ORCHESTRATOR_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "functions/parse/orchestrator.h"
#include "functions/parse/tokenizer.h"
#include <greatest.h>
/* clang-format on */

extern C_CDD_EXPORT int g_force_find_allocations_fail;
extern C_CDD_EXPORT int g_force_parse_tokens_fail;
extern C_CDD_EXPORT int g_force_tokenize_fail;
extern C_CDD_EXPORT int g_cdd_fail_stricmp;

TEST test_orchestrator_internals(void) {
  struct TokenList *tl = NULL;
  char *out_fix = NULL;
  struct TokenList dst;
  size_t out = 0;
  char *name = NULL;
  struct FixWalkContext ctx = {0};
  char *argv3[] = {(char *)(size_t)(size_t) "1", (char *)(size_t)(size_t) "2",
                   (char *)(size_t)(size_t) "3"};
  int is_src = 0;
  int eq = 0;
  int is_ptr = 0;
  int is_void = 0;
  char *type_str = NULL;

  tokenize(az_span_create_from_str((char *)(size_t) "int a = 1;"), &tl);

  /* Test get_token_slice error & null checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_token_slice(NULL, 0, 0, &dst));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_token_slice(tl, 0, 0, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_token_slice(tl, 100, 10, &dst));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, get_token_slice(tl, 2, 1, &dst));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            get_token_slice(tl, 0, tl->size + 1, &dst));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            get_token_slice(tl, tl->size + 1, tl->size + 1, &dst));

  /* Test find_token_in_range null & boundary cases */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            find_token_in_range(tl, 0, tl->size, TOKEN_LPAREN, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            find_token_in_range(NULL, 0, 10, TOKEN_LPAREN, &out));
  ASSERT_EQ(10, out);

  /* Test find_token_in_range not found */
  ASSERT_EQ(CDD_C_SUCCESS,
            find_token_in_range(tl, 0, tl->size, TOKEN_LPAREN, &out));
  ASSERT_EQ(tl->size, out);

  /* Test find_token_in_range start >= end */
  ASSERT_EQ(CDD_C_SUCCESS,
            find_token_in_range(tl, tl->size, tl->size, TOKEN_LPAREN, &out));
  ASSERT_EQ(tl->size, out);

  /* Test token_eq_str */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, token_eq_str(NULL, "a", &eq));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            token_eq_str(&tl->tokens[0], NULL, &eq));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            token_eq_str(&tl->tokens[0], "a", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, token_eq_str(&tl->tokens[0], "int", &eq));
  ASSERT_EQ(1, eq);
  ASSERT_EQ(CDD_C_SUCCESS, token_eq_str(&tl->tokens[0], "other", &eq));
  ASSERT_EQ(0, eq);

  /* Test extract_func_name invalid args & not found */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            extract_func_name(tl, 0, tl->size, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, extract_func_name(NULL, 0, 1, &name));
  ASSERT_EQ(CDD_C_SUCCESS, extract_func_name(tl, 0, tl->size, &name));
  ASSERT_EQ(NULL, name);

  {
    struct TokenList *tl_paren = NULL;
    tokenize(az_span_create_from_str((char *)(size_t) "()"), &tl_paren);
    ASSERT_EQ(CDD_C_SUCCESS,
              extract_func_name(tl_paren, 0, tl_paren->size, &name));
    ASSERT_EQ(NULL, name);
    free_token_list(tl_paren);
  }

  {
    struct TokenList *tl_paren = NULL;
    tokenize(az_span_create_from_str((char *)(size_t) "foo()"), &tl_paren);
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              extract_func_name(tl_paren, 0, tl_paren->size, &name));
    ASSERT_EQ(NULL, name);
    g_cdd_alloc_fail = 0;
    free_token_list(tl_paren);
  }

  /* Test join_tokens_str invalid args & allocation failure */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            join_tokens_str(tl, 0, tl->size, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, join_tokens_str(NULL, 0, 1, &name));
  {
    struct TokenList *tl_paren = NULL;
    tokenize(az_span_create_from_str((char *)(size_t) "void foo()"), &tl_paren);
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              join_tokens_str(tl_paren, 0, tl_paren->size, &name));
    ASSERT_EQ(NULL, name);
    g_cdd_alloc_fail = 0;
    free_token_list(tl_paren);
  }

  free_token_list(tl);

  /* Test analyze_signature_tokens null args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            analyze_signature_tokens(NULL, 0, 0, &is_ptr, &is_void, &type_str));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            analyze_signature_tokens(tl, 0, 0, NULL, &is_void, &type_str));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            analyze_signature_tokens(tl, 0, 0, &is_ptr, NULL, &type_str));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            analyze_signature_tokens(tl, 0, 0, &is_ptr, &is_void, NULL));

  /* Test analyze_signature_tokens when no LPAREN (lparen == body_start) */
  {
    struct TokenList *my_tl = NULL;
    tokenize(az_span_create_from_str((char *)(size_t) "int x;"), &my_tl);
    ASSERT_EQ(CDD_C_SUCCESS,
              analyze_signature_tokens(my_tl, 0, my_tl->size, &is_ptr, &is_void,
                                       &type_str));
    ASSERT_EQ(NULL, type_str);
    free_token_list(my_tl);
  }

  /* Test analyze_signature_tokens void as identifier */
  {
    struct TokenList *my_tl = NULL;
    tokenize(az_span_create_from_str((char *)(size_t) "void foo()"), &my_tl);
    /* Change keyword void to identifier */
    my_tl->tokens[0].kind = TOKEN_IDENTIFIER;

    ASSERT_EQ(CDD_C_SUCCESS, analyze_signature_tokens(my_tl, 0, 5, &is_ptr,
                                                      &is_void, &type_str));
    ASSERT_EQ(1, is_void);
    if (type_str)
      free(type_str);
    free_token_list(my_tl);
  }

  /* Test orchestrate_fix errors */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, orchestrate_fix(NULL, &out_fix));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, orchestrate_fix("void A() {}", NULL));

  g_force_parse_tokens_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, orchestrate_fix("void A() {}", &out_fix));
  g_force_parse_tokens_fail = 0;

  g_force_tokenize_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, orchestrate_fix("void A() {}", &out_fix));
  g_force_tokenize_fail = 0;

  g_force_find_allocations_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, orchestrate_fix("void A() {}", &out_fix));
  g_force_find_allocations_fail = 0;

  /* Test fix_code_main usage */
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, fix_code_main(0, NULL));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, fix_code_main(3, argv3));

  {
    char *argv_dir[1] = {(char *)(size_t)(size_t) "."};
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, fix_code_main(1, argv_dir));
  }

  {
    char *argv_null[1] = {NULL};
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fix_code_main(1, argv_null));
  }

  /* Test is_c_source */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, is_c_source("a.c", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, is_c_source(NULL, &is_src));
  ASSERT_EQ(CDD_C_SUCCESS, is_c_source("no_ext", &is_src));
  ASSERT_EQ(0, is_src);

  /* Test is_c_source stricmp failure */
  g_cdd_fail_stricmp = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, is_c_source("test.c", &is_src));
  g_cdd_fail_stricmp = 0;

  /* Test fix_file_callback invalid args & not a source */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fix_file_callback(NULL, &ctx));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fix_file_callback("a.c", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, fix_file_callback("ignore.txt", &ctx));

  /* Test fix_file_callback is_c_source failure */
  g_cdd_fail_stricmp = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, fix_file_callback("test.c", &ctx));
  g_cdd_fail_stricmp = 0;

  /* Test fix_file_callback read failure */
  g_fail_io_after = 0;
  ASSERT_EQ(CDD_C_SUCCESS, fix_file_callback("does_not_exist.c", &ctx));
  g_fail_io_after = -1;

  /* Test fix_file_callback write failure */
  {
    const char *test_file =
        (char *)(size_t)(size_t) "test_orchestrator_internals.c";
    FILE *f;
    ctx.single_output_file = NULL;
#if defined(_MSC_VER)
    if (fopen_s(&f, test_file, "w") != 0)
      f = NULL;
#else
    f = fopen(test_file, "w");
#endif
    if (f) {
      fputs("void A() { malloc(1); }", f);
      fclose(f);
      g_fail_io_after = 1;
      ASSERT_EQ(CDD_C_SUCCESS, fix_file_callback(test_file, &ctx));
      g_fail_io_after = -1;
      remove(test_file);
    }
  }

  /* Test fix_file_callback orchestrator failure */
  {
    const char *test_file =
        (char *)(size_t)(size_t) "test_orchestrator_internals2.c";
    FILE *f;
    ctx.single_output_file = NULL;
#if defined(_MSC_VER)
    if (fopen_s(&f, test_file, "w") != 0)
      f = NULL;
#else
    f = fopen(test_file, "w");
#endif
    if (f) {
      fputs("void A() { malloc(1); }", f);
      fclose(f);
      g_force_parse_tokens_fail = 1;
      ASSERT_EQ(CDD_C_SUCCESS, fix_file_callback(test_file, &ctx));
      g_force_parse_tokens_fail = 0;
      remove(test_file);
    }
  }

  /* Test concat_strings invalid argument and NULL s2 */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, concat_strings("a", "b", "c", NULL));
  {
    char *concat_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, concat_strings("a", "b", NULL, &concat_out));
    ASSERT(concat_out != NULL);
    C_CDD_FREE(concat_out);
  }

  /* Test graph_add_node invalid arguments */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, graph_add_node(NULL, 0, "a"));
  {
    struct DependencyGraph g;
    struct FuncNode nodes[1];
    memset(&g, 0, sizeof(g));
    memset(nodes, 0, sizeof(nodes));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, graph_add_node(&g, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, graph_add_node(&g, 10, "a"));
    g.nodes = nodes;
    g.count = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, graph_add_node(&g, 0, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, graph_add_node(&g, 5, "fn"));
  }

  /* Test graph_add_edge invalid arguments */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, graph_add_edge(NULL, 0, 0));
  {
    struct DependencyGraph g;
    struct FuncNode nodes[2];
    memset(&g, 0, sizeof(g));
    memset(nodes, 0, sizeof(nodes));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, graph_add_edge(&g, 0, 10));
    g.nodes = nodes;
    g.count = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, graph_add_edge(&g, 5, 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, graph_add_edge(&g, 0, 5));
  }

  /* Test graph_free_contents with NULL and empty */
  ASSERT_EQ(CDD_C_SUCCESS, graph_free_contents(NULL));
  {
    struct DependencyGraph g;
    memset(&g, 0, sizeof(g));
    ASSERT_EQ(CDD_C_SUCCESS, graph_free_contents(&g));
  }

  /* Test propagate_refactor_mark invalid arguments & recursion error */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, propagate_refactor_mark(NULL, 0));
  {
    struct DependencyGraph g;
    memset(&g, 0, sizeof(g));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, propagate_refactor_mark(&g, 10));
  }
  {
    struct DependencyGraph g;
    struct FuncNode nodes[2];
    size_t caller = 999;
    memset(&g, 0, sizeof(g));
    memset(nodes, 0, sizeof(nodes));
    nodes[0].name = (char *)(size_t) "A";
    nodes[0].callers = &caller;
    nodes[0].num_callers = 1;
    g.nodes = nodes;
    g.count = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, propagate_refactor_mark(&g, 0));
  }

  /* Test token_eq_str failure inside orchestrate_fix */
  {
    const char *code = "void A() { malloc(1); }\nvoid B() { A(); }\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_fail_token_eq_str;
    g_cdd_fail_token_eq_str = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, orchestrate_fix(code, &out_str));
    g_cdd_fail_token_eq_str = 0;
  }

  /* Test propagate failure inside orchestrate_fix */
  {
    const char *code = "void A() { malloc(1); }\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_fail_propagate;
    g_cdd_fail_propagate = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, orchestrate_fix(code, &out_str));
    g_cdd_fail_propagate = 0;
  }

  /* Test anonymous function with strdup failure */
  {
    const char *code = "void () { malloc(1); }\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_strdup_fail;
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, orchestrate_fix(code, &out_str));
    g_cdd_strdup_fail = 0;
  }

  /* Test extract_func_name failure inside orchestrate_fix */
  {
    const char *code = "void A() { malloc(1); }\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_fail_extract_func_name;
    g_cdd_fail_extract_func_name = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, orchestrate_fix(code, &out_str));
    g_cdd_fail_extract_func_name = 0;
  }

  /* Test allocation_site_list_init failure inside orchestrate_fix */
  {
    const char *code = "void A() { malloc(1); }\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_fail_local_alloc_init;
    g_cdd_fail_local_alloc_init = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, orchestrate_fix(code, &out_str));
    g_cdd_fail_local_alloc_init = 0;
  }

  /* Test graph_add_node strdup failure inside orchestrate_fix */
  {
    const char *code = "void A() { malloc(1); }\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_strdup_fail;
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, orchestrate_fix(code, &out_str));
    g_cdd_strdup_fail = 0;
  }

  /* Test 10 allocation variables (triggers local_allocs realloc past initial
   * capacity 8) */
  {
    const char *code = "void A() {\n"
                       "  p1 = malloc(1);\n"
                       "  p2 = malloc(2);\n"
                       "  p3 = malloc(3);\n"
                       "  p4 = malloc(4);\n"
                       "  p5 = malloc(5);\n"
                       "  p6 = malloc(6);\n"
                       "  p7 = malloc(7);\n"
                       "  p8 = malloc(8);\n"
                       "  p9 = malloc(9);\n"
                       "  p10 = malloc(10);\n"
                       "}\n";
    char *out_str = NULL;
    int rc;
    int i;
    rc = orchestrate_fix(code, &out_str);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(out_str != NULL);
    C_CDD_FREE(out_str);
    out_str = NULL;

    /* OOM sweep on 10 allocs to hit realloc OOM */
    for (i = 1; i <= 35; ++i) {
      g_cdd_alloc_fail = i;
      orchestrate_fix(code, &out_str);
      if (out_str) {
        C_CDD_FREE(out_str);
        out_str = NULL;
      }
      g_cdd_alloc_fail = 0;
    }
  }

  /* Test concat_strings failures inside orchestrate_fix */
  {
    const char *code = "void A() { malloc(1); }\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_fail_concat_strings;
    g_cdd_fail_concat_strings = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, orchestrate_fix(code, &out_str));
    g_cdd_fail_concat_strings = 0;
  }
  {
    const char *code = "#define FOO 1\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_fail_concat_strings;
    g_cdd_fail_concat_strings = 1; /* Fails on non-function concat */
    ASSERT_EQ(CDD_C_ERROR_MEMORY, orchestrate_fix(code, &out_str));
    g_cdd_fail_concat_strings = 0;
  }
  {
    const char *code = "#define FOO 1\n";
    char *out_str = NULL;
    int i;
    for (i = 1; i <= 10; ++i) {
      g_cdd_alloc_fail = i;
      orchestrate_fix(code, &out_str);
      if (out_str) {
        C_CDD_FREE(out_str);
        out_str = NULL;
      }
      g_cdd_alloc_fail = 0;
    }
  }

  /* Test mock condition evaluation when counter > 1 */
  {
    extern C_CDD_EXPORT int g_cdd_fail_token_eq_str;
    extern C_CDD_EXPORT int g_cdd_fail_extract_func_name;
    extern C_CDD_EXPORT int g_cdd_fail_propagate;
    extern C_CDD_EXPORT int g_cdd_fail_local_alloc_init;
    extern C_CDD_EXPORT int g_cdd_fail_get_token_slice;
    int eq_mock = 0;
    char *str_mock = NULL;
    struct DependencyGraph g_mock;
    struct FuncNode nodes_mock[1];
    struct TokenList dst_mock;

    g_cdd_fail_token_eq_str = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, token_eq_str(NULL, "a", &eq_mock));
    g_cdd_fail_token_eq_str = 0;

    g_cdd_fail_extract_func_name = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              extract_func_name(NULL, 0, 0, &str_mock));
    g_cdd_fail_extract_func_name = 0;

    memset(&g_mock, 0, sizeof(g_mock));
    memset(nodes_mock, 0, sizeof(nodes_mock));
    g_mock.nodes = nodes_mock;
    g_mock.count = 1;
    g_cdd_fail_propagate = 2;
    ASSERT_EQ(CDD_C_SUCCESS, propagate_refactor_mark(&g_mock, 0));
    g_cdd_fail_propagate = 0;

    g_cdd_fail_get_token_slice = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              get_token_slice(NULL, 0, 0, &dst_mock));
    g_cdd_fail_get_token_slice = 0;
  }

  /* Test get_token_slice failure in orchestrate_fix */
  {
    const char *code = "void A() { malloc(1); }\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_fail_get_token_slice;
    g_cdd_fail_get_token_slice = 1;
    orchestrate_fix(code, &out_str);
    if (out_str) {
      C_CDD_FREE(out_str);
      out_str = NULL;
    }
    g_cdd_fail_get_token_slice = 0;
  }

  /* Test multiple functions with earlier function containing no allocs */
  {
    const char *code = "void F1() { int x = 1; }\nvoid F2() { malloc(1); }\n";
    char *out_str = NULL;
    int rc = orchestrate_fix(code, &out_str);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(out_str != NULL);
    C_CDD_FREE(out_str);
  }

  /* Test identifier at end of function without LPAREN */
  {
    const char *code = "void A() { int x = y; }\n";
    char *out_str = NULL;
    int rc = orchestrate_fix(code, &out_str);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(out_str != NULL);
    C_CDD_FREE(out_str);
  }

  /* Test fix_code_main argv NULL checks */
  {
    FILE *f_tmp;
#if defined(_MSC_VER)
    if (fopen_s(&f_tmp, "test_empty.c", "w") != 0)
      f_tmp = NULL;
#else
    f_tmp = fopen("test_empty.c", "w");
#endif
    if (f_tmp) {
      fputs("int main(){}\n", f_tmp);
      fclose(f_tmp);
    }
    {
      char *argv_null2[] = {(char *)(size_t) "test_empty.c", NULL};
      ASSERT_EQ(CDD_C_SUCCESS, fix_code_main(2, argv_null2));
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN, fix_code_main(1, NULL));
    }
    remove("test_empty.c");
  }

  /* Test second get_token_slice failure branch in orchestrate_fix */
  {
    const char *code = "void A() { malloc(1); }\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_fail_get_token_slice;
    g_cdd_fail_get_token_slice = 2;
    orchestrate_fix(code, &out_str);
    if (out_str) {
      C_CDD_FREE(out_str);
      out_str = NULL;
    }
    g_cdd_fail_get_token_slice = 0;
  }

  /* Test local_alloc_init mock branch when > 1 */
  {
    const char *code = "void A() { malloc(1); }\n";
    char *out_str = NULL;
    extern C_CDD_EXPORT int g_cdd_fail_local_alloc_init;
    g_cdd_fail_local_alloc_init = 2;
    orchestrate_fix(code, &out_str);
    if (out_str) {
      C_CDD_FREE(out_str);
      out_str = NULL;
    }
    g_cdd_fail_local_alloc_init = 0;
  }

  /* Test identifier as last token in body */
  {
    const char *code = "void A() { B";
    char *out_str = NULL;
    orchestrate_fix(code, &out_str);
    if (out_str) {
      C_CDD_FREE(out_str);
      out_str = NULL;
    }
  }

  PASS();
}

SUITE(orchestrator_internals_suite) { RUN_TEST(test_orchestrator_internals); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_ORCHESTRATOR_INTERNALS_H */
