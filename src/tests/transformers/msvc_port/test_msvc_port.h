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
extern C_CDD_EXPORT int g_cdd_query_err_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
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
  int rc = 0;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));

  rc = cdd_cst_parse(az_span_create_from_str(code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_transform_msvc(tree, &config);
  ASSERT_EQ(0, rc);

  {
    int i;
    for (i = 0; i < 50; i++) {
      if (1) {
        cdd_cst_tree_t *tree_copy = NULL;
        rc = cdd_cst_parse(az_span_create_from_str(code), &tree_copy);
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
          g_cdd_last_alloc_err = cdd_cst_tree_free(tree_copy);
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
    g_cdd_last_alloc_err = cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &node);
    tree2 = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
    tree2->root = node;
    ASSERT_EQ(
        0, cdd_transform_msvc(tree2, &config)); /* Should not crash on empty */
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree2);
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
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
/* extern C_CDD_EXPORT int g_msvc_port_bld_fail; (moved to global) */
#endif

/**
 * @brief Unit test test_cdd_transform_msvc_context.
 * @return Greatest test result.
 */
TEST test_cdd_transform_msvc_context(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "struct A { int strdup; };\n"
                     "int strdup = 1;\n"
                     "char * strcasecmp = NULL;\n"
                     "#define MACRO(strdup) strdup\n"
                     "void foo() { struct A a; a.strdup = 1; }\n";
  char *out = NULL;
  int rc = 0;
  cdd_transform_config_t config;
  memset(&config, 0, sizeof(config));

  rc = cdd_cst_parse(az_span_create_from_str(code), &tree);
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
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Unit test test_cdd_transform_msvc_builder_fails.
 * @return Greatest test result.
 */
TEST test_cdd_transform_msvc_builder_fails(void) {
  cdd_cst_tree_t *tree = NULL;
  int rc = 0;
  const char *code = cdd_test_mut_str("#if defined(_WIN32)\n#include "
                                      "<io.h>\n#else\n#include "
                                      "<unistd.h>\n#endif\nvoid f() { "
                                      "__builtin_expect(1, 1); }\n");
  cdd_transform_config_t config;
  printf("HELLO FROM test_cdd_transform_msvc_builder_fails!!\nCODE STRING IS: "
         "%s\n",
         code);
  memset(&config, 0, sizeof(config));

  g_cdd_last_alloc_err = cdd_cst_parse(az_span_create_from_str(code), &tree);

  g_cdd_last_alloc_err = cdd_transform_msvc(tree, &config);

  /* We need a fresh tree since tokens get replaced */
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  tree = NULL;

  {
    const char *simple_code = cdd_test_mut_str("#include <unistd.h>\n");
    g_cdd_last_alloc_err =
        cdd_cst_parse(az_span_create_from_str(simple_code), &tree);
    g_cdd_query_err_fail = 1;
    rc = cdd_transform_msvc(tree, &config);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_query_err_fail = 0;

    g_cdd_cst_alloc_node_fail = 1;
    rc = cdd_transform_msvc(tree, &config);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_cst_alloc_node_fail = 0;

    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Test wrap_node builder error */
  {
    const char *simple_code = cdd_test_mut_str("#include <unistd.h>\n");
    g_cdd_last_alloc_err =
        cdd_cst_parse(az_span_create_from_str(simple_code), &tree);
    g_cdd_cst_alloc_token_fail = 1;
    rc = cdd_transform_msvc(tree, &config);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_cst_alloc_token_fail = 0;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  {
    int r_idx;
    for (r_idx = 1; r_idx <= 20; r_idx++) {
      g_cdd_last_alloc_err =
          cdd_cst_parse(az_span_create_from_str(code), &tree);
      tree->root->capacity = tree->root->num_children;
      g_cdd_cst_realloc_fail = r_idx;
      g_cdd_last_alloc_err = cdd_transform_msvc(tree, &config);
      g_cdd_cst_realloc_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }

  /* Test all possible allocation failures to cover wrap_node and deps_node NULL
   * branches */
  {
    /*  (moved to global) */
    int fail_idx;
    for (fail_idx = 1; fail_idx < 150; fail_idx++) {
      g_cdd_last_alloc_err =
          cdd_cst_parse(az_span_create_from_str(code), &tree);
      printf("SETTING fail_idx=%d\n", fail_idx);
      g_cdd_alloc_fail = fail_idx;
      g_cdd_last_alloc_err = cdd_transform_msvc(tree, &config);
      g_cdd_alloc_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
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
    g_cdd_last_alloc_err = cdd_cst_parse(az_span_create_from_str(misc), &tree);
    rc = cdd_transform_msvc(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Check NULL prev_prev_token */
  {
    const char *misc2 = cdd_test_mut_str("* strdup;");
    g_cdd_last_alloc_err = cdd_cst_parse(az_span_create_from_str(misc2), &tree);
    rc = cdd_transform_msvc(tree, &config);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
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
      int parse_rc = cdd_cst_parse(az_span_create_from_str(fails[i]), &tree);
      ASSERT_EQ(0, parse_rc);
      ASSERT(tree != NULL);
      g_cdd_cst_alloc_token_fail = 1;
      rc = cdd_transform_msvc(tree, &config);
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      g_cdd_cst_alloc_token_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests `--with-auto-win-msvc`, `mkdir`/`setmode` call-arity
 * disambiguation, `--rename-main`, empty-TU guard injection, and OOM paths.
 * @return Greatest test result.
 */
TEST test_cdd_transform_msvc_auto_win_and_arity(void) {
  cdd_cst_tree_t *tree = NULL;
  char *out = NULL;
  cdd_transform_config_t cfg;

  /* 1. Default mode: 2-arg mkdir -> _mkdir(p), 1-arg setmode -> bsd_setmode,
        2-arg setmode -> _setmode, 0/3-arg & nested-paren & non-call edges */
  memset(&cfg, 0, sizeof(cfg));
  {
    const char *src = "int use_posix(int fd, const char *p) {\n"
                      "  mkdir(p, 0755);\n"
                      "  setmode(p);\n"
                      "  setmode((p));\n"
                      "  setmode(fd, 1);\n"
                      "  setmode();\n"
                      "  setmode(fd, 1, 2);\n"
                      "  setmode;\n"
                      "  return 0;\n"
                      "}\n";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "_mkdir(p)") != NULL);
    ASSERT(strstr(out, "bsd_setmode(p)") != NULL);
    ASSERT(strstr(out, "bsd_setmode((p))") != NULL);
    ASSERT(strstr(out, "_setmode(fd, 1)") != NULL);
    free(out);
    out = NULL;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 2. --with-auto-win-msvc + --rename-main + non-empty TU */
  memset(&cfg, 0, sizeof(cfg));
  cfg.with_auto_win_msvc = 1;
  cfg.rename_main = "awm_tool_main";
  {
    const char *src = "#include <unistd.h>\n"
                      "typedef int (*main)(void);\n"
                      "int helper(int (main)) { return main; }\n"
                      "int main(int argc, char **argv) {\n"
                      "  ssize_t n = read(0, argv, 1);\n"
                      "  off_t o = 0; pid_t p = 0; mode_t m = 0;\n"
                      "  open(\"a\", 0); close(0); write(1, \"x\", 1);\n"
                      "  mkdir(\"d\", 0755);\n"
                      "  setmode(\"0755\");\n"
                      "  setmode(0, 1);\n"
                      "  return (int)(n + o + p + m);\n"
                      "}\n";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(src), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
    ASSERT(strstr(out, "#ifndef _MSC_VER") == NULL);
    ASSERT(strstr(out, "int awm_tool_main(int argc") != NULL);
    ASSERT(strstr(out, "typedef int (*main)(void);") != NULL);
    ASSERT(strstr(out, "bsd_setmode(\"0755\")") != NULL);
    ASSERT(strstr(out, "setmode(0, 1)") != NULL);
    ASSERT(strstr(out, "__awm_nonempty_tu") == NULL);
    free(out);
    out = NULL;
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Empty rename_main string "" + NULL config */
  cfg.rename_main = "";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("int x = 1;\n"), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, NULL));
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  tree = NULL;

  /* 3. Empty TU guard injection (preproc-only TU, trivia-only TU, block TU) */
  memset(&cfg, 0, sizeof(cfg));
  cfg.target_profile = CDD_TARGET_PROFILE_MSVC_MODERN;
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_cst_parse(az_span_create_from_str("#include <stdio.h>\n"), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_emit(tree, &out));
  ASSERT(strstr(out, "typedef int __awm_nonempty_tu;") != NULL);
  free(out);
  out = NULL;
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_cst_parse(az_span_create_from_str("/* only trivia */\n"), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  tree = NULL;

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cst_parse(az_span_create_from_str("{ int b; }\n"), &tree));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
  g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  tree = NULL;

  {
    cdd_cst_node_t *empty_u = NULL;
    cdd_cst_node_t *nested_u = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_parse(az_span_create_from_str(""), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &empty_u));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_insert_child_node_at(tree->root, 0, empty_u));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &nested_u));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_insert_child_node_at(empty_u, 0, nested_u));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Extra rename_main token shapes: (main)(); {}(); int (x); void test(); ) */
  cfg.rename_main = "entry_fn";
  {
    const char *rm_shapes = "(main)();\n"
                            "{}();\n"
                            "int (x);\n"
                            "void test(void) { setmode(\"0755\"); }\n"
                            ");\n";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(rm_shapes), &tree));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_transform_msvc(tree, &cfg));
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* Unclosed paren / child-node / zero-length token edges in arity helper */
  memset(&cfg, 0, sizeof(cfg));
  {
    cdd_cst_node_t *sub = NULL;
    cdd_token_t *zt = NULL;
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("int x = setmode(a"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;

    /* Inject a child node & zero-length token inside mkdir(a, b) */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("int x = mkdir(a, b);"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &sub));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_create_token(tree, CDD_TOKEN_OTHER, "", &zt));
    /* Insert empty token before 'a' and child node after ',' in root child 0 */
    {
      cdd_cst_node_t *decl = tree->root->children[0].val.node;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_insert_child_node_at(
                                   decl, decl->num_children - 2, sub));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_replace_token_child(decl, 5, zt));
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;

    /* Also test setmode followed by a child node instead of token */
    ASSERT_EQ(
        CDD_C_SUCCESS,
        cdd_cst_parse(az_span_create_from_str("int x = setmode;"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &sub));
    {
      cdd_cst_node_t *decl = tree->root->children[0].val.node;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_insert_child_node_at(decl, 4, sub));
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;

    /* Also test setmode at the very end of node->children (lparen_idx ==
     * num_children) */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("setmode"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;
  }

  /* 4. OOM paths on mkdir(p, m), setmode(1), setmode(2), rename-main, empty TU
   */
  {
    const char *oom_cases[] = {
        "void f() { mkdir(\"d\", 0755); }", "void f() { setmode(\"0755\"); }",
        "void f() { setmode(0, 1); }", "int main(void) { return 0; }", ""};
    size_t ci;
    for (ci = 0; ci < sizeof(oom_cases) / sizeof(oom_cases[0]); ci++) {
      int fi;
      memset(&cfg, 0, sizeof(cfg));
      if (ci == 3)
        cfg.rename_main = "renamed_main";
      if (ci == 4)
        cfg.with_auto_win_msvc = 1;
      for (fi = 1; fi <= 10; fi++) {
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &tree));
        tree->root->capacity = tree->root->num_children;
        g_cdd_alloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_msvc(tree, &cfg);
        g_cdd_alloc_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;

        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &tree));
        tree->root->capacity = tree->root->num_children;
        g_cdd_cst_realloc_fail = fi;
        g_cdd_last_alloc_err = cdd_transform_msvc(tree, &cfg);
        g_cdd_cst_realloc_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
      }
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &tree));
      g_cdd_cst_alloc_token_fail = 1;
      g_cdd_last_alloc_err = cdd_transform_msvc(tree, &cfg);
      g_cdd_cst_alloc_token_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
      tree = NULL;

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &tree));
      g_cdd_cst_alloc_token_fail = 2;
      g_cdd_last_alloc_err = cdd_transform_msvc(tree, &cfg);
      g_cdd_cst_alloc_token_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
      tree = NULL;

      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_cst_parse(az_span_create_from_str(oom_cases[ci]), &tree));
      g_cdd_cst_alloc_node_fail = 1;
      g_cdd_last_alloc_err = cdd_transform_msvc(tree, &cfg);
      g_cdd_cst_alloc_node_fail = 0;
      g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
      tree = NULL;
    }
  }

  /* Cover remaining rename_main and empty-TU branches in msvc_port.c */
  {
    cdd_transform_config_t cfg_extra;
    cdd_token_t *syn_tok = NULL;
    cdd_cst_node_t *sub_node = NULL;
    memset(&cfg_extra, 0, sizeof(cfg_extra));
    cfg_extra.with_auto_win_msvc = 1;
    cfg_extra.rename_main = "app_entry";

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str(
                                "int testing(void) { return 0; }\n"
                                "int main(void) { return testing(); }\n"),
                            &tree));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_create_token(tree, CDD_TOKEN_OTHER, "", &syn_tok));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(tree->root, syn_tok));
    /* Insert a child node immediately before `(` on a top-level node */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &sub_node));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_create_token(tree, CDD_TOKEN_LPAREN, "(", &syn_tok));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, sub_node));
    {
      cdd_cst_node_t *inner = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &inner));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(sub_node, inner));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(sub_node, syn_tok));
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg_extra));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;

    /* Empty TU with comment-only EOF token + zero-length token child */
    cfg_extra.rename_main = "";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_parse(az_span_create_from_str("/* empty */\n"), &tree));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_alloc_node(CDD_CST_UNKNOWN, &sub_node));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cst_create_token(tree, CDD_TOKEN_OTHER, "", &syn_tok));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_token(sub_node, syn_tok));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_append_child_node(tree->root, sub_node));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_transform_msvc(tree, &cfg_extra));
    g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
    tree = NULL;

#ifdef CDD_BUILD_TESTS
    {
      const char *step_src = "#include <unistd.h>\n"
                             "#include <sys/time.h>\n"
                             "int main(void) {\n"
                             "  ssize_t s = 0;\n"
                             "  __builtin_expect(1, 1);\n"
                             "  mkdir(\"d\", 0755);\n"
                             "  setmode(0, 1);\n"
                             "  return (int)s;\n"
                             "}\n";
      int step;
      cdd_c_error_t s_rc;
      cfg_extra.rename_main = "entry_fn";
      for (step = 1; step <= 250; ++step) {
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(step_src), &tree));
        g_cdd_step_fail = step;
        s_rc = cdd_transform_msvc(tree, &cfg_extra);
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
        if (s_rc == CDD_C_SUCCESS && g_cdd_step_fail > 0) {
          g_cdd_step_fail = 0;
          break;
        }
        g_cdd_step_fail = 0;
      }
      for (step = 1; step <= 25; ++step) {
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(step_src), &tree));
        g_cdd_cst_alloc_token_fail = step;
        g_cdd_last_alloc_err = cdd_transform_msvc(tree, &cfg_extra);
        g_cdd_cst_alloc_token_fail = 0;
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
      }
      cfg_extra.with_auto_win_msvc = 0;
      for (step = 1; step <= 120; ++step) {
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(step_src), &tree));
        g_cdd_step_fail = step;
        s_rc = cdd_transform_msvc(tree, &cfg_extra);
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
        if (s_rc == CDD_C_SUCCESS && g_cdd_step_fail > 0) {
          g_cdd_step_fail = 0;
          break;
        }
        g_cdd_step_fail = 0;
      }
      cfg_extra.with_auto_win_msvc = 1;
      for (step = 1; step <= 40; ++step) {
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_cst_parse(az_span_create_from_str(""), &tree));
        g_cdd_step_fail = step;
        s_rc = cdd_transform_msvc(tree, &cfg_extra);
        g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
        tree = NULL;
        if (s_rc == CDD_C_SUCCESS && g_cdd_step_fail > 0) {
          g_cdd_step_fail = 0;
          break;
        }
        g_cdd_step_fail = 0;
      }
    }
#endif
  }

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
  RUN_TEST(test_cdd_transform_msvc_auto_win_and_arity);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_TRANSFORM_MSVC_PORT_H */
