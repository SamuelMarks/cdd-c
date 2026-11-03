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
 * @return Test or helper result.
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
  int rc = 0;

  nodes = (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));

  {
    az_span span;
    span = az_span_create((uint8_t *)(size_t)src, strlen(src));
    rc = tokenize(span, &tokens);
    ASSERT_EQ(0, rc);

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

    ASSERT_EQ(CDD_C_SUCCESS, cli_command_init(&cmd));
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
    cmd.options[0].long_flag = strdup("port");
    free(cmd.options[2].description);
    cmd.options[2].description = NULL;

    cli_command_free(&cmd);

    /* Test free with cmd.name == NULL */
    ASSERT_EQ(CDD_C_SUCCESS, cli_command_init(&cmd));
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
        rc = cst_extract_cli_command(nodes, tokens, &cmd_oom);
        if (rc != CDD_C_SUCCESS) {
          ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
        }
        g_cdd_alloc_fail = 0;
        cli_command_free(&cmd_oom);
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
 * @brief Tests `getopt_long` `struct option` extraction, `strcmp(argv[i], ...)`
 * extraction, `getenv("...")` fallback association, and
 * `cli_command_to_struct_fields`.
 * @return Test or helper result.
 */
TEST test_cli_parser_mappings(void) {
  const char src[] =
      "static struct option long_opts[] = {\n"
      "  { \"input\", required_argument, 0, 'i' },\n"
      "  { \"verbose\", no_argument, 0, 'v' },\n"
      "  { 0, 0, 0, 0 }\n"
      "};\n"
      "int main(int argc, char **argv) {\n"
      "  const char *input = getenv(\"CDD_INPUT\");\n"
      "  int port = 8080;\n"
      "  int i;\n"
      "  for (i = 1; i < argc; ++i) {\n"
      "    if (strcmp(argv[i], \"--input\") == 0 && i + 1 < argc) {\n"
      "      input = argv[++i];\n"
      "    } else if (strcmp(argv[i], \"--port\") == 0 && i + 1 < argc) {\n"
      "      port = atoi(argv[++i]);\n"
      "    }\n"
      "  }\n"
      "  return 0;\n"
      "}\n";
  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes =
      (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));
  struct CliCommand cmd;
  struct StructFields sf;
  az_span span = az_span_create((uint8_t *)(size_t)src, strlen(src));

  ASSERT_EQ(0, tokenize(span, &tokens));
  ASSERT_EQ(0, parse_tokens(tokens, nodes));
  ASSERT_EQ(CDD_C_SUCCESS, cst_extract_cli_command(nodes, tokens, &cmd));

  ASSERT(cmd.n_options >= 3);
  ASSERT_EQ('i', cmd.options[0].short_flag);
  ASSERT_STR_EQ("input", cmd.options[0].long_flag);
  ASSERT_EQ(1, cmd.options[0].has_arg);
  ASSERT_STR_EQ("input", cmd.options[0].mapped_struct_field);
  ASSERT_STR_EQ("CDD_INPUT", cmd.options[0].env_var);

  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cli_command_to_struct_fields(NULL, &sf));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cli_command_to_struct_fields(&cmd, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cli_command_to_struct_fields(&cmd, &sf));
  ASSERT_EQ(1, sf.is_cli_command);
  ASSERT(sf.size >= 3);
  ASSERT_STR_EQ("input", sf.fields[0].name);
  ASSERT_EQ('i', sf.fields[0].cli_short);
  ASSERT_STR_EQ("input", sf.fields[0].cli_long);
  ASSERT_STR_EQ("CDD_INPUT", sf.fields[0].cli_env);

  struct_fields_free(&sf);
  cli_command_free(&cmd);
  free_cst_node_list(nodes);
  free(nodes);
  free_token_list(tokens);

  /* Exhaustive coverage of cli_parser.c branches */
  {
    struct CliCommand c;
    struct StructFields sf2;
    cli_command_init(&c);
    c.version = strdup("1.0.0");
    c.about = strdup("About CLI");
    c.author = strdup("Author");
    c.subcommands = (struct CliCommand *)calloc(1, sizeof(struct CliCommand));
    c.n_subcommands = 1;
    c.subcommands[0].name = strdup("sub");
    c.options = (struct CliOption *)calloc(5, sizeof(struct CliOption));
    c.n_options = 5;
    c.options[0].default_value = strdup("def");
    c.options[0].value_name = strdup("VAL");
    c.options[0].long_flag = strdup("opt-zero");
    c.options[0].type_str = strdup("integer");
    c.options[0].description = strdup("");
    c.options[0].env_var = strdup("");

    c.options[1].short_flag = 's';
    c.options[1].has_arg = 1;
    c.options[1].value_name = strdup("");

    c.options[2].short_flag = 'b';
    c.options[2].has_arg = 0;

    c.options[3].mapped_struct_field = strdup("");
    c.options[3].long_flag = strdup("");
    c.options[3].short_flag = '\0';

    c.options[4].mapped_struct_field = strdup("valid_field");
    c.options[4].value_name = strdup(" MY_VAL ");

    struct_fields_init(&sf2);
    ASSERT_EQ(CDD_C_SUCCESS, cli_command_to_struct_fields(&c, &sf2));
    struct_fields_free(&sf2);

#ifdef CDD_BUILD_TESTS
    {
      int k;
      for (k = 1; k <= 10; ++k) {
        g_cdd_alloc_fail = k;
        struct_fields_init(&sf2);
        cli_command_to_struct_fields(&c, &sf2);
        g_cdd_alloc_fail = 0;
        struct_fields_free(&sf2);
      }
      for (k = 1; k <= 10; ++k) {
        g_cdd_strdup_fail = k;
        struct_fields_init(&sf2);
        cli_command_to_struct_fields(&c, &sf2);
        g_cdd_strdup_fail = 0;
        struct_fields_free(&sf2);
      }
    }
#endif
    cli_command_free(&c);

    cli_command_init(&c);
    c.options = (struct CliOption *)calloc(2, sizeof(struct CliOption));
    c.n_options = 2;
    c.options[0].short_flag = 'x';
    c.options[1].short_flag = 'y';
    c.options[1].long_flag = strdup("");
#ifdef CDD_BUILD_TESTS
    memset(&sf2, 0, sizeof(sf2));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cli_command_to_struct_fields(&c, &sf2));
    g_cdd_alloc_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cli_command_to_struct_fields(&c, &sf2));
    g_cdd_alloc_fail = 0;
    struct_fields_init(&sf2);
    ASSERT_EQ(CDD_C_SUCCESS, cli_command_to_struct_fields(&c, &sf2));
    struct_fields_free(&sf2);
#endif
    cli_command_free(&c);

    cli_command_init(&c);
    c.name = strdup("");
    c.version = strdup("");
    c.about = strdup("");
    struct_fields_init(&sf2);
    ASSERT_EQ(CDD_C_SUCCESS, cli_command_to_struct_fields(&c, &sf2));
    struct_fields_free(&sf2);
    cli_command_free(&c);
  }

  /* Complex C source with all getopt_long, struct option, strcmp, and getenv
   * edge cases */
  {
    const char *complex_parts[] = {
        "struct { int a; int b; int c; };\n"
        "struct opt123 { int a; int b; int c; };\n"
        "struct not_opt { int x; };\n"
        "struct option fwd_decl;\n"
        "struct /* comment */ option opts[] = {\n"
        "  { 0 },\n"
        "  { \"\", 0, 0, 0 },\n"
        "  { "
        "\"verylongoptionnamethatexceedssixtythreecharacterslimit12345678901234"
        "5\", optional_argument, 0, 'L' },\n"
        "  { \"optarg\", seventeen_char_id, 0, 0 },\n"
        "  { \"reqarg\", required_argument, 0, 'r' },\n"
        "  { \"reqarg\", required_argument, 0, 'r' }\n"
        "};\n",
        "int main(int argc, char **argv) {\n"
        "  int c = getopt_long(argc, argv, \"o::r:\");\n"
        "  if (strcmp(argv[i], \"--help\") == 0) {}\n"
        "  if (strcmp(argv[i], \"--version\") == 0) {}\n"
        "  if (strcmp(argv[i], \"-h\") == 0) {}\n"
        "  if (strcmp(argv[i], \"-V\") == 0) {}\n"
        "  if (strcmp(argv[i], \"-\") == 0) {}\n"
        "  if (strcmp(argv[i], \"--\") == 0) {}\n"
        "  if (strcmp(argv[i], \"-abc\") == 0) {}\n"
        "  if (strcmp(argv[i], \"--a-b\") == 0) {}\n"
        "  if (strcmp(argv[i], \"pos\") == 0) {}\n"
        "  if (strcmp(other, \"--x\") == 0) {}\n"
        "  if (strcmp(argv[i], \"\") == 0) {}\n"
        "}\n",
        "int main2(int argc, char **argv) {\n"
        "  if (strcmp(argx[i], \"--x\") == 0) {}\n"
        "  if (strcmp(argv[i], \"--a\", 123) == 0) {\n"
        "    x = four(1) + sixchr(2);\n"
        "  }\n"
        "  if (strcmp(argv[i], "
        "\"--verylongstrcmpflagnamethatexceedssixtythreecharacterslimit12345678"
        "90\") == 0) {\n"
        "    i = 1;\n"
        "    123 = 456;\n"
        "    "
        "verylongfieldnamethatexceedssixtythreecharacterslimit123456789012345 "
        "= strtol(argv[++i], 0, 10);\n"
        "  }\n"
        "  verylongfieldnamethatexceedssixtythreecharacterslimit123456789012345"
        " = getenv(\"LONG_F_ENV\");\n"
        "}\n",
        "int main3(int argc, char **argv) {\n"
        "  if (strcmp(argv[i], \"-s\") == 0) { short_f = 1; }\n"
        "  if (strcmp(argv[i], \"--short-long\") == 0) { short_f = 1; }\n"
        "  if (strcmp(argv[i], \"-s\") == 0) { short_f = 1; }\n"
        "  if (strcmp(argv[i], \"-r\") == 0) { req_f = argv[++i]; }\n"
        "  short_f = getenv(\"SHORT_ENV\");\n"
        "  short_f = getenv(\"SHORT_ENV_DUP\");\n"
        "}\n",
        "int main4(int argc, char **argv) {\n"
        "  unmapped_f = getenv("
        "\"VERYLONGENVVARNAMETHATEXCEEDSSIXTYTHREECHARACTERSLIMIT12345678901234"
        "5\");\n"
        "  123 = getenv(\"BAD_LHS\");\n"
        "  ; getenv(\"NO_ASSIGN\");\n"
        "  { getenv(\"BRACE_NO_ASSIGN\"); }\n"
        "  getenv(123); getenv(\"\"); getenv;\n"
        "}\n"};
    size_t p_idx;
    for (p_idx = 0; p_idx < sizeof(complex_parts) / sizeof(complex_parts[0]);
         ++p_idx) {
      struct TokenList *tok_c = NULL;
      struct CstNodeList *nodes_c =
          (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));
      struct CliCommand cmd_c;
      az_span sp_c = az_span_create((uint8_t *)(size_t)complex_parts[p_idx],
                                    strlen(complex_parts[p_idx]));
      int k;

      ASSERT_EQ(0, tokenize(sp_c, &tok_c));
      ASSERT_EQ(0, parse_tokens(tok_c, nodes_c));
      ASSERT_EQ(CDD_C_SUCCESS, cst_extract_cli_command(nodes_c, tok_c, &cmd_c));
      cli_command_free(&cmd_c);

#ifdef CDD_BUILD_TESTS
      for (k = 1; k <= 25; ++k) {
        g_cdd_alloc_fail = k;
        if (cst_extract_cli_command(nodes_c, tok_c, &cmd_c) == CDD_C_SUCCESS) {
          cli_command_free(&cmd_c);
        }
        g_cdd_alloc_fail = 0;
      }
      for (k = 1; k <= 25; ++k) {
        g_cdd_strdup_fail = k;
        if (cst_extract_cli_command(nodes_c, tok_c, &cmd_c) == CDD_C_SUCCESS) {
          cli_command_free(&cmd_c);
        }
        g_cdd_strdup_fail = 0;
      }
#endif
      free_cst_node_list(nodes_c);
      free(nodes_c);
      free_token_list(tok_c);
    }
  }

  /* Unclosed EOF edge cases for struct option, strcmp, and getenv */
  {
    const char *eof_cases[] = {
        "int a; int b; int c; int d; struct",
        "struct option x = 1 + 2 + 3 + 4 + 5",
        "int a; int b; int c; int d; struct option x = { {",
        "int a; int b; int c; int d; struct option x = { { \"a\", 0",
        "strcmp(argv[i], \"--x\"",
        "getenv(\"X\")",
        "getenv(",
        "getenv",
        "a = 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + getenv(\"X\")",
        "= getenv(\"X\")",
        "   = getenv(\"X\")",
        "= 1; if (strcmp(argv[i], \"--x\") == 0) { = 2; }"};
    size_t idx;
    for (idx = 0; idx < sizeof(eof_cases) / sizeof(eof_cases[0]); ++idx) {
      struct TokenList *tl = NULL;
      struct CstNodeList nl = {0};
      struct CliCommand cmd_e;
      az_span sp = az_span_create((uint8_t *)(size_t)eof_cases[idx],
                                  strlen(eof_cases[idx]));
      ASSERT_EQ(0, tokenize(sp, &tl));
      ASSERT_EQ(CDD_C_SUCCESS, cst_extract_cli_command(&nl, tl, &cmd_e));
      cli_command_free(&cmd_e);
      free_token_list(tl);
    }
  }

  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the cli_parser_suite test suite.
 */
SUITE(cli_parser_suite) {
  RUN_TEST(test_cli_parser_getopt);
  RUN_TEST(test_cli_parser_mappings);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CLI_PARSER_H */
