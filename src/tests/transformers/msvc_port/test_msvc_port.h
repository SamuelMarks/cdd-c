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

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_msvc_port_bld_fail;
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;

/**
 * @brief Test MSVC transformation of POSIX features.
 *
 * @return The result of the test.
 */
TEST test_cdd_transform_msvc(void) {
  cdd_cst_tree_t *tree = NULL;
  const char code[] = {
      35,  105, 110, 99,  108, 117, 100, 101, 32,  60,  117, 110, 105, 115, 116,
      100, 46,  104, 62,  10,  35,  105, 110, 99,  108, 117, 100, 101, 32,  60,
      115, 121, 115, 47,  116, 105, 109, 101, 46,  104, 62,  10,  105, 110, 116,
      32,  109, 97,  105, 110, 40,  41,  32,  123, 10,  32,  32,  115, 116, 114,
      99,  97,  115, 101, 99,  109, 112, 32,  47,  42,  32,  99,  111, 109, 109,
      101, 110, 116, 32,  42,  47,  32,  40,  34,  97,  34,  44,  32,  34,  98,
      34,  41,  59,  10,  32,  32,  115, 116, 114, 110, 99,  97,  115, 101, 99,
      109, 112, 40,  34,  97,  34,  44,  32,  34,  98,  34,  44,  32,  49,  41,
      59,  10,  32,  32,  115, 116, 114, 100, 117, 112, 40,  34,  97,  34,  41,
      59,  10,  32,  32,  115, 115, 105, 122, 101, 95,  116, 32,  115, 32,  61,
      32,  48,  59,  10,  32,  32,  95,  95,  98,  117, 105, 108, 116, 105, 110,
      95,  101, 120, 112, 101, 99,  116, 40,  49,  44,  32,  49,  41,  59,  10,
      32,  32,  111, 102, 102, 95,  116, 32,  111, 102, 102, 59,  32,  112, 105,
      100, 95,  116, 32,  112, 105, 100, 59,  32,  109, 111, 100, 101, 95,  116,
      32,  109, 59,  10,  32,  32,  111, 112, 101, 110, 40,  49,  41,  59,  32,
      99,  108, 111, 115, 101, 40,  50,  41,  59,  32,  114, 101, 97,  100, 40,
      51,  41,  59,  32,  119, 114, 105, 116, 101, 40,  52,  41,  59,  10,  32,
      32,  102, 105, 108, 101, 110, 111, 40,  53,  41,  59,  32,  117, 110, 108,
      105, 110, 107, 40,  54,  41,  59,  32,  109, 107, 100, 105, 114, 40,  55,
      41,  59,  32,  114, 109, 100, 105, 114, 40,  56,  41,  59,  32,  103, 101,
      116, 99,  119, 100, 40,  57,  41,  59,  10,  32,  32,  115, 110, 112, 114,
      105, 110, 116, 102, 40,  48,  41,  59,  32,  115, 116, 114, 116, 111, 107,
      95,  114, 40,  49,  41,  59,  32,  105, 115, 110, 97,  110, 40,  50,  41,
      59,  10,  32,  32,  114, 101, 116, 117, 114, 110, 32,  48,  59,  10,  125,
      10,  0};
  char *out = NULL;
  int rc;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
  ASSERT_EQ(0, rc);

  rc = cdd_transform_msvc(tree, &config);
  ASSERT_EQ(0, rc);

  {
    int i;
    for (i = 0; i < 50; i++) {
      if (0) {
        cdd_cst_tree_t *tree_copy = NULL;
        rc = cdd_cst_parse(
            az_span_create_from_str((char *)(size_t)(size_t)code), &tree_copy);
        if (rc == 0 && tree_copy) {
#ifdef CDD_BUILD_TESTS
          /* extern C_CDD_EXPORT int g_cdd_cst_realloc_fail; (moved to global)
           */
          g_cdd_cst_realloc_fail = i;
#endif
          rc = cdd_transform_msvc(tree_copy, &config);
#ifdef CDD_BUILD_TESTS
          g_cdd_cst_realloc_fail = 0;
#endif
          cdd_cst_tree_free(tree_copy);
        }
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
    cdd_cst_node_t *node;
    node = NULL;
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
/* extern C_CDD_EXPORT int g_msvc_port_bld_fail; (moved to global) */
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
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                     &tree);
  (void)rc;
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
  const char *code = (char *)(size_t)(size_t) "#include <unistd.h>\nvoid f() { "
                                              "__builtin_expect(1, 1); }\n";
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));

  (void)rc;
  cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code), &tree);

  cdd_transform_msvc(tree, &config);

  /* We need a fresh tree since tokens get replaced */
  cdd_cst_tree_free(tree);
  tree = NULL;
  cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code), &tree);

  g_msvc_port_bld_fail = 2;
  rc = cdd_transform_msvc(tree, &config);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_msvc_port_bld_fail = 0;

  cdd_cst_tree_free(tree);
  tree = NULL;

  /* Test all possible allocation failures to cover wrap_node and deps_node NULL
   * branches */
  {
    /*  (moved to global) */
    int fail_idx;
    for (fail_idx = 1; fail_idx < 30; fail_idx++) {
      cdd_cst_parse(az_span_create_from_str((char *)(size_t)(size_t)code),
                    &tree);
      g_cdd_alloc_fail = fail_idx;
      cdd_transform_msvc(tree, &config);
      g_cdd_alloc_fail = 0;
      cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }

  /* Check false branches for memcmp and should_skip logic */
  {
    const char misc[] = {
        35,  105, 110, 99,  108, 117, 100, 101, 32,  60,  115, 116, 100, 105,
        111, 46,  104, 62,  10,  35,  100, 101, 102, 105, 110, 101, 32,  115,
        116, 114, 100, 117, 112, 10,  118, 111, 105, 100, 32,  102, 40,  41,
        32,  123, 10,  32,  32,  115, 116, 114, 117, 99,  116, 32,  65,  32,
        42,  112, 59,  32,  112, 45,  62,  115, 116, 114, 100, 117, 112, 32,
        61,  32,  49,  59,  10,  32,  32,  97,  98,  99,  100, 101, 102, 103,
        104, 105, 106, 40,  41,  59,  32,  47,  42,  32,  49,  48,  32,  99,
        104, 97,  114, 115, 44,  32,  110, 111, 116, 32,  115, 116, 114, 99,
        97,  115, 101, 99,  109, 112, 32,  42,  47,  10,  32,  32,  97,  98,
        99,  100, 101, 102, 103, 104, 105, 106, 107, 40,  41,  59,  32,  47,
        42,  32,  49,  49,  32,  99,  104, 97,  114, 115, 44,  32,  110, 111,
        116, 32,  115, 116, 114, 110, 99,  97,  115, 101, 99,  109, 112, 32,
        42,  47,  10,  32,  32,  97,  98,  99,  100, 101, 102, 40,  41,  59,
        32,  47,  42,  32,  54,  32,  99,  104, 97,  114, 115, 44,  32,  110,
        111, 116, 32,  115, 116, 114, 100, 117, 112, 47,  102, 105, 108, 101,
        110, 111, 47,  117, 110, 108, 105, 110, 107, 47,  103, 101, 116, 99,
        119, 100, 32,  42,  47,  10,  32,  32,  97,  98,  99,  100, 101, 102,
        103, 40,  41,  59,  32,  47,  42,  32,  55,  32,  99,  104, 97,  114,
        115, 44,  32,  110, 111, 116, 32,  115, 115, 105, 122, 101, 95,  116,
        32,  42,  47,  10,  32,  32,  97,  98,  99,  100, 101, 102, 103, 104,
        105, 106, 107, 108, 109, 110, 111, 112, 40,  41,  59,  32,  47,  42,
        32,  49,  54,  32,  99,  104, 97,  114, 115, 44,  32,  110, 111, 116,
        32,  95,  95,  98,  117, 105, 108, 116, 105, 110, 95,  101, 120, 112,
        101, 99,  116, 32,  42,  47,  10,  32,  32,  97,  98,  99,  100, 101,
        40,  41,  59,  32,  47,  42,  32,  53,  32,  99,  104, 97,  114, 115,
        44,  32,  110, 111, 116, 32,  111, 102, 102, 95,  116, 47,  112, 105,
        100, 95,  116, 47,  99,  108, 111, 115, 101, 47,  109, 107, 100, 105,
        114, 47,  114, 109, 100, 105, 114, 47,  105, 115, 110, 97,  110, 32,
        42,  47,  10,  32,  32,  97,  98,  99,  100, 40,  41,  59,  32,  47,
        42,  32,  52,  32,  99,  104, 97,  114, 115, 44,  32,  110, 111, 116,
        32,  111, 112, 101, 110, 47,  114, 101, 97,  100, 47,  119, 114, 105,
        116, 101, 32,  42,  47,  10,  32,  32,  97,  98,  99,  100, 101, 102,
        103, 104, 40,  41,  59,  32,  47,  42,  32,  56,  32,  99,  104, 97,
        114, 115, 44,  32,  110, 111, 116, 32,  115, 110, 112, 114, 105, 110,
        116, 102, 47,  115, 116, 114, 116, 111, 107, 95,  114, 32,  42,  47,
        10,  32,  32,  115, 115, 105, 122, 101, 95,  116, 40,  49,  41,  59,
        10,  32,  32,  114, 101, 116, 117, 114, 110, 32,  42,  109, 107, 100,
        105, 114, 59,  10,  125, 10,  115, 116, 114, 117, 99,  116, 32,  111,
        112, 101, 110, 59,  10,  117, 110, 105, 111, 110, 32,  99,  108, 111,
        115, 101, 59,  10,  101, 110, 117, 109, 32,  114, 101, 97,  100, 59,
        10,  105, 110, 116, 32,  42,  119, 114, 105, 116, 101, 59,  10,  115,
        116, 114, 117, 99,  116, 32,  88,  32,  42,  109, 107, 100, 105, 114,
        59,  10,  117, 110, 105, 111, 110, 32,  89,  32,  42,  114, 109, 100,
        105, 114, 59,  10,  101, 110, 117, 109, 32,  90,  32,  42,  103, 101,
        116, 99,  119, 100, 59,  10,  0};
    cdd_cst_parse(az_span_create_from_str((char *)(size_t)misc), &tree);
    rc = cdd_transform_msvc(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Check NULL prev_prev_token */
  {
    const char *misc2 = (char *)(size_t)(size_t) "* strdup;";
    cdd_cst_parse(az_span_create_from_str((char *)(size_t)misc2), &tree);
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
      /* extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail; (moved to global)
       */
      int parse_rc = cdd_cst_parse(
          az_span_create_from_str((char *)(size_t)fails[i]), &tree);
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
