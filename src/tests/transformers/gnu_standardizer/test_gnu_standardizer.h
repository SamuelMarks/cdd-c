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
#include "c_str_span.h"
/* clang-format on */

extern C_CDD_EXPORT int g_gnu_standardizer_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;

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
  int rc;
  size_t i;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
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
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_stmt_expr(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n"
                     "  int a = ({ int b = 1; b; });\n"
                     "  return a;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "int a =  int b = 1; b;") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_computed_goto(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  void *ptr = &&my_label;\n  goto "
                     "*ptr;\nmy_label:\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "goto *ptr;") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_case_ranges(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "int main() {\n  int x = 3;\n  switch(x) {\n  case 1 ... 5:\n    return "
      "1;\n  case -5 ... -1:\n    return 2;\n  }\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "case 1: case 2: case 3: case 4: case 5") != NULL);
  ASSERT(strstr(out, "case -5: case -4: case -3: case -2: case -1") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_range_init(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      (char *)(size_t)(size_t) "int main() {\n  int arr[10] = { [2 ... "
                               "5] = 1 };\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "[2] = 1, [3] = 1, [4] = 1, [5] = 1") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* The __label__ declaration should be gone, and labels renamed */
  ASSERT(strstr(out, "__label__") == NULL);
  ASSERT(strstr(out, "__cdd_ll_my_label_1") != NULL);
  ASSERT(strstr(out, "__cdd_ll_other_label_2") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
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
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "malloc") != NULL);
  ASSERT(strstr(out, "free(arr)") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_trailing_comma(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      (char *)(size_t)(size_t) "enum X { A, B, C, }; int arr[] = { 1, 2, 3, };";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "A, B, C  }") != NULL || strstr(out, "A, B, C }") != NULL);
  ASSERT(strstr(out, "1, 2, 3  }") != NULL || strstr(out, "1, 2, 3 }") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_zero_length_array(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "struct flexible {\n"
                     "  int length;\n"
                     "  int data[0];\n"
                     "};\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "int data[1];") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_empty_initializer(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "struct foo { int a; int b[0]; };\n"
                     "struct foo f = { 1, { } };\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "{ 1, {") != NULL);
  ASSERT(strstr(out, "0 } }") != NULL || strstr(out, "0} }") != NULL ||
         strstr(out, "0 }") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
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
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_vla_params(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      (char *)(size_t)(size_t) "void func(int n, int arr[n]) {}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* Should strip 'n' */
  ASSERT(strstr(out, "void func(int n, int arr[])") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_return_void_expr(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      (char *)(size_t)(size_t) "void func() { return (void)0; }\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "(void)0; return;") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_overlapping_case_ranges(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "int main() {\n  int x = 3;\n  switch(x) {\n  case 1 ... 5:\n    return "
      "1;\n  case 4 ... 8:\n    return 2;\n  }\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* We just test it generates properly, the compiler will complain. */
  ASSERT(strstr(out, "case 1: case 2: case 3: case 4: case 5") != NULL);
  ASSERT(strstr(out, "case 4: case 5: case 6: case 7: case 8") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_empty_fallthrough_block(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  int x = 1;\n  switch(x) {\n  case 1:\n  "
                     "{}\n  case 2:\n    return 2;\n  }\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* Should rewrite {} after case label to ; */
  ASSERT(
      strstr(out, "case 1: ;") != NULL || strstr(out, "case 1:\n  ;") != NULL ||
      strstr(out, "case 1: ;") != NULL || strstr(out, "case 1:\n   ;") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_attributes(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int __attribute__((vector_size(16))) v;\n"
                     "int __attribute__((aligned(16))) a;\n"
                     "int __attribute__((mode(QI))) m;\n"
                     "int __attribute__((pure)) p;\n"
                     "int __attribute__((nonnull(1))) q;\n";
  cdd_transform_config_t config;
  char *out = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* Should strip vector_size, change aligned to _Alignas, and polyfill mode */
  ASSERT(strstr(out, "vector_size") == NULL);
  ASSERT(strstr(out, "_Alignas(16)") != NULL);
  ASSERT(strstr(out, "/* mode(QI) -> int8_t */") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_magic_identifiers(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void my_super_func() {\n"
                     "  const char *a = __FUNCTION__;\n"
                     "  const char *b = __PRETTY_FUNCTION__;\n"
                     "  const char *c = __func__;\n"
                     "}\n";
  cdd_transform_config_t config;
  char *out = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "\"my_super_func\"") != NULL);
  ASSERT(strstr(out, "__FUNCTION__") == NULL);
  ASSERT(strstr(out, "__PRETTY_FUNCTION__") == NULL);
  ASSERT(strstr(out, "__func__") == NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_trampoline(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void foo() {\n"
                     "  void bar() {}\n"
                     "  void (*p)() = bar;\n"
                     "}\n";
  cdd_transform_config_t config;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, cdd_transform_gnu(tree, &config));

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_shuffle(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void foo() {\n"
                     "  __builtin_shuffle(a, b, mask);\n"
                     "  __builtin_shufflevector(a, b, 0, 1);\n"
                     "}\n";
  cdd_transform_config_t config;
  char *out = NULL;

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "cdd_builtin_shuffle(a, b, mask)") != NULL);
  ASSERT(strstr(out, "cdd_builtin_shufflevector(a, b, 0, 1)") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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

  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));

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
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
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
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_variadic_macros(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#define LOG1(fmt, args...) printf(fmt, ##args)\n"
                     "#define LOG2(fmt, ...) printf(fmt, ##__VA_ARGS__)\n"
                     "#define LOG3(args...) printf(args)\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  printf("MACRO OUTPUT: %s\n", out);

  ASSERT(
      strstr(out,
             "#define LOG1(fmt, ...) printf(fmt __VA_OPT__(,) __VA_ARGS__)") !=
      NULL);
  ASSERT(
      strstr(out,
             "#define LOG2(fmt, ...) printf(fmt __VA_OPT__(,) __VA_ARGS__)") !=
      NULL);
  ASSERT(strstr(out, "#define LOG3(...) printf(__VA_ARGS__)") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_transform_complex_numbers(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void calc() {\n"
                     "  __complex__ float z;\n"
                     "  __real__ z = 1.0f;\n"
                     "  __imag__ z = 2.0f;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t cfg = {0};
  int rc;

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
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
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "/* block start */") != NULL);
  ASSERT(strstr(out, "// inline comment") != NULL);
  ASSERT(strstr(out, "/* mid-expr */") != NULL);
  ASSERT(strstr(out, "/* vla decl */") != NULL);
  ASSERT(strstr(out, "/* block end */") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
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
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gnu_standardizer_lvalue_cast(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      (char *)(size_t)(size_t) "void f() { int x; (char)x = 5; }\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  printf("LVALUE OUT: %s\n", out);
  ASSERT(strstr(out, "*(char*)&x = 5;") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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
  ASSERT_EQ(0,
            cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                          &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "a ? a  : b") != NULL);
  ASSERT(strstr(out, "(a + b) ? (a + b)  : 5") != NULL);
  ASSERT(strstr(out, "a || b ? a || b  : c") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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

#endif /* TEST_CDD_TRANSFORM_GNU_STANDARDIZER_H */
