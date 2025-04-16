#ifndef TEST_PARSING_H
#define TEST_PARSING_H

#include "cdd_helpers.h"

#include <greatest.h>
#include <stdio.h>
#include <string.h>

#include "cst_parser.h"
#include "tokenizer.h"

#include <c_str_precondition.h>

static void print_span(const char *const start, const size_t length) {
  size_t i;
  putchar('"');
  for (i = 0; i < length; i++) {
    const char c = start[i];
    switch (c) {
    case '\n':
      puts("\\n");
      break;
    case '\r':
      puts("\\r");
      break;
    case '\t':
      puts("\\t");
      break;
    case '"':
      puts("\\\"");
      break;
    default:
      putchar(c);
    }
  }
  putchar('"');
}

TEST parsing_test(const char *const test_name, const char *const source,
                  const size_t expected_structs, const size_t expected_enums,
                  const size_t expected_unions, const size_t expected_comments,
                  const size_t expected_macros,
                  const size_t expected_whitespace) {
  struct TokenList *tokens = calloc(1, sizeof *tokens);
  struct CstNodeList *cst_nodes = calloc(1, sizeof *cst_nodes);
  size_t s_count = 0, e_count = 0, u_count = 0, cm_count = 0, m_count = 0,
         w_count = 0;
  size_t i;
  int rc = EXIT_SUCCESS;

  printf("Running test: %s\n", test_name);

  if (tokenize(source, strlen(source), tokens) != 0) {
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
    case CST_NODE_WHITESPACE:
      w_count++;
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
  ASSERT_GTEm("whitespace count mismatch (at least expected)", w_count,
              expected_whitespace);

  printf("Test '%s' passed.\n", test_name);

cleanup:
  free_token_list(tokens);
  free_cst_node_list(cst_nodes);
  free(tokens);
  free(cst_nodes);
  if (rc == EXIT_SUCCESS)
    PASS();
  FAIL();
}

TEST x_test_parsing_struct(void) {
  /* Struct test */
  parsing_test("Struct parsing", "struct Point { int x; int y; };",
               1, /* structs */
               0, /* enums */
               0, /* unions */
               0, /* comments */
               0, /* macros */
               1  /* whitespace tokens expected (spaces) */
  );
  PASS();
}

TEST x_test_parsing_enum(void) {
  /* Enum test */
  parsing_test("Enum parsing", "enum Color { RED, GREEN, BLUE };", 0, 1, 0, 0,
               0, 1);
  PASS();
}

TEST x_test_parsing_union(void) {
  /* Union test */
  parsing_test("Union parsing", "union Data { int i; float f; };", 0, 0, 1, 0,
               0, 1);
  PASS();
}

TEST x_test_parsing_comments(void) {
  /* Comments test */
  parsing_test("Comments parsing",
               "/* comment block */\n// line comment\nint x;", 0, 0, 0,
               2,   /* one block and one line comment */
               0, 3 /* some whitespace tokens: newlines/spaces */
  );
  PASS();
}

TEST x_test_parsing_macros(void) {
  /* Macros test */
  parsing_test("Macros parsing", "#define MAX 100\nint a;", 0, 0, 0, 0, 1, 2);
  PASS();
}

TEST x_test_parsing_complex(void) {
  /* Complex test */
  parsing_test("Complex parsing",
               "/* block comment */\n"
               "#include <stdio.h>\n"
               "struct S { int a; union U { float f; int i; } u; };\n"
               "enum E { X, Y, Z };\n"
               "// single line comment\n"
               "int main() { return 0; }\n",
               1, /* structs */
               1, /* enums */
               1, /* unions */
               2, /* 1 block + 1 line */
               1, /* macro (#include) */
               7  /* whitespace tokens */
  );

  PASS();
}

SUITE(parsing_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_parsing_struct);
  RUN_TEST(x_test_parsing_enum);
  RUN_TEST(x_test_parsing_union);
  RUN_TEST(x_test_parsing_comments);
  RUN_TEST(x_test_parsing_macros);
  RUN_TEST(x_test_parsing_complex);
}

#endif /* TEST_PARSING_H */
