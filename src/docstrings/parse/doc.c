/* clang-format off */
#include "c_cdd/memory.h"
/**
 * @file doc.c
 * @brief Implementation of the documentation comment parser.
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

cdd_c_error_t doc_metadata_init(struct DocMetadata *meta) {
  if (!meta)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  memset(meta, 0, sizeof(*meta));
  return CDD_C_SUCCESS;
}

void doc_metadata_free(struct DocMetadata *meta) {
  size_t i;
  if (!meta)
    return;

  if (meta->route)
    C_CDD_FREE(meta->route);
  if (meta->verb)
    C_CDD_FREE(meta->verb);
  if (meta->operation_id)
    C_CDD_FREE(meta->operation_id);
  if (meta->json_schema_dialect)
    C_CDD_FREE(meta->json_schema_dialect);
  if (meta->summary)
    C_CDD_FREE(meta->summary);
  if (meta->description)
    C_CDD_FREE(meta->description);
  if (meta->info_title)
    C_CDD_FREE(meta->info_title);
  if (meta->info_version)
    C_CDD_FREE(meta->info_version);
  if (meta->info_summary)
    C_CDD_FREE(meta->info_summary);
  if (meta->info_description)
    C_CDD_FREE(meta->info_description);
  if (meta->terms_of_service)
    C_CDD_FREE(meta->terms_of_service);
  if (meta->contact_name)
    C_CDD_FREE(meta->contact_name);
  if (meta->contact_url)
    C_CDD_FREE(meta->contact_url);
  if (meta->contact_email)
    C_CDD_FREE(meta->contact_email);
  if (meta->license_name)
    C_CDD_FREE(meta->license_name);
  if (meta->license_identifier)
    C_CDD_FREE(meta->license_identifier);
  if (meta->license_url)
    C_CDD_FREE(meta->license_url);
  if (meta->external_docs_url)
    C_CDD_FREE(meta->external_docs_url);
  if (meta->external_docs_description)
    C_CDD_FREE(meta->external_docs_description);
  if (meta->tags) {
    for (i = 0; i < meta->n_tags; ++i) {
      C_CDD_FREE(meta->tags[i]);
    }
    C_CDD_FREE(meta->tags);
  }

  if (meta->params) {
    for (i = 0; i < meta->n_params; ++i) {
      C_CDD_FREE(meta->params[i].name);
      C_CDD_FREE(meta->params[i].in_loc);
      C_CDD_FREE(meta->params[i].description);
      if (meta->params[i].format)
        C_CDD_FREE(meta->params[i].format);
      if (meta->params[i].content_type)
        C_CDD_FREE(meta->params[i].content_type);
      if (meta->params[i].example)
        C_CDD_FREE(meta->params[i].example);
    }
    C_CDD_FREE(meta->params);
  }

  if (meta->returns) {
    for (i = 0; i < meta->n_returns; ++i) {
      C_CDD_FREE(meta->returns[i].code);
      if (meta->returns[i].summary)
        C_CDD_FREE(meta->returns[i].summary);
      C_CDD_FREE(meta->returns[i].description);
      if (meta->returns[i].content_type)
        C_CDD_FREE(meta->returns[i].content_type);
      if (meta->returns[i].example)
        C_CDD_FREE(meta->returns[i].example);
    }
    C_CDD_FREE(meta->returns);
  }

  if (meta->response_headers) {
    for (i = 0; i < meta->n_response_headers; ++i) {
      C_CDD_FREE(meta->response_headers[i].code);
      C_CDD_FREE(meta->response_headers[i].name);
      C_CDD_FREE(meta->response_headers[i].type);
      if (meta->response_headers[i].format)
        C_CDD_FREE(meta->response_headers[i].format);
      if (meta->response_headers[i].content_type)
        C_CDD_FREE(meta->response_headers[i].content_type);
      C_CDD_FREE(meta->response_headers[i].description);
      if (meta->response_headers[i].example)
        C_CDD_FREE(meta->response_headers[i].example);
    }
    C_CDD_FREE(meta->response_headers);
  }

  if (meta->links) {
    for (i = 0; i < meta->n_links; ++i) {
      struct DocLink *link = &meta->links[i];
      C_CDD_FREE(link->code);
      C_CDD_FREE(link->name);
      if (link->operation_id)
        C_CDD_FREE(link->operation_id);
      if (link->operation_ref)
        C_CDD_FREE(link->operation_ref);
      if (link->summary)
        C_CDD_FREE(link->summary);
      if (link->description)
        C_CDD_FREE(link->description);
      if (link->parameters_json)
        C_CDD_FREE(link->parameters_json);
      if (link->request_body_json)
        C_CDD_FREE(link->request_body_json);
      if (link->server_url)
        C_CDD_FREE(link->server_url);
      if (link->server_name)
        C_CDD_FREE(link->server_name);
      if (link->server_description)
        C_CDD_FREE(link->server_description);
    }
    C_CDD_FREE(meta->links);
  }

  if (meta->security) {
    for (i = 0; i < meta->n_security; ++i) {
      size_t s;
      C_CDD_FREE(meta->security[i].scheme);
      if (meta->security[i].scopes) {
        for (s = 0; s < meta->security[i].n_scopes; ++s)
          C_CDD_FREE(meta->security[i].scopes[s]);
        C_CDD_FREE(meta->security[i].scopes);
      }
    }
    C_CDD_FREE(meta->security);
  }

  if (meta->security_schemes) {
    for (i = 0; i < meta->n_security_schemes; ++i) {
      size_t f;
      struct DocSecurityScheme *sch = &meta->security_schemes[i];
      C_CDD_FREE(sch->name);
      if (sch->description)
        C_CDD_FREE(sch->description);
      if (sch->scheme)
        C_CDD_FREE(sch->scheme);
      if (sch->bearer_format)
        C_CDD_FREE(sch->bearer_format);
      if (sch->param_name)
        C_CDD_FREE(sch->param_name);
      if (sch->open_id_connect_url)
        C_CDD_FREE(sch->open_id_connect_url);
      if (sch->oauth2_metadata_url)
        C_CDD_FREE(sch->oauth2_metadata_url);
      if (sch->flows) {
        for (f = 0; f < sch->n_flows; ++f) {
          size_t s;
          struct DocOAuthFlow *flow = &sch->flows[f];
          if (flow->authorization_url)
            C_CDD_FREE(flow->authorization_url);
          if (flow->token_url)
            C_CDD_FREE(flow->token_url);
          if (flow->refresh_url)
            C_CDD_FREE(flow->refresh_url);
          if (flow->device_authorization_url)
            C_CDD_FREE(flow->device_authorization_url);
          if (flow->scopes) {
            for (s = 0; s < flow->n_scopes; ++s) {
              C_CDD_FREE(flow->scopes[s].name);
              if (flow->scopes[s].description)
                C_CDD_FREE(flow->scopes[s].description);
            }
            C_CDD_FREE(flow->scopes);
          }
        }
        C_CDD_FREE(sch->flows);
      }
    }
    C_CDD_FREE(meta->security_schemes);
  }

  if (meta->servers) {
    for (i = 0; i < meta->n_servers; ++i) {
      size_t v;
      C_CDD_FREE(meta->servers[i].url);
      C_CDD_FREE(meta->servers[i].name);
      C_CDD_FREE(meta->servers[i].description);
      if (meta->servers[i].variables) {
        for (v = 0; v < meta->servers[i].n_variables; ++v) {
          size_t e;
          struct DocServerVar *var = &meta->servers[i].variables[v];
          C_CDD_FREE(var->name);
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
        C_CDD_FREE(meta->servers[i].variables);
      }
    }
    C_CDD_FREE(meta->servers);
  }

  if (meta->request_bodies) {
    for (i = 0; i < meta->n_request_bodies; ++i) {
      C_CDD_FREE(meta->request_bodies[i].content_type);
      C_CDD_FREE(meta->request_bodies[i].description);
      if (meta->request_bodies[i].example)
        C_CDD_FREE(meta->request_bodies[i].example);
    }
    C_CDD_FREE(meta->request_bodies);
  }

  if (meta->encodings) {
    for (i = 0; i < meta->n_encodings; ++i) {
      if (meta->encodings[i].name)
        C_CDD_FREE(meta->encodings[i].name);
      if (meta->encodings[i].content_type)
        C_CDD_FREE(meta->encodings[i].content_type);
    }
    C_CDD_FREE(meta->encodings);
  }

  if (meta->tag_meta) {
    for (i = 0; i < meta->n_tag_meta; ++i) {
      C_CDD_FREE(meta->tag_meta[i].name);
      C_CDD_FREE(meta->tag_meta[i].summary);
      C_CDD_FREE(meta->tag_meta[i].description);
      C_CDD_FREE(meta->tag_meta[i].parent);
      C_CDD_FREE(meta->tag_meta[i].kind);
      C_CDD_FREE(meta->tag_meta[i].external_docs_url);
      C_CDD_FREE(meta->tag_meta[i].external_docs_description);
    }
    C_CDD_FREE(meta->tag_meta);
  }

  if (meta->request_body_description)
    C_CDD_FREE(meta->request_body_description);
  if (meta->request_body_content_type)
    C_CDD_FREE(meta->request_body_content_type);

  memset(meta, 0, sizeof(*meta));
}

cdd_c_error_t doc_parse_block(const char *comment, struct DocMetadata *out) {
  const char *p = comment;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!comment || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Skip initial marker if present (e.g. block or line comments) via simple
   * scan loop
   */
  while (*p) {
    const char *line_end;
    const char *scan;

    /* Find end of current line */
    line_end = p;
    while (*line_end && !DOC_IS_EOL(*line_end)) {
      line_end++;
    }

    /* Analyze line content */
    scan = p;

    /* Skip leading whitespace */
    while (scan < line_end && isspace((unsigned char)*scan))
      scan++;

    /* Skip Comment Decorators */
    if (scan < line_end && *scan == '/') {
      scan++;
      if (scan < line_end && *scan == '*') {
        /* Block comment start */
        scan++;
        while (scan < line_end && *scan == '*')
          scan++;
      } else if (scan < line_end && *scan == '/') {
        /* Line comment start */
        scan++;
        while (scan < line_end && *scan == '/')
          scan++;
      }
    } else if (scan < line_end && *scan == '*') {
      /* Continuation line start */
      scan++;
      /* Also skip closing slash if present */
      if (scan < line_end && *scan == '/')
        scan++;
    }

    /* Skip whitespace again after decorators */
    while (scan < line_end && isspace((unsigned char)*scan))
      scan++;

    /* Check for Directive (@ or \) */
    if (scan < line_end && (*scan == '@' || *scan == '\\')) {
      const char *cmd_start = scan + 1;
      const char *cmd_end = cmd_start;
      char *cmd = NULL;

      while (cmd_end < line_end && isalpha((unsigned char)*cmd_end)) {
        cmd_end++;
      }

      {
        size_t cmd_len = (size_t)(cmd_end - cmd_start);
        cmd = (char *)(size_t)C_CDD_MALLOC(cmd_len + 1);
        if (!cmd) {
          rc = CDD_C_ERROR_MEMORY;
          goto cleanup;
        }
        memcpy(cmd, cmd_start, cmd_len);
        cmd[cmd_len] = '\0';

        /* Dispatch */
        if (strcmp(cmd, "route") == 0) {
          rc = doc_parse_route_line(cmd_end, line_end, out);
          out->is_webhook = 0;
        } else if (strcmp(cmd, "webhook") == 0) {
          rc = doc_parse_route_line(cmd_end, line_end, out);
          out->is_webhook = 1;
        } else if (strcmp(cmd, "param") == 0) {
          rc = doc_parse_param_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "return") == 0 || strcmp(cmd, "returns") == 0) {
          rc = doc_parse_return_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "responseHeader") == 0 ||
                   strcmp(cmd, "responseheader") == 0) {
          rc = doc_parse_response_header_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "link") == 0) {
          rc = doc_parse_link_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "summary") == 0 || strcmp(cmd, "brief") == 0) {
          if (out->summary)
            C_CDD_FREE(out->summary);
          rc = doc_extract_rest(cmd_end, line_end, &out->summary);
        } else if (strcmp(cmd, "operationId") == 0 ||
                   strcmp(cmd, "operationid") == 0) {
          if (out->operation_id)
            C_CDD_FREE(out->operation_id);
          rc = doc_extract_rest(cmd_end, line_end, &out->operation_id);
        } else if (strcmp(cmd, "description") == 0 ||
                   strcmp(cmd, "details") == 0) {
          if (out->description)
            C_CDD_FREE(out->description);
          rc = doc_extract_rest(cmd_end, line_end, &out->description);
        } else if (strcmp(cmd, "tag") == 0 || strcmp(cmd, "tags") == 0) {
          rc = doc_parse_tags_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "tagMeta") == 0 || strcmp(cmd, "tagmeta") == 0) {
          rc = doc_parse_tag_meta_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "deprecated") == 0) {
          rc = doc_parse_deprecated_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "externalDocs") == 0 ||
                   strcmp(cmd, "externaldocs") == 0) {
          rc = doc_parse_external_docs_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "security") == 0) {
          rc = doc_parse_security_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "securityScheme") == 0 ||
                   strcmp(cmd, "securityscheme") == 0) {
          rc = doc_parse_security_scheme_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "server") == 0) {
          rc = doc_parse_server_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "serverVar") == 0 ||
                   strcmp(cmd, "servervar") == 0) {
          rc = doc_parse_server_var_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "requestBody") == 0 ||
                   strcmp(cmd, "requestbody") == 0) {
          rc = doc_parse_request_body_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "encoding") == 0) {
          rc = doc_parse_encoding_line(cmd_end, line_end, out, 0);
        } else if (strcmp(cmd, "prefixEncoding") == 0 ||
                   strcmp(cmd, "prefixencoding") == 0) {
          rc = doc_parse_encoding_line(cmd_end, line_end, out, 1);
        } else if (strcmp(cmd, "itemEncoding") == 0 ||
                   strcmp(cmd, "itemencoding") == 0) {
          rc = doc_parse_encoding_line(cmd_end, line_end, out, 2);
        } else if (strcmp(cmd, "jsonSchemaDialect") == 0 ||
                   strcmp(cmd, "jsonschemadialect") == 0) {
          if (out->json_schema_dialect)
            C_CDD_FREE(out->json_schema_dialect);
          rc = doc_extract_rest(cmd_end, line_end, &out->json_schema_dialect);
        } else if (strcmp(cmd, "infoTitle") == 0 ||
                   strcmp(cmd, "infotitle") == 0) {
          if (out->info_title)
            C_CDD_FREE(out->info_title);
          rc = doc_extract_rest(cmd_end, line_end, &out->info_title);
        } else if (strcmp(cmd, "infoVersion") == 0 ||
                   strcmp(cmd, "infoversion") == 0) {
          if (out->info_version)
            C_CDD_FREE(out->info_version);
          rc = doc_extract_rest(cmd_end, line_end, &out->info_version);
        } else if (strcmp(cmd, "infoSummary") == 0 ||
                   strcmp(cmd, "infosummary") == 0) {
          if (out->info_summary)
            C_CDD_FREE(out->info_summary);
          rc = doc_extract_rest(cmd_end, line_end, &out->info_summary);
        } else if (strcmp(cmd, "infoDescription") == 0 ||
                   strcmp(cmd, "infodescription") == 0) {
          if (out->info_description)
            C_CDD_FREE(out->info_description);
          rc = doc_extract_rest(cmd_end, line_end, &out->info_description);
        } else if (strcmp(cmd, "termsOfService") == 0 ||
                   strcmp(cmd, "termsofservice") == 0) {
          if (out->terms_of_service)
            C_CDD_FREE(out->terms_of_service);
          rc = doc_extract_rest(cmd_end, line_end, &out->terms_of_service);
        } else if (strcmp(cmd, "contact") == 0) {
          rc = doc_parse_contact_line(cmd_end, line_end, out);
        } else if (strcmp(cmd, "license") == 0) {
          rc = doc_parse_license_line(cmd_end, line_end, out);
        }

        C_CDD_FREE(cmd);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    } else {
      /* Continuation / Description lines not handled in this basic pass */
    }

    /* Advance to next line */
    p = line_end;
    while (*p && DOC_IS_EOL(*p))
      p++;
  }

cleanup:
  return rc;
}

#ifdef CDD_BUILD_TESTS
cdd_c_error_t trim_segment_test(char *s, char **out) {
  return doc_trim_segment(s, out);
}

cdd_c_error_t parse_bool_text_test(const char *s, int *out) {
  return doc_parse_bool_text(s, out);
}

cdd_c_error_t parse_tag_meta_line_test(const char *line, const char *end,
                                       struct DocMetadata *out) {
  return doc_parse_tag_meta_line(line, end, out);
}

cdd_c_error_t parse_style_text_test(const char *s, enum DocParamStyle *out) {
  return doc_parse_style_text(s, out);
}

cdd_c_error_t parse_optional_example_attr_test(const char *attr, char **out) {
  return doc_parse_optional_example_attr(attr, out);
}

cdd_c_error_t parse_optional_bool_attr_test(const char *attr, const char *key,
                                            int *out_set, int *out_val) {
  return doc_parse_optional_bool_attr(attr, key, out_set, out_val);
}

cdd_c_error_t split_scopes_test(const char *s, char ***out_scopes,
                                size_t *out_count) {
  return doc_split_scopes(s, out_scopes, out_count);
}

cdd_c_error_t parse_security_type_text_test(const char *s,
                                            enum DocSecurityType *out) {
  return doc_parse_security_type_text(s, out);
}

cdd_c_error_t parse_security_in_text_test(const char *s,
                                          enum DocSecurityIn *out) {
  return doc_parse_security_in_text(s, out);
}

cdd_c_error_t parse_oauth_flow_type_text_test(const char *s,
                                              enum DocOAuthFlowType *out) {
  return doc_parse_oauth_flow_type_text(s, out);
}

cdd_c_error_t parse_oauth_scopes_test(const char *s, struct DocOAuthScope **out,
                                      size_t *out_count) {
  return doc_parse_oauth_scopes(s, out, out_count);
}

cdd_c_error_t split_enum_values_test(const char *s, char ***out_vals,
                                     size_t *out_count) {
  return doc_split_enum_values(s, out_vals, out_count);
}

cdd_c_error_t find_key_token_test(char *s, const char *key, size_t *key_len,
                                  char **out) {
  return doc_find_key_token(s, key, key_len, out);
}
#endif
