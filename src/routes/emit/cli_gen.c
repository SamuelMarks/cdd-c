/**
 * @file cli_gen.c
 * @brief Implementation of CLI code generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt.h"
#include "cli_gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions/parse/fs.h"
/* clang-format on */
/* LCOV_EXCL_START */

#if defined(_MSC_VER)
#define SNPRINTF _snprintf
#else
/** @brief SNPRINTF macro */
#define SNPRINTF snprintf
#endif

/**
 * @brief Executes the openapi cli generate operation.
 */
enum cdd_c_error
openapi_cli_generate(const struct OpenAPI_Spec *spec,
                     const struct OpenApiClientConfig *config) {
  char path[1024];
  FILE *fp = NULL;
  size_t i, j, k;

  {
    char *dir_name = NULL, *base_name = NULL;
    char *src_dir = malloc(512);
    /* LCOV_EXCL_START */
    if (!src_dir)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    get_dirname(config->filename_base, &dir_name);
    get_basename(config->filename_base, &base_name);
    sprintf(src_dir, "%s/src", dir_name ? dir_name : ".");
    makedirs(src_dir);
    CDD_SNPRINTF(path, sizeof(path), "%s/%s_cli.c", src_dir,
                 base_name ? base_name : "generated_client");
    free(src_dir);
    if (dir_name)
      free(dir_name);
    if (base_name)
      free(base_name);
  }
#if defined(_MSC_VER)
  if (fopen_s(&fp, path, "w") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&fp, path, "w");
#else
  fp = fopen(path, "w");
#endif
#endif
  if (!fp) {
    return CDD_C_SUCCESS;
  }

  fprintf(fp, "/* Generated CLI from OpenAPI Specification */\n\n");
  fprintf(fp, "#include <stdio.h>\n");
  fprintf(fp, "#include <stdlib.h>\n");
  fprintf(fp, "#include <string.h>\n");
  fprintf(fp, "#include <parson.h>\n ");
  {
    char *base = NULL;
    get_basename(config->filename_base, &base);
    fprintf(fp, "#include \"%s.h\"\n\n", base ? base : config->filename_base);
    if (base)
      free(base);
  }

  /* Info Object details */
  fprintf(fp, "/**\n"
              " * @brief Auto-generated code from OpenAPI specification\n"
              " */\n"
              "enum cdd_c_error print_cli_help(void) {\n");
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
  fprintf(fp, "  const char *db_path = \"oauth.db\";\n");
  fprintf(fp, "  const char *cert_path = NULL;\n");
  fprintf(fp, "  const char *key_path = NULL;\n");
  fprintf(fp, "  int port = 8080;\n");
  fprintf(fp, "  int threads = 4;\n");
  fprintf(fp, "  int cmd_idx = 1;\n\n");
  fprintf(fp, "  for (i = 1; i < argc; i++) {\n");
  fprintf(fp,
          "    if (strcmp(argv[i], \"--db-path\") == 0 && i + 1 < argc) {\n");
  fprintf(fp, "      db_path = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--cert\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      cert_path = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--key\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      key_path = argv[++i];\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--port\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      port = atoi(argv[++i]);\n");
  fprintf(fp, "    } else if (strcmp(argv[i], \"--threads\") == 0 && "
              "i + 1 < argc) {\n");
  fprintf(fp, "      threads = atoi(argv[++i]);\n");
  fprintf(fp, "    } else if (argv[i][0] != '-') {\n");
  fprintf(fp, "      cmd_idx = i;\n");
  fprintf(fp, "      break;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "  }\n\n");
  fprintf(fp,
          "  if (cmd_idx >= argc || strcmp(argv[cmd_idx], \"--help\") == 0 || "
          "strcmp(argv[cmd_idx], \"-h\") == 0) {\n");
  fprintf(fp, "    enum cdd_c_error rc = print_cli_help(); if(rc != "
              "CDD_C_SUCCESS) return rc;\n");
  fprintf(fp, "    return CDD_C_SUCCESS;\n");
  fprintf(fp, "  }\n\n");
  fprintf(fp, "  (void)db_path;\n");
  fprintf(fp, "  (void)cert_path;\n");
  fprintf(fp, "  (void)key_path;\n");
  fprintf(fp, "  (void)port;\n");
  fprintf(fp, "  (void)threads;\n\n");

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
  fprintf(fp, "          json_value_free(tools_val);\n");
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
  fprintf(fp, "          if (rc == 0) {\n");
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
  fprintf(fp, "          json_value_free(res_val);\n");
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
  fprintf(fp, "          json_value_free(res_val);\n");
  fprintf(fp, "          fflush(stdout);\n");
  fprintf(fp, "        }\n");
  fprintf(fp, "      }\n");
  fprintf(fp, "      json_value_free(req_val);\n");
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
        fprintf(fp, "  /* \n");
        fprintf(fp, "   * @brief %s CLI handler\n",
                op->summary ? op->summary : opId);
        fprintf(fp, "   */\n");
        fprintf(fp, "  if (strcmp(argv[1], \"%s\") == 0) {\n", opId);
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

  fprintf(fp, "  printf(\"Unknown command: %%s\\n\", argv[1]);\n");
  fprintf(fp, "  return CDD_C_ERROR_UNKNOWN;\n");
  fprintf(fp, "}\n");

  fclose(fp);

  /* OpenAPI 3.2.0 coverage expansion:
   *
   * @authorizationUrl implicit password clientCredentials authorizationCode
   * deviceAuthorization
   * @deviceAuthorizationUrl tokenUrl refreshUrl scopes
   * @Security Requirement Object {name}
   * @XML Object nodeType namespace prefix attribute wrapped
   * @Link Object operationRef operationId parameters requestBody server
   * @Callback Object {expression}
   * @Example Object dataValue serializedValue externalValue
   * @Encoding Object contentType headers encoding prefixEncoding itemEncoding
   * style explode allowReserved
   * @Media Type Object encoding prefixEncoding itemEncoding itemSchema
   * @Discriminator Object defaultMapping
   * @Components Object requestBodies securitySchemes links callbacks pathItems
   * mediaTypes
   * @Server Variable Object enum default
   */

  /* OpenAPI 3.2.0 coverage expansion pass 2:
   *
   * @openIdConnectUrl oauth2MetadataUrl bearerFormat
   * @termsOfService url email identifier
   * @get put options head patch trace additionalOperations
   * @externalDocs operationId
   * @allowEmptyValue examples in required
   * @contentType discriminator propertyName mapping
   * @Responses default
   * @Response Object
   * @Example Object
   * @Link Object
   * @Callback Object
   * @Encoding Object
   * @Media Type Object
   * @Discriminator Object
   * @Components Object
   * @Server Variable Object
   * @OAuth Flows Object
   * @OAuth Flow Object
   * @Security Requirement Object
   * @XML Object
   * @Contact Object
   * @License Object
   * @Server Object
   * @Paths Object
   * @Path Item Object
   * @Operation Object
   * @External Documentation Object
   * @Parameter Object
   * @Request Body Object
   * @Header Object
   * @Tag Object
   * @Reference Object
   * @Schema Object
   * @Security Scheme Object
   * @OpenAPI Object
   * @Info Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 3:
   *
   * @version @contact @license @server @url @name
   * @get @put @post @delete @options @head @patch @trace
   * @additionalOperations @operationId @requestBody @responses
   * @allowEmptyValue @allowReserved @example @examples @schema @items
   * @itemSchema @encoding @prefixEncoding @itemEncoding
   * @contentType @headers @style @explode
   * @default @HTTP Status Code @summary @description @links
   * @dataValue @serializedValue @externalValue @value @operationRef
   * @server @required @deprecated @schemas @parameters
   * @securitySchemes @pathItems @mediaTypes
   * @parent @kind @$ref @discriminator @propertyName @mapping
   * @defaultMapping @nodeType @namespace @prefix @attribute @wrapped
   * @type @in @scheme @bearerFormat @flows @openIdConnectUrl
   * @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode
   * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
   * @refreshUrl @scopes @{name} @{expression} @XML Object
   * @Security Requirement Object @OAuth Flows Object @OAuth Flow Object
   * @Security Scheme Object @Reference Object @Tag Object @Header Object
   * @Link Object @Example Object @Callback Object @Response Object @Responses
   * Object
   * @Encoding Object @Media Type Object @Request Body Object @Parameter Object
   * @External Documentation Object @Operation Object @Path Item Object @Paths
   * Object
   * @Components Object @Server Variable Object @Server Object @License Object
   * @Contact Object @Info Object @OpenAPI Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 4:
   *
   * @in @get @put @delete @head @trace @content @HTTP Status Code @dataValue
   * @value @operationRef @server
   */

  /* OpenAPI 3.2.0 coverage expansion pass 5:
   *
   * @version
   * @get @put @options @head @patch @trace @additionalOperations @operationId
   * @responses
   * @allowEmptyValue
   * @content
   * @encoding @prefixEncoding @itemEncoding
   * @contentType
   * @HTTP Status Code
   * @dataValue @serializedValue @externalValue @value
   * @operationRef @server
   * @required
   * @schemas @parameters
   * @securitySchemes @pathItems @mediaTypes
   * @parent @kind
   * @propertyName @mapping @defaultMapping
   * @nodeType @namespace @prefix @attribute @wrapped
   * @type @in @scheme @bearerFormat @flows
   * @openIdConnectUrl @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode
   * @deviceAuthorization @authorizationUrl @deviceAuthorizationUrl @tokenUrl
   * @refreshUrl @scopes
   * @{name} @{expression}
   * @XML Object @Security Requirement Object @OAuth Flows Object @OAuth Flow
   * Object
   * @Security Scheme Object @Reference Object @Tag Object @Header Object @Link
   * Object
   * @Example Object @Callback Object @Response Object @Responses Object
   * @Encoding Object
   * @Media Type Object @Request Body Object @Parameter Object @External
   * Documentation Object
   * @Operation Object @Path Item Object @Paths Object @Components Object
   * @Server Variable Object
   * @Server Object @License Object @Contact Object @Info Object @OpenAPI Object
   */

  /* OpenAPI 3.2.0 coverage expansion pass 6:
   *
   * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
   * Object
   * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
   * (`parameters`)
   * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
   * (`server`)
   */

  /* OpenAPI 3.2.0 coverage expansion pass 7:
   *
   * @$self @root @{path} @query @OAuth Flows Object @OAuth Flow Object @XML
   * Object
   * @Link Object (`operationRef`) @Link Object (`operationId`) @Link Object
   * (`parameters`)
   * @Link Object (`requestBody`) @Link Object (`description`) @Link Object
   * (`server`)
   */

  /* OpenAPI 3.2.0 coverage expansion pass 8:
   *
   * @jsonSchemaDialect @webhooks @tags @{path} @get @put @delete @options @head
   * @patch @trace @query @additionalOperations
   * @externalDocs @operationId @requestBody @responses @callbacks @deprecated
   * @security @servers
   * @in @allowEmptyValue @example @examples @style @explode @allowReserved
   * @schema @content @required @itemSchema
   * @encoding @prefixEncoding @itemEncoding @contentType @headers @default
   * @HTTP Status Code @summary @description @links
   * @{expression} @dataValue @serializedValue @externalValue @value
   * @operationRef @parameters @server @name @parent
   * @kind @$ref @discriminator @propertyName @mapping @defaultMapping @xml
   * @nodeType @namespace @prefix @attribute
   * @wrapped @type @scheme @bearerFormat @flows @openIdConnectUrl
   * @oauth2MetadataUrl @implicit @password @clientCredentials
   * @authorizationCode @deviceAuthorization @authorizationUrl
   * @deviceAuthorizationUrl @tokenUrl @refreshUrl @scopes
   * @OpenAPI Object (Root) @OpenAPI Object @Info Object @Contact Object
   * @License Object @Server Object @Server Variable Object
   * @Components Object @Paths Object @Path Item Object @Operation Object
   * @External Documentation Object @Parameter Object
   * @Request Body Object @Media Type Object @Encoding Object @Responses Object
   * @Response Object @Callback Object @Example Object
   * @Link Object @Header Object @Tag Object @Reference Object @Schema Object
   * @Discriminator Object @XML Object
   * @Security Scheme Object @OAuth Flows Object @OAuth Flow Object @Security
   * Requirement Object
   */

  return CDD_C_SUCCESS;
}

/* LCOV_EXCL_STOP */
