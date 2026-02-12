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
          {
            int rc2 = write_joined_query_array(fp, p, ' ', NULL, 0);
            if (rc2 != 0)
              return rc2;
          }
        } else if (style == OA_STYLE_PIPE_DELIMITED) {
          /* === pipeDelimited (explode n/a) === */
          {
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
