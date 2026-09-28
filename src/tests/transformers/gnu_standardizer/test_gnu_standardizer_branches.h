/**
 * @file test_gnu_standardizer_branches.h
 * @brief Branch and error case unit tests for the GNU standardizer transformer.
 */

#ifndef TEST_CDD_TRANSFORM_GNU_STANDARDIZER_BRANCHES_H
#define TEST_CDD_TRANSFORM_GNU_STANDARDIZER_BRANCHES_H

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

extern C_CDD_EXPORT int g_gnu_standardizer_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;

TEST test_gnu_standardizer_invalid_args(void) {
  cdd_cst_tree_t tree;
  memset(&tree, 0, sizeof(tree));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_gnu(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_gnu(&tree, NULL));
  PASS();
}

TEST test_gnu_standardizer_transparent_union(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "typedef union { int i; } U __attribute__((transparent_union));\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_vector_and_mode(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "typedef int v4si __attribute__((vector_size(16)));\n"
                     "typedef int q_t __attribute__((mode(QI)));\n"
                     "typedef int h_t __attribute__((mode(HI)));\n"
                     "typedef int s_t __attribute__((mode(SI)));\n"
                     "typedef int d_t __attribute__((mode(DI)));\n"
                     "typedef int t_t __attribute__((mode(TI)));\n"
                     "int x __attribute__((aligned(8)));\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_anon_and_compound(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "struct S { struct { int a; }; union { int b; }; };\n"
                     "void test_u(void) {\n"
                     "  union U { int x; };\n"
                     "  int val = 0;\n"
                     "  (union U)val = 5;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_stmt_expr_contexts(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n"
                     "  int a = ({ int x = 1; x; });\n"
                     "  int b = ({ int y = 2; y; });\n"
                     "  return a + b;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_cleanup_and_longjmp(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void clean_int(int *p) {}\n"
                     "void test_early_ret(void) {\n"
                     "  int __attribute__((cleanup(clean_int))) x = 10;\n"
                     "  if (x) return;\n"
                     "  longjmp(0, 1);\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  config.fallback_vla_to_malloc = 1;
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_builtin_expect(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int test_exp(int x) {\n"
                     "  if (__builtin_expect(x, 1)) return 1;\n"
                     "  return 0;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_more_cases(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int test_sw(int x) {\n"
                     "  switch (x) {\n"
                     "  case 5 ... 1: return 1;\n"
                     "  case -10 ... -5: return 2;\n"
                     "  default: return 0;\n"
                     "  }\n"
                     "}\n"
                     "void test_typeof_unqual(void) {\n"
                     "  const volatile int a = 1;\n"
                     "  typeof_unqual(a) b = 2;\n"
                     "  __typeof_unqual__(a) c = 3;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_more_cases_100(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t config;
  const char *code1 =
      "void my_printf(const char *fmt, ...) __attribute__((format(printf, 1, "
      "2)));\n"
      "int m1 __attribute__((mode(XX)));\n"
      "int m2 __attribute__((mode(byte)));\n"
      "struct __attribute__((packed)) PackedStruct { char a; int b; };\n"
      "union { int u; };\n"
      "struct S_unused_fam { int a[0]; int vla[n]; };\n"
      "void f_auto(int *ptr) {\n"
      "  __auto_type (int) ax = 1;\n"
      "  __auto_type a = short;\n"
      "  typeof(int) my_int = 5;\n"
      "  __typeof_unqual__(volatile int) cv = 1;\n"
      "  __typeof_unqual__(restrict int *) r = 0;\n"
      "}\n";
  const char *code_tramp = "void f_tramp_outer(void) {\n"
                           "  void f_tramp_inner(void) {}\n"
                           "  void (*fp)(void) = f_tramp_inner;\n"
                           "}\n";
  const char *code_zero_mid = "struct S_zero_mid { int a[0]; int b; };\n";
  const char *code2 = "void f_lit(void) {\n"
                      "  unsigned long long x = 18446744073709551616ULL;\n"
                      "}\n"
                      "void f_label_ref(void) {\n"
                      "  __label__ my_lbl;\n"
                      "  void *p = &&my_lbl;\n"
                      "my_lbl:;\n"
                      "}\n"
                      "void f_vla_goto(int n) {\n"
                      "  int a[n];\n"
                      "  goto done;\n"
                      "done:;\n"
                      "}\n";
  const char *code3 = "void f_vla_alloca(int n) {\n"
                      "  {\n"
                      "    int a[n];\n"
                      "  }\n"
                      "}\n"
                      "void f_switch_default(int x) {\n"
                      "  switch (x) {\n"
                      "  default: {}\n"
                      "  }\n"
                      "}\n"
                      "int empty_init_arr[] = {};\n"
                      "int f_expect(int x) {\n"
                      "  if (__builtin_expect((x > 0), 1)) return 1;\n"
                      "  if (__builtin_expect(x)) return 2;\n"
                      "  return 0;\n"
                      "}\n";
  const char *code4 = "#define LOG_WS(fmt, ...) printf(fmt, ## __VA_ARGS__)\n";
  const char *code5 = "int f_omitted_cond(int x, int y, int z) {\n"
                      "  int a = 1, b = x ? : y;\n"
                      "  int c = (x ? : y);\n"
                      "  a = x ? y ? : z : a;\n"
                      "  a = x ? y : z ? : a;\n"
                      "  a = x ? : y;\n"
                      "  case x ? : y;\n"
                      "  while x ? : y;\n"
                      "  for x ? : y;\n"
                      "  do x ? : y;\n"
                      "  switch x ? : y;\n"
                      "  return x + y * z ? : y;\n"
                      "}\n";

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code1), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  tree = NULL;
  out = NULL;

  /* Trampoline detection returns CDD_C_ERROR_SYSTEM */
  ASSERT_EQ(0, cdd_cst_parse(
                   az_span_create_from_str((char *)(size_t)code_tramp), &tree));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;

  /* Zero length in middle polyfills to -1 comment */
  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_zero_mid),
                       &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "-1 /* zero-length array in middle of struct */") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  tree = NULL;
  out = NULL;

  /* Code2 with label ref and VLA goto */
  config.fallback_vla_to_malloc = 1;
  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code2), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  tree = NULL;
  out = NULL;

  /* Code3 with alloca mode */
  config.fallback_vla_to_malloc = 0;
  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code3), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  tree = NULL;
  out = NULL;

  /* Code4 with target_c89 for macro */
  config.fallback_vla_to_malloc = 1;
  config.target_c89 = 1;
  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code4), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  tree = NULL;
  out = NULL;

  /* Code5 for omitted conditional */
  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code5), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  free(out);
  cdd_cst_tree_free(tree);
  tree = NULL;
  out = NULL;

  PASS();
}

TEST test_gnu_standardizer_error_branches(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_transform_config_t config;
  const char *code_ret_void = "void f(void) { return (void)0; }\n";
  const char *code_noarg_attr = "void f(void) __attribute__((pure));\n";
  const char *code_scope_cleanup =
      "void clean(int *p) {}\n"
      "void f(void) {\n"
      "  {\n"
      "    int __attribute__((cleanup(clean))) x = 1;\n"
      "  }\n"
      "}\n";
  const char *code_cleanup = "void clean(int *p) {}\n"
                             "void f(void) {\n"
                             "  int __attribute__((cleanup(clean))) x = 1;\n"
                             "  return;\n"
                             "}\n";
  const char *code_lbl = "void f(void) {\n"
                         "  __label__ l1;\n"
                         "  goto l1;\n"
                         "l1:;\n"
                         "}\n";
  const char *code_lbl_break = "void f(void) { __label__ l1 = 0; }\n";
  const char *code_cases = "int f(int x) {\n"
                           "  switch (x) {\n"
                           "  case 1 ... 3: return 1;\n"
                           "  }\n"
                           "  return 0;\n"
                           "}\n";
  const char *code_desig = "int arr[5] = { [0 ... 2] = 1 };\n";
  const char *code_vla = "void f(int n) { int a[n]; }\n";
  const char *code_vla_block = "void f(int n) { { int a[n]; } }\n";
  const char *code_stmt = "int x = ({ int a = 1; a; });\n";
  const char *code_complex =
      "__complex__ float z; void fc(void) { __real__ z; __imag__ z; }\n";
  const char *code_align = "int x __attribute__((aligned(8)));\n";
  const char *code_typeof_arr = "typeof(int[5]) arr;\n";
  const char *code_packed = "struct __attribute__((packed)) X { int a; };\n";
  const char *code_tramp =
      "void f_out(void) { void f_in(void) {} void (*fp)(void) = f_in; }\n";
  const char *code_lval = "void fl(void) { int val = 0; (char)val = 5; }\n";
  const char *code_cond = "int fc(int x, int y) { return x ? : y; }\n";

  memset(&config, 0, sizeof(config));
  config.fallback_vla_to_malloc = 1;

  /* g_gnu_standardizer_fail == 13 (pool string strdup/malloc fail) */
  {
    const char *pout = NULL;
    cdd_cst_tree_t tr;
    memset(&tr, 0, sizeof(tr));
    g_gnu_standardizer_fail = 13;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_pool_string_safe(&tr, "abc", &pout));
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_pool_string_safe_len(&tr, "abc", 3, &pout));
    g_gnu_standardizer_fail = 0;
  }

  /* g_gnu_standardizer_fail == 14 (tramp parent_func == NULL) */
  g_gnu_standardizer_fail = 14;
  ASSERT_EQ(0, cdd_cst_parse(
                   az_span_create_from_str((char *)(size_t)code_tramp), &tree));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 15 (typeof rb == NULL) */
  g_gnu_standardizer_fail = 15;
  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_typeof_arr),
                       &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 16 (packed n_tok == NULL) */
  g_gnu_standardizer_fail = 16;
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_packed),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 6 (return void expr semi_tok failure) */
  g_gnu_standardizer_fail = 6;
  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_ret_void),
                       &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 7 (no-arg attribute p_node NULL) */
  g_gnu_standardizer_fail = 7;
  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_noarg_attr),
                       &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* code_lbl_break */
  ASSERT_EQ(
      0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_lbl_break),
                       &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;

  /* g_gnu_standardizer_fail == 8 (parent == NULL in RBRACE scope cleanups) */
  g_gnu_standardizer_fail = 8;
  ASSERT_EQ(0, cdd_cst_parse(
                   az_span_create_from_str((char *)(size_t)code_scope_cleanup),
                   &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 17 (pooled == NULL in RBRACE scope cleanups) */
  g_gnu_standardizer_fail = 17;
  ASSERT_EQ(0, cdd_cst_parse(
                   az_span_create_from_str((char *)(size_t)code_scope_cleanup),
                   &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 10 (owning_node == NULL in RBRACE) */
  g_gnu_standardizer_fail = 10;
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_cleanup),
                          &tree));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 11 (parent == NULL in return cleanups) */
  g_gnu_standardizer_fail = 11;
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_cleanup),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 12 (parent == NULL in local label ref) */
  g_gnu_standardizer_fail = 12;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_lbl),
                             &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 3 (cdd_append_int failure in local label) */
  g_gnu_standardizer_fail = 3;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_lbl),
                             &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 3 (cdd_append_int failure in case ranges) */
  g_gnu_standardizer_fail = 3;
  ASSERT_EQ(0, cdd_cst_parse(
                   az_span_create_from_str((char *)(size_t)code_cases), &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 1 (pool_string_safe failure in case ranges) */
  g_gnu_standardizer_fail = 1;
  ASSERT_EQ(0, cdd_cst_parse(
                   az_span_create_from_str((char *)(size_t)code_cases), &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 3 (cdd_append_int failure in designated init
   * ranges) */
  g_gnu_standardizer_fail = 3;
  ASSERT_EQ(0, cdd_cst_parse(
                   az_span_create_from_str((char *)(size_t)code_desig), &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 1 (pool_string_safe failure in designated init
   * ranges) */
  g_gnu_standardizer_fail = 1;
  ASSERT_EQ(0, cdd_cst_parse(
                   az_span_create_from_str((char *)(size_t)code_desig), &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 1 in return cleanups and scope cleanups */
  g_gnu_standardizer_fail = 1;
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_cleanup),
                          &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 1 in local labels */
  g_gnu_standardizer_fail = 1;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_lbl),
                             &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 1 in lvalue cast */
  g_gnu_standardizer_fail = 1;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_lval),
                             &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 1 in conditional ? : */
  g_gnu_standardizer_fail = 1;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_cond),
                             &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 19 (conditional lhs_start == 0) */
  g_gnu_standardizer_fail = 19;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_cond),
                             &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_gnu_standardizer_fail == 1 in variadic macro */
  g_gnu_standardizer_fail = 1;
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str(
                              (char *)(size_t) "#define M(args...) f(args)\n"),
                          &tree));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_gnu(tree, &config));
  cdd_cst_tree_free(tree);
  tree = NULL;
  g_gnu_standardizer_fail = 0;

  /* g_cdd_cst_alloc_node_fail error returns */
  if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_stmt),
                    &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  {
    int vi;
    for (vi = 1; vi <= 6; vi++) {
      if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_vla),
                        &tree) == 0) {
        g_cdd_cst_alloc_node_fail = vi;
        cdd_transform_gnu(tree, &config);
        g_cdd_cst_alloc_node_fail = 0;
        cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    config.fallback_vla_to_malloc = 0;
    if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_vla),
                      &tree) == 0) {
      g_cdd_cst_alloc_node_fail = 1;
      cdd_transform_gnu(tree, &config);
      g_cdd_cst_alloc_node_fail = 0;
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
    config.fallback_vla_to_malloc = 1;
    for (vi = 1; vi <= 6; vi++) {
      if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_vla_block),
                        &tree) == 0) {
        g_cdd_cst_alloc_node_fail = vi;
        cdd_transform_gnu(tree, &config);
        g_cdd_cst_alloc_node_fail = 0;
        cdd_cst_tree_free(tree);
        tree = NULL;
      }
    }
    /* mock 20 for zero-length array fwd continue */
    g_gnu_standardizer_fail = 20;
    if (cdd_cst_parse(az_span_create_from_str(
                          (char *)(size_t) "struct S { int a[0]; int b; };\n"),
                      &tree) == 0) {
      cdd_transform_gnu(tree, &config);
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
    g_gnu_standardizer_fail = 0;
  }
  if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_typeof_arr),
                    &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_typeof_arr),
                    &tree) == 0) {
    g_gnu_standardizer_fail = 15;
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_gnu_standardizer_fail = 0;
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(
          az_span_create_from_str((char *)(size_t) "typeof(int) x;\n"),
          &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(
          az_span_create_from_str((char *)(size_t) "__auto_type x = 1;\n"),
          &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "unsigned long long x = "
                                         "18446744073709551616ULL;\n"),
                    &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(
          az_span_create_from_str(
              (char *)(size_t) "struct Anon { struct { int a; }; };\n"),
          &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(
          az_span_create_from_str(
              (char *)(size_t) "struct AnonU { union { int a; }; };\n"),
          &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_complex),
                    &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_align),
                    &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(az_span_create_from_str((char *)(size_t)code_scope_cleanup),
                    &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* g_gnu_standardizer_fail == 18 (builtin expect j.length == 0 loop) */
  g_gnu_standardizer_fail = 18;
  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "int f(int x) { return "
                                         "__builtin_expect(x, 1); }\n"),
                    &tree) == 0) {
    cdd_transform_gnu(tree, &config);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  g_gnu_standardizer_fail = 0;

  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "void fc(float z) { __real__ z; }\n"),
                    &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(az_span_create_from_str(
                        (char *)(size_t) "void fc(float z) { __imag__ z; }\n"),
                    &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  if (cdd_cst_parse(
          az_span_create_from_str(
              (char *)(size_t) "void fc(void) { __auto_type a = 1; }\n"),
          &tree) == 0) {
    g_cdd_cst_alloc_node_fail = 1;
    cdd_transform_gnu(tree, &config);
    g_cdd_cst_alloc_node_fail = 0;
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  PASS();
}

SUITE(transformer_gnu_standardizer_branches_suite) {
  RUN_TEST(test_gnu_standardizer_invalid_args);
  RUN_TEST(test_gnu_standardizer_transparent_union);
  RUN_TEST(test_gnu_standardizer_vector_and_mode);
  RUN_TEST(test_gnu_standardizer_anon_and_compound);
  RUN_TEST(test_gnu_standardizer_stmt_expr_contexts);
  RUN_TEST(test_gnu_standardizer_cleanup_and_longjmp);
  RUN_TEST(test_gnu_standardizer_builtin_expect);
  RUN_TEST(test_gnu_standardizer_more_cases);
  RUN_TEST(test_gnu_standardizer_more_cases_100);
  RUN_TEST(test_gnu_standardizer_error_branches);
}
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_GNU_STANDARDIZER_BRANCHES_H */
