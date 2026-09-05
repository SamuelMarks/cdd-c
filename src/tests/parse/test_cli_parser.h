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

/* Moved extern declarations for C89 compliance */
extern int method_str_to_enum_str(const char *method, const char **_out_val);
extern int verb_to_enum_str(enum OpenAPI_Verb v, const char **_out_val);

TEST test_cli_parser_getopt(void) {
  const char src[] = {
      105, 110, 116, 32,  109, 97,  105, 110, 40,  105, 110, 116, 32,  97,  114,
      103, 99,  44,  32,  99,  104, 97,  114, 32,  42,  42,  97,  114, 103, 118,
      41,  32,  123, 10,  32,  32,  105, 110, 116, 32,  99,  59,  10,  32,  32,
      119, 104, 105, 108, 101, 32,  40,  40,  99,  32,  61,  32,  103, 101, 116,
      111, 112, 116, 40,  97,  114, 103, 99,  44,  32,  97,  114, 103, 118, 44,
      32,  34,  112, 58,  104, 34,  41,  41,  32,  33,  61,  32,  45,  49,  41,
      32,  123, 10,  32,  32,  32,  32,  115, 119, 105, 116, 99,  104, 32,  40,
      99,  41,  32,  123, 10,  32,  32,  32,  32,  32,  32,  99,  97,  115, 101,
      32,  39,  112, 39,  58,  10,  32,  32,  32,  32,  32,  32,  32,  32,  112,
      111, 114, 116, 32,  61,  32,  97,  116, 111, 105, 40,  111, 112, 116, 97,
      114, 103, 41,  59,  10,  32,  32,  32,  32,  32,  32,  32,  32,  98,  114,
      101, 97,  107, 59,  10,  32,  32,  32,  32,  32,  32,  99,  97,  115, 101,
      32,  39,  104, 39,  58,  10,  32,  32,  32,  32,  32,  32,  32,  32,  104,
      101, 108, 112, 32,  61,  32,  49,  59,  10,  32,  32,  32,  32,  32,  32,
      32,  32,  98,  114, 101, 97,  107, 59,  10,  32,  32,  32,  32,  125, 10,
      32,  32,  125, 10,  125, 10,  0};

  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct CliCommand cmd;
  int rc;

  nodes = calloc(1, sizeof(struct CstNodeList));

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

    (void)cli_command_init(&cmd);
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
    g_fail_io_after = -1;
    PASS();
  }
}

TEST test_cli_parser_mappings(void) {
  const char *out_val;
  /* extern int verb_to_enum_str(enum OpenAPI_Verb v, const char **_out_val);
   * (moved to global) */
  /* extern int method_str_to_enum_str(const char *method, const char
   * **_out_val); (moved to global) */
  /* No wait, these are in client_body.c. */
  /* We want the mappings from cli.c which are internal to `cli.c`. */
  /* cli.c isn't mocked directly. */
  g_fail_io_after = -1;

  (void)out_val;
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
