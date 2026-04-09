#ifndef TEST_CDD_CST_TRANSFORM_H
#define TEST_CDD_CST_TRANSFORM_H

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

TEST test_cdd_transform_extern_c(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "/* license */\n#include <stdio.h>\n\nint main() {\n  return 0;\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "extern \"C\" {") != NULL);
  ASSERT(strstr(out, "}") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_extern_c_already_exists(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "#ifdef __cplusplus\nextern \"C\" {\n#endif\nint "
                     "main(){}\n#ifdef __cplusplus\n}\n#endif\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_extern_c(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_msvc(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "#include <unistd.h>\n#include <sys/time.h>\nint "
      "main() {\n  strcasecmp /* comment */ (\"a\", \"b\");\n  "
      "strncasecmp(\"a\", \"b\", 1);\n  strdup(\"a\");\n  ssize_t s = "
      "0;\n  __builtin_expect(1, 1);\n  return 0;\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_msvc(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "_stricmp /* comment */") != NULL);
  ASSERT(strstr(out, "_strnicmp") != NULL);
  ASSERT(strstr(out, "_strdup") != NULL);
  ASSERT(strstr(out, "SSIZE_T") != NULL);
  ASSERT(strstr(out, "cdd_builtin_expect") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

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
  cdd_transform_config_t config = {0, 2};

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
  ASSERT(strstr(out, "alloca(a * sizeof(*arr))") != NULL);
  ASSERT(strstr(out, "_Alignof") != NULL);
  ASSERT(strstr(out, "char _pad;") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_percolate_errors(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "void foo() {\n  int *ptr = malloc(sizeof(int));\n  return;\n}\nvoid "
      "bar(int a) {\n  void *b = calloc(1, 1);\n  foo();\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_percolate_errors(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "int foo(void **out_result)") != NULL);
  ASSERT(strstr(out, "if (!ptr) { return ENOMEM; }") != NULL);
  ASSERT(strstr(out, "rc = 0; goto cleanup;") != NULL);
  ASSERT(strstr(out, "cleanup:") != NULL);
  ASSERT(strstr(out, "return rc;") != NULL);
  ASSERT(strstr(out, "int bar(int a, void **out_result)") != NULL);
  ASSERT(strstr(out, "if (!b) { return ENOMEM; }") != NULL);
  ASSERT(strstr(out, "if (foo(&out_foo) != 0)") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_percolate_errors_complex(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "void complex_allocs() {\n"
                     "  int *p1 = malloc(1);\n"
                     "  int *p2 = malloc(2);\n"
                     "  return;\n"
                     "}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_percolate_errors(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "if (!p1) { return ENOMEM; }") != NULL);
  ASSERT(strstr(out, "if (!p2) { rc = ENOMEM; goto cleanup; }") != NULL);
  ASSERT(strstr(out, "rc = 0; goto cleanup;") != NULL);
  ASSERT(strstr(out, "cleanup:") != NULL);
  ASSERT(strstr(out, "return rc;") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_safe_crt(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "void foo() {\n"
      "  char dest[10];\n"
      "  strcpy(dest, \"abc\");\n"
      "  strncpy(dest, \"def\", 3);\n"
      "  sprintf(dest, \"%d\", 1);\n"
      "  FILE *f;\n"
      "  f = fopen(\"a.txt\", \"r\");\n"
      "  strcat(dest, \"x\");\n"
      "  strncat(dest, \"y\", 1);\n"
      "  memcpy(dest, \"z\", 1);\n"
      "  memmove(dest, \"w\", 1);\n"
      "}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_safe_crt(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "strcpy_s(dest, sizeof(dest), \"abc\");") != NULL);
  ASSERT(strstr(out, "strncpy_s(dest, 3 + 1, \"def\", 3);") != NULL);
  ASSERT(strstr(out, "sprintf_s(dest, sizeof(dest), \"%d\", 1);") != NULL);
  ASSERT(strstr(out, "fopen_s(&f") != NULL);
  ASSERT(strstr(out, "strcat_s(dest, sizeof(dest), \"x\");") != NULL);
  ASSERT(strstr(out, "strncat_s(dest, 1 + 1, \"y\", 1);") != NULL);
  ASSERT(strstr(out, "memcpy_s(dest, 1, \"z\", 1);") != NULL);
  ASSERT(strstr(out, "memmove_s(dest, 1, \"w\", 1);") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

SUITE(cdd_cst_transform_suite) {
  RUN_TEST(test_cdd_transform_extern_c);
  RUN_TEST(test_cdd_transform_extern_c_already_exists);
  RUN_TEST(test_cdd_transform_msvc);
  RUN_TEST(test_cdd_transform_gnu);
  RUN_TEST(test_cdd_transform_percolate_errors);
  RUN_TEST(test_cdd_transform_percolate_errors_complex);
  RUN_TEST(test_cdd_transform_safe_crt);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_TRANSFORM_H */
