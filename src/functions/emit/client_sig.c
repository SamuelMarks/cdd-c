/**
 * @file codegen_client_sig.c
 * @brief Implementation of Client Signature Generation.
 *
 * Updated to support Grouped naming convention (Resource_Prefix_OpId).
 * Appends standard `struct ApiError **api_error` argument to all operations.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/client_sig.h"
#include "functions/parse/str.h"

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

static const char *map_type_to_c_arg(const char *oa_type) {
  if (!oa_type)
    return "const void *";
  if (strcmp(oa_type, "integer") == 0)
    return "int ";
  if (strcmp(oa_type, "string") == 0)
    return "const char *";
  if (strcmp(oa_type, "boolean") == 0)
    return "int ";
  if (strcmp(oa_type, "number") == 0)
    return "double ";
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
         p->in == OA_PARAM_IN_HEADER || p->in == OA_PARAM_IN_COOKIE;
}

static size_t media_type_base_len(const char *media_type) {
  size_t i = 0;
  if (!media_type)
    return 0;
  while (media_type[i] && media_type[i] != ';')
    ++i;
  return i;
}

static int media_type_has_prefix(const char *media_type, const char *prefix) {
  size_t i;
  size_t len;
  size_t pre_len;
  if (!media_type || !prefix)
    return 0;
  len = media_type_base_len(media_type);
  pre_len = strlen(prefix);
  if (len < pre_len)
    return 0;
  for (i = 0; i < pre_len; ++i) {
    char a = media_type[i];
    char b = prefix[i];
    if (a >= 'A' && a <= 'Z')
      a = (char)(a - 'A' + 'a');
    if (b >= 'A' && b <= 'Z')
      b = (char)(b - 'A' + 'a');
    if (a != b)
      return 0;
  }
  return 1;
}

static int media_type_has_suffix(const char *media_type, const char *suffix) {
  size_t i;
  size_t len;
  size_t suf_len;
  size_t start;
  if (!media_type || !suffix)
    return 0;
  len = media_type_base_len(media_type);
  suf_len = strlen(suffix);
  if (len < suf_len)
    return 0;
  start = len - suf_len;
  for (i = 0; i < suf_len; ++i) {
    char a = media_type[start + i];
    char b = suffix[i];
    if (a >= 'A' && a <= 'Z')
      a = (char)(a - 'A' + 'a');
    if (b >= 'A' && b <= 'Z')
      b = (char)(b - 'A' + 'a');
    if (a != b)
      return 0;
  }
  return 1;
}

static int media_type_ieq(const char *media_type, const char *expected) {
  size_t i;
  size_t len;
  size_t exp_len;
  if (!media_type || !expected)
    return 0;
  len = media_type_base_len(media_type);
  exp_len = strlen(expected);
  if (len != exp_len)
    return 0;
  for (i = 0; i < len; ++i) {
    char a = media_type[i];
    char b = expected[i];
    if (a >= 'A' && a <= 'Z')
      a = (char)(a - 'A' + 'a');
    if (b >= 'A' && b <= 'Z')
      b = (char)(b - 'A' + 'a');
    if (a != b)
      return 0;
  }
  return 1;
}

static int media_type_is_json(const char *media_type) {
  if (!media_type)
    return 0;
  if (media_type_ieq(media_type, "application/json"))
    return 1;
  return media_type_has_suffix(media_type, "+json");
}

static int media_type_is_form(const char *media_type) {
  return media_type_ieq(media_type, "application/x-www-form-urlencoded");
}

static int media_type_is_text_plain(const char *media_type) {
  return media_type_ieq(media_type, "text/plain");
}

static int media_type_is_multipart(const char *media_type) {
  return media_type_has_prefix(media_type, "multipart/");
}

static int media_type_is_multipart_form(const char *media_type) {
  return media_type_ieq(media_type, "multipart/form-data");
}

static const struct OpenAPI_MediaType *
find_media_type(const struct OpenAPI_MediaType *mts, size_t n,
                const char *name) {
  size_t i;
  if (!mts || !name)
    return NULL;
  for (i = 0; i < n; ++i) {
    if (mts[i].name && strcmp(mts[i].name, name) == 0)
      return &mts[i];
  }
  return NULL;
}

static int media_type_is_textual(const char *media_type) {
  if (!media_type)
    return 0;
  if (media_type_is_text_plain(media_type))
    return 1;
  if (media_type_has_prefix(media_type, "text/"))
    return 1;
  if (media_type_ieq(media_type, "application/xml"))
    return 1;
  if (media_type_has_suffix(media_type, "+xml"))
    return 1;
  return 0;
}

static int media_type_is_binary(const char *media_type) {
  if (!media_type)
    return 0;
  if (media_type_is_json(media_type))
    return 0;
  if (media_type_is_form(media_type))
    return 0;
  if (media_type_is_multipart(media_type))
    return 0;
  if (media_type_is_textual(media_type))
    return 0;
  return 1;
}

static int querystring_param_is_form_object(const struct OpenAPI_Parameter *p) {
  if (!p)
    return 0;
  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return 0;
  if (!media_type_is_form(p->content_type))
    return 0;
  if (p->schema.ref_name)
    return 1;
  if (p->schema.inline_type && strcmp(p->schema.inline_type, "object") == 0)
    return 1;
  if (p->type && strcmp(p->type, "object") == 0)
    return 1;
  return 0;
}

static int querystring_param_is_json_ref(const struct OpenAPI_Parameter *p) {
  if (!p)
    return 0;
  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return 0;
  if (!media_type_is_json(p->content_type))
    return 0;
  if (p->schema.is_array || (p->type && strcmp(p->type, "array") == 0))
    return 0;
  return p->schema.ref_name != NULL;
}

static const char *
querystring_param_json_primitive_type(const struct OpenAPI_Parameter *p) {
  const char *type = NULL;
  if (!p)
    return NULL;
  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return NULL;
  if (!media_type_is_json(p->content_type))
    return NULL;
  if (p->schema.is_array || (p->type && strcmp(p->type, "array") == 0))
    return NULL;
  if (p->schema.inline_type)
    type = p->schema.inline_type;
  else if (p->type)
    type = p->type;
  if (!type)
    return NULL;
  if (strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
      strcmp(type, "number") == 0 || strcmp(type, "boolean") == 0)
    return type;
  return NULL;
}

static const char *
querystring_param_json_array_item_type(const struct OpenAPI_Parameter *p) {
  const char *item_type = NULL;
  if (!p)
    return NULL;
  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return NULL;
  if (!media_type_is_json(p->content_type))
    return NULL;
  if (!(p->schema.is_array || (p->type && strcmp(p->type, "array") == 0) ||
        p->is_array))
    return NULL;
  if (p->schema.inline_type)
    item_type = p->schema.inline_type;
  else if (p->items_type)
    item_type = p->items_type;
  if (!item_type)
    return NULL;
  if (strcmp(item_type, "string") == 0 || strcmp(item_type, "integer") == 0 ||
      strcmp(item_type, "number") == 0 || strcmp(item_type, "boolean") == 0)
    return item_type;
  return NULL;
}

static const char *
querystring_param_json_array_item_ref(const struct OpenAPI_Parameter *p) {
  const char *item_type = NULL;
  if (!p)
    return NULL;
  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return NULL;
  if (!media_type_is_json(p->content_type))
    return NULL;
  if (!(p->schema.is_array || (p->type && strcmp(p->type, "array") == 0) ||
        p->is_array))
    return NULL;
  if (p->schema.inline_type)
    item_type = p->schema.inline_type;
  else if (p->items_type)
    item_type = p->items_type;
  if (!item_type)
    return NULL;
  if (strcmp(item_type, "string") == 0 || strcmp(item_type, "integer") == 0 ||
      strcmp(item_type, "number") == 0 || strcmp(item_type, "boolean") == 0)
    return NULL;
  if (strcmp(item_type, "object") == 0)
    return NULL;
  return item_type;
}

static const char *
querystring_param_raw_primitive_type(const struct OpenAPI_Parameter *p) {
  const char *type = NULL;
  if (!p)
    return NULL;
  if (p->in != OA_PARAM_IN_QUERYSTRING)
    return NULL;
  if (!p->content_type)
    return NULL;
  if (media_type_is_json(p->content_type))
    return NULL;
  if (media_type_is_form(p->content_type))
    return NULL;
  if (p->schema.inline_type)
    type = p->schema.inline_type;
  else if (p->type)
    type = p->type;
  if (!type)
    return "string";
  if (strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
      strcmp(type, "number") == 0 || strcmp(type, "boolean") == 0)
    return type;
  return "string";
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

static void sanitize_ident(char *out, size_t outsz, const char *in) {
  size_t i = 0;
  size_t j = 0;
  if (!out || outsz == 0)
    return;
  out[0] = '\0';
  if (!in)
    return;
  for (i = 0; in[i] && j + 1 < outsz; ++i) {
    const unsigned char c = (unsigned char)in[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9')) {
      out[j++] = (char)c;
    } else {
      out[j++] = '_';
    }
  }
  out[j] = '\0';
  if (j > 0 && out[0] >= '0' && out[0] <= '9') {
    if (j + 1 < outsz) {
      memmove(out + 1, out, j + 1);
      out[0] = '_';
    } else {
      out[0] = '_';
    }
  }
}

static void multipart_header_param_name(char *out, size_t outsz,
                                        const char *field, const char *header) {
  char hdr_sanitized[128];
  if (!out || outsz == 0) {
    return;
  }
  out[0] = '\0';
  if (!field || !header)
    return;
  sanitize_ident(hdr_sanitized, sizeof(hdr_sanitized), header);
  snprintf(out, outsz, "%s_hdr_%s", field, hdr_sanitized);
}

static int header_name_is_content_type(const char *name) {
  if (!name)
    return 0;
  return c_cdd_str_iequal(name, "Content-Type") != 0;
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

static int schema_has_payload(const struct OpenAPI_SchemaRef *schema) {
  if (!schema)
    return 0;
  if (schema->ref_name && schema->ref_name[0] != '\0')
    return 1;
  if (schema_has_inline(schema))
    return 1;
  if (schema->is_array)
    return 1;
  return 0;
}

static const struct OpenAPI_Response *
get_success_response(const struct OpenAPI_Operation *op) {
  const struct OpenAPI_Response *default_resp = NULL;
  size_t i;
  if (!op)
    return NULL;
  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *resp = &op->responses[i];
    const char *c = resp->code;
    if (!c)
      continue;
    if (strcmp(c, "default") == 0) {
      default_resp = resp;
      continue;
    }
    if (strlen(c) == 3 && c[0] == '2' && c[1] == 'X' && c[2] == 'X')
      return resp;
    if (c[0] == '2')
      return resp;
  }
  return default_resp;
}

static int response_is_binary_success(const struct OpenAPI_Operation *op) {
  const struct OpenAPI_Response *resp = get_success_response(op);
  if (!resp || !resp->content_type)
    return 0;
  if (!media_type_is_binary(resp->content_type))
    return 0;
  return 1;
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
  int success_is_binary;
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
    if (p->in == OA_PARAM_IN_QUERYSTRING) {
      const char *qs_json_item = querystring_param_json_array_item_type(p);
      const char *qs_json_obj = querystring_param_json_array_item_ref(p);
      const char *qs_json_prim = querystring_param_json_primitive_type(p);
      const char *qs_raw = querystring_param_raw_primitive_type(p);
      if (querystring_param_is_form_object(p)) {
        CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                         p->name, p->name));
      } else if (querystring_param_is_json_ref(p)) {
        CHECK_IO(
            fprintf(fp, ", const struct %s *%s", p->schema.ref_name, p->name));
      } else if (qs_json_obj) {
        CHECK_IO(fprintf(fp, ", const struct %s **%s, size_t %s_len",
                         qs_json_obj, p->name, p->name));
      } else if (qs_json_item) {
        const char *c_type = map_array_item_type(qs_json_item);
        CHECK_IO(
            fprintf(fp, ", %s%s, size_t %s_len", c_type, p->name, p->name));
      } else if (qs_json_prim) {
        const char *c_type = map_type_to_c_arg(qs_json_prim);
        CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
      } else if (qs_raw) {
        const char *c_type = map_type_to_c_arg(qs_raw);
        CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
      } else {
        CHECK_IO(fprintf(fp, ", const char *%s", p->name));
      }
      continue;
    }
    if (p->content_type && media_type_is_json(p->content_type)) {
      const char *ref_name = p->schema.ref_name;
      if (!ref_name && p->type && !is_primitive_type(p->type) &&
          strcmp(p->type, "object") != 0 && strcmp(p->type, "array") != 0) {
        ref_name = p->type;
      }
      if (p->is_array) {
        const char *item_type =
            p->items_type ? p->items_type : p->schema.inline_type;
        if (item_type && is_primitive_type(item_type)) {
          const char *c_type = map_array_item_type(item_type);
          CHECK_IO(
              fprintf(fp, ", %s%s, size_t %s_len", c_type, p->name, p->name));
        } else if (item_type && strcmp(item_type, "object") != 0) {
          CHECK_IO(fprintf(fp, ", const struct %s **%s, size_t %s_len",
                           item_type, p->name, p->name));
        } else {
          CHECK_IO(
              fprintf(fp, ", const void *%s, size_t %s_len", p->name, p->name));
        }
      } else if (ref_name) {
        CHECK_IO(fprintf(fp, ", const struct %s *%s", ref_name, p->name));
      } else if (p->type && strcmp(p->type, "object") == 0) {
        CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                         p->name, p->name));
      } else {
        const char *prim = p->type ? p->type : p->schema.inline_type;
        const char *c_type = map_type_to_c_arg(prim ? prim : "string");
        CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
      }
      continue;
    }
    if (param_is_object_kv(p)) {
      CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                       p->name, p->name));
    } else if (p->is_array) {
      /* Emit pointer + length */
      const char *c_type = map_array_item_type(p->items_type);
      CHECK_IO(fprintf(fp, ", %s%s, size_t %s_len", c_type, p->name, p->name));
    } else {
      const char *c_type = map_type_to_c_arg(p->type);
      CHECK_IO(fprintf(fp, ", %s%s", c_type, p->name));
    }
  }

  /* 2. Request Body */
  if (op->req_body.content_type &&
      (media_type_is_binary(op->req_body.content_type) ||
       (media_type_is_multipart(op->req_body.content_type) &&
        !media_type_is_multipart_form(op->req_body.content_type)))) {
    CHECK_IO(fprintf(fp, ", const unsigned char *body, size_t body_len"));
  } else if (op->req_body.content_type &&
             media_type_is_textual(op->req_body.content_type)) {
    CHECK_IO(fprintf(fp, ", const char *req_body"));
  } else if (op->req_body.content_type && op->req_body.ref_name) {
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
      CHECK_IO(fprintf(fp, ", %sbody, size_t body_len", c_type));
    } else {
      const char *c_type = map_type_to_c_arg(op->req_body.inline_type);
      CHECK_IO(fprintf(fp, ", %sreq_body", c_type));
    }
  }

  /* 2b. Multipart per-part encoding headers */
  if (op->req_body.content_type &&
      media_type_is_multipart_form(op->req_body.content_type)) {
    const struct OpenAPI_MediaType *mt =
        find_media_type(op->req_body_media_types, op->n_req_body_media_types,
                        "multipart/form-data");
    if (mt && mt->encoding && mt->n_encoding > 0) {
      size_t e;
      for (e = 0; e < mt->n_encoding; ++e) {
        const struct OpenAPI_Encoding *enc = &mt->encoding[e];
        size_t h;
        if (!enc->name || !enc->headers || enc->n_headers == 0)
          continue;
        for (h = 0; h < enc->n_headers; ++h) {
          const struct OpenAPI_Header *hdr = &enc->headers[h];
          const char *hdr_type = hdr->type ? hdr->type : "string";
          int hdr_is_array = hdr->is_array || ((hdr_type != NULL) &&
                                               strcmp(hdr_type, "array") == 0);
          char param_name[256];
          if (!hdr->name || header_name_is_content_type(hdr->name))
            continue;
          multipart_header_param_name(param_name, sizeof(param_name), enc->name,
                                      hdr->name);
          if (param_name[0] == '\0')
            continue;
          if (hdr_is_array) {
            const char *item_type =
                hdr->items_type ? hdr->items_type : "string";
            const char *c_type = map_array_item_type(item_type);
            CHECK_IO(fprintf(fp, ", %s%s, size_t %s_len", c_type, param_name,
                             param_name));
          } else if (strcmp(hdr_type, "object") == 0) {
            CHECK_IO(fprintf(fp, ", const struct OpenAPI_KV *%s, size_t %s_len",
                             param_name, param_name));
          } else {
            const char *c_type = map_type_to_c_arg(hdr_type);
            CHECK_IO(fprintf(fp, ", %s%s", c_type, param_name));
          }
        }
      }
    }
  }

  /* 3. Success Output */
  success_is_binary = response_is_binary_success(op);
  success_schema = get_success_schema(op);
  if (success_is_binary)
    success_schema = NULL;
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
        CHECK_IO(fprintf(fp, ", %sout, size_t *out_len", out_type));
      }
    } else if (success_schema->ref_name) {
      CHECK_IO(fprintf(fp, ", struct %s **out", success_schema->ref_name));
    } else if (success_schema->inline_type) {
      const char *out_type = map_type_to_c_out(success_schema->inline_type);
      CHECK_IO(fprintf(fp, ", %sout", out_type));
    }
  } else if (success_is_binary) {
    CHECK_IO(fprintf(fp, ", unsigned char **out, size_t *out_len"));
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
