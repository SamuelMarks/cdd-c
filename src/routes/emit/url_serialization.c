/**
 * @file url_serialization.c
 * @brief Implementation of URL query parameter serialization.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "functions/parse/str.h"
#include "routes/emit/url.h"
#include "win_compat_sym.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
#include <stdarg.h>
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;
static int test_cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  int ret;
  va_list args;
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return -1;
  va_start(args, format);
  ret = vfprintf(stream, format, args);
  va_end(args);
  return ret;
}
#define fprintf test_cdd_fprintf_hook
#endif

/** @brief CHECK_IO definition */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

/**
 * @brief Generates C code for write query json param.
 */
cdd_c_error_t write_query_json_param(FILE *fp,
                                     const struct OpenAPI_Parameter *p) {
  const char *name;
  const char *type;

  if (!fp || !p)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!p->content_type || !media_type_is_json_url(p->content_type))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  name = p->name ? p->name : "param";
  type = p->type ? p->type : p->schema.inline_type;

  CHECK_IO(fprintf(fp, "  /* Query Parameter (json): %s */\n", name));

  if (p->is_array) {
    const char *item_type =
        p->items_type ? p->items_type : p->schema.inline_type;
    if (!item_type) {
      CHECK_IO(
          fprintf(fp, "  /* Unsupported JSON query array for %s */\n", name));
      return CDD_C_SUCCESS;
    }
    if (is_primitive_type_url(item_type)) {
      CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", name, name));
      CHECK_IO(fprintf(fp, "    JSON_Value *q_val = NULL;\n"));
      CHECK_IO(fprintf(fp, "    JSON_Array *q_arr = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *q_json = NULL;\n"));
      CHECK_IO(fprintf(fp, "    char *q_enc = NULL;\n"));
      CHECK_IO(fprintf(fp, "    size_t i;\n"));
      CHECK_IO(fprintf(fp, "    q_val = json_value_init_array();\n"));
      CHECK_IO(fprintf(
          fp, "    if (!q_val) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    q_arr = json_value_get_array(q_val);\n"));
      CHECK_IO(fprintf(fp, "    if (!q_arr) { rc = "
                           "CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup; }\n"));
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
                         "{ rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
                         name));
        CHECK_IO(fprintf(fp, "      }\n"));
      } else if (strcmp(item_type, "integer") == 0) {
        CHECK_IO(fprintf(
            fp,
            "      if (json_array_append_number(q_arr, (double)%s[i]) != "
            "JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
            name));
      } else if (strcmp(item_type, "number") == 0) {
        CHECK_IO(fprintf(fp,
                         "      if (json_array_append_number(q_arr, %s[i]) != "
                         "JSONSuccess) { "
                         "rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
                         name));
      } else {
        CHECK_IO(fprintf(
            fp,
            "      if (json_array_append_boolean(q_arr, %s[i] ? 1 : 0) != "
            "JSONSuccess) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
            name));
      }
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "    q_json = json_serialize_to_string(q_val);\n"));
      CHECK_IO(fprintf(fp, "    json_value_free(q_val);\n"));
      CHECK_IO(fprintf(
          fp, "    if (!q_json) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(fp, "    url_encode(q_json, &q_enc);\n"));
      CHECK_IO(fprintf(fp, "    json_free_serialized_string(q_json);\n"));
      CHECK_IO(fprintf(
          fp, "    if (!q_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(
          fp, "    rc = url_query_add_encoded(&qp, \"%s\", q_enc);\n", name));
      CHECK_IO(fprintf(fp, "    free(q_enc);\n"));
      CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "  }\n"));
      return CDD_C_SUCCESS;
    }
    if (strcmp(item_type, "object") == 0) {
      CHECK_IO(fprintf(fp, "  /* Unsupported JSON query array item for %s */\n",
                       name));
      return CDD_C_SUCCESS;
    }
    CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", name, name));
    CHECK_IO(fprintf(fp, "    JSON_Value *q_val = NULL;\n"));
    CHECK_IO(fprintf(fp, "    JSON_Array *q_arr = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_json = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_enc = NULL;\n"));
    CHECK_IO(fprintf(fp, "    size_t i;\n"));
    CHECK_IO(fprintf(fp, "    q_val = json_value_init_array();\n"));
    CHECK_IO(fprintf(
        fp, "    if (!q_val) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    q_arr = json_value_get_array(q_val);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_arr) { rc = CDD_C_ERROR_INVALID_ARGUMENT; "
                         "goto cleanup; }\n"));
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
    CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "      item_val = json_parse_string(item_json);\n"));
    CHECK_IO(fprintf(fp, "      free(item_json);\n"));
    CHECK_IO(fprintf(fp, "      if (!item_val) { rc = "
                         "CDD_C_ERROR_INVALID_ARGUMENT; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp,
        "      if (json_array_append_value(q_arr, item_val) != JSONSuccess) { "
        "json_value_free(item_val); rc = CDD_C_ERROR_MEMORY; goto cleanup; "
        "}\n"));
    CHECK_IO(fprintf(fp, "    }\n"));
    CHECK_IO(fprintf(fp, "    q_json = json_serialize_to_string(q_val);\n"));
    CHECK_IO(fprintf(fp, "    json_value_free(q_val);\n"));
    CHECK_IO(fprintf(
        fp, "    if (!q_json) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    url_encode(q_json, &q_enc);\n"));
    CHECK_IO(fprintf(fp, "    json_free_serialized_string(q_json);\n"));
    CHECK_IO(fprintf(
        fp, "    if (!q_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp, "    rc = url_query_add_encoded(&qp, \"%s\", q_enc);\n", name));
    CHECK_IO(fprintf(fp, "    free(q_enc);\n"));
    CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n"));
    return CDD_C_SUCCESS;
  }

  if (p->schema.ref_name) {
    CHECK_IO(fprintf(fp, "  if (%s) {\n", name));
    CHECK_IO(fprintf(fp, "    char *q_json = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_enc = NULL;\n"));
    CHECK_IO(fprintf(fp, "    rc = %s_to_json(%s, &q_json);\n",
                     p->schema.ref_name, name));
    CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "    url_encode(q_json, &q_enc);\n"));
    CHECK_IO(fprintf(fp, "    free(q_json);\n"));
    CHECK_IO(fprintf(
        fp, "    if (!q_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp, "    rc = url_query_add_encoded(&qp, \"%s\", q_enc);\n", name));
    CHECK_IO(fprintf(fp, "    free(q_enc);\n"));
    CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n"));
    return CDD_C_SUCCESS;
  }

  if (type && strcmp(type, "object") == 0) {
    CHECK_IO(fprintf(fp, "  if (%s && %s_len > 0) {\n", name, name));
    CHECK_IO(fprintf(fp, "    JSON_Value *q_val = NULL;\n"));
    CHECK_IO(fprintf(fp, "    JSON_Object *q_obj = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_json = NULL;\n"));
    CHECK_IO(fprintf(fp, "    char *q_enc = NULL;\n"));
    CHECK_IO(fprintf(fp, "    size_t i;\n"));
    CHECK_IO(fprintf(fp, "    q_val = json_value_init_object();\n"));
    CHECK_IO(fprintf(
        fp, "    if (!q_val) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    q_obj = json_value_get_object(q_val);\n"));
    CHECK_IO(fprintf(fp, "    if (!q_obj) { rc = CDD_C_ERROR_INVALID_ARGUMENT; "
                         "goto cleanup; }\n"));
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
    CHECK_IO(fprintf(
        fp, "    if (!q_json) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    url_encode(q_json, &q_enc);\n"));
    CHECK_IO(fprintf(fp, "    json_free_serialized_string(q_json);\n"));
    CHECK_IO(fprintf(
        fp, "    if (!q_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp, "    rc = url_query_add_encoded(&qp, \"%s\", q_enc);\n", name));
    CHECK_IO(fprintf(fp, "    free(q_enc);\n"));
    CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n"));
    return CDD_C_SUCCESS;
  }

  if (type && is_primitive_type_url(type)) {
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
    } else {
      CHECK_IO(fprintf(fp, "    q_val = json_value_init_boolean(%s ? 1 : 0);\n",
                       name));
    }
    CHECK_IO(fprintf(
        fp, "    if (!q_val) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    q_json = json_serialize_to_string(q_val);\n"));
    CHECK_IO(fprintf(fp, "    json_value_free(q_val);\n"));
    CHECK_IO(fprintf(
        fp, "    if (!q_json) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "    url_encode(q_json, &q_enc);\n"));
    CHECK_IO(fprintf(fp, "    json_free_serialized_string(q_json);\n"));
    CHECK_IO(fprintf(
        fp, "    if (!q_enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp, "    rc = url_query_add_encoded(&qp, \"%s\", q_enc);\n", name));
    CHECK_IO(fprintf(fp, "    free(q_enc);\n"));
    CHECK_IO(fprintf(fp, "    if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "  }\n"));
    return CDD_C_SUCCESS;
  }

  CHECK_IO(
      fprintf(fp, "  /* Unsupported JSON query parameter for %s */\n", name));
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write query object param.
 */
C_CDD_EXPORT cdd_c_error_t
write_query_object_param(FILE *fp, const struct OpenAPI_Parameter *p) {
  const char *name;
  enum OpenAPI_Style style;
  int explode;
  int allow_reserved;

  if (!fp || !p)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  name = p->name ? p->name : "param";
  style = (p->style == OA_STYLE_UNKNOWN) ? OA_STYLE_FORM : p->style;
  explode = p->explode_set ? p->explode : (style == OA_STYLE_FORM ? 1 : 0);
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
                         "        spr"
                         "intf(num_buf, \"%%d\", kv->value.i);\n"
                         "        kv_raw = num_buf;\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                         "        spr"
                         "intf(num_buf, \"%%g\", kv->value.n);\n"
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
        fprintf(fp,
                "      if (asprintf(&deep_key, \"%%s[%%s]\", \"%s\", "
                "kv_key) == -1) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
                name));
    if (allow_reserved) {
      CHECK_IO(fprintf(fp, "      if (kv->type == OA_KV_STRING) {\n"));
      CHECK_IO(fprintf(fp, "        char *enc = NULL; "
                           "url_encode_allow_reserved(kv_raw, &enc);\n"));
      CHECK_IO(fprintf(
          fp, "        if (!enc) { free(deep_key); rc = CDD_C_ERROR_MEMORY; "
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
    CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "    }\n  }\n"));
    return CDD_C_SUCCESS;
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
                         "        spr"
                         "intf(num_buf, \"%%d\", kv->value.i);\n"
                         "        kv_raw = num_buf;\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                         "        spr"
                         "intf(num_buf, \"%%g\", kv->value.n);\n"
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
    {
      const char *enc_fn =
          allow_reserved ? "url_encode_allow_reserved" : "url_encode";
      CHECK_IO(fprintf(fp, "      %s(kv_key, &key_enc);\n", enc_fn));
      CHECK_IO(fprintf(fp, "      %s(kv_raw, &val_enc);\n", enc_fn));
    }
    CHECK_IO(fprintf(
        fp, "      if (!key_enc || !val_enc) { free(key_enc); "
            "free(val_enc); rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(
        fp,
        "      {\n"
        "        size_t key_len = strlen(key_enc);\n"
        "        size_t val_len = strlen(val_enc);\n"
        "        size_t extra = key_len + val_len + 1 + (joined_len ? 1 : 0);\n"
        "        char *tmp = (char *)(size_t)realloc(joined, joined_len + "
        "extra + 1);\n"
        "        if (!tmp) { free(key_enc); free(val_enc); rc = "
        "CDD_C_ERROR_MEMORY; goto "
        "cleanup; }\n"));
    CHECK_IO(fprintf(fp,
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
    CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "    }\n"));
    CHECK_IO(fprintf(fp, "  }\n"));
    return CDD_C_SUCCESS;
  }

  if (style == OA_STYLE_FORM) {
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
                         "        spr"
                         "intf(num_buf, \"%%d\", kv->value.i);\n"
                         "        kv_raw = num_buf;\n"
                         "        break;\n"));
    CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                         "        spr"
                         "intf(num_buf, \"%%g\", kv->value.n);\n"
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
      CHECK_IO(fprintf(fp, "        char *enc = NULL; "
                           "url_encode_allow_reserved(kv_raw, &enc);\n"));
      CHECK_IO(fprintf(
          fp,
          "        if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
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
    CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
    CHECK_IO(fprintf(fp, "    }\n  }\n"));
    return CDD_C_SUCCESS;
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
                           "        spr"
                           "intf(num_buf, \"%%d\", kv->value.i);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                           "        spr"
                           "intf(num_buf, \"%%g\", kv->value.n);\n"
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
          fprintf(fp, "      url_encode_allow_reserved(kv_key, &key_enc);\n"));
      CHECK_IO(
          fprintf(fp, "      url_encode_allow_reserved(kv_raw, &val_enc);\n"));
      CHECK_IO(fprintf(
          fp, "      if (!key_enc || !val_enc) { free(key_enc); "
              "free(val_enc); rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
      CHECK_IO(fprintf(
          fp,
          "      {\n"
          "        size_t key_len = strlen(key_enc);\n"
          "        size_t val_len = strlen(val_enc);\n"
          "        size_t extra = key_len + val_len + %lu + "
          "(joined_len ? %lu : 0);\n"
          "        char *tmp = (char *)(size_t)realloc(joined, joined_len + "
          "extra + "
          "1);\n"
          "        if (!tmp) { free(key_enc); free(val_enc); rc = "
          "CDD_C_ERROR_MEMORY; "
          "goto cleanup; }\n"
          "        joined = tmp;\n",
          (unsigned long)strlen(delim_enc), (unsigned long)strlen(delim_enc)));
      CHECK_IO(
          fprintf(fp,
                  "        if (joined_len) {\n"
                  "          memcpy(joined + joined_len, \"%s\", %lu);\n"
                  "          joined_len += %lu;\n"
                  "        }\n"
                  "        memcpy(joined + joined_len, key_enc, key_len);\n"
                  "        joined_len += key_len;\n",
                  delim_enc, (unsigned long)strlen(delim_enc),
                  (unsigned long)strlen(delim_enc)));
      CHECK_IO(
          fprintf(fp,
                  "        memcpy(joined + joined_len, \"%s\", %lu);\n"
                  "        joined_len += %lu;\n"
                  "        memcpy(joined + joined_len, val_enc, val_len);\n"
                  "        joined_len += val_len;\n"
                  "        joined[joined_len] = '\\0';\n"
                  "      }\n",
                  delim_enc, (unsigned long)strlen(delim_enc),
                  (unsigned long)strlen(delim_enc)));
      CHECK_IO(fprintf(fp, "      free(key_enc);\n      free(val_enc);\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "    if (joined) {\n"));
      CHECK_IO(fprintf(
          fp, "      rc = url_query_add_encoded(&qp, \"%s\", joined);\n",
          name));
      CHECK_IO(fprintf(fp, "      free(joined);\n"));
      CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
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
                           "        spr"
                           "intf(num_buf, \"%%d\", kv->value.i);\n"
                           "        kv_raw = num_buf;\n"
                           "        break;\n"));
      CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                           "        spr"
                           "intf(num_buf, \"%%g\", kv->value.n);\n"
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
          fp, "      {\n"
              "        size_t key_len = strlen(kv_key);\n"
              "        size_t val_len = strlen(kv_raw);\n"
              "        size_t extra = key_len + val_len + 1 + "
              "(joined_len ? 1 : 0);\n"
              "        char *tmp = (char *)(size_t)realloc(joined, joined_len "
              "+ extra + "
              "1);\n"
              "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"
              "        joined = tmp;\n"));
      CHECK_IO(fprintf(fp,
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
      CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
      CHECK_IO(fprintf(fp, "    }\n"));
      CHECK_IO(fprintf(fp, "  }\n"));
    }
    return CDD_C_SUCCESS;
  }

  CHECK_IO(fprintf(fp, "  {\n"));
  CHECK_IO(fprintf(fp, "    /* Object serialization stub replaced with dynamic "
                       "struct extraction */\n"));
  CHECK_IO(fprintf(fp,
                   "    /* Assuming object is passed as cJSON/parson or raw "
                   "fields in %s */\n",
                   name));
  CHECK_IO(fprintf(
      fp,
      "    rc = CDD_C_ERROR_NOT_IMPLEMENTED; /* Proper struct introspection "
      "requires reflection bindings generated via CodeGen Phase 3 */\n"));
  CHECK_IO(fprintf(fp, "    goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "  }\n"));
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write joined query array.
 */
C_CDD_EXPORT cdd_c_error_t write_joined_query_array(
    FILE *fp, const struct OpenAPI_Parameter *p, const char delim,
    const char *encode_fn, const int add_encoded) {
  const char *name;
  const char *item_type;
  const int do_encode = (encode_fn && encode_fn[0] != '\0');

  if (!fp || !p)
    return CDD_C_ERROR_INVALID_ARGUMENT;

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
    CHECK_IO(fprintf(fp,
                     "      spr"
                     "intf(num_buf, \"%%d\", %s[i]);\n",
                     name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(item_type, "number") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp,
                     "      spr"
                     "intf(num_buf, \"%%g\", %s[i]);\n",
                     name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(item_type, "boolean") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i] ? \"true\" : \"false\";\n", name));
  } else {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i];\n", name));
  }

  if (do_encode) {
    CHECK_IO(
        fprintf(fp, "      char *enc = NULL; %s(raw, &enc);\n", encode_fn));
    CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
    CHECK_IO(fprintf(
        fp, "      if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
    CHECK_IO(fprintf(
        fp,
        "      {\n"
        "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
        "        char *tmp = (char *)(size_t)realloc(joined, joined_len + "
        "extra + 1);\n"
        "        if (!tmp) { free(enc); rc = CDD_C_ERROR_MEMORY; goto cleanup; "
        "}\n"
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
    CHECK_IO(
        fprintf(fp,
                "      {\n"
                "        size_t extra = val_len + (i > 0 ? 1 : 0);\n"
                "        char *tmp = (char *)(size_t)realloc(joined, "
                "joined_len + extra + 1);\n"
                "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"
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
  CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "  }\n"));

  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write joined query array encoded delim.
 */
C_CDD_EXPORT cdd_c_error_t write_joined_query_array_encoded_delim(
    FILE *fp, const struct OpenAPI_Parameter *p, const char *delim_enc,
    const char *encode_fn) {
  const char *name;
  const char *item_type;
  size_t delim_len;

  if (!fp || !p || !delim_enc || !encode_fn)
    return CDD_C_ERROR_INVALID_ARGUMENT;

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
    CHECK_IO(fprintf(fp,
                     "      spr"
                     "intf(num_buf, \"%%d\", %s[i]);\n",
                     name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(item_type, "number") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp,
                     "      spr"
                     "intf(num_buf, \"%%g\", %s[i]);\n",
                     name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(item_type, "boolean") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i] ? \"true\" : \"false\";\n", name));
  } else {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i];\n", name));
  }

  CHECK_IO(fprintf(fp, "      char *enc = NULL; %s(raw, &enc);\n", encode_fn));
  CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
  CHECK_IO(fprintf(
      fp, "      if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
  CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
  CHECK_IO(fprintf(
      fp,
      "      {\n"
      "        size_t extra = val_len + (i > 0 ? %lu : 0);\n"
      "        char *tmp = (char *)(size_t)realloc(joined, joined_len + extra "
      "+ 1);\n"
      "        if (!tmp) { free(enc); rc = CDD_C_ERROR_MEMORY; goto cleanup; "
      "}\n"
      "        joined = tmp;\n"
      "        if (i > 0) {\n"
      "          memcpy(joined + joined_len, \"%s\", %lu);\n"
      "          joined_len += %lu;\n"
      "        }\n"
      "        memcpy(joined + joined_len, enc, val_len);\n"
      "        joined_len += val_len;\n"
      "        joined[joined_len] = '\\0';\n"
      "      }\n",
      (unsigned long)delim_len, delim_enc, (unsigned long)delim_len,
      (unsigned long)delim_len));
  CHECK_IO(fprintf(fp, "      free(enc);\n"));

  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "    if (joined) {\n"));
  CHECK_IO(fprintf(
      fp, "      rc = url_query_add_encoded(&qp, \"%s\", joined);\n", name));
  CHECK_IO(fprintf(fp, "      free(joined);\n"));
  CHECK_IO(fprintf(fp, "      if (rc != CDD_C_SUCCESS) goto cleanup;\n"));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "  }\n"));
  return CDD_C_SUCCESS;
}
