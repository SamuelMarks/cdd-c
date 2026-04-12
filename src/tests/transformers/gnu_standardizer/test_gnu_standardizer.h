#ifndef TEST_CDD_TRANSFORM_GNU_STANDARDIZER_H
#define TEST_CDD_TRANSFORM_GNU_STANDARDIZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/emit/cdd_cst_emit.h"
#include "c_str_span.h"
/* clang-format on */

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
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_gnu(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "/* unused */") != NULL);
  ASSERT(strstr(out, "_Noreturn") != NULL);
  ASSERT(strstr(out, "pack(push, 1)") != NULL);
  ASSERT(strstr(out, "pack(pop)") != NULL);
  ASSERT(strstr(out, "alloca((a) * sizeof(*arr))") != NULL);
  ASSERT(strstr(out, "_Alignof") != NULL);
  ASSERT(strstr(out, "char _pad;") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
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
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "int a =  int b = 1; b;") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_computed_goto(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  void *ptr = &&my_label;\n  goto "
                     "*ptr;\nmy_label:\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "goto *ptr;") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
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
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "case 1: case 2: case 3: case 4: case 5") != NULL);
  ASSERT(strstr(out, "case -5: case -4: case -3: case -2: case -1") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_range_init(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "int main() {\n  int arr[10] = { [2 ... 5] = 1 };\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "[2] = 1, [3] = 1, [4] = 1, [5] = 1") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_local_labels(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n"
                     "  {\n"
                     "    __label__ my_label;\n"
                     "    goto my_label;\n"
                     "my_label:\n"
                     "    return 1;\n"
                     "  }\n"
                     "  return 0;\n"
                     "}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* The __label__ declaration should be gone, and my_label renamed to something
   * like __cdd_ll_my_label_1 */
  ASSERT(strstr(out, "__label__") == NULL);
  ASSERT(strstr(out, "__cdd_ll_my_label_1") != NULL);
  ASSERT(strstr(out, "goto __cdd_ll_my_label_1") != NULL);
  ASSERT(strstr(out, "__cdd_ll_my_label_1:") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
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
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "int *arr = malloc((n) * sizeof(*arr))") != NULL);
  ASSERT(strstr(out, "free(arr)") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
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
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "int *arr = malloc((x * y * z) * sizeof(*arr))") != NULL);
  ASSERT(strstr(out, "free(arr)") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_trailing_comma(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "enum X { A, B, C, }; int arr[] = { 1, 2, 3, };";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "A, B, C  }") != NULL || strstr(out, "A, B, C }") != NULL);
  ASSERT(strstr(out, "1, 2, 3  }") != NULL || strstr(out, "1, 2, 3 }") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
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
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "int data[1];") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_empty_initializer(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "struct foo { int a; int b[0]; };\n"
                     "struct foo f = { 1, { } };\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "{ 1, {") != NULL);
  ASSERT(strstr(out, "0 } }") != NULL || strstr(out, "0} }") != NULL ||
         strstr(out, "0 }") != NULL);
  free(out);
  cdd_cst_tree_free(tree);
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
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* 18446744073709551616 is 2^64, so high=1, low=0 */
  ASSERT(strstr(out, "cdd_make_uint128(0x1ULL, 0x0ULL)") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_vla_params(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void func(int n, int arr[n]) {}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* Should strip 'n' */
  ASSERT(strstr(out, "void func(int n, int arr[])") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_return_void_expr(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void func() { return (void)0; }\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "(void)0; return;") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
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
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* We just test it generates properly, the compiler will complain. */
  ASSERT(strstr(out, "case 1: case 2: case 3: case 4: case 5") != NULL);
  ASSERT(strstr(out, "case 4: case 5: case 6: case 7: case 8") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_empty_fallthrough_block(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  int x = 1;\n  switch(x) {\n  case 1:\n  "
                     "{}\n  case 2:\n    return 2;\n  }\n  return 0;\n}\n";
  char *out = NULL;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* Should rewrite {} after case label to ; */
  ASSERT(
      strstr(out, "case 1: ;") != NULL || strstr(out, "case 1:\n  ;") != NULL ||
      strstr(out, "case 1: ;") != NULL || strstr(out, "case 1:\n   ;") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_attributes(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int __attribute__((vector_size(16))) v;\n"
                     "int __attribute__((aligned(16))) a;\n"
                     "int __attribute__((mode(QI))) m;\n";
  cdd_transform_config_t config;
  char *out = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  /* Should strip vector_size, change aligned to _Alignas, and polyfill mode */
  ASSERT(strstr(out, "vector_size") == NULL);
  ASSERT(strstr(out, "_Alignas(16)") != NULL);
  ASSERT(strstr(out, "/* mode(QI) -> int8_t */") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
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

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "\"my_super_func\"") != NULL);
  ASSERT(strstr(out, "__FUNCTION__") == NULL);
  ASSERT(strstr(out, "__PRETTY_FUNCTION__") == NULL);
  ASSERT(strstr(out, "__func__") == NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_gnu_standardizer_trampoline(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void foo() {\n"
                     "  void bar() {}\n"
                     "  void (*p)() = bar;\n"
                     "}\n";
  cdd_transform_config_t config;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(129, cdd_transform_gnu(tree, &config));

  cdd_cst_tree_free(tree);
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

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));

  memset(&config, 0, sizeof(config));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "cdd_builtin_shuffle(a, b, mask)") != NULL);
  if (strstr(out, "cdd_builtin_shufflevector(a, b, 0, 1)") == NULL) {
    printf("OUT WAS: %s\n", out);
  }
  ASSERT(strstr(out, "cdd_builtin_shufflevector(a, b, 0, 1)") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
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

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));

  memset(&config, 0, sizeof(config));
  {
    int rc = cdd_transform_gnu(tree, &config);
    if (rc != 0) {
      printf("ERROR CODE: %d\n", rc);
    }
    ASSERT_EQ(0, rc);
  }

  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  if (strstr(out, "my_free(&x);") == NULL) {
    FILE *f = fopen("DEBUG_OUT.txt", "wb");
    fwrite(out, 1, strlen(out), f);
    fclose(f);
  }
  ASSERT(strstr(out, "my_free(&x);") != NULL);
  ASSERT(strstr(out, "goto cross-scope cleanups unsupported") != NULL);
  ASSERT(strstr(out, "return __cdd_ret;") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
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
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
  ASSERT_EQ(0, cdd_transform_gnu(tree, &config));
  ASSERT_EQ(0, cdd_cst_emit(tree, &out));

  ASSERT(strstr(out, "int a = 1;") != NULL);
  ASSERT(strstr(out, "char b = 'A';") != NULL);
  ASSERT(strstr(out, "float c = 1.0f;") != NULL);
  ASSERT(strstr(out, "double d = 1.0;") != NULL);
  ASSERT(strstr(out, "int e = 2;") != NULL);
  ASSERT(strstr(out, "typedef int __cdd_typeof_arr_") != NULL);
  ASSERT(strstr(out, "[5]; __cdd_typeof_arr_") != NULL);
  ASSERT(strstr(out, " arr = {1, 2, 3, 4, 5};") != NULL);
  ASSERT(strstr(out, "int x = 0;") != NULL);
  ASSERT(strstr(out, "int y = 0;") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
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
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str((char *)code), &tree));
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

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_gnu(tree, &cfg);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  printf("COMPLEX MACRO OUT: %s\n", out);

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
}
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_GNU_STANDARDIZER_H */