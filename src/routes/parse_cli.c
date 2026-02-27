/**
 * @file c2openapi_cli.c
 * @brief Implementation of the C-to-OpenAPI CLI orchestrator.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit_schema.h" /* For Type Registry */
#include "classes/parse_inspector.h"
#include "docstrings/parse_doc.h"
#include "functions/parse_cst.h"
#include "functions/parse_fs.h"
#include "functions/parse_str.h"
#include "functions/parse_tokenizer.h"
#include "routes/emit_aggregator.h"
#include "routes/emit_openapi.h"
#include "routes/emit_operation.h" /* For OpBuilder and C2OpenAPI_ParsedSig */
#include "routes/parse_cli.h"
#include "routes/parse_openapi.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

/* --- Helpers --- */

static int is_source_file(const char *path) {
  const char *ext = strrchr(path, '.');
  if (!ext)
    return 0;
  return (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0);
}

static int spec_has_tag(const struct OpenAPI_Spec *spec, const char *name) {
  size_t i;
  if (!spec || !name)
    return 0;
  for (i = 0; i < spec->n_tags; ++i) {
    if (spec->tags[i].name && strcmp(spec->tags[i].name, name) == 0)
      return 1;
  }
  return 0;
}

static int spec_add_tag(struct OpenAPI_Spec *spec, const char *name) {
  struct OpenAPI_Tag *new_tags;
  struct OpenAPI_Tag *tag;

  if (!spec || !name || !*name)
    return 0;
  if (spec_has_tag(spec, name))
    return 0;

  new_tags = (struct OpenAPI_Tag *)realloc(
      spec->tags, (spec->n_tags + 1) * sizeof(struct OpenAPI_Tag));
  if (!new_tags)
    return ENOMEM;
  spec->tags = new_tags;
  tag = &spec->tags[spec->n_tags++];
  memset(tag, 0, sizeof(*tag));
  tag->name = c_cdd_strdup(name);
  if (!tag->name)
    return ENOMEM;
  return 0;
}

static struct OpenAPI_Tag *spec_find_tag(struct OpenAPI_Spec *spec,
                                         const char *name) {
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_tags; ++i) {
    if (spec->tags[i].name && strcmp(spec->tags[i].name, name) == 0)
      return &spec->tags[i];
  }
  return NULL;
}

static enum OpenAPI_SecurityType
map_doc_security_type(enum DocSecurityType type) {
  switch (type) {
  case DOC_SEC_APIKEY:
    return OA_SEC_APIKEY;
  case DOC_SEC_HTTP:
    return OA_SEC_HTTP;
  case DOC_SEC_MUTUALTLS:
    return OA_SEC_MUTUALTLS;
  case DOC_SEC_OAUTH2:
    return OA_SEC_OAUTH2;
  case DOC_SEC_OPENID:
    return OA_SEC_OPENID;
  case DOC_SEC_UNSET:
  default:
    return OA_SEC_UNKNOWN;
  }
}

static enum OpenAPI_SecurityIn map_doc_security_in(enum DocSecurityIn in) {
  switch (in) {
  case DOC_SEC_IN_QUERY:
    return OA_SEC_IN_QUERY;
  case DOC_SEC_IN_HEADER:
    return OA_SEC_IN_HEADER;
  case DOC_SEC_IN_COOKIE:
    return OA_SEC_IN_COOKIE;
  case DOC_SEC_IN_UNSET:
  default:
    return OA_SEC_IN_UNKNOWN;
  }
}

static enum OpenAPI_OAuthFlowType
map_doc_flow_type(enum DocOAuthFlowType type) {
  switch (type) {
  case DOC_OAUTH_FLOW_IMPLICIT:
    return OA_OAUTH_FLOW_IMPLICIT;
  case DOC_OAUTH_FLOW_PASSWORD:
    return OA_OAUTH_FLOW_PASSWORD;
  case DOC_OAUTH_FLOW_CLIENT_CREDENTIALS:
    return OA_OAUTH_FLOW_CLIENT_CREDENTIALS;
  case DOC_OAUTH_FLOW_AUTHORIZATION_CODE:
    return OA_OAUTH_FLOW_AUTHORIZATION_CODE;
  case DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION:
    return OA_OAUTH_FLOW_DEVICE_AUTHORIZATION;
  case DOC_OAUTH_FLOW_UNSET:
  default:
    return OA_OAUTH_FLOW_UNKNOWN;
  }
}

static struct OpenAPI_SecurityScheme *
spec_find_security_scheme(struct OpenAPI_Spec *spec, const char *name) {
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_security_schemes; ++i) {
    if (spec->security_schemes[i].name &&
        strcmp(spec->security_schemes[i].name, name) == 0)
      return &spec->security_schemes[i];
  }
  return NULL;
}

static int set_str_if_missing(char **dst, const char *src) {
  if (!src || !*src)
    return 0;
  if (!*dst) {
    *dst = c_cdd_strdup(src);
    if (!*dst)
      return ENOMEM;
    return 0;
  }
  if (strcmp(*dst, src) != 0)
    return EINVAL;
  return 0;
}

static void free_openapi_server_variables(struct OpenAPI_Server *srv) {
  size_t i;
  if (!srv || !srv->variables)
    return;
  for (i = 0; i < srv->n_variables; ++i) {
    size_t e;
    struct OpenAPI_ServerVariable *var = &srv->variables[i];
    if (var->name)
      free(var->name);
    if (var->default_value)
      free(var->default_value);
    if (var->description)
      free(var->description);
    if (var->enum_values) {
      for (e = 0; e < var->n_enum_values; ++e) {
        free(var->enum_values[e]);
      }
      free(var->enum_values);
    }
  }
  free(srv->variables);
  srv->variables = NULL;
  srv->n_variables = 0;
}

static int copy_doc_server_variables(struct OpenAPI_Server *dst,
                                     const struct DocServer *src) {
  size_t i;
  if (!dst || !src || src->n_variables == 0)
    return 0;

  dst->variables = (struct OpenAPI_ServerVariable *)calloc(
      src->n_variables, sizeof(struct OpenAPI_ServerVariable));
  if (!dst->variables)
    return ENOMEM;
  dst->n_variables = src->n_variables;

  for (i = 0; i < src->n_variables; ++i) {
    size_t e;
    int found_default = 0;
    const struct DocServerVar *sv = &src->variables[i];
    struct OpenAPI_ServerVariable *dv = &dst->variables[i];

    if (!sv->name || !sv->default_value) {
      free_openapi_server_variables(dst);
      return EINVAL;
    }
    dv->name = c_cdd_strdup(sv->name);
    if (!dv->name) {
      free_openapi_server_variables(dst);
      return ENOMEM;
    }
    dv->default_value = c_cdd_strdup(sv->default_value);
    if (!dv->default_value) {
      free_openapi_server_variables(dst);
      return ENOMEM;
    }
    if (sv->description) {
      dv->description = c_cdd_strdup(sv->description);
      if (!dv->description) {
        free_openapi_server_variables(dst);
        return ENOMEM;
      }
    }
    if (sv->enum_values && sv->n_enum_values > 0) {
      dv->enum_values = (char **)calloc(sv->n_enum_values, sizeof(char *));
      if (!dv->enum_values) {
        free_openapi_server_variables(dst);
        return ENOMEM;
      }
      dv->n_enum_values = sv->n_enum_values;
      for (e = 0; e < sv->n_enum_values; ++e) {
        dv->enum_values[e] = c_cdd_strdup(sv->enum_values[e]);
        if (!dv->enum_values[e]) {
          free_openapi_server_variables(dst);
          return ENOMEM;
        }
        if (strcmp(sv->enum_values[e], sv->default_value) == 0)
          found_default = 1;
      }
      if (!found_default) {
        free_openapi_server_variables(dst);
        return EINVAL;
      }
    }
  }

  return 0;
}

static int merge_scopes(struct OpenAPI_OAuthFlow *dst,
                        const struct DocOAuthFlow *src) {
  size_t i;
  if (!dst || !src || !src->scopes || src->n_scopes == 0)
    return 0;

  for (i = 0; i < src->n_scopes; ++i) {
    const char *name = src->scopes[i].name;
    const char *desc = src->scopes[i].description;
    size_t j;
    int found = 0;
    for (j = 0; j < dst->n_scopes; ++j) {
      if (dst->scopes[j].name && name &&
          strcmp(dst->scopes[j].name, name) == 0) {
        found = 1;
        if (dst->scopes[j].description && desc &&
            strcmp(dst->scopes[j].description, desc) != 0)
          return EINVAL;
        if (!dst->scopes[j].description && desc) {
          dst->scopes[j].description = c_cdd_strdup(desc);
          if (!dst->scopes[j].description)
            return ENOMEM;
        }
        break;
      }
    }
    if (!found) {
      struct OpenAPI_OAuthScope *new_scopes =
          (struct OpenAPI_OAuthScope *)realloc(
              dst->scopes, (dst->n_scopes + 1) * sizeof(*dst->scopes));
      if (!new_scopes)
        return ENOMEM;
      dst->scopes = new_scopes;
      dst->scopes[dst->n_scopes].name = c_cdd_strdup(name ? name : "");
      if (!dst->scopes[dst->n_scopes].name)
        return ENOMEM;
      dst->scopes[dst->n_scopes].description = desc ? c_cdd_strdup(desc) : NULL;
      if (desc && !dst->scopes[dst->n_scopes].description)
        return ENOMEM;
      dst->n_scopes++;
    }
  }
  return 0;
}

static struct OpenAPI_OAuthFlow *
find_oauth_flow(struct OpenAPI_SecurityScheme *scheme,
                enum OpenAPI_OAuthFlowType type) {
  size_t i;
  if (!scheme || !scheme->flows)
    return NULL;
  for (i = 0; i < scheme->n_flows; ++i) {
    if (scheme->flows[i].type == type)
      return &scheme->flows[i];
  }
  return NULL;
}

static int merge_oauth_flow(struct OpenAPI_OAuthFlow *dst,
                            const struct DocOAuthFlow *src) {
  int rc;
  if (!dst || !src)
    return 0;
  rc = set_str_if_missing(&dst->authorization_url, src->authorization_url);
  if (rc != 0)
    return rc;
  rc = set_str_if_missing(&dst->token_url, src->token_url);
  if (rc != 0)
    return rc;
  rc = set_str_if_missing(&dst->refresh_url, src->refresh_url);
  if (rc != 0)
    return rc;
  rc = set_str_if_missing(&dst->device_authorization_url,
                          src->device_authorization_url);
  if (rc != 0)
    return rc;
  return merge_scopes(dst, src);
}

static int validate_doc_oauth_flow(const struct DocOAuthFlow *flow) {
  if (!flow)
    return EINVAL;
  if (flow->type == DOC_OAUTH_FLOW_UNSET)
    return EINVAL;
  switch (flow->type) {
  case DOC_OAUTH_FLOW_IMPLICIT:
    if (!flow->authorization_url)
      return EINVAL;
    break;
  case DOC_OAUTH_FLOW_PASSWORD:
    if (!flow->token_url)
      return EINVAL;
    break;
  case DOC_OAUTH_FLOW_CLIENT_CREDENTIALS:
    if (!flow->token_url)
      return EINVAL;
    break;
  case DOC_OAUTH_FLOW_AUTHORIZATION_CODE:
    if (!flow->authorization_url || !flow->token_url)
      return EINVAL;
    break;
  case DOC_OAUTH_FLOW_DEVICE_AUTHORIZATION:
    if (!flow->device_authorization_url || !flow->token_url)
      return EINVAL;
    break;
  default:
    return EINVAL;
  }
  return 0;
}

static int add_oauth_flows(struct OpenAPI_SecurityScheme *scheme,
                           const struct DocSecurityScheme *doc) {
  size_t i;
  if (!scheme || !doc)
    return 0;
  for (i = 0; i < doc->n_flows; ++i) {
    enum OpenAPI_OAuthFlowType flow_type =
        map_doc_flow_type(doc->flows[i].type);
    struct OpenAPI_OAuthFlow *dst_flow;
    if (flow_type == OA_OAUTH_FLOW_UNKNOWN)
      return EINVAL;
    dst_flow = find_oauth_flow(scheme, flow_type);
    if (dst_flow) {
      int rc = merge_oauth_flow(dst_flow, &doc->flows[i]);
      if (rc != 0)
        return rc;
      continue;
    }
    {
      struct OpenAPI_OAuthFlow *new_flows = (struct OpenAPI_OAuthFlow *)realloc(
          scheme->flows,
          (scheme->n_flows + 1) * sizeof(struct OpenAPI_OAuthFlow));
      if (!new_flows)
        return ENOMEM;
      scheme->flows = new_flows;
      dst_flow = &scheme->flows[scheme->n_flows];
      memset(dst_flow, 0, sizeof(*dst_flow));
      dst_flow->type = flow_type;
      if (doc->flows[i].authorization_url)
        dst_flow->authorization_url =
            c_cdd_strdup(doc->flows[i].authorization_url);
      if (doc->flows[i].token_url)
        dst_flow->token_url = c_cdd_strdup(doc->flows[i].token_url);
      if (doc->flows[i].refresh_url)
        dst_flow->refresh_url = c_cdd_strdup(doc->flows[i].refresh_url);
      if (doc->flows[i].device_authorization_url)
        dst_flow->device_authorization_url =
            c_cdd_strdup(doc->flows[i].device_authorization_url);
      if ((doc->flows[i].authorization_url && !dst_flow->authorization_url) ||
          (doc->flows[i].token_url && !dst_flow->token_url) ||
          (doc->flows[i].refresh_url && !dst_flow->refresh_url) ||
          (doc->flows[i].device_authorization_url &&
           !dst_flow->device_authorization_url))
        return ENOMEM;
      if (doc->flows[i].scopes && doc->flows[i].n_scopes > 0) {
        size_t s;
        dst_flow->scopes = (struct OpenAPI_OAuthScope *)calloc(
            doc->flows[i].n_scopes, sizeof(struct OpenAPI_OAuthScope));
        if (!dst_flow->scopes)
          return ENOMEM;
        dst_flow->n_scopes = doc->flows[i].n_scopes;
        for (s = 0; s < doc->flows[i].n_scopes; ++s) {
          const char *name = doc->flows[i].scopes[s].name;
          const char *desc = doc->flows[i].scopes[s].description;
          dst_flow->scopes[s].name = c_cdd_strdup(name ? name : "");
          if (!dst_flow->scopes[s].name)
            return ENOMEM;
          if (desc) {
            dst_flow->scopes[s].description = c_cdd_strdup(desc);
            if (!dst_flow->scopes[s].description)
              return ENOMEM;
          }
        }
      }
      scheme->n_flows++;
    }
  }
  return 0;
}

static int spec_add_security_scheme(struct OpenAPI_Spec *spec,
                                    const struct DocSecurityScheme *doc) {
  struct OpenAPI_SecurityScheme *scheme;
  enum OpenAPI_SecurityType type;

  if (!spec || !doc || !doc->name || !*doc->name)
    return 0;

  type = map_doc_security_type(doc->type);
  if (type == OA_SEC_UNKNOWN)
    return EINVAL;

  if (type == OA_SEC_OAUTH2 && doc->n_flows > 0) {
    size_t i;
    for (i = 0; i < doc->n_flows; ++i) {
      int rc = validate_doc_oauth_flow(&doc->flows[i]);
      if (rc != 0)
        return rc;
    }
  }

  scheme = spec_find_security_scheme(spec, doc->name);
  if (!scheme) {
    struct OpenAPI_SecurityScheme *new_schemes =
        (struct OpenAPI_SecurityScheme *)realloc(
            spec->security_schemes, (spec->n_security_schemes + 1) *
                                        sizeof(struct OpenAPI_SecurityScheme));
    if (!new_schemes)
      return ENOMEM;
    spec->security_schemes = new_schemes;
    scheme = &spec->security_schemes[spec->n_security_schemes];
    memset(scheme, 0, sizeof(*scheme));
    scheme->name = c_cdd_strdup(doc->name);
    if (!scheme->name)
      return ENOMEM;
    scheme->type = type;
    spec->n_security_schemes++;
  } else if (scheme->type != type) {
    return EINVAL;
  }

  if (doc->description) {
    int rc = set_str_if_missing(&scheme->description, doc->description);
    if (rc != 0)
      return rc;
  }
  if (doc->deprecated_set) {
    if (!scheme->deprecated_set) {
      scheme->deprecated_set = 1;
      scheme->deprecated = doc->deprecated;
    } else if (scheme->deprecated != doc->deprecated) {
      return EINVAL;
    }
  }

  switch (type) {
  case OA_SEC_APIKEY: {
    enum OpenAPI_SecurityIn in = map_doc_security_in(doc->in);
    if (!doc->param_name || !*doc->param_name || in == OA_SEC_IN_UNKNOWN)
      return EINVAL;
    scheme->in = in;
    {
      int rc = set_str_if_missing(&scheme->key_name, doc->param_name);
      if (rc != 0)
        return rc;
    }
    break;
  }
  case OA_SEC_HTTP:
    if (!doc->scheme || !*doc->scheme)
      return EINVAL;
    {
      int rc = set_str_if_missing(&scheme->scheme, doc->scheme);
      if (rc != 0)
        return rc;
    }
    if (doc->bearer_format) {
      int rc = set_str_if_missing(&scheme->bearer_format, doc->bearer_format);
      if (rc != 0)
        return rc;
    }
    break;
  case OA_SEC_OPENID:
    if (!doc->open_id_connect_url || !*doc->open_id_connect_url)
      return EINVAL;
    {
      int rc = set_str_if_missing(&scheme->open_id_connect_url,
                                  doc->open_id_connect_url);
      if (rc != 0)
        return rc;
    }
    break;
  case OA_SEC_OAUTH2:
    if (doc->oauth2_metadata_url) {
      int rc = set_str_if_missing(&scheme->oauth2_metadata_url,
                                  doc->oauth2_metadata_url);
      if (rc != 0)
        return rc;
    }
    if (doc->n_flows > 0) {
      int rc = add_oauth_flows(scheme, doc);
      if (rc != 0)
        return rc;
    } else if (scheme->n_flows == 0) {
      return EINVAL;
    }
    break;
  case OA_SEC_MUTUALTLS:
    break;
  default:
    return EINVAL;
  }

  return 0;
}

static int apply_doc_security_schemes(struct OpenAPI_Spec *spec,
                                      const struct DocMetadata *meta) {
  size_t i;
  if (!spec || !meta || meta->n_security_schemes == 0)
    return 0;
  for (i = 0; i < meta->n_security_schemes; ++i) {
    int rc = spec_add_security_scheme(spec, &meta->security_schemes[i]);
    if (rc != 0)
      return rc;
  }
  return 0;
}

static int append_root_security(struct OpenAPI_Spec *spec,
                                const struct DocMetadata *meta) {
  size_t i;
  if (!spec || !meta || meta->n_security == 0)
    return 0;

  {
    struct OpenAPI_SecurityRequirementSet *new_sets =
        (struct OpenAPI_SecurityRequirementSet *)realloc(
            spec->security, (spec->n_security + meta->n_security) *
                                sizeof(struct OpenAPI_SecurityRequirementSet));
    if (!new_sets)
      return ENOMEM;
    spec->security = new_sets;
  }
  for (i = 0; i < meta->n_security; ++i) {
    const struct DocSecurityRequirement *src = &meta->security[i];
    struct OpenAPI_SecurityRequirementSet *set =
        &spec->security[spec->n_security + i];
    memset(set, 0, sizeof(*set));
    set->requirements = (struct OpenAPI_SecurityRequirement *)calloc(
        1, sizeof(struct OpenAPI_SecurityRequirement));
    if (!set->requirements)
      return ENOMEM;
    set->n_requirements = 1;
    set->requirements[0].scheme = c_cdd_strdup(src->scheme ? src->scheme : "");
    if (!set->requirements[0].scheme)
      return ENOMEM;
    if (src->n_scopes > 0) {
      size_t s;
      set->requirements[0].scopes =
          (char **)calloc(src->n_scopes, sizeof(char *));
      if (!set->requirements[0].scopes)
        return ENOMEM;
      set->requirements[0].n_scopes = src->n_scopes;
      for (s = 0; s < src->n_scopes; ++s) {
        set->requirements[0].scopes[s] =
            c_cdd_strdup(src->scopes[s] ? src->scopes[s] : "");
        if (!set->requirements[0].scopes[s])
          return ENOMEM;
      }
    }
  }
  spec->n_security += meta->n_security;
  spec->security_set = 1;
  return 0;
}

static int append_root_servers(struct OpenAPI_Spec *spec,
                               const struct DocMetadata *meta) {
  size_t i;
  if (!spec || !meta || meta->n_servers == 0)
    return 0;
  {
    struct OpenAPI_Server *new_servers = (struct OpenAPI_Server *)realloc(
        spec->servers,
        (spec->n_servers + meta->n_servers) * sizeof(struct OpenAPI_Server));
    if (!new_servers)
      return ENOMEM;
    spec->servers = new_servers;
  }
  for (i = 0; i < meta->n_servers; ++i) {
    const struct DocServer *src = &meta->servers[i];
    struct OpenAPI_Server *dst = &spec->servers[spec->n_servers + i];
    memset(dst, 0, sizeof(*dst));
    if (src->url) {
      dst->url = c_cdd_strdup(src->url);
      if (!dst->url)
        return ENOMEM;
    }
    if (src->name) {
      dst->name = c_cdd_strdup(src->name);
      if (!dst->name)
        return ENOMEM;
    }
    if (src->description) {
      dst->description = c_cdd_strdup(src->description);
      if (!dst->description)
        return ENOMEM;
    }
    if (src->n_variables > 0) {
      int vrc = copy_doc_server_variables(dst, src);
      if (vrc != 0)
        return vrc;
    }
  }
  spec->n_servers += meta->n_servers;
  return 0;
}

static int apply_doc_global_meta(struct OpenAPI_Spec *spec,
                                 const struct DocMetadata *meta) {
  int rc;
  if (!spec || !meta)
    return 0;
  if (meta->json_schema_dialect) {
    rc = set_str_if_missing(&spec->json_schema_dialect,
                            meta->json_schema_dialect);
    if (rc != 0)
      return rc;
  }
  if (meta->info_title) {
    rc = set_str_if_missing(&spec->info.title, meta->info_title);
    if (rc != 0)
      return rc;
  }
  if (meta->info_version) {
    rc = set_str_if_missing(&spec->info.version, meta->info_version);
    if (rc != 0)
      return rc;
  }
  if (meta->info_summary) {
    rc = set_str_if_missing(&spec->info.summary, meta->info_summary);
    if (rc != 0)
      return rc;
  }
  if (meta->info_description) {
    rc = set_str_if_missing(&spec->info.description, meta->info_description);
    if (rc != 0)
      return rc;
  }
  if (meta->terms_of_service) {
    rc = set_str_if_missing(&spec->info.terms_of_service,
                            meta->terms_of_service);
    if (rc != 0)
      return rc;
  }
  if (meta->contact_name || meta->contact_url || meta->contact_email) {
    if (meta->contact_name) {
      rc = set_str_if_missing(&spec->info.contact.name, meta->contact_name);
      if (rc != 0)
        return rc;
    }
    if (meta->contact_url) {
      rc = set_str_if_missing(&spec->info.contact.url, meta->contact_url);
      if (rc != 0)
        return rc;
    }
    if (meta->contact_email) {
      rc = set_str_if_missing(&spec->info.contact.email, meta->contact_email);
      if (rc != 0)
        return rc;
    }
  }
  if (meta->license_name || meta->license_url || meta->license_identifier) {
    if (!meta->license_name && !spec->info.license.name)
      return EINVAL;
    if (meta->license_url && meta->license_identifier)
      return EINVAL;
    if (spec->info.license.url && meta->license_identifier)
      return EINVAL;
    if (spec->info.license.identifier && meta->license_url)
      return EINVAL;
    if (meta->license_name) {
      rc = set_str_if_missing(&spec->info.license.name, meta->license_name);
      if (rc != 0)
        return rc;
    }
    if (meta->license_url) {
      rc = set_str_if_missing(&spec->info.license.url, meta->license_url);
      if (rc != 0)
        return rc;
    }
    if (meta->license_identifier) {
      rc = set_str_if_missing(&spec->info.license.identifier,
                              meta->license_identifier);
      if (rc != 0)
        return rc;
    }
  }
  if (meta->external_docs_url) {
    if (!spec->external_docs.url) {
      spec->external_docs.url = c_cdd_strdup(meta->external_docs_url);
      if (!spec->external_docs.url)
        return ENOMEM;
      if (meta->external_docs_description) {
        spec->external_docs.description =
            c_cdd_strdup(meta->external_docs_description);
        if (!spec->external_docs.description)
          return ENOMEM;
      }
    } else if (strcmp(spec->external_docs.url, meta->external_docs_url) != 0) {
      return EINVAL;
    } else if (!spec->external_docs.description &&
               meta->external_docs_description) {
      spec->external_docs.description =
          c_cdd_strdup(meta->external_docs_description);
      if (!spec->external_docs.description)
        return ENOMEM;
    }
  }
  rc = append_root_servers(spec, meta);
  if (rc != 0)
    return rc;
  rc = append_root_security(spec, meta);
  if (rc != 0)
    return rc;
  return 0;
}

static int spec_apply_tag_meta(struct OpenAPI_Spec *spec,
                               const struct DocTagMeta *meta) {
  struct OpenAPI_Tag *tag;
  if (!spec || !meta || !meta->name || !*meta->name)
    return 0;
  if (!spec_has_tag(spec, meta->name)) {
    int rc = spec_add_tag(spec, meta->name);
    if (rc != 0)
      return rc;
  }
  tag = spec_find_tag(spec, meta->name);
  if (!tag)
    return 0;
  if (meta->summary && !tag->summary) {
    tag->summary = c_cdd_strdup(meta->summary);
    if (!tag->summary)
      return ENOMEM;
  }
  if (meta->description && !tag->description) {
    tag->description = c_cdd_strdup(meta->description);
    if (!tag->description)
      return ENOMEM;
  }
  if (meta->parent && !tag->parent) {
    tag->parent = c_cdd_strdup(meta->parent);
    if (!tag->parent)
      return ENOMEM;
  }
  if (meta->kind && !tag->kind) {
    tag->kind = c_cdd_strdup(meta->kind);
    if (!tag->kind)
      return ENOMEM;
  }
  if (meta->external_docs_url && !tag->external_docs.url) {
    tag->external_docs.url = c_cdd_strdup(meta->external_docs_url);
    if (!tag->external_docs.url)
      return ENOMEM;
  }
  if (meta->external_docs_description && !tag->external_docs.description &&
      tag->external_docs.url) {
    tag->external_docs.description =
        c_cdd_strdup(meta->external_docs_description);
    if (!tag->external_docs.description)
      return ENOMEM;
  }
  return 0;
}

static int apply_doc_tag_meta(struct OpenAPI_Spec *spec,
                              const struct DocMetadata *meta) {
  size_t i;
  int rc = 0;
  if (!spec || !meta || !meta->tag_meta || meta->n_tag_meta == 0)
    return 0;
  for (i = 0; i < meta->n_tag_meta; ++i) {
    rc = spec_apply_tag_meta(spec, &meta->tag_meta[i]);
    if (rc != 0)
      return rc;
  }
  return 0;
}

static int collect_tags_from_op(struct OpenAPI_Spec *spec,
                                const struct OpenAPI_Operation *op) {
  size_t i;
  if (!spec || !op || !op->tags)
    return 0;
  for (i = 0; i < op->n_tags; ++i) {
    int rc = spec_add_tag(spec, op->tags[i]);
    if (rc != 0)
      return rc;
  }
  return 0;
}

static int collect_tags_from_paths(struct OpenAPI_Spec *spec,
                                   const struct OpenAPI_Path *paths,
                                   size_t n_paths) {
  size_t i;
  if (!spec || !paths)
    return 0;
  for (i = 0; i < n_paths; ++i) {
    size_t j;
    const struct OpenAPI_Path *path = &paths[i];
    for (j = 0; j < path->n_operations; ++j) {
      int rc = collect_tags_from_op(spec, &path->operations[j]);
      if (rc != 0)
        return rc;
    }
    for (j = 0; j < path->n_additional_operations; ++j) {
      int rc = collect_tags_from_op(spec, &path->additional_operations[j]);
      if (rc != 0)
        return rc;
    }
  }
  return 0;
}

static int collect_spec_tags(struct OpenAPI_Spec *spec) {
  int rc;
  if (!spec)
    return EINVAL;
  rc = collect_tags_from_paths(spec, spec->paths, spec->n_paths);
  if (rc != 0)
    return rc;
  rc = collect_tags_from_paths(spec, spec->webhooks, spec->n_webhooks);
  if (rc != 0)
    return rc;
  return 0;
}

/**
 * @brief Simple signature parser to split "int foo(int x, char *y)"
 * Populates `out`. Caller must free internals.
 */
static int parse_c_signature_string(const char *sig_str,
                                    struct C2OpenAPI_ParsedSig *out) {
  struct TokenList *tl = NULL;
  size_t i;
  int rc = 0;
  size_t lp = 0, rp = 0;

  if (!sig_str || !out)
    return EINVAL;

  memset(out, 0, sizeof(*out));

  if (tokenize(az_span_create_from_str((char *)sig_str), &tl) != 0) {
    return EINVAL;
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
    return EINVAL;
  }

  /* Extract Name (token before lparen, ignoring WS) */
  {
    size_t k = lp - 1;
    while (k > 0 && tl->tokens[k].kind == TOKEN_WHITESPACE)
      k--;
    if (tl->tokens[k].kind == TOKEN_IDENTIFIER) {
      size_t len = tl->tokens[k].length;
      char *n = malloc(len + 1);
      if (!n) {
        rc = ENOMEM;
        goto cleanup;
      }
      memcpy(n, tl->tokens[k].start, len);
      n[len] = '\0';
      out->name = n;
    }
  }

  if (!out->name) {
    rc = EINVAL;
    goto cleanup;
  }

  /* Extract Args between ( and ) */
  /* Split by COMMA. For each segment, last ID is name, rest is type. */
  rp = token_find_next(tl, lp, tl->size, TOKEN_RPAREN);
  if (rp >= tl->size) {
    rc = EINVAL;
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
          /* Type is start..name_idx (exclusive) + modifiers after? */
          /* Simple approach: Type is [start, name_idx), Name is name_idx.
             Postfix arrays `[]` might be after name. */
          /* Let's grab name string */
          const struct Token *nt = &tl->tokens[name_idx];
          size_t t_end = name_idx;

          /* Check if type is pointer/const/struct before name */
          /* Construct type string */
          size_t t_len = 0;
          char *t_str;
          size_t m;
          char *n_str = malloc(nt->length + 1);
          if (!n_str) {
            rc = ENOMEM;
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

          t_str = malloc(t_len + 1);
          if (!t_str) {
            free(n_str);
            rc = ENOMEM;
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

          /* Add to list */
          {
            struct C2OpenAPI_ParsedArg *new_arr =
                realloc(out->args,
                        (out->n_args + 1) * sizeof(struct C2OpenAPI_ParsedArg));
            if (!new_arr) {
              free(n_str);
              free(t_str);
              rc = ENOMEM;
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
  if (rc != 0) {
    if (out->name)
      free(out->name);
    if (out->args) {
      size_t k;
      for (k = 0; k < out->n_args; k++) {
        free(out->args[k].name);
        free(out->args[k].type);
      }
      free(out->args);
    }
    memset(out, 0, sizeof(*out));
  }
  return rc;
}

static void free_parsed_sig(struct C2OpenAPI_ParsedSig *sig) {
  size_t i;
  if (sig->name)
    free(sig->name);
  if (sig->return_type)
    free(sig->return_type);
  if (sig->args) {
    for (i = 0; i < sig->n_args; ++i) {
      free(sig->args[i].name);
      free(sig->args[i].type);
    }
    free(sig->args);
  }
  memset(sig, 0, sizeof(*sig));
}

static int process_file(const char *path, struct OpenAPI_Spec *spec) {
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tokens = NULL;
  struct CstNodeList cst = {0};
  int *comment_used = NULL;
  int rc;
  size_t i;

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
  if (rc != 0)
    return rc;

  if (tokenize(az_span_create_from_str(content), &tokens) != 0) {
    free(content);
    return EIO;
  }
  parse_tokens(tokens, &cst); /* Best effort */

  if (cst.size > 0) {
    comment_used = (int *)calloc(cst.size, sizeof(int));
    if (!comment_used) {
      free_cst_node_list(&cst);
      free_token_list(tokens);
      free(content);
      return ENOMEM;
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
      } else if (i > 1 && cst.nodes[i - 1].kind == CST_NODE_WHITESPACE &&
                 cst.nodes[i - 2].kind == CST_NODE_COMMENT) {
        doc_node = &cst.nodes[i - 2];
        doc_index = i - 2;
      }

      if (doc_node) {
        /* Extract comment text */
        char *doc_text = malloc(doc_node->length + 1);
        if (doc_text) {
          struct DocMetadata meta;
          memcpy(doc_text, doc_node->start, doc_node->length);
          doc_text[doc_node->length] = '\0';

          doc_metadata_init(&meta);
          if (doc_parse_block(doc_text, &meta) == 0) {
            int rc_meta = apply_doc_tag_meta(spec, &meta);
            if (rc_meta == 0)
              rc_meta = apply_doc_security_schemes(spec, &meta);
            if (rc_meta == 0)
              rc_meta = apply_doc_global_meta(spec, &meta);
            if (rc_meta != 0) {
              doc_metadata_free(&meta);
              free(doc_text);
              free_cst_node_list(&cst);
              free_token_list(tokens);
              free(content);
              if (comment_used)
                free(comment_used);
              return rc_meta;
            }
            if (comment_used && doc_index != (size_t)-1)
              comment_used[doc_index] = 1;
          }
          if (meta.route) {
            /* Found Valid Documented Route! */

            /* Extract Signature Text */
            size_t sig_len =
                func_node->length; /* Approximation, includes body? */
            /* We need signature string up to brace. CST Node includes body. */
            char *sig_raw = malloc(sig_len + 1);
            if (sig_raw) {
              struct C2OpenAPI_ParsedSig psig;
              const uint8_t *brace = memchr(func_node->start, '{', sig_len);
              size_t effective_len =
                  brace ? (size_t)(brace - func_node->start) : sig_len;

              memcpy(sig_raw, func_node->start, effective_len);
              sig_raw[effective_len] = '\0';

              /* Parse Signature */
              if (parse_c_signature_string(sig_raw, &psig) == 0) {
                struct OpenAPI_Operation op = {0};
                struct OpBuilderContext ctx;

                ctx.sig = &psig;
                ctx.doc = &meta;
                ctx.func_name = psig.name;

                /* Build Operation */
                if (c2openapi_build_operation(&ctx, &op) == 0) {
                  /* Aggregate */
                  if (meta.is_webhook) {
                    openapi_aggregator_add_webhook_operation(spec, meta.route,
                                                             &op);
                  } else {
                    openapi_aggregator_add_operation(spec, meta.route, &op);
                  }
                }
                free_parsed_sig(&psig);
              }
              free(sig_raw);
            }
          }
          doc_metadata_free(&meta);
          free(doc_text);
        }
      }
    }
  }

  /* Parse standalone comment blocks for global metadata. */
  if (comment_used) {
    for (i = 0; i < cst.size; ++i) {
      if (cst.nodes[i].kind == CST_NODE_COMMENT && !comment_used[i]) {
        struct DocMetadata meta;
        char *doc_text = malloc(cst.nodes[i].length + 1);
        if (!doc_text)
          continue;
        memcpy(doc_text, cst.nodes[i].start, cst.nodes[i].length);
        doc_text[cst.nodes[i].length] = '\0';

        doc_metadata_init(&meta);
        if (doc_parse_block(doc_text, &meta) == 0) {
          int rc_meta = apply_doc_tag_meta(spec, &meta);
          if (rc_meta == 0)
            rc_meta = apply_doc_security_schemes(spec, &meta);
          if (rc_meta == 0)
            rc_meta = apply_doc_global_meta(spec, &meta);
          if (rc_meta != 0) {
            doc_metadata_free(&meta);
            free(doc_text);
            free_cst_node_list(&cst);
            free_token_list(tokens);
            free(content);
            free(comment_used);
            return rc_meta;
          }
        }
        doc_metadata_free(&meta);
        free(doc_text);
      }
    }
  }

  free_cst_node_list(&cst);
  free_token_list(tokens);
  free(content);
  if (comment_used)
    free(comment_used);
  return 0;
}

static int walker_cb(const char *path, void *user_data) {
  struct OpenAPI_Spec *spec = (struct OpenAPI_Spec *)user_data;
  if (!is_source_file(path))
    return 0;
  printf("Scanning: %s\n", path);
  process_file(path, spec);
  return 0;
}

static int load_base_spec(const char *path, struct OpenAPI_Spec *spec) {
  JSON_Value *root = NULL;
  int rc;

  if (!path || !spec)
    return EINVAL;

  root = json_parse_file(path);
  if (!root)
    return EINVAL;

  rc = openapi_load_from_json(root, spec);
  json_value_free(root);
  return rc;
}

int c2openapi_cli_main(int argc, char **argv) {
  struct OpenAPI_Spec spec;
  const char *src_dir;
  const char *out_file;
  const char *base_file = NULL;
  const char *self_uri = NULL;
  const char *dialect_uri = NULL;
  char *json = NULL;
  int rc;
  int argi = 1;

  while (argi < argc && argv[argi][0] == '-') {
    if (strcmp(argv[argi], "--base") == 0 || strcmp(argv[argi], "-b") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr,
                "Usage: c2openapi [--base <openapi.json>] [--self <uri>] "
                "[--dialect <uri>] "
                "<src_dir> <out.json>\n");
        return EXIT_FAILURE;
      }
      base_file = argv[argi + 1];
      argi += 2;
      continue;
    }
    if (strcmp(argv[argi], "--self") == 0 || strcmp(argv[argi], "-s") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr,
                "Usage: c2openapi [--base <openapi.json>] [--self <uri>] "
                "[--dialect <uri>] "
                "<src_dir> <out.json>\n");
        return EXIT_FAILURE;
      }
      self_uri = argv[argi + 1];
      argi += 2;
      continue;
    }
    if (strcmp(argv[argi], "--dialect") == 0 ||
        strcmp(argv[argi], "--jsonSchemaDialect") == 0) {
      if (argi + 1 >= argc) {
        fprintf(stderr,
                "Usage: c2openapi [--base <openapi.json>] [--self <uri>] "
                "[--dialect <uri>] "
                "<src_dir> <out.json>\n");
        return EXIT_FAILURE;
      }
      dialect_uri = argv[argi + 1];
      argi += 2;
      continue;
    }
    break;
  }

  if (argc - argi != 2) {
    fprintf(stderr, "Usage: c2openapi [--base <openapi.json>] [--self <uri>] "
                    "[--dialect <uri>] "
                    "<src_dir> <out.json>\n");
    return EXIT_FAILURE;
  }

  src_dir = argv[argi];
  out_file = argv[argi + 1];

  openapi_spec_init(&spec);

  if (base_file) {
    rc = load_base_spec(base_file, &spec);
    if (rc != 0) {
      fprintf(stderr, "Failed to load base OpenAPI spec %s: %d\n", base_file,
              rc);
      openapi_spec_free(&spec);
      return EXIT_FAILURE;
    }
  }

  if (self_uri && *self_uri) {
    if (spec.self_uri)
      free(spec.self_uri);
    spec.self_uri = strdup(self_uri);
    if (!spec.self_uri) {
      fprintf(stderr, "Failed to set $self URI\n");
      openapi_spec_free(&spec);
      return EXIT_FAILURE;
    }
  }
  if (dialect_uri && *dialect_uri) {
    if (spec.json_schema_dialect)
      free(spec.json_schema_dialect);
    spec.json_schema_dialect = strdup(dialect_uri);
    if (!spec.json_schema_dialect) {
      fprintf(stderr, "Failed to set jsonSchemaDialect\n");
      openapi_spec_free(&spec);
      return EXIT_FAILURE;
    }
  }

  /* 1. Walk & Process */
  rc = walk_directory(src_dir, walker_cb, &spec);
  if (rc != 0) {
    fprintf(stderr, "Error walking directory %s: %d\n", src_dir, rc);
    openapi_spec_free(&spec);
    return EXIT_FAILURE;
  }

  /* Derive top-level tags from operation tags */
  rc = collect_spec_tags(&spec);
  if (rc != 0) {
    fprintf(stderr, "Error collecting tags: %d\n", rc);
    openapi_spec_free(&spec);
    return EXIT_FAILURE;
  }

  /* 2. Write */
  rc = openapi_write_spec_to_json(&spec, &json);
  if (rc != 0 || !json) {
    fprintf(stderr, "Error serializing spec: %d\n", rc);
    openapi_spec_free(&spec);
    return EXIT_FAILURE;
  }

  rc = fs_write_to_file(out_file, json);
  if (rc != 0) {
    fprintf(stderr, "Failed to write %s\n", out_file);
    rc = EXIT_FAILURE;
  } else {
    printf("Written %s\n", out_file);
    rc = 0;
  }

  free(json);
  openapi_spec_free(&spec);

  return (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
