/**
 * @file openapi_utils.c
 * @brief Parsing helper utilities, enum mapping, and value conversion.
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/memory.h"
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/* --- Parsing Helpers --- */

/**
 * @brief Parses HTTP verb string to enum.
 *
 * @param[in] v The HTTP verb string.
 * @param[out] _out_val Output verb enum value.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t parse_verb(const char *v, enum OpenAPI_Verb *_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!v) {
    *_out_val = OA_VERB_UNKNOWN;
    return CDD_C_SUCCESS;
  }
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
  *_out_val = OA_VERB_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if fixed operation method.
 *
 * @param[in] method The HTTP method string to check.
 * @param[out] out_is_fixed Set to 1 if fixed operation method, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t is_fixed_operation_method(const char *method, int *out_is_fixed) {
  size_t i;
  int eq = 0;
  cdd_c_error_t rc;
  static const char *const fixed_methods[] = {"get",     "post",  "put",
                                              "delete",  "patch", "head",
                                              "options", "trace", "query"};
  if (!out_is_fixed)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_fixed = 0;
  if (!method)
    return CDD_C_SUCCESS;
  for (i = 0; i < sizeof(fixed_methods) / sizeof(fixed_methods[0]); ++i) {
    rc = c_cdd_str_iequal(method, fixed_methods[i], &eq);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (eq) {
      *out_is_fixed = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses param in from the given input.
 *
 * @param[in] in The parameter location string.
 * @param[out] _out_val Output parameter location enum value.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t parse_param_in(const char *in, enum OpenAPI_ParamIn *_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!in) {
    *_out_val = OA_PARAM_IN_UNKNOWN;
    return CDD_C_SUCCESS;
  }
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
  *_out_val = OA_PARAM_IN_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses param style from the given input.
 *
 * @param[in] s The parameter style string.
 * @param[out] _out_val Output parameter style enum value.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t parse_param_style(const char *s, enum OpenAPI_Style *_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
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
  *_out_val = OA_STYLE_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether an OpenAPI parameter type name is a scalar primitive.
 *
 * @param[in] type The parameter type string.
 * @param[out] out_is_prim Set to 1 if primitive, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t param_type_is_primitive(const char *type, int *out_is_prim) {
  if (!out_is_prim)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_prim = 0;
  if (!type)
    return CDD_C_SUCCESS;
  *out_is_prim = (strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
                  strcmp(type, "number") == 0 || strcmp(type, "boolean") == 0)
                     ? 1
                     : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether an OpenAPI parameter represents an object or map
 * structure.
 *
 * @param[in] p The OpenAPI parameter structure.
 * @param[out] out_is_obj Set to 1 if object-like, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t param_type_is_object_like(const struct OpenAPI_Parameter *p,
                                        int *out_is_obj) {
  int is_prim = 0;
  if (!out_is_obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_obj = 0;
  if (!p || !p->type)
    return CDD_C_SUCCESS;
  if (strcmp(p->type, "array") == 0)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = param_type_is_primitive(p->type, &is_prim);
  *out_is_obj = !is_prim ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Validates that a parameter's style and type combination is legal.
 *
 * @param[in] p Parameter to validate.
 * @param[in] has_content Non-zero if parameter uses `content` instead of
 * `schema`.
 * @return CDD_C_SUCCESS on valid style, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
cdd_c_error_t validate_parameter_style(const struct OpenAPI_Parameter *p,
                                       int has_content) {
  enum OpenAPI_Style style;
  int is_obj = 0;
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

  g_cdd_last_alloc_err = param_type_is_object_like(p, &is_obj);

  if (style == OA_STYLE_DEEP_OBJECT) {
    if (p->is_array || !is_obj)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (style == OA_STYLE_SPACE_DELIMITED || style == OA_STYLE_PIPE_DELIMITED) {
    if (!p->is_array && !is_obj)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a component map key matches `^[a-zA-Z0-9._-]+$`.
 *
 * @param[in] name Component key string.
 * @param[out] out_valid Set to 1 if valid, 0 if invalid.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t component_key_is_valid(const char *name, int *out_valid) {
  size_t i;
  if (!out_valid)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_valid = 0;
  if (!name || !*name)
    return CDD_C_SUCCESS;
  for (i = 0; name[i]; ++i) {
    const unsigned char c = (unsigned char)name[i];
    if (!(isalnum(c) || c == '.' || c == '-' || c == '_'))
      return CDD_C_SUCCESS;
  }
  *out_valid = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a media type component key is valid.
 *
 * @param[in] name Media type key string.
 * @param[out] out_valid Set to 1 if valid, 0 if invalid.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t media_type_key_is_valid(const char *name, int *out_valid) {
  size_t i;
  if (!out_valid)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_valid = 0;
  if (!name || !*name)
    return CDD_C_SUCCESS;
  for (i = 0; name[i]; ++i) {
    const unsigned char c = (unsigned char)name[i];
    if (!(isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/' ||
          c == '+' || c == '*'))
      return CDD_C_SUCCESS;
  }
  *out_valid = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Validates all keys in a components sub-map.
 *
 * @param[in] obj JSON object representing a component map.
 * @return CDD_C_SUCCESS if all keys are valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
cdd_c_error_t validate_component_key_map(const JSON_Object *obj) {
  size_t i, count = 0;
  if (!obj)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_object_get_count(obj, &count);
  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    int is_valid = 0;
    g_cdd_last_alloc_err = cdd_json_object_get_name(obj, i, &name);
    g_cdd_last_alloc_err = component_key_is_valid(name, &is_valid);
    if (!is_valid)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Validates all keys in `components.mediaTypes`.
 *
 * @param[in] obj JSON object representing the `mediaTypes` map.
 * @return CDD_C_SUCCESS if all keys are valid, or CDD_C_ERROR_INVALID_ARGUMENT.
 */
cdd_c_error_t validate_media_type_key_map(const JSON_Object *obj) {
  size_t i, count = 0;
  if (!obj)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_object_get_count(obj, &count);
  for (i = 0; i < count; ++i) {
    const char *name = NULL;
    int is_valid = 0;
    g_cdd_last_alloc_err = cdd_json_object_get_name(obj, i, &name);
    g_cdd_last_alloc_err = media_type_key_is_valid(name, &is_valid);
    if (!is_valid)
      return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks case-insensitively whether a header name is Content-Type.
 *
 * @param[in] name The header name string.
 * @param[out] out_is_ct Set to 1 if Content-Type, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t header_name_is_content_type(const char *name, int *out_is_ct) {
  int eq = 0;
  cdd_c_error_t rc;
  if (!out_is_ct)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_ct = 0;
  if (!name)
    return CDD_C_SUCCESS;
  rc = c_cdd_str_iequal(name, "Content-Type", &eq);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_is_ct = eq ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether an HTTP header name is reserved by OpenAPI (Accept,
 * Content-Type, Authorization).
 *
 * @param[in] param The OpenAPI parameter structure.
 * @param[out] out_is_res Set to 1 if reserved header, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t header_param_is_reserved(const struct OpenAPI_Parameter *param,
                                       int *out_is_res) {
  size_t i;
  int eq = 0;
  cdd_c_error_t rc;
  static const char *const reserved_headers[] = {"Accept", "Content-Type",
                                                 "Authorization"};
  if (!out_is_res)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_res = 0;
  if (!param || param->in != OA_PARAM_IN_HEADER || !param->name)
    return CDD_C_SUCCESS;
  for (i = 0; i < sizeof(reserved_headers) / sizeof(reserved_headers[0]); ++i) {
    rc = c_cdd_str_iequal(param->name, reserved_headers[i], &eq);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (eq) {
      *out_is_res = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses security type from the given input.
 *
 * @param[in] type The security type string.
 * @param[out] _out_val Output security type enum value.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t parse_security_type(const char *type,
                                  enum OpenAPI_SecurityType *_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
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
  *_out_val = OA_SEC_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses security in from the given input.
 *
 * @param[in] in The security location string.
 * @param[out] _out_val Output security location enum value.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t parse_security_in(const char *in,
                                enum OpenAPI_SecurityIn *_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
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
  *_out_val = OA_SEC_IN_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses oauth flow type from the given input.
 *
 * @param[in] flow The OAuth flow type string.
 * @param[out] _out_val Output OAuth flow type enum value.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t parse_oauth_flow_type(const char *flow,
                                    enum OpenAPI_OAuthFlowType *_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
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
  *_out_val = OA_OAUTH_FLOW_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses xml node type from the given input.
 *
 * @param[in] node_type The XML node type string.
 * @param[out] _out_val Output XML node type enum value.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t parse_xml_node_type(const char *node_type,
                                  enum OpenAPI_XmlNodeType *_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
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
  *_out_val = OA_XML_NODE_UNSET;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses any value from the given input.
 *
 * @param[in] val The source JSON value.
 * @param[out] out Output OpenAPI_Any structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_any_value(const JSON_Value *val, struct OpenAPI_Any *out) {
  JSON_Value_Type t = JSONError;
  const char *s = NULL;
  char *json_str = NULL;
  cdd_c_error_t rc;

  if (!val || !out)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_get_type(val, &t);
  out->type = OA_ANY_UNSET;

  if (t == JSONString) {
    g_cdd_last_alloc_err = cdd_json_value_get_string(val, &s);
    out->type = OA_ANY_STRING;
    rc = c_cdd_strdup(s, &out->string);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (t == JSONNumber) {
    out->type = OA_ANY_NUMBER;
    g_cdd_last_alloc_err = cdd_json_value_get_number(val, &out->number);
  } else if (t == JSONBoolean) {
    out->type = OA_ANY_BOOL;
    g_cdd_last_alloc_err = cdd_json_value_get_boolean(val, &out->boolean);
  } else if (t == JSONNull) {
    out->type = OA_ANY_NULL;
  } else {
#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT int g_cdd_fail_any_serialize;
      if (g_cdd_fail_any_serialize && --g_cdd_fail_any_serialize == 0)
        json_str = NULL;
      else
        g_cdd_last_alloc_err = cdd_json_serialize_to_string(val, &json_str);
    }
#else
    cdd_json_serialize_to_string(val, &json_str);
#endif
    if (!json_str)
      return CDD_C_ERROR_MEMORY;
    out->type = OA_ANY_JSON;
    rc = c_cdd_strdup(json_str, &out->json);
    g_cdd_last_alloc_err = cdd_json_free_serialized_string(json_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses any field from the given input.
 *
 * @param[in] obj The source JSON object.
 * @param[in] key The field key name.
 * @param[out] out Output OpenAPI_Any structure.
 * @param[out] out_set Output flag set to 1 if field was present.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_any_field(const JSON_Object *obj, const char *key,
                              struct OpenAPI_Any *out, int *out_set) {
  JSON_Value *val = NULL;
  int has_val = 0;
  cdd_c_error_t rc;

  if (!obj || !key || !out || !out_set)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_object_has_value(obj, key, &has_val);
  if (!has_val)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_object_get_value(obj, key, &val);
  rc = parse_any_value(val, out);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_set = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses any array from the given input.
 *
 * @param[in] arr The source JSON array.
 * @param[out] out Output array of OpenAPI_Any structures.
 * @param[out] out_count Output count of parsed elements.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_any_array(const JSON_Array *arr, struct OpenAPI_Any **out,
                              size_t *out_count) {
  size_t i, count = 0;

  if (!out || !out_count)
    return CDD_C_SUCCESS;
  *out = NULL;
  *out_count = 0;
  if (!arr)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_array_get_count(arr, &count);
  if (count == 0)
    return CDD_C_SUCCESS;

  *out = (struct OpenAPI_Any *)C_CDD_CALLOC(count, sizeof(struct OpenAPI_Any));
  if (!*out)
    return CDD_C_ERROR_MEMORY;
  *out_count = count;

  for (i = 0; i < count; ++i) {
    JSON_Value *val = NULL;
    g_cdd_last_alloc_err = cdd_json_array_get_value(arr, i, &val);
    if (parse_any_value(val, &(*out)[i]) != CDD_C_SUCCESS) {
      size_t j;
      for (j = 0; j < i; ++j)
        g_cdd_last_alloc_err = free_any_value(&(*out)[j]);
      C_CDD_FREE(*out);
      *out = NULL;
      *out_count = 0;
      return CDD_C_ERROR_MEMORY;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether a string key already exists in a list of strings.
 *
 * @param[in] key The key string to search for.
 * @param[in] list Array of string keys.
 * @param[in] count Number of elements in list.
 * @param[out] out_in_list Set to 1 if found, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t key_in_list(const char *key, const char **list, size_t count,
                          int *out_in_list) {
  size_t i;
  if (!out_in_list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_in_list = 0;
  if (!key || !list)
    return CDD_C_SUCCESS;
  for (i = 0; i < count; ++i) {
    if (list[i] && strcmp(list[i], key) == 0) {
      *out_in_list = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if extension key.
 *
 * @param[in] key The property key name.
 * @param[out] out_is_ext Set to 1 if key starts with x-, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t is_extension_key(const char *key, int *out_is_ext) {
  if (!out_is_ext)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_ext = (key && key[0] == 'x' && key[1] == '-') ? 1 : 0;
  return CDD_C_SUCCESS;
}

/*
 * @brief Deep-copies a Parson JSON_Value via serialization round-trip.
 *
 * @param[in] val The source JSON value to clone.
 * @param[out] _out_val Pointer to the cloned JSON value output.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t clone_json_value(const JSON_Value *val, JSON_Value **_out_val) {
  char *serialized = NULL;
  JSON_Value *copy = NULL;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!val) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_clone_json_serialize;
    if (g_cdd_fail_clone_json_serialize &&
        --g_cdd_fail_clone_json_serialize == 0)
      serialized = NULL;
    else
      g_cdd_last_alloc_err = cdd_json_serialize_to_string(val, &serialized);
  }
#else
  cdd_json_serialize_to_string(val, &serialized);
#endif
  if (!serialized) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_clone_json_parse;
    if (g_cdd_fail_clone_json_parse && --g_cdd_fail_clone_json_parse == 0)
      copy = NULL;
    else
      g_cdd_last_alloc_err = cdd_json_parse_string(serialized, &copy);
  }
#else
  cdd_json_parse_string(serialized, &copy);
#endif
  g_cdd_last_alloc_err = cdd_json_free_serialized_string(serialized);
  if (!copy) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  *_out_val = copy;
  return CDD_C_SUCCESS;
}

/**
 * @brief Collects schema extras.
 *
 * @param[in] obj The source JSON object.
 * @param[in] skip_keys Array of known keys to skip.
 * @param[in] skip_count Number of keys in skip_keys.
 * @param[out] out_json Output serialized JSON string of extra properties.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t collect_schema_extras(const JSON_Object *obj,
                                    const char **skip_keys, size_t skip_count,
                                    char **out_json) {
  JSON_Value *extras_val = NULL;
  JSON_Object *extras_obj = NULL;
  size_t i, count = 0, extras_count = 0;
  char *serialized = NULL;
  cdd_c_error_t rc;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_extras_init;
    if (g_cdd_fail_extras_init && --g_cdd_fail_extras_init == 0)
      extras_val = NULL;
    else
      g_cdd_last_alloc_err = cdd_json_value_init_object(&extras_val);
  }
#else
  cdd_json_value_init_object(&extras_val);
#endif
  if (!extras_val)
    return CDD_C_ERROR_MEMORY;
  g_cdd_last_alloc_err = cdd_json_value_get_object(extras_val, &extras_obj);

  g_cdd_last_alloc_err = cdd_json_object_get_count(obj, &count);
  for (i = 0; i < count; ++i) {
    const char *key = NULL;
    JSON_Value *val = NULL;
    JSON_Value *copy = NULL;
    int in_list = 0;

    g_cdd_last_alloc_err = cdd_json_object_get_name(obj, i, &key);
    g_cdd_last_alloc_err = key_in_list(key, skip_keys, skip_count, &in_list);
    if (in_list)
      continue;
    g_cdd_last_alloc_err = cdd_json_object_get_value(obj, key, &val);
    rc = clone_json_value(val, &copy);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
      return rc;
    }
#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
      if (g_cdd_fail_json_set_value && --g_cdd_fail_json_set_value == 0) {
        g_cdd_last_alloc_err = cdd_json_value_free(copy);
        g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
        return CDD_C_ERROR_MEMORY;
      }
    }
#endif
    g_cdd_last_alloc_err =
        cdd_json_object_set_value_safe(extras_obj, key, copy);
  }

  g_cdd_last_alloc_err = cdd_json_object_get_count(extras_obj, &extras_count);
  if (extras_count == 0) {
    g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_extras_serialize;
    if (g_cdd_fail_extras_serialize && --g_cdd_fail_extras_serialize == 0)
      serialized = NULL;
    else
      g_cdd_last_alloc_err =
          cdd_json_serialize_to_string(extras_val, &serialized);
  }
#else
  cdd_json_serialize_to_string(extras_val, &serialized);
#endif
  if (!serialized) {
    g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
    return CDD_C_ERROR_MEMORY;
  }
  rc = c_cdd_strdup(serialized, out_json);
  g_cdd_last_alloc_err = cdd_json_free_serialized_string(serialized);
  g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
  return rc;
}

/**
 * @brief Collects extensions.
 *
 * @param[in] obj The source JSON object.
 * @param[out] out_json Output serialized JSON string of x- extensions.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t collect_extensions(const JSON_Object *obj, char **out_json) {
  JSON_Value *extras_val = NULL;
  JSON_Object *extras_obj = NULL;
  size_t i, count = 0, extras_count = 0;
  char *serialized = NULL;
  cdd_c_error_t rc;

  if (out_json)
    *out_json = NULL;
  if (!obj || !out_json)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_extensions_init;
    if (g_cdd_fail_extensions_init && --g_cdd_fail_extensions_init == 0)
      extras_val = NULL;
    else
      g_cdd_last_alloc_err = cdd_json_value_init_object(&extras_val);
  }
#else
  cdd_json_value_init_object(&extras_val);
#endif
  if (!extras_val)
    return CDD_C_ERROR_MEMORY;
  g_cdd_last_alloc_err = cdd_json_value_get_object(extras_val, &extras_obj);

  g_cdd_last_alloc_err = cdd_json_object_get_count(obj, &count);
  for (i = 0; i < count; ++i) {
    const char *key = NULL;
    JSON_Value *val = NULL;
    JSON_Value *copy = NULL;
    int is_ext = 0;
    g_cdd_last_alloc_err = cdd_json_object_get_name(obj, i, &key);
    g_cdd_last_alloc_err = is_extension_key(key, &is_ext);
    if (!is_ext)
      continue;
    g_cdd_last_alloc_err = cdd_json_object_get_value(obj, key, &val);
    rc = clone_json_value(val, &copy);
    if (rc != CDD_C_SUCCESS) {
      g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
      return rc;
    }
#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
      if (g_cdd_fail_json_set_value && --g_cdd_fail_json_set_value == 0) {
        g_cdd_last_alloc_err = cdd_json_value_free(copy);
        g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
        return CDD_C_ERROR_MEMORY;
      }
    }
#endif
    g_cdd_last_alloc_err =
        cdd_json_object_set_value_safe(extras_obj, key, copy);
  }

  g_cdd_last_alloc_err = cdd_json_object_get_count(extras_obj, &extras_count);
  if (extras_count == 0) {
    g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
    return CDD_C_SUCCESS;
  }

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_extensions_serialize;
    if (g_cdd_fail_extensions_serialize &&
        --g_cdd_fail_extensions_serialize == 0)
      serialized = NULL;
    else
      g_cdd_last_alloc_err =
          cdd_json_serialize_to_string(extras_val, &serialized);
  }
#else
  cdd_json_serialize_to_string(extras_val, &serialized);
#endif
  if (!serialized) {
    g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
    return CDD_C_ERROR_MEMORY;
  }
  rc = c_cdd_strdup(serialized, out_json);
  g_cdd_last_alloc_err = cdd_json_free_serialized_string(serialized);
  g_cdd_last_alloc_err = cdd_json_value_free(extras_val);
  return rc;
}

/**
 * @brief Checks if a URL string contains a query (`?`) or fragment (`#`).
 *
 * @param[in] url URL string to inspect.
 * @param[out] out_has_qf Set to 1 if query or fragment is present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t url_has_query_or_fragment(const char *url, int *out_has_qf) {
  if (!out_has_qf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has_qf = 0;
  if (!url)
    return CDD_C_SUCCESS;
  *out_has_qf = (strchr(url, '?') != NULL || strchr(url, '#') != NULL) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if an OpenAPI version string (`3.0.x`, `3.1.x`, `3.2.x`) is
 * supported.
 *
 * @param[in] version Version string.
 * @param[out] out_supported Set to 1 if supported, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t openapi_version_supported(const char *version,
                                        int *out_supported) {
  if (!out_supported)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_supported = 0;
  if (!version || !*version)
    return CDD_C_SUCCESS;
  *out_supported =
      (version[0] == '3' && version[1] == '.' &&
       (version[2] == '0' || version[2] == '1' || version[2] == '2'))
          ? 1
          : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if an Example object's mutual exclusion rules are satisfied.
 *
 * @param[in] ex Example structure to check.
 * @param[out] out_valid Set to 1 if valid, 0 if invalid.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t example_fields_valid(const struct OpenAPI_Example *ex,
                                   int *out_valid) {
  if (!out_valid)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_valid = 0;
  if (!ex) {
    *out_valid = 1;
    return CDD_C_SUCCESS;
  }
  if (ex->data_value_set && ex->value_set)
    return CDD_C_SUCCESS;
  if (ex->serialized_value && ex->external_value)
    return CDD_C_SUCCESS;
  if (ex->value_set && (ex->serialized_value || ex->external_value))
    return CDD_C_SUCCESS;
  *out_valid = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a JSON object contains both `example` and `examples`.
 *
 * @param[in] obj JSON object to inspect.
 * @param[out] out_both Set to 1 if both are present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t object_has_example_and_examples(const JSON_Object *obj,
                                              int *out_both) {
  int has_ex = 0, has_exs = 0;
  if (!out_both)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_both = 0;
  if (!obj)
    return CDD_C_SUCCESS;
  g_cdd_last_alloc_err = cdd_json_object_has_value(obj, "example", &has_ex);
  g_cdd_last_alloc_err = cdd_json_object_has_value(obj, "examples", &has_exs);
  *out_both = (has_ex && has_exs) ? 1 : 0;
  return CDD_C_SUCCESS;
}
