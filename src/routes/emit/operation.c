/**
 * @file operation.c
 * @brief Implementation of operation logic.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../win_compat_sym.h"

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

/* --- Helpers --- */

/**
 * @brief Checks if reserved header name.
 */
cdd_c_error_t is_reserved_header_name(const char *name, int *out_is_reserved) {
  int diff = 0;
  cdd_c_error_t rc;
  if (out_is_reserved)
    *out_is_reserved = 0;
  if (!name || !*name || !out_is_reserved)
    return CDD_C_SUCCESS;
  rc = c_cdd_stricmp(name, "accept", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out_is_reserved = 1;
    return CDD_C_SUCCESS;
  }
  rc = c_cdd_stricmp(name, "content-type", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out_is_reserved = 1;
    return CDD_C_SUCCESS;
  }
  rc = c_cdd_stricmp(name, "authorization", &diff);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (diff == 0) {
    *out_is_reserved = 1;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

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
 * @brief Parses link params json from the given input.
 */
cdd_c_error_t parse_link_params_json(const char *json,
                                     struct OpenAPI_LinkParam **out,
                                     size_t *out_count) {
  JSON_Value *val;
  JSON_Object *obj;
  size_t count, i;
  cdd_c_error_t rc;

  if (out)
    *out = NULL;
  if (out_count)
    *out_count = 0;
  if (!json || !out || !out_count)
    return CDD_C_SUCCESS;

  val = json_parse_string(json);
  if (!val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (json_value_get_type(val) != JSONObject) {
    json_value_free(val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  obj = json_value_get_object(val);
  count = json_object_get_count(obj);
  if (count == 0) {
    json_value_free(val);
    return CDD_C_SUCCESS;
  }

  *out = (struct OpenAPI_LinkParam *)calloc(count,
                                            sizeof(struct OpenAPI_LinkParam));
  if (!*out) {
    json_value_free(val);
    return CDD_C_ERROR_MEMORY;
  }
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(obj, i);
    const JSON_Value *v = json_object_get_value_at(obj, i);
    struct OpenAPI_LinkParam *lp = &(*out)[i];

    rc = c_cdd_strdup(name, &lp->name);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(val);
      goto cleanup;
    }
    rc = any_from_json_value(v, &lp->value);
    if (rc != CDD_C_SUCCESS) {
      json_value_free(val);
      goto cleanup;
    }
  }

  json_value_free(val);
  return CDD_C_SUCCESS;

cleanup: {
  size_t j;
  for (j = 0; j < count; ++j) {
    struct OpenAPI_LinkParam *lp = &(*out)[j];
    if (lp->name)
      free(lp->name);
    free_any_value_local(&lp->value);
  }
  free(*out);
}
  *out = NULL;
  *out_count = 0;
  return CDD_C_ERROR_MEMORY;
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
 * @brief Retrieves the doc param.
 */
cdd_c_error_t find_doc_param(const struct DocMetadata *doc, const char *name,
                             struct DocParam **_out_val) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_find_doc_param;
  if (g_op_fail_find_doc_param) {
    g_op_fail_find_doc_param = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;
  if (!doc || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < doc->n_params; ++i) {
    if (doc->params[i].name && strcmp(doc->params[i].name, name) == 0) {
      *_out_val = &doc->params[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if path param.
 */
cdd_c_error_t is_path_param(const char *route, const char *name,
                            int *out_is_path) {
  char tmpl[128];
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_is_path_param;
  if (g_op_fail_is_path_param) {
    g_op_fail_is_path_param = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (out_is_path)
    *out_is_path = 0;
  if (!route || !name || !out_is_path)
    return CDD_C_SUCCESS;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(tmpl, sizeof(tmpl), "{%s}", name);
#else
  snprintf(tmpl, sizeof(tmpl), "{%s}", name);
#endif
  *out_is_path = (strstr(route, tmpl) != NULL) ? 1 : 0;
  return CDD_C_SUCCESS;
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
 * @brief Retrieves the response by code.
 */
cdd_c_error_t find_response_by_code(struct OpenAPI_Operation *op,
                                    const char *code,
                                    struct OpenAPI_Response **_out_val) {
  int diff = 0;
  size_t i;
  cdd_c_error_t rc;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;
  if (!op || !code)
    return CDD_C_SUCCESS;
  for (i = 0; i < op->n_responses; ++i) {
    if (op->responses[i].code) {
      rc = c_cdd_stricmp(op->responses[i].code, code, &diff);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (diff == 0) {
        *_out_val = &op->responses[i];
        return CDD_C_SUCCESS;
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
 * @brief Applies example to response.
 */
cdd_c_error_t apply_example_to_response(struct OpenAPI_Response *resp,
                                        const char *example,
                                        const char *content_type) {
  size_t i;
  struct OpenAPI_Any parsed;
  cdd_c_error_t rc;

  if (!resp || !example)
    return CDD_C_SUCCESS;

  memset(&parsed, 0, sizeof(parsed));

  if (resp->content_media_types && resp->n_content_media_types > 0) {
    rc = parse_example_any(example, &parsed);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (content_type) {
      struct OpenAPI_MediaType *mt = NULL;
      rc = find_media_type_op(resp->content_media_types,
                              resp->n_content_media_types, content_type, &mt);
      if (rc != CDD_C_SUCCESS) {
        free_any_value_local(&parsed);
        return rc;
      }
      if (mt && !mt->example_set) {
        rc = copy_any_value_local(&mt->example, &parsed);
        if (rc != CDD_C_SUCCESS) {
          free_any_value_local(&parsed);
          return rc;
        }
        mt->example_set = 1;
      }
      free_any_value_local(&parsed);
      return CDD_C_SUCCESS;
    }
    for (i = 0; i < resp->n_content_media_types; ++i) {
      struct OpenAPI_MediaType *mt = &resp->content_media_types[i];
      if (mt->example_set)
        continue;
      rc = copy_any_value_local(&mt->example, &parsed);
      if (rc != CDD_C_SUCCESS) {
        free_any_value_local(&parsed);
        return rc;
      }
      mt->example_set = 1;
    }
    free_any_value_local(&parsed);
    return CDD_C_SUCCESS;
  }

  if (resp->example_set)
    return CDD_C_SUCCESS;
  rc = parse_example_any(example, &resp->example);
  if (rc != CDD_C_SUCCESS)
    return rc;
  resp->example_set = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the ensure response for code operation.
 */
cdd_c_error_t ensure_response_for_code(struct OpenAPI_Operation *op,
                                       const char *code,
                                       struct OpenAPI_Response **_out_val) {
  int diff = 0;
  int is_200 = 0;
  struct OpenAPI_Response *resp = NULL;
  struct OpenAPI_Response *new_resps;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_ensure_response_for_code;
  extern C_CDD_EXPORT int g_op_fail_ensure_response_null;
  if (g_op_fail_ensure_response_for_code) {
    g_op_fail_ensure_response_for_code = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (g_op_fail_ensure_response_null) {
    g_op_fail_ensure_response_null = 0;
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
#endif

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;
  if (!op || !code)
    return CDD_C_SUCCESS;

  rc = find_response_by_code(op, code, &resp);
  if (rc != CDD_C_SUCCESS) {
    *_out_val = NULL;
    return rc;
  }
  if (resp) {
    *_out_val = resp;
    return CDD_C_SUCCESS;
  }

  new_resps = (struct OpenAPI_Response *)realloc(
      op->responses, (op->n_responses + 1) * sizeof(struct OpenAPI_Response));
  if (!new_resps) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  op->responses = new_resps;
  resp = &op->responses[op->n_responses];
  memset(resp, 0, sizeof(*resp));
  rc = c_cdd_strdup(code, &resp->code);
  if (rc != CDD_C_SUCCESS) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  rc = c_cdd_stricmp(code, "200", &diff);
  if (rc != CDD_C_SUCCESS) {
    free(resp->code);
    resp->code = NULL;
    *_out_val = NULL;
    return rc;
  }
  is_200 = (diff == 0);
  rc = c_cdd_strdup(is_200 ? "Success" : "Response", &resp->description);
  if (rc != CDD_C_SUCCESS) {
    free(resp->code);
    resp->code = NULL;
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  op->n_responses++;
  *_out_val = resp;
  return CDD_C_SUCCESS;
}

static void free_header_fields(struct OpenAPI_Header *hdr) {
  free(hdr->name);
  hdr->name = NULL;
  if (hdr->description) {
    free(hdr->description);
    hdr->description = NULL;
  }
  if (hdr->type) {
    free(hdr->type);
    hdr->type = NULL;
  }
  if (hdr->content_type) {
    free(hdr->content_type);
    hdr->content_type = NULL;
  }
  if (hdr->schema.inline_type) {
    free(hdr->schema.inline_type);
    hdr->schema.inline_type = NULL;
  }
  if (hdr->schema.format) {
    free(hdr->schema.format);
    hdr->schema.format = NULL;
  }
}

/**
 * @brief Frees dynamically allocated fields of an OpenAPI Encoding.
 */
cdd_c_error_t free_encoding_fields(struct OpenAPI_Encoding *enc) {
  if (!enc)
    return CDD_C_SUCCESS;
  if (enc->name) {
    free(enc->name);
    enc->name = NULL;
  }
  if (enc->content_type) {
    free(enc->content_type);
    enc->content_type = NULL;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets header to response.
 */
cdd_c_error_t add_header_to_response(struct OpenAPI_Response *resp,
                                     const struct DocResponseHeader *dh) {
  struct OpenAPI_Header *new_headers;
  struct OpenAPI_Header *hdr;
  size_t i;
  cdd_c_error_t rc;

  if (!resp || !dh || !dh->name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < resp->n_headers; ++i) {
    if (resp->headers[i].name) {
      int diff = 0;
      rc = c_cdd_stricmp(resp->headers[i].name, dh->name, &diff);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (diff == 0) {
        hdr = &resp->headers[i];
        if (dh->description && !hdr->description) {
          rc = c_cdd_strdup(dh->description, &hdr->description);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        if (dh->type && !hdr->type) {
          rc = c_cdd_strdup(dh->type, &hdr->type);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        if (dh->content_type && !hdr->content_type) {
          rc = c_cdd_strdup(dh->content_type, &hdr->content_type);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        if (dh->format) {
          hdr->schema_set = 1;
          if (!hdr->schema.inline_type) {
            rc = c_cdd_strdup(hdr->type ? hdr->type : "string",
                              &hdr->schema.inline_type);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (hdr->schema.format) {
            free(hdr->schema.format);
            hdr->schema.format = NULL;
          }
          rc = c_cdd_strdup(dh->format, &hdr->schema.format);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        if (dh->required_set)
          hdr->required = dh->required ? 1 : 0;
        if (dh->example && !hdr->example_set) {
          rc = parse_example_any(dh->example, &hdr->example);
          if (rc != CDD_C_SUCCESS)
            return rc;
          hdr->example_set = 1;
          hdr->example_location =
              hdr->content_type ? OA_EXAMPLE_LOC_MEDIA : OA_EXAMPLE_LOC_OBJECT;
        }
        return CDD_C_SUCCESS;
      }
    }
  }

  new_headers = (struct OpenAPI_Header *)realloc(
      resp->headers, (resp->n_headers + 1) * sizeof(struct OpenAPI_Header));
  if (!new_headers) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  resp->headers = new_headers;
  hdr = &resp->headers[resp->n_headers++];
  memset(hdr, 0, sizeof(*hdr));
  rc = c_cdd_strdup(dh->name, &hdr->name);
  if (rc != CDD_C_SUCCESS) {
    resp->n_headers--;
    return rc;
  }
  if (dh->description) {
    rc = c_cdd_strdup(dh->description, &hdr->description);
    if (rc != CDD_C_SUCCESS) {
      free_header_fields(hdr);
      resp->n_headers--;
      return rc;
    }
  }
  rc = c_cdd_strdup(dh->type ? dh->type : "string", &hdr->type);
  if (rc != CDD_C_SUCCESS) {
    free_header_fields(hdr);
    resp->n_headers--;
    return rc;
  }
  if (dh->content_type) {
    rc = c_cdd_strdup(dh->content_type, &hdr->content_type);
    if (rc != CDD_C_SUCCESS) {
      free_header_fields(hdr);
      resp->n_headers--;
      return rc;
    }
  }
  if (dh->format) {
    hdr->schema_set = 1;
    rc = c_cdd_strdup(hdr->type, &hdr->schema.inline_type);
    if (rc != CDD_C_SUCCESS) {
      free_header_fields(hdr);
      resp->n_headers--;
      return rc;
    }
    rc = c_cdd_strdup(dh->format, &hdr->schema.format);
    if (rc != CDD_C_SUCCESS) {
      free_header_fields(hdr);
      resp->n_headers--;
      return rc;
    }
  }
  if (dh->required_set)
    hdr->required = dh->required ? 1 : 0;
  if (dh->example) {
    rc = parse_example_any(dh->example, &hdr->example);
    if (rc != CDD_C_SUCCESS) {
      free_header_fields(hdr);
      resp->n_headers--;
      return rc;
    }
    hdr->example_set = 1;
    hdr->example_location =
        hdr->content_type ? OA_EXAMPLE_LOC_MEDIA : OA_EXAMPLE_LOC_OBJECT;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Cleans up dynamically allocated fields of an OpenAPI Link.
 */
cdd_c_error_t cleanup_link_fields(struct OpenAPI_Link *link) {
  if (!link)
    return CDD_C_SUCCESS;
  if (link->parameters) {
    size_t p;
    for (p = 0; p < link->n_parameters; ++p) {
      if (link->parameters[p].name)
        free(link->parameters[p].name);
      free_any_value_local(&link->parameters[p].value);
    }
    free(link->parameters);
    link->parameters = NULL;
    link->n_parameters = 0;
  }
  if (link->server) {
    if (link->server->name)
      free(link->server->name);
    if (link->server->url)
      free(link->server->url);
    if (link->server->description)
      free(link->server->description);
    free(link->server);
    link->server = NULL;
    link->server_set = 0;
  }
  if (link->operation_ref) {
    free(link->operation_ref);
    link->operation_ref = NULL;
  }
  if (link->operation_id) {
    free(link->operation_id);
    link->operation_id = NULL;
  }
  if (link->description) {
    free(link->description);
    link->description = NULL;
  }
  if (link->summary) {
    free(link->summary);
    link->summary = NULL;
  }
  if (link->name) {
    free(link->name);
    link->name = NULL;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets link to response.
 */
cdd_c_error_t add_link_to_response(struct OpenAPI_Response *resp,
                                   const struct DocLink *dl) {
  struct OpenAPI_Link *new_links;
  struct OpenAPI_Link *link;
  size_t i;
  cdd_c_error_t rc;

  if (!resp || !dl || !dl->name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if ((!dl->operation_id && !dl->operation_ref) ||
      (dl->operation_id && dl->operation_ref))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < resp->n_links; ++i) {
    if (resp->links[i].name && strcmp(resp->links[i].name, dl->name) == 0) {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }

  new_links = (struct OpenAPI_Link *)realloc(
      resp->links, (resp->n_links + 1) * sizeof(struct OpenAPI_Link));
  if (!new_links) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  resp->links = new_links;
  link = &resp->links[resp->n_links];
  memset(link, 0, sizeof(*link));

  rc = c_cdd_strdup(dl->name, &link->name);
  if (rc != CDD_C_SUCCESS) {
    if (resp->n_links == 0) {
      free(resp->links);
      resp->links = NULL;
    }
    return rc;
  }
  if (dl->summary) {
    rc = c_cdd_strdup(dl->summary, &link->summary);
    if (rc != CDD_C_SUCCESS) {
      cleanup_link_fields(link);
      return rc;
    }
  }
  if (dl->description) {
    rc = c_cdd_strdup(dl->description, &link->description);
    if (rc != CDD_C_SUCCESS) {
      cleanup_link_fields(link);
      return rc;
    }
  }
  if (dl->operation_id) {
    rc = c_cdd_strdup(dl->operation_id, &link->operation_id);
    if (rc != CDD_C_SUCCESS) {
      cleanup_link_fields(link);
      return rc;
    }
  }
  if (dl->operation_ref) {
    rc = c_cdd_strdup(dl->operation_ref, &link->operation_ref);
    if (rc != CDD_C_SUCCESS) {
      cleanup_link_fields(link);
      return rc;
    }
  }
  if (dl->parameters_json) {
    rc = parse_link_params_json(dl->parameters_json, &link->parameters,
                                &link->n_parameters);
    if (rc != CDD_C_SUCCESS) {
      cleanup_link_fields(link);
      return rc;
    }
  }
  if (dl->request_body_json) {
    rc = parse_example_any(dl->request_body_json, &link->request_body);
    if (rc != CDD_C_SUCCESS) {
      cleanup_link_fields(link);
      return rc;
    }
    link->request_body_set = 1;
  }
  if (dl->server_url) {
    link->server = (struct OpenAPI_Server *)calloc(1, sizeof(*link->server));
    if (!link->server) {
      cleanup_link_fields(link);
      return CDD_C_ERROR_MEMORY;
    }
    link->server_set = 1;
    rc = c_cdd_strdup(dl->server_url, &link->server->url);
    if (rc != CDD_C_SUCCESS) {
      cleanup_link_fields(link);
      return rc;
    }
    if (dl->server_name) {
      rc = c_cdd_strdup(dl->server_name, &link->server->name);
      if (rc != CDD_C_SUCCESS) {
        cleanup_link_fields(link);
        return rc;
      }
    }
    if (dl->server_description) {
      rc = c_cdd_strdup(dl->server_description, &link->server->description);
      if (rc != CDD_C_SUCCESS) {
        cleanup_link_fields(link);
        return rc;
      }
    }
  }

  resp->n_links++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets param to op.
 */
cdd_c_error_t add_param_to_op(struct OpenAPI_Operation *op,
                              struct OpenAPI_Parameter *p) {
  struct OpenAPI_Parameter *new_arr;
  size_t new_count;

  if (!op || !p)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  new_count = op->n_parameters + 1;
  new_arr = (struct OpenAPI_Parameter *)realloc(
      op->parameters, new_count * sizeof(struct OpenAPI_Parameter));
  if (!new_arr) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  op->parameters = new_arr;
  op->parameters[op->n_parameters] = *p; /* Copy struct */
  op->n_parameters = new_count;
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
 * @brief Frees dynamically allocated fields of an OpenAPI Parameter.
 */
cdd_c_error_t free_param_fields(struct OpenAPI_Parameter *p) {
  if (!p)
    return CDD_C_SUCCESS;
  if (p->name) {
    free(p->name);
    p->name = NULL;
  }
  if (p->type) {
    free(p->type);
    p->type = NULL;
  }
  if (p->description) {
    free(p->description);
    p->description = NULL;
  }
  if (p->items_type) {
    free(p->items_type);
    p->items_type = NULL;
  }
  if (p->content_type) {
    free(p->content_type);
    p->content_type = NULL;
  }
  if (p->schema.ref_name) {
    free(p->schema.ref_name);
    p->schema.ref_name = NULL;
  }
  if (p->schema.ref) {
    free(p->schema.ref);
    p->schema.ref = NULL;
  }
  if (p->schema.inline_type) {
    free(p->schema.inline_type);
    p->schema.inline_type = NULL;
  }
  if (p->schema.items_ref) {
    free(p->schema.items_ref);
    p->schema.items_ref = NULL;
  }
  if (p->schema.format) {
    free(p->schema.format);
    p->schema.format = NULL;
  }
  if (p->schema.items_format) {
    free(p->schema.items_format);
    p->schema.items_format = NULL;
  }
  if (p->schema.content_media_type) {
    free(p->schema.content_media_type);
    p->schema.content_media_type = NULL;
  }
  if (p->schema.content_encoding) {
    free(p->schema.content_encoding);
    p->schema.content_encoding = NULL;
  }
  if (p->schema.items_content_media_type) {
    free(p->schema.items_content_media_type);
    p->schema.items_content_media_type = NULL;
  }
  if (p->schema.items_content_encoding) {
    free(p->schema.items_content_encoding);
    p->schema.items_content_encoding = NULL;
  }
  if (p->example_set) {
    free_any_value_local(&p->example);
    p->example_set = 0;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the response has media type operation.
 */
cdd_c_error_t response_has_media_type(const struct OpenAPI_Response *resp,
                                      const char *name, int *out_has) {
  size_t i;
  if (!resp || !name || !out_has) {
    if (out_has)
      *out_has = 0;
    return CDD_C_SUCCESS;
  }
  if (resp->content_type && strcmp(resp->content_type, name) == 0) {
    *out_has = 1;
    return CDD_C_SUCCESS;
  }
  if (!resp->content_media_types || resp->n_content_media_types == 0) {
    *out_has = 0;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < resp->n_content_media_types; ++i) {
    const struct OpenAPI_MediaType *mt = &resp->content_media_types[i];
    if (mt->name && strcmp(mt->name, name) == 0) {
      *out_has = 1;
      return CDD_C_SUCCESS;
    }
  }
  *out_has = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the init media type from response operation.
 */
cdd_c_error_t init_media_type_from_response(struct OpenAPI_MediaType *mt,
                                            const char *name,
                                            const struct OpenAPI_Response *resp,
                                            int is_item_schema) {
  int has_data = 0;
  cdd_c_error_t rc;
  if (!mt || !name || !resp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(mt, 0, sizeof(*mt));
  rc = c_cdd_strdup(name, &mt->name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = schema_ref_has_data_basic(&resp->schema, &has_data);
  if (rc != CDD_C_SUCCESS) {
    free(mt->name);
    mt->name = NULL;
    return rc;
  }
  if (has_data) {
    if (is_item_schema) {
      rc = copy_schema_ref_basic(&mt->item_schema, &resp->schema);
      if (rc != CDD_C_SUCCESS) {
        free(mt->name);
        mt->name = NULL;
        return rc;
      }
      mt->item_schema_set = 1;
    } else {
      rc = copy_schema_ref_basic(&mt->schema, &resp->schema);
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
 * @brief Adds or sets response media type.
 */
cdd_c_error_t add_response_media_type(struct OpenAPI_Response *resp,
                                      const char *name, int is_item_schema) {
  struct OpenAPI_MediaType *new_mts;
  size_t new_count;
  int _has_mt = 0;

  if (!resp || !name || !*name)
    return CDD_C_SUCCESS;

  if (!resp->content_media_types) {
    size_t base = resp->content_type ? 1 : 0;
    resp->content_media_types = (struct OpenAPI_MediaType *)calloc(
        base + 1, sizeof(struct OpenAPI_MediaType));
    if (!resp->content_media_types) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    resp->n_content_media_types = 0;
    if (resp->content_type) {
      if (init_media_type_from_response(&resp->content_media_types[0],
                                        resp->content_type, resp,
                                        is_item_schema) != 0)
        return CDD_C_ERROR_MEMORY;
      resp->n_content_media_types = 1;
    }
  }

  response_has_media_type(resp, name, &_has_mt);
  if (_has_mt)
    return CDD_C_SUCCESS;

  new_count = resp->n_content_media_types + 1;
  new_mts = (struct OpenAPI_MediaType *)realloc(
      resp->content_media_types, new_count * sizeof(struct OpenAPI_MediaType));
  if (!new_mts) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  resp->content_media_types = new_mts;
  if (init_media_type_from_response(
          &resp->content_media_types[resp->n_content_media_types], name, resp,
          is_item_schema) != 0)
    return CDD_C_ERROR_MEMORY;
  resp->n_content_media_types = new_count;
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

/**
 * @brief Adds or sets querystring schema from type map.
 */
cdd_c_error_t set_querystring_schema_from_type_map(
    struct OpenAPI_Parameter *param,
    const struct OpenApiTypeMapping *type_map) {
  cdd_c_error_t rc;
  if (!param || !type_map)
    return CDD_C_SUCCESS;
  if (type_map->ref_name) {
    param->schema_set = 1;
    param->schema.is_array = (type_map->kind == OA_TYPE_ARRAY);
    rc = c_cdd_strdup(type_map->ref_name, &param->schema.ref_name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
  if (type_map->kind == OA_TYPE_ARRAY) {
    param->is_array = 1;
    rc = c_cdd_strdup("array", &param->type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (type_map->oa_type) {
      rc = c_cdd_strdup(type_map->oa_type, &param->items_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }
  if (type_map->oa_type) {
    rc = c_cdd_strdup(type_map->oa_type, &param->type);
  } else {
    rc = c_cdd_strdup("string", &param->type);
  }
  return rc;
}

/**
 * @brief Checks if an OpenAPI type name is a primitive type.
 *
 * @param[in] type Type name string.
 * @param[out] out_is_primitive Pointer to int receiving 1 if primitive, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, error code otherwise.
 */
cdd_c_error_t oa_type_is_primitive(const char *type, int *out_is_primitive) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_oa_type_is_primitive;
  if (g_op_fail_oa_type_is_primitive) {
    g_op_fail_oa_type_is_primitive = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (out_is_primitive)
    *out_is_primitive = 0;
  if (!type || !out_is_primitive)
    return CDD_C_SUCCESS;
  *out_is_primitive =
      (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0 ||
       strcmp(type, "string") == 0 || strcmp(type, "boolean") == 0)
          ? 1
          : 0;
  return CDD_C_SUCCESS;
}

/* Apply format from type mapping (or override) to a SchemaRef.
 * Returns: 1 if applied, 0 if not applicable, or ENOMEM on allocation failure.
 */
/**
 * @brief Applies format to schema ref.
 */
cdd_c_error_t apply_format_to_schema_ref(struct OpenAPI_SchemaRef *schema,
                                         const struct OpenApiTypeMapping *map,
                                         const char *override_format,
                                         int *out_applied) {
  const char *fmt;
  int is_prim = 0;
  cdd_c_error_t rc;

  if (out_applied)
    *out_applied = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_apply_format) {
    g_cdd_fail_apply_format = 0;
    return CDD_C_ERROR_MEMORY;
  }
#endif
  if (!schema || !map)
    return CDD_C_SUCCESS;
  fmt =
      (override_format && *override_format) ? override_format : map->oa_format;
  if (!fmt || !*fmt)
    return CDD_C_SUCCESS;
  if (!map->oa_type)
    return CDD_C_SUCCESS;
  rc = oa_type_is_primitive(map->oa_type, &is_prim);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!is_prim)
    return CDD_C_SUCCESS;

  if (map->kind == OA_TYPE_ARRAY) {
    schema->is_array = 1;
    if (!schema->inline_type) {
      rc = c_cdd_strdup(map->oa_type, &schema->inline_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (schema->items_format) {
      free(schema->items_format);
      schema->items_format = NULL;
    }
    rc = c_cdd_strdup(fmt, &schema->items_format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    if (!schema->inline_type) {
      rc = c_cdd_strdup(map->oa_type, &schema->inline_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (schema->format) {
      free(schema->format);
      schema->format = NULL;
    }
    rc = c_cdd_strdup(fmt, &schema->format);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (out_applied)
    *out_applied = 1;
  return CDD_C_SUCCESS;
}

/* --- Type Analysis --- */

/**
 * @brief Determine if a type is a struct pointer eligible for Body.
 * Heuristic: Contains "struct", ends with "*" or "**".
 */
/**
 * @brief Checks if struct pointer.
 */
cdd_c_error_t is_struct_pointer(const char *type, int *is_double_ptr,
                                int *out_is_struct_ptr) {
  const char *p;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_is_struct_pointer;
  if (g_op_fail_is_struct_pointer) {
    g_op_fail_is_struct_pointer = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (out_is_struct_ptr)
    *out_is_struct_ptr = 0;
  if (!type)
    return CDD_C_SUCCESS;
  if (!strstr(type, "struct "))
    return CDD_C_SUCCESS;

  p = strrchr(type, '*');
  if (!p)
    return CDD_C_SUCCESS;

  if (is_double_ptr)
    *is_double_ptr = (p > type && *(p - 1) == '*') ? 1 : 0;

  if (out_is_struct_ptr)
    *out_is_struct_ptr = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the doc style to openapi operation.
 */
cdd_c_error_t doc_style_to_openapi(enum DocParamStyle style,
                                   enum OpenAPI_Style *_out_val) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_op_fail_doc_style_to_openapi;
  if (g_op_fail_doc_style_to_openapi) {
    g_op_fail_doc_style_to_openapi = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (style) {
  case DOC_PARAM_STYLE_FORM: {
    *_out_val = OA_STYLE_FORM;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_SIMPLE: {
    *_out_val = OA_STYLE_SIMPLE;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_MATRIX: {
    *_out_val = OA_STYLE_MATRIX;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_LABEL: {
    *_out_val = OA_STYLE_LABEL;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_SPACE_DELIMITED: {
    *_out_val = OA_STYLE_SPACE_DELIMITED;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_PIPE_DELIMITED: {
    *_out_val = OA_STYLE_PIPE_DELIMITED;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_DEEP_OBJECT: {
    *_out_val = OA_STYLE_DEEP_OBJECT;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_COOKIE: {
    *_out_val = OA_STYLE_COOKIE;
    return CDD_C_SUCCESS;
  }
  case DOC_PARAM_STYLE_UNSET:
  default: {
    *_out_val = OA_STYLE_UNKNOWN;
    return CDD_C_SUCCESS;
  }
  }
}

/* --- Core Logic --- */

/**
 * @brief Executes the c2openapi build operation operation.
 */
cdd_c_error_t c2openapi_build_operation(const struct OpBuilderContext *ctx,
                                        struct OpenAPI_Operation *out_op) {
  const struct C2OpenAPI_ParsedSig *sig;
  const struct DocMetadata *doc;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!ctx || !out_op || !ctx->sig)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(out_op, 0, sizeof(*out_op));
  sig = ctx->sig;
  doc = ctx->doc;

  /* 0. Basic Metadata */
  if (doc && doc->verb) {
    int diff;
    /* Map string verb to enum */
    /* Only basic check here, loader does robust parsing */
    c_cdd_stricmp(doc->verb, "GET", &diff);
    if (diff == 0)
      out_op->verb = OA_VERB_GET;
    else {
      c_cdd_stricmp(doc->verb, "POST", &diff);
      if (diff == 0)
        out_op->verb = OA_VERB_POST;
      else {
        c_cdd_stricmp(doc->verb, "PUT", &diff);
        if (diff == 0)
          out_op->verb = OA_VERB_PUT;
        else {
          c_cdd_stricmp(doc->verb, "DELETE", &diff);
          if (diff == 0)
            out_op->verb = OA_VERB_DELETE;
          else {
            c_cdd_stricmp(doc->verb, "PATCH", &diff);
            if (diff == 0)
              out_op->verb = OA_VERB_PATCH;
            else {
              c_cdd_stricmp(doc->verb, "HEAD", &diff);
              if (diff == 0)
                out_op->verb = OA_VERB_HEAD;
              else {
                c_cdd_stricmp(doc->verb, "OPTIONS", &diff);
                if (diff == 0)
                  out_op->verb = OA_VERB_OPTIONS;
                else {
                  c_cdd_stricmp(doc->verb, "TRACE", &diff);
                  if (diff == 0)
                    out_op->verb = OA_VERB_TRACE;
                  else {
                    c_cdd_stricmp(doc->verb, "QUERY", &diff);
                    if (diff == 0)
                      out_op->verb = OA_VERB_QUERY;
                    else {
                      out_op->verb = OA_VERB_UNKNOWN;
                      out_op->is_additional = 1;
                      rc = c_cdd_strdup(doc->verb, &out_op->method);
                      if (rc != CDD_C_SUCCESS) {
                        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
                        return rc;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  } else {
    /* Guess from name? e.g. "api_get_..." */
    int is_prefix = 0;
    if (ctx->func_name) {
      c_cdd_str_starts_with(ctx->func_name, "api_post_", &is_prefix);
      if (is_prefix || strstr(ctx->func_name, "_create"))
        out_op->verb = OA_VERB_POST;
      else {
        c_cdd_str_starts_with(ctx->func_name, "api_put_", &is_prefix);
        if (is_prefix || strstr(ctx->func_name, "_update"))
          out_op->verb = OA_VERB_PUT;
        else {
          c_cdd_str_starts_with(ctx->func_name, "api_delete_", &is_prefix);
          if (is_prefix || strstr(ctx->func_name, "_delete"))
            out_op->verb = OA_VERB_DELETE;
          else
            out_op->verb = OA_VERB_GET;
        }
      }
    } else {
      out_op->verb = OA_VERB_GET;
    }
  }

  if (doc && doc->operation_id) {
    rc = c_cdd_strdup(doc->operation_id, &out_op->operation_id);
  } else {
    rc = c_cdd_strdup(ctx->func_name, &out_op->operation_id);
  }
  if (rc != CDD_C_SUCCESS) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return rc;
  }
  if (doc && doc->summary) {
    rc = c_cdd_strdup(doc->summary, &out_op->summary);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return rc;
    }
  }
  if (doc && doc->description) {
    rc = c_cdd_strdup(doc->description, &out_op->description);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return rc;
    }
  }
  if (doc && doc->deprecated_set) {
    out_op->deprecated = doc->deprecated ? 1 : 0;
  }
  if (doc && doc->external_docs_url) {
    rc = c_cdd_strdup(doc->external_docs_url, &out_op->external_docs.url);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (doc->external_docs_description) {
      rc = c_cdd_strdup(doc->external_docs_description,
                        &out_op->external_docs.description);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (doc && doc->n_tags > 0) {
    size_t t;
    out_op->tags = (char **)calloc(doc->n_tags, sizeof(char *));
    if (!out_op->tags) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    out_op->n_tags = doc->n_tags;
    for (t = 0; t < doc->n_tags; ++t) {
      rc = c_cdd_strdup(doc->tags[t], &out_op->tags[t]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (doc && doc->n_security > 0) {
    size_t s;
    out_op->security = (struct OpenAPI_SecurityRequirementSet *)calloc(
        doc->n_security, sizeof(struct OpenAPI_SecurityRequirementSet));
    if (!out_op->security) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    out_op->n_security = doc->n_security;
    out_op->security_set = 1;
    for (s = 0; s < doc->n_security; ++s) {
      struct OpenAPI_SecurityRequirementSet *set = &out_op->security[s];
      const struct DocSecurityRequirement *src = &doc->security[s];
      set->requirements = (struct OpenAPI_SecurityRequirement *)calloc(
          1, sizeof(struct OpenAPI_SecurityRequirement));
      if (!set->requirements) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }
      set->n_requirements = 1;
      rc = c_cdd_strdup(src->scheme, &set->requirements[0].scheme);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (src->n_scopes > 0) {
        size_t k;
        set->requirements[0].scopes =
            (char **)calloc(src->n_scopes, sizeof(char *));
        if (!set->requirements[0].scopes)
          return CDD_C_ERROR_MEMORY;
        set->requirements[0].n_scopes = src->n_scopes;
        for (k = 0; k < src->n_scopes; ++k) {
          rc = c_cdd_strdup(src->scopes[k], &set->requirements[0].scopes[k]);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }

  if (doc && doc->n_servers > 0) {
    size_t s;
    out_op->servers = (struct OpenAPI_Server *)calloc(
        doc->n_servers, sizeof(struct OpenAPI_Server));
    if (!out_op->servers) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    out_op->n_servers = doc->n_servers;
    for (s = 0; s < doc->n_servers; ++s) {
      const struct DocServer *src = &doc->servers[s];
      if (src->url) {
        rc = c_cdd_strdup(src->url, &out_op->servers[s].url);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (src->name) {
        rc = c_cdd_strdup(src->name, &out_op->servers[s].name);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (src->description) {
        rc = c_cdd_strdup(src->description, &out_op->servers[s].description);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (src->n_variables > 0) {
        cdd_c_error_t vrc =
            copy_doc_server_variables_op(&out_op->servers[s], src);
        if (vrc != CDD_C_SUCCESS)
          return vrc;
      }
    }
  }

  /* 1. Argument Iteration */
  for (i = 0; i < sig->n_args; ++i) {
    const struct C2OpenAPI_ParsedArg *arg = &sig->args[i];
    struct DocParam *dp = NULL;
    struct OpenAPI_Parameter curr_param;
    struct OpenApiTypeMapping type_map;
    int is_path = 0;
    int is_body = 0;
    int is_out_ptr = 0;
    int is_querystring = 0;
    int _is_reserved = 0;
    int _is_path_param = 0;

    memset(&curr_param, 0, sizeof(curr_param));
    rc = c_mapping_init(&type_map);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = find_doc_param(doc, arg->name, &dp);
    if (rc != CDD_C_SUCCESS) {
      c_mapping_free(&type_map);
      return rc;
    }

    /* --- Heuristic: Role Detection --- */

    /* A. Explicit Documentation Override */
    if (dp && dp->in_loc) {
      if (strcmp(dp->in_loc, "path") == 0)
        is_path = 1;
      else if (strcmp(dp->in_loc, "querystring") == 0)
        is_querystring = 1;
      /* "body" isn't a parameter location in OpenAPI 3, but a concept.
         If user says @param [in:body], we treat as Body. */
      else if (strcmp(dp->in_loc, "body") == 0)
        is_body = 1;
    } else {
      /* B. Implicit Path: Matches {name} in route */
      if (doc && doc->route) {
        rc = is_path_param(doc->route, arg->name, &_is_path_param);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
        if (_is_path_param)
          is_path = 1;
      }
      /* C. Implicit Body: "struct *" without const in POST/PUT/PATCH? */
      if (!is_path) {
        int is_double = 0;
        int _is_struct_ptr = 0;
        rc = is_struct_pointer(arg->type, &is_double, &_is_struct_ptr);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
        if (_is_struct_ptr) {
          if (is_double) {
            /* Double pointer usually `struct X **out` -> Response Body (Output)
             */
            is_out_ptr = 1;
          } else if (strstr(arg->type, "const ")) {
            /* `const struct X *in` -> Request Body */
            if (out_op->verb == OA_VERB_POST || out_op->verb == OA_VERB_PUT ||
                out_op->verb == OA_VERB_PATCH) {
              is_body = 1;
            }
          } else {
            /* `struct X *` (non-const) is ambiguous.
               Could be in-out, or body.
               Default to Request Body for state-changing verbs. */
            if (out_op->verb == OA_VERB_POST || out_op->verb == OA_VERB_PUT) {
              is_body = 1;
            }
          }
        } else if (strstr(arg->type, "**")) {
          is_out_ptr = 1;
        }
      }
    }

    /* Analyze Type using C Mapper */
    rc = c_mapping_map_type(arg->type, arg->name, &type_map);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }

    if (is_out_ptr) {
      /* This is an output parameter (Response Body Schema).
         We store it to populate a "200 OK" response later. */
      struct OpenAPI_Response *r;
      size_t r_idx = out_op->n_responses; /* Add new response */
      struct OpenAPI_Response *new_resps = (struct OpenAPI_Response *)realloc(
          out_op->responses,
          (out_op->n_responses + 1) * sizeof(struct OpenAPI_Response));
      if (!new_resps) {
        c_mapping_free(&type_map);
        return CDD_C_ERROR_MEMORY;
      }
      out_op->responses = new_resps;
      r = &out_op->responses[r_idx];
      memset(r, 0, sizeof(*r));
      rc = c_cdd_strdup("200", &r->code);
      if (rc != CDD_C_SUCCESS) {
        c_mapping_free(&type_map);
        return rc;
      }
      rc = c_cdd_strdup("Success", &r->description);
      if (rc != CDD_C_SUCCESS) {
        free(r->code);
        r->code = NULL;
        c_mapping_free(&type_map);
        return rc;
      }
      out_op->n_responses++;

      /* Map Schema */
      r->schema.is_array = (type_map.kind == OA_TYPE_ARRAY);
      if (type_map.ref_name) {
        rc = c_cdd_strdup(type_map.ref_name, &r->schema.ref_name);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
      } else {
        rc = c_cdd_strdup(type_map.oa_type, &r->schema.inline_type);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
      }
      {
        int fmt_applied = 0;
        rc = apply_format_to_schema_ref(&r->schema, &type_map, NULL,
                                        &fmt_applied);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
      }

      c_mapping_free(&type_map);
      continue; /* Done with this arg */
    }

    if (is_body) {
      /* Request Body Population */
      rc = c_cdd_strdup("application/json", &out_op->req_body.content_type);
      if (rc != CDD_C_SUCCESS) {
        c_mapping_free(&type_map);
        return rc;
      }
      out_op->req_body.is_array = (type_map.kind == OA_TYPE_ARRAY);
      /* Use ref_name if object, or type if primitive */
      if (type_map.ref_name) {
        rc = c_cdd_strdup(type_map.ref_name, &out_op->req_body.ref_name);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
      } else {
        rc = c_cdd_strdup(type_map.oa_type, &out_op->req_body.inline_type);
        if (rc != CDD_C_SUCCESS) {
          c_mapping_free(&type_map);
          return rc;
        }
      }
      out_op->req_body_required = 1;
      out_op->req_body_required_set = 1;
      {
        int fmt_applied = 0;
        rc = apply_format_to_schema_ref(&out_op->req_body, &type_map, NULL,
                                        &fmt_applied);
        if (rc != CDD_C_SUCCESS) {
          free(out_op->req_body.content_type);
          out_op->req_body.content_type = NULL;
          if (out_op->req_body.ref_name) {
            free(out_op->req_body.ref_name);
            out_op->req_body.ref_name = NULL;
          }
          if (out_op->req_body.inline_type) {
            free(out_op->req_body.inline_type);
            out_op->req_body.inline_type = NULL;
          }
          c_mapping_free(&type_map);
          return rc;
        }
      }

      c_mapping_free(&type_map);
      continue;
    }

    /* --- Standard Parameter (Query/Path/Header/Cookie) --- */

    rc = c_cdd_strdup(arg->name, &curr_param.name);
    if (rc != CDD_C_SUCCESS) {
      c_mapping_free(&type_map);
      return rc;
    }
    curr_param.required = is_path; /* Path params always required */
    if (dp && dp->required)
      curr_param.required = 1;
    if (dp && dp->description) {
      rc = c_cdd_strdup(dp->description, &curr_param.description);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
    }

    curr_param.in = is_path ? OA_PARAM_IN_PATH : OA_PARAM_IN_QUERY;
    if (dp && dp->in_loc && strcmp(dp->in_loc, "header") == 0)
      curr_param.in = OA_PARAM_IN_HEADER;
    else if (dp && dp->in_loc && strcmp(dp->in_loc, "cookie") == 0)
      curr_param.in = OA_PARAM_IN_COOKIE;
    else if (is_querystring)
      curr_param.in = OA_PARAM_IN_QUERYSTRING;

    if (curr_param.in == OA_PARAM_IN_HEADER) {
      rc = is_reserved_header_name(curr_param.name, &_is_reserved);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
      if (_is_reserved) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        continue;
      }
    }

    /* Map Types */
    if (is_querystring) {
      rc = c_cdd_strdup("application/x-www-form-urlencoded",
                        &curr_param.content_type);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
      rc = set_querystring_schema_from_type_map(&curr_param, &type_map);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
    } else if (type_map.kind == OA_TYPE_ARRAY) {
      curr_param.is_array = 1;
      /* Logic for items_type: c_mapper stores item type in oa_type/ref_name
       * when kind=ARRAY */
      if (type_map.oa_type) {
        rc = c_cdd_strdup(type_map.oa_type, &curr_param.items_type);
        if (rc != CDD_C_SUCCESS) {
          free_param_fields(&curr_param);
          c_mapping_free(&type_map);
          return rc;
        }
      } else {
        rc = c_cdd_strdup(type_map.ref_name, &curr_param.items_type);
        if (rc != CDD_C_SUCCESS) {
          free_param_fields(&curr_param);
          c_mapping_free(&type_map);
          return rc;
        }
      }

      rc = c_cdd_strdup("array", &curr_param.type);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
    } else {
      /* Primitive / Object (if scalar param is allowed object??) usually string
       */
      /* Spec allows object parameters but they serialize weirdly. Assume string
       * representation unless primitive. */
      if (type_map.oa_type)
        rc = c_cdd_strdup(type_map.oa_type, &curr_param.type);
      else
        rc = c_cdd_strdup("string", &curr_param.type);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
    }

    {
      const char *fmt_override = (dp && dp->format) ? dp->format : NULL;
      int fmt_applied = 0;
      rc = apply_format_to_schema_ref(&curr_param.schema, &type_map,
                                      fmt_override, &fmt_applied);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
      if (fmt_applied) {
        if (dp && dp->item_schema)
          curr_param.item_schema_set = 1;
        else
          curr_param.schema_set = 1;
      }
    }

    if (dp) {
      if (dp->content_type) {
        if (curr_param.content_type)
          free(curr_param.content_type);
        rc = c_cdd_strdup(dp->content_type, &curr_param.content_type);
        if (rc != CDD_C_SUCCESS) {
          free_param_fields(&curr_param);
          c_mapping_free(&type_map);
          return rc;
        }
      }
      if (!curr_param.content_type) {
        if (dp->style_set) {
          enum OpenAPI_Style style = OA_STYLE_UNKNOWN;
          rc = doc_style_to_openapi(dp->style, &style);
          if (rc != CDD_C_SUCCESS) {
            free_param_fields(&curr_param);
            c_mapping_free(&type_map);
            return rc;
          }
          if (style != OA_STYLE_UNKNOWN)
            curr_param.style = style;
        }
        if (dp->explode_set) {
          curr_param.explode_set = 1;
          curr_param.explode = dp->explode ? 1 : 0;
        }
        if (dp->allow_reserved_set) {
          curr_param.allow_reserved_set = 1;
          curr_param.allow_reserved = dp->allow_reserved ? 1 : 0;
        }
        if (dp->allow_empty_value_set) {
          curr_param.allow_empty_value_set = 1;
          curr_param.allow_empty_value = dp->allow_empty_value ? 1 : 0;
        }
      }
      if (dp->deprecated_set) {
        curr_param.deprecated_set = 1;
        curr_param.deprecated = dp->deprecated ? 1 : 0;
      }
    }
    if (!curr_param.content_type && (!dp || !dp->style_set)) {
      if (curr_param.in == OA_PARAM_IN_QUERY ||
          curr_param.in == OA_PARAM_IN_COOKIE)
        curr_param.style = OA_STYLE_FORM;
      else
        curr_param.style = OA_STYLE_SIMPLE;
    }

    if (dp && dp->example) {
      rc = parse_example_any(dp->example, &curr_param.example);
      if (rc != CDD_C_SUCCESS) {
        free_param_fields(&curr_param);
        c_mapping_free(&type_map);
        return rc;
      }
      curr_param.example_set = 1;
      if (curr_param.content_type) {
        curr_param.example_location = OA_EXAMPLE_LOC_MEDIA;
      } else {
        curr_param.example_location = OA_EXAMPLE_LOC_OBJECT;
      }
    }

    rc = add_param_to_op(out_op, &curr_param);
    c_mapping_free(&type_map);
    if (rc != CDD_C_SUCCESS) {
      free_param_fields(&curr_param);
      return rc;
    }
  }

  if (doc) {
    if (doc->n_request_bodies > 0) {
      size_t rb_idx;
      for (rb_idx = 0; rb_idx < doc->n_request_bodies; ++rb_idx) {
        const struct DocRequestBody *rb = &doc->request_bodies[rb_idx];
        const char *rb_content_type =
            rb->content_type ? rb->content_type : "application/json";
        if (rb_idx == 0) {
          if (out_op->req_body.content_type)
            free(out_op->req_body.content_type);
          rc = c_cdd_strdup(rb_content_type, &out_op->req_body.content_type);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        rc = add_request_body_media_type(out_op, rb_content_type,
                                         rb->item_schema);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (rb->example) {
          struct OpenAPI_MediaType *mt = NULL;
          rc = find_media_type_op(out_op->req_body_media_types,
                                  out_op->n_req_body_media_types,
                                  rb_content_type, &mt);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = apply_example_to_media_type(mt, rb->example);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }

        if (doc->n_encodings > 0) {
          struct OpenAPI_MediaType *mt = NULL;
          size_t enc_i;
          rc = find_media_type_op(out_op->req_body_media_types,
                                  out_op->n_req_body_media_types,
                                  rb_content_type, &mt);
          if (rc != CDD_C_SUCCESS)
            return rc;
          for (enc_i = 0; enc_i < doc->n_encodings; ++enc_i) {
            const struct DocEncoding *d_enc = &doc->encodings[enc_i];
            struct OpenAPI_Encoding enc;
            memset(&enc, 0, sizeof(enc));

            if (d_enc->name) {
              rc = c_cdd_strdup(d_enc->name, &enc.name);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
            if (d_enc->content_type) {
              rc = c_cdd_strdup(d_enc->content_type, &enc.content_type);
              if (rc != CDD_C_SUCCESS) {
                free_encoding_fields(&enc);
                return rc;
              }
            }
            if (d_enc->style) {
              rc = doc_style_to_openapi(d_enc->style, &enc.style);
              if (rc != CDD_C_SUCCESS) {
                free_encoding_fields(&enc);
                return rc;
              }
            }

            enc.explode = d_enc->explode;
            enc.explode_set = d_enc->explode_set;
            enc.allow_reserved = d_enc->allow_reserved;
            enc.allow_reserved_set = d_enc->allow_reserved_set;

            if (d_enc->kind == 1) {
              struct OpenAPI_Encoding *new_encs = realloc(
                  mt->prefix_encoding, (mt->n_prefix_encoding + 1) *
                                           sizeof(struct OpenAPI_Encoding));
              if (!new_encs) {
                free_encoding_fields(&enc);
                return CDD_C_ERROR_MEMORY;
              }
              mt->prefix_encoding = new_encs;
              mt->prefix_encoding[mt->n_prefix_encoding++] = enc;
            } else if (d_enc->kind == 2) {
              if (!mt->item_encoding) {
                mt->item_encoding = calloc(1, sizeof(struct OpenAPI_Encoding));
                if (!mt->item_encoding) {
                  free_encoding_fields(&enc);
                  return CDD_C_ERROR_MEMORY;
                }
              }
              *mt->item_encoding = enc;
            } else {
              struct OpenAPI_Encoding *new_encs =
                  realloc(mt->encoding, (mt->n_encoding + 1) *
                                            sizeof(struct OpenAPI_Encoding));
              if (!new_encs) {
                free_encoding_fields(&enc);
                return CDD_C_ERROR_MEMORY;
              }
              mt->encoding = new_encs;
              mt->encoding[mt->n_encoding++] = enc;
            }
          }
        }
      }
    }
    if (doc->request_body_description) {
      rc = c_cdd_strdup(doc->request_body_description,
                        &out_op->req_body_description);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return rc;
      }
    }
    if (doc->request_body_required_set) {
      out_op->req_body_required_set = 1;
      out_op->req_body_required = doc->request_body_required ? 1 : 0;
    }
    if (doc->request_body_content_type && doc->n_request_bodies == 0) {
      if (out_op->req_body.content_type)
        free(out_op->req_body.content_type);
      rc = c_cdd_strdup(doc->request_body_content_type,
                        &out_op->req_body.content_type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  /* 2. Responses (Doc overrides) */
  if (doc && doc->n_returns > 0) {
    /* If doc has specific returns, use them. If we identified an output body
     * earlier (200), we might merge. */
    /* Simple logic: If strict error codes documented, add them. */
    for (i = 0; i < doc->n_returns; ++i) {
      /* Check if response code already exists (e.g. 200 from output param) */
      int exists = 0;
      size_t k;
      for (k = 0; k < out_op->n_responses; ++k) {
        if (doc->returns[i].code &&
            strcmp(out_op->responses[k].code, doc->returns[i].code) == 0) {
          exists = 1;
          if (!out_op->responses[k].summary && doc->returns[i].summary) {
            rc = c_cdd_strdup(doc->returns[i].summary,
                              &out_op->responses[k].summary);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (doc->returns[i].description) {
            if (out_op->responses[k].description)
              free(out_op->responses[k].description);
            rc = c_cdd_strdup(doc->returns[i].description,
                              &out_op->responses[k].description);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (doc->returns[i].content_type) {
            rc = add_response_media_type(&out_op->responses[k],
                                         doc->returns[i].content_type,
                                         doc->returns[i].item_schema);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (!out_op->responses[k].content_type) {
              rc = c_cdd_strdup(doc->returns[i].content_type,
                                &out_op->responses[k].content_type);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
          if (doc->returns[i].example) {
            rc = apply_example_to_response(&out_op->responses[k],
                                           doc->returns[i].example,
                                           doc->returns[i].content_type);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          break;
        }
      }
      if (!exists) {
        /* Add new response (likely error code) */
        struct OpenAPI_Response *new_resps = (struct OpenAPI_Response *)realloc(
            out_op->responses,
            (out_op->n_responses + 1) * sizeof(struct OpenAPI_Response));
        struct OpenAPI_Response *r;
        if (!new_resps) {
          C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
          return CDD_C_ERROR_MEMORY;
        }
        out_op->responses = new_resps;
        r = &out_op->responses[out_op->n_responses++];
        memset(r, 0, sizeof(*r));
        rc = c_cdd_strdup(doc->returns[i].code, &r->code);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (doc->returns[i].summary) {
          rc = c_cdd_strdup(doc->returns[i].summary, &r->summary);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
            return rc;
          }
        }
        if (doc->returns[i].description) {
          rc = c_cdd_strdup(doc->returns[i].description, &r->description);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
            return rc;
          }
        }
        if (doc->returns[i].content_type) {
          rc = add_response_media_type(r, doc->returns[i].content_type,
                                       doc->returns[i].item_schema);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = c_cdd_strdup(doc->returns[i].content_type, &r->content_type);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
            return rc;
          }
        }
        if (doc->returns[i].example) {
          rc = apply_example_to_response(r, doc->returns[i].example,
                                         doc->returns[i].content_type);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        /* Schema for error is usually generic Error struct, logic outside scope
         * here, leaves NULL */
      }
    }
  }

  if (doc && doc->n_response_headers > 0) {
    for (i = 0; i < doc->n_response_headers; ++i) {
      struct OpenAPI_Response *resp = NULL;
      rc = ensure_response_for_code(out_op, doc->response_headers[i].code,
                                    &resp);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (!resp) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }
      rc = add_header_to_response(resp, &doc->response_headers[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (doc && doc->n_links > 0) {
    for (i = 0; i < doc->n_links; ++i) {
      struct OpenAPI_Response *resp = NULL;
      rc = ensure_response_for_code(out_op, doc->links[i].code, &resp);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (!resp) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }
      rc = add_link_to_response(resp, &doc->links[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (out_op->n_responses == 0) {
    struct OpenAPI_Response *new_resps = (struct OpenAPI_Response *)realloc(
        out_op->responses, sizeof(struct OpenAPI_Response));
    struct OpenAPI_Response *r;
    if (!new_resps) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    out_op->responses = new_resps;
    r = &out_op->responses[out_op->n_responses++];
    memset(r, 0, sizeof(*r));
    rc = c_cdd_strdup("200", &r->code);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return rc;
    }
    rc = c_cdd_strdup("Success", &r->description);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return rc;
    }
  }

  /* 3. Global Tags */
  /* Heuristic: use first part of function name? e.g. api_pet_get -> "pet" */
  if (ctx->func_name && out_op->n_tags == 0) {
    char *dup_name = NULL;
    rc = c_cdd_strdup(ctx->func_name, &dup_name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    {
      char *token;
      char *ctx_ptr = NULL;
/* assume snake case */
#ifdef _WIN32
      token = strtok_s(dup_name, "_", &ctx_ptr);
#else
      token = strtok_r(dup_name, "_", &ctx_ptr);
#endif /* prefix */
#ifdef _WIN32
      token = strtok_s(NULL, "_", &ctx_ptr);
#else
      token = strtok_r(NULL, "_", &ctx_ptr);
#endif /* resource or next */
      if (token) {
        out_op->tags = (char **)malloc(sizeof(char *));
        if (out_op->tags) {
          rc = c_cdd_strdup(token, &out_op->tags[0]);
          if (rc == CDD_C_SUCCESS) {
            out_op->tags[0][0] = (char)toupper(
                (unsigned char)out_op->tags[0][0]); /* Capitalize */
            out_op->n_tags = 1;
          }
        }
      }
      free(dup_name);
    }
  }

  return CDD_C_SUCCESS;
}
