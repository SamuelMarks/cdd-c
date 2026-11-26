/**
 * @file test_gnu_standardizer.h
 * @brief Unit tests for the GNU standardizer transformer.
 */

#ifndef TEST_CDD_TRANSFORM_GNU_STANDARDIZER_H
#define TEST_CDD_TRANSFORM_GNU_STANDARDIZER_H

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
#include "transformers/gnu_standardizer/gnu_standardizer_internal.h"
#include "c_str_span.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;

/**
 * @brief Unit test test_cdd_transform_gnu.
 * @return Greatest test result.
 */
TEST test_cdd_transform_gnu(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void foo() __attribute__((unused));\n"
                     "void bar() __attribute__((noreturn));\n"
                     "struct __attribute__((packed)) X { int a; };\n"
                     "__extension__ int y;\n"
                     "int z = __alignof__(int);\n"
                     "int main() {\n"
                     "  int a = ({ int b = 1; b; });\n"
                     "  int arr[a];\n"
                     "  struct Empty {};\n"
                     "  return 0;\n"
                     "}\n";
  char *out = NULL;
  int rc = 0;
  size_t i;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));

  rc = cdd_cst_parse(az_span_create_from_str(code), &tree);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->length > 0) {
      fprintf(stderr, "TOKEN: [%.*s]\n", (int)t->length, t->start);
    }
  }

  rc = cdd_transform_gnu(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  fprintf(stderr, "OUT WAS: [%s]\n", out);
  ASSERT(strstr(out, "/* unused */") != NULL);
  ASSERT(strstr(out, "_Noreturn") != NULL);
  ASSERT(strstr(out, "pack(push, 1)") != NULL);
  ASSERT(strstr(out, "pack(pop)") != NULL);
  ASSERT(strstr(out, "alloca((a) * sizeof(*arr))") != NULL);
  ASSERT(strstr(out, "_Alignof") != NULL);
  ASSERT(strstr(out, "char _pad;") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_stmt_expr.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_stmt_expr(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "void side_fx(void);\n"
      "int main() {\n"
      "  int a = ({ int b = 1; b; });\n"
      "  int n = ({ int x = ({ int y = 2; y + 1; }); x * 3; });\n"
      "  ({ side_fx(); });\n"
      "  return a + n;\n"
      "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "int __cdd_stmt_expr_0;") != NULL);
  ASSERT(strstr(out, "__cdd_stmt_expr_0 = (b);") != NULL);
  ASSERT(strstr(out, "int a = __cdd_stmt_expr_0;") != NULL);
  ASSERT(strstr(out, "__cdd_stmt_expr_2 = (y + 1);") != NULL);
  ASSERT(strstr(out, "int n = __cdd_stmt_expr_1;") != NULL);
  ASSERT(strstr(out, "{ side_fx(); }") != NULL);
  ASSERT(strstr(out, "((void)0);") != NULL);
  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_computed_goto.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_computed_goto(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  void *ptr = &&my_label;\n  goto "
                     "*ptr;\nmy_label:\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "goto *") == NULL);
  ASSERT(strstr(out, "((void *)(size_t)1)") != NULL);
  ASSERT(strstr(out, "switch (__cdd_cgoto_target)") != NULL);
  ASSERT(strstr(out, "case 1: goto my_label;") != NULL);
  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_case_ranges.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_case_ranges(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "int main() {\n  int x = 3;\n  switch(x) {\n  case 1 ... 5:\n    return "
      "1;\n  case -5 ... -1:\n    return 2;\n  }\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "case 1: case 2: case 3: case 4: case 5") != NULL);
  ASSERT(strstr(out, "case -5: case -4: case -3: case -2: case -1") != NULL);
  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_range_init.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_range_init(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = cdd_test_mut_str("int main() {\n  int arr[10] = { [2 ... "
                                      "5] = 1 };\n  return 0;\n}\n");
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "[2] = 1, [3] = 1, [4] = 1, [5] = 1") != NULL);
  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_local_labels.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_local_labels(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n"
                     "  {\n"
                     "    __label__ my_label, other_label;\n"
                     "    void *ptr = &&my_label;\n"
                     "    goto my_label;\n"
                     "my_label:\n"
                     "    goto other_label;\n"
                     "other_label:\n"
                     "    return 1;\n"
                     "  }\n"
                     "  return 0;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* The __label__ declaration should be gone, and labels renamed */
  ASSERT(strstr(out, "__label__") == NULL);
  ASSERT(strstr(out, "__cdd_ll_my_label_1") != NULL);
  ASSERT(strstr(out, "__cdd_ll_other_label_2") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_vla_malloc.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_vla_malloc(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n"
                     "  int n = 10;\n"
                     "  int arr[n];\n"
                     "  return 0;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  config.fallback_vla_to_malloc = 1;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  printf("test_gnu_standardizer_vla_malloc: parsed\n");
  fflush(stdout);
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  printf("test_gnu_standardizer_vla_malloc: transformed\n");
  fflush(stdout);
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  printf("test_gnu_standardizer_vla_malloc: emitted out=%p\n", (void *)out);
  fflush(stdout);

  if (out) {
    printf("OUT WAS:\n%s\n", out);
    fflush(stdout);
    ASSERT(strstr(out, "malloc((n) * sizeof(*arr))") != NULL);
    ASSERT(strstr(out, "free(arr)") != NULL);
  }

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_vla_multidim.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_vla_multidim(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n"
                     "  int x = 2, y = 3, z = 4;\n"
                     "  int arr[x][y][z];\n"
                     "  return 0;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  config.fallback_vla_to_malloc = 1;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "malloc") != NULL);
  ASSERT(strstr(out, "free(arr)") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_trailing_comma.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_trailing_comma(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      cdd_test_mut_str("enum X { A, B, C, }; int arr[] = { 1, 2, 3, };");
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "A, B, C  }") != NULL || strstr(out, "A, B, C }") != NULL);
  ASSERT(strstr(out, "1, 2, 3  }") != NULL || strstr(out, "1, 2, 3 }") != NULL);
  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_zero_length_array.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_zero_length_array(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "struct flexible {\n"
                     "  int length;\n"
                     "  int data[0];\n"
                     "};\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "int data[1];") != NULL);
  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_empty_initializer.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_empty_initializer(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "struct foo { int a; int b[0]; };\n"
                     "struct foo f = { 1, { } };\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "{ 1, {") != NULL);
  ASSERT(strstr(out, "0 } }") != NULL || strstr(out, "0} }") != NULL ||
         strstr(out, "0 }") != NULL);
  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_128_bit_literals.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_128_bit_literals(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n"
                     "  __int128 a = 18446744073709551616;\n"
                     "  unsigned __int128 b = 0x10000000000000000;\n"
                     "  return 0;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* 18446744073709551616 is 2^64, so high=1, low=0 */
  printf("OUT: %s\n", out);
  ASSERT(
      strstr(
          out,
          "cdd_make_uint128(0x0000000000000001ULL, 0x0000000000000000ULL)") !=
      NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_vla_params.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_vla_params(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = cdd_test_mut_str("void func(int n, int arr[n]) {}\n");
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* Should strip 'n' */
  ASSERT(strstr(out, "void func(int n, int arr[])") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_return_void_expr.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_return_void_expr(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = cdd_test_mut_str("void func() { return (void)0; }\n");
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "(void)0; return;") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_overlapping_case_ranges.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_overlapping_case_ranges(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "int main() {\n  int x = 3;\n  switch(x) {\n  case 1 ... 5:\n    return "
      "1;\n  case 4 ... 8:\n    return 2;\n  }\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* We just test it generates properly, the compiler will complain. */
  ASSERT(strstr(out, "case 1: case 2: case 3: case 4: case 5") != NULL);
  ASSERT(strstr(out, "case 4: case 5: case 6: case 7: case 8") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_empty_fallthrough_block.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_empty_fallthrough_block(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  int x = 1;\n  switch(x) {\n  case 1:\n  "
                     "{}\n  case 2:\n    return 2;\n  }\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* Should rewrite {} after case label to ; */
  ASSERT(
      strstr(out, "case 1: ;") != NULL || strstr(out, "case 1:\n  ;") != NULL ||
      strstr(out, "case 1: ;") != NULL || strstr(out, "case 1:\n   ;") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_attributes.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_attributes(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int __attribute__((vector_size(16))) v;\n"
                     "int __attribute__((aligned(16))) a;\n"
                     "int __attribute__((mode(QI))) m;\n"
                     "int __attribute__((pure)) p;\n"
                     "int __attribute__((nonnull(1))) q;\n";
  cdd_transform_config_t config;
  char *out = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* Should strip vector_size, change aligned to _Alignas, and polyfill mode */
  ASSERT(strstr(out, "vector_size") == NULL);
  ASSERT(strstr(out, "_Alignas(16)") != NULL);
  ASSERT(strstr(out, "/* mode(QI) -> int8_t */") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_magic_identifiers.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_magic_identifiers(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void my_super_func() {\n"
                     "  const char *a = __FUNCTION__;\n"
                     "  const char *b = __PRETTY_FUNCTION__;\n"
                     "  const char *c = __func__;\n"
                     "}\n";
  cdd_transform_config_t config;
  char *out = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "\"my_super_func\"") != NULL);
  ASSERT(strstr(out, "__FUNCTION__") == NULL);
  ASSERT(strstr(out, "__PRETTY_FUNCTION__") == NULL);
  ASSERT(strstr(out, "__func__") == NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_trampoline.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_trampoline(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void foo() {\n"
                     "  void bar() {}\n"
                     "  void (*p)() = bar;\n"
                     "}\n";
  cdd_transform_config_t config;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, cdd_transform_gnu(tree, &config));

  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_shuffle.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_shuffle(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void foo() {\n"
                     "  __builtin_shuffle(a, b, mask);\n"
                     "  __builtin_shufflevector(a, b, 0, 1);\n"
                     "}\n";
  cdd_transform_config_t config;
  char *out = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "cdd_builtin_shuffle(a, b, mask)") != NULL);
  ASSERT(strstr(out, "cdd_builtin_shufflevector(a, b, 0, 1)") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_cleanup.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_cleanup(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void my_free(int *p) {}\n"
                     "void test_func() {\n"
                     "  int __attribute__((cleanup(my_free))) x = 5;\n"
                     "  if (x) return 1;\n"
                     "  goto end;\n"
                     "end:;\n"
                     "}\n";
  cdd_transform_config_t config;
  char *out = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));

  memset(&config, 0, sizeof(config));
  {
    int rc = cdd_transform_gnu(tree, &config);
    ASSERT_EQ(0, rc);
  }

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "my_free(&x);") != NULL);
  ASSERT(strstr(out, "goto cross-scope cleanups unsupported") != NULL);
  ASSERT(strstr(out, "return __cdd_ret;") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_typeof.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_typeof(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "typeof(int) a = 1;\n"
                     "__typeof__(char) b = 'A';\n"
                     "typeof_unqual(float) c = 1.0f;\n"
                     "__typeof_unqual__(double) d = 1.0;\n"
                     "__auto_type e = 2;\n"
                     "typeof(int[5]) arr = {1, 2, 3, 4, 5};\n"
                     "typeof(s.bf) x = 0;\n"
                     "typeof(ptr->bf) y = 0;\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  printf("TYPEOF OUT: %s\n", out);
  ASSERT(strstr(out, "int a = 1;") != NULL);
  ASSERT(strstr(out, "char b = 'A';") != NULL);
  ASSERT(strstr(out, "float c = 1.0f;") != NULL);
  ASSERT(strstr(out, "double d = 1.0;") != NULL);
  ASSERT(strstr(out, "int e = 2;") != NULL);
  printf("OUT: %s\n", out);
  ASSERT(strstr(out, "typedef int __cdd_typeof_arr_") != NULL);
  printf("OUT: %s\n", out);
  ASSERT(strstr(out, "[5]; __cdd_typeof_arr_") != NULL);
  ASSERT(strstr(out, " arr = {1, 2, 3, 4, 5};") != NULL);
  ASSERT(strstr(out, "int x = 0;") != NULL);
  ASSERT(strstr(out, "int y = 0;") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_variadic_macros.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_variadic_macros(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#define LOG1(fmt, args...) printf(fmt, ##args)\n"
                     "#define LOG2(fmt, ...) printf(fmt, ##__VA_ARGS__)\n"
                     "#define LOG3(args...) printf(args)\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  printf("MACRO OUTPUT: %s\n", out);

  ASSERT(strstr(out, "#define LOG1(fmt, ...) printf(fmt, __VA_ARGS__)") !=
         NULL);
  ASSERT(strstr(out, "#define LOG2(fmt, ...) printf(fmt, __VA_ARGS__)") !=
         NULL);
  ASSERT(strstr(out, "#define LOG3(...) printf(__VA_ARGS__)") != NULL);
  ASSERT(strstr(out, "__VA_OPT__") == NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);

  /* Strict ISO C89 call-site variadic macro expansion */
  {
    const char *c89_code = "#define DBG(fmt, args...) printf(fmt, ##args)\n"
                           "void run(void) { DBG(\"val=%d\", 7); }\n";
    tree = NULL;
    out = NULL;
    memset(&config, 0, sizeof(config));
    config.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    config.target_c89 = 1;
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(c89_code), &tree));
    ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
    ASSERT_EQ(0, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "printf(\"val=%d\", 7)") != NULL);
    free(out);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  }
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_cdd_transform_complex_numbers.
 * @return Greatest test result.
 */
TEST test_cdd_transform_complex_numbers(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void calc() {\n"
                     "  __complex__ float z;\n"
                     "  __real__ z = 1.0f;\n"
                     "  __imag__ z = 2.0f;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t cfg = {0};
  int rc = 0;

  rc = cdd_cst_parse(az_span_create_from_str(code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_gnu(tree, &cfg);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  printf("COMPLEX MACRO OUT: %s\n", out);

  printf("COMPLEX MACRO OUT: [%s]\n", out);
  ASSERT(strstr(out, "struct { float real, imag; }") != NULL);
  ASSERT(strstr(out, "z . real = 1.0f;") != NULL ||
         strstr(out, "z.real = 1.0f;") != NULL ||
         strstr(out, "z.real  = 1.0f;") != NULL ||
         strstr(out, "z.real = 1.0f;") != NULL ||
         strstr(out, "z.real  = 1.0f;") != NULL);
  ASSERT(strstr(out, "z . imag = 2.0f;") != NULL ||
         strstr(out, "z.imag = 2.0f;") != NULL ||
         strstr(out, "z.imag  = 2.0f;") != NULL ||
         strstr(out, "z.imag = 2.0f;") != NULL ||
         strstr(out, "z.imag  = 2.0f;") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_comment_preservation.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_comment_preservation(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "/* block start */\n"
                     "int main() { // inline comment\n"
                     "  int a = ({ int b = 1; /* mid-expr */ b; });\n"
                     "  int arr[a]; /* vla decl */\n"
                     "  return 0;\n"
                     "}\n"
                     "/* block end */";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "/* block start */") != NULL);
  ASSERT(strstr(out, "// inline comment") != NULL);
  ASSERT(strstr(out, "/* mid-expr */") != NULL);
  ASSERT(strstr(out, "/* vla decl */") != NULL);
  ASSERT(strstr(out, "/* block end */") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_float_extensions.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_float_extensions(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "_Decimal32 a;\n"
                     "_Decimal64 b;\n"
                     "_Decimal128 c;\n"
                     "__fp16 d;\n"
                     "_Float16 e;\n"
                     "__bf16 f;\n"
                     "_Fract g;\n"
                     "_Accum h;\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "float a;") != NULL);
  ASSERT(strstr(out, "double b;") != NULL);
  ASSERT(strstr(out, "double c;") != NULL);
  ASSERT(strstr(out, "uint16_t d;") != NULL);
  ASSERT(strstr(out, "uint16_t e;") != NULL);
  ASSERT(strstr(out, "uint16_t f;") != NULL);
  ASSERT(strstr(out, "float g;") != NULL);
  ASSERT(strstr(out, "double h;") != NULL);
  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_lvalue_cast.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_lvalue_cast(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = cdd_test_mut_str("void f() { int x; (char)x = 5; }\n");
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  printf("LVALUE OUT: %s\n", out);
  ASSERT(strstr(out, "*(char*)&x = 5;") != NULL);
  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_omitted_conditional.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_omitted_conditional(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n"
                     "  int a = 1, b = 2;\n"
                     "  int c = a ? : b;\n"
                     "  int d = (a + b) ? : 5;\n"
                     "  return a || b ? : c;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "a ? a  : b") != NULL);
  ASSERT(strstr(out, "(a + b) ? (a + b)  : 5") != NULL);
  ASSERT(strstr(out, "a || b ? a || b  : c") != NULL);
  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_semantic_include_and_typeof.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_semantic_include_and_typeof(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t config;
  const char *inc_dirs[1];
  const char *hdr_content =
      "typedef struct { char *label; unsigned long count; } hdr_item_t;\n"
      "const char *fetch_item_label(hdr_item_t *it);\n";
  const char *src_code = "#include \"phase2_sem_hdr.h\"\n"
                         "void demo(hdr_item_t *it_ptr) {\n"
                         "  hdr_item_t local_it;\n"
                         "  int nums[4];\n"
                         "  typeof(it_ptr) p_copy = it_ptr;\n"
                         "  typeof(local_it.label) lbl = 0;\n"
                         "  typeof(it_ptr->count) cnt = 0;\n"
                         "  typeof(nums) nums_copy;\n"
                         "  __auto_type fetched = fetch_item_label(it_ptr);\n"
                         "}\n";
  FILE *fp = fopen("phase2_sem_hdr.h", "wb");
  ASSERT(fp != NULL);
  ASSERT_EQ(strlen(hdr_content),
            fwrite(hdr_content, 1, strlen(hdr_content), fp));
  fclose(fp);

  inc_dirs[0] = ".";
  memset(&config, 0, sizeof(config));
  config.include_dirs = inc_dirs;
  config.num_include_dirs = 1;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(src_code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "hdr_item_t * p_copy = it_ptr;") != NULL);
  ASSERT(strstr(out, "char * lbl = 0;") != NULL);
  ASSERT(strstr(out, "unsigned long cnt = 0;") != NULL);
  ASSERT(
      strstr(
          out,
          "typedef int __cdd_typeof_arr_0[4]; __cdd_typeof_arr_0 nums_copy;") !=
      NULL);
  ASSERT(strstr(out, "const char * fetched = fetch_item_label(it_ptr);") !=
         NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  remove("phase2_sem_hdr.h");

  /* VS 2022 17.9+ C23 mode preserves native typeof / typeof_unqual */
  {
    const char *c23_src =
        "__typeof__(int *) p; __typeof_unqual__(const int) q;\n";
    tree = NULL;
    out = NULL;
    memset(&config, 0, sizeof(config));
    config.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    config.msvc_ver = 1939;
    ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(c23_src), &tree));
    ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
    ASSERT_EQ(0, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "typeof(int *) p;") != NULL);
    ASSERT(strstr(out, "typeof_unqual(const int) q;") != NULL);
    free(out);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  }
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_cleanup_return_break_continue.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_cleanup_return_break_continue(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t config;
  const char *code =
      "void free_str(char **p);\n"
      "char *work(int mode) {\n"
      "  char * __attribute__((__cleanup__(free_str))) outer_s = 0;\n"
      "  while (mode > 0) {\n"
      "    char * __attribute__((cleanup(free_str))) loop_s = 0;\n"
      "    switch (mode) {\n"
      "      case 1:\n"
      "        continue;\n"
      "      default:\n"
      "        break;\n"
      "    }\n"
      "    if (mode == 2)\n"
      "      break;\n"
      "  }\n"
      "  return outer_s;\n"
      "}\n";
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "__auto_type") == NULL);
  ASSERT(strstr(out, "{ char * __cdd_ret = ( outer_s); free_str(&outer_s); "
                     "return __cdd_ret; }") != NULL);
  ASSERT(strstr(out, "{ free_str(&loop_s); continue; }") != NULL);
  ASSERT(strstr(out, "{ free_str(&loop_s); break; }") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_elvis_typed_hoist.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_elvis_typed_hoist(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t config;
  const char *code = "char *get_opt_str(int id);\n"
                     "char *pick(int id) {\n"
                     "  char *s = get_opt_str(id) ? : \"fallback\";\n"
                     "  int n = id++ ? : 99;\n"
                     "  return s;\n"
                     "}\n";
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "char * __cdd_cond_0 = (get_opt_str(id));") != NULL);
  ASSERT(strstr(out, "char *s = __cdd_cond_0 ? __cdd_cond_0 : \"fallback\";") !=
         NULL);
  ASSERT(strstr(out, "int __cdd_cond_1 = (id++);") != NULL);
  ASSERT(strstr(out, "int n = __cdd_cond_1 ? __cdd_cond_1 : 99;") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Unit test test_gnu_standardizer_msvc_anon_struct_union.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_msvc_anon_struct_union(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t config;
  const char *code = "struct Packet {\n"
                     "  struct { int hdr; };\n"
                     "  union { int i; float f; };\n"
                     "};\n";
  memset(&config, 0, sizeof(config));
  config.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "_cdd_anon_") == NULL);
  ASSERT(strstr(out, "#pragma warning(disable: 4201)") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Unit test for MSVC attribute lowering, `__attr__` spellings,
 * post-struct `packed`, SAL `_Printf_format_string_`, and `.CRT$XCU`
 * constructors/destructors.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_msvc_attributes_unified(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t config;
  const char *code1 =
      "struct PostPacked { int a; char b; } __attribute__((__packed__));\n"
      "int g_al __attribute__((__aligned__(32)));\n"
      "void die_now(void) __attribute__((__noreturn__));\n"
      "static inline int fast_fn(int x) __attribute__((__always_inline__)) {\n"
      "  return x + 1;\n"
      "}\n";
  const char *code2 =
      "void slow_fn(void) __attribute__((__noinline__));\n"
      "int api_fn(void) __attribute__((__visibility__(\"default\"), "
      "__warn_unused_result__, __deprecated__));\n"
      "int log_msg(int lvl, const char *fmt, ...) "
      "__attribute__((__format__(__printf__, 2, 3)));\n"
      "static void my_init(void) __attribute__((__constructor__)) {}\n"
      "static void my_fini(void) __attribute__((__destructor__)) {}\n";
  char full_code[1536];
  CDD_SNPRINTF(full_code, sizeof(full_code), "%s%s", code1, code2);

  memset(&config, 0, sizeof(config));
  config.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(full_code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "#pragma pack(push, 1)\nstruct PostPacked") != NULL);
  ASSERT(strstr(out, "#pragma pack(pop)") != NULL);
  ASSERT(strstr(out, "__declspec(align(32)) int g_al") != NULL);
  ASSERT(strstr(out, "__declspec(noreturn) void die_now") != NULL);
  ASSERT(strstr(out, "__forceinline static") != NULL);
  ASSERT(strstr(out, "__declspec(noinline) void slow_fn") != NULL);
  ASSERT(strstr(out, "__declspec(dllexport)") != NULL);
  ASSERT(strstr(out, "_Check_return_") != NULL);
  ASSERT(strstr(out, "__declspec(deprecated)") != NULL);
  ASSERT(strstr(out, "_Printf_format_string_ const char *fmt") != NULL);
  ASSERT(strstr(out, "__cdd_ctor_my_init") != NULL);
  ASSERT(strstr(out, "__cdd_dtor_my_fini") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Unit test for character, hex, octal, binary, and enum-constant case
 * and array-initializer ranges.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_extended_ranges(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t config;
  const char *code =
      "enum Bounds { B_LO = 2, B_HI = 4 };\n"
      "int test_rng(int c) {\n"
      "  int tbl[8] = { [B_LO ... B_HI] = (1 + 1), [0b00 ... 0b01] = 9 };\n"
      "  switch (c) {\n"
      "  case 'a' ... 'c': return tbl[0];\n"
      "  case '\\x00' ... '\\x02': return 2;\n"
      "  case 0x10 ... 0x12: return 3;\n"
      "  case B_LO ... B_HI: return 4;\n"
      "  default: return 0;\n"
      "  }\n"
      "}\n";
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "case 'a': case 'b': case 'c'") != NULL);
  ASSERT(strstr(out, "case 0: case 1: case 2") != NULL);
  ASSERT(strstr(out, "case 16: case 17: case 18") != NULL);
  ASSERT(strstr(out, "case 2: case 3: case 4") != NULL);
  ASSERT(strstr(out, "[2] = (1 + 1), [3] = (1 + 1), [4] = (1 + 1)") != NULL);
  ASSERT(strstr(out, "[0] = 9, [1] = 9") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Unit test for GNU `__builtin_*` intrinsics and GCC inline assembly
 * lowering.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_builtins_and_asm(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t config;
  const char *code1a =
      "typedef int my_i32_t;\n"
      "struct Pair { int first; int second; };\n"
      "int use_bi(unsigned int x, int dyn) {\n"
      "  int sum = 0;\n"
      "  int c1 = __builtin_clz(x) + __builtin_ctz(x) + "
      "__builtin_ffs((int)x);\n"
      "  int c2 = __builtin_popcount(x) + __builtin_parity(x) + "
      "(int)__builtin_bswap32(x);\n";
  const char *code1b =
      "  int ov = __builtin_add_overflow((int)x, dyn, &sum);\n"
      "  int k1 = __builtin_constant_p(42);\n"
      "  int k2 = __builtin_constant_p(dyn);\n"
      "  int ch = __builtin_choose_expr(1, 10, 20);\n"
      "  int tc = __builtin_types_compatible_p(my_i32_t, int);\n"
      "  size_t off = __builtin_offsetof(struct Pair, second);\n";
  const char *code2 =
      "  void *tmp = __builtin_alloca(16);\n"
      "  __builtin_memset(tmp, 0, 16);\n"
      "  __builtin_prefetch(tmp, 0, 1);\n"
      "  __asm__ __volatile__(\"\" : : : \"memory\");\n"
      "  __asm__(\"pause\");\n"
      "  __asm__(\"bswap %0\" : \"+r\"(x));\n"
      "  __asm__(\"cli\");\n"
      "  if (dyn < -999) { __builtin_trap(); __builtin_unreachable(); }\n"
      "  return c1 + c2 + ov + k1 + k2 + ch + tc + (int)off + sum;\n"
      "}\n";
  char full_code[1536];
  CDD_SNPRINTF(full_code, sizeof(full_code), "%s%s%s", code1a, code1b, code2);

  memset(&config, 0, sizeof(config));
  config.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(full_code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "__cdd_builtin_clz(") != NULL);
  ASSERT(strstr(out, "__cdd_builtin_ctz(") != NULL);
  ASSERT(strstr(out, "__cdd_builtin_ffs(") != NULL);
  ASSERT(strstr(out, "__cdd_builtin_popcount(") != NULL);
  ASSERT(strstr(out, "__cdd_builtin_parity(") != NULL);
  ASSERT(strstr(out, "__cdd_builtin_bswap32(") != NULL);
  ASSERT(strstr(out, "__cdd_builtin_sadd_overflow(") != NULL);
  ASSERT(strstr(out, "int k1 = 1;") != NULL);
  ASSERT(strstr(out, "int k2 = 0;") != NULL);
  ASSERT(strstr(out, "int ch = (10);") != NULL);
  ASSERT(strstr(out, "int tc = 1;") != NULL);
  ASSERT(strstr(out, "((size_t)&(((struct Pair *)0)->second))") != NULL);
  ASSERT(strstr(out, "_alloca(16)") != NULL);
  ASSERT(strstr(out, "memset(tmp, 0, 16)") != NULL);
  ASSERT(strstr(out, "_ReadWriteBarrier()") != NULL);
  ASSERT(strstr(out, "_mm_pause()") != NULL);
  ASSERT(strstr(out, "#if !defined(_MSC_VER)") != NULL);
  ASSERT(strstr(out, "__debugbreak()") != NULL);
  ASSERT(strstr(out, "__assume(0)") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Unit test for lifting non-capturing and outer-local-capturing nested
 * functions.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_nested_functions_lift(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t config;
  const char *code =
      "int outer_calc(int base) {\n"
      "  int acc = base * 2;\n"
      "  int pure_twice(int v) { return v * 2; }\n"
      "  int add_to_acc(int delta) { acc = acc + delta; return acc; }\n"
      "  return pure_twice(3) + add_to_acc(5);\n"
      "}\n";
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "static int pure_twice(int v)") != NULL);
  ASSERT(strstr(out, "struct __cdd_frame_add_to_acc { int *acc; };") != NULL);
  ASSERT(strstr(out, "static int add_to_acc(struct __cdd_frame_add_to_acc "
                     "*__cdd_frame, int delta)") != NULL);
  ASSERT(strstr(out, "(*__cdd_frame->acc)") != NULL);
  ASSERT(strstr(out, "__cdd_f_add_to_acc.acc = &acc") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Unit test for lexical extensions (`0b...`, `\e`, `#warning`), `void *`
 * arithmetic, and `__int128` operator lowering.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_lexical_vptr_and_int128_ops(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t config;
  const char *code =
      "#warning \"portability notice\"\n"
      "int run_ext(void *buf, const void *end) {\n"
      "  int mask = 0b10101100;\n"
      "  const char *ansi = \"\\e[31mred\\\\e\";\n"
      "  char esc_ch = '\\e';\n"
      "  __int128 a = 1;\n"
      "  __int128 b = 2;\n"
      "  __int128 c;\n"
      "  buf += 4;\n"
      "  buf++;\n"
      "  c = a + b;\n"
      "  a += b;\n"
      "  if (c < a) { buf = buf + 2; }\n"
      "  return (int)(end - buf) + mask + (int)ansi[0] + (int)esc_ch;\n"
      "}\n";
  memset(&config, 0, sizeof(config));
  config.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
  config.msvc_ver = 1310;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "#include <cdd_int128.h>") != NULL);
  ASSERT(strstr(out, "#pragma message(\"warning: portability notice\")") !=
         NULL);
  ASSERT(strstr(out, "0xac") != NULL);
  ASSERT(strstr(out, "\"\\033[31mred\\\\e\"") != NULL);
  ASSERT(strstr(out, "'\\033'") != NULL);
  ASSERT(strstr(out, "buf = (void *)((char *)(buf) + (4))") != NULL);
  ASSERT(strstr(out, "cdd_int128_add(") != NULL);
  ASSERT(strstr(out, "cdd_int128_cmp(") != NULL);
  ASSERT(strstr(out, "((void *)((char *)(buf) + (2)))") != NULL);
  ASSERT(strstr(out, "((const char *)(end) - (const char *)(buf))") != NULL);

  free(out);
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Golden CST transformation and OOM percolation suite across
 * `--target-msvc=legacy`, `--target-msvc=modern`, and `--target-c89`.
 * @return Greatest test result.
 */
TEST test_gnu_standardizer_golden_legacy_modern_and_oom(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t cfg;
  int flag = 0;

  /* 1. Direct entry-point wrappers & helper profile predicates */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, gnu_is_msvc_target(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, gnu_is_strict_iso_c89(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, gnu_is_msvc_target(NULL, &flag));
  ASSERT_EQ(0, flag);
  ASSERT_EQ(CDD_C_SUCCESS, gnu_is_strict_iso_c89(NULL, &flag));
  ASSERT_EQ(0, flag);
  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
  ASSERT_EQ(CDD_C_SUCCESS, gnu_is_strict_iso_c89(&cfg, &flag));
  ASSERT_EQ(0, flag);
  cfg.target_profile = CDD_TARGET_PROFILE_WATCOM;
  ASSERT_EQ(CDD_C_SUCCESS, gnu_is_strict_iso_c89(&cfg, &flag));
  ASSERT_EQ(1, flag);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("int x = 1;\n"), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(tree));
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_expr(tree));
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_ranges(tree));
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_types(tree));
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_unroll(tree, &cfg));
  {
    uint64_t hi = 0, lo = 0;
    const char *inf_t = NULL;
    char ebuf[32];
    struct gnu_standardizer_ctx_t tmp_gctx;
    cdd_cst_scope_env_t null_env;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_parse_bin_128_literal(NULL, 0, NULL, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_parse_bin_128_literal(NULL, 0, &hi, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_parse_bin_128_literal(NULL, 0, &hi, &lo));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_parse_bin_128_literal("0b1'01ULL", 9, &hi, &lo));
    ASSERT_EQ(5U, (unsigned int)lo);

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_eval_expr_type_scoped(NULL, NULL, NULL, 0, 0, NULL, 0, NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_eval_expr_type_scoped(NULL, NULL, NULL, 0, 0, ebuf,
                                        sizeof(ebuf), &flag));
    memset(&null_env, 0, sizeof(null_env));
    memset(&tmp_gctx, 0, sizeof(tmp_gctx));
    tmp_gctx.sem_env = &null_env;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_eval_expr_type_scoped(
                                 &tmp_gctx, NULL, tree->base_tokens->tokens, 1,
                                 0, ebuf, sizeof(ebuf), &flag));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_infer_type(tree->base_tokens->tokens,
                                            tree->base_tokens->size, &inf_t));
  }
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  tree = NULL;

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("1 + 2"), &tree));
  {
    const char *inf_t = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_infer_type(tree->base_tokens->tokens, 3, &inf_t));
    ASSERT_STR_EQ("int", inf_t);
  }
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
  tree = NULL;

  /* 2. MSVC Legacy golden: rdtsc/cpuid/bswap asm, builtins, void* & int128 */
  {
    const char *p1 =
        "#warning \"check legacy\"\n"
        "typedef int my_i32_t;\n"
        "int g_se = ({ int t = 4; t + 2; });\n"
        "struct WrapU { union { int u_a; long u_b; }; };\n"
        "int msvc_gold(void *vp, const void *cvp, int dyn) {\n"
        "  unsigned int lo = 0, hi = 0, eax = 0, ebx = 0, ecx = 0, edx = 0;\n";
    const char *p2 =
        "  unsigned long long v64 = 0b1000000000000000000000000000000000ULL;\n"
        "  long long s64 = 0b1011LL;\n"
        "  unsigned __int128 big = "
        "0b100000000000000000000000000000000000000000000000000000000000000000;"
        "\n"
        "  __int128 sa = 0, sb = 0;\n"
        "  __asm__ volatile(\"rdtsc\" : \"=a\"(lo), \"=d\"(hi));\n"
        "  __asm__(\"rdtsc\");\n";
    const char *p3 = "  __asm__(\"cpuid\" : \"=a\"(eax), \"=b\"(ebx), "
                     "\"=c\"(ecx), \"=d\"(edx) : \"a\"(1));\n"
                     "  __asm__(\"cpuid\" : : \"a\"(0));\n"
                     "  __asm__(\"bswapl %0\" : \"+r\"(eax));\n"
                     "  __asm__(\"bswapq %0\" : \"+r\"(v64));\n"
                     "  vp++; ++vp; cvp--; --cvp; vp += 4; cvp -= 2;\n"
                     "  vp = 2 + vp;\n";
    const char *p4 = "  sa += sb; sa -= sb; sa *= sb; sa /= sb; sa %= sb;\n"
                     "  sa &= sb; sa |= sb; sa ^= sb; sa <<= 2; big >>= 1;\n"
                     "  sa = -sb; big = ~big;\n"
                     "  sa = (sa * sb) ^ (sa << 1);\n"
                     "  if (sa <= sb || big != big) { ({ lo++; }); }\n";
    const char *p5 = "  int c1 = __builtin_choose_expr(dyn, 10, 20);\n"
                     "  int c2 = __builtin_types_compatible_p(my_i32_t, int);\n"
                     "  int ov = __builtin_sadd_overflow(1, 2, &c1) + "
                     "__builtin_usubll_overflow(4ULL, 1ULL, &v64);\n"
                     "  int bits = __builtin_parity(3) + __builtin_ffs(4) + "
                     "__builtin_clzll(8ULL) + __builtin_ctzll(8ULL) + "
                     "__builtin_popcountll(7ULL) + __builtin_bswap16(258);\n"
                     "  __builtin_prefetch(vp, 0, 3);\n"
                     "  return c1 + c2 + ov + bits + (int)s64;\n}\n";
    char full_src[3072];
    const char *fp = full_src;
    CDD_SNPRINTF(full_src, sizeof(full_src), "%s%s%s%s%s", p1, p2, p3, p4, p5);

    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
    cfg.msvc_ver = 1310;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(fp), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "#pragma message(\"warning: check legacy\")") != NULL);
    ASSERT(strstr(out, "__cdd_stmt_fn_0") != NULL);
    ASSERT(strstr(out, "#pragma warning(disable: 4201)") != NULL);
    ASSERT(strstr(out, "__rdtsc()") != NULL);
    ASSERT(strstr(out, "__cpuid(") != NULL);
    ASSERT(strstr(out, "UI64") != NULL);
    ASSERT(strstr(out, "I64") != NULL);
    ASSERT(strstr(out, "cdd_int128_neg") != NULL);
    ASSERT(strstr(out, "cdd_uint128_not") != NULL);
    ASSERT(strstr(out, "cdd_int128_shl") != NULL);
    ASSERT(strstr(out, "cdd_uint128_shr") != NULL);
    free(out);
    out = NULL;

#ifdef CDD_BUILD_TESTS
    {
      int k;
      for (k = 1; k <= 45; k++) {
        cdd_cst_tree_t *ot = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(fp), &ot));
        g_cdd_alloc_fail = k;
        g_cdd_last_alloc_err = cdd_transform_gnu(ot, &cfg);
        g_cdd_alloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ot));
      }
    }
#endif
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 3. Strict ISO C89 golden: variadic macro call-site expansion, escape char
        ranges, forward computed gotos, and non-MSVC #warning */
  {
    const char *c89_a = "#warning plain_warn\n"
                        "#define LOGA(fmt, args...) call_log(fmt, ##args)\n"
                        "#define LOGB(fmt, ...) call_log(fmt, __VA_ARGS__)\n"
                        "#define LOGR(...) raw_log(__VA_ARGS__)\n"
                        "struct HasAnonU { union { int ua; }; };\n";
    const char *c89_b =
        "int c89_fn(int ch) {\n"
        "  void *lbl = &&fwd_lbl;\n"
        "  LOGA(\"only\");\n"
        "  LOGA(\"two\", 1);\n"
        "  LOGB(\"one\");\n"
        "  LOGB(\"more\", 2, 3);\n"
        "  LOGR(9);\n"
        "  switch (ch) {\n"
        "    case '\\a' ... '\\b': return 1;\n"
        "    case '\\t' ... '\\n': return 2;\n"
        "    case '\\v' ... '\\r': return 3;\n"
        "    case '\\e': case '\\\\': case '\\'': case '\\\"': case '\\?':\n"
        "    case '\\010' ... '\\011': case '\\z': return 4;\n"
        "    default: break;\n"
        "  }\n"
        "  goto *lbl;\n"
        "fwd_lbl:\n"
        "  return 0;\n"
        "}\n";
    char c89_src[1536];
    const char *cp = c89_src;
    CDD_SNPRINTF(c89_src, sizeof(c89_src), "%s%s", c89_a, c89_b);

    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    cfg.target_c89 = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(cp), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "/* #warning plain_warn */") != NULL);
    ASSERT(strstr(out, "call_log(\"only\")") != NULL);
    ASSERT(strstr(out, "call_log(\"two\", 1)") != NULL);
    ASSERT(strstr(out, "raw_log(9)") != NULL);
    ASSERT(strstr(out, "_cdd_anon_") != NULL);
    ASSERT(strstr(out, "__cdd_cgoto_dispatch") != NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  /* 4. Modern MSVC golden */
  {
    const char *mod_src =
        "int mod_fn(char *d, const char *s) {\n"
        "  __builtin_memcpy(d, s, 4);\n"
        "  __builtin_memmove(d, s, 4);\n"
        "  __builtin_memset(d, 0, 4);\n"
        "  if ((int)__builtin_strlen(d) < 0) __builtin_trap();\n"
        "  return (int)__builtin_bswap64(1ULL);\n"
        "}\n";
    memset(&cfg, 0, sizeof(cfg));
    cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(mod_src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "memcpy(d, s, 4)") != NULL);
    ASSERT(strstr(out, "__debugbreak") != NULL);
    free(out);
    out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tree));
    tree = NULL;
  }

  PASS();
}

/**
 * @brief Unit test test_gnu_builtins_expr_types_complete_coverage.
 * @return Greatest test result.
 */
TEST test_gnu_builtins_expr_types_complete_coverage(void) {
  cdd_cst_tree_t *tc_tree = NULL;
  char *tc_out = NULL;
  cdd_transform_config_t tc_cfg;

  /* 1. Null and contextless entry points for builtins & inline asm */
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(NULL, NULL));
  {
    const char *ctx0_src =
        "int ctx0(int x) {\n"
        "  __asm__(\"yield\");\n"
        "  __asm__(\"\" : : : \"memory\");\n"
        "  return __builtin_ffs(x) + __builtin_constant_p(42) +\n"
        "         __builtin_constant_p(x) + (int)(long)__builtin_alloca(4);\n"
        "}\n";
    struct gnu_standardizer_ctx_t empty_gctx;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ctx0_src), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_asm_ctx(tc_tree, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;

    memset(&empty_gctx, 0, sizeof(empty_gctx));
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("int k0 = 0;\n"), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              gnu_standardize_builtins_ctx(tc_tree, &empty_gctx));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;
  }

  /* 2a. Inline ASM variants across MSVC_MODERN, ISO_C89, and Default */
  {
    const char *a1 =
        "int asm_all(void) {\n"
        "  unsigned int lo = 0, hi = 0, ea = 0, eb = 0, ec = 0, ed = 0;\n"
        "  asm volatile(\"\" : : : \"memory\");\n"
        "  __asm __volatile__(\"pause\");\n"
        "  __asm__(\"yield\");\n"
        "  asm(\"rep; nop\");\n"
        "  __asm(\"rep;nop\");\n";
    const char *a2 =
        "  __asm__(\"rdtsc\" : \"=a\"(lo), \"=d\"(hi));\n"
        "  { __asm__(\"rdtsc\" : \"=a\"(lo), \"=d\"(hi)) }\n"
        "  __asm__(\"rdtsc\" : \"=a\"(lo));\n"
        "  __asm__(\"rdtsc\");\n"
        "  __asm__(\"cpuid\" : \"=a\"(ea), \"=b\"(eb), \"=c\"(ec), \"=d\"(ed)"
        " : \"a\"(1) : \"cc\");\n";
    const char *a3 = "  __asm__(\"cpuid\" : \"=a\"(ea) : \"a\"(0));\n"
                     "  __asm__(\"cpuid\" : \"=a\"(ea));\n"
                     "  __asm__(\"bswap %0\" : \"+r\"(ea));\n"
                     "  __asm__(\"cli\");\n"
                     "  { __asm__(\"cli\") }\n"
                     "  __asm__ goto(\"cli\" : : : \"memory\" : end_lbl);\n"
                     "  asm();\n"
                     "end_lbl:\n"
                     "  return (int)(lo + hi + ea + eb + ec + ed);\n}\n";
    char asm_buf[2048];
    const char *asm_p = asm_buf;
    CDD_SNPRINTF(asm_buf, sizeof(asm_buf), "%s%s%s", a1, a2, a3);

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(asm_p), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "_ReadWriteBarrier()") != NULL);
    ASSERT(strstr(tc_out, "_mm_pause()") != NULL);
    ASSERT(strstr(tc_out, "__rdtsc()") != NULL);
    ASSERT(strstr(tc_out, "__cpuid(") != NULL);
    ASSERT(strstr(tc_out, "!defined(_MSC_VER)") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    tc_cfg.target_c89 = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(asm_p), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "((void)0)") != NULL);
    ASSERT(strstr(tc_out, "defined(__GNUC__)") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(asm_p), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "((void)0)") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;
  }

  /* 2b. Complete __builtin_* suite on MSVC_MODERN and ISO_C89 */
  {
    const char *b1 =
        "enum { BI_EC = 3 };\n"
        "typedef long bi_long_t;\n"
        "struct BiS { int a; int f; };\n"
        "int bi_fn(int x, unsigned int u, long sl, unsigned long ul,\n"
        "          long long sll, unsigned long long ull, char *d,\n"
        "          const char *s, size_t n) {\n"
        "  int arr[2] = { 1, 2 };\n"
        "  unsigned short u16 = 7;\n"
        "  int r = 0;\n";
    const char *b2 =
        "  r += __builtin_clz(u) + __builtin_clzl(ul) + __builtin_clzll(ull);\n"
        "  r += __builtin_ctz(u) + __builtin_ctzl(ul) + __builtin_ctzll(ull);\n"
        "  r += __builtin_ffs(x) + __builtin_ffsl(sl) + __builtin_ffsll(sll);\n"
        "  r += __builtin_popcount(u) + __builtin_popcountl(ul) +\n"
        "       __builtin_popcountll(ull);\n";
    const char *b3 =
        "  r += __builtin_parity(u) + __builtin_parityl(ul) +\n"
        "       __builtin_parityll(ull);\n"
        "  r += (int)__builtin_bswap16(u16) + (int)__builtin_bswap32(u) +\n"
        "       (int)__builtin_bswap64(ull);\n"
        "  r += __builtin_sadd_overflow(x, 1, &x) +\n"
        "       __builtin_saddl_overflow(sl, 1L, &sl) +\n"
        "       __builtin_saddll_overflow(sll, 1LL, &sll);\n";
    const char *b4 = "  r += __builtin_uadd_overflow(u, 1U, &u) +\n"
                     "       __builtin_uaddl_overflow(ul, 1UL, &ul) +\n"
                     "       __builtin_uaddll_overflow(ull, 1ULL, &ull);\n"
                     "  r += __builtin_ssub_overflow(x, 1, &x) +\n"
                     "       __builtin_ssubl_overflow(sl, 1L, &sl) +\n"
                     "       __builtin_ssubll_overflow(sll, 1LL, &sll);\n";
    const char *b5 = "  r += __builtin_usub_overflow(u, 1U, &u) +\n"
                     "       __builtin_usubl_overflow(ul, 1UL, &ul) +\n"
                     "       __builtin_usubll_overflow(ull, 1ULL, &ull);\n"
                     "  r += __builtin_smul_overflow(x, 2, &x) +\n"
                     "       __builtin_smull_overflow(sl, 2L, &sl) +\n"
                     "       __builtin_smulll_overflow(sll, 2LL, &sll);\n";
    const char *b6 = "  r += __builtin_umul_overflow(u, 2U, &u) +\n"
                     "       __builtin_umull_overflow(ul, 2UL, &ul) +\n"
                     "       __builtin_umulll_overflow(ull, 2ULL, &ull);\n"
                     "  r += __builtin_add_overflow(x, 1, &x) +\n"
                     "       __builtin_sub_overflow(ul, 1UL, &ul) +\n"
                     "       __builtin_mul_overflow(sll, 2LL, &sll) +\n"
                     "       __builtin_add_overflow(ull, 1ULL, &ull);\n";
    const char *b7 =
        "  if (x == -99) __builtin_unreachable();\n"
        "  if (x == -98) __builtin_trap();\n"
        "  __builtin_prefetch(d);\n"
        "  __builtin_prefetch(d, 0, 2);\n"
        "  r += __builtin_constant_p(42) + __builtin_constant_p(x);\n"
        "  r += __builtin_choose_expr(1, arr[0] + 1, arr[1] + 2);\n"
        "  r += __builtin_choose_expr(0, arr[0], arr[1]);\n"
        "  r += __builtin_choose_expr(x, 10, 20);\n";
    const char *b8 =
        "  r += __builtin_choose_expr(0x1, 3, 4) +\n"
        "       __builtin_choose_expr(0b0, 5, 6) +\n"
        "       __builtin_choose_expr(!0 + -1 + sizeof(int) + 'a' + BI_EC,\n"
        "                             7, 8);\n"
        "  r += __builtin_types_compatible_p(int, int) +\n"
        "       __builtin_types_compatible_p(int, long) +\n"
        "       __builtin_types_compatible_p(long, bi_long_t);\n";
    const char *b9 = "  r += (int)__builtin_offsetof(struct BiS, f);\n"
                     "  if (__builtin_alloca(n) != 0) {\n"
                     "    __builtin_memcpy(d, s, 4);\n"
                     "    __builtin_memset(d, 0, 4);\n"
                     "    __builtin_memmove(d, s, 4);\n"
                     "    r += (int)__builtin_strlen(d);\n"
                     "  }\n"
                     "  return r;\n}\n";
    char bi_buf[4096];
    const char *bi_p = bi_buf;
    CDD_SNPRINTF(bi_buf, sizeof(bi_buf), "%s%s%s%s%s%s%s%s%s", b1, b2, b3, b4,
                 b5, b6, b7, b8, b9);

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(bi_p), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "_alloca") != NULL);
    ASSERT(strstr(tc_out, "__assume(0)") != NULL);
    ASSERT(strstr(tc_out, "__debugbreak()") != NULL);
    ASSERT(strstr(tc_out, "((size_t)&(((struct BiS *)0)->f))") != NULL);
    ASSERT(strstr(tc_out, "__cdd_builtin_smul_overflow") != NULL);
    ASSERT(strstr(tc_out, "__cdd_builtin_umul_overflow") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    tc_cfg.target_c89 = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(bi_p), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "alloca(n)") != NULL);
    ASSERT(strstr(tc_out, "abort()") != NULL);
    ASSERT(strstr(tc_out, "((void)0)") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;
  }

  /* 3. Expressions: fp/const void* arith, full __int128 ops, Elvis, anon,
        and __alignof on MSVC vs ISO_C89 */
  {
    const char *e1 =
        "struct Anon1 { struct { int a1; }; union { int a2; long a3; }; };\n"
        "struct Anon2 { struct { int b1; }; union { char b2; int b3; }; };\n"
        "int g_al_a = (int)__alignof(int);\n"
        "int g_al_b = (int)__alignof__(long);\n"
        "int expr_all(void (*fp)(void), const void *cvp, const void *cvp2,\n"
        "             int x, int y) {\n";
    const char *e2 = "  __int128 sa = 0, sb = 1, sc = 0;\n"
                     "  unsigned __int128 ua = 2, ub = 1, uc = 0;\n"
                     "  long pdiff;\n"
                     "  int cmp = 0, el1, el2, el3;\n"
                     "  fp = fp + 1; fp = 1 + fp; fp = fp - 1;\n"
                     "  cvp = cvp + 1; cvp = 1 + cvp; cvp = cvp - 1;\n"
                     "  pdiff = (long)(cvp - cvp2);\n";
    const char *e3 = "  fp += 2; fp -= 2; cvp += (1 + 1); cvp -= 2;\n"
                     "  fp++; ++fp; fp--; --fp;\n"
                     "  cvp++; ++cvp; cvp--; --cvp;\n"
                     "  sc = sa + sb; sc = sa - sb; sc = sa * sb;\n"
                     "  sc = sa / sb; sc = sa % sb;\n"
                     "  sc = sa & sb; sc = sa | sb; sc = sa ^ sb;\n"
                     "  sc = sa << 2; sc = sa >> 2;\n";
    const char *e4 = "  uc = ua + ub; uc = ua - ub; uc = ua * ub;\n"
                     "  uc = ua / ub; uc = ua % ub;\n"
                     "  uc = ua & ub; uc = ua | ub; uc = ua ^ ub;\n"
                     "  uc = ua << 2; uc = ua >> 2;\n"
                     "  cmp += (sa == sb); cmp += (sa != sb);\n"
                     "  cmp += (sa < sb); cmp += (sa <= sb);\n"
                     "  cmp += (sa > sb); cmp += (sa >= sb);\n";
    const char *e5 = "  cmp += (ua == ub); cmp += (ua != ub);\n"
                     "  cmp += (ua < ub); cmp += (ua <= ub);\n"
                     "  cmp += (ua > ub); cmp += (ua >= ub);\n"
                     "  sa += sb; sa -= sb; sa *= sb; sa /= sb; sa %= sb;\n"
                     "  sa &= sb; sa |= sb; sa ^= sb; sa <<= 2; sa >>= 2;\n"
                     "  ua += ub; ua -= ub; ua *= ub; ua /= ub; ua %= ub;\n"
                     "  ua &= ub; ua |= ub; ua ^= ub; ua <<= 2; ua >>= 2;\n";
    const char *e6 = "  sc = -sa; sc = ~sa; uc = -ua; uc = ~ua;\n"
                     "  if (sa) { cmp++; }\n"
                     "  if (ua) { cmp++; }\n"
                     "  while (sa) { break; }\n"
                     "  while (sa > sb) { break; }\n"
                     "  el1 = --x ?: y;\n"
                     "  el2 = (x = 1) ?: y;\n"
                     "  el3 = ++x ?: y;\n"
                     "  return cmp + el1 + el2 + el3 + (int)pdiff;\n}\n";
    char ex_buf[3584];
    const char *ex_p = ex_buf;
    CDD_SNPRINTF(ex_buf, sizeof(ex_buf), "%s%s%s%s%s%s", e1, e2, e3, e4, e5,
                 e6);

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
    tc_cfg.msvc_ver = 1900;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ex_p), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "__alignof") != NULL);
    ASSERT(strstr(tc_out, "#pragma warning(disable: 4201)") != NULL);
    ASSERT(strstr(tc_out, "const char *") != NULL);
    ASSERT(strstr(tc_out, "const void *") != NULL);
    ASSERT(strstr(tc_out, "cdd_int128_mod") != NULL);
    ASSERT(strstr(tc_out, "cdd_int128_shr") != NULL);
    ASSERT(strstr(tc_out, "cdd_uint128_mod") != NULL);
    ASSERT(strstr(tc_out, "cdd_uint128_shr") != NULL);
    ASSERT(strstr(tc_out, "__cdd_cond_") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    tc_cfg.target_c89 = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ex_p), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "_cdd_anon_") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;
  }

  /* 4a. Attributes & #warning across MSVC_LEGACY, ISO_C89, and Default C11 */
  {
    const char *t1 = "#warning \"quoted msg\"\n"
                     "#warning bare_msg\n"
                     "#  warning   spaced_msg   \n"
                     "__attribute__((unused)) int g_lead;\n"
                     "int g_al_def __attribute__((aligned));\n"
                     "int g_al_16 __attribute((aligned(16)));\n"
                     "int g_sec __attribute__((section(\".mysec\"))) = 1;\n"
                     "int g_wk __attribute__((weak)) = 2;\n"
                     "int g_us __attribute__((used)) = 3;\n";
    const char *t2 =
        "void fn_nr(void) __attribute__((noreturn));\n"
        "void fn_nr2(void) __attribute((noreturn));\n"
        "int fn_ni(int a) __attribute__((noinline)) { return a; }\n"
        "static int fn_ai0(void) __attribute__((always_inline)) { return 0; }\n"
        "static inline int fn_ai1(void) __attribute__((always_inline)) {\n"
        "  return 1;\n}\n";
    const char *t3 =
        "static __inline int fn_ai2(void) __attribute__((always_inline)) {\n"
        "  return 2;\n}\n"
        "static __inline__ int fn_ai3(void) __attribute__((always_inline)) {\n"
        "  return 3;\n}\n"
        "int fn_vd(void) __attribute__((visibility(\"default\"))) {\n"
        "  return 1;\n}\n"
        "int fn_vh(void) __attribute__((visibility(\"hidden\"))) {\n"
        "  return 2;\n}\n";
    const char *t4 =
        "int fn_wur(void) __attribute__((warn_unused_result)) { return 3; }\n"
        "void fn_f1(const char *fmt, ...)\n"
        "    __attribute__((format(printf, 1, 2)));\n"
        "void fn_f2(void (*cb)(int), const char *fmt, ...)\n"
        "    __attribute__((format(printf, 2, 3)));\n"
        "void fn_fs(const char *fmt, ...)\n"
        "    __attribute__((format(scanf, 1, 2)));\n"
        "void init_proto(void) __attribute__((constructor));\n"
        "void fini_proto(void) __attribute__((destructor));\n";
    char ty_buf[3072];
    const char *ty_p = ty_buf;
    CDD_SNPRINTF(ty_buf, sizeof(ty_buf), "%s%s%s%s", t1, t2, t3, t4);

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ty_p), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "#pragma message(\"warning: quoted msg\")") != NULL);
    ASSERT(strstr(tc_out, "#pragma message(\"warning: bare_msg\")") != NULL);
    ASSERT(strstr(tc_out, "__declspec(align(16))") != NULL);
    ASSERT(strstr(tc_out, "__declspec(allocate(\".mysec\"))") != NULL);
    ASSERT(strstr(tc_out, "__declspec(noreturn)") != NULL);
    ASSERT(strstr(tc_out, "__declspec(noinline)") != NULL);
    ASSERT(strstr(tc_out, "__forceinline") != NULL);
    ASSERT(strstr(tc_out, "__declspec(dllexport)") != NULL);
    ASSERT(strstr(tc_out, "_Check_return_") != NULL);
    ASSERT(strstr(tc_out, "_Printf_format_string_") != NULL);
    ASSERT(strstr(tc_out, ".CRT$XCU") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    tc_cfg.target_c89 = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ty_p), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "/* #warning") != NULL);
    ASSERT(strstr(tc_out, "/* aligned(16) */") != NULL);
    ASSERT(strstr(tc_out, "/* noreturn */") != NULL);
    ASSERT(strstr(tc_out, "/* always_inline */") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(ty_p), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "_Alignas(16)") != NULL);
    ASSERT(strstr(tc_out, "_Noreturn") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;
  }

  /* 4b. Binary literals (msvc_ver 1300 vs 1900), C23 typeof (msvc_ver 1940),
         and , ##args variadic macros under target_c89 = 1 */
  {
    const char *bin_src = "unsigned long long b1 = 0B1010'0101ULL;\n"
                          "unsigned long long b2 = 0b11ull;\n"
                          "long long b3 = 0b11ll;\n"
                          "long long b4 = 0B101LL;\n"
                          "int b5 = 0b110;\n";
    const char *tf_src = "int base_v = 7;\n"
                         "typeof(base_v) tv1 = 8;\n"
                         "__typeof__(base_v) tv2 = 9;\n";
    const char *vm_src = "#define VMA(fmt, args...) vm_fn(fmt, ##args)\n"
                         "#define VMB( fmt, rest) vm_fn(fmt, ## rest, 0)\n"
                         "int vm_entry(void) {\n"
                         "  VMA(\"a\");\n"
                         "  VMA(\"b\", 1, 2);\n"
                         "  VMB(\"c\");\n"
                         "  VMB(\"d\", 9);\n"
                         "  return 0;\n}\n";

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
    tc_cfg.msvc_ver = 1300;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(bin_src), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "0xa5UI64") != NULL);
    ASSERT(strstr(tc_out, "0x3UI64") != NULL);
    ASSERT(strstr(tc_out, "0x3I64") != NULL);
    ASSERT(strstr(tc_out, "0x5I64") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    tc_cfg.msvc_ver = 1900;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(bin_src), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "0xa5ULL") != NULL);
    ASSERT(strstr(tc_out, "0x3ull") != NULL);
    ASSERT(strstr(tc_out, "0x3ll") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    tc_cfg.msvc_ver = 1940;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(tf_src), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "typeof(base_v)") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;

    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
    tc_cfg.target_c89 = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(vm_src), &tc_tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(tc_tree, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tc_tree, &tc_out));
    ASSERT(strstr(tc_out, "vm_fn(\"a\")") != NULL);
    ASSERT(strstr(tc_out, "vm_fn(\"b\", 1, 2)") != NULL);
    ASSERT(strstr(tc_out, "vm_fn(\"c\", 0)") != NULL);
    ASSERT(strstr(tc_out, "vm_fn(\"d\", 9, 0)") != NULL);
    free(tc_out);
    tc_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(tc_tree));
    tc_tree = NULL;
  }

  /* 5. OOM / fault-injection sweeps across builtins, exprs, and types */
#ifdef CDD_BUILD_TESTS
  {
    const char *oom_bi1 = "struct OS { int f; };\n"
                          "int obi(int x, unsigned int u, char *p) {\n"
                          "  unsigned int lo = 0, hi = 0;\n"
                          "  __asm__ volatile(\"\" : : : \"memory\");\n"
                          "  __asm__(\"pause\");\n"
                          "  __asm__(\"rdtsc\" : \"=a\"(lo), \"=d\"(hi));\n"
                          "  __asm__(\"cpuid\" : \"=a\"(lo) : \"a\"(0));\n"
                          "  __asm__(\"bswap %0\" : \"+r\"(u));\n"
                          "  __asm__(\"cli\");\n";
    const char *oom_bi2 =
        "  if (x < -9) { __builtin_unreachable(); __builtin_trap(); }\n"
        "  __builtin_prefetch(p);\n"
        "  return __builtin_smul_overflow(x, 2, &x) +\n"
        "         __builtin_add_overflow(x, 1, &x) +\n"
        "         __builtin_choose_expr(1, 3, 4) +\n"
        "         __builtin_constant_p(x) +\n"
        "         __builtin_types_compatible_p(int, int) +\n"
        "         (int)__builtin_offsetof(struct OS, f) +\n"
        "         (int)(long)__builtin_alloca(4);\n}\n";
    const char *oom_ex = "struct OAnon { union { int u; }; };\n"
                         "int oex(void *vp, int x) {\n"
                         "  __int128 sa = 0, sb = 1;\n"
                         "  int r = (--x) ?: 2;\n"
                         "  vp++; ++vp; vp += 1; vp = vp + 1;\n"
                         "  sa += sb; sa = -sb; sa = sa + sb;\n"
                         "  return r + (int)__alignof__(int);\n}\n";
    const char *oom_ty1 =
        "#warning \"oom_w\"\n"
        "#define OVM(f, args...) ofn(f, ##args)\n"
        "int g_ov = 0b101ULL;\n"
        "int g_oa __attribute__((aligned(16)));\n"
        "int g_os __attribute__((section(\".s\"))) = 1;\n"
        "void o_nr(void) __attribute__((noreturn));\n"
        "int o_ni(void) __attribute__((noinline)) { return 0; }\n"
        "static inline int o_ai(void) __attribute__((always_inline)) {\n"
        "  return 1;\n}\n";
    const char *oom_ty2a =
        "#define OVM2(f, args...) printf(f, ##args)\r\n"
        "int o_vd(void) __attribute__((visibility(\"default\"))) {\n"
        "  return 2;\n}\n"
        "int o_wk __attribute__((weak)) = 0;\n"
        "int o_mx __attribute__((mode(XX))) = 0;\n"
        "int o_wr(void) __attribute__((warn_unused_result)) { return 1; }\n"
        "void o_fm(const char *p, ...) __attribute__((format(printf, 1, 2)));\n"
        "void o_ip(void) __attribute__((constructor));\n";
    const char *oom_ty2b =
        "__int128 o_bigb = "
        "0b10000000000000000000000000000000000000000000000000000000000000001;\n"
        "int o_b64 = 0b100000000000000000000000000000000LL;\n"
        "int o_arr[4]; typeof(o_arr) o_arr2;\n"
        "int o_m(void) { while (o_bigb < 0) {} OVM(\"a\", 1); "
        "return __builtin_choose_expr(0b1, 1, 2); }\n";
    char oom_bi_buf[1024];
    char oom_ty_buf[1536];
    const char *oom_cases[4];
    int oom_c89[4];
    size_t ci;
    int fi;

    CDD_SNPRINTF(oom_bi_buf, sizeof(oom_bi_buf), "%s%s", oom_bi1, oom_bi2);
    CDD_SNPRINTF(oom_ty_buf, sizeof(oom_ty_buf), "%s%s%s", oom_ty1, oom_ty2a,
                 oom_ty2b);
    oom_cases[0] = oom_bi_buf;
    oom_c89[0] = 0;
    oom_cases[1] = oom_ex;
    oom_c89[1] = 0;
    oom_cases[2] = oom_ty_buf;
    oom_c89[2] = 0;
    oom_cases[3] = oom_ty_buf;
    oom_c89[3] = 1;

    for (ci = 0; ci < 4; ci++) {
      memset(&tc_cfg, 0, sizeof(tc_cfg));
      if (oom_c89[ci]) {
        tc_cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
        tc_cfg.target_c89 = 1;
      } else {
        tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
        tc_cfg.msvc_ver = 1300;
      }
      for (fi = 1; fi <= 30; fi++) {
        cdd_cst_tree_t *ot = NULL;

        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &ot));
        g_gnu_alloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(ot, &tc_cfg);
        g_gnu_alloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ot));

        ot = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &ot));
        g_gnu_replace_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(ot, &tc_cfg);
        g_gnu_replace_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ot));

        ot = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &ot));
        g_gnu_bld_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(ot, &tc_cfg);
        g_gnu_bld_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ot));

        ot = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &ot));
        g_cdd_alloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(ot, &tc_cfg);
        g_cdd_alloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ot));

        ot = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &ot));
        g_cdd_strdup_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(ot, &tc_cfg);
        g_cdd_strdup_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ot));

        ot = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &ot));
        g_cdd_cst_alloc_token_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(ot, &tc_cfg);
        g_cdd_cst_alloc_token_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ot));

        ot = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &ot));
        g_cdd_cst_realloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(ot, &tc_cfg);
        g_cdd_cst_realloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ot));

        ot = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &ot));
        g_cdd_cst_alloc_node_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(ot, &tc_cfg);
        g_cdd_cst_alloc_node_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(ot));
      }
    }
  }
#endif

  /* Focused edge cases and micro-OOM sweeps for builtins, expr, and types */
  {
    char long_asm[512];
    char pad_a[270];
    const char *micro_cases[] = {
        "void f(int n, int *p) { __builtin_unreachable( /* c */  ); "
        "__asm__(\"cli\"); __builtin_alloca(n); "
        "__builtin_add_overflow(1, 2, p); }\n",
        "void f(__int128 a, __int128 b) { __int128 c = ~a; "
        "for (; (~a), (a + b), 0; ) {} }\n",
        "const char *g_esc = \"\\e\"; "
        "__inline__ int f_ai(void) __attribute__((always_inline, deprecated)) "
        "{ return 0; }\n",
        "#define MV(f, a...) fn(f, ##a)\nvoid h(void) { MV(1, 2); }\n",
        "void f_vptr(void **pp, void *arr[2]) { "
        "*pp += (__extension__ 4); (__extension__ arr[0]) += 2; "
        "if ((*pp += 1)) {} }\n",
        "__attribute__((aligned(8))) int g_lead_al;\n"
        "void f_p_attr(int x __attribute__((unused)));\n"
        "int g_70 "
        "__attribute__((__custom_attribute_name_that_definitely_exceeds_sixty_"
        "four_characters_limit__));\n"
        "__attribute__((format(printf, 1, 2))) int g_fmt_lead;\n",
        "void f_fmt_hi(const char *s) __attribute__((format(printf, 5, 6)));\n"
        "void f_fmt_w(const char *s, ...) "
        "__attribute__((format(printf, 00000000000000001, 2)));\n"
        "int g_ctor_var __attribute__((constructor));\n"
        "int g_b_sfx = 0b1010uuuuuuuuuuuuuuuuuu;\n"
        "extern int ga_e, gb_e; int g_elv_top = (ga_e + gb_e) ?: 3;\n"
        "void f_elv_nest(int a, int b) { for (; (a + b) ?: 0; ) {} "
        "if ((__auto_type x = 1)) {} }\n"};
    size_t mi;
    int fi;
    cdd_cst_tree_t *mt = NULL;

    memset(pad_a, 'x', 262);
    pad_a[262] = '\0';
    CDD_SNPRINTF(long_asm, sizeof(long_asm),
                 "void f(void) { __asm__ goto (\"%s\" : : : : lbl); lbl: ; }\n",
                 pad_a);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(long_asm), &mt));
    memset(&tc_cfg, 0, sizeof(tc_cfg));
    tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(mt, &tc_cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(mt));

    /* Direct gnu_standardize_builtins for 0b1, empty choose_expr arg, wiped tok
     */
    mt = NULL;
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str(
                          "int x = __builtin_choose_expr(0 + 1, 2 + 3, 4) + "
                          "__builtin_choose_expr(0b1, 1, 2) + "
                          "__builtin_choose_expr(, 3, 4);\n"),
                      &mt));
    mt->base_tokens->tokens[6].length = 0;
    mt->base_tokens->tokens[10].length = 0;
    ASSERT_EQ(CDD_C_SUCCESS, gnu_standardize_builtins(mt));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(mt));

    /* Empty root with unclosed call, unclosed struct {, and msvc_ver=1940 OOM
     */
    {
      cdd_cst_tree_t empty_tr;
      cdd_cst_node_t dummy_root;
      struct gnu_standardizer_ctx_t empty_gctx;
      cdd_transform_config_t iso_c89_cfg;
      memset(&empty_tr, 0, sizeof(empty_tr));
      memset(&dummy_root, 0, sizeof(dummy_root));
      memset(&empty_gctx, 0, sizeof(empty_gctx));
      memset(&iso_c89_cfg, 0, sizeof(iso_c89_cfg));
      iso_c89_cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
      iso_c89_cfg.target_c89 = 1;
      empty_tr.root = &dummy_root;
      empty_gctx.tree = &empty_tr;
      empty_gctx.config = &iso_c89_cfg;
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_lexer_tokenize(az_span_create_from_str(
                                 "struct { int a; __builtin_add_overflow(1"),
                             &empty_tr.base_tokens));
      g_cdd_last_alloc_err =
          gnu_standardize_builtins_ctx(&empty_tr, &empty_gctx);
      g_cdd_last_alloc_err = gnu_standardize_expr_ctx(&empty_tr, &empty_gctx);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_lexer_free_token_list(empty_tr.base_tokens));

      mt = NULL;
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_cst_parse(az_span_create_from_str("__typeof__(int) y;\n"), &mt));
      memset(&tc_cfg, 0, sizeof(tc_cfg));
      tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
      tc_cfg.msvc_ver = 1940;
      g_gnu_replace_fail = 1;
      g_cdd_last_alloc_err = cdd_transform_gnu(mt, &tc_cfg);
      g_gnu_replace_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(mt));

      mt = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(
                    az_span_create_from_str("void f(__int128 a, __int128 b) { "
                                            "a = a + (0 + b); }\n"),
                    &mt));
      mt->base_tokens->tokens[16].length = 0;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(mt, &tc_cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(mt));
    }

    for (mi = 0; mi < sizeof(micro_cases) / sizeof(micro_cases[0]); mi++) {
      memset(&tc_cfg, 0, sizeof(tc_cfg));
      if (mi == 3) {
        tc_cfg.target_profile = CDD_TARGET_PROFILE_ISO_C89;
        tc_cfg.target_c89 = 1;
      } else {
        tc_cfg.target_profile = CDD_TARGET_PROFILE_MSVC_LEGACY;
        tc_cfg.msvc_ver = 1300;
      }
      mt = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(micro_cases[mi]), &mt));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_gnu(mt, &tc_cfg));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(mt));

#ifdef CDD_BUILD_TESTS
      for (fi = 1; fi <= 25; fi++) {
        mt = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(micro_cases[mi]), &mt));
        g_gnu_replace_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(mt, &tc_cfg);
        g_gnu_replace_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(mt));

        mt = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(micro_cases[mi]), &mt));
        g_gnu_malloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(mt, &tc_cfg);
        g_gnu_malloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(mt));

        mt = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(micro_cases[mi]), &mt));
        g_cdd_alloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(mt, &tc_cfg);
        g_cdd_alloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(mt));

        mt = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(micro_cases[mi]), &mt));
        g_cdd_cst_alloc_token_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(mt, &tc_cfg);
        g_cdd_cst_alloc_token_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(mt));

        mt = NULL;
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(micro_cases[mi]), &mt));
        g_cdd_cst_realloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_gnu(mt, &tc_cfg);
        g_cdd_cst_realloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_tree_free(mt));
      }
#endif
    }
  }

  PASS();
}

/**
 * @brief Executes the transformer_gnu_standardizer_suite test suite.
 */
SUITE(transformer_gnu_standardizer_suite) {
  RUN_TEST(test_cdd_transform_gnu);
  RUN_TEST(test_gnu_standardizer_stmt_expr);
  RUN_TEST(test_gnu_standardizer_computed_goto);
  RUN_TEST(test_gnu_standardizer_case_ranges);
  RUN_TEST(test_gnu_standardizer_range_init);
  RUN_TEST(test_gnu_standardizer_local_labels);
  RUN_TEST(test_gnu_standardizer_vla_malloc);
  RUN_TEST(test_gnu_standardizer_vla_multidim);
  RUN_TEST(test_gnu_standardizer_trailing_comma);
  RUN_TEST(test_gnu_standardizer_zero_length_array);
  RUN_TEST(test_gnu_standardizer_empty_initializer);
  RUN_TEST(test_gnu_standardizer_128_bit_literals);
  RUN_TEST(test_gnu_standardizer_vla_params);
  RUN_TEST(test_gnu_standardizer_return_void_expr);
  RUN_TEST(test_gnu_standardizer_overlapping_case_ranges);
  RUN_TEST(test_gnu_standardizer_empty_fallthrough_block);
  RUN_TEST(test_gnu_standardizer_attributes);
  RUN_TEST(test_gnu_standardizer_magic_identifiers);
  RUN_TEST(test_gnu_standardizer_trampoline);
  RUN_TEST(test_gnu_standardizer_cleanup);
  RUN_TEST(test_gnu_standardizer_shuffle);
  RUN_TEST(test_gnu_standardizer_typeof);
  RUN_TEST(test_gnu_standardizer_variadic_macros);
  RUN_TEST(test_cdd_transform_complex_numbers);
  RUN_TEST(test_gnu_standardizer_comment_preservation);
  RUN_TEST(test_gnu_standardizer_float_extensions);
  RUN_TEST(test_gnu_standardizer_lvalue_cast);
  RUN_TEST(test_gnu_standardizer_omitted_conditional);
  RUN_TEST(test_gnu_standardizer_semantic_include_and_typeof);
  RUN_TEST(test_gnu_standardizer_cleanup_return_break_continue);
  RUN_TEST(test_gnu_standardizer_elvis_typed_hoist);
  RUN_TEST(test_gnu_standardizer_msvc_anon_struct_union);
  RUN_TEST(test_gnu_standardizer_msvc_attributes_unified);
  RUN_TEST(test_gnu_standardizer_extended_ranges);
  RUN_TEST(test_gnu_standardizer_builtins_and_asm);
  RUN_TEST(test_gnu_standardizer_nested_functions_lift);
  RUN_TEST(test_gnu_standardizer_lexical_vptr_and_int128_ops);
  RUN_TEST(test_gnu_standardizer_golden_legacy_modern_and_oom);
  RUN_TEST(test_gnu_builtins_expr_types_complete_coverage);
}
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_GNU_STANDARDIZER_H */
