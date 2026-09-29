/* clang-format off */
#include "c_cdd/memory.h"
/**
 * @file doc_security.c
 * @brief Security and securityScheme documentation directive parsing.
 *
 * @author Samuel Marks
 */

#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd_export.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "docstrings/parse/doc.h"
#include "docstrings/parse/doc_internal.h"
#include "functions/parse/str.h"
/* clang-format on */

cdd_c_error_t doc_parse_security_type_text(const char *text,
                                           enum DocSecurityType *out_val) {
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_doc_fail_sec_text;
    if (g_doc_fail_sec_text == 1)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!text) {
    *out_val = DOC_SEC_UNSET;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "apiKey") == 0) {
    *out_val = DOC_SEC_APIKEY;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "http") == 0) {
    *out_val = DOC_SEC_HTTP;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "mutualTLS") == 0) {
    *out_val = DOC_SEC_MUTUALTLS;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "oauth2") == 0) {
    *out_val = DOC_SEC_OAUTH2;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "openIdConnect") == 0) {
    *out_val = DOC_SEC_OPENID;
    return CDD_C_SUCCESS;
  }
  *out_val = DOC_SEC_UNSET;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_security_in_text(const char *text,
                                         enum DocSecurityIn *out_val) {
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_doc_fail_sec_text;
    if (g_doc_fail_sec_text == 2)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!text) {
    *out_val = DOC_SEC_IN_UNSET;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "query") == 0) {
    *out_val = DOC_SEC_IN_QUERY;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "header") == 0) {
    *out_val = DOC_SEC_IN_HEADER;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "cookie") == 0) {
    *out_val = DOC_SEC_IN_COOKIE;
    return CDD_C_SUCCESS;
  }
  *out_val = DOC_SEC_IN_UNSET;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_oauth_flow_type_text(const char *text,
                                             enum DocOAuthFlowType *out_val) {
  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_doc_fail_sec_text;
    if (g_doc_fail_sec_text == 3)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!text) {
    *out_val = DOC_OAUTH_FLOW_UNSET;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "implicit") == 0) {
    *out_val = DOC_OAUTH_FLOW_IMPLICIT;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "password") == 0) {
    *out_val = DOC_OAUTH_FLOW_PASSWORD;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "clientCredentials") == 0) {
    *out_val = DOC_OAUTH_FLOW_CLIENT_CREDENTIALS;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "authorizationCode") == 0) {
    *out_val = DOC_OAUTH_FLOW_AUTHORIZATION_CODE;
    return CDD_C_SUCCESS;
  }
  if (strcmp(text, "deviceAuthorization") == 0) {
    *out_val = DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION;
    return CDD_C_SUCCESS;
  }
  *out_val = DOC_OAUTH_FLOW_UNSET;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_oauth_scopes(const char *input,
                                     struct DocOAuthScope **out,
                                     size_t *out_count) {
  char **names = NULL;
  size_t n = 0;
  size_t i;
  struct DocOAuthScope *scopes;
  cdd_c_error_t rc;

  if (!out || !out_count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out = NULL;
  *out_count = 0;

  rc = doc_split_scopes(input, &names, &n);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (n == 0)
    return CDD_C_SUCCESS;

  scopes =
      (struct DocOAuthScope *)C_CDD_CALLOC(n, sizeof(struct DocOAuthScope));
  if (!scopes) {
    for (i = 0; i < n; ++i)
      C_CDD_FREE(names[i]);
    C_CDD_FREE(names);
    return CDD_C_ERROR_MEMORY;
  }

  for (i = 0; i < n; ++i) {
    scopes[i].name = names[i];
    scopes[i].description = NULL;
    names[i] = NULL;
  }
  C_CDD_FREE(names);
  *out = scopes;
  *out_count = n;
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_security_line(const char *line, const char *end,
                                      struct DocMetadata *out) {
  const char *cur = line;
  char *scheme = NULL;
  char *rest = NULL;
  char **scopes = NULL;
  size_t n_scopes = 0;
  struct DocSecurityRequirement *new_reqs;
  struct DocSecurityRequirement *req;
  cdd_c_error_t rc;

  rc = doc_extract_word(cur, end, &cur, &scheme);
  if (rc != CDD_C_SUCCESS || !scheme)
    return rc;

  rc = doc_extract_rest(cur, end, &rest);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(scheme);
    return rc;
  }

  rc = doc_split_scopes(rest, &scopes, &n_scopes);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(scheme);
    C_CDD_FREE(rest);
    return rc;
  }

  new_reqs = (struct DocSecurityRequirement *)C_CDD_REALLOC(
      out->security,
      (out->n_security + 1) * sizeof(struct DocSecurityRequirement));
  if (!new_reqs) {
    size_t i;
    for (i = 0; i < n_scopes; ++i)
      C_CDD_FREE(scopes[i]);
    if (scopes)
      C_CDD_FREE(scopes);
    C_CDD_FREE(scheme);
    if (rest)
      C_CDD_FREE(rest);
    return CDD_C_ERROR_MEMORY;
  }
  out->security = new_reqs;
  req = &out->security[out->n_security];
  memset(req, 0, sizeof(*req));
  req->scheme = scheme;
  req->scopes = scopes;
  req->n_scopes = n_scopes;
  out->n_security++;

  if (rest)
    C_CDD_FREE(rest);
  return CDD_C_SUCCESS;
}

cdd_c_error_t doc_parse_security_scheme_line(const char *line, const char *end,
                                             struct DocMetadata *out) {
  cdd_c_error_t rc_opt;
  struct DocSecurityScheme *new_schemes;
  struct DocSecurityScheme *scheme;
  const char *cur = line;
  struct DocOAuthFlow *current_flow = NULL;
  cdd_c_error_t rc;

  new_schemes = (struct DocSecurityScheme *)C_CDD_REALLOC(
      out->security_schemes,
      (out->n_security_schemes + 1) * sizeof(struct DocSecurityScheme));
  if (!new_schemes) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  out->security_schemes = new_schemes;
  scheme = &out->security_schemes[out->n_security_schemes];
  memset(scheme, 0, sizeof(*scheme));
  scheme->type = DOC_SEC_UNSET;
  scheme->in = DOC_SEC_IN_UNSET;

  rc = doc_extract_word(cur, end, &cur, &scheme->name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!scheme->name)
    return CDD_C_SUCCESS;

  rc = doc_skip_ws(cur, &cur);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (cur < end && *cur == '[') {
    const char *close_bracket = cur;
    while (close_bracket < end && *close_bracket != ']')
      close_bracket++;

    if (close_bracket < end) {
      const char *inner_start = cur + 1;
      size_t inner_len = (size_t)(close_bracket - inner_start);
      char *attr = (char *)(size_t)C_CDD_MALLOC(inner_len + 1);
      if (attr) {
        memcpy(attr, inner_start, inner_len);
        attr[inner_len] = '\0';

        if (strncmp(attr, "type:", 5) == 0 || strncmp(attr, "type=", 5) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 5, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          rc = doc_parse_security_type_text(val, &scheme->type);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
        } else if (strncmp(attr, "description:", 12) == 0 ||
                   strncmp(attr, "description=", 12) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 12, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (*val) {
            if (scheme->description)
              C_CDD_FREE(scheme->description);
            rc = c_cdd_strdup(val, &scheme->description);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "scheme:", 7) == 0 ||
                   strncmp(attr, "scheme=", 7) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 7, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (*val) {
            if (scheme->scheme)
              C_CDD_FREE(scheme->scheme);
            rc = c_cdd_strdup(val, &scheme->scheme);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "bearerFormat:", 13) == 0 ||
                   strncmp(attr, "bearerFormat=", 13) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 13, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (*val) {
            if (scheme->bearer_format)
              C_CDD_FREE(scheme->bearer_format);
            rc = c_cdd_strdup(val, &scheme->bearer_format);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "paramName:", 10) == 0 ||
                   strncmp(attr, "paramName=", 10) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 10, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (*val) {
            if (scheme->param_name)
              C_CDD_FREE(scheme->param_name);
            rc = c_cdd_strdup(val, &scheme->param_name);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "in:", 3) == 0 ||
                   strncmp(attr, "in=", 3) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 3, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          rc = doc_parse_security_in_text(val, &scheme->in);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
        } else if (strncmp(attr, "openIdConnectUrl:", 17) == 0 ||
                   strncmp(attr, "openIdConnectUrl=", 17) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 17, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (*val) {
            if (scheme->open_id_connect_url)
              C_CDD_FREE(scheme->open_id_connect_url);
            rc = c_cdd_strdup(val, &scheme->open_id_connect_url);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "oauth2MetadataUrl:", 18) == 0 ||
                   strncmp(attr, "oauth2MetadataUrl=", 18) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 18, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (*val) {
            if (scheme->oauth2_metadata_url)
              C_CDD_FREE(scheme->oauth2_metadata_url);
            rc = c_cdd_strdup(val, &scheme->oauth2_metadata_url);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "flow:", 5) == 0 ||
                   strncmp(attr, "flow=", 5) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 5, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          {
            enum DocOAuthFlowType flow_type = DOC_OAUTH_FLOW_UNSET;
            rc = doc_parse_oauth_flow_type_text(val, &flow_type);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
            if (flow_type != DOC_OAUTH_FLOW_UNSET) {
              struct DocOAuthFlow *new_flows =
                  (struct DocOAuthFlow *)C_CDD_REALLOC(
                      scheme->flows,
                      (scheme->n_flows + 1) * sizeof(struct DocOAuthFlow));
              if (new_flows) {
                scheme->flows = new_flows;
                current_flow = &scheme->flows[scheme->n_flows];
                memset(current_flow, 0, sizeof(*current_flow));
                current_flow->type = flow_type;
                scheme->n_flows++;
                if (scheme->type == DOC_SEC_UNSET)
                  scheme->type = DOC_SEC_OAUTH2;
              }
            }
          }
        } else if (strncmp(attr, "authorizationUrl:", 17) == 0 ||
                   strncmp(attr, "authorizationUrl=", 17) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 17, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (current_flow && *val) {
            if (current_flow->authorization_url)
              C_CDD_FREE(current_flow->authorization_url);
            rc = c_cdd_strdup(val, &current_flow->authorization_url);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "tokenUrl:", 9) == 0 ||
                   strncmp(attr, "tokenUrl=", 9) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 9, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (current_flow && *val) {
            if (current_flow->token_url)
              C_CDD_FREE(current_flow->token_url);
            rc = c_cdd_strdup(val, &current_flow->token_url);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "refreshUrl:", 11) == 0 ||
                   strncmp(attr, "refreshUrl=", 11) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 11, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (current_flow && *val) {
            if (current_flow->refresh_url)
              C_CDD_FREE(current_flow->refresh_url);
            rc = c_cdd_strdup(val, &current_flow->refresh_url);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "deviceAuthorizationUrl:", 23) == 0 ||
                   strncmp(attr, "deviceAuthorizationUrl=", 23) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 23, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (current_flow && *val) {
            if (current_flow->device_authorization_url)
              C_CDD_FREE(current_flow->device_authorization_url);
            rc = c_cdd_strdup(val, &current_flow->device_authorization_url);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(attr);
              return rc;
            }
          }
        } else if (strncmp(attr, "scopes:", 7) == 0 ||
                   strncmp(attr, "scopes=", 7) == 0) {
          char *val = NULL;
          rc = doc_trim_segment(attr + 7, &val);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(attr);
            return rc;
          }
          if (current_flow) {
            struct DocOAuthScope *scopes = NULL;
            size_t n_scopes = 0;
            if (doc_parse_oauth_scopes(val, &scopes, &n_scopes) ==
                CDD_C_SUCCESS) {
              size_t i;
              for (i = 0; i < current_flow->n_scopes; ++i) {
                C_CDD_FREE(current_flow->scopes[i].name);
                C_CDD_FREE(current_flow->scopes[i].description);
              }
              if (current_flow->scopes)
                C_CDD_FREE(current_flow->scopes);
              current_flow->scopes = scopes;
              current_flow->n_scopes = n_scopes;
            }
          }
        } else {
          rc_opt = doc_parse_optional_bool_attr(
              attr, "deprecated", &scheme->deprecated_set, &scheme->deprecated);
          if (rc_opt == CDD_C_ERROR_MEMORY) {
            C_CDD_FREE(attr);
            return rc_opt;
          }
        }
        C_CDD_FREE(attr);
      }
      cur = close_bracket + 1;
      rc = doc_skip_ws(cur, &cur);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      break;
    }
  }

  out->n_security_schemes++;
  return CDD_C_SUCCESS;
}
