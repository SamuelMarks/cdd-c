/**
 * @file codegen_url.c
 * @brief Implementation of URL interpolator generation.
 *
 * Updated to include `codegen_url_write_query_params` which handles the
 * complexity of Loop Generation for Array Parameters.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_url.h"
#include "str_utils.h"

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

struct UrlSegment {
  int is_var;
  char *text;
};

static int is_primitive_type(const char *type) {
  if (!type)
    return 0;
  return strcmp(type, "integer") == 0 || strcmp(type, "string") == 0 ||
         strcmp(type, "boolean") == 0 || strcmp(type, "number") == 0;
}

static int param_is_object_kv(const struct OpenAPI_Parameter *p) {
  if (!p)
    return 0;
  if (p->is_array)
    return 0;
  if (p->in != OA_PARAM_IN_QUERY)
    return 0;
  if (!p->type)
    return 0;
  return !is_primitive_type(p->type);
}

static size_t media_type_base_len(const char *media_type) {
  size_t i = 0;
  if (!media_type)
    return 0;
  while (media_type[i] && media_type[i] != ';')
    ++i;
  return i;
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
  size_t len;
  if (!media_type)
    return 0;
  if (media_type_ieq(media_type, "application/json"))
    return 1;
  len = media_type_base_len(media_type);
  if (len < 5)
    return 0;
  {
    const char *suffix = "+json";
    size_t start = len - 5;
    size_t i;
    for (i = 0; i < 5; ++i) {
      char a = media_type[start + i];
      char b = suffix[i];
      if (a >= 'A' && a <= 'Z')
        a = (char)(a - 'A' + 'a');
      if (b >= 'A' && b <= 'Z')
        b = (char)(b - 'A' + 'a');
      if (a != b)
        return 0;
    }
  }
  return 1;
}

static int media_type_is_form(const char *media_type) {
  return media_type_ieq(media_type, "application/x-www-form-urlencoded");
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

static int write_query_json_param(FILE *const fp,
                                  const struct OpenAPI_Parameter *const p) {
  const char *name;
  const char *type;

  if (!fp || !p)
    return EINVAL;
  if (!p->content_type || !media_type_is_json(p->content_type))
    return EINVAL;

  name = p->name ? p->name : "param";
  type = p->type ? p->type : p->schema.inline_type;

  CHECK_IO(fprintf(fp, "  /* Query Parameter (json): %s */\n", name));

  if (p->is_array) {
    const char *item_type =
        p->items_type ? p->items_type : p->schema.inline_type;
    if (!item_type) {
      CHECK_IO(
          fprintf(fp, "  /* Unsupported JSON query array for %s */\n", name));
      return 0;
    }
    if (is_primitive_type(item_type)) {
      CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", name, name));
      CHECK_IO(fprintf(fp, "    JSON_Value *q_val = NULL;\n"));
      CHECK_IO(fprintf(fp, "    JSON_Array *q_arr = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *q_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *q_enc = NULL;\n"));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    q_val = json_value_init_array();\n"));
      CHECK_IO(fprintf(fp, "    if (!q_val) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    q_arr = json_value_get_array(q_val);\n"));
      CHECK_IO(fprintf(fp, "    if (!q_arr) { rc = EINVAL; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    for (i = 0; i < %s_len; ++i) {\n", name));
      if (strcmp(item_type, "string") == 0) {
        CHECK_IO(fprintf(fp, "      if (!%s[i]) {\n", name));
        CHECK_IO(fprintf(
            fp,
            "        if (json_array_append_null(q_arr) != JSONSuccess) { rc = "
            "ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "      } else {\n"));
        CHECK_IO(fprintf(fp,
                         "        if (json_array_append_string(q_arr, %s[i]) "
                         "!= JSONSuccess) "
                         "{ rc = ENOMEM; goto cleanup; }\n",
                         name));
        CHECK_IO(fprintf(fp, "      }\n"));
      } else if (strcmp(item_type, "integer") == 0) {
        CHECK_IO(fprintf(
            fp,
            "      if (json_array_append_number(q_arr, (double)%s[i]) != "
            "JSONSuccess) { rc = ENOMEM; goto cleanup; }\n",
            name));
      } else if (strcmp(item_type, "number") == 0) {
        CHECK_IO(fprintf(fp,
                         "      if (json_array_append_number(q_arr, %s[i]) != "
                         "JSONSuccess) { "
                         "rc = ENOMEM; goto cleanup; }\n",
                         name));
      } else if (strcmp(item_type, "boolean") == 0) {
        CHECK_IO(fprintf(
            fp,
            "      if (json_array_append_boolean(q_arr, %s[i] ? 1 : 0) != "
            "JSONSuccess) { rc = ENOMEM; goto cleanup; }\n",
            name));
      }
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "    q_json = json_serialize_to_string(q_val);\n"));
      CHECK_IO(fprintf(fp, "    json_value_free(q_val);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!q_json) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    q_enc = url_encode(q_json);\n"));
      CHECK_IO(fprintf(fp, "    json_free_serialized_string(q_json);\n"));
      CHECK_IO(fprintf(fp, "    if (!q_enc) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(
          fp, "    rc = url_query_add_encoded(&qp, \"%s\", q_enc);\n", name));
      CHECK_IO(fprintf(fp, "    free(q_enc);\n"));
      CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "  }\n"));
      return 0;
    }
    if (strcmp(item_type, "object") == 0) {
      CHECK_IO(fprintf(fp, "  /* Unsupported JSON query array item for %s */\n",
                       name));
      return 0;
    }
    CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", name, name));
    CHECK_IO(fprintf(fp, "    JSON_Value *q_val = NULL;\n"));
    CHECK_IO(fprintf(fp, "    JSON_Array *q_arr = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_json = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_enc = NULL;\n"));
    CHECK_IO(fprintf(fp, "    size_t i;\n"));
    CHECK_IO(fprintf(fp, "    q_val = json_value_init_array();\n"));
    CHECK_IO(fprintf(fp, "    if (!q_val) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    q_arr = json_value_get_array(q_val);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_arr) { rc = EINVAL; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    for (i = 0; i < %s_len; ++i) {\n", name));
    CHECK_IO(fprintf(fp, "      char *item_json = NULL;\n"));
    CHECK_IO(fprintf(fp, "      JSON_Value *item_val = NULL;\n"));
    CHECK_IO(fprintf(fp, "      if (!%s[i]) {\n", name));
    CHECK_IO(fprintf(
        fp, "        if (json_array_append_null(q_arr) != JSONSuccess) { rc = "
            "ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "        continue;\n"));
    CHECK_IO(fprintf(fp, "      }\n"));
    CHECK_IO(fprintf(fp, "      rc = %s_to_json(%s[i], &item_json);\n",
                     item_type, name));
    CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "      item_val = json_parse_string(item_json);\n"));
    CHECK_IO(fprintf(fp, "      free(item_json);\n"));
    CHECK_IO(
        fprintf(fp, "      if (!item_val) { rc = EINVAL; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp,
        "      if (json_array_append_value(q_arr, item_val) != JSONSuccess) { "
        "json_value_free(item_val); rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    }\n"));
    CHECK_IO(fprintf(fp, "    q_json = json_serialize_to_string(q_val);\n"));
    CHECK_IO(fprintf(fp, "    json_value_free(q_val);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_json) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    q_enc = url_encode(q_json);\n"));
    CHECK_IO(fprintf(fp, "    json_free_serialized_string(q_json);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_enc) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp, "    rc = url_query_add_encoded(&qp, \"%s\", q_enc);\n", name));
    CHECK_IO(fprintf(fp, "    free(q_enc);\n"));
    CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n"));
    return 0;
  }

  if (p->schema.ref_name) {
    CHECK_IO(fprintf(fp, "  if (%s) {\n", name));
    CHECK_IO(fprintf(fp, "    char *q_json = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_enc = NULL;\n"));
    CHECK_IO(fprintf(fp, "    rc = %s_to_json(%s, &q_json);\n",
                     p->schema.ref_name, name));
    CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "    q_enc = url_encode(q_json);\n"));
    CHECK_IO(fprintf(fp, "    free(q_json);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_enc) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp, "    rc = url_query_add_encoded(&qp, \"%s\", q_enc);\n", name));
    CHECK_IO(fprintf(fp, "    free(q_enc);\n"));
    CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n"));
    return 0;
  }

  if (type && strcmp(type, "object") == 0) {
    CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", name, name));
    CHECK_IO(fprintf(fp, "    JSON_Value *q_val = NULL;\n"));
    CHECK_IO(fprintf(fp, "    JSON_Object *q_obj = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_json = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_enc = NULL;\n"));
    CHECK_IO(fprintf(fp, "    size_t i;\n"));
    CHECK_IO(fprintf(fp, "    q_val = json_value_init_object();\n"));
    CHECK_IO(fprintf(fp, "    if (!q_val) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    q_obj = json_value_get_object(q_val);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_obj) { rc = EINVAL; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    for (i = 0; i < %s_len; ++i) {\n", name));
    CHECK_IO(
        fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n", name));
    CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
    CHECK_IO(fprintf(fp, "      if (!kv_key) continue;\n"));
    CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"
                         "        if (kv->value.s) {\n"
                         "          json_object_set_string(q_obj, kv_key, "
                         "kv->value.s);\n"
                         "        } else {\n"
                         "          json_object_set_null(q_obj, kv_key);\n"
                         "        }\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_INTEGER:\n"
                         "        json_object_set_number(q_obj, kv_key, "
                         "(double)kv->value.i);\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                         "        json_object_set_number(q_obj, kv_key, "
                         "kv->value.n);\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_BOOLEAN:\n"
                         "        json_object_set_boolean(q_obj, kv_key, "
                         "kv->value.b ? 1 : 0);\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      default:\n"
                         "        json_object_set_null(q_obj, kv_key);\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      }\n"));
    CHECK_IO(fprintf(fp, "    }\n"));
    CHECK_IO(fprintf(fp, "    q_json = json_serialize_to_string(q_val);\n"));
    CHECK_IO(fprintf(fp, "    json_value_free(q_val);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_json) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    q_enc = url_encode(q_json);\n"));
    CHECK_IO(fprintf(fp, "    json_free_serialized_string(q_json);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_enc) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp, "    rc = url_query_add_encoded(&qp, \"%s\", q_enc);\n", name));
    CHECK_IO(fprintf(fp, "    free(q_enc);\n"));
    CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n"));
    return 0;
  }

  if (type && is_primitive_type(type)) {
    if (strcmp(type, "string") == 0) {
      CHECK_IO(fprintf(fp, "  if (%s) {\n", name));
    } else {
      CHECK_IO(fprintf(fp, "  {\n"));
    }
    CHECK_IO(fprintf(fp, "    JSON_Value *q_val = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_json = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_enc = NULL;\n"));
    if (strcmp(type, "string") == 0) {
      CHECK_IO(fprintf(fp, "    q_val = json_value_init_string(%s);\n", name));
    } else if (strcmp(type, "integer") == 0) {
      CHECK_IO(fprintf(fp, "    q_val = json_value_init_number((double)%s);\n",
                       name));
    } else if (strcmp(type, "number") == 0) {
      CHECK_IO(fprintf(fp, "    q_val = json_value_init_number(%s);\n", name));
    } else if (strcmp(type, "boolean") == 0) {
      CHECK_IO(fprintf(fp, "    q_val = json_value_init_boolean(%s ? 1 : 0);\n",
                       name));
    }
    CHECK_IO(fprintf(fp, "    if (!q_val) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    q_json = json_serialize_to_string(q_val);\n"));
    CHECK_IO(fprintf(fp, "    json_value_free(q_val);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_json) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    q_enc = url_encode(q_json);\n"));
    CHECK_IO(fprintf(fp, "    json_free_serialized_string(q_json);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_enc) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp, "    rc = url_query_add_encoded(&qp, \"%s\", q_enc);\n", name));
    CHECK_IO(fprintf(fp, "    free(q_enc);\n"));
    CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n"));
    return 0;
  }

  CHECK_IO(
      fprintf(fp, "  /* Unsupported JSON query parameter for %s */\n", name));
  return 0;
}

static int write_query_object_param(FILE *const fp,
                                    const struct OpenAPI_Parameter *const p) {
  const char *name;
  enum OpenAPI_Style style;
  int explode;
  int allow_reserved;

  if (!fp || !p)
    return EINVAL;

  name = p->name ? p->name : "param";
  style = (p->style == OA_STYLE_UNKNOWN) ? OA_STYLE_FORM : p->style;
  explode =
      p->explode_set
          ? p->explode
          : ((style == OA_STYLE_FORM || style == OA_STYLE_COOKIE) ? 1 : 0);
  allow_reserved = p->allow_reserved_set && p->allow_reserved;

  CHECK_IO(fprintf(fp, "  /* Query Object Parameter: %s */\n", name));

  if (style == OA_STYLE_DEEP_OBJECT) {
    CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
    CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", name));
    CHECK_IO(
        fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n", name));
    CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
    CHECK_IO(fprintf(fp, "      const char *kv_raw = NULL;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp, "      char *deep_key = NULL;\n"));
    CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"));
    CHECK_IO(fprintf(fp, "        kv_raw = kv->value.s;\n        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_INTEGER:\n"
                         "        sprintf(num_buf, \"%%d\", kv->value.i);\n"
                         "        kv_raw = num_buf;\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                         "        sprintf(num_buf, \"%%g\", kv->value.n);\n"
                         "        kv_raw = num_buf;\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp,
                     "      case OA_KV_BOOLEAN:\n"
                     "        kv_raw = kv->value.b ? \"true\" : \"false\";\n"
                     "        break;\n"));
    CHECK_IO(fprintf(
        fp, "      default:\n        kv_raw = NULL;\n        break;\n"));
    CHECK_IO(fprintf(fp, "      }\n"));
    CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
    CHECK_IO(fprintf(fp,
                     "      if (asprintf(&deep_key, \"%%s[%%s]\", \"%s\", "
                     "kv_key) == -1) { rc = ENOMEM; goto cleanup; }\n",
                     name));
    if (allow_reserved) {
      CHECK_IO(fprintf(fp, "      if (kv->type == OA_KV_STRING) {\n"));
      CHECK_IO(fprintf(
          fp, "        char *enc = url_encode_allow_reserved(kv_raw);\n"));
      CHECK_IO(fprintf(fp, "        if (!enc) { free(deep_key); rc = ENOMEM; "
                           "goto cleanup; }\n"));
      CHECK_IO(fprintf(
          fp, "        rc = url_query_add_encoded(&qp, deep_key, enc);\n"));
      CHECK_IO(fprintf(fp, "        free(enc);\n"));
      CHECK_IO(fprintf(fp, "      } else {\n"));
      CHECK_IO(
          fprintf(fp, "        rc = url_query_add(&qp, deep_key, kv_raw);\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
    } else {
      CHECK_IO(
          fprintf(fp, "      rc = url_query_add(&qp, deep_key, kv_raw);\n"));
    }
    CHECK_IO(fprintf(fp, "      free(deep_key);\n"));
    CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "    }\n  }\n"));
    return 0;
  }

  if (style == OA_STYLE_FORM && !explode) {
    CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
    CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
    CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
    CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", name));
    CHECK_IO(
        fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n", name));
    CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
    CHECK_IO(fprintf(fp, "      const char *kv_raw = NULL;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp, "      char *key_enc = NULL;\n"));
    CHECK_IO(fprintf(fp, "      char *val_enc = NULL;\n"));
    CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"));
    CHECK_IO(fprintf(fp, "        kv_raw = kv->value.s;\n        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_INTEGER:\n"
                         "        sprintf(num_buf, \"%%d\", kv->value.i);\n"
                         "        kv_raw = num_buf;\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                         "        sprintf(num_buf, \"%%g\", kv->value.n);\n"
                         "        kv_raw = num_buf;\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp,
                     "      case OA_KV_BOOLEAN:\n"
                     "        kv_raw = kv->value.b ? \"true\" : \"false\";\n"
                     "        break;\n"));
    CHECK_IO(fprintf(
        fp, "      default:\n        kv_raw = NULL;\n        break;\n"));
    CHECK_IO(fprintf(fp, "      }\n"));
    CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
    if (allow_reserved) {
      CHECK_IO(
          fprintf(fp, "      key_enc = url_encode_allow_reserved(kv_key);\n"));
      CHECK_IO(
          fprintf(fp, "      val_enc = url_encode_allow_reserved(kv_raw);\n"));
    } else {
      CHECK_IO(fprintf(fp, "      key_enc = url_encode(kv_key);\n"));
      CHECK_IO(fprintf(fp, "      val_enc = url_encode(kv_raw);\n"));
    }
    CHECK_IO(fprintf(fp, "      if (!key_enc || !val_enc) { free(key_enc); "
                         "free(val_enc); rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp,
        "      {\n"
        "        size_t key_len = strlen(key_enc);\n"
        "        size_t val_len = strlen(val_enc);\n"
        "        size_t extra = key_len + val_len + 1 + (joined_len ? 1 : 0);\n"
        "        char *tmp = (char *)realloc(joined, joined_len + extra + 1);\n"
        "        if (!tmp) { free(key_enc); free(val_enc); rc = ENOMEM; goto "
        "cleanup; }\n"
        "        joined = tmp;\n"
        "        if (joined_len) joined[joined_len++] = ',';\n"
        "        memcpy(joined + joined_len, key_enc, key_len);\n"
        "        joined_len += key_len;\n"
        "        joined[joined_len++] = ',';\n"
        "        memcpy(joined + joined_len, val_enc, val_len);\n"
        "        joined_len += val_len;\n"
        "        joined[joined_len] = '\\0';\n"
        "      }\n"));
    CHECK_IO(fprintf(fp, "      free(key_enc);\n      free(val_enc);\n"));
    CHECK_IO(fprintf(fp, "    }\n"));
    CHECK_IO(fprintf(fp, "    if (joined) {\n"));
    CHECK_IO(fprintf(
        fp, "      rc = url_query_add_encoded(&qp, \"%s\", joined);\n", name));
    CHECK_IO(fprintf(fp, "      free(joined);\n"));
    CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "    }\n"));
    CHECK_IO(fprintf(fp, "  }\n"));
    return 0;
  }

  if (style == OA_STYLE_FORM && explode) {
    CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
    CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", name));
    CHECK_IO(
        fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n", name));
    CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
    CHECK_IO(fprintf(fp, "      const char *kv_raw = NULL;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"));
    CHECK_IO(fprintf(fp, "        kv_raw = kv->value.s;\n        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_INTEGER:\n"
                         "        sprintf(num_buf, \"%%d\", kv->value.i);\n"
                         "        kv_raw = num_buf;\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                         "        sprintf(num_buf, \"%%g\", kv->value.n);\n"
                         "        kv_raw = num_buf;\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp,
                     "      case OA_KV_BOOLEAN:\n"
                     "        kv_raw = kv->value.b ? \"true\" : \"false\";\n"
                     "        break;\n"));
    CHECK_IO(fprintf(
        fp, "      default:\n        kv_raw = NULL;\n        break;\n"));
    CHECK_IO(fprintf(fp, "      }\n"));
    CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
    if (allow_reserved) {
      CHECK_IO(fprintf(fp, "      if (kv->type == OA_KV_STRING) {\n"));
      CHECK_IO(fprintf(
          fp, "        char *enc = url_encode_allow_reserved(kv_raw);\n"));
      CHECK_IO(
          fprintf(fp, "        if (!enc) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(
          fp, "        rc = url_query_add_encoded(&qp, kv_key, enc);\n"));
      CHECK_IO(fprintf(fp, "        free(enc);\n"));
      CHECK_IO(fprintf(fp, "      } else {\n"));
      CHECK_IO(
          fprintf(fp, "        rc = url_query_add(&qp, kv_key, kv_raw);\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
    } else {
      CHECK_IO(fprintf(fp, "      rc = url_query_add(&qp, kv_key, kv_raw);\n"));
    }
    CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "    }\n  }\n"));
    return 0;
  }

  if (style == OA_STYLE_SPACE_DELIMITED || style == OA_STYLE_PIPE_DELIMITED) {
    const char delim = (style == OA_STYLE_SPACE_DELIMITED) ? ' ' : '|';
    const char *delim_enc = (style == OA_STYLE_SPACE_DELIMITED) ? "%20" : "%7C";

    if (allow_reserved) {
      CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
      CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
      CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", name));
      CHECK_IO(
          fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n", name));
      CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
      CHECK_IO(fprintf(fp, "      const char *kv_raw = NULL;\n"));
      CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
      CHECK_IO(fprintf(fp, "      char *key_enc = NULL;\n"));
      CHECK_IO(fprintf(fp, "      char *val_enc = NULL;\n"));
      CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"));
      CHECK_IO(fprintf(fp, "        kv_raw = kv->value.s;\n        break;\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_INTEGER:\n"
                           "        sprintf(num_buf, \"%%d\", kv->value.i);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                           "        sprintf(num_buf, \"%%g\", kv->value.n);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));
      CHECK_IO(fprintf(fp,
                       "      case OA_KV_BOOLEAN:\n"
                       "        kv_raw = kv->value.b ? \"true\" : \"false\";\n"
                       "        break;\n"));
      CHECK_IO(fprintf(
          fp, "      default:\n        kv_raw = NULL;\n        break;\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
      CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
      CHECK_IO(
          fprintf(fp, "      key_enc = url_encode_allow_reserved(kv_key);\n"));
      CHECK_IO(
          fprintf(fp, "      val_enc = url_encode_allow_reserved(kv_raw);\n"));
      CHECK_IO(fprintf(fp, "      if (!key_enc || !val_enc) { free(key_enc); "
                           "free(val_enc); rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(
          fp,
          "      {\n"
          "        size_t key_len = strlen(key_enc);\n"
          "        size_t val_len = strlen(val_enc);\n"
          "        size_t extra = key_len + val_len + %zu + "
          "(joined_len ? %zu : 0);\n"
          "        char *tmp = (char *)realloc(joined, joined_len + extra + "
          "1);\n"
          "        if (!tmp) { free(key_enc); free(val_enc); rc = ENOMEM; "
          "goto cleanup; }\n"
          "        joined = tmp;\n"
          "        if (joined_len) {\n"
          "          memcpy(joined + joined_len, \"%s\", %zu);\n"
          "          joined_len += %zu;\n"
          "        }\n"
          "        memcpy(joined + joined_len, key_enc, key_len);\n"
          "        joined_len += key_len;\n"
          "        memcpy(joined + joined_len, \"%s\", %zu);\n"
          "        joined_len += %zu;\n"
          "        memcpy(joined + joined_len, val_enc, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          strlen(delim_enc), strlen(delim_enc), delim_enc, strlen(delim_enc),
          strlen(delim_enc), delim_enc, strlen(delim_enc), strlen(delim_enc)));
      CHECK_IO(fprintf(fp, "      free(key_enc);\n      free(val_enc);\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "    if (joined) {\n"));
      CHECK_IO(fprintf(
          fp, "      rc = url_query_add_encoded(&qp, \"%s\", joined);\n",
          name));
      CHECK_IO(fprintf(fp, "      free(joined);\n"));
      CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "  }\n"));
    } else {
      CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
      CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
      CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", name));
      CHECK_IO(
          fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n", name));
      CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
      CHECK_IO(fprintf(fp, "      const char *kv_raw = NULL;\n"));
      CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
      CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"));
      CHECK_IO(fprintf(fp, "        kv_raw = kv->value.s;\n        break;\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_INTEGER:\n"
                           "        sprintf(num_buf, \"%%d\", kv->value.i);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                           "        sprintf(num_buf, \"%%g\", kv->value.n);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));
      CHECK_IO(fprintf(fp,
                       "      case OA_KV_BOOLEAN:\n"
                       "        kv_raw = kv->value.b ? \"true\" : \"false\";\n"
                       "        break;\n"));
      CHECK_IO(fprintf(
          fp, "      default:\n        kv_raw = NULL;\n        break;\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
      CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
      CHECK_IO(fprintf(
          fp,
          "      {\n"
          "        size_t key_len = strlen(kv_key);\n"
          "        size_t val_len = strlen(kv_raw);\n"
          "        size_t extra = key_len + val_len + 1 + "
          "(joined_len ? 1 : 0);\n"
          "        char *tmp = (char *)realloc(joined, joined_len + extra + "
          "1);\n"
          "        if (!tmp) { rc = ENOMEM; goto cleanup; }\n"
          "        joined = tmp;\n"
          "        if (joined_len) joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, kv_key, key_len);\n"
          "        joined_len += key_len;\n"
          "        joined[joined_len++] = '%c';\n"
          "        memcpy(joined + joined_len, kv_raw, val_len);\n"
          "        joined_len += val_len;\n"
          "        joined[joined_len] = '\\0';\n"
          "      }\n",
          delim, delim));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "    if (joined) {\n"));
      CHECK_IO(fprintf(fp, "      rc = url_query_add(&qp, \"%s\", joined);\n",
                       name));
      CHECK_IO(fprintf(fp, "      free(joined);\n"));
      CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "  }\n"));
    }
    return 0;
  }

  CHECK_IO(
      fprintf(fp, "  /* Object style not yet supported for %s */\n", name));
  return 0;
}

static int write_path_object_serialization(FILE *const fp,
                                           const struct OpenAPI_Parameter *p) {
  const char *name;
  enum OpenAPI_Style style;
  int explode;
  const char *prefix = "";
  const char *pair_delim = ",";
  char buf_prefix[96];
  size_t prefix_len;
  size_t delim_len;
  const char *encode_fn;

  if (!fp || !p)
    return EINVAL;

  name = p->name ? p->name : "param";
  style = (p->style == OA_STYLE_UNKNOWN) ? OA_STYLE_SIMPLE : p->style;
  explode =
      p->explode_set
          ? p->explode
          : ((style == OA_STYLE_FORM || style == OA_STYLE_COOKIE) ? 1 : 0);

  if (style == OA_STYLE_LABEL) {
    prefix = ".";
    pair_delim = explode ? "." : ",";
  } else if (style == OA_STYLE_MATRIX) {
    if (explode) {
      prefix = ";";
      pair_delim = ";";
    } else {
      sprintf(buf_prefix, ";%s=", name);
      prefix = buf_prefix;
      pair_delim = ",";
    }
  } else {
    prefix = "";
    pair_delim = ",";
  }

  prefix_len = strlen(prefix);
  delim_len = strlen(pair_delim);

  encode_fn = (p->allow_reserved_set && p->allow_reserved)
                  ? "url_encode_allow_reserved"
                  : "url_encode";

  CHECK_IO(fprintf(fp, "  char *path_%s = NULL;\n", name));
  CHECK_IO(fprintf(fp, "  {\n"
                       "    size_t i;\n"
                       "    size_t path_len = 0;\n"
                       "    int first = 1;\n"));
  CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", name));
  CHECK_IO(fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n", name));
  CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
  CHECK_IO(fprintf(fp, "      const char *kv_raw = NULL;\n"));
  CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
  CHECK_IO(fprintf(fp, "      char *key_enc = NULL;\n"));
  CHECK_IO(fprintf(fp, "      char *val_enc = NULL;\n"));
  CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
  CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"));
  CHECK_IO(fprintf(fp, "        kv_raw = kv->value.s;\n        break;\n"));
  CHECK_IO(fprintf(fp, "      case OA_KV_INTEGER:\n"
                       "        sprintf(num_buf, \"%%d\", kv->value.i);\n"
                       "        kv_raw = num_buf;\n"
                       "        break;\n"));
  CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                       "        sprintf(num_buf, \"%%g\", kv->value.n);\n"
                       "        kv_raw = num_buf;\n"
                       "        break;\n"));
  CHECK_IO(fprintf(fp, "      case OA_KV_BOOLEAN:\n"
                       "        kv_raw = kv->value.b ? \"true\" : \"false\";\n"
                       "        break;\n"));
  CHECK_IO(fprintf(fp, "      default:\n"
                       "        kv_raw = NULL;\n"
                       "        break;\n"));
  CHECK_IO(fprintf(fp, "      }\n"));
  CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));

  CHECK_IO(fprintf(fp, "      key_enc = %s(kv_key);\n", encode_fn));
  CHECK_IO(fprintf(fp, "      val_enc = %s(kv_raw);\n", encode_fn));
  CHECK_IO(fprintf(fp, "      if (!key_enc || !val_enc) {\n"
                       "        free(key_enc);\n"
                       "        free(val_enc);\n"
                       "        rc = ENOMEM;\n"
                       "        goto cleanup;\n"
                       "      }\n"));
  CHECK_IO(fprintf(fp, "      {\n"));
  if (explode) {
    CHECK_IO(fprintf(
        fp,
        "        size_t key_len = strlen(key_enc);\n"
        "        size_t val_len = strlen(val_enc);\n"
        "        size_t extra = key_len + val_len + 1 + (first ? %zu : %zu);\n"
        "        char *tmp = (char *)realloc(path_%s, path_len + extra + 1);\n"
        "        if (!tmp) { free(key_enc); free(val_enc); rc = ENOMEM; goto "
        "cleanup; }\n"
        "        path_%s = tmp;\n"
        "        if (first && %zu) { memcpy(path_%s + path_len, \"%s\", %zu); "
        "path_len += %zu; }\n"
        "        if (!first && %zu) { memcpy(path_%s + path_len, \"%s\", %zu); "
        "path_len += %zu; }\n"
        "        memcpy(path_%s + path_len, key_enc, key_len);\n"
        "        path_len += key_len;\n"
        "        path_%s[path_len++] = '=';\n"
        "        memcpy(path_%s + path_len, val_enc, val_len);\n"
        "        path_len += val_len;\n"
        "        path_%s[path_len] = '\\0';\n",
        prefix_len, delim_len, name, name, prefix_len, name, prefix, prefix_len,
        prefix_len, delim_len, name, pair_delim, delim_len, delim_len, name,
        name, name, name, name));
  } else {
    CHECK_IO(fprintf(
        fp,
        "        size_t key_len = strlen(key_enc);\n"
        "        size_t val_len = strlen(val_enc);\n"
        "        size_t extra = key_len + val_len + 1 + (first ? %zu : %zu) + "
        "%zu;\n"
        "        char *tmp = (char *)realloc(path_%s, path_len + extra + 1);\n"
        "        if (!tmp) { free(key_enc); free(val_enc); rc = ENOMEM; goto "
        "cleanup; }\n"
        "        path_%s = tmp;\n"
        "        if (first && %zu) { memcpy(path_%s + path_len, \"%s\", %zu); "
        "path_len += %zu; }\n"
        "        if (!first && %zu) { memcpy(path_%s + path_len, \"%s\", %zu); "
        "path_len += %zu; }\n"
        "        memcpy(path_%s + path_len, key_enc, key_len);\n"
        "        path_len += key_len;\n"
        "        memcpy(path_%s + path_len, \"%s\", %zu);\n"
        "        path_len += %zu;\n"
        "        memcpy(path_%s + path_len, val_enc, val_len);\n"
        "        path_len += val_len;\n"
        "        path_%s[path_len] = '\\0';\n",
        prefix_len, delim_len, delim_len, name, name, prefix_len, name, prefix,
        prefix_len, prefix_len, delim_len, name, pair_delim, delim_len,
        delim_len, name, name, pair_delim, delim_len, delim_len, name, name));
  }
  CHECK_IO(fprintf(fp, "      }\n"));
  CHECK_IO(fprintf(fp, "      free(key_enc);\n"));
  CHECK_IO(fprintf(fp, "      free(val_enc);\n"));
  CHECK_IO(fprintf(fp, "      first = 0;\n"));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "    if (!path_%s) {\n", name));
  CHECK_IO(fprintf(fp, "      path_%s = strdup(\"%s\");\n", name, prefix));
  CHECK_IO(fprintf(fp, "      if (!path_%s) { rc = ENOMEM; goto cleanup; }\n",
                   name));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "  }\n"));

  return 0;
}

static int write_path_array_serialization(FILE *const fp,
                                          const struct OpenAPI_Parameter *p,
                                          const char *prefix,
                                          const char *delim) {
  size_t prefix_len;
  size_t delim_len;
  const char *name;
  const char *items_type;
  const char *encode_fn = NULL;

  if (!fp || !p || !prefix || !delim)
    return EINVAL;

  name = p->name ? p->name : "param";
  items_type = p->items_type ? p->items_type : "string";
  prefix_len = strlen(prefix);
  delim_len = strlen(delim);

  if (strcmp(items_type, "string") == 0) {
    if (p->allow_reserved_set && p->allow_reserved)
      encode_fn = "url_encode_allow_reserved";
    else
      encode_fn = "url_encode";
  }

  CHECK_IO(fprintf(fp, "  char *path_%s = NULL;\n", name));
  CHECK_IO(fprintf(fp, "  {\n    size_t i;\n    size_t path_len = 0;\n"));
  CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", name));

  if (strcmp(items_type, "integer") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
    CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", %s[i]);\n", name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(items_type, "number") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%g\", %s[i]);\n", name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(items_type, "boolean") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i] ? \"true\" : \"false\";\n", name));
  } else {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i];\n", name));
  }

  if (encode_fn) {
    CHECK_IO(fprintf(fp, "      char *enc = %s(raw);\n", encode_fn));
    CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
    CHECK_IO(fprintf(fp, "      if (!enc) { rc = ENOMEM; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
    CHECK_IO(fprintf(
        fp,
        "      {\n"
        "        size_t extra = val_len + (i > 0 ? %zu : 0) + (i == 0 ? %zu : "
        "0);\n"
        "        char *tmp = (char *)realloc(path_%s, path_len + extra + 1);\n"
        "        if (!tmp) { free(enc); rc = ENOMEM; goto cleanup; }\n"
        "        path_%s = tmp;\n"
        "        if (i == 0 && %zu) { memcpy(path_%s + path_len, \"%s\", %zu); "
        "path_len += %zu; }\n"
        "        if (i > 0 && %zu) { memcpy(path_%s + path_len, \"%s\", %zu); "
        "path_len += %zu; }\n"
        "        memcpy(path_%s + path_len, enc, val_len);\n"
        "        path_len += val_len;\n"
        "        path_%s[path_len] = '\\0';\n"
        "      }\n",
        delim_len, prefix_len, name, name, prefix_len, name, prefix, prefix_len,
        prefix_len, delim_len, name, delim, delim_len, delim_len, name, name));
    CHECK_IO(fprintf(fp, "      free(enc);\n"));
  } else {
    CHECK_IO(fprintf(fp, "      size_t val_len = strlen(raw);\n"));
    CHECK_IO(fprintf(
        fp,
        "      {\n"
        "        size_t extra = val_len + (i > 0 ? %zu : 0) + (i == 0 ? %zu : "
        "0);\n"
        "        char *tmp = (char *)realloc(path_%s, path_len + extra + 1);\n"
        "        if (!tmp) { rc = ENOMEM; goto cleanup; }\n"
        "        path_%s = tmp;\n"
        "        if (i == 0 && %zu) { memcpy(path_%s + path_len, \"%s\", %zu); "
        "path_len += %zu; }\n"
        "        if (i > 0 && %zu) { memcpy(path_%s + path_len, \"%s\", %zu); "
        "path_len += %zu; }\n"
        "        memcpy(path_%s + path_len, raw, val_len);\n"
        "        path_len += val_len;\n"
        "        path_%s[path_len] = '\\0';\n"
        "      }\n",
        delim_len, prefix_len, name, name, prefix_len, name, prefix, prefix_len,
        prefix_len, delim_len, name, delim, delim_len, delim_len, name, name));
  }

  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "    if (!path_%s) {\n", name));
  CHECK_IO(fprintf(fp, "      path_%s = strdup(\"%s\");\n", name, prefix));
  CHECK_IO(fprintf(fp, "      if (!path_%s) { rc = ENOMEM; goto cleanup; }\n",
                   name));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "  }\n"));
  return 0;
}

static int write_joined_query_array(FILE *const fp,
                                    const struct OpenAPI_Parameter *const p,
                                    const char delim, const char *encode_fn,
                                    const int add_encoded) {
  const char *name;
  const char *item_type;
  const int do_encode = (encode_fn && encode_fn[0] != '\0');

  if (!fp || !p)
    return EINVAL;

  name = p->name ? p->name : "param";
  item_type = p->items_type ? p->items_type : "string";

  CHECK_IO(fprintf(fp, "  {\n"));
  CHECK_IO(fprintf(fp, "    size_t i;\n"));
  CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
  CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
  CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", name));

  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
    CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", %s[i]);\n", name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(item_type, "number") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%g\", %s[i]);\n", name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(item_type, "boolean") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i] ? \"true\" : \"false\";\n", name));
  } else {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i];\n", name));
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
        fp, "      rc = url_query_add_encoded(&qp, \"%s\", joined);\n", name));
  } else {
    CHECK_IO(
        fprintf(fp, "      rc = url_query_add(&qp, \"%s\", joined);\n", name));
  }
  CHECK_IO(fprintf(fp, "      free(joined);\n"));
  CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "  }\n"));

  return 0;
}

static int write_joined_query_array_encoded_delim(
    FILE *const fp, const struct OpenAPI_Parameter *const p,
    const char *const delim_enc, const char *const encode_fn) {
  const char *name;
  const char *item_type;
  size_t delim_len;

  if (!fp || !p || !delim_enc || !encode_fn)
    return EINVAL;

  name = p->name ? p->name : "param";
  item_type = p->items_type ? p->items_type : "string";
  delim_len = strlen(delim_enc);

  CHECK_IO(fprintf(fp, "  {\n"));
  CHECK_IO(fprintf(fp, "    size_t i;\n"));
  CHECK_IO(fprintf(fp, "    char *joined = NULL;\n"));
  CHECK_IO(fprintf(fp, "    size_t joined_len = 0;\n"));
  CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", name));

  if (strcmp(item_type, "integer") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
    CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", %s[i]);\n", name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(item_type, "number") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%g\", %s[i]);\n", name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(item_type, "boolean") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i] ? \"true\" : \"false\";\n", name));
  } else {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i];\n", name));
  }

  CHECK_IO(fprintf(fp, "      char *enc = %s(raw);\n", encode_fn));
  CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
  CHECK_IO(fprintf(fp, "      if (!enc) { rc = ENOMEM; goto cleanup; }\n"));
  CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
  CHECK_IO(fprintf(
      fp,
      "      {\n"
      "        size_t extra = val_len + (i > 0 ? %zu : 0);\n"
      "        char *tmp = (char *)realloc(joined, joined_len + extra + 1);\n"
      "        if (!tmp) { free(enc); rc = ENOMEM; goto cleanup; }\n"
      "        joined = tmp;\n"
      "        if (i > 0) {\n"
      "          memcpy(joined + joined_len, \"%s\", %zu);\n"
      "          joined_len += %zu;\n"
      "        }\n"
      "        memcpy(joined + joined_len, enc, val_len);\n"
      "        joined_len += val_len;\n"
      "        joined[joined_len] = '\\0';\n"
      "      }\n",
      delim_len, delim_enc, delim_len, delim_len));
  CHECK_IO(fprintf(fp, "      free(enc);\n"));

  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "    if (joined) {\n"));
  CHECK_IO(fprintf(
      fp, "      rc = url_query_add_encoded(&qp, \"%s\", joined);\n", name));
  CHECK_IO(fprintf(fp, "      free(joined);\n"));
  CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "  }\n"));
  return 0;
}

static const struct OpenAPI_Parameter *
find_param(const char *name, const struct OpenAPI_Parameter *params,
           size_t n_params) {
  size_t i;
  for (i = 0; i < n_params; ++i) {
    if (params[i].name && strcmp(params[i].name, name) == 0 &&
        params[i].in == OA_PARAM_IN_PATH) {
      return &params[i];
    }
  }
  return NULL;
}

static int parse_segments(const char *tmpl, struct UrlSegment **out_segments,
                          size_t *out_count) {
  const char *p = tmpl;
  const char *start = p;
  struct UrlSegment *segs = NULL;
  size_t count = 0;
  size_t cap = 0;

  while (*p) {
    if (*p == '{') {
      if (p > start) {
        size_t len = p - start;
        if (count >= cap) {
          cap = (cap == 0) ? 8 : cap * 2;
          segs = (struct UrlSegment *)realloc(segs,
                                              cap * sizeof(struct UrlSegment));
          if (!segs)
            return ENOMEM;
        }
        segs[count].is_var = 0;
        segs[count].text = malloc(len + 1);
        if (!segs[count].text)
          return ENOMEM;
        memcpy(segs[count].text, start, len);
        segs[count].text[len] = '\0';
        count++;
      }
      start = p + 1;
      {
        const char *close = strchr(start, '}');
        if (!close) {
          size_t i;
          for (i = 0; i < count; ++i)
            free(segs[i].text);
          free(segs);
          return EINVAL;
        }
        {
          size_t len = close - start;
          if (count >= cap) {
            cap = (cap == 0) ? 8 : cap * 2;
            segs = (struct UrlSegment *)realloc(
                segs, cap * sizeof(struct UrlSegment));
            if (!segs)
              return ENOMEM;
          }
          segs[count].is_var = 1;
          segs[count].text = malloc(len + 1);
          if (!segs[count].text)
            return ENOMEM;
          memcpy(segs[count].text, start, len);
          segs[count].text[len] = '\0';
          count++;
        }
        p = close + 1;
        start = p;
      }
    } else {
      p++;
    }
  }
  if (p > start) {
    size_t len = p - start;
    if (count >= cap) {
      cap = (cap == 0) ? 8 : cap * 2;
      segs =
          (struct UrlSegment *)realloc(segs, cap * sizeof(struct UrlSegment));
      if (!segs)
        return ENOMEM;
    }
    segs[count].is_var = 0;
    segs[count].text = malloc(len + 1);
    if (!segs[count].text)
      return ENOMEM;
    memcpy(segs[count].text, start, len);
    segs[count].text[len] = '\0';
    count++;
  }
  *out_segments = segs;
  *out_count = count;
  return 0;
}

int codegen_url_write_builder(FILE *const fp, const char *const path_template,
                              const struct OpenAPI_Parameter *params,
                              size_t n_params,
                              const struct CodegenUrlConfig *const config) {
  struct UrlSegment *segs = NULL;
  size_t n_segs = 0;
  size_t i;
  int rc = 0;
  const char *base_var = (config && config->base_variable)
                             ? config->base_variable
                             : "ctx->base_url";
  const char *out_var =
      (config && config->out_variable) ? config->out_variable : "url";

  if (!fp || !path_template)
    return EINVAL;

  if ((rc = parse_segments(path_template, &segs, &n_segs)) != 0) {
    return rc;
  }

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p =
          find_param(segs[i].text, params, n_params);
      if (p) {
        const char *name = p->name ? p->name : segs[i].text;
        enum OpenAPI_Style style =
            (p->style == OA_STYLE_UNKNOWN) ? OA_STYLE_SIMPLE : p->style;
        int explode =
            p->explode_set
                ? p->explode
                : ((style == OA_STYLE_FORM || style == OA_STYLE_COOKIE) ? 1
                                                                        : 0);
        if (p->type && strcmp(p->type, "object") == 0 && !p->is_array) {
          if (write_path_object_serialization(fp, p) != 0)
            return EIO;
        } else if (p->is_array) {
          const char *prefix = "";
          const char *delim = ",";
          if (style == OA_STYLE_LABEL) {
            prefix = ".";
            delim = explode ? "." : ",";
          } else if (style == OA_STYLE_MATRIX) {
            static char buf_prefix[96];
            static char buf_delim[96];
            sprintf(buf_prefix, ";%s=", name);
            sprintf(buf_delim, ";%s=", name);
            prefix = buf_prefix;
            delim = explode ? buf_delim : ",";
          } else {
            prefix = "";
            delim = ",";
          }
          if (write_path_array_serialization(fp, p, prefix, delim) != 0)
            return EIO;
        } else {
          const char *encode_fn = (p->allow_reserved_set && p->allow_reserved)
                                      ? "url_encode_allow_reserved"
                                      : "url_encode";
          const char *prefix = "";
          if (style == OA_STYLE_LABEL) {
            prefix = ".";
          } else if (style == OA_STYLE_MATRIX) {
            static char buf_prefix[96];
            sprintf(buf_prefix, ";%s=", name);
            prefix = buf_prefix;
          }
          CHECK_IO(fprintf(fp, "  char *path_%s = NULL;\n", name));
          if (strcmp(p->type, "string") == 0) {
            CHECK_IO(
                fprintf(fp, "  {\n    char *enc = %s(%s);\n", encode_fn, name));
            CHECK_IO(fprintf(fp, "    if (!enc) return ENOMEM;\n"));
            CHECK_IO(fprintf(fp,
                             "    if (asprintf(&path_%s, \"%s%%s\", enc) == "
                             "-1) { free(enc); return ENOMEM; }\n",
                             name, prefix));
            CHECK_IO(fprintf(fp, "    free(enc);\n  }\n"));
          } else if (strcmp(p->type, "integer") == 0) {
            CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
            CHECK_IO(fprintf(fp, "    sprintf(num_buf, \"%%d\", %s);\n", name));
            CHECK_IO(fprintf(fp,
                             "    if (asprintf(&path_%s, \"%s%%s\", num_buf) "
                             "== -1) return ENOMEM;\n",
                             name, prefix));
            CHECK_IO(fprintf(fp, "  }\n"));
          } else if (strcmp(p->type, "number") == 0) {
            CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
            CHECK_IO(fprintf(fp, "    sprintf(num_buf, \"%%g\", %s);\n", name));
            CHECK_IO(fprintf(fp,
                             "    if (asprintf(&path_%s, \"%s%%s\", num_buf) "
                             "== -1) return ENOMEM;\n",
                             name, prefix));
            CHECK_IO(fprintf(fp, "  }\n"));
          } else if (strcmp(p->type, "boolean") == 0) {
            CHECK_IO(fprintf(fp,
                             "  if (asprintf(&path_%s, \"%s%%s\", %s ? "
                             "\"true\" : \"false\") == -1) return ENOMEM;\n",
                             name, prefix, name));
          } else {
            CHECK_IO(fprintf(fp,
                             "  if (asprintf(&path_%s, \"%s%%s\", %s) == -1) "
                             "return ENOMEM;\n",
                             name, prefix, name));
          }
        }
      }
    }
  }

  CHECK_IO(fprintf(fp, "  if (asprintf(&%s, \"%%s", out_var));

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      CHECK_IO(fprintf(fp, "%%s"));
    } else {
      CHECK_IO(fprintf(fp, "%s", segs[i].text));
    }
  }

  CHECK_IO(fprintf(fp, "\", %s", base_var));

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p =
          find_param(segs[i].text, params, n_params);
      if (p && p->name) {
        CHECK_IO(fprintf(fp, ", path_%s", p->name));
      } else {
        CHECK_IO(fprintf(fp, ", %s", segs[i].text));
      }
    }
  }
  CHECK_IO(fprintf(fp, ") == -1) {\n"));

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p =
          find_param(segs[i].text, params, n_params);
      if (p && p->name) {
        CHECK_IO(fprintf(fp, "    free(path_%s);\n", p->name));
      }
    }
  }
  CHECK_IO(fprintf(fp, "    return ENOMEM;\n  }\n"));

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p =
          find_param(segs[i].text, params, n_params);
      if (p && p->name) {
        CHECK_IO(fprintf(fp, "  free(path_%s);\n", p->name));
      }
    }
  }

  for (i = 0; i < n_segs; ++i)
    free(segs[i].text);
  free(segs);

  return 0;
}

int codegen_url_write_query_params(FILE *const fp,
                                   const struct OpenAPI_Operation *op,
                                   int qp_tracking) {
  size_t i;
  int has_query = 0;
  const struct OpenAPI_Parameter *querystring_param = NULL;

  if (!fp || !op)
    return EINVAL;

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_QUERYSTRING) {
      querystring_param = &op->parameters[i];
      break;
    }
  }

  if (querystring_param) {
    const char *qs_name =
        querystring_param->name ? querystring_param->name : "querystring";
    const char *qs_json_item =
        querystring_param_json_array_item_type(querystring_param);
    const char *qs_json_obj =
        querystring_param_json_array_item_ref(querystring_param);
    const char *qs_json_prim =
        querystring_param_json_primitive_type(querystring_param);
    if (querystring_param_is_form_object(querystring_param)) {
      CHECK_IO(fprintf(fp, "  /* Querystring Parameter (form object): %s */\n",
                       qs_name));
      CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", qs_name, qs_name));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_form_body = NULL;\n"));
      CHECK_IO(fprintf(fp, "    rc = url_query_init(&qp);\n"));
      CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", qs_name));
      CHECK_IO(fprintf(fp, "      const struct OpenAPI_KV *kv = &%s[i];\n",
                       qs_name));
      CHECK_IO(fprintf(fp, "      const char *kv_key = kv->key;\n"));
      CHECK_IO(fprintf(fp, "      const char *kv_raw = NULL;\n"));
      CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
      CHECK_IO(fprintf(fp, "      switch (kv->type) {\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_STRING:\n"));
      CHECK_IO(fprintf(fp, "        kv_raw = kv->value.s;\n        break;\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_INTEGER:\n"
                           "        sprintf(num_buf, \"%%d\", kv->value.i);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                           "        sprintf(num_buf, \"%%g\", kv->value.n);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));
      CHECK_IO(fprintf(fp,
                       "      case OA_KV_BOOLEAN:\n"
                       "        kv_raw = kv->value.b ? \"true\" : \"false\";\n"
                       "        break;\n"));
      CHECK_IO(fprintf(
          fp, "      default:\n        kv_raw = NULL;\n        break;\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
      CHECK_IO(fprintf(fp, "      if (!kv_key || !kv_raw) continue;\n"));
      CHECK_IO(fprintf(fp, "      rc = url_query_add(&qp, kv_key, kv_raw);\n"));
      CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "    rc = url_query_build_form(&qp, "
                           "&qs_form_body);\n"));
      CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    if (qs_form_body && qs_form_body[0] != "
                           "'\\0') {\n"));
      CHECK_IO(fprintf(fp, "      if (asprintf(&query_str, \"?%%s\", "
                           "qs_form_body) == -1) { rc = ENOMEM; goto cleanup; "
                           "}\n"));
      CHECK_IO(fprintf(fp, "    } else {\n"));
      CHECK_IO(fprintf(fp, "      query_str = strdup(\"\");\n"));
      CHECK_IO(fprintf(fp, "      if (!query_str) { rc = ENOMEM; goto cleanup; "
                           "}\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "    free(qs_form_body);\n"));
      CHECK_IO(fprintf(fp, "  } else {\n"));
      CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
      CHECK_IO(
          fprintf(fp, "    if (!query_str) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "  }\n\n"));
      return 0;
    }
    if (qs_json_obj) {
      CHECK_IO(fprintf(
          fp, "  /* Querystring Parameter (json array objects): %s */\n",
          qs_name));
      CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", qs_name, qs_name));
      CHECK_IO(fprintf(fp, "    JSON_Value *qs_val = NULL;\n"));
      CHECK_IO(fprintf(fp, "    JSON_Array *qs_arr = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    qs_val = json_value_init_array();\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_val) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    qs_arr = json_value_get_array(qs_val);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_arr) { rc = EINVAL; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    for (i = 0; i < %s_len; ++i) {\n", qs_name));
      CHECK_IO(fprintf(fp, "      char *item_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "      JSON_Value *item_val = NULL;\n"));
      CHECK_IO(fprintf(fp, "      if (!%s[i]) {\n", qs_name));
      CHECK_IO(fprintf(fp, "        if (json_array_append_null(qs_arr) != "
                           "JSONSuccess) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "        continue;\n"));
      CHECK_IO(fprintf(fp, "      }\n"));
      CHECK_IO(fprintf(fp, "      rc = %s_to_json(%s[i], &item_json);\n",
                       qs_json_obj, qs_name));
      CHECK_IO(fprintf(fp, "      if (rc != 0) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "      item_val = json_parse_string(item_json);\n"));
      CHECK_IO(fprintf(fp, "      free(item_json);\n"));
      CHECK_IO(
          fprintf(fp, "      if (!item_val) { rc = EINVAL; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp,
                       "      if (json_array_append_value(qs_arr, item_val) "
                       "!= JSONSuccess) { json_value_free(item_val); rc = "
                       "ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(
          fprintf(fp, "    qs_json = json_serialize_to_string(qs_val);\n"));
      CHECK_IO(fprintf(fp, "    json_value_free(qs_val);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_json) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    qs_enc = url_encode(qs_json);\n"));
      CHECK_IO(fprintf(fp, "    json_free_serialized_string(qs_json);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_enc) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp,
                       "    if (asprintf(&query_str, \"?%%s\", qs_enc) == -1) "
                       "{ rc = ENOMEM; free(qs_enc); goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
      CHECK_IO(fprintf(fp, "  } else {\n"));
      CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
      CHECK_IO(
          fprintf(fp, "    if (!query_str) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "  }\n\n"));
      return 0;
    }
    if (qs_json_item) {
      CHECK_IO(fprintf(fp, "  /* Querystring Parameter (json array): %s */\n",
                       qs_name));
      CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", qs_name, qs_name));
      CHECK_IO(fprintf(fp, "    JSON_Value *qs_val = NULL;\n"));
      CHECK_IO(fprintf(fp, "    JSON_Array *qs_arr = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    qs_val = json_value_init_array();\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_val) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    qs_arr = json_value_get_array(qs_val);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_arr) { rc = EINVAL; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    for (i = 0; i < %s_len; ++i) {\n", qs_name));
      if (strcmp(qs_json_item, "string") == 0) {
        CHECK_IO(fprintf(fp, "      if (!%s[i]) {\n", qs_name));
        CHECK_IO(fprintf(fp, "        if (json_array_append_null(qs_arr) != "
                             "JSONSuccess) { rc = ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "      } else {\n"));
        CHECK_IO(fprintf(fp,
                         "        if (json_array_append_string(qs_arr, %s[i]) "
                         "!= JSONSuccess) { rc = ENOMEM; goto cleanup; }\n",
                         qs_name));
        CHECK_IO(fprintf(fp, "      }\n"));
      } else if (strcmp(qs_json_item, "integer") == 0) {
        CHECK_IO(
            fprintf(fp,
                    "      if (json_array_append_number(qs_arr, (double)%s[i]) "
                    "!= JSONSuccess) { rc = ENOMEM; goto cleanup; }\n",
                    qs_name));
      } else if (strcmp(qs_json_item, "number") == 0) {
        CHECK_IO(fprintf(fp,
                         "      if (json_array_append_number(qs_arr, %s[i]) "
                         "!= JSONSuccess) { rc = ENOMEM; goto cleanup; }\n",
                         qs_name));
      } else if (strcmp(qs_json_item, "boolean") == 0) {
        CHECK_IO(fprintf(
            fp,
            "      if (json_array_append_boolean(qs_arr, %s[i] ? 1 : 0) "
            "!= JSONSuccess) { rc = ENOMEM; goto cleanup; }\n",
            qs_name));
      } else {
        CHECK_IO(fprintf(fp, "      rc = EINVAL; goto cleanup;\n"));
      }
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(
          fprintf(fp, "    qs_json = json_serialize_to_string(qs_val);\n"));
      CHECK_IO(fprintf(fp, "    json_value_free(qs_val);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_json) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    qs_enc = url_encode(qs_json);\n"));
      CHECK_IO(fprintf(fp, "    json_free_serialized_string(qs_json);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_enc) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp,
                       "    if (asprintf(&query_str, \"?%%s\", qs_enc) == -1) "
                       "{ rc = ENOMEM; free(qs_enc); goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
      CHECK_IO(fprintf(fp, "  } else {\n"));
      CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
      CHECK_IO(
          fprintf(fp, "    if (!query_str) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "  }\n\n"));
      return 0;
    }
    if (qs_json_prim) {
      CHECK_IO(fprintf(
          fp, "  /* Querystring Parameter (json primitive): %s */\n", qs_name));
      if (strcmp(qs_json_prim, "string") == 0) {
        CHECK_IO(fprintf(fp, "  if (%s) {\n", qs_name));
      } else {
        CHECK_IO(fprintf(fp, "  {\n"));
      }
      CHECK_IO(fprintf(fp, "    JSON_Value *qs_val = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
      if (strcmp(qs_json_prim, "string") == 0) {
        CHECK_IO(
            fprintf(fp, "    qs_val = json_value_init_string(%s);\n", qs_name));
      } else if (strcmp(qs_json_prim, "integer") == 0) {
        CHECK_IO(fprintf(
            fp, "    qs_val = json_value_init_number((double)%s);\n", qs_name));
      } else if (strcmp(qs_json_prim, "number") == 0) {
        CHECK_IO(
            fprintf(fp, "    qs_val = json_value_init_number(%s);\n", qs_name));
      } else if (strcmp(qs_json_prim, "boolean") == 0) {
        CHECK_IO(fprintf(fp,
                         "    qs_val = json_value_init_boolean(%s ? 1 : 0);\n",
                         qs_name));
      } else {
        CHECK_IO(fprintf(fp, "    rc = EINVAL; goto cleanup;\n"));
      }
      CHECK_IO(
          fprintf(fp, "    if (!qs_val) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(
          fprintf(fp, "    qs_json = json_serialize_to_string(qs_val);\n"));
      CHECK_IO(fprintf(fp, "    json_value_free(qs_val);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_json) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    qs_enc = url_encode(qs_json);\n"));
      CHECK_IO(fprintf(fp, "    json_free_serialized_string(qs_json);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_enc) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp,
                       "    if (asprintf(&query_str, \"?%%s\", qs_enc) == -1) "
                       "{ rc = ENOMEM; free(qs_enc); goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
      if (strcmp(qs_json_prim, "string") == 0) {
        CHECK_IO(fprintf(fp, "  } else {\n"));
        CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
        CHECK_IO(fprintf(
            fp, "    if (!query_str) { rc = ENOMEM; goto cleanup; }\n"));
        CHECK_IO(fprintf(fp, "  }\n\n"));
      } else {
        CHECK_IO(fprintf(fp, "  }\n\n"));
      }
      return 0;
    }
    if (querystring_param_is_json_ref(querystring_param)) {
      CHECK_IO(
          fprintf(fp, "  /* Querystring Parameter (json): %s */\n", qs_name));
      CHECK_IO(fprintf(fp, "  if (%s) {\n", qs_name));
      CHECK_IO(fprintf(fp, "    char *qs_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
      CHECK_IO(fprintf(fp, "    rc = %s_to_json(%s, &qs_json);\n",
                       querystring_param->schema.ref_name, qs_name));
      CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    qs_enc = url_encode(qs_json);\n"));
      CHECK_IO(fprintf(fp, "    free(qs_json);\n"));
      CHECK_IO(
          fprintf(fp, "    if (!qs_enc) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp,
                       "    if (asprintf(&query_str, \"?%%s\", qs_enc) == -1) "
                       "{ rc = ENOMEM; free(qs_enc); goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
      CHECK_IO(fprintf(fp, "  } else {\n"));
      CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
      CHECK_IO(
          fprintf(fp, "    if (!query_str) { rc = ENOMEM; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "  }\n\n"));
      return 0;
    }
    {
      const char *qs_raw =
          querystring_param_raw_primitive_type(querystring_param);
      if (qs_raw) {
        CHECK_IO(
            fprintf(fp, "  /* Querystring Parameter (raw): %s */\n", qs_name));
        if (strcmp(qs_raw, "string") == 0) {
          CHECK_IO(fprintf(fp, "  if (%s) {\n", qs_name));
          CHECK_IO(
              fprintf(fp, "    char *qs_enc = url_encode(%s);\n", qs_name));
          CHECK_IO(
              fprintf(fp, "    if (!qs_enc) { rc = ENOMEM; goto cleanup; }\n"));
          CHECK_IO(fprintf(fp,
                           "    if (asprintf(&query_str, \"?%%s\", qs_enc) "
                           "== -1) { rc = ENOMEM; free(qs_enc); goto cleanup; "
                           "}\n"));
          CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
          CHECK_IO(fprintf(fp, "  } else {\n"));
          CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
          CHECK_IO(fprintf(
              fp, "    if (!query_str) { rc = ENOMEM; goto cleanup; }\n"));
          CHECK_IO(fprintf(fp, "  }\n\n"));
        } else if (strcmp(qs_raw, "integer") == 0) {
          CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
          CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
          CHECK_IO(
              fprintf(fp, "    sprintf(num_buf, \"%%d\", %s);\n", qs_name));
          CHECK_IO(fprintf(fp, "    qs_enc = url_encode(num_buf);\n"));
          CHECK_IO(
              fprintf(fp, "    if (!qs_enc) { rc = ENOMEM; goto cleanup; }\n"));
          CHECK_IO(fprintf(fp,
                           "    if (asprintf(&query_str, \"?%%s\", qs_enc) "
                           "== -1) { rc = ENOMEM; free(qs_enc); goto cleanup; "
                           "}\n"));
          CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
          CHECK_IO(fprintf(fp, "  }\n\n"));
        } else if (strcmp(qs_raw, "number") == 0) {
          CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
          CHECK_IO(fprintf(fp, "    char *qs_enc = NULL;\n"));
          CHECK_IO(
              fprintf(fp, "    sprintf(num_buf, \"%%g\", %s);\n", qs_name));
          CHECK_IO(fprintf(fp, "    qs_enc = url_encode(num_buf);\n"));
          CHECK_IO(
              fprintf(fp, "    if (!qs_enc) { rc = ENOMEM; goto cleanup; }\n"));
          CHECK_IO(fprintf(fp,
                           "    if (asprintf(&query_str, \"?%%s\", qs_enc) "
                           "== -1) { rc = ENOMEM; free(qs_enc); goto cleanup; "
                           "}\n"));
          CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
          CHECK_IO(fprintf(fp, "  }\n\n"));
        } else if (strcmp(qs_raw, "boolean") == 0) {
          CHECK_IO(fprintf(fp, "  {\n"));
          CHECK_IO(fprintf(
              fp, "    const char *raw_val = %s ? \"true\" : \"false\";\n",
              qs_name));
          CHECK_IO(fprintf(fp, "    char *qs_enc = url_encode(raw_val);\n"));
          CHECK_IO(
              fprintf(fp, "    if (!qs_enc) { rc = ENOMEM; goto cleanup; }\n"));
          CHECK_IO(fprintf(fp,
                           "    if (asprintf(&query_str, \"?%%s\", qs_enc) "
                           "== -1) { rc = ENOMEM; free(qs_enc); goto cleanup; "
                           "}\n"));
          CHECK_IO(fprintf(fp, "    free(qs_enc);\n"));
          CHECK_IO(fprintf(fp, "  }\n\n"));
        } else {
          CHECK_IO(fprintf(fp, "  rc = EINVAL; goto cleanup;\n"));
        }
        return 0;
      }
    }
    CHECK_IO(fprintf(fp, "  rc = url_query_init(&qp);\n"));
    CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  /* Querystring Parameter: %s */\n", qs_name));
    CHECK_IO(fprintf(fp, "  if (%s && %s[0] != '\\0') {\n", qs_name, qs_name));
    CHECK_IO(fprintf(fp, "    if (%s[0] == '?') {\n", qs_name));
    CHECK_IO(fprintf(fp, "      query_str = strdup(%s);\n", qs_name));
    CHECK_IO(fprintf(fp, "      if (!query_str) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "    } else {\n"));
    CHECK_IO(fprintf(fp,
                     "      if (asprintf(&query_str, \"?%%s\", %s) == -1) "
                     "{ rc = ENOMEM; goto cleanup; }\n",
                     qs_name));
    CHECK_IO(fprintf(fp, "    }\n"));
    CHECK_IO(fprintf(fp, "  } else {\n"));
    CHECK_IO(fprintf(fp, "    query_str = strdup(\"\");\n"));
    CHECK_IO(fprintf(fp, "    if (!query_str) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n\n"));
    return 0;
  }

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].in == OA_PARAM_IN_QUERY) {
      const struct OpenAPI_Parameter *p = &op->parameters[i];
      enum OpenAPI_Style style =
          (p->style == OA_STYLE_UNKNOWN) ? OA_STYLE_FORM : p->style;
      int explode =
          p->explode_set
              ? p->explode
              : ((style == OA_STYLE_FORM || style == OA_STYLE_COOKIE) ? 1 : 0);

      if (!has_query) {
        if (qp_tracking) {
          CHECK_IO(fprintf(fp, "  if (!qp_initialized) {\n"));
          CHECK_IO(fprintf(fp, "    rc = url_query_init(&qp);\n"));
          CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
          CHECK_IO(fprintf(fp, "    qp_initialized = 1;\n"));
          CHECK_IO(fprintf(fp, "  }\n"));
        } else {
          CHECK_IO(fprintf(fp, "  rc = url_query_init(&qp);\n"));
          CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
        }
        has_query = 1;
      }

      CHECK_IO(fprintf(fp, "  /* Query Parameter: %s */\n", p->name));

      if (p->content_type && media_type_is_json(p->content_type)) {
        int rc2 = write_query_json_param(fp, p);
        if (rc2 != 0)
          return rc2;
        continue;
      }

      if (param_is_object_kv(p)) {
        int rc2 = write_query_object_param(fp, p);
        if (rc2 != 0)
          return rc2;
        continue;
      }

      if (p->is_array) {
        if (style == OA_STYLE_FORM) {
          if (explode) {
            /* === form + explode=true === */
            CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
            CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));

            if (p->items_type && strcmp(p->items_type, "string") == 0) {
              if (p->allow_reserved_set && p->allow_reserved) {
                CHECK_IO(fprintf(
                    fp, "      char *enc = url_encode_allow_reserved(%s[i]);\n",
                    p->name));
                CHECK_IO(fprintf(
                    fp, "      if (!enc) { rc = ENOMEM; goto cleanup; }\n"));
                CHECK_IO(fprintf(
                    fp, "      rc = url_query_add_encoded(&qp, \"%s\", enc);\n",
                    p->name));
                CHECK_IO(fprintf(fp, "      free(enc);\n"));
              } else {
                CHECK_IO(fprintf(
                    fp, "      rc = url_query_add(&qp, \"%s\", %s[i]);\n",
                    p->name, p->name));
              }
            } else if (p->items_type && strcmp(p->items_type, "integer") == 0) {
              CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
              CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", %s[i]);\n",
                               p->name));
              CHECK_IO(fprintf(
                  fp, "      rc = url_query_add(&qp, \"%s\", num_buf);\n",
                  p->name));
            } else if (p->items_type && strcmp(p->items_type, "number") == 0) {
              CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
              CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%g\", %s[i]);\n",
                               p->name));
              CHECK_IO(fprintf(
                  fp, "      rc = url_query_add(&qp, \"%s\", num_buf);\n",
                  p->name));
            } else if (p->items_type && strcmp(p->items_type, "boolean") == 0) {
              CHECK_IO(fprintf(fp,
                               "      rc = url_query_add(&qp, \"%s\", %s[i] ? "
                               "\"true\" : \"false\");\n",
                               p->name, p->name));
            }
            CHECK_IO(
                fprintf(fp, "      if (rc != 0) goto cleanup;\n    }\n  }\n"));
          } else {
            /* === form + explode=false (CSV) === */
            {
              const char *encode_fn =
                  (p->allow_reserved_set && p->allow_reserved)
                      ? "url_encode_allow_reserved"
                      : "url_encode";
              int rc2 = write_joined_query_array(fp, p, ',', encode_fn, 1);
              if (rc2 != 0)
                return rc2;
            }
          }
        } else if (style == OA_STYLE_SPACE_DELIMITED) {
          /* === spaceDelimited (explode n/a) === */
          if (p->allow_reserved_set && p->allow_reserved) {
            int rc2 = write_joined_query_array_encoded_delim(
                fp, p, "%20", "url_encode_allow_reserved");
            if (rc2 != 0)
              return rc2;
          } else {
            int rc2 = write_joined_query_array(fp, p, ' ', NULL, 0);
            if (rc2 != 0)
              return rc2;
          }
        } else if (style == OA_STYLE_PIPE_DELIMITED) {
          /* === pipeDelimited (explode n/a) === */
          if (p->allow_reserved_set && p->allow_reserved) {
            int rc2 = write_joined_query_array_encoded_delim(
                fp, p, "%7C", "url_encode_allow_reserved");
            if (rc2 != 0)
              return rc2;
          } else {
            int rc2 = write_joined_query_array(fp, p, '|', NULL, 0);
            if (rc2 != 0)
              return rc2;
          }
        } else if (explode) {
          /* === fallback explode=true === */
          CHECK_IO(fprintf(fp, "  {\n    size_t i;\n"));
          CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", p->name));
          if (p->items_type && strcmp(p->items_type, "string") == 0) {
            if (p->allow_reserved_set && p->allow_reserved) {
              CHECK_IO(fprintf(
                  fp, "      char *enc = url_encode_allow_reserved(%s[i]);\n",
                  p->name));
              CHECK_IO(fprintf(
                  fp, "      if (!enc) { rc = ENOMEM; goto cleanup; }\n"));
              CHECK_IO(fprintf(
                  fp, "      rc = url_query_add_encoded(&qp, \"%s\", enc);\n",
                  p->name));
              CHECK_IO(fprintf(fp, "      free(enc);\n"));
            } else {
              CHECK_IO(
                  fprintf(fp, "      rc = url_query_add(&qp, \"%s\", %s[i]);\n",
                          p->name, p->name));
            }
          } else if (p->items_type && strcmp(p->items_type, "integer") == 0) {
            CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
            CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%d\", %s[i]);\n",
                             p->name));
            CHECK_IO(
                fprintf(fp, "      rc = url_query_add(&qp, \"%s\", num_buf);\n",
                        p->name));
          } else if (p->items_type && strcmp(p->items_type, "number") == 0) {
            CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
            CHECK_IO(fprintf(fp, "      sprintf(num_buf, \"%%g\", %s[i]);\n",
                             p->name));
            CHECK_IO(
                fprintf(fp, "      rc = url_query_add(&qp, \"%s\", num_buf);\n",
                        p->name));
          } else if (p->items_type && strcmp(p->items_type, "boolean") == 0) {
            CHECK_IO(fprintf(fp,
                             "      rc = url_query_add(&qp, \"%s\", %s[i] ? "
                             "\"true\" : \"false\");\n",
                             p->name, p->name));
          }
          CHECK_IO(
              fprintf(fp, "      if (rc != 0) goto cleanup;\n    }\n  }\n"));
        } else {
          CHECK_IO(fprintf(fp, "  /* Array style not yet supported for %s */\n",
                           p->name));
        }
      } else {
        /* === Scalar === */
        if (strcmp(p->type, "string") == 0) {
          CHECK_IO(fprintf(fp, "  if (%s) {\n", p->name));
          if (p->allow_reserved_set && p->allow_reserved) {
            CHECK_IO(fprintf(fp,
                             "    char *enc = url_encode_allow_reserved(%s);\n",
                             p->name));
            CHECK_IO(
                fprintf(fp, "    if (!enc) { rc = ENOMEM; goto cleanup; }\n"));
            CHECK_IO(fprintf(
                fp, "    rc = url_query_add_encoded(&qp, \"%s\", enc);\n",
                p->name));
            CHECK_IO(fprintf(fp, "    free(enc);\n"));
          } else {
            CHECK_IO(fprintf(fp, "    rc = url_query_add(&qp, \"%s\", %s);\n",
                             p->name, p->name));
          }
          CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
        } else if (strcmp(p->type, "integer") == 0) {
          CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
          CHECK_IO(
              fprintf(fp, "    sprintf(num_buf, \"%%d\", %s);\n", p->name));
          CHECK_IO(fprintf(
              fp, "    rc = url_query_add(&qp, \"%s\", num_buf);\n", p->name));
          CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
        } else if (strcmp(p->type, "number") == 0) {
          CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
          CHECK_IO(
              fprintf(fp, "    sprintf(num_buf, \"%%g\", %s);\n", p->name));
          CHECK_IO(fprintf(
              fp, "    rc = url_query_add(&qp, \"%s\", num_buf);\n", p->name));
          CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n  }\n"));
        } else if (strcmp(p->type, "boolean") == 0) {
          CHECK_IO(fprintf(fp,
                           "  rc = url_query_add(&qp, \"%s\", %s ? \"true\" : "
                           "\"false\");\n",
                           p->name, p->name));
          CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n"));
        }
      }
    }
  }

  if (has_query) {
    CHECK_IO(fprintf(fp, "  rc = url_query_build(&qp, &query_str);\n"));
    CHECK_IO(fprintf(fp, "  if (rc != 0) goto cleanup;\n\n"));
  } else if (qp_tracking) {
    CHECK_IO(fprintf(fp, "  if (qp_initialized) {\n"));
    CHECK_IO(fprintf(fp, "    rc = url_query_build(&qp, &query_str);\n"));
    CHECK_IO(fprintf(fp, "    if (rc != 0) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n\n"));
  }

  return 0;
}
