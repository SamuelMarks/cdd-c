#ifndef TEST_CDD_API_H
#define TEST_CDD_API_H

/* clang-format off */
#include "cdd_api.h"
#include <greatest.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

TEST test_cdd_generate_from_openapi(void) {
  cdd_from_openapi_config_t config = {0};

  config.input = "nonexistent.json";
  config.output = "out";
  config.subcommand = "to_sdk";
  cdd_generate_from_openapi(&config);

  config.subcommand = NULL;
  config.input = NULL;
  config.input_dir = "nonexistent_dir";
  config.no_github_actions = 1;
  config.no_installable_package = 1;
  config.tests = 1;
  cdd_generate_from_openapi(&config);

  /* Cover branches where flags are false but input_dir is not set, input not
   * set */
  config.input_dir = NULL;
  config.no_github_actions = 0;
  config.no_installable_package = 0;
  config.tests = 0;
  config.output = NULL;
  cdd_generate_from_openapi(&config);

  PASS();
}

TEST test_cdd_generate_to_openapi(void) {
  cdd_to_openapi_config_t config = {0};

  config.input = "nonexistent.c";
  config.output = "out.json";
  cdd_generate_to_openapi(&config);

  config.input = NULL;
  config.output = NULL;
  cdd_generate_to_openapi(&config);

  PASS();
}

TEST test_cdd_generate_docs_json(void) {
  cdd_docs_json_config_t config = {0};

  config.input = "nonexistent.json";
  config.output = "out.json";
  config.no_imports = 1;
  config.no_wrapping = 1;
  cdd_generate_docs_json(&config);

  config.input = NULL;
  config.no_imports = 0;
  config.no_wrapping = 0;
  config.output = NULL;
  cdd_generate_docs_json(&config);

  PASS();
}

TEST test_cdd_serve_json_rpc(void) {
  cdd_serve_json_rpc_config_t config = {0};

  config.port = 1; /* port > 0 */
  config.listen_host = "255";
  cdd_serve_json_rpc(&config);

  config.port = 0;
  config.listen_host = NULL;
  cdd_serve_json_rpc(&config);

  PASS();
}

extern volatile int g_ffi_extractor_alloc_fail;
extern int g_cdd_ffi_ir_calloc_fail;

TEST test_cdd_generate_bindings(void) {
  cdd_generate_bindings_config_t config = {0};
  FILE *f;
  const char *langs[] = {
      "python",      "rust",    "csharp", "typescript",  "napi",
      "java",        "cpp",     "go",     "swift",       "dart",
      "ruby",        "kotlin",  "php",    "lua",         "zig",
      "odin",        "julia",   "r",      "matlab",      "haskell",
      "ocaml",       "elixir",  "erlang", "common_lisp", "racket",
      "scheme",      "scala",   "fsharp", "clojure",     "groovy",
      "webassembly", "nim",     "vlang",  "dlang",       "perl",
      "tcl",         "fortran", "delphi", "ada",         "objc",
      "crystal",     "all",     "*"};
  size_t i;

  /* NULL config / args */
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_generate_bindings(NULL));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_generate_bindings(&config));

  config.input = "nonexistent.c";
  config.output_dir = "out_dir";
  config.target_langs = "all";
  /* Read file failure */
  ASSERT_NEQ(0, cdd_generate_bindings(&config));

  /* Create a valid dummy file */
  f = fopen("test_dummy_bindings.h", "w");
  if (f) {
    fprintf(f, "/* dummy */\nstruct Point { int x; };\n");
    fclose(f);
  }

  config.input = "test_dummy_bindings.h";

  /* Extractor failure */
  g_ffi_extractor_alloc_fail = 1;
  ASSERT_NEQ(0, cdd_generate_bindings(&config));
  g_ffi_extractor_alloc_fail = 0;

  /* Sort failure */
  g_cdd_ffi_ir_calloc_fail = 1;
  ASSERT_NEQ(0, cdd_generate_bindings(&config));
  g_cdd_ffi_ir_calloc_fail = 0;

  /* Test failure branch (rc != 0) for each language */
  config.output_dir = "nonexistent_dir_12345/nonexistent";
  for (i = 0; i < sizeof(langs) / sizeof(langs[0]); i++) {
    config.target_langs = langs[i];
    ASSERT_NEQ(0, cdd_generate_bindings(&config));
  }

  /* Test success branch for each language */
  config.output_dir = ".";
  for (i = 0; i < sizeof(langs) / sizeof(langs[0]); i++) {
    config.target_langs = langs[i];
    int rc = cdd_generate_bindings(&config);
    if (rc != 0) {
      printf("Failed for language: %s, rc = %d\n", langs[i], rc);
    }
    ASSERT_EQ(0, rc);
  }

  remove("test_dummy_bindings.h");

  PASS();
}

SUITE(cdd_api_suite) {
  RUN_TEST(test_cdd_generate_from_openapi);
  RUN_TEST(test_cdd_generate_to_openapi);
  RUN_TEST(test_cdd_generate_docs_json);
  RUN_TEST(test_cdd_serve_json_rpc);
  RUN_TEST(test_cdd_generate_bindings);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
