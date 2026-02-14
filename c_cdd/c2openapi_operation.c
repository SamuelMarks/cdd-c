/**
 * @file c2openapi_operation.c
 * @brief Implementation of the Operation Builder.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c2openapi_operation.h"
#include "c_mapping.h"
#include "str_utils.h"

/* --- Helpers --- */

static int is_reserved_header_name(const char *name) {
  if (!name || !*name)
    return 0;
  if (c_cdd_str_iequal(name, "accept"))
    return 1;
  if (c_cdd_str_iequal(name, "content-type"))
    return 1;
  if (c_cdd_str_iequal(name, "authorization"))
    return 1;
  return 0;
}

static int parse_example_any(const char *example, struct OpenAPI_Any *out) {
  JSON_Value *val;
  JSON_Value_Type t;
  const char *s;
  char *json_str;

  if (!example || !out)
    return 0;

  memset(out, 0, sizeof(*out));
  val = json_parse_string(example);
  if (!val) {
    out->type = OA_ANY_STRING;
    out->string = c_cdd_strdup(example);
    return out->string ? 0 : ENOMEM;
  }

  t = json_value_get_type(val);
  switch (t) {
  case JSONString:
    s = json_value_get_string(val);
    out->type = OA_ANY_STRING;
    out->string = c_cdd_strdup(s ? s : "");
    if (!out->string) {
      json_value_free(val);
      return ENOMEM;
    }
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
    if (!json_str) {
      json_value_free(val);
      return ENOMEM;
    }
    out->type = OA_ANY_JSON;
    out->json = c_cdd_strdup(json_str);
    json_free_serialized_string(json_str);
    if (!out->json) {
      json_value_free(val);
      return ENOMEM;
    }
    break;
  default:
    break;
  }

  json_value_free(val);
  return 0;
}

static int any_from_json_value(const JSON_Value *val, struct OpenAPI_Any *out) {
  JSON_Value_Type t;
  const char *s;
  char *json_str;

  if (!out)
    return EINVAL;
  memset(out, 0, sizeof(*out));
  if (!val)
    return 0;

  t = json_value_get_type(val);
  switch (t) {
  case JSONString:
    s = json_value_get_string(val);
    out->type = OA_ANY_STRING;
    out->string = c_cdd_strdup(s ? s : "");
    return out->string ? 0 : ENOMEM;
  case JSONNumber:
    out->type = OA_ANY_NUMBER;
    out->number = json_value_get_number(val);
    return 0;
  case JSONBoolean:
    out->type = OA_ANY_BOOL;
    out->boolean = json_value_get_boolean(val);
    return 0;
  case JSONNull:
    out->type = OA_ANY_NULL;
    return 0;
  case JSONObject:
  case JSONArray:
    json_str = json_serialize_to_string((JSON_Value *)val);
    if (!json_str)
      return ENOMEM;
    out->type = OA_ANY_JSON;
    out->json = c_cdd_strdup(json_str);
    json_free_serialized_string(json_str);
    return out->json ? 0 : ENOMEM;
  default:
    return 0;
  }
}

static int parse_link_params_json(const char *json,
                                  struct OpenAPI_LinkParam **out,
                                  size_t *out_count) {
  JSON_Value *val;
  JSON_Object *obj;
  size_t count, i;

  if (out)
    *out = NULL;
  if (out_count)
    *out_count = 0;
  if (!json || !out || !out_count)
    return 0;

  val = json_parse_string(json);
  if (!val)
    return EINVAL;

  if (json_value_get_type(val) != JSONObject) {
    json_value_free(val);
    return EINVAL;
  }

  obj = json_value_get_object(val);
  count = json_object_get_count(obj);
  if (count == 0) {
    json_value_free(val);
    return 0;
  }

  *out = (struct OpenAPI_LinkParam *)calloc(count,
                                            sizeof(struct OpenAPI_LinkParam));
  if (!*out) {
    json_value_free(val);
    return ENOMEM;
  }
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(obj, i);
    const JSON_Value *v = json_object_get_value_at(obj, i);
    struct OpenAPI_LinkParam *lp = &(*out)[i];

    lp->name = c_cdd_strdup(name ? name : "");
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
  return 0;

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
  return ENOMEM;
}

static int copy_any_value_local(struct OpenAPI_Any *dst,
                                const struct OpenAPI_Any *src) {
  if (!dst || !src)
    return 0;
  memset(dst, 0, sizeof(*dst));
  dst->type = src->type;
  switch (src->type) {
  case OA_ANY_STRING:
    dst->string = c_cdd_strdup(src->string ? src->string : "");
    return dst->string ? 0 : ENOMEM;
  case OA_ANY_JSON:
    dst->json = c_cdd_strdup(src->json ? src->json : "");
    return dst->json ? 0 : ENOMEM;
  case OA_ANY_NUMBER:
    dst->number = src->number;
    return 0;
  case OA_ANY_BOOL:
    dst->boolean = src->boolean;
    return 0;
  case OA_ANY_NULL:
  case OA_ANY_UNSET:
  default:
    return 0;
  }
}

static void free_any_value_local(struct OpenAPI_Any *val) {
  if (!val)
    return;
  if (val->type == OA_ANY_STRING && val->string)
    free(val->string);
  if (val->type == OA_ANY_JSON && val->json)
    free(val->json);
  memset(val, 0, sizeof(*val));
}

static const struct DocParam *find_doc_param(const struct DocMetadata *doc,
                                             const char *name) {
  size_t i;
  if (!doc || !name)
    return NULL;
  for (i = 0; i < doc->n_params; ++i) {
    if (doc->params[i].name && strcmp(doc->params[i].name, name) == 0) {
      return &doc->params[i];
    }
  }
  return NULL;
}

static int is_path_param(const char *route, const char *name) {
  char tmpl[128];
  if (!route || !name)
    return 0;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  sprintf_s(tmpl, sizeof(tmpl), "{%s}", name);
#else
  sprintf(tmpl, "{%s}", name);
#endif
  return strstr(route, tmpl) != NULL;
}

static void free_openapi_server_variables(struct OpenAPI_Server *srv) {
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

static int copy_doc_server_variables(struct OpenAPI_Server *dst,
                                     const struct DocServer *src) {
  size_t i;
  if (!dst || !src || src->n_variables == 0)
    return 0;

  dst->variables = (struct OpenAPI_ServerVariable *)calloc(
      src->n_variables, sizeof(struct OpenAPI_ServerVariable));
  if (!dst->variables)
    return ENOMEM;
  dst->n_variables = src->n_variables;

  for (i = 0; i < src->n_variables; ++i) {
    size_t e;
    int found_default = 0;
    const struct DocServerVar *sv = &src->variables[i];
    struct OpenAPI_ServerVariable *dv = &dst->variables[i];

    if (!sv->name || !sv->default_value) {
      free_openapi_server_variables(dst);
      return EINVAL;
    }
    dv->name = c_cdd_strdup(sv->name);
    if (!dv->name) {
      free_openapi_server_variables(dst);
      return ENOMEM;
    }
    dv->default_value = c_cdd_strdup(sv->default_value);
    if (!dv->default_value) {
      free_openapi_server_variables(dst);
      return ENOMEM;
    }
    if (sv->description) {
      dv->description = c_cdd_strdup(sv->description);
      if (!dv->description) {
        free_openapi_server_variables(dst);
        return ENOMEM;
      }
    }
    if (sv->enum_values && sv->n_enum_values > 0) {
      dv->enum_values = (char **)calloc(sv->n_enum_values, sizeof(char *));
      if (!dv->enum_values) {
        free_openapi_server_variables(dst);
        return ENOMEM;
      }
      dv->n_enum_values = sv->n_enum_values;
      for (e = 0; e < sv->n_enum_values; ++e) {
        dv->enum_values[e] = c_cdd_strdup(sv->enum_values[e]);
        if (!dv->enum_values[e]) {
          free_openapi_server_variables(dst);
          return ENOMEM;
        }
        if (strcmp(sv->enum_values[e], sv->default_value) == 0)
          found_default = 1;
      }
      if (!found_default) {
        free_openapi_server_variables(dst);
        return EINVAL;
      }
    }
  }

  return 0;
}

static struct OpenAPI_Response *
find_response_by_code(struct OpenAPI_Operation *op, const char *code) {
  size_t i;
  if (!op || !code)
    return NULL;
  for (i = 0; i < op->n_responses; ++i) {
    if (op->responses[i].code &&
        c_cdd_str_iequal(op->responses[i].code, code)) {
      return &op->responses[i];
    }
  }
  return NULL;
}

static struct OpenAPI_MediaType *find_media_type(struct OpenAPI_MediaType *mts,
                                                 size_t n, const char *name) {
  size_t i;
  if (!mts || !name)
    return NULL;
  for (i = 0; i < n; ++i) {
    if (mts[i].name && strcmp(mts[i].name, name) == 0)
      return &mts[i];
  }
  return NULL;
}

static int apply_example_to_media_type(struct OpenAPI_MediaType *mt,
                                       const char *example) {
  if (!mt || !example || mt->example_set)
    return 0;
  if (parse_example_any(example, &mt->example) != 0)
    return ENOMEM;
  mt->example_set = 1;
  return 0;
}

static int apply_example_to_response(struct OpenAPI_Response *resp,
                                     const char *example,
                                     const char *content_type) {
  size_t i;
  struct OpenAPI_Any parsed = {0};

  if (!resp || !example)
    return 0;

  if (resp->content_media_types && resp->n_content_media_types > 0) {
    if (parse_example_any(example, &parsed) != 0)
      return ENOMEM;
    if (content_type) {
      struct OpenAPI_MediaType *mt = find_media_type(
          resp->content_media_types, resp->n_content_media_types, content_type);
      if (mt && !mt->example_set) {
        if (copy_any_value_local(&mt->example, &parsed) != 0) {
          free_any_value_local(&parsed);
          return ENOMEM;
        }
        mt->example_set = 1;
      }
      free_any_value_local(&parsed);
      return 0;
    }
    for (i = 0; i < resp->n_content_media_types; ++i) {
      struct OpenAPI_MediaType *mt = &resp->content_media_types[i];
      if (mt->example_set)
        continue;
      if (copy_any_value_local(&mt->example, &parsed) != 0) {
        free_any_value_local(&parsed);
        return ENOMEM;
      }
      mt->example_set = 1;
    }
    free_any_value_local(&parsed);
    return 0;
  }

  if (resp->example_set)
    return 0;
  if (parse_example_any(example, &resp->example) != 0)
    return ENOMEM;
  resp->example_set = 1;
  return 0;
}

static struct OpenAPI_Response *
ensure_response_for_code(struct OpenAPI_Operation *op, const char *code) {
  struct OpenAPI_Response *resp;
  struct OpenAPI_Response *new_resps;
  if (!op || !code)
    return NULL;

  resp = find_response_by_code(op, code);
  if (resp)
    return resp;

  new_resps = (struct OpenAPI_Response *)realloc(
      op->responses, (op->n_responses + 1) * sizeof(struct OpenAPI_Response));
  if (!new_resps)
    return NULL;
  op->responses = new_resps;
  resp = &op->responses[op->n_responses++];
  memset(resp, 0, sizeof(*resp));
  resp->code = c_cdd_strdup(code);
  if (!resp->code)
    return NULL;
  resp->description =
      c_cdd_strdup(c_cdd_str_iequal(code, "200") ? "Success" : "Response");
  if (!resp->description)
    return NULL;
  return resp;
}

static int add_header_to_response(struct OpenAPI_Response *resp,
                                  const struct DocResponseHeader *dh) {
  struct OpenAPI_Header *new_headers;
  struct OpenAPI_Header *hdr;
  size_t i;

  if (!resp || !dh || !dh->name)
    return EINVAL;

  for (i = 0; i < resp->n_headers; ++i) {
    if (resp->headers[i].name &&
        c_cdd_str_iequal(resp->headers[i].name, dh->name)) {
      hdr = &resp->headers[i];
      if (dh->description && !hdr->description) {
        hdr->description = c_cdd_strdup(dh->description);
        if (!hdr->description)
          return ENOMEM;
      }
      if (dh->type && !hdr->type) {
        hdr->type = c_cdd_strdup(dh->type);
        if (!hdr->type)
          return ENOMEM;
      }
      if (dh->content_type && !hdr->content_type) {
        hdr->content_type = c_cdd_strdup(dh->content_type);
        if (!hdr->content_type)
          return ENOMEM;
      }
      if (dh->format) {
        hdr->schema_set = 1;
        if (!hdr->schema.inline_type) {
          hdr->schema.inline_type =
              c_cdd_strdup(hdr->type ? hdr->type : "string");
          if (!hdr->schema.inline_type)
            return ENOMEM;
        }
        if (hdr->schema.format)
          free(hdr->schema.format);
        hdr->schema.format = c_cdd_strdup(dh->format);
        if (!hdr->schema.format)
          return ENOMEM;
      }
      if (dh->required_set)
        hdr->required = dh->required ? 1 : 0;
      if (dh->example && !hdr->example_set) {
        if (parse_example_any(dh->example, &hdr->example) != 0)
          return ENOMEM;
        hdr->example_set = 1;
        hdr->example_location =
            hdr->content_type ? OA_EXAMPLE_LOC_MEDIA : OA_EXAMPLE_LOC_OBJECT;
      }
      return 0;
    }
  }

  new_headers = (struct OpenAPI_Header *)realloc(
      resp->headers, (resp->n_headers + 1) * sizeof(struct OpenAPI_Header));
  if (!new_headers)
    return ENOMEM;
  resp->headers = new_headers;
  hdr = &resp->headers[resp->n_headers++];
  memset(hdr, 0, sizeof(*hdr));
  hdr->name = c_cdd_strdup(dh->name);
  if (!hdr->name)
    return ENOMEM;
  if (dh->description) {
    hdr->description = c_cdd_strdup(dh->description);
    if (!hdr->description)
      return ENOMEM;
  }
  hdr->type = c_cdd_strdup(dh->type ? dh->type : "string");
  if (!hdr->type)
    return ENOMEM;
  if (dh->content_type) {
    hdr->content_type = c_cdd_strdup(dh->content_type);
    if (!hdr->content_type)
      return ENOMEM;
  }
  if (dh->format) {
    hdr->schema_set = 1;
    hdr->schema.inline_type = c_cdd_strdup(hdr->type ? hdr->type : "string");
    if (!hdr->schema.inline_type)
      return ENOMEM;
    hdr->schema.format = c_cdd_strdup(dh->format);
    if (!hdr->schema.format)
      return ENOMEM;
  }
  if (dh->required_set)
    hdr->required = dh->required ? 1 : 0;
  if (dh->example && !hdr->example_set) {
    if (parse_example_any(dh->example, &hdr->example) != 0)
      return ENOMEM;
    hdr->example_set = 1;
    hdr->example_location =
        hdr->content_type ? OA_EXAMPLE_LOC_MEDIA : OA_EXAMPLE_LOC_OBJECT;
  }
  return 0;
}

static int add_link_to_response(struct OpenAPI_Response *resp,
                                const struct DocLink *dl) {
  struct OpenAPI_Link *new_links;
  struct OpenAPI_Link *link;
  size_t i;

  if (!resp || !dl || !dl->name)
    return EINVAL;

  if ((!dl->operation_id && !dl->operation_ref) ||
      (dl->operation_id && dl->operation_ref))
    return EINVAL;

  for (i = 0; i < resp->n_links; ++i) {
    if (resp->links[i].name && strcmp(resp->links[i].name, dl->name) == 0) {
      return EINVAL;
    }
  }

  new_links = (struct OpenAPI_Link *)realloc(
      resp->links, (resp->n_links + 1) * sizeof(struct OpenAPI_Link));
  if (!new_links)
    return ENOMEM;
  resp->links = new_links;
  link = &resp->links[resp->n_links++];
  memset(link, 0, sizeof(*link));

  link->name = c_cdd_strdup(dl->name);
  if (!link->name)
    return ENOMEM;
  if (dl->summary) {
    link->summary = c_cdd_strdup(dl->summary);
    if (!link->summary)
      return ENOMEM;
  }
  if (dl->description) {
    link->description = c_cdd_strdup(dl->description);
    if (!link->description)
      return ENOMEM;
  }
  if (dl->operation_id) {
    link->operation_id = c_cdd_strdup(dl->operation_id);
    if (!link->operation_id)
      return ENOMEM;
  }
  if (dl->operation_ref) {
    link->operation_ref = c_cdd_strdup(dl->operation_ref);
    if (!link->operation_ref)
      return ENOMEM;
  }
  if (dl->parameters_json) {
    int rc = parse_link_params_json(dl->parameters_json, &link->parameters,
                                    &link->n_parameters);
    if (rc != 0)
      return rc;
  }
  if (dl->request_body_json) {
    if (parse_example_any(dl->request_body_json, &link->request_body) != 0)
      return ENOMEM;
    link->request_body_set = 1;
  }
  if (dl->server_url) {
    link->server = (struct OpenAPI_Server *)calloc(1, sizeof(*link->server));
    if (!link->server)
      return ENOMEM;
    link->server_set = 1;
    link->server->url = c_cdd_strdup(dl->server_url);
    if (!link->server->url) {
      free(link->server);
      link->server = NULL;
      link->server_set = 0;
      return ENOMEM;
    }
    if (dl->server_name) {
      link->server->name = c_cdd_strdup(dl->server_name);
      if (!link->server->name) {
        free(link->server->url);
        free(link->server);
        link->server = NULL;
        link->server_set = 0;
        return ENOMEM;
      }
    }
    if (dl->server_description) {
      link->server->description = c_cdd_strdup(dl->server_description);
      if (!link->server->description) {
        if (link->server->name)
          free(link->server->name);
        free(link->server->url);
        free(link->server);
        link->server = NULL;
        link->server_set = 0;
        return ENOMEM;
      }
    }
  }

  return 0;
}

static int add_param_to_op(struct OpenAPI_Operation *op,
                           struct OpenAPI_Parameter *p) {
  struct OpenAPI_Parameter *new_arr;
  size_t new_count = op->n_parameters + 1;

  new_arr = (struct OpenAPI_Parameter *)realloc(
      op->parameters, new_count * sizeof(struct OpenAPI_Parameter));
  if (!new_arr)
    return ENOMEM;

  op->parameters = new_arr;
  op->parameters[op->n_parameters] = *p; /* Copy struct */
  op->n_parameters = new_count;
  return 0;
}

static int schema_ref_has_data_basic(const struct OpenAPI_SchemaRef *ref) {
  if (!ref)
    return 0;
  return (ref->ref_name && *ref->ref_name) || (ref->ref && *ref->ref) ||
         (ref->inline_type && *ref->inline_type) || ref->is_array;
}

static int copy_schema_ref_basic(struct OpenAPI_SchemaRef *dst,
                                 const struct OpenAPI_SchemaRef *src) {
  if (!dst || !src)
    return 0;
  memset(dst, 0, sizeof(*dst));
  dst->is_array = src->is_array;
  if (src->ref_name) {
    dst->ref_name = c_cdd_strdup(src->ref_name);
    if (!dst->ref_name)
      return ENOMEM;
  }
  if (src->ref) {
    dst->ref = c_cdd_strdup(src->ref);
    if (!dst->ref)
      return ENOMEM;
  }
  dst->ref_is_dynamic = src->ref_is_dynamic;
  if (src->inline_type) {
    dst->inline_type = c_cdd_strdup(src->inline_type);
    if (!dst->inline_type)
      return ENOMEM;
  }
  if (src->items_ref) {
    dst->items_ref = c_cdd_strdup(src->items_ref);
    if (!dst->items_ref)
      return ENOMEM;
  }
  dst->items_ref_is_dynamic = src->items_ref_is_dynamic;
  if (src->format) {
    dst->format = c_cdd_strdup(src->format);
    if (!dst->format)
      return ENOMEM;
  }
  if (src->items_format) {
    dst->items_format = c_cdd_strdup(src->items_format);
    if (!dst->items_format)
      return ENOMEM;
  }
  return 0;
}

static int response_has_media_type(const struct OpenAPI_Response *resp,
                                   const char *name) {
  size_t i;
  if (!resp || !name)
    return 0;
  if (resp->content_type && strcmp(resp->content_type, name) == 0)
    return 1;
  if (!resp->content_media_types || resp->n_content_media_types == 0)
    return 0;
  for (i = 0; i < resp->n_content_media_types; ++i) {
    const struct OpenAPI_MediaType *mt = &resp->content_media_types[i];
    if (mt->name && strcmp(mt->name, name) == 0)
      return 1;
  }
  return 0;
}

static int init_media_type_from_response(struct OpenAPI_MediaType *mt,
                                         const char *name,
                                         const struct OpenAPI_Response *resp) {
  if (!mt || !name || !resp)
    return EINVAL;
  memset(mt, 0, sizeof(*mt));
  mt->name = c_cdd_strdup(name);
  if (!mt->name)
    return ENOMEM;
  if (schema_ref_has_data_basic(&resp->schema)) {
    if (copy_schema_ref_basic(&mt->schema, &resp->schema) != 0)
      return ENOMEM;
    mt->schema_set = 1;
  }
  return 0;
}

static int add_response_media_type(struct OpenAPI_Response *resp,
                                   const char *name) {
  struct OpenAPI_MediaType *new_mts;
  size_t new_count;

  if (!resp || !name || !*name)
    return 0;
  if (response_has_media_type(resp, name))
    return 0;

  if (!resp->content_media_types) {
    size_t base = resp->content_type ? 1 : 0;
    resp->content_media_types = (struct OpenAPI_MediaType *)calloc(
        base + 1, sizeof(struct OpenAPI_MediaType));
    if (!resp->content_media_types)
      return ENOMEM;
    resp->n_content_media_types = 0;
    if (resp->content_type) {
      if (init_media_type_from_response(&resp->content_media_types[0],
                                        resp->content_type, resp) != 0)
        return ENOMEM;
      resp->n_content_media_types = 1;
    }
  }

  new_count = resp->n_content_media_types + 1;
  new_mts = (struct OpenAPI_MediaType *)realloc(
      resp->content_media_types, new_count * sizeof(struct OpenAPI_MediaType));
  if (!new_mts)
    return ENOMEM;
  resp->content_media_types = new_mts;
  if (init_media_type_from_response(
          &resp->content_media_types[resp->n_content_media_types], name,
          resp) != 0)
    return ENOMEM;
  resp->n_content_media_types = new_count;
  return 0;
}

static int request_body_has_media_type(const struct OpenAPI_Operation *op,
                                       const char *name) {
  size_t i;
  if (!op || !name)
    return 0;
  if (op->req_body.content_type && strcmp(op->req_body.content_type, name) == 0)
    return 1;
  if (!op->req_body_media_types || op->n_req_body_media_types == 0)
    return 0;
  for (i = 0; i < op->n_req_body_media_types; ++i) {
    const struct OpenAPI_MediaType *mt = &op->req_body_media_types[i];
    if (mt->name && strcmp(mt->name, name) == 0)
      return 1;
  }
  return 0;
}

static int
init_media_type_from_request_body(struct OpenAPI_MediaType *mt,
                                  const char *name,
                                  const struct OpenAPI_Operation *op) {
  if (!mt || !name || !op)
    return EINVAL;
  memset(mt, 0, sizeof(*mt));
  mt->name = c_cdd_strdup(name);
  if (!mt->name)
    return ENOMEM;
  if (schema_ref_has_data_basic(&op->req_body)) {
    if (copy_schema_ref_basic(&mt->schema, &op->req_body) != 0)
      return ENOMEM;
    mt->schema_set = 1;
  }
  return 0;
}

static int add_request_body_media_type(struct OpenAPI_Operation *op,
                                       const char *name) {
  struct OpenAPI_MediaType *new_mts;
  size_t new_count;

  if (!op || !name || !*name)
    return 0;
  if (request_body_has_media_type(op, name))
    return 0;

  if (!op->req_body_media_types) {
    size_t base = op->req_body.content_type ? 1 : 0;
    op->req_body_media_types = (struct OpenAPI_MediaType *)calloc(
        base + 1, sizeof(struct OpenAPI_MediaType));
    if (!op->req_body_media_types)
      return ENOMEM;
    op->n_req_body_media_types = 0;
    if (op->req_body.content_type) {
      if (init_media_type_from_request_body(&op->req_body_media_types[0],
                                            op->req_body.content_type, op) != 0)
        return ENOMEM;
      op->n_req_body_media_types = 1;
    }
  }

  new_count = op->n_req_body_media_types + 1;
  new_mts = (struct OpenAPI_MediaType *)realloc(
      op->req_body_media_types, new_count * sizeof(struct OpenAPI_MediaType));
  if (!new_mts)
    return ENOMEM;
  op->req_body_media_types = new_mts;
  if (init_media_type_from_request_body(
          &op->req_body_media_types[op->n_req_body_media_types], name, op) != 0)
    return ENOMEM;
  op->n_req_body_media_types = new_count;
  return 0;
}

static int set_querystring_schema_from_type_map(
    struct OpenAPI_Parameter *param,
    const struct OpenApiTypeMapping *type_map) {
  if (!param || !type_map)
    return 0;
  if (type_map->ref_name) {
    param->schema_set = 1;
    param->schema.is_array = (type_map->kind == OA_TYPE_ARRAY);
    param->schema.ref_name = c_cdd_strdup(type_map->ref_name);
    if (!param->schema.ref_name)
      return ENOMEM;
    return 0;
  }
  if (type_map->kind == OA_TYPE_ARRAY) {
    param->is_array = 1;
    param->type = c_cdd_strdup("array");
    if (!param->type)
      return ENOMEM;
    if (type_map->oa_type) {
      param->items_type = c_cdd_strdup(type_map->oa_type);
      if (!param->items_type)
        return ENOMEM;
    }
    return 0;
  }
  if (type_map->oa_type) {
    param->type = c_cdd_strdup(type_map->oa_type);
  } else {
    param->type = c_cdd_strdup("string");
  }
  return param->type ? 0 : ENOMEM;
}

static int oa_type_is_primitive(const char *type) {
  if (!type)
    return 0;
  return strcmp(type, "integer") == 0 || strcmp(type, "number") == 0 ||
         strcmp(type, "string") == 0 || strcmp(type, "boolean") == 0;
}

/* Apply format from type mapping (or override) to a SchemaRef.
 * Returns: 1 if applied, 0 if not applicable, or ENOMEM on allocation failure.
 */
static int apply_format_to_schema_ref(struct OpenAPI_SchemaRef *schema,
                                      const struct OpenApiTypeMapping *map,
                                      const char *override_format) {
  const char *fmt;
  if (!schema || !map)
    return 0;
  fmt =
      (override_format && *override_format) ? override_format : map->oa_format;
  if (!fmt || !*fmt)
    return 0;
  if (!map->oa_type || !oa_type_is_primitive(map->oa_type))
    return 0;

  if (map->kind == OA_TYPE_ARRAY) {
    schema->is_array = 1;
    if (!schema->inline_type) {
      schema->inline_type = c_cdd_strdup(map->oa_type);
      if (!schema->inline_type)
        return ENOMEM;
    }
    if (schema->items_format)
      free(schema->items_format);
    schema->items_format = c_cdd_strdup(fmt);
    if (!schema->items_format)
      return ENOMEM;
  } else {
    if (!schema->inline_type) {
      schema->inline_type = c_cdd_strdup(map->oa_type);
      if (!schema->inline_type)
        return ENOMEM;
    }
    if (schema->format)
      free(schema->format);
    schema->format = c_cdd_strdup(fmt);
    if (!schema->format)
      return ENOMEM;
  }
  return 1;
}

/* --- Type Analysis --- */

/**
 * @brief Determine if a type is a struct pointer eligible for Body.
 * Heuristic: Contains "struct", ends with "*" or "**".
 */
static int is_struct_pointer(const char *type, int *is_double_ptr) {
  const char *p;
  if (!type)
    return 0;
  if (!strstr(type, "struct "))
    return 0;

  p = strrchr(type, '*');
  if (!p)
    return 0;

  if (p > type && *(p - 1) == '*')
    *is_double_ptr = 1;
  else
    *is_double_ptr = 0;

  return 1;
}

static enum OpenAPI_Style doc_style_to_openapi(enum DocParamStyle style) {
  switch (style) {
  case DOC_PARAM_STYLE_FORM:
    return OA_STYLE_FORM;
  case DOC_PARAM_STYLE_SIMPLE:
    return OA_STYLE_SIMPLE;
  case DOC_PARAM_STYLE_MATRIX:
    return OA_STYLE_MATRIX;
  case DOC_PARAM_STYLE_LABEL:
    return OA_STYLE_LABEL;
  case DOC_PARAM_STYLE_SPACE_DELIMITED:
    return OA_STYLE_SPACE_DELIMITED;
  case DOC_PARAM_STYLE_PIPE_DELIMITED:
    return OA_STYLE_PIPE_DELIMITED;
  case DOC_PARAM_STYLE_DEEP_OBJECT:
    return OA_STYLE_DEEP_OBJECT;
  case DOC_PARAM_STYLE_COOKIE:
    return OA_STYLE_COOKIE;
  case DOC_PARAM_STYLE_UNSET:
  default:
    return OA_STYLE_UNKNOWN;
  }
}

/* --- Core Logic --- */

int c2openapi_build_operation(const struct OpBuilderContext *const ctx,
                              struct OpenAPI_Operation *const out_op) {
  const struct C2OpenAPI_ParsedSig *sig = ctx->sig;
  const struct DocMetadata *doc = ctx->doc;
  size_t i;
  int rc = 0;

  if (!ctx || !out_op || !sig)
    return EINVAL;

  /* 0. Basic Metadata */
  if (doc && doc->verb) {
    /* Map string verb to enum */
    /* Only basic check here, loader does robust parsing */
    if (strcasecmp(doc->verb, "GET") == 0)
      out_op->verb = OA_VERB_GET;
    else if (strcasecmp(doc->verb, "POST") == 0)
      out_op->verb = OA_VERB_POST;
    else if (strcasecmp(doc->verb, "PUT") == 0)
      out_op->verb = OA_VERB_PUT;
    else if (strcasecmp(doc->verb, "DELETE") == 0)
      out_op->verb = OA_VERB_DELETE;
    else if (strcasecmp(doc->verb, "PATCH") == 0)
      out_op->verb = OA_VERB_PATCH;
    else if (strcasecmp(doc->verb, "HEAD") == 0)
      out_op->verb = OA_VERB_HEAD;
    else if (strcasecmp(doc->verb, "OPTIONS") == 0)
      out_op->verb = OA_VERB_OPTIONS;
    else if (strcasecmp(doc->verb, "TRACE") == 0)
      out_op->verb = OA_VERB_TRACE;
    else if (strcasecmp(doc->verb, "QUERY") == 0)
      out_op->verb = OA_VERB_QUERY;
    else {
      out_op->verb = OA_VERB_UNKNOWN;
      out_op->is_additional = 1;
      out_op->method = c_cdd_strdup(doc->verb);
      if (!out_op->method)
        return ENOMEM;
    }
  } else {
    /* Guess from name? e.g. "api_get_..." */
    /* Heuristics:
       api_post_X
       api_X_create  -> _create
       api_X_update  -> _update
       api_X_delete  -> _delete
    */
    if (c_cdd_str_starts_with(ctx->func_name, "api_post_") ||
        strstr(ctx->func_name, "_create"))
      out_op->verb = OA_VERB_POST;
    else if (c_cdd_str_starts_with(ctx->func_name, "api_put_") ||
             strstr(ctx->func_name, "_update"))
      out_op->verb = OA_VERB_PUT;
    else if (c_cdd_str_starts_with(ctx->func_name, "api_delete_") ||
             strstr(ctx->func_name, "_delete"))
      out_op->verb = OA_VERB_DELETE;
    else
      out_op->verb = OA_VERB_GET;
  }

  if (doc && doc->operation_id) {
    out_op->operation_id = c_cdd_strdup(doc->operation_id);
  } else {
    out_op->operation_id = c_cdd_strdup(ctx->func_name);
  }
  if (!out_op->operation_id)
    return ENOMEM;
  if (doc && doc->summary) {
    out_op->summary = c_cdd_strdup(doc->summary);
    if (!out_op->summary)
      return ENOMEM;
  }
  if (doc && doc->description) {
    out_op->description = c_cdd_strdup(doc->description);
    if (!out_op->description)
      return ENOMEM;
  }
  if (doc && doc->deprecated_set) {
    out_op->deprecated = doc->deprecated ? 1 : 0;
  }
  if (doc && doc->external_docs_url) {
    out_op->external_docs.url = c_cdd_strdup(doc->external_docs_url);
    if (!out_op->external_docs.url)
      return ENOMEM;
    if (doc->external_docs_description) {
      out_op->external_docs.description =
          c_cdd_strdup(doc->external_docs_description);
      if (!out_op->external_docs.description)
        return ENOMEM;
    }
  }
  if (doc && doc->n_tags > 0) {
    size_t t;
    out_op->tags = (char **)calloc(doc->n_tags, sizeof(char *));
    if (!out_op->tags)
      return ENOMEM;
    out_op->n_tags = doc->n_tags;
    for (t = 0; t < doc->n_tags; ++t) {
      out_op->tags[t] = c_cdd_strdup(doc->tags[t] ? doc->tags[t] : "");
      if (!out_op->tags[t])
        return ENOMEM;
    }
  }

  if (doc && doc->n_security > 0) {
    size_t s;
    out_op->security = (struct OpenAPI_SecurityRequirementSet *)calloc(
        doc->n_security, sizeof(struct OpenAPI_SecurityRequirementSet));
    if (!out_op->security)
      return ENOMEM;
    out_op->n_security = doc->n_security;
    out_op->security_set = 1;
    for (s = 0; s < doc->n_security; ++s) {
      struct OpenAPI_SecurityRequirementSet *set = &out_op->security[s];
      const struct DocSecurityRequirement *src = &doc->security[s];
      set->requirements = (struct OpenAPI_SecurityRequirement *)calloc(
          1, sizeof(struct OpenAPI_SecurityRequirement));
      if (!set->requirements)
        return ENOMEM;
      set->n_requirements = 1;
      set->requirements[0].scheme =
          c_cdd_strdup(src->scheme ? src->scheme : "");
      if (!set->requirements[0].scheme)
        return ENOMEM;
      if (src->n_scopes > 0) {
        size_t k;
        set->requirements[0].scopes =
            (char **)calloc(src->n_scopes, sizeof(char *));
        if (!set->requirements[0].scopes)
          return ENOMEM;
        set->requirements[0].n_scopes = src->n_scopes;
        for (k = 0; k < src->n_scopes; ++k) {
          set->requirements[0].scopes[k] =
              c_cdd_strdup(src->scopes[k] ? src->scopes[k] : "");
          if (!set->requirements[0].scopes[k])
            return ENOMEM;
        }
      }
    }
  }

  if (doc && doc->n_servers > 0) {
    size_t s;
    out_op->servers = (struct OpenAPI_Server *)calloc(
        doc->n_servers, sizeof(struct OpenAPI_Server));
    if (!out_op->servers)
      return ENOMEM;
    out_op->n_servers = doc->n_servers;
    for (s = 0; s < doc->n_servers; ++s) {
      const struct DocServer *src = &doc->servers[s];
      if (src->url) {
        out_op->servers[s].url = c_cdd_strdup(src->url);
        if (!out_op->servers[s].url)
          return ENOMEM;
      }
      if (src->name) {
        out_op->servers[s].name = c_cdd_strdup(src->name);
        if (!out_op->servers[s].name)
          return ENOMEM;
      }
      if (src->description) {
        out_op->servers[s].description = c_cdd_strdup(src->description);
        if (!out_op->servers[s].description)
          return ENOMEM;
      }
      if (src->n_variables > 0) {
        int vrc = copy_doc_server_variables(&out_op->servers[s], src);
        if (vrc != 0)
          return vrc;
      }
    }
  }

  /* 1. Argument Iteration */
  for (i = 0; i < sig->n_args; ++i) {
    const struct C2OpenAPI_ParsedArg *arg = &sig->args[i];
    const struct DocParam *dp = find_doc_param(doc, arg->name);
    struct OpenAPI_Parameter curr_param;
    struct OpenApiTypeMapping type_map;
    int is_path = 0;
    int is_body = 0;
    int is_out_ptr = 0;
    int is_querystring = 0;

    memset(&curr_param, 0, sizeof(curr_param));
    c_mapping_init(&type_map);

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
      if (is_struct_pointer(arg->type, &is_double)) {
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
      if (!new_resps) {
        c_mapping_free(&type_map);
        return ENOMEM;
      }
      out_op->responses = new_resps;
      out_op->n_responses++;

      r = &out_op->responses[r_idx];
      memset(r, 0, sizeof(*r));
      r->code = c_cdd_strdup("200"); /* Default success */
      r->description = c_cdd_strdup("Success");
      if (!r->description) {
        c_mapping_free(&type_map);
        return ENOMEM;
      }

      /* Map Schema */
      r->schema.is_array = (type_map.kind == OA_TYPE_ARRAY);
      if (type_map.ref_name) {
        r->schema.ref_name = c_cdd_strdup(type_map.ref_name);
      } else if (type_map.oa_type) {
        r->schema.inline_type = c_cdd_strdup(type_map.oa_type);
      }
      {
        int fmt_rc = apply_format_to_schema_ref(&r->schema, &type_map, NULL);
        if (fmt_rc == ENOMEM) {
          c_mapping_free(&type_map);
          return ENOMEM;
        }
      }

      c_mapping_free(&type_map);
      continue; /* Done with this arg */
    }

    if (is_body) {
      /* Request Body Population */
      out_op->req_body.content_type = c_cdd_strdup("application/json");
      out_op->req_body.is_array = (type_map.kind == OA_TYPE_ARRAY);
      /* Use ref_name if object, or type if primitive */
      if (type_map.ref_name) {
        out_op->req_body.ref_name = c_cdd_strdup(type_map.ref_name);
      } else if (type_map.oa_type) {
        out_op->req_body.inline_type = c_cdd_strdup(type_map.oa_type);
      }
      out_op->req_body_required = 1;
      out_op->req_body_required_set = 1;
      {
        int fmt_rc =
            apply_format_to_schema_ref(&out_op->req_body, &type_map, NULL);
        if (fmt_rc == ENOMEM) {
          c_mapping_free(&type_map);
          return ENOMEM;
        }
      }

      c_mapping_free(&type_map);
      continue;
    }

    /* --- Standard Parameter (Query/Path/Header/Cookie) --- */

    curr_param.name = c_cdd_strdup(arg->name);
    curr_param.required = is_path; /* Path params always required */
    if (dp && dp->required)
      curr_param.required = 1;
    if (dp && dp->description) {
      curr_param.description = c_cdd_strdup(dp->description);
      if (!curr_param.description) {
        c_mapping_free(&type_map);
        return ENOMEM;
      }
    }

    curr_param.in = is_path ? OA_PARAM_IN_PATH : OA_PARAM_IN_QUERY;
    if (dp && dp->in_loc && strcmp(dp->in_loc, "header") == 0)
      curr_param.in = OA_PARAM_IN_HEADER;
    else if (dp && dp->in_loc && strcmp(dp->in_loc, "cookie") == 0)
      curr_param.in = OA_PARAM_IN_COOKIE;
    else if (is_querystring)
      curr_param.in = OA_PARAM_IN_QUERYSTRING;

    if (curr_param.in == OA_PARAM_IN_HEADER &&
        is_reserved_header_name(curr_param.name)) {
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
          c_cdd_strdup("application/x-www-form-urlencoded");
      if (!curr_param.content_type) {
        c_mapping_free(&type_map);
        return ENOMEM;
      }
      rc = set_querystring_schema_from_type_map(&curr_param, &type_map);
      if (rc != 0)
        return rc;
    } else if (type_map.kind == OA_TYPE_ARRAY) {
      curr_param.is_array = 1;
      /* Logic for items_type: c_mapper stores item type in oa_type/ref_name
       * when kind=ARRAY */
      if (type_map.oa_type)
        curr_param.items_type = c_cdd_strdup(type_map.oa_type);
      else if (type_map.ref_name)
        curr_param.items_type = c_cdd_strdup(type_map.ref_name);

      curr_param.type = c_cdd_strdup("array");
    } else {
      /* Primitive / Object (if scalar param is allowed object??) usually string
       */
      /* Spec allows object parameters but they serialize weirdly. Assume string
       * representation unless primitive. */
      if (type_map.oa_type)
        curr_param.type = c_cdd_strdup(type_map.oa_type);
      else
        curr_param.type =
            c_cdd_strdup("string"); /* Fallback for complex types in params */
    }

    {
      const char *fmt_override = (dp && dp->format) ? dp->format : NULL;
      int fmt_rc = apply_format_to_schema_ref(&curr_param.schema, &type_map,
                                              fmt_override);
      if (fmt_rc == ENOMEM) {
        c_mapping_free(&type_map);
        return ENOMEM;
      }
      if (fmt_rc > 0)
        curr_param.schema_set = 1;
    }

    if (dp) {
      if (dp->content_type) {
        if (curr_param.content_type)
          free(curr_param.content_type);
        curr_param.content_type = c_cdd_strdup(dp->content_type);
        if (!curr_param.content_type) {
          c_mapping_free(&type_map);
          return ENOMEM;
        }
      }
      if (!curr_param.content_type) {
        if (dp->style_set) {
          enum OpenAPI_Style style = doc_style_to_openapi(dp->style);
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
      if (ex_rc == ENOMEM) {
        c_mapping_free(&type_map);
        return ENOMEM;
      }
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
        if (rb_idx == 0) {
          if (out_op->req_body.content_type)
            free(out_op->req_body.content_type);
          out_op->req_body.content_type = c_cdd_strdup(rb_content_type);
          if (!out_op->req_body.content_type)
            return ENOMEM;
        }
        if (add_request_body_media_type(out_op, rb_content_type) != 0)
          return ENOMEM;
        if (rb->example) {
          struct OpenAPI_MediaType *mt =
              find_media_type(out_op->req_body_media_types,
                              out_op->n_req_body_media_types, rb_content_type);
          if (mt && apply_example_to_media_type(mt, rb->example) != 0)
            return ENOMEM;
        }
      }
    }
    if (doc->request_body_description) {
      out_op->req_body_description =
          c_cdd_strdup(doc->request_body_description);
      if (!out_op->req_body_description)
        return ENOMEM;
    }
    if (doc->request_body_required_set) {
      out_op->req_body_required_set = 1;
      out_op->req_body_required = doc->request_body_required ? 1 : 0;
    }
    if (doc->request_body_content_type && doc->n_request_bodies == 0) {
      if (out_op->req_body.content_type)
        free(out_op->req_body.content_type);
      out_op->req_body.content_type =
          c_cdd_strdup(doc->request_body_content_type);
      if (!out_op->req_body.content_type)
        return ENOMEM;
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
        if (strcmp(out_op->responses[k].code, doc->returns[i].code) == 0) {
          exists = 1;
          if (!out_op->responses[k].summary && doc->returns[i].summary) {
            out_op->responses[k].summary =
                c_cdd_strdup(doc->returns[i].summary);
            if (!out_op->responses[k].summary)
              return ENOMEM;
          }
          if (!out_op->responses[k].description &&
              doc->returns[i].description) {
            out_op->responses[k].description =
                c_cdd_strdup(doc->returns[i].description);
            if (!out_op->responses[k].description)
              return ENOMEM;
          }
          if (doc->returns[i].content_type) {
            int add_rc = add_response_media_type(&out_op->responses[k],
                                                 doc->returns[i].content_type);
            if (add_rc != 0)
              return add_rc;
            if (!out_op->responses[k].content_type) {
              out_op->responses[k].content_type =
                  c_cdd_strdup(doc->returns[i].content_type);
              if (!out_op->responses[k].content_type)
                return ENOMEM;
            }
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
        if (!new_resps)
          return ENOMEM;
        out_op->responses = new_resps;
        r = &out_op->responses[out_op->n_responses++];
        memset(r, 0, sizeof(*r));
        r->code = c_cdd_strdup(doc->returns[i].code);
        if (doc->returns[i].summary) {
          r->summary = c_cdd_strdup(doc->returns[i].summary);
          if (!r->summary)
            return ENOMEM;
        }
        if (doc->returns[i].description) {
          r->description = c_cdd_strdup(doc->returns[i].description);
          if (!r->description)
            return ENOMEM;
        }
        if (doc->returns[i].content_type) {
          r->content_type = c_cdd_strdup(doc->returns[i].content_type);
          if (!r->content_type)
            return ENOMEM;
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
          ensure_response_for_code(out_op, doc->response_headers[i].code);
      if (!resp)
        return ENOMEM;
      if (!resp->description) {
        resp->description =
            c_cdd_strdup(c_cdd_str_iequal(doc->response_headers[i].code, "200")
                             ? "Success"
                             : "Response");
        if (!resp->description)
          return ENOMEM;
      }
      if (add_header_to_response(resp, &doc->response_headers[i]) != 0)
        return ENOMEM;
    }
  }

  if (doc && doc->n_links > 0) {
    for (i = 0; i < doc->n_links; ++i) {
      struct OpenAPI_Response *resp =
          ensure_response_for_code(out_op, doc->links[i].code);
      if (!resp)
        return ENOMEM;
      if (!resp->description) {
        resp->description = c_cdd_strdup(
            c_cdd_str_iequal(doc->links[i].code, "200") ? "Success"
                                                        : "Response");
        if (!resp->description)
          return ENOMEM;
      }
      if (add_link_to_response(resp, &doc->links[i]) != 0)
        return ENOMEM;
    }
  }

  if (out_op->n_responses == 0) {
    struct OpenAPI_Response *new_resps = (struct OpenAPI_Response *)realloc(
        out_op->responses, sizeof(struct OpenAPI_Response));
    struct OpenAPI_Response *r;
    if (!new_resps)
      return ENOMEM;
    out_op->responses = new_resps;
    r = &out_op->responses[out_op->n_responses++];
    memset(r, 0, sizeof(*r));
    r->code = c_cdd_strdup("200");
    if (!r->code)
      return ENOMEM;
    r->description = c_cdd_strdup("Success");
    if (!r->description)
      return ENOMEM;
  }

  /* 3. Global Tags */
  /* Heuristic: use first part of function name? e.g. api_pet_get -> "pet" */
  {
    /* Extract resource name if pattern matches prefix_Resource_... */
    if (ctx->func_name && out_op->n_tags == 0) {
      char *dup_name = c_cdd_strdup(ctx->func_name);
      char *token;
      char *ctx_ptr = NULL;
      /* assume snake case */
      token = strtok_r(dup_name, "_", &ctx_ptr); /* prefix */
      token = strtok_r(NULL, "_", &ctx_ptr);     /* resource or next */
      if (token) {
        out_op->tags = (char **)malloc(sizeof(char *));
        if (out_op->tags) {
          out_op->tags[0] = c_cdd_strdup(token);
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

  return 0;
}
