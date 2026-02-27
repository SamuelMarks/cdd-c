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

#include "classes/parse_code2schema.h"
#include "functions/parse_str.h"
#include "openapi/emit_openapi.h"

/* --- Helper Prototypes --- */

static const char *verb_to_str(enum OpenAPI_Verb v);
static const char *param_in_to_str(enum OpenAPI_ParamIn in);
static const char *style_to_str(enum OpenAPI_Style s);
static const char *oauth_flow_type_to_str(enum OpenAPI_OAuthFlowType t);
static const char *xml_node_type_to_str(enum OpenAPI_XmlNodeType t);
static int is_schema_primitive(const char *type);
static int license_fields_invalid(const struct OpenAPI_License *lic);
static int server_url_has_query_or_fragment(const char *url);
static JSON_Value *clone_json_value(const JSON_Value *val);
static int merge_schema_extras_object(JSON_Object *target,
                                      const char *extras_json);
static JSON_Value *any_to_json_value(const struct OpenAPI_Any *val);
static void write_example_object(JSON_Object *ex_obj,
                                 const struct OpenAPI_Example *ex);
static int write_examples_object(JSON_Object *parent, const char *key,
                                 const struct OpenAPI_Example *examples,
                                 size_t n_examples);
static void write_example_fields(JSON_Object *parent,
                                 const struct OpenAPI_Any *example,
                                 int example_set,
                                 const struct OpenAPI_Example *examples,
                                 size_t n_examples);
static void write_schema_example(JSON_Object *obj,
                                 const struct OpenAPI_Any *example,
                                 int example_set);
static void write_numeric_constraints(JSON_Object *obj, int has_min,
                                      double min_val, int exclusive_min,
                                      int has_max, double max_val,
                                      int exclusive_max);
static void write_string_constraints(JSON_Object *obj, int has_min_len,
                                     size_t min_len, int has_max_len,
                                     size_t max_len, const char *pattern);
static void write_array_constraints(JSON_Object *obj, int has_min_items,
                                    size_t min_items, int has_max_items,
                                    size_t max_items, int unique_items);
static void write_items_schema_fields(JSON_Object *item_obj,
                                      const struct OpenAPI_SchemaRef *ref);
static void write_external_docs(JSON_Object *parent, const char *key,
                                const struct OpenAPI_ExternalDocs *docs);
static void write_discriminator_object(JSON_Object *parent,
                                       const struct OpenAPI_Discriminator *disc,
                                       int disc_set);
static void write_xml_object(JSON_Object *parent, const struct OpenAPI_Xml *xml,
                             int xml_set);
static void write_info(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);
static void write_server_object(JSON_Object *srv_obj,
                                const struct OpenAPI_Server *srv);
static void write_schema_ref(JSON_Object *parent, const char *key,
                             const struct OpenAPI_SchemaRef *ref);
static void write_schema_from_type_fields(JSON_Object *parent, const char *key,
                                          const char *type, int is_array,
                                          const char *items_type);
static void write_multipart_schema(JSON_Object *parent, const char *key,
                                   const struct OpenAPI_SchemaRef *ref);
static void write_parameter_object(JSON_Object *p_obj,
                                   const struct OpenAPI_Parameter *p);
static void write_header_object(JSON_Object *h_obj,
                                const struct OpenAPI_Header *h);
static void write_link_object(JSON_Object *l_obj,
                              const struct OpenAPI_Link *link);
static int write_headers_map(JSON_Object *parent, const char *key,
                             const struct OpenAPI_Header *headers,
                             size_t n_headers, int ignore_content_type);
static int write_headers(JSON_Object *parent,
                         const struct OpenAPI_Response *resp);
static int write_links(JSON_Object *parent,
                       const struct OpenAPI_Response *resp);
static int write_media_type_object(JSON_Object *media_obj,
                                   const struct OpenAPI_MediaType *mt);
static int write_media_type_map(JSON_Object *parent, const char *key,
                                const struct OpenAPI_MediaType *mts,
                                size_t n_mts);
static int write_encoding_object(JSON_Object *enc_obj,
                                 const struct OpenAPI_Encoding *enc);
static int write_encoding_map(JSON_Object *media_obj,
                              const struct OpenAPI_Encoding *encoding,
                              size_t n_encoding);
static int write_encoding_array(JSON_Object *parent, const char *key,
                                const struct OpenAPI_Encoding *encoding,
                                size_t n_encoding);
static void write_response_object(JSON_Object *r_obj,
                                  const struct OpenAPI_Response *resp);
static int write_operation_object(JSON_Object *op_obj,
                                  const struct OpenAPI_Operation *op);
static int write_parameters(JSON_Object *parent,
                            const struct OpenAPI_Parameter *params,
                            size_t n_params);
static int write_responses(JSON_Object *op_obj,
                           const struct OpenAPI_Operation *op);
static int write_request_body(JSON_Object *op_obj,
                              const struct OpenAPI_Operation *op);
static int write_callbacks(JSON_Object *op_obj,
                           const struct OpenAPI_Operation *op);
static int write_operations(JSON_Object *path_item,
                            const struct OpenAPI_Path *path);
static int write_additional_operations(JSON_Object *path_item,
                                       const struct OpenAPI_Path *path);
static int write_path_item_object(JSON_Object *item_obj,
                                  const struct OpenAPI_Path *path);
static int write_paths(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);
static int write_servers(JSON_Object *root_obj,
                         const struct OpenAPI_Spec *spec);
static int write_server_array(JSON_Object *parent, const char *key,
                              const struct OpenAPI_Server *servers,
                              size_t n_servers);
static int
write_security_requirements(JSON_Object *parent, const char *key,
                            const struct OpenAPI_SecurityRequirementSet *sets,
                            size_t count, int set_flag);
static int write_security_schemes(JSON_Object *components,
                                  const struct OpenAPI_Spec *spec);
static int write_component_parameters(JSON_Object *components,
                                      const struct OpenAPI_Spec *spec);
static int write_component_responses(JSON_Object *components,
                                     const struct OpenAPI_Spec *spec);
static int write_component_headers(JSON_Object *components,
                                   const struct OpenAPI_Spec *spec);
static int write_component_media_types(JSON_Object *components,
                                       const struct OpenAPI_Spec *spec);
static int write_component_examples(JSON_Object *components,
                                    const struct OpenAPI_Spec *spec);
static int write_component_links(JSON_Object *components,
                                 const struct OpenAPI_Spec *spec);
static int write_component_callbacks(JSON_Object *components,
                                     const struct OpenAPI_Spec *spec);
static int write_component_path_items(JSON_Object *components,
                                      const struct OpenAPI_Spec *spec);
static int write_components(JSON_Object *root_obj,
                            const struct OpenAPI_Spec *spec);
static int write_tags(JSON_Object *root_obj, const struct OpenAPI_Spec *spec);
static int write_webhooks(JSON_Object *root_obj,
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

static const char *xml_node_type_to_str(enum OpenAPI_XmlNodeType t) {
  switch (t) {
  case OA_XML_NODE_ELEMENT:
    return "element";
  case OA_XML_NODE_ATTRIBUTE:
    return "attribute";
  case OA_XML_NODE_TEXT:
    return "text";
  case OA_XML_NODE_CDATA:
    return "cdata";
  case OA_XML_NODE_NONE:
    return "none";
  default:
    return NULL;
  }
}

static int header_name_is_content_type(const char *const name) {
  if (!name)
    return 0;
  return c_cdd_str_iequal(name, "Content-Type");
}

static int param_is_reserved_header(const struct OpenAPI_Parameter *p) {
  if (!p || p->in != OA_PARAM_IN_HEADER || !p->name)
    return 0;
  return c_cdd_str_iequal(p->name, "Accept") ||
         c_cdd_str_iequal(p->name, "Content-Type") ||
         c_cdd_str_iequal(p->name, "Authorization");
}

static const char *oauth_flow_type_to_str(enum OpenAPI_OAuthFlowType t) {
  switch (t) {
  case OA_OAUTH_FLOW_IMPLICIT:
    return "implicit";
  case OA_OAUTH_FLOW_PASSWORD:
    return "password";
  case OA_OAUTH_FLOW_CLIENT_CREDENTIALS:
    return "clientCredentials";
  case OA_OAUTH_FLOW_AUTHORIZATION_CODE:
    return "authorizationCode";
  case OA_OAUTH_FLOW_DEVICE_AUTHORIZATION:
    return "deviceAuthorization";
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

static int license_fields_invalid(const struct OpenAPI_License *lic) {
  int has_any;
  if (!lic)
    return 0;
  has_any = lic->name || lic->identifier || lic->url || lic->extensions_json;
  if (!has_any)
    return 0;
  if (!lic->name || lic->name[0] == '\0')
    return 1;
  if (lic->identifier && lic->url)
    return 1;
  return 0;
}

static int server_url_has_query_or_fragment(const char *url) {
  if (!url)
    return 0;
  return strchr(url, '?') != NULL || strchr(url, '#') != NULL;
}

static JSON_Value *clone_json_value(const JSON_Value *val) {
  char *serialized;
  JSON_Value *copy;

  if (!val)
    return NULL;
  serialized = json_serialize_to_string((JSON_Value *)val);
  if (!serialized)
    return NULL;
  copy = json_parse_string(serialized);
  json_free_serialized_string(serialized);
  return copy;
}

static int merge_schema_extras_object(JSON_Object *target,
                                      const char *extras_json) {
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;

  if (!target || !extras_json || extras_json[0] == '\0')
    return 0;

  extras_val = json_parse_string(extras_json);
  if (!extras_val)
    return 0;
  extras_obj = json_value_get_object(extras_val);
  if (!extras_obj) {
    json_value_free(extras_val);
    return 0;
  }

  count = json_object_get_count(extras_obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(extras_obj, i);
    const JSON_Value *val;
    JSON_Value *copy;
    if (!key || json_object_has_value(target, key))
      continue;
    val = json_object_get_value(extras_obj, key);
    copy = clone_json_value(val);
    if (!copy) {
      json_value_free(extras_val);
      return ENOMEM;
    }
    if (json_object_set_value(target, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return ENOMEM;
    }
  }

  json_value_free(extras_val);
  return 0;
}

static int schema_ref_has_data(const struct OpenAPI_SchemaRef *ref) {
  if (!ref)
    return 0;
  return ref->schema_is_boolean || ref->ref_name || ref->ref ||
         ref->inline_type || ref->n_type_union > 0 || ref->is_array ||
         ref->format || ref->content_media_type || ref->content_encoding ||
         ref->items_format || ref->n_items_type_union > 0 ||
         ref->items_content_media_type || ref->items_content_encoding ||
         ref->n_multipart_fields > 0 || ref->nullable || ref->items_nullable ||
         ref->default_value_set || ref->n_enum_values > 0 ||
         ref->n_items_enum_values > 0 || ref->summary || ref->description ||
         ref->deprecated_set || ref->read_only_set || ref->write_only_set ||
         ref->const_value_set || ref->n_examples > 0 || ref->example_set ||
         ref->has_min || ref->has_max || ref->has_min_len || ref->has_max_len ||
         ref->pattern || ref->has_min_items || ref->has_max_items ||
         ref->unique_items || ref->items_has_min || ref->items_has_max ||
         ref->items_has_min_len || ref->items_has_max_len ||
         ref->items_pattern || ref->items_has_min_items ||
         ref->items_has_max_items || ref->items_unique_items ||
         ref->items_example_set || ref->n_items_examples > 0 ||
         ref->items_schema_is_boolean || ref->schema_extra_json ||
         ref->external_docs_set || ref->discriminator_set || ref->xml_set ||
         ref->items_extra_json || ref->items_const_value_set ||
         ref->items_default_value_set;
}

static void write_schema_type(JSON_Object *obj, const char *type,
                              int nullable) {
  if (!obj || !type)
    return;
  if (nullable && strcmp(type, "null") != 0) {
    JSON_Value *type_val = json_value_init_array();
    JSON_Array *type_arr = json_value_get_array(type_val);
    if (!type_arr) {
      json_value_free(type_val);
      return;
    }
    json_array_append_string(type_arr, type);
    json_array_append_string(type_arr, "null");
    json_object_set_value(obj, "type", type_val);
    return;
  }
  json_object_set_string(obj, "type", type);
}

static int type_union_contains(char **types, size_t n_types,
                               const char *value) {
  size_t i;
  if (!types || !value)
    return 0;
  for (i = 0; i < n_types; ++i) {
    if (types[i] && strcmp(types[i], value) == 0)
      return 1;
  }
  return 0;
}

static void write_schema_type_union(JSON_Object *obj, const char *type,
                                    int nullable, char **type_union,
                                    size_t n_type_union) {
  size_t i;
  if (!obj)
    return;
  if (type_union && n_type_union > 0) {
    JSON_Value *type_val = json_value_init_array();
    JSON_Array *type_arr = json_value_get_array(type_val);
    if (!type_arr) {
      json_value_free(type_val);
      return;
    }
    for (i = 0; i < n_type_union; ++i) {
      if (type_union[i])
        json_array_append_string(type_arr, type_union[i]);
    }
    if (nullable && !type_union_contains(type_union, n_type_union, "null"))
      json_array_append_string(type_arr, "null");
    json_object_set_value(obj, "type", type_val);
    return;
  }
  if (type)
    write_schema_type(obj, type, nullable);
}

static void write_enum_values(JSON_Object *obj, const char *key, char **values,
                              size_t n_values) {
  JSON_Value *enum_val;
  JSON_Array *enum_arr;
  size_t i;

  if (!obj || !key || !values || n_values == 0)
    return;

  enum_val = json_value_init_array();
  if (!enum_val)
    return;
  enum_arr = json_value_get_array(enum_val);
  if (!enum_arr) {
    json_value_free(enum_val);
    return;
  }
  for (i = 0; i < n_values; ++i) {
    if (values[i])
      json_array_append_string(enum_arr, values[i]);
  }
  json_object_set_value(obj, key, enum_val);
}

static void write_enum_any_values(JSON_Object *obj, const char *key,
                                  const struct OpenAPI_Any *values,
                                  size_t n_values) {
  JSON_Value *enum_val;
  JSON_Array *enum_arr;
  size_t i;

  if (!obj || !key || !values || n_values == 0)
    return;

  enum_val = json_value_init_array();
  enum_arr = json_value_get_array(enum_val);
  if (!enum_arr) {
    json_value_free(enum_val);
    return;
  }

  for (i = 0; i < n_values; ++i) {
    JSON_Value *val = any_to_json_value(&values[i]);
    if (val)
      json_array_append_value(enum_arr, val);
  }

  json_object_set_value(obj, key, enum_val);
}

static void write_any_array_values(JSON_Object *obj, const char *key,
                                   const struct OpenAPI_Any *values,
                                   size_t n_values) {
  write_enum_any_values(obj, key, values, n_values);
}

static JSON_Value *any_to_json_value(const struct OpenAPI_Any *val) {
  if (!val)
    return NULL;
  switch (val->type) {
  case OA_ANY_STRING:
    return json_value_init_string(val->string ? val->string : "");
  case OA_ANY_NUMBER:
    return json_value_init_number(val->number);
  case OA_ANY_BOOL:
    return json_value_init_boolean(val->boolean ? 1 : 0);
  case OA_ANY_NULL:
    return json_value_init_null();
  case OA_ANY_JSON:
    if (val->json) {
      JSON_Value *parsed = json_parse_string(val->json);
      if (parsed)
        return parsed;
    }
    return json_value_init_string(val->json ? val->json : "");
  default:
    break;
  }
  return NULL;
}

static void write_example_object(JSON_Object *ex_obj,
                                 const struct OpenAPI_Example *ex) {
  JSON_Value *val;
  if (!ex_obj || !ex)
    return;

  if (ex->ref) {
    json_object_set_string(ex_obj, "$ref", ex->ref);
    if (ex->summary)
      json_object_set_string(ex_obj, "summary", ex->summary);
    if (ex->description)
      json_object_set_string(ex_obj, "description", ex->description);
    return;
  }

  if (ex->summary)
    json_object_set_string(ex_obj, "summary", ex->summary);
  if (ex->description)
    json_object_set_string(ex_obj, "description", ex->description);

  if (ex->data_value_set) {
    val = any_to_json_value(&ex->data_value);
    if (val)
      json_object_set_value(ex_obj, "dataValue", val);
  } else if (ex->value_set) {
    val = any_to_json_value(&ex->value);
    if (val)
      json_object_set_value(ex_obj, "value", val);
  }

  if (ex->serialized_value)
    json_object_set_string(ex_obj, "serializedValue", ex->serialized_value);
  if (ex->external_value)
    json_object_set_string(ex_obj, "externalValue", ex->external_value);
  if (ex->extensions_json)
    merge_schema_extras_object(ex_obj, ex->extensions_json);
}

static int write_examples_object(JSON_Object *parent, const char *key,
                                 const struct OpenAPI_Example *examples,
                                 size_t n_examples) {
  JSON_Value *examples_val;
  JSON_Object *examples_obj;
  size_t i;

  if (!parent || !key || !examples || n_examples == 0)
    return 0;

  examples_val = json_value_init_object();
  if (!examples_val)
    return ENOMEM;
  examples_obj = json_value_get_object(examples_val);

  for (i = 0; i < n_examples; ++i) {
    const struct OpenAPI_Example *ex = &examples[i];
    if (!ex->name)
      continue;
    {
      JSON_Value *ex_val = json_value_init_object();
      JSON_Object *ex_obj = json_value_get_object(ex_val);
      write_example_object(ex_obj, ex);
      json_object_set_value(examples_obj, ex->name, ex_val);
    }
  }

  json_object_set_value(parent, key, examples_val);
  return 0;
}

static void write_example_fields(JSON_Object *parent,
                                 const struct OpenAPI_Any *example,
                                 int example_set,
                                 const struct OpenAPI_Example *examples,
                                 size_t n_examples) {
  if (!parent)
    return;
  if (examples && n_examples > 0) {
    (void)write_examples_object(parent, "examples", examples, n_examples);
  } else if (example_set && example) {
    JSON_Value *val = any_to_json_value(example);
    if (val)
      json_object_set_value(parent, "example", val);
  }
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
  if (docs->extensions_json)
    merge_schema_extras_object(ext_obj, docs->extensions_json);

  json_object_set_value(parent, key, ext_val);
}

static void write_discriminator_object(JSON_Object *parent,
                                       const struct OpenAPI_Discriminator *disc,
                                       int disc_set) {
  JSON_Value *disc_val;
  JSON_Object *disc_obj;
  JSON_Value *mapping_val;
  JSON_Object *mapping_obj;
  size_t i;

  if (!parent || !disc || !disc_set)
    return;
  if (!disc->property_name && disc->n_mapping == 0 && !disc->default_mapping)
    return;

  disc_val = json_value_init_object();
  if (!disc_val)
    return;
  disc_obj = json_value_get_object(disc_val);

  if (disc->property_name)
    json_object_set_string(disc_obj, "propertyName", disc->property_name);
  if (disc->default_mapping)
    json_object_set_string(disc_obj, "defaultMapping", disc->default_mapping);

  if (disc->mapping && disc->n_mapping > 0) {
    mapping_val = json_value_init_object();
    mapping_obj = json_value_get_object(mapping_val);
    for (i = 0; i < disc->n_mapping; ++i) {
      const struct OpenAPI_DiscriminatorMap *m = &disc->mapping[i];
      if (m->value && m->schema)
        json_object_set_string(mapping_obj, m->value, m->schema);
    }
    json_object_set_value(disc_obj, "mapping", mapping_val);
  }

  if (disc->extensions_json)
    merge_schema_extras_object(disc_obj, disc->extensions_json);

  json_object_set_value(parent, "discriminator", disc_val);
}

static void write_xml_object(JSON_Object *parent, const struct OpenAPI_Xml *xml,
                             int xml_set) {
  JSON_Value *xml_val;
  JSON_Object *xml_obj;
  const char *node_type;

  if (!parent || !xml || !xml_set)
    return;

  xml_val = json_value_init_object();
  if (!xml_val)
    return;
  xml_obj = json_value_get_object(xml_val);

  node_type = xml_node_type_to_str(xml->node_type);
  if (xml->node_type_set && node_type)
    json_object_set_string(xml_obj, "nodeType", node_type);
  if (xml->name)
    json_object_set_string(xml_obj, "name", xml->name);
  if (xml->namespace_uri)
    json_object_set_string(xml_obj, "namespace", xml->namespace_uri);
  if (xml->prefix)
    json_object_set_string(xml_obj, "prefix", xml->prefix);
  if (xml->attribute_set)
    json_object_set_boolean(xml_obj, "attribute", xml->attribute ? 1 : 0);
  if (xml->wrapped_set)
    json_object_set_boolean(xml_obj, "wrapped", xml->wrapped ? 1 : 0);
  if (xml->extensions_json)
    merge_schema_extras_object(xml_obj, xml->extensions_json);

  json_object_set_value(parent, "xml", xml_val);
}

static void write_info(JSON_Object *root_obj, const struct OpenAPI_Spec *spec) {
  JSON_Value *info_val = json_value_init_object();
  JSON_Object *info_obj = json_value_get_object(info_val);
  JSON_Value *contact_val;
  JSON_Object *contact_obj;
  JSON_Value *license_val;
  JSON_Object *license_obj;
  const char *title =
      spec->info.title ? spec->info.title : "Generated Specification";
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
  if (spec->info.extensions_json)
    merge_schema_extras_object(info_obj, spec->info.extensions_json);

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
    if (spec->info.contact.extensions_json)
      merge_schema_extras_object(contact_obj,
                                 spec->info.contact.extensions_json);
    json_object_set_value(info_obj, "contact", contact_val);
  }

  if (spec->info.license.name || spec->info.license.identifier ||
      spec->info.license.url || spec->info.license.extensions_json) {
    license_val = json_value_init_object();
    license_obj = json_value_get_object(license_val);
    if (spec->info.license.name)
      json_object_set_string(license_obj, "name", spec->info.license.name);
    if (spec->info.license.identifier)
      json_object_set_string(license_obj, "identifier",
                             spec->info.license.identifier);
    if (spec->info.license.url)
      json_object_set_string(license_obj, "url", spec->info.license.url);
    if (spec->info.license.extensions_json)
      merge_schema_extras_object(license_obj,
                                 spec->info.license.extensions_json);
    json_object_set_value(info_obj, "license", license_val);
  }

  json_object_set_value(root_obj, "info", info_val);
}

static void write_server_object(JSON_Object *srv_obj,
                                const struct OpenAPI_Server *srv) {
  if (!srv_obj || !srv)
    return;

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
      if (var->extensions_json)
        merge_schema_extras_object(var_obj, var->extensions_json);
      if (var->name)
        json_object_set_value(vars_obj, var->name, var_val);
      else
        json_value_free(var_val);
    }
    json_object_set_value(srv_obj, "variables", vars_val);
  }
  if (srv->extensions_json)
    merge_schema_extras_object(srv_obj, srv->extensions_json);
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

static void write_schema_example(JSON_Object *obj,
                                 const struct OpenAPI_Any *example,
                                 int example_set) {
  if (!obj || !example_set)
    return;
  JSON_Value *ex_val = any_to_json_value(example);
  if (ex_val)
    json_object_set_value(obj, "example", ex_val);
}

static void write_numeric_constraints(JSON_Object *obj, int has_min,
                                      double min_val, int exclusive_min,
                                      int has_max, double max_val,
                                      int exclusive_max) {
  if (!obj)
    return;
  if (has_min) {
    if (exclusive_min)
      json_object_set_number(obj, "exclusiveMinimum", min_val);
    else
      json_object_set_number(obj, "minimum", min_val);
  } else if (exclusive_min) {
    json_object_set_boolean(obj, "exclusiveMinimum", 1);
  }
  if (has_max) {
    if (exclusive_max)
      json_object_set_number(obj, "exclusiveMaximum", max_val);
    else
      json_object_set_number(obj, "maximum", max_val);
  } else if (exclusive_max) {
    json_object_set_boolean(obj, "exclusiveMaximum", 1);
  }
}

static void write_string_constraints(JSON_Object *obj, int has_min_len,
                                     size_t min_len, int has_max_len,
                                     size_t max_len, const char *pattern) {
  if (!obj)
    return;
  if (has_min_len)
    json_object_set_number(obj, "minLength", (double)min_len);
  if (has_max_len)
    json_object_set_number(obj, "maxLength", (double)max_len);
  if (pattern)
    json_object_set_string(obj, "pattern", pattern);
}

static void write_array_constraints(JSON_Object *obj, int has_min_items,
                                    size_t min_items, int has_max_items,
                                    size_t max_items, int unique_items) {
  if (!obj)
    return;
  if (has_min_items)
    json_object_set_number(obj, "minItems", (double)min_items);
  if (has_max_items)
    json_object_set_number(obj, "maxItems", (double)max_items);
  if (unique_items)
    json_object_set_boolean(obj, "uniqueItems", 1);
}

static void write_items_schema_fields(JSON_Object *item_obj,
                                      const struct OpenAPI_SchemaRef *ref) {
  if (!item_obj || !ref)
    return;

  write_numeric_constraints(item_obj, ref->items_has_min, ref->items_min_val,
                            ref->items_exclusive_min, ref->items_has_max,
                            ref->items_max_val, ref->items_exclusive_max);
  write_string_constraints(item_obj, ref->items_has_min_len, ref->items_min_len,
                           ref->items_has_max_len, ref->items_max_len,
                           ref->items_pattern);
  write_array_constraints(item_obj, ref->items_has_min_items,
                          ref->items_min_items, ref->items_has_max_items,
                          ref->items_max_items, ref->items_unique_items);
  write_schema_example(item_obj, &ref->items_example, ref->items_example_set);
  if (ref->n_items_examples > 0) {
    write_any_array_values(item_obj, "examples", ref->items_examples,
                           ref->n_items_examples);
  }
  if (ref->items_format)
    json_object_set_string(item_obj, "format", ref->items_format);
  if (ref->items_content_media_type)
    json_object_set_string(item_obj, "contentMediaType",
                           ref->items_content_media_type);
  if (ref->items_content_schema) {
    write_schema_ref(item_obj, "contentSchema", ref->items_content_schema);
  }
  if (ref->items_content_encoding)
    json_object_set_string(item_obj, "contentEncoding",
                           ref->items_content_encoding);
  if (ref->n_items_enum_values > 0)
    write_enum_any_values(item_obj, "enum", ref->items_enum_values,
                          ref->n_items_enum_values);
  if (ref->items_const_value_set) {
    JSON_Value *const_val = any_to_json_value(&ref->items_const_value);
    if (const_val)
      json_object_set_value(item_obj, "const", const_val);
  }
  if (ref->items_default_value_set) {
    JSON_Value *def_val = any_to_json_value(&ref->items_default_value);
    if (def_val)
      json_object_set_value(item_obj, "default", def_val);
  }
  if (ref->items_extra_json)
    merge_schema_extras_object(item_obj, ref->items_extra_json);
}

static const char *schema_ref_keyword(int is_dynamic) {
  return is_dynamic ? "$dynamicRef" : "$ref";
}

/**
 * @brief Write a Schema Reference object (or inline Type).
 *
 * Handles `$ref`, `type: array`, and basic types.
 * Populates `parent` at `key` (e.g. key="schema").
 */
static void write_schema_ref(JSON_Object *parent, const char *key,
                             const struct OpenAPI_SchemaRef *ref) {
  JSON_Value *sch_val;
  JSON_Object *sch_obj;
  char ref_path[256];

  if (!parent || !key || !ref)
    return;

  if (ref->schema_is_boolean) {
    json_object_set_boolean(parent, key, ref->schema_boolean_value ? 1 : 0);
    return;
  }

  sch_val = json_value_init_object();
  sch_obj = json_value_get_object(sch_val);

  /* Case 1: Built-in Multipart Fields (Inline Schema) */
  if (ref->n_multipart_fields > 0) {
    json_value_free(sch_val); /* Discard placeholder */
    write_multipart_schema(parent, key, ref);
    return;
  }

  /* Case 2: Array */
  if (ref->is_array) {
    write_schema_type_union(sch_obj, "array", ref->nullable, ref->type_union,
                            ref->n_type_union);
    if (ref->items_schema_is_boolean) {
      json_object_set_boolean(sch_obj, "items",
                              ref->items_schema_boolean_value ? 1 : 0);
    } else if (ref->inline_type) {
      JSON_Value *item_val = json_value_init_object();
      JSON_Object *item_obj = json_value_get_object(item_val);
      write_schema_type_union(item_obj, ref->inline_type, ref->items_nullable,
                              ref->items_type_union, ref->n_items_type_union);
      write_items_schema_fields(item_obj, ref);
      json_object_set_value(sch_obj, "items", item_val);
    } else if (ref->items_ref) {
      JSON_Value *item_val = json_value_init_object();
      JSON_Object *item_obj = json_value_get_object(item_val);
      json_object_set_string(item_obj,
                             schema_ref_keyword(ref->items_ref_is_dynamic),
                             ref->items_ref);
      write_items_schema_fields(item_obj, ref);
      json_object_set_value(sch_obj, "items", item_val);
    } else if (ref->ref_name) {
      /* Simple detection: if standard type, use type, else ref */
      if (is_schema_primitive(ref->ref_name)) {
        JSON_Value *item_val = json_value_init_object();
        JSON_Object *item_obj = json_value_get_object(item_val);
        write_schema_type_union(item_obj, ref->ref_name, ref->items_nullable,
                                ref->items_type_union, ref->n_items_type_union);
        write_items_schema_fields(item_obj, ref);
        json_object_set_value(sch_obj, "items", item_val);
      } else {
        JSON_Value *item_val = json_value_init_object();
        JSON_Object *item_obj = json_value_get_object(item_val);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(ref_path, sizeof(ref_path), "#/components/schemas/%s",
                  ref->ref_name);
#else
        sprintf(ref_path, "#/components/schemas/%s", ref->ref_name);
#endif
        json_object_set_string(item_obj, "$ref", ref_path);
        write_items_schema_fields(item_obj, ref);
        json_object_set_value(sch_obj, "items", item_val);
      }
    }
  }
  /* Case 3: Reference or Primitive */
  else if (ref->inline_type) {
    write_schema_type_union(sch_obj, ref->inline_type, ref->nullable,
                            ref->type_union, ref->n_type_union);
  } else if (ref->ref) {
    json_object_set_string(sch_obj, schema_ref_keyword(ref->ref_is_dynamic),
                           ref->ref);
  } else if (ref->ref_name) {
    if (is_schema_primitive(ref->ref_name)) {
      write_schema_type_union(sch_obj, ref->ref_name, ref->nullable,
                              ref->type_union, ref->n_type_union);
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

  if (ref->format)
    json_object_set_string(sch_obj, "format", ref->format);
  write_numeric_constraints(sch_obj, ref->has_min, ref->min_val,
                            ref->exclusive_min, ref->has_max, ref->max_val,
                            ref->exclusive_max);
  write_string_constraints(sch_obj, ref->has_min_len, ref->min_len,
                           ref->has_max_len, ref->max_len, ref->pattern);
  write_array_constraints(sch_obj, ref->has_min_items, ref->min_items,
                          ref->has_max_items, ref->max_items,
                          ref->unique_items);
  if (ref->content_media_type)
    json_object_set_string(sch_obj, "contentMediaType",
                           ref->content_media_type);
  if (ref->content_schema) {
    write_schema_ref(sch_obj, "contentSchema", ref->content_schema);
  }
  if (ref->content_encoding)
    json_object_set_string(sch_obj, "contentEncoding", ref->content_encoding);
  if (ref->summary && (ref->ref_name || ref->ref))
    json_object_set_string(sch_obj, "summary", ref->summary);
  if (ref->description)
    json_object_set_string(sch_obj, "description", ref->description);
  if (ref->external_docs_set)
    write_external_docs(sch_obj, "externalDocs", &ref->external_docs);
  write_discriminator_object(sch_obj, &ref->discriminator,
                             ref->discriminator_set);
  write_xml_object(sch_obj, &ref->xml, ref->xml_set);
  if (ref->deprecated_set)
    json_object_set_boolean(sch_obj, "deprecated", ref->deprecated ? 1 : 0);
  if (ref->read_only_set)
    json_object_set_boolean(sch_obj, "readOnly", ref->read_only ? 1 : 0);
  if (ref->write_only_set)
    json_object_set_boolean(sch_obj, "writeOnly", ref->write_only ? 1 : 0);
  if (ref->const_value_set) {
    JSON_Value *const_val = any_to_json_value(&ref->const_value);
    if (const_val)
      json_object_set_value(sch_obj, "const", const_val);
  }
  write_schema_example(sch_obj, &ref->example, ref->example_set);
  if (ref->n_examples > 0 && ref->examples) {
    JSON_Value *examples_val = json_value_init_array();
    JSON_Array *examples_arr = json_value_get_array(examples_val);
    size_t i;
    if (examples_arr) {
      for (i = 0; i < ref->n_examples; ++i) {
        JSON_Value *ex_val = any_to_json_value(&ref->examples[i]);
        if (ex_val)
          json_array_append_value(examples_arr, ex_val);
      }
      json_object_set_value(sch_obj, "examples", examples_val);
    } else {
      json_value_free(examples_val);
    }
  }
  if (ref->n_enum_values > 0)
    write_enum_any_values(sch_obj, "enum", ref->enum_values,
                          ref->n_enum_values);
  if (ref->default_value_set) {
    JSON_Value *def_val = any_to_json_value(&ref->default_value);
    if (def_val)
      json_object_set_value(sch_obj, "default", def_val);
  }
  if (ref->schema_extra_json)
    merge_schema_extras_object(sch_obj, ref->schema_extra_json);

  json_object_set_value(parent, key, sch_val);
}

static void write_schema_from_type_fields(JSON_Object *parent, const char *key,
                                          const char *type, int is_array,
                                          const char *items_type) {
  JSON_Value *sch_val = json_value_init_object();
  JSON_Object *sch_obj = json_value_get_object(sch_val);
  char ref_path[128];

  if (is_array) {
    json_object_set_string(sch_obj, "type", "array");
    if (items_type) {
      JSON_Value *item_val = json_value_init_object();
      JSON_Object *item_obj = json_value_get_object(item_val);
      if (is_schema_primitive(items_type)) {
        json_object_set_string(item_obj, "type", items_type);
      } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(ref_path, sizeof(ref_path), "#/components/schemas/%s",
                  items_type);
#else
        sprintf(ref_path, "#/components/schemas/%s", items_type);
#endif
        json_object_set_string(item_obj, "$ref", ref_path);
      }
      json_object_set_value(sch_obj, "items", item_val);
    }
    json_object_set_value(parent, key, sch_val);
    return;
  }

  if (type && (is_schema_primitive(type) || strcmp(type, "array") == 0)) {
    json_object_set_string(sch_obj, "type", type);
  } else if (type) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(ref_path, sizeof(ref_path), "#/components/schemas/%s", type);
#else
    sprintf(ref_path, "#/components/schemas/%s", type);
#endif
    json_object_set_string(sch_obj, "$ref", ref_path);
  } else {
    json_object_set_string(sch_obj, "type", "string");
  }

  json_object_set_value(parent, key, sch_val);
}

static void write_parameter_object(JSON_Object *p_obj,
                                   const struct OpenAPI_Parameter *p) {
  const char *in_str;
  const char *style_str;

  if (!p_obj || !p)
    return;

  if (p->ref) {
    json_object_set_string(p_obj, "$ref", p->ref);
    if (p->description)
      json_object_set_string(p_obj, "description", p->description);
    return;
  }

  in_str = param_in_to_str(p->in);
  style_str = style_to_str(p->style);

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
  if (p->allow_empty_value_set && p->in == OA_PARAM_IN_QUERY)
    json_object_set_boolean(p_obj, "allowEmptyValue",
                            p->allow_empty_value ? 1 : 0);
  if (p->example_location == OA_EXAMPLE_LOC_OBJECT) {
    write_example_fields(p_obj, &p->example, p->example_set, p->examples,
                         p->n_examples);
  }

  if (!(p->content_media_types && p->n_content_media_types > 0) &&
      !(p->content_type || p->content_ref ||
        p->in == OA_PARAM_IN_QUERYSTRING)) {
    if (style_str)
      json_object_set_string(p_obj, "style", style_str);
    if (p->explode_set)
      json_object_set_boolean(p_obj, "explode", p->explode ? 1 : 0);
    else if (p->explode)
      json_object_set_boolean(p_obj, "explode", 1);
    if (p->allow_reserved_set)
      json_object_set_boolean(p_obj, "allowReserved",
                              p->allow_reserved ? 1 : 0);
  }

  if (p->content_media_types && p->n_content_media_types > 0) {
    (void)write_media_type_map(p_obj, "content", p->content_media_types,
                               p->n_content_media_types);
  } else if (p->content_ref) {
    JSON_Value *content_val = json_value_init_object();
    JSON_Object *content_obj = json_value_get_object(content_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);
    const char *content_key = p->content_type
                                  ? p->content_type
                                  : (p->in == OA_PARAM_IN_QUERYSTRING
                                         ? "application/x-www-form-urlencoded"
                                         : "application/json");

    json_object_set_string(media_obj, "$ref", p->content_ref);
    json_object_set_value(content_obj, content_key, media_val);
    json_object_set_value(p_obj, "content", content_val);
  } else if (p->content_type || p->in == OA_PARAM_IN_QUERYSTRING) {
    JSON_Value *content_val = json_value_init_object();
    JSON_Object *content_obj = json_value_get_object(content_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);

    if (p->item_schema_set && schema_ref_has_data(&p->schema)) {
      write_schema_ref(media_obj, "itemSchema", &p->schema);
    } else if (p->schema_set && schema_ref_has_data(&p->schema)) {
      write_schema_ref(media_obj, "schema", &p->schema);
    } else if (p->type || p->is_array) {
      write_schema_from_type_fields(
          media_obj, p->item_schema_set ? "itemSchema" : "schema",
          p->type ? p->type : "string", p->is_array, p->items_type);
    }
    if (p->example_location == OA_EXAMPLE_LOC_MEDIA) {
      write_example_fields(media_obj, &p->example, p->example_set, p->examples,
                           p->n_examples);
    }

    json_object_set_value(content_obj,
                          p->content_type ? p->content_type
                                          : "application/x-www-form-urlencoded",
                          media_val);
    json_object_set_value(p_obj, "content", content_val);
  } else if (p->schema_set && schema_ref_has_data(&p->schema)) {
    write_schema_ref(p_obj, "schema", &p->schema);
  } else if (p->is_array || p->type) {
    write_schema_from_type_fields(p_obj, "schema", p->type ? p->type : "string",
                                  p->is_array, p->items_type);
  }

  if (p->extensions_json)
    merge_schema_extras_object(p_obj, p->extensions_json);
}

static void write_header_object(JSON_Object *h_obj,
                                const struct OpenAPI_Header *h) {
  const char *style_str;

  if (!h_obj || !h)
    return;

  if (h->ref) {
    json_object_set_string(h_obj, "$ref", h->ref);
    if (h->description)
      json_object_set_string(h_obj, "description", h->description);
    return;
  }

  if (h->description)
    json_object_set_string(h_obj, "description", h->description);
  if (h->required)
    json_object_set_boolean(h_obj, "required", 1);
  if (h->deprecated_set)
    json_object_set_boolean(h_obj, "deprecated", h->deprecated ? 1 : 0);
  if (h->style_set) {
    style_str = style_to_str(h->style);
    if (style_str)
      json_object_set_string(h_obj, "style", style_str);
  }
  if (h->explode_set)
    json_object_set_boolean(h_obj, "explode", h->explode ? 1 : 0);
  if (h->example_location == OA_EXAMPLE_LOC_OBJECT) {
    write_example_fields(h_obj, &h->example, h->example_set, h->examples,
                         h->n_examples);
  }

  if (h->content_media_types && h->n_content_media_types > 0) {
    (void)write_media_type_map(h_obj, "content", h->content_media_types,
                               h->n_content_media_types);
  } else if (h->content_ref) {
    JSON_Value *content_val = json_value_init_object();
    JSON_Object *content_obj = json_value_get_object(content_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);

    json_object_set_string(media_obj, "$ref", h->content_ref);
    json_object_set_value(
        content_obj, h->content_type ? h->content_type : "application/json",
        media_val);
    json_object_set_value(h_obj, "content", content_val);
  } else if (h->content_type) {
    JSON_Value *content_val = json_value_init_object();
    JSON_Object *content_obj = json_value_get_object(content_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);

    if (h->schema_set && schema_ref_has_data(&h->schema)) {
      write_schema_ref(media_obj, "schema", &h->schema);
    } else if (h->type || h->is_array) {
      write_schema_from_type_fields(media_obj, "schema",
                                    h->type ? h->type : "string", h->is_array,
                                    h->items_type);
    }
    if (h->example_location == OA_EXAMPLE_LOC_MEDIA) {
      write_example_fields(media_obj, &h->example, h->example_set, h->examples,
                           h->n_examples);
    }

    json_object_set_value(content_obj, h->content_type, media_val);
    json_object_set_value(h_obj, "content", content_val);
  } else if (h->schema_set && schema_ref_has_data(&h->schema)) {
    write_schema_ref(h_obj, "schema", &h->schema);
  } else {
    write_schema_from_type_fields(h_obj, "schema", h->type ? h->type : "string",
                                  h->is_array, h->items_type);
  }

  if (h->extensions_json)
    merge_schema_extras_object(h_obj, h->extensions_json);
}

static int write_encoding_object(JSON_Object *enc_obj,
                                 const struct OpenAPI_Encoding *enc) {
  if (!enc_obj || !enc)
    return 0;

  if (enc->content_type)
    json_object_set_string(enc_obj, "contentType", enc->content_type);
  if (enc->style_set) {
    const char *style_str = style_to_str(enc->style);
    if (style_str)
      json_object_set_string(enc_obj, "style", style_str);
  }
  if (enc->explode_set)
    json_object_set_boolean(enc_obj, "explode", enc->explode ? 1 : 0);
  if (enc->allow_reserved_set)
    json_object_set_boolean(enc_obj, "allowReserved",
                            enc->allow_reserved ? 1 : 0);
  if (enc->headers && enc->n_headers > 0) {
    (void)write_headers_map(enc_obj, "headers", enc->headers, enc->n_headers,
                            1);
  }
  if (enc->encoding && enc->n_encoding > 0) {
    if (write_encoding_map(enc_obj, enc->encoding, enc->n_encoding) != 0)
      return ENOMEM;
  }
  if (enc->prefix_encoding && enc->n_prefix_encoding > 0) {
    if (write_encoding_array(enc_obj, "prefixEncoding", enc->prefix_encoding,
                             enc->n_prefix_encoding) != 0)
      return ENOMEM;
  }
  if (enc->item_encoding && enc->item_encoding_set) {
    JSON_Value *item_val = json_value_init_object();
    JSON_Object *item_obj = json_value_get_object(item_val);
    if (!item_val)
      return ENOMEM;
    if (write_encoding_object(item_obj, enc->item_encoding) != 0) {
      json_value_free(item_val);
      return ENOMEM;
    }
    json_object_set_value(enc_obj, "itemEncoding", item_val);
  }

  if (enc->extensions_json)
    merge_schema_extras_object(enc_obj, enc->extensions_json);

  return 0;
}

static int write_encoding_map(JSON_Object *media_obj,
                              const struct OpenAPI_Encoding *encoding,
                              size_t n_encoding) {
  JSON_Value *enc_val;
  JSON_Object *enc_obj;
  size_t i;

  if (!media_obj || !encoding || n_encoding == 0)
    return 0;

  enc_val = json_value_init_object();
  if (!enc_val)
    return ENOMEM;
  enc_obj = json_value_get_object(enc_val);

  for (i = 0; i < n_encoding; ++i) {
    const struct OpenAPI_Encoding *enc = &encoding[i];
    JSON_Value *e_val = json_value_init_object();
    JSON_Object *e_obj = json_value_get_object(e_val);
    const char *name = enc->name ? enc->name : "encoding";

    if (!e_val)
      return ENOMEM;
    if (write_encoding_object(e_obj, enc) != 0) {
      json_value_free(e_val);
      return ENOMEM;
    }
    json_object_set_value(enc_obj, name, e_val);
  }

  json_object_set_value(media_obj, "encoding", enc_val);
  return 0;
}

static int write_encoding_array(JSON_Object *parent, const char *key,
                                const struct OpenAPI_Encoding *encoding,
                                size_t n_encoding) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (!parent || !key || !encoding || n_encoding == 0)
    return 0;

  arr_val = json_value_init_array();
  if (!arr_val)
    return ENOMEM;
  arr = json_value_get_array(arr_val);

  for (i = 0; i < n_encoding; ++i) {
    JSON_Value *e_val = json_value_init_object();
    JSON_Object *e_obj = json_value_get_object(e_val);
    if (!e_val)
      return ENOMEM;
    if (write_encoding_object(e_obj, &encoding[i]) != 0) {
      json_value_free(e_val);
      return ENOMEM;
    }
    json_array_append_value(arr, e_val);
  }

  json_object_set_value(parent, key, arr_val);
  return 0;
}

static int write_media_type_object(JSON_Object *media_obj,
                                   const struct OpenAPI_MediaType *mt) {
  if (!media_obj || !mt)
    return 0;
  if (mt->ref) {
    json_object_set_string(media_obj, "$ref", mt->ref);
    return 0;
  }
  if (mt->schema_set || schema_ref_has_data(&mt->schema)) {
    write_schema_ref(media_obj, "schema", &mt->schema);
  }
  if (mt->item_schema_set || schema_ref_has_data(&mt->item_schema)) {
    write_schema_ref(media_obj, "itemSchema", &mt->item_schema);
  }
  write_example_fields(media_obj, &mt->example, mt->example_set, mt->examples,
                       mt->n_examples);
  if (mt->encoding && mt->n_encoding > 0) {
    if (write_encoding_map(media_obj, mt->encoding, mt->n_encoding) != 0)
      return ENOMEM;
  }
  if (mt->prefix_encoding && mt->n_prefix_encoding > 0) {
    if (write_encoding_array(media_obj, "prefixEncoding", mt->prefix_encoding,
                             mt->n_prefix_encoding) != 0)
      return ENOMEM;
  }
  if (mt->item_encoding && mt->item_encoding_set) {
    JSON_Value *item_val = json_value_init_object();
    JSON_Object *item_obj = json_value_get_object(item_val);
    if (!item_val)
      return ENOMEM;
    if (write_encoding_object(item_obj, mt->item_encoding) != 0) {
      json_value_free(item_val);
      return ENOMEM;
    }
    json_object_set_value(media_obj, "itemEncoding", item_val);
  }
  if (mt->extensions_json)
    merge_schema_extras_object(media_obj, mt->extensions_json);
  return 0;
}

static int write_media_type_map(JSON_Object *parent, const char *key,
                                const struct OpenAPI_MediaType *mts,
                                size_t n_mts) {
  JSON_Value *content_val;
  JSON_Object *content_obj;
  size_t i;

  if (!parent || !key || !mts || n_mts == 0)
    return 0;

  content_val = json_value_init_object();
  if (!content_val)
    return ENOMEM;
  content_obj = json_value_get_object(content_val);

  for (i = 0; i < n_mts; ++i) {
    const struct OpenAPI_MediaType *mt = &mts[i];
    const char *name = mt->name ? mt->name : "application/json";
    JSON_Value *mt_val = json_value_init_object();
    JSON_Object *mt_obj = json_value_get_object(mt_val);

    if (write_media_type_object(mt_obj, mt) != 0) {
      json_value_free(mt_val);
      json_value_free(content_val);
      return ENOMEM;
    }
    json_object_set_value(content_obj, name, mt_val);
  }

  json_object_set_value(parent, key, content_val);
  return 0;
}

static void write_link_object(JSON_Object *l_obj,
                              const struct OpenAPI_Link *link) {
  if (!l_obj || !link)
    return;

  if (link->ref) {
    json_object_set_string(l_obj, "$ref", link->ref);
    if (link->summary)
      json_object_set_string(l_obj, "summary", link->summary);
    if (link->description)
      json_object_set_string(l_obj, "description", link->description);
    return;
  }

  if (link->operation_ref)
    json_object_set_string(l_obj, "operationRef", link->operation_ref);
  if (link->operation_id)
    json_object_set_string(l_obj, "operationId", link->operation_id);
  if (link->description)
    json_object_set_string(l_obj, "description", link->description);

  if (link->n_parameters > 0 && link->parameters) {
    JSON_Value *params_val = json_value_init_object();
    JSON_Object *params_obj = json_value_get_object(params_val);
    size_t i;
    for (i = 0; i < link->n_parameters; ++i) {
      const struct OpenAPI_LinkParam *param = &link->parameters[i];
      JSON_Value *val = any_to_json_value(&param->value);
      if (val && param->name) {
        json_object_set_value(params_obj, param->name, val);
      } else if (val) {
        json_value_free(val);
      }
    }
    json_object_set_value(l_obj, "parameters", params_val);
  }

  if (link->request_body_set) {
    JSON_Value *rb_val = any_to_json_value(&link->request_body);
    if (rb_val)
      json_object_set_value(l_obj, "requestBody", rb_val);
  }

  if (link->server_set && link->server) {
    JSON_Value *srv_val = json_value_init_object();
    JSON_Object *srv_obj = json_value_get_object(srv_val);
    write_server_object(srv_obj, link->server);
    json_object_set_value(l_obj, "server", srv_val);
  }

  if (link->extensions_json)
    merge_schema_extras_object(l_obj, link->extensions_json);
}

static int write_headers_map(JSON_Object *parent, const char *key,
                             const struct OpenAPI_Header *headers,
                             size_t n_headers, int ignore_content_type) {
  JSON_Value *headers_val;
  JSON_Object *headers_obj;
  size_t i;
  size_t written = 0;

  if (!parent || !key || !headers || n_headers == 0)
    return 0;

  headers_val = json_value_init_object();
  if (!headers_val)
    return ENOMEM;
  headers_obj = json_value_get_object(headers_val);

  for (i = 0; i < n_headers; ++i) {
    const struct OpenAPI_Header *h = &headers[i];
    const char *name = h->name ? h->name : "header";
    JSON_Value *h_val;
    JSON_Object *h_obj;

    if (ignore_content_type && header_name_is_content_type(name))
      continue;
    h_val = json_value_init_object();
    h_obj = json_value_get_object(h_val);
    write_header_object(h_obj, h);
    json_object_set_value(headers_obj, name, h_val);
    written++;
  }

  if (written == 0) {
    json_value_free(headers_val);
    return 0;
  }
  json_object_set_value(parent, key, headers_val);
  return 0;
}

static int write_headers(JSON_Object *parent,
                         const struct OpenAPI_Response *resp) {
  if (!parent || !resp || resp->n_headers == 0 || !resp->headers)
    return 0;
  return write_headers_map(parent, "headers", resp->headers, resp->n_headers,
                           1);
}

static int write_links(JSON_Object *parent,
                       const struct OpenAPI_Response *resp) {
  JSON_Value *links_val;
  JSON_Object *links_obj;
  size_t i;

  if (!parent || !resp || resp->n_links == 0 || !resp->links)
    return 0;

  links_val = json_value_init_object();
  if (!links_val)
    return ENOMEM;
  links_obj = json_value_get_object(links_val);

  for (i = 0; i < resp->n_links; ++i) {
    const struct OpenAPI_Link *link = &resp->links[i];
    JSON_Value *l_val = json_value_init_object();
    JSON_Object *l_obj = json_value_get_object(l_val);
    const char *name = link->name ? link->name : "link";

    write_link_object(l_obj, link);
    json_object_set_value(links_obj, name, l_val);
  }

  json_object_set_value(parent, "links", links_val);
  return 0;
}

static void write_response_object(JSON_Object *r_obj,
                                  const struct OpenAPI_Response *resp) {
  if (!r_obj || !resp)
    return;

  if (resp->ref) {
    json_object_set_string(r_obj, "$ref", resp->ref);
    if (resp->summary)
      json_object_set_string(r_obj, "summary", resp->summary);
    if (resp->description)
      json_object_set_string(r_obj, "description", resp->description);
    return;
  }

  if (resp->summary)
    json_object_set_string(r_obj, "summary", resp->summary);
  json_object_set_string(r_obj, "description",
                         resp->description ? resp->description : "");

  if (resp->headers && resp->n_headers > 0) {
    write_headers(r_obj, resp);
  }
  if (resp->links && resp->n_links > 0) {
    write_links(r_obj, resp);
  }

  if (resp->content_media_types && resp->n_content_media_types > 0) {
    (void)write_media_type_map(r_obj, "content", resp->content_media_types,
                               resp->n_content_media_types);
  } else if (resp->content_ref) {
    JSON_Value *cont_val = json_value_init_object();
    JSON_Object *cont_obj = json_value_get_object(cont_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);

    json_object_set_string(media_obj, "$ref", resp->content_ref);
    json_object_set_value(
        cont_obj, resp->content_type ? resp->content_type : "application/json",
        media_val);
    json_object_set_value(r_obj, "content", cont_val);
  } else if (schema_ref_has_data(&resp->schema) || resp->content_type) {
    JSON_Value *cont_val = json_value_init_object();
    JSON_Object *cont_obj = json_value_get_object(cont_val);
    JSON_Value *media_val = json_value_init_object();
    JSON_Object *media_obj = json_value_get_object(media_val);

    if (schema_ref_has_data(&resp->schema)) {
      write_schema_ref(media_obj, "schema", &resp->schema);
    }
    write_example_fields(media_obj, &resp->example, resp->example_set,
                         resp->examples, resp->n_examples);

    json_object_set_value(
        cont_obj, resp->content_type ? resp->content_type : "application/json",
        media_val);
    json_object_set_value(r_obj, "content", cont_val);
  }

  if (resp->extensions_json)
    merge_schema_extras_object(r_obj, resp->extensions_json);
}

static int write_parameters(JSON_Object *parent,
                            const struct OpenAPI_Parameter *params,
                            size_t n_params) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;
  size_t written = 0;

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
    if (param_is_reserved_header(p)) {
      json_value_free(p_val);
      continue;
    }
    write_parameter_object(p_obj, p);

    json_array_append_value(arr, p_val);
    written++;
  }

  if (written == 0) {
    json_value_free(arr_val);
    return 0;
  }
  json_object_set_value(parent, "parameters", arr_val);
  return 0;
}

static int write_request_body_object(JSON_Object *rb_obj,
                                     const struct OpenAPI_RequestBody *rb) {
  JSON_Value *content_val;
  JSON_Object *content_obj;
  JSON_Value *media_val;
  JSON_Object *media_obj;

  if (!rb_obj || !rb)
    return 0;

  if (rb->content_media_types && rb->n_content_media_types > 0) {
    if (write_media_type_map(rb_obj, "content", rb->content_media_types,
                             rb->n_content_media_types) != 0)
      return ENOMEM;
  } else if (rb->content_ref) {
    content_val = json_value_init_object();
    if (!content_val)
      return ENOMEM;
    content_obj = json_value_get_object(content_val);

    media_val = json_value_init_object();
    if (!media_val) {
      json_value_free(content_val);
      return ENOMEM;
    }
    media_obj = json_value_get_object(media_val);

    json_object_set_string(media_obj, "$ref", rb->content_ref);
    json_object_set_value(content_obj,
                          rb->schema.content_type ? rb->schema.content_type
                                                  : "application/json",
                          media_val);
    json_object_set_value(rb_obj, "content", content_val);
  } else {
    content_val = json_value_init_object();
    if (!content_val)
      return ENOMEM;
    content_obj = json_value_get_object(content_val);

    media_val = json_value_init_object();
    if (!media_val) {
      json_value_free(content_val);
      return ENOMEM;
    }
    media_obj = json_value_get_object(media_val);

    write_schema_ref(media_obj, "schema", &rb->schema);
    write_example_fields(media_obj, &rb->example, rb->example_set, rb->examples,
                         rb->n_examples);

    json_object_set_value(content_obj,
                          rb->schema.content_type ? rb->schema.content_type
                                                  : "application/json",
                          media_val);
    json_object_set_value(rb_obj, "content", content_val);
  }

  if (rb->description) {
    json_object_set_string(rb_obj, "description", rb->description);
  }
  if (rb->required_set) {
    json_object_set_boolean(rb_obj, "required", rb->required ? 1 : 0);
  }

  if (rb->extensions_json)
    merge_schema_extras_object(rb_obj, rb->extensions_json);

  return 0;
}

static int write_request_body(JSON_Object *op_obj,
                              const struct OpenAPI_Operation *op) {
  JSON_Value *rb_val;
  JSON_Object *rb_obj;

  if (!op_obj || !op)
    return 0;

  if (op->req_body_ref) {
    rb_val = json_value_init_object();
    if (!rb_val)
      return ENOMEM;
    rb_obj = json_value_get_object(rb_val);
    json_object_set_string(rb_obj, "$ref", op->req_body_ref);
    if (op->req_body_description)
      json_object_set_string(rb_obj, "description", op->req_body_description);
    if (op->req_body_extensions_json)
      merge_schema_extras_object(rb_obj, op->req_body_extensions_json);
    json_object_set_value(op_obj, "requestBody", rb_val);
    return 0;
  }

  /* If body is empty and no fields, skip */
  if (!schema_ref_has_data(&op->req_body) &&
      op->req_body.content_type == NULL && op->n_req_body_media_types == 0) {
    return 0;
  }

  rb_val = json_value_init_object();
  if (!rb_val)
    return ENOMEM;
  rb_obj = json_value_get_object(rb_val);

  {
    struct OpenAPI_RequestBody rb = {0};
    rb.description = op->req_body_description;
    rb.required = op->req_body_required;
    rb.required_set = op->req_body_required_set;
    rb.schema = op->req_body;
    rb.content_media_types = op->req_body_media_types;
    rb.n_content_media_types = op->n_req_body_media_types;
    rb.extensions_json = op->req_body_extensions_json;
    if (write_request_body_object(rb_obj, &rb) != 0) {
      json_value_free(rb_val);
      return ENOMEM;
    }
  }

  json_object_set_value(op_obj, "requestBody", rb_val);
  return 0;
}

static void write_callback_object(JSON_Object *cb_obj,
                                  const struct OpenAPI_Callback *cb) {
  size_t i;
  int rc;

  if (!cb_obj || !cb)
    return;

  if (cb->ref) {
    json_object_set_string(cb_obj, "$ref", cb->ref);
    if (cb->summary)
      json_object_set_string(cb_obj, "summary", cb->summary);
    if (cb->description)
      json_object_set_string(cb_obj, "description", cb->description);
    return;
  }

  if (cb->paths && cb->n_paths > 0) {
    for (i = 0; i < cb->n_paths; ++i) {
      const struct OpenAPI_Path *p = &cb->paths[i];
      const char *route = p->route ? p->route : "callback";
      JSON_Value *item_val = json_value_init_object();
      JSON_Object *item_obj = json_value_get_object(item_val);

      rc = write_path_item_object(item_obj, p);
      if (rc != 0) {
        json_value_free(item_val);
        continue;
      }

      json_object_set_value(cb_obj, route, item_val);
    }
  }

  if (cb->extensions_json)
    merge_schema_extras_object(cb_obj, cb->extensions_json);
}

static int write_callbacks(JSON_Object *op_obj,
                           const struct OpenAPI_Operation *op) {
  JSON_Value *cbs_val;
  JSON_Object *cbs_obj;
  size_t i;

  if (!op_obj || !op || op->n_callbacks == 0 || !op->callbacks)
    return 0;

  cbs_val = json_value_init_object();
  if (!cbs_val)
    return ENOMEM;
  cbs_obj = json_value_get_object(cbs_val);

  for (i = 0; i < op->n_callbacks; ++i) {
    const struct OpenAPI_Callback *cb = &op->callbacks[i];
    const char *name = cb->name ? cb->name : "callback";
    JSON_Value *cb_val = json_value_init_object();
    JSON_Object *cb_obj = json_value_get_object(cb_val);

    write_callback_object(cb_obj, cb);
    json_object_set_value(cbs_obj, name, cb_val);
  }

  json_object_set_value(op_obj, "callbacks", cbs_val);
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

    write_response_object(r_obj, r);

    json_object_set_value(resps_obj, r->code ? r->code : "default", r_val);
  }
  if (op->responses_extensions_json)
    merge_schema_extras_object(resps_obj, op->responses_extensions_json);

  json_object_set_value(op_obj, "responses", resps_val);
  return 0;
}

static int write_operation_object(JSON_Object *op_obj,
                                  const struct OpenAPI_Operation *op) {
  int rc;

  if (!op_obj || !op)
    return 0;

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
    return rc;
  }

  rc = write_request_body(op_obj, op);
  if (rc != 0) {
    return rc;
  }

  rc = write_responses(op_obj, op);
  if (rc != 0) {
    return rc;
  }

  rc = write_callbacks(op_obj, op);
  if (rc != 0) {
    return rc;
  }

  if (op->n_servers > 0 && op->servers) {
    rc = write_server_array(op_obj, "servers", op->servers, op->n_servers);
    if (rc != 0) {
      return rc;
    }
  }

  if (op->extensions_json)
    merge_schema_extras_object(op_obj, op->extensions_json);

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

    rc = write_operation_object(op_obj, op);
    if (rc != 0) {
      json_value_free(op_val);
      return rc;
    }

    json_object_set_value(path_item, verb, op_val);
  }
  return 0;
}

static int write_additional_operations(JSON_Object *path_item,
                                       const struct OpenAPI_Path *path) {
  JSON_Value *add_val;
  JSON_Object *add_obj;
  size_t i;
  int rc;

  if (!path_item || !path || path->n_additional_operations == 0 ||
      !path->additional_operations)
    return 0;

  add_val = json_value_init_object();
  if (!add_val)
    return ENOMEM;
  add_obj = json_value_get_object(add_val);

  for (i = 0; i < path->n_additional_operations; ++i) {
    const struct OpenAPI_Operation *op = &path->additional_operations[i];
    const char *method = op->method ? op->method : verb_to_str(op->verb);
    JSON_Value *op_val;
    JSON_Object *op_obj;

    if (!method)
      continue;

    op_val = json_value_init_object();
    op_obj = json_value_get_object(op_val);

    rc = write_operation_object(op_obj, op);
    if (rc != 0) {
      json_value_free(op_val);
      json_value_free(add_val);
      return rc;
    }

    json_object_set_value(add_obj, method, op_val);
  }

  json_object_set_value(path_item, "additionalOperations", add_val);
  return 0;
}

static int write_path_item_object(JSON_Object *item_obj,
                                  const struct OpenAPI_Path *path) {
  int rc;

  if (!item_obj || !path)
    return 0;

  if (path->summary)
    json_object_set_string(item_obj, "summary", path->summary);
  if (path->description)
    json_object_set_string(item_obj, "description", path->description);
  if (path->ref)
    json_object_set_string(item_obj, "$ref", path->ref);
  if (path->n_parameters > 0) {
    rc = write_parameters(item_obj, path->parameters, path->n_parameters);
    if (rc != 0)
      return rc;
  }
  if (path->n_servers > 0 && path->servers) {
    rc =
        write_server_array(item_obj, "servers", path->servers, path->n_servers);
    if (rc != 0)
      return rc;
  }

  rc = write_operations(item_obj, path);
  if (rc != 0)
    return rc;

  rc = write_additional_operations(item_obj, path);
  if (rc != 0)
    return rc;

  if (path->extensions_json)
    merge_schema_extras_object(item_obj, path->extensions_json);

  return 0;
}

static int write_paths(JSON_Object *root_obj, const struct OpenAPI_Spec *spec) {
  JSON_Value *paths_val = json_value_init_object();
  JSON_Object *paths_obj = json_value_get_object(paths_val);
  size_t i;
  int rc = 0;

  if (spec && spec->paths_extensions_json) {
    merge_schema_extras_object(paths_obj, spec->paths_extensions_json);
  }

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

    rc = write_path_item_object(item_obj, p);
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

static int write_servers(JSON_Object *root_obj,
                         const struct OpenAPI_Spec *spec) {
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
    if (server_url_has_query_or_fragment(srv->url)) {
      json_value_free(arr_val);
      return EINVAL;
    }
    JSON_Value *srv_val = json_value_init_object();
    JSON_Object *srv_obj = json_value_get_object(srv_val);
    write_server_object(srv_obj, srv);

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
    if (tag->extensions_json)
      merge_schema_extras_object(tag_obj, tag->extensions_json);

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

  if (!spec)
    return 0;
  if ((spec->n_webhooks == 0 || !spec->webhooks) &&
      !spec->webhooks_extensions_json)
    return 0;

  hooks_val = json_value_init_object();
  if (!hooks_val)
    return ENOMEM;
  hooks_obj = json_value_get_object(hooks_val);

  if (spec->webhooks_extensions_json) {
    merge_schema_extras_object(hooks_obj, spec->webhooks_extensions_json);
  }

  for (i = 0; i < spec->n_webhooks; ++i) {
    const struct OpenAPI_Path *p = &spec->webhooks[i];
    const char *route = p->route ? p->route : "webhook";
    JSON_Value *item_val = json_value_init_object();
    JSON_Object *item_obj = json_value_get_object(item_val);

    rc = write_path_item_object(item_obj, p);
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

static int
write_security_requirements(JSON_Object *parent, const char *key,
                            const struct OpenAPI_SecurityRequirementSet *sets,
                            size_t count, int set_flag) {
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

    if (set->extensions_json)
      merge_schema_extras_object(set_obj, set->extensions_json);

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

    if (s->description)
      json_object_set_string(s_obj, "description", s->description);
    if (s->deprecated_set)
      json_object_set_boolean(s_obj, "deprecated", s->deprecated ? 1 : 0);

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

    case OA_SEC_MUTUALTLS:
      json_object_set_string(s_obj, "type", "mutualTLS");
      break;

    case OA_SEC_OAUTH2:
      json_object_set_string(s_obj, "type", "oauth2");
      if (s->oauth2_metadata_url) {
        json_object_set_string(s_obj, "oauth2MetadataUrl",
                               s->oauth2_metadata_url);
      }
      if (s->flows && s->n_flows > 0) {
        JSON_Value *flows_val = json_value_init_object();
        JSON_Object *flows_obj = json_value_get_object(flows_val);
        size_t f;
        for (f = 0; f < s->n_flows; ++f) {
          const struct OpenAPI_OAuthFlow *flow = &s->flows[f];
          const char *flow_key = oauth_flow_type_to_str(flow->type);
          JSON_Value *flow_val;
          JSON_Object *flow_obj;
          JSON_Value *scopes_val;
          JSON_Object *scopes_obj;
          size_t sc;
          if (!flow_key)
            continue;
          flow_val = json_value_init_object();
          flow_obj = json_value_get_object(flow_val);
          if (flow->authorization_url)
            json_object_set_string(flow_obj, "authorizationUrl",
                                   flow->authorization_url);
          if (flow->token_url)
            json_object_set_string(flow_obj, "tokenUrl", flow->token_url);
          if (flow->refresh_url)
            json_object_set_string(flow_obj, "refreshUrl", flow->refresh_url);
          if (flow->device_authorization_url)
            json_object_set_string(flow_obj, "deviceAuthorizationUrl",
                                   flow->device_authorization_url);
          scopes_val = json_value_init_object();
          scopes_obj = json_value_get_object(scopes_val);
          if (flow->scopes) {
            for (sc = 0; sc < flow->n_scopes; ++sc) {
              const char *scope_name = flow->scopes[sc].name;
              const char *scope_desc = flow->scopes[sc].description;
              if (scope_name)
                json_object_set_string(scopes_obj, scope_name,
                                       scope_desc ? scope_desc : "");
            }
          }
          json_object_set_value(flow_obj, "scopes", scopes_val);
          if (flow->extensions_json)
            merge_schema_extras_object(flow_obj, flow->extensions_json);
          json_object_set_value(flows_obj, flow_key, flow_val);
        }
        json_object_set_value(s_obj, "flows", flows_val);
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

    if (s->extensions_json)
      merge_schema_extras_object(s_obj, s->extensions_json);

    json_object_set_value(sec_obj, s->name ? s->name : "unknown", s_val);
  }

  json_object_set_value(components, "securitySchemes", sec_val);
  return 0;
}

static int write_component_parameters(JSON_Object *components,
                                      const struct OpenAPI_Spec *spec) {
  JSON_Value *params_val;
  JSON_Object *params_obj;
  size_t i;

  if (!spec || spec->n_component_parameters == 0)
    return 0;

  params_val = json_value_init_object();
  if (!params_val)
    return ENOMEM;
  params_obj = json_value_get_object(params_val);

  for (i = 0; i < spec->n_component_parameters; ++i) {
    const char *name = spec->component_parameter_names[i];
    const struct OpenAPI_Parameter *param = &spec->component_parameters[i];
    JSON_Value *p_val = json_value_init_object();
    JSON_Object *p_obj = json_value_get_object(p_val);
    if (!name) {
      json_value_free(p_val);
      continue;
    }
    write_parameter_object(p_obj, param);
    json_object_set_value(params_obj, name, p_val);
  }

  json_object_set_value(components, "parameters", params_val);
  return 0;
}

static int write_component_responses(JSON_Object *components,
                                     const struct OpenAPI_Spec *spec) {
  JSON_Value *resp_val;
  JSON_Object *resp_obj;
  size_t i;

  if (!spec || spec->n_component_responses == 0)
    return 0;

  resp_val = json_value_init_object();
  if (!resp_val)
    return ENOMEM;
  resp_obj = json_value_get_object(resp_val);

  for (i = 0; i < spec->n_component_responses; ++i) {
    const char *name = spec->component_response_names[i];
    const struct OpenAPI_Response *resp = &spec->component_responses[i];
    JSON_Value *r_val = json_value_init_object();
    JSON_Object *r_obj = json_value_get_object(r_val);
    if (!name) {
      json_value_free(r_val);
      continue;
    }
    write_response_object(r_obj, resp);
    json_object_set_value(resp_obj, name, r_val);
  }

  json_object_set_value(components, "responses", resp_val);
  return 0;
}

static int write_component_headers(JSON_Object *components,
                                   const struct OpenAPI_Spec *spec) {
  JSON_Value *hdrs_val;
  JSON_Object *hdrs_obj;
  size_t i;

  if (!spec || spec->n_component_headers == 0)
    return 0;

  hdrs_val = json_value_init_object();
  if (!hdrs_val)
    return ENOMEM;
  hdrs_obj = json_value_get_object(hdrs_val);

  for (i = 0; i < spec->n_component_headers; ++i) {
    const char *name = spec->component_header_names[i];
    const struct OpenAPI_Header *hdr = &spec->component_headers[i];
    JSON_Value *h_val = json_value_init_object();
    JSON_Object *h_obj = json_value_get_object(h_val);
    if (!name) {
      json_value_free(h_val);
      continue;
    }
    write_header_object(h_obj, hdr);
    json_object_set_value(hdrs_obj, name, h_val);
  }

  json_object_set_value(components, "headers", hdrs_val);
  return 0;
}

static int write_component_media_types(JSON_Object *components,
                                       const struct OpenAPI_Spec *spec) {
  JSON_Value *media_val;
  JSON_Object *media_obj;
  size_t i;

  if (!spec || spec->n_component_media_types == 0)
    return 0;

  media_val = json_value_init_object();
  if (!media_val)
    return ENOMEM;
  media_obj = json_value_get_object(media_val);

  for (i = 0; i < spec->n_component_media_types; ++i) {
    const char *name = spec->component_media_type_names[i];
    const struct OpenAPI_MediaType *mt = &spec->component_media_types[i];
    JSON_Value *mt_val = json_value_init_object();
    JSON_Object *mt_obj = json_value_get_object(mt_val);

    if (!name) {
      json_value_free(mt_val);
      continue;
    }
    if (write_media_type_object(mt_obj, mt) != 0) {
      json_value_free(mt_val);
      return ENOMEM;
    }

    json_object_set_value(media_obj, name, mt_val);
  }

  json_object_set_value(components, "mediaTypes", media_val);
  return 0;
}

static int write_component_examples(JSON_Object *components,
                                    const struct OpenAPI_Spec *spec) {
  JSON_Value *examples_val;
  JSON_Object *examples_obj;
  size_t i;

  if (!spec || spec->n_component_examples == 0 || !spec->component_examples)
    return 0;

  examples_val = json_value_init_object();
  if (!examples_val)
    return ENOMEM;
  examples_obj = json_value_get_object(examples_val);

  for (i = 0; i < spec->n_component_examples; ++i) {
    const char *name = spec->component_example_names[i];
    const struct OpenAPI_Example *ex = &spec->component_examples[i];
    JSON_Value *ex_val;
    JSON_Object *ex_obj;
    if (!name)
      continue;
    ex_val = json_value_init_object();
    ex_obj = json_value_get_object(ex_val);
    write_example_object(ex_obj, ex);
    json_object_set_value(examples_obj, name, ex_val);
  }

  json_object_set_value(components, "examples", examples_val);
  return 0;
}

static int write_component_links(JSON_Object *components,
                                 const struct OpenAPI_Spec *spec) {
  JSON_Value *links_val;
  JSON_Object *links_obj;
  size_t i;

  if (!spec || spec->n_component_links == 0 || !spec->component_links)
    return 0;

  links_val = json_value_init_object();
  if (!links_val)
    return ENOMEM;
  links_obj = json_value_get_object(links_val);

  for (i = 0; i < spec->n_component_links; ++i) {
    const struct OpenAPI_Link *link = &spec->component_links[i];
    const char *name = link->name ? link->name : "link";
    JSON_Value *l_val = json_value_init_object();
    JSON_Object *l_obj = json_value_get_object(l_val);

    write_link_object(l_obj, link);
    json_object_set_value(links_obj, name, l_val);
  }

  json_object_set_value(components, "links", links_val);
  return 0;
}

static int write_component_callbacks(JSON_Object *components,
                                     const struct OpenAPI_Spec *spec) {
  JSON_Value *cbs_val;
  JSON_Object *cbs_obj;
  size_t i;

  if (!spec || spec->n_component_callbacks == 0 || !spec->component_callbacks)
    return 0;

  cbs_val = json_value_init_object();
  if (!cbs_val)
    return ENOMEM;
  cbs_obj = json_value_get_object(cbs_val);

  for (i = 0; i < spec->n_component_callbacks; ++i) {
    const struct OpenAPI_Callback *cb = &spec->component_callbacks[i];
    const char *name = cb->name ? cb->name : "callback";
    JSON_Value *cb_val = json_value_init_object();
    JSON_Object *cb_obj = json_value_get_object(cb_val);

    write_callback_object(cb_obj, cb);
    json_object_set_value(cbs_obj, name, cb_val);
  }

  json_object_set_value(components, "callbacks", cbs_val);
  return 0;
}

static int write_component_path_items(JSON_Object *components,
                                      const struct OpenAPI_Spec *spec) {
  JSON_Value *paths_val;
  JSON_Object *paths_obj;
  size_t i;
  int rc;

  if (!spec || spec->n_component_path_items == 0)
    return 0;

  paths_val = json_value_init_object();
  if (!paths_val)
    return ENOMEM;
  paths_obj = json_value_get_object(paths_val);

  for (i = 0; i < spec->n_component_path_items; ++i) {
    const struct OpenAPI_Path *p = &spec->component_path_items[i];
    const char *name = spec->component_path_item_names
                           ? spec->component_path_item_names[i]
                           : p->route;
    JSON_Value *item_val = json_value_init_object();
    JSON_Object *item_obj = json_value_get_object(item_val);

    if (!name) {
      json_value_free(item_val);
      continue;
    }

    rc = write_path_item_object(item_obj, p);
    if (rc != 0) {
      json_value_free(item_val);
      json_value_free(paths_val);
      return rc;
    }

    json_object_set_value(paths_obj, name, item_val);
  }

  json_object_set_value(components, "pathItems", paths_val);
  return 0;
}

static int write_component_request_bodies(JSON_Object *components,
                                          const struct OpenAPI_Spec *spec) {
  JSON_Value *rbs_val;
  JSON_Object *rbs_obj;
  size_t i;

  if (spec->n_component_request_bodies == 0)
    return 0;

  rbs_val = json_value_init_object();
  if (!rbs_val)
    return ENOMEM;
  rbs_obj = json_value_get_object(rbs_val);

  for (i = 0; i < spec->n_component_request_bodies; ++i) {
    const char *name = spec->component_request_body_names[i];
    const struct OpenAPI_RequestBody *rb = &spec->component_request_bodies[i];
    JSON_Value *rb_val = json_value_init_object();
    JSON_Object *rb_obj = json_value_get_object(rb_val);
    if (!name) {
      json_value_free(rb_val);
      continue;
    }
    if (rb->ref) {
      json_object_set_string(rb_obj, "$ref", rb->ref);
    } else {
      if (write_request_body_object(rb_obj, rb) != 0) {
        json_value_free(rb_val);
        json_value_free(rbs_val);
        return ENOMEM;
      }
    }
    json_object_set_value(rbs_obj, name, rb_val);
  }

  json_object_set_value(components, "requestBodies", rbs_val);
  return 0;
}

static int write_components(JSON_Object *root_obj,
                            const struct OpenAPI_Spec *spec) {
  JSON_Value *comps_val;
  JSON_Object *comps_obj;
  int rc;

  /* Only create components block if there is something to write */
  if (spec->n_defined_schemas == 0 && spec->n_raw_schemas == 0 &&
      spec->n_security_schemes == 0 && spec->n_component_parameters == 0 &&
      spec->n_component_responses == 0 && spec->n_component_headers == 0 &&
      spec->n_component_request_bodies == 0 &&
      spec->n_component_media_types == 0 && spec->n_component_examples == 0 &&
      spec->n_component_links == 0 && spec->n_component_callbacks == 0 &&
      spec->n_component_path_items == 0 && !spec->components_extensions_json) {
    return 0;
  }

  comps_val = json_value_init_object();
  if (!comps_val)
    return ENOMEM;
  comps_obj = json_value_get_object(comps_val);

  if (spec->components_extensions_json)
    merge_schema_extras_object(comps_obj, spec->components_extensions_json);

  /* Schemas */
  if (spec->n_defined_schemas > 0 || spec->n_raw_schemas > 0) {
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

    for (i = 0; i < spec->n_raw_schemas; ++i) {
      JSON_Value *raw_val;
      if (!spec->raw_schema_names[i] || !spec->raw_schema_json[i])
        continue;
      raw_val = json_parse_string(spec->raw_schema_json[i]);
      if (!raw_val) {
        json_value_free(comps_val);
        json_value_free(schemas_val);
        return EINVAL;
      }
      json_object_set_value(schemas_obj, spec->raw_schema_names[i], raw_val);
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

  /* Parameters */
  rc = write_component_parameters(comps_obj, spec);
  if (rc != 0) {
    json_value_free(comps_val);
    return rc;
  }

  /* Responses */
  rc = write_component_responses(comps_obj, spec);
  if (rc != 0) {
    json_value_free(comps_val);
    return rc;
  }

  /* Headers */
  rc = write_component_headers(comps_obj, spec);
  if (rc != 0) {
    json_value_free(comps_val);
    return rc;
  }

  /* Request Bodies */
  rc = write_component_request_bodies(comps_obj, spec);
  if (rc != 0) {
    json_value_free(comps_val);
    return rc;
  }

  /* Media Types */
  rc = write_component_media_types(comps_obj, spec);
  if (rc != 0) {
    json_value_free(comps_val);
    return rc;
  }

  /* Examples */
  rc = write_component_examples(comps_obj, spec);
  if (rc != 0) {
    json_value_free(comps_val);
    return rc;
  }

  /* Links */
  rc = write_component_links(comps_obj, spec);
  if (rc != 0) {
    json_value_free(comps_val);
    return rc;
  }

  /* Callbacks */
  rc = write_component_callbacks(comps_obj, spec);
  if (rc != 0) {
    json_value_free(comps_val);
    return rc;
  }

  /* Path Items */
  rc = write_component_path_items(comps_obj, spec);
  if (rc != 0) {
    json_value_free(comps_val);
    return rc;
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
  if (spec->is_schema_document) {
    if (!spec->schema_root_json)
      return EINVAL;
    *json_out = c_cdd_strdup(spec->schema_root_json);
    return *json_out ? 0 : ENOMEM;
  }
  if (license_fields_invalid(&spec->info.license))
    return EINVAL;

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
  if (spec->extensions_json)
    merge_schema_extras_object(root_obj, spec->extensions_json);
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

  if (spec->n_paths > 0 || spec->paths_extensions_json) {
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
