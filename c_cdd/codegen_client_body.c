/**
 * @file codegen_client_body.c
 * @brief Implementation of the Request Orchestrator.
 *
 * Generates C implementation for API client functions including retry logic
 * and standardized error parsing via `ApiError`.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "codegen_client_body.h"
#include "codegen_security.h"
#include "codegen_struct.h"
#include "codegen_url.h"
#include "openapi_loader.h"
#include "str_utils.h"

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

static const char *verb_to_enum_str(enum OpenAPI_Verb v) {
  switch (v) {
  case OA_VERB_GET:
    return "HTTP_GET";
  case OA_VERB_POST:
    return "HTTP_POST";
  case OA_VERB_PUT:
    return "HTTP_PUT";
  case OA_VERB_DELETE:
    return "HTTP_DELETE";
  case OA_VERB_HEAD:
    return "HTTP_HEAD";
  case OA_VERB_PATCH:
    return "HTTP_PATCH";
  case OA_VERB_OPTIONS:
    return "HTTP_OPTIONS";
  case OA_VERB_TRACE:
    return "HTTP_TRACE";
  case OA_VERB_QUERY:
    return "HTTP_QUERY";
  default:
    return "HTTP_GET";
  }
}

static const char *method_str_to_enum_str(const char *method) {
  if (!method)
    return NULL;
  if (c_cdd_str_iequal(method, "get"))
    return "HTTP_GET";
  if (c_cdd_str_iequal(method, "post"))
    return "HTTP_POST";
  if (c_cdd_str_iequal(method, "put"))
    return "HTTP_PUT";
  if (c_cdd_str_iequal(method, "delete"))
    return "HTTP_DELETE";
  if (c_cdd_str_iequal(method, "patch"))
    return "HTTP_PATCH";
  if (c_cdd_str_iequal(method, "head"))
    return "HTTP_HEAD";
  if (c_cdd_str_iequal(method, "options"))
    return "HTTP_OPTIONS";
  if (c_cdd_str_iequal(method, "trace"))
    return "HTTP_TRACE";
  if (c_cdd_str_iequal(method, "query"))
    return "HTTP_QUERY";
  if (c_cdd_str_iequal(method, "connect"))
    return "HTTP_CONNECT";
  return NULL;
}

static int mapped_err_code(int status) {
  if (status == 400)
    return 22; /* EINVAL */
  if (status == 401 || status == 403)
    return 13; /* EACCES */
  if (status == 404)
    return 2; /* ENOENT */
  return 5;   /* EIO generic */
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

static const struct OpenAPI_Encoding *
find_encoding(const struct OpenAPI_MediaType *mt, const char *name) {
  size_t i;
  if (!mt || !name || !mt->encoding)
    return NULL;
  for (i = 0; i < mt->n_encoding; ++i) {
    if (mt->encoding[i].name && strcmp(mt->encoding[i].name, name) == 0) {
      return &mt->encoding[i];
    }
  }
  return NULL;
}

static int schema_has_inline(const struct OpenAPI_SchemaRef *schema) {
  if (!schema)
    return 0;
  return schema->inline_type != NULL;
}

static int schema_has_payload(const struct OpenAPI_SchemaRef *schema) {
  if (!schema)
    return 0;
  if (schema->ref_name)
    return 1;
  if (schema_has_inline(schema))
    return 1;
  return 0;
}

static int write_inline_json_parse(FILE *fp,
                                   const struct OpenAPI_SchemaRef *schema) {
  const char *type;
  if (!fp || !schema || !schema->inline_type)
    return EINVAL;

  type = schema->inline_type;

  if (schema->is_array) {
    CHECK_IO(fprintf(fp, "      if (res->body && out && out_len) {\n"));
    CHECK_IO(fprintf(fp, "        JSON_Value *val = json_parse_string((const "
                         "char*)res->body);\n"));
    CHECK_IO(fprintf(fp, "        JSON_Array *arr = NULL;\n"));
    CHECK_IO(fprintf(fp, "        size_t count = 0;\n"));
    CHECK_IO(fprintf(fp, "        if (!val) { rc = EINVAL; }\n"));
    CHECK_IO(fprintf(fp, "        if (rc == 0) {\n"));
    CHECK_IO(fprintf(fp, "          arr = json_value_get_array(val);\n"));
    CHECK_IO(fprintf(fp, "          if (!arr) rc = EINVAL;\n"));
    CHECK_IO(fprintf(fp, "        }\n"));
    CHECK_IO(fprintf(fp, "        if (rc == 0) {\n"));
    CHECK_IO(fprintf(fp, "          count = json_array_get_count(arr);\n"));
    CHECK_IO(fprintf(fp, "          *out_len = count;\n"));
    CHECK_IO(fprintf(fp, "          if (count == 0) {\n"));
    CHECK_IO(fprintf(fp, "            *out = NULL;\n"));
    CHECK_IO(fprintf(fp, "          } else {\n"));
    if (strcmp(type, "string") == 0) {
      CHECK_IO(fprintf(fp, "            char **tmp = (char **)calloc(count, "
                           "sizeof(char *));\n"));
      CHECK_IO(fprintf(fp, "            size_t i;\n"));
      CHECK_IO(fprintf(fp, "            if (!tmp) { rc = ENOMEM; }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == 0) {\n"));
      CHECK_IO(fprintf(fp, "              for (i = 0; i < count; ++i) {\n"));
      CHECK_IO(fprintf(fp, "                const char *s = "
                           "json_array_get_string(arr, i);\n"));
      CHECK_IO(
          fprintf(fp, "                if (!s) { rc = EINVAL; break; }\n"));
      CHECK_IO(fprintf(fp, "                tmp[i] = strdup(s);\n"));
      CHECK_IO(fprintf(fp, "                if (!tmp[i]) { rc = ENOMEM; break; "
                           "}\n"));
      CHECK_IO(fprintf(fp, "              }\n"));
      CHECK_IO(fprintf(fp, "            }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == 0) {\n"));
      CHECK_IO(fprintf(fp, "              *out = tmp;\n"));
      CHECK_IO(fprintf(fp, "            } else if (tmp) {\n"));
      CHECK_IO(fprintf(fp, "              size_t k;\n"));
      CHECK_IO(fprintf(fp, "              for (k = 0; k < count; ++k) "
                           "free(tmp[k]);\n"));
      CHECK_IO(fprintf(fp, "              free(tmp);\n"));
      CHECK_IO(fprintf(fp, "            }\n"));
    } else if (strcmp(type, "integer") == 0) {
      CHECK_IO(fprintf(fp, "            int *tmp = (int *)calloc(count, "
                           "sizeof(int));\n"));
      CHECK_IO(fprintf(fp, "            size_t i;\n"));
      CHECK_IO(fprintf(fp, "            if (!tmp) { rc = ENOMEM; }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == 0) {\n"));
      CHECK_IO(fprintf(fp, "              for (i = 0; i < count; ++i) {\n"));
      CHECK_IO(fprintf(fp,
                       "                if "
                       "(json_array_get_value(arr, i) && "
                       "json_value_get_type(json_array_get_value(arr, i)) != "
                       "JSONNumber) { rc = EINVAL; break; }\n"));
      CHECK_IO(fprintf(fp,
                       "                tmp[i] = (int)json_array_get_number("
                       "arr, i);\n"));
      CHECK_IO(fprintf(fp, "              }\n"));
      CHECK_IO(fprintf(fp, "            }\n"));
      CHECK_IO(fprintf(
          fp, "            if (rc == 0) *out = tmp; else free(tmp);\n"));
    } else if (strcmp(type, "number") == 0) {
      CHECK_IO(fprintf(fp, "            double *tmp = (double *)calloc(count, "
                           "sizeof(double));\n"));
      CHECK_IO(fprintf(fp, "            size_t i;\n"));
      CHECK_IO(fprintf(fp, "            if (!tmp) { rc = ENOMEM; }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == 0) {\n"));
      CHECK_IO(fprintf(fp, "              for (i = 0; i < count; ++i) {\n"));
      CHECK_IO(fprintf(fp,
                       "                if "
                       "(json_array_get_value(arr, i) && "
                       "json_value_get_type(json_array_get_value(arr, i)) != "
                       "JSONNumber) { rc = EINVAL; break; }\n"));
      CHECK_IO(fprintf(fp,
                       "                tmp[i] = json_array_get_number(arr, "
                       "i);\n"));
      CHECK_IO(fprintf(fp, "              }\n"));
      CHECK_IO(fprintf(fp, "            }\n"));
      CHECK_IO(fprintf(
          fp, "            if (rc == 0) *out = tmp; else free(tmp);\n"));
    } else if (strcmp(type, "boolean") == 0) {
      CHECK_IO(fprintf(fp, "            int *tmp = (int *)calloc(count, "
                           "sizeof(int));\n"));
      CHECK_IO(fprintf(fp, "            size_t i;\n"));
      CHECK_IO(fprintf(fp, "            if (!tmp) { rc = ENOMEM; }\n"));
      CHECK_IO(fprintf(fp, "            if (rc == 0) {\n"));
      CHECK_IO(fprintf(fp, "              for (i = 0; i < count; ++i) {\n"));
      CHECK_IO(fprintf(fp,
                       "                if "
                       "(json_array_get_value(arr, i) && "
                       "json_value_get_type(json_array_get_value(arr, i)) != "
                       "JSONBoolean) { rc = EINVAL; break; }\n"));
      CHECK_IO(fprintf(fp,
                       "                tmp[i] = json_array_get_boolean(arr, "
                       "i) ? 1 : 0;\n"));
      CHECK_IO(fprintf(fp, "              }\n"));
      CHECK_IO(fprintf(fp, "            }\n"));
      CHECK_IO(fprintf(
          fp, "            if (rc == 0) *out = tmp; else free(tmp);\n"));
    } else {
      CHECK_IO(fprintf(fp, "            rc = EINVAL;\n"));
    }
    CHECK_IO(fprintf(fp, "          }\n"));
    CHECK_IO(fprintf(fp, "        }\n"));
    CHECK_IO(fprintf(fp, "        if (val) json_value_free(val);\n"));
    CHECK_IO(fprintf(fp, "      }\n"));
  } else {
    CHECK_IO(fprintf(fp, "      if (res->body && out) {\n"));
    CHECK_IO(fprintf(fp, "        JSON_Value *val = json_parse_string((const "
                         "char*)res->body);\n"));
    CHECK_IO(fprintf(fp, "        if (!val) { rc = EINVAL; }\n"));
    CHECK_IO(fprintf(fp, "        if (rc == 0) {\n"));
    if (strcmp(type, "string") == 0) {
      CHECK_IO(fprintf(
          fp, "          const char *s = json_value_get_string(val);\n"));
      CHECK_IO(fprintf(fp, "          if (!s) { rc = EINVAL; }\n"));
      CHECK_IO(fprintf(fp, "          if (rc == 0) {\n"));
      CHECK_IO(fprintf(fp, "            *out = strdup(s);\n"));
      CHECK_IO(fprintf(fp, "            if (!*out) rc = ENOMEM;\n"));
      CHECK_IO(fprintf(fp, "          }\n"));
    } else if (strcmp(type, "integer") == 0) {
      CHECK_IO(fprintf(fp,
                       "          if (json_value_get_type(val) != JSONNumber) "
                       "{ rc = EINVAL; }\n"));
      CHECK_IO(fprintf(fp, "          if (rc == 0) *out = "
                           "(int)json_value_get_number(val);\n"));
    } else if (strcmp(type, "number") == 0) {
      CHECK_IO(fprintf(fp,
                       "          if (json_value_get_type(val) != JSONNumber) "
                       "{ rc = EINVAL; }\n"));
      CHECK_IO(fprintf(fp, "          if (rc == 0) *out = "
                           "json_value_get_number(val);\n"));
    } else if (strcmp(type, "boolean") == 0) {
      CHECK_IO(fprintf(fp,
                       "          if (json_value_get_type(val) != JSONBoolean) "
                       "{ rc = EINVAL; }\n"));
      CHECK_IO(fprintf(fp, "          if (rc == 0) *out = "
                           "json_value_get_boolean(val) ? 1 : 0;\n"));
    } else {
      CHECK_IO(fprintf(fp, "          rc = EINVAL;\n"));
    }
    CHECK_IO(fprintf(fp, "        }\n"));
    CHECK_IO(fprintf(fp, "        if (val) json_value_free(val);\n"));
    CHECK_IO(fprintf(fp, "      }\n"));
  }

  return 0;
}

static int write_joined_form_array(FILE *fp, const char *field,
                                   const char *len_field,
                                   const char *items_type, char delim,
                                   const char *encode_fn, int add_encoded) {
  const int do_encode = (encode_fn && encode_fn[0] != '\0');

  if (!fp || !field || !len_field)
    return EINVAL;

  CHECK_IO(fprintf(fp, "  {\n"));
  CHECK_IO(fprintf(fp, "    size_t i;\n"));
  CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
  CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
  CHECK_IO(fprintf(fp, "    for(i=0; i < req_body->%s; ++i) {\n", len_field));

  if (items_type && strcmp(items_type, "integer") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
    CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", req_body->%s[i]);\n",
                     field));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (items_type && strcmp(items_type, "number") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%g\", req_body->%s[i]);\n",
                     field));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (items_type && strcmp(items_type, "boolean") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(
        fp, "      raw = req_body->%s[i] ? \"true\" : \"false\";\n", field));
  } else {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = req_body->%s[i];\n", field));
  }

  if (do_encode) {
    CHECK_IO(fprintf(fp, "      char *enc = %s(raw);\n", encode_fn));
    CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
    CHECK_IO(fprintf(fp, "      if (!enc) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
    CHECK_IO(fprintf(
        fp,
        "      {\n"
        "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
        "        char *tmp = (char *)realloc(joined, joined_len + extra + 1);\n"
        "        if (!tmp) { free(enc); rc = ENOMEM; goto cleanup; }\n"
        "        joined = tmp;\n"
        "        if (i > 0) joined[joined_len++] = '%c';\n"
        "        memcpy(joined + joined_len, enc, val_len);\n"
        "        joined_len += val_len;\n"
        "        joined[joined_len] = '\\0';\n"
        "      }\n",
        delim));
    CHECK_IO(fprintf(fp, "      free(enc);\n"));
  } else {
    CHECK_IO(fprintf(fp, "      size_t val_len = strlen(raw);\n"));
    CHECK_IO(fprintf(
        fp,
        "      {\n"
        "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
        "        char *tmp = (char *)realloc(joined, joined_len + extra + 1);\n"
        "        if (!tmp) { rc = ENOMEM; goto cleanup; }\n"
        "        joined = tmp;\n"
        "        if (i > 0) joined[joined_len++] = '%c';\n"
        "        memcpy(joined + joined_len, raw, val_len);\n"
        "        joined_len += val_len;\n"
        "        joined[joined_len] = '\\0';\n"
        "      }\n",
        delim));
  }

  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "    if (joined) {\n"));
  if (add_encoded) {
    CHECK_IO(fprintf(
        fp, "      rc = url_query_add_encoded(&form_qp, \"%s\", joined);\n",
        field));
  } else {
    CHECK_IO(fprintf(
        fp, "      rc = url_query_add(&form_qp, \"%s\", joined);\n", field));
  }
  CHECK_IO(fprintf(fp, "      free(joined);\n"));
  CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "  }\n"));

  return 0;
}

static int write_header_param_logic(FILE *fp,
                                    const struct OpenAPI_Operation *op) {
  size_t i;
  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_HEADER) {
      const struct OpenAPI_Parameter *p = &op->parameters[i];
      CHECK_IO(fprintf(fp, "  /* Header Parameter: %s */\n", p->name));
      if (p->is_array) {
        const char *item_type = p->items_type ? p->items_type : "string";
        CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
        CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
        CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
        CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));
        if (strcmp(item_type, "integer") == 0) {
          CHECK_IO(fprintf(fp, "      const char *raw;\n"));
          CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
          CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", %s[i]);\n",
                           p->name));
          CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
        } else if (strcmp(item_type, "number") == 0) {
          CHECK_IO(fprintf(fp, "      const char *raw;\n"));
          CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
          CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%g\", %s[i]);\n",
                           p->name));
          CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
        } else if (strcmp(item_type, "boolean") == 0) {
          CHECK_IO(fprintf(fp, "      const char *raw;\n"));
          CHECK_IO(fprintf(fp, "      raw = %s[i] ? \"true\" : \"false\";\n",
                           p->name));
        } else {
          CHECK_IO(fprintf(fp, "      const char *raw;\n"));
          CHECK_IO(fprintf(fp, "      raw = %s[i];\n", p->name));
        }
        CHECK_IO(fprintf(fp, "      if (raw) {\n"));
        CHECK_IO(fprintf(fp, "        size_t val_len = strlen(raw);\n"));
        CHECK_IO(fprintf(
            fp,
            "        size_t extra = val_len + (joined_len > 0 ? 1 : 0);\n"));
        CHECK_IO(fprintf(fp, "        char *tmp = (char *)realloc(joined, "
                             "joined_len + extra + 1);\n"));
        CHECK_IO(
            fprintf(fp, "        if (!tmp) { rc = ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "        joined = tmp;\n"));
        CHECK_IO(fprintf(
            fp, "        if (joined_len > 0) joined[joined_len++] = ',';\n"));
        CHECK_IO(fprintf(
            fp, "        memcpy(joined + joined_len, raw, val_len);\n"));
        CHECK_IO(fprintf(fp, "        joined_len += val_len;\n"));
        CHECK_IO(fprintf(fp, "        joined[joined_len] = '\\0';\n"));
        CHECK_IO(fprintf(fp, "      }\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
        CHECK_IO(fprintf(fp, "    if (joined) {\n"));
        CHECK_IO(fprintf(
            fp, "      rc = http_headers_add(&req.headers, \"%s\", joined);\n",
            p->name));
        CHECK_IO(fprintf(fp, "      free(joined);\n"));
        CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "object") == 0) {
        int explode = p->explode_set ? p->explode : 0;
        CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
        CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
        CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
        CHECK_IO(fprintf(fp, "    int first = 1;\n"));
        CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));
        CHECK_IO(fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n",
                         p->name));
        CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
        CHECK_IO(fprintf(fp, "      const char *kv_raw = NULL;\n"));
        CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
        CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
        CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"));
        CHECK_IO(
            fprintf(fp, "        kv_raw = kv->value.s;\n        break;\n"));
        CHECK_IO(fprintf(fp, "      case OA_KV_INTEGER:\n"
                             "        sprintf(num_buf, \"%%d\", kv->value.i);\n"
                             "        kv_raw = num_buf;\n"
                             "        break;\n"));
        CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                             "        sprintf(num_buf, \"%%g\", kv->value.n);\n"
                             "        kv_raw = num_buf;\n"
                             "        break;\n"));
        CHECK_IO(
            fprintf(fp, "      case OA_KV_BOOLEAN:\n"
                        "        kv_raw = kv->value.b ? \"true\" : \"false\";\n"
                        "        break;\n"));
        CHECK_IO(fprintf(fp, "      default:\n"
                             "        kv_raw = NULL;\n"
                             "        break;\n"));
        CHECK_IO(fprintf(fp, "      }\n"));
        CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
        CHECK_IO(fprintf(fp, "      {\n"));
        if (explode) {
          CHECK_IO(fprintf(
              fp, "        size_t key_len = strlen(kv_key);\n"
                  "        size_t val_len = strlen(kv_raw);\n"
                  "        size_t extra = key_len + val_len + 1 + (first ? 0 : "
                  "1);\n"
                  "        char *tmp = (char *)realloc(joined, joined_len + "
                  "extra + 1);\n"
                  "        if (!tmp) { rc = ENOMEM; goto cleanup; }\n"
                  "        joined = tmp;\n"
                  "        if (!first) joined[joined_len++] = ',';\n"
                  "        memcpy(joined + joined_len, kv_key, key_len);\n"
                  "        joined_len += key_len;\n"
                  "        joined[joined_len++] = '=';\n"
                  "        memcpy(joined + joined_len, kv_raw, val_len);\n"
                  "        joined_len += val_len;\n"
                  "        joined[joined_len] = '\\0';\n"));
        } else {
          CHECK_IO(fprintf(
              fp, "        size_t key_len = strlen(kv_key);\n"
                  "        size_t val_len = strlen(kv_raw);\n"
                  "        size_t extra = key_len + val_len + 1 + (first ? 0 : "
                  "1) + 1;\n"
                  "        char *tmp = (char *)realloc(joined, joined_len + "
                  "extra + 1);\n"
                  "        if (!tmp) { rc = ENOMEM; goto cleanup; }\n"
                  "        joined = tmp;\n"
                  "        if (!first) joined[joined_len++] = ',';\n"
                  "        memcpy(joined + joined_len, kv_key, key_len);\n"
                  "        joined_len += key_len;\n"
                  "        joined[joined_len++] = ',';\n"
                  "        memcpy(joined + joined_len, kv_raw, val_len);\n"
                  "        joined_len += val_len;\n"
                  "        joined[joined_len] = '\\0';\n"));
        }
        CHECK_IO(fprintf(fp, "      }\n"));
        CHECK_IO(fprintf(fp, "      first = 0;\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
        CHECK_IO(fprintf(fp, "    if (joined) {\n"));
        CHECK_IO(fprintf(
            fp, "      rc = http_headers_add(&req.headers, \"%s\", joined);\n",
            p->name));
        CHECK_IO(fprintf(fp, "      free(joined);\n"));
        CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "string") == 0) {
        CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));
        CHECK_IO(fprintf(
            fp, "    rc = http_headers_add(&req.headers, \"%s\", %s);\n",
            p->name, p->name));
        CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "integer") == 0) {
        CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
        CHECK_IO(fprintf(fp, "    sprintf(num_buf, \"%%d\", %s);\n", p->name));
        CHECK_IO(fprintf(
            fp, "    rc = http_headers_add(&req.headers, \"%s\", num_buf);\n",
            p->name));
        CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
      } else if (strcmp(p->type, "number") == 0) {
        CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
        CHECK_IO(fprintf(fp, "    sprintf(num_buf, \"%%g\", %s);\n", p->name));
        CHECK_IO(fprintf(
            fp, "    rc = http_headers_add(&req.headers, \"%s\", num_buf);\n",
            p->name));
        CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
      } else if (strcmp(p->type, "boolean") == 0) {
        CHECK_IO(fprintf(fp,
                         "  rc = http_headers_add(&req.headers, \"%s\", %s ? "
                         "\"true\" : \"false\");\n",
                         p->name, p->name));
        CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
      }
    }
  }
  return 0;
}

static int write_form_urlencoded_body(FILE *fp,
                                      const struct OpenAPI_Operation *op,
                                      const struct OpenAPI_Spec *spec) {
  const struct OpenAPI_MediaType *mt;
  const struct StructFields *sf;
  size_t i;

  if (!fp || !op || !spec)
    return EINVAL;

  mt = find_media_type(op->req_body_media_types, op->n_req_body_media_types,
                       "application/x-www-form-urlencoded");

  sf = openapi_spec_find_schema(spec, op->req_body.ref_name);
  if (!sf) {
    CHECK_IO(fprintf(
        fp,
        "  /* Warning: Schema %s definition not found, skipping form body */\n",
        op->req_body.ref_name));
    return 0;
  }

  CHECK_IO(fprintf(fp, "  /* Form URL-Encoded Body Construction */\n"));
  CHECK_IO(fprintf(fp, "  rc = url_query_init(&form_qp);\n"));
  CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    const struct OpenAPI_Encoding *enc = find_encoding(mt, f->name);
    enum OpenAPI_Style style =
        (enc && enc->style_set) ? enc->style : OA_STYLE_FORM;
    int explode = (enc && enc->explode_set) ? enc->explode
                                            : (style == OA_STYLE_FORM ? 1 : 0);
    int allow_reserved =
        (enc && enc->allow_reserved_set) ? enc->allow_reserved : 0;

    if (strcmp(f->type, "array") == 0) {
      const char *items_type = f->ref ? f->ref : "string";
      char len_field[80];
      const char *encode_fn = NULL;
      int add_encoded = 0;

      sprintf(len_field, "n_%s", f->name);

      if (strcmp(items_type, "string") == 0 && allow_reserved) {
        encode_fn = "url_encode_form_allow_reserved";
      }

      if (style == OA_STYLE_FORM && explode) {
        CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
        CHECK_IO(
            fprintf(fp, "    for(i=0; i < req_body->%s; ++i) {\n", len_field));
        if (strcmp(items_type, "string") == 0) {
          if (allow_reserved) {
            CHECK_IO(fprintf(fp,
                             "      char *enc = url_encode_form_allow_reserved"
                             "(req_body->%s[i]);\n",
                             f->name));
            CHECK_IO(fprintf(
                fp, "      if (!enc) { rc = ENOMEM; goto cleanup; }\n"));
            CHECK_IO(fprintf(
                fp,
                "      rc = url_query_add_encoded(&form_qp, \"%s\", enc);\n",
                f->name));
            CHECK_IO(fprintf(fp, "      free(enc);\n"));
          } else {
            CHECK_IO(fprintf(fp,
                             "      rc = url_query_add(&form_qp, \"%s\", "
                             "req_body->%s[i]);\n",
                             f->name, f->name));
          }
        } else if (strcmp(items_type, "integer") == 0) {
          CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
          CHECK_IO(
              fprintf(fp, "      sprintf(num_buf, \"%%d\", req_body->%s[i]);\n",
                      f->name));
          CHECK_IO(fprintf(
              fp, "      rc = url_query_add(&form_qp, \"%s\", num_buf);\n",
              f->name));
        } else if (strcmp(items_type, "number") == 0) {
          CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
          CHECK_IO(
              fprintf(fp, "      sprintf(num_buf, \"%%g\", req_body->%s[i]);\n",
                      f->name));
          CHECK_IO(fprintf(
              fp, "      rc = url_query_add(&form_qp, \"%s\", num_buf);\n",
              f->name));
        } else if (strcmp(items_type, "boolean") == 0) {
          CHECK_IO(fprintf(fp,
                           "      rc = url_query_add(&form_qp, \"%s\", "
                           "req_body->%s[i] ? \"true\" : \"false\");\n",
                           f->name, f->name));
        } else {
          CHECK_IO(fprintf(
              fp, "      /* Unsupported array item type for %s */\n", f->name));
        }
        CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n    }\n  }\n"));
      } else if (style == OA_STYLE_FORM && !explode) {
        add_encoded = 1;
        if (write_joined_form_array(fp, f->name, len_field, items_type, ',',
                                    encode_fn ? encode_fn : "url_encode_form",
                                    add_encoded) != 0)
          return EIO;
      } else if (style == OA_STYLE_SPACE_DELIMITED) {
        if (write_joined_form_array(fp, f->name, len_field, items_type, ' ',
                                    NULL, 0) != 0)
          return EIO;
      } else if (style == OA_STYLE_PIPE_DELIMITED) {
        if (write_joined_form_array(fp, f->name, len_field, items_type, '|',
                                    NULL, 0) != 0)
          return EIO;
      } else {
        CHECK_IO(fprintf(
            fp, "  /* Array style not supported for %s in form body */\n",
            f->name));
      }
      continue;
    }

    if (strcmp(f->type, "string") == 0) {
      CHECK_IO(fprintf(fp, "  if (req_body->%s) {\n", f->name));
      if (allow_reserved) {
        CHECK_IO(fprintf(
            fp,
            "    char *enc = url_encode_form_allow_reserved(req_body->%s);\n",
            f->name));
        CHECK_IO(fprintf(fp, "    if (!enc) { rc = ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(
            fp, "    rc = url_query_add_encoded(&form_qp, \"%s\", enc);\n",
            f->name));
        CHECK_IO(fprintf(fp, "    free(enc);\n"));
      } else {
        CHECK_IO(fprintf(
            fp, "    rc = url_query_add(&form_qp, \"%s\", req_body->%s);\n",
            f->name, f->name));
      }
      CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
    } else if (strcmp(f->type, "integer") == 0) {
      CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
      CHECK_IO(fprintf(fp, "    sprintf(num_buf, \"%%d\", req_body->%s);\n",
                       f->name));
      CHECK_IO(fprintf(
          fp, "    rc = url_query_add(&form_qp, \"%s\", num_buf);\n", f->name));
      CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
    } else if (strcmp(f->type, "number") == 0) {
      CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
      CHECK_IO(fprintf(fp, "    sprintf(num_buf, \"%%g\", req_body->%s);\n",
                       f->name));
      CHECK_IO(fprintf(
          fp, "    rc = url_query_add(&form_qp, \"%s\", num_buf);\n", f->name));
      CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
    } else if (strcmp(f->type, "boolean") == 0) {
      CHECK_IO(fprintf(fp,
                       "  rc = url_query_add(&form_qp, \"%s\", req_body->%s ? "
                       "\"true\" : \"false\");\n",
                       f->name, f->name));
      CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
    } else {
      CHECK_IO(
          fprintf(fp, "  /* Unsupported form field type for %s */\n", f->name));
    }
  }

  CHECK_IO(fprintf(fp, "  rc = url_query_build_form(&form_qp, &form_body);\n"));
  CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "  req.body = form_body;\n"));
  CHECK_IO(fprintf(fp, "  req.body_len = strlen(form_body);\n"));
  CHECK_IO(fprintf(
      fp, "  http_headers_add(&req.headers, "
          "\"Content-Type\", \"application/x-www-form-urlencoded\");\n\n"));

  return 0;
}

static int write_cookie_param_logic(FILE *fp,
                                    const struct OpenAPI_Operation *op) {
  size_t i;
  int has_cookie = 0;

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_COOKIE) {
      has_cookie = 1;
      break;
    }
  }

  if (!has_cookie)
    return 0;

  CHECK_IO(fprintf(fp, "  /* Cookie Parameters */\n"));

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in != OA_PARAM_IN_COOKIE)
      continue;

    {
      const struct OpenAPI_Parameter *p = &op->parameters[i];
      const char *item_type = p->items_type ? p->items_type : "string";
      enum OpenAPI_Style style =
          (p->style == OA_STYLE_UNKNOWN) ? OA_STYLE_FORM : p->style;
      int explode =
          p->explode_set
              ? p->explode
              : ((style == OA_STYLE_FORM || style == OA_STYLE_COOKIE) ? 1 : 0);

      CHECK_IO(fprintf(fp, "  /* Cookie Parameter: %s */\n", p->name));

      if (p->is_array) {
        if (explode) {
          CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
          CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));
          if (strcmp(item_type, "integer") == 0) {
            CHECK_IO(fprintf(fp, "      const char *cookie_val;\n"));
            CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
            CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", %s[i]);\n",
                             p->name));
            CHECK_IO(fprintf(fp, "      cookie_val = num_buf;\n"));
          } else if (strcmp(item_type, "number") == 0) {
            CHECK_IO(fprintf(fp, "      const char *cookie_val;\n"));
            CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
            CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%g\", %s[i]);\n",
                             p->name));
            CHECK_IO(fprintf(fp, "      cookie_val = num_buf;\n"));
          } else if (strcmp(item_type, "boolean") == 0) {
            CHECK_IO(fprintf(fp, "      const char *cookie_val;\n"));
            CHECK_IO(fprintf(
                fp, "      cookie_val = %s[i] ? \"true\" : \"false\";\n",
                p->name));
          } else {
            CHECK_IO(fprintf(fp, "      const char *cookie_val;\n"));
            CHECK_IO(fprintf(fp, "      cookie_val = %s[i];\n", p->name));
          }
          CHECK_IO(fprintf(fp, "      if (cookie_val) {\n"));
          CHECK_IO(fprintf(fp, "        size_t name_len = strlen(\"%s\");\n",
                           p->name));
          CHECK_IO(
              fprintf(fp, "        size_t val_len = strlen(cookie_val);\n"));
          CHECK_IO(fprintf(fp,
                           "        size_t extra = name_len + 1 + val_len + "
                           "(cookie_len ? 2 : 0);\n"));
          CHECK_IO(fprintf(fp,
                           "        char *tmp = (char *)realloc(cookie_str, "
                           "cookie_len + extra + 1);\n"));
          CHECK_IO(fprintf(
              fp, "        if (!tmp) { rc = ENOMEM; goto cleanup; }\n"));
          CHECK_IO(fprintf(fp, "        cookie_str = tmp;\n"));
          CHECK_IO(fprintf(
              fp, "        if (cookie_len) { cookie_str[cookie_len++] = "
                  "';'; cookie_str[cookie_len++] = ' '; }\n"));
          CHECK_IO(fprintf(fp,
                           "        memcpy(cookie_str + cookie_len, \"%s\", "
                           "name_len);\n",
                           p->name));
          CHECK_IO(fprintf(fp, "        cookie_len += name_len;\n"));
          CHECK_IO(fprintf(fp, "        cookie_str[cookie_len++] = '=';\n"));
          CHECK_IO(
              fprintf(fp, "        memcpy(cookie_str + cookie_len, cookie_val, "
                          "val_len);\n"));
          CHECK_IO(fprintf(fp, "        cookie_len += val_len;\n"));
          CHECK_IO(fprintf(fp, "        cookie_str[cookie_len] = '\\0';\n"));
          CHECK_IO(fprintf(fp, "      }\n"));
          CHECK_IO(fprintf(fp, "    }\n  }\n"));
        } else {
          CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
          CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
          CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
          CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));
          if (strcmp(item_type, "integer") == 0) {
            CHECK_IO(fprintf(fp, "      const char *raw;\n"));
            CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
            CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", %s[i]);\n",
                             p->name));
            CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
          } else if (strcmp(item_type, "number") == 0) {
            CHECK_IO(fprintf(fp, "      const char *raw;\n"));
            CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
            CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%g\", %s[i]);\n",
                             p->name));
            CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
          } else if (strcmp(item_type, "boolean") == 0) {
            CHECK_IO(fprintf(fp, "      const char *raw;\n"));
            CHECK_IO(fprintf(fp, "      raw = %s[i] ? \"true\" : \"false\";\n",
                             p->name));
          } else {
            CHECK_IO(fprintf(fp, "      const char *raw;\n"));
            CHECK_IO(fprintf(fp, "      raw = %s[i];\n", p->name));
          }
          CHECK_IO(fprintf(fp, "      if (raw) {\n"));
          CHECK_IO(fprintf(fp, "        size_t val_len = strlen(raw);\n"));
          CHECK_IO(fprintf(
              fp,
              "        size_t extra = val_len + (joined_len > 0 ? 1 : 0);\n"));
          CHECK_IO(fprintf(fp, "        char *tmp = (char *)realloc(joined, "
                               "joined_len + extra + 1);\n"));
          CHECK_IO(fprintf(
              fp, "        if (!tmp) { rc = ENOMEM; goto cleanup; }\n"));
          CHECK_IO(fprintf(fp, "        joined = tmp;\n"));
          CHECK_IO(fprintf(
              fp, "        if (joined_len > 0) joined[joined_len++] = ',';\n"));
          CHECK_IO(fprintf(
              fp, "        memcpy(joined + joined_len, raw, val_len);\n"));
          CHECK_IO(fprintf(fp, "        joined_len += val_len;\n"));
          CHECK_IO(fprintf(fp, "        joined[joined_len] = '\\0';\n"));
          CHECK_IO(fprintf(fp, "      }\n"));
          CHECK_IO(fprintf(fp, "    }\n"));
          CHECK_IO(fprintf(fp, "    if (joined) {\n"));
          CHECK_IO(fprintf(fp, "      size_t name_len = strlen(\"%s\");\n",
                           p->name));
          CHECK_IO(fprintf(fp, "      size_t val_len = strlen(joined);\n"));
          CHECK_IO(fprintf(fp, "      size_t extra = name_len + 1 + val_len + "
                               "(cookie_len ? 2 : 0);\n"));
          CHECK_IO(fprintf(fp, "      char *tmp = (char *)realloc(cookie_str, "
                               "cookie_len + extra + 1);\n"));
          CHECK_IO(fprintf(fp,
                           "      if (!tmp) { free(joined); rc = ENOMEM; goto "
                           "cleanup; }\n"));
          CHECK_IO(fprintf(fp, "      cookie_str = tmp;\n"));
          CHECK_IO(fprintf(fp,
                           "      if (cookie_len) { cookie_str[cookie_len++] = "
                           "';'; cookie_str[cookie_len++] = ' '; }\n"));
          CHECK_IO(fprintf(fp,
                           "      memcpy(cookie_str + cookie_len, \"%s\", "
                           "name_len);\n",
                           p->name));
          CHECK_IO(fprintf(fp, "      cookie_len += name_len;\n"));
          CHECK_IO(fprintf(fp, "      cookie_str[cookie_len++] = '=';\n"));
          CHECK_IO(fprintf(fp, "      memcpy(cookie_str + cookie_len, joined, "
                               "val_len);\n"));
          CHECK_IO(fprintf(fp, "      cookie_len += val_len;\n"));
          CHECK_IO(fprintf(fp, "      cookie_str[cookie_len] = '\\0';\n"));
          CHECK_IO(fprintf(fp, "      free(joined);\n"));
          CHECK_IO(fprintf(fp, "    }\n  }\n"));
        }
      } else if (strcmp(p->type, "string") == 0) {
        CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));
        CHECK_IO(fprintf(fp, "    const char *cookie_val = %s;\n", p->name));
        CHECK_IO(
            fprintf(fp, "    size_t name_len = strlen(\"%s\");\n", p->name));
        CHECK_IO(fprintf(fp, "    size_t val_len = strlen(cookie_val);\n"));
        CHECK_IO(fprintf(fp, "    size_t extra = name_len + 1 + val_len + "
                             "(cookie_len ? 2 : 0);\n"));
        CHECK_IO(fprintf(fp, "    char *tmp = (char *)realloc(cookie_str, "
                             "cookie_len + extra + 1);\n"));
        CHECK_IO(fprintf(fp, "    if (!tmp) { rc = ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "    cookie_str = tmp;\n"));
        CHECK_IO(fprintf(fp, "    if (cookie_len) { cookie_str[cookie_len++] = "
                             "';'; cookie_str[cookie_len++] = ' '; }\n"));
        CHECK_IO(fprintf(
            fp, "    memcpy(cookie_str + cookie_len, \"%s\", name_len);\n",
            p->name));
        CHECK_IO(fprintf(fp, "    cookie_len += name_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len++] = '=';\n"));
        CHECK_IO(fprintf(
            fp, "    memcpy(cookie_str + cookie_len, cookie_val, val_len);\n"));
        CHECK_IO(fprintf(fp, "    cookie_len += val_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len] = '\\0';\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "integer") == 0) {
        CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
        CHECK_IO(fprintf(fp, "    sprintf(num_buf, \"%%d\", %s);\n", p->name));
        CHECK_IO(
            fprintf(fp, "    size_t name_len = strlen(\"%s\");\n", p->name));
        CHECK_IO(fprintf(fp, "    size_t val_len = strlen(num_buf);\n"));
        CHECK_IO(fprintf(fp, "    size_t extra = name_len + 1 + val_len + "
                             "(cookie_len ? 2 : 0);\n"));
        CHECK_IO(fprintf(fp, "    char *tmp = (char *)realloc(cookie_str, "
                             "cookie_len + extra + 1);\n"));
        CHECK_IO(fprintf(fp, "    if (!tmp) { rc = ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "    cookie_str = tmp;\n"));
        CHECK_IO(fprintf(fp, "    if (cookie_len) { cookie_str[cookie_len++] = "
                             "';'; cookie_str[cookie_len++] = ' '; }\n"));
        CHECK_IO(fprintf(
            fp, "    memcpy(cookie_str + cookie_len, \"%s\", name_len);\n",
            p->name));
        CHECK_IO(fprintf(fp, "    cookie_len += name_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len++] = '=';\n"));
        CHECK_IO(fprintf(
            fp, "    memcpy(cookie_str + cookie_len, num_buf, val_len);\n"));
        CHECK_IO(fprintf(fp, "    cookie_len += val_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len] = '\\0';\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "number") == 0) {
        CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
        CHECK_IO(fprintf(fp, "    sprintf(num_buf, \"%%g\", %s);\n", p->name));
        CHECK_IO(
            fprintf(fp, "    size_t name_len = strlen(\"%s\");\n", p->name));
        CHECK_IO(fprintf(fp, "    size_t val_len = strlen(num_buf);\n"));
        CHECK_IO(fprintf(fp, "    size_t extra = name_len + 1 + val_len + "
                             "(cookie_len ? 2 : 0);\n"));
        CHECK_IO(fprintf(fp, "    char *tmp = (char *)realloc(cookie_str, "
                             "cookie_len + extra + 1);\n"));
        CHECK_IO(fprintf(fp, "    if (!tmp) { rc = ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "    cookie_str = tmp;\n"));
        CHECK_IO(fprintf(fp, "    if (cookie_len) { cookie_str[cookie_len++] = "
                             "';'; cookie_str[cookie_len++] = ' '; }\n"));
        CHECK_IO(fprintf(
            fp, "    memcpy(cookie_str + cookie_len, \"%s\", name_len);\n",
            p->name));
        CHECK_IO(fprintf(fp, "    cookie_len += name_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len++] = '=';\n"));
        CHECK_IO(fprintf(
            fp, "    memcpy(cookie_str + cookie_len, num_buf, val_len);\n"));
        CHECK_IO(fprintf(fp, "    cookie_len += val_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len] = '\\0';\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      } else if (strcmp(p->type, "boolean") == 0) {
        CHECK_IO(fprintf(
            fp,
            "  {\n    const char *cookie_val = %s ? \"true\" : \"false\";\n",
            p->name));
        CHECK_IO(
            fprintf(fp, "    size_t name_len = strlen(\"%s\");\n", p->name));
        CHECK_IO(fprintf(fp, "    size_t val_len = strlen(cookie_val);\n"));
        CHECK_IO(fprintf(fp, "    size_t extra = name_len + 1 + val_len + "
                             "(cookie_len ? 2 : 0);\n"));
        CHECK_IO(fprintf(fp, "    char *tmp = (char *)realloc(cookie_str, "
                             "cookie_len + extra + 1);\n"));
        CHECK_IO(fprintf(fp, "    if (!tmp) { rc = ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "    cookie_str = tmp;\n"));
        CHECK_IO(fprintf(fp, "    if (cookie_len) { cookie_str[cookie_len++] = "
                             "';'; cookie_str[cookie_len++] = ' '; }\n"));
        CHECK_IO(fprintf(
            fp, "    memcpy(cookie_str + cookie_len, \"%s\", name_len);\n",
            p->name));
        CHECK_IO(fprintf(fp, "    cookie_len += name_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len++] = '=';\n"));
        CHECK_IO(fprintf(
            fp, "    memcpy(cookie_str + cookie_len, cookie_val, val_len);\n"));
        CHECK_IO(fprintf(fp, "    cookie_len += val_len;\n"));
        CHECK_IO(fprintf(fp, "    cookie_str[cookie_len] = '\\0';\n"));
        CHECK_IO(fprintf(fp, "  }\n"));
      }
    }
  }

  CHECK_IO(fprintf(fp, "  if (cookie_str) {\n"));
  CHECK_IO(fprintf(
      fp,
      "    rc = http_headers_add(&req.headers, \"Cookie\", cookie_str);\n"));
  CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "  }\n"));

  return 0;
}

static int write_multipart_body(FILE *fp, const struct OpenAPI_Operation *op,
                                const struct OpenAPI_Spec *spec) {
  const struct StructFields *sf;
  size_t i;

  sf = openapi_spec_find_schema(spec, op->req_body.ref_name);
  if (!sf) {
    CHECK_IO(fprintf(
        fp,
        "  /* Warning: Schema %s definition not found, skipping multipart */\n",
        op->req_body.ref_name));
    return 0;
  }

  CHECK_IO(fprintf(fp, "  /* Multipart Body Construction */\n"));
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    if (strcmp(f->type, "array") == 0) {
      const char *items_type = f->ref ? f->ref : "string";
      char len_field[80];
      sprintf(len_field, "n_%s", f->name);

      CHECK_IO(fprintf(fp, "  if (req_body->%s) {\n", f->name));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      if (strcmp(items_type, "string") == 0) {
        CHECK_IO(fprintf(fp, "    for (i = 0; i < req_body->%s; ++i) {\n",
                         len_field));
        CHECK_IO(
            fprintf(fp, "      const char *val = req_body->%s[i];\n", f->name));
        CHECK_IO(fprintf(fp, "      if (!val) continue;\n"));
        CHECK_IO(fprintf(fp,
                         "      rc = http_request_add_part(&req, \"%s\", "
                         "NULL, NULL, val, strlen(val));\n",
                         f->name));
        CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
      } else if (strcmp(items_type, "integer") == 0) {
        CHECK_IO(fprintf(fp, "    for (i = 0; i < req_body->%s; ++i) {\n",
                         len_field));
        CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
        CHECK_IO(fprintf(fp,
                         "      sprintf(num_buf, \"%%d\", "
                         "req_body->%s[i]);\n",
                         f->name));
        CHECK_IO(fprintf(fp,
                         "      rc = http_request_add_part(&req, \"%s\", "
                         "NULL, NULL, num_buf, strlen(num_buf));\n",
                         f->name));
        CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
      } else if (strcmp(items_type, "number") == 0) {
        CHECK_IO(fprintf(fp, "    for (i = 0; i < req_body->%s; ++i) {\n",
                         len_field));
        CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
        CHECK_IO(fprintf(fp,
                         "      sprintf(num_buf, \"%%g\", "
                         "req_body->%s[i]);\n",
                         f->name));
        CHECK_IO(fprintf(fp,
                         "      rc = http_request_add_part(&req, \"%s\", "
                         "NULL, NULL, num_buf, strlen(num_buf));\n",
                         f->name));
        CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
      } else if (strcmp(items_type, "boolean") == 0) {
        CHECK_IO(fprintf(fp, "    for (i = 0; i < req_body->%s; ++i) {\n",
                         len_field));
        CHECK_IO(fprintf(fp,
                         "      const char *val = req_body->%s[i] ? "
                         "\"true\" : \"false\";\n",
                         f->name));
        CHECK_IO(fprintf(fp,
                         "      rc = http_request_add_part(&req, \"%s\", "
                         "NULL, NULL, val, strlen(val));\n",
                         f->name));
        CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
      } else {
        CHECK_IO(fprintf(
            fp, "    /* Unsupported array item type for %s in multipart */\n",
            f->name));
      }
      CHECK_IO(fprintf(fp, "  }\n"));
    } else if (strcmp(f->type, "string") == 0) {
      CHECK_IO(fprintf(fp, "    if (req_body->%s) {\n", f->name));
      CHECK_IO(fprintf(fp,
                       "      rc = http_request_add_part(&req, \"%s\", NULL, "
                       "NULL, req_body->%s, strlen(req_body->%s));\n",
                       f->name, f->name, f->name));
      CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
    } else if (strcmp(f->type, "integer") == 0) {
      CHECK_IO(fprintf(fp, "    {\n      char num_buf[32];\n"));
      CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", req_body->%s);\n",
                       f->name));
      CHECK_IO(fprintf(fp,
                       "      rc = http_request_add_part(&req, \"%s\", NULL, "
                       "NULL, num_buf, strlen(num_buf));\n",
                       f->name));
      CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n    }\n"));
    } else if (strcmp(f->type, "number") == 0) {
      CHECK_IO(fprintf(fp, "    {\n      char num_buf[64];\n"));
      CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%g\", req_body->%s);\n",
                       f->name));
      CHECK_IO(fprintf(fp,
                       "      rc = http_request_add_part(&req, \"%s\", NULL, "
                       "NULL, num_buf, strlen(num_buf));\n",
                       f->name));
      CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n    }\n"));
    } else if (strcmp(f->type, "boolean") == 0) {
      CHECK_IO(fprintf(fp,
                       "    {\n      const char *val = req_body->%s ? "
                       "\"true\" : \"false\";\n",
                       f->name));
      CHECK_IO(fprintf(fp,
                       "      rc = http_request_add_part(&req, \"%s\", NULL, "
                       "NULL, val, strlen(val));\n",
                       f->name));
      CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n    }\n"));
    }
  }
  CHECK_IO(fprintf(fp, "  rc = http_request_flatten_parts(&req);\n"));
  CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n\n"));
  return 0;
}

static int is_status_range_code(const char *code) {
  if (!code)
    return 0;
  return strlen(code) == 3 && code[0] >= '1' && code[0] <= '5' &&
         code[1] == 'X' && code[2] == 'X';
}

static int status_range_prefix(const char *code) {
  if (!is_status_range_code(code))
    return 0;
  return code[0] - '0';
}

static int is_status_code_literal(const char *code) {
  if (!code || strlen(code) != 3)
    return 0;
  return code[0] >= '0' && code[0] <= '9' && code[1] >= '0' && code[1] <= '9' &&
         code[2] >= '0' && code[2] <= '9';
}

int codegen_client_write_body(FILE *const fp,
                              const struct OpenAPI_Operation *const op,
                              const struct OpenAPI_Spec *const spec,
                              const char *const path_template) {
  int query_exists = 0;
  int cookie_exists = 0;
  int has_querystring = 0;
  int security_query = 0;
  int security_cookie = 0;
  size_t i;
  struct CodegenUrlConfig url_cfg;
  const struct OpenAPI_Response *default_resp = NULL;
  const struct OpenAPI_Response *range_resp[6] = {0};
  int has_range = 0;
  int has_success = 0;
  const char *success_schema_name = NULL;
  const char *success_inline_type = NULL;
  int success_inline_is_array = 0;

  if (!fp || !op || !path_template)
    return EINVAL;

  if (spec) {
    security_query = codegen_security_requires_query(op, spec);
    security_cookie = codegen_security_requires_cookie(op, spec);
  }

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_QUERY ||
        op->parameters[i].in == OA_PARAM_IN_QUERYSTRING) {
      query_exists = 1;
      if (op->parameters[i].in == OA_PARAM_IN_QUERYSTRING)
        has_querystring = 1;
    }
    if (op->parameters[i].in == OA_PARAM_IN_COOKIE)
      cookie_exists = 1;
  }
  if (has_querystring && security_query)
    security_query = 0;
  if (security_query)
    query_exists = 1;
  if (security_cookie)
    cookie_exists = 1;

  /* --- 1. Declarations --- */
  CHECK_IO(fprintf(fp, "  struct HttpRequest req;\n"));
  CHECK_IO(fprintf(fp, "  struct HttpResponse *res = NULL;\n"));
  CHECK_IO(fprintf(fp, "  int rc = 0;\n"));
  CHECK_IO(fprintf(fp, "  int attempt = 0;\n"));

  if (query_exists) {
    CHECK_IO(fprintf(fp, "  struct UrlQueryParams qp = {0};\n"));
    CHECK_IO(fprintf(fp, "  char *query_str = NULL;\n"));
    CHECK_IO(fprintf(fp, "  char *path_str = NULL;\n"));
    CHECK_IO(fprintf(fp, "  int qp_initialized = 0;\n"));
  } else {
    CHECK_IO(fprintf(fp, "  char *url = NULL;\n"));
  }
  if (cookie_exists) {
    CHECK_IO(fprintf(fp, "  char *cookie_str = NULL;\n"));
    CHECK_IO(fprintf(fp, "  size_t cookie_len = 0;\n"));
  }

  if (op->req_body.content_type &&
      strcmp(op->req_body.content_type, "application/json") == 0 &&
      (op->req_body.ref_name || schema_has_inline(&op->req_body))) {
    CHECK_IO(fprintf(fp, "  char *req_json = NULL;\n"));
  }
  if (op->req_body.ref_name && op->req_body.content_type &&
      strcmp(op->req_body.content_type, "application/x-www-form-urlencoded") ==
          0) {
    CHECK_IO(fprintf(fp, "  struct UrlQueryParams form_qp;\n"));
    CHECK_IO(fprintf(fp, "  char *form_body = NULL;\n"));
  }

  /* Ensure ApiError out is initialized */
  CHECK_IO(fprintf(fp, "  if (api_error) *api_error = NULL;\n\n"));

  /* --- 2. Init & Security --- */
  CHECK_IO(fprintf(fp, "  if (!ctx || !ctx->send) return EINVAL;\n"));
  CHECK_IO(fprintf(fp, "  rc = http_request_init(&req);\n"));
  CHECK_IO(fprintf(fp, "  if (rc != 0) return rc;\n\n"));

  if (spec) {
    if (codegen_security_write_apply(fp, op, spec) != 0)
      return EIO;
  }

  /* --- 3. Header Param Logic --- */
  if (write_header_param_logic(fp, op) != 0)
    return EIO;

  /* --- 4. Cookie Param Logic --- */
  if (write_cookie_param_logic(fp, op) != 0)
    return EIO;

  /* --- 5. Query Param Logic --- */
  if (codegen_url_write_query_params(fp, op, query_exists ? 1 : 0) != 0)
    return EIO;

  /* --- 6. Body Serialization --- */
  if (op->req_body.ref_name) {
    if (op->req_body.content_type) {
      if (strcmp(op->req_body.content_type, "multipart/form-data") == 0) {
        if (write_multipart_body(fp, op, spec) != 0)
          return EIO;
      } else if (strcmp(op->req_body.content_type,
                        "application/x-www-form-urlencoded") == 0) {
        if (write_form_urlencoded_body(fp, op, spec) != 0)
          return EIO;
      } else if (strcmp(op->req_body.content_type, "application/json") == 0) {
        CHECK_IO(fprintf(fp, "  rc = %s_to_json(req_body, &req_json);\n",
                         op->req_body.ref_name));
        CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
        CHECK_IO(fprintf(fp, "  req.body = req_json;\n"));
        CHECK_IO(fprintf(fp, "  req.body_len = strlen(req_json);\n"));
        CHECK_IO(fprintf(fp, "  http_headers_add(&req.headers, "
                             "\"Content-Type\", \"application/json\");\n\n"));
      }
    }
  } else if (op->req_body.content_type &&
             strcmp(op->req_body.content_type, "application/json") == 0 &&
             schema_has_inline(&op->req_body)) {
    CHECK_IO(fprintf(fp, "  {\n"));
    CHECK_IO(fprintf(fp, "    JSON_Value *req_val = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *tmp_json = NULL;\n"));
    if (op->req_body.is_array) {
      CHECK_IO(fprintf(fp, "    JSON_Array *req_arr = NULL;\n"));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    req_val = json_value_init_array();\n"));
      CHECK_IO(
          fprintf(fp, "    if (!req_val) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    req_arr = json_value_get_array(req_val);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!req_arr) { rc = EINVAL; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    for (i = 0; i < body_len; ++i) {\n"));
      if (op->req_body.inline_type &&
          strcmp(op->req_body.inline_type, "string") == 0) {
        CHECK_IO(fprintf(fp, "      if (!body[i]) {\n"));
        CHECK_IO(fprintf(fp, "        if (json_array_append_null(req_arr) != "
                             "JSONSuccess) { rc = ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "      } else {\n"));
        CHECK_IO(fprintf(fp, "        if (json_array_append_string(req_arr, "
                             "body[i]) != JSONSuccess) { rc = ENOMEM; goto "
                             "cleanup; }\n"));
        CHECK_IO(fprintf(fp, "      }\n"));
      } else if (op->req_body.inline_type &&
                 strcmp(op->req_body.inline_type, "integer") == 0) {
        CHECK_IO(fprintf(fp, "      if (json_array_append_number(req_arr, "
                             "(double)body[i]) != JSONSuccess) { rc = ENOMEM; "
                             "goto cleanup; }\n"));
      } else if (op->req_body.inline_type &&
                 strcmp(op->req_body.inline_type, "number") == 0) {
        CHECK_IO(fprintf(fp, "      if (json_array_append_number(req_arr, "
                             "body[i]) != JSONSuccess) { rc = ENOMEM; goto "
                             "cleanup; }\n"));
      } else if (op->req_body.inline_type &&
                 strcmp(op->req_body.inline_type, "boolean") == 0) {
        CHECK_IO(fprintf(fp, "      if (json_array_append_boolean(req_arr, "
                             "body[i] ? 1 : 0) != JSONSuccess) { rc = ENOMEM; "
                             "goto cleanup; }\n"));
      } else {
        CHECK_IO(fprintf(fp, "      rc = EINVAL; goto cleanup;\n"));
      }
      CHECK_IO(fprintf(fp, "    }\n"));
    } else if (op->req_body.inline_type &&
               strcmp(op->req_body.inline_type, "string") == 0) {
      CHECK_IO(
          fprintf(fp, "    if (!req_body) { rc = EINVAL; goto cleanup; }\n"));
      CHECK_IO(
          fprintf(fp, "    req_val = json_value_init_string(req_body);\n"));
    } else if (op->req_body.inline_type &&
               strcmp(op->req_body.inline_type, "integer") == 0) {
      CHECK_IO(fprintf(fp, "    req_val = json_value_init_number((double)"
                           "req_body);\n"));
    } else if (op->req_body.inline_type &&
               strcmp(op->req_body.inline_type, "number") == 0) {
      CHECK_IO(
          fprintf(fp, "    req_val = json_value_init_number(req_body);\n"));
    } else if (op->req_body.inline_type &&
               strcmp(op->req_body.inline_type, "boolean") == 0) {
      CHECK_IO(fprintf(fp,
                       "    req_val = json_value_init_boolean(req_body ? 1 : "
                       "0);\n"));
    } else {
      CHECK_IO(fprintf(fp, "    rc = EINVAL; goto cleanup;\n"));
    }
    CHECK_IO(fprintf(fp, "    if (!req_val) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(
        fprintf(fp, "    tmp_json = json_serialize_to_string(req_val);\n"));
    CHECK_IO(fprintf(fp, "    if (!tmp_json) { json_value_free(req_val); rc = "
                         "ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    req_json = strdup(tmp_json);\n"));
    CHECK_IO(fprintf(fp, "    json_free_serialized_string(tmp_json);\n"));
    CHECK_IO(fprintf(fp, "    json_value_free(req_val);\n"));
    CHECK_IO(
        fprintf(fp, "    if (!req_json) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    req.body = req_json;\n"));
    CHECK_IO(fprintf(fp, "    req.body_len = strlen(req_json);\n"));
    CHECK_IO(fprintf(fp, "    http_headers_add(&req.headers, "
                         "\"Content-Type\", \"application/json\");\n"));
    CHECK_IO(fprintf(fp, "  }\n\n"));
  }

  /* --- 7. URL Construction --- */
  memset(&url_cfg, 0, sizeof(url_cfg));
  url_cfg.out_variable = query_exists ? "path_str" : "url";
  url_cfg.base_variable = "ctx->base_url";

  if (codegen_url_write_builder(fp, path_template, op->parameters,
                                op->n_parameters, &url_cfg) != 0) {
    return EIO;
  }

  if (query_exists) {
    CHECK_IO(fprintf(fp, "  if (asprintf(&req.url, \"%%s%%s\", path_str, "
                         "query_str) == -1) { rc = ENOMEM; goto cleanup; }\n"));
  } else {
    CHECK_IO(fprintf(fp, "  req.url = url;\n"));
  }

  {
    const char *method_enum = verb_to_enum_str(op->verb);
    if (op->is_additional) {
      const char *mapped = method_str_to_enum_str(op->method);
      if (mapped) {
        method_enum = mapped;
      } else if (op->method && op->method[0] != '\0') {
        CHECK_IO(fprintf(fp,
                         "  /* Warning: unsupported HTTP method '%s', "
                         "defaulting to GET */\n",
                         op->method));
        method_enum = "HTTP_GET";
      }
    }
    CHECK_IO(fprintf(fp, "  req.method = %s;\n\n", method_enum));
  }

  /* --- 8. Send with Retry Logic --- */
  CHECK_IO(fprintf(fp, "  do {\n"));
  CHECK_IO(fprintf(fp, "    if(attempt > 0) {\n"));
  CHECK_IO(fprintf(fp, "      /* Implement backoff delay here if needed */\n"));
  CHECK_IO(fprintf(fp, "    }\n"));

  CHECK_IO(fprintf(fp, "    rc = ctx->send(ctx->transport, &req, &res);\n"));
  CHECK_IO(fprintf(fp, "    attempt++;\n"));
  CHECK_IO(fprintf(
      fp, "  } while (rc != 0 && attempt <= ctx->config.retry_count);\n\n"));

  CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "  if (!res) { rc = EIO; goto cleanup; }\n\n"));

  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *resp = &op->responses[i];
    if (!resp->code)
      continue;
    if (strcmp(resp->code, "default") == 0) {
      default_resp = resp;
      continue;
    }
    if (is_status_range_code(resp->code)) {
      int bucket = status_range_prefix(resp->code);
      if (bucket >= 1 && bucket <= 5) {
        range_resp[bucket] = resp;
        has_range = 1;
        if (bucket == 2)
          has_success = 1;
        if (bucket == 2) {
          if (!success_schema_name && resp->schema.ref_name)
            success_schema_name = resp->schema.ref_name;
          if (!success_inline_type && schema_has_inline(&resp->schema)) {
            success_inline_type = resp->schema.inline_type;
            success_inline_is_array = resp->schema.is_array ? 1 : 0;
          }
        }
      }
      continue;
    }
    if (resp->code[0] == '2') {
      has_success = 1;
      if (!success_schema_name && resp->schema.ref_name)
        success_schema_name = resp->schema.ref_name;
      if (!success_inline_type && schema_has_inline(&resp->schema)) {
        success_inline_type = resp->schema.inline_type;
        success_inline_is_array = resp->schema.is_array ? 1 : 0;
      }
    }
  }
  if (!success_schema_name && !success_inline_type && default_resp &&
      schema_has_payload(&default_resp->schema) && !has_success) {
    if (default_resp->schema.ref_name) {
      success_schema_name = default_resp->schema.ref_name;
    } else if (schema_has_inline(&default_resp->schema)) {
      success_inline_type = default_resp->schema.inline_type;
      success_inline_is_array = default_resp->schema.is_array ? 1 : 0;
    }
  }

  /* --- 9. Responses --- */
  CHECK_IO(fprintf(fp, "  int handled = 0;\n"));
  CHECK_IO(fprintf(fp, "  switch (res->status_code) {\n"));
  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *resp = &op->responses[i];
    if (!resp->code)
      continue;
    if (strcmp(resp->code, "default") == 0)
      continue;
    if (is_status_range_code(resp->code))
      continue;
    if (!is_status_code_literal(resp->code))
      continue;
    CHECK_IO(fprintf(fp, "    case %s:\n", resp->code));
    CHECK_IO(fprintf(fp, "      handled = 1;\n"));
    if (resp->code[0] == '2') {
      if (resp->schema.ref_name) {
        CHECK_IO(fprintf(fp, "      if (res->body && out) {\n"));
        CHECK_IO(fprintf(
            fp, "        rc = %s_from_json((const char*)res->body, out%s);\n",
            resp->schema.ref_name, resp->schema.is_array ? ", out_len" : ""));
        CHECK_IO(fprintf(fp, "      }\n"));
      } else if (schema_has_inline(&resp->schema)) {
        if (write_inline_json_parse(fp, &resp->schema) != 0)
          return EIO;
      }
      CHECK_IO(fprintf(fp, "      break;\n"));
    } else {
      CHECK_IO(
          fprintf(fp, "      rc = %d;\n", mapped_err_code(atoi(resp->code))));
      CHECK_IO(fprintf(fp, "      if (res->body && api_error) {\n"));
      CHECK_IO(fprintf(
          fp,
          "        ApiError_from_json((const char*)res->body, api_error);\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
      CHECK_IO(fprintf(fp, "      break;\n"));
    }
  }
  CHECK_IO(fprintf(fp, "    default:\n"));
  CHECK_IO(fprintf(fp, "      break;\n"));
  CHECK_IO(fprintf(fp, "  }\n"));

  if (has_range) {
    CHECK_IO(fprintf(fp, "  if (!handled) {\n"));
    for (i = 1; i <= 5; ++i) {
      const struct OpenAPI_Response *resp = range_resp[i];
      if (!resp)
        continue;
      if (i == 2) {
        CHECK_IO(fprintf(fp, "    if (res->status_code >= 200 && "
                             "res->status_code < 300) {\n"));
        CHECK_IO(fprintf(fp, "      handled = 1;\n"));
        if (resp->schema.ref_name) {
          CHECK_IO(fprintf(fp, "      if (res->body && out) {\n"));
          CHECK_IO(fprintf(
              fp, "        rc = %s_from_json((const char*)res->body, out%s);\n",
              resp->schema.ref_name, resp->schema.is_array ? ", out_len" : ""));
          CHECK_IO(fprintf(fp, "      }\n"));
        } else if (schema_has_inline(&resp->schema)) {
          if (write_inline_json_parse(fp, &resp->schema) != 0)
            return EIO;
        }
        CHECK_IO(fprintf(fp, "    }\n"));
      } else {
        CHECK_IO(fprintf(fp,
                         "    if (res->status_code >= %d && "
                         "res->status_code < %d) {\n",
                         i * 100, (i + 1) * 100));
        CHECK_IO(fprintf(fp, "      handled = 1;\n"));
        CHECK_IO(fprintf(fp, "      rc = %d;\n", mapped_err_code(i * 100)));
        CHECK_IO(fprintf(fp, "      if (res->body && api_error) {\n"));
        CHECK_IO(fprintf(fp, "        ApiError_from_json((const "
                             "char*)res->body, api_error);\n"));
        CHECK_IO(fprintf(fp, "      }\n"));
        CHECK_IO(fprintf(fp, "    }\n"));
      }
    }
    CHECK_IO(fprintf(fp, "  }\n"));
  }

  CHECK_IO(fprintf(fp, "  if (!handled) {\n"));
  if (default_resp) {
    int default_is_success =
        (!has_success && schema_has_payload(&default_resp->schema));
    int default_matches_success = 0;
    if (success_schema_name && default_resp->schema.ref_name &&
        strcmp(success_schema_name, default_resp->schema.ref_name) == 0) {
      default_matches_success = 1;
    }
    if (success_inline_type && schema_has_inline(&default_resp->schema) &&
        default_resp->schema.inline_type &&
        strcmp(success_inline_type, default_resp->schema.inline_type) == 0 &&
        success_inline_is_array == (default_resp->schema.is_array ? 1 : 0)) {
      default_matches_success = 1;
    }
    CHECK_IO(fprintf(fp, "    /* default response */\n"));
    if (default_is_success || default_matches_success) {
      if (default_resp->schema.ref_name) {
        CHECK_IO(fprintf(fp, "    if (res->body && out) {\n"));
        CHECK_IO(fprintf(
            fp, "      rc = %s_from_json((const char*)res->body, out%s);\n",
            default_resp->schema.ref_name,
            default_resp->schema.is_array ? ", out_len" : ""));
        CHECK_IO(fprintf(fp, "    }\n"));
      } else if (schema_has_inline(&default_resp->schema)) {
        if (write_inline_json_parse(fp, &default_resp->schema) != 0)
          return EIO;
      }
    } else {
      CHECK_IO(fprintf(fp, "    rc = EIO;\n"));
      CHECK_IO(fprintf(fp, "    if (res->body && api_error) {\n"));
      CHECK_IO(fprintf(
          fp,
          "      ApiError_from_json((const char*)res->body, api_error);\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
    }
  } else {
    CHECK_IO(fprintf(fp, "    rc = EIO;\n"));
    CHECK_IO(fprintf(fp, "    if (res->body && api_error) {\n"));
    CHECK_IO(fprintf(
        fp, "      ApiError_from_json((const char*)res->body, api_error);\n"));
    CHECK_IO(fprintf(fp, "    }\n"));
  }
  CHECK_IO(fprintf(fp, "  }\n\n"));

  /* --- 10. Cleanup --- */
  CHECK_IO(fprintf(fp, "cleanup:\n"));
  if (op->req_body.content_type &&
      strcmp(op->req_body.content_type, "application/json") == 0 &&
      (op->req_body.ref_name || schema_has_inline(&op->req_body))) {
    CHECK_IO(fprintf(fp, "  if (req_json) free(req_json);\n"));
  }
  if (op->req_body.ref_name && op->req_body.content_type &&
      strcmp(op->req_body.content_type, "application/x-www-form-urlencoded") ==
          0) {
    CHECK_IO(fprintf(fp, "  if (form_body) free(form_body);\n"));
    CHECK_IO(fprintf(fp, "  url_query_free(&form_qp);\n"));
  }
  if (query_exists) {
    CHECK_IO(fprintf(fp, "  if (path_str) free(path_str);\n"));
    CHECK_IO(fprintf(fp, "  if (query_str) free(query_str);\n"));
    CHECK_IO(fprintf(fp, "  url_query_free(&qp);\n"));
  }
  if (cookie_exists) {
    CHECK_IO(fprintf(fp, "  if (cookie_str) free(cookie_str);\n"));
  }
  CHECK_IO(fprintf(fp, "  http_request_free(&req);\n"));
  CHECK_IO(fprintf(fp, "  if (res) { http_response_free(res); free(res); }\n"));
  CHECK_IO(fprintf(fp, "  return rc;\n}\n"));

  return 0;
}
