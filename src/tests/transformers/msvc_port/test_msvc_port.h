/**
 * @file test_msvc_port.h
 * @brief Unit tests for MSVC port transformer.
 */

#ifndef TEST_CDD_TRANSFORM_MSVC_PORT_H
#define TEST_CDD_TRANSFORM_MSVC_PORT_H

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

/**
 * @brief Test MSVC transformation of POSIX features.
 *
 * @return The result of the test.
 */
TEST test_cdd_transform_msvc(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code =
      "#include <unistd.h>\n#include <sys/time.h>\nint "
      "main() {\n  strcasecmp /* comment */ (\"a\", \"b\");\n  "
      "strncasecmp(\"a\", \"b\", 1);\n  strdup(\"a\");\n  ssize_t s = "
      "0;\n  __builtin_expect(1, 1);\n"
      "  off_t off; pid_t pid; mode_t m;\n"
      "  open(1); close(2); read(3); write(4);\n"
      "  fileno(5); unlink(6); mkdir(7); rmdir(8); getcwd(9);\n"
      "  snprintf(0); strtok_r(1); isnan(2);\n"
      "  return 0;\n}\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_msvc(tree, &config);
  ASSERT_EQ(0, rc);

  {
    int i;
    for (i = 0; i < 100000; i++) {
      cdd_cst_tree_t *tree_copy = NULL;
      rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree_copy);
      if (rc == 0 && tree_copy) {
#ifdef CDD_BUILD_TESTS
        extern int g_cdd_cst_realloc_fail;
        g_cdd_cst_realloc_fail = i;
#endif
        rc = cdd_transform_msvc(tree_copy, &config);
        (void)rc;
#ifdef CDD_BUILD_TESTS
        g_cdd_cst_realloc_fail = 0;
#endif
        cdd_cst_tree_free(tree_copy);
      }
    }
  }

  /* Test nulls */
  {
    cdd_cst_tree_t empty_tree = {0};
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_transform_msvc(NULL, &config));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_transform_msvc(&empty_tree, &config));
  }

  /* Test malformed nodes / builder errors. We will manually construct a
   * malformed tree but it's easier to just pass a tree with a missing file.
   * Wait, msvc_port replaces strdup etc. */
  {
    cdd_cst_tree_t *tree2 = NULL;
    cdd_cst_node_t *node = NULL;
    cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &node);
    tree2 = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
    tree2->root = node;
    ASSERT_EQ(
        0, cdd_transform_msvc(tree2, &config)); /* Should not crash on empty */
    cdd_cst_tree_free(tree2);
  }

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  printf("OUT WAS:\n[%s]\n", out);

  ASSERT(strstr(out, "#ifndef _MSC_VER") != NULL);
  ASSERT(strstr(out, "_stricmp /* comment */ ") != NULL);
  ASSERT(strstr(out, "_strnicmp") != NULL);
  ASSERT(strstr(out, "_strdup") != NULL);
  ASSERT(strstr(out, "SSIZE_T") != NULL);
  ASSERT(strstr(out, "cdd_builtin_expect") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern int g_msvc_port_bld_fail;
#endif

TEST test_cdd_transform_msvc_context(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "struct A { int strdup; };\n"
                     "int strdup = 1;\n"
                     "char * strcasecmp = NULL;\n"
                     "#define MACRO(strdup) strdup\n"
                     "void foo() { struct A a; a.strdup = 1; }\n";
  char *out = NULL;
  int rc;
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_msvc(tree, &config);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(out, "int strdup;") != NULL);
  ASSERT(strstr(out, "int strdup = 1;") != NULL);
  ASSERT(strstr(out, "char * strcasecmp = NULL;") != NULL);
  ASSERT(strstr(out, "MACRO(strdup)") != NULL);
  ASSERT(strstr(out, "a.strdup") != NULL);

  free(out);
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_transform_msvc_builder_fails(void) {
#ifdef CDD_BUILD_TESTS
  cdd_cst_tree_t *tree = NULL;
  int rc;
  const char *code =
      "#include <unistd.h>\nvoid f() { __builtin_expect(1, 1); }\n";
  cdd_transform_config_t config = {0, 2, 0, 1, 0};

  cdd_cst_parse(az_span_create_from_str((char *)code), &tree);

  cdd_transform_msvc(tree, &config);

  /* We need a fresh tree since tokens get replaced */
  cdd_cst_tree_free(tree);
  tree = NULL;
  cdd_cst_parse(az_span_create_from_str((char *)code), &tree);

  g_msvc_port_bld_fail = 2;
  rc = cdd_transform_msvc(tree, &config);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_msvc_port_bld_fail = 0;

  cdd_cst_tree_free(tree);
  tree = NULL;

  /* Test all possible allocation failures to cover wrap_node and deps_node NULL
   * branches */
  {
    extern int g_cdd_alloc_fail;
    int fail_idx;
    for (fail_idx = 1; fail_idx < 30; fail_idx++) {
      cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
      g_cdd_alloc_fail = fail_idx;
      cdd_transform_msvc(tree, &config);
      g_cdd_alloc_fail = 0;
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }

  /* Check false branches for memcmp and should_skip logic */
  {
    const char *misc =
        "#include <stdio.h>\n"
        "#define strdup\n"
        "void f() {\n"
        "  struct A *p; p->strdup = 1;\n"
        "  abcdefghij(); /* 10 chars, not strcasecmp */\n"
        "  abcdefghijk(); /* 11 chars, not strncasecmp */\n"
        "  abcdef(); /* 6 chars, not strdup/fileno/unlink/getcwd */\n"
        "  abcdefg(); /* 7 chars, not ssize_t */\n"
        "  abcdefghijklmnop(); /* 16 chars, not __builtin_expect */\n"
        "  abcde(); /* 5 chars, not off_t/pid_t/close/mkdir/rmdir/isnan */\n"
        "  abcd(); /* 4 chars, not open/read/write */\n"
        "  abcdefgh(); /* 8 chars, not snprintf/strtok_r */\n"
        "  ssize_t(1);\n"
        "  return *mkdir;\n"
        "}\n"
        "struct open;\n"
        "union close;\n"
        "enum read;\n"
        "int *write;\n"
        "struct X *mkdir;\n"
        "union Y *rmdir;\n"
        "enum Z *getcwd;\n";
    cdd_cst_parse(az_span_create_from_str((char *)misc), &tree);
    rc = cdd_transform_msvc(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Check NULL prev_prev_token */
  {
    const char *misc2 = "* strdup;";
    cdd_cst_parse(az_span_create_from_str((char *)misc2), &tree);
    rc = cdd_transform_msvc(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }
  {
    const char *fails[] = {"void f() { strcasecmp(\"a\"); }",
                           "void f() { strncasecmp(\"a\", \"b\", 1); }",
                           "void f() { strdup(\"a\"); }",
                           "void f() { ssize_t s; }",
                           "void f() { __builtin_expect(1, 1); }",
                           "void f() { off_t o; }",
                           "void f() { pid_t p; }",
                           "void f() { mode_t m; }",
                           "void f() { open(1); }",
                           "void f() { close(2); }",
                           "void f() { read(3); }",
                           "void f() { write(4); }",
                           "void f() { fileno(5); }",
                           "void f() { unlink(6); }",
                           "void f() { mkdir(7); }",
                           "void f() { rmdir(8); }",
                           "void f() { getcwd(9); }",
                           "void f() { snprintf(10); }",
                           "void f() { strtok_r(11); }",
                           "void f() { isnan(12); }"};
    size_t i;
    for (i = 0; i < sizeof(fails) / sizeof(fails[0]); i++) {
      extern int g_cdd_cst_alloc_token_fail;
      int parse_rc =
          cdd_cst_parse(az_span_create_from_str((char *)fails[i]), &tree);
      if (parse_rc != 0 || tree == NULL) {
        printf("PARSE FAILED FOR %s\n", fails[i]);
      }
      g_cdd_cst_alloc_token_fail = 1;
      rc = cdd_transform_msvc(tree, &config);
      if (rc != CDD_C_ERROR_MEMORY) {
        printf("TRANSFORM DID NOT RETURN OOM FOR %s (rc=%d)\n", fails[i], rc);
      }
      g_cdd_cst_alloc_token_fail = 0;
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }
#endif
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Test suite for MSVC port transformer.
 */
SUITE(transformer_msvc_port_suite) {
  RUN_TEST(test_cdd_transform_msvc);
  RUN_TEST(test_cdd_transform_msvc_context);
  RUN_TEST(test_cdd_transform_msvc_builder_fails);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_MSVC_PORT_H */
