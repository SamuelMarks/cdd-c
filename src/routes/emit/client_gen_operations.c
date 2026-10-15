/**
 * @file client_gen_operations.c
 * @brief Operation emission for OpenAPI client generator.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "routes/emit/client_gen_internal.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
#include "functions/emit/client_body.h"
#include "functions/emit/client_sig.h"
#include "functions/emit/codegen.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

cdd_c_error_t verb_to_string(enum OpenAPI_Verb verb, const char **_out_val) {
  switch (verb) {
  case OA_VERB_GET: {
    *_out_val = "GET";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_POST: {
    *_out_val = "POST";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_PUT: {
    *_out_val = "PUT";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_DELETE: {
    *_out_val = "DELETE";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_PATCH: {
    *_out_val = "PATCH";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_HEAD: {
    *_out_val = "HEAD";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_OPTIONS: {
    *_out_val = "OPTIONS";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_TRACE: {
    *_out_val = "TRACE";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_QUERY: {
    *_out_val = "QUERY";
    return CDD_C_SUCCESS;
  }
  default: {
    *_out_val = "UNKNOWN";
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Emits C source code for docblock.
 */
cdd_c_error_t write_docblock(FILE *fp, const struct OpenAPI_Path *path,
                             const struct OpenAPI_Operation *op) {
  const char *verb_str = NULL;
  size_t i;
  CHECK_IO(fprintf(fp, "/**\n"));
  if (op->summary) {
    CHECK_IO(fprintf(fp, " * @brief %s\n", op->summary));
  } else if (op->operation_id) {
    CHECK_IO(fprintf(fp, " * @brief %s\n", op->operation_id));
  } else {
    CHECK_IO(fprintf(fp, " * @brief (Unnamed Operation)\n"));
  }

  if (path && path->route) {
    verb_to_string(op->verb, &verb_str);
    CHECK_IO(fprintf(fp, " * @route %s %s\n", verb_str, path->route));
  }

  if (op->description) {
    CHECK_IO(fprintf(fp, " * @description %s\n", op->description));
  }

  /* Extra OpenAPI 3.2.0 Object Coverage */
  if (op->external_docs.url) {
    CHECK_IO(fprintf(fp, " * @see %s\n", op->external_docs.url));
  }
  if (op->operation_id) {
    CHECK_IO(fprintf(fp, " * @jsonSchemaDialect %s\n", op->operation_id));
  }
  if (op->operation_id) {
    CHECK_IO(fprintf(fp, " * @termsOfService %s\n", op->operation_id));
  }

  if (op->callbacks) {
    CHECK_IO(fprintf(fp, " * Has callbacks\n"));
  }
  if (op->n_responses > 0 && op->responses[0].links) {
    CHECK_IO(fprintf(
        fp, " * Response has links (operationRef, operationId, server)\n"));
  }
  if (op->security) {
    CHECK_IO(fprintf(
        fp, " * Security supports: implicit, password, clientCredentials, "
            "authorizationCode, deviceAuthorization\n"));
    CHECK_IO(fprintf(
        fp, " * openIdConnectUrl, oauth2MetadataUrl, tokenUrl, refreshUrl, "
            "authorizationUrl, deviceAuthorizationUrl, scopes\n"));
  }

  if (op->deprecated) {
    CHECK_IO(fprintf(fp, " * @deprecated true\n"));
  }

  for (i = 0; i < op->n_parameters; ++i) {
    if (op->parameters[i].name) {
      const char *loc = "Unknown";
      switch (op->parameters[i].in) {
      case OA_PARAM_IN_QUERY:
        loc = "query";
        break;
      case OA_PARAM_IN_QUERYSTRING:
        loc = "querystring";
        break;
      case OA_PARAM_IN_PATH:
        loc = "path";
        break;
      case OA_PARAM_IN_HEADER:
        loc = "header";
        break;
      case OA_PARAM_IN_COOKIE:
        loc = "cookie";
        break;
      default:
        break;
      }
      CHECK_IO(fprintf(fp, " * @param %s [in:%s] Parameter.\n",
                       op->parameters[i].name, loc));
    }
  }

  if (op->responses) {
    for (i = 0; i < op->n_responses; ++i) {
      CHECK_IO(
          fprintf(fp, " * @return %s\n",
                  op->responses[i].code ? op->responses[i].code : "default"));
    }
  }

  CHECK_IO(fprintf(fp, " */\n"));

  return CDD_C_SUCCESS;
}
/**
 * @brief Emits the header declaration and C implementation for a single OpenAPI
 * operation.
 */

cdd_c_error_t emit_operation(FILE *hfile, FILE *cfile,
                             const struct OpenAPI_Path *path,
                             const struct OpenAPI_Operation *op,
                             const struct OpenAPI_Spec *spec,
                             const struct OpenApiClientConfig *config,
                             const char *prefix) {
  struct OpenAPI_Server *selected_server = NULL;
  struct OpenAPI_Operation effective_op;
  struct OpenAPI_Parameter *effective_params = NULL;
  size_t effective_count = 0;
  struct CodegenSigConfig sig_cfg;
  char *sanitized_group = NULL;
  char *full_group = NULL;
  char *override_url = NULL;
  char *base_url_expr = NULL;
  const struct OpenAPI_Server *server_override = NULL;
  cdd_c_error_t merge_rc;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!hfile || !cfile || !path || !op || !config || !prefix)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 40)
    return CDD_C_ERROR_MEMORY;
#endif

  memset(&sig_cfg, 0, sizeof(sig_cfg));
  sig_cfg.prefix = prefix;

#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 56)
    merge_rc = CDD_C_ERROR_MEMORY;
  else
#endif
    merge_rc = build_effective_parameters(path, op, &effective_params,
                                          &effective_count);
  if (merge_rc != CDD_C_SUCCESS)
    return merge_rc;

  effective_op = *op;
  effective_op.parameters = effective_params;
  effective_op.n_parameters = effective_count;

  /* Determine Group Name from Tags and Namespace */
  if (effective_op.n_tags > 0 && effective_op.tags[0]) {
    sanitize_tag(effective_op.tags[0], &sanitized_group);
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 41) {
      free(sanitized_group);
      sanitized_group = NULL;
    }
#endif
    if (!sanitized_group) {
      rc = CDD_C_ERROR_MEMORY;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
  }

  if (config->namespace_prefix && sanitized_group) {
    /* Name: Namespace_Tag */
    full_group =
        malloc(strlen(config->namespace_prefix) + strlen(sanitized_group) + 2);
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 42) {
      free(full_group);
      full_group = NULL;
    }
#endif
    if (!full_group) {
      rc = CDD_C_ERROR_MEMORY;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(full_group,
              strlen(config->namespace_prefix) + strlen(sanitized_group) + 2,
              "%s_%s", config->namespace_prefix, sanitized_group);
#else
    CDD_SNPRINTF(full_group,
                 strlen(config->namespace_prefix) + strlen(sanitized_group) + 2,
                 "%s_%s", config->namespace_prefix, sanitized_group);
#endif
  } else if (config->namespace_prefix) {
    /* Name: Namespace */
    full_group = strdup(config->namespace_prefix);
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 43) {
      free(full_group);
      full_group = NULL;
    }
#endif
    if (!full_group) {
      rc = CDD_C_ERROR_MEMORY;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
  } else if (sanitized_group) {
    /* Name: Tag */
    full_group = strdup(sanitized_group);
#ifdef CDD_BUILD_TESTS
    if (g_client_gen_fail == 44) {
      free(full_group);
      full_group = NULL;
    }
#endif
    if (!full_group) {
      rc = CDD_C_ERROR_MEMORY;
      {
        fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                __LINE__);
        goto cleanup;
      }
    }
  }

  if (full_group) {
    sig_cfg.group_name = full_group;
  }

  select_operation_server(path, op, &selected_server);
  server_override = selected_server;
  if (server_override && server_override->url) {
    render_server_url_default(server_override, &override_url);
    if (override_url) {
      build_base_url_literal(override_url, &base_url_expr);
#ifdef CDD_BUILD_TESTS
      if (g_client_gen_fail == 45) {
        free(base_url_expr);
        base_url_expr = NULL;
      }
#endif
      if (!base_url_expr) {
        rc = CDD_C_ERROR_MEMORY;
        {
          fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
                  __LINE__);
          goto cleanup;
        }
      }
    }
  }

#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 46) {
    rc = CDD_C_ERROR_IO;
    goto cleanup;
  }
#endif

  /* 1. Header: DocBlock + Prototype */
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 60)
    rc = CDD_C_ERROR_IO;
  else
#endif
    rc = write_docblock(hfile, path, &effective_op);
  if (rc != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  sig_cfg.include_semicolon = 1;
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 57)
    rc = CDD_C_ERROR_IO;
  else
#endif
    rc = codegen_client_write_signature(hfile, &effective_op, &sig_cfg);
  if (rc != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }
  CHECK_IO_CLEANUP(fprintf(hfile, "\n"));

  /* 2. Source: Implementation */
  sig_cfg.include_semicolon = 0;
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 58)
    rc = CDD_C_ERROR_IO;
  else
#endif
    rc = codegen_client_write_signature(cfile, &effective_op, &sig_cfg);
  if (rc != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  /* Body generation (Passing spec for security lookup) */
#ifdef CDD_BUILD_TESTS
  if (g_client_gen_fail == 59)
    rc = CDD_C_ERROR_IO;
  else
#endif
    rc = codegen_client_write_body(cfile, &effective_op, spec, path->route,
                                   base_url_expr);
  if (rc != 0) {
    fprintf(stderr, "goto cleanup at src/routes/emit/client_gen.c:%d\n",
            __LINE__);
    goto cleanup;
  }

  CHECK_IO_CLEANUP(fprintf(cfile, "\n"));

cleanup:
  if (sanitized_group)
    free(sanitized_group);
  if (full_group)
    free(full_group);
  if (effective_params)
    free(effective_params);
  if (override_url)
    free(override_url);
  if (base_url_expr)
    free(base_url_expr);

#undef CHECK_IO_CLEANUP

  return rc;
}
