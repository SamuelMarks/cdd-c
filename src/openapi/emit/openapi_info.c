/**
 * @file openapi_info.c
 * @brief OpenAPI emitter info, servers, and utilities.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#include "c_cdd/log.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
/* clang-format on */

cdd_c_error_t verb_to_str_openapi(enum OpenAPI_Verb v, char **_out_val) {
  switch (v) {
  case OA_VERB_GET: {
    *_out_val = "get";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_POST: {
    *_out_val = "post";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_PUT: {
    *_out_val = "put";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_DELETE: {
    *_out_val = "delete";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_PATCH: {
    *_out_val = "patch";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_HEAD: {
    *_out_val = "head";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_OPTIONS: {
    *_out_val = "options";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_TRACE: {
    *_out_val = "trace";
    return CDD_C_SUCCESS;
  }
  case OA_VERB_QUERY: {
    *_out_val = "query";
    return CDD_C_SUCCESS;
  }
  default: {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Converts parameter in to string.
 *
 * @param in The parameter in.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
cdd_c_error_t param_in_to_str_openapi(enum OpenAPI_ParamIn in,
                                      char **_out_val) {
  switch (in) {
  case OA_PARAM_IN_PATH: {
    *_out_val = "path";
    return CDD_C_SUCCESS;
  }
  case OA_PARAM_IN_QUERY: {
    *_out_val = "query";
    return CDD_C_SUCCESS;
  }
  case OA_PARAM_IN_QUERYSTRING: {
    *_out_val = "querystring";
    return CDD_C_SUCCESS;
  }
  case OA_PARAM_IN_HEADER: {
    *_out_val = "header";
    return CDD_C_SUCCESS;
  }
  case OA_PARAM_IN_COOKIE: {
    *_out_val = "cookie";
    return CDD_C_SUCCESS;
  }
  default: {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Converts style to string.
 *
 * @param s The style.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
cdd_c_error_t style_to_str_openapi(enum OpenAPI_Style s, char **_out_val) {
  switch (s) {
  case OA_STYLE_FORM: {
    *_out_val = "form";
    return CDD_C_SUCCESS;
  }
  case OA_STYLE_SIMPLE: {
    *_out_val = "simple";
    return CDD_C_SUCCESS;
  }
  case OA_STYLE_MATRIX: {
    *_out_val = "matrix";
    return CDD_C_SUCCESS;
  }
  case OA_STYLE_LABEL: {
    *_out_val = "label";
    return CDD_C_SUCCESS;
  }
  case OA_STYLE_SPACE_DELIMITED: {
    *_out_val = "spaceDelimited";
    return CDD_C_SUCCESS;
  }
  case OA_STYLE_PIPE_DELIMITED: {
    *_out_val = "pipeDelimited";
    return CDD_C_SUCCESS;
  }
  case OA_STYLE_DEEP_OBJECT: {
    *_out_val = "deepObject";
    return CDD_C_SUCCESS;
  }
  case OA_STYLE_COOKIE: {
    *_out_val = "cookie";
    return CDD_C_SUCCESS;
  }
  default: {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Converts xml node type to string.
 *
 * @param t The xml node type.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
cdd_c_error_t xml_node_type_to_str_openapi(enum OpenAPI_XmlNodeType t,
                                           char **_out_val) {
  switch (t) {
  case OA_XML_NODE_ELEMENT: {
    *_out_val = "element";
    return CDD_C_SUCCESS;
  }
  case OA_XML_NODE_ATTRIBUTE: {
    *_out_val = "attribute";
    return CDD_C_SUCCESS;
  }
  case OA_XML_NODE_TEXT: {
    *_out_val = "text";
    return CDD_C_SUCCESS;
  }
  case OA_XML_NODE_CDATA: {
    *_out_val = "cdata";
    return CDD_C_SUCCESS;
  }
  case OA_XML_NODE_NONE: {
    *_out_val = "none";
    return CDD_C_SUCCESS;
  }
  default: {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Checks if header name is content type.
 *
 * @param name The name.
 * @return 1 if true, 0 otherwise.
 */
cdd_c_error_t header_name_is_content_type_openapi(const char *name) {
  int _ast_iequal_0 = false;
  if (!name)
    return CDD_C_SUCCESS;
  return (c_cdd_str_iequal(name, "Content-Type", &_ast_iequal_0),
          _ast_iequal_0) != 0;
}

/**
 * @brief Checks if parameter is a reserved header.
 *
 * @param p The parameter.
 * @return 1 if true, 0 otherwise.
 */
cdd_c_error_t
param_is_reserved_header_openapi(const struct OpenAPI_Parameter *p) {
  int _ast_iequal_1 = false;
  int _ast_iequal_2 = false;
  int _ast_iequal_3 = false;
  if (!p || p->in != OA_PARAM_IN_HEADER || !p->name)
    return CDD_C_SUCCESS;
  return (c_cdd_str_iequal(p->name, "Accept", &_ast_iequal_1), _ast_iequal_1) !=
             0 ||
         (c_cdd_str_iequal(p->name, "Content-Type", &_ast_iequal_2),
          _ast_iequal_2) ||
         (c_cdd_str_iequal(p->name, "Authorization", &_ast_iequal_3),
          _ast_iequal_3);
}

/**
 * @brief Converts oauth flow type to string.
 *
 * @param t The oauth flow type.
 * @param _out_val Pointer to the output string.
 * @return 0 on success.
 */
C_CDD_EXPORT cdd_c_error_t
oauth_flow_type_to_str_openapi(enum OpenAPI_OAuthFlowType t, char **_out_val) {
  switch (t) {
  case OA_OAUTH_FLOW_IMPLICIT: {
    *_out_val = "implicit";
    return CDD_C_SUCCESS;
  }
  case OA_OAUTH_FLOW_PASSWORD: {
    *_out_val = "password";
    return CDD_C_SUCCESS;
  }
  case OA_OAUTH_FLOW_CLIENT_CREDENTIALS: {
    *_out_val = "clientCredentials";
    return CDD_C_SUCCESS;
  }
  case OA_OAUTH_FLOW_AUTHORIZATION_CODE: {
    *_out_val = "authorizationCode";
    return CDD_C_SUCCESS;
  }
  case OA_OAUTH_FLOW_DEVICE_AUTHORIZATION: {
    *_out_val = "deviceAuthorization";
    return CDD_C_SUCCESS;
  }
  default: {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Checks if a schema type is primitive.
 *
 * @param type The schema type string.
 * @return 1 if true, 0 otherwise.
 */
C_CDD_EXPORT cdd_c_error_t is_schema_primitive_openapi(const char *type) {
  if (!type)
    return CDD_C_SUCCESS;
  return strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
         strcmp(type, "boolean") == 0 || strcmp(type, "number") == 0 ||
         strcmp(type, "object") == 0 || strcmp(type, "null") == 0;
}

/**
 * @brief Executes the license fields invalid operation.
 */
C_CDD_EXPORT cdd_c_error_t
license_fields_invalid(const struct OpenAPI_License *lic) {
  int has_any;
  if (!lic)
    return CDD_C_SUCCESS;
  has_any = lic->name || lic->identifier || lic->url || lic->extensions_json;
  if (!has_any)
    return CDD_C_SUCCESS;
  if (!lic->name || lic->name[0] == '\0')
    return CDD_C_ERROR_UNKNOWN;
  if (lic->identifier && lic->url)
    return CDD_C_ERROR_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the server url has query or fragment operation.
 */
C_CDD_EXPORT cdd_c_error_t server_url_has_query_or_fragment(const char *url) {
  if (!url)
    return CDD_C_SUCCESS;
  return strchr(url, '?') != NULL || strchr(url, '#') != NULL;
}

/**
 * @brief Executes the clone json value operation.
 */
C_CDD_EXPORT cdd_c_error_t clone_json_value(const JSON_Value *val,
                                            JSON_Value **_out_val) {
  char *serialized;
  JSON_Value *copy;

  if (!val) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  serialized = json_serialize_to_string((JSON_Value *)val);
  if (!serialized) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  copy = json_parse_string(serialized);
  json_free_serialized_string(serialized);
  {
    *_out_val = copy;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Merges extra schema object properties.
 *
 * @param target The target json object.
 * @param extras_json The extra json string.
 * @return 0 on success.
 */
C_CDD_EXPORT cdd_c_error_t merge_schema_extras_object_openapi(
    JSON_Object *target, const char *extras_json) {
  JSON_Value *_ast_clone_json_value_0;
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;

  if (!target || !extras_json || extras_json[0] == '\0')
    return CDD_C_SUCCESS;

  extras_val = json_parse_string(extras_json);
  if (!extras_val)
    return CDD_C_SUCCESS;
  extras_obj = json_value_get_object(extras_val);
  if (!extras_obj) {
    json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

  count = json_object_get_count(extras_obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(extras_obj, i);
    const JSON_Value *val;
    JSON_Value *copy;
    if (json_object_has_value(target, key))
      continue;
    val = json_object_get_value(extras_obj, key);
    copy = (clone_json_value(val, &_ast_clone_json_value_0),
            _ast_clone_json_value_0);
    if (!copy) {
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
    if (json_object_set_value(target, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  json_value_free(extras_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write example object.
 */
C_CDD_EXPORT void write_example_object(JSON_Object *ex_obj,
                                       const struct OpenAPI_Example *ex) {
  JSON_Value *_ast_any_to_json_value_2;
  JSON_Value *_ast_any_to_json_value_3;
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
    val = (any_to_json_value(&ex->data_value, &_ast_any_to_json_value_2),
           _ast_any_to_json_value_2);
    if (val)
      json_object_set_value(ex_obj, "dataValue", val);
  } else if (ex->value_set) {
    val = (any_to_json_value(&ex->value, &_ast_any_to_json_value_3),
           _ast_any_to_json_value_3);
    if (val)
      json_object_set_value(ex_obj, "value", val);
  }

  if (ex->serialized_value)
    json_object_set_string(ex_obj, "serializedValue", ex->serialized_value);
  if (ex->external_value)
    json_object_set_string(ex_obj, "externalValue", ex->external_value);
  if (ex->extensions_json)
    merge_schema_extras_object_openapi(ex_obj, ex->extensions_json);
}

/**
 * @brief Generates C code for write examples object.
 */
C_CDD_EXPORT cdd_c_error_t write_examples_object(
    JSON_Object *parent, const char *key,
    const struct OpenAPI_Example *examples, size_t n_examples) {
  JSON_Value *examples_val;
  JSON_Object *examples_obj;
  size_t i;

  if (!parent || !key || !examples || n_examples == 0)
    return CDD_C_SUCCESS;

  examples_val = json_value_init_object();
  if (!examples_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
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
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write example fields.
 */
C_CDD_EXPORT void write_example_fields(JSON_Object *parent,
                                       const struct OpenAPI_Any *example,
                                       int example_set,
                                       const struct OpenAPI_Example *examples,
                                       size_t n_examples) {
  JSON_Value *_ast_any_to_json_value_4;
  if (!parent)
    return;
  if (examples && n_examples > 0) {
    {
      cdd_c_error_t rc_oa_tmp =
          write_examples_object(parent, "examples", examples, n_examples);
      if (rc_oa_tmp != CDD_C_SUCCESS)
        return;
    }
  } else if (example_set && example) {
    JSON_Value *val = (any_to_json_value(example, &_ast_any_to_json_value_4),
                       _ast_any_to_json_value_4);
    if (val)
      json_object_set_value(parent, "example", val);
  }
}

/**
 * @brief Generates C code for write external docs.
 */
C_CDD_EXPORT void write_external_docs(JSON_Object *parent, const char *key,
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
    merge_schema_extras_object_openapi(ext_obj, docs->extensions_json);

  json_object_set_value(parent, key, ext_val);
}

/**
 * @brief Generates C code for write discriminator object.
 */
C_CDD_EXPORT void
write_discriminator_object(JSON_Object *parent,
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
    merge_schema_extras_object_openapi(disc_obj, disc->extensions_json);

  json_object_set_value(parent, "discriminator", disc_val);
}

/**
 * @brief Generates C code for write xml object.
 */
C_CDD_EXPORT void write_xml_object(JSON_Object *parent,
                                   const struct OpenAPI_Xml *xml, int xml_set) {
  char *_ast_xml_node_type_to_str_5 = NULL;
  JSON_Value *xml_val;
  JSON_Object *xml_obj;
  const char *node_type;

  if (!parent || !xml || !xml_set)
    return;

  xml_val = json_value_init_object();
  if (!xml_val)
    return;
  xml_obj = json_value_get_object(xml_val);

  node_type = (xml_node_type_to_str_openapi(xml->node_type,
                                            &_ast_xml_node_type_to_str_5),
               _ast_xml_node_type_to_str_5);
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
    merge_schema_extras_object_openapi(xml_obj, xml->extensions_json);

  json_object_set_value(parent, "xml", xml_val);
}

/**
 * @brief Generates C code for write info.
 */
C_CDD_EXPORT void write_info(JSON_Object *root_obj,
                             const struct OpenAPI_Spec *spec) {
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
    merge_schema_extras_object_openapi(info_obj, spec->info.extensions_json);

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
      merge_schema_extras_object_openapi(contact_obj,
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
      merge_schema_extras_object_openapi(license_obj,
                                         spec->info.license.extensions_json);
    json_object_set_value(info_obj, "license", license_val);
  }

  json_object_set_value(root_obj, "info", info_val);
}

/**
 * @brief Generates C code for write server object.
 */
C_CDD_EXPORT void write_server_object(JSON_Object *srv_obj,
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
        merge_schema_extras_object_openapi(var_obj, var->extensions_json);
      if (var->name)
        json_object_set_value(vars_obj, var->name, var_val);
      else
        json_value_free(var_val);
    }
    json_object_set_value(srv_obj, "variables", vars_val);
  }
  if (srv->extensions_json)
    merge_schema_extras_object_openapi(srv_obj, srv->extensions_json);
}

/**
 * @brief Generates C code for write servers.
 */
C_CDD_EXPORT cdd_c_error_t write_servers(JSON_Object *root_obj,
                                         const struct OpenAPI_Spec *spec) {
  if (!spec)
    return CDD_C_SUCCESS;
  return write_server_array(root_obj, "servers", spec->servers,
                            spec->n_servers);
}

/**
 * @brief Generates C code for write server array.
 */
C_CDD_EXPORT cdd_c_error_t
write_server_array(JSON_Object *parent, const char *key,
                   const struct OpenAPI_Server *servers, size_t n_servers) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (!parent || !key || !servers || n_servers == 0)
    return CDD_C_SUCCESS;

  arr_val = json_value_init_array();
  if (!arr_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  arr = json_value_get_array(arr_val);

  for (i = 0; i < n_servers; ++i) {
    JSON_Value *srv_val;
    JSON_Object *srv_obj;
    const struct OpenAPI_Server *srv = &servers[i];
    if (server_url_has_query_or_fragment(srv->url)) {
      json_value_free(arr_val);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    srv_val = json_value_init_object();
    srv_obj = json_value_get_object(srv_val);
    write_server_object(srv_obj, srv);

    json_array_append_value(arr, srv_val);
  }

  json_object_set_value(parent, key, arr_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Generates C code for write tags.
 */
C_CDD_EXPORT cdd_c_error_t write_tags(JSON_Object *root_obj,
                                      const struct OpenAPI_Spec *spec) {
  JSON_Value *arr_val;
  JSON_Array *arr;
  size_t i;

  if (!spec || spec->n_tags == 0)
    return CDD_C_SUCCESS;

  arr_val = json_value_init_array();
  if (!arr_val) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
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
      merge_schema_extras_object_openapi(tag_obj, tag->extensions_json);

    json_array_append_value(arr, tag_val);
  }

  json_object_set_value(root_obj, "tags", arr_val);
  return CDD_C_SUCCESS;
}
