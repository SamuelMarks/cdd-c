/**
 * @file openapi_writer.c
 * @brief Implementation of OpenAPI serialization logic (Advanced).
 *
 * Handles deep object serialization, parameter styles, security schemes,
 * and multipart schema construction.
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
static void write_schema_ref(JSON_Object *parent, const char *key,
                             const struct OpenAPI_SchemaRef *ref);
static void write_multipart_schema(JSON_Object *parent, const char *key,
                                   const struct OpenAPI_SchemaRef *ref);
static int write_parameters(JSON_Object *op_obj,
                            const struct OpenAPI_Operation *op);
static int write_responses(JSON_Object *op_obj,
                           const struct OpenAPI_Operation *op);
static int write_request_body(JSON_Object *op_obj,
                              const struct OpenAPI_Operation *op);
static int write_operations(JSON_Object *path_item,
                            const struct OpenAPI_Path *path);
static int write_paths(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);
static int write_security_schemes(JSON_Object *components,
                                  const struct OpenAPI_Spec *spec);
static int write_components(JSON_Object *root_obj,
                            const struct OpenAPI_Spec *spec);

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
  case OA_STYLE_SPACE_DELIMITED:
    return "spaceDelimited";
  case OA_STYLE_PIPE_DELIMITED:
    return "pipeDelimited";
  case OA_STYLE_DEEP_OBJECT:
    return "deepObject";
  default:
    return NULL;
  }
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
    if (ref->ref_name) {
      /* Simple detection: if standard type, use type, else ref */
      if (strcmp(ref->ref_name, "string") == 0 ||
          strcmp(ref->ref_name, "integer") == 0 ||
          strcmp(ref->ref_name, "boolean") == 0 ||
          strcmp(ref->ref_name, "number") == 0) {
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
  else if (ref->ref_name) {
    if (strcmp(ref->ref_name, "string") == 0 ||
        strcmp(ref->ref_name, "integer") == 0 ||
        strcmp(ref->ref_name, "boolean") == 0 ||
        strcmp(ref->ref_name, "number") == 0) {
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

static int write_parameters(JSON_Object *op_obj,
                            const struct OpenAPI_Operation *op) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (op->n_parameters == 0)
    return 0;

  arr_val = json_value_init_array();
  if (arr_val == NULL)
    return ENOMEM;
  arr = json_value_get_array(arr_val);

  for (i = 0; i < op->n_parameters; ++i) {
    const struct OpenAPI_Parameter *p = &op->parameters[i];
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

    if (style_str)
      json_object_set_string(p_obj, "style", style_str);
    if (p->explode)
      json_object_set_boolean(p_obj, "explode", 1);

    if (p->is_array) {
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

  json_object_set_value(op_obj, "parameters", arr_val);
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
  if (op->req_body.ref_name == NULL && op->req_body.multipart_fields == NULL &&
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
  json_object_set_boolean(rb_obj, "required", 1); /* Assume required */

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
                           ""); /* Description mandatory */

    if (r->schema.ref_name || r->schema.is_array) {
      JSON_Value *cont_val = json_value_init_object();
      JSON_Object *cont_obj = json_value_get_object(cont_val);
      JSON_Value *media_val = json_value_init_object();
      JSON_Object *media_obj = json_value_get_object(media_val);

      write_schema_ref(media_obj, "schema", &r->schema);

      json_object_set_value(cont_obj, "application/json", media_val);
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

    if (op->n_tags > 0) {
      JSON_Value *tags_val = json_value_init_array();
      JSON_Array *tags_arr = json_value_get_array(tags_val);
      size_t k;
      for (k = 0; k < op->n_tags; ++k) {
        json_array_append_string(tags_arr, op->tags[k]);
      }
      json_object_set_value(op_obj, "tags", tags_val);
    }

    rc = write_parameters(op_obj, op);
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
        json_object_set_string(s_obj, "bearerFormat", "JWT"); /* Common */
      }
      break;

    case OA_SEC_OAUTH2:
      json_object_set_string(s_obj, "type", "oauth2");
      break;

    case OA_SEC_OPENID:
      json_object_set_string(s_obj, "type", "openIdConnect");
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

  json_object_set_string(root_obj, "openapi", "3.0.0");
  {
    JSON_Value *info_val = json_value_init_object();
    JSON_Object *info_obj = json_value_get_object(info_val);
    json_object_set_string(info_obj, "title", "Generated Specification");
    json_object_set_string(info_obj, "version", "1.0.0");
    json_object_set_value(root_obj, "info", info_val);
  }

  rc = write_components(root_obj, spec);
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
