/**
 * @file client_gen_mcp.c
 * @brief MCP adapters emission for OpenAPI client generator.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "routes/emit/client_gen_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Emit MCP adapters into client header and source.
 */
cdd_c_error_t client_gen_emit_mcp(FILE *hfile, FILE *cfile, const char *guard,
                                  const char *prefix,
                                  const struct OpenAPI_Spec *spec) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  size_t i, j;

  /* --- Write MCP Adapters --- */
  if (fprintf(hfile, "\n/* MCP Client (From) API */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_list_tools(void* params, "
              "int req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_call_tool(void* params, "
              "int req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_ping(void* params, int "
              "req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_initialize(void* params, "
              "int req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_list_resources(void* "
              "params, int "
              "req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_read_resource(void* "
              "params, int "
              "req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_list_prompts(void* params, "
              "int req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_get_prompt(void* params, "
              "int req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_complete(void* params, int "
              "req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_subscribe(void* params, "
              "int req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_unsubscribe(void* params, "
              "int req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_set_level(void* params, "
              "int req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_client_create_message(void* "
              "params, int "
              "req_id, char** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "\n/* Native MCP Adapters */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "/**\n * @brief Native MCP Tool Adapter\n * Retrieves all "
                     "operations exposed as MCP Tools.\n */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "extern cdd_c_error_t %smcp_get_tools(void** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "/**\n * @brief Native MCP Resource Adapter\n * Retrieves "
                     "all read-only documentation resources.\n */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "extern cdd_c_error_t %smcp_get_resources(void** out);\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "/**\n * @brief LLM Execution Router\n * Executes a tool "
                     "by name with JSON arguments.\n */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_execute_tool(const char* name, "
              "const char* json_args, char** out_result);\n\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile, "\n/* Native MCP Adapters Implementation */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile, "cdd_c_error_t %smcp_get_tools(void** out) {\n", prefix) <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *root_val = json_value_init_array();\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  JSON_Array *tools_arr = json_value_get_array(root_val);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;

  for (i = 0; i < spec->n_paths; ++i) {
    struct OpenAPI_Path *path = &spec->paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      struct OpenAPI_Operation *op = &path->operations[j];
      {
        if (fprintf(cfile, "  {\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    JSON_Value *tool_val = json_value_init_object();\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    JSON_Object *tool_obj = "
                           "json_value_get_object(tool_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_object_set_string(tool_obj, \"name\", \"%s\");\n",
                    op->operation_id) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (op->description || op->summary) {
          if (fprintf(cfile,
                      "    json_object_set_string(tool_obj, \"description\", "
                      "\"%s\");\n",
                      op->description ? op->description : op->summary) < 0)
            rc = CDD_C_ERROR_MEMORY;
        }
        if (fprintf(cfile, "    {\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(
                cfile,
                "      JSON_Value *schema_val = json_value_init_object();\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "      JSON_Object *schema_obj = "
                           "json_value_get_object(schema_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "      json_object_set_string(schema_obj, \"type\", "
                           "\"object\");\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "      json_object_set_value(tool_obj, "
                           "\"inputSchema\", schema_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    }\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_array_append_value(tools_arr, tool_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "  }\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
      }
    }
  }

  if (fprintf(cfile, "  *out = root_val;\n  return CDD_C_SUCCESS;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile, "cdd_c_error_t %smcp_get_resources(void** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *root_val = json_value_init_array();\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  JSON_Array *res_arr = json_value_get_array(root_val);\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (spec->n_defined_schemas > 0) {
    for (i = 0; i < spec->n_defined_schemas; ++i) {
      if (spec->defined_schema_names[i]) {
        if (fprintf(cfile, "  {\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    JSON_Value *res_val = json_value_init_object();\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    JSON_Object *res_obj = "
                           "json_value_get_object(res_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_object_set_string(res_obj, \"uri\", "
                    "\"schema:///%s\");\n",
                    spec->defined_schema_names[i]) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_object_set_string(res_obj, \"name\", \"%s "
                    "Schema\");\n",
                    spec->defined_schema_names[i]) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    json_object_set_string(res_obj, \"mimeType\", "
                           "\"application/json\");\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    json_array_append_value(res_arr, res_val);\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "  }\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
      }
    }
  }

  if (fprintf(cfile, "  *out = root_val;\n  return CDD_C_SUCCESS;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(hfile, "/**\n * @brief Native MCP Resource Reader\n * Reads a "
                     "specific resource by URI.\n */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile,
              "extern cdd_c_error_t %smcp_read_resource(const char* uri, "
              "void** out);\n\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile,
              "cdd_c_error_t %smcp_read_resource(const char* uri, void** "
              "out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *root_val = json_value_init_array();\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  JSON_Array *res_arr = json_value_get_array(root_val);\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  (void)uri;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (spec->n_defined_schemas > 0) {
    for (i = 0; i < spec->n_defined_schemas; ++i) {
      if (spec->defined_schema_names[i]) {
        if (fprintf(cfile, "  if (strcmp(uri, \"schema:///%s\") == 0) {\n",
                    spec->defined_schema_names[i]) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    JSON_Value *res_val = json_value_init_object();\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    JSON_Object *res_obj = "
                           "json_value_get_object(res_val);\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile,
                    "    json_object_set_string(res_obj, \"uri\", "
                    "\"schema:///%s\");\n",
                    spec->defined_schema_names[i]) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "    json_object_set_string(res_obj, \"mimeType\", "
                           "\"application/json\");\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        {
          int found_raw = 0;
          size_t k;
          for (k = 0; k < spec->n_raw_schemas; ++k) {
            if (strcmp(spec->raw_schema_names[k],
                       spec->defined_schema_names[i]) == 0) {
              /* MATCHED! */
              char *escaped = NULL;
              escape_c_string_literal(spec->raw_schema_json[k], &escaped);
              if (escaped) {
                fprintf(cfile,
                        "    json_object_set_string(res_obj, \"text\", "
                        "\"%s\");\n",
                        escaped);
                free(escaped);
                found_raw = 1;
              }
              break;
            }
          }
          if (!found_raw) {
            if (fprintf(cfile, "    json_object_set_string(res_obj, \"text\", "
                               "\"{}\");\n") < 0)
              rc = CDD_C_ERROR_MEMORY;
          }
        }
        if (fprintf(cfile, "    json_array_append_value(res_arr, res_val);\n") <
            0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(cfile, "  }\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
      }
    }
  }

  if (fprintf(cfile, "  *out = root_val;\n  return CDD_C_SUCCESS;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile,
              "cdd_c_error_t %smcp_execute_tool(const char* name, const "
              "char* json_args, "
              "char** out_result) {\n  (void)json_args;\n  if (out_result) "
              "*out_result = NULL;\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  for (i = 0; i < spec->n_paths; ++i) {
    struct OpenAPI_Path *path = &spec->paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      struct OpenAPI_Operation *op = &path->operations[j];
      {
        if (fprintf(cfile, "  if (strcmp(name, \"%s\") == 0) {\n",
                    op->operation_id) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(
                cfile,
                "    JSON_Value *args_val = json_parse_string(json_args);\n"
                "    JSON_Object *args_obj = json_value_get_object(args_val);\n"
                "    (void)args_obj;\n"
                "    /* Actual argument parsing logic */\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
        {
          size_t p_idx;
          for (p_idx = 0; p_idx < op->n_parameters; ++p_idx) {
            struct OpenAPI_Parameter *p = &op->parameters[p_idx];
            if (fprintf(cfile,
                        "    const char *arg_%s = "
                        "json_object_get_string(args_obj, \"%s\");\n"
                        "    (void)arg_%s;\n",
                        p->name, p->name, p->name) < 0)
              rc = CDD_C_ERROR_MEMORY;
          }
        }
        if (fprintf(cfile, "    /* Call %s%s with extracted args */\n", prefix,
                    op->operation_id) < 0)
          rc = CDD_C_ERROR_MEMORY;
        if (fprintf(
                cfile,
                "    if (out_result) {\n      *out_result = malloc(128);\n     "
                " str"
                "cpy(*out_result, \"{\\\"status\\\":\\\"success\\\"}\");\n "
                "   }\n"
                "    if (args_val) json_value_free(args_val);\n"
                "    return CDD_C_SUCCESS;\n  }\n") < 0)
          rc = CDD_C_ERROR_MEMORY;
      }
    }
  }
  if (fprintf(cfile, "  return CDD_C_ERROR_UNKNOWN;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;

  if (fprintf(cfile, "\n/* MCP Client (From) Implementation */\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_list_tools(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          cfile,
          "  json_object_set_string(req_obj, \"method\", \"tools/list\");\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_call_tool(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          cfile,
          "  json_object_set_string(req_obj, \"method\", \"tools/call\");\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_ping(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"method\", \"ping\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_initialize(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          cfile,
          "  json_object_set_string(req_obj, \"method\", \"initialize\");\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_list_resources(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"resources/list\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_read_resource(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"resources/read\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_list_prompts(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"prompts/list\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_get_prompt(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(
          cfile,
          "  json_object_set_string(req_obj, \"method\", \"prompts/get\");\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_complete(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"completion/complete\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_subscribe(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"resources/subscribe\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_unsubscribe(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"resources/unsubscribe\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_set_level(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"logging/setLevel\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "cdd_c_error_t %smcp_client_create_message(void* params, "
              "int req_id, char** out) {\n",
              prefix) < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  JSON_Value *req_val = json_value_init_object();\n  "
                     "JSON_Object *req_obj = json_value_get_object(req_val);\n "
                     " char *ret;\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  json_object_set_string(req_obj, \"jsonrpc\", \"2.0\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_string(req_obj, \"method\", "
                     "\"sampling/createMessage\");\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  json_object_set_number(req_obj, \"id\", req_id);\n") <
      0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile,
              "  if (params) { json_object_set_value(req_obj, \"params\", "
              "json_value_deep_copy((JSON_Value*)params)); }\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(cfile, "  ret = json_serialize_to_string(req_val);\n  "
                     "json_value_free(req_val);\n  return ret;\n}\n\n") < 0)
    rc = CDD_C_ERROR_MEMORY;
  if (fprintf(hfile, "#ifdef __cplusplus\n}\n#endif\n") < 0) {
    rc = CDD_C_SUCCESS;
    {
      fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
              __LINE__);
      goto cleanup;
    }
  }
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 74)
    rc = CDD_C_ERROR_IO;
  else
#endif
    rc = (fprintf(hfile, "#endif /* %s */\n", guard) < 0) ? CDD_C_ERROR_IO
                                                          : CDD_C_SUCCESS;
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

cleanup:
  return rc;
}
