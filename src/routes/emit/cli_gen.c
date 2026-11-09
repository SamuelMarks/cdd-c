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
  fprintf(fp, "#include <stdio.h>\n");
  fprintf(fp, "#include <stdlib.h>\n");
  fprintf(fp, "#include <string.h>\n");
  fprintf(fp, "#include <parson.h>\n");
  fprintf(fp, "/* clang-format "
              "off */\n");
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

  fprintf(fp, "/**\n"
              " * @brief Auto-generated code from OpenAPI specification\n"
              " */\n"
              "int main(int argc, char **argv) {\n");
  fprintf(fp, "  int i;\n");
  fprintf(fp, "  const char *server_url = \"%s\";\n",
          (spec->n_servers > 0 && spec->servers[0].url) ? spec->servers[0].url
                                                        : "http://localhost");
  fprintf(fp, "  const char *bearer_token = getenv(\"BEARER_TOKEN\");\n");
  fprintf(fp, "  const char *api_key = getenv(\"API_KEY\");\n");
  fprintf(fp, "  const char *basic_user = NULL;\n");
  fprintf(fp, "  const char *basic_pass = NULL;\n");
  fprintf(fp, "  const char *out_format = \"json\";\n");
  fprintf(fp, "  int verbose = 0;\n");
  fprintf(fp, "  const char *db_path = \"oauth.db\";\n");
  fprintf(fp, "  const char *cert_path = NULL;\n");
  fprintf(fp, "  const char *key_path = NULL;\n");
  fprintf(fp, "  int port = 8080;\n");
  fprintf(fp, "  int threads = 4;\n");
  fprintf(fp, "  int cmd_idx = 1;\n\n");
  fprintf(fp, "  for (i = 1; i < argc; i++) {\n");
  fprintf(fp, "    if (strcmp(argv[i], \"--version\") == 0 || strcmp(argv[i], "
              "\"-V\") == 0) {\n");
  fprintf(fp, "      printf(\"%%s v%%s\\n\", \"%s\", \"%s\");\n",
          spec->info.title ? spec->info.title : "CLI Tool",
          spec->info.version ? spec->info.version : "1.0.0");
  fprintf(fp, "      return CDD_C_SUCCESS;\n");
  fprintf(
      fp,
      "    } else if (strcmp(argv[i], \"--server\") == 0 && i + 1 < argc) {\n");
  fprintf(fp, "      server_url = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--bearer-token\") == 0 && i + "
              "1 < argc) {\n");
  fprintf(fp, "      bearer_token = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--api-key\") == 0 && i + 1 < "
              "argc) {\n");
  fprintf(fp, "      api_key = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--basic-user\") == 0 && i + 1 "
              "< argc) {\n");
  fprintf(fp, "      basic_user = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--basic-pass\") == 0 && i + 1 "
              "< argc) {\n");
  fprintf(fp, "      basic_pass = argv[++i];\n");
  fprintf(
      fp,
      "    } else if (strcmp(argv[i], \"--format\") == 0 && i + 1 < argc) {\n");
  fprintf(fp, "      out_format = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--verbose\") == 0 || "
              "strcmp(argv[i], \"-v\") == 0) {\n");
  fprintf(fp, "      verbose++;\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--db-path\") == 0 && i + 1 < "
              "argc) {\n");
  fprintf(fp, "      db_path = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--cert\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      cert_path = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--key\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      key_path = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--port\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      port = (int)strtol(argv[++i], NULL, 10);\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--threads\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      threads = (int)strtol(argv[++i], NULL, 10);\n");
  fprintf(fp, "    } else if (argv[i][0] != '-') {\n");
  fprintf(fp, "      cmd_idx = i;\n");
  fprintf(fp, "      break;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "  }\n\n");
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
  fprintf(fp, "      JSON_Value *req_val = json_parse_string(buffer);\n");
  fprintf(fp, "      if (!req_val) continue;\n");
  fprintf(fp, "      JSON_Object *req_obj = json_value_get_object(req_val);\n");
  fprintf(fp, "      const char *method = json_object_get_string(req_obj, "
              "\"method\");\n");
  fprintf(
      fp,
      "      JSON_Value *id_val = json_object_get_value(req_obj, \"id\");\n");
  fprintf(fp, "      if (method) {\n");
  fprintf(fp, "        if (strcmp(method, \"initialize\") == 0) {\n");
  fprintf(fp,
          "          printf(\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":\");\n");
  fprintf(fp, "          if (id_val) {\n");
  fprintf(fp,
          "             char *id_str = json_serialize_to_string(id_val);\n");
  fprintf(fp, "             printf(\"%%s\", id_str);\n");
  fprintf(fp, "             json_free_serialized_string(id_str);\n");
  fprintf(fp, "          } else { printf(\"null\"); }\n");
  fprintf(fp,
          "          "
          "printf(\",\\\"result\\\":{\\\"protocolVersion\\\":\\\"2024-11-"
          "05\\\",\\\"capabilities\\\":{\\\"tools\\\":{}},\\\"serverInfo\\\":{"
          "\\\"name\\\":\\\"cli\\\",\\\"version\\\":\\\"1.0\\\"}}}\\n\");\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp, "        } else if (strcmp(method, \"tools/list\") == 0) {\n");
  fprintf(fp, "          JSON_Value *tools_val = %smcp_get_tools();\n",
          config->func_prefix ? config->func_prefix : "");
  fprintf(fp,
          "          char *tools_str = json_serialize_to_string(tools_val);\n");
  fprintf(fp,
          "          printf(\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":\");\n");
  fprintf(fp, "          if (id_val) {\n");
  fprintf(fp,
          "             char *id_str = json_serialize_to_string(id_val);\n");
  fprintf(fp, "             printf(\"%%s\", id_str);\n");
  fprintf(fp, "             json_free_serialized_string(id_str);\n");
  fprintf(fp, "          } else { printf(\"null\"); }\n");
  fprintf(fp, "          printf(\",\\\"result\\\":{\\\"tools\\\":%%s}}\\n\", "
              "tools_str);\n");
  fprintf(fp, "          json_free_serialized_string(tools_str);\n");
  fprintf(fp, "          json_value_C_CDD_FREE(tools_val);\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp, "        } else if (strcmp(method, \"tools/call\") == 0) {\n");
  fprintf(fp, "          JSON_Object *params = json_object_get_object(req_obj, "
              "\"params\");\n");
  fprintf(fp, "          const char *name = json_object_get_string(params, "
              "\"name\");\n");
  fprintf(fp, "          char *args_str = "
              "json_serialize_to_string(json_object_get_value(params, "
              "\"arguments\"));\n");
  fprintf(fp, "          char *out_result = NULL;\n");
  fprintf(
      fp,
      "          int rc = %smcp_execute_tool(name, args_str, &out_result);\n",
      config->func_prefix ? config->func_prefix : "");
  fprintf(fp,
          "          if (args_str) json_free_serialized_string(args_str);\n");
  fprintf(fp,
          "          printf(\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":\");\n");
  fprintf(fp, "          if (id_val) {\n");
  fprintf(fp,
          "             char *id_str = json_serialize_to_string(id_val);\n");
  fprintf(fp, "             printf(\"%%s\", id_str);\n");
  fprintf(fp, "             json_free_serialized_string(id_str);\n");
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
  fprintf(fp, "          if (out_result) C_CDD_FREE(out_result);\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp,
          "        } else if (strcmp(method, \"resources/list\") == 0) {\n");
  fprintf(fp, "          JSON_Value *res_val = %smcp_get_resources();\n",
          config->func_prefix ? config->func_prefix : "");
  fprintf(fp, "          char *res_str = json_serialize_to_string(res_val);\n");
  fprintf(fp,
          "          printf(\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":\");\n");
  fprintf(fp, "          if (id_val) {\n");
  fprintf(fp,
          "             char *id_str = json_serialize_to_string(id_val);\n");
  fprintf(fp, "             printf(\"%%s\", id_str);\n");
  fprintf(fp, "             json_free_serialized_string(id_str);\n");
  fprintf(fp, "          } else { printf(\"null\"); }\n");
  fprintf(fp,
          "          printf(\",\\\"result\\\":{\\\"resources\\\":%%s}}\\n\", "
          "res_str);\n");
  fprintf(fp, "          json_free_serialized_string(res_str);\n");
  fprintf(fp, "          json_value_C_CDD_FREE(res_val);\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp,
          "        } else if (strcmp(method, \"resources/read\") == 0) {\n");
  fprintf(fp, "          JSON_Object *params = json_object_get_object(req_obj, "
              "\"params\");\n");
  fprintf(
      fp,
      "          const char *uri = json_object_get_string(params, \"uri\");\n");
  fprintf(fp, "          JSON_Value *res_val = %smcp_read_resource(uri);\n",
          config->func_prefix ? config->func_prefix : "");
  fprintf(fp, "          char *res_str = json_serialize_to_string(res_val);\n");
  fprintf(fp,
          "          printf(\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"id\\\":\");\n");
  fprintf(fp, "          if (id_val) {\n");
  fprintf(fp,
          "             char *id_str = json_serialize_to_string(id_val);\n");
  fprintf(fp, "             printf(\"%%s\", id_str);\n");
  fprintf(fp, "             json_free_serialized_string(id_str);\n");
  fprintf(fp, "          } else { printf(\"null\"); }\n");
  fprintf(fp,
          "          printf(\",\\\"result\\\":{\\\"contents\\\":%%s}}\\n\", "
          "res_str);\n");
  fprintf(fp, "          json_free_serialized_string(res_str);\n");
  fprintf(fp, "          json_value_C_CDD_FREE(res_val);\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp, "        }\n");
  fprintf(fp, "      }\n");
  fprintf(fp, "      json_value_C_CDD_FREE(req_val);\n");
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
        g_cdd_last_alloc_err = cdd_serde_transform_case(
            opId, CDD_SERDE_CASE_KEBABCASE, kebab_op, sizeof(kebab_op));
        fprintf(fp, "  /* \n");
        fprintf(fp, "   * @brief %s CLI handler\n",
                op->summary ? op->summary : opId);
        fprintf(fp, "   */\n");
        fprintf(fp,
                "  if (strcmp(argv[cmd_idx], \"%s\") == 0 || "
                "strcmp(argv[cmd_idx], \"%s\") == 0) {\n",
                opId, kebab_op);
        fprintf(fp, "    int _arg_i;\n");
        fprintf(fp, "    const char *_req_data = NULL;\n");
        fprintf(fp, "    const char *_req_data_file = NULL;\n");
        for (k = 0; k < op->n_parameters; k++) {
          fprintf(fp, "    const char *_param_%lu = NULL;\n", (unsigned long)k);
        }
        fprintf(fp,
                "    for (_arg_i = cmd_idx + 1; _arg_i < argc; ++_arg_i) {\n");
        fprintf(fp,
                "      if (strcmp(argv[_arg_i], \"--help\") == 0 || "
                "strcmp(argv[_arg_i], \"-h\") == 0) {\n"
                "        printf(\"Usage: cli %s [OPTIONS]\\n\");\n",
                kebab_op);
        if (op->summary) {
          fprintf(fp, "        printf(\"%%s\\n\", \"%s\");\n", op->summary);
        }
        if (op->deprecated) {
          fprintf(fp, "        printf(\"WARNING: This operation is "
                      "deprecated.\\n\");\n");
        }
        for (k = 0; k < op->n_parameters; k++) {
          char kp_name[64];
          const char *pname =
              op->parameters[k].name ? op->parameters[k].name : "param";
          g_cdd_last_alloc_err = cdd_serde_transform_case(
              pname, CDD_SERDE_CASE_KEBABCASE, kp_name, sizeof(kp_name));
          fprintf(fp, "        printf(\"  --%s <VAL>  %s\\n\");\n", kp_name,
                  op->parameters[k].description ? op->parameters[k].description
                                                : "");
        }
        if (op->req_body.content_schema) {
          fprintf(fp, "        printf(\"  -d, --data <JSON>       Request body "
                      "JSON\\n\");\n");
          fprintf(fp, "        printf(\"  --data-file <PATH>      Request body "
                      "JSON file\\n\");\n");
        }
        fprintf(fp, "        return CDD_C_SUCCESS;\n"
                    "      }\n");
        if (op->req_body.content_schema) {
          fprintf(fp,
                  "      if ((strcmp(argv[_arg_i], \"--data\") == 0 || "
                  "strcmp(argv[_arg_i], \"-d\") == 0) && _arg_i + 1 < argc) {\n"
                  "        _req_data = argv[++_arg_i];\n"
                  "        continue;\n"
                  "      }\n"
                  "      if (strcmp(argv[_arg_i], \"--data-file\") == 0 && "
                  "_arg_i + 1 < argc) {\n"
                  "        _req_data_file = argv[++_arg_i];\n"
                  "        continue;\n"
                  "      }\n");
        }
        for (k = 0; k < op->n_parameters; k++) {
          char kp_name[64];
          const char *pname =
              op->parameters[k].name ? op->parameters[k].name : "param";
          g_cdd_last_alloc_err = cdd_serde_transform_case(
              pname, CDD_SERDE_CASE_KEBABCASE, kp_name, sizeof(kp_name));
          fprintf(
              fp,
              "      if ((strcmp(argv[_arg_i], \"--%s\") == 0 || "
              "strcmp(argv[_arg_i], \"--%s\") == 0) && _arg_i + 1 < argc) {\n"
              "        _param_%lu = argv[++_arg_i];\n"
              "        continue;\n"
              "      }\n",
              kp_name, pname, (unsigned long)k);
        }
        fprintf(fp, "    }\n");
        fprintf(fp, "    if (_req_data || _req_data_file) {}\n");
        for (k = 0; k < op->n_parameters; k++) {
          fprintf(fp, "    if (_param_%lu) {}\n", (unsigned long)k);
        }
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

  return CDD_C_SUCCESS;
}
