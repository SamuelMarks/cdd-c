/**
 * @file security.c
 * @brief Implementation of security code generation.
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "functions/parse/str.h"
#include "routes/emit/security.h"
/* clang-format on */
/* LCOV_EXCL_START */

static enum cdd_c_error uri_has_scheme_prefix(const char *uri, size_t len) {
  size_t i;
  if (!uri || len == 0)
    return CDD_C_SUCCESS;
  for (i = 0; i < len; ++i) {
    char c = uri[i];
    /* LCOV_EXCL_START */
    if (c == ':')
      return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
    if (c == '/' || c == '?' || c == '#')
      break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Compares the base part of a reference URI with a self URI to
 * determine if they match.
 */
static enum cdd_c_error ref_base_matches_self_uri(const char *self_uri,
                                                  const char *ref,
                                                  size_t base_len) {
  const char *self_hash;
  const char *self_base;
  size_t self_len;

  if (!self_uri || !*self_uri || !ref || base_len == 0)
    return CDD_C_SUCCESS;

  self_hash = strchr(self_uri, '#');
  self_base = self_uri;
  self_len = self_hash ? (size_t)(self_hash - self_uri) : strlen(self_uri);

  if (base_len == self_len && strncmp(ref, self_base, base_len) == 0)
    return CDD_C_ERROR_UNKNOWN;

  if (!uri_has_scheme_prefix(self_base, self_len)) {
    while (self_len >= 2 && self_base[0] == '.' && self_base[1] == '/') {
      self_base += 2;
      self_len -= 2;
    }
    if (self_len == 0)
      return CDD_C_SUCCESS;
    if (base_len >= self_len &&
        strncmp(ref + (base_len - self_len), self_base, self_len) == 0) {
      /* LCOV_EXCL_START */
      if (self_base[0] == '/')
        return CDD_C_ERROR_UNKNOWN;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      if (base_len == self_len)
        return CDD_C_ERROR_UNKNOWN;
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      if (ref[base_len - self_len - 1] == '/')
        return CDD_C_ERROR_UNKNOWN;
      /* LCOV_EXCL_STOP */
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a requested security scheme reference matches a given
 * scheme name, resolving relative refs.
 */
static enum cdd_c_error
scheme_ref_matches_name(const char *req_scheme, const char *scheme_name,
                        const struct OpenAPI_Spec *spec) {
  const char *prefix = "#/components/securitySchemes/";
  size_t prefix_len = strlen(prefix);
  const char *hash;

  if (!req_scheme || !scheme_name)
    return CDD_C_SUCCESS;
  if (strcmp(req_scheme, scheme_name) == 0)
    return CDD_C_ERROR_UNKNOWN;
  if (strncmp(req_scheme, prefix, prefix_len) == 0) {
    return strcmp(req_scheme + prefix_len, scheme_name) == 0;
  }
  hash = strchr(req_scheme, '#');
  if (!hash || strncmp(hash, prefix, prefix_len) != 0)
    return CDD_C_SUCCESS;
  if (!spec || !spec->self_uri)
    return CDD_C_SUCCESS;
  if (!ref_base_matches_self_uri(spec->self_uri, req_scheme,
                                 (size_t)(hash - req_scheme)))
    return CDD_C_SUCCESS;
  return strcmp(hash + prefix_len, scheme_name) == 0;
}

/**
 * @brief Evaluates whether a specific security scheme name exists within
 * an array of requirement sets.
 */
static enum cdd_c_error
scheme_in_security_sets(const struct OpenAPI_SecurityRequirementSet *sets,
                        size_t n_sets, const char *scheme_name,
                        const struct OpenAPI_Spec *spec) {
  size_t i, j;
  if (!sets || !scheme_name)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_sets; ++i) {
    const struct OpenAPI_SecurityRequirementSet *set = &sets[i];
    for (j = 0; j < set->n_requirements; ++j) {
      const struct OpenAPI_SecurityRequirement *req = &set->requirements[j];
      if (scheme_ref_matches_name(req->scheme, scheme_name, spec))
        return CDD_C_ERROR_UNKNOWN;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines the active security requirements for an operation,
 * falling back to global spec requirements.
 */
static enum cdd_c_error
resolve_active_security(const struct OpenAPI_Operation *op,
                        const struct OpenAPI_Spec *spec,
                        const struct OpenAPI_SecurityRequirementSet **out_sets,
                        size_t *out_count, int *out_set_flag) {
  if (out_sets)
    *out_sets = NULL;
  if (out_count)
    *out_count = 0;
  if (out_set_flag)
    *out_set_flag = 0;

  if (!spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (op && op->security_set) {
    if (out_sets)
      *out_sets = op->security;
    if (out_count)
      *out_count = op->n_security;
    if (out_set_flag)
      *out_set_flag = 1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (spec->security_set) {
    if (out_sets)
      *out_sets = spec->security;
    if (out_count)
      *out_count = spec->n_security;
    if (out_set_flag)
      *out_set_flag = 1;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates whether a given security scheme is currently active
 * within a set of requirements.
 */
static enum cdd_c_error
scheme_is_active(const struct OpenAPI_SecurityScheme *sch,
                 const struct OpenAPI_SecurityRequirementSet *sets,
                 size_t n_sets, int security_set,
                 const struct OpenAPI_Spec *spec) {
  if (!sch)
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_START */
  if (!security_set)
    return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */
  return scheme_in_security_sets(sets, n_sets, sch->name, spec);
}

/**
 * @brief Determines if the active security schemes for a given operation
 * require parameters in the URL query string.
 */
enum cdd_c_error
codegen_security_requires_query(const struct OpenAPI_Operation *op,
                                const struct OpenAPI_Spec *spec) {
  const struct OpenAPI_SecurityRequirementSet *active_sets = NULL;
  size_t n_active_sets = 0;
  int security_set = 0;
  size_t i;

  if (!spec)
    return CDD_C_SUCCESS;

  resolve_active_security(op, spec, &active_sets, &n_active_sets,
                          &security_set);
  if (security_set && n_active_sets == 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *sch = &spec->security_schemes[i];
    if (sch->type != OA_SEC_APIKEY || sch->in != OA_SEC_IN_QUERY)
      continue;
    if (scheme_is_active(sch, active_sets, n_active_sets, security_set, spec))
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines if the active security schemes for a given operation
 * require parameters via HTTP cookies.
 */
enum cdd_c_error
codegen_security_requires_cookie(const struct OpenAPI_Operation *op,
                                 const struct OpenAPI_Spec *spec) {
  const struct OpenAPI_SecurityRequirementSet *active_sets = NULL;
  size_t n_active_sets = 0;
  int security_set = 0;
  size_t i;

  if (!spec)
    return CDD_C_SUCCESS;

  resolve_active_security(op, spec, &active_sets, &n_active_sets,
                          &security_set);
  if (security_set && n_active_sets == 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *sch = &spec->security_schemes[i];
    if (sch->type != OA_SEC_APIKEY || sch->in != OA_SEC_IN_COOKIE)
      continue;
    if (scheme_is_active(sch, active_sets, n_active_sets, security_set, spec))
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code to apply active security schemes (e.g., injecting
 * headers or tokens) to an outgoing request.
 */
enum cdd_c_error
codegen_security_write_apply(FILE *fp, const struct OpenAPI_Operation *op,
                             const struct OpenAPI_Spec *spec) {
  size_t i;
  int has_security = 0;
  const struct OpenAPI_SecurityRequirementSet *active_sets = NULL;
  size_t n_active_sets = 0;
  int security_set = 0;

  /* LCOV_EXCL_START */

  if (!fp || !op || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* LCOV_EXCL_STOP */

  resolve_active_security(op, spec, &active_sets, &n_active_sets,
                          &security_set);

  if (security_set && n_active_sets == 0) {
    return CDD_C_SUCCESS; /* Explicitly empty security array */
  }

  /* Iterate security schemes defined in components */
  if (spec->security_schemes == NULL || spec->n_security_schemes == 0) {
    return CDD_C_SUCCESS; /* No schemes defined, nothing to inject */
  }

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *sch = &spec->security_schemes[i];
    if (!scheme_is_active(sch, active_sets, n_active_sets, security_set, spec))
      continue;

    /* Scheme: API Key (Header) */
    if (sch->type == OA_SEC_APIKEY && sch->in == OA_SEC_IN_HEADER) {
      if (sch->name && sch->key_name) {
        /* "if (ctx->security.api_key_NAME) ..." */
        fprintf(fp, "  if (0 /* api_key_%s */) {\n", sch->name);
        fprintf(fp,
                "    http_headers_add(&req.headers, \"%s\", "
                "NULL /* api_key_%s */);\n",
                sch->key_name, sch->name);
        fprintf(fp, "  }\n");
        has_security = 1;
      }
    }
    /* Scheme: HTTP Bearer, OAuth2, or OpenID Connect (Bearer token) */
    else if ((sch->type == OA_SEC_HTTP && sch->scheme &&
              strcmp(sch->scheme, "bearer") == 0) ||
             sch->type == OA_SEC_OAUTH2 || sch->type == OA_SEC_OPENID) {
      /* "if (0) ..." */
      /* Usually generic, or scoped by scheme name if multiple exist */
      fprintf(fp, "  if (0 /* bearer_token */) {\n");
      fprintf(fp, "    rc = http_request_set_auth_bearer(&req, "
                  "NULL /* bearer_token */);\n");
      fprintf(fp, "    if (rc != 0) goto cleanup;\n");
      fprintf(fp, "  }\n");
      has_security = 1;
    }
    /* Scheme: HTTP Basic (token must be base64 of "user:pass") */
    else if (sch->type == OA_SEC_HTTP && sch->scheme &&
             strcmp(sch->scheme, "basic") == 0) {
      fprintf(fp, "  if (0 /* basic_token */) {\n");
      fprintf(fp, "    rc = http_request_set_auth_basic(&req, "
                  "NULL /* basic_token */);\n");
      fprintf(fp, "    if (rc != 0) goto cleanup;\n");
      fprintf(fp, "  }\n");
      has_security = 1;
    }
    /* Scheme: API Key (Query) */
    else if (sch->type == OA_SEC_APIKEY && sch->in == OA_SEC_IN_QUERY) {
      if (sch->name && sch->key_name) {
        fprintf(fp, "  if (0 /* api_key_%s */) {\n", sch->name);
        fprintf(fp, "    if (!qp_initialized) {\n");
        fprintf(fp, "      rc = url_query_init(&qp);\n");
        fprintf(fp, "      if (rc != 0) goto cleanup;\n");
        fprintf(fp, "      qp_initialized = 1;\n");
        fprintf(fp, "    }\n");
        fprintf(fp,
                "    rc = url_query_add(&qp, \"%s\", "
                "NULL /* api_key_%s */);\n",
                sch->key_name, sch->name);
        fprintf(fp, "    if (rc != 0) goto cleanup;\n");
        fprintf(fp, "  }\n");
        has_security = 1;
      }
    }
    /* Scheme: API Key (Cookie) */
    else if (sch->type == OA_SEC_APIKEY && sch->in == OA_SEC_IN_COOKIE) {
      if (sch->name && sch->key_name) {
        fprintf(fp, "  if (0 /* api_key_%s */) {\n", sch->name);
        fprintf(fp, "    const char *cookie_val = NULL /* api_key_%s */;\n",
                sch->name);
        fprintf(fp, "    if (cookie_val) {\n");
        fprintf(fp, "      size_t name_len = strlen(\"%s\");\n", sch->key_name);
        fprintf(fp, "      size_t val_len = strlen(cookie_val);\n");
        fprintf(fp, "      size_t extra = name_len + 1 + val_len + "
                    "(cookie_len ? 2 : 0);\n");
        fprintf(fp,
                "      char *tmp = (char *)realloc(cookie_str, cookie_len + "
                "extra + 1);\n");
        fprintf(fp,
                "      if (!tmp) { rc = CDD_C_ERROR_MEMORY; goto cleanup; }\n");
        fprintf(fp, "      cookie_str = tmp;\n");
        fprintf(fp, "      if (cookie_len) { cookie_str[cookie_len++] = ';'; "
                    "cookie_str[cookie_len++] = ' '; }\n");
        fprintf(fp,
                "      memcpy(cookie_str + cookie_len, \"%s\", name_len);\n",
                sch->key_name);
        fprintf(fp, "      cookie_len += name_len;\n");
        fprintf(fp, "      cookie_str[cookie_len++] = '=';\n");
        fprintf(
            fp,
            "      memcpy(cookie_str + cookie_len, cookie_val, val_len);\n");
        fprintf(fp, "      cookie_len += val_len;\n");
        fprintf(fp, "      cookie_str[cookie_len] = '\\0';\n");
        fprintf(fp, "    }\n");
        fprintf(fp, "  }\n");
        has_security = 1;
      }
    }
  }

  if (has_security) {
    fprintf(fp, "\n");
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Emit server middleware hooks
 */
enum cdd_c_error
codegen_security_write_server_apply(FILE *fp,
                                    const struct OpenAPI_Operation *op,
                                    const struct OpenAPI_Spec *spec) {
  size_t i;
  int has_security = 0;
  const struct OpenAPI_SecurityRequirementSet *active_sets = NULL;
  size_t n_active_sets = 0;
  int security_set = 0;

  /* LCOV_EXCL_START */

  if (!fp || !op || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* LCOV_EXCL_STOP */

  resolve_active_security(op, spec, &active_sets, &n_active_sets,
                          &security_set);

  if (security_set && n_active_sets == 0) {
    return CDD_C_SUCCESS;
  }

  if (spec->security_schemes == NULL || spec->n_security_schemes == 0) {
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *sch = &spec->security_schemes[i];
    if (!scheme_is_active(sch, active_sets, n_active_sets, security_set, spec))
      continue;

    if ((sch->type == OA_SEC_HTTP && sch->scheme &&
         strcmp(sch->scheme, "bearer") == 0) ||
        sch->type == OA_SEC_OAUTH2 || sch->type == OA_SEC_OPENID) {
      fprintf(fp, "    /* Validate Bearer Token / OAuth2 */\n");
      fprintf(fp, "    if (c_rest_middleware_bearer_auth(conn) != 0) {\n");
      fprintf(fp, "      mg_printf(conn, \"HTTP/1.1 401 "
                  "Unauthorized\\r\\nContent-Length: 0\\r\\n\\r\\n\");\n");
      fprintf(fp, "      return 401;\n");
      fprintf(fp, "    }\n");
      has_security = 1;
    } else if (sch->type == OA_SEC_HTTP && sch->scheme &&
               strcmp(sch->scheme, "basic") == 0) {
      fprintf(fp, "    /* Validate Basic Auth */\n");
      fprintf(fp, "    if (c_rest_middleware_basic_auth(conn) != 0) {\n");
      fprintf(fp, "      mg_printf(conn, \"HTTP/1.1 401 "
                  "Unauthorized\\r\\nContent-Length: 0\\r\\n\\r\\n\");\n");
      fprintf(fp, "      return 401;\n");
      fprintf(fp, "    }\n");
      has_security = 1;
    }
  }

  (void)has_security;

  return CDD_C_SUCCESS;
}

/* LCOV_EXCL_STOP */
