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
static int parse_schema_ref(const JSON_Object *schema,
                            struct OpenAPI_SchemaRef *out);
static int parse_parameters(const JSON_Array *arr,
                            struct OpenAPI_Operation *op);
static int parse_operation(const char *verb_str, const JSON_Object *op_obj,
                           struct OpenAPI_Operation *out_op);
static int parse_components(const JSON_Object *components,
                            struct OpenAPI_Spec *out);

/* --- Lifecycle Implementation --- */

void openapi_spec_init(struct OpenAPI_Spec *const spec) {
  if (spec) {
    spec->paths = NULL;
    spec->n_paths = 0;
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

static void free_operation(struct OpenAPI_Operation *op) {
  size_t i;
  if (!op)
    return;
  if (op->operation_id)
    free(op->operation_id);

  if (op->tags) {
    for (i = 0; i < op->n_tags; ++i) {
      if (op->tags[i])
        free(op->tags[i]);
    }
    free(op->tags);
  }

  free_schema_ref_content(&op->req_body);

  if (op->parameters) {
    for (i = 0; i < op->n_parameters; ++i) {
      free(op->parameters[i].name);
      free(op->parameters[i].type);
      if (op->parameters[i].items_type)
        free(op->parameters[i].items_type);
    }
    free(op->parameters);
  }

  if (op->responses) {
    for (i = 0; i < op->n_responses; ++i) {
      free(op->responses[i].code);
      free_schema_ref_content(&op->responses[i].schema);
    }
    free(op->responses);
  }
}

void openapi_spec_free(struct OpenAPI_Spec *const spec) {
  size_t i, j;
  if (!spec)
    return;

  if (spec->paths) {
    for (i = 0; i < spec->n_paths; ++i) {
      struct OpenAPI_Path *p = &spec->paths[i];
      if (p->route)
        free(p->route);
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

  if (spec->security_schemes) {
    for (i = 0; i < spec->n_security_schemes; ++i) {
      if (spec->security_schemes[i].name)
        free(spec->security_schemes[i].name);
      if (spec->security_schemes[i].scheme)
        free(spec->security_schemes[i].scheme);
      if (spec->security_schemes[i].key_name)
        free(spec->security_schemes[i].key_name);
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
  return OA_VERB_UNKNOWN;
}

static enum OpenAPI_ParamIn parse_param_in(const char *const in) {
  if (strcmp(in, "path") == 0)
    return OA_PARAM_IN_PATH;
  if (strcmp(in, "query") == 0)
    return OA_PARAM_IN_QUERY;
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
  if (strcmp(s, "spaceDelimited") == 0)
    return OA_STYLE_SPACE_DELIMITED;
  if (strcmp(s, "pipeDelimited") == 0)
    return OA_STYLE_PIPE_DELIMITED;
  if (strcmp(s, "deepObject") == 0)
    return OA_STYLE_DEEP_OBJECT;
  return OA_STYLE_UNKNOWN;
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
  out->content_type = NULL;
  out->multipart_fields = NULL;
  out->n_multipart_fields = 0;

  if (ref) {
    out->ref_name = clean_ref(ref);
    return out->ref_name ? 0 : ENOMEM;
  }

  if (type && strcmp(type, "array") == 0) {
    const JSON_Object *items = json_object_get_object(schema, "items");
    if (items) {
      const char *item_ref = json_object_get_string(items, "$ref");
      if (item_ref) {
        out->is_array = 1;
        out->ref_name = clean_ref(item_ref);
        return out->ref_name ? 0 : ENOMEM;
      }
    }
  }

  return 0;
}

static int parse_parameters(const JSON_Array *const arr,
                            struct OpenAPI_Operation *const op) {
  size_t i, count;
  if (!arr || !op)
    return 0;

  count = json_array_get_count(arr);
  op->n_parameters = count;
  op->parameters = (struct OpenAPI_Parameter *)calloc(
      count, sizeof(struct OpenAPI_Parameter));
  if (!op->parameters)
    return ENOMEM;

  for (i = 0; i < count; ++i) {
    const JSON_Object *p_obj = json_array_get_object(arr, i);
    const char *name = json_object_get_string(p_obj, "name");
    const char *in = json_object_get_string(p_obj, "in");
    int req = json_object_get_boolean(p_obj, "required");
    const JSON_Object *schema = json_object_get_object(p_obj, "schema");
    const char *type = NULL;
    const char *style_str = json_object_get_string(p_obj, "style");
    int explode_present = json_object_has_value(p_obj, "explode");
    int explode_val = json_object_get_boolean(p_obj, "explode");

    if (schema)
      type = json_object_get_string(schema, "type");

    op->parameters[i].name = c_cdd_strdup(name ? name : "");
    op->parameters[i].in = in ? parse_param_in(in) : OA_PARAM_IN_UNKNOWN;
    op->parameters[i].required = (req == 1);
    op->parameters[i].type = c_cdd_strdup(type ? type : "string");

    /* Handle Arrays */
    if (type && strcmp(type, "array") == 0) {
      const JSON_Object *items = json_object_get_object(schema, "items");
      op->parameters[i].is_array = 1;
      if (items) {
        const char *i_type = json_object_get_string(items, "type");
        if (i_type) {
          op->parameters[i].items_type = c_cdd_strdup(i_type);
        }
      }
    } else {
      op->parameters[i].is_array = 0;
      op->parameters[i].items_type = NULL;
    }

    /* Handle Style / Explode */
    if (style_str) {
      op->parameters[i].style = parse_param_style(style_str);
    } else {
      /* Defaults */
      if (op->parameters[i].in == OA_PARAM_IN_QUERY)
        op->parameters[i].style = OA_STYLE_FORM;
      else if (op->parameters[i].in == OA_PARAM_IN_PATH)
        op->parameters[i].style = OA_STYLE_SIMPLE;
      else
        op->parameters[i].style = OA_STYLE_SIMPLE;
    }

    if (explode_present) {
      op->parameters[i].explode = explode_val;
    } else {
      /* Defaults based on style */
      if (op->parameters[i].style == OA_STYLE_FORM)
        op->parameters[i].explode = 1;
      else
        op->parameters[i].explode = 0;
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
      const JSON_Object *content = json_object_get_object(resp_obj, "content");
      if (content) {
        const JSON_Object *json_media =
            json_object_get_object(content, "application/json");
        if (json_media) {
          const JSON_Object *schema =
              json_object_get_object(json_media, "schema");
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

  if (!verb_str || !op_obj || !out_op)
    return EINVAL;

  out_op->verb = parse_verb(verb_str);
  if (out_op->verb == OA_VERB_UNKNOWN)
    return 0;

  op_id = json_object_get_string(op_obj, "operationId");
  out_op->operation_id = c_cdd_strdup(op_id ? op_id : "unnamed");

  /* 1. Parameters */
  params = json_object_get_array(op_obj, "parameters");
  if (parse_parameters(params, out_op) != 0)
    return ENOMEM;

  /* 2. Request Body */
  req_body = json_object_get_object(op_obj, "requestBody");
  if (req_body) {
    const JSON_Object *content = json_object_get_object(req_body, "content");
    /* Priority: JSON -> Form -> Multipart */
    const JSON_Object *media_obj = NULL;
    const char *detected_type = NULL;

    if ((media_obj = json_object_get_object(content, "application/json"))) {
      detected_type = "application/json";
    } else if ((media_obj = json_object_get_object(
                    content, "application/x-www-form-urlencoded"))) {
      detected_type = "application/x-www-form-urlencoded";
    }

    if (media_obj) {
      const JSON_Object *schema = json_object_get_object(media_obj, "schema");
      if (schema) {
        parse_schema_ref(schema, &out_op->req_body);
        out_op->req_body.content_type = c_cdd_strdup(detected_type);
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

  return 0;
}

static int parse_components(const JSON_Object *components,
                            struct OpenAPI_Spec *out) {
  const JSON_Object *schemas;
  size_t i, count;

  if (!components || !out)
    return 0;

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

int openapi_load_from_json(const JSON_Value *const root,
                           struct OpenAPI_Spec *const out) {
  const JSON_Object *root_obj;
  const JSON_Object *paths_obj;
  const JSON_Object *comps_obj;
  size_t i, n_paths;

  if (!root || !out)
    return EINVAL;

  root_obj = json_value_get_object(root);
  if (!root_obj)
    return EINVAL;

  paths_obj = json_object_get_object(root_obj, "paths");
  comps_obj = json_object_get_object(root_obj, "components");

  /* Load Schemas First */
  if (comps_obj) {
    if (parse_components(comps_obj, out) != 0) {
      openapi_spec_free(out);
      return ENOMEM;
    }
  }

  if (!paths_obj)
    return 0;

  n_paths = json_object_get_count(paths_obj);
  out->n_paths = n_paths;
  out->paths =
      (struct OpenAPI_Path *)calloc(n_paths, sizeof(struct OpenAPI_Path));
  if (!out->paths)
    return ENOMEM;

  for (i = 0; i < n_paths; ++i) {
    const char *route = json_object_get_name(paths_obj, i);
    const JSON_Value *p_val = json_object_get_value_at(paths_obj, i);
    const JSON_Object *p_obj = json_value_get_object(p_val);
    struct OpenAPI_Path *curr_path = &out->paths[i];
    size_t n_ops_in_obj = json_object_get_count(p_obj);
    size_t k, valid_ops = 0;

    curr_path->route = c_cdd_strdup(route);
    curr_path->operations = (struct OpenAPI_Operation *)calloc(
        n_ops_in_obj, sizeof(struct OpenAPI_Operation));

    if (!curr_path->operations) {
      openapi_spec_free(out);
      return ENOMEM;
    }

    for (k = 0; k < n_ops_in_obj; ++k) {
      const char *verb = json_object_get_name(p_obj, k);
      const JSON_Object *op_obj =
          json_value_get_object(json_object_get_value_at(p_obj, k));

      if (parse_operation(verb, op_obj, &curr_path->operations[valid_ops]) ==
          0) {
        if (curr_path->operations[valid_ops].verb != OA_VERB_UNKNOWN) {
          valid_ops++;
        }
      }
    }
    curr_path->n_operations = valid_ops;
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
