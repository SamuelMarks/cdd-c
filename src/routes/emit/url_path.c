/**
 * @file url_path.c
 * @brief Implementation of URL path generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/code_writer.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "functions/parse/str.h"
#include "routes/emit/url.h"
#include "win_compat_sym.h"
/* clang-format on */

/** @brief fprintf macro alias for stream hook */
#define fprintf cdd_io_fprintf_hook

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
 * @brief Emits C code to serialize an object path parameter (`simple`, `label`,
 * `matrix`).
 *
 * @param[in,out] fp Output file stream.
 * @param[in] p The OpenAPI parameter structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
write_path_object_serialization(FILE *fp, const struct OpenAPI_Parameter *p) {
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
    return CDD_C_ERROR_INVALID_ARGUMENT;

  name = p->name ? p->name : "param";
  style = (p->style == OA_STYLE_UNKNOWN) ? OA_STYLE_SIMPLE : p->style;
  explode = p->explode_set ? p->explode : 0;

  if (style == OA_STYLE_LABEL) {
    prefix = ".";
    pair_delim = explode ? "." : ",";
  } else if (style == OA_STYLE_MATRIX) {
    if (explode) {
      prefix = ";";
      pair_delim = ";";
    } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      sprintf_s(buf_prefix, sizeof(buf_prefix), ";%s=", name);
#else
      CDD_SNPRINTF(buf_prefix, sizeof(buf_prefix), ";%s=", name);
#endif
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
                       "        spr"
                       "intf(num_buf, \"%%d\", kv->value.i);\n"
                       "        kv_raw = num_buf;\n"
                       "        break;\n"));
  CHECK_IO(fprintf(fp, "      case OA_KV_NUMBER:\n"
                       "        spr"
                       "intf(num_buf, \"%%g\", kv->value.n);\n"
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

  CHECK_IO(fprintf(fp, "        %s(kv_key, &key_enc);\n", encode_fn));
  CHECK_IO(fprintf(fp, "        %s(kv_raw, &val_enc);\n", encode_fn));
  CHECK_IO(fprintf(fp, "      if (!key_enc || !val_enc) {\n"
                       "        free(key_enc);\n"
                       "        free(val_enc);\n"
                       "        rc = CDD_C_ERROR_MEMORY;\n"
                       "        goto cleanup;\n"
                       "      }\n"));
  CHECK_IO(fprintf(fp, "      {\n"));
  if (explode) {
    CHECK_IO(fprintf(fp,
                     "        size_t key_len = strlen(key_enc);\n"
                     "        size_t val_len = strlen(val_enc);\n"
                     "        size_t extra = key_len + val_len + 1 + (first ? "
                     "%lu : %lu);\n"
                     "        char *tmp = (char *)realloc(path_%s, "
                     "path_len + extra + 1);\n"
                     "        if (!tmp) { free(key_enc); free(val_enc); rc = "
                     "CDD_C_ERROR_MEMORY; goto "
                     "cleanup; }\n"
                     "        path_%s = tmp;\n",
                     (unsigned long)prefix_len, (unsigned long)delim_len, name,
                     name));
    CHECK_IO(fprintf(fp,
                     "        if (first && %lu"
                     ") { memcpy(path_%s + path_len, \"%s\", %lu); "
                     "path_len += %lu; }\n"
                     "        if (!first && %lu"
                     ") { memcpy(path_%s + path_len, \"%s\", %lu); "
                     "path_len += %lu; }\n",
                     (unsigned long)prefix_len, name, prefix,
                     (unsigned long)prefix_len, (unsigned long)prefix_len,
                     (unsigned long)delim_len, name, pair_delim,
                     (unsigned long)delim_len, (unsigned long)delim_len));
    CHECK_IO(fprintf(fp,
                     "        memcpy(path_%s + path_len, key_enc, key_len);\n"
                     "        path_len += key_len;\n"
                     "        path_%s[path_len++] = '=';\n"
                     "        memcpy(path_%s + path_len, val_enc, val_len);\n"
                     "        path_len += val_len;\n"
                     "        path_%s[path_len] = '\\0';\n",
                     name, name, name, name));
  } else {
    CHECK_IO(fprintf(fp,
                     "        size_t key_len = strlen(key_enc);\n"
                     "        size_t val_len = strlen(val_enc);\n"
                     "        size_t extra = key_len + val_len + 1 + (first ? "
                     "%lu : %lu) + "
                     "%lu;\n"
                     "        char *tmp = (char *)realloc(path_%s, "
                     "path_len + extra + 1);\n"
                     "        if (!tmp) { free(key_enc); free(val_enc); rc = "
                     "CDD_C_ERROR_MEMORY; goto "
                     "cleanup; }\n"
                     "        path_%s = tmp;\n",
                     (unsigned long)prefix_len, (unsigned long)delim_len,
                     (unsigned long)delim_len, name, name));
    CHECK_IO(fprintf(fp,
                     "        if (first && %lu"
                     ") { memcpy(path_%s + path_len, \"%s\", %lu); "
                     "path_len += %lu; }\n"
                     "        if (!first && %lu"
                     ") { memcpy(path_%s + path_len, \"%s\", %lu); "
                     "path_len += %lu; }\n",
                     (unsigned long)prefix_len, name, prefix,
                     (unsigned long)prefix_len, (unsigned long)prefix_len,
                     (unsigned long)delim_len, name, pair_delim,
                     (unsigned long)delim_len, (unsigned long)delim_len));
    CHECK_IO(fprintf(fp,
                     "        memcpy(path_%s + path_len, key_enc, key_len);\n"
                     "        path_len += key_len;\n"
                     "        memcpy(path_%s + path_len, \"%s\", %lu);\n"
                     "        path_len += %lu;\n"
                     "        memcpy(path_%s + path_len, val_enc, val_len);\n"
                     "        path_len += val_len;\n"
                     "        path_%s[path_len] = '\\0';\n",
                     name, name, pair_delim, (unsigned long)delim_len,
                     (unsigned long)delim_len, name, name));
  }
  CHECK_IO(fprintf(fp, "      }\n"));
  CHECK_IO(fprintf(fp, "      free(key_enc);\n"));
  CHECK_IO(fprintf(fp, "      free(val_enc);\n"));
  CHECK_IO(fprintf(fp, "      first = 0;\n"));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "    if (!path_%s) {\n", name));
  CHECK_IO(fprintf(fp, "      path_%s = strdup(\"%s\");\n", name, prefix));
  CHECK_IO(fprintf(
      fp, "      if (!path_%s) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
      name));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "  }\n"));

  return CDD_C_SUCCESS;
}

/**
 * @brief Emits C code to serialize an array path parameter.
 *
 * @param[in,out] fp Output file stream.
 * @param[in] p The OpenAPI parameter structure.
 * @param[in] prefix Style prefix string (e.g. `.` or `;`).
 * @param[in] delim Element delimiter string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
write_path_array_serialization(FILE *fp, const struct OpenAPI_Parameter *p,
                               const char *prefix, const char *delim) {
  size_t prefix_len;
  size_t delim_len;
  const char *name;
  const char *items_type;
  const char *encode_fn = NULL;

  if (!fp || !p || !prefix || !delim)
    return CDD_C_ERROR_INVALID_ARGUMENT;

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

  CHECK_IO(fprintf(fp, "  {\n    size_t i;\n    size_t path_len = 0;\n"));
  CHECK_IO(fprintf(fp, "    for(i=0; i < %s_len; ++i) {\n", name));

  if (strcmp(items_type, "integer") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[32];\n"));
    CHECK_IO(fprintf(fp,
                     "      spr"
                     "intf(num_buf, \"%%d\", %s[i]);\n",
                     name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(items_type, "number") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      char num_buf[64];\n"));
    CHECK_IO(fprintf(fp,
                     "      spr"
                     "intf(num_buf, \"%%g\", %s[i]);\n",
                     name));
    CHECK_IO(fprintf(fp, "      raw = num_buf;\n"));
  } else if (strcmp(items_type, "boolean") == 0) {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i] ? \"true\" : \"false\";\n", name));
  } else {
    CHECK_IO(fprintf(fp, "      const char *raw;\n"));
    CHECK_IO(fprintf(fp, "      raw = %s[i];\n", name));
  }

  if (encode_fn) {
    CHECK_IO(
        fprintf(fp, "      char *enc = NULL; %s(raw, &enc);\n", encode_fn));
    CHECK_IO(fprintf(fp, "      size_t val_len;\n"));
    CHECK_IO(fprintf(
        fp, "      if (!enc) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"));
    CHECK_IO(fprintf(fp, "      val_len = strlen(enc);\n"));
    CHECK_IO(fprintf(
        fp,
        "      {\n"
        "        size_t extra = val_len + (i > 0 ? %lu"
        " : 0) + (i == 0 ? %lu : "
        "0);\n"
        "        char *tmp = (char *)realloc(path_%s, path_len + extra "
        "+ 1);\n"
        "        if (!tmp) { free(enc); rc = CDD_C_ERROR_MEMORY; goto cleanup; "
        "}\n"
        "        path_%s = tmp;\n",
        (unsigned long)delim_len, (unsigned long)prefix_len, name, name));
    CHECK_IO(fprintf(fp,
                     "        if (i == 0 && %lu"
                     ") { memcpy(path_%s + path_len, \"%s\", %lu); "
                     "path_len += %lu; }\n"
                     "        if (i > 0 && %lu"
                     ") { memcpy(path_%s + path_len, \"%s\", %lu); "
                     "path_len += %lu; }\n",
                     (unsigned long)prefix_len, name, prefix,
                     (unsigned long)prefix_len, (unsigned long)prefix_len,
                     (unsigned long)delim_len, name, delim,
                     (unsigned long)delim_len, (unsigned long)delim_len));
    CHECK_IO(fprintf(fp,
                     "        memcpy(path_%s + path_len, enc, val_len);\n"
                     "        path_len += val_len;\n"
                     "        path_%s[path_len] = '\\0';\n"
                     "      }\n",
                     name, name));
    CHECK_IO(fprintf(fp, "      free(enc);\n"));
  } else {
    CHECK_IO(fprintf(fp, "      size_t val_len = strlen(raw);\n"));
    CHECK_IO(fprintf(
        fp,
        "      {\n"
        "        size_t extra = val_len + (i > 0 ? %lu"
        " : 0) + (i == 0 ? %lu : "
        "0);\n"
        "        char *tmp = (char *)realloc(path_%s, path_len "
        "+ extra + 1);\n"
        "        if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n"
        "        path_%s = tmp;\n",
        (unsigned long)delim_len, (unsigned long)prefix_len, name, name));
    CHECK_IO(fprintf(fp,
                     "        if (i == 0 && %lu"
                     ") { memcpy(path_%s + path_len, \"%s\", %lu); "
                     "path_len += %lu; }\n"
                     "        if (i > 0 && %lu"
                     ") { memcpy(path_%s + path_len, \"%s\", %lu); "
                     "path_len += %lu; }\n",
                     (unsigned long)prefix_len, name, prefix,
                     (unsigned long)prefix_len, (unsigned long)prefix_len,
                     (unsigned long)delim_len, name, delim,
                     (unsigned long)delim_len, (unsigned long)delim_len));
    CHECK_IO(fprintf(fp,
                     "        memcpy(path_%s + path_len, raw, val_len);\n"
                     "        path_len += val_len;\n"
                     "        path_%s[path_len] = '\\0';\n"
                     "      }\n",
                     name, name));
  }

  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "    if (!path_%s) {\n", name));
  CHECK_IO(fprintf(fp, "      path_%s = strdup(\"%s\");\n", name, prefix));
  CHECK_IO(fprintf(
      fp, "      if (!path_%s) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n",
      name));
  CHECK_IO(fprintf(fp, "    }\n"));
  CHECK_IO(fprintf(fp, "  }\n"));
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds a parameter by name in a parameter array.
 *
 * @param[in] name Parameter name to search for.
 * @param[in] params Array of OpenAPI parameters.
 * @param[in] n_params Number of parameters in `params`.
 * @param[out] _out_val Output pointer to the found parameter, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t
find_param(const char *name, const struct OpenAPI_Parameter *params,
           size_t n_params, const struct OpenAPI_Parameter **_out_val) {
  size_t i;
  for (i = 0; i < n_params; ++i) {
    if (params[i].name && strcmp(params[i].name, name) == 0 &&
        params[i].in == OA_PARAM_IN_PATH) {
      {
        *_out_val = &params[i];
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
 * @brief Parses a URL path template into literal and variable segments.
 *
 * @param[in] tmpl URL path template string.
 * @param[out] out_segments Output array of parsed URL segments.
 * @param[out] out_count Output number of segments.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t parse_segments(const char *tmpl,
                                          struct UrlSegment **out_segments,
                                          size_t *out_count) {
  const char *p = tmpl;
  const char *start = p;
  struct UrlSegment *segs = NULL;
  size_t count = 0;
  size_t cap = 0;
  size_t i;

  while (*p) {
    if (*p == '{') {
      if (p > start) {
        size_t len = (size_t)(p - start);
        if (count >= cap) {
          struct UrlSegment *new_segs;
          cap = (cap == 0) ? 8 : cap * 2;
          new_segs = (struct UrlSegment *)C_CDD_REALLOC(
              segs, cap * sizeof(struct UrlSegment));
          if (!new_segs) {
            for (i = 0; i < count; ++i)
              C_CDD_FREE(segs[i].text);
            C_CDD_FREE(segs);
            C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
            return CDD_C_ERROR_MEMORY;
          }
          segs = new_segs;
        }
        segs[count].is_var = 0;
        segs[count].text = (char *)C_CDD_MALLOC(len + 1);
        if (!segs[count].text) {
          for (i = 0; i < count; ++i)
            C_CDD_FREE(segs[i].text);
          C_CDD_FREE(segs);
          return CDD_C_ERROR_MEMORY;
        }
        memcpy(segs[count].text, start, len);
        segs[count].text[len] = '\0';
        count++;
      }
      start = p + 1;
      {
        const char *close = strchr(start, '}');
        if (!close) {
          for (i = 0; i < count; ++i)
            C_CDD_FREE(segs[i].text);
          C_CDD_FREE(segs);
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
        {
          size_t len = (size_t)(close - start);
          if (count >= cap) {
            struct UrlSegment *new_segs;
            cap = (cap == 0) ? 8 : cap * 2;
            new_segs = (struct UrlSegment *)C_CDD_REALLOC(
                segs, cap * sizeof(struct UrlSegment));
            if (!new_segs) {
              for (i = 0; i < count; ++i)
                C_CDD_FREE(segs[i].text);
              C_CDD_FREE(segs);
              C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
              return CDD_C_ERROR_MEMORY;
            }
            segs = new_segs;
          }
          segs[count].is_var = 1;
          segs[count].text = (char *)C_CDD_MALLOC(len + 1);
          if (!segs[count].text) {
            for (i = 0; i < count; ++i)
              C_CDD_FREE(segs[i].text);
            C_CDD_FREE(segs);
            return CDD_C_ERROR_MEMORY;
          }
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
    size_t len = (size_t)(p - start);
    if (count >= cap) {
      struct UrlSegment *new_segs;
      cap = (cap == 0) ? 8 : cap * 2;
      new_segs = (struct UrlSegment *)C_CDD_REALLOC(
          segs, cap * sizeof(struct UrlSegment));
      if (!new_segs) {
        for (i = 0; i < count; ++i)
          C_CDD_FREE(segs[i].text);
        C_CDD_FREE(segs);
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }
      segs = new_segs;
    }
    segs[count].is_var = 0;
    segs[count].text = (char *)C_CDD_MALLOC(len + 1);
    if (!segs[count].text) {
      for (i = 0; i < count; ++i)
        C_CDD_FREE(segs[i].text);
      C_CDD_FREE(segs);
      return CDD_C_ERROR_MEMORY;
    }
    memcpy(segs[count].text, start, len);
    segs[count].text[len] = '\0';
    count++;
  }
  *out_segments = segs;
  *out_count = count;
  return CDD_C_SUCCESS;
}

/**
 * @brief Generate C code to construct a URL from a path template.
 *
 * @param[in] fp The file stream to write C code to.
 * @param[in] path_template The raw OpenAPI route string (e.g. "/users/{id}").
 * @param[in] params Array of operation parameters to look up types.
 * @param[in] n_params param count.
 * @param[in] config Configuration options (can be NULL for defaults).
 * @return 0 on success, error code (EINVAL/ENOMEM) on failure.
 */
cdd_c_error_t codegen_url_write_builder(FILE *fp, const char *path_template,
                                        const struct OpenAPI_Parameter *params,
                                        size_t n_params,
                                        const struct CodegenUrlConfig *config) {
  struct UrlSegment *segs = NULL;
  size_t n_segs = 0;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  const char *base_var = (config && config->base_variable)
                             ? config->base_variable
                             : "ctx->base_url";
  const char *out_var =
      (config && config->out_variable) ? config->out_variable : "url";

  if (!fp || !path_template)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if ((rc = parse_segments(path_template, &segs, &n_segs)) != 0) {
    return rc;
  }

#undef CHECK_IO
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;) {                                                           \
    rc = CDD_C_ERROR_IO;                                                       \
    goto cleanup_segs;                                                         \
  }

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p = NULL;
      find_param(segs[i].text, params, n_params, &p);
      if (p) {
        const char *name = p->name;
        enum OpenAPI_Style style =
            (p->style == OA_STYLE_UNKNOWN) ? OA_STYLE_SIMPLE : p->style;
        int explode = p->explode_set ? p->explode : 0;
        if (p->type && strcmp(p->type, "object") == 0 && !p->is_array) {
          if (write_path_object_serialization(fp, p) != 0) {
            rc = CDD_C_ERROR_IO;
            goto cleanup_segs;
          }
        } else if (p->is_array) {
          const char *prefix = "";
          const char *delim = ",";
          if (style == OA_STYLE_LABEL) {
            prefix = ".";
            delim = explode ? "." : ",";
          } else if (style == OA_STYLE_MATRIX) {
            static char buf_prefix[96];
            static char buf_delim[96];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
            sprintf_s(buf_prefix, sizeof(buf_prefix), ";%s=", name);
#else
            CDD_SNPRINTF(buf_prefix, sizeof(buf_prefix), ";%s=", name);
#endif
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
            sprintf_s(buf_delim, sizeof(buf_delim), ";%s=", name);
#else
            CDD_SNPRINTF(buf_delim, sizeof(buf_delim), ";%s=", name);
#endif
            prefix = buf_prefix;
            delim = explode ? buf_delim : ",";
          } else {
            prefix = "";
            delim = ",";
          }
          if (write_path_array_serialization(fp, p, prefix, delim) != 0) {
            rc = CDD_C_ERROR_IO;
            goto cleanup_segs;
          }
        } else {
          const char *encode_fn = (p->allow_reserved_set && p->allow_reserved)
                                      ? "url_encode_allow_reserved"
                                      : "url_encode";
          const char *prefix = "";
          if (style == OA_STYLE_LABEL) {
            prefix = ".";
          } else if (style == OA_STYLE_MATRIX) {
            static char buf_prefix[96];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
            sprintf_s(buf_prefix, sizeof(buf_prefix), ";%s=", name);
#else
            CDD_SNPRINTF(buf_prefix, sizeof(buf_prefix), ";%s=", name);
#endif
            prefix = buf_prefix;
          }

          if (strcmp(p->type, "string") == 0) {
            CHECK_IO(fprintf(fp, "  {\n    char *enc = NULL; %s(%s, &enc);\n",
                             encode_fn, name));
            CHECK_IO(fprintf(fp,
                             "    if (asprintf(&path_%s, \"%s%%s\", enc) == "
                             "-1) { free(enc); return CDD_C_ERROR_MEMORY; }\n",
                             name, prefix));
            CHECK_IO(fprintf(fp, "    free(enc);\n  }\n"));
          } else if (strcmp(p->type, "integer") == 0) {
            CHECK_IO(fprintf(fp, "  {\n    char num_buf[32];\n"));
            CHECK_IO(fprintf(fp,
                             "    spr"
                             "intf(num_buf, \"%%d\", %s);\n",
                             name));
            CHECK_IO(fprintf(fp,
                             "    if (asprintf(&path_%s, \"%s%%s\", num_buf) "
                             "== -1) return CDD_C_ERROR_MEMORY;\n",
                             name, prefix));
            CHECK_IO(fprintf(fp, "  }\n"));
          } else if (strcmp(p->type, "number") == 0) {
            CHECK_IO(fprintf(fp, "  {\n    char num_buf[64];\n"));
            CHECK_IO(fprintf(fp,
                             "    spr"
                             "intf(num_buf, \"%%g\", %s);\n",
                             name));
            CHECK_IO(fprintf(fp,
                             "    if (asprintf(&path_%s, \"%s%%s\", num_buf) "
                             "== -1) return CDD_C_ERROR_MEMORY;\n",
                             name, prefix));
            CHECK_IO(fprintf(fp, "  }\n"));
          } else if (strcmp(p->type, "boolean") == 0) {
            CHECK_IO(fprintf(
                fp,
                "  if (asprintf(&path_%s, \"%s%%s\", %s ? "
                "\"true\" : \"false\") == -1) return CDD_C_ERROR_MEMORY;\n",
                name, prefix, name));
          } else {
            CHECK_IO(fprintf(fp,
                             "  if (asprintf(&path_%s, \"%s%%s\", %s) == -1) "
                             "return CDD_C_ERROR_MEMORY;\n",
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
      const struct OpenAPI_Parameter *p = NULL;
      find_param(segs[i].text, params, n_params, &p);
      if (p) {
        CHECK_IO(fprintf(fp, ", path_%s", p->name));
      } else {
        CHECK_IO(fprintf(fp, ", %s", segs[i].text));
      }
    }
  }
  CHECK_IO(fprintf(fp, ") == -1) {\n"));

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p = NULL;
      find_param(segs[i].text, params, n_params, &p);
      if (p) {
        CHECK_IO(fprintf(fp, "    free(path_%s);\n", p->name));
      }
    }
  }
  CHECK_IO(fprintf(fp, "    return CDD_C_ERROR_MEMORY;\n  }\n"));

  for (i = 0; i < n_segs; ++i) {
    if (segs[i].is_var) {
      const struct OpenAPI_Parameter *p = NULL;
      find_param(segs[i].text, params, n_params, &p);
      if (p) {
        CHECK_IO(fprintf(fp, "  free(path_%s);\n", p->name));
      }
    }
  }

cleanup_segs:
  for (i = 0; i < n_segs; ++i)
    C_CDD_FREE(segs[i].text);
  C_CDD_FREE(segs);

  return rc;
}
