/**
 * @file codegen_client_sig.c
 * @brief Implementation of Client Signature Generation.
 *
 * Updated to support Grouped naming convention (Resource_Prefix_OpId).
 * Appends standard `struct ApiError **api_error` argument to all operations.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_client_sig.h"
#include "str_utils.h"

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

static const char *map_type_to_c_arg(const char *oa_type) {
  if (strcmp(oa_type, "integer") == 0)
    return "int";
  if (strcmp(oa_type, "string") == 0)
    return "const char *";
  if (strcmp(oa_type, "boolean") == 0)
    return "int";
  if (strcmp(oa_type, "number") == 0)
    return "double";
  return "const void *";
}

static int is_primitive_type(const char *oa_type) {
  if (!oa_type)
    return 0;
  return strcmp(oa_type, "integer") == 0 || strcmp(oa_type, "string") == 0 ||
         strcmp(oa_type, "boolean") == 0 || strcmp(oa_type, "number") == 0;
}

static int param_is_object_kv(const struct OpenAPI_Parameter *p) {
  if (!p)
    return 0;
  if (p->is_array)
    return 0;
  if (!p->type)
    return 0;
  if (strcmp(p->type, "object") != 0)
    return 0;
  return p->in == OA_PARAM_IN_QUERY || p->in == OA_PARAM_IN_PATH ||
         p->in == OA_PARAM_IN_HEADER;
}

static const char *map_array_item_type(const char *oa_type) {
  if (!oa_type)
    return "const void *";
  if (strcmp(oa_type, "integer") == 0)
    return "const int *";
  if (strcmp(oa_type, "boolean") == 0)
    return "const int *";
  if (strcmp(oa_type, "string") == 0)
    return "const char **"; /* Array of strings */
  if (strcmp(oa_type, "number") == 0)
    return "const double *";
  return "const void *";
}

static const char *map_type_to_c_out(const char *oa_type) {
  if (!oa_type)
    return "void *";
  if (strcmp(oa_type, "integer") == 0)
    return "int *";
  if (strcmp(oa_type, "boolean") == 0)
    return "int *";
  if (strcmp(oa_type, "string") == 0)
    return "char **";
  if (strcmp(oa_type, "number") == 0)
    return "double *";
  return "void *";
}

static const char *map_array_item_type_out(const char *oa_type) {
  if (!oa_type)
    return "void **";
  if (strcmp(oa_type, "integer") == 0)
    return "int **";
  if (strcmp(oa_type, "boolean") == 0)
    return "int **";
  if (strcmp(oa_type, "string") == 0)
    return "char ***";
  if (strcmp(oa_type, "number") == 0)
    return "double **";
  return "void **";
}

static int schema_has_inline(const struct OpenAPI_SchemaRef *schema) {
  if (!schema)
    return 0;
  if (schema->inline_type)
    return 1;
  if (schema->is_array && schema->inline_type)
    return 1;
  return 0;
}

static const struct OpenAPI_SchemaRef *
get_success_schema(const struct OpenAPI_Operation *op) {
  const struct OpenAPI_Response *default_resp = NULL;
  size_t i;
  for (i = 0; i < op->n_responses; ++i) {
    const char *c = op->responses[i].code;
    if (!c)
      continue;
    if (strcmp(c, "default") == 0) {
      default_resp = &op->responses[i];
      continue;
    }
    if (strlen(c) == 3 && c[1] == 'X' && c[2] == 'X' && c[0] == '2') {
      if (op->responses[i].schema.ref_name ||
          schema_has_inline(&op->responses[i].schema))
        return &op->responses[i].schema;
      continue;
    }
    if (c[0] == '2') {
      if (op->responses[i].schema.ref_name ||
          schema_has_inline(&op->responses[i].schema))
        return &op->responses[i].schema;
    }
  }
  if (default_resp && (default_resp->schema.ref_name ||
                       schema_has_inline(&default_resp->schema)))
    return &default_resp->schema;
  return &op->req_body;
}

int codegen_client_write_signature(
    FILE *const fp, const struct OpenAPI_Operation *const op,
    const struct CodegenSigConfig *const config) {
  const char *ctx_type =
      (config && config->ctx_type) ? config->ctx_type : "struct HttpClient *";
  const char *prefix = (config && config->prefix) ? config->prefix : "";
  const char *func_name = op->operation_id ? op->operation_id : "unnamed_op";
  const struct OpenAPI_SchemaRef *success_schema;
  const char *group =
      (config && config->group_name) ? config->group_name : NULL;
  size_t i;

  if (!fp || !op)
    return EINVAL;

  /* Construct function name: [Group_][Prefix][OpName] */
  CHECK_IO(fprintf(fp, "int "));
  if (group && *group) {
    CHECK_IO(fprintf(fp, "%s_", group));
  }
  CHECK_IO(fprintf(fp, "%s%s(%sctx", prefix, func_name, ctx_type));

  /* 1. Parameters */
  for (i = 0; i < op->n_parameters; ++i) {
    const struct OpenAPI_Parameter *p = &op->parameters[i];
    if (param_is_object_kv(p)) {
      CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                       p->name, p->name));
    } else if (p->is_array) {
      /* Emit pointer + length */
      const char *c_type = map_array_item_type(p->items_type);
      CHECK_IO(fprintf(fp, ", %s %s, size_t %s_len", c_type, p->name, p->name));
    } else {
      const char *c_type = map_type_to_c_arg(p->type);
      CHECK_IO(fprintf(fp, ", %s %s", c_type, p->name));
    }
  }

  /* 2. Request Body */
  if (op->req_body.content_type && op->req_body.ref_name) {
    if (op->req_body.is_array) {
      if (strcmp(op->req_body.ref_name, "string") == 0) {
        CHECK_IO(fprintf(fp, ", const char **body, size_t body_len"));
      } else if (strcmp(op->req_body.ref_name, "integer") == 0) {
        CHECK_IO(fprintf(fp, ", const int *body, size_t body_len"));
      } else {
        CHECK_IO(fprintf(fp, ", struct %s **body, size_t body_len",
                         op->req_body.ref_name));
      }
    } else {
      CHECK_IO(
          fprintf(fp, ", const struct %s *req_body", op->req_body.ref_name));
    }
  } else if (op->req_body.content_type &&
             (op->req_body.inline_type ||
              (op->req_body.is_array && op->req_body.inline_type))) {
    if (op->req_body.is_array) {
      const char *item_type = op->req_body.inline_type;
      const char *c_type = map_array_item_type(item_type);
      CHECK_IO(fprintf(fp, ", %s body, size_t body_len", c_type));
    } else {
      const char *c_type = map_type_to_c_arg(op->req_body.inline_type);
      CHECK_IO(fprintf(fp, ", %s req_body", c_type));
    }
  }

  /* 3. Success Output */
  success_schema = get_success_schema(op);
  if (success_schema &&
      (success_schema->ref_name || schema_has_inline(success_schema) ||
       success_schema->is_array)) {
    if (success_schema->is_array) {
      if (success_schema->ref_name) {
        if (strcmp(success_schema->ref_name, "string") == 0) {
          CHECK_IO(fprintf(fp, ", char ***out, size_t *out_len"));
        } else if (strcmp(success_schema->ref_name, "integer") == 0) {
          CHECK_IO(fprintf(fp, ", int **out, size_t *out_len"));
        } else {
          CHECK_IO(fprintf(fp, ", struct %s ***out, size_t *out_len",
                           success_schema->ref_name));
        }
      } else if (success_schema->inline_type) {
        const char *out_type =
            map_array_item_type_out(success_schema->inline_type);
        CHECK_IO(fprintf(fp, ", %s out, size_t *out_len", out_type));
      }
    } else if (success_schema->ref_name) {
      CHECK_IO(fprintf(fp, ", struct %s **out", success_schema->ref_name));
    } else if (success_schema->inline_type) {
      const char *out_type = map_type_to_c_out(success_schema->inline_type);
      CHECK_IO(fprintf(fp, ", %s out", out_type));
    }
  }

  /* 4. Global Error Output */
  /* Always appended to standardise error handling */
  CHECK_IO(fprintf(fp, ", struct ApiError **api_error"));

  CHECK_IO(fprintf(fp, ")"));

  if (config && config->include_semicolon) {
    CHECK_IO(fprintf(fp, ";\n"));
  } else {
    CHECK_IO(fprintf(fp, " {\n"));
  }

  return 0;
}
