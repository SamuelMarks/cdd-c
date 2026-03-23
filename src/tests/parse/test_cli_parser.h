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
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse/cli_parser.h"
#include "functions/parse/cst.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

TEST test_cli_parser_getopt(void) {
  const char *src = ""
                    "int main(int argc, char **argv) {\n"
                    "  int c;\n"
                    "  while ((c = getopt(argc, argv, \"p:h\")) != -1) {\n"
                    "    switch (c) {\n"
                    "      case 'p':\n"
                    "        port = atoi(optarg);\n"
                    "        break;\n"
                    "      case 'h':\n"
                    "        help = 1;\n"
                    "        break;\n"
                    "    }\n"
                    "  }\n"
                    "}\n";

  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct CliCommand cmd;
  int rc;

  nodes = calloc(1, sizeof(struct CstNodeList));

  az_span span = az_span_create((uint8_t *)src, strlen(src));
  rc = tokenize(span, &tokens);
  if (rc != 0) {
    free(nodes);
    FAILm("tokenize failed");
  }

  rc = parse_tokens(tokens, nodes);
  ASSERT_EQ(0, rc);

  cli_command_init(&cmd);
  rc = cst_extract_cli_command(nodes, tokens, &cmd);
  ASSERT_EQ(0, rc);

  if (cmd.n_options != 2) {
    size_t k;
    printf("DEBUG: n_options=%d\n", (int)cmd.n_options);
    for (k = 0; k < cmd.n_options; ++k) {
      printf("DEBUG: opt[%d] flag=%c field=%s\n", (int)k,
             cmd.options[k].short_flag,
             cmd.options[k].mapped_struct_field
                 ? cmd.options[k].mapped_struct_field
                 : "null");
    }
  }

  ASSERT_EQ(2, cmd.n_options);
  ASSERT_EQ('p', cmd.options[0].short_flag);
  ASSERT_EQ(1, cmd.options[0].has_arg);
  ASSERT_STR_EQ("port", cmd.options[0].mapped_struct_field);

  ASSERT_EQ('h', cmd.options[1].short_flag);
  ASSERT_EQ(0, cmd.options[1].has_arg);
  ASSERT_STR_EQ("help", cmd.options[1].mapped_struct_field);

  cli_command_free(&cmd);
  free_cst_node_list(nodes);
  free(nodes);
  free_token_list(tokens);
  PASS();
}

SUITE(cli_parser_suite) { RUN_TEST(test_cli_parser_getopt); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_PARSER_H */
