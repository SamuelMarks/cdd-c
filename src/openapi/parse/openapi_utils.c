/**
 * @file openapi_utils.c
 * @brief Parsing helper utilities, enum mapping, and value conversion.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/* --- Parsing Helpers --- */

cdd_c_error_t parse_verb(const char *v, enum OpenAPI_Verb *_out_val) {
  if (strcmp(v, "get") == 0) {
    *_out_val = OA_VERB_GET;
    return CDD_C_SUCCESS;
  }
  if (strcmp(v, "post") == 0) {
    *_out_val = OA_VERB_POST;
    return CDD_C_SUCCESS;
  }
  if (strcmp(v, "put") == 0) {
    *_out_val = OA_VERB_PUT;
    return CDD_C_SUCCESS;
  }
  if (strcmp(v, "delete") == 0) {
    *_out_val = OA_VERB_DELETE;
    return CDD_C_SUCCESS;
  }
  if (strcmp(v, "patch") == 0) {
    *_out_val = OA_VERB_PATCH;
    return CDD_C_SUCCESS;
  }
  if (strcmp(v, "head") == 0) {
    *_out_val = OA_VERB_HEAD;
    return CDD_C_SUCCESS;
  }
  if (strcmp(v, "options") == 0) {
    *_out_val = OA_VERB_OPTIONS;
    return CDD_C_SUCCESS;
  }
  if (strcmp(v, "trace") == 0) {
    *_out_val = OA_VERB_TRACE;
    return CDD_C_SUCCESS;
  }
  if (strcmp(v, "query") == 0) {
    *_out_val = OA_VERB_QUERY;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = OA_VERB_UNKNOWN;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if fixed operation method.
 */
cdd_c_error_t is_fixed_operation_method(const char *method) {
  int _ast_iequal_0 = false;
  int _ast_iequal_1 = false;
  int _ast_iequal_2 = false;
  int _ast_iequal_3 = false;
  int _ast_iequal_4 = false;
  int _ast_iequal_5 = false;
  int _ast_iequal_6 = false;
  int _ast_iequal_7 = false;
  int _ast_iequal_8 = false;
  if (!method)
    return CDD_C_SUCCESS;
  return (c_cdd_str_iequal(method, "get", &_ast_iequal_0), _ast_iequal_0) !=
             0 ||
         (c_cdd_str_iequal(method, "post", &_ast_iequal_1), _ast_iequal_1) !=
             0 ||
         (c_cdd_str_iequal(method, "put", &_ast_iequal_2), _ast_iequal_2) ||
         (c_cdd_str_iequal(method, "delete", &_ast_iequal_3), _ast_iequal_3) ||
         (c_cdd_str_iequal(method, "patch", &_ast_iequal_4), _ast_iequal_4) ||
         (c_cdd_str_iequal(method, "head", &_ast_iequal_5), _ast_iequal_5) ||
         (c_cdd_str_iequal(method, "options", &_ast_iequal_6), _ast_iequal_6) ||
         (c_cdd_str_iequal(method, "trace", &_ast_iequal_7), _ast_iequal_7) ||
         (c_cdd_str_iequal(method, "query", &_ast_iequal_8), _ast_iequal_8);
}

/**
 * @brief Parses param in from the given input.
 */
cdd_c_error_t parse_param_in(const char *in, enum OpenAPI_ParamIn *_out_val) {
  if (strcmp(in, "path") == 0) {
    *_out_val = OA_PARAM_IN_PATH;
    return CDD_C_SUCCESS;
  }
  if (strcmp(in, "query") == 0) {
    *_out_val = OA_PARAM_IN_QUERY;
    return CDD_C_SUCCESS;
  }
  if (strcmp(in, "querystring") == 0) {
    *_out_val = OA_PARAM_IN_QUERYSTRING;
    return CDD_C_SUCCESS;
  }
  if (strcmp(in, "header") == 0) {
    *_out_val = OA_PARAM_IN_HEADER;
    return CDD_C_SUCCESS;
  }
  if (strcmp(in, "cookie") == 0) {
    *_out_val = OA_PARAM_IN_COOKIE;
    return CDD_C_SUCCESS;
  }
  if (strcmp(in, "body") == 0) {
    *_out_val = OA_PARAM_IN_BODY;
    return CDD_C_SUCCESS;
  }
  if (strcmp(in, "formData") == 0) {
    *_out_val = OA_PARAM_IN_FORM_DATA;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = OA_PARAM_IN_UNKNOWN;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses param style from the given input.
 */
cdd_c_error_t parse_param_style(const char *s, enum OpenAPI_Style *_out_val) {
  if (!s) {
    *_out_val = OA_STYLE_UNKNOWN;
    return CDD_C_SUCCESS;
  }
  if (strcmp(s, "form") == 0) {
    *_out_val = OA_STYLE_FORM;
    return CDD_C_SUCCESS;
  }
  if (strcmp(s, "simple") == 0) {
    *_out_val = OA_STYLE_SIMPLE;
    return CDD_C_SUCCESS;
  }
  if (strcmp(s, "matrix") == 0) {
    *_out_val = OA_STYLE_MATRIX;
    return CDD_C_SUCCESS;
  }
  if (strcmp(s, "label") == 0) {
    *_out_val = OA_STYLE_LABEL;
    return CDD_C_SUCCESS;
  }
  if (strcmp(s, "spaceDelimited") == 0) {
    *_out_val = OA_STYLE_SPACE_DELIMITED;
    return CDD_C_SUCCESS;
  }
  if (strcmp(s, "pipeDelimited") == 0) {
    *_out_val = OA_STYLE_PIPE_DELIMITED;
    return CDD_C_SUCCESS;
  }
  if (strcmp(s, "deepObject") == 0) {
    *_out_val = OA_STYLE_DEEP_OBJECT;
    return CDD_C_SUCCESS;
  }
  if (strcmp(s, "cookie") == 0) {
    *_out_val = OA_STYLE_COOKIE;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = OA_STYLE_UNKNOWN;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the param type is primitive operation.
 */
cdd_c_error_t param_type_is_primitive(const char *type) {
  if (!type)
    return CDD_C_SUCCESS;
  return strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
         strcmp(type, "number") == 0 || strcmp(type, "boolean") == 0;
}

/**
 * @brief Executes the param type is object like operation.
 */
cdd_c_error_t param_type_is_object_like(const struct OpenAPI_Parameter *p) {
  if (!p || !p->type)
    return CDD_C_SUCCESS;
  if (strcmp(p->type, "array") == 0)
    return CDD_C_SUCCESS;
  return !param_type_is_primitive(p->type);
}

/**
 * @brief Executes the validate parameter style operation.
 */
cdd_c_error_t validate_parameter_style(const struct OpenAPI_Parameter *p,
                                       int has_content) {
  enum OpenAPI_Style style;
  if (!p)
    return CDD_C_SUCCESS;
  if (has_content)
    return CDD_C_SUCCESS;
  if (p->in == OA_PARAM_IN_QUERYSTRING)
    return CDD_C_SUCCESS;

  style = p->style;
  switch (p->in) {
  case OA_PARAM_IN_QUERY:
    if (style != OA_STYLE_FORM && style != OA_STYLE_SPACE_DELIMITED &&
        style != OA_STYLE_PIPE_DELIMITED && style != OA_STYLE_DEEP_OBJECT)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    break;
  case OA_PARAM_IN_PATH:
    if (style != OA_STYLE_SIMPLE && style != OA_STYLE_MATRIX &&
        style != OA_STYLE_LABEL)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    break;
  case OA_PARAM_IN_HEADER:
    if (style != OA_STYLE_SIMPLE)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    break;
  case OA_PARAM_IN_COOKIE:
    if (style != OA_STYLE_FORM && style != OA_STYLE_COOKIE)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    break;
  default:
    break;
  }

  if (style == OA_STYLE_DEEP_OBJECT) {
    if (p->is_array || !param_type_is_object_like(p))
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (style == OA_STYLE_SPACE_DELIMITED || style == OA_STYLE_PIPE_DELIMITED) {
    if (!p->is_array && !param_type_is_object_like(p))
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the component key is valid operation.
 */
cdd_c_error_t component_key_is_valid(const char *name) {
  size_t i;
  if (!name || !*name)
    return CDD_C_SUCCESS;
  for (i = 0; name[i]; ++i) {
    const unsigned char c = (unsigned char)name[i];
    if (!(isalnum(c) || c == '.' || c == '-' || c == '_'))
      return CDD_C_SUCCESS;
  }
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief media type key is valid.
 */
cdd_c_error_t media_type_key_is_valid(const char *name) {
  size_t i;
  if (!name || !*name)
    return CDD_C_SUCCESS;
  for (i = 0; name[i]; ++i) {
    const unsigned char c = (unsigned char)name[i];
    if (!(isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/' ||
          c == '+' || c == '*'))
      return CDD_C_SUCCESS;
  }
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Executes the validate component key map operation.
 */
cdd_c_error_t validate_component_key_map(const JSON_Object *obj) {
  size_t i, count;
  if (!obj)
    return CDD_C_SUCCESS;
  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(obj, i);
    if (!component_key_is_valid(name))
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief validate media type key map.
 */
cdd_c_error_t validate_media_type_key_map(const JSON_Object *obj) {
  size_t i, count;
  if (!obj)
    return CDD_C_SUCCESS;
  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *name = json_object_get_name(obj, i);
    if (!media_type_key_is_valid(name))
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the header name is content type operation.
 */
cdd_c_error_t header_name_is_content_type(const char *name) {
  int _ast_iequal_9 = false;
  if (!name)
    return CDD_C_SUCCESS;
  return (c_cdd_str_iequal(name, "Content-Type", &_ast_iequal_9),
          _ast_iequal_9) != 0;
}

/**
 * @brief Executes the header param is reserved operation.
 */
cdd_c_error_t header_param_is_reserved(const struct OpenAPI_Parameter *param) {
  int _ast_iequal_10 = false;
  int _ast_iequal_11 = false;
  int _ast_iequal_12 = false;
  if (!param || param->in != OA_PARAM_IN_HEADER || !param->name)
    return CDD_C_SUCCESS;
  return (c_cdd_str_iequal(param->name, "Accept", &_ast_iequal_10),
          _ast_iequal_10) != 0 ||
         (c_cdd_str_iequal(param->name, "Content-Type", &_ast_iequal_11),
          _ast_iequal_11) ||
         (c_cdd_str_iequal(param->name, "Authorization", &_ast_iequal_12),
          _ast_iequal_12);
}

/**
 * @brief Parses security type from the given input.
 */
cdd_c_error_t parse_security_type(const char *type,
                                  enum OpenAPI_SecurityType *_out_val) {
  if (!type) {
    *_out_val = OA_SEC_UNKNOWN;
    return CDD_C_SUCCESS;
  }
  if (strcmp(type, "apiKey") == 0) {
    *_out_val = OA_SEC_APIKEY;
    return CDD_C_SUCCESS;
  }
  if (strcmp(type, "http") == 0) {
    *_out_val = OA_SEC_HTTP;
    return CDD_C_SUCCESS;
  }
  if (strcmp(type, "mutualTLS") == 0) {
    *_out_val = OA_SEC_MUTUALTLS;
    return CDD_C_SUCCESS;
  }
  if (strcmp(type, "oauth2") == 0) {
    *_out_val = OA_SEC_OAUTH2;
    return CDD_C_SUCCESS;
  }
  if (strcmp(type, "openIdConnect") == 0) {
    *_out_val = OA_SEC_OPENID;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = OA_SEC_UNKNOWN;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses security in from the given input.
 */
cdd_c_error_t parse_security_in(const char *in,
                                enum OpenAPI_SecurityIn *_out_val) {
  if (!in) {
    *_out_val = OA_SEC_IN_UNKNOWN;
    return CDD_C_SUCCESS;
  }
  if (strcmp(in, "query") == 0) {
    *_out_val = OA_SEC_IN_QUERY;
    return CDD_C_SUCCESS;
  }
  if (strcmp(in, "header") == 0) {
    *_out_val = OA_SEC_IN_HEADER;
    return CDD_C_SUCCESS;
  }
  if (strcmp(in, "cookie") == 0) {
    *_out_val = OA_SEC_IN_COOKIE;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = OA_SEC_IN_UNKNOWN;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses oauth flow type from the given input.
 */
cdd_c_error_t parse_oauth_flow_type(const char *flow,
                                    enum OpenAPI_OAuthFlowType *_out_val) {
  if (!flow) {
    *_out_val = OA_OAUTH_FLOW_UNKNOWN;
    return CDD_C_SUCCESS;
  }
  if (strcmp(flow, "implicit") == 0) {
    *_out_val = OA_OAUTH_FLOW_IMPLICIT;
    return CDD_C_SUCCESS;
  }
  if (strcmp(flow, "password") == 0) {
    *_out_val = OA_OAUTH_FLOW_PASSWORD;
    return CDD_C_SUCCESS;
  }
  if (strcmp(flow, "clientCredentials") == 0) {
    *_out_val = OA_OAUTH_FLOW_CLIENT_CREDENTIALS;
    return CDD_C_SUCCESS;
  }
  if (strcmp(flow, "authorizationCode") == 0) {
    *_out_val = OA_OAUTH_FLOW_AUTHORIZATION_CODE;
    return CDD_C_SUCCESS;
  }
  if (strcmp(flow, "deviceAuthorization") == 0) {
    *_out_val = OA_OAUTH_FLOW_DEVICE_AUTHORIZATION;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = OA_OAUTH_FLOW_UNKNOWN;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses xml node type from the given input.
 */
cdd_c_error_t parse_xml_node_type(const char *node_type,
                                  enum OpenAPI_XmlNodeType *_out_val) {
  if (!node_type) {
    *_out_val = OA_XML_NODE_UNSET;
    return CDD_C_SUCCESS;
  }
  if (strcmp(node_type, "element") == 0) {
    *_out_val = OA_XML_NODE_ELEMENT;
    return CDD_C_SUCCESS;
  }
  if (strcmp(node_type, "attribute") == 0) {
    *_out_val = OA_XML_NODE_ATTRIBUTE;
    return CDD_C_SUCCESS;
  }
  if (strcmp(node_type, "text") == 0) {
    *_out_val = OA_XML_NODE_TEXT;
    return CDD_C_SUCCESS;
  }
  if (strcmp(node_type, "cdata") == 0) {
    *_out_val = OA_XML_NODE_CDATA;
    return CDD_C_SUCCESS;
  }
  if (strcmp(node_type, "none") == 0) {
    *_out_val = OA_XML_NODE_NONE;
    return CDD_C_SUCCESS;
  }
  {
    *_out_val = OA_XML_NODE_UNSET;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses any value from the given input.
 */
cdd_c_error_t parse_any_value(const JSON_Value *val, struct OpenAPI_Any *out) {
  char *_ast_strdup_13 = NULL;
  char *_ast_strdup_14 = NULL;
  JSON_Value_Type t;
  const char *s;
  char *json_str;

  if (!val || !out)
    return CDD_C_SUCCESS;

  t = json_value_get_type(val);
  out->type = OA_ANY_UNSET;

  switch (t) {
  case JSONString:
    s = json_value_get_string(val);
    out->type = OA_ANY_STRING;
    out->string = (c_cdd_strdup(s ? s : "", &_ast_strdup_13), _ast_strdup_13);
    if (!out->string)
      return CDD_C_ERROR_MEMORY;
    break;
  case JSONNumber:
    out->type = OA_ANY_NUMBER;
    out->number = json_value_get_number(val);
    break;
  case JSONBoolean:
    out->type = OA_ANY_BOOL;
    out->boolean = json_value_get_boolean(val);
    break;
  case JSONNull:
    out->type = OA_ANY_NULL;
    break;
  case JSONObject:
  case JSONArray:
    json_str = json_serialize_to_string(val);
    if (!json_str)
      return CDD_C_ERROR_MEMORY;
    out->type = OA_ANY_JSON;
    out->json = (c_cdd_strdup(json_str, &_ast_strdup_14), _ast_strdup_14);
    json_free_serialized_string(json_str);
    if (!out->json)
      return CDD_C_ERROR_MEMORY;
    break;
  default:
    break;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses any field from the given input.
 */
cdd_c_error_t parse_any_field(const JSON_Object *obj, const char *key,
                              struct OpenAPI_Any *out, int *out_set) {
  const JSON_Value *val;
  if (!obj || !key || !out || !out_set)
    return CDD_C_SUCCESS;
  if (!json_object_has_value(obj, key))
    return CDD_C_SUCCESS;
  val = json_object_get_value(obj, key);
  if (!val)
    return CDD_C_SUCCESS;
  {
    cdd_c_error_t _rc = parse_any_value(val, out);
    if (_rc != CDD_C_SUCCESS)
      return _rc;
  }
  *out_set = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses any array from the given input.
 */
cdd_c_error_t parse_any_array(const JSON_Array *arr, struct OpenAPI_Any **out,
                              size_t *out_count) {
  size_t i, count;

  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!arr)
    return CDD_C_SUCCESS;

  count = json_array_get_count(arr);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out = (struct OpenAPI_Any *)calloc(count, sizeof(struct OpenAPI_Any));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    const JSON_Value *val = json_array_get_value(arr, i);
    if (parse_any_value(val, &(*out)[i]) != 0) {
      size_t j;
      for (j = 0; j < i; ++j)
        free_any_value(&(*out)[j]);
      free(*out);
      *out = NULL;
      *out_count = 0;
      return CDD_C_ERROR_MEMORY;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the key in list operation.
 */
cdd_c_error_t key_in_list(const char *key, const char **list, size_t count) {
  size_t i;
  if (!key || !list)
    return CDD_C_SUCCESS;
  for (i = 0; i < count; ++i) {
    if (list[i] && strcmp(list[i], key) == 0)
      return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if extension key.
 */
cdd_c_error_t is_extension_key(const char *key) {
  return key && key[0] == 'x' && key[1] == '-';
}

/**
 * @brief Executes the clone json value operation.
 */
cdd_c_error_t clone_json_value(const JSON_Value *val, JSON_Value **_out_val) {
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
 * @brief Collects schema extras.
 */
cdd_c_error_t collect_schema_extras(const JSON_Object *obj,
                                    const char **skip_keys, size_t skip_count,
                                    char **out_json) {
  JSON_Value *_ast_clone_json_value_0;
  char *_ast_strdup_15 = NULL;
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;
  char *serialized;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  extras_val = json_value_init_object();
  if (!extras_val)
    return CDD_C_ERROR_MEMORY;
  extras_obj = json_value_get_object(extras_val);

  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(obj, i);
    const JSON_Value *val;
    JSON_Value *copy;

    if (!key || key_in_list(key, skip_keys, skip_count))
      continue;
    val = json_object_get_value(obj, key);
    copy = (clone_json_value(val, &_ast_clone_json_value_0),
            _ast_clone_json_value_0);
    if (!copy) {
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
    if (json_object_set_value(extras_obj, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (json_object_get_count(extras_obj) == 0) {
    json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

  serialized = json_serialize_to_string(extras_val);
  if (!serialized) {
    json_value_free(extras_val);
    return CDD_C_ERROR_MEMORY;
  }
  *out_json = (c_cdd_strdup(serialized, &_ast_strdup_15), _ast_strdup_15);
  json_free_serialized_string(serialized);
  json_value_free(extras_val);
  return *out_json ? 0 : ENOMEM;
}

/**
 * @brief Collects extensions.
 */
cdd_c_error_t collect_extensions(const JSON_Object *obj, char **out_json) {
  JSON_Value *_ast_clone_json_value_1;
  char *_ast_strdup_16 = NULL;
  JSON_Value *extras_val;
  JSON_Object *extras_obj;
  size_t i, count;
  char *serialized;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  extras_val = json_value_init_object();
  if (!extras_val)
    return CDD_C_ERROR_MEMORY;
  extras_obj = json_value_get_object(extras_val);

  count = json_object_get_count(obj);
  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(obj, i);
    const JSON_Value *val;
    JSON_Value *copy;
    if (!is_extension_key(key))
      continue;
    val = json_object_get_value(obj, key);
    copy = (clone_json_value(val, &_ast_clone_json_value_1),
            _ast_clone_json_value_1);
    if (!copy) {
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
    if (json_object_set_value(extras_obj, key, copy) != JSONSuccess) {
      json_value_free(copy);
      json_value_free(extras_val);
      return CDD_C_ERROR_MEMORY;
    }
  }

  if (json_object_get_count(extras_obj) == 0) {
    json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

  serialized = json_serialize_to_string(extras_val);
  if (!serialized) {
    json_value_free(extras_val);
    return CDD_C_ERROR_MEMORY;
  }
  *out_json = (c_cdd_strdup(serialized, &_ast_strdup_16), _ast_strdup_16);
  json_free_serialized_string(serialized);
  json_value_free(extras_val);
  return *out_json ? 0 : ENOMEM;
}

/**
 * @brief Executes the url has query or fragment operation.
 */
cdd_c_error_t url_has_query_or_fragment(const char *url) {
  if (!url)
    return CDD_C_SUCCESS;
  return (strchr(url, '?') != NULL || strchr(url, '#') != NULL);
}

/**
 * @brief Executes the openapi version supported operation.
 */
cdd_c_error_t openapi_version_supported(const char *version) {
  if (!version || !*version)
    return CDD_C_SUCCESS;
  return (version[0] == '3' && version[1] == '.' &&
          (version[2] == '0' || version[2] == '1' || version[2] == '2'));
}

/**
 * @brief Executes the example fields valid operation.
 */
cdd_c_error_t example_fields_valid(const struct OpenAPI_Example *ex) {
  if (!ex)
    return CDD_C_ERROR_UNKNOWN;
  if (ex->data_value_set && ex->value_set)
    return CDD_C_SUCCESS;
  if (ex->serialized_value && ex->external_value)
    return CDD_C_SUCCESS;
  if (ex->value_set && (ex->serialized_value || ex->external_value))
    return CDD_C_SUCCESS;
  return CDD_C_ERROR_UNKNOWN;
}

/**
 * @brief Executes the object has example and examples operation.
 */
cdd_c_error_t object_has_example_and_examples(const JSON_Object *obj) {
  if (!obj)
    return CDD_C_SUCCESS;
  if (json_object_has_value(obj, "example") &&
      json_object_has_value(obj, "examples")) {
    return CDD_C_ERROR_UNKNOWN;
  }
  return CDD_C_SUCCESS;
}
