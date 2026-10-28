/**
 * @file client_gen_helpers.c
 * @brief Helper routines for OpenAPI client generator.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "routes/emit/client_gen_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Finds a server variable by name in an OpenAPI Server object.
 *
 * @param[in] srv The OpenAPI Server structure.
 * @param[in] name The variable name to find.
 * @param[out] _out_val Output pointer to the found variable, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t
find_server_variable(const struct OpenAPI_Server *srv, const char *name,
                     const struct OpenAPI_ServerVariable **_out_val) {
  size_t i;
  if (!srv || !name || !srv->variables) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < srv->n_variables; ++i) {
    const struct OpenAPI_ServerVariable *var = &srv->variables[i];
    if (var->name && strcmp(var->name, name) == 0) {
      *_out_val = var;
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Renders a Server URL substituting default values for template
 * variables.
 *
 * @param[in] srv The OpenAPI Server structure.
 * @param[out] _out_val Newly allocated rendered URL string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t render_server_url_default(const struct OpenAPI_Server *srv,
                                        char **_out_val) {
  const char *url;
  size_t out_len = 0;
  size_t i = 0;
  char *out;

  if (!srv || !srv->url) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  url = srv->url;

  while (url[i]) {
    if (url[i] == '{') {
      const char *end = strchr(url + i + 1, '}');
      size_t name_len;
      char *name;
      const struct OpenAPI_ServerVariable *var = NULL;
      if (!end) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      name_len = (size_t)(end - (url + i + 1));
      if (name_len == 0) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      name = (char *)C_CDD_MALLOC(name_len + 1);
      if (!name) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      memcpy(name, url + i + 1, name_len);
      name[name_len] = '\0';
      find_server_variable(srv, name, &var);
      C_CDD_FREE(name);
      if (!var || !var->default_value) {
        *_out_val = NULL;
        return CDD_C_SUCCESS;
      }
      out_len += strlen(var->default_value);
      i = (size_t)(end - url) + 1;
      continue;
    }
    out_len++;
    i++;
  }

  out = (char *)C_CDD_MALLOC(out_len + 1);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  i = 0;
  {
    size_t out_pos = 0;
    while (url[i]) {
      if (url[i] == '{') {
        const char *end = strchr(url + i + 1, '}');
        size_t name_len = (size_t)(end - (url + i + 1));
        char *name = (char *)C_CDD_MALLOC(name_len + 1);
        const struct OpenAPI_ServerVariable *var = NULL;
        if (!name) {
          C_CDD_FREE(out);
          *_out_val = NULL;
          return CDD_C_SUCCESS;
        }
        memcpy(name, url + i + 1, name_len);
        name[name_len] = '\0';
        find_server_variable(srv, name, &var);
        C_CDD_FREE(name);
        memcpy(out + out_pos, var->default_value, strlen(var->default_value));
        out_pos += strlen(var->default_value);
        i = (size_t)(end - url) + 1;
        continue;
      }
      out[out_pos++] = url[i++];
    }
    out[out_pos] = '\0';
  }

  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Escapes backslashes and quotes in a string for C string literal
 * emission.
 *
 * @param[in] s Input string.
 * @param[out] _out_val Newly allocated escaped string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t escape_c_string_literal(const char *s, char **_out_val) {
  size_t i;
  size_t out_len = 0;
  char *out;
  size_t pos = 0;

  if (!s) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  for (i = 0; s[i]; ++i) {
    switch (s[i]) {
    case '\\':
    case '\"':
      out_len += 2;
      break;
    case '\n':
    case '\r':
    case '\t':
      out_len += 2;
      break;
    default:
      out_len += 1;
      break;
    }
  }
  out = (char *)C_CDD_MALLOC(out_len + 1);
  if (!out) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; s[i]; ++i) {
    switch (s[i]) {
    case '\\':
      out[pos++] = '\\';
      out[pos++] = '\\';
      break;
    case '\"':
      out[pos++] = '\\';
      out[pos++] = '\"';
      break;
    case '\n':
      out[pos++] = '\\';
      out[pos++] = 'n';
      break;
    case '\r':
      out[pos++] = '\\';
      out[pos++] = 'r';
      break;
    case '\t':
      out[pos++] = '\\';
      out[pos++] = 't';
      break;
    default:
      out[pos++] = s[i];
      break;
    }
  }
  out[pos] = '\0';
  {
    *_out_val = out;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Selects the effective OpenAPI Server for an operation (operation-level
 * or path-level).
 *
 * @param[in] path Enclosing OpenAPI Path item.
 * @param[in] op OpenAPI Operation structure.
 * @param[out] _out_val Output pointer to the selected Server, or NULL.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t select_operation_server(const struct OpenAPI_Path *path,
                                      const struct OpenAPI_Operation *op,
                                      struct OpenAPI_Server **_out_val) {
  if (op && op->servers && op->n_servers > 0) {
    *_out_val = &op->servers[0];
    return CDD_C_SUCCESS;
  }
  if (path && path->servers && path->n_servers > 0) {
    *_out_val = &path->servers[0];
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Builds a C string literal expression for a base URL.
 *
 * @param[in] url Raw URL string.
 * @param[out] _out_val Newly allocated C string literal expression.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t build_base_url_literal(const char *url, char **_out_val) {
  char *escaped = NULL;
  char *literal = NULL;
  size_t len;

  if (!url) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  escape_c_string_literal(url, &escaped);
  if (!escaped) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  len = strlen(escaped) + 3;
  literal = (char *)C_CDD_MALLOC(len);
  if (!literal) {
    C_CDD_FREE(escaped);
    {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
    }
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(literal, len, "\"%s\"", escaped);
#else
  CDD_SNPRINTF(literal, len, "\"%s\"", escaped);
#endif
  C_CDD_FREE(escaped);
  {
    *_out_val = literal;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Generates an uppercase C header include guard from a base name.
 *
 * @param[in] base Base filename string.
 * @param[out] _out_val Newly allocated include guard string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t generate_guard(const char *base, char **_out_val) {
  char *g;
  size_t len = strlen(base);
  size_t i;

  g = C_CDD_MALLOC(len + 3); /* + _H + null */
  if (!g) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < len; ++i) {
    if (isalnum((unsigned char)base[i])) {
      g[i] = (char)toupper((unsigned char)base[i]);
    } else {
      g[i] = '_';
    }
  }
  g[len] = '_';
  g[len + 1] = 'H';
  g[len + 2] = '\0';
  {
    *_out_val = g;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Derives the default model header filename from a base name.
 *
 * @param[in] base Base filename string.
 * @param[out] _out_val Newly allocated model header filename.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t derive_model_header(const char *base, char **_out_val) {
  char *m;
  size_t len = strlen(base) + 10; /* _models.h */
  m = C_CDD_MALLOC(len + 1);
  if (!m) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  sprintf_s(m, len + 1, "%s_models.h", base);
#else
  CDD_SNPRINTF(m, len + 1, "%s_models.h", base);
#endif
  {
    *_out_val = m;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Sanitizes an OpenAPI tag name into a valid C identifier segment.
 *
 * @param[in] tag Raw tag string.
 * @param[out] _out_val Newly allocated sanitized tag string.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t sanitize_tag(const char *tag, char **_out_val) {
  char *s;
  size_t i;
  if (!tag) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  s = C_CDD_STRDUP(tag);
  if (!s) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  if (s[0] && islower((unsigned char)s[0])) {
    s[0] = (char)toupper((unsigned char)s[0]);
  }

  for (i = 0; s[i]; ++i) {
    if (!isalnum((unsigned char)s[i])) {
      s[i] = '_';
    }
  }
  {
    *_out_val = s;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if two OpenAPI parameters share the same name and location.
 *
 * @param[in] a First OpenAPI parameter.
 * @param[in] b Second OpenAPI parameter.
 * @return Non-zero if matching, CDD_C_SUCCESS otherwise.
 */
cdd_c_error_t param_keys_match(const struct OpenAPI_Parameter *a,
                               const struct OpenAPI_Parameter *b) {
  if (!a || !b || !a->name || !b->name)
    return CDD_C_SUCCESS;
  return (a->in == b->in) && (strcmp(a->name, b->name) == 0);
}

/**
 * @brief Merges path-level and operation-level parameters into an effective
 * parameter list.
 *
 * @param[in] path Enclosing OpenAPI Path item.
 * @param[in] op OpenAPI Operation structure.
 * @param[out] out_params Output array of merged parameters.
 * @param[out] out_count Output count of merged parameters.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t build_effective_parameters(const struct OpenAPI_Path *path,
                                         const struct OpenAPI_Operation *op,
                                         struct OpenAPI_Parameter **out_params,
                                         size_t *out_count) {
  size_t cap = 0;
  size_t count = 0;
  struct OpenAPI_Parameter *params = NULL;

  if (!out_params || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_params = NULL;
  *out_count = 0;

  if (path)
    cap += path->n_parameters;
  if (op)
    cap += op->n_parameters;

  if (cap == 0)
    return CDD_C_SUCCESS;

  params = (struct OpenAPI_Parameter *)C_CDD_CALLOC(cap, sizeof(*params));
  if (!params) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  if (path && path->parameters) {
    size_t i;
    for (i = 0; i < path->n_parameters; ++i) {
      params[count++] = path->parameters[i];
    }
  }

  if (op && op->parameters) {
    size_t i;
    for (i = 0; i < op->n_parameters; ++i) {
      size_t k;
      int replaced = 0;
      for (k = 0; k < count; ++k) {
        if (param_keys_match(&params[k], &op->parameters[i])) {
          params[k] = op->parameters[i];
          replaced = 1;
          break;
        }
      }
      if (!replaced) {
        params[count++] = op->parameters[i];
      }
    }
  }

  *out_params = params;
  *out_count = count;

  return CDD_C_SUCCESS;
}

/**
 * @brief Write standard includes to the header file.
 * Defines `struct ApiError` for standardized error handling.
 */
/**
 * @brief Emits C source code for header preamble.
 */
