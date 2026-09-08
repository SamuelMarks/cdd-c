/**
 * @file test_cli_parser.h
 * @brief Unit tests for parsing C CLI argument parsers.
 */

#ifndef TEST_CLI_PARSER_H
#define TEST_CLI_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse/cli_parser.h"
#include "functions/parse/cst.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_strdup_fail;
extern C_CDD_EXPORT cdd_c_error_t test_cli_parser_internal_errors(void);
#endif

/**
 * @brief Test CLI parser getopt extraction and edge cases.
 */
TEST test_cli_parser_getopt(void) {
  const char src[] = "int main(int argc, char **argv) {\n"
                     "  int c;\n"
                     "  while ((c = getopt(argc, argv, \"p:h-az\")) != -1) {\n"
                     "    switch (c) {\n"
                     "      case 99:\n"
                     "        break;\n"
                     "      case 'x':\n"
                     "        break;\n"
                     "      case 'z':\n"
                     "        break;\n"
                     "      case 'p':\n"
                     "        1 = 2;\n"
                     "        port = atoi(optarg);\n"
                     "        break;\n"
                     "      case 'h':\n"
                     "        help = 1;\n"
                     "        break;\n"
                     "      case 'a':\n"
                     "        ;\n"
                     "    }\n"
                     "  }\n"
                     "}\n";

  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct CliCommand cmd;
  int rc;

  nodes = (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));

  {
    az_span span;
    span = az_span_create((uint8_t *)(size_t)src, strlen(src));
    rc = tokenize(span, &tokens);
    (void)rc;
    if (rc != 0) {
      free(nodes);
      FAILm("tokenize failed");
    }

    rc = parse_tokens(tokens, nodes);
    ASSERT_EQ(0, rc);

    /* Test invalid arguments */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cli_command_init(NULL));
    cli_command_free(NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cst_extract_cli_command(NULL, tokens, &cmd));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cst_extract_cli_command(nodes, NULL, &cmd));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cst_extract_cli_command(nodes, tokens, NULL));

    (void)cli_command_init(&cmd);
    rc = cst_extract_cli_command(nodes, tokens, &cmd);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    ASSERT_EQ(4, cmd.n_options);
    ASSERT_EQ('p', cmd.options[0].short_flag);
    ASSERT_EQ(1, cmd.options[0].has_arg);
    ASSERT_STR_EQ("port", cmd.options[0].mapped_struct_field);

    ASSERT_EQ('h', cmd.options[1].short_flag);
    ASSERT_EQ(0, cmd.options[1].has_arg);
    ASSERT_STR_EQ("help", cmd.options[1].mapped_struct_field);

    ASSERT_EQ('a', cmd.options[2].short_flag);
    ASSERT_EQ(0, cmd.options[2].has_arg);

    ASSERT_EQ('z', cmd.options[3].short_flag);
    ASSERT_EQ(0, cmd.options[3].has_arg);

    /* Test freeing options with long_flag and with NULL
     * description/mapped_struct_field */
    cmd.options[0].long_flag = (char *)(size_t)(size_t)strdup("port");
    free(cmd.options[2].description);
    cmd.options[2].description = NULL;

    cli_command_free(&cmd);

    /* Test free with cmd.name == NULL */
    (void)cli_command_init(&cmd);
    cli_command_free(&cmd);

#ifdef CDD_BUILD_TESTS
    {
      struct CliCommand cmd_oom;
      int i;

      ASSERT_EQ(CDD_C_SUCCESS, test_cli_parser_internal_errors());

      /* cmd->name strdup failure */
      g_cdd_strdup_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cst_extract_cli_command(nodes, tokens, &cmd_oom));
      g_cdd_strdup_fail = 0;

      /* add_option realloc failure */
      g_cdd_alloc_fail = 1;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cst_extract_cli_command(nodes, tokens, &cmd_oom));
      g_cdd_alloc_fail = 0;

      /* opt->description strdup failure */
      g_cdd_strdup_fail = 2;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cst_extract_cli_command(nodes, tokens, &cmd_oom));
      g_cdd_strdup_fail = 0;

      /* mapped_struct_field cdd_strndup2 failure */
      for (i = 1; i <= 20; i++) {
        g_cdd_alloc_fail = i;
        (void)cst_extract_cli_command(nodes, tokens, &cmd_oom);
        g_cdd_alloc_fail = 0;
      }
    }
#endif

    /* Test source without getopt */
    {
      const char no_getopt[] = "int foo(void) { return 0; }\n";
      struct TokenList *tok2 = NULL;
      struct CstNodeList *nodes2 =
          (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));
      struct CliCommand cmd2;
      az_span span2 =
          az_span_create((uint8_t *)(size_t)no_getopt, strlen(no_getopt));
      ASSERT_EQ(0, tokenize(span2, &tok2));
      ASSERT_EQ(0, parse_tokens(tok2, nodes2));
      ASSERT_EQ(CDD_C_SUCCESS, cst_extract_cli_command(nodes2, tok2, &cmd2));
      cli_command_free(&cmd2);
      free_cst_node_list(nodes2);
      free(nodes2);
      free_token_list(tok2);
    }

    /* Test getopt without string literal and with tokens->size <= i + 10 */
    {
      const char getopt_no_str[] = "void f() { getopt(); }";
      struct TokenList *tok3 = NULL;
      struct CstNodeList *nodes3 =
          (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));
      struct CliCommand cmd3;
      az_span span3 = az_span_create((uint8_t *)(size_t)getopt_no_str,
                                     strlen(getopt_no_str));
      ASSERT_EQ(0, tokenize(span3, &tok3));
      ASSERT_EQ(0, parse_tokens(tok3, nodes3));
      ASSERT_EQ(CDD_C_SUCCESS, cst_extract_cli_command(nodes3, tok3, &cmd3));
      cli_command_free(&cmd3);
      free_cst_node_list(nodes3);
      free(nodes3);
      free_token_list(tok3);
    }

    /* Test case at end of tokens without char literal */
    {
      const char case_at_end[] = "int f() { getopt(1, 2, \"h\"); case";
      struct TokenList *tok4 = NULL;
      struct CstNodeList *nodes4 =
          (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));
      struct CliCommand cmd4;
      az_span span4 =
          az_span_create((uint8_t *)(size_t)case_at_end, strlen(case_at_end));
      ASSERT_EQ(0, tokenize(span4, &tok4));
      ASSERT_EQ(CDD_C_SUCCESS, cst_extract_cli_command(nodes4, tok4, &cmd4));
      cli_command_free(&cmd4);
      free_cst_node_list(nodes4);
      free(nodes4);
      free_token_list(tok4);
    }

    free_cst_node_list(nodes);
    free(nodes);
    free_token_list(tokens);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Placeholder test for CLI mappings.
 */
TEST test_cli_parser_mappings(void) {
  g_fail_io_after = -1;
  PASS();
}

SUITE(cli_parser_suite) {
  RUN_TEST(test_cli_parser_getopt);
  RUN_TEST(test_cli_parser_mappings);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_PARSER_H */
