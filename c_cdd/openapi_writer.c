/**
 * @file openapi_writer.c
 * @brief Implementation of OpenAPI serialization logic (Advanced).
 *
 * Handles deep object serialization, parameter styles, security schemes,
 * servers, and multipart schema construction.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#include "code2schema.h"
#include "openapi_writer.h"
#include "str_utils.h"

/* --- Helper Prototypes --- */

static const char *verb_to_str(enum OpenAPI_Verb v);
static const char *param_in_to_str(enum OpenAPI_ParamIn in);
static const char *style_to_str(enum OpenAPI_Style s);
static int is_schema_primitive(const char *type);
static void write_external_docs(JSON_Object *parent, const char *key,
                                const struct OpenAPI_ExternalDocs *docs);
static void write_info(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);
static void write_schema_ref(JSON_Object *parent, const char *key,
                             const struct OpenAPI_SchemaRef *ref);
static void write_multipart_schema(JSON_Object *parent, const char *key,
                                   const struct OpenAPI_SchemaRef *ref);
static int write_parameters(JSON_Object *parent,
                            const struct OpenAPI_Parameter *params,
                            size_t n_params);
static int write_responses(JSON_Object *op_obj,
                           const struct OpenAPI_Operation *op);
static int write_request_body(JSON_Object *op_obj,
                              const struct OpenAPI_Operation *op);
static int write_operations(JSON_Object *path_item,
                            const struct OpenAPI_Path *path);
static int write_paths(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);
static int write_servers(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);
static int write_server_array(JSON_Object *parent, const char *key,
                              const struct OpenAPI_Server *servers,
                              size_t n_servers);
static int write_security_requirements(
    JSON_Object *parent, const char *key,
    const struct OpenAPI_SecurityRequirementSet *sets, size_t count,
    int set_flag);
static int write_security_schemes(JSON_Object *components,
                                  const struct OpenAPI_Spec *spec);
static int write_components(JSON_Object *root_obj,
                            const struct OpenAPI_Spec *spec);
static int write_tags(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);
static int write_webhooks(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);

/* --- Implementations --- */

static const char *verb_to_str(enum OpenAPI_Verb v) {
  switch (v) {
  case OA_VERB_GET:
    return "get";
  case OA_VERB_POST:
    return "post";
  case OA_VERB_PUT:
    return "put";
  case OA_VERB_DELETE:
    return "delete";
  case OA_VERB_PATCH:
    return "patch";
  case OA_VERB_HEAD:
    return "head";
  case OA_VERB_OPTIONS:
    return "options";
  case OA_VERB_TRACE:
    return "trace";
  case OA_VERB_QUERY:
    return "query";
  default:
    return NULL;
  }
}

static const char *param_in_to_str(enum OpenAPI_ParamIn in) {
  switch (in) {
  case OA_PARAM_IN_PATH:
    return "path";
  case OA_PARAM_IN_QUERY:
    return "query";
  case OA_PARAM_IN_QUERYSTRING:
    return "querystring";
  case OA_PARAM_IN_HEADER:
    return "header";
  case OA_PARAM_IN_COOKIE:
    return "cookie";
  default:
    return NULL;
  }
}

static const char *style_to_str(enum OpenAPI_Style s) {
  switch (s) {
  case OA_STYLE_FORM:
    return "form";
  case OA_STYLE_SIMPLE:
    return "simple";
  case OA_STYLE_MATRIX:
    return "matrix";
  case OA_STYLE_LABEL:
    return "label";
  case OA_STYLE_SPACE_DELIMITED:
    return "spaceDelimited";
  case OA_STYLE_PIPE_DELIMITED:
    return "pipeDelimited";
  case OA_STYLE_DEEP_OBJECT:
    return "deepObject";
  case OA_STYLE_COOKIE:
    return "cookie";
  default:
    return NULL;
  }
}

static int is_schema_primitive(const char *type) {
  if (!type)
    return 0;
  return strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
         strcmp(type, "boolean") == 0 || strcmp(type, "number") == 0 ||
         strcmp(type, "object") == 0 || strcmp(type, "null") == 0;
}

static void write_external_docs(JSON_Object *parent, const char *key,
                                const struct OpenAPI_ExternalDocs *docs) {
  JSON_Value *ext_val;
  JSON_Object *ext_obj;

  if (!parent || !docs || !docs->url)
    return;

  ext_val = json_value_init_object();
  ext_obj = json_value_get_object(ext_val);

  json_object_set_string(ext_obj, "url", docs->url);
  if (docs->description)
    json_object_set_string(ext_obj, "description", docs->description);

  json_object_set_value(parent, key, ext_val);
}

static void write_info(JSON_Object *root_obj, const struct OpenAPI_Spec *spec) {
  JSON_Value *info_val = json_value_init_object();
  JSON_Object *info_obj = json_value_get_object(info_val);
  JSON_Value *contact_val;
  JSON_Object *contact_obj;
  JSON_Value *license_val;
  JSON_Object *license_obj;
  const char *title = spec->info.title ? spec->info.title
                                       : "Generated Specification";
  const char *version = spec->info.version ? spec->info.version : "1.0.0";

  json_object_set_string(info_obj, "title", title);
  json_object_set_string(info_obj, "version", version);
  if (spec->info.summary)
    json_object_set_string(info_obj, "summary", spec->info.summary);
  if (spec->info.description)
    json_object_set_string(info_obj, "description", spec->info.description);
  if (spec->info.terms_of_service)
    json_object_set_string(info_obj, "termsOfService",
                           spec->info.terms_of_service);

  if (spec->info.contact.name || spec->info.contact.url ||
      spec->info.contact.email) {
    contact_val = json_value_init_object();
    contact_obj = json_value_get_object(contact_val);
    if (spec->info.contact.name)
      json_object_set_string(contact_obj, "name", spec->info.contact.name);
    if (spec->info.contact.url)
      json_object_set_string(contact_obj, "url", spec->info.contact.url);
    if (spec->info.contact.email)
      json_object_set_string(contact_obj, "email", spec->info.contact.email);
    json_object_set_value(info_obj, "contact", contact_val);
  }

  if (spec->info.license.name || spec->info.license.identifier ||
      spec->info.license.url) {
    license_val = json_value_init_object();
    license_obj = json_value_get_object(license_val);
    if (spec->info.license.name) {
      json_object_set_string(license_obj, "name", spec->info.license.name);
    } else {
      json_object_set_string(license_obj, "name", "Unknown");
    }
    if (spec->info.license.identifier)
      json_object_set_string(license_obj, "identifier",
                             spec->info.license.identifier);
    if (spec->info.license.url)
      json_object_set_string(license_obj, "url", spec->info.license.url);
    json_object_set_value(info_obj, "license", license_val);
  }

  json_object_set_value(root_obj, "info", info_val);
}

/**
 * @brief Construct an inline schema definition for Multipart fields.
 */
static void write_multipart_schema(JSON_Object *parent, const char *key,
                                   const struct OpenAPI_SchemaRef *ref) {
  JSON_Value *sch_val = json_value_init_object();
  JSON_Object *sch_obj = json_value_get_object(sch_val);
  JSON_Value *props_val = json_value_init_object();
  JSON_Object *props_obj = json_value_get_object(props_val);
  size_t i;

  json_object_set_string(sch_obj, "type", "object");

  for (i = 0; i < ref->n_multipart_fields; ++i) {
    const struct OpenAPI_MultipartField *f = &ref->multipart_fields[i];
    JSON_Value *prop_val = json_value_init_object();
    JSON_Object *prop_obj = json_value_get_object(prop_val);

    if (f->is_binary) {
      json_object_set_string(prop_obj, "type", "string");
      json_object_set_string(prop_obj, "format", "binary");
    } else if (f->type) {
      json_object_set_string(prop_obj, "type", f->type);
    } else {
      /* Default to string */
      json_object_set_string(prop_obj, "type", "string");
    }
    json_object_set_value(props_obj, f->name ? f->name : "unknown", prop_val);
  }

  json_object_set_value(sch_obj, "properties", props_val);
  json_object_set_value(parent, key, sch_val);
}

/**
 * @brief Write a Schema Reference object (or inline Type).
 *
 * Handles `$ref`, `type: array`, and basic types.
 * Populates `parent` at `key` (e.g. key="schema").
 */
static void write_schema_ref(JSON_Object *parent, const char *key,
                             const struct OpenAPI_SchemaRef *ref) {
  JSON_Value *sch_val = json_value_init_object();
  JSON_Object *sch_obj = json_value_get_object(sch_val);
  char ref_path[256];

  /* Case 1: Built-in Multipart Fields (Inline Schema) */
  if (ref->n_multipart_fields > 0) {
    json_value_free(sch_val); /* Discard placeholder */
    write_multipart_schema(parent, key, ref);
    return;
  }

  /* Case 2: Array */
  if (ref->is_array) {
    json_object_set_string(sch_obj, "type", "array");
    if (ref->inline_type) {
      JSON_Value *item_val = json_value_init_object();
      json_object_set_string(json_value_get_object(item_val), "type",
                             ref->inline_type);
      json_object_set_value(sch_obj, "items", item_val);
    } else if (ref->ref_name) {
      /* Simple detection: if standard type, use type, else ref */
      if (is_schema_primitive(ref->ref_name)) {
        JSON_Value *item_val = json_value_init_object();
        json_object_set_string(json_value_get_object(item_val), "type",
                               ref->ref_name);
        json_object_set_value(sch_obj, "items", item_val);
      } else {
        JSON_Value *item_val = json_value_init_object();
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(ref_path, sizeof(ref_path), "#/components/schemas/%s",
                  ref->ref_name);
#else
        sprintf(ref_path, "#/components/schemas/%s", ref->ref_name);
#endif
        json_object_set_string(json_value_get_object(item_val), "$ref",
                               ref_path);
        json_object_set_value(sch_obj, "items", item_val);
      }
    }
  }
  /* Case 3: Reference or Primitive */
  else if (ref->inline_type) {
    json_object_set_string(sch_obj, "type", ref->inline_type);
  } else if (ref->ref_name) {
    if (is_schema_primitive(ref->ref_name)) {
      json_object_set_string(sch_obj, "type", ref->ref_name);
    } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(ref_path, sizeof(ref_path), "#/components/schemas/%s",
                ref->ref_name);
#else
      sprintf(ref_path, "#/components/schemas/%s", ref->ref_name);
#endif
      json_object_set_string(sch_obj, "$ref", ref_path);
    }
  }

  json_object_set_value(parent, key, sch_val);
}

static int write_parameters(JSON_Object *parent,
                            const struct OpenAPI_Parameter *params,
                            size_t n_params) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (!parent || !params || n_params == 0)
    return 0;

  arr_val = json_value_init_array();
  if (arr_val == NULL)
    return ENOMEM;
  arr = json_value_get_array(arr_val);

  for (i = 0; i < n_params; ++i) {
    const struct OpenAPI_Parameter *p = &params[i];
    JSON_Value *p_val = json_value_init_object();
    JSON_Object *p_obj = json_value_get_object(p_val);
    const char *in_str = param_in_to_str(p->in);
    const char *style_str = style_to_str(p->style);

    if (p->name)
      json_object_set_string(p_obj, "name", p->name);
    if (in_str)
      json_object_set_string(p_obj, "in", in_str);
    if (p->required)
      json_object_set_boolean(p_obj, "required", 1);
    if (p->description)
      json_object_set_string(p_obj, "description", p->description);
    if (p->deprecated_set)
      json_object_set_boolean(p_obj, "deprecated", p->deprecated ? 1 : 0);
    if (p->allow_empty_value_set)
      json_object_set_boolean(p_obj, "allowEmptyValue",
                              p->allow_empty_value ? 1 : 0);

    if (!(p->content_type || p->in == OA_PARAM_IN_QUERYSTRING)) {
      if (style_str)
        json_object_set_string(p_obj, "style", style_str);
      if (p->explode)
        json_object_set_boolean(p_obj, "explode", 1);
      if (p->allow_reserved_set)
        json_object_set_boolean(p_obj, "allowReserved",
                                p->allow_reserved ? 1 : 0);
    }

    if (p->content_type || p->in == OA_PARAM_IN_QUERYSTRING) {
      JSON_Value *content_val = json_value_init_object();
      JSON_Object *content_obj = json_value_get_object(content_val);
      JSON_Value *media_val = json_value_init_object();
      JSON_Object *media_obj = json_value_get_object(media_val);

      if (p->type) {
        JSON_Value *sch_val = json_value_init_object();
        json_object_set_string(json_value_get_object(sch_val), "type", p->type);
        json_object_set_value(media_obj, "schema", sch_val);
      }

      json_object_set_value(
          content_obj,
          p->content_type ? p->content_type
                          : "application/x-www-form-urlencoded",
          media_val);
      json_object_set_value(p_obj, "content", content_val);
    } else if (p->is_array) {
      JSON_Value *sch_val = json_value_init_object();
      JSON_Object *sch_obj = json_value_get_object(sch_val);
      json_object_set_string(sch_obj, "type", "array");
      if (p->items_type) {
        JSON_Value *it_val = json_value_init_object();
        json_object_set_string(json_value_get_object(it_val), "type",
                               p->items_type);
        json_object_set_value(sch_obj, "items", it_val);
      }
      json_object_set_value(p_obj, "schema", sch_val);
    } else if (p->type) {
      JSON_Value *sch_val = json_value_init_object();
      json_object_set_string(json_value_get_object(sch_val), "type", p->type);
      json_object_set_value(p_obj, "schema", sch_val);
    }

    json_array_append_value(arr, p_val);
  }

  json_object_set_value(parent, "parameters", arr_val);
  return 0;
}

static int write_request_body(JSON_Object *op_obj,
                              const struct OpenAPI_Operation *op) {
  JSON_Value *rb_val;
  JSON_Object *rb_obj;
  JSON_Value *content_val;
  JSON_Object *content_obj;
  JSON_Value *media_val;
  JSON_Object *media_obj;

  /* If body is empty and no fields, skip */
  if (op->req_body.ref_name == NULL && op->req_body.inline_type == NULL &&
      op->req_body.multipart_fields == NULL &&
      op->req_body.n_multipart_fields == 0) {
    return 0;
  }

  rb_val = json_value_init_object();
  rb_obj = json_value_get_object(rb_val);
  content_val = json_value_init_object();
  content_obj = json_value_get_object(content_val);
  media_val = json_value_init_object();
  media_obj = json_value_get_object(media_val);

  write_schema_ref(media_obj, "schema", &op->req_body);

  json_object_set_value(content_obj,
                        op->req_body.content_type ? op->req_body.content_type
                                                  : "application/json",
                        media_val);
  json_object_set_value(rb_obj, "content", content_val);
  if (op->req_body_description) {
    json_object_set_string(rb_obj, "description", op->req_body_description);
  }
  if (op->req_body_required_set) {
    json_object_set_boolean(rb_obj, "required", op->req_body_required ? 1 : 0);
  }

  json_object_set_value(op_obj, "requestBody", rb_val);
  return 0;
}

static int write_responses(JSON_Object *op_obj,
                           const struct OpenAPI_Operation *op) {
  JSON_Value *resps_val = json_value_init_object();
  JSON_Object *resps_obj = json_value_get_object(resps_val);
  size_t i;

  for (i = 0; i < op->n_responses; ++i) {
    const struct OpenAPI_Response *r = &op->responses[i];
    JSON_Value *r_val = json_value_init_object();
    JSON_Object *r_obj = json_value_get_object(r_val);

    json_object_set_string(r_obj, "description",
                           r->description ? r->description : "");

    if (r->schema.ref_name || r->schema.inline_type || r->schema.is_array ||
        r->content_type) {
      JSON_Value *cont_val = json_value_init_object();
      JSON_Object *cont_obj = json_value_get_object(cont_val);
      JSON_Value *media_val = json_value_init_object();
      JSON_Object *media_obj = json_value_get_object(media_val);

      if (r->schema.ref_name || r->schema.inline_type || r->schema.is_array ||
          r->schema.n_multipart_fields > 0) {
        write_schema_ref(media_obj, "schema", &r->schema);
      }

      json_object_set_value(
          cont_obj, r->content_type ? r->content_type : "application/json",
          media_val);
      json_object_set_value(r_obj, "content", cont_val);
    }

    json_object_set_value(resps_obj, r->code ? r->code : "default", r_val);
  }

  json_object_set_value(op_obj, "responses", resps_val);
  return 0;
}

static int write_operations(JSON_Object *path_item,
                            const struct OpenAPI_Path *path) {
  size_t i;
  int rc;

  for (i = 0; i < path->n_operations; ++i) {
    const struct OpenAPI_Operation *op = &path->operations[i];
    const char *verb = verb_to_str(op->verb);
    JSON_Value *op_val;
    JSON_Object *op_obj;

    if (!verb)
      continue;

    op_val = json_value_init_object();
    op_obj = json_value_get_object(op_val);

    if (op->operation_id) {
      json_object_set_string(op_obj, "operationId", op->operation_id);
    }
    if (op->summary) {
      json_object_set_string(op_obj, "summary", op->summary);
    }
    if (op->description) {
      json_object_set_string(op_obj, "description", op->description);
    }
    if (op->external_docs.url) {
      write_external_docs(op_obj, "externalDocs", &op->external_docs);
    }
    if (op->deprecated) {
      json_object_set_boolean(op_obj, "deprecated", 1);
    }
    rc = write_security_requirements(op_obj, "security", op->security,
                                     op->n_security, op->security_set);
    if (rc != 0) {
      json_value_free(op_val);
      return rc;
    }

    if (op->n_tags > 0) {
      JSON_Value *tags_val = json_value_init_array();
      JSON_Array *tags_arr = json_value_get_array(tags_val);
      size_t k;
      for (k = 0; k < op->n_tags; ++k) {
        json_array_append_string(tags_arr, op->tags[k]);
      }
      json_object_set_value(op_obj, "tags", tags_val);
    }

    rc = write_parameters(op_obj, op->parameters, op->n_parameters);
    if (rc != 0) {
      json_value_free(op_val);
      return rc;
    }

    rc = write_request_body(op_obj, op);
    if (rc != 0) {
      json_value_free(op_val);
      return rc;
    }

    rc = write_responses(op_obj, op);
    if (rc != 0) {
      json_value_free(op_val);
      return rc;
    }

    if (op->n_servers > 0 && op->servers) {
      rc = write_server_array(op_obj, "servers", op->servers, op->n_servers);
      if (rc != 0) {
        json_value_free(op_val);
        return rc;
      }
    }

    json_object_set_value(path_item, verb, op_val);
  }
  return 0;
}

static int write_paths(JSON_Object *root_obj, const struct OpenAPI_Spec *spec) {
  JSON_Value *paths_val = json_value_init_object();
  JSON_Object *paths_obj = json_value_get_object(paths_val);
  size_t i;
  int rc = 0;

  for (i = 0; i < spec->n_paths; ++i) {
    const struct OpenAPI_Path *p = &spec->paths[i];
    const char *route = p->route ? p->route : "/";
    JSON_Value *item_val;
    JSON_Object *item_obj;

    if (json_object_has_value(paths_obj, route)) {
      item_obj = json_object_get_object(paths_obj, route);
    } else {
      item_val = json_value_init_object();
      item_obj = json_value_get_object(item_val);
      json_object_set_value(paths_obj, route, item_val);
    }

    if (p->summary)
      json_object_set_string(item_obj, "summary", p->summary);
    if (p->description)
      json_object_set_string(item_obj, "description", p->description);
    if (p->ref)
      json_object_set_string(item_obj, "$ref", p->ref);
    if (p->n_parameters > 0) {
      rc = write_parameters(item_obj, p->parameters, p->n_parameters);
      if (rc != 0)
        break;
    }
    if (p->n_servers > 0 && p->servers) {
      rc = write_server_array(item_obj, "servers", p->servers, p->n_servers);
      if (rc != 0)
        break;
    }

    rc = write_operations(item_obj, p);
    if (rc != 0)
      break;
  }

  if (rc == 0) {
    json_object_set_value(root_obj, "paths", paths_val);
  } else {
    json_value_free(paths_val);
  }
  return rc;
}

static int write_servers(JSON_Object *root_obj, const struct OpenAPI_Spec *spec) {
  if (!spec)
    return 0;
  return write_server_array(root_obj, "servers", spec->servers,
                            spec->n_servers);
}

static int write_server_array(JSON_Object *parent, const char *key,
                              const struct OpenAPI_Server *servers,
                              size_t n_servers) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (!parent || !key || !servers || n_servers == 0)
    return 0;

  arr_val = json_value_init_array();
  if (!arr_val)
    return ENOMEM;
  arr = json_value_get_array(arr_val);

  for (i = 0; i < n_servers; ++i) {
    const struct OpenAPI_Server *srv = &servers[i];
    JSON_Value *srv_val = json_value_init_object();
    JSON_Object *srv_obj = json_value_get_object(srv_val);

    json_object_set_string(srv_obj, "url", srv->url ? srv->url : "/");
    if (srv->description)
      json_object_set_string(srv_obj, "description", srv->description);
    if (srv->name)
      json_object_set_string(srv_obj, "name", srv->name);
    if (srv->n_variables > 0 && srv->variables) {
      JSON_Value *vars_val = json_value_init_object();
      JSON_Object *vars_obj = json_value_get_object(vars_val);
      size_t v;
      for (v = 0; v < srv->n_variables; ++v) {
        const struct OpenAPI_ServerVariable *var = &srv->variables[v];
        JSON_Value *var_val = json_value_init_object();
        JSON_Object *var_obj = json_value_get_object(var_val);
        if (var->default_value)
          json_object_set_string(var_obj, "default", var->default_value);
        if (var->description)
          json_object_set_string(var_obj, "description", var->description);
        if (var->n_enum_values > 0 && var->enum_values) {
          JSON_Value *enum_val = json_value_init_array();
          JSON_Array *enum_arr = json_value_get_array(enum_val);
          size_t e;
          for (e = 0; e < var->n_enum_values; ++e) {
            if (var->enum_values[e])
              json_array_append_string(enum_arr, var->enum_values[e]);
          }
          json_object_set_value(var_obj, "enum", enum_val);
        }
        if (var->name)
          json_object_set_value(vars_obj, var->name, var_val);
        else
          json_value_free(var_val);
      }
      json_object_set_value(srv_obj, "variables", vars_val);
    }

    json_array_append_value(arr, srv_val);
  }

  json_object_set_value(parent, key, arr_val);
  return 0;
}

static int write_tags(JSON_Object *root_obj, const struct OpenAPI_Spec *spec) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (!spec || spec->n_tags == 0)
    return 0;

  arr_val = json_value_init_array();
  if (!arr_val)
    return ENOMEM;
  arr = json_value_get_array(arr_val);

  for (i = 0; i < spec->n_tags; ++i) {
    const struct OpenAPI_Tag *tag = &spec->tags[i];
    JSON_Value *tag_val = json_value_init_object();
    JSON_Object *tag_obj = json_value_get_object(tag_val);

    if (tag->name)
      json_object_set_string(tag_obj, "name", tag->name);
    if (tag->summary)
      json_object_set_string(tag_obj, "summary", tag->summary);
    if (tag->description)
      json_object_set_string(tag_obj, "description", tag->description);
    if (tag->parent)
      json_object_set_string(tag_obj, "parent", tag->parent);
    if (tag->kind)
      json_object_set_string(tag_obj, "kind", tag->kind);
    if (tag->external_docs.url)
      write_external_docs(tag_obj, "externalDocs", &tag->external_docs);

    json_array_append_value(arr, tag_val);
  }

  json_object_set_value(root_obj, "tags", arr_val);
  return 0;
}

static int write_webhooks(JSON_Object *root_obj,
                          const struct OpenAPI_Spec *spec) {
  JSON_Value *hooks_val;
  JSON_Object *hooks_obj;
  size_t i;
  int rc = 0;

  if (!spec || spec->n_webhooks == 0 || !spec->webhooks)
    return 0;

  hooks_val = json_value_init_object();
  if (!hooks_val)
    return ENOMEM;
  hooks_obj = json_value_get_object(hooks_val);

  for (i = 0; i < spec->n_webhooks; ++i) {
    const struct OpenAPI_Path *p = &spec->webhooks[i];
    const char *route = p->route ? p->route : "webhook";
    JSON_Value *item_val = json_value_init_object();
    JSON_Object *item_obj = json_value_get_object(item_val);

    if (p->summary)
      json_object_set_string(item_obj, "summary", p->summary);
    if (p->description)
      json_object_set_string(item_obj, "description", p->description);
    if (p->ref)
      json_object_set_string(item_obj, "$ref", p->ref);
    if (p->n_parameters > 0) {
      rc = write_parameters(item_obj, p->parameters, p->n_parameters);
      if (rc != 0)
        break;
    }
    if (p->n_servers > 0 && p->servers) {
      rc = write_server_array(item_obj, "servers", p->servers, p->n_servers);
      if (rc != 0)
        break;
    }
    rc = write_operations(item_obj, p);
    if (rc != 0)
      break;

    json_object_set_value(hooks_obj, route, item_val);
  }

  if (rc == 0) {
    json_object_set_value(root_obj, "webhooks", hooks_val);
  } else {
    json_value_free(hooks_val);
  }
  return rc;
}

static int write_security_requirements(
    JSON_Object *parent, const char *key,
    const struct OpenAPI_SecurityRequirementSet *sets, size_t count,
    int set_flag) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (!parent || !key || !set_flag)
    return 0;

  arr_val = json_value_init_array();
  if (!arr_val)
    return ENOMEM;
  arr = json_value_get_array(arr_val);

  if (count == 0) {
    json_object_set_value(parent, key, arr_val);
    return 0;
  }

  for (i = 0; i < count; ++i) {
    const struct OpenAPI_SecurityRequirementSet *set = &sets[i];
    JSON_Value *set_val = json_value_init_object();
    JSON_Object *set_obj = json_value_get_object(set_val);
    size_t j;

    if (!set_val) {
      json_value_free(arr_val);
      return ENOMEM;
    }

    for (j = 0; j < set->n_requirements; ++j) {
      const struct OpenAPI_SecurityRequirement *req = &set->requirements[j];
      JSON_Value *scopes_val = json_value_init_array();
      JSON_Array *scopes_arr = json_value_get_array(scopes_val);
      size_t k;

      if (!scopes_val) {
        json_value_free(set_val);
        json_value_free(arr_val);
        return ENOMEM;
      }

      for (k = 0; k < req->n_scopes; ++k) {
        json_array_append_string(scopes_arr, req->scopes[k]);
      }

      json_object_set_value(set_obj, req->scheme ? req->scheme : "",
                            scopes_val);
    }

    json_array_append_value(arr, set_val);
  }

  json_object_set_value(parent, key, arr_val);
  return 0;
}

/**
 * @brief Write security schemes to components.
 */
static int write_security_schemes(JSON_Object *components,
                                  const struct OpenAPI_Spec *spec) {
  JSON_Value *sec_val;
  JSON_Object *sec_obj;
  size_t i;

  if (spec->n_security_schemes == 0)
    return 0;

  sec_val = json_value_init_object();
  if (!sec_val)
    return ENOMEM;
  sec_obj = json_value_get_object(sec_val);

  for (i = 0; i < spec->n_security_schemes; ++i) {
    const struct OpenAPI_SecurityScheme *s = &spec->security_schemes[i];
    JSON_Value *s_val = json_value_init_object();
    JSON_Object *s_obj = json_value_get_object(s_val);

    switch (s->type) {
    case OA_SEC_APIKEY:
      json_object_set_string(s_obj, "type", "apiKey");
      if (s->in == OA_SEC_IN_HEADER)
        json_object_set_string(s_obj, "in", "header");
      else if (s->in == OA_SEC_IN_QUERY)
        json_object_set_string(s_obj, "in", "query");
      else if (s->in == OA_SEC_IN_COOKIE)
        json_object_set_string(s_obj, "in", "cookie");

      if (s->key_name)
        json_object_set_string(s_obj, "name", s->key_name);
      break;

    case OA_SEC_HTTP:
      json_object_set_string(s_obj, "type", "http");
      if (s->scheme)
        json_object_set_string(s_obj, "scheme", s->scheme);
      if (s->scheme && strcmp(s->scheme, "bearer") == 0) {
        if (s->bearer_format) {
          json_object_set_string(s_obj, "bearerFormat", s->bearer_format);
        } else {
          json_object_set_string(s_obj, "bearerFormat", "JWT"); /* Common */
        }
      }
      break;

    case OA_SEC_OAUTH2:
      json_object_set_string(s_obj, "type", "oauth2");
      if (s->oauth2_metadata_url) {
        json_object_set_string(s_obj, "oauth2MetadataUrl",
                               s->oauth2_metadata_url);
      }
      break;

    case OA_SEC_OPENID:
      json_object_set_string(s_obj, "type", "openIdConnect");
      if (s->open_id_connect_url) {
        json_object_set_string(s_obj, "openIdConnectUrl",
                               s->open_id_connect_url);
      }
      break;

    default:
      break;
    }

    json_object_set_value(sec_obj, s->name ? s->name : "unknown", s_val);
  }

  json_object_set_value(components, "securitySchemes", sec_val);
  return 0;
}

static int write_components(JSON_Object *root_obj,
                            const struct OpenAPI_Spec *spec) {
  JSON_Value *comps_val;
  JSON_Object *comps_obj;
  int rc;

  /* Only create components block if there is something to write */
  if (spec->n_defined_schemas == 0 && spec->n_security_schemes == 0) {
    return 0;
  }

  comps_val = json_value_init_object();
  if (!comps_val)
    return ENOMEM;
  comps_obj = json_value_get_object(comps_val);

  /* Schemas */
  if (spec->n_defined_schemas > 0) {
    JSON_Value *schemas_val = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(schemas_val);
    size_t i;

    for (i = 0; i < spec->n_defined_schemas; ++i) {
      if (spec->defined_schema_names[i]) {
        rc = write_struct_to_json_schema(schemas_obj,
                                         spec->defined_schema_names[i],
                                         &spec->defined_schemas[i]);
        if (rc != 0) {
          json_value_free(comps_val);
          json_value_free(schemas_val);
          return rc;
        }
      }
    }
    json_object_set_value(comps_obj, "schemas", schemas_val);
  }

  /* Security Schemes */
  if (spec->n_security_schemes > 0) {
    rc = write_security_schemes(comps_obj, spec);
    if (rc != 0) {
      json_value_free(comps_val);
      return rc;
    }
  }

  json_object_set_value(root_obj, "components", comps_val);
  return 0;
}

int openapi_write_spec_to_json(const struct OpenAPI_Spec *const spec,
                               char **json_out) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  int rc;

  if (!spec || !json_out) {
    return EINVAL;
  }

  root_val = json_value_init_object();
  if (!root_val)
    return ENOMEM;
  root_obj = json_value_get_object(root_val);

  json_object_set_string(root_obj, "openapi",
                         spec->openapi_version ? spec->openapi_version
                                               : "3.2.0");
  if (spec->self_uri)
    json_object_set_string(root_obj, "$self", spec->self_uri);
  if (spec->json_schema_dialect)
    json_object_set_string(root_obj, "jsonSchemaDialect",
                           spec->json_schema_dialect);
  write_info(root_obj, spec);
  if (spec->external_docs.url)
    write_external_docs(root_obj, "externalDocs", &spec->external_docs);
  rc = write_tags(root_obj, spec);
  if (rc != 0) {
    json_value_free(root_val);
    return rc;
  }

  rc = write_security_requirements(root_obj, "security", spec->security,
                                   spec->n_security, spec->security_set);
  if (rc != 0) {
    json_value_free(root_val);
    return rc;
  }

  rc = write_servers(root_obj, spec);
  if (rc != 0) {
    json_value_free(root_val);
    return rc;
  }

  rc = write_components(root_obj, spec);
  if (rc != 0) {
    json_value_free(root_val);
    return rc;
  }

  rc = write_webhooks(root_obj, spec);
  if (rc != 0) {
    json_value_free(root_val);
    return rc;
  }

  if (spec->n_paths > 0) {
    rc = write_paths(root_obj, spec);
    if (rc != 0) {
      json_value_free(root_val);
      return rc;
    }
  } else {
    JSON_Value *empty = json_value_init_object();
    json_object_set_value(root_obj, "paths", empty);
  }

  *json_out = json_serialize_to_string_pretty(root_val);
  json_value_free(root_val);

  return *json_out ? 0 : ENOMEM;
}
