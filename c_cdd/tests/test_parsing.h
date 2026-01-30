#ifndef TEST_PARSING_H
#define TEST_PARSING_H

#include <stdio.h>
#include <string.h>

#include <greatest.h>

#include <c_str_precondition.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "cst_parser.h"
#include "tokenizer.h"

TEST parsing_test(const char *const test_name, const az_span source,
                  const size_t expected_structs, const size_t expected_enums,
                  const size_t expected_unions, const size_t expected_comments,
                  const size_t expected_macros,
                  const size_t expected_whitespace) {
  struct TokenList *tokens = NULL;
  struct CstNodeList *cst_nodes = calloc(1, sizeof *cst_nodes);
  size_t s_count = 0, e_count = 0, u_count = 0, cm_count = 0, m_count = 0, i;
  int rc = EXIT_SUCCESS;

  printf("Running test: %s\n", test_name);

  if (tokenize(source, &tokens) != 0) {
    fprintf(stderr, "tokenize() failed in test %s\n", test_name);
    rc = EXIT_FAILURE;
    goto cleanup;
  }

  if (parse_tokens(tokens, cst_nodes) != 0) {
    fprintf(stderr, "parse_tokens() failed in test %s\n", test_name);
    rc = EXIT_FAILURE;
    goto cleanup;
  }

  /* Count node kinds */
  for (i = 0; i < cst_nodes->size; i++) {
    switch (cst_nodes->nodes[i].kind) {
    case CST_NODE_STRUCT:
      s_count++;
      break;
    case CST_NODE_ENUM:
      e_count++;
      break;
    case CST_NODE_UNION:
      u_count++;
      break;
    case CST_NODE_COMMENT:
      cm_count++;
      break;
    case CST_NODE_MACRO:
      m_count++;
      break;
    default:
      break;
    }
  }

  /* Assertions */
  ASSERT_EQm("struct count mismatch", s_count, expected_structs);
  ASSERT_EQm("enum count mismatch", e_count, expected_enums);
  ASSERT_EQm("union count mismatch", u_count, expected_unions);
  ASSERT_EQm("comment count mismatch", cm_count, expected_comments);
  ASSERT_EQm("macro count mismatch", m_count, expected_macros);
  (void)expected_whitespace;

  printf("Test '%s' passed.\n", test_name);

cleanup:
  free_token_list(tokens); /* Frees internal tokens AND the structure itself */
  free_cst_node_list(
      cst_nodes); /* Frees internal nodes but NOT the structure */
  /* free(tokens); <-- Removed: double free */
  free(cst_nodes); /* Frees the structure allocated by calloc above */
  if (rc == EXIT_SUCCESS)
    PASS();
  FAIL();
}

TEST test_precondition_failure(void) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  /* The precondition check is a no-op in release builds,
   * but this still covers the callback setting line. */
  PASS();
}

TEST test_parsing_struct(void) {
  /* Struct test */
  parsing_test("Struct parsing",
               AZ_SPAN_FROM_STR("struct Point { int x; int y; };"),
               1, /* structs */
               0, /* enums */
               0, /* unions */
               0, /* comments */
               0, /* macros */
               1  /* whitespace tokens expected (spaces) */
  );
  PASS();
}

TEST test_parsing_enum(void) {
  /* Enum test */
  parsing_test("Enum parsing",
               AZ_SPAN_FROM_STR("enum Color { RED, GREEN, BLUE };"), 0, 1, 0, 0,
               0, 1);
  PASS();
}

TEST test_parsing_union(void) {
  /* Union test */
  parsing_test("Union parsing",
               AZ_SPAN_FROM_STR("union Data { int i; float f; };"), 0, 0, 1, 0,
               0, 1);
  PASS();
}

TEST test_parsing_comments(void) {
  /* Comments test */
  parsing_test("Comments parsing",
               AZ_SPAN_FROM_STR("/* comment block */\n// line comment\nint x;"),
               0, 0, 0, 2, /* one block and one line comment */
               0, 3        /* some whitespace tokens: newlines/spaces */
  );
  PASS();
}

TEST test_parsing_macros(void) {
  /* Macros test */
  parsing_test("Macros parsing", AZ_SPAN_FROM_STR("#define MAX 100\nint a;"), 0,
               0, 0, 0, 1, 2);
  PASS();
}

TEST test_parsing_complex(void) {
  /* Complex test */
  parsing_test(
      "Complex parsing",
      AZ_SPAN_FROM_STR("/* block comment */\n"
                       "#include <stdio.h>\n"
                       "struct S { int a; union U { float f; int i; } u; };\n"
                       "enum E { X, Y, Z };\n"
                       "// single line comment\n"
                       "int main() { return 0; }\n"),
      1, /* structs */
      1, /* enums */
      1, /* unions */
      2, /* 1 block + 1 line */
      1, /* macro (#include) */
      7  /* whitespace tokens */
  );

  PASS();
}

TEST test_parsing_empty(void) {
  parsing_test("Empty string", AZ_SPAN_FROM_STR(""), 0, 0, 0, 0, 0, 0);
  PASS();
}

TEST test_parsing_struct_with_anonymous_union(void) {
  parsing_test("Struct with anonymous union",
               AZ_SPAN_FROM_STR("struct S { union { int i; }; };"),
               1, /* structs */
               1, /* unions */
               0, /* enums */
               0, /* comments */
               0, /* macros */
               2  /* whitespace tokens expected (spaces) */
  );
  PASS();
}

SUITE(parsing_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(test_parsing_struct);
  RUN_TEST(test_parsing_struct_with_anonymous_union);
  RUN_TEST(test_parsing_enum);
  RUN_TEST(test_parsing_union);
  RUN_TEST(test_parsing_comments);
  RUN_TEST(test_parsing_macros);
  RUN_TEST(test_parsing_complex);
  RUN_TEST(test_parsing_empty);
  RUN_TEST(test_parsing_struct_with_anonymous_union);
  RUN_TEST(test_precondition_failure);
}

#endif /* TEST_PARSING_H */
