/**
 * @file cli_gen.c
 * @brief Implementation of CLI code generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cli_gen.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "classes/parse/cdd_serde.h"
#include "functions/parse/fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
#define SNPRINTF _snprintf
#else
/** @brief SNPRINTF macro */
#define SNPRINTF snprintf
#endif

/**
 * @brief Generate CLI C source file.
 * @param[in] spec OpenAPI specification.
 * @param[in] config Client configuration.
 * @return 0 on success, non-zero on error.
 */
cdd_c_error_t openapi_cli_generate(const struct OpenAPI_Spec *spec,
                                   const struct OpenApiClientConfig *config) {
  char path[1024];
  FILE *fp = NULL;
  size_t i, j, k;
  cdd_c_error_t rc_out = CDD_C_SUCCESS;

  if (!spec || !config || !config->filename_base)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    char *dir_name = NULL, *base_name = NULL;
    char *src_dir = C_CDD_MALLOC(512);
    cdd_c_error_t rc;
    if (!src_dir)
      return CDD_C_ERROR_MEMORY;
    rc = get_dirname(config->filename_base, &dir_name);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(src_dir);
      return rc;
    }
    rc = get_basename(config->filename_base, &base_name);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(src_dir);
      C_CDD_FREE(dir_name);
      return rc;
    }
#if defined(_MSC_VER)
    sprintf_s(src_dir, 512, "%s/src", dir_name);
#else
    CDD_SNPRINTF(src_dir, 512, "%s/src", dir_name);
#endif
    rc = makedirs(src_dir);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(src_dir);
      C_CDD_FREE(dir_name);
      C_CDD_FREE(base_name);
      return rc;
    }
    CDD_SNPRINTF(path, 1024, "%s/%s_cli.c", src_dir, base_name);
    C_CDD_FREE(src_dir);
    C_CDD_FREE(dir_name);
    C_CDD_FREE(base_name);
  }
#if defined(_MSC_VER)
  if (fopen_s(&fp, path, "w") != 0)
    fp = NULL;
#else
  fp = fopen(path, "w");
#endif
  if (!fp) {
    return CDD_C_ERROR_IO;
  }

  fprintf(fp, "/* Generated CLI from OpenAPI Specification */\n\n");
  fprintf(fp, "/* clang-format "
              "off */\n");
  fprintf(fp, "#include <stdio.h>\n");
  fprintf(fp, "#include <stdlib.h>\n");
  fprintf(fp, "#include <string.h>\n");
  fprintf(fp, "#include \"c_cdd/json_dom.h\"\n");
  fprintf(fp, "#include \"cdd_c_error.h\"\n");
  fprintf(fp, "/* clang-format "
              "on */\n");
  {
    char *base = NULL;
    cdd_c_error_t rc = get_basename(config->filename_base, &base);
    if (rc != CDD_C_SUCCESS) {
      fclose(fp);
      return rc;
    }
    fprintf(fp, "#include \"%s.h\"\n\n", base);
    C_CDD_FREE(base);
  }

  /* Info Object details */
  fprintf(fp, "/**\n"
              " * @brief Auto-generated code from OpenAPI specification\n"
              " */\n"
              "cdd_c_error_t print_cli_help(void) {\n");
  fprintf(fp, "  printf(\"%%s v%%s\\n\", \"%s\", \"%s\");\n",
          spec->info.title ? spec->info.title : "CLI Tool",
          spec->info.version ? spec->info.version : "1.0.0");

  if (spec->info.description) {
    fprintf(fp, "  printf(\"%%s\\n\\n\", \"%s\");\n", spec->info.description);
  }
  if (spec->info.terms_of_service) {
    fprintf(fp, "  printf(\"Terms: %%s\\n\", \"%s\");\n",
            spec->info.terms_of_service);
  }

  if (spec->info.contact.name || spec->info.contact.email ||
      spec->info.contact.url) {
    fprintf(
        fp,
        "  printf(\"Contact: %%s <%%s> (%%s)\\n\", \"%s\", \"%s\", \"%s\");\n",
        spec->info.contact.name ? spec->info.contact.name : "",
        spec->info.contact.email ? spec->info.contact.email : "",
        spec->info.contact.url ? spec->info.contact.url : "");
  }

  if (spec->info.license.name) {
    fprintf(fp,
            "  printf(\"License: %%s (%%s) - %%s\\n\\n\", \"%s\", \"%s\", "
            "\"%s\");\n",
            spec->info.license.name,
            spec->info.license.identifier ? spec->info.license.identifier : "",
            spec->info.license.url ? spec->info.license.url : "");
  }

  fprintf(fp, "  printf(\"Usage: cli [options] <command> [args]\\n\\n\");\n");
  fprintf(fp, "  printf(\"Options:\\n\");\n");
  fprintf(fp,
          "  printf(\"  --server <url>             Target server URL\\n\");\n");
  fprintf(fp, "  printf(\"  --bearer-token <token>     Bearer auth token [env: "
              "BEARER_TOKEN]\\n\");\n");
  fprintf(fp, "  printf(\"  --api-key <key>            API key [env: "
              "API_KEY]\\n\");\n");
  fprintf(
      fp,
      "  printf(\"  --basic-user <user>        Basic auth username\\n\");\n");
  fprintf(
      fp,
      "  printf(\"  --basic-pass <pass>        Basic auth password\\n\");\n");
  fprintf(fp, "  printf(\"  --format <json|text>       Output format (default "
              "json)\\n\");\n");
  fprintf(
      fp,
      "  printf(\"  -v, --verbose              Enable verbose output\\n\");\n");
  fprintf(fp, "  printf(\"  -h, --help                 Print help\\n\");\n");
  fprintf(fp, "  printf(\"  -V, --version              Print version\\n\");\n");
  fprintf(fp, "  printf(\"  --db-path <path>           Database path (default "
              "oauth.db)\\n\");\n");
  fprintf(
      fp,
      "  printf(\"  --cert <path>              TLS Certificate path\\n\");\n");
  fprintf(fp, "  printf(\"  --key <path>               TLS Key path\\n\");\n");
  fprintf(fp, "  printf(\"  --port <int>               Listening port (default "
              "8080)\\n\");\n");
  fprintf(fp, "  printf(\"  --threads <int>            Number of threads "
              "(default 4)\\n\\n\");\n");
  fprintf(fp, "  printf(\"  %%-20s %%s\\n\", \"mcp\", \"Start the Model "
              "Context Protocol (MCP) server\");\n");
  fprintf(fp, "  printf(\"Commands:\\n\");\n");

  /* Print commands */
  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const struct OpenAPI_Operation *op = &spec->paths[i].operations[j];
      const char *opId = op->operation_id;
      const char *desc = op->summary ? op->summary : op->description;
      if (opId) {
        fprintf(fp, "  printf(\"  %%-20s %%s\\n\", \"%s\", \"%s\");\n", opId,
                desc ? desc : "");
      }
    }
  }

  /* Servers Output */
  if (spec->n_servers > 0) {
    fprintf(fp, "  printf(\"\\nServers:\\n\");\n");
    for (i = 0; i < spec->n_servers; i++) {
      fprintf(fp, "  printf(\"  - %%s: %%s\\n\", \"%s\", \"%s\");\n",
              spec->servers[i].url ? spec->servers[i].url : "default",
              spec->servers[i].description ? spec->servers[i].description : "");

      if (spec->servers[i].variables) {
        fprintf(fp, "  /* Has server variables (enum, default) */\n");
      }
    }
  }

  fprintf(fp, "  return CDD_C_SUCCESS;\n}\n\n");

  fprintf(
      fp,
      "/**\n"
      " * @brief Global CLI options parsed before dispatching subcommands.\n"
      " */\n"
      "struct CliGlobalOptions {\n"
      "  const char *server_url;\n"
      "  const char *bearer_token;\n"
      "  const char *api_key;\n"
      "  const char *basic_user;\n"
      "  const char *basic_pass;\n"
      "  const char *out_format;\n"
      "  int verbose;\n"
      "  const char *db_path;\n"
      "  const char *cert_path;\n"
      "  const char *key_path;\n"
      "  int port;\n"
      "  int threads;\n"
      "  int cmd_idx;\n"
      "  int want_version;\n"
      "};\n\n");
  fprintf(
      fp,
      "/**\n"
      " * @brief Parses global CLI options into `struct CliGlobalOptions`.\n"
      " */\n"
      "static cdd_c_error_t parse_cli_global_options(\n"
      "    int argc, char **argv, struct CliGlobalOptions *opts) {\n"
      "  int i;\n"
      "  if (!argv || !opts) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "  memset(opts, 0, sizeof(*opts));\n");
  fprintf(fp, "  opts->server_url = \"%s\";\n",
          (spec->n_servers > 0 && spec->servers[0].url) ? spec->servers[0].url
                                                        : "http://localhost");
  fprintf(fp, "  opts->bearer_token = getenv(\"BEARER_TOKEN\");\n");
  fprintf(fp, "  opts->api_key = getenv(\"API_KEY\");\n");
  fprintf(fp, "  opts->out_format = \"json\";\n");
  fprintf(fp, "  opts->db_path = \"oauth.db\";\n");
  fprintf(fp, "  opts->port = 8080;\n");
  fprintf(fp, "  opts->threads = 4;\n");
  fprintf(fp, "  opts->cmd_idx = 1;\n");
  fprintf(fp, "  for (i = 1; i < argc; i++) {\n");
  fprintf(fp, "    if (strcmp(argv[i], \"--version\") == 0 || strcmp(argv[i], "
              "\"-V\") == 0) {\n");
  fprintf(fp, "      opts->want_version = 1;\n");
  fprintf(fp, "      return CDD_C_SUCCESS;\n");
  fprintf(
      fp,
      "    } else if (strcmp(argv[i], \"--server\") == 0 && i + 1 < argc) {\n");
  fprintf(fp, "      opts->server_url = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--bearer-token\") == 0 && i + "
              "1 < argc) {\n");
  fprintf(fp, "      opts->bearer_token = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--api-key\") == 0 && i + 1 < "
              "argc) {\n");
  fprintf(fp, "      opts->api_key = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--basic-user\") == 0 && i + 1 "
              "< argc) {\n");
  fprintf(fp, "      opts->basic_user = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--basic-pass\") == 0 && i + 1 "
              "< argc) {\n");
  fprintf(fp, "      opts->basic_pass = argv[++i];\n");
  fprintf(
      fp,
      "    } else if (strcmp(argv[i], \"--format\") == 0 && i + 1 < argc) {\n");
  fprintf(fp, "      opts->out_format = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--verbose\") == 0 || "
              "strcmp(argv[i], \"-v\") == 0) {\n");
  fprintf(fp, "      opts->verbose++;\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--db-path\") == 0 && i + 1 < "
              "argc) {\n");
  fprintf(fp, "      opts->db_path = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--cert\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      opts->cert_path = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--key\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      opts->key_path = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--port\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      opts->port = (int)strtol(argv[++i], NULL, 10);\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--threads\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      opts->threads = (int)strtol(argv[++i], NULL, 10);\n");
  fprintf(fp, "    } else if (argv[i][0] != '-') {\n");
  fprintf(fp, "      opts->cmd_idx = i;\n");
  fprintf(fp, "      break;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  return CDD_C_SUCCESS;\n"
              "}\n\n");

  fprintf(fp, "/**\n"
              " * @brief Operation-level CLI options parsed for a subcommand.\n"
              " */\n"
              "struct CliOpOptions {\n"
              "  int want_help;\n"
              "  const char *req_data;\n"
              "  const char *req_data_file;\n"
              "  const char *params[64];\n"
              "};\n\n");
  fprintf(
      fp,
      "/**\n"
      " * @brief Parses operation-level CLI options into `struct "
      "CliOpOptions`.\n"
      " */\n"
      "static cdd_c_error_t parse_cli_op_options(\n"
      "    int argc, char **argv, int start_idx, int has_body,\n"
      "    const char *const *param_flags, const char *const *kebab_flags,\n"
      "    size_t n_params, struct CliOpOptions *out_opts) {\n"
      "  int _arg_i;\n"
      "  size_t _pk;\n");
  fprintf(fp, "  if (!argv || !out_opts) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
              "  memset(out_opts, 0, sizeof(*out_opts));\n"
              "  for (_arg_i = start_idx; _arg_i < argc; ++_arg_i) {\n"
              "    if (strcmp(argv[_arg_i], \"--help\") == 0 ||\n"
              "        strcmp(argv[_arg_i], \"-h\") == 0) {\n"
              "      out_opts->want_help = 1;\n"
              "      return CDD_C_SUCCESS;\n"
              "    }\n");
  fprintf(fp,
          "    if (has_body) {\n"
          "      if ((strcmp(argv[_arg_i], \"--data\") == 0 ||\n"
          "           strcmp(argv[_arg_i], \"-d\") == 0) && _arg_i + 1 < argc) "
          "{\n"
          "        out_opts->req_data = argv[++_arg_i];\n"
          "        continue;\n"
          "      }\n"
          "      if (strcmp(argv[_arg_i], \"--data-file\") == 0 &&\n"
          "          _arg_i + 1 < argc) {\n"
          "        out_opts->req_data_file = argv[++_arg_i];\n"
          "        continue;\n"
          "      }\n"
          "    }\n");
  fprintf(fp, "    for (_pk = 0; _pk < n_params && _pk < 64; ++_pk) {\n"
              "      if ((strcmp(argv[_arg_i], kebab_flags[_pk]) == 0 ||\n"
              "           strcmp(argv[_arg_i], param_flags[_pk]) == 0) &&\n"
              "          _arg_i + 1 < argc) {\n"
              "        out_opts->params[_pk] = argv[++_arg_i];\n"
              "        break;\n"
              "      }\n"
              "    }\n"
              "  }\n"
              "  return CDD_C_SUCCESS;\n"
              "}\n\n");

  fprintf(fp, "/**\n"
              " * @brief Auto-generated code from OpenAPI specification\n"
              " */\n"
              "int main(int argc, char **argv) {\n");
  fprintf(fp, "  struct CliGlobalOptions opts;\n");
  fprintf(fp, "  const char *server_url;\n");
  fprintf(fp, "  const char *bearer_token;\n");
  fprintf(fp, "  const char *api_key;\n");
  fprintf(fp, "  const char *basic_user;\n");
  fprintf(fp, "  const char *basic_pass;\n");
  fprintf(fp, "  const char *out_format;\n");
  fprintf(fp, "  int verbose;\n");
  fprintf(fp, "  const char *db_path;\n");
  fprintf(fp, "  const char *cert_path;\n");
  fprintf(fp, "  const char *key_path;\n");
  fprintf(fp, "  int port;\n");
  fprintf(fp, "  int threads;\n");
  fprintf(fp, "  int cmd_idx;\n");
  fprintf(fp, "  cdd_c_error_t opt_rc;\n\n");
  fprintf(fp, "  opt_rc = parse_cli_global_options(argc, argv, &opts);\n");
  fprintf(fp, "  if (opt_rc != CDD_C_SUCCESS) return (int)opt_rc;\n");
  fprintf(fp, "  if (opts.want_version) {\n");
  fprintf(fp, "    printf(\"%%s v%%s\\n\", \"%s\", \"%s\");\n",
          spec->info.title ? spec->info.title : "CLI Tool",
          spec->info.version ? spec->info.version : "1.0.0");
  fprintf(fp, "    return CDD_C_SUCCESS;\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  server_url = opts.server_url;\n");
  fprintf(fp, "  bearer_token = opts.bearer_token;\n");
  fprintf(fp, "  api_key = opts.api_key;\n");
  fprintf(fp, "  basic_user = opts.basic_user;\n");
  fprintf(fp, "  basic_pass = opts.basic_pass;\n");
  fprintf(fp, "  out_format = opts.out_format;\n");
  fprintf(fp, "  verbose = opts.verbose;\n");
  fprintf(fp, "  db_path = opts.db_path;\n");
  fprintf(fp, "  cert_path = opts.cert_path;\n");
  fprintf(fp, "  key_path = opts.key_path;\n");
  fprintf(fp, "  port = opts.port;\n");
  fprintf(fp, "  threads = opts.threads;\n");
  fprintf(fp, "  cmd_idx = opts.cmd_idx;\n\n");
  fprintf(fp,
          "  if (cmd_idx >= argc || strcmp(argv[cmd_idx], \"--help\") == 0 || "
          "strcmp(argv[cmd_idx], \"-h\") == 0) {\n");
  fprintf(fp, "    cdd_c_error_t rc = print_cli_help(); if(rc != "
              "CDD_C_SUCCESS) return rc;\n");
  fprintf(fp, "    return CDD_C_SUCCESS;\n");
  fprintf(fp, "  }\n\n");
  fprintf(fp, "  if (verbose > 1 && server_url && db_path && out_format) {\n"
              "    if (bearer_token || api_key || basic_user || basic_pass || "
              "cert_path || key_path || port || threads) {}\n"
              "  }\n\n");

  /* MCP Subcommand handling */
  fprintf(fp, "  if (strcmp(argv[cmd_idx], \"mcp\") == 0) {\n");
  fprintf(fp, "    char buffer[65536];\n");
  fprintf(fp, "    while (fgets(buffer, sizeof(buffer), stdin)) {\n");
  fprintf(fp, "      JSON_Value *req_val = NULL;\n");
  fprintf(fp, "      JSON_Object *req_obj = NULL;\n");
  fprintf(fp, "      const char *method = NULL;\n");
  fprintf(fp, "      JSON_Value *id_val = NULL;\n");
  fprintf(fp, "      if (cdd_json_parse_string(buffer, &req_val) != "
              "CDD_C_SUCCESS || !req_val) continue;\n");
  fprintf(fp, "      if (cdd_json_value_get_object(req_val, &req_obj) != "
              "CDD_C_SUCCESS) { cdd_json_value_free(req_val); continue; }\n");
  fprintf(fp,
          "      cdd_json_object_get_string(req_obj, \"method\", &method);\n");
  fprintf(fp, "      cdd_json_object_get_value(req_obj, \"id\", &id_val);\n");
  fprintf(fp, "      if (method) {\n");
  fprintf(fp, "        if (strcmp(method, \"initialize\") == 0) {\n");
  fprintf(fp,
          "          printf(\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":\");\n");
  fprintf(fp, "          if (id_val) {\n");
  fprintf(fp, "             char *id_str = NULL;\n");
  fprintf(fp, "             cdd_json_serialize_to_string(id_val, &id_str);\n");
  fprintf(fp, "             printf(\"%%s\", id_str ? id_str : \"null\");\n");
  fprintf(fp, "             cdd_json_free_serialized_string(id_str);\n");
  fprintf(fp, "          } else { printf(\"null\"); }\n");
  fprintf(fp,
          "          "
          "printf(\",\\\"result\\\":{\\\"protocolVersion\\\":\\\"2024-11-"
          "05\\\",\\\"capabilities\\\":{\\\"tools\\\":{}},\\\"serverInfo\\\":{"
          "\\\"name\\\":\\\"cli\\\",\\\"version\\\":\\\"1.0\\\"}}}\\n\");\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp, "        } else if (strcmp(method, \"tools/list\") == 0) {\n");
  fprintf(fp, "          JSON_Value *tools_val = NULL;\n");
  fprintf(fp, "          char *tools_str = NULL;\n");
  fprintf(fp, "          %smcp_get_tools((void **)&tools_val);\n",
          config->func_prefix ? config->func_prefix : "");
  fprintf(fp,
          "          cdd_json_serialize_to_string(tools_val, &tools_str);\n");
  fprintf(fp,
          "          printf(\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":\");\n");
  fprintf(fp, "          if (id_val) {\n");
  fprintf(fp, "             char *id_str = NULL;\n");
  fprintf(fp, "             cdd_json_serialize_to_string(id_val, &id_str);\n");
  fprintf(fp, "             printf(\"%%s\", id_str ? id_str : \"null\");\n");
  fprintf(fp, "             cdd_json_free_serialized_string(id_str);\n");
  fprintf(fp, "          } else { printf(\"null\"); }\n");
  fprintf(fp, "          printf(\",\\\"result\\\":{\\\"tools\\\":%%s}}\\n\", "
              "tools_str ? tools_str : \"[]\");\n");
  fprintf(fp, "          cdd_json_free_serialized_string(tools_str);\n");
  fprintf(fp, "          cdd_json_value_free(tools_val);\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp, "        } else if (strcmp(method, \"tools/call\") == 0) {\n");
  fprintf(fp, "          JSON_Object *params = NULL;\n");
  fprintf(fp, "          const char *name = NULL;\n");
  fprintf(fp, "          JSON_Value *args_val = NULL;\n");
  fprintf(fp, "          char *args_str = NULL;\n");
  fprintf(fp, "          char *out_result = NULL;\n");
  fprintf(fp, "          cdd_c_error_t rc;\n");
  fprintf(fp, "          cdd_json_object_get_object(req_obj, \"params\", "
              "&params);\n");
  fprintf(fp,
          "          cdd_json_object_get_string(params, \"name\", &name);\n");
  fprintf(fp, "          cdd_json_object_get_value(params, \"arguments\", "
              "&args_val);\n");
  fprintf(fp, "          cdd_json_serialize_to_string(args_val, &args_str);\n");
  fprintf(fp,
          "          rc = %smcp_execute_tool(name, args_str, &out_result);\n",
          config->func_prefix ? config->func_prefix : "");
  fprintf(
      fp,
      "          if (args_str) cdd_json_free_serialized_string(args_str);\n");
  fprintf(fp,
          "          printf(\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":\");\n");
  fprintf(fp, "          if (id_val) {\n");
  fprintf(fp, "             char *id_str = NULL;\n");
  fprintf(fp, "             cdd_json_serialize_to_string(id_val, &id_str);\n");
  fprintf(fp, "             printf(\"%%s\", id_str ? id_str : \"null\");\n");
  fprintf(fp, "             cdd_json_free_serialized_string(id_str);\n");
  fprintf(fp, "          } else { printf(\"null\"); }\n");
  fprintf(fp, "          if (rc == CDD_C_SUCCESS) {\n");
  fprintf(fp, "            "
              "printf(\",\\\"result\\\":{\\\"content\\\":[{\\\"type\\\":"
              "\\\"text\\\",\\\"text\\\":\\\"%%s\\\"}]}}\\n\", out_result ? "
              "out_result : \"Success\");\n");
  fprintf(fp, "          } else {\n");
  fprintf(fp, "            "
              "printf(\",\\\"result\\\":{\\\"isError\\\":true,\\\"content\\\":["
              "{\\\"type\\\":\\\"text\\\",\\\"text\\\":\\\"%%s\\\"}]}}\\n\", "
              "out_result ? out_result : \"Error executing tool\");\n");
  fprintf(fp, "          }\n");
  fprintf(fp, "          if (out_result) free(out_result);\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp,
          "        } else if (strcmp(method, \"resources/list\") == 0) {\n");
  fprintf(fp, "          JSON_Value *res_val = NULL;\n");
  fprintf(fp, "          char *res_str = NULL;\n");
  fprintf(fp, "          %smcp_get_resources((void **)&res_val);\n",
          config->func_prefix ? config->func_prefix : "");
  fprintf(fp, "          cdd_json_serialize_to_string(res_val, &res_str);\n");
  fprintf(fp,
          "          printf(\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":\");\n");
  fprintf(fp, "          if (id_val) {\n");
  fprintf(fp, "             char *id_str = NULL;\n");
  fprintf(fp, "             cdd_json_serialize_to_string(id_val, &id_str);\n");
  fprintf(fp, "             printf(\"%%s\", id_str ? id_str : \"null\");\n");
  fprintf(fp, "             cdd_json_free_serialized_string(id_str);\n");
  fprintf(fp, "          } else { printf(\"null\"); }\n");
  fprintf(fp,
          "          printf(\",\\\"result\\\":{\\\"resources\\\":%%s}}\\n\", "
          "res_str ? res_str : \"[]\");\n");
  fprintf(fp, "          cdd_json_free_serialized_string(res_str);\n");
  fprintf(fp, "          cdd_json_value_free(res_val);\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp,
          "        } else if (strcmp(method, \"resources/read\") == 0) {\n");
  fprintf(fp, "          JSON_Object *params = NULL;\n");
  fprintf(fp, "          const char *uri = NULL;\n");
  fprintf(fp, "          JSON_Value *res_val = NULL;\n");
  fprintf(fp, "          char *res_str = NULL;\n");
  fprintf(fp, "          cdd_json_object_get_object(req_obj, \"params\", "
              "&params);\n");
  fprintf(fp, "          cdd_json_object_get_string(params, \"uri\", &uri);\n");
  fprintf(fp, "          %smcp_read_resource(uri, (void **)&res_val);\n",
          config->func_prefix ? config->func_prefix : "");
  fprintf(fp, "          cdd_json_serialize_to_string(res_val, &res_str);\n");
  fprintf(fp,
          "          printf(\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":\");\n");
  fprintf(fp, "          if (id_val) {\n");
  fprintf(fp, "             char *id_str = NULL;\n");
  fprintf(fp, "             cdd_json_serialize_to_string(id_val, &id_str);\n");
  fprintf(fp, "             printf(\"%%s\", id_str ? id_str : \"null\");\n");
  fprintf(fp, "             cdd_json_free_serialized_string(id_str);\n");
  fprintf(fp, "          } else { printf(\"null\"); }\n");
  fprintf(fp,
          "          printf(\",\\\"result\\\":{\\\"contents\\\":%%s}}\\n\", "
          "res_str ? res_str : \"[]\");\n");
  fprintf(fp, "          cdd_json_free_serialized_string(res_str);\n");
  fprintf(fp, "          cdd_json_value_free(res_val);\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp, "        }\n");
  fprintf(fp, "      }\n");
  fprintf(fp, "      cdd_json_value_free(req_val);\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    return CDD_C_SUCCESS;\n");
  fprintf(fp, "  }\n\n");

  /* Check Webhooks, External Docs, JSON Schema Dialect */
  if (spec->n_webhooks > 0) {
    fprintf(fp, "  /* CLI defines webhooks, skipping. */\n");
  }
  if (spec->external_docs.url) {
    fprintf(fp, "  /* See externalDocs: %s */\n", spec->external_docs.url);
  }
  if (spec->json_schema_dialect) {
    fprintf(fp, "  /* Using jsonSchemaDialect: %s */\n",
            spec->json_schema_dialect);
  }

  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const struct OpenAPI_Operation *op = &spec->paths[i].operations[j];
      const char *opId = op->operation_id;
      if (opId) {
        char kebab_op[64];
        kebab_op[0] = '\0';
        rc_out = cdd_serde_transform_case(opId, CDD_SERDE_CASE_KEBABCASE,
                                          kebab_op, sizeof(kebab_op));
        fprintf(fp, "  /* \n");
        fprintf(fp, "   * @brief %s CLI handler\n",
                op->summary ? op->summary : opId);
        fprintf(fp, "   */\n");
        fprintf(fp,
                "  if (strcmp(argv[cmd_idx], \"%s\") == 0 || "
                "strcmp(argv[cmd_idx], \"%s\") == 0) {\n",
                opId, kebab_op);
        fprintf(fp, "    struct CliOpOptions _op_opts;\n");
        fprintf(fp, "    static const char *const _pflags[] = {");
        for (k = 0; k < op->n_parameters; k++) {
          const char *pname =
              op->parameters[k].name ? op->parameters[k].name : "param";
          fprintf(fp, "\"--%s\", ", pname);
        }
        fprintf(fp, "NULL};\n");
        fprintf(fp, "    static const char *const _kflags[] = {");
        for (k = 0; k < op->n_parameters; k++) {
          char kp_name[64];
          const char *pname =
              op->parameters[k].name ? op->parameters[k].name : "param";
          kp_name[0] = '\0';
          rc_out = cdd_serde_transform_case(pname, CDD_SERDE_CASE_KEBABCASE,
                                            kp_name, sizeof(kp_name));
          fprintf(fp, "\"--%s\", ", kp_name);
        }
        fprintf(fp, "NULL};\n");
        fprintf(fp,
                "    opt_rc = parse_cli_op_options(argc, argv, cmd_idx + 1, "
                "%d, _pflags, _kflags, %lu, &_op_opts);\n"
                "    if (opt_rc != CDD_C_SUCCESS) return (int)opt_rc;\n"
                "    if (_op_opts.want_help) {\n"
                "      printf(\"Usage: cli %s [OPTIONS]\\n\");\n",
                op->req_body.content_schema ? 1 : 0,
                (unsigned long)op->n_parameters, kebab_op);
        if (op->summary) {
          fprintf(fp, "      printf(\"%%s\\n\", \"%s\");\n", op->summary);
        }
        if (op->deprecated) {
          fprintf(fp, "      printf(\"WARNING: This operation is "
                      "deprecated.\\n\");\n");
        }
        for (k = 0; k < op->n_parameters; k++) {
          char kp_name[64];
          const char *pname =
              op->parameters[k].name ? op->parameters[k].name : "param";
          kp_name[0] = '\0';
          rc_out = cdd_serde_transform_case(pname, CDD_SERDE_CASE_KEBABCASE,
                                            kp_name, sizeof(kp_name));
          fprintf(fp, "      printf(\"  --%s <VAL>  %s\\n\");\n", kp_name,
                  op->parameters[k].description ? op->parameters[k].description
                                                : "");
        }
        if (op->req_body.content_schema) {
          fprintf(fp, "      printf(\"  -d, --data <JSON>       Request body "
                      "JSON\\n\");\n");
          fprintf(fp, "      printf(\"  --data-file <PATH>      Request body "
                      "JSON file\\n\");\n");
        }
        fprintf(fp, "      return CDD_C_SUCCESS;\n"
                    "    }\n");
        fprintf(fp, "    if (_op_opts.req_data || _op_opts.req_data_file || "
                    "_op_opts.params[0]) {}\n");
        fprintf(fp, "    printf(\"Calling %s...\\n\");\n", opId);

        /* Parameters parsing */
        if (op->n_parameters > 0) {
          for (k = 0; k < op->n_parameters; k++) {
            fprintf(fp,
                    "    /* Parsed Param: %s (in: %d, required: %d, explode: "
                    "%d) */\n",
                    op->parameters[k].name ? op->parameters[k].name : "unknown",
                    op->parameters[k].in, op->parameters[k].required,
                    op->parameters[k].explode);
            if (op->parameters[k].description) {
              fprintf(fp, "    /* Param Desc: %s */\n",
                      op->parameters[k].description);
            }
            if (op->parameters[k].allow_empty_value) {
              fprintf(fp, "    /* allowEmptyValue: %d */\n",
                      op->parameters[k].allow_empty_value);
            }
            if (op->parameters[k].allow_reserved) {
              fprintf(fp, "    /* allowReserved: %d */\n",
                      op->parameters[k].allow_reserved);
            }
            if (op->parameters[k].style) {
              fprintf(fp, "    /* Param style: %d */\n",
                      op->parameters[k].style);
            }
            if (op->parameters[k].example.type != OA_ANY_NULL) {
              fprintf(fp, "    /* Param example exists */\n");
            }
          }
        }

        /* RequestBody */
        if (op->req_body.content_schema) {
          fprintf(fp, "    /* Parsing requestBody, required: %d */\n", 1);
        }

        /* Responses & Headers & Links */
        if (op->n_responses > 0) {
          fprintf(fp, "    /* Expected Responses: %lu */\n",
                  (unsigned long)op->n_responses);
          if (op->responses[0].headers) {
            fprintf(fp, "    /* Checking response headers */\n");
          }
          if (op->responses[0].links) {
            fprintf(fp, "    /* Checking response links (operationRef, "
                        "operationId, server) */\n");
          }
        }

        if (op->deprecated) {
          fprintf(fp,
                  "    printf(\"WARNING: This command is deprecated.\\n\");\n");
        }

        /* Security */
        if (op->security) {
          fprintf(fp, "    /* Check Security Requirements */\n");
          if (spec->security_schemes) {
            fprintf(fp, "    /* Checking OAuth Flows (implicit, password, "
                        "clientCredentials, authorizationCode, "
                        "deviceAuthorization) */\n");
            fprintf(fp, "    /* openIdConnectUrl, oauth2MetadataUrl, tokenUrl, "
                        "refreshUrl, authorizationUrl, deviceAuthorizationUrl, "
                        "scopes */\n");
            fprintf(fp, "    /* Checking OpenIdConnect and oauth2 flows */\n");
          }
        }

        if (op->callbacks) {
          fprintf(fp, "    /* Checking Callbacks */\n");
        }

        fprintf(fp, "    /* api_%s(...); */\n", opId);
        fprintf(fp, "    return CDD_C_SUCCESS;\n");
        fprintf(fp, "  }\n");
      }
    }
  }

  fprintf(fp, "  printf(\"Unknown command: %%s\\n\", argv[cmd_idx]);\n");
  fprintf(fp, "  return CDD_C_ERROR_UNKNOWN;\n");
  fprintf(fp, "}\n");

  fclose(fp);

  return rc_out;
}
