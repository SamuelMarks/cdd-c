/**
 * @file test_gnu_standardizer_internals_coverage.h
 * @brief Grammar branch and coverage unit tests for the GNU standardizer
 * transformer.
 */

#ifndef TEST_GNU_STANDARDIZER_INTERNALS_COVERAGE_H
#define TEST_GNU_STANDARDIZER_INTERNALS_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include "../../../transformers/gnu_standardizer/gnu_standardizer_internal.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Test definition g_gnu_alloc_fail. */
extern C_CDD_EXPORT volatile int g_gnu_alloc_fail;
/** @brief Test definition g_gnu_replace_fail. */
extern C_CDD_EXPORT volatile int g_gnu_replace_fail;
/** @brief Test definition g_gnu_bld_fail. */
extern C_CDD_EXPORT volatile int g_gnu_bld_fail;
/** @brief Test definition g_gnu_malloc_fail. */
extern C_CDD_EXPORT volatile int g_gnu_malloc_fail;
/** @brief Test definition g_gnu_find_fail. */
extern C_CDD_EXPORT volatile int g_gnu_find_fail;
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;

/**
 * @brief Unit test test_gnu_standardizer_reach_100_percent_coverage.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_reach_100_percent_coverage(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_tree_t empty_tree;
  cdd_transform_config_t config;
  size_t i;
  static const char *const snippets[] = {
      "#define OBJ_MACRO 123\n"
      "#define EMPTY_MACRO\n"
      "#define ADD(a, b) ((a) + (b))\n"
      "#define LOG(my_arg...) fn(my_arg)\n"
      "#define FOO(args...) foo(##args, ##args_extra, args, args_extra)\n"
      "#define VA_MACRO(...) fn(__VA_ARGS__)\n"
      "#define TEST_MACRO(my_args, a, args...) foo(##a, ##args, ##args1, "
      "##args_extra, a, args, args1, args_extra, myargs, _args)\n",

      "int dummy_identifier19 = 1;\n"
      "int dummy_length_16_x = 2;\n"
      "int dummy_len_23_identifier = 3;\n"
      "int dummy_len_17_ident = 4;\n"
      "int dummy_len11 = 5;\n"
      "int dummy_len9 = 6;\n"
      "int dummy_len_13_x = 7;\n"
      "int alignof_x = 8;\n",

      "void __attribute__((noinline)) f1(void);\n"
      "void __attribute__((always)) f2(void);\n"
      "void __attribute__((transparent_unit_)) f3(void);\n"
      "void __attribute__((vector_wide)) f4(void);\n"
      "void __attribute__((cleanup)) f5(void);\n"
      "void __attribute__((pure)) f6(void);\n",

      "typedef int QItype __attribute__((mode(QI)));\n"
      "typedef int HItype __attribute__((mode(HI)));\n"
      "typedef int SItype __attribute__((mode(SI)));\n"
      "typedef int DItype __attribute__((mode(DI)));\n"
      "typedef int TItype __attribute__((mode(TI)));\n"
      "typedef int XXtype __attribute__((mode(XX)));\n"
      "typedef int Xtype __attribute__((mode(X)));\n",

      "unsigned __int128 h1 = 0X10000000000000000;\n"
      "unsigned __int128 o1 = 01777777777777777777777;\n"
      "volatile __int128 v128;\n"
      "__int128 i128;\n"
      "_Decimal32 d32; _Fract fr;\n"
      "_Decimal64 d64; _Decimal128 d128; _Accum acc;\n"
      "__fp16 f16; _Float16 fl16; __bf16 bf;\n"
      "typeof_unqual(const volatile restrict unsigned float) unq;\n",

      "union MyU { int a; }; union MyU u = {};\n"
      "enum MyE { E0 }; enum MyE e = {};\n"
      "int myvar = {};\n"
      "int item = {};\n",

      "int test_cases(int x) {\n"
      "  switch(x) { case 5 ... 1: break; default: break; }\n"
      "  return 0;\n"
      "}\n"
      "int test_neg(int x) {\n"
      "  switch(x) { case -5 ... -1: break; case 1 ... -1: break; }\n"
      "  return 0;\n"
      "}\n"
      "int r1[10] = { [5 ... 1] = 0 };\n"
      "int r2[10] = { [0 ... 3] };\n"
      "int r3[10] = { [-5 ... -1] = 1 };\n",

      "void test_elvis(int x) {\n"
      "  int item = x ?: 0;\n"
      "  int value = x ?: 0;\n"
      "  int num = x ?: 0;\n"
      "  int id = x ?: 0;\n"
      "  int result = x ?: 0;\n"
      "  if (x ?: 0) {}\n"
      "  while (x ?: 0) {}\n"
      "  for (x ?: 0; ; ) break;\n"
      "  switch (x ?: 0) { case 1 ?: 0: break; }\n"
      "  do {} while (x ?: 0);\n"
      "}\n",

      "int test_ret_cast(char c) { return (char)c; }\n"
      "int test_exp(int x) {\n"
      "  if (__builtin_expect(x, 1)) return 1;\n"
      "  if (__builtin_expect(x)) return 2;\n"
      "  return 0;\n"
      "}\n"
      "int test_amp(int x) { int *p = &x; return *p; }\n"
      "void f_nest(void) { { void g_nest(void) {} } }\n",

      "void clean_fn(int *p) {}\n"
      "void f_clean(void) { int __attribute__((cleanup(clean_fn))) a, b; }\n"
      "void f_addr_lbl(void) { __label__ l1; void *p = &&l1; goto l1; l1:; }\n"
      "void f_vla_params(int n, int a[n], int b[n]) {}\n"
      "int f_vla_ret(int n) { int a[n]; return 1; }\n"};

  memset(&config, 0, sizeof(config));
  config.fallback_vla_to_malloc = 1;
  memset(&empty_tree, 0, sizeof(empty_tree));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_gnu(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_gnu(&empty_tree, NULL));

  /* Test replace_token_with_text edge cases directly */
  {
    cdd_token_t dummy_t;
    cdd_cst_tree_t tr;
    cdd_cst_tree_t *tr_valid = NULL;
    memset(&dummy_t, 0, sizeof(dummy_t));
    memset(&tr, 0, sizeof(tr));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        replace_token_with_text(NULL, &dummy_t, CDD_TOKEN_IDENTIFIER, "", 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              replace_token_with_text(&tr, NULL, CDD_TOKEN_IDENTIFIER, "", 0));
    ASSERT_NEQ(CDD_C_SUCCESS, replace_token_with_text(
                                  &tr, &dummy_t, CDD_TOKEN_IDENTIFIER, "", 0));

    if (cdd_cst_parse(az_span_create_from_str("int x;"), &tr_valid) == 0 &&
        tr_valid) {
      g_cdd_cst_alloc_token_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                replace_token_with_text(tr_valid,
                                        &tr_valid->base_tokens->tokens[0],
                                        CDD_TOKEN_IDENTIFIER, "", 0));
      g_cdd_cst_alloc_token_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(az_span_create_from_str("void f(void) {}\n"),
                      &tr_valid) == 0 &&
        tr_valid) {
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_cdd_alloc_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(az_span_create_from_str("typeof(1) x;\n"), &tr_valid) ==
            0 &&
        tr_valid) {
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_cdd_alloc_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(az_span_create_from_str(
                          "unsigned __int128 x = 0x10000000000000000;\n"),
                      &tr_valid) == 0 &&
        tr_valid) {
      g_cdd_alloc_fail = 3;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_cdd_alloc_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(az_span_create_from_str(
                          "unsigned __int128 x = 18446744073709551616;\n"),
                      &tr_valid) == 0 &&
        tr_valid) {
      g_cdd_alloc_fail = 3;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_cdd_alloc_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(az_span_create_from_str("typeof(int[5) x;\n"),
                      &tr_valid) == 0 &&
        tr_valid) {
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_cdd_alloc_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(az_span_create_from_str("typeof(int[5) x;\n"),
                      &tr_valid) == 0 &&
        tr_valid) {
      g_cdd_cst_alloc_token_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tr_valid, &config));
      g_cdd_cst_alloc_token_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(az_span_create_from_str("__auto_type x = int;\n"),
                      &tr_valid) == 0 &&
        tr_valid) {
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tr_valid, &config));
      g_cdd_last_alloc_err = cdd_cst_tree_free(tr_valid);
    }
    if (cdd_cst_parse(
            az_span_create_from_str("int f(int n) { int a[n]; return 1; }\n"),
            &tr_valid) == 0 &&
        tr_valid) {
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tr_valid, NULL));
      g_cdd_last_alloc_err = cdd_cst_tree_free(tr_valid);
    }
  }

  {
    static const char *const test_codes[] = {
        "void f(void) __attribute__((pure));\n",
        "struct __attribute__((packed)) X { int a; };\n",
        "void f(void) __attribute__((noreturn));\n",
        "void f(void) __attribute__((unused));\n",
        "union __attribute__((transparent_union)) TU { int a; };\n",
        "typedef int v4si __attribute__((vector_size(16)));\n",
        "int x __attribute__((aligned(8)));\n",
        "typedef int SItype __attribute__((mode(SI)));\n",
        "void f(void) __attribute__((visibility(\"default\"), used));\n",
        "unsigned __int128 u128; __int128 s128;\n",
        "_Decimal32 d32; _Fract fr; _Decimal64 d64; _Float16 f16;\n",
        "void f(void) { return (void)0; }\n",
        "void cl(int *p) {}\n"
        "void f(void) { int __attribute__((cleanup(cl))) x = 1; return; }\n",
        "void cl(int *p) {}\n"
        "void f(void) { int __attribute__((cleanup(cl))) x = 1; return 1; }\n",
        "void cl(int *p) {}\n"
        "void f(void) {\n"
        "  int __attribute__((cleanup(cl))) x = 1;\n"
        "  goto l1;\n"
        "l1:;\n"
        "  longjmp(0, 1);\n"
        "}\n",
        "void f(void) { __label__ l1; goto l1; l1:; }\n",
        "int x = ({ int a = 1; a; });\n",
        "void f(int n) { int a[n]; }\n",
        "int f(int n) { int a[n]; return 1; }\n",
        "struct S { int a[0]; }; struct M { int a[0]; int b; };\n",
        "enum E { E1, }; struct S { int a, };\n",
        "int f(int x) { switch(x) { case 1 ... 5: break; } return 0; }\n",
        "int arr[10] = { [0 ... 4] = 1 };\n",
        "int f(int x) { return x ?: 0; }\n",
        "void f(void) { int v = 0; (char)v = 1; (int){ 2 }; }\n",
        "int f(int x) { return __builtin_expect(x, 1); }\n",
        "int f(void) { return __extension__ 1; }\n",
        "int f(void) { return __builtin_shuffle(1, 2); }\n",
        "int f(void) { return __builtin_shufflevector(1, 2); }\n",
        "int f(void) { return __alignof__(int); }\n",
        "__complex__ float cf;\n",
        "float rf = __real__ cf;\n",
        "float imf = __imag__ cf;\n",
        "int ty_v = 1; typeof(ty_v) ty;\n",
        "typeof(int[5]) tarr;\n",
        "typeof(int[2][3]) tarr2d;\n",
        "typeof(int[]) tarr2;\n",
        "__auto_type ax = 1;\n",
        "union { int a; };\n",
        "struct { int a; };\n",
        "unsigned __int128 n128 = 0x10000000000000000;\n",
        "void f(void) { (int){ 2 }; }\n",
        "void f(void) { int v = 0; (union U)v = 1; }\n",
        "void f(int n) { int a[n]; goto l1; l1:; }\n",
        "void f(int *ptr) { goto *ptr; }\n",
        "void f(void) { __label__ l1, l2, l3; goto l1; l1:; }\n",
        "int top_zero[0];"};
    size_t j;
    int vla_mode;
    for (vla_mode = 0; vla_mode <= 1; vla_mode++) {
      config.fallback_vla_to_malloc = vla_mode;
      for (j = 0; j < sizeof(test_codes) / sizeof(test_codes[0]); j++) {
        int k;
        for (k = 1; k <= 12; k++) {
          g_gnu_replace_fail = k;
          if (cdd_cst_parse(az_span_create_from_str(test_codes[j]), &tree) ==
                  0 &&
              tree) {
            g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
            g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_gnu_replace_fail = 0;

        for (k = 1; k <= 40; k++) {
          if (cdd_cst_parse(az_span_create_from_str(test_codes[j]), &tree) ==
                  0 &&
              tree) {
            g_cdd_alloc_fail = k;
            g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
            g_cdd_alloc_fail = 0;
            g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_cdd_alloc_fail = 0;

        for (k = 1; k <= 12; k++) {
          g_gnu_alloc_fail = k;
          if (cdd_cst_parse(az_span_create_from_str(test_codes[j]), &tree) ==
                  0 &&
              tree) {
            g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
            g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_gnu_alloc_fail = 0;

        for (k = 1; k <= 12; k++) {
          g_gnu_malloc_fail = k;
          if (cdd_cst_parse(az_span_create_from_str(test_codes[j]), &tree) ==
                  0 &&
              tree) {
            g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
            g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_gnu_malloc_fail = 0;

        for (k = 1; k <= 12; k++) {
          g_gnu_find_fail = k;
          if (cdd_cst_parse(az_span_create_from_str(test_codes[j]), &tree) ==
                  0 &&
              tree) {
            g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
            g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_gnu_find_fail = 0;

        for (k = 1; k <= 35; k++) {
          if (cdd_cst_parse(az_span_create_from_str(test_codes[j]), &tree) ==
                  0 &&
              tree) {
            g_cdd_cst_alloc_token_fail = k;
            g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
            g_cdd_cst_alloc_token_fail = 0;
            g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_cdd_cst_alloc_token_fail = 0;

        for (k = 1; k <= 10; k++) {
          if (cdd_cst_parse(az_span_create_from_str(test_codes[j]), &tree) ==
                  0 &&
              tree) {
            g_cdd_strdup_fail = k;
            g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
            g_cdd_strdup_fail = 0;
            g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
            tree = NULL;
          }
        }
        g_cdd_strdup_fail = 0;
      }
    }
  }

  for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); i++) {
    int k;
    for (k = 1; k <= 15; k++) {
      if (cdd_cst_parse(az_span_create_from_str(snippets[i]), &tree) == 0 &&
          tree) {
        g_cdd_alloc_fail = k;
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
        g_cdd_alloc_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    g_cdd_alloc_fail = 0;

    for (k = 1; k <= 15; k++) {
      g_gnu_malloc_fail = k;
      if (cdd_cst_parse(az_span_create_from_str(snippets[i]), &tree) == 0 &&
          tree) {
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    g_gnu_malloc_fail = 0;

    for (k = 1; k <= 15; k++) {
      g_gnu_alloc_fail = k;
      if (cdd_cst_parse(az_span_create_from_str(snippets[i]), &tree) == 0 &&
          tree) {
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    g_gnu_alloc_fail = 0;

    for (k = 1; k <= 15; k++) {
      g_gnu_replace_fail = k;
      if (cdd_cst_parse(az_span_create_from_str(snippets[i]), &tree) == 0 &&
          tree) {
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    g_gnu_replace_fail = 0;

    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(snippets[i]), &tree));
    ASSERT(tree != NULL);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &config));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_all_grammar_branches.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_all_grammar_branches(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config;
  size_t i;
  static const char *const codes[] = {
      "__complex__",
      "__real__",
      "__real__ 123",
      "__imag__",
      "__imag__ 123",
      "typeof",
      "typeof x;",
      "typeof(int * const) x;",
      "typeof(",
      "typeof((int))",
      "typeof(int",
      "typeof(int[5) x;",
      "__auto_type",
      "__auto_type x;",
      "__auto_type x = \"str\";",
      "__auto_type x = y;",
      "__auto_type s = struct;",
      "__auto_type x =",
      "017777777777777777777777777777777777777777ULL",
      "int __attribute__;",
      "int __attribute__((packed, x));",
      "int __attribute__ (noreturn) x;",
      "struct __attribute__((packed)) X { int a; }",
      "int __attribute__(((always))) x;",
      "int __attribute__((a, b)) x;",
      "int __attribute__((vector_wide(16))) x;",
      "int __attribute__((pure(16))) x;",
      "int __attribute__((cleanup(8))) x;",
      "__attribute__((",
      "__attribute__ x;",
      "union",
      "struct",
      "union U { int a; };",
      "struct S { int a; };",
      "void f_casts(void) { int v; (struct S)v = 1; (short)v = 1; (char)v = 1; "
      "}",
      "void f_c_eof(void) { (int)x = 1",
      "void f_c_blk(void) { { (int)x = 1; } }",
      "int f_tern(int x) { return x ? 1 : 2; }",
      "int f_ret_c(char c) { return (char)c; }",
      "#define \n",
      "#define\tFOO\t1\n",
      "#define FOO\n",
      "#define FOO(a\n",
      "#define FOO(a, b)\n",
      "#define FOO(x) ...\n",
      "#define FOO(...)\n",
      "#define FOO(args...) foo(##a, ##args, ##args1, ##args_extra, , "
      "\t##args, a, args, args1, args_extra, myargs, _args)\n",
      "#define FOO(a, args...) foo(a, #args)\n",
      "#define FOO(args...) foo(args_more)\n",
      "volatile __int128 v128;\n",
      "int __int128 x;\n",
      "; __int128 x_semi;\n",
      "signed __int128 x_sgn;\n",
      "int f_exp1(void) { return __builtin_expect; }\n",
      "int f_exp2(void) { return __builtin_expect",
      "int f_exp3(int a, int b) { return __builtin_expect(fn(a, b), 1); }\n",
      "int f_cases(int x) { switch(x) { case 1 ... 5: break; case -5 ... -1: "
      "break; case 5 ... 1: break; default: break; } return 0; }\n",
      "int f_cases2(int x) { switch (x) { case 1: ; case 2 ... 5: break; } "
      "return 0; }\n",
      "int f_cases3(int x) { switch (x) { { case 1 ... 5: break; } } return 0; "
      "}\n",
      "int f_c_dot(int x) { return x.y; }\n",
      "int f_c_dot2(int x) { return x..",
      "int a1[10] = { [0 ... 4] = 1 };\n",
      "int a2[10] = { [-5 ... -1] = 1 };\n",
      "int a3[10] = { [5 ... 1] = 1 };\n",
      "int a4[10] = { [0 ... 4] };\n",
      "int a5[10] = { [0 ... 1, 2] = 0 };\n",
      "void f_lbls(void) { __label__ l1, l2; void *p = &&l1; goto l1; l1:; }\n",
      "void f_goto_ptr(int *ptr) { goto *ptr; }\n",
      "void f_lj(void) { longjmp(0, 1); }\n",
      "void f_vla_alloca(int n) { int a[n]; }\n",
      "void f_vla_p(int n, int a[n], int b[n]) {}\n",
      "int f_vla_r(int n) { int a[n]; return 1; }\n",
      "int a_multi[n][m];\n",
      "int a_multi3[n][m][p];\n",
      "int a_multi_num[n][5];\n",
      "int a_empty[];\n",
      "int a_eof[n]",
      "struct S1 { int a[0]; };\n",
      "struct S2 { int a[0]; int b; };\n",
      "int arr_std[1];\n",
      "int arr_add[0 + 1];\n",
      "int arr_init[0] = {0};\n",
      "int arr_eof[0]",
      "enum E { E1, };\n",
      "enum E_eof { E1, ",
      "struct S3 { int a, };\n",
      "struct S1 s1 = {};\n",
      "union U u1 = {};\n",
      "struct dummy {} s_dummy;\n",
      "struct item {} s_item;\n",
      "int value_empty = {};\n",
      "int item_empty = {};\n",
      "struct S s_desig = { .a : {} };\n",
      "int x = __alignof(int);\n",
      "void f_lbl_ref(void) { int *p = &l1; l1 + 1; }\n",
      "void f_lbl_top(void) { l1:; &l1; }\n",
      "int f_par_one(void) { return (1); }\n",
      "void f_ret_par(void) { return (1); }\n",
      "int f_ret_char(void) { return (char)1; }\n",
      "void f_ret_voidptr(void) { return (void * )0; }\n",
      "void f_ret_void_eof(void) { return (void)0",
      "int __attribute__((cleanup(cl))) cl_a, cl_b;\n",
      "int __attribute__((cleanup(cl)));\n",
      "{ void file_level_nested() {} }\n",
      "int (*fp)(void) { return 0; }\n",
      "void f_c_lval_eof(void) { (int }",
      "(int)x = 1;\n",
      "my_label:;\n",
      "{}\n",
      "#define",
      "int __attribute__((mode(SI)) x_m2;\n",
      "int __attribute__((noreturn) x_n1;\n",
      "void f_elvis_kw(int b) {\n"
      "  int item = 1, value = 2, num = 3, id = 4, result = 5;\n"
      "  switch (result ?: b) { case item ?: b: break; }\n"
      "  while (value ?: b) {}\n"
      "  for (num ?: b; ; ) break;\n"
      "  while (id ?: b) {}\n"
      "}\n",
      "void f_lbl_norm(void) { goto norm_lbl; norm_lbl:; }\n",
      "[1];\n",
      "[0 ... 4] = 1;\n",
      "void cl(int *p) {}\n"
      "void f_5_c(void) {\n"
      "  int __attribute__((cleanup(cl))) c1;\n"
      "  int __attribute__((cleanup(cl))) c2;\n"
      "  int __attribute__((cleanup(cl))) c3;\n"
      "  int __attribute__((cleanup(cl))) c4;\n"
      "  int __attribute__((cleanup(cl))) c5;\n"
      "}\n",
      "void f_5_l(void) {\n"
      "  __label__ l1;\n"
      "  __label__ l2;\n"
      "  __label__ l3;\n"
      "  __label__ l4;\n"
      "  __label__ l5;\n"
      "  goto l1; l1:;\n"
      "}\n",
      "void f_5_v(int n) {\n"
      "  int v1[n]; int v2[n]; int v3[n]; int v4[n]; int v5[n];\n"
      "}\n",
      "void f_elvis_cases(int a, int b) {\n"
      "  int x = (a ?: b);\n"
      "  int arr[10];\n"
      "  arr[a ?: b] = 1;\n"
      "  int z = a ?: b;\n"
      "  ; a ?: b;\n"
      "  { a ?: b; }\n"
      "  if (a ?: b) {}\n"
      "  while (a ?: b) {}\n"
      "  for (a ?: b; ; ) break;\n"
      "  do {} while (a ?: b);\n"
      "  switch (a ?: b) { case 1 ?: 0: break; }\n"
      "  int item = a ?: b;\n"
      "  int value = a ?: b;\n"
      "  int num = a ?: b;\n"
      "  int id = a ?: b;\n"
      "  int switch_var = a ?: b;\n"
      "  a ?: b;\n"
      "}\n",
      "int f_elvis_tern(int cond, int a, int b, int c) {\n"
      "  return cond ? a : b ?: c;\n"
      "}\n",
      "void f_33_cleanups(void) {\n"
      "  int __attribute__((cleanup(cl))) c01, c02, c03, c04, c05, c06, c07, "
      "c08, c09, c10,\n"
      "                                  c11, c12, c13, c14, c15, c16, c17, "
      "c18, c19, c20,\n"
      "                                  c21, c22, c23, c24, c25, c26, c27, "
      "c28, c29, c30,\n"
      "                                  c31, c32, c33;\n"
      "}\n",
      "void f_33_labels(void) {\n"
      "  __label__ l01, l02, l03, l04, l05, l06, l07, l08, l09, l10,\n"
      "            l11, l12, l13, l14, l15, l16, l17, l18, l19, l20,\n"
      "            l21, l22, l23, l24, l25, l26, l27, l28, l29, l30,\n"
      "            l31, l32, l33;\n"
      "}\n",
      "void f_5_cleanups(void) {\n"
      "  int __attribute__((cleanup(cl))) c1;\n"
      "  int __attribute__((cleanup(cl))) c2;\n"
      "  int __attribute__((cleanup(cl))) c3;\n"
      "  int __attribute__((cleanup(cl))) c4;\n"
      "  int __attribute__((cleanup(cl))) c5;\n"
      "}\n",
      "void f_5_labels(void) {\n"
      "  __label__ l1;\n"
      "  __label__ l2;\n"
      "  __label__ l3;\n"
      "  __label__ l4;\n"
      "  __label__ l5;\n"
      "  goto l1; l1:;\n"
      "}\n",
      "void f_5_vlas(int n) {\n"
      "  int v1[n]; int v2[n]; int v3[n]; int v4[n]; int v5[n];\n"
      "}\n",
      "a ?: b;\n",
      "? : 0;\n",
      "#define FOO",
      "#define M1(a, args...) fn(a, ##\targs, ##other)\n",
      "__int128 x_at_zero;\n",
      "int __attribute__((mode(SI) x;\n",
      "int __attribute__((noreturn x;\n",
      "void f_c_u1(void) { (union U)v\n",
      "void f_c_u2(void) { (union U)v + 1; }\n",
      "x ?",
      "void f_outer(void) { int __attribute__((cleanup(cl))) x; { int dummy; } "
      "}\n",
      "void f_outer_lbl(void) { __label__ l1; { int dummy; } l1:; }\n",
      "void f_outer_vla(int n) { int a[n]; { int dummy; } }\n",
      "void f_ret_cl_eof(void) { int __attribute__((cleanup(cl))) x; return",
      "void f_goto_eof(void) { goto",
      "void f_lbl_eof(void) { __label__ l1",
      "void f_lbl_full(void) { __label__ l1; l1:; &l1; + &l1; l1",
      "[0]",
      "struct S_zero_eof { int a[0];",
      "{ 1 }",
      "int f_case_def(int x) { switch (x) { case 1: ; case 2 ... 5: break; "
      "default: break; } return 0; }\n",
      "1 ... 5:",
      "case 1 ... -",
      "case 1 ... -x:",
      "case 1 ... x:",
      "[0 ... 1] =",
      "int f_exp_eof(void) { return __builtin_expect(x, 1",
      "void f_exp_range(void) { int item = 1 ... 5; }\n",
      "void f_semi_range(void) { int x = 0; 1 ... 5; }\n",
      "void f_lbl_unmatched(void) { __label__ l1; goto norm_lbl; norm_lbl:; "
      "goto l1; l1:; }\n",
      "int __attribute__((cleanup(cl)));\n"};

  int vla_mode;
  memset(&config, 0, sizeof(config));

  for (vla_mode = 0; vla_mode <= 1; vla_mode++) {
    config.fallback_vla_to_malloc = vla_mode;
    for (i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
      if (cdd_cst_parse(az_span_create_from_str(codes[i]), &tree) == 0 &&
          tree) {
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
  }

  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_final_branches.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_final_branches(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config;
  void *m_ptr = NULL;
  memset(&config, 0, sizeof(config));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, gnu_malloc(10, NULL));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, gnu_malloc(10, &m_ptr));
  g_cdd_alloc_fail = 0;

  /* 1. L1710 & L1712: is_cast_lvalue == 2 loop without closing ; or = */
  if (cdd_cst_parse(
          az_span_create_from_str("void f(void) { (union U)v = 1; }\n"),
          &tree) == 0 &&
      tree) {
    tree->base_tokens->tokens[11].kind = CDD_TOKEN_PLUS;
    tree->base_tokens->size = 12;
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    tree->base_tokens->tokens[11].kind = CDD_TOKEN_ASSIGN;
    tree->base_tokens->size = 16;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 2. L1855: return (void)expr loop without ; */
  if (cdd_cst_parse(
          az_span_create_from_str("void f(void) { return (void)0; }\n"),
          &tree) == 0 &&
      tree) {
    tree->base_tokens->size -= 2;
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    tree->base_tokens->size += 2;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 3. L1936: cleanup loop without ; */
  if (cdd_cst_parse(
          az_span_create_from_str("void cl(int *p) {} void f(void) { int "
                                  "__attribute__((cleanup(cl))) x; }\n"),
          &tree) == 0 &&
      tree) {
    tree->base_tokens->size -= 2;
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    tree->base_tokens->size += 2;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 4. L2108: return with cleanups loop without ; */
  if (cdd_cst_parse(
          az_span_create_from_str("void cl(int *p) {} void f(void) { int "
                                  "__attribute__((cleanup(cl))) x = 1; "
                                  "return 1; }\n"),
          &tree) == 0 &&
      tree) {
    tree->base_tokens->size -= 3;
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    tree->base_tokens->size += 3;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 5. L2140: return with cleanups and NULL config */
  if (cdd_cst_parse(
          az_span_create_from_str("void cl(int *p) {} int f(void) { int "
                                  "__attribute__((cleanup(cl))) x = 1; "
                                  "return 1; }\n"),
          &tree) == 0 &&
      tree) {
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, NULL));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 6. L2229: while (i < size) in __label__ reaches end */
  if (cdd_cst_parse(
          az_span_create_from_str("void f(void) { __label__ l1, l2; }\n"),
          &tree) == 0 &&
      tree) {
    size_t ti;
    for (ti = 0; ti < tree->base_tokens->size; ti++) {
      if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_SEMICOLON ||
          tree->base_tokens->tokens[ti].kind == CDD_TOKEN_RBRACE)
        tree->base_tokens->tokens[ti].kind = CDD_TOKEN_IDENTIFIER;
    }
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 7. L2541: zero-length array loop reaches end */
  if (cdd_cst_parse(az_span_create_from_str("struct S { int a[0]; }\n"),
                    &tree) == 0 &&
      tree) {
    tree->base_tokens->size = 10;
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    tree->base_tokens->size = 11;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 8. L2884: builtin expect loop reaches end without seeing ')' */
  if (cdd_cst_parse(az_span_create_from_str("int f(int x) { return "
                                            "__builtin_expect(x, 1); }\n"),
                    &tree) == 0 &&
      tree) {
    tree->base_tokens->tokens[13].kind = CDD_TOKEN_PLUS;
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    tree->base_tokens->tokens[13].kind = CDD_TOKEN_RPAREN;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(az_span_create_from_str("int f(int x) { return "
                                            "__builtin_expect(x, 1); }\n"),
                    &tree) == 0 &&
      tree) {
    tree->base_tokens->size = 9;
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    tree->base_tokens->size = 17;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Empty enum and union defs */
  if (cdd_cst_parse(az_span_create_from_str("void f_ed(void) { union dummy {}; "
                                            "enum item {}; }\n"),
                    &tree) == 0 &&
      tree) {
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Trailing comma loop reaches end */
  if (cdd_cst_parse(az_span_create_from_str("enum E { E1, };\n"), &tree) == 0 &&
      tree) {
    tree->base_tokens->size = 5;
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    tree->base_tokens->size = 7;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Empty struct at EOF */
  if (cdd_cst_parse(az_span_create_from_str("struct S {};\n"), &tree) == 0 &&
      tree) {
    tree->base_tokens->size = 3;
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    tree->base_tokens->size = 6;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Elvis with comma operator */
  if (cdd_cst_parse(az_span_create_from_str("int f_c_elv(int a, int b) { "
                                            "return a, a ?: b; }\n"),
                    &tree) == 0 &&
      tree) {
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Range without case or delimiter */
  if (cdd_cst_parse(az_span_create_from_str("int val = 1 ... 5;\n"), &tree) ==
          0 &&
      tree) {
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(az_span_create_from_str(
                        "int f_br(void) { { 1 ... 5; } return 0; }\n"),
                    &tree) == 0 &&
      tree) {
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Single & before local label */
  if (cdd_cst_parse(az_span_create_from_str(
                        "void f_lbl_amp(void) { __label__ l1; void *p = "
                        "&l1; (void)p; l1:; }\n"),
                    &tree) == 0 &&
      tree) {
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 9. L1676: cast closed by RBRACE instead of RPAREN */
  if (cdd_cst_parse(
          az_span_create_from_str("void f(void) { int x; (int)x = 1; }\n"),
          &tree) == 0 &&
      tree) {
    size_t ti;
    for (ti = 0; ti < tree->base_tokens->size; ti++) {
      if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_RPAREN) {
        tree->base_tokens->tokens[ti].kind = CDD_TOKEN_RBRACE;
        break;
      }
    }
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 10. Elvis RBRACKET and RBRACE before ?: */
  if (cdd_cst_parse(
          az_span_create_from_str("int f(int a, int b) { int arr[10]; int z = "
                                  "arr[a] ?: b; return z; }\n"),
          &tree) == 0 &&
      tree) {
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(
          az_span_create_from_str("int f(int a, int b) { int z = ({ int v = a; "
                                  "v; }) ?: b; return z; }\n"),
          &tree) == 0 &&
      tree) {
    g_cdd_last_alloc_err = cdd_transform_gnu(tree, &config);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 11. Cover remaining branches in gnu_standardizer_helpers.c */
  {
    struct gnu_standardizer_ctx_t gctx_empty;
    const char *inf_ty = NULL;
    char eval_b[64];
    int is_tn = 0;
    int flag_out = 0;
    cdd_transform_config_t cfg_tmp;

    memset(&gctx_empty, 0, sizeof(gctx_empty));
    memset(&cfg_tmp, 0, sizeof(cfg_tmp));

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("&x"), &tree));
    /* cdd_asm_visitor with gctx->tree == NULL */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_asm_visitor(tree->root, &gctx_empty));
    /* cdd_infer_type returning non-canonical "int *" -> fallback to "int" */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_infer_type(tree->base_tokens->tokens, 2, &inf_ty));
    ASSERT_STR_EQ("int", inf_ty);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;

    /* cdd_infer_type where cdd_cst_eval_tokens_type sets is_tn = 1 */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("(_Bool)"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_infer_type(tree->base_tokens->tokens, 3, &inf_ty));
    ASSERT_STR_EQ("int", inf_ty);

    /* cdd_eval_expr_type_scoped branches */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_eval_expr_type_scoped(NULL, NULL, tree->base_tokens->tokens,
                                        1, 0, NULL, 16, &is_tn));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_eval_expr_type_scoped(NULL, NULL, tree->base_tokens->tokens,
                                        1, 0, eval_b, 0, &is_tn));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_eval_expr_type_scoped(NULL, NULL, NULL, 1, 0, eval_b,
                                        sizeof(eval_b), NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_eval_expr_type_scoped(NULL, NULL, tree->base_tokens->tokens,
                                        0, 0, eval_b, sizeof(eval_b), &is_tn));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_eval_expr_type_scoped(NULL, NULL, tree->base_tokens->tokens,
                                        1, 0, eval_b, sizeof(eval_b), &is_tn));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_eval_expr_type_scoped(
                                 &gctx_empty, NULL, tree->base_tokens->tokens,
                                 1, 0, eval_b, sizeof(eval_b), &is_tn));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;

    /* gnu_is_msvc_target and gnu_is_strict_iso_c89 branches */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, gnu_is_msvc_target(&cfg_tmp, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              gnu_is_strict_iso_c89(&cfg_tmp, NULL));
    cfg_tmp.msvc_ver = 1900;
    ASSERT_EQ(CDD_C_SUCCESS, gnu_is_msvc_target(&cfg_tmp, &flag_out));
    ASSERT_EQ(1, flag_out);
  }

  /* 12. Cover remaining lines and OOM paths in gnu_standardizer.c */
  {
    struct gnu_standardizer_ctx_t gctx_tmp;
    cdd_transform_config_t cfg_tmp;
    int fi;
    const char *nest_src =
        "void outer1(void) {\n"
        "  struct P { int m; } s;\n"
        "  struct P *sp = &s;\n"
        "  int cap = 1;\n"
        "  static int nest_void(void) { int loc = 2; return cap + cap + loc + "
        "s.m + sp->m; }\n"
        "  int nest_empty() { return cap; }\n"
        "  int nest_fp(int (*cb)(int)) { return cb ? cb(cap) : cap; }\n"
        "  int r = nest_void() + nest_empty() + nest_fp((0));\n"
        "}\n";
    const char *filter_src =
        "void outer_filters(void) {\n"
        "  int sizeof(int x) { return x; }\n"
        "  typedef int f_td(void) { return 0; }\n"
        "  int v_eq = f_eq(void) { return 0; }\n"
        "  int fn_long_01234567890123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789012345678901234567890123456789"
        "(void) { return 0; }\n"
        "}\n";
    const char *oom_gnu_cases[] = {
        "void out(void) { int c = 1; int nf(void) { return c; } "
        "int nf2(int a) { return c + a; } nf(); nf2((2)); }\n",
        "void f_magic(void) { const char *p = __FUNCTION__; "
        "__asm__(\"\" ::: \"memory\"); }\n",
        "__int128 g_i128 = 1; int f_bi(unsigned int x) { return "
        "__builtin_clz(x); }\n"};
    size_t ci;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, gnu_standardize_fn(NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(nest_src), &tree));
    memset(&gctx_tmp, 0, sizeof(gctx_tmp));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_fn_ctx(tree, &gctx_tmp));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;

    memset(&cfg_tmp, 0, sizeof(cfg_tmp));
    cfg_tmp.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(nest_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &cfg_tmp));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(filter_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &cfg_tmp));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;

    for (ci = 0; ci < sizeof(oom_gnu_cases) / sizeof(oom_gnu_cases[0]); ci++) {
      for (fi = 1; fi <= 35; fi++) {
        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(oom_gnu_cases[ci]), &tree));
        g_gnu_alloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &cfg_tmp);
        g_gnu_alloc_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(oom_gnu_cases[ci]), &tree));
        g_gnu_replace_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &cfg_tmp);
        g_gnu_replace_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(oom_gnu_cases[ci]), &tree));
        g_gnu_bld_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &cfg_tmp);
        g_gnu_bld_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(oom_gnu_cases[ci]), &tree));
        g_cdd_strdup_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &cfg_tmp);
        g_cdd_strdup_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(oom_gnu_cases[ci]), &tree));
        g_cdd_alloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &cfg_tmp);
        g_cdd_alloc_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(oom_gnu_cases[ci]), &tree));
        g_cdd_cst_alloc_token_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &cfg_tmp);
        g_cdd_cst_alloc_token_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(oom_gnu_cases[ci]), &tree));
        g_cdd_cst_realloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &cfg_tmp);
        g_cdd_cst_realloc_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;

        ASSERT_EQ(
            CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str(oom_gnu_cases[ci]), &tree));
        g_cdd_cst_alloc_node_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(tree, &cfg_tmp);
        g_cdd_cst_alloc_node_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
      }
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_parse(az_span_create_from_str(oom_gnu_cases[ci]), &tree));
      g_gnu_find_fail = 1;
      g_cdd_last_alloc_err = cdd_transform_gnu(tree, &cfg_tmp);
      g_gnu_find_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }

  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_ranges_and_fn_edges.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_ranges_and_fn_edges(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;

  /* 1. Range decoder & evaluator edges in gnu_standardizer_ranges.c */
  {
    const char *re1 = "void f_rng(int x) {\n"
                      "  int non_enum = 1;\n"
                      "  int a1[] = { [0 ... 1] = {1, 2} };\n"
                      "  switch (x) {\n"
                      "    case L'a' ... L'c': break;\n"
                      "    case '\\x00410' ... '\\x00412': break;\n"
                      "    case '\\x/' ... 'z': break;\n"
                      "    case '\\x@' ... 'z': break;\n"
                      "    case '\\!' ... '~': break;\n"
                      "    case '\\0123' ... '\\0125': break;\n"
                      "    case '\\0!' ... 'z': break;\n"
                      "    case '\\08' ... 'z': break;\n";
    const char *re2 =
        "    case -0b10 ... 0b11: break;\n"
        "    case 0b2 ... 1: break;\n"
        "    case 0b10000000000000000000000000000000000000000000000000000000000"
        "000000 ... 1: break;\n"
        "    case 0.0 ... 1: break;\n"
        "    case 0u ... 10: break;\n"
        "    case non_enum ... 5: break;\n"
        "    case '\\x7e' ... '\\x7f': break;\n"
        "    case 'a' ... 100: break;\n"
        "  }\n"
        "  int a2[] = { [0 ... 1] = a1[0], 9 };\n"
        "  { } 1 ... 2;\n"
        "}\n"
        "int g_top = (1 ... 2);\n";
    char range_edges[1024];
    CDD_SNPRINTF(range_edges, sizeof(range_edges), "%s%s", re1, re2);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(range_edges), &tree));
    /* Run gnu_standardize_ranges directly first so 0b... literals reach it */
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_ranges(tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* Range with >128-char identifier, empty gctx->sem_env, unclosed char,
     enum with has_enum_value == 0, gnu_find_node fail, and EOF range init */
  {
    struct gnu_standardizer_ctx_t empty_gctx;
    char long_case[320];
    memset(&empty_gctx, 0, sizeof(empty_gctx));
    memset(long_case, 'A', 175);
    memcpy(long_case, "void f(int x){switch(x){case ", 29);
    memcpy(long_case + 169, " ... 2: break;}}", 17);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(long_case), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_ranges_ctx(tree, &empty_gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Short identifier with gctx != NULL && gctx->sem_env == NULL */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str(
                      "void f(int x){switch(x){case E_SHORT ... 2: break;}}"),
                  &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_ranges_ctx(tree, &empty_gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "void f(int x){switch(x){case 'a' ... "
                                "'b': break;}}"),
                            &tree));
    /* Mutate first CHAR token to "L'a'" (raw[0] != '\''), second to "'b" */
    {
      size_t ti;
      int seen_ch = 0;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_CHAR) {
          if (!seen_ch) {
            tree->base_tokens->tokens[ti].start = (const uint8_t *)"L'a'";
            tree->base_tokens->tokens[ti].length = 4;
            seen_ch = 1;
          } else {
            tree->base_tokens->tokens[ti].start = (const uint8_t *)"'ba";
            tree->base_tokens->tokens[ti].length = 3;
            break;
          }
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_ranges(tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Mutate NUMBER token to invalid 3-byte binary "0b2" */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str(
                          "void f(int x){switch(x){case 1 ... 2: break;}}"),
                      &tree));
    {
      size_t ti;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_NUMBER) {
          tree->base_tokens->tokens[ti].start = (const uint8_t *)"0b2";
          tree->base_tokens->tokens[ti].length = 3;
          break;
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_ranges(tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Enum constant with has_enum_value = 0 and g_gnu_find_fail = 1 */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str(
                      "enum E { EA = 1 }; void f(int x){switch(x){case EA ... "
                      "3: break;}}"),
                  &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_build_semantic_info_ex(
                                 tree, NULL, 0, &empty_gctx.sem_env));
    {
      cdd_cst_symbol_t *es = NULL;
      if (cdd_cst_scope_lookup_symbol(empty_gctx.sem_env, "EA",
                                      CDD_CST_SYMBOL_ENUM_CONSTANT,
                                      &es) == CDD_C_SUCCESS &&
          es) {
        es->has_enum_value = 0;
      }
    }
    g_gnu_find_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_ranges_ctx(tree, &empty_gctx));
    g_gnu_find_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(empty_gctx.sem_env));
    empty_gctx.sem_env = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Unterminated array range initializer at EOF */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("int a = [0 ... 1] = 5"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_ranges(tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 2. Nested function & gnu_standardize_fn edges in gnu_standardizer.c */
  {
    /* Direct gnu_standardize_fn (gctx == NULL) on nested function */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str(
                      "void outer(void) { static int inner(void) { return 1; } "
                      "inner(); }"),
                  &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_fn(tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Capturing >32 outer vars, >64 outer/inner vars, >=96-char var, static
       nested fn, single non-void param token, and non-call identifier match */
    {
      char big_fn[8192];
      size_t pos = 0;
      int k;
      char long_id[110];
      memset(long_id, 'v', 100);
      long_id[100] = '\0';
      pos += (size_t)CDD_SNPRINTF(big_fn + pos, sizeof(big_fn) - pos,
                                  "typedef int T444; void outer(void) {\n");
      pos += (size_t)CDD_SNPRINTF(big_fn + pos, sizeof(big_fn) - pos,
                                  "  int %s = 0;\n", long_id);
      for (k = 0; k < 66; k++) {
        pos += (size_t)CDD_SNPRINTF(big_fn + pos, sizeof(big_fn) - pos,
                                    "  int o%d = %d;\n", k, k);
      }
      pos += (size_t)CDD_SNPRINTF(big_fn + pos, sizeof(big_fn) - pos,
                                  "  static int inner1(int) {\n");
      for (k = 0; k < 66; k++) {
        pos += (size_t)CDD_SNPRINTF(big_fn + pos, sizeof(big_fn) - pos,
                                    "    int i%d = %d;\n", k, k);
      }
      pos += (size_t)CDD_SNPRINTF(big_fn + pos, sizeof(big_fn) - pos,
                                  "    int s = %s", long_id);
      for (k = 0; k < 40; k++) {
        pos += (size_t)CDD_SNPRINTF(big_fn + pos, sizeof(big_fn) - pos,
                                    " + o%d", k);
      }
      pos += (size_t)CDD_SNPRINTF(
          big_fn + pos, sizeof(big_fn) - pos,
          ";\n    return s;\n  }\n"
          "  inline int inner2(T444) { return o10; }\n"
          "  int proto_only(int a);\n"
          "  struct SLoc { int m; } fn_after(void) { return (struct SLoc){0}; "
          "}\n"
          "  inner1(;\n"
          "}\n"
          "(MACRO_FN)(void) {}\n");
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(big_fn), &tree));
      /* Also inject one outer variable symbol with NULL type_str & semicolon
         before '(' synthetic node inside block */
      {
        struct gnu_standardizer_ctx_t gctx;
        cdd_cst_node_t *syn_fn = NULL;
        cdd_cst_node_t *sub_u = NULL;
        cdd_cst_node_t *sub_u2 = NULL;
        cdd_token_t *st1 = NULL;
        cdd_token_t *st2 = NULL;
        cdd_token_t *st3 = NULL;
        memset(&gctx, 0, sizeof(gctx));
        gctx.config = &cfg;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
        /* Find outer function scope and add a variable with type_str == NULL */
        if (gctx.sem_env->global_scope->num_children > 0) {
          cdd_cst_scope_t *osc = gctx.sem_env->global_scope->children[0];
          cdd_cst_scope_t *saved = gctx.sem_env->current_scope;
          gctx.sem_env->current_scope = osc;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_cst_scope_add_symbol(gctx.sem_env, "null_typed_v",
                                             CDD_CST_SYMBOL_VARIABLE, NULL));
          gctx.sem_env->current_scope = saved;
        }
        /* Synthetic FUNCTION_DEFINITION nodes inside block */
        if (tree->root->num_children > 1) {
          cdd_cst_node_t *fn_outer = tree->root->children[1].val.node;
          size_t bi;
          for (bi = 0; bi < fn_outer->num_children; bi++) {
            if (fn_outer->children[bi].kind == CDD_CST_CHILD_NODE &&
                fn_outer->children[bi].val.node->kind == CDD_CST_BLOCK) {
              cdd_cst_node_t *blk = fn_outer->children[bi].val.node;
              ASSERT_EQ(
                  CDD_C_SUCCESS,
                  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &syn_fn));
              ASSERT_EQ(
                  CDD_C_SUCCESS,
                  cdd_cst_create_token(tree, CDD_TOKEN_SEMICOLON, ";", &st1));
              ASSERT_EQ(
                  CDD_C_SUCCESS,
                  cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "sf", &st2));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_create_token(
                                           tree, CDD_TOKEN_LPAREN, "(", &st3));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(syn_fn, st1));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(syn_fn, st2));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(syn_fn, st3));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(blk, syn_fn));

              /* Second synthetic FUNCTION_DEFINITION: child_node before 'sf2',
                 and non-BLOCK child_node after '(' */
              syn_fn = NULL;
              ASSERT_EQ(
                  CDD_C_SUCCESS,
                  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &syn_fn));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_cst_alloc_node(CDD_CST_UNKNOWN, &sub_u));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_cst_alloc_node(CDD_CST_UNKNOWN, &sub_u2));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "sf2",
                                             &st2));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_create_token(
                                           tree, CDD_TOKEN_LPAREN, "(", &st3));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_cst_append_child_node(syn_fn, sub_u));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(syn_fn, st2));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(syn_fn, st3));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_cst_append_child_node(syn_fn, sub_u2));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(blk, syn_fn));

              /* Third synthetic FUNCTION_DEFINITION: child_node immediately
                 before '(' (j > 0 && children[j - 1].kind != CHILD_TOKEN) */
              syn_fn = NULL;
              sub_u = NULL;
              ASSERT_EQ(
                  CDD_C_SUCCESS,
                  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &syn_fn));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_cst_alloc_node(CDD_CST_UNKNOWN, &sub_u));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_create_token(
                                           tree, CDD_TOKEN_LPAREN, "(", &st3));
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_cst_append_child_node(syn_fn, sub_u));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(syn_fn, st3));
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(blk, syn_fn));
              break;
            }
          }
        }
        ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_fn_ctx(tree, &gctx));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
      }
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }

    /* Outer function captures null_typed_v referenced in inner nested fn */
    {
      struct gnu_standardizer_ctx_t gctx;
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(
                    az_span_create_from_str(
                        "void outer(void) { int inner2(int x) { return ntv + "
                        "x; } inner2(1); }"),
                    &tree));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
      if (gctx.sem_env->global_scope->num_children > 0) {
        cdd_cst_scope_t *osc = gctx.sem_env->global_scope->children[0];
        cdd_cst_scope_t *saved = gctx.sem_env->current_scope;
        gctx.sem_env->current_scope = osc;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_scope_add_symbol(gctx.sem_env, "ntv",
                                           CDD_CST_SYMBOL_VARIABLE, NULL));
        gctx.sem_env->current_scope = saved;
      }
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_fn_ctx(tree, &gctx));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
  }

  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_builtins_and_unroll_edges.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_builtins_and_unroll_edges(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  struct gnu_standardizer_ctx_t gctx;

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;

  /* 1. NULL tree calls */
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(NULL));

  /* 2. Inline asm edge cases on MSVC */
  {
    const char *as1 =
        "void f_asm(unsigned int a, unsigned int b, unsigned int c, "
        "unsigned int d, unsigned int e) {\n"
        "  __asm__ volatile8not(0);\n"
        "  __asm__ __volatile12(0);\n"
        "  asm 123;\n"
        "  asm(123);\n"
        "  asm(\"\" : \"=r\"(a) : \"r\"((b)) : \"nomem8ch\" : : );\n"
        "  asm(\"rdtsc\" : \"=a\"(a), \"=d\"(b), \"=c\"(c), \"()\");\n";
    const char *as2 =
        "  asm(\"cpuid\" : \"=a\"(a), \"=b\"(b), \"=c\"(c), \"=d\"(d), "
        "\"=S\"(e), \"()\" : \"()\");\n"
        "  asm(\"cpuid\" : \"=a\"(a));\n"
        "  asm(\"bswapl %0\");\n"
        "  asm(\"bswapl %0\" : \"()\");\n"
        "}\n";
    char as_buf[1024];
    CDD_SNPRINTF(as_buf, sizeof(as_buf), "%s%s", as1, as2);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(as_buf), &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Trailing `asm`, trailing rdtsc/cpuid/unrecognized asm without `;`,
       unclosed `(` inside asm operand */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x; asm"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "asm(\"rdtsc\" : \"=a\"(a), "
                                               "\"=d\"(b))"),
                                           &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(
                                               "asm(\"cpuid\" : \"=a\"(a))"),
                                           &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("asm(\"cli\")"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 3. Builtin intrinsic edge cases */
  {
    const char *bi1 =
        "typedef unsigned long long u64_t;\n"
        "typedef unsigned long size_t;\n"
        "void f_bi(size_t *szp, u64_t *u64p, long *lp) {\n"
        "  int r = 0;\n"
        "  __builtin_many({1, 2}, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);\n"
        "  __builtin_mul_overflow(1, 2);\n"
        "  __builtin_mul_overflow(1, 2, );\n"
        "  __builtin_add_overflow(1, 2, szp);\n"
        "  __builtin_sub_overflow(1, 2, u64p);\n";
    const char *bi2 =
        "  __builtin_mul_overflow(1, 2, lp);\n"
        "  __builtin_xxxx(1);\n"
        "  __builtin_prefetch();\n"
        "  r = __builtin_constant_p();\n"
        "  r = __builtin_constant_p(0X10 + 0B10 * 2 / 1 + ~0 + 'a' + \"s\" + "
        "sizeof r + sizeof(int));\n"
        "  r = __builtin_choose_expr(1, 2);\n"
        "  r = __builtin_types_compatible_p(int);\n"
        "  r = __builtin_types_compatible_p(, int);\n"
        "  r = __builtin_offsetof(struct S);\n"
        "  __builtin_xxxxxxxx(1, 2);\n"
        "}\n";
    char bi_buf[1536];
    CDD_SNPRINTF(bi_buf, sizeof(bi_buf), "%s%s", bi1, bi2);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(bi_buf), &tree));
    /* Run directly without gctx first for gctx==NULL paths */
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(bi_buf), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Long slice (>256 bytes) in __builtin_choose_expr, >128-char id in
       __builtin_constant_p, enum with has_enum_value == 0, typedef with
       NULL type_str, and g_gnu_find_fail */
    {
      char long_bi[1024];
      char id130[136];
      char expr300[320];
      memset(id130, 'Z', 132);
      id130[132] = '\0';
      memset(expr300, '1', 300);
      {
        int p;
        for (p = 1; p < 299; p += 2)
          expr300[p] = '+';
      }
      expr300[300] = '\0';
      CDD_SNPRINTF(
          long_bi, sizeof(long_bi),
          "enum E { EC = 2 }; typedef int TNull1; typedef int TNull2;\n"
          "void f(int v) {\n"
          "  int a = __builtin_constant_p(%s);\n"
          "  int b = __builtin_constant_p(EC);\n"
          "  int c = __builtin_choose_expr(%s + v, %s, 0);\n"
          "  int d = __builtin_types_compatible_p(TNull1, TNull2);\n"
          "}\n",
          id130, expr300, expr300);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(long_bi), &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
      {
        cdd_cst_symbol_t *s_ec = NULL;
        cdd_cst_symbol_t *s_t1 = NULL;
        cdd_cst_symbol_t *s_t2 = NULL;
        if (cdd_cst_scope_lookup_symbol(gctx.sem_env, "EC",
                                        CDD_CST_SYMBOL_ENUM_CONSTANT,
                                        &s_ec) == CDD_C_SUCCESS)
          s_ec->has_enum_value = 0;
        if (cdd_cst_scope_lookup_symbol(gctx.sem_env, "TNull1",
                                        CDD_CST_SYMBOL_TYPEDEF,
                                        &s_t1) == CDD_C_SUCCESS) {
          C_CDD_FREE(s_t1->type_str);
          s_t1->type_str = NULL;
        }
        if (cdd_cst_scope_lookup_symbol(gctx.sem_env, "TNull2",
                                        CDD_CST_SYMBOL_TYPEDEF,
                                        &s_t2) == CDD_C_SUCCESS) {
          C_CDD_FREE(s_t2->type_str);
          s_t2->type_str = NULL;
        }
      }
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins_ctx(tree, &gctx));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      /* Empty gctx (sem_env == NULL) on __builtin_constant_p(ident) */
      memset(&gctx, 0, sizeof(gctx));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(
                    az_span_create_from_str(
                        "void f(int v){ int x = __builtin_constant_p(v); }"),
                    &tree));
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins_ctx(tree, &gctx));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
  }

  /* 4. Unroll edges: stmt-expr non-expr finals, nested wrapper cur=cur->parent,
        cleanup break/continue in deep switches, local-label single &, cgoto */
  {
    const char *ur1 = "typedef int my_td_t;\n"
                      "struct S0 { int a; int z[0]; int b; };\n"
                      "void f_cln(int *p) { *p = 0; }\n"
                      "void f_ur(int x, int *ptr) {\n"
                      "  int s1 = ({ return; });\n"
                      "  int s2 = ({ goto l1; });\n"
                      "  int s3 = ({ struct S0 sv; });\n"
                      "  int s4 = ({ __int128 iv = 0; });\n"
                      "  int s5 = ({ my_td_t *tp = 0; });\n"
                      "  int s6 = ({ int arr[2] = {0, 1}; arr[0]; });\n";
    const char *ur2 =
        "  l1: ;\n"
        "  switch (x) { default: { int v __attribute((__cleanup__(f_cln))) = "
        "1; continue; } }\n"
        "  for (;;) { break; }\n"
        "  { __label__ L; int y = 0; int *py = ptr + &y; L: goto L; }\n"
        "}\n"
        "void f_cg(int idx) {\n"
        "  void *t1 = &&L_A;\n"
        "  void *arr[] = { &&L_B, [1] = &&L_C };\n"
        "  void *t2 = (idx ? &&L_D : &&L_A);\n"
        "  L_A: L_B: L_C: L_D: goto *t1;\n"
        "}\n"
        "void *f_cg_ret(void) { return &&L_R; L_R: return 0; }\n";
    char ur_buf[1536];
    CDD_SNPRINTF(ur_buf, sizeof(ur_buf), "%s%s", ur1, ur2);
    cfg.fallback_vla_to_malloc = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ur_buf), &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Nested CST node wrapping lp_parent (`cur = cur->parent;` on L231) */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str("void f(void){ int x = ({ 1; }); }"),
                  &tree));
    /* Wrap the statement `int x = ({ 1; });` inside an extra CDD_CST_EXPRESSION
       between CDD_CST_BLOCK and lp_parent */
    {
      cdd_cst_node_t *fn_n = tree->root->children[0].val.node;
      size_t bi;
      for (bi = 0; bi < fn_n->num_children; bi++) {
        if (fn_n->children[bi].kind == CDD_CST_CHILD_NODE &&
            fn_n->children[bi].val.node->kind == CDD_CST_BLOCK) {
          cdd_cst_node_t *blk = fn_n->children[bi].val.node;
          size_t ci;
          for (ci = 0; ci < blk->num_children; ci++) {
            if (blk->children[ci].kind == CDD_CST_CHILD_NODE) {
              cdd_cst_node_t *orig = blk->children[ci].val.node;
              cdd_cst_node_t *wrap = NULL;
              ASSERT_EQ(CDD_C_SUCCESS,
                        cdd_cst_alloc_node(CDD_CST_EXPRESSION, &wrap));
              wrap->parent = blk;
              blk->children[ci].val.node = wrap;
              ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(wrap, orig));
              break;
            }
          }
          break;
        }
      }
    }
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_builtins_unroll_complete.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_builtins_unroll_complete(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  struct gnu_standardizer_ctx_t gctx;

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;

  /* 1. Empty tree (base_tokens == NULL) */
  {
    cdd_cst_tree_t empty_t;
    memset(&empty_t, 0, sizeof(empty_t));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(&empty_t, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins_ctx(&empty_t, NULL));
  }

  /* 2. Complete builtins.c branches */
  {
    const char *bc1 =
        "enum E { EC = 5 };\n"
        "int g_ec = __builtin_constant_p(EC);\n"
        "void f_bc(int r, int *ip) {\n"
        "  int a = __builtin_constant_p(123 + 077 + ((1)) + (1 % 2));\n"
        "  int b = __builtin_constant_p(!0 + !1);\n"
        "  int c = __builtin_constant_p(1 ... 2);\n"
        "  int d = __builtin_constant_p(foobar);\n"
        "  int e = __builtin_constant_p(sizeof r);\n"
        "  int f = __builtin_constant_p(sizeof((int)));\n"
        "  int g = __builtin_constant_p(undecl_id);\n";
    const char *bc2 = "  __builtin_xxx_overflow(1, 2, ip);\n"
                      "  __builtin_0123456789(1);\n"
                      "  __builtin_01234567890(1, 2, 3);\n"
                      "  __builtin_012345678901234567(int, int);\n"
                      "  asm volatilX(\"nop\");\n"
                      "  asm __volatile__(\"nop\");\n"
                      "  asm __volatile12(\"nop\");\n"
                      "  asm(\"nop\" : [ ) );\n"
                      "  asm(\"\" : : : \"123456\");\n";
    const char *bc3 = "  asm(\"rdtsc\" : \"=a\"(), \"=d\"(r) : );\n"
                      "  asm(\"rdtsc\" : \"=a\"(r ] );\n"
                      "  asm(\"cpuid\");\n"
                      "  asm(\"cpuid\" : \"=a\"(r ] );\n"
                      "  asm(\"cpuid\" : \"=a\"(), \"=b\"(r));\n"
                      "  asm(\"cpuid\" : \"=a\"(r) : \"a\"(r ] );\n"
                      "  asm(\"cpuid\" : \"=a\"(r) : \"a\"(), \"c\"(1));\n"
                      "  r = (asm(\"cpuid\"), 0);\n"
                      "  asm(\"bswapl %0\" : \"+r\"(r ] );\n"
                      "  asm(\"bswapl %0\" : \"+r\"(), \"+r\"(r));\n"
                      "}\n";
    char bc_buf[2048];
    CDD_SNPRINTF(bc_buf, sizeof(bc_buf), "%s%s%s", bc1, bc2, bc3);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(bc_buf), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Unclosed asm("nop", string length < 2, bswap with gctx == NULL,
       token line == 0 in gnu_replace_token_range */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("asm(\"nop\""), &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("void f(int x){ asm(\"\"); "
                                                    "asm(\"bswapl %0\" : "
                                                    "\"+r\"(x)); }"),
                            &tree));
    {
      size_t ti;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_STRING) {
          tree->base_tokens->tokens[ti].length = 1;
          break;
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(tree, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str("void f(void){ __builtin_trap(); }"),
                  &tree));
    {
      size_t ti;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].length == 14 &&
            memcmp(tree->base_tokens->tokens[ti].start, "__builtin_trap", 14) ==
                0) {
          tree->base_tokens->tokens[ti + 1].line = 0;
          break;
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins_ctx(tree, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* g_gnu_find_fail, >64-token eval OOM, "uint32_t *", gctx->sem_env == NULL,
       and s1->type_str == "" */
    {
      char expr80[256];
      char oom_src[1024];
      int p;
      memset(expr80, ' ', sizeof(expr80));
      for (p = 0; p < 70; p++) {
        expr80[p * 2] = '1';
        expr80[p * 2 + 1] = '+';
      }
      expr80[139] = '\0';

      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_parse(
              az_span_create_from_str(
                  "enum E { EC = 1 }; void f(int *p){ int a = "
                  "__builtin_constant_p(EC); __builtin_add_overflow(1, 2, p); "
                  "int b = __builtin_types_compatible_p(int, int); }"),
              &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
      /* Add variable uint_p with type "uint32_t *" & typedef TEmpty = "" */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_scope_add_symbol(gctx.sem_env, "uint_p",
                                         CDD_CST_SYMBOL_VARIABLE, NULL));
      {
        cdd_cst_symbol_t *su = NULL;
        if (cdd_cst_scope_lookup_symbol(gctx.sem_env, "uint_p",
                                        CDD_CST_SYMBOL_VARIABLE,
                                        &su) == CDD_C_SUCCESS) {
          C_CDD_FREE(su->type_str);
          su->type_str = (char *)calloc(16, 1);
          memcpy(su->type_str, "uint32_t *", 10);
        }
      }
      g_gnu_find_fail = 1;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins_ctx(tree, &gctx));
      g_gnu_find_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      /* uint_p without "unsigned", empty typedef type_str, sem_env == NULL */
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_parse(az_span_create_from_str(
                            "typedef int TEmpty; void f(void){ "
                            "__builtin_add_overflow(1, 2, uint_p); int x = "
                            "__builtin_types_compatible_p(TEmpty, int); }"),
                        &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins_ctx(tree, &gctx));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_parse(az_span_create_from_str(
                            "typedef int TEmpty; void f(void){ "
                            "__builtin_add_overflow(1, 2, uint_p); int x = "
                            "__builtin_types_compatible_p(TEmpty, int); }"),
                        &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
      {
        cdd_cst_symbol_t *ste = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_scope_add_symbol(gctx.sem_env, "uint_p",
                                           CDD_CST_SYMBOL_VARIABLE, NULL));
        if (cdd_cst_scope_lookup_symbol(gctx.sem_env, "uint_p",
                                        CDD_CST_SYMBOL_VARIABLE,
                                        &ste) == CDD_C_SUCCESS) {
          C_CDD_FREE(ste->type_str);
          ste->type_str = (char *)calloc(16, 1);
          memcpy(ste->type_str, "uint32_t *", 10);
        }
        if (cdd_cst_scope_lookup_symbol(gctx.sem_env, "TEmpty",
                                        CDD_CST_SYMBOL_TYPEDEF,
                                        &ste) == CDD_C_SUCCESS) {
          C_CDD_FREE(ste->type_str);
          ste->type_str = (char *)calloc(4, 1);
        }
      }
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins_ctx(tree, &gctx));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      /* Separate g_gnu_find_fail = 1 for overflow and types_compatible_p */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(
                    az_span_create_from_str(
                        "void f(int *p){ __builtin_add_overflow(1, 2, p); }"),
                    &tree));
      g_gnu_find_fail = 1;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(tree));
      g_gnu_find_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(
                                  "void f(void){ int b = "
                                  "__builtin_types_compatible_p(int, int); }"),
                              &tree));
      g_gnu_find_fail = 1;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(tree));
      g_gnu_find_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      /* Mutate token inside __builtin_constant_p(x) to 2-char CDD_TOKEN_OTHER
       */
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_parse(az_span_create_from_str(
                            "void f(void){ int b = __builtin_constant_p(x); }"),
                        &tree));
      {
        size_t ti;
        for (ti = 0; ti < tree->base_tokens->size; ti++) {
          if (tree->base_tokens->tokens[ti].length == 1 &&
              tree->base_tokens->tokens[ti].start[0] == 'x') {
            tree->base_tokens->tokens[ti].kind = CDD_TOKEN_OTHER;
            tree->base_tokens->tokens[ti].start = (const uint8_t *)"==";
            tree->base_tokens->tokens[ti].length = 2;
            break;
          }
        }
      }
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(tree));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      /* Separate >64-token OOM in overflow arg3, compatible_p arg1, and arg2 */
      CDD_SNPRINTF(oom_src, sizeof(oom_src),
                   "void f(void){ __builtin_add_overflow(1, 2, %s); }", expr80);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(oom_src), &tree));
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(tree));
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      CDD_SNPRINTF(
          oom_src, sizeof(oom_src),
          "void f(void){ int x = __builtin_types_compatible_p(%s, int); }",
          expr80);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(oom_src), &tree));
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(tree));
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      CDD_SNPRINTF(
          oom_src, sizeof(oom_src),
          "void f(void){ int x = __builtin_types_compatible_p(int, %s); }",
          expr80);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(oom_src), &tree));
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(tree));
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
  }

  /* 3. Complete unroll.c branches */
  {
    char big_ur[8192];
    size_t up = 0;
    int k;
    char id130[135];
    memset(id130, 'M', 130);
    id130[130] = '\0';

    /* Global void stmt-expr, >=127-char fn/label, () {} */
    up += (size_t)CDD_SNPRINTF(
        big_ur + up, sizeof(big_ur) - up,
        "typedef int my_td_t;\n"
        "int g_void_se = (({ int a = 1; }), 2);\n"
        "void f_cln(int *p) { *p = 0; }\n"
        "void %s(void) { int v __attribute__((__cleanup__(f_cln))) = 1; return "
        "; }\n"
        "() {}\n",
        id130);
    up += (size_t)CDD_SNPRINTF(big_ur + up, sizeof(big_ur) - up,
                               "void f_u2(int n, int x) {\n"
                               "  int a1 = ({ + 1; });\n"
                               "  int a2 = ({ my_td_t tv; });\n"
                               "  int a3 = ({ %s z; });\n"
                               "  int a4 = ({ non_td z; });\n"
                               "  int a5 = ({ lbl_x + 1; });\n"
                               "  int a6 = ({ ] 1; });\n"
                               "  int a7 = ({ { { int q = 1; } } 2; });\n"
                               "  int a8 = ({ ({ int q = 1; }); 2; });\n",
                               id130);
    up += (size_t)CDD_SNPRINTF(
        big_ur + up, sizeof(big_ur) - up,
        "  for (x = 0; x < 1; x++) ) ;\n"
        "  switch (x) { default: { int __attribute__((__cleanup__(f_cln))) cv "
        "= 1; continue; } }\n"
        "  switch (x) { default: { int __attribute__((__nothrow__(f_cln))) nv "
        "= 1; break } }\n"
        "  for (;;) { int vla_a[n]; break; }\n"
        "  { a1[0]; } a1[0]; (a1[0]);\n"
        "  int z_eq = a1[0];\n");
    /* 34 nested for loops */
    up += (size_t)CDD_SNPRINTF(big_ur + up, sizeof(big_ur) - up, "  ");
    for (k = 0; k < 34; k++) {
      up += (size_t)CDD_SNPRINTF(big_ur + up, sizeof(big_ur) - up, "for(;0;){");
    }
    for (k = 0; k < 34; k++) {
      up += (size_t)CDD_SNPRINTF(big_ur + up, sizeof(big_ur) - up, "}");
    }
    up += (size_t)CDD_SNPRINTF(big_ur + up, sizeof(big_ur) - up, "\n}\n");

    /* Computed goto with >64 labels, duplicate label, >=127-char label,
       `&&123`, `u_arr[&&U_BRK]`, `&&id130` */
    up += (size_t)CDD_SNPRINTF(big_ur + up, sizeof(big_ur) - up,
                               "void f_cg65(int c, void *p) {\n"
                               "  void *u_arr[] = { &&U_A, &&U_B };\n"
                               "  void *u_bk = u_arr[(size_t)0 + 0];\n"
                               "  int u_sub = u_arr[&&U_C];\n"
                               "  void *u_c = c ? &&U_A : &&U_D;\n"
                               "  void *u_long = &&%s;\n"
                               "  int u_num = (1 &&123);\n"
                               "  int non_l = (c && c);\n"
                               "  int sep_amp = (c & &c);\n"
                               "  %s: ;\n"
                               "  L0: L0:\n",
                               id130, id130);
    for (k = 1; k < 66; k++) {
      up += (size_t)CDD_SNPRINTF(big_ur + up, sizeof(big_ur) - up, "  L%d: ;\n",
                                 k);
    }
    up += (size_t)CDD_SNPRINTF(big_ur + up, sizeof(big_ur) - up,
                               "  void *u_over = &&U_OVER;\n"
                               "  goto L0;\n"
                               "  goto *p;\n"
                               "}\n"
                               "void f_cg_trail1(void) { goto }\n"
                               "void f_cg_trail2(void) { ident_end }\n"
                               "void f_cg_trail3(void *p) { goto *p }\n");

    cfg.fallback_vla_to_malloc = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(big_ur), &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Malformed stmt-exprs `({ 1; }`, `({ 1; };`, `break` at EOF, `__label__`
       with `x & y` and trailing ident, `[` near EOF, `arr[0];` at token 1 */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x = ({ 1; }"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("int x = ({ 1; };"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("arr[0]; int b["), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "__label__ L; int z = val & y; tail_id"),
                            &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("break"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* config == NULL on loop break, `() {}` at token 0, `int ) {}` without `(`,
       `int x = ({` without `}`, `g_gnu_find_fail = 3`, and child_node before
       inner_block in lp_parent */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str("void f(void){ for(;;){ break; } }"),
                  &tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll(tree, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("() {}"), &tree));
    memset(&gctx, 0, sizeof(gctx));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int ) {}"), &tree));
    memset(&gctx, 0, sizeof(gctx));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x = ();"), &tree));
    {
      size_t ti;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_RPAREN) {
          tree->base_tokens->tokens[ti].kind = CDD_TOKEN_LBRACE;
          break;
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str("void f(void){ int x = ({ 1; }); }"),
                  &tree));
    g_gnu_find_fail = 3;
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, gnu_standardize_unroll(tree, &cfg));
    g_gnu_find_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Prepend a CDD_CST_UNKNOWN child node before inner_block on lp_parent */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str("void f(void){ int x = ({ 1; }); }"),
                  &tree));
    {
      size_t ti;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_LPAREN &&
            ti + 1 < tree->base_tokens->size &&
            tree->base_tokens->tokens[ti + 1].kind == CDD_TOKEN_LBRACE) {
          size_t cidx = 0;
          cdd_cst_node_t *lpp = NULL;
          cdd_cst_node_t *pre_n = NULL;
          cdd_cst_child_t ch;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_cst_find_node_for_token(tree->root,
                                                &tree->base_tokens->tokens[ti],
                                                &cidx, &lpp));
          ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &pre_n));
          ch.kind = CDD_CST_CHILD_NODE;
          ch.val.node = pre_n;
          ASSERT_EQ(CDD_C_SUCCESS,
                    cdd_cst_splice_children(tree, &lpp, 0, 0, &ch, 1));
          break;
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* fsym->type_str == NULL + >64-token return expr with g_cdd_alloc_fail=1,
       plus g_gnu_find_fail = 3 on stmt-expr */
    {
      char ret_oom[512];
      char expr70[160];
      int p;
      for (p = 0; p < 70; p++) {
        expr70[p * 2] = '1';
        expr70[p * 2 + 1] = '+';
      }
      expr70[139] = '\0';
      CDD_SNPRINTF(ret_oom, sizeof(ret_oom),
                   "void f_cln(int *p){*p=0;} int f_ret_oom(void){ int "
                   "__attribute__((cleanup(f_cln))) v = 1; return %s; }",
                   expr70);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(ret_oom), &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
      {
        cdd_cst_symbol_t *sfn = NULL;
        if (cdd_cst_scope_lookup_symbol(gctx.sem_env, "f_ret_oom",
                                        CDD_CST_SYMBOL_FUNCTION,
                                        &sfn) == CDD_C_SUCCESS) {
          C_CDD_FREE(sfn->type_str);
          sfn->type_str = NULL;
        }
      }
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, gnu_standardize_unroll_ctx(tree, &gctx));
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      /* Return variable with type_str == "" */
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_parse(
              az_span_create_from_str(
                  "void f_cln(int *p){*p=0;} void f_re(void){ int "
                  "__attribute__((cleanup(f_cln))) v = 1; return emp_v; }"),
              &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_scope_add_symbol(gctx.sem_env, "emp_v",
                                         CDD_CST_SYMBOL_VARIABLE, NULL));
      {
        cdd_cst_symbol_t *sev = NULL;
        if (cdd_cst_scope_lookup_symbol(gctx.sem_env, "emp_v",
                                        CDD_CST_SYMBOL_VARIABLE,
                                        &sev) == CDD_C_SUCCESS) {
          C_CDD_FREE(sev->type_str);
          sev->type_str = (char *)calloc(4, 1);
        }
      }
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll_ctx(tree, &gctx));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
  }

  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_expr_complete.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_expr_complete(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  struct gnu_standardizer_ctx_t gctx;
  int fi;

  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;

  /* 1. Final 2 branches in gnu_standardizer_unroll.c */
  cfg.fallback_vla_to_malloc = 1;
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_cst_parse(az_span_create_from_str(
                        "void f(int n){ for(;;){ int vla_a[n]; break; } }"),
                    &tree));
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  tree = NULL;

  {
    char ret_oom[512];
    char expr70[160];
    int p;
    for (p = 0; p < 70; p++) {
      expr70[p * 2] = '1';
      expr70[p * 2 + 1] = '+';
    }
    expr70[139] = '\0';
    CDD_SNPRINTF(ret_oom, sizeof(ret_oom),
                 "void f_cln(int *p){*p=0;} int f_ret_oom(void){ int "
                 "__attribute__((cleanup(f_cln))) v = 1; return %s; }",
                 expr70);
    for (fi = 1; fi <= 25; fi++) {
      cdd_c_error_t rc_u;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(ret_oom), &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
      {
        cdd_cst_symbol_t *sfn = NULL;
        if (cdd_cst_scope_lookup_symbol(gctx.sem_env, "f_ret_oom",
                                        CDD_CST_SYMBOL_FUNCTION,
                                        &sfn) == CDD_C_SUCCESS) {
          C_CDD_FREE(sfn->type_str);
          sfn->type_str = NULL;
        }
      }
      g_cdd_alloc_fail = fi;
      rc_u = gnu_standardize_unroll_ctx(tree, &gctx);
      g_cdd_alloc_fail = 0;
      ASSERT(rc_u == CDD_C_SUCCESS || rc_u == CDD_C_ERROR_MEMORY);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
  }

  /* 2. Function pointer arithmetic, const void *, global Elvis, global i128,
        unnamed struct edges, operator & boundary cases in expr.c */
  {
    const char *ex1 = "typedef void *vptr_td_t;\n"
                      "typedef __int128 i128_td_t;\n"
                      "int foo_se(void);\n"
                      "int g_elv = foo_se() ?: 1;\n"
                      "__int128 gx;\n"
                      "__int128 gy = -gx;\n"
                      "__int128 gz = gx + gy;\n"
                      "struct SDecl { int a; } g_sd;\n";
    const char *ex2 =
        "void f_ex(int (*fp)(int), const void *cvp, void *vp, int *ip,\n"
        "          __int128 ix, unsigned __int128 ux, int c) {\n"
        "  long pdiff;\n"
        "  void *vp2;\n"
        "  int r1 = --c ?: 1;\n"
        "  int r2 = (c - + c) ?: 1;\n"
        "  c = c + + c;\n"
        "  ++c; fp++; ++fp; fp--; --fp;\n"
        "  ++cvp; --cvp;\n"
        "  fp += 1; fp -= 1; cvp += 1;\n";
    const char *ex3 =
        "  fp = fp + 1; fp = 1 + fp; cvp = 1 + cvp; pdiff = fp - fp;\n"
        "  vp = 'a' + vp;\n"
        "  vp = 0; vp + vp; 1 - vp; vp + ip; ip + vp;\n"
        "  sizeof(vptr_td_t) + 1;\n"
        "  ix = c ? -ix : -ix;\n"
        "  { -ix; -ix; ~123; ~c; }\n"
        "  ix = (-ix, -ix);\n";
    const char *ex4 =
        "  __int128 arr128[2];\n"
        "  arr128[0] += (ix + 1);\n"
        "  { vp += 1 } vp += 1; vp2 = vp += 1, vp += arr128[0], c = 0;\n"
        "  ; += 1; vp += ;\n"
        "  ix << = 1; ix < <= 1;\n"
        "  c = (1 == vp + 1) && (1 != vp + 1) && (1 < vp + 1) && (1 > vp + "
        "1);\n";
    const char *ex5 =
        "  c = (gy + gz == ix) || (gy + gz != ix) || (gy + gz < ix) || "
        "(gy + gz > ix);\n"
        "  vp2 = c ? vp + 1 : vp + 1;\n"
        "  { vp + 1 }\n"
        "  vp + ;\n"
        "  c = (1 < = 2) || (2 > = 1) || (1 & & 2) || (1 | | 2) || (1 < < 2);\n"
        "  ux = 1 + ux; ux = ix + ux;\n"
        "  if (c) return -ix ? 1 : 0;\n"
        "  return 0;\n"
        "}\n"
        "struct OuterNest { struct { struct { int in_m; }; int m2; }; };\n"
        "struct { int tail_m; }";
    char ex_buf[3584];
    CDD_SNPRINTF(ex_buf, sizeof(ex_buf), "%s%s%s%s%s", ex1, ex2, ex3, ex4, ex5);
    /* First run expr_ctx BEFORE types_ctx so sem_env has raw `unsigned
     * __int128` */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ex_buf), &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    gctx.used_int128 = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_expr_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Second run after types_ctx (`cdd_int128_t` / `cdd_uint128_t`) */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ex_buf), &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    gctx.used_int128 = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
    gctx.sem_env = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_scope_add_symbol(gctx.sem_env, "null_ty_sym",
                                       CDD_CST_SYMBOL_VARIABLE, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_expr_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 3. OOM sweeps & boundary token slices in gnu_standardizer_expr.c */
  /* 3a. OOM on expr_emit_msvc_4201_once (L316) */
  for (fi = 1; fi <= 8; fi++) {
    cdd_c_error_t rc_e;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "struct Outer { struct { int a; }; };"),
                            &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    g_gnu_alloc_fail = fi;
    rc_e = gnu_standardize_expr_ctx(tree, &gctx);
    g_gnu_alloc_fail = 0;
    ASSERT(rc_e == CDD_C_SUCCESS || rc_e == CDD_C_ERROR_MEMORY);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "struct Outer { struct { int a; }; };"),
                            &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    g_cdd_cst_alloc_token_fail = fi;
    rc_e = gnu_standardize_expr_ctx(tree, &gctx);
    g_cdd_cst_alloc_token_fail = 0;
    ASSERT(rc_e == CDD_C_SUCCESS || rc_e == CDD_C_ERROR_MEMORY);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 3b. OOM & g_gnu_find_fail on side-effecting Elvis (L513, L541) and
         binary cdd_int128_t hnode builder (L1398-1402) */
  for (fi = 1; fi <= 10; fi++) {
    cdd_c_error_t rc_e;
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str(
                          "int foo(void); void f(void){ int x = foo() ?: 1; }"),
                      &tree));
    g_gnu_alloc_fail = fi;
    rc_e = gnu_standardize_expr(tree);
    g_gnu_alloc_fail = 0;
    ASSERT(rc_e == CDD_C_SUCCESS || rc_e == CDD_C_ERROR_MEMORY);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(
                  az_span_create_from_str(
                      "typedef struct { int lo; } cdd_int128_t;\n"
                      "void f(cdd_int128_t a, cdd_int128_t b) { cdd_int128_t "
                      "c = a + b; }"),
                  &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    gctx.used_int128 = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
    g_cdd_cst_alloc_token_fail = fi;
    rc_e = gnu_standardize_expr_ctx(tree, &gctx);
    g_cdd_cst_alloc_token_fail = 0;
    ASSERT(rc_e == CDD_C_SUCCESS || rc_e == CDD_C_ERROR_MEMORY);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_cst_parse(az_span_create_from_str(
                        "int foo(void); void f(void){ int x = foo() ?: 1; }"),
                    &tree));
  g_gnu_find_fail = 1;
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_expr(tree));
  g_gnu_find_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  tree = NULL;

  /* Elvis splice realloc OOM (L539) with tight UNKNOWN + BLOCK capacities */
  for (fi = 1; fi <= 3; fi++) {
    cdd_c_error_t rc_e;
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str(
                          "int foo(void); void f(void){ int x = foo() ?: 1; }"),
                      &tree));
    {
      cdd_cst_query_result_t bres;
      size_t bi;
      if (cdd_cst_find_nodes_by_type(tree->root, CDD_CST_BLOCK, &bres) ==
          CDD_C_SUCCESS) {
        for (bi = 0; bi < bres.size; bi++)
          bres.nodes[bi]->capacity = bres.nodes[bi]->num_children;
        C_CDD_FREE(bres.nodes);
      }
      if (cdd_cst_find_nodes_by_type(tree->root, CDD_CST_UNKNOWN, &bres) ==
          CDD_C_SUCCESS) {
        for (bi = 0; bi < bres.size; bi++)
          bres.nodes[bi]->capacity = bres.nodes[bi]->num_children;
        C_CDD_FREE(bres.nodes);
      }
    }
    g_cdd_cst_realloc_fail = fi;
    rc_e = gnu_standardize_expr(tree);
    g_cdd_cst_realloc_fail = 0;
    ASSERT(rc_e == CDD_C_SUCCESS || rc_e == CDD_C_ERROR_MEMORY);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* Direct cdd_uint128_t typedef operations (strstr(..., "uint128") != NULL) */
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_cst_parse(
          az_span_create_from_str(
              "typedef struct { int lo; } cdd_uint128_t;\n"
              "void f(cdd_uint128_t u, int x){ u += 1; u = ~u; u = x + u; "
              "u = u + x; }"),
          &tree));
  memset(&gctx, 0, sizeof(gctx));
  gctx.config = &cfg;
  gctx.used_int128 = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_expr_ctx(tree, &gctx));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  tree = NULL;

  /* Unclosed unnamed struct (mutate '}' to ';') and sem_env==NULL +
   * used_int128=1 */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("struct { int a; }"), &tree));
  {
    size_t ti;
    for (ti = 0; ti < tree->base_tokens->size; ti++) {
      if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_RBRACE) {
        tree->base_tokens->tokens[ti].kind = CDD_TOKEN_SEMICOLON;
        break;
      }
    }
  }
  memset(&gctx, 0, sizeof(gctx));
  gctx.config = &cfg;
  gctx.used_int128 = 1;
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_expr_ctx(tree, &gctx));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  tree = NULL;

  /* g_gnu_find_fail = 1..3 on vp++, ++vp, vp += 1, ~ix, vp + 1 */
  {
    static const char *const ff_cases[] = {
        "void f(void *vp){ vp++; }", "void f(void *vp){ ++vp; }",
        "void f(void *vp){ vp += 1; }", "void f(__int128 ix){ ~ix; }",
        "void f(void *vp){ vp + 1; }"};
    size_t qi;
    for (qi = 0; qi < sizeof(ff_cases) / sizeof(ff_cases[0]); qi++) {
      for (fi = 1; fi <= 3; fi++) {
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(ff_cases[qi]), &tree));
        memset(&gctx, 0, sizeof(gctx));
        gctx.config = &cfg;
        gctx.used_int128 = 1;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
        g_gnu_find_fail = fi;
        ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_expr_ctx(tree, &gctx));
        g_gnu_find_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
    }
  }

  /* 3c. Leading/trailing operators, >=96-char vptr, typedef-name tokens,
         long >200-byte LHS/RHS slices, and >64-token eval OOM in += and + */
  {
    char big_ex[4096];
    char id100[108];
    char long_rhs[300];
    char sub70[240];
    int p;
    memset(id100, 'v', 100);
    id100[100] = '\0';
    for (p = 0; p < 90; p++) {
      long_rhs[p * 3] = '1';
      long_rhs[p * 3 + 1] = '0';
      long_rhs[p * 3 + 2] = '*';
    }
    long_rhs[269] = '1';
    long_rhs[270] = '\0';
    sub70[0] = 'a';
    for (p = 0; p < 25; p++) {
      memcpy(sub70 + 1 + p * 3, "[0]", 3);
    }
    sub70[1 + 25 * 3] = '\0';

    CDD_SNPRINTF(
        big_ex, sizeof(big_ex),
        "typedef void *vptr_t;\n"
        "void f_bex(void *%s, void *vp) {\n"
        "  %s++; ++%s;\n"
        "  vptr_t++; ++vptr_t; vptr_t += 1; ~vptr_t;\n"
        "  vptr_t + 1; 1 + vptr_t;\n"
        "  ((((((((((((((((((((((((((((((((((((((((vp))))))))))))))))))))))))))"
        ")))))))))))))) += (%s);\n"
        "  vp = (%s) + vp;\n"
        "  vp = vp + (%s);\n"
        "}\n",
        id100, id100, id100, long_rhs, long_rhs, long_rhs);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(big_ex), &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    gctx.used_int128 = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_expr_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* >64-token subscript-chain LHS/RHS eval OOM (no binary ops inside!) */
    for (fi = 1; fi <= 4; fi++) {
      CDD_SNPRINTF(big_ex, sizeof(big_ex),
                   "void f(int a){ %s += 1; int x = %s + 1; int y = 1 + %s; }",
                   sub70, sub70, sub70);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(big_ex), &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      gctx.used_int128 = 1;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
      g_cdd_alloc_fail = fi;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_expr_ctx(tree, &gctx));
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }

    /* Leading/trailing operators at token 0 and EOF */
    {
      static const char *const frags[] = {
          "++",  "+= 1", "vp += 1", "x +=",  "+ 1",   "x +",
          "-x",  "~",    "x <<=",   "x >>=", "x &",   "x |",
          "x <", "x >",  "x & y",   "x | y", "x < y", "x > y"};
      size_t qi;
      for (qi = 0; qi < sizeof(frags) / sizeof(frags[0]); qi++) {
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(frags[qi]), &tree));
        memset(&gctx, 0, sizeof(gctx));
        gctx.config = &cfg;
        gctx.used_int128 = 1;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
        /* Ensure has_vptr = 1 as well */
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_scope_add_symbol(gctx.sem_env, " dummy_vp",
                                           CDD_CST_SYMBOL_VARIABLE, NULL));
        {
          cdd_cst_symbol_t *svp = NULL;
          if (cdd_cst_scope_lookup_symbol(gctx.sem_env, " dummy_vp",
                                          CDD_CST_SYMBOL_VARIABLE,
                                          &svp) == CDD_C_SUCCESS) {
            C_CDD_FREE(svp->type_str);
            svp->type_str = (char *)calloc(12, 1);
            memcpy(svp->type_str, "void *", 6);
          }
        }
        ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_expr_ctx(tree, &gctx));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
    }
  }

  PASS();
}

/**
 * @brief Recursively shrinks `capacity = num_children` on every CST node in
 * `node` so subsequent `cdd_cst_splice_children` calls must `realloc`.
 *
 * @param[in,out] node Subtree root.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t test_shrink_all_caps(cdd_cst_node_t *node) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!node)
    return CDD_C_SUCCESS;
  node->capacity = node->num_children;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      rc = (cdd_c_error_t)((int)rc | (int)test_shrink_all_caps(
                                         node->children[i].val.node));
    }
  }
  return rc;
}

/**
 * @brief Unit test test_gnu_standardizer_types_complete.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_types_complete(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t cfg;
  struct gnu_standardizer_ctx_t gctx;
  int fi;

  /* 1. Final Elvis splice realloc OOM (expr.c L539) shrinking ALL nodes */
  for (fi = 1; fi <= 25; fi++) {
    cdd_c_error_t rc_e;
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str(
                          "int foo(void); void f(void){ int x = foo() ?: 1; }"),
                      &tree));
    ASSERT_EQ(CDD_C_SUCCESS, test_shrink_all_caps(tree->root));
    g_cdd_alloc_fail = fi;
    rc_e = gnu_standardize_expr(tree);
    g_cdd_alloc_fail = 0;
    ASSERT(rc_e == CDD_C_SUCCESS || rc_e == CDD_C_ERROR_MEMORY);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 2. Attribute normalization, decl_start preproc boundary, SAL printf,
        CRT XCU constructor/destructor, typeof C23/empty, __auto_type, #warning
   */
  {
    const char *tp1 =
        "{ int b0; } __attribute__((packed)) struct SPk { int a; };\n"
        "#include <stdio.h>\n"
        "__attribute__((packed)) __attribute__((aligned(4))) struct SPk2 { int "
        "a; };\n"
        "__attribute__((_abcd, __ab_, __ab_c, __abcd_e, "
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        ")) int g_at1;\n"
        "__attribute__((format(printf, 2, 3))) __attribute__((noinline)) "
        "__declspec(dllexport) __declsp10(0) "
        "void f_sal1(void (*cb)(int), const char *fmt, ...) {}\n"
        "__attribute__((format(printf, 1, 2))) __attribute13(0);\n";
    const char *tp2 =
        "__attribute__((format(printf, 1, 2))) struct SAfter { int a; };\n"
        "__attribute__((format(printf, 9, 10))) void f_sal2(const char *fmt);\n"
        "__attribute__((format(scanf, 1, 2))) void f_sal3(const char *fmt);\n"
        "__attribute__((format(printf, bad_idx, 2))) void f_sal4(const char "
        "*f);\n"
        "__attribute__((format(printf))) void f_sal5(const char *f);\n"
        "__attribute__((constructor)) __attribute__((noinline)) "
        "__declspec(dllexport) __attribute13(0) __declsp10(0) "
        "void f_ctor1(void) { { int x = 1; } }\n";
    const char *tp3 =
        "__attribute__((constructor)) void f_ctor_proto(void);\n"
        "__attribute__((constructor)) int g_not_fn;\n"
        "__inline id_8char __attribute__((always_inline)) void f_ai(void) {}\n"
        "__attribute__((visibility)) int g_vis0;\n"
        "__attribute__((visibility(\"prote99\"))) int g_vis2;\n"
        "__attribute__((section)) int g_sec0;\n"
        "__attribute__((section(123))) int g_sec2;\n"
        "__attribute__((mode)) int g_md0;\n";
    const char *tp4 = "typedef int my_auto_td_t;\n"
                      "void f_auto(int *arr) {\n"
                      "  typeof() x_emp;\n"
                      "  __auto_type a1 = arr[0], a2 = (1 + 2);\n"
                      "  __auto_type a3 = { 1 };\n"
                      "  __auto_type a4 = my_auto_td_t;\n"
                      "}\n"
                      "#\twarning\t\"tabbed msg\" \t\r\n"
                      "#warning\n"
                      "#warningx\n"
                      "#warning unquoted\n";
    char tp_buf[3584];
    CDD_SNPRINTF(tp_buf, sizeof(tp_buf), "%s%s%s%s", tp1, tp2, tp3, tp4);

    /* Run on MSVC Modern */
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    cfg.msvc_ver = 1900;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(tp_buf), &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_build_semantic_info_ex(tree, NULL, 0, &gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_scope_env_free(gctx.sem_env));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Mutate ';' after constructor attribute to '}' (saw_body == 0 on '}') */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str(
                          "__attribute__((constructor)) + void f_cs(void) {}"),
                      &tree));
    {
      size_t ti;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].kind == CDD_TOKEN_PLUS) {
          tree->base_tokens->tokens[ti].kind = CDD_TOKEN_RBRACE;
          break;
        }
      }
    }
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Run with config == NULL (default non-MSVC, non-C89) */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str(
                          "#define M_NOCFG(a...) a\n"
                          "__attribute__((deprecated)) int g_dep;\n"
                          "__attribute__((aligned + 1)) int g_al_plus;\n"
                          "__attribute__((foo + bar +)) int g_al_8a;\n"
                          "__attribute__((aligned() +)) int g_al_8b;\n"
                          "__attribute__((packed, aligned)) int g_al2;\n"
                          "__attribute__((, packed)) struct SCom { int a; };\n"
                          "#warning \"no_cfg\"\n"
                          "typeof(int) g_ty_nocfg;\n"
                          "int g_bin_nocfg = 0b101;\n"
                          "int w1; int w2; int w3; int w4; int w5;\n"),
                      &tree));
    /* Mutate w1..w5 in base_tokens to 8+ byte CDD_TOKEN_OTHER #warning edges */
    {
      size_t ti;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        cdd_token_t *bt = &tree->base_tokens->tokens[ti];
        if (bt->length == 2 && bt->start[0] == 'w') {
          bt->kind = CDD_TOKEN_OTHER;
          if (bt->start[1] == '1') {
            bt->start = (const uint8_t *)"????????";
            bt->length = 8;
          } else if (bt->start[1] == '2') {
            bt->start = (const uint8_t *)"#       ";
            bt->length = 8;
          } else if (bt->start[1] == '3') {
            bt->start = (const uint8_t *)"#warn123";
            bt->length = 8;
          } else if (bt->start[1] == '4') {
            bt->start = (const uint8_t *)"#warning \"unclosed";
            bt->length = 18;
          } else if (bt->start[1] == '5') {
            bt->start = (const uint8_t *)"#warning \"a\"\n";
            bt->length = 13;
          }
        }
      }
    }
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types(tree));
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Wiped 0-length param token before fmt in gnu_inject_printf_sal */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(
            az_span_create_from_str(
                "__attribute__((format(printf, 1, 2))) "
                "void f_w(__attribute__((unused)) const char *fmt, ...);"),
            &tree));
    /* Pre-wipe the inner `__attribute__` token at param_lp + 1 to length 0 */
    {
      size_t ti;
      int seen_attr = 0;
      for (ti = 0; ti < tree->base_tokens->size; ti++) {
        if (tree->base_tokens->tokens[ti].length == 13 &&
            memcmp(tree->base_tokens->tokens[ti].start, "__attribute__", 13) ==
                0) {
          seen_attr++;
          if (seen_attr == 2) {
            tree->base_tokens->tokens[ti].length = 0;
            break;
          }
        }
      }
    }
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 3. Modern MSVC C23 typeof/typeof_unqual combinations (msvc_ver >= 1939) */
  {
    int c89_f;
    int c99_f;
    for (c89_f = 0; c89_f <= 1; c89_f++) {
      for (c99_f = 0; c99_f <= 1; c99_f++) {
        memset(&cfg, 0, sizeof(cfg));
        cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
        cfg.msvc_ver = 1940;
        cfg.target_c89 = c89_f;
        cfg.target_c99 = c99_f;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(
                                    "typeof(1) a; __typeof_unqual__(1) b;"),
                                &tree));
        memset(&gctx, 0, sizeof(gctx));
        gctx.config = &cfg;
        ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
    }
  }

  /* 4. EOF / incomplete attribute & SAL & CRT XCU & >128-bit binary & OOM */
  {
    static const char *const eof_cases[] = {
        "__attribute__((packed))",
        "__attribute__((format(printf, 1, 2))) void f_eof",
        "__attribute__((format(printf, 9, 10))) void f_eof(const char *fmt",
        "__attribute__((constructor)) void f_ceof",
        "__attribute__((constructor)) void f_ceof(void)",
        "__attribute__((constructor))"};
    size_t qi;
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
    cfg.msvc_ver = 1310;
    for (qi = 0; qi < sizeof(eof_cases) / sizeof(eof_cases[0]); qi++) {
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(eof_cases[qi]), &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }

    /* >128-bit binary literal and 80-bit binary literal under msvc_ver = 1310
     */
    {
      char bin140[180];
      char bin80[120];
      memcpy(bin140, "int x = 0b", 10);
      memset(bin140 + 10, '1', 135);
      memcpy(bin140 + 145, ";", 2);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(bin140), &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      memcpy(bin80, "int x = 0b", 10);
      memset(bin80 + 10, '1', 80);
      memcpy(bin80 + 90, ";", 2);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(bin80), &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      /* g_gnu_find_fail = 1 on >64-bit binary literal (L1147) */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(bin80), &tree));
      g_gnu_find_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, gnu_standardize_types(tree));
      g_gnu_find_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }

    /* Binary literal replace_token_with_text fail via g_gnu_replace_fail */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("int x = 0b1010;"), &tree));
    g_gnu_replace_fail = 1;
    ASSERT_NEQ(CDD_C_SUCCESS, gnu_standardize_types(tree));
    g_gnu_replace_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    {
      char auto_oom[512];
      char sub70[240];
      int p;
      sub70[0] = 'a';
      for (p = 0; p < 25; p++)
        memcpy(sub70 + 1 + p * 3, "[0]", 3);
      sub70[1 + 25 * 3] = '\0';
      CDD_SNPRINTF(auto_oom, sizeof(auto_oom),
                   "void f(int *a){ __auto_type v = %s; }", sub70);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(auto_oom), &tree));
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types(tree));
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
  }

  /* 5. Strict ISO C89 variadic macro call-site expansion edges */
  {
    const char *vm1 =
        "#define\tM_TAB(\tfmt_1,\targs_1...\t)\tprintf(fmt_1 ,\t##\targs_1 "
        ")\r\n"
        "#define M_RAW(a_p, ...) printf(a_p ,\t##\tother_1)\n"
        "#define M_VA1(a, ...) fn(a_1, a ,\t##\t__VA_ARGS__, __VA_ARGS__1, "
        "__VA_ARGS___)\n"
        "#define M_ZERO(...) fn(__VA_ARGS__, __VA_ARGS__1, __VA_ARGS___)\n"
        "#define M_SHASH(a, ...) fn(a , # a , __VA_ARGS__)\n"
        "#define M_OBJ 1\n";
    const char *vm2 = "void f_vm(void) {\n"
                      "  M_TAB(\"hi\");\n"
                      "  M_TAB(\"%d\", (1, 2), 3);\n"
                      "  M_TAB(, 1);\n"
                      "  M_TAB(\"x\", );\n"
                      "  M_RAW(\"hi\");\n"
                      "  M_VA1(\"hi\");\n"
                      "  M_VA1(\"hi\", 1);\n"
                      "  M_ZERO();\n"
                      "  M_ZERO(42);\n"
                      "  M_SHASH(1, 2);\n"
                      "  int z = M_OBJ;\n"
                      "  M_TAB;\n"
                      "}\n";
    char vm_buf[1536];
    CDD_SNPRINTF(vm_buf, sizeof(vm_buf), "%s%s", vm1, vm2);
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(vm_buf), &tree));
    memset(&gctx, 0, sizeof(gctx));
    gctx.config = &cfg;
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;

    /* Separate malformed PREPROC_DEFINE tokens test (also hits L1689 error) */
    {
      static const char *const bad_defs[] = {
          "#define",       "#define ",        "#define M",
          "#define M(",    "#define M(a",     "#define M(a,",
          "#define M(...", "#define M(...) ", "#define M(...)\n"};
      size_t di;
      for (di = 0; di < sizeof(bad_defs) / sizeof(bad_defs[0]); di++) {
        cdd_c_error_t rc_bd;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(
                                    "#define V(...) __VA_ARGS__\nint x;"),
                                &tree));
        /* Mutate `int` token to CDD_TOKEN_PREPROC_DEFINE so Loop 1 sets
           saw_variadic_macro on V(...), and Loop 2 scans bad_defs[di] */
        if (tree->base_tokens->size > 1) {
          tree->base_tokens->tokens[1].kind = CDD_TOKEN_PREPROC_DEFINE;
          tree->base_tokens->tokens[1].start = (const uint8_t *)bad_defs[di];
          tree->base_tokens->tokens[1].length = strlen(bad_defs[di]);
        }
        memset(&gctx, 0, sizeof(gctx));
        gctx.config = &cfg;
        rc_bd = gnu_standardize_types_ctx(tree, &gctx);
        ASSERT_EQ(CDD_C_SUCCESS, rc_bd);
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
        tree = NULL;
      }
    }

    /* Unclosed call `M_ZERO(1`, >=64-char macro name, >256-byte macro body,
       >512-byte arg0, >1024-byte vargs, >2048-byte expansion, and macros OOM */
    {
      char big_vm[8192];
      char m65[70];
      char body300[310];
      char arg600[620];
      char varg1100[1120];
      char arg450[490];
      int p;
      memset(m65, 'M', 65);
      m65[65] = '\0';
      for (p = 0; p < 145; p++) {
        body300[p * 2] = '1';
        body300[p * 2 + 1] = '+';
      }
      body300[289] = '1';
      body300[290] = '\0';
      for (p = 0; p < 290; p++) {
        arg600[p * 2] = '2';
        arg600[p * 2 + 1] = '+';
      }
      arg600[579] = '2';
      arg600[580] = '\0';
      memset(arg450, 'x', 120);
      arg450[120] = '\0';
      for (p = 0; p < 540; p++) {
        varg1100[p * 2] = '3';
        varg1100[p * 2 + 1] = '+';
      }
      varg1100[1079] = '3';
      varg1100[1080] = '\0';

      CDD_SNPRINTF(
          big_vm, sizeof(big_vm),
          "#define %s(a, ...) (a)\n"
          "#define MBIG(a, ...) (%s)\n"
          "#define MEXP(a, ...) (a+a+a+a+"
          "123456789012345678901234567890123456789012345678901234567890"
          "+12345678901234567890123456789012345678901234567890123456789"
          "+12345678901234567890123456789012345678901234567890123456789"
          "+__VA_ARGS__)\n"
          "#define MEXP0(...) (__VA_ARGS__)\n"
          "void f(void) {\n"
          "  MEXP(%s, %s);\n"
          "  MEXP(%s, 1);\n"
          "  MEXP0(%s);\n"
          "}\n"
          "MEXP0(1",
          m65, body300, arg600, varg1100, arg450, varg1100);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(big_vm), &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types_ctx(tree, &gctx));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;

      /* Variadic macro with cdd_transform_macros OOM (L1689 via FOO(2)) */
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(
                                  "#define M1(a, ...) (a)\n"
                                  "int f(void){ FOO(2); return M1(1, 2); }"),
                              &tree));
      memset(&gctx, 0, sizeof(gctx));
      gctx.config = &cfg;
      g_cdd_cst_alloc_node_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, gnu_standardize_types_ctx(tree, &gctx));
      g_cdd_cst_alloc_node_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
      tree = NULL;
    }
  }

  PASS();
}

/**
 * @brief Executes the transformer_gnu_standardizer_internals_coverage_suite
 * test suite.
 */
SUITE(transformer_gnu_standardizer_internals_coverage_suite) {
  RUN_TEST(test_gnu_standardizer_reach_100_percent_coverage);
  RUN_TEST(test_gnu_standardizer_all_grammar_branches);
  RUN_TEST(test_gnu_standardizer_final_branches);
  RUN_TEST(test_gnu_standardizer_ranges_and_fn_edges);
  RUN_TEST(test_gnu_standardizer_builtins_and_unroll_edges);
  RUN_TEST(test_gnu_standardizer_builtins_unroll_complete);
  RUN_TEST(test_gnu_standardizer_expr_complete);
  RUN_TEST(test_gnu_standardizer_types_complete);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_GNU_STANDARDIZER_INTERNALS_COVERAGE_H */
