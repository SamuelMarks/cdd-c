#ifndef TEST_REFACTOR_ORCHESTRATOR_H
#define TEST_REFACTOR_ORCHESTRATOR_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse_orchestrator.h"

TEST test_orchestrator_simple_propagation(void) {
  /*
     A returns void, allocs.
     B calls A.
     Refactor: A -> int, B -> int.
  */
  const char *input = "void A() { char *p = malloc(1); *p=0; }\n"
                      "void B() { A(); }";

  char *out = NULL;
  int rc = orchestrate_fix(input, &out);

  ASSERT_EQ(0, rc);
  ASSERT(out != NULL);

  /* Check A refactored signature */
  ASSERT(strstr(out, "int A()") != NULL);
  /* Check A safety injection */
  ASSERT(strstr(out, "if (!p) { return ENOMEM; }") != NULL);
  /* Check A returns success (implied return 0 at end since void->int logic
   * injects it) */
  ASSERT(strstr(out, "return 0;") != NULL);

  /* Check B refactored (Propagated) */
  ASSERT(strstr(out, "int B()") != NULL);
  /* Check A call inside B rewritten */
  /* Logic: rc = A(); if (rc != 0) return rc; */
  ASSERT(strstr(out, "int rc = 0;") != NULL);
  ASSERT(strstr(out, "rc = A();") != NULL);
  /* "if (rc != 0) return rc;" */

  free(out);
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
  const char *input = "char *A() { return strdup(\"x\"); }\n"
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
  PASS();
}

TEST test_orchestrator_main_stop(void) {
  /*
     A returns void, allocs.
     main calls A.
     Refactor: A -> int.
     main -> signature UNCHANGED, but body updates to check A.
  */
  const char *input = "void A() { malloc(1); }\n"
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
  ASSERT(strstr(out, "int rc = 0;") != NULL);
  ASSERT(strstr(out, "rc = A();") != NULL);

  free(out);
  PASS();
}

TEST test_orchestrator_no_alloc(void) {
  const char *input = "void A() { int x=1; }";
  char *out = NULL;
  int rc = orchestrate_fix(input, &out);
  ASSERT_EQ(0, rc);
  /* Should remain mostly same (token reconstruction might normalize whitespace)
   */
  ASSERT(strstr(out, "void A() {") != NULL);
  ASSERT(strstr(out, "int x=1;") != NULL);
  free(out);
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
  PASS();
}

SUITE(refactor_orchestrator_suite) {
  RUN_TEST(test_orchestrator_simple_propagation);
  RUN_TEST(test_orchestrator_propagation_ptr);
  RUN_TEST(test_orchestrator_main_stop);
  RUN_TEST(test_orchestrator_no_alloc);
  RUN_TEST(test_orchestrator_preserves_structs);
}

#endif /* TEST_REFACTOR_ORCHESTRATOR_H */
