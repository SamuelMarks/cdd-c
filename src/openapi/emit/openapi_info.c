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

#include "c_cdd/json_dom.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include "openapi/emit/openapi.h"
/* clang-format on */

/**
 * @brief Converts verb to string.
 *
 * @param[in] verb The verb.
 * @param[out] _out_val Pointer to the output string.
 * @return 0 on success.
 */
cdd_c_error_t verb_to_str_openapi(enum OpenAPI_Verb verb,
                                  const char **_out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  switch (verb) {
  case OA_VERB_GET:
    *_out_val = "get";
    return CDD_C_SUCCESS;
  case OA_VERB_POST:
    *_out_val = "post";
    return CDD_C_SUCCESS;
  case OA_VERB_PUT:
    *_out_val = "put";
    return CDD_C_SUCCESS;
  case OA_VERB_DELETE:
    *_out_val = "delete";
    return CDD_C_SUCCESS;
  case OA_VERB_PATCH:
    *_out_val = "patch";
    return CDD_C_SUCCESS;
  case OA_VERB_HEAD:
    *_out_val = "head";
    return CDD_C_SUCCESS;
  case OA_VERB_OPTIONS:
    *_out_val = "options";
    return CDD_C_SUCCESS;
  case OA_VERB_TRACE:
    *_out_val = "trace";
    return CDD_C_SUCCESS;
  case OA_VERB_QUERY:
    *_out_val = "query";
    return CDD_C_SUCCESS;
  default:
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Converts parameter in to string.
 *
 * @param[in] in The parameter in.
 * @param[out] _out_val Pointer to the output string.
 * @return 0 on success.
 */
cdd_c_error_t param_in_to_str_openapi(enum OpenAPI_ParamIn in,
                                      const char **_out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  switch (in) {
  case OA_PARAM_IN_PATH:
    *_out_val = "path";
    return CDD_C_SUCCESS;
  case OA_PARAM_IN_QUERY:
    *_out_val = "query";
    return CDD_C_SUCCESS;
  case OA_PARAM_IN_QUERYSTRING:
    *_out_val = "querystring";
    return CDD_C_SUCCESS;
  case OA_PARAM_IN_HEADER:
    *_out_val = "header";
    return CDD_C_SUCCESS;
  case OA_PARAM_IN_COOKIE:
    *_out_val = "cookie";
    return CDD_C_SUCCESS;
  default:
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Converts style to string.
 *
 * @param[in] s The style.
 * @param[out] _out_val Pointer to the output string.
 * @return 0 on success.
 */
cdd_c_error_t style_to_str_openapi(enum OpenAPI_Style s,
                                   const char **_out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  switch (s) {
  case OA_STYLE_FORM:
    *_out_val = "form";
    return CDD_C_SUCCESS;
  case OA_STYLE_SIMPLE:
    *_out_val = "simple";
    return CDD_C_SUCCESS;
  case OA_STYLE_MATRIX:
    *_out_val = "matrix";
    return CDD_C_SUCCESS;
  case OA_STYLE_LABEL:
    *_out_val = "label";
    return CDD_C_SUCCESS;
  case OA_STYLE_SPACE_DELIMITED:
    *_out_val = "spaceDelimited";
    return CDD_C_SUCCESS;
  case OA_STYLE_PIPE_DELIMITED:
    *_out_val = "pipeDelimited";
    return CDD_C_SUCCESS;
  case OA_STYLE_DEEP_OBJECT:
    *_out_val = "deepObject";
    return CDD_C_SUCCESS;
  case OA_STYLE_COOKIE:
    *_out_val = "cookie";
    return CDD_C_SUCCESS;
  default:
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Converts xml node type to string.
 *
 * @param[in] t The xml node type.
 * @param[out] _out_val Pointer to the output string.
 * @return 0 on success.
 */
cdd_c_error_t xml_node_type_to_str_openapi(enum OpenAPI_XmlNodeType t,
                                           const char **_out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  switch (t) {
  case OA_XML_NODE_ELEMENT:
    *_out_val = "element";
    return CDD_C_SUCCESS;
  case OA_XML_NODE_ATTRIBUTE:
    *_out_val = "attribute";
    return CDD_C_SUCCESS;
  case OA_XML_NODE_TEXT:
    *_out_val = "text";
    return CDD_C_SUCCESS;
  case OA_XML_NODE_CDATA:
    *_out_val = "cdata";
    return CDD_C_SUCCESS;
  case OA_XML_NODE_NONE:
    *_out_val = "none";
    return CDD_C_SUCCESS;
  default:
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if header name is content type.
 *
 * @param[in] name The name.
 * @param[out] out_is_ct Set to 1 if true, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t header_name_is_content_type_openapi(const char *name,
                                                  int *out_is_ct) {
  int eq = 0;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!out_is_ct)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_ct = 0;
  if (!name)
    return CDD_C_SUCCESS;
  rc = c_cdd_str_iequal(name, "Content-Type", &eq);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_is_ct = (eq != 0) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if parameter is a reserved header.
 *
 * @param[in] p The parameter.
 * @param[out] out_is_reserved Set to 1 if true, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
cdd_c_error_t
param_is_reserved_header_openapi(const struct OpenAPI_Parameter *p,
                                 int *out_is_reserved) {
  int eq_accept = 0;
  int eq_ct = 0;
  int eq_auth = 0;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!out_is_reserved)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_reserved = 0;
  if (!p || p->in != OA_PARAM_IN_HEADER || !p->name)
    return CDD_C_SUCCESS;
  rc = c_cdd_str_iequal(p->name, "Accept", &eq_accept);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c_cdd_str_iequal(p->name, "Content-Type", &eq_ct);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c_cdd_str_iequal(p->name, "Authorization", &eq_auth);
  if (rc != CDD_C_SUCCESS)
    return rc;
  *out_is_reserved = (eq_accept != 0 || eq_ct != 0 || eq_auth != 0) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Converts oauth flow type to string.
 *
 * @param[in] t The oauth flow type.
 * @param[out] _out_val Pointer to the output string.
 * @return 0 on success.
 */
C_CDD_EXPORT cdd_c_error_t oauth_flow_type_to_str_openapi(
    enum OpenAPI_OAuthFlowType t, const char **_out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  switch (t) {
  case OA_OAUTH_FLOW_IMPLICIT:
    *_out_val = "implicit";
    return CDD_C_SUCCESS;
  case OA_OAUTH_FLOW_PASSWORD:
    *_out_val = "password";
    return CDD_C_SUCCESS;
  case OA_OAUTH_FLOW_CLIENT_CREDENTIALS:
    *_out_val = "clientCredentials";
    return CDD_C_SUCCESS;
  case OA_OAUTH_FLOW_AUTHORIZATION_CODE:
    *_out_val = "authorizationCode";
    return CDD_C_SUCCESS;
  case OA_OAUTH_FLOW_DEVICE_AUTHORIZATION:
    *_out_val = "deviceAuthorization";
    return CDD_C_SUCCESS;
  default:
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if a schema type is primitive.
 *
 * @param[in] type The schema type string.
 * @param[out] out_is_prim Set to 1 if true, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
C_CDD_EXPORT cdd_c_error_t is_schema_primitive_openapi(const char *type,
                                                       int *out_is_prim) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!out_is_prim)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_prim = 0;
  if (!type)
    return CDD_C_SUCCESS;
  *out_is_prim = (strcmp(type, "string") == 0 || strcmp(type, "integer") == 0 ||
                  strcmp(type, "boolean") == 0 || strcmp(type, "number") == 0 ||
                  strcmp(type, "object") == 0 || strcmp(type, "null") == 0)
                     ? 1
                     : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if license fields are invalid.
 *
 * @param[in] lic The license structure to check.
 * @param[out] out_invalid Set to 1 if invalid, 0 if valid.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
C_CDD_EXPORT cdd_c_error_t
license_fields_invalid(const struct OpenAPI_License *lic, int *out_invalid) {
  int has_any;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!out_invalid)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_invalid = 0;
  if (!lic)
    return CDD_C_SUCCESS;
  has_any = lic->name || lic->identifier || lic->url || lic->extensions_json;
  if (!has_any)
    return CDD_C_SUCCESS;
  if (!lic->name || lic->name[0] == '\0') {
    *out_invalid = 1;
    return CDD_C_SUCCESS;
  }
  if (lic->identifier && lic->url) {
    *out_invalid = 1;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if server URL contains query or fragment parts.
 *
 * @param[in] url The server URL to validate.
 * @param[out] out_has_qf Set to 1 if query or fragment is present, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code on invalid argument.
 */
C_CDD_EXPORT cdd_c_error_t server_url_has_query_or_fragment(const char *url,
                                                            int *out_has_qf) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!out_has_qf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_has_qf = 0;
  if (!url)
    return CDD_C_SUCCESS;
  *out_has_qf = (strchr(url, '?') != NULL || strchr(url, '#') != NULL) ? 1 : 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Merges extra schema object properties.
 *
 * @param[in,out] target The target json object.
 * @param[in] extras_json The extra json string.
 * @return 0 on success.
 */
C_CDD_EXPORT cdd_c_error_t merge_schema_extras_object_openapi(
    JSON_Object *target, const char *extras_json) {
  JSON_Value *extras_val = NULL;
  JSON_Object *extras_obj = NULL;
  size_t i, count = 0;
  cdd_c_error_t rc;

  if (!target || !extras_json || extras_json[0] == '\0')
    return CDD_C_SUCCESS;

  rc = cdd_json_parse_string(extras_json, &extras_val);
  if (rc != CDD_C_SUCCESS) {
    if (rc == CDD_C_ERROR_MEMORY)
      return rc;
    return CDD_C_SUCCESS;
  }
  if (extras_val->type != JSONObject) {
    return cdd_json_value_free(extras_val);
  }
  extras_obj = extras_val->object_val;

  rc = cdd_json_object_get_count(extras_obj, &count);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(extras_val));
  }
  for (i = 0; i < count; ++i) {
    const char *key = NULL;
    JSON_Value *val = NULL;
    JSON_Value *copy = NULL;
    int has_val = 0;
    rc = cdd_json_object_get_name(extras_obj, i, &key);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(extras_val));
    }
    rc = cdd_json_object_has_value(target, key, &has_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(extras_val));
    }
    if (has_val)
      continue;
    rc = cdd_json_object_get_value_at(extras_obj, i, &val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(extras_val));
    }
    rc = clone_json_value(val, &copy);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(extras_val));
    }
    rc = cdd_json_object_set_value_safe(target, key, copy);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(extras_val));
    }
  }

  return cdd_json_value_free(extras_val);
}

/**
 * @brief Writes an OpenAPI_Example to a JSON object.
 *
 * @param[in,out] ex_obj The JSON object to populate.
 * @param[in] ex The example structure to write.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t
write_example_object(JSON_Object *ex_obj, const struct OpenAPI_Example *ex) {
  JSON_Value *val = NULL;
  cdd_c_error_t rc;
  if (!ex_obj || !ex)
    return CDD_C_SUCCESS;

  if (ex->ref) {
    rc = cdd_json_object_set_string(ex_obj, "$ref", ex->ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (ex->summary) {
      rc = cdd_json_object_set_string(ex_obj, "summary", ex->summary);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (ex->description) {
      rc = cdd_json_object_set_string(ex_obj, "description", ex->description);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    return CDD_C_SUCCESS;
  }

  if (ex->data_value_set) {
    rc = any_to_json_value(&ex->data_value, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (val) {
      rc = cdd_json_object_set_value_safe(ex_obj, "dataValue", val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  } else if (ex->value_set) {
    rc = any_to_json_value(&ex->value, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (val) {
      rc = cdd_json_object_set_value_safe(ex_obj, "value", val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  rc = openapi_emit_reflected_strings(ex_obj, &OpenAPI_Example_type_info, ex);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (ex->extensions_json) {
    return merge_schema_extras_object_openapi(ex_obj, ex->extensions_json);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes an examples dictionary to parent JSON object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] key The property key name.
 * @param[in] examples Array of example structures.
 * @param[in] n_examples Number of examples in the array.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_examples_object(
    JSON_Object *parent, const char *key,
    const struct OpenAPI_Example *examples, size_t n_examples) {
  JSON_Value *examples_val = NULL;
  JSON_Object *examples_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!parent || !key || !examples || n_examples == 0)
    return CDD_C_SUCCESS;

  rc = oa_emit_new_object(&examples_val, &examples_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < n_examples; ++i) {
    const struct OpenAPI_Example *ex = &examples[i];
    if (!ex->name)
      continue;
    {
      JSON_Value *ex_val = NULL;
      JSON_Object *ex_obj = NULL;
      rc = oa_emit_new_object(&ex_val, &ex_obj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(examples_val));
      }
      rc = write_example_object(ex_obj, ex);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t f1 = cdd_json_value_free(ex_val);
        cdd_c_error_t f2 = cdd_json_value_free(examples_val);
        return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
      }
      rc = cdd_json_object_set_value_safe(examples_obj, ex->name, ex_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(examples_val));
      }
    }
  }

  return cdd_json_object_set_value_safe(parent, key, examples_val);
}

/**
 * @brief Writes example or examples fields to a JSON object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] example Single example structure.
 * @param[in] example_set Non-zero if single example is set.
 * @param[in] examples Array of multiple examples.
 * @param[in] n_examples Count of multiple examples.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t write_example_fields(
    JSON_Object *parent, const struct OpenAPI_Any *example, int example_set,
    const struct OpenAPI_Example *examples, size_t n_examples) {
  cdd_c_error_t rc;
  if (!parent)
    return CDD_C_SUCCESS;
  if (examples && n_examples > 0) {
    return write_examples_object(parent, "examples", examples, n_examples);
  }
  if (example_set && example) {
    JSON_Value *val = NULL;
    rc = any_to_json_value(example, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (val) {
      return cdd_json_object_set_value_safe(parent, "example", val);
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Table-driven helper to serialize reflected fields into a `JSON_Object`
 * using `cdd_reflect_to_json_object`.
 *
 * @param[in,out] obj Destination JSON object.
 * @param[in] ti Runtime reflection descriptor.
 * @param[in] src Source C struct pointer.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
cdd_c_error_t openapi_emit_reflected_strings(JSON_Object *obj,
                                             const struct cdd_type_info *ti,
                                             const void *src) {
  return cdd_reflect_to_json_object(src, ti, obj);
}

/**
 * @brief Writes external documentation object to parent JSON object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] key The property key name.
 * @param[in] docs The external documentation structure.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t
write_external_docs(JSON_Object *parent, const char *key,
                    const struct OpenAPI_ExternalDocs *docs) {
  JSON_Value *ext_val = NULL;
  JSON_Object *ext_obj = NULL;
  cdd_c_error_t rc;

  if (!parent || !docs || !docs->url)
    return CDD_C_SUCCESS;

  rc = oa_emit_new_object(&ext_val, &ext_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = openapi_emit_reflected_strings(ext_obj, &OpenAPI_ExternalDocs_type_info,
                                      docs);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(ext_val));
  }
  if (docs->extensions_json) {
    rc = merge_schema_extras_object_openapi(ext_obj, docs->extensions_json);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(ext_val));
    }
  }

  return cdd_json_object_set_value_safe(parent, key, ext_val);
}

/**
 * @brief Writes discriminator object to parent JSON object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] disc The discriminator structure.
 * @param[in] disc_set Non-zero if discriminator is set.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t write_discriminator_object(
    JSON_Object *parent, const struct OpenAPI_Discriminator *disc,
    int disc_set) {
  JSON_Value *disc_val = NULL;
  JSON_Object *disc_obj = NULL;
  JSON_Value *mapping_val = NULL;
  JSON_Object *mapping_obj = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!parent || !disc || !disc_set)
    return CDD_C_SUCCESS;
  if (!disc->property_name && disc->n_mapping == 0 && !disc->default_mapping)
    return CDD_C_SUCCESS;

  rc = oa_emit_new_object(&disc_val, &disc_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = openapi_emit_reflected_strings(disc_obj,
                                      &OpenAPI_Discriminator_type_info, disc);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_disc;

  if (disc->mapping && disc->n_mapping > 0) {
    rc = oa_emit_new_object(&mapping_val, &mapping_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_disc;
    for (i = 0; i < disc->n_mapping; ++i) {
      const struct OpenAPI_DiscriminatorMap *m = &disc->mapping[i];
      if (m->value && m->schema) {
        rc = cdd_json_object_set_string(mapping_obj, m->value, m->schema);
        if (rc != CDD_C_SUCCESS) {
          cdd_c_error_t f1 = cdd_json_value_free(mapping_val);
          cdd_c_error_t f2 = cdd_json_value_free(disc_val);
          return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
        }
      }
    }
    rc = cdd_json_object_set_value_safe(disc_obj, "mapping", mapping_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_disc;
  }

  if (disc->extensions_json) {
    rc = merge_schema_extras_object_openapi(disc_obj, disc->extensions_json);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_disc;
  }

  return cdd_json_object_set_value_safe(parent, "discriminator", disc_val);

cleanup_disc: { return cdd_c_keep_err(rc, cdd_json_value_free(disc_val)); }
}

/**
 * @brief Writes XML object to parent JSON object.
 *
 * @param[in,out] parent The parent JSON object.
 * @param[in] xml The XML structure.
 * @param[in] xml_set Non-zero if XML structure is set.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t write_xml_object(JSON_Object *parent,
                                            const struct OpenAPI_Xml *xml,
                                            int xml_set) {
  JSON_Value *xml_val = NULL;
  JSON_Object *xml_obj = NULL;
  const char *node_type = NULL;
  cdd_c_error_t rc;

  if (!parent || !xml || !xml_set)
    return CDD_C_SUCCESS;

  rc = oa_emit_new_object(&xml_val, &xml_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = xml_node_type_to_str_openapi(xml->node_type, &node_type);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_xml;
  if (xml->node_type_set && node_type) {
    rc = cdd_json_object_set_string(xml_obj, "nodeType", node_type);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_xml;
  }
  rc = openapi_emit_reflected_strings(xml_obj, &OpenAPI_Xml_type_info, xml);
  if (rc != CDD_C_SUCCESS)
    goto cleanup_xml;
  if (xml->attribute_set) {
    rc = cdd_json_object_set_boolean(xml_obj, "attribute",
                                     xml->attribute ? 1 : 0);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_xml;
  }
  if (xml->wrapped_set) {
    rc = cdd_json_object_set_boolean(xml_obj, "wrapped", xml->wrapped ? 1 : 0);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_xml;
  }
  if (xml->extensions_json) {
    rc = merge_schema_extras_object_openapi(xml_obj, xml->extensions_json);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_xml;
  }

  return cdd_json_object_set_value_safe(parent, "xml", xml_val);

cleanup_xml: { return cdd_c_keep_err(rc, cdd_json_value_free(xml_val)); }
}

C_CDD_EXPORT cdd_c_error_t OpenAPI_Contact_to_jsonObject(
    const struct OpenAPI_Contact *contact, JSON_Object *contact_obj) {
  cdd_c_error_t rc = openapi_emit_reflected_strings(
      contact_obj, &OpenAPI_Contact_type_info, contact);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (contact->extensions_json) {
    return merge_schema_extras_object_openapi(contact_obj,
                                              contact->extensions_json);
  }
  return CDD_C_SUCCESS;
}

C_CDD_EXPORT cdd_c_error_t OpenAPI_License_to_jsonObject(
    const struct OpenAPI_License *license, JSON_Object *license_obj) {
  cdd_c_error_t rc = openapi_emit_reflected_strings(
      license_obj, &OpenAPI_License_type_info, license);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (license->extensions_json) {
    return merge_schema_extras_object_openapi(license_obj,
                                              license->extensions_json);
  }
  return CDD_C_SUCCESS;
}

C_CDD_EXPORT cdd_c_error_t OpenAPI_Info_to_jsonObject(
    const struct OpenAPI_Info *info, JSON_Object *info_obj) {
  JSON_Value *contact_val = NULL;
  JSON_Object *contact_obj = NULL;
  JSON_Value *license_val = NULL;
  JSON_Object *license_obj = NULL;
  const char *title = info->title ? info->title : "Generated Specification";
  const char *version = info->version ? info->version : "1.0.0";
  cdd_c_error_t rc;

  rc = openapi_emit_reflected_strings(info_obj, &OpenAPI_Info_type_info, info);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_object_set_string(info_obj, "title", title);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_object_set_string(info_obj, "version", version);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (info->extensions_json) {
    rc = merge_schema_extras_object_openapi(info_obj, info->extensions_json);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (info->contact.name || info->contact.url || info->contact.email) {
    rc = oa_emit_new_object(&contact_val, &contact_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = OpenAPI_Contact_to_jsonObject(&info->contact, contact_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(contact_val));
    }
    rc = cdd_json_object_set_value_safe(info_obj, "contact", contact_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (info->license.name || info->license.identifier || info->license.url ||
      info->license.extensions_json) {
    rc = oa_emit_new_object(&license_val, &license_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = OpenAPI_License_to_jsonObject(&info->license, license_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(license_val));
    }
    rc = cdd_json_object_set_value_safe(info_obj, "license", license_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes info metadata object to root specification object.
 *
 * @param[in,out] root_obj The root JSON object.
 * @param[in] spec The OpenAPI specification structure.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t write_info(JSON_Object *root_obj,
                                      const struct OpenAPI_Spec *spec) {
  JSON_Value *info_val = NULL;
  JSON_Object *info_obj = NULL;
  cdd_c_error_t rc;
  rc = oa_emit_new_object(&info_val, &info_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = OpenAPI_Info_to_jsonObject(&spec->info, info_obj);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(info_val));
  }
  return cdd_json_object_set_value_safe(root_obj, "info", info_val);
}

/**
 * @brief Writes server object fields to JSON object.
 *
 * @param[in,out] srv_obj The server JSON object.
 * @param[in] srv The OpenAPI server structure.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t
write_server_object(JSON_Object *srv_obj, const struct OpenAPI_Server *srv) {
  cdd_c_error_t rc;
  if (!srv_obj || !srv)
    return CDD_C_SUCCESS;

  rc = cdd_json_object_set_string(srv_obj, "url", srv->url ? srv->url : "/");
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = openapi_emit_reflected_strings(srv_obj, &OpenAPI_Server_type_info, srv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (srv->n_variables > 0 && srv->variables) {
    JSON_Value *vars_val = NULL;
    JSON_Object *vars_obj = NULL;
    size_t v;
    rc = oa_emit_new_object(&vars_val, &vars_obj);
    if (rc != CDD_C_SUCCESS)
      return rc;
    for (v = 0; v < srv->n_variables; ++v) {
      const struct OpenAPI_ServerVariable *var = &srv->variables[v];
      JSON_Value *var_val = NULL;
      JSON_Object *var_obj = NULL;
      rc = oa_emit_new_object(&var_val, &var_obj);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(vars_val));
      }
      rc = openapi_emit_reflected_strings(
          var_obj, &OpenAPI_ServerVariable_type_info, var);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t f1 = cdd_json_value_free(var_val);
        cdd_c_error_t f2 = cdd_json_value_free(vars_val);
        return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
      }
      if (var->n_enum_values > 0 && var->enum_values) {
        JSON_Value *enum_val = NULL;
        JSON_Array *enum_arr = NULL;
        size_t e;
        rc = oa_emit_new_array(&enum_val, &enum_arr);
        if (rc != CDD_C_SUCCESS) {
          cdd_c_error_t f1 = cdd_json_value_free(var_val);
          cdd_c_error_t f2 = cdd_json_value_free(vars_val);
          return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
        }
        for (e = 0; e < var->n_enum_values; ++e) {
          if (var->enum_values[e]) {
            rc = cdd_json_array_append_string(enum_arr, var->enum_values[e]);
            if (rc != CDD_C_SUCCESS) {
              cdd_c_error_t f0 = cdd_json_value_free(enum_val);
              cdd_c_error_t f1 = cdd_json_value_free(var_val);
              cdd_c_error_t f2 = cdd_json_value_free(vars_val);
              return cdd_c_keep_err(rc,
                                    cdd_c_keep_err(f0, cdd_c_keep_err(f1, f2)));
            }
          }
        }
        rc = cdd_json_object_set_value_safe(var_obj, "enum", enum_val);
        if (rc != CDD_C_SUCCESS) {
          cdd_c_error_t f1 = cdd_json_value_free(var_val);
          cdd_c_error_t f2 = cdd_json_value_free(vars_val);
          return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
        }
      }
      if (var->extensions_json) {
        rc = merge_schema_extras_object_openapi(var_obj, var->extensions_json);
        if (rc != CDD_C_SUCCESS) {
          cdd_c_error_t f1 = cdd_json_value_free(var_val);
          cdd_c_error_t f2 = cdd_json_value_free(vars_val);
          return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
        }
      }
      rc = cdd_json_object_set_value_safe(
          vars_obj, var->name ? var->name : "var", var_val);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(vars_val));
      }
    }
    rc = cdd_json_object_set_value_safe(srv_obj, "variables", vars_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (srv->extensions_json) {
    return merge_schema_extras_object_openapi(srv_obj, srv->extensions_json);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes servers array to root JSON object.
 *
 * @param[in,out] root_obj Root JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_servers(JSON_Object *root_obj,
                                         const struct OpenAPI_Spec *spec) {
  if (!spec || spec->n_servers == 0 || !spec->servers)
    return CDD_C_SUCCESS;
  return write_server_array(root_obj, "servers", spec->servers,
                            spec->n_servers);
}

/**
 * @brief Writes server array to parent JSON object.
 *
 * @param[in,out] parent Parent JSON object.
 * @param[in] key Property key name.
 * @param[in] servers Array of server structures.
 * @param[in] n_servers Number of servers.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t
write_server_array(JSON_Object *parent, const char *key,
                   const struct OpenAPI_Server *servers, size_t n_servers) {
  JSON_Value *arr_val = NULL;
  JSON_Array *arr = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!parent || !key || !servers || n_servers == 0)
    return CDD_C_SUCCESS;

  rc = oa_emit_new_array(&arr_val, &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < n_servers; ++i) {
    JSON_Value *srv_val = NULL;
    JSON_Object *srv_obj = NULL;
    int has_qf = 0;
    const struct OpenAPI_Server *srv = &servers[i];
    rc = server_url_has_query_or_fragment(srv->url, &has_qf);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
    }
    if (has_qf) {
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_json_value_free(arr_val));
    }
    rc = oa_emit_new_object(&srv_val, &srv_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
    }
    rc = write_server_object(srv_obj, srv);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(srv_val);
      cdd_c_error_t f2 = cdd_json_value_free(arr_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }

    rc = cdd_json_array_append_value_safe(arr, srv_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
    }
  }

  return cdd_json_object_set_value_safe(parent, key, arr_val);
}

/**
 * @brief Writes tags array to root JSON object.
 *
 * @param[in,out] root_obj Root JSON object.
 * @param[in] spec Specification structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t write_tags(JSON_Object *root_obj,
                                      const struct OpenAPI_Spec *spec) {
  JSON_Value *arr_val = NULL;
  JSON_Array *arr = NULL;
  size_t i;
  cdd_c_error_t rc;

  if (!spec || spec->n_tags == 0)
    return CDD_C_SUCCESS;

  rc = oa_emit_new_array(&arr_val, &arr);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < spec->n_tags; ++i) {
    const struct OpenAPI_Tag *tag = &spec->tags[i];
    JSON_Value *tag_val = NULL;
    JSON_Object *tag_obj = NULL;
    rc = oa_emit_new_object(&tag_val, &tag_obj);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
    }

    if (tag->name) {
      rc = cdd_json_object_set_string(tag_obj, "name", tag->name);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t f1 = cdd_json_value_free(tag_val);
        cdd_c_error_t f2 = cdd_json_value_free(arr_val);
        return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
      }
    }
    rc = openapi_emit_reflected_strings(tag_obj, &OpenAPI_Tag_type_info, tag);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t f1 = cdd_json_value_free(tag_val);
      cdd_c_error_t f2 = cdd_json_value_free(arr_val);
      return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
    }
    if (tag->external_docs.url) {
      rc = write_external_docs(tag_obj, "externalDocs", &tag->external_docs);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t f1 = cdd_json_value_free(tag_val);
        cdd_c_error_t f2 = cdd_json_value_free(arr_val);
        return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
      }
    }
    if (tag->extensions_json) {
      rc = merge_schema_extras_object_openapi(tag_obj, tag->extensions_json);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t f1 = cdd_json_value_free(tag_val);
        cdd_c_error_t f2 = cdd_json_value_free(arr_val);
        return cdd_c_keep_err(rc, cdd_c_keep_err(f1, f2));
      }
    }

    rc = cdd_json_array_append_value_safe(arr, tag_val);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
    }
  }

  return cdd_json_object_set_value_safe(root_obj, "tags", arr_val);
}
