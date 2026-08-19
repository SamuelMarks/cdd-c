/* Generated CLI from OpenAPI Specification */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parson.h>
/* clang-format off */
#include "cdd_c_error.h"
/* clang-format on */
#include "test_cli.h"

/**
 * @brief Auto-generated code from OpenAPI specification
 */
cdd_c_error_t print_cli_help(void) {
  printf("%s v%s\n", "CLI Tool", "1.0.0");
  printf("Usage: cli [options] <command> [args]\n\n");
  printf("Options:\n");
  printf("  --db-path <path>           Database path (default oauth.db)\n");
  printf("  --cert <path>              TLS Certificate path\n");
  printf("  --key <path>               TLS Key path\n");
  printf("  --port <int>               Listening port (default 8080)\n");
  printf("  --threads <int>            Number of threads (default 4)\n\n");
  printf("  %-20s %s\n", "mcp", "Start the Model Context Protocol (MCP) server");
  printf("Commands:\n");
  return CDD_C_SUCCESS;
}

/**
 * @brief Auto-generated code from OpenAPI specification
 */
int main(int argc, char **argv) {
  int i;
  const char *db_path = "oauth.db";
  const char *cert_path = NULL;
  const char *key_path = NULL;
  int port = 8080;
  int threads = 4;
  int cmd_idx = 1;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--db-path") == 0 && i + 1 < argc) {
      db_path = argv[++i];
    } else if (strcmp(argv[i], "--cert") == 0 && i + 1 < argc) {
      cert_path = argv[++i];
    } else if (strcmp(argv[i], "--key") == 0 && i + 1 < argc) {
      key_path = argv[++i];
    } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
      port = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
      threads = atoi(argv[++i]);
    } else if (argv[i][0] != '-') {
      cmd_idx = i;
      break;
    }
  }

  if (cmd_idx >= argc || strcmp(argv[cmd_idx], "--help") == 0 || strcmp(argv[cmd_idx], "-h") == 0) {
    cdd_c_error_t rc = print_cli_help(); if(rc != CDD_C_SUCCESS) return rc;
    return CDD_C_SUCCESS;
  }

  (void)db_path;
  (void)cert_path;
  (void)key_path;
  (void)port;
  (void)threads;

  if (strcmp(argv[cmd_idx], "mcp") == 0) {
    char buffer[65536];
    while (fgets(buffer, sizeof(buffer), stdin)) {
      JSON_Value *req_val = json_parse_string(buffer);
      if (!req_val) continue;
      JSON_Object *req_obj = json_value_get_object(req_val);
      const char *method = json_object_get_string(req_obj, "method");
      JSON_Value *id_val = json_object_get_value(req_obj, "id");
      if (method) {
        if (strcmp(method, "initialize") == 0) {
          printf("{\"jsonrpc\":\"2.0\",\"id\":");
          if (id_val) {
             char *id_str = json_serialize_to_string(id_val);
             printf("%s", id_str);
             json_free_serialized_string(id_str);
          } else { printf("null"); }
          printf(",\"result\":{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{\"tools\":{}},\"serverInfo\":{\"name\":\"cli\",\"version\":\"1.0\"}}}\n");
          fflush(stdout);
        } else if (strcmp(method, "tools/list") == 0) {
          JSON_Value *tools_val = mcp_get_tools();
          char *tools_str = json_serialize_to_string(tools_val);
          printf("{\"jsonrpc\":\"2.0\",\"id\":");
          if (id_val) {
             char *id_str = json_serialize_to_string(id_val);
             printf("%s", id_str);
             json_free_serialized_string(id_str);
          } else { printf("null"); }
          printf(",\"result\":{\"tools\":%s}}\n", tools_str);
          json_free_serialized_string(tools_str);
          json_value_C_CDD_FREE(tools_val);
          fflush(stdout);
        } else if (strcmp(method, "tools/call") == 0) {
          JSON_Object *params = json_object_get_object(req_obj, "params");
          const char *name = json_object_get_string(params, "name");
          char *args_str = json_serialize_to_string(json_object_get_value(params, "arguments"));
          char *out_result = NULL;
          int rc = mcp_execute_tool(name, args_str, &out_result);
          if (args_str) json_free_serialized_string(args_str);
          printf("{\"jsonrpc\":\"2.0\",\"id\":");
          if (id_val) {
             char *id_str = json_serialize_to_string(id_val);
             printf("%s", id_str);
             json_free_serialized_string(id_str);
          } else { printf("null"); }
          if (rc == 0) {
            printf(",\"result\":{\"content\":[{\"type\":\"text\",\"text\":\"%s\"}]}}\n", out_result ? out_result : "Success");
          } else {
            printf(",\"result\":{\"isError\":true,\"content\":[{\"type\":\"text\",\"text\":\"%s\"}]}}\n", out_result ? out_result : "Error executing tool");
          }
          if (out_result) C_CDD_FREE(out_result);
          fflush(stdout);
        } else if (strcmp(method, "resources/list") == 0) {
          JSON_Value *res_val = mcp_get_resources();
          char *res_str = json_serialize_to_string(res_val);
          printf("{\"jsonrpc\":\"2.0\",\"id\":");
          if (id_val) {
             char *id_str = json_serialize_to_string(id_val);
             printf("%s", id_str);
             json_free_serialized_string(id_str);
          } else { printf("null"); }
          printf(",\"result\":{\"resources\":%s}}\n", res_str);
          json_free_serialized_string(res_str);
          json_value_C_CDD_FREE(res_val);
          fflush(stdout);
        } else if (strcmp(method, "resources/read") == 0) {
          JSON_Object *params = json_object_get_object(req_obj, "params");
          const char *uri = json_object_get_string(params, "uri");
          JSON_Value *res_val = mcp_read_resource(uri);
          char *res_str = json_serialize_to_string(res_val);
          printf("{\"jsonrpc\":\"2.0\",\"id\":");
          if (id_val) {
             char *id_str = json_serialize_to_string(id_val);
             printf("%s", id_str);
             json_free_serialized_string(id_str);
          } else { printf("null"); }
          printf(",\"result\":{\"contents\":%s}}\n", res_str);
          json_free_serialized_string(res_str);
          json_value_C_CDD_FREE(res_val);
          fflush(stdout);
        }
      }
      json_value_C_CDD_FREE(req_val);
    }
    return CDD_C_SUCCESS;
  }

  printf("Unknown command: %s\n", argv[1]);
  return CDD_C_ERROR_UNKNOWN;
}
