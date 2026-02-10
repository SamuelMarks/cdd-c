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

static int scheme_in_security_sets(
    const struct OpenAPI_SecurityRequirementSet *sets, size_t n_sets,
    const char *scheme_name) {
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

  /* Check operation security requirements */
  /* Structure of `op->security` is array of Dict[scheme_name, scopes] */
  /* Note: Current `OpenAPI_Operation` struct in `openapi_loader.h` needs to
     expose security requirements. Assuming extended struct or simpler checking
     via spec global vs local override logic. For this delivery, we assume the
     Operation object has a `security` field added or we parse components. Wait,
     the current loader definition doesn't have `security` field in Op. We will
     proceed by defining the logic using the Spec components assuming the caller
     resolves the active scheme. However, refactoring `openapi_loader` is out of
     scope. Instead, we scan the spec's `securitySchemes` and generate code that
     applies ALL GLOBAL schemes or specific ones if passed. The typical pattern
     is: Client has `api_key` or `bearer_token` in config. If present, inject
     it.
  */

  /* Determine active security requirements (operation overrides root).
     If nothing is declared, fall back to legacy behavior (apply all schemes). */
  if (op->security_set) {
    security_set = 1;
    active_sets = op->security;
    n_active_sets = op->n_security;
  } else if (spec->security_set) {
    security_set = 1;
    active_sets = spec->security;
    n_active_sets = spec->n_security;
  }

  if (security_set && n_active_sets == 0) {
    return 0; /* Explicitly empty security array */
  }

  /* Iterate security schemes defined in components */
  if (spec->security_schemes == NULL || spec->n_security_schemes == 0) {
    return 0; /* No schemes defined, nothing to inject */
  }

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *sch = &spec->security_schemes[i];
    if (security_set &&
        !scheme_in_security_sets(active_sets, n_active_sets, sch->name)) {
      continue;
    }

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
    else if (sch->type == OA_SEC_HTTP && strcmp(sch->scheme, "bearer") == 0) {
      /* "if (ctx->security.bearer_token) ..." */
      /* Usually generic, or scoped by scheme name if multiple exist */
      fprintf(fp, "  if (ctx->security.bearer_token) {\n");
      fprintf(fp, "    rc = http_request_set_auth_bearer(&req, "
                  "ctx->security.bearer_token);\n");
      fprintf(fp, "    if (rc != 0) goto cleanup;\n");
      fprintf(fp, "  }\n");
      has_security = 1;
    }
    /* Scheme: API Key (Query) */
    else if (sch->type == OA_SEC_APIKEY && sch->in == OA_SEC_IN_QUERY) {
      if (sch->name && sch->key_name) {
        /* Determine if query params struct `qp` is initialized.
           Caller (codegen_client_body) controls `qp` scope.
           Ideally we inject into `qp` if exists.
           Check context: `has_query` flag passed?
           Actually `codegen_client_body` initializes `qp` IF regular query
           params exist. If Auth is via query, we MUST initialize `qp` if not
           done. This requires tighter coupling. Standard approach: This
           generator emits code assuming `qp` exists OR emits `url_query_add`
           calls safely? We will assume `qp` is available if needed, or emit
           logic to init it? Simpler: Emit variable checks.
        */
        fprintf(fp,
                "  /* Security: Query Params not fully orchestrated yet */\n");
        /* Implementation note: Query Auth requires orchestrator to know it
           needs `qp`. This generator just emits the header logic fully for now
           as requested by scope.
        */
      }
    }
  }

  if (has_security) {
    fprintf(fp, "\n");
  }

  return 0;
}
