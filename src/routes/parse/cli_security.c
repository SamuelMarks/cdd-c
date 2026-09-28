/**
 * @file cli_security.c
 * @brief Security scheme and OAuth flow routines for C to OpenAPI CLI routes
 * parser.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/str.h"
#include "openapi/parse/openapi.h"
#include "routes/parse/cli_internal.h"
#include "routes/parse/cli.h"
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

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
cdd_c_error_t
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
