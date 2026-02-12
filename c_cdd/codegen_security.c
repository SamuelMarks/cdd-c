/**
 * @file codegen_security.c
 * @brief Implementation of Security Code Generation.
 *
 * Scans an OpenAPI Operation for `security` requirements and matches them
 * against defined `securitySchemes`. Emits C code during the request setup
 * phase to inject tokens into Headers or Query Strings.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "codegen_security.h"
#include "str_utils.h"

static int
scheme_in_security_sets(const struct OpenAPI_SecurityRequirementSet *sets,
                        size_t n_sets, const char *scheme_name) {
  size_t i, j;
  if (!sets || !scheme_name)
    return 0;
  for (i = 0; i < n_sets; ++i) {
    const struct OpenAPI_SecurityRequirementSet *set = &sets[i];
    for (j = 0; j < set->n_requirements; ++j) {
      const struct OpenAPI_SecurityRequirement *req = &set->requirements[j];
      if (req->scheme && strcmp(req->scheme, scheme_name) == 0)
        return 1;
    }
  }
  return 0;
}

static void
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
    return;

  if (op && op->security_set) {
    if (out_sets)
      *out_sets = op->security;
    if (out_count)
      *out_count = op->n_security;
    if (out_set_flag)
      *out_set_flag = 1;
    return;
  }
  if (spec->security_set) {
    if (out_sets)
      *out_sets = spec->security;
    if (out_count)
      *out_count = spec->n_security;
    if (out_set_flag)
      *out_set_flag = 1;
  }
}

static int scheme_is_active(const struct OpenAPI_SecurityScheme *sch,
                            const struct OpenAPI_SecurityRequirementSet *sets,
                            size_t n_sets, int security_set) {
  if (!sch)
    return 0;
  if (!security_set)
    return 1;
  return scheme_in_security_sets(sets, n_sets, sch->name);
}

int codegen_security_requires_query(const struct OpenAPI_Operation *op,
                                    const struct OpenAPI_Spec *spec) {
  const struct OpenAPI_SecurityRequirementSet *active_sets = NULL;
  size_t n_active_sets = 0;
  int security_set = 0;
  size_t i;

  if (!spec)
    return 0;

  resolve_active_security(op, spec, &active_sets, &n_active_sets,
                          &security_set);
  if (security_set && n_active_sets == 0)
    return 0;

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *sch = &spec->security_schemes[i];
    if (sch->type != OA_SEC_APIKEY || sch->in != OA_SEC_IN_QUERY)
      continue;
    if (scheme_is_active(sch, active_sets, n_active_sets, security_set))
      return 1;
  }
  return 0;
}

int codegen_security_requires_cookie(const struct OpenAPI_Operation *op,
                                     const struct OpenAPI_Spec *spec) {
  const struct OpenAPI_SecurityRequirementSet *active_sets = NULL;
  size_t n_active_sets = 0;
  int security_set = 0;
  size_t i;

  if (!spec)
    return 0;

  resolve_active_security(op, spec, &active_sets, &n_active_sets,
                          &security_set);
  if (security_set && n_active_sets == 0)
    return 0;

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *sch = &spec->security_schemes[i];
    if (sch->type != OA_SEC_APIKEY || sch->in != OA_SEC_IN_COOKIE)
      continue;
    if (scheme_is_active(sch, active_sets, n_active_sets, security_set))
      return 1;
  }
  return 0;
}

int codegen_security_write_apply(FILE *const fp,
                                 const struct OpenAPI_Operation *const op,
                                 const struct OpenAPI_Spec *const spec) {
  size_t i;
  int has_security = 0;
  const struct OpenAPI_SecurityRequirementSet *active_sets = NULL;
  size_t n_active_sets = 0;
  int security_set = 0;

  if (!fp || !op || !spec)
    return EINVAL;

  resolve_active_security(op, spec, &active_sets, &n_active_sets,
                          &security_set);

  if (security_set && n_active_sets == 0) {
    return 0; /* Explicitly empty security array */
  }

  /* Iterate security schemes defined in components */
  if (spec->security_schemes == NULL || spec->n_security_schemes == 0) {
    return 0; /* No schemes defined, nothing to inject */
  }

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *sch = &spec->security_schemes[i];
    if (!scheme_is_active(sch, active_sets, n_active_sets, security_set))
      continue;

    /* Scheme: API Key (Header) */
    if (sch->type == OA_SEC_APIKEY && sch->in == OA_SEC_IN_HEADER) {
      if (sch->name && sch->key_name) {
        /* "if (ctx->security.api_key_NAME) ..." */
        fprintf(fp, "  if (ctx->security.api_key_%s) {\n", sch->name);
        fprintf(fp,
                "    http_headers_add(&req.headers, \"%s\", "
                "ctx->security.api_key_%s);\n",
                sch->key_name, sch->name);
        fprintf(fp, "  }\n");
        has_security = 1;
      }
    }
    /* Scheme: HTTP Bearer */
    else if (sch->type == OA_SEC_HTTP && sch->scheme &&
             strcmp(sch->scheme, "bearer") == 0) {
      /* "if (ctx->security.bearer_token) ..." */
      /* Usually generic, or scoped by scheme name if multiple exist */
      fprintf(fp, "  if (ctx->security.bearer_token) {\n");
      fprintf(fp, "    rc = http_request_set_auth_bearer(&req, "
                  "ctx->security.bearer_token);\n");
      fprintf(fp, "    if (rc != 0) goto cleanup;\n");
      fprintf(fp, "  }\n");
      has_security = 1;
    }
    /* Scheme: HTTP Basic (token must be base64 of "user:pass") */
    else if (sch->type == OA_SEC_HTTP && sch->scheme &&
             strcmp(sch->scheme, "basic") == 0) {
      fprintf(fp, "  if (ctx->security.basic_token) {\n");
      fprintf(fp, "    rc = http_request_set_auth_basic(&req, "
                  "ctx->security.basic_token);\n");
      fprintf(fp, "    if (rc != 0) goto cleanup;\n");
      fprintf(fp, "  }\n");
      has_security = 1;
    }
    /* Scheme: API Key (Query) */
    else if (sch->type == OA_SEC_APIKEY && sch->in == OA_SEC_IN_QUERY) {
      if (sch->name && sch->key_name) {
        fprintf(fp, "  if (ctx->security.api_key_%s) {\n", sch->name);
        fprintf(fp, "    if (!qp_initialized) {\n");
        fprintf(fp, "      rc = url_query_init(&qp);\n");
        fprintf(fp, "      if (rc != 0) goto cleanup;\n");
        fprintf(fp, "      qp_initialized = 1;\n");
        fprintf(fp, "    }\n");
        fprintf(fp,
                "    rc = url_query_add(&qp, \"%s\", "
                "ctx->security.api_key_%s);\n",
                sch->key_name, sch->name);
        fprintf(fp, "    if (rc != 0) goto cleanup;\n");
        fprintf(fp, "  }\n");
        has_security = 1;
      }
    }
    /* Scheme: API Key (Cookie) */
    else if (sch->type == OA_SEC_APIKEY && sch->in == OA_SEC_IN_COOKIE) {
      if (sch->name && sch->key_name) {
        fprintf(fp, "  if (ctx->security.api_key_%s) {\n", sch->name);
        fprintf(fp, "    const char *cookie_val = ctx->security.api_key_%s;\n",
                sch->name);
        fprintf(fp, "    if (cookie_val) {\n");
        fprintf(fp, "      size_t name_len = strlen(\"%s\");\n", sch->key_name);
        fprintf(fp, "      size_t val_len = strlen(cookie_val);\n");
        fprintf(fp, "      size_t extra = name_len + 1 + val_len + "
                    "(cookie_len ? 2 : 0);\n");
        fprintf(fp,
                "      char *tmp = (char *)realloc(cookie_str, cookie_len + "
                "extra + 1);\n");
        fprintf(fp, "      if (!tmp) { rc = ENOMEM; goto cleanup; }\n");
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

  return 0;
}
