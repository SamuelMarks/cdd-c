/**
 * @file test_codegen_cli.h
 * @brief Exhaustive unit and integration test suite for Guided Structured CLI
 * Reflection (`structopt` / `clap` parity).
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLI_H
#define TEST_CODEGEN_CLI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "classes/emit/cli_struct.h"
#include "classes/emit/enum.h"
#include "classes/emit/json.h"
#include "classes/emit/struct.h"
#include "classes/parse/cdd_serde.h"
#include "classes/parse/code2schema.h"
#include "classes/parse/inspector.h"
#include "functions/emit/codegen.h"
#include "functions/emit/sync.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_fprintf_fail;

/* --- Self-contained generated CLI struct for live runtime testing --- */

/** @brief Test CLI struct for live runtime parsing and roundtrip tests. */
struct RuntimeCliApp {
  char *output;           /**< Output path */
  int port;               /**< Port number */
  int color;              /**< Color flag */
  int verbose;            /**< Verbosity count */
  size_t n_tags;          /**< Number of tags */
  char **tags;            /**< Tags array */
  char *mode;             /**< Mode string */
  char *cert;             /**< Cert string */
  char *fast_opt;         /**< Fast option */
  char *slow_opt;         /**< Slow option */
  int solo;               /**< Solo flag */
  char *input_file;       /**< Input file */
  char *extra_after_dash; /**< Extra argument after -- */
};

/**
 * @brief Allocates default RuntimeCliApp instance.
 *
 * @param[out] out Output pointer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t RuntimeCliApp_default(struct RuntimeCliApp **out) {
  struct RuntimeCliApp *p;
  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  p = (struct RuntimeCliApp *)calloc(1, sizeof(struct RuntimeCliApp));
  if (!p)
    return CDD_C_ERROR_MEMORY;
  p->port = 8080;
  p->color = 1;
  *out = p;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a RuntimeCliApp instance.
 *
 * @param[in] p Instance to free.
 */
static void RuntimeCliApp_cleanup(struct RuntimeCliApp *p) {
  size_t i;
  if (!p)
    return;
  if (p->output)
    free(p->output);
  if (p->tags) {
    for (i = 0; i < p->n_tags; ++i) {
      if (p->tags[i])
        free(p->tags[i]);
    }
    free((void *)p->tags);
  }
  if (p->mode)
    free(p->mode);
  if (p->cert)
    free(p->cert);
  if (p->fast_opt)
    free(p->fast_opt);
  if (p->slow_opt)
    free(p->slow_opt);
  if (p->input_file)
    free(p->input_file);
  if (p->extra_after_dash)
    free(p->extra_after_dash);
  free(p);
}

/**
 * @brief Tests `cdd_cli_parse_action`, `cdd_cli_action_to_str`,
 * `cdd_cli_parse_value_hint`, `cdd_cli_value_hint_to_str`, and CLI name/env
 * resolution helpers.
 *
 * @return Greatest test result.
 */
TEST test_cli_action_and_hint_parsers(void) {
  enum CddCliAction act = CDD_CLI_ACTION_AUTO;
  enum CddCliValueHint hint = CDD_CLI_HINT_NONE;
  const char *s = NULL;
  struct StructFields sf;
  struct StructField *f = NULL;
  struct EnumMembers em;
  char buf[64];

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cli_parse_action(NULL, &act));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cli_parse_action("set", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cli_action_to_str(act, NULL));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("auto", &act));
  ASSERT_EQ(CDD_CLI_ACTION_AUTO, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(act, &s));
  ASSERT_STR_EQ("auto", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("set", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(act, &s));
  ASSERT_STR_EQ("set", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("set_true", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET_TRUE, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("true", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET_TRUE, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(act, &s));
  ASSERT_STR_EQ("set_true", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("set_false", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET_FALSE, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("false", &act));
  ASSERT_EQ(CDD_CLI_ACTION_SET_FALSE, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(act, &s));
  ASSERT_STR_EQ("set_false", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("count", &act));
  ASSERT_EQ(CDD_CLI_ACTION_COUNT, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(act, &s));
  ASSERT_STR_EQ("count", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("append", &act));
  ASSERT_EQ(CDD_CLI_ACTION_APPEND, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(act, &s));
  ASSERT_STR_EQ("append", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("help", &act));
  ASSERT_EQ(CDD_CLI_ACTION_HELP, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(act, &s));
  ASSERT_STR_EQ("help", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_action("version", &act));
  ASSERT_EQ(CDD_CLI_ACTION_VERSION, act);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_action_to_str(act, &s));
  ASSERT_STR_EQ("version", s);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_parse_action("unknown_act", &act));

  /* Value hints */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_parse_value_hint(NULL, &hint));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_parse_value_hint("file", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_value_hint_to_str(hint, NULL));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("none", &hint));
  ASSERT_EQ(CDD_CLI_HINT_NONE, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(hint, &s));
  ASSERT_STR_EQ("none", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("file_path", &hint));
  ASSERT_EQ(CDD_CLI_HINT_FILE_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(hint, &s));
  ASSERT_STR_EQ("file", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("dir_path", &hint));
  ASSERT_EQ(CDD_CLI_HINT_DIR_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(hint, &s));
  ASSERT_STR_EQ("dir", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("any_path", &hint));
  ASSERT_EQ(CDD_CLI_HINT_ANY_PATH, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(hint, &s));
  ASSERT_STR_EQ("path", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("executable", &hint));
  ASSERT_EQ(CDD_CLI_HINT_EXECUTABLE, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(hint, &s));
  ASSERT_STR_EQ("executable", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("url", &hint));
  ASSERT_EQ(CDD_CLI_HINT_URL, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(hint, &s));
  ASSERT_STR_EQ("url", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("hostname", &hint));
  ASSERT_EQ(CDD_CLI_HINT_HOSTNAME, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(hint, &s));
  ASSERT_STR_EQ("hostname", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("username", &hint));
  ASSERT_EQ(CDD_CLI_HINT_USERNAME, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(hint, &s));
  ASSERT_STR_EQ("username", s);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_parse_value_hint("email", &hint));
  ASSERT_EQ(CDD_CLI_HINT_EMAIL, hint);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_value_hint_to_str(hint, &s));
  ASSERT_STR_EQ("email", s);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_parse_value_hint("bad_hint", &hint));

  /* Resolution helpers */
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strncpy_s(sf.cli_env_prefix, sizeof(sf.cli_env_prefix), "APP_", _TRUNCATE);
#else
  strncpy(sf.cli_env_prefix, "APP_", sizeof(sf.cli_env_prefix) - 1);
#endif
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "output_path", "string", NULL, NULL, NULL));
  f = &sf.fields[0];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strncpy_s(f->cli_env, sizeof(f->cli_env), "1", _TRUNCATE);
#else
  strncpy(f->cli_env, "1", sizeof(f->cli_env) - 1);
#endif

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cli_resolve_field_long(&sf, f, buf, sizeof(buf)));
  ASSERT_STR_EQ("output-path", buf);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_resolve_field_env(&sf, f, buf, sizeof(buf)));
  ASSERT_STR_EQ("APP_OUTPUT_PATH", buf);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_resolve_value_name(f, buf, sizeof(buf)));
  ASSERT_STR_EQ("OUTPUT_PATH", buf);

  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add_cli_alias(&sf, "run"));
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add_cli_alias(&sf, "run"));
  ASSERT_EQ(1u, sf.n_cli_aliases);

  ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_cli_conflict(f, "other"));
  ASSERT_EQ(1u, f->n_cli_conflicts_with);
  ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_cli_requires(f, "req_f"));
  ASSERT_EQ(1u, f->n_cli_requires);
  ASSERT_EQ(CDD_C_SUCCESS, struct_field_add_cli_required_unless(f, "alt_f"));
  ASSERT_EQ(1u, f->n_cli_required_unless);

  struct_fields_free(&sf);

  /* Enum CLI metadata */
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_init(&em));
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_add(&em, "JSON"));
  ASSERT_EQ(CDD_C_SUCCESS, enum_members_add(&em, "TEXT"));
  ASSERT_EQ(CDD_C_SUCCESS,
            enum_member_set_cli_meta(&em, 0, "JSON output format", 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            enum_member_set_cli_meta(&em, 1, "Hidden format", 1));
  ASSERT_STR_EQ("JSON output format", em.cli_descriptions[0]);
  ASSERT_EQ(1, em.cli_hidden[1]);
  enum_members_free(&em);

  PASS();
}

/**
 * @brief Tests comment directive grammar parsing for `\@cdd cli(...)`,
 * `#[command(...)]`, `#[arg(...)]`, `#[clap(...)]`, `#[structopt(...)]`, and
 * Doxygen fallbacks.
 *
 * @return Greatest test result.
 */
TEST test_cli_comment_directive_parsing(void) {
  struct StructFields sf;
  struct StructField *f;

  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_container_comment(
                "/**\n"
                " * @brief Fast CLI tool\n"
                " * @details Extended multi-line description\n"
                " * @author Author <author@example.com>\n"
                " * @cdd cli(name = \"my-tool\", bin_name = \"my-tool-bin\", "
                "version = \"2.1.0\", before_help = \"Banner\", after_help = "
                "\"Footer\", env_prefix = \"MY_\", arg_required_else_help, "
                "subcommand_required, allow_windows_slash_flags, "
                "allow_response_files, case_insensitive, alias = \"mt\")\n"
                " */",
                &sf));

  ASSERT_EQ(1, sf.is_cli_command);
  ASSERT_STR_EQ("my-tool", sf.cli_name);
  ASSERT_STR_EQ("my-tool-bin", sf.cli_bin_name);
  ASSERT_STR_EQ("2.1.0", sf.cli_version);
  ASSERT_STR_EQ("Author <author@example.com>", sf.cli_author);
  ASSERT_STR_EQ("Fast CLI tool", sf.cli_about);
  ASSERT_STR_EQ("Extended multi-line description", sf.cli_long_about);
  ASSERT_STR_EQ("Banner", sf.cli_before_help);
  ASSERT_STR_EQ("Footer", sf.cli_after_help);
  ASSERT_STR_EQ("MY_", sf.cli_env_prefix);
  ASSERT_EQ(1, sf.cli_arg_required_else_help);
  ASSERT_EQ(1, sf.cli_subcommand_required);
  ASSERT_EQ(1, sf.cli_allow_windows_slash_flags);
  ASSERT_EQ(1, sf.cli_allow_response_files);
  ASSERT_EQ(1, sf.cli_case_insensitive_flags);
  ASSERT_EQ(1u, sf.n_cli_aliases);
  ASSERT_STR_EQ("mt", sf.cli_aliases[0]);

  /* Field directives using #[arg(...)] and @cdd cli(...) */
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "output_file", "string", NULL, NULL, NULL));
  f = &sf.fields[0];
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_serde_parse_field_comment(
          "/**\n"
          " * @brief Output file path\n"
          " * #[arg(short, short_alias = 'O', long, value_name = \"PATH\", "
          "env, hide_env, default_missing_value = \"stdout.txt\", "
          "value_hint = \"file\", num_args = \"1..=3\", require_equals, "
          "allow_hyphen_values, global, conflicts_with = \"quiet\", "
          "requires = \"config\", required_unless_present = \"stdin_mode\", "
          "required_if_eq = (\"mode\", \"tls\"), group = \"out_grp\")]\n"
          " */",
          f));

  ASSERT_EQ('o', f->cli_short);
  ASSERT_STR_EQ("O", f->cli_short_aliases);
  ASSERT_EQ(1, f->cli_has_long);
  ASSERT_STR_EQ("output-file", f->cli_long);
  ASSERT_STR_EQ("PATH", f->cli_value_name);
  ASSERT_STR_EQ("Output file path", f->cli_help);
  ASSERT_STR_EQ("OUTPUT_FILE", f->cli_env);
  ASSERT_EQ(1, f->cli_hide_env);
  ASSERT_STR_EQ("stdout.txt", f->cli_default_missing_val);
  ASSERT_EQ(CDD_CLI_HINT_FILE_PATH, f->cli_value_hint);
  ASSERT_EQ(1, f->cli_num_args_min);
  ASSERT_EQ(3, f->cli_num_args_max);
  ASSERT_EQ(1, f->cli_require_equals);
  ASSERT_EQ(1, f->cli_allow_hyphen_values);
  ASSERT_EQ(1, f->cli_global);
  ASSERT_EQ(1u, f->n_cli_conflicts_with);
  ASSERT_STR_EQ("quiet", f->cli_conflicts_with[0]);
  ASSERT_EQ(1u, f->n_cli_requires);
  ASSERT_STR_EQ("config", f->cli_requires[0]);
  ASSERT_EQ(1u, f->n_cli_required_unless);
  ASSERT_STR_EQ("stdin_mode", f->cli_required_unless[0]);
  ASSERT_STR_EQ("mode", f->cli_required_if_field);
  ASSERT_STR_EQ("tls", f->cli_required_if_val);
  ASSERT_STR_EQ("out_grp", f->cli_group);

  struct_fields_free(&sf);
  PASS();
}

/**
 * @brief Tests Levenshtein typo suggestion (`cdd_cli_suggest_closest`).
 *
 * @return Greatest test result.
 */
TEST test_cli_levenshtein_suggestions(void) {
  static const char *const cands[] = {"--output", "--verbose", "--port",
                                      "--format", "deploy"};
  const char *best = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_suggest_closest(NULL, cands, 5, &best));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_suggest_closest("--ouput", NULL, 5, &best));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cli_suggest_closest("--ouput", cands, 5, NULL));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_suggest_closest("--ouput", cands, 5, &best));
  ASSERT(best != NULL);
  ASSERT_STR_EQ("--output", best);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cli_suggest_closest("--vervose", cands, 5, &best));
  ASSERT(best != NULL);
  ASSERT_STR_EQ("--verbose", best);

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_cli_suggest_closest("--prot=8080", cands, 5, &best));
  ASSERT(best != NULL);
  ASSERT_STR_EQ("--port", best);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_cli_suggest_closest(
                               "--completely-unrelated-flag", cands, 5, &best));
  ASSERT_EQ(NULL, best);

  PASS();
}

/**
 * @brief Tests `@args.rsp` response file expansion
 * (`cdd_cli_expand_response_files`), including nested response files and cycle
 * detection.
 *
 * @return Greatest test result.
 */
TEST test_cli_response_file_expansion(void) {
  FILE *fp;
  int out_argc = 0;
  char **out_argv = NULL;

  /* Write inner.rsp and outer.rsp */
  fp = fopen("test_inner.rsp", "wb");
  ASSERT(fp != NULL);
  fputs("# comment line\n--port 9090\n'quoted value'\n", fp);
  fclose(fp);

  fp = fopen("test_outer.rsp", "wb");
  ASSERT(fp != NULL);
  fputs("--output \"my file.txt\" @test_inner.rsp\n", fp);
  fclose(fp);

  {
    char *in_argv[3];
    in_argv[0] = cdd_test_mut_str("prog");
    in_argv[1] = cdd_test_mut_str("@test_outer.rsp");
    in_argv[2] = cdd_test_mut_str("--verbose");

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_cli_expand_response_files(3, in_argv, &out_argc, &out_argv));
    ASSERT_EQ(7, out_argc);
    ASSERT_STR_EQ("prog", out_argv[0]);
    ASSERT_STR_EQ("--output", out_argv[1]);
    ASSERT_STR_EQ("my file.txt", out_argv[2]);
    ASSERT_STR_EQ("--port", out_argv[3]);
    ASSERT_STR_EQ("9090", out_argv[4]);
    ASSERT_STR_EQ("quoted value", out_argv[5]);
    ASSERT_STR_EQ("--verbose", out_argv[6]);
    cdd_cli_free_argv(out_argc, out_argv);
  }

  /* Test self-referencing cycle rejection */
  fp = fopen("test_cycle.rsp", "wb");
  ASSERT(fp != NULL);
  fputs("@test_cycle.rsp\n", fp);
  fclose(fp);
  {
    char *cyc_argv[2];
    cyc_argv[0] = cdd_test_mut_str("prog");
    cyc_argv[1] = cdd_test_mut_str("@test_cycle.rsp");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cli_expand_response_files(2, cyc_argv, &out_argc, &out_argv));
  }

  remove("test_inner.rsp");
  remove("test_outer.rsp");
  remove("test_cycle.rsp");
  PASS();
}

/**
 * @brief Tests generating all CLI functions, OpenAPI `x-cdd-cli-*` roundtrip,
 * header emission, and `sync_code` integration.
 *
 * @return Greatest test result.
 */
TEST test_cli_codegen_and_schema_roundtrip(void) {
  struct StructFields sf;
  struct StructFields sf_loaded;
  struct CodegenCliConfig cli_cfg;
  JSON_Value *root_val = json_value_init_object();
  JSON_Object *schemas_obj = json_value_get_object(root_val);
  FILE *fp;
  long sz;
  char *code = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_serde_parse_container_comment(
          "/* @cdd cli(name = \"demo\", version = \"1.2.3\", about = \"Demo "
          "CLI\", author = \"Author\", env_prefix = \"DEMO_\", "
          "allow_windows_slash_flags) */",
          &sf));

  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_add(&sf, "output", "string", NULL,
                                             "\"out.json\"", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* @cdd cli(short = 'o', long = \"output\", env = \"OUT\", "
                "value_name = \"FILE\", help = \"Output path\") */",
                &sf.fields[0]));

  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "verbose", "integer", NULL, "0", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* @cdd cli(short = 'v', long = \"verbose\", action = "
                "\"count\") */",
                &sf.fields[1]));

  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "color", "boolean", NULL, "1", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* @cdd cli(long = \"color\", negatable) */", &sf.fields[2]));

  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "tags", "array", "string", NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_serde_parse_field_comment(
                "/* @cdd cli(short = 't', long = \"tag\", value_delimiter = "
                "',') */",
                &sf.fields[3]));

  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "input", "string", NULL, NULL, NULL));
  ASSERT_EQ(
      CDD_C_SUCCESS,
      cdd_serde_parse_field_comment(
          "/* @cdd cli(positional, index = 0, required) */", &sf.fields[4]));

  memset(&cli_cfg, 0, sizeof(cli_cfg));
  cli_cfg.guard_macro = "DEMO_CLI_GUARD";

  fp = tmpfile();
  ASSERT(fp != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_cli_declarations_h(fp, "DemoCli", &cli_cfg));
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_cli_all_funcs(fp, "DemoCli", &sf, &cli_cfg));
  fseek(fp, 0, SEEK_END);
  sz = ftell(fp);
  ASSERT(sz > 0);
  fseek(fp, 0, SEEK_SET);
  code = (char *)calloc(1, (size_t)sz + 1);
  ASSERT(code != NULL);
  fread(code, 1, (size_t)sz, fp);
  fclose(fp);

  ASSERT(strstr(code, "DemoCli_print_version(") != NULL);
  ASSERT(strstr(code, "DemoCli_print_help(") != NULL);
  ASSERT(strstr(code, "DemoCli_print_completions(") != NULL);
  ASSERT(strstr(code, "DemoCli_parse_cli_into(") != NULL);
  ASSERT(strstr(code, "DemoCli_from_cli(") != NULL);
  ASSERT(strstr(code, "DemoCli_to_cli_argv(") != NULL);
  ASSERT(strstr(code, "--[no-]color") != NULL);
  ASSERT(strstr(code, "[env: DEMO_OUT]") != NULL);
  ASSERT(strstr(code, "Register-ArgumentCompleter") != NULL);
  free(code);

  /* JSON Schema x-cdd-cli-* roundtrip */
  ASSERT_EQ(CDD_C_SUCCESS,
            write_struct_to_json_schema(schemas_obj, "DemoCli", &sf));
  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf_loaded));
  ASSERT_EQ(CDD_C_SUCCESS, json_object_to_struct_fields_ex(
                               json_object_get_object(schemas_obj, "DemoCli"),
                               &sf_loaded, schemas_obj, "DemoCli"));

  ASSERT_EQ(1, sf_loaded.is_cli_command);
  ASSERT_STR_EQ("demo", sf_loaded.cli_name);
  ASSERT_STR_EQ("1.2.3", sf_loaded.cli_version);
  ASSERT_STR_EQ("Demo CLI", sf_loaded.cli_about);
  ASSERT_STR_EQ("DEMO_", sf_loaded.cli_env_prefix);
  ASSERT_EQ('o', sf_loaded.fields[0].cli_short);
  ASSERT_STR_EQ("output", sf_loaded.fields[0].cli_long);
  ASSERT_EQ(CDD_CLI_ACTION_COUNT, sf_loaded.fields[1].cli_action);
  ASSERT_EQ(1, sf_loaded.fields[2].cli_negatable);
  ASSERT_EQ(',', sf_loaded.fields[3].cli_value_delimiter);
  ASSERT_EQ(1, sf_loaded.fields[4].cli_positional);
  ASSERT_EQ(0, sf_loaded.fields[4].cli_index);

  struct_fields_free(&sf_loaded);
  struct_fields_free(&sf);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Prints RuntimeCliApp version string.
 *
 * @param[in] prog_name Program name.
 * @param[in] out_stream Output stream.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t RuntimeCliApp_print_version(const char *prog_name,
                                                 FILE *out_stream) {
  const char *_cdd_prog =
      (prog_name && prog_name[0] != '\0') ? prog_name : "runtime-cli";
  if (!out_stream)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (fprintf(out_stream, "%s 1.0.0\n", _cdd_prog) < 0)
    return CDD_C_ERROR_IO;
  return CDD_C_SUCCESS;
}

/**
 * @brief Prints RuntimeCliApp help message.
 *
 * @param[in] prog_name Program name.
 * @param[in] out_stream Output stream.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t RuntimeCliApp_print_help(const char *prog_name,
                                              FILE *out_stream) {
  const char *_cdd_prog =
      (prog_name && prog_name[0] != '\0') ? prog_name : "runtime-cli";
  if (!out_stream)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (fprintf(out_stream, "Usage: %s [OPTIONS] [INPUT_FILE]\n", _cdd_prog) < 0)
    return CDD_C_ERROR_IO;
  return CDD_C_SUCCESS;
}

/**
 * @brief Tests `sync_code_main` regenerating CLI functions from an annotated
 * header file and verifying all generated functions are present.
 *
 * @return Greatest test result.
 */
TEST test_cli_sync_code_regeneration(void) {
  FILE *f;
  long sz;
  char *c_buf = NULL;
  char *sync_argv[3];

  f = fopen("test_cli_sync_hdr.h", "wb");
  ASSERT(f != NULL);
  fputs(
      "/**\n"
      " * @brief SyncCli test struct\n"
      " * @cdd cli(name = \"sync-cli\", version = \"3.0.0\", about = \"Sync "
      "tool\", allow_windows_slash_flags)\n"
      " */\n"
      "struct SyncCli {\n"
      "  /* @cdd cli(short = 'o', long = \"output\", env = \"SYNC_OUT\") */\n"
      "  char *output;\n"
      "  /* @cdd cli(short = 'v', long = \"verbose\", action = \"count\") */\n"
      "  int verbose;\n"
      "  /* @cdd cli(short = 'p', long = \"port\", default = \"8080\") */\n"
      "  int port;\n"
      "  /* @cdd cli(long = \"color\", negatable) */\n"
      "  int color;\n"
      "  /* @cdd cli(positional, index = 0) */\n"
      "  char *target;\n"
      "};\n",
      f);
  fclose(f);

  sync_argv[0] = cdd_test_mut_str("test_cli_sync_hdr.h");
  sync_argv[1] = cdd_test_mut_str("test_cli_sync_impl.c");
  sync_argv[2] = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, sync_code_main(2, sync_argv));

  f = fopen("test_cli_sync_impl.c", "rb");
  ASSERT(f != NULL);
  fseek(f, 0, SEEK_END);
  sz = ftell(f);
  ASSERT(sz > 0);
  fseek(f, 0, SEEK_SET);
  c_buf = (char *)calloc(1, (size_t)sz + 1);
  ASSERT(c_buf != NULL);
  fread(c_buf, 1, (size_t)sz, f);
  fclose(f);

  ASSERT(strstr(c_buf, "SyncCli_from_cli(") != NULL);
  ASSERT(strstr(c_buf, "SyncCli_parse_cli_into(") != NULL);
  ASSERT(strstr(c_buf, "SyncCli_to_cli_argv(") != NULL);
  ASSERT(strstr(c_buf, "SyncCli_print_help(") != NULL);
  ASSERT(strstr(c_buf, "SyncCli_print_version(") != NULL);
  ASSERT(strstr(c_buf, "SyncCli_print_completions(") != NULL);
  free(c_buf);

  remove("test_cli_sync_hdr.h");
  remove("test_cli_sync_impl.c");
  PASS();
}

/**
 * @brief Tests I/O failure (`g_cdd_fprintf_fail`) and invalid argument branches
 * in `cli_struct.c`.
 *
 * @return Greatest test result.
 */
TEST test_cli_io_and_invalid_arg_branches(void) {
  struct StructFields sf;
  struct CodegenCliConfig cfg;
  FILE *fp;
  int k;

  ASSERT_EQ(CDD_C_SUCCESS, struct_fields_init(&sf));
  sf.is_cli_command = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            struct_fields_add(&sf, "flag", "boolean", NULL, NULL, NULL));
  sf.fields[0].cli_short = 'f';

  memset(&cfg, 0, sizeof(cfg));
  cfg.guard_macro = "G_CLI";

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_cli_declarations_h(NULL, "S", &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_print_version_func(NULL, "S", &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_print_help_func(NULL, "S", &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_print_completions_func(NULL, "S", &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_cli_parse_into_func(NULL, "S", &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_from_cli_func(NULL, "S", &sf, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_struct_to_cli_argv_func(NULL, "S", &sf, &cfg));

  for (k = 1; k <= 30; ++k) {
    fp = tmpfile();
    if (!fp)
      break;
    g_cdd_fprintf_fail = k;
    write_struct_cli_all_funcs(fp, "S", &sf, &cfg);
    g_cdd_fprintf_fail = 0;
    fclose(fp);
  }

  if (RuntimeCliApp_print_version("app", NULL) != CDD_C_ERROR_INVALID_ARGUMENT)
    FAIL();
  if (RuntimeCliApp_print_help("app", NULL) != CDD_C_ERROR_INVALID_ARGUMENT)
    FAIL();
  {
    struct RuntimeCliApp *app = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, RuntimeCliApp_default(&app));
    RuntimeCliApp_cleanup(app);
  }

  struct_fields_free(&sf);
  PASS();
}

/**
 * @brief Suite for Guided Structured CLI Reflection code generation.
 */
SUITE(codegen_cli_suite) {
  RUN_TEST(test_cli_action_and_hint_parsers);
  RUN_TEST(test_cli_comment_directive_parsing);
  RUN_TEST(test_cli_levenshtein_suggestions);
  RUN_TEST(test_cli_response_file_expansion);
  RUN_TEST(test_cli_codegen_and_schema_roundtrip);
  RUN_TEST(test_cli_sync_code_regeneration);
  RUN_TEST(test_cli_io_and_invalid_arg_branches);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLI_H */
