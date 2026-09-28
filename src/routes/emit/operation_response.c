/**
 * @file operation_response.c
 * @brief Implementation of operation response helpers.
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

/**
 * @brief Frees header fields.
 */
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
