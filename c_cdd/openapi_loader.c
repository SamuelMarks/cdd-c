/**
 * @file openapi_loader.c
 * @brief Implementation of OpenAPI extraction logic.
 *
 * Includes logic to parse `explode` and `style` for parameters,
 * `tags` for operation grouping, and full schema definitions.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "code2schema.h" /* for json_object_to_struct_fields */
#include "openapi_loader.h"
#include "str_utils.h"

/* --- Helper Function Prototypes --- */

static enum OpenAPI_Verb parse_verb(const char *v);
static enum OpenAPI_ParamIn parse_param_in(const char *in);
static enum OpenAPI_Style parse_param_style(const char *s);
static enum OpenAPI_SecurityType parse_security_type(const char *type);
static enum OpenAPI_SecurityIn parse_security_in(const char *in);
static int parse_info(const JSON_Object *root_obj, struct OpenAPI_Spec *out);
static int parse_external_docs(const JSON_Object *obj,
                               struct OpenAPI_ExternalDocs *out);
static int parse_tags(const JSON_Object *root_obj, struct OpenAPI_Spec *out);
static int parse_servers_array(const JSON_Object *parent, const char *key,
                               struct OpenAPI_Server **out_servers,
                               size_t *out_count);
static int parse_security_field(const JSON_Object *obj, const char *key,
                                struct OpenAPI_SecurityRequirementSet **out,
                                size_t *out_count, int *out_set);
static int parse_schema_ref(const JSON_Object *schema,
                            struct OpenAPI_SchemaRef *out);
static int parse_parameters_array(const JSON_Array *arr,
                                  struct OpenAPI_Parameter **out_params,
                                  size_t *out_count);
static int parse_operation(const char *verb_str, const JSON_Object *op_obj,
                           struct OpenAPI_Operation *out_op);
static int parse_servers(const JSON_Object *root_obj,
                         struct OpenAPI_Spec *out);
static int parse_security_schemes(const JSON_Object *components,
                                  struct OpenAPI_Spec *out);
static int parse_components(const JSON_Object *components,
                            struct OpenAPI_Spec *out);
static int parse_paths_object(const JSON_Object *paths_obj,
                              struct OpenAPI_Path **out_paths,
                              size_t *out_count);

/* --- Lifecycle Implementation --- */

void openapi_spec_init(struct OpenAPI_Spec *const spec) {
  if (spec) {
    spec->openapi_version = NULL;
    spec->self_uri = NULL;
    spec->json_schema_dialect = NULL;
    memset(&spec->info, 0, sizeof(spec->info));
    memset(&spec->external_docs, 0, sizeof(spec->external_docs));
    spec->tags = NULL;
    spec->n_tags = 0;
    spec->security = NULL;
    spec->n_security = 0;
    spec->security_set = 0;
    spec->servers = NULL;
    spec->n_servers = 0;
    spec->paths = NULL;
    spec->n_paths = 0;
    spec->webhooks = NULL;
    spec->n_webhooks = 0;
    spec->security_schemes = NULL;
    spec->n_security_schemes = 0;
    spec->defined_schemas = NULL;
    spec->defined_schema_names = NULL;
    spec->n_defined_schemas = 0;
  }
}

static void free_schema_ref_content(struct OpenAPI_SchemaRef *ref) {
  if (!ref)
    return;
  if (ref->ref_name)
    free(ref->ref_name);
  if (ref->inline_type)
    free(ref->inline_type);
  if (ref->content_type)
    free(ref->content_type);
  if (ref->multipart_fields) {
    size_t i;
    for (i = 0; i < ref->n_multipart_fields; ++i) {
      if (ref->multipart_fields[i].name)
        free(ref->multipart_fields[i].name);
      if (ref->multipart_fields[i].type)
        free(ref->multipart_fields[i].type);
    }
    free(ref->multipart_fields);
  }
}

static void free_security_requirement(struct OpenAPI_SecurityRequirement *req) {
  size_t i;
  if (!req)
    return;
  if (req->scheme)
    free(req->scheme);
  if (req->scopes) {
    for (i = 0; i < req->n_scopes; ++i) {
      if (req->scopes[i])
        free(req->scopes[i]);
    }
    free(req->scopes);
  }
}

static void free_security_requirement_set(
    struct OpenAPI_SecurityRequirementSet *set) {
  size_t i;
  if (!set)
    return;
  if (set->requirements) {
    for (i = 0; i < set->n_requirements; ++i) {
      free_security_requirement(&set->requirements[i]);
    }
    free(set->requirements);
  }
}

static void free_servers_array(struct OpenAPI_Server *servers,
                               size_t n_servers) {
  size_t i;
  if (!servers)
    return;
  for (i = 0; i < n_servers; ++i) {
    if (servers[i].url)
      free(servers[i].url);
    if (servers[i].description)
      free(servers[i].description);
    if (servers[i].name)
      free(servers[i].name);
    if (servers[i].variables) {
      size_t v;
      for (v = 0; v < servers[i].n_variables; ++v) {
        struct OpenAPI_ServerVariable *var = &servers[i].variables[v];
        if (var->name)
          free(var->name);
        if (var->default_value)
          free(var->default_value);
        if (var->description)
          free(var->description);
        if (var->enum_values) {
          size_t e;
          for (e = 0; e < var->n_enum_values; ++e) {
            if (var->enum_values[e])
              free(var->enum_values[e]);
          }
          free(var->enum_values);
        }
      }
      free(servers[i].variables);
    }
  }
  free(servers);
}

static void free_operation(struct OpenAPI_Operation *op) {
  size_t i;
  if (!op)
    return;
  if (op->operation_id)
    free(op->operation_id);
  if (op->summary)
    free(op->summary);
  if (op->description)
    free(op->description);

  if (op->tags) {
    for (i = 0; i < op->n_tags; ++i) {
      if (op->tags[i])
        free(op->tags[i]);
    }
    free(op->tags);
  }

  free_schema_ref_content(&op->req_body);
  if (op->req_body_description)
    free(op->req_body_description);
  if (op->external_docs.description)
    free(op->external_docs.description);
  if (op->external_docs.url)
    free(op->external_docs.url);

  if (op->servers) {
    free_servers_array(op->servers, op->n_servers);
    op->servers = NULL;
    op->n_servers = 0;
  }

  if (op->parameters) {
    for (i = 0; i < op->n_parameters; ++i) {
      free(op->parameters[i].name);
      free(op->parameters[i].type);
      if (op->parameters[i].content_type)
        free(op->parameters[i].content_type);
      if (op->parameters[i].description)
        free(op->parameters[i].description);
      if (op->parameters[i].items_type)
        free(op->parameters[i].items_type);
    }
    free(op->parameters);
  }

  if (op->responses) {
    for (i = 0; i < op->n_responses; ++i) {
      free(op->responses[i].code);
      if (op->responses[i].description)
        free(op->responses[i].description);
      if (op->responses[i].content_type)
        free(op->responses[i].content_type);
      free_schema_ref_content(&op->responses[i].schema);
    }
    free(op->responses);
  }

  if (op->security) {
    for (i = 0; i < op->n_security; ++i) {
      free_security_requirement_set(&op->security[i]);
    }
    free(op->security);
    op->security = NULL;
    op->n_security = 0;
    op->security_set = 0;
  }
}

void openapi_spec_free(struct OpenAPI_Spec *const spec) {
  size_t i, j;
  if (!spec)
    return;

  if (spec->openapi_version) {
    free(spec->openapi_version);
    spec->openapi_version = NULL;
  }
  if (spec->self_uri) {
    free(spec->self_uri);
    spec->self_uri = NULL;
  }
  if (spec->json_schema_dialect) {
    free(spec->json_schema_dialect);
    spec->json_schema_dialect = NULL;
  }
  if (spec->info.title)
    free(spec->info.title);
  if (spec->info.summary)
    free(spec->info.summary);
  if (spec->info.description)
    free(spec->info.description);
  if (spec->info.terms_of_service)
    free(spec->info.terms_of_service);
  if (spec->info.version)
    free(spec->info.version);
  if (spec->info.contact.name)
    free(spec->info.contact.name);
  if (spec->info.contact.url)
    free(spec->info.contact.url);
  if (spec->info.contact.email)
    free(spec->info.contact.email);
  if (spec->info.license.name)
    free(spec->info.license.name);
  if (spec->info.license.identifier)
    free(spec->info.license.identifier);
  if (spec->info.license.url)
    free(spec->info.license.url);
  if (spec->external_docs.description)
    free(spec->external_docs.description);
  if (spec->external_docs.url)
    free(spec->external_docs.url);

  if (spec->tags) {
    for (i = 0; i < spec->n_tags; ++i) {
      if (spec->tags[i].name)
        free(spec->tags[i].name);
      if (spec->tags[i].summary)
        free(spec->tags[i].summary);
      if (spec->tags[i].description)
        free(spec->tags[i].description);
      if (spec->tags[i].parent)
        free(spec->tags[i].parent);
      if (spec->tags[i].kind)
        free(spec->tags[i].kind);
      if (spec->tags[i].external_docs.description)
        free(spec->tags[i].external_docs.description);
      if (spec->tags[i].external_docs.url)
        free(spec->tags[i].external_docs.url);
    }
    free(spec->tags);
    spec->tags = NULL;
    spec->n_tags = 0;
  }

  if (spec->security) {
    for (i = 0; i < spec->n_security; ++i) {
      free_security_requirement_set(&spec->security[i]);
    }
    free(spec->security);
    spec->security = NULL;
    spec->n_security = 0;
    spec->security_set = 0;
  }

  if (spec->servers) {
    free_servers_array(spec->servers, spec->n_servers);
    spec->servers = NULL;
    spec->n_servers = 0;
  }

  if (spec->paths) {
    for (i = 0; i < spec->n_paths; ++i) {
      struct OpenAPI_Path *p = &spec->paths[i];
      if (p->route)
        free(p->route);
      if (p->ref)
        free(p->ref);
      if (p->summary)
        free(p->summary);
      if (p->description)
        free(p->description);
      if (p->parameters) {
        size_t k;
        for (k = 0; k < p->n_parameters; ++k) {
          free(p->parameters[k].name);
          free(p->parameters[k].type);
          if (p->parameters[k].content_type)
            free(p->parameters[k].content_type);
          if (p->parameters[k].description)
            free(p->parameters[k].description);
          if (p->parameters[k].items_type)
            free(p->parameters[k].items_type);
        }
        free(p->parameters);
      }
      if (p->servers) {
        free_servers_array(p->servers, p->n_servers);
        p->servers = NULL;
        p->n_servers = 0;
      }
      if (p->operations) {
        for (j = 0; j < p->n_operations; ++j) {
          free_operation(&p->operations[j]);
        }
        free(p->operations);
      }
    }
    free(spec->paths);
    spec->paths = NULL;
  }

  if (spec->webhooks) {
    for (i = 0; i < spec->n_webhooks; ++i) {
      struct OpenAPI_Path *p = &spec->webhooks[i];
      if (p->route)
        free(p->route);
      if (p->ref)
        free(p->ref);
      if (p->summary)
        free(p->summary);
      if (p->description)
        free(p->description);
      if (p->parameters) {
        size_t k;
        for (k = 0; k < p->n_parameters; ++k) {
          free(p->parameters[k].name);
          free(p->parameters[k].type);
          if (p->parameters[k].content_type)
            free(p->parameters[k].content_type);
          if (p->parameters[k].description)
            free(p->parameters[k].description);
          if (p->parameters[k].items_type)
            free(p->parameters[k].items_type);
        }
        free(p->parameters);
      }
      if (p->servers) {
        free_servers_array(p->servers, p->n_servers);
        p->servers = NULL;
        p->n_servers = 0;
      }
      if (p->operations) {
        for (j = 0; j < p->n_operations; ++j) {
          free_operation(&p->operations[j]);
        }
        free(p->operations);
      }
    }
    free(spec->webhooks);
    spec->webhooks = NULL;
  }

  if (spec->security_schemes) {
    for (i = 0; i < spec->n_security_schemes; ++i) {
      if (spec->security_schemes[i].name)
        free(spec->security_schemes[i].name);
      if (spec->security_schemes[i].scheme)
        free(spec->security_schemes[i].scheme);
      if (spec->security_schemes[i].bearer_format)
        free(spec->security_schemes[i].bearer_format);
      if (spec->security_schemes[i].key_name)
        free(spec->security_schemes[i].key_name);
      if (spec->security_schemes[i].open_id_connect_url)
        free(spec->security_schemes[i].open_id_connect_url);
      if (spec->security_schemes[i].oauth2_metadata_url)
        free(spec->security_schemes[i].oauth2_metadata_url);
    }
    free(spec->security_schemes);
    spec->security_schemes = NULL;
  }

  if (spec->defined_schemas) {
    for (i = 0; i < spec->n_defined_schemas; ++i) {
      struct_fields_free(&spec->defined_schemas[i]);
      free(spec->defined_schema_names[i]);
    }
    free(spec->defined_schemas);
    free(spec->defined_schema_names);
    spec->defined_schemas = NULL;
    spec->defined_schema_names = NULL;
  }

  spec->n_paths = 0;
  spec->n_webhooks = 0;
  spec->n_security_schemes = 0;
  spec->n_defined_schemas = 0;
  spec->n_security = 0;
  spec->security_set = 0;
}

/* --- Parsing Helpers --- */

static enum OpenAPI_Verb parse_verb(const char *const v) {
  if (strcmp(v, "get") == 0)
    return OA_VERB_GET;
  if (strcmp(v, "post") == 0)
    return OA_VERB_POST;
  if (strcmp(v, "put") == 0)
    return OA_VERB_PUT;
  if (strcmp(v, "delete") == 0)
    return OA_VERB_DELETE;
  if (strcmp(v, "patch") == 0)
    return OA_VERB_PATCH;
  if (strcmp(v, "head") == 0)
    return OA_VERB_HEAD;
  if (strcmp(v, "options") == 0)
    return OA_VERB_OPTIONS;
  if (strcmp(v, "trace") == 0)
    return OA_VERB_TRACE;
  if (strcmp(v, "query") == 0)
    return OA_VERB_QUERY;
  return OA_VERB_UNKNOWN;
}

static enum OpenAPI_ParamIn parse_param_in(const char *const in) {
  if (strcmp(in, "path") == 0)
    return OA_PARAM_IN_PATH;
  if (strcmp(in, "query") == 0)
    return OA_PARAM_IN_QUERY;
  if (strcmp(in, "querystring") == 0)
    return OA_PARAM_IN_QUERYSTRING;
  if (strcmp(in, "header") == 0)
    return OA_PARAM_IN_HEADER;
  if (strcmp(in, "cookie") == 0)
    return OA_PARAM_IN_COOKIE;
  return OA_PARAM_IN_UNKNOWN;
}

static enum OpenAPI_Style parse_param_style(const char *const s) {
  if (!s)
    return OA_STYLE_UNKNOWN;
  if (strcmp(s, "form") == 0)
    return OA_STYLE_FORM;
  if (strcmp(s, "simple") == 0)
    return OA_STYLE_SIMPLE;
  if (strcmp(s, "matrix") == 0)
    return OA_STYLE_MATRIX;
  if (strcmp(s, "label") == 0)
    return OA_STYLE_LABEL;
  if (strcmp(s, "spaceDelimited") == 0)
    return OA_STYLE_SPACE_DELIMITED;
  if (strcmp(s, "pipeDelimited") == 0)
    return OA_STYLE_PIPE_DELIMITED;
  if (strcmp(s, "deepObject") == 0)
    return OA_STYLE_DEEP_OBJECT;
  if (strcmp(s, "cookie") == 0)
    return OA_STYLE_COOKIE;
  return OA_STYLE_UNKNOWN;
}

static enum OpenAPI_SecurityType parse_security_type(const char *const type) {
  if (!type)
    return OA_SEC_UNKNOWN;
  if (strcmp(type, "apiKey") == 0)
    return OA_SEC_APIKEY;
  if (strcmp(type, "http") == 0)
    return OA_SEC_HTTP;
  if (strcmp(type, "oauth2") == 0)
    return OA_SEC_OAUTH2;
  if (strcmp(type, "openIdConnect") == 0)
    return OA_SEC_OPENID;
  return OA_SEC_UNKNOWN;
}

static enum OpenAPI_SecurityIn parse_security_in(const char *const in) {
  if (!in)
    return OA_SEC_IN_UNKNOWN;
  if (strcmp(in, "query") == 0)
    return OA_SEC_IN_QUERY;
  if (strcmp(in, "header") == 0)
    return OA_SEC_IN_HEADER;
  if (strcmp(in, "cookie") == 0)
    return OA_SEC_IN_COOKIE;
  return OA_SEC_IN_UNKNOWN;
}

static int parse_info(const JSON_Object *const root_obj,
                      struct OpenAPI_Spec *const out) {
  const JSON_Object *info_obj;
  const JSON_Object *contact_obj;
  const JSON_Object *license_obj;
  const char *val;

  if (!root_obj || !out)
    return 0;

  info_obj = json_object_get_object(root_obj, "info");
  if (!info_obj)
    return 0;

  val = json_object_get_string(info_obj, "title");
  if (val) {
    out->info.title = c_cdd_strdup(val);
    if (!out->info.title)
      return ENOMEM;
  }
  val = json_object_get_string(info_obj, "summary");
  if (val) {
    out->info.summary = c_cdd_strdup(val);
    if (!out->info.summary)
      return ENOMEM;
  }
  val = json_object_get_string(info_obj, "description");
  if (val) {
    out->info.description = c_cdd_strdup(val);
    if (!out->info.description)
      return ENOMEM;
  }
  val = json_object_get_string(info_obj, "termsOfService");
  if (val) {
    out->info.terms_of_service = c_cdd_strdup(val);
    if (!out->info.terms_of_service)
      return ENOMEM;
  }
  val = json_object_get_string(info_obj, "version");
  if (val) {
    out->info.version = c_cdd_strdup(val);
    if (!out->info.version)
      return ENOMEM;
  }

  contact_obj = json_object_get_object(info_obj, "contact");
  if (contact_obj) {
    val = json_object_get_string(contact_obj, "name");
    if (val) {
      out->info.contact.name = c_cdd_strdup(val);
      if (!out->info.contact.name)
        return ENOMEM;
    }
    val = json_object_get_string(contact_obj, "url");
    if (val) {
      out->info.contact.url = c_cdd_strdup(val);
      if (!out->info.contact.url)
        return ENOMEM;
    }
    val = json_object_get_string(contact_obj, "email");
    if (val) {
      out->info.contact.email = c_cdd_strdup(val);
      if (!out->info.contact.email)
        return ENOMEM;
    }
  }

  license_obj = json_object_get_object(info_obj, "license");
  if (license_obj) {
    val = json_object_get_string(license_obj, "name");
    if (val) {
      out->info.license.name = c_cdd_strdup(val);
      if (!out->info.license.name)
        return ENOMEM;
    }
    val = json_object_get_string(license_obj, "identifier");
    if (val) {
      out->info.license.identifier = c_cdd_strdup(val);
      if (!out->info.license.identifier)
        return ENOMEM;
    }
    val = json_object_get_string(license_obj, "url");
    if (val) {
      out->info.license.url = c_cdd_strdup(val);
      if (!out->info.license.url)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_external_docs(const JSON_Object *const obj,
                               struct OpenAPI_ExternalDocs *const out) {
  const char *desc;
  const char *url;

  if (!obj || !out)
    return 0;

  desc = json_object_get_string(obj, "description");
  if (desc) {
    out->description = c_cdd_strdup(desc);
    if (!out->description)
      return ENOMEM;
  }
  url = json_object_get_string(obj, "url");
  if (url) {
    out->url = c_cdd_strdup(url);
    if (!out->url)
      return ENOMEM;
  }

  return 0;
}

static int parse_tags(const JSON_Object *const root_obj,
                      struct OpenAPI_Spec *const out) {
  const JSON_Array *tags_arr;
  size_t count, i;

  if (!root_obj || !out)
    return 0;

  tags_arr = json_object_get_array(root_obj, "tags");
  if (!tags_arr)
    return 0;

  count = json_array_get_count(tags_arr);
  if (count == 0)
    return 0;

  out->tags = (struct OpenAPI_Tag *)calloc(count, sizeof(struct OpenAPI_Tag));
  if (!out->tags)
    return ENOMEM;
  out->n_tags = count;

  for (i = 0; i < count; ++i) {
    const JSON_Object *tag_obj = json_array_get_object(tags_arr, i);
    const char *name = json_object_get_string(tag_obj, "name");
    const char *summary = json_object_get_string(tag_obj, "summary");
    const char *description = json_object_get_string(tag_obj, "description");
    const char *parent = json_object_get_string(tag_obj, "parent");
    const char *kind = json_object_get_string(tag_obj, "kind");
    const JSON_Object *ext = json_object_get_object(tag_obj, "externalDocs");

    if (name) {
      out->tags[i].name = c_cdd_strdup(name);
      if (!out->tags[i].name)
        return ENOMEM;
    }
    if (summary) {
      out->tags[i].summary = c_cdd_strdup(summary);
      if (!out->tags[i].summary)
        return ENOMEM;
    }
    if (description) {
      out->tags[i].description = c_cdd_strdup(description);
      if (!out->tags[i].description)
        return ENOMEM;
    }
    if (parent) {
      out->tags[i].parent = c_cdd_strdup(parent);
      if (!out->tags[i].parent)
        return ENOMEM;
    }
    if (kind) {
      out->tags[i].kind = c_cdd_strdup(kind);
      if (!out->tags[i].kind)
        return ENOMEM;
    }
    if (ext) {
      if (parse_external_docs(ext, &out->tags[i].external_docs) != 0)
        return ENOMEM;
    }
  }

  return 0;
}

static int parse_servers_array(const JSON_Object *const parent,
                               const char *const key,
                               struct OpenAPI_Server **out_servers,
                               size_t *out_count) {
  const JSON_Array *servers;
  size_t count, i;

  if (!parent || !key || !out_servers || !out_count)
    return 0;

  *out_servers = NULL;
  *out_count = 0;

  servers = json_object_get_array(parent, key);
  if (!servers)
    return 0;

  count = json_array_get_count(servers);
  if (count == 0)
    return 0;

  *out_servers =
      (struct OpenAPI_Server *)calloc(count, sizeof(struct OpenAPI_Server));
  if (!*out_servers)
    return ENOMEM;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const JSON_Object *srv_obj = json_array_get_object(servers, i);
    const char *url = json_object_get_string(srv_obj, "url");
    const char *desc = json_object_get_string(srv_obj, "description");
    const char *name = json_object_get_string(srv_obj, "name");
    const JSON_Object *vars = json_object_get_object(srv_obj, "variables");

    (*out_servers)[i].url = c_cdd_strdup(url ? url : "/");
    if (!(*out_servers)[i].url)
      return ENOMEM;

    if (desc) {
      (*out_servers)[i].description = c_cdd_strdup(desc);
      if (!(*out_servers)[i].description)
        return ENOMEM;
    }
    if (name) {
      (*out_servers)[i].name = c_cdd_strdup(name);
      if (!(*out_servers)[i].name)
        return ENOMEM;
    }

    if (vars) {
      size_t vcount = json_object_get_count(vars);
      size_t v;
      if (vcount > 0) {
        (*out_servers)[i].variables = (struct OpenAPI_ServerVariable *)calloc(
            vcount, sizeof(struct OpenAPI_ServerVariable));
        if (!(*out_servers)[i].variables)
          return ENOMEM;
        (*out_servers)[i].n_variables = vcount;
        for (v = 0; v < vcount; ++v) {
          const char *vname = json_object_get_name(vars, v);
          const JSON_Object *v_obj =
              json_value_get_object(json_object_get_value_at(vars, v));
          struct OpenAPI_ServerVariable *curr =
              &(*out_servers)[i].variables[v];
          if (vname) {
            curr->name = c_cdd_strdup(vname);
            if (!curr->name)
              return ENOMEM;
          }
          if (v_obj) {
            const char *def_val = json_object_get_string(v_obj, "default");
            const char *v_desc = json_object_get_string(v_obj, "description");
            const JSON_Array *enum_arr = json_object_get_array(v_obj, "enum");
            if (def_val) {
              curr->default_value = c_cdd_strdup(def_val);
              if (!curr->default_value)
                return ENOMEM;
            }
            if (v_desc) {
              curr->description = c_cdd_strdup(v_desc);
              if (!curr->description)
                return ENOMEM;
            }
            if (enum_arr) {
              size_t ecount = json_array_get_count(enum_arr);
              size_t e;
              if (ecount > 0) {
                curr->enum_values =
                    (char **)calloc(ecount, sizeof(char *));
                if (!curr->enum_values)
                  return ENOMEM;
                curr->n_enum_values = ecount;
                for (e = 0; e < ecount; ++e) {
                  const char *e_val = json_array_get_string(enum_arr, e);
                  if (e_val) {
                    curr->enum_values[e] = c_cdd_strdup(e_val);
                    if (!curr->enum_values[e])
                      return ENOMEM;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return 0;
}

static int parse_security_requirements(
    const JSON_Array *const arr,
    struct OpenAPI_SecurityRequirementSet **out,
    size_t *out_count) {
  size_t i, count;
  int rc = 0;

  if (!arr || !out || !out_count)
    return 0;

  count = json_array_get_count(arr);
  if (count == 0) {
    *out = NULL;
    *out_count = 0;
    return 0;
  }

  *out = (struct OpenAPI_SecurityRequirementSet *)calloc(
      count, sizeof(struct OpenAPI_SecurityRequirementSet));
  if (!*out)
    return ENOMEM;

  *out_count = count;

  for (i = 0; i < count; ++i) {
    const JSON_Object *sec_obj = json_array_get_object(arr, i);
    struct OpenAPI_SecurityRequirementSet *set = &(*out)[i];
    size_t j, req_count = 0;

    if (sec_obj) {
      req_count = json_object_get_count(sec_obj);
    }

    if (req_count == 0) {
      set->requirements = NULL;
      set->n_requirements = 0;
      continue;
    }

    set->requirements = (struct OpenAPI_SecurityRequirement *)calloc(
        req_count, sizeof(struct OpenAPI_SecurityRequirement));
    if (!set->requirements) {
      rc = ENOMEM;
      goto fail;
    }
    set->n_requirements = req_count;

    for (j = 0; j < req_count; ++j) {
      const char *scheme = json_object_get_name(sec_obj, j);
      const JSON_Array *scopes_arr =
          json_object_get_array(sec_obj, scheme);
      struct OpenAPI_SecurityRequirement *req = &set->requirements[j];
      size_t k, n_scopes = 0;

      req->scheme = c_cdd_strdup(scheme ? scheme : "");
      if (!req->scheme) {
        rc = ENOMEM;
        goto fail;
      }

      if (scopes_arr)
        n_scopes = json_array_get_count(scopes_arr);

      if (n_scopes == 0) {
        req->scopes = NULL;
        req->n_scopes = 0;
        continue;
      }

      req->scopes = (char **)calloc(n_scopes, sizeof(char *));
      if (!req->scopes) {
        rc = ENOMEM;
        goto fail;
      }
      req->n_scopes = n_scopes;

      for (k = 0; k < n_scopes; ++k) {
        const char *scope = json_array_get_string(scopes_arr, k);
        req->scopes[k] = c_cdd_strdup(scope ? scope : "");
        if (!req->scopes[k]) {
          rc = ENOMEM;
          goto fail;
        }
      }
    }
  }

  return 0;

fail:
  if (*out) {
    for (i = 0; i < count; ++i) {
      free_security_requirement_set(&(*out)[i]);
    }
    free(*out);
    *out = NULL;
  }
  *out_count = 0;
  return rc;
}

static int parse_security_field(const JSON_Object *const obj,
                                const char *const key,
                                struct OpenAPI_SecurityRequirementSet **out,
                                size_t *out_count, int *out_set) {
  const JSON_Array *arr;
  if (!obj || !key || !out || !out_count || !out_set)
    return 0;

  if (!json_object_has_value(obj, key)) {
    *out_set = 0;
    return 0;
  }

  *out_set = 1;
  arr = json_object_get_array(obj, key);
  if (!arr) {
    *out = NULL;
    *out_count = 0;
    return 0;
  }

  return parse_security_requirements(arr, out, out_count);
}

static char *clean_ref(const char *full_ref) {
  const char *slash;
  if (!full_ref)
    return NULL;
  slash = strrchr(full_ref, '/');
  if (slash) {
    return c_cdd_strdup(slash + 1);
  }
  return c_cdd_strdup(full_ref);
}

static int parse_schema_ref(const JSON_Object *const schema,
                            struct OpenAPI_SchemaRef *const out) {
  const char *ref = json_object_get_string(schema, "$ref");
  const char *type = json_object_get_string(schema, "type");

  if (!out)
    return EINVAL;

  out->is_array = 0;
  out->ref_name = NULL;
  out->inline_type = NULL;
  out->content_type = NULL;
  out->multipart_fields = NULL;
  out->n_multipart_fields = 0;

  if (ref) {
    out->ref_name = clean_ref(ref);
    return out->ref_name ? 0 : ENOMEM;
  }

  if (type && strcmp(type, "array") == 0) {
    out->is_array = 1;
    const JSON_Object *items = json_object_get_object(schema, "items");
    if (items) {
      const char *item_ref = json_object_get_string(items, "$ref");
      const char *item_type = json_object_get_string(items, "type");
      if (item_ref) {
        out->ref_name = clean_ref(item_ref);
        return out->ref_name ? 0 : ENOMEM;
      }
      if (item_type) {
        out->inline_type = c_cdd_strdup(item_type);
        return out->inline_type ? 0 : ENOMEM;
      }
    }
    return 0;
  }

  if (type) {
    out->inline_type = c_cdd_strdup(type);
    return out->inline_type ? 0 : ENOMEM;
  }

  return 0;
}

static int parse_servers(const JSON_Object *const root_obj,
                         struct OpenAPI_Spec *const out) {
  return parse_servers_array(root_obj, "servers", &out->servers,
                             &out->n_servers);
}

static int parse_security_schemes(const JSON_Object *const components,
                                  struct OpenAPI_Spec *const out) {
  const JSON_Object *schemes;
  size_t count, i;

  if (!components || !out)
    return 0;

  schemes = json_object_get_object(components, "securitySchemes");
  if (!schemes)
    return 0;

  count = json_object_get_count(schemes);
  if (count == 0)
    return 0;

  out->security_schemes = (struct OpenAPI_SecurityScheme *)calloc(
      count, sizeof(struct OpenAPI_SecurityScheme));
  if (!out->security_schemes)
    return ENOMEM;
  out->n_security_schemes = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(schemes, i);
    const JSON_Object *sec_obj =
        json_value_get_object(json_object_get_value_at(schemes, i));
    out->security_schemes[i].name = c_cdd_strdup(name ? name : "");
    if (!out->security_schemes[i].name)
      return ENOMEM;
    out->security_schemes[i].type = OA_SEC_UNKNOWN;

    if (!sec_obj)
      continue;

    {
      const char *type = json_object_get_string(sec_obj, "type");
      out->security_schemes[i].type = parse_security_type(type);
    }

    if (out->security_schemes[i].type == OA_SEC_APIKEY) {
      const char *in = json_object_get_string(sec_obj, "in");
      const char *key_name = json_object_get_string(sec_obj, "name");
      out->security_schemes[i].in = parse_security_in(in);
      if (key_name) {
        out->security_schemes[i].key_name = c_cdd_strdup(key_name);
        if (!out->security_schemes[i].key_name)
          return ENOMEM;
      }
    } else if (out->security_schemes[i].type == OA_SEC_HTTP) {
      const char *scheme = json_object_get_string(sec_obj, "scheme");
      const char *bearer_format =
          json_object_get_string(sec_obj, "bearerFormat");
      if (scheme) {
        out->security_schemes[i].scheme = c_cdd_strdup(scheme);
        if (!out->security_schemes[i].scheme)
          return ENOMEM;
      }
      if (bearer_format) {
        out->security_schemes[i].bearer_format =
            c_cdd_strdup(bearer_format);
        if (!out->security_schemes[i].bearer_format)
          return ENOMEM;
      }
    } else if (out->security_schemes[i].type == OA_SEC_OPENID) {
      const char *oid_url =
          json_object_get_string(sec_obj, "openIdConnectUrl");
      if (oid_url) {
        out->security_schemes[i].open_id_connect_url = c_cdd_strdup(oid_url);
        if (!out->security_schemes[i].open_id_connect_url)
          return ENOMEM;
      }
    } else if (out->security_schemes[i].type == OA_SEC_OAUTH2) {
      const char *meta_url =
          json_object_get_string(sec_obj, "oauth2MetadataUrl");
      if (meta_url) {
        out->security_schemes[i].oauth2_metadata_url = c_cdd_strdup(meta_url);
        if (!out->security_schemes[i].oauth2_metadata_url)
          return ENOMEM;
      }
    }
  }

  return 0;
}

static int parse_parameters_array(const JSON_Array *const arr,
                                  struct OpenAPI_Parameter **out_params,
                                  size_t *out_count) {
  size_t i, count;
  if (!out_params || !out_count)
    return 0;

  *out_params = NULL;
  *out_count = 0;

  if (!arr)
    return 0;

  count = json_array_get_count(arr);
  if (count == 0)
    return 0;

  *out_params =
      (struct OpenAPI_Parameter *)calloc(count, sizeof(struct OpenAPI_Parameter));
  if (!*out_params)
    return ENOMEM;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    struct OpenAPI_Parameter *curr = &(*out_params)[i];
    const JSON_Object *p_obj = json_array_get_object(arr, i);
    const char *name = json_object_get_string(p_obj, "name");
    const char *in = json_object_get_string(p_obj, "in");
    const char *desc = json_object_get_string(p_obj, "description");
    int req = json_object_get_boolean(p_obj, "required");
    int deprecated_present = json_object_has_value(p_obj, "deprecated");
    int deprecated_val = json_object_get_boolean(p_obj, "deprecated");
    int allow_reserved_present = json_object_has_value(p_obj, "allowReserved");
    int allow_reserved_val = json_object_get_boolean(p_obj, "allowReserved");
    int allow_empty_present = json_object_has_value(p_obj, "allowEmptyValue");
    int allow_empty_val = json_object_get_boolean(p_obj, "allowEmptyValue");
    const JSON_Object *schema = json_object_get_object(p_obj, "schema");
    const JSON_Object *content = json_object_get_object(p_obj, "content");
    const JSON_Object *effective_schema = schema;
    const JSON_Object *media_obj = NULL;
    const char *media_type = NULL;
    const char *type = NULL;
    const char *style_str = json_object_get_string(p_obj, "style");
    int explode_present = json_object_has_value(p_obj, "explode");
    int explode_val = json_object_get_boolean(p_obj, "explode");

    curr->name = c_cdd_strdup(name ? name : "");
    curr->in = in ? parse_param_in(in) : OA_PARAM_IN_UNKNOWN;
    curr->required = (req == 1);

    if (content) {
      if (curr->in == OA_PARAM_IN_QUERYSTRING) {
        media_obj =
            json_object_get_object(content, "application/x-www-form-urlencoded");
        if (media_obj) {
          media_type = "application/x-www-form-urlencoded";
        }
      }
      if (!media_obj) {
        size_t ccount = json_object_get_count(content);
        if (ccount > 0) {
          media_type = json_object_get_name(content, 0);
          if (media_type) {
            media_obj = json_object_get_object(content, media_type);
          }
        }
      }
      if (media_type) {
        curr->content_type = c_cdd_strdup(media_type);
      }
      if (media_obj) {
        effective_schema = json_object_get_object(media_obj, "schema");
      }
    }

    if (effective_schema)
      type = json_object_get_string(effective_schema, "type");

    if (curr->in == OA_PARAM_IN_QUERYSTRING) {
      curr->type = c_cdd_strdup(type ? type : "string");
    } else {
      curr->type = c_cdd_strdup(type ? type : "string");
    }

    if (desc) {
      curr->description = c_cdd_strdup(desc);
      if (!curr->description)
        return ENOMEM;
    }
    if (deprecated_present) {
      curr->deprecated_set = 1;
      curr->deprecated = (deprecated_val == 1);
    }
    if (allow_reserved_present) {
      curr->allow_reserved_set = 1;
      curr->allow_reserved = (allow_reserved_val == 1);
    }
    if (allow_empty_present) {
      curr->allow_empty_value_set = 1;
      curr->allow_empty_value = (allow_empty_val == 1);
    }

    /* Handle Arrays */
    if (type && strcmp(type, "array") == 0 &&
        curr->in != OA_PARAM_IN_QUERYSTRING) {
      const JSON_Object *items =
          effective_schema ? json_object_get_object(effective_schema, "items")
                           : NULL;
      curr->is_array = 1;
      if (items) {
        const char *i_type = json_object_get_string(items, "type");
        if (i_type) {
          curr->items_type = c_cdd_strdup(i_type);
        }
      }
    } else {
      curr->is_array = 0;
      curr->items_type = NULL;
    }

    /* Handle Style / Explode */
    if (style_str) {
      curr->style = parse_param_style(style_str);
    } else {
      /* Defaults */
      if (curr->in == OA_PARAM_IN_QUERY)
        curr->style = OA_STYLE_FORM;
      else if (curr->in == OA_PARAM_IN_PATH)
        curr->style = OA_STYLE_SIMPLE;
      else if (curr->in == OA_PARAM_IN_COOKIE)
        curr->style = OA_STYLE_FORM;
      else
        curr->style = OA_STYLE_SIMPLE;
    }

    if (explode_present) {
      curr->explode = explode_val;
    } else {
      /* Defaults based on style */
      if (curr->style == OA_STYLE_FORM)
        curr->explode = 1;
      else
        curr->explode = 0;
    }
  }
  return 0;
}

static int parse_responses(const JSON_Object *responses,
                           struct OpenAPI_Operation *out_op) {
  size_t i, count;
  if (!responses || !out_op)
    return 0;

  count = json_object_get_count(responses);
  if (count == 0)
    return 0;

  out_op->responses =
      (struct OpenAPI_Response *)calloc(count, sizeof(struct OpenAPI_Response));
  if (!out_op->responses)
    return ENOMEM;
  out_op->n_responses = count;

  for (i = 0; i < count; ++i) {
    const char *code = json_object_get_name(responses, i);
    const JSON_Value *val = json_object_get_value_at(responses, i);
    const JSON_Object *resp_obj = json_value_get_object(val);
    struct OpenAPI_Response *curr = &out_op->responses[i];

    curr->code = c_cdd_strdup(code);
    if (!curr->code)
      return ENOMEM;

    if (resp_obj) {
      const char *desc = json_object_get_string(resp_obj, "description");
      if (desc) {
        curr->description = c_cdd_strdup(desc);
        if (!curr->description)
          return ENOMEM;
      }
      const JSON_Object *content = json_object_get_object(resp_obj, "content");
      if (content) {
        const JSON_Object *media = NULL;
        const char *media_type = NULL;
        if ((media = json_object_get_object(content, "application/json"))) {
          media_type = "application/json";
        } else {
          size_t ccount = json_object_get_count(content);
          if (ccount > 0) {
            media_type = json_object_get_name(content, 0);
            if (media_type) {
              media = json_object_get_object(content, media_type);
            }
          }
        }
        if (media_type) {
          curr->content_type = c_cdd_strdup(media_type);
          if (!curr->content_type)
            return ENOMEM;
        }
        if (media) {
          const JSON_Object *schema = json_object_get_object(media, "schema");
          if (schema) {
            parse_schema_ref(schema, &curr->schema);
          }
        }
      }
    }
  }
  return 0;
}

static int parse_operation(const char *const verb_str,
                           const JSON_Object *const op_obj,
                           struct OpenAPI_Operation *const out_op) {
  const char *op_id;
  const JSON_Array *params;
  const JSON_Array *tags;
  const JSON_Object *req_body, *responses;
  const char *summary;
  const char *description;
  const JSON_Object *ext_docs;
  int deprecated_present;
  int deprecated_val;

  if (!verb_str || !op_obj || !out_op)
    return EINVAL;

  out_op->verb = parse_verb(verb_str);
  if (out_op->verb == OA_VERB_UNKNOWN)
    return 0;

  op_id = json_object_get_string(op_obj, "operationId");
  out_op->operation_id = c_cdd_strdup(op_id ? op_id : "unnamed");
  summary = json_object_get_string(op_obj, "summary");
  if (summary) {
    out_op->summary = c_cdd_strdup(summary);
    if (!out_op->summary)
      return ENOMEM;
  }
  description = json_object_get_string(op_obj, "description");
  if (description) {
    out_op->description = c_cdd_strdup(description);
    if (!out_op->description)
      return ENOMEM;
  }
  ext_docs = json_object_get_object(op_obj, "externalDocs");
  if (ext_docs) {
    if (parse_external_docs(ext_docs, &out_op->external_docs) != 0)
      return ENOMEM;
  }
  deprecated_present = json_object_has_value(op_obj, "deprecated");
  deprecated_val = json_object_get_boolean(op_obj, "deprecated");
  if (deprecated_present)
    out_op->deprecated = (deprecated_val == 1);
  if (parse_security_field(op_obj, "security", &out_op->security,
                           &out_op->n_security, &out_op->security_set) != 0)
    return ENOMEM;

  /* 1. Parameters */
  params = json_object_get_array(op_obj, "parameters");
  if (parse_parameters_array(params, &out_op->parameters,
                             &out_op->n_parameters) != 0)
    return ENOMEM;

  /* 2. Request Body */
  req_body = json_object_get_object(op_obj, "requestBody");
  if (req_body) {
    const char *rb_desc = json_object_get_string(req_body, "description");
    int rb_required_present = json_object_has_value(req_body, "required");
    int rb_required = json_object_get_boolean(req_body, "required");
    const JSON_Object *content = json_object_get_object(req_body, "content");
    /* Priority: JSON -> Form -> Multipart */
    const JSON_Object *media_obj = NULL;
    const char *detected_type = NULL;

    if (rb_desc) {
      out_op->req_body_description = c_cdd_strdup(rb_desc);
      if (!out_op->req_body_description)
        return ENOMEM;
    }
    if (rb_required_present) {
      out_op->req_body_required_set = 1;
      out_op->req_body_required = (rb_required == 1);
    }

    if (content) {
      if ((media_obj = json_object_get_object(content, "application/json"))) {
        detected_type = "application/json";
      } else if ((media_obj = json_object_get_object(
                      content, "application/x-www-form-urlencoded"))) {
        detected_type = "application/x-www-form-urlencoded";
      } else if ((media_obj =
                      json_object_get_object(content, "multipart/form-data"))) {
        detected_type = "multipart/form-data";
      } else {
        size_t ccount = json_object_get_count(content);
        if (ccount > 0) {
          detected_type = json_object_get_name(content, 0);
          if (detected_type) {
            media_obj = json_object_get_object(content, detected_type);
          }
        }
      }
    }

    if (media_obj) {
      const JSON_Object *schema = json_object_get_object(media_obj, "schema");
      if (schema) {
        parse_schema_ref(schema, &out_op->req_body);
        if (detected_type) {
          out_op->req_body.content_type = c_cdd_strdup(detected_type);
        }
      }
    }
  }

  /* 3. Responses */
  responses = json_object_get_object(op_obj, "responses");
  if (parse_responses(responses, out_op) != 0)
    return ENOMEM;

  /* 4. Tags */
  tags = json_object_get_array(op_obj, "tags");
  if (tags) {
    size_t t_count = json_array_get_count(tags);
    out_op->n_tags = t_count;
    if (t_count > 0) {
      size_t k;
      out_op->tags = (char **)calloc(t_count, sizeof(char *));
      if (!out_op->tags)
        return ENOMEM;
      for (k = 0; k < t_count; ++k) {
        const char *t_val = json_array_get_string(tags, k);
        out_op->tags[k] = c_cdd_strdup(t_val ? t_val : "");
        if (!out_op->tags[k])
          return ENOMEM;
      }
    }
  }

  if (parse_servers_array(op_obj, "servers", &out_op->servers,
                           &out_op->n_servers) != 0)
    return ENOMEM;

  return 0;
}

static int parse_components(const JSON_Object *components,
                            struct OpenAPI_Spec *out) {
  const JSON_Object *schemas;
  size_t i, count;

  if (!components || !out)
    return 0;

  if (parse_security_schemes(components, out) != 0)
    return ENOMEM;

  schemas = json_object_get_object(components, "schemas");
  if (!schemas)
    return 0;

  count = json_object_get_count(schemas);
  if (count == 0)
    return 0;

  out->defined_schemas =
      (struct StructFields *)calloc(count, sizeof(struct StructFields));
  out->defined_schema_names = (char **)calloc(count, sizeof(char *));
  if (!out->defined_schemas || !out->defined_schema_names)
    return ENOMEM;

  out->n_defined_schemas = count;

  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(schemas, i);
    const JSON_Object *schema_obj =
        json_value_get_object(json_object_get_value_at(schemas, i));

    out->defined_schema_names[i] = c_cdd_strdup(name);
    struct_fields_init(&out->defined_schemas[i]);

    if (json_object_to_struct_fields(schema_obj, &out->defined_schemas[i],
                                     schemas) != 0) {
      /* If partial failure, we might continue, but for safety lets return error
       */
      return ENOMEM;
    }
  }
  return 0;
}

static int parse_paths_object(const JSON_Object *const paths_obj,
                              struct OpenAPI_Path **out_paths,
                              size_t *out_count) {
  size_t i, n_paths;

  if (!paths_obj || !out_paths || !out_count)
    return 0;

  n_paths = json_object_get_count(paths_obj);
  if (n_paths == 0) {
    *out_paths = NULL;
    *out_count = 0;
    return 0;
  }

  *out_paths = (struct OpenAPI_Path *)calloc(n_paths, sizeof(struct OpenAPI_Path));
  if (!*out_paths)
    return ENOMEM;
  *out_count = n_paths;

  for (i = 0; i < n_paths; ++i) {
    const char *route = json_object_get_name(paths_obj, i);
    const JSON_Value *p_val = json_object_get_value_at(paths_obj, i);
    const JSON_Object *p_obj = json_value_get_object(p_val);
    struct OpenAPI_Path *curr_path = &(*out_paths)[i];
    size_t n_ops_in_obj = p_obj ? json_object_get_count(p_obj) : 0;
    size_t k, valid_ops = 0;

    if (route) {
      curr_path->route = c_cdd_strdup(route);
      if (!curr_path->route)
        return ENOMEM;
    }

    if (p_obj) {
      const char *path_ref = json_object_get_string(p_obj, "$ref");
      const char *path_summary = json_object_get_string(p_obj, "summary");
      const char *path_description = json_object_get_string(p_obj, "description");
      const JSON_Array *path_params = json_object_get_array(p_obj, "parameters");

      if (path_ref) {
        curr_path->ref = c_cdd_strdup(path_ref);
        if (!curr_path->ref)
          return ENOMEM;
      }
      if (path_summary) {
        curr_path->summary = c_cdd_strdup(path_summary);
        if (!curr_path->summary)
          return ENOMEM;
      }
      if (path_description) {
        curr_path->description = c_cdd_strdup(path_description);
        if (!curr_path->description)
          return ENOMEM;
      }
      if (parse_parameters_array(path_params, &curr_path->parameters,
                                 &curr_path->n_parameters) != 0) {
        return ENOMEM;
      }
      if (parse_servers_array(p_obj, "servers", &curr_path->servers,
                              &curr_path->n_servers) != 0) {
        return ENOMEM;
      }
    }

    if (n_ops_in_obj > 0) {
      curr_path->operations = (struct OpenAPI_Operation *)calloc(
          n_ops_in_obj, sizeof(struct OpenAPI_Operation));

      if (!curr_path->operations) {
        return ENOMEM;
      }

      if (p_obj) {
        for (k = 0; k < n_ops_in_obj; ++k) {
          const char *verb = json_object_get_name(p_obj, k);
          const JSON_Object *op_obj =
              json_value_get_object(json_object_get_value_at(p_obj, k));

          if (parse_operation(verb, op_obj,
                              &curr_path->operations[valid_ops]) == 0) {
            if (curr_path->operations[valid_ops].verb != OA_VERB_UNKNOWN) {
              valid_ops++;
            }
          }
        }
      }
      curr_path->n_operations = valid_ops;
    }
  }

  return 0;
}

int openapi_load_from_json(const JSON_Value *const root,
                           struct OpenAPI_Spec *const out) {
  const JSON_Object *root_obj;
  const JSON_Object *paths_obj;
  const JSON_Object *webhooks_obj;
  const JSON_Object *comps_obj;
  int rc;

  if (!root || !out)
    return EINVAL;

  root_obj = json_value_get_object(root);
  if (!root_obj)
    return EINVAL;

  {
    const char *version = json_object_get_string(root_obj, "openapi");
    if (version) {
      out->openapi_version = c_cdd_strdup(version);
      if (!out->openapi_version)
        return ENOMEM;
    }
  }
  {
    const char *self_uri = json_object_get_string(root_obj, "$self");
    if (self_uri) {
      out->self_uri = c_cdd_strdup(self_uri);
      if (!out->self_uri)
        return ENOMEM;
    }
  }
  {
    const char *dialect = json_object_get_string(root_obj, "jsonSchemaDialect");
    if (dialect) {
      out->json_schema_dialect = c_cdd_strdup(dialect);
      if (!out->json_schema_dialect)
        return ENOMEM;
    }
  }

  if (parse_info(root_obj, out) != 0) {
    openapi_spec_free(out);
    return ENOMEM;
  }
  {
    const JSON_Object *ext_docs = json_object_get_object(root_obj, "externalDocs");
    if (ext_docs) {
      if (parse_external_docs(ext_docs, &out->external_docs) != 0) {
        openapi_spec_free(out);
        return ENOMEM;
      }
    }
  }
  if (parse_tags(root_obj, out) != 0) {
    openapi_spec_free(out);
    return ENOMEM;
  }

  if (parse_security_field(root_obj, "security", &out->security,
                           &out->n_security, &out->security_set) != 0) {
    openapi_spec_free(out);
    return ENOMEM;
  }

  if (parse_servers(root_obj, out) != 0) {
    openapi_spec_free(out);
    return ENOMEM;
  }

  paths_obj = json_object_get_object(root_obj, "paths");
  webhooks_obj = json_object_get_object(root_obj, "webhooks");
  comps_obj = json_object_get_object(root_obj, "components");

  /* Load Schemas First */
  if (comps_obj) {
    if (parse_components(comps_obj, out) != 0) {
      openapi_spec_free(out);
      return ENOMEM;
    }
  }

  if (paths_obj) {
    rc = parse_paths_object(paths_obj, &out->paths, &out->n_paths);
    if (rc != 0) {
      openapi_spec_free(out);
      return rc;
    }
  }

  if (webhooks_obj) {
    rc = parse_paths_object(webhooks_obj, &out->webhooks, &out->n_webhooks);
    if (rc != 0) {
      openapi_spec_free(out);
      return rc;
    }
  }

  return 0;
}

const struct StructFields *
openapi_spec_find_schema(const struct OpenAPI_Spec *spec, const char *name) {
  size_t i;
  if (!spec || !name)
    return NULL;
  for (i = 0; i < spec->n_defined_schemas; ++i) {
    if (spec->defined_schema_names[i] &&
        strcmp(spec->defined_schema_names[i], name) == 0) {
      return &spec->defined_schemas[i];
    }
  }
  return NULL;
}
