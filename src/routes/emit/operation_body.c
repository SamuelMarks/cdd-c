/**
 * @file operation_body.c
 * @brief Implementation of operation body and schema helpers.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win_compat_sym.h"

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "classes/parse/mapping.h"
#include "functions/parse/str.h"
#include "routes/emit/operation.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
#undef malloc
#define malloc(sz) C_CDD_MALLOC(sz)
#undef realloc
#define realloc(ptr, sz) C_CDD_REALLOC(ptr, sz)
#undef calloc
#define calloc(n, sz) C_CDD_CALLOC(n, sz)
extern C_CDD_EXPORT int g_cdd_fail_schema_ref_has_data;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
extern C_CDD_EXPORT int g_cdd_fail_apply_format;
#endif

/**
 * @brief Executes the any from json value operation.
 */
cdd_c_error_t any_from_json_value(const JSON_Value *val,
                                  struct OpenAPI_Any *out) {
  JSON_Value_Type t;
  const char *s;
  char *json_str;
  cdd_c_error_t rc;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(out, 0, sizeof(*out));
  if (!val)
    return CDD_C_SUCCESS;

  t = json_value_get_type(val);
  switch (t) {
  case JSONString:
    s = json_value_get_string(val);
    out->type = OA_ANY_STRING;
    rc = c_cdd_strdup(s, &out->string);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  case JSONNumber:
    out->type = OA_ANY_NUMBER;
    out->number = json_value_get_number(val);
    return CDD_C_SUCCESS;
  case JSONBoolean:
    out->type = OA_ANY_BOOL;
    out->boolean = json_value_get_boolean(val);
    return CDD_C_SUCCESS;
  case JSONNull:
    out->type = OA_ANY_NULL;
    return CDD_C_SUCCESS;
  case JSONObject:
  case JSONArray:
    json_str = json_serialize_to_string((JSON_Value *)val);
#ifdef CDD_BUILD_TESTS
    if (g_cdd_fail_json_serialize) {
      g_cdd_fail_json_serialize = 0;
      json_free_serialized_string(json_str);
      json_str = NULL;
    }
#endif
    if (!json_str) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    out->type = OA_ANY_JSON;
    rc = c_cdd_strdup(json_str, &out->json);
    json_free_serialized_string(json_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  default:
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses example any from the given input.
 */
cdd_c_error_t parse_example_any(const char *example, struct OpenAPI_Any *out) {
  JSON_Value *val;
  cdd_c_error_t rc;

  if (!out)
    return CDD_C_SUCCESS;

  memset(out, 0, sizeof(*out));
  if (!example)
    return CDD_C_SUCCESS;

  val = json_parse_string(example);
  if (!val) {
    out->type = OA_ANY_STRING;
    rc = c_cdd_strdup(example, &out->string);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }

  rc = any_from_json_value(val, out);
  json_value_free(val);
  return rc;
}

/**
 * @brief Frees the memory associated with any value local.
 */
void free_any_value_local(struct OpenAPI_Any *val) {
  if (!val)
    return;
  if (val->type == OA_ANY_STRING) {
    if (val->string)
      free(val->string);
  } else if (val->type == OA_ANY_JSON) {
    if (val->json)
      free(val->json);
  }
  memset(val, 0, sizeof(*val));
}

/**
 * @brief Creates a deep copy of any value local.
 */
cdd_c_error_t copy_any_value_local(struct OpenAPI_Any *dst,
                                   const struct OpenAPI_Any *src) {
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  memset(dst, 0, sizeof(*dst));
  dst->type = src->type;
  switch (src->type) {
  case OA_ANY_STRING:
    rc = c_cdd_strdup(src->string, &dst->string);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  case OA_ANY_JSON:
    rc = c_cdd_strdup(src->json, &dst->json);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  case OA_ANY_NUMBER:
    dst->number = src->number;
    return CDD_C_SUCCESS;
  case OA_ANY_BOOL:
    dst->boolean = src->boolean;
    return CDD_C_SUCCESS;
  case OA_ANY_NULL:
  case OA_ANY_UNSET:
  default:
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Frees the memory associated with openapi server variables.
 */
void free_openapi_server_variables_op(struct OpenAPI_Server *srv) {
  size_t i;
  if (!srv || !srv->variables)
    return;
  for (i = 0; i < srv->n_variables; ++i) {
    size_t e;
    struct OpenAPI_ServerVariable *var = &srv->variables[i];
    if (var->name)
      free(var->name);
    if (var->default_value)
      free(var->default_value);
    if (var->description)
      free(var->description);
    if (var->enum_values) {
      for (e = 0; e < var->n_enum_values; ++e) {
        free(var->enum_values[e]);
      }
      free(var->enum_values);
    }
  }
  free(srv->variables);
  srv->variables = NULL;
  srv->n_variables = 0;
}

/**
 * @brief Creates a deep copy of doc server variables.
 */
cdd_c_error_t copy_doc_server_variables_op(struct OpenAPI_Server *dst,
                                           const struct DocServer *src) {
  size_t i;
  cdd_c_error_t rc;
  if (!dst || !src || src->n_variables == 0)
    return CDD_C_SUCCESS;

  dst->variables = (struct OpenAPI_ServerVariable *)calloc(
      src->n_variables, sizeof(struct OpenAPI_ServerVariable));
  if (!dst->variables) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  dst->n_variables = src->n_variables;

  for (i = 0; i < src->n_variables; ++i) {
    size_t e;
    int found_default = 0;
    const struct DocServerVar *sv = &src->variables[i];
    struct OpenAPI_ServerVariable *dv = &dst->variables[i];

    if (!sv->name || !sv->default_value) {
      free_openapi_server_variables_op(dst);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    rc = c_cdd_strdup(sv->name, &dv->name);
    if (rc != CDD_C_SUCCESS) {
      free_openapi_server_variables_op(dst);
      return rc;
    }
    rc = c_cdd_strdup(sv->default_value, &dv->default_value);
    if (rc != CDD_C_SUCCESS) {
      free_openapi_server_variables_op(dst);
      return rc;
    }
    if (sv->description) {
      rc = c_cdd_strdup(sv->description, &dv->description);
      if (rc != CDD_C_SUCCESS) {
        free_openapi_server_variables_op(dst);
        return rc;
      }
    }
    if (sv->enum_values && sv->n_enum_values > 0) {
      dv->enum_values = (char **)calloc(sv->n_enum_values, sizeof(char *));
      if (!dv->enum_values) {
        free_openapi_server_variables_op(dst);
        return CDD_C_ERROR_MEMORY;
      }
      dv->n_enum_values = sv->n_enum_values;
      for (e = 0; e < sv->n_enum_values; ++e) {
        rc = c_cdd_strdup(sv->enum_values[e], &dv->enum_values[e]);
        if (rc != CDD_C_SUCCESS) {
          free_openapi_server_variables_op(dst);
          return rc;
        }
        if (strcmp(sv->enum_values[e], sv->default_value) == 0)
          found_default = 1;
      }
      if (!found_default) {
        free_openapi_server_variables_op(dst);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the media type.
 */
cdd_c_error_t find_media_type_op(struct OpenAPI_MediaType *mts, size_t n,
                                 const char *name,
                                 struct OpenAPI_MediaType **_out_val) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_find_media_type_op;
  if (g_op_fail_find_media_type_op) {
    g_op_fail_find_media_type_op = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;
  if (!mts || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < n; ++i) {
    if (mts[i].name && strcmp(mts[i].name, name) == 0) {
      *_out_val = &mts[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Applies example to media type.
 */
cdd_c_error_t apply_example_to_media_type(struct OpenAPI_MediaType *mt,
                                          const char *example) {
  cdd_c_error_t rc;
  if (!mt || !example || mt->example_set)
    return CDD_C_SUCCESS;
  rc = parse_example_any(example, &mt->example);
  if (rc != CDD_C_SUCCESS)
    return rc;
  mt->example_set = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the schema ref has data basic operation.
 */
cdd_c_error_t schema_ref_has_data_basic(const struct OpenAPI_SchemaRef *ref,
                                        int *out_has_data) {
  if (out_has_data)
    *out_has_data = 0;
  if (!ref || !out_has_data)
    return CDD_C_SUCCESS;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_schema_ref_has_data) {
    g_cdd_fail_schema_ref_has_data = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  *out_has_data =
      ((ref->ref_name && *ref->ref_name) || (ref->ref && *ref->ref) ||
       (ref->inline_type && *ref->inline_type) || ref->is_array)
          ? 1
          : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees schema reference fields.
 */
static void free_schema_ref_fields(struct OpenAPI_SchemaRef *ref) {
  if (ref->ref_name) {
    free(ref->ref_name);
    ref->ref_name = NULL;
  }
  if (ref->ref) {
    free(ref->ref);
    ref->ref = NULL;
  }
  if (ref->inline_type) {
    free(ref->inline_type);
    ref->inline_type = NULL;
  }
  if (ref->items_ref) {
    free(ref->items_ref);
    ref->items_ref = NULL;
  }
  if (ref->format) {
    free(ref->format);
    ref->format = NULL;
  }
}

/**
 * @brief Creates a deep copy of schema ref basic.
 */
cdd_c_error_t copy_schema_ref_basic(struct OpenAPI_SchemaRef *dst,
                                    const struct OpenAPI_SchemaRef *src) {
  cdd_c_error_t rc;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  memset(dst, 0, sizeof(*dst));
  dst->is_array = src->is_array;
  if (src->ref_name) {
    rc = c_cdd_strdup(src->ref_name, &dst->ref_name);
    if (rc != CDD_C_SUCCESS) {
      free_schema_ref_fields(dst);
      return rc;
    }
  }
  if (src->ref) {
    rc = c_cdd_strdup(src->ref, &dst->ref);
    if (rc != CDD_C_SUCCESS) {
      free_schema_ref_fields(dst);
      return rc;
    }
  }
  dst->ref_is_dynamic = src->ref_is_dynamic;
  if (src->inline_type) {
    rc = c_cdd_strdup(src->inline_type, &dst->inline_type);
    if (rc != CDD_C_SUCCESS) {
      free_schema_ref_fields(dst);
      return rc;
    }
  }
  if (src->items_ref) {
    rc = c_cdd_strdup(src->items_ref, &dst->items_ref);
    if (rc != CDD_C_SUCCESS) {
      free_schema_ref_fields(dst);
      return rc;
    }
  }
  dst->items_ref_is_dynamic = src->items_ref_is_dynamic;
  if (src->format) {
    rc = c_cdd_strdup(src->format, &dst->format);
    if (rc != CDD_C_SUCCESS) {
      free_schema_ref_fields(dst);
      return rc;
    }
  }
  if (src->items_format) {
    rc = c_cdd_strdup(src->items_format, &dst->items_format);
    if (rc != CDD_C_SUCCESS) {
      free_schema_ref_fields(dst);
      return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if request body already has a specific media type.
 *
 * @param[in] op Pointer to OpenAPI Operation.
 * @param[in] name Media type name.
 * @param[out] out_has Pointer to int receiving 1 if present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
cdd_c_error_t request_body_has_media_type(const struct OpenAPI_Operation *op,
                                          const char *name, int *out_has) {
  size_t i;
  if (out_has)
    *out_has = 0;
  if (!op || !name || !out_has)
    return CDD_C_SUCCESS;
  if (op->req_body.content_type &&
      strcmp(op->req_body.content_type, name) == 0) {
    *out_has = 1;
    return CDD_C_SUCCESS;
  }
  if (!op->req_body_media_types || op->n_req_body_media_types == 0) {
    *out_has = 0;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < op->n_req_body_media_types; ++i) {
    const struct OpenAPI_MediaType *mt = &op->req_body_media_types[i];
    if (mt->name && strcmp(mt->name, name) == 0) {
      *out_has = 1;
      return CDD_C_SUCCESS;
    }
  }
  *out_has = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the init media type from request body operation.
 */
cdd_c_error_t init_media_type_from_request_body(
    struct OpenAPI_MediaType *mt, const char *name,
    const struct OpenAPI_Operation *op, int is_item_schema) {
  int has_data = 0;
  cdd_c_error_t rc;
  if (!mt || !name || !op)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(mt, 0, sizeof(*mt));
  rc = c_cdd_strdup(name, &mt->name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = schema_ref_has_data_basic(&op->req_body, &has_data);
  if (rc != CDD_C_SUCCESS) {
    free(mt->name);
    mt->name = NULL;
    return rc;
  }
  if (has_data) {
    if (is_item_schema) {
      rc = copy_schema_ref_basic(&mt->item_schema, &op->req_body);
      if (rc != CDD_C_SUCCESS) {
        free(mt->name);
        mt->name = NULL;
        return rc;
      }
      mt->item_schema_set = 1;
    } else {
      rc = copy_schema_ref_basic(&mt->schema, &op->req_body);
      if (rc != CDD_C_SUCCESS) {
        free(mt->name);
        mt->name = NULL;
        return rc;
      }
      mt->schema_set = 1;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets request body media type.
 */
cdd_c_error_t add_request_body_media_type(struct OpenAPI_Operation *op,
                                          const char *name,
                                          int is_item_schema) {
  struct OpenAPI_MediaType *new_mts;
  size_t new_count;
  int _has_mt = 0;

  if (!op || !name || !*name)
    return CDD_C_SUCCESS;

  if (!op->req_body_media_types) {
    size_t base = op->req_body.content_type ? 1 : 0;
    op->req_body_media_types = (struct OpenAPI_MediaType *)calloc(
        base + 1, sizeof(struct OpenAPI_MediaType));
    if (!op->req_body_media_types) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    op->n_req_body_media_types = 0;
    if (op->req_body.content_type) {
      if (init_media_type_from_request_body(&op->req_body_media_types[0],
                                            op->req_body.content_type, op,
                                            is_item_schema) != 0)
        return CDD_C_ERROR_MEMORY;
      op->n_req_body_media_types = 1;
    }
  }

  request_body_has_media_type(op, name, &_has_mt);
  if (_has_mt)
    return CDD_C_SUCCESS;

  new_count = op->n_req_body_media_types + 1;
  new_mts = (struct OpenAPI_MediaType *)realloc(
      op->req_body_media_types, new_count * sizeof(struct OpenAPI_MediaType));
  if (!new_mts) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  op->req_body_media_types = new_mts;
  if (init_media_type_from_request_body(
          &op->req_body_media_types[op->n_req_body_media_types], name, op,
          is_item_schema) != 0)
    return CDD_C_ERROR_MEMORY;
  op->n_req_body_media_types = new_count;
  return CDD_C_SUCCESS;
}
