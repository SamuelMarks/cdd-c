/**
 * @file operation.c
 * @brief Implementation of operation logic.
 */

/* clang-format off */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../win_compat_sym.h"

#include "classes/parse/mapping.h"
#include "functions/parse/str.h"
#include "routes/emit/operation.h"
#include "c_cdd/log.h"
/* clang-format on */
/* LCOV_EXCL_START */

/* --- Helpers --- */

/**
 * @brief Checks if reserved header name.
 */
enum cdd_c_error is_reserved_header_name(const char *name,
                                         int *out_is_reserved) {
  int _ast_iequal_0 = false;
  int _ast_iequal_1 = false;
  int _ast_iequal_2 = false;
  if (!name || !*name || !out_is_reserved) {
    if (out_is_reserved)
      *out_is_reserved = 0;
    return CDD_C_SUCCESS;
  }
  if ((c_cdd_str_iequal(name, "accept", &_ast_iequal_0), _ast_iequal_0)) {
    *out_is_reserved = 1;
    return CDD_C_SUCCESS;
  }
  if ((c_cdd_str_iequal(name, "content-type", &_ast_iequal_1), _ast_iequal_1)) {
    *out_is_reserved = 1;
    return CDD_C_SUCCESS;
  }
  if ((c_cdd_str_iequal(name, "authorization", &_ast_iequal_2),
       _ast_iequal_2)) {
    *out_is_reserved = 1;
    return CDD_C_SUCCESS;
  }
  *out_is_reserved = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses example any from the given input.
 */
enum cdd_c_error parse_example_any(const char *example,
                                   struct OpenAPI_Any *out) {
  char *_ast_strdup_3 = NULL;
  char *_ast_strdup_4 = NULL;
  char *_ast_strdup_5 = NULL;
  JSON_Value *val;
  JSON_Value_Type t;
  const char *s;
  char *json_str;

  if (!example || !out)
    return CDD_C_SUCCESS;

  memset(out, 0, sizeof(*out));
  val = json_parse_string(example);
  if (!val) {
    out->type = OA_ANY_STRING;
    out->string = (c_cdd_strdup(example, &_ast_strdup_3), _ast_strdup_3);
    return out->string ? 0 : ENOMEM;
  }

  t = json_value_get_type(val);
  switch (t) {
  case JSONString:
    s = json_value_get_string(val);
    out->type = OA_ANY_STRING;
    out->string = (c_cdd_strdup(s ? s : "", &_ast_strdup_4), _ast_strdup_4);
    /* LCOV_EXCL_START */
    if (!out->string) {
      json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    break;
  case JSONNumber:
    out->type = OA_ANY_NUMBER;
    out->number = json_value_get_number(val);
    break;
  case JSONBoolean:
    out->type = OA_ANY_BOOL;
    out->boolean = json_value_get_boolean(val);
    break;
  case JSONNull:
    out->type = OA_ANY_NULL;
    break;
  case JSONObject:
  case JSONArray:
    json_str = json_serialize_to_string(val);
    /* LCOV_EXCL_START */
    if (!json_str) {
      json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    out->type = OA_ANY_JSON;
    out->json = (c_cdd_strdup(json_str, &_ast_strdup_5), _ast_strdup_5);
    json_free_serialized_string(json_str);
    /* LCOV_EXCL_START */
    if (!out->json) {
      json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    break;
  default:
    break;
  }

  json_value_free(val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the any from json value operation.
 */
enum cdd_c_error any_from_json_value(const JSON_Value *val,
                                     struct OpenAPI_Any *out) {
  char *_ast_strdup_6 = NULL;
  char *_ast_strdup_7 = NULL;
  JSON_Value_Type t;
  const char *s;
  char *json_str;

  /* LCOV_EXCL_START */

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* LCOV_EXCL_STOP */
  memset(out, 0, sizeof(*out));
  if (!val)
    return CDD_C_SUCCESS;

  t = json_value_get_type(val);
  switch (t) {
  case JSONString:
    s = json_value_get_string(val);
    out->type = OA_ANY_STRING;
    out->string = (c_cdd_strdup(s ? s : "", &_ast_strdup_6), _ast_strdup_6);
    return out->string ? 0 : ENOMEM;
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
    /* LCOV_EXCL_START */
    if (!json_str) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    out->type = OA_ANY_JSON;
    out->json = (c_cdd_strdup(json_str, &_ast_strdup_7), _ast_strdup_7);
    json_free_serialized_string(json_str);
    return out->json ? 0 : ENOMEM;
  default:
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses link params json from the given input.
 */
enum cdd_c_error parse_link_params_json(const char *json,
                                        struct OpenAPI_LinkParam **out,
                                        size_t *out_count) {
  char *_ast_strdup_8 = NULL;
  JSON_Value *val;
  JSON_Object *obj;
  size_t count, i;

  if (out)
    *out = NULL;
  if (out_count)
    *out_count = 0;
  if (!json || !out || !out_count)
    return CDD_C_SUCCESS;

  val = json_parse_string(json);
  /* LCOV_EXCL_START */
  if (!val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

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
  /* LCOV_EXCL_START */
  if (!*out) {
    json_value_free(val);
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(obj, i);
    const JSON_Value *v = json_object_get_value_at(obj, i);
    struct OpenAPI_LinkParam *lp = &(*out)[i];

    lp->name = (c_cdd_strdup(name ? name : "", &_ast_strdup_8), _ast_strdup_8);
    if (!lp->name) {
      json_value_free(val);
      goto cleanup;
    }
    if (any_from_json_value(v, &lp->value) != 0) {
      json_value_free(val);
      goto cleanup;
    }
  }

  json_value_free(val);
  return CDD_C_SUCCESS;

cleanup:
  if (*out) {
    size_t j;
    for (j = 0; j < count; ++j) {
      struct OpenAPI_LinkParam *lp = &(*out)[j];
      if (lp->name)
        free(lp->name);
      if (lp->value.type == OA_ANY_STRING && lp->value.string)
        free(lp->value.string);
      if (lp->value.type == OA_ANY_JSON && lp->value.json)
        free(lp->value.json);
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
static enum cdd_c_error copy_any_value_local(struct OpenAPI_Any *dst,
                                             const struct OpenAPI_Any *src) {
  char *_ast_strdup_9 = NULL;
  char *_ast_strdup_10 = NULL;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  memset(dst, 0, sizeof(*dst));
  dst->type = src->type;
  switch (src->type) {
  case OA_ANY_STRING:
    dst->string = (c_cdd_strdup(src->string ? src->string : "", &_ast_strdup_9),
                   _ast_strdup_9);
    return dst->string ? 0 : ENOMEM;
  case OA_ANY_JSON:
    dst->json = (c_cdd_strdup(src->json ? src->json : "", &_ast_strdup_10),
                 _ast_strdup_10);
    return dst->json ? 0 : ENOMEM;
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
 * @brief Frees the memory associated with any value local.
 */
static void free_any_value_local(struct OpenAPI_Any *val) {
  if (!val)
    return;
  if (val->type == OA_ANY_STRING && val->string)
    free(val->string);
  if (val->type == OA_ANY_JSON && val->json)
    free(val->json);
  memset(val, 0, sizeof(*val));
}

/**
 * @brief Retrieves the doc param.
 */
static enum cdd_c_error find_doc_param(const struct DocMetadata *doc,
                                       const char *name,
                                       struct DocParam **_out_val) {
  size_t i;
  /* LCOV_EXCL_START */
  if (!doc || !name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
  for (i = 0; i < doc->n_params; ++i) {
    if (doc->params[i].name && strcmp(doc->params[i].name, name) == 0) {
      {
        *_out_val = &doc->params[i];
        return CDD_C_SUCCESS;
      }
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if path param.
 */
static enum cdd_c_error is_path_param(const char *route, const char *name) {
  char tmpl[128];
  if (!route || !name)
    return CDD_C_SUCCESS;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(tmpl, sizeof(tmpl), "{%s}", name);
#else
  sprintf(tmpl, "{%s}", name);
#endif
  return strstr(route, tmpl) != NULL;
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
enum cdd_c_error copy_doc_server_variables_op(struct OpenAPI_Server *dst,
                                              const struct DocServer *src) {
  char *_ast_strdup_11 = NULL;
  char *_ast_strdup_12 = NULL;
  char *_ast_strdup_13 = NULL;
  char *_ast_strdup_14 = NULL;
  size_t i;
  if (!dst || !src || src->n_variables == 0)
    return CDD_C_SUCCESS;

  dst->variables = (struct OpenAPI_ServerVariable *)calloc(
      src->n_variables, sizeof(struct OpenAPI_ServerVariable));
  /* LCOV_EXCL_START */
  if (!dst->variables) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  dst->n_variables = src->n_variables;

  for (i = 0; i < src->n_variables; ++i) {
    size_t e;
    int found_default = 0;
    const struct DocServerVar *sv = &src->variables[i];
    struct OpenAPI_ServerVariable *dv = &dst->variables[i];

    /* LCOV_EXCL_START */

    if (!sv->name || !sv->default_value) {
      free_openapi_server_variables_op(dst);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    /* LCOV_EXCL_STOP */
    dv->name = (c_cdd_strdup(sv->name, &_ast_strdup_11), _ast_strdup_11);
    /* LCOV_EXCL_START */
    if (!dv->name) {
      free_openapi_server_variables_op(dst);
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    dv->default_value =
        (c_cdd_strdup(sv->default_value, &_ast_strdup_12), _ast_strdup_12);
    /* LCOV_EXCL_START */
    if (!dv->default_value) {
      free_openapi_server_variables_op(dst);
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    if (sv->description) {
      dv->description =
          (c_cdd_strdup(sv->description, &_ast_strdup_13), _ast_strdup_13);
      /* LCOV_EXCL_START */
      if (!dv->description) {
        free_openapi_server_variables_op(dst);
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
    }
    if (sv->enum_values && sv->n_enum_values > 0) {
      dv->enum_values = (char **)calloc(sv->n_enum_values, sizeof(char *));
      /* LCOV_EXCL_START */
      if (!dv->enum_values) {
        free_openapi_server_variables_op(dst);
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
      dv->n_enum_values = sv->n_enum_values;
      for (e = 0; e < sv->n_enum_values; ++e) {
        dv->enum_values[e] =
            (c_cdd_strdup(sv->enum_values[e], &_ast_strdup_14), _ast_strdup_14);
        /* LCOV_EXCL_START */
        if (!dv->enum_values[e]) {
          free_openapi_server_variables_op(dst);
          return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */
        if (strcmp(sv->enum_values[e], sv->default_value) == 0)
          found_default = 1;
      }
      /* LCOV_EXCL_START */
      if (!found_default) {
        free_openapi_server_variables_op(dst);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      /* LCOV_EXCL_STOP */
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the response by code.
 */
enum cdd_c_error find_response_by_code(struct OpenAPI_Operation *op,
                                       const char *code,
                                       struct OpenAPI_Response **_out_val) {
  int _ast_iequal_15 = false;
  size_t i;
  /* LCOV_EXCL_START */
  if (!op || !code) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
  for (i = 0; i < op->n_responses; ++i) {
    if (op->responses[i].code &&
        (c_cdd_str_iequal(op->responses[i].code, code, &_ast_iequal_15),
         _ast_iequal_15)) {
      {
        *_out_val = &op->responses[i];
        return CDD_C_SUCCESS;
      }
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Retrieves the media type.
 */
enum cdd_c_error find_media_type_op(struct OpenAPI_MediaType *mts, size_t n,
                                    const char *name,
                                    struct OpenAPI_MediaType **_out_val) {
  size_t i;
  /* LCOV_EXCL_START */
  if (!mts || !name) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
  for (i = 0; i < n; ++i) {
    if (mts[i].name && strcmp(mts[i].name, name) == 0) {
      *_out_val = &mts[i];
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Applies example to media type.
 */
enum cdd_c_error apply_example_to_media_type(struct OpenAPI_MediaType *mt,
                                             const char *example) {
  if (!mt || !example || mt->example_set)
    return CDD_C_SUCCESS;
  if (parse_example_any(example, &mt->example) != 0)
    return CDD_C_ERROR_MEMORY;
  mt->example_set = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Applies example to response.
 */
enum cdd_c_error apply_example_to_response(struct OpenAPI_Response *resp,
                                           const char *example,
                                           const char *content_type) {
  struct OpenAPI_MediaType *_ast_find_media_type_0;
  size_t i;
  struct OpenAPI_Any parsed = {0};

  if (!resp || !example)
    return CDD_C_SUCCESS;

  if (resp->content_media_types && resp->n_content_media_types > 0) {
    if (parse_example_any(example, &parsed) != 0)
      return CDD_C_ERROR_MEMORY;
    if (content_type) {
      struct OpenAPI_MediaType *mt =
          (find_media_type_op(resp->content_media_types,
                              resp->n_content_media_types, content_type,
                              &_ast_find_media_type_0),
           _ast_find_media_type_0);
      if (mt && !mt->example_set) {
        if (copy_any_value_local(&mt->example, &parsed) != 0) {
          free_any_value_local(&parsed);
          return CDD_C_ERROR_MEMORY;
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
      if (copy_any_value_local(&mt->example, &parsed) != 0) {
        free_any_value_local(&parsed);
        return CDD_C_ERROR_MEMORY;
      }
      mt->example_set = 1;
    }
    free_any_value_local(&parsed);
    return CDD_C_SUCCESS;
  }

  if (resp->example_set)
    return CDD_C_SUCCESS;
  if (parse_example_any(example, &resp->example) != 0)
    return CDD_C_ERROR_MEMORY;
  resp->example_set = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the ensure response for code operation.
 */
enum cdd_c_error ensure_response_for_code(struct OpenAPI_Operation *op,
                                          const char *code,
                                          struct OpenAPI_Response **_out_val) {
  struct OpenAPI_Response *_ast_find_response_by_code_1;
  char *_ast_strdup_16 = NULL;
  char *_ast_strdup_17 = NULL;
  int _ast_iequal_18 = false;
  struct OpenAPI_Response *resp;
  struct OpenAPI_Response *new_resps;
  /* LCOV_EXCL_START */
  if (!op || !code) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */

  resp = (find_response_by_code(op, code, &_ast_find_response_by_code_1),
          _ast_find_response_by_code_1);
  if (resp) {
    *_out_val = resp;
    return CDD_C_SUCCESS;
  }

  new_resps = (struct OpenAPI_Response *)realloc(
      op->responses, (op->n_responses + 1) * sizeof(struct OpenAPI_Response));
  /* LCOV_EXCL_START */
  if (!new_resps) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
  op->responses = new_resps;
  resp = &op->responses[op->n_responses++];
  memset(resp, 0, sizeof(*resp));
  resp->code = (c_cdd_strdup(code, &_ast_strdup_16), _ast_strdup_16);
  /* LCOV_EXCL_START */
  if (!resp->code) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
  resp->description =
      (c_cdd_strdup(
           (c_cdd_str_iequal(code, "200", &_ast_iequal_18), _ast_iequal_18)
               ? "Success"
               : "Response",
           &_ast_strdup_17),
       _ast_strdup_17);
  /* LCOV_EXCL_START */
  if (!resp->description) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  /* LCOV_EXCL_STOP */
  {
    *_out_val = resp;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Adds or sets header to response.
 */
enum cdd_c_error add_header_to_response(struct OpenAPI_Response *resp,
                                        const struct DocResponseHeader *dh) {
  int _ast_iequal_19 = false;
  char *_ast_strdup_20 = NULL;
  char *_ast_strdup_21 = NULL;
  char *_ast_strdup_22 = NULL;
  char *_ast_strdup_23 = NULL;
  char *_ast_strdup_24 = NULL;
  char *_ast_strdup_25 = NULL;
  char *_ast_strdup_26 = NULL;
  char *_ast_strdup_27 = NULL;
  char *_ast_strdup_28 = NULL;
  char *_ast_strdup_29 = NULL;
  char *_ast_strdup_30 = NULL;
  struct OpenAPI_Header *new_headers;
  struct OpenAPI_Header *hdr;
  size_t i;

  /* LCOV_EXCL_START */

  if (!resp || !dh || !dh->name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* LCOV_EXCL_STOP */

  for (i = 0; i < resp->n_headers; ++i) {
    if (resp->headers[i].name &&
        (c_cdd_str_iequal(resp->headers[i].name, dh->name, &_ast_iequal_19),
         _ast_iequal_19)) {
      hdr = &resp->headers[i];
      if (dh->description && !hdr->description) {
        hdr->description =
            (c_cdd_strdup(dh->description, &_ast_strdup_20), _ast_strdup_20);
        /* LCOV_EXCL_START */
        if (!hdr->description) {
          C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
          return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */
      }
      if (dh->type && !hdr->type) {
        hdr->type = (c_cdd_strdup(dh->type, &_ast_strdup_21), _ast_strdup_21);
        /* LCOV_EXCL_START */
        if (!hdr->type) {
          C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
          return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */
      }
      if (dh->content_type && !hdr->content_type) {
        hdr->content_type =
            (c_cdd_strdup(dh->content_type, &_ast_strdup_22), _ast_strdup_22);
        /* LCOV_EXCL_START */
        if (!hdr->content_type) {
          C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
          return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */
      }
      if (dh->format) {
        hdr->schema_set = 1;
        /* LCOV_EXCL_START */
        if (!hdr->schema.inline_type) {
          hdr->schema.inline_type =
              (c_cdd_strdup(hdr->type ? hdr->type : "string", &_ast_strdup_23),
               _ast_strdup_23);
          /* LCOV_EXCL_START */
          if (!hdr->schema.inline_type)
            return CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
        /* LCOV_EXCL_STOP */
        if (hdr->schema.format)
          free(hdr->schema.format);
        hdr->schema.format =
            (c_cdd_strdup(dh->format, &_ast_strdup_24), _ast_strdup_24);
        /* LCOV_EXCL_START */
        if (!hdr->schema.format)
          return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
      if (dh->required_set)
        hdr->required = dh->required ? 1 : 0;
      if (dh->example && !hdr->example_set) {
        if (parse_example_any(dh->example, &hdr->example) != 0)
          return CDD_C_ERROR_MEMORY;
        hdr->example_set = 1;
        hdr->example_location =
            hdr->content_type ? OA_EXAMPLE_LOC_MEDIA : OA_EXAMPLE_LOC_OBJECT;
      }
      return CDD_C_SUCCESS;
    }
  }

  new_headers = (struct OpenAPI_Header *)realloc(
      resp->headers, (resp->n_headers + 1) * sizeof(struct OpenAPI_Header));
  /* LCOV_EXCL_START */
  if (!new_headers) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  resp->headers = new_headers;
  hdr = &resp->headers[resp->n_headers++];
  memset(hdr, 0, sizeof(*hdr));
  hdr->name = (c_cdd_strdup(dh->name, &_ast_strdup_25), _ast_strdup_25);
  /* LCOV_EXCL_START */
  if (!hdr->name) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  if (dh->description) {
    hdr->description =
        (c_cdd_strdup(dh->description, &_ast_strdup_26), _ast_strdup_26);
    /* LCOV_EXCL_START */
    if (!hdr->description) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  hdr->type = (c_cdd_strdup(dh->type ? dh->type : "string", &_ast_strdup_27),
               _ast_strdup_27);
  /* LCOV_EXCL_START */
  if (!hdr->type) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  if (dh->content_type) {
    hdr->content_type =
        (c_cdd_strdup(dh->content_type, &_ast_strdup_28), _ast_strdup_28);
    /* LCOV_EXCL_START */
    if (!hdr->content_type) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  if (dh->format) {
    hdr->schema_set = 1;
    hdr->schema.inline_type =
        (c_cdd_strdup(hdr->type ? hdr->type : "string", &_ast_strdup_29),
         _ast_strdup_29);
    /* LCOV_EXCL_START */
    if (!hdr->schema.inline_type)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    hdr->schema.format =
        (c_cdd_strdup(dh->format, &_ast_strdup_30), _ast_strdup_30);
    /* LCOV_EXCL_START */
    if (!hdr->schema.format)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_STOP */
  if (dh->required_set)
    hdr->required = dh->required ? 1 : 0;
  if (dh->example && !hdr->example_set) {
    if (parse_example_any(dh->example, &hdr->example) != 0)
      return CDD_C_ERROR_MEMORY;
    hdr->example_set = 1;
    hdr->example_location =
        hdr->content_type ? OA_EXAMPLE_LOC_MEDIA : OA_EXAMPLE_LOC_OBJECT;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets link to response.
 */
enum cdd_c_error add_link_to_response(struct OpenAPI_Response *resp,
                                      const struct DocLink *dl) {
  char *_ast_strdup_31 = NULL;
  char *_ast_strdup_32 = NULL;
  char *_ast_strdup_33 = NULL;
  char *_ast_strdup_34 = NULL;
  char *_ast_strdup_35 = NULL;
  char *_ast_strdup_36 = NULL;
  char *_ast_strdup_37 = NULL;
  char *_ast_strdup_38 = NULL;
  struct OpenAPI_Link *new_links;
  struct OpenAPI_Link *link;
  size_t i;

  /* LCOV_EXCL_START */

  if (!resp || !dl || !dl->name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* LCOV_EXCL_STOP */

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
  /* LCOV_EXCL_START */
  if (!new_links) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  resp->links = new_links;
  link = &resp->links[resp->n_links++];
  memset(link, 0, sizeof(*link));

  link->name = (c_cdd_strdup(dl->name, &_ast_strdup_31), _ast_strdup_31);
  /* LCOV_EXCL_START */
  if (!link->name) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  if (dl->summary) {
    link->summary =
        (c_cdd_strdup(dl->summary, &_ast_strdup_32), _ast_strdup_32);
    /* LCOV_EXCL_START */
    if (!link->summary) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  if (dl->description) {
    link->description =
        (c_cdd_strdup(dl->description, &_ast_strdup_33), _ast_strdup_33);
    /* LCOV_EXCL_START */
    if (!link->description) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  if (dl->operation_id) {
    link->operation_id =
        (c_cdd_strdup(dl->operation_id, &_ast_strdup_34), _ast_strdup_34);
    /* LCOV_EXCL_START */
    if (!link->operation_id) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  if (dl->operation_ref) {
    link->operation_ref =
        (c_cdd_strdup(dl->operation_ref, &_ast_strdup_35), _ast_strdup_35);
    /* LCOV_EXCL_START */
    if (!link->operation_ref) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  if (dl->parameters_json) {
    int rc = parse_link_params_json(dl->parameters_json, &link->parameters,
                                    &link->n_parameters);
    if (rc != 0)
      return rc;
  }
  if (dl->request_body_json) {
    if (parse_example_any(dl->request_body_json, &link->request_body) != 0)
      return CDD_C_ERROR_MEMORY;
    link->request_body_set = 1;
  }
  if (dl->server_url) {
    link->server = (struct OpenAPI_Server *)calloc(1, sizeof(*link->server));
    /* LCOV_EXCL_START */
    if (!link->server) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    link->server_set = 1;
    link->server->url =
        (c_cdd_strdup(dl->server_url, &_ast_strdup_36), _ast_strdup_36);
    /* LCOV_EXCL_START */
    if (!link->server->url) {
      free(link->server);
      link->server = NULL;
      link->server_set = 0;
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    if (dl->server_name) {
      link->server->name =
          (c_cdd_strdup(dl->server_name, &_ast_strdup_37), _ast_strdup_37);
      /* LCOV_EXCL_START */
      if (!link->server->name) {
        free(link->server->url);
        free(link->server);
        link->server = NULL;
        link->server_set = 0;
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
    }
    if (dl->server_description) {
      link->server->description =
          (c_cdd_strdup(dl->server_description, &_ast_strdup_38),
           _ast_strdup_38);
      /* LCOV_EXCL_START */
      if (!link->server->description) {
        if (link->server->name)
          free(link->server->name);
        free(link->server->url);
        free(link->server);
        link->server = NULL;
        link->server_set = 0;
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets param to op.
 */
enum cdd_c_error add_param_to_op(struct OpenAPI_Operation *op,
                                 struct OpenAPI_Parameter *p) {
  struct OpenAPI_Parameter *new_arr;
  size_t new_count = op->n_parameters + 1;

  new_arr = (struct OpenAPI_Parameter *)realloc(
      op->parameters, new_count * sizeof(struct OpenAPI_Parameter));
  /* LCOV_EXCL_START */
  if (!new_arr) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */

  op->parameters = new_arr;
  op->parameters[op->n_parameters] = *p; /* Copy struct */
  op->n_parameters = new_count;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the schema ref has data basic operation.
 */
enum cdd_c_error
schema_ref_has_data_basic(const struct OpenAPI_SchemaRef *ref) {
  if (!ref)
    return CDD_C_SUCCESS;
  return (ref->ref_name && *ref->ref_name) || (ref->ref && *ref->ref) ||
         (ref->inline_type && *ref->inline_type) || ref->is_array;
}

/**
 * @brief Creates a deep copy of schema ref basic.
 */
enum cdd_c_error copy_schema_ref_basic(struct OpenAPI_SchemaRef *dst,
                                       const struct OpenAPI_SchemaRef *src) {
  char *_ast_strdup_39 = NULL;
  char *_ast_strdup_40 = NULL;
  char *_ast_strdup_41 = NULL;
  char *_ast_strdup_42 = NULL;
  char *_ast_strdup_43 = NULL;
  char *_ast_strdup_44 = NULL;
  if (!dst || !src)
    return CDD_C_SUCCESS;
  memset(dst, 0, sizeof(*dst));
  dst->is_array = src->is_array;
  if (src->ref_name) {
    dst->ref_name =
        (c_cdd_strdup(src->ref_name, &_ast_strdup_39), _ast_strdup_39);
    /* LCOV_EXCL_START */
    if (!dst->ref_name) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  if (src->ref) {
    dst->ref = (c_cdd_strdup(src->ref, &_ast_strdup_40), _ast_strdup_40);
    /* LCOV_EXCL_START */
    if (!dst->ref) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  dst->ref_is_dynamic = src->ref_is_dynamic;
  if (src->inline_type) {
    dst->inline_type =
        (c_cdd_strdup(src->inline_type, &_ast_strdup_41), _ast_strdup_41);
    /* LCOV_EXCL_START */
    if (!dst->inline_type) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  if (src->items_ref) {
    dst->items_ref =
        (c_cdd_strdup(src->items_ref, &_ast_strdup_42), _ast_strdup_42);
    /* LCOV_EXCL_START */
    if (!dst->items_ref) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  dst->items_ref_is_dynamic = src->items_ref_is_dynamic;
  if (src->format) {
    dst->format = (c_cdd_strdup(src->format, &_ast_strdup_43), _ast_strdup_43);
    /* LCOV_EXCL_START */
    if (!dst->format) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  if (src->items_format) {
    dst->items_format =
        (c_cdd_strdup(src->items_format, &_ast_strdup_44), _ast_strdup_44);
    /* LCOV_EXCL_START */
    if (!dst->items_format) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the response has media type operation.
 */
enum cdd_c_error response_has_media_type(const struct OpenAPI_Response *resp,
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
static enum cdd_c_error
init_media_type_from_response(struct OpenAPI_MediaType *mt, const char *name,
                              const struct OpenAPI_Response *resp,
                              int is_item_schema) {
  char *_ast_strdup_45 = NULL;
  /* LCOV_EXCL_START */
  if (!mt || !name || !resp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  memset(mt, 0, sizeof(*mt));
  mt->name = (c_cdd_strdup(name, &_ast_strdup_45), _ast_strdup_45);
  /* LCOV_EXCL_START */
  if (!mt->name) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  if (schema_ref_has_data_basic(&resp->schema)) {
    if (is_item_schema) {
      if (copy_schema_ref_basic(&mt->item_schema, &resp->schema) != 0)
        return CDD_C_ERROR_MEMORY;
      mt->item_schema_set = 1;
    } else {
      if (copy_schema_ref_basic(&mt->schema, &resp->schema) != 0)
        return CDD_C_ERROR_MEMORY;
      mt->schema_set = 1;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets response media type.
 */
static enum cdd_c_error add_response_media_type(struct OpenAPI_Response *resp,
                                                const char *name,
                                                int is_item_schema) {
  struct OpenAPI_MediaType *new_mts;
  size_t new_count;
  int _has_mt = 0;

  if (!resp || !name || !*name)
    return CDD_C_SUCCESS;

  if (!resp->content_media_types) {
    size_t base = resp->content_type ? 1 : 0;
    resp->content_media_types = (struct OpenAPI_MediaType *)calloc(
        base + 1, sizeof(struct OpenAPI_MediaType));
    /* LCOV_EXCL_START */
    if (!resp->content_media_types) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
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
  /* LCOV_EXCL_START */
  if (!new_mts) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  resp->content_media_types = new_mts;
  if (init_media_type_from_response(
          &resp->content_media_types[resp->n_content_media_types], name, resp,
          is_item_schema) != 0)
    return CDD_C_ERROR_MEMORY;
  resp->n_content_media_types = new_count;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the request body has media type operation.
 */
static enum cdd_c_error
request_body_has_media_type(const struct OpenAPI_Operation *op,
                            const char *name) {
  size_t i;
  if (!op || !name)
    return CDD_C_SUCCESS;
  if (op->req_body.content_type && strcmp(op->req_body.content_type, name) == 0)
    return CDD_C_ERROR_UNKNOWN;
  if (!op->req_body_media_types || op->n_req_body_media_types == 0)
    return CDD_C_SUCCESS;
  for (i = 0; i < op->n_req_body_media_types; ++i) {
    const struct OpenAPI_MediaType *mt = &op->req_body_media_types[i];
    if (mt->name && strcmp(mt->name, name) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the init media type from request body operation.
 */
static enum cdd_c_error init_media_type_from_request_body(
    struct OpenAPI_MediaType *mt, const char *name,
    const struct OpenAPI_Operation *op, int is_item_schema) {
  char *_ast_strdup_46 = NULL;
  /* LCOV_EXCL_START */
  if (!mt || !name || !op)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  memset(mt, 0, sizeof(*mt));
  mt->name = (c_cdd_strdup(name, &_ast_strdup_46), _ast_strdup_46);
  /* LCOV_EXCL_START */
  if (!mt->name) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  if (schema_ref_has_data_basic(&op->req_body)) {
    if (is_item_schema) {
      if (copy_schema_ref_basic(&mt->item_schema, &op->req_body) != 0)
        return CDD_C_ERROR_MEMORY;
      mt->item_schema_set = 1;
    } else {
      if (copy_schema_ref_basic(&mt->schema, &op->req_body) != 0)
        return CDD_C_ERROR_MEMORY;
      mt->schema_set = 1;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets request body media type.
 */
static enum cdd_c_error
add_request_body_media_type(struct OpenAPI_Operation *op, const char *name,
                            int is_item_schema) {
  struct OpenAPI_MediaType *new_mts;
  size_t new_count;

  if (!op || !name || !*name)
    return CDD_C_SUCCESS;

  if (!op->req_body_media_types) {
    size_t base = op->req_body.content_type ? 1 : 0;
    op->req_body_media_types = (struct OpenAPI_MediaType *)calloc(
        base + 1, sizeof(struct OpenAPI_MediaType));
    /* LCOV_EXCL_START */
    if (!op->req_body_media_types) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    op->n_req_body_media_types = 0;
    if (op->req_body.content_type) {
      if (init_media_type_from_request_body(&op->req_body_media_types[0],
                                            op->req_body.content_type, op,
                                            is_item_schema) != 0)
        return CDD_C_ERROR_MEMORY;
      op->n_req_body_media_types = 1;
    }
  }

  if (request_body_has_media_type(op, name))
    return CDD_C_SUCCESS;

  new_count = op->n_req_body_media_types + 1;
  new_mts = (struct OpenAPI_MediaType *)realloc(
      op->req_body_media_types, new_count * sizeof(struct OpenAPI_MediaType));
  /* LCOV_EXCL_START */
  if (!new_mts) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
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
static enum cdd_c_error set_querystring_schema_from_type_map(
    struct OpenAPI_Parameter *param,
    const struct OpenApiTypeMapping *type_map) {
  char *_ast_strdup_47 = NULL;
  char *_ast_strdup_48 = NULL;
  char *_ast_strdup_49 = NULL;
  char *_ast_strdup_50 = NULL;
  char *_ast_strdup_51 = NULL;
  if (!param || !type_map)
    return CDD_C_SUCCESS;
  if (type_map->ref_name) {
    param->schema_set = 1;
    param->schema.is_array = (type_map->kind == OA_TYPE_ARRAY);
    param->schema.ref_name =
        (c_cdd_strdup(type_map->ref_name, &_ast_strdup_47), _ast_strdup_47);
    /* LCOV_EXCL_START */
    if (!param->schema.ref_name)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    return CDD_C_SUCCESS;
  }
  if (type_map->kind == OA_TYPE_ARRAY) {
    param->is_array = 1;
    param->type = (c_cdd_strdup("array", &_ast_strdup_48), _ast_strdup_48);
    /* LCOV_EXCL_START */
    if (!param->type) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    if (type_map->oa_type) {
      param->items_type =
          (c_cdd_strdup(type_map->oa_type, &_ast_strdup_49), _ast_strdup_49);
      /* LCOV_EXCL_START */
      if (!param->items_type) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
    }
    return CDD_C_SUCCESS;
  }
  if (type_map->oa_type) {
    param->type =
        (c_cdd_strdup(type_map->oa_type, &_ast_strdup_50), _ast_strdup_50);
  } else {
    param->type = (c_cdd_strdup("string", &_ast_strdup_51), _ast_strdup_51);
  }
  return param->type ? 0 : ENOMEM;
}

/**
 * @brief Executes the oa type is primitive operation.
 */
static enum cdd_c_error oa_type_is_primitive(const char *type) {
  if (!type)
    return CDD_C_SUCCESS;
  return strcmp(type, "integer") == 0 || strcmp(type, "number") == 0 ||
         strcmp(type, "string") == 0 || strcmp(type, "boolean") == 0;
}

/* Apply format from type mapping (or override) to a SchemaRef.
 * Returns: 1 if applied, 0 if not applicable, or ENOMEM on allocation failure.
 */
/**
 * @brief Applies format to schema ref.
 */
static enum cdd_c_error
apply_format_to_schema_ref(struct OpenAPI_SchemaRef *schema,
                           const struct OpenApiTypeMapping *map,
                           const char *override_format) {
  char *_ast_strdup_52 = NULL;
  char *_ast_strdup_53 = NULL;
  char *_ast_strdup_54 = NULL;
  char *_ast_strdup_55 = NULL;
  const char *fmt;
  if (!schema || !map)
    return CDD_C_SUCCESS;
  fmt =
      (override_format && *override_format) ? override_format : map->oa_format;
  if (!fmt || !*fmt)
    return CDD_C_SUCCESS;
  if (!map->oa_type || !oa_type_is_primitive(map->oa_type))
    return CDD_C_SUCCESS;

  if (map->kind == OA_TYPE_ARRAY) {
    schema->is_array = 1;
    if (!schema->inline_type) {
      schema->inline_type =
          (c_cdd_strdup(map->oa_type, &_ast_strdup_52), _ast_strdup_52);
      /* LCOV_EXCL_START */
      if (!schema->inline_type) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
    }
    if (schema->items_format)
      free(schema->items_format);
    schema->items_format = (c_cdd_strdup(fmt, &_ast_strdup_53), _ast_strdup_53);
    /* LCOV_EXCL_START */
    if (!schema->items_format) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  } else {
    if (!schema->inline_type) {
      schema->inline_type =
          (c_cdd_strdup(map->oa_type, &_ast_strdup_54), _ast_strdup_54);
      /* LCOV_EXCL_START */
      if (!schema->inline_type) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
    }
    if (schema->format)
      free(schema->format);
    schema->format = (c_cdd_strdup(fmt, &_ast_strdup_55), _ast_strdup_55);
    /* LCOV_EXCL_START */
    if (!schema->format) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  return CDD_C_ERROR_UNKNOWN;
}

/* --- Type Analysis --- */

/**
 * @brief Determine if a type is a struct pointer eligible for Body.
 * Heuristic: Contains "struct", ends with "*" or "**".
 */
/**
 * @brief Checks if struct pointer.
 */
enum cdd_c_error is_struct_pointer(const char *type, int *is_double_ptr,
                                   int *out_is_struct_ptr) {
  const char *p;
  if (out_is_struct_ptr)
    *out_is_struct_ptr = 0;
  if (!type)
    return CDD_C_SUCCESS;
  if (!strstr(type, "struct "))
    return CDD_C_SUCCESS;

  p = strrchr(type, '*');
  if (!p)
    return CDD_C_SUCCESS;

  if (p > type && *(p - 1) == '*') {
    if (is_double_ptr)
      *is_double_ptr = 1;
  } else {
    if (is_double_ptr)
      *is_double_ptr = 0;
  }

  if (out_is_struct_ptr)
    *out_is_struct_ptr = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the doc style to openapi operation.
 */
enum cdd_c_error doc_style_to_openapi(enum DocParamStyle style,
                                      enum OpenAPI_Style *_out_val) {
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
enum cdd_c_error c2openapi_build_operation(const struct OpBuilderContext *ctx,
                                           struct OpenAPI_Operation *out_op) {
  struct DocParam *_ast_find_doc_param_2;
  enum OpenAPI_Style _ast_doc_style_to_openapi_3;
  struct OpenAPI_MediaType *_ast_find_media_type_4;
  struct OpenAPI_MediaType *_ast_find_media_type_5;
  enum OpenAPI_Style _ast_doc_style_to_openapi_6;
  struct OpenAPI_Response *_ast_ensure_response_for_code_7;
  struct OpenAPI_Response *_ast_ensure_response_for_code_8;
  char *_ast_strdup_56 = NULL;
  int _ast_starts_with_57 = false;
  int _ast_starts_with_58 = false;
  int _ast_starts_with_59 = false;
  char *_ast_strdup_60 = NULL;
  char *_ast_strdup_61 = NULL;
  char *_ast_strdup_62 = NULL;
  char *_ast_strdup_63 = NULL;
  char *_ast_strdup_64 = NULL;
  char *_ast_strdup_65 = NULL;
  char *_ast_strdup_66 = NULL;
  char *_ast_strdup_67 = NULL;
  char *_ast_strdup_68 = NULL;
  char *_ast_strdup_69 = NULL;
  char *_ast_strdup_70 = NULL;
  char *_ast_strdup_71 = NULL;
  char *_ast_strdup_72 = NULL;
  char *_ast_strdup_73 = NULL;
  char *_ast_strdup_74 = NULL;
  char *_ast_strdup_75 = NULL;
  char *_ast_strdup_76 = NULL;
  char *_ast_strdup_77 = NULL;
  char *_ast_strdup_78 = NULL;
  char *_ast_strdup_79 = NULL;
  char *_ast_strdup_80 = NULL;
  char *_ast_strdup_81 = NULL;
  char *_ast_strdup_82 = NULL;
  char *_ast_strdup_83 = NULL;
  char *_ast_strdup_84 = NULL;
  char *_ast_strdup_85 = NULL;
  char *_ast_strdup_86 = NULL;
  char *_ast_strdup_87 = NULL;
  char *_ast_strdup_88 = NULL;
  char *_ast_strdup_89 = NULL;
  char *_ast_strdup_90 = NULL;
  char *_ast_strdup_91 = NULL;
  char *_ast_strdup_92 = NULL;
  char *_ast_strdup_93 = NULL;
  char *_ast_strdup_94 = NULL;
  char *_ast_strdup_95 = NULL;
  char *_ast_strdup_96 = NULL;
  char *_ast_strdup_97 = NULL;
  char *_ast_strdup_98 = NULL;
  char *_ast_strdup_99 = NULL;
  char *_ast_strdup_100 = NULL;
  int _ast_iequal_101 = false;
  char *_ast_strdup_102 = NULL;
  int _ast_iequal_103 = false;
  char *_ast_strdup_104 = NULL;
  char *_ast_strdup_105 = NULL;
  char *_ast_strdup_106 = NULL;
  char *_ast_strdup_107 = NULL;
  const struct C2OpenAPI_ParsedSig *sig = ctx->sig;
  const struct DocMetadata *doc = ctx->doc;
  size_t i;
  int rc = 0;

  /* LCOV_EXCL_START */

  if (!ctx || !out_op || !sig)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* LCOV_EXCL_STOP */

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
                      out_op->method =
                          (c_cdd_strdup(doc->verb, &_ast_strdup_56),
                           _ast_strdup_56);
                      /* LCOV_EXCL_START */
                      if (!out_op->method) {
                        C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
                        return CDD_C_ERROR_MEMORY;
                      }
                      /* LCOV_EXCL_STOP */
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
    /* Heuristics:
       api_post_X
       api_X_create  -> _create
       api_X_update  -> _update
       api_X_delete  -> _delete
    */
    if ((c_cdd_str_starts_with(ctx->func_name, "api_post_",
                               &_ast_starts_with_57),
         _ast_starts_with_57) ||
        strstr(ctx->func_name, "_create"))
      out_op->verb = OA_VERB_POST;
    else if ((c_cdd_str_starts_with(ctx->func_name, "api_put_",
                                    &_ast_starts_with_58),
              _ast_starts_with_58) ||
             strstr(ctx->func_name, "_update"))
      out_op->verb = OA_VERB_PUT;
    else if ((c_cdd_str_starts_with(ctx->func_name, "api_delete_",
                                    &_ast_starts_with_59),
              _ast_starts_with_59) ||
             strstr(ctx->func_name, "_delete"))
      out_op->verb = OA_VERB_DELETE;
    else
      out_op->verb = OA_VERB_GET;
  }

  if (doc && doc->operation_id) {
    out_op->operation_id =
        (c_cdd_strdup(doc->operation_id, &_ast_strdup_60), _ast_strdup_60);
  } else {
    out_op->operation_id =
        (c_cdd_strdup(ctx->func_name, &_ast_strdup_61), _ast_strdup_61);
  }
  /* LCOV_EXCL_START */
  if (!out_op->operation_id) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
    return CDD_C_ERROR_MEMORY;
  }
  /* LCOV_EXCL_STOP */
  if (doc && doc->summary) {
    out_op->summary =
        (c_cdd_strdup(doc->summary, &_ast_strdup_62), _ast_strdup_62);
    /* LCOV_EXCL_START */
    if (!out_op->summary) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  if (doc && doc->description) {
    out_op->description =
        (c_cdd_strdup(doc->description, &_ast_strdup_63), _ast_strdup_63);
    /* LCOV_EXCL_START */
    if (!out_op->description) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }
  if (doc && doc->deprecated_set) {
    out_op->deprecated = doc->deprecated ? 1 : 0;
  }
  if (doc && doc->external_docs_url) {
    out_op->external_docs.url =
        (c_cdd_strdup(doc->external_docs_url, &_ast_strdup_64), _ast_strdup_64);
    /* LCOV_EXCL_START */
    if (!out_op->external_docs.url)
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
    /* LCOV_EXCL_START */
    if (doc->external_docs_description) {
      out_op->external_docs.description =
          (c_cdd_strdup(doc->external_docs_description, &_ast_strdup_65),
           _ast_strdup_65);
      /* LCOV_EXCL_START */
      if (!out_op->external_docs.description)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_STOP */
  }
  if (doc && doc->n_tags > 0) {
    size_t t;
    out_op->tags = (char **)calloc(doc->n_tags, sizeof(char *));
    /* LCOV_EXCL_START */
    if (!out_op->tags) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    out_op->n_tags = doc->n_tags;
    for (t = 0; t < doc->n_tags; ++t) {
      out_op->tags[t] =
          (c_cdd_strdup(doc->tags[t] ? doc->tags[t] : "", &_ast_strdup_66),
           _ast_strdup_66);
      /* LCOV_EXCL_START */
      if (!out_op->tags[t])
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
  }

  if (doc && doc->n_security > 0) {
    size_t s;
    out_op->security = (struct OpenAPI_SecurityRequirementSet *)calloc(
        doc->n_security, sizeof(struct OpenAPI_SecurityRequirementSet));
    /* LCOV_EXCL_START */
    if (!out_op->security) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    out_op->n_security = doc->n_security;
    out_op->security_set = 1;
    for (s = 0; s < doc->n_security; ++s) {
      struct OpenAPI_SecurityRequirementSet *set = &out_op->security[s];
      const struct DocSecurityRequirement *src = &doc->security[s];
      set->requirements = (struct OpenAPI_SecurityRequirement *)calloc(
          1, sizeof(struct OpenAPI_SecurityRequirement));
      /* LCOV_EXCL_START */
      if (!set->requirements) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
      set->n_requirements = 1;
      set->requirements[0].scheme =
          (c_cdd_strdup(src->scheme ? src->scheme : "", &_ast_strdup_67),
           _ast_strdup_67);
      /* LCOV_EXCL_START */
      if (!set->requirements[0].scheme)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
      if (src->n_scopes > 0) {
        size_t k;
        set->requirements[0].scopes =
            (char **)calloc(src->n_scopes, sizeof(char *));
        /* LCOV_EXCL_START */
        if (!set->requirements[0].scopes)
          return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
        set->requirements[0].n_scopes = src->n_scopes;
        for (k = 0; k < src->n_scopes; ++k) {
          set->requirements[0].scopes[k] =
              (c_cdd_strdup(src->scopes[k] ? src->scopes[k] : "",
                            &_ast_strdup_68),
               _ast_strdup_68);
          /* LCOV_EXCL_START */
          if (!set->requirements[0].scopes[k])
            return CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
      }
    }
  }

  if (doc && doc->n_servers > 0) {
    size_t s;
    out_op->servers = (struct OpenAPI_Server *)calloc(
        doc->n_servers, sizeof(struct OpenAPI_Server));
    /* LCOV_EXCL_START */
    if (!out_op->servers) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    out_op->n_servers = doc->n_servers;
    for (s = 0; s < doc->n_servers; ++s) {
      const struct DocServer *src = &doc->servers[s];
      /* LCOV_EXCL_START */
      if (src->url) {
        out_op->servers[s].url =
            (c_cdd_strdup(src->url, &_ast_strdup_69), _ast_strdup_69);
        /* LCOV_EXCL_START */
        if (!out_op->servers[s].url)
          return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      if (src->name) {
        out_op->servers[s].name =
            (c_cdd_strdup(src->name, &_ast_strdup_70), _ast_strdup_70);
        /* LCOV_EXCL_START */
        if (!out_op->servers[s].name)
          return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      if (src->description) {
        out_op->servers[s].description =
            (c_cdd_strdup(src->description, &_ast_strdup_71), _ast_strdup_71);
        /* LCOV_EXCL_START */
        if (!out_op->servers[s].description)
          return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_STOP */
      if (src->n_variables > 0) {
        int vrc = copy_doc_server_variables_op(&out_op->servers[s], src);
        if (vrc != 0)
          return vrc;
      }
    }
  }

  /* 1. Argument Iteration */
  for (i = 0; i < sig->n_args; ++i) {
    const struct C2OpenAPI_ParsedArg *arg = &sig->args[i];
    const struct DocParam *dp =
        (find_doc_param(doc, arg->name, &_ast_find_doc_param_2),
         _ast_find_doc_param_2);
    struct OpenAPI_Parameter curr_param;
    struct OpenApiTypeMapping type_map;
    int is_path = 0;
    int is_body = 0;
    int is_out_ptr = 0;
    int is_querystring = 0;
    int _is_reserved = 0;

    memset(&curr_param, 0, sizeof(curr_param));
    (void)c_mapping_init(&type_map);

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
    }
    /* B. Implicit Path: Matches {name} in route */
    else if (doc && is_path_param(doc->route, arg->name)) {
      is_path = 1;
    }
    /* C. Implicit Body: "struct *" without const in POST/PUT/PATCH? */
    else {
      int is_double = 0;
      int _is_struct_ptr = 0;
      is_struct_pointer(arg->type, &is_double, &_is_struct_ptr);
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
      }
    }

    /* Analyze Type using C Mapper */
    rc = c_mapping_map_type(arg->type, arg->name, &type_map);
    if (rc != 0) {
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
      /* LCOV_EXCL_START */
      if (!new_resps) {
        c_mapping_free(&type_map);
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
      out_op->responses = new_resps;
      out_op->n_responses++;

      r = &out_op->responses[r_idx];
      memset(r, 0, sizeof(*r));
      r->code = (c_cdd_strdup("200", &_ast_strdup_72),
                 _ast_strdup_72); /* Default success */
      r->description =
          (c_cdd_strdup("Success", &_ast_strdup_73), _ast_strdup_73);
      /* LCOV_EXCL_START */
      if (!r->description) {
        c_mapping_free(&type_map);
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */

      /* Map Schema */
      r->schema.is_array = (type_map.kind == OA_TYPE_ARRAY);
      if (type_map.ref_name) {
        r->schema.ref_name =
            (c_cdd_strdup(type_map.ref_name, &_ast_strdup_74), _ast_strdup_74);
      } else if (type_map.oa_type) {
        r->schema.inline_type =
            (c_cdd_strdup(type_map.oa_type, &_ast_strdup_75), _ast_strdup_75);
      }
      {
        int fmt_rc = apply_format_to_schema_ref(&r->schema, &type_map, NULL);
        /* LCOV_EXCL_START */
        if (fmt_rc == ENOMEM) {
          c_mapping_free(&type_map);
          return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */
      }

      c_mapping_free(&type_map);
      continue; /* Done with this arg */
    }

    if (is_body) {
      /* Request Body Population */
      out_op->req_body.content_type =
          (c_cdd_strdup("application/json", &_ast_strdup_76), _ast_strdup_76);
      out_op->req_body.is_array = (type_map.kind == OA_TYPE_ARRAY);
      /* Use ref_name if object, or type if primitive */
      if (type_map.ref_name) {
        out_op->req_body.ref_name =
            (c_cdd_strdup(type_map.ref_name, &_ast_strdup_77), _ast_strdup_77);
      } else if (type_map.oa_type) {
        out_op->req_body.inline_type =
            (c_cdd_strdup(type_map.oa_type, &_ast_strdup_78), _ast_strdup_78);
      }
      out_op->req_body_required = 1;
      out_op->req_body_required_set = 1;
      {
        int fmt_rc =
            apply_format_to_schema_ref(&out_op->req_body, &type_map, NULL);
        /* LCOV_EXCL_START */
        if (fmt_rc == ENOMEM) {
          c_mapping_free(&type_map);
          return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */
      }

      c_mapping_free(&type_map);
      continue;
    }

    /* --- Standard Parameter (Query/Path/Header/Cookie) --- */

    curr_param.name =
        (c_cdd_strdup(arg->name, &_ast_strdup_79), _ast_strdup_79);
    curr_param.required = is_path; /* Path params always required */
    if (dp && dp->required)
      curr_param.required = 1;
    if (dp && dp->description) {
      curr_param.description =
          (c_cdd_strdup(dp->description, &_ast_strdup_80), _ast_strdup_80);
      /* LCOV_EXCL_START */
      if (!curr_param.description) {
        c_mapping_free(&type_map);
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
    }

    curr_param.in = is_path ? OA_PARAM_IN_PATH : OA_PARAM_IN_QUERY;
    if (dp && dp->in_loc && strcmp(dp->in_loc, "header") == 0)
      curr_param.in = OA_PARAM_IN_HEADER;
    else if (dp && dp->in_loc && strcmp(dp->in_loc, "cookie") == 0)
      curr_param.in = OA_PARAM_IN_COOKIE;
    else if (is_querystring)
      curr_param.in = OA_PARAM_IN_QUERYSTRING;

    if (curr_param.in == OA_PARAM_IN_HEADER) {
      is_reserved_header_name(curr_param.name, &_is_reserved);
    }
    if (curr_param.in == OA_PARAM_IN_HEADER && _is_reserved) {
      if (curr_param.name)
        free(curr_param.name);
      if (curr_param.description)
        free(curr_param.description);
      c_mapping_free(&type_map);
      continue;
    }

    /* Map Types */
    if (is_querystring) {
      curr_param.content_type =
          (c_cdd_strdup("application/x-www-form-urlencoded", &_ast_strdup_81),
           _ast_strdup_81);
      /* LCOV_EXCL_START */
      if (!curr_param.content_type) {
        c_mapping_free(&type_map);
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
      rc = set_querystring_schema_from_type_map(&curr_param, &type_map);
      if (rc != 0)
        return rc;
    } else if (type_map.kind == OA_TYPE_ARRAY) {
      curr_param.is_array = 1;
      /* Logic for items_type: c_mapper stores item type in oa_type/ref_name
       * when kind=ARRAY */
      if (type_map.oa_type)
        curr_param.items_type =
            (c_cdd_strdup(type_map.oa_type, &_ast_strdup_82), _ast_strdup_82);
      else if (type_map.ref_name)
        curr_param.items_type =
            (c_cdd_strdup(type_map.ref_name, &_ast_strdup_83), _ast_strdup_83);

      curr_param.type =
          (c_cdd_strdup("array", &_ast_strdup_84), _ast_strdup_84);
    } else {
      /* Primitive / Object (if scalar param is allowed object??) usually string
       */
      /* Spec allows object parameters but they serialize weirdly. Assume string
       * representation unless primitive. */
      if (type_map.oa_type)
        curr_param.type =
            (c_cdd_strdup(type_map.oa_type, &_ast_strdup_85), _ast_strdup_85);
      else
        curr_param.type =
            (c_cdd_strdup("string", &_ast_strdup_86),
             _ast_strdup_86); /* Fallback for complex types in params */
    }

    {
      const char *fmt_override = (dp && dp->format) ? dp->format : NULL;
      int fmt_rc = apply_format_to_schema_ref(&curr_param.schema, &type_map,
                                              fmt_override);
      /* LCOV_EXCL_START */
      if (fmt_rc == ENOMEM) {
        c_mapping_free(&type_map);
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
      if (fmt_rc > 0) {
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
        curr_param.content_type =
            (c_cdd_strdup(dp->content_type, &_ast_strdup_87), _ast_strdup_87);
        /* LCOV_EXCL_START */
        if (!curr_param.content_type) {
          c_mapping_free(&type_map);
          return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */
      }
      if (!curr_param.content_type) {
        if (dp->style_set) {
          enum OpenAPI_Style style =
              (doc_style_to_openapi(dp->style, &_ast_doc_style_to_openapi_3),
               _ast_doc_style_to_openapi_3);
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
      else if (curr_param.in == OA_PARAM_IN_PATH ||
               curr_param.in == OA_PARAM_IN_HEADER)
        curr_param.style = OA_STYLE_SIMPLE;
    }

    if (dp && dp->example) {
      int ex_rc = parse_example_any(dp->example, &curr_param.example);
      /* LCOV_EXCL_START */
      if (ex_rc == ENOMEM) {
        c_mapping_free(&type_map);
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
      curr_param.example_set = 1;
      if (curr_param.content_type || curr_param.in == OA_PARAM_IN_QUERYSTRING) {
        curr_param.example_location = OA_EXAMPLE_LOC_MEDIA;
      } else {
        curr_param.example_location = OA_EXAMPLE_LOC_OBJECT;
      }
    }

    rc = add_param_to_op(out_op, &curr_param);
    c_mapping_free(&type_map);
    if (rc != 0)
      return rc;
  }

  if (doc) {
    if (doc->n_request_bodies > 0) {
      size_t rb_idx;
      for (rb_idx = 0; rb_idx < doc->n_request_bodies; ++rb_idx) {
        const struct DocRequestBody *rb = &doc->request_bodies[rb_idx];
        const char *rb_content_type =
            rb->content_type ? rb->content_type : "application/json";
        /* LCOV_EXCL_START */
        if (rb_idx == 0) {
          if (out_op->req_body.content_type)
            free(out_op->req_body.content_type);
          out_op->req_body.content_type =
              (c_cdd_strdup(rb_content_type, &_ast_strdup_88), _ast_strdup_88);
          /* LCOV_EXCL_START */
          if (!out_op->req_body.content_type)
            return CDD_C_ERROR_MEMORY;
          /* LCOV_EXCL_STOP */
        }
        /* LCOV_EXCL_STOP */
        if (add_request_body_media_type(out_op, rb_content_type,
                                        rb->item_schema) != 0)
          return CDD_C_ERROR_MEMORY;
        /* LCOV_EXCL_START */
        if (rb->example) {
          struct OpenAPI_MediaType *mt =
              (find_media_type_op(out_op->req_body_media_types,
                                  out_op->n_req_body_media_types,
                                  rb_content_type, &_ast_find_media_type_4),
               _ast_find_media_type_4);
          if (mt && apply_example_to_media_type(mt, rb->example) != 0)
            return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */

        {
          struct OpenAPI_MediaType *mt =
              (find_media_type_op(out_op->req_body_media_types,
                                  out_op->n_req_body_media_types,
                                  rb_content_type, &_ast_find_media_type_5),
               _ast_find_media_type_5);
          if (mt && doc->n_encodings > 0) {
            size_t enc_i;
            for (enc_i = 0; enc_i < doc->n_encodings; ++enc_i) {
              const struct DocEncoding *d_enc = &doc->encodings[enc_i];
              struct OpenAPI_Encoding enc;
              memset(&enc, 0, sizeof(enc));

              if (d_enc->name)
                enc.name = (c_cdd_strdup(d_enc->name, &_ast_strdup_89),
                            _ast_strdup_89);
              if (d_enc->content_type)
                enc.content_type =
                    (c_cdd_strdup(d_enc->content_type, &_ast_strdup_90),
                     _ast_strdup_90);
              if (d_enc->style)
                enc.style = (doc_style_to_openapi(d_enc->style,
                                                  &_ast_doc_style_to_openapi_6),
                             _ast_doc_style_to_openapi_6);

              enc.explode = d_enc->explode;
              enc.explode_set = d_enc->explode_set;
              enc.allow_reserved = d_enc->allow_reserved;
              enc.allow_reserved_set = d_enc->allow_reserved_set;

              if (d_enc->kind == 0) {
                struct OpenAPI_Encoding *new_encs =
                    realloc(mt->encoding, (mt->n_encoding + 1) *
                                              sizeof(struct OpenAPI_Encoding));
                if (new_encs) {
                  mt->encoding = new_encs;
                  mt->encoding[mt->n_encoding++] = enc;
                }
              } else if (d_enc->kind == 1) {
                struct OpenAPI_Encoding *new_encs = realloc(
                    mt->prefix_encoding, (mt->n_prefix_encoding + 1) *
                                             sizeof(struct OpenAPI_Encoding));
                if (new_encs) {
                  mt->prefix_encoding = new_encs;
                  mt->prefix_encoding[mt->n_prefix_encoding++] = enc;
                }
              } else if (d_enc->kind == 2) {
                if (!mt->item_encoding) {
                  mt->item_encoding =
                      calloc(1, sizeof(struct OpenAPI_Encoding));
                }
                if (mt->item_encoding) {
                  *mt->item_encoding = enc;
                }
              }
            }
          }
        }
      }
    }
    if (doc->request_body_description) {
      out_op->req_body_description =
          (c_cdd_strdup(doc->request_body_description, &_ast_strdup_91),
           _ast_strdup_91);
      /* LCOV_EXCL_START */
      if (!out_op->req_body_description) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
    }
    if (doc->request_body_required_set) {
      out_op->req_body_required_set = 1;
      out_op->req_body_required = doc->request_body_required ? 1 : 0;
    }
    /* LCOV_EXCL_START */
    if (doc->request_body_content_type && doc->n_request_bodies == 0) {
      if (out_op->req_body.content_type)
        free(out_op->req_body.content_type);
      out_op->req_body.content_type =
          (c_cdd_strdup(doc->request_body_content_type, &_ast_strdup_92),
           _ast_strdup_92);
      /* LCOV_EXCL_START */
      if (!out_op->req_body.content_type)
        return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_STOP */
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
        if (strcmp(out_op->responses[k].code, doc->returns[i].code) == 0) {
          exists = 1;
          /* LCOV_EXCL_START */
          if (!out_op->responses[k].summary && doc->returns[i].summary) {
            out_op->responses[k].summary =
                (c_cdd_strdup(doc->returns[i].summary, &_ast_strdup_93),
                 _ast_strdup_93);
            /* LCOV_EXCL_START */
            if (!out_op->responses[k].summary)
              return CDD_C_ERROR_MEMORY;
            /* LCOV_EXCL_STOP */
          }
          /* LCOV_EXCL_STOP */
          /* LCOV_EXCL_START */
          if (!out_op->responses[k].description &&
              doc->returns[i].description) {
            out_op->responses[k].description =
                (c_cdd_strdup(doc->returns[i].description, &_ast_strdup_94),
                 _ast_strdup_94);
            /* LCOV_EXCL_START */
            if (!out_op->responses[k].description)
              return CDD_C_ERROR_MEMORY;
            /* LCOV_EXCL_STOP */
          }
          /* LCOV_EXCL_STOP */
          if (doc->returns[i].content_type) {
            int add_rc = add_response_media_type(&out_op->responses[k],
                                                 doc->returns[i].content_type,
                                                 doc->returns[i].item_schema);
            if (add_rc != 0)
              return add_rc;
            /* LCOV_EXCL_START */
            if (!out_op->responses[k].content_type) {
              out_op->responses[k].content_type =
                  (c_cdd_strdup(doc->returns[i].content_type, &_ast_strdup_95),
                   _ast_strdup_95);
              /* LCOV_EXCL_START */
              if (!out_op->responses[k].content_type)
                return CDD_C_ERROR_MEMORY;
              /* LCOV_EXCL_STOP */
            }
            /* LCOV_EXCL_STOP */
          }
          if (doc->returns[i].example) {
            int ex_rc = apply_example_to_response(&out_op->responses[k],
                                                  doc->returns[i].example,
                                                  doc->returns[i].content_type);
            if (ex_rc != 0)
              return ex_rc;
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
        /* LCOV_EXCL_START */
        if (!new_resps) {
          C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
          return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */
        out_op->responses = new_resps;
        r = &out_op->responses[out_op->n_responses++];
        memset(r, 0, sizeof(*r));
        r->code = (c_cdd_strdup(doc->returns[i].code, &_ast_strdup_96),
                   _ast_strdup_96);
        if (doc->returns[i].summary) {
          r->summary = (c_cdd_strdup(doc->returns[i].summary, &_ast_strdup_97),
                        _ast_strdup_97);
          /* LCOV_EXCL_START */
          if (!r->summary) {
            C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
            return CDD_C_ERROR_MEMORY;
          }
          /* LCOV_EXCL_STOP */
        }
        if (doc->returns[i].description) {
          r->description =
              (c_cdd_strdup(doc->returns[i].description, &_ast_strdup_98),
               _ast_strdup_98);
          /* LCOV_EXCL_START */
          if (!r->description) {
            C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
            return CDD_C_ERROR_MEMORY;
          }
          /* LCOV_EXCL_STOP */
        }
        if (doc->returns[i].content_type) {
          int add_rc = add_response_media_type(r, doc->returns[i].content_type,
                                               doc->returns[i].item_schema);
          if (add_rc != 0)
            return add_rc;
          r->content_type =
              (c_cdd_strdup(doc->returns[i].content_type, &_ast_strdup_99),
               _ast_strdup_99);
          /* LCOV_EXCL_START */
          if (!r->content_type) {
            C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
            return CDD_C_ERROR_MEMORY;
          }
          /* LCOV_EXCL_STOP */
        }
        if (doc->returns[i].example) {
          int ex_rc = apply_example_to_response(r, doc->returns[i].example,
                                                doc->returns[i].content_type);
          if (ex_rc != 0)
            return ex_rc;
        }
        /* Schema for error is usually generic Error struct, logic outside scope
         * here, leaves NULL */
      }
    }
  }

  if (doc && doc->n_response_headers > 0) {
    for (i = 0; i < doc->n_response_headers; ++i) {
      struct OpenAPI_Response *resp =
          (ensure_response_for_code(out_op, doc->response_headers[i].code,
                                    &_ast_ensure_response_for_code_7),
           _ast_ensure_response_for_code_7);
      /* LCOV_EXCL_START */
      if (!resp) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
      if (!resp->description) {
        resp->description =
            (c_cdd_strdup((c_cdd_str_iequal(doc->response_headers[i].code,
                                            "200", &_ast_iequal_101),
                           _ast_iequal_101)
                              ? "Success"
                              : "Response",
                          &_ast_strdup_100),
             _ast_strdup_100);
        /* LCOV_EXCL_START */
        if (!resp->description) {
          C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
          return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */
      }
      if (add_header_to_response(resp, &doc->response_headers[i]) != 0)
        return CDD_C_ERROR_MEMORY;
    }
  }

  if (doc && doc->n_links > 0) {
    for (i = 0; i < doc->n_links; ++i) {
      struct OpenAPI_Response *resp =
          (ensure_response_for_code(out_op, doc->links[i].code,
                                    &_ast_ensure_response_for_code_8),
           _ast_ensure_response_for_code_8);
      /* LCOV_EXCL_START */
      if (!resp) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
        return CDD_C_ERROR_MEMORY;
      }
      /* LCOV_EXCL_STOP */
      if (!resp->description) {
        resp->description =
            (c_cdd_strdup(
                 (c_cdd_str_iequal(doc->links[i].code, "200", &_ast_iequal_103),
                  _ast_iequal_103)
                     ? "Success"
                     : "Response",
                 &_ast_strdup_102),
             _ast_strdup_102);
        /* LCOV_EXCL_START */
        if (!resp->description) {
          C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
          return CDD_C_ERROR_MEMORY;
        }
        /* LCOV_EXCL_STOP */
      }
      if (add_link_to_response(resp, &doc->links[i]) != 0)
        return CDD_C_ERROR_MEMORY;
    }
  }

  if (out_op->n_responses == 0) {
    struct OpenAPI_Response *new_resps = (struct OpenAPI_Response *)realloc(
        out_op->responses, sizeof(struct OpenAPI_Response));
    struct OpenAPI_Response *r;
    /* LCOV_EXCL_START */
    if (!new_resps) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    out_op->responses = new_resps;
    r = &out_op->responses[out_op->n_responses++];
    memset(r, 0, sizeof(*r));
    r->code = (c_cdd_strdup("200", &_ast_strdup_104), _ast_strdup_104);
    /* LCOV_EXCL_START */
    if (!r->code) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
    r->description =
        (c_cdd_strdup("Success", &_ast_strdup_105), _ast_strdup_105);
    /* LCOV_EXCL_START */
    if (!r->description) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\\n");
      return CDD_C_ERROR_MEMORY;
    }
    /* LCOV_EXCL_STOP */
  }

  /* 3. Global Tags */
  /* Heuristic: use first part of function name? e.g. api_pet_get -> "pet" */
  {
    /* Extract resource name if pattern matches prefix_Resource_... */
    if (ctx->func_name && out_op->n_tags == 0) {
      char *dup_name =
          (c_cdd_strdup(ctx->func_name, &_ast_strdup_106), _ast_strdup_106);
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
          out_op->tags[0] =
              (c_cdd_strdup(token, &_ast_strdup_107), _ast_strdup_107);
          if (out_op->tags[0]) {
            if (out_op->tags[0][0])
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

/* LCOV_EXCL_STOP */
