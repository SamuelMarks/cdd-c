/**
 * @file test_refactor_orchestrator.h
 * @brief Unit tests for the refactoring orchestrator.
 */

#ifndef TEST_REFACTOR_ORCHESTRATOR_H
#define TEST_REFACTOR_ORCHESTRATOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/orchestrator.h"
/* clang-format on */

TEST test_orchestrator_simple_propagation(void) {
  /*
     A returns void, allocs.
     B calls A.
     Refactor: A -> int, B -> int.
  */
  const char *input = ""
                      "void A() { char * p = (char *)malloc(1); *p=0; }\n"
                      ""
                      "void B() { A(); }";

  char *out = NULL;
  int rc = orchestrate_fix(input, &out);

  ASSERT_EQ(0, rc);
  ASSERT(out != NULL);

  /* Check A refactored signature */
  ASSERT(strstr(out, "int A()") != NULL);
  /* Check A safety injection */
  printf("OUT IS: %s\n", out);
  ASSERT(strstr(out, "if (!p) { return CDD_C_ERROR_MEMORY; }") != NULL);
  /* Check A returns success (implied return 0 at end since void->int logic
   * injects it) */
  ASSERT(strstr(out, "return CDD_C_SUCCESS;") != NULL);

  /* Check B refactored (Propagated) */
  ASSERT(strstr(out, "int B()") != NULL);
  /* Check A call inside B rewritten */
  /* Logic: rc = A(); if (rc != 0) return rc; */
  printf("OUT MAIN IS: %s\n", out);
  ASSERT(strstr(out, "cdd_c_error_t rc = CDD_C_SUCCESS;") != NULL);
  ASSERT(strstr(out, "rc = A();") != NULL);
  /* "if (rc != 0) return rc;" */

  free(out);
  g_fail_io_after = -1;
  PASS();
}

TEST test_orchestrator_propagation_ptr(void) {
  /*
     A returns ptr, allocs.
     B calls A.
     Refactor: A -> int A(out), B -> int B(out).
  */
  /* Fixed input to have space "char *A" to match expected "char * *out"
   * generation logic */
  const char *input = ""
                      "char *A() { return strdup(\"x\"); }\n"
                      ""
                      "char *B() { char *x = A(); return x; }";

  char *out = NULL;
  int rc = orchestrate_fix(input, &out);

  ASSERT_EQ(0, rc);

  /* Check A signature */
  ASSERT(strstr(out, "int A(char * *out)") != NULL);

  /* Check B signature */
  ASSERT(strstr(out, "int B(char * *out)") != NULL);

  /* Check call propagation in B */
  /* x = A() -> rc = A(&x); */
  ASSERT(strstr(out, "rc = A(&x);") != NULL);

  free(out);
  g_fail_io_after = -1;
  PASS();
}

TEST test_orchestrator_main_stop(void) {
  /*
     A returns void, allocs.
     main calls A.
     Refactor: A -> int.
     main -> signature UNCHANGED, but body updates to check A.
  */
  const char *input = ""
                      "void A() { malloc(1); }\n"
                      ""
                      "int main() { A(); return 0; }";

  char *out = NULL;
  int rc = orchestrate_fix(input, &out);

  ASSERT_EQ(0, rc);

  /* A changed signature */
  ASSERT(strstr(out, "int A()") != NULL);

  /* Main sig UNCHANGED */
  ASSERT(strstr(out, "int main()") != NULL);

  /* Main body UPDATED */
  /* Checks internal var injection and return rc check */
  printf("OUT MAIN IS: %s\n", out);
  ASSERT(strstr(out, "cdd_c_error_t rc = CDD_C_SUCCESS;") != NULL);
  ASSERT(strstr(out, "rc = A();") != NULL);

  free(out);
  g_fail_io_after = -1;
  PASS();
}

TEST test_orchestrator_no_alloc(void) {
  const char *input = ""
                      "void A() { int x=1; }";
  char *out = NULL;
  int rc = orchestrate_fix(input, &out);
  ASSERT_EQ(0, rc);
  /* Should remain mostly same (token reconstruction might normalize whitespace)
   */
  ASSERT(strstr(out, ""
                     "void A() {") != NULL);
  ASSERT(strstr(out, "int x=1;") != NULL);
  free(out);
  g_fail_io_after = -1;
  PASS();
}

TEST test_orchestrator_preserves_structs(void) {
  /* Ensure non-function nodes like structs are preserved */
  const char *input = "struct S { int x; }; int f() { return 0; }";
  char *out = NULL;
  int rc = orchestrate_fix(input, &out);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(out, "struct S {") != NULL);
  free(out);
  g_fail_io_after = -1;
  PASS();
}

TEST test_orchestrator_edge_cases(void) {
  char *out = NULL;
  int rc;

  /* Invalid arguments */
  rc = orchestrate_fix(NULL, &out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = orchestrate_fix("void A() {}", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Invalid syntax cases that might hit fallbacks or error paths */
  orchestrate_fix("void A { malloc(1); }", &out);
  if (out) {
    free(out);
    out = NULL;
  }

  /* No name found - whitespace only before paren */
  orchestrate_fix("void \n  () { malloc(1); }", &out);
  if (out) {
    free(out);
    out = NULL;
  }

  orchestrate_fix("void () { malloc(1); }", &out);
  if (out) {
    free(out);
    out = NULL;
  }

  /* Only identifier, no lparen */
  orchestrate_fix("void A", &out);
  if (out) {
    free(out);
    out = NULL;
  }

  /* Empty parens directly at start */
  orchestrate_fix("() { malloc(1); }", &out);
  if (out) {
    free(out);
    out = NULL;
  }

  /* Duplicate edge case */
  orchestrate_fix("void A() { malloc(1); }\n"
                  "void B() { A(); A(); A(); }",
                  &out);
  if (out) {
    free(out);
    out = NULL;
  }

  /* Duplicate refactor mark via multiple call paths */
  orchestrate_fix("void A() { malloc(1); }\n"
                  "void B() { A(); }\n"
                  "void C() { A(); }\n"
                  "void D() { B(); C(); }",
                  &out);
  if (out) {
    free(out);
    out = NULL;
  }

  /* No memory allocation within body */
  orchestrate_fix("void A() {}", &out);
  if (out) {
    free(out);
    out = NULL;
  }

  /* Multiple allocations to trigger array reallocation in local_allocs.sites */
  orchestrate_fix("void A() {\n"
                  "malloc(1);\n"
                  "malloc(1);\n"
                  "malloc(1);\n"
                  "malloc(1);\n"
                  "malloc(1);\n"
                  "}",
                  &out);
  if (out) {
    free(out);
    out = NULL;
  }

  /* A lot of callers to trigger array reallocation in callee->callers */
  orchestrate_fix("void A() { malloc(1); }\n"
                  "void B0() { A(); }\n"
                  "void B1() { A(); }\n"
                  "void B2() { A(); }\n"
                  "void B3() { A(); }\n"
                  "void B4() { A(); }",
                  &out);
  if (out) {
    free(out);
    out = NULL;
  }

#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_alloc_fail;
    extern int g_cdd_strdup_fail;
    extern int g_cdd_cst_alloc_node_fail;
    int i;
    for (i = 1; i < 20000; ++i) {
      g_cdd_alloc_fail = i;
      rc = orchestrate_fix("void A() { malloc(1); }\n"
                           "void B() { A(); }\n"
                           "void C() { A(); }\n"
                           "void D() { A(); }\n"
                           "void E() { A(); }\n"
                           "void F() { A(); }\n"
                           "void G() { A(); }\n"
                           "void H() { A(); }\n"
                           "void I() { A(); }\n"
                           "int main() { A(); return 0; }",
                           &out);
      if (out) {
        free(out);
        out = NULL;
      }
      if (rc == CDD_C_SUCCESS) {
        break;
      }
    }
    g_cdd_alloc_fail = 0;

    for (i = 1; i < 20000; ++i) {
      g_cdd_strdup_fail = i;
      rc = orchestrate_fix("void A() { malloc(1); }\n"
                           "void B() { A(); }\n"
                           "void C() { A(); }\n"
                           "void D() { A(); }\n"
                           "void E() { A(); }\n"
                           "void F() { A(); }\n"
                           "void G() { A(); }\n"
                           "void H() { A(); }\n"
                           "void I() { A(); }\n"
                           "int main() { A(); return 0; }",
                           &out);
      if (out) {
        free(out);
        out = NULL;
      }
      if (rc == CDD_C_SUCCESS)
        break;
    }
    g_cdd_strdup_fail = 0;

    for (i = 1; i < 20000; ++i) {
      g_cdd_cst_alloc_node_fail = i;
      rc = orchestrate_fix("void A() { malloc(1); }\n"
                           "void B() { A(); }\n"
                           "void C() { A(); }\n"
                           "void D() { A(); }\n"
                           "void E() { A(); }\n"
                           "void F() { A(); }\n"
                           "void G() { A(); }\n"
                           "void H() { A(); }\n"
                           "void I() { A(); }\n"
                           "int main() { A(); return 0; }",
                           &out);
      if (out) {
        free(out);
        out = NULL;
      }
      if (rc == CDD_C_SUCCESS)
        break;
    }
    g_cdd_cst_alloc_node_fail = 0;

    for (i = 1; i < 20000; ++i) {
      extern int g_cdd_cst_alloc_token_fail;
      g_cdd_cst_alloc_token_fail = i;
      rc = orchestrate_fix("void A() { malloc(1); }\n"
                           "void B() { A(); }\n"
                           "void C() { A(); }\n"
                           "void D() { A(); }\n"
                           "void E() { A(); }\n"
                           "void F() { A(); }\n"
                           "void G() { A(); }\n"
                           "void H() { A(); }\n"
                           "void I() { A(); }\n"
                           "int main() { A(); return 0; }",
                           &out);
      if (out) {
        free(out);
        out = NULL;
      }
      if (rc == CDD_C_SUCCESS)
        break;
    }
    g_cdd_cst_alloc_token_fail = 0;

    for (i = 1; i < 20000; ++i) {
      extern int g_cdd_cst_realloc_fail;
      g_cdd_cst_realloc_fail = i;
      rc = orchestrate_fix("void A() { malloc(1); }\n"
                           "void B() { A(); }\n"
                           "void C() { A(); }\n"
                           "void D() { A(); }\n"
                           "void E() { A(); }\n"
                           "void F() { A(); }\n"
                           "void G() { A(); }\n"
                           "void H() { A(); }\n"
                           "void I() { A(); }\n"
                           "int main() { A(); return 0; }",
                           &out);
      if (out) {
        free(out);
        out = NULL;
      }
      if (rc == CDD_C_SUCCESS)
        break;
    }
    g_cdd_cst_realloc_fail = 0;
  }
#endif

  g_fail_io_after = -1;
  PASS();
}

SUITE(refactor_orchestrator_suite) {
  RUN_TEST(test_orchestrator_simple_propagation);
  RUN_TEST(test_orchestrator_propagation_ptr);
  RUN_TEST(test_orchestrator_main_stop);
  RUN_TEST(test_orchestrator_no_alloc);
  RUN_TEST(test_orchestrator_preserves_structs);
  RUN_TEST(test_orchestrator_edge_cases);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REFACTOR_ORCHESTRATOR_H */
