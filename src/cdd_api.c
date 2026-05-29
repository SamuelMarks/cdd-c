/* clang-format off */
#include "cdd_api.h"
#include "routes/emit/serve_json_rpc.h"
#include "routes/parse/cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */
#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_fail_alloc = 0;
C_CDD_EXPORT int g_cdd_fprintf_fail = 0;
C_CDD_EXPORT int g_cdd_mock_dlopen_success = 0;
#endif

#define MAX_ARGS 32

int cdd_generate_from_openapi(const cdd_from_openapi_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;

  argv[argc++] = "from_openapi";

  if (config->subcommand) {
    argv[argc++] = (char *)config->subcommand;
  } else {
    argv[argc++] = "to_sdk";
  }

  if (config->input) {
    argv[argc++] = "-i";
    argv[argc++] = (char *)config->input;
  } else if (config->input_dir) {
    argv[argc++] = "--input-dir";
    argv[argc++] = (char *)config->input_dir;
  }

  if (config->output) {
    argv[argc++] = "-o";
    argv[argc++] = (char *)config->output;
  }

  if (config->no_github_actions) {
    argv[argc++] = "--no-github-actions";
  }

  if (config->no_installable_package) {
    argv[argc++] = "--no-installable-package";
  }

  if (config->tests) {
    argv[argc++] = "--tests";
  }

  return from_openapi_cli_main(argc, argv);
}

int cdd_generate_to_openapi(const cdd_to_openapi_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;

  argv[argc++] = "to_openapi";

  if (config->input) {
    argv[argc++] = "-i";
    argv[argc++] = (char *)config->input;
  }

  if (config->output) {
    argv[argc++] = "-o";
    argv[argc++] = (char *)config->output;
  }

  return to_openapi_cli_main(argc, argv);
}

int cdd_generate_docs_json(const cdd_docs_json_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;

  argv[argc++] = "to_docs_json";

  if (config->input) {
    argv[argc++] = "-i";
    argv[argc++] = (char *)config->input;
  }

  if (config->output) {
    argv[argc++] = "-o";
    argv[argc++] = (char *)config->output;
  }

  if (config->no_imports) {
    argv[argc++] = "--no-imports";
  }

  if (config->no_wrapping) {
    argv[argc++] = "--no-wrapping";
  }

  return to_docs_json_cli_main(argc, argv);
}

int cdd_serve_json_rpc(const cdd_serve_json_rpc_config_t *config) {
  char *argv[MAX_ARGS];
  int argc = 0;
  char port_str[32];

  argv[argc++] = "serve_json_rpc";

  if (config->port > 0) {
    sprintf(port_str, "%d", config->port);
    argv[argc++] = "-p";
    argv[argc++] = port_str;
  }

  if (config->listen_host) {
    argv[argc++] = "-l";
    argv[argc++] = (char *)config->listen_host;
  }

  return serve_json_rpc_main(argc, argv);
}
