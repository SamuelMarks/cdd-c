/**
 * @file cli.c
 * @brief Implementation of CLI parsing.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../win_compat_sym.h"

#include "../../cdd_api.h"
#include "classes/emit/schema.h" /* For Type Registry */
#include "classes/parse/code2schema.h"
#include "classes/parse/inspector.h"
#include "docstrings/parse/doc.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
#include "openapi/emit/openapi.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/aggregator.h"
#include "routes/emit/operation.h" /* For OpBuilder and C2OpenAPI_ParsedSig */
#include "routes/parse/cli.h"
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

/* --- Helpers --- */

/**
 * @brief Checks whether the given path corresponds to a C source or header
 * file.
 *
 * @param[in] path File path to check.
 * @param[out] out_is_source Pointer receiving 1 if source/header file, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * pointers.
 */
cdd_c_error_t is_source_file(const char *path, int *out_is_source) {
  const char *ext;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_is_source_file;
  if (g_cli_fail_is_source_file) {
    g_cli_fail_is_source_file = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!path || !out_is_source)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_source = 0;
  ext = strrchr(path, '.');
  if (!ext)
    return CDD_C_SUCCESS;
  *out_is_source = (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the spec has tag operation.
 */
cdd_c_error_t spec_has_tag(const struct OpenAPI_Spec *spec, const char *name,
                           int *out_has_tag) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_spec_has_tag;
  if (g_cli_fail_spec_has_tag) {
    g_cli_fail_spec_has_tag = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!out_has_tag)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has_tag = 0;
  if (!spec || !name)
    return CDD_C_SUCCESS;
  for (i = 0; i < spec->n_tags; ++i) {
    if (spec->tags[i].name && strcmp(spec->tags[i].name, name) == 0) {
      *out_has_tag = 1;
      return CDD_C_SUCCESS;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the spec add tag operation.
 */
cdd_c_error_t spec_add_tag(struct OpenAPI_Spec *spec, const char *name) {
  struct OpenAPI_Tag *new_tags;
  struct OpenAPI_Tag *tag;

  if (!spec || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  {
    int has_tag = 0;
    cdd_c_error_t rc_tag = spec_has_tag(spec, name, &has_tag);
    if (rc_tag != CDD_C_SUCCESS)
      return rc_tag;
    if (has_tag)
      return CDD_C_SUCCESS;
  }

  new_tags = (struct OpenAPI_Tag *)C_CDD_REALLOC(
      spec->tags, (spec->n_tags + 1) * sizeof(struct OpenAPI_Tag));
  if (!new_tags)
    return CDD_C_ERROR_MEMORY;
  spec->tags = new_tags;
  tag = &spec->tags[spec->n_tags];
  memset(tag, 0, sizeof(*tag));
  {
    cdd_c_error_t rc = c_cdd_strdup(name, &tag->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  spec->n_tags++;

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the spec find tag operation.
 */
cdd_c_error_t spec_find_tag(struct OpenAPI_Spec *spec, const char *name,
                            struct OpenAPI_Tag **_out_val) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_spec_find_tag;
  if (g_cli_fail_spec_find_tag) {
    g_cli_fail_spec_find_tag = 0;
    return CDD_C_ERROR_MEMORY;
  }
#endif

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *_out_val = NULL;

  if (!spec || !name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < spec->n_tags; ++i) {
    if (spec->tags[i].name && strcmp(spec->tags[i].name, name) == 0) {
      *_out_val = &spec->tags[i];
      return CDD_C_SUCCESS;
    }
  }
  *_out_val = NULL;
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Maps DocSecurityType enumeration to OpenAPI_SecurityType.
 *
 * @param[in] type Input doc security type.
 * @param[out] out_val Output OpenAPI security type.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t
c2openapi_map_doc_security_type(enum DocSecurityType type,
                                enum OpenAPI_SecurityType *out_val) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_map_doc_security_type;
  if (g_cli_fail_map_doc_security_type) {
    g_cli_fail_map_doc_security_type = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (type) {
  case DOC_SEC_APIKEY: {
    *out_val = OA_SEC_APIKEY;
    return CDD_C_SUCCESS;
  }
  case DOC_SEC_HTTP: {
    *out_val = OA_SEC_HTTP;
    return CDD_C_SUCCESS;
  }
  case DOC_SEC_MUTUALTLS: {
    *out_val = OA_SEC_MUTUALTLS;
    return CDD_C_SUCCESS;
  }
  case DOC_SEC_OAUTH2: {
    *out_val = OA_SEC_OAUTH2;
    return CDD_C_SUCCESS;
  }
  case DOC_SEC_OPENID: {
    *out_val = OA_SEC_OPENID;
    return CDD_C_SUCCESS;
  }
  case DOC_SEC_UNSET:
  default: {
    *out_val = OA_SEC_UNKNOWN;
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Executes the map doc security in operation.
 */
cdd_c_error_t map_doc_security_in(enum DocSecurityIn in,
                                  enum OpenAPI_SecurityIn *_out_val) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_map_doc_security_in;
  if (g_cli_fail_map_doc_security_in) {
    g_cli_fail_map_doc_security_in = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (in) {
  case DOC_SEC_IN_QUERY: {
    *_out_val = OA_SEC_IN_QUERY;
    return CDD_C_SUCCESS;
  }
  case DOC_SEC_IN_HEADER: {
    *_out_val = OA_SEC_IN_HEADER;
    return CDD_C_SUCCESS;
  }
  case DOC_SEC_IN_COOKIE: {
    *_out_val = OA_SEC_IN_COOKIE;
    return CDD_C_SUCCESS;
  }
  case DOC_SEC_IN_UNSET:
  default: {
    *_out_val = OA_SEC_IN_UNKNOWN;
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Executes the map doc flow type operation.
 */
cdd_c_error_t map_doc_flow_type(enum DocOAuthFlowType type,
                                enum OpenAPI_OAuthFlowType *_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (type) {
  case DOC_OAUTH_FLOW_IMPLICIT: {
    *_out_val = OA_OAUTH_FLOW_IMPLICIT;
    return CDD_C_SUCCESS;
  }
  case DOC_OAUTH_FLOW_PASSWORD: {
    *_out_val = OA_OAUTH_FLOW_PASSWORD;
    return CDD_C_SUCCESS;
  }
  case DOC_OAUTH_FLOW_CLIENT_CREDENTIALS: {
    *_out_val = OA_OAUTH_FLOW_CLIENT_CREDENTIALS;
    return CDD_C_SUCCESS;
  }
  case DOC_OAUTH_FLOW_AUTHORIZATION_CODE: {
    *_out_val = OA_OAUTH_FLOW_AUTHORIZATION_CODE;
    return CDD_C_SUCCESS;
  }
  case DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION: {
    *_out_val = OA_OAUTH_FLOW_DEVICE_AUTHORIZATION;
    return CDD_C_SUCCESS;
  }
  case DOC_OAUTH_FLOW_UNSET:
  default: {
    *_out_val = OA_OAUTH_FLOW_UNKNOWN;
    return CDD_C_ERROR_UNKNOWN;
  }
  }
}

/**
 * @brief Executes the spec find security scheme operation.
 */
static cdd_c_error_t
spec_find_security_scheme(struct OpenAPI_Spec *spec, const char *name,
                          struct OpenAPI_SecurityScheme **_out_val) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_spec_find_security_scheme;
  if (g_cli_fail_spec_find_security_scheme) {
    g_cli_fail_spec_find_security_scheme = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  for (i = 0; i < spec->n_security_schemes; ++i) {
    if (strcmp(spec->security_schemes[i].name, name) == 0) {
      *_out_val = &spec->security_schemes[i];
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Adds or sets str if missing.
 */
static cdd_c_error_t set_str_if_missing(char **dst, const char *src) {
  if (!src || !*src)
    return CDD_C_SUCCESS;
  if (!*dst) {
    cdd_c_error_t rc = c_cdd_strdup(src, dst);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return CDD_C_SUCCESS;
  }
  if (strcmp(*dst, src) != 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  return CDD_C_SUCCESS;
}

/**
 * @brief Frees the memory associated with openapi server variables.
 */
void free_openapi_server_variables(struct OpenAPI_Server *srv) {
  size_t i;
  if (!srv || !srv->variables)
    return;
  for (i = 0; i < srv->n_variables; ++i) {
    size_t e;
    struct OpenAPI_ServerVariable *var = &srv->variables[i];
    if (var->name)
      C_CDD_FREE(var->name);
    if (var->default_value)
      C_CDD_FREE(var->default_value);
    if (var->description)
      C_CDD_FREE(var->description);
    if (var->enum_values) {
      for (e = 0; e < var->n_enum_values; ++e) {
        C_CDD_FREE(var->enum_values[e]);
      }
      C_CDD_FREE(var->enum_values);
    }
  }
  C_CDD_FREE(srv->variables);
  srv->variables = NULL;
  srv->n_variables = 0;
}

/**
 * @brief Creates a deep copy of doc server variables.
 */
cdd_c_error_t copy_doc_server_variables(struct OpenAPI_Server *dst,
                                        const struct DocServer *src) {
  size_t i;
  cdd_c_error_t rc;

  if (!dst || !src)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (src->n_variables == 0) {
    dst->variables = NULL;
    dst->n_variables = 0;
    return CDD_C_SUCCESS;
  }

  dst->variables = (struct OpenAPI_ServerVariable *)C_CDD_CALLOC(
      src->n_variables, sizeof(struct OpenAPI_ServerVariable));
  if (!dst->variables)
    return CDD_C_ERROR_MEMORY;
  dst->n_variables = src->n_variables;

  for (i = 0; i < src->n_variables; ++i) {
    size_t e;
    int found_default = 0;
    const struct DocServerVar *sv = &src->variables[i];
    struct OpenAPI_ServerVariable *dv = &dst->variables[i];

    if (!sv->name || !sv->default_value) {
      free_openapi_server_variables(dst);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    rc = c_cdd_strdup(sv->name, &dv->name);
    if (rc != CDD_C_SUCCESS) {
      free_openapi_server_variables(dst);
      return rc;
    }
    rc = c_cdd_strdup(sv->default_value, &dv->default_value);
    if (rc != CDD_C_SUCCESS) {
      free_openapi_server_variables(dst);
      return rc;
    }
    if (sv->description) {
      rc = c_cdd_strdup(sv->description, &dv->description);
      if (rc != CDD_C_SUCCESS) {
        free_openapi_server_variables(dst);
        return rc;
      }
    }
    if (sv->enum_values && sv->n_enum_values > 0) {
      dv->enum_values =
          (char **)C_CDD_CALLOC(sv->n_enum_values, sizeof(char *));
      if (!dv->enum_values) {
        free_openapi_server_variables(dst);
        return CDD_C_ERROR_MEMORY;
      }
      dv->n_enum_values = sv->n_enum_values;
      for (e = 0; e < sv->n_enum_values; ++e) {
        rc = c_cdd_strdup(sv->enum_values[e], &dv->enum_values[e]);
        if (rc != CDD_C_SUCCESS) {
          free_openapi_server_variables(dst);
          return rc;
        }
        if (strcmp(sv->enum_values[e], sv->default_value) == 0)
          found_default = 1;
      }
      if (!found_default) {
        free_openapi_server_variables(dst);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Merges scopes.
 */
cdd_c_error_t merge_scopes(struct OpenAPI_OAuthFlow *dst,
                           const struct DocOAuthFlow *src) {
  size_t i;
  cdd_c_error_t rc;

  if (!dst || !src)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < src->n_scopes; ++i) {
    const char *name = src->scopes[i].name;
    const char *desc = src->scopes[i].description;
    size_t j;
    int found = 0;
    for (j = 0; j < dst->n_scopes; ++j) {
      if (dst->scopes[j].name && name &&
          strcmp(dst->scopes[j].name, name) == 0) {
        found = 1;
        break;
      }
    }
    if (!found) {
      struct OpenAPI_OAuthScope *new_scopes =
          (struct OpenAPI_OAuthScope *)C_CDD_REALLOC(
              dst->scopes, (dst->n_scopes + 1) * sizeof(*dst->scopes));
      if (!new_scopes)
        return CDD_C_ERROR_MEMORY;
      dst->scopes = new_scopes;
      memset(&dst->scopes[dst->n_scopes], 0, sizeof(*dst->scopes));
      rc = c_cdd_strdup(name ? name : "", &dst->scopes[dst->n_scopes].name);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (desc) {
        rc = c_cdd_strdup(desc, &dst->scopes[dst->n_scopes].description);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      dst->n_scopes++;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the oauth flow.
 */
cdd_c_error_t find_oauth_flow(struct OpenAPI_SecurityScheme *scheme,
                              enum OpenAPI_OAuthFlowType type,
                              struct OpenAPI_OAuthFlow **_out_val) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_find_oauth_flow;
  if (g_cli_fail_find_oauth_flow) {
    g_cli_fail_find_oauth_flow = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!scheme || !scheme->flows) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < scheme->n_flows; ++i) {
    if (scheme->flows[i].type == type) {
      *_out_val = &scheme->flows[i];
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Merges oauth flow.
 */
cdd_c_error_t merge_oauth_flow(struct OpenAPI_OAuthFlow *dst,
                               const struct DocOAuthFlow *src) {
  cdd_c_error_t rc;
  rc = set_str_if_missing(&dst->authorization_url, src->authorization_url);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = set_str_if_missing(&dst->token_url, src->token_url);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = set_str_if_missing(&dst->refresh_url, src->refresh_url);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = set_str_if_missing(&dst->device_authorization_url,
                          src->device_authorization_url);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return merge_scopes(dst, src);
}

/**
 * @brief Executes the validate doc oauth flow operation.
 */
cdd_c_error_t validate_doc_oauth_flow(const struct DocOAuthFlow *flow) {
  if (!flow)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (flow->type == DOC_OAUTH_FLOW_UNSET)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (flow->type) {
  case DOC_OAUTH_FLOW_IMPLICIT:
    if (!flow->authorization_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    break;
  case DOC_OAUTH_FLOW_PASSWORD:
    if (!flow->token_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    break;
  case DOC_OAUTH_FLOW_CLIENT_CREDENTIALS:
    if (!flow->token_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    break;
  case DOC_OAUTH_FLOW_AUTHORIZATION_CODE:
    if (!flow->authorization_url || !flow->token_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    break;
  case DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION:
    if (!flow->device_authorization_url || !flow->token_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    break;
  default:
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Adds or sets oauth flows.
 */
cdd_c_error_t add_oauth_flows(struct OpenAPI_SecurityScheme *scheme,
                              const struct DocSecurityScheme *doc) {
  size_t i;
  cdd_c_error_t rc;

  if (!scheme || !doc)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < doc->n_flows; ++i) {
    enum OpenAPI_OAuthFlowType flow_type;
    struct OpenAPI_OAuthFlow *dst_flow = NULL;
    cdd_c_error_t rc_map;

    rc_map = map_doc_flow_type(doc->flows[i].type, &flow_type);
    if (rc_map != CDD_C_SUCCESS)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    rc = find_oauth_flow(scheme, flow_type, &dst_flow);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (dst_flow) {
      rc = merge_oauth_flow(dst_flow, &doc->flows[i]);
      if (rc != CDD_C_SUCCESS)
        return rc;
      continue;
    }
    {
      struct OpenAPI_OAuthFlow *new_flows =
          (struct OpenAPI_OAuthFlow *)C_CDD_REALLOC(
              scheme->flows,
              (scheme->n_flows + 1) * sizeof(struct OpenAPI_OAuthFlow));
      if (!new_flows)
        return CDD_C_ERROR_MEMORY;
      scheme->flows = new_flows;
      dst_flow = &scheme->flows[scheme->n_flows];
      memset(dst_flow, 0, sizeof(*dst_flow));
      dst_flow->type = flow_type;
      if (doc->flows[i].authorization_url) {
        rc = c_cdd_strdup(doc->flows[i].authorization_url,
                          &dst_flow->authorization_url);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (doc->flows[i].token_url) {
        rc = c_cdd_strdup(doc->flows[i].token_url, &dst_flow->token_url);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (doc->flows[i].refresh_url) {
        rc = c_cdd_strdup(doc->flows[i].refresh_url, &dst_flow->refresh_url);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (doc->flows[i].device_authorization_url) {
        rc = c_cdd_strdup(doc->flows[i].device_authorization_url,
                          &dst_flow->device_authorization_url);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (doc->flows[i].scopes && doc->flows[i].n_scopes > 0) {
        size_t s;
        dst_flow->scopes = (struct OpenAPI_OAuthScope *)C_CDD_CALLOC(
            doc->flows[i].n_scopes, sizeof(struct OpenAPI_OAuthScope));
        if (!dst_flow->scopes)
          return CDD_C_ERROR_MEMORY;
        dst_flow->n_scopes = doc->flows[i].n_scopes;
        for (s = 0; s < doc->flows[i].n_scopes; ++s) {
          const char *name = doc->flows[i].scopes[s].name;
          const char *desc = doc->flows[i].scopes[s].description;
          rc = c_cdd_strdup(name ? name : "", &dst_flow->scopes[s].name);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (desc) {
            rc = c_cdd_strdup(desc, &dst_flow->scopes[s].description);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      }
      scheme->n_flows++;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the spec add security scheme operation.
 */
cdd_c_error_t spec_add_security_scheme(struct OpenAPI_Spec *spec,
                                       const struct DocSecurityScheme *doc) {
  struct OpenAPI_SecurityScheme *scheme;
  enum OpenAPI_SecurityType type;
  cdd_c_error_t rc;

  if (!spec || !doc || !doc->name || !*doc->name)
    return CDD_C_SUCCESS;

  rc = c2openapi_map_doc_security_type(doc->type, &type);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (type == OA_SEC_UNKNOWN) {
    fprintf(stderr, "Warning: Unknown security scheme type ignored: %s\n",
            doc->name);
    return CDD_C_SUCCESS;
  }

  if (type == OA_SEC_OAUTH2 && doc->n_flows > 0) {
    size_t i;
    for (i = 0; i < doc->n_flows; ++i) {
      rc = validate_doc_oauth_flow(&doc->flows[i]);
      if (rc != CDD_C_SUCCESS) {
        fprintf(stderr, "Warning: Invalid OAuth flow ignored: %s\n", doc->name);
        return CDD_C_SUCCESS;
      }
    }
  }

  scheme = NULL;
  rc = spec_find_security_scheme(spec, doc->name, &scheme);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!scheme) {
    struct OpenAPI_SecurityScheme *new_schemes =
        (struct OpenAPI_SecurityScheme *)C_CDD_REALLOC(
            spec->security_schemes, (spec->n_security_schemes + 1) *
                                        sizeof(struct OpenAPI_SecurityScheme));
    cdd_c_error_t rc_str;
    if (!new_schemes)
      return CDD_C_ERROR_MEMORY;
    spec->security_schemes = new_schemes;
    scheme = &spec->security_schemes[spec->n_security_schemes];
    memset(scheme, 0, sizeof(*scheme));
    rc_str = c_cdd_strdup(doc->name, &scheme->name);
    if (rc_str != CDD_C_SUCCESS)
      return rc_str;
    scheme->type = type;
    spec->n_security_schemes++;
  } else if (scheme->type != type) {
    fprintf(stderr, "Warning: Security scheme type collision ignored: %s\n",
            doc->name);
    return CDD_C_SUCCESS;
  }

  if (doc->description) {
    rc = set_str_if_missing(&scheme->description, doc->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (doc->deprecated_set) {
    if (!scheme->deprecated_set) {
      scheme->deprecated_set = 1;
      scheme->deprecated = doc->deprecated;
    } else if (scheme->deprecated != doc->deprecated) {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }

  if (type == OA_SEC_APIKEY) {
    enum OpenAPI_SecurityIn in;
    rc = map_doc_security_in(doc->in, &in);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!doc->param_name || !*doc->param_name || in == OA_SEC_IN_UNKNOWN)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    scheme->in = in;
    {
      rc = set_str_if_missing(&scheme->key_name, doc->param_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  } else if (type == OA_SEC_HTTP) {
    if (!doc->scheme || !*doc->scheme)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    {
      rc = set_str_if_missing(&scheme->scheme, doc->scheme);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (doc->bearer_format) {
      rc = set_str_if_missing(&scheme->bearer_format, doc->bearer_format);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  } else if (type == OA_SEC_OAUTH2) {
    if (doc->oauth2_metadata_url) {
      rc = set_str_if_missing(&scheme->oauth2_metadata_url,
                              doc->oauth2_metadata_url);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (doc->n_flows > 0) {
      rc = add_oauth_flows(scheme, doc);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (scheme->n_flows == 0) {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  } else if (type == OA_SEC_OPENID) {
    if (!doc->open_id_connect_url || !*doc->open_id_connect_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    {
      rc = set_str_if_missing(&scheme->open_id_connect_url,
                              doc->open_id_connect_url);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies doc security schemes.
 */
cdd_c_error_t apply_doc_security_schemes(struct OpenAPI_Spec *spec,
                                         const struct DocMetadata *meta) {
  size_t i;
  cdd_c_error_t rc;
  if (!spec || !meta || meta->n_security_schemes == 0)
    return CDD_C_SUCCESS;
  for (i = 0; i < meta->n_security_schemes; ++i) {
    rc = spec_add_security_scheme(spec, &meta->security_schemes[i]);
    if (rc == CDD_C_ERROR_MEMORY)
      return rc;
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Warning: Failed to add security scheme, ignoring.\n");
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the append root security operation.
 */
cdd_c_error_t append_root_security(struct OpenAPI_Spec *spec,
                                   const struct DocMetadata *meta) {
  size_t i;
  if (!spec || !meta || meta->n_security == 0)
    return CDD_C_SUCCESS;

  {
    struct OpenAPI_SecurityRequirementSet *new_sets =
        (struct OpenAPI_SecurityRequirementSet *)C_CDD_REALLOC(
            spec->security, (spec->n_security + meta->n_security) *
                                sizeof(struct OpenAPI_SecurityRequirementSet));
    if (!new_sets)
      return CDD_C_ERROR_MEMORY;
    spec->security = new_sets;
  }
  for (i = 0; i < meta->n_security; ++i) {
    const struct DocSecurityRequirement *src = &meta->security[i];
    struct OpenAPI_SecurityRequirementSet *set =
        &spec->security[spec->n_security + i];
    memset(set, 0, sizeof(*set));
    set->requirements = (struct OpenAPI_SecurityRequirement *)C_CDD_CALLOC(
        1, sizeof(struct OpenAPI_SecurityRequirement));
    if (!set->requirements)
      return CDD_C_ERROR_MEMORY;
    set->n_requirements = 1;
    {
      cdd_c_error_t rc_str = c_cdd_strdup(src->scheme ? src->scheme : "",
                                          &set->requirements[0].scheme);
      if (rc_str != CDD_C_SUCCESS)
        return rc_str;
    }
    if (src->n_scopes > 0) {
      size_t s;
      set->requirements[0].scopes =
          (char **)C_CDD_CALLOC(src->n_scopes, sizeof(char *));
      if (!set->requirements[0].scopes)
        return CDD_C_ERROR_MEMORY;
      set->requirements[0].n_scopes = src->n_scopes;
      for (s = 0; s < src->n_scopes; ++s) {
        cdd_c_error_t rc_str =
            c_cdd_strdup(src->scopes[s] ? src->scopes[s] : "",
                         &set->requirements[0].scopes[s]);
        if (rc_str != CDD_C_SUCCESS)
          return rc_str;
      }
    }
  }
  spec->n_security += meta->n_security;
  spec->security_set = 1;

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the append root servers operation.
 */
cdd_c_error_t append_root_servers(struct OpenAPI_Spec *spec,
                                  const struct DocMetadata *meta) {
  size_t i;
  if (!spec || !meta || meta->n_servers == 0)
    return CDD_C_SUCCESS;
  {
    struct OpenAPI_Server *new_servers = (struct OpenAPI_Server *)C_CDD_REALLOC(
        spec->servers,
        (spec->n_servers + meta->n_servers) * sizeof(struct OpenAPI_Server));
    if (!new_servers)
      return CDD_C_ERROR_MEMORY;
    spec->servers = new_servers;
  }
  for (i = 0; i < meta->n_servers; ++i) {
    const struct DocServer *src = &meta->servers[i];
    struct OpenAPI_Server *dst = &spec->servers[spec->n_servers + i];
    memset(dst, 0, sizeof(*dst));
    if (src->url) {
      cdd_c_error_t rc_str = c_cdd_strdup(src->url, &dst->url);
      if (rc_str != CDD_C_SUCCESS) {
        spec->n_servers += i;
        return rc_str;
      }
    }
    if (src->name) {
      cdd_c_error_t rc_str = c_cdd_strdup(src->name, &dst->name);
      if (rc_str != CDD_C_SUCCESS) {
        C_CDD_FREE(dst->url);
        spec->n_servers += i;
        return rc_str;
      }
    }
    if (src->description) {
      cdd_c_error_t rc_str = c_cdd_strdup(src->description, &dst->description);
      if (rc_str != CDD_C_SUCCESS) {
        C_CDD_FREE(dst->url);
        C_CDD_FREE(dst->name);
        spec->n_servers += i;
        return rc_str;
      }
    }
    if (src->n_variables > 0) {
      cdd_c_error_t vrc = copy_doc_server_variables(dst, src);
      if (vrc != CDD_C_SUCCESS) {
        C_CDD_FREE(dst->url);
        C_CDD_FREE(dst->name);
        C_CDD_FREE(dst->description);
        spec->n_servers += i;
        return vrc;
      }
    }
  }
  spec->n_servers += meta->n_servers;

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies doc global meta.
 */
cdd_c_error_t apply_doc_global_meta(struct OpenAPI_Spec *spec,
                                    const struct DocMetadata *meta) {
  cdd_c_error_t rc;
  if (!spec || !meta)
    return CDD_C_SUCCESS;
  if (meta->json_schema_dialect) {
    rc = set_str_if_missing(&spec->json_schema_dialect,
                            meta->json_schema_dialect);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_title) {
    rc = set_str_if_missing(&spec->info.title, meta->info_title);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_version) {
    rc = set_str_if_missing(&spec->info.version, meta->info_version);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_summary) {
    rc = set_str_if_missing(&spec->info.summary, meta->info_summary);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->info_description) {
    rc = set_str_if_missing(&spec->info.description, meta->info_description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->terms_of_service) {
    rc = set_str_if_missing(&spec->info.terms_of_service,
                            meta->terms_of_service);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->contact_name || meta->contact_url || meta->contact_email) {
    if (meta->contact_name) {
      rc = set_str_if_missing(&spec->info.contact.name, meta->contact_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (meta->contact_url) {
      rc = set_str_if_missing(&spec->info.contact.url, meta->contact_url);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (meta->contact_email) {
      rc = set_str_if_missing(&spec->info.contact.email, meta->contact_email);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (meta->license_name || meta->license_url || meta->license_identifier) {
    if (!meta->license_name && !spec->info.license.name)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (meta->license_url && meta->license_identifier)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (spec->info.license.url && meta->license_identifier)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (spec->info.license.identifier && meta->license_url)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    if (meta->license_name) {
      rc = set_str_if_missing(&spec->info.license.name, meta->license_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (meta->license_url) {
      rc = set_str_if_missing(&spec->info.license.url, meta->license_url);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (meta->license_identifier) {
      rc = set_str_if_missing(&spec->info.license.identifier,
                              meta->license_identifier);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  if (meta->external_docs_url) {
    if (!spec->external_docs.url) {
      rc = c_cdd_strdup(meta->external_docs_url, &spec->external_docs.url);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (meta->external_docs_description) {
        rc = c_cdd_strdup(meta->external_docs_description,
                          &spec->external_docs.description);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (strcmp(spec->external_docs.url, meta->external_docs_url) != 0) {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    if (!spec->external_docs.description && meta->external_docs_description) {
      rc = c_cdd_strdup(meta->external_docs_description,
                        &spec->external_docs.description);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  rc = append_root_servers(spec, meta);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = append_root_security(spec, meta);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the spec apply tag meta operation.
 */
cdd_c_error_t spec_apply_tag_meta(struct OpenAPI_Spec *spec,
                                  const struct DocTagMeta *meta) {
  struct OpenAPI_Tag *tag;
  cdd_c_error_t rc;
  if (!spec || !meta || !meta->name || !*meta->name)
    return CDD_C_SUCCESS;
  tag = NULL;
  rc = spec_find_tag(spec, meta->name, &tag);
  if (rc != CDD_C_SUCCESS && rc != CDD_C_ERROR_UNKNOWN)
    return rc;
  if (!tag) {
    rc = spec_add_tag(spec, meta->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
    tag = &spec->tags[spec->n_tags - 1];
  }
  if (meta->summary && !tag->summary) {
    rc = c_cdd_strdup(meta->summary, &tag->summary);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->description && !tag->description) {
    rc = c_cdd_strdup(meta->description, &tag->description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->parent && !tag->parent) {
    rc = c_cdd_strdup(meta->parent, &tag->parent);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->kind && !tag->kind) {
    rc = c_cdd_strdup(meta->kind, &tag->kind);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->external_docs_url && !tag->external_docs.url) {
    rc = c_cdd_strdup(meta->external_docs_url, &tag->external_docs.url);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (meta->external_docs_description && !tag->external_docs.description &&
      tag->external_docs.url) {
    rc = c_cdd_strdup(meta->external_docs_description,
                      &tag->external_docs.description);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies doc tag meta.
 */
cdd_c_error_t apply_doc_tag_meta(struct OpenAPI_Spec *spec,
                                 const struct DocMetadata *meta) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!spec || !meta || !meta->tag_meta || meta->n_tag_meta == 0)
    return CDD_C_SUCCESS;
  for (i = 0; i < meta->n_tag_meta; ++i) {
    rc = spec_apply_tag_meta(spec, &meta->tag_meta[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Collects tags from op.
 */
cdd_c_error_t collect_tags_from_op(struct OpenAPI_Spec *spec,
                                   const struct OpenAPI_Operation *op) {
  size_t i;
  if (!spec || !op || !op->tags)
    return CDD_C_SUCCESS;
  for (i = 0; i < op->n_tags; ++i) {
    cdd_c_error_t rc = spec_add_tag(spec, op->tags[i]);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Collects tags from paths.
 */
cdd_c_error_t collect_tags_from_paths(struct OpenAPI_Spec *spec,
                                      const struct OpenAPI_Path *paths,
                                      size_t n_paths) {
  size_t i;
  if (!spec || !paths)
    return CDD_C_SUCCESS;
  for (i = 0; i < n_paths; ++i) {
    size_t j;
    const struct OpenAPI_Path *path = &paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      cdd_c_error_t rc = collect_tags_from_op(spec, &path->operations[j]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    for (j = 0; j < path->n_additional_operations; ++j) {
      cdd_c_error_t rc =
          collect_tags_from_op(spec, &path->additional_operations[j]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Collects spec tags.
 */
cdd_c_error_t collect_spec_tags(struct OpenAPI_Spec *spec) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cli_fail_collect_spec_tags;
  if (g_cli_fail_collect_spec_tags) {
    g_cli_fail_collect_spec_tags = 0;
    return CDD_C_ERROR_MEMORY;
  }
#endif
  if (!spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = collect_tags_from_paths(spec, spec->paths, spec->n_paths);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = collect_tags_from_paths(spec, spec->webhooks, spec->n_webhooks);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

/**
 * @brief Applies all doc metadata (tags, security schemes, and global).
 */
cdd_c_error_t c2openapi_apply_all_doc_meta(struct OpenAPI_Spec *spec,
                                           const struct DocMetadata *meta) {
  cdd_c_error_t rc = apply_doc_tag_meta(spec, meta);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = apply_doc_security_schemes(spec, meta);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return apply_doc_global_meta(spec, meta);
}

/**
 * @brief Simple signature parser to split "int foo(int x, char
 * *y)" Populates `out`. Caller must free internals.
 */
cdd_c_error_t parse_c_signature_string(const char *sig_str,
                                       struct C2OpenAPI_ParsedSig *out) {
  struct TokenList *tl = NULL;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  size_t lp = 0, rp = 0;

  if (!sig_str || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(out, 0, sizeof(*out));

  if (tokenize(az_span_create_from_str((char *)(size_t)sig_str), &tl) != 0) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  /* Naive extraction: Name is identifier before LPAREN */
  for (i = 0; i < tl->size; ++i) {
    if (tl->tokens[i].kind == TOKEN_LPAREN) {
      lp = i;
      break;
    }
  }

  /* Need at least name and parens */
  if (lp < 1) {
    free_token_list(tl);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  /* Extract Name (token before lparen, ignoring WS) */
  {
    size_t k = lp - 1;
    while (k > 0 && tl->tokens[k].kind == TOKEN_WHITESPACE)
      k--;
    if (tl->tokens[k].kind == TOKEN_IDENTIFIER) {
      size_t len = tl->tokens[k].length;
      char *n = C_CDD_MALLOC(len + 1);
      if (!n) {
        rc = CDD_C_ERROR_MEMORY;
        goto cleanup;
      }
      memcpy(n, tl->tokens[k].start, len);
      n[len] = '\0';
      out->name = n;
    }
  }

  if (!out->name) {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  /* Extract Args between ( and ) */
  /* Split by COMMA. For each segment, last ID is name, rest is
   * type. */
  rc = token_find_next(tl, lp, tl->size, TOKEN_RPAREN, &rp);
  if (rc != CDD_C_SUCCESS) {
    goto cleanup;
  }
  if (rp >= tl->size) {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (rp > lp + 1) {
    /* Non-empty args */
    size_t start = lp + 1;
    size_t end = start;

    while (end < rp) {
      /* Find comma or RP */
      size_t seg_end = end;
      while (seg_end < rp && tl->tokens[seg_end].kind != TOKEN_COMMA)
        seg_end++;

      /* Process segment [start, seg_end) */
      {
        /* Find name: last identifier in segment */
        size_t k = seg_end;
        size_t name_idx = 0;
        int found_name = 0;

        while (k > start) {
          k--;
          if (tl->tokens[k].kind == TOKEN_IDENTIFIER) {
            name_idx = k;
            found_name = 1;
            break;
          }
        }

        if (found_name) {
          /* Type is start..name_idx (exclusive) + modifiers
           * after? */
          /* Simple approach: Type is [start, name_idx), Name is
             name_idx. Postfix arrays `[]` might be after name.
           */
          /* Let's grab name string */
          const struct Token *nt = &tl->tokens[name_idx];
          size_t t_end = name_idx;

          /* Check if type is pointer/const/struct before name
           */
          /* Construct type string */
          size_t t_len = 0;
          char *t_str;
          size_t m;
          char *n_str = C_CDD_MALLOC(nt->length + 1);
          if (!n_str) {
            rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
          }
          memcpy(n_str, nt->start, nt->length);
          n_str[nt->length] = '\0';

          /* Calc type len */
          for (m = start; m < t_end; m++)
            t_len += tl->tokens[m].length;
          /* Add postfix */
          for (m = name_idx + 1; m < seg_end; m++)
            t_len += tl->tokens[m].length;

          t_str = C_CDD_MALLOC(t_len + 1);
          if (!t_str) {
            C_CDD_FREE(n_str);
            rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
          }
          {
            char *p = t_str;
            for (m = start; m < t_end; m++) {
              memcpy(p, tl->tokens[m].start, tl->tokens[m].length);
              p += tl->tokens[m].length;
            }
            for (m = name_idx + 1; m < seg_end; m++) {
              memcpy(p, tl->tokens[m].start, tl->tokens[m].length);
              p += tl->tokens[m].length;
            }
            *p = '\0';
          }
          c_cdd_str_trim_trailing_whitespace(t_str);
          {
            char *tp = t_str;
            while (*tp == ' ' || *tp == '\t')
              tp++;
            if (tp > t_str)
              memmove(t_str, tp, strlen(tp) + 1);
          }

          /* Add to list */
          {
            struct C2OpenAPI_ParsedArg *new_arr = C_CDD_REALLOC(
                out->args,
                (out->n_args + 1) * sizeof(struct C2OpenAPI_ParsedArg));
            if (!new_arr) {
              C_CDD_FREE(n_str);
              C_CDD_FREE(t_str);
              rc = CDD_C_ERROR_MEMORY;
              goto cleanup;
            }
            out->args = new_arr;
            out->args[out->n_args].name = n_str;
            out->args[out->n_args].type = t_str;
            out->n_args++;
          }

        } else {
          /* Void arg or unnamed? ignore */
        }
      }

      start = seg_end + 1; /* Skip comma */
      end = start;
    }
  }

cleanup:
  free_token_list(tl);
  if (rc != CDD_C_SUCCESS) {
    free_parsed_sig(out);
  }
  return rc;
}

/**
 * @brief Frees the memory associated with parsed sig.
 */
void free_parsed_sig(struct C2OpenAPI_ParsedSig *sig) {
  size_t i;
  if (!sig)
    return;
  if (sig->name)
    C_CDD_FREE(sig->name);
  if (sig->return_type)
    C_CDD_FREE(sig->return_type);
  if (sig->args) {
    for (i = 0; i < sig->n_args; ++i) {
      C_CDD_FREE(sig->args[i].name);
      C_CDD_FREE(sig->args[i].type);
    }
    C_CDD_FREE(sig->args);
  }
  memset(sig, 0, sizeof(*sig));
}

/**
 * @brief Executes the process file operation.
 */
cdd_c_error_t process_file(const char *path, struct OpenAPI_Spec *spec) {
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  struct CstNodeList cst = {0};
  int *comment_used = NULL;
  cdd_c_error_t rc;
  size_t i;

  if (!path || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Register Types (Structs/Enums) */
  {
    struct TypeDefList types;
    type_def_list_init(&types);
    if (c_inspector_scan_file_types(path, &types) == 0) {
      c2openapi_register_types(spec, &types);
    }
    type_def_list_free(&types);
  }

  /* 2. Parse Code for Functions & Docs */
  rc = read_to_file(path, "r", &content, &sz);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (tokenize(az_span_create_from_str(content), &tokens) != 0) {
    C_CDD_FREE(content);
    return CDD_C_ERROR_IO;
  }
  parse_tokens(tokens, &cst); /* Best effort */

  if (cst.size > 0) {
    comment_used = (int *)C_CDD_CALLOC(cst.size, sizeof(int));
    if (!comment_used) {
      free_cst_node_list(&cst);
      free_token_list(tokens);
      C_CDD_FREE(content);
      return CDD_C_ERROR_MEMORY;
    }
  }

  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_FUNCTION) {
      struct CstNode *func_node = &cst.nodes[i];
      struct CstNode *doc_node = NULL;
      size_t doc_index = (size_t)-1;

      /* Look backwards for doc comment */
      if (i > 0 && cst.nodes[i - 1].kind == CST_NODE_COMMENT) {
        doc_node = &cst.nodes[i - 1];
        doc_index = i - 1;
      }

      if (doc_node) {
        /* Extract comment text */
        char *doc_text = C_CDD_MALLOC(doc_node->length + 1);
        if (doc_text) {
          struct DocMetadata meta;
          memcpy(doc_text, doc_node->start, doc_node->length);
          doc_text[doc_node->length] = '\0';

          doc_metadata_init(&meta);
          if (doc_parse_block(doc_text, &meta) == 0) {
            cdd_c_error_t rc_meta = apply_all_doc_meta(spec, &meta);
            if (rc_meta != CDD_C_SUCCESS) {
              doc_metadata_free(&meta);
              C_CDD_FREE(doc_text);
              free_cst_node_list(&cst);
              free_token_list(tokens);
              C_CDD_FREE(content);
              C_CDD_FREE(comment_used);
              return rc_meta;
            }
            comment_used[doc_index] = 1;
          }
          if (meta.route) {
            /* Found Valid Documented Route! */

            /* Extract Signature Text */
            size_t sig_len = func_node->length; /* Approximation, includes
                                                   body? */
            /* We need signature string up to brace. CST Node
             * includes body. */
            char *sig_raw = (char *)C_CDD_MALLOC(sig_len + 1);
            if (sig_raw) {
              struct C2OpenAPI_ParsedSig psig;
              const uint8_t *brace = memchr(func_node->start, '{', sig_len);
              size_t effective_len = (size_t)(brace - func_node->start);

              memcpy(sig_raw, func_node->start, effective_len);
              sig_raw[effective_len] = '\0';

              if (parse_c_signature_string(sig_raw, &psig) == CDD_C_SUCCESS) {
                struct OpenAPI_Operation op = {0};
                struct OpBuilderContext ctx;

                ctx.sig = &psig;
                ctx.doc = &meta;
                ctx.func_name = psig.name;

                if (c2openapi_build_operation(&ctx, &op) == CDD_C_SUCCESS) {
                  if (meta.is_webhook) {
                    openapi_aggregator_add_webhook_operation(spec, meta.route,
                                                             &op);
                  } else {
                    openapi_aggregator_add_operation(spec, meta.route, &op);
                  }
                }
                free_parsed_sig(&psig);
              }
              C_CDD_FREE(sig_raw);
            }
          }
          doc_metadata_free(&meta);
          C_CDD_FREE(doc_text);
        }
      }
    }
  }

  /* Parse standalone comment blocks for global metadata. */
  if (comment_used) {
    for (i = 0; i < cst.size; ++i) {
      if (cst.nodes[i].kind == CST_NODE_COMMENT && !comment_used[i]) {
        struct DocMetadata meta;
        char *doc_text = C_CDD_MALLOC(cst.nodes[i].length + 1);
        if (!doc_text)
          continue;
        memcpy(doc_text, cst.nodes[i].start, cst.nodes[i].length);
        doc_text[cst.nodes[i].length] = '\0';

        doc_metadata_init(&meta);
        if (doc_parse_block(doc_text, &meta) == 0) {
          cdd_c_error_t rc_meta = apply_all_doc_meta(spec, &meta);
          if (rc_meta != CDD_C_SUCCESS) {
            doc_metadata_free(&meta);
            C_CDD_FREE(doc_text);
            free_cst_node_list(&cst);
            free_token_list(tokens);
            C_CDD_FREE(content);
            C_CDD_FREE(comment_used);
            return rc_meta;
          }
        }
        doc_metadata_free(&meta);
        C_CDD_FREE(doc_text);
      }
    }
  }

  free_cst_node_list(&cst);
  free_token_list(tokens);
  C_CDD_FREE(content);
  C_CDD_FREE(comment_used);

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the walker cb operation.
 */
cdd_c_error_t walker_cb(const char *path, void *user_data) {
  struct OpenAPI_Spec *spec = (struct OpenAPI_Spec *)user_data;
  if (!path || !user_data)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  {
    int is_src = 0;
    cdd_c_error_t rc_src = is_source_file(path, &is_src);
    if (rc_src != CDD_C_SUCCESS)
      return rc_src;
    if (!is_src)
      return CDD_C_SUCCESS;
  }
  printf("Scanning: %s\n", path);
  {

    cdd_c_error_t rc = process_file(path, spec);

    if (rc == CDD_C_ERROR_MEMORY)
      return rc;
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Warning: Failed to process %s (error %d), skipping.\n",
              path, rc);
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the load base spec operation.
 */
cdd_c_error_t load_base_spec(const char *path, struct OpenAPI_Spec *spec) {
  JSON_Value *root = NULL;
  cdd_c_error_t rc;

  if (!path || !spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  root = json_parse_file(path);
  if (!root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = openapi_load_from_json(root, spec);
  json_value_free(root);
  return rc;
}

/**
 * @brief Executes the c2openapi cli main operation.
 */
C_CDD_EXPORT cdd_c_error_t c2openapi_cli_main(int argc, char **argv) {
  struct OpenAPI_Spec spec;
  const char *src_dir;
  const char *out_file;
  const char *base_file = NULL;
  const char *self_uri = NULL;
  const char *dialect_uri = NULL;
  char *json = NULL;
  cdd_c_error_t rc;
  int argi = 1;

  while (argi < argc && argv[argi][0] == '-') {
    if (strcmp(argv[argi], "--base") == 0 || strcmp(argv[argi], "-b") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr, "Usage: c2openapi [--base "
                        "<openapi.json>] [--self <uri>] "
                        "[--dialect <uri>] "
                        "<src_dir> <out.json>\n");
        return CDD_C_ERROR_UNKNOWN;
      }
      base_file = argv[argi + 1];
      argi += 2;
      continue;
    }
    if (strcmp(argv[argi], "--self") == 0 || strcmp(argv[argi], "-s") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr, "Usage: c2openapi [--base "
                        "<openapi.json>] [--self <uri>] "
                        "[--dialect <uri>] "
                        "<src_dir> <out.json>\n");
        return CDD_C_ERROR_UNKNOWN;
      }
      self_uri = argv[argi + 1];
      argi += 2;
      continue;
    }
    if (strcmp(argv[argi], "--dialect") == 0 ||
        strcmp(argv[argi], "--jsonSchemaDialect") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr, "Usage: c2openapi [--base "
                        "<openapi.json>] [--self <uri>] "
                        "[--dialect <uri>] "
                        "<src_dir> <out.json>\n");
        return CDD_C_ERROR_UNKNOWN;
      }
      dialect_uri = argv[argi + 1];
      argi += 2;
      continue;
    }
    break;
  }

  if (argc - argi != 2) {
    fprintf(stderr, "Usage: c2openapi [--base <openapi.json>] "
                    "[--self <uri>] "
                    "[--dialect <uri>] "
                    "<src_dir> <out.json>\n");
    return CDD_C_ERROR_UNKNOWN;
  }

  src_dir = argv[argi];
  out_file = argv[argi + 1];
  rc = openapi_spec_init(&spec);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (base_file) {
    rc = load_base_spec(base_file, &spec);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Failed to load base OpenAPI spec %s: %d\n", base_file,
              rc);
      openapi_spec_free(&spec);
      return CDD_C_ERROR_UNKNOWN;
    }
  }

  if (self_uri && *self_uri) {
    C_CDD_FREE(spec.self_uri);
    spec.self_uri = NULL;
    rc = c_cdd_strdup(self_uri, &spec.self_uri);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Failed to set $self URI\n");
      openapi_spec_free(&spec);
      return CDD_C_ERROR_UNKNOWN;
    }
  }
  if (dialect_uri && *dialect_uri) {
    C_CDD_FREE(spec.json_schema_dialect);
    spec.json_schema_dialect = NULL;
    rc = c_cdd_strdup(dialect_uri, &spec.json_schema_dialect);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Failed to set jsonSchemaDialect\n");
      openapi_spec_free(&spec);
      return CDD_C_ERROR_UNKNOWN;
    }
  }

  /* 1. Walk & Process */
  rc = walk_directory(src_dir, walker_cb, &spec);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Error walking directory %s: %d\n", src_dir, rc);
    openapi_spec_free(&spec);
    return CDD_C_ERROR_UNKNOWN;
  }

  /* Derive top-level tags from operation tags */
  rc = collect_spec_tags(&spec);
  if (rc != CDD_C_SUCCESS) {
    openapi_spec_free(&spec);
    return rc;
  }

  /* 2. Write */
  rc = openapi_write_spec_to_json(&spec, &json);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Error serializing spec: %d\n", rc);
    openapi_spec_free(&spec);
    return CDD_C_ERROR_UNKNOWN;
  }

  rc = fs_write_to_file(out_file, json);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Failed to write %s\n", out_file);
    rc = CDD_C_ERROR_UNKNOWN;
  } else {
    printf("Written %s\n", out_file);
    rc = CDD_C_SUCCESS;
  }

  C_CDD_FREE(json);
  openapi_spec_free(&spec);

  return rc;
}

/**
 * @brief Executes the to docs json cli main operation.
 */
C_CDD_EXPORT cdd_c_error_t to_docs_json_cli_main(int argc, char **argv) {
  const char *input_file =
      getenv("CDD_INPUT") ? getenv("CDD_INPUT") : getenv("INPUT_FILE");
  int no_imports = getenv("CDD_NO_IMPORTS") ? 1 : 0;
  int no_wrapping = getenv("CDD_NO_WRAPPING") ? 1 : 0;
  int i;
  struct OpenAPI_Spec spec = {0};
  cdd_c_error_t rc;
  JSON_Value *parsed_root;
  JSON_Value *root_val;
  JSON_Object *root_obj;
  JSON_Value *endpoints_val;
  JSON_Object *endpoints_obj;
  size_t p, op_idx;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      return CDD_C_SUCCESS;
    } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
      if (i + 1 < argc)
        input_file = argv[++i];
    } else if (strcmp(argv[i], "--no-imports") == 0) {
      no_imports = 1;
    } else if (strcmp(argv[i], "--no-wrapping") == 0) {
      no_wrapping = 1;
    }
  }

  if (!input_file)
    return CDD_C_ERROR_UNKNOWN;

  parsed_root = json_parse_file(input_file);
  if (!parsed_root)
    return CDD_C_ERROR_UNKNOWN;

  rc = openapi_load_from_json(parsed_root, &spec);
  json_value_free(parsed_root);
  if (rc != CDD_C_SUCCESS)
    return rc;

  root_val = json_value_init_object();
  root_obj = json_value_get_object(root_val);
  endpoints_val = json_value_init_object();
  endpoints_obj = json_value_get_object(endpoints_val);

  for (p = 0; p < spec.n_paths; p++) {
    struct OpenAPI_Path *pi = &spec.paths[p];
    JSON_Value *path_val = json_value_init_object();
    JSON_Object *path_obj = json_value_get_object(path_val);
    json_object_set_value(endpoints_obj, pi->route, path_val);

    for (op_idx = 0; op_idx < pi->n_operations; op_idx++) {
      struct OpenAPI_Operation *op = &pi->operations[op_idx];
      const char *method = "";
      char snippet[4096];
      char final_code[8192];
      const char *op_id = op->operation_id ? op->operation_id : "unknown";

      switch (op->verb) {
      case OA_VERB_GET:
        method = "get";
        break;
      case OA_VERB_POST:
        method = "post";
        break;
      case OA_VERB_PUT:
        method = "put";
        break;
      case OA_VERB_DELETE:
        method = "delete";
        break;
      case OA_VERB_PATCH:
        method = "patch";
        break;
      case OA_VERB_HEAD:
        method = "head";
        break;
      case OA_VERB_OPTIONS:
        method = "options";
        break;
      case OA_VERB_TRACE:
        method = "trace";
        break;
      default:
        method = "custom";
        break;
      }

      snippet[0] = '\0';
      final_code[0] = '\0';

      if (!no_imports) {
        CDD_STRCAT(final_code, sizeof(final_code),
                   "#include \"generated_client.h\"\n#include "
                   "<stdio.h>\n\n");
      }
      if (!no_wrapping) {
        CDD_STRCAT(final_code, sizeof(final_code),
                   "int main(void) {\n  struct HttpClient client;\n  "
                   "struct ApiError *err = NULL;\n  api_init(&client, "
                   "\"https://api.example.com\");\n");
      }

      snprintf(snippet, sizeof(snippet),
               "  /* Call the %s API */\n  cdd_c_error_t rc = "
               "api_%s(&client, &err);\n  "
               "if (rc != CDD_C_SUCCESS) {\n    /* handle error */\n  }\n",
               op_id, op_id);
      CDD_STRCAT(final_code, sizeof(final_code), snippet);

      if (!no_wrapping) {
        CDD_STRCAT(final_code, sizeof(final_code),
                   "  api_cleanup(&client);\n  return "
                   "CDD_C_SUCCESS;\n}\n");
      }

      json_object_set_string(path_obj, method, final_code);
    }
  }

  json_object_set_value(root_obj, "endpoints", endpoints_val);

  {
    char *serialized = json_serialize_to_string_pretty(root_val);
    printf("%s\n", serialized);
    json_free_serialized_string(serialized);
  }

  json_value_free(root_val);
  openapi_spec_free(&spec);
  return CDD_C_SUCCESS;
}

/**
 * @brief CLI entry point for binding generation (e.g., `cdd-c
 * bind`).
 */
C_CDD_EXPORT cdd_c_error_t generate_bindings_cli_main(int argc, char **argv) {
  cdd_generate_bindings_config_t config = {0};
  int i;
  cdd_c_error_t rc;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      puts("Usage: cdd-c bind [OPTIONS]\n\n"
           "Options:\n"
           "  -i, --input <file|dir>    Input C header/source "
           "or directory\n"
           "  -o, --output-dir <dir>    Output directory for "
           "bindings\n"
           "  -l, --lang <langs>        Comma-separated list "
           "of languages or "
           "'*' "
           "(e.g., python,rust)");
      puts("  -n, --lib-name <name>     Name of the shared "
           "library (e.g., "
           "sqlite3)\n"
           "  -m, --module-name <name>  Name of the generated "
           "namespace/module\n"
           "  --skip-static             Skip static inline "
           "functions\n"
           "  --opaque-pointers         Treat unknown structs "
           "as void* "
           "(opaque)\n"
           "  --generate-tests          Generate basic "
           "sanity-check tests\n"
           "  -h, --help                Show this help "
           "message");
      return CDD_C_SUCCESS;
    } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
      if (i + 1 < argc)
        config.input = argv[++i];
    } else if (strcmp(argv[i], "-o") == 0 ||
               strcmp(argv[i], "--output-dir") == 0) {
      if (i + 1 < argc)
        config.output_dir = argv[++i];
    } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lang") == 0) {
      if (i + 1 < argc)
        config.target_langs = argv[++i];
    } else if (strcmp(argv[i], "-n") == 0 ||
               strcmp(argv[i], "--lib-name") == 0) {
      if (i + 1 < argc)
        config.library_name = argv[++i];
    } else if (strcmp(argv[i], "-m") == 0 ||
               strcmp(argv[i], "--module-name") == 0) {
      if (i + 1 < argc)
        config.module_name = argv[++i];
    } else if (strcmp(argv[i], "--skip-static") == 0) {
      config.skip_static = 1;
    } else if (strcmp(argv[i], "--opaque-pointers") == 0) {
      config.opaque_pointers = 1;
    } else if (strcmp(argv[i], "--generate-tests") == 0) {
      config.generate_tests = 1;
    }
  }

  if (!config.input || !config.output_dir || !config.target_langs) {
    fprintf(stderr, "Error: --input, --output-dir, and --lang "
                    "are required.\n");
    return CDD_C_ERROR_UNKNOWN;
  }

  rc = cdd_generate_bindings(&config);
  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Error: Binding generation failed with code %d\n", rc);
    return CDD_C_ERROR_UNKNOWN;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Registers parsed C types into an OpenAPI specification schemas list.
 */
C_CDD_EXPORT cdd_c_error_t c2openapi_register_types(
    struct OpenAPI_Spec *spec, const struct TypeDefList *types) {
  size_t i, j;
  if (!spec || !types)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < types->size; i++) {
    const struct TypeDefinition *def = &types->items[i];

    /* Skip duplicates */
    int is_duplicate = 0;
    for (j = 0; j < spec->n_defined_schemas; j++) {
      if (strcmp(spec->defined_schema_names[j], def->name) == 0) {
        is_duplicate = 1;
        break;
      }
    }
    if (is_duplicate)
      continue;

    if (def->kind == KIND_STRUCT && def->name && def->details.struct_fields) {
      size_t new_idx = spec->n_defined_schemas;
      spec->n_defined_schemas++;
      spec->defined_schema_names = C_CDD_REALLOC(
          spec->defined_schema_names, spec->n_defined_schemas * sizeof(char *));
      spec->defined_schemas =
          C_CDD_REALLOC(spec->defined_schemas,
                        spec->n_defined_schemas * sizeof(struct StructFields));
      spec->defined_schema_names[new_idx] = C_CDD_STRDUP(def->name);

      struct_fields_init(&spec->defined_schemas[new_idx]);
      for (j = 0; j < def->details.struct_fields->size; j++) {
        struct StructField *f = &def->details.struct_fields->fields[j];
        struct StructField *new_f;
        size_t k;
        struct_fields_add(&spec->defined_schemas[new_idx], f->name, f->type,
                          f->ref, f->default_val, f->bit_width);

        new_f = &spec->defined_schemas[new_idx]
                     .fields[spec->defined_schemas[new_idx].size - 1];
        if (f->n_type_union > 0) {
          new_f->n_type_union = f->n_type_union;
          new_f->type_union = C_CDD_CALLOC(f->n_type_union, sizeof(char *));
          for (k = 0; k < f->n_type_union; k++)
            new_f->type_union[k] = C_CDD_STRDUP(f->type_union[k]);
        }
        if (f->n_items_type_union > 0) {
          new_f->n_items_type_union = f->n_items_type_union;
          new_f->items_type_union =
              C_CDD_CALLOC(f->n_items_type_union, sizeof(char *));
          for (k = 0; k < f->n_items_type_union; k++)
            new_f->items_type_union[k] = C_CDD_STRDUP(f->items_type_union[k]);
        }
      }
    } else if (def->kind == KIND_ENUM && def->name &&
               def->details.enum_members) {
      size_t new_idx = spec->n_defined_schemas;
      spec->n_defined_schemas++;
      spec->defined_schema_names = C_CDD_REALLOC(
          spec->defined_schema_names, spec->n_defined_schemas * sizeof(char *));
      spec->defined_schemas =
          C_CDD_REALLOC(spec->defined_schemas,
                        spec->n_defined_schemas * sizeof(struct StructFields));
      spec->defined_schema_names[new_idx] = C_CDD_STRDUP(def->name);

      struct_fields_init(&spec->defined_schemas[new_idx]);
      spec->defined_schemas[new_idx].is_enum = 1;
      enum_members_init(&spec->defined_schemas[new_idx].enum_members);
      for (j = 0; j < def->details.enum_members->size; j++) {
        enum_members_add(&spec->defined_schemas[new_idx].enum_members,
                         def->details.enum_members->members[j]);
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets custom JSON memory allocation functions in the library.
 *
 * @param[in] malloc_fun Custom memory allocation function.
 * @param[in] free_fun Custom memory deallocation function.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t c2openapi_set_json_allocators(
    void *(*malloc_fun)(size_t), void (*free_fun)(void *)) {
  json_set_allocation_functions(malloc_fun, free_fun);
  return CDD_C_SUCCESS;
}
