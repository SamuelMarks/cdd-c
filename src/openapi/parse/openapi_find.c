/**
 * @file openapi_find.c
 * @brief Component lookup routines by name or reference.
 * @author Samuel Marks
 */

/* clang-format off */
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

/**
 * @brief Retrieves the component parameter.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found parameter.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t find_component_parameter(const struct OpenAPI_Spec *spec,
                                       const char *ref,
                                       struct OpenAPI_Parameter **_out_val) {
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  const char *name = NULL;
  size_t i;
  resolve_ref_target(spec, ref, &resolved);
  target = resolved.spec;
  ref_name_from_prefix(target, resolved.ref, "#/components/parameters/", &name);
  if (!target || !name) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < target->n_component_parameters; ++i) {
    if (target->component_parameter_names[i] &&
        strcmp(target->component_parameter_names[i], name) == 0) {
      if (resolved.resolved_ref)
        C_CDD_FREE(resolved.resolved_ref);
      *_out_val = &target->component_parameters[i];
      return CDD_C_SUCCESS;
    }
  }
  if (resolved.resolved_ref)
    C_CDD_FREE(resolved.resolved_ref);
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the component response.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found response.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t find_component_response(const struct OpenAPI_Spec *spec,
                                      const char *ref,
                                      struct OpenAPI_Response **_out_val) {
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  const char *name = NULL;
  size_t i;
  resolve_ref_target(spec, ref, &resolved);
  target = resolved.spec;
  ref_name_from_prefix(target, resolved.ref, "#/components/responses/", &name);
  if (!target || !name) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < target->n_component_responses; ++i) {
    if (target->component_response_names[i] &&
        strcmp(target->component_response_names[i], name) == 0) {
      if (resolved.resolved_ref)
        C_CDD_FREE(resolved.resolved_ref);
      *_out_val = &target->component_responses[i];
      return CDD_C_SUCCESS;
    }
  }
  if (resolved.resolved_ref)
    C_CDD_FREE(resolved.resolved_ref);
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the component header.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found header.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t find_component_header(const struct OpenAPI_Spec *spec,
                                    const char *ref,
                                    struct OpenAPI_Header **_out_val) {
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  const char *name = NULL;
  size_t i;
  resolve_ref_target(spec, ref, &resolved);
  target = resolved.spec;
  ref_name_from_prefix(target, resolved.ref, "#/components/headers/", &name);
  if (!target || !name) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < target->n_component_headers; ++i) {
    if (target->component_header_names[i] &&
        strcmp(target->component_header_names[i], name) == 0) {
      if (resolved.resolved_ref)
        C_CDD_FREE(resolved.resolved_ref);
      *_out_val = &target->component_headers[i];
      return CDD_C_SUCCESS;
    }
  }
  if (resolved.resolved_ref)
    C_CDD_FREE(resolved.resolved_ref);
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the component request body.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found request body.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t
find_component_request_body(const struct OpenAPI_Spec *spec, const char *ref,
                            struct OpenAPI_RequestBody **_out_val) {
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  const char *name = NULL;
  size_t i;
  resolve_ref_target(spec, ref, &resolved);
  target = resolved.spec;
  ref_name_from_prefix(target, resolved.ref, "#/components/requestBodies/",
                       &name);
  if (!target || !name) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < target->n_component_request_bodies; ++i) {
    if (target->component_request_body_names[i] &&
        strcmp(target->component_request_body_names[i], name) == 0) {
      if (resolved.resolved_ref)
        C_CDD_FREE(resolved.resolved_ref);
      *_out_val = &target->component_request_bodies[i];
      return CDD_C_SUCCESS;
    }
  }
  if (resolved.resolved_ref)
    C_CDD_FREE(resolved.resolved_ref);
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the component media type.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found media type.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t find_component_media_type(const struct OpenAPI_Spec *spec,
                                        const char *ref,
                                        struct OpenAPI_MediaType **_out_val) {
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  const char *name_enc = NULL;
  char *name_dec = NULL;
  size_t i;
  resolve_ref_target(spec, ref, &resolved);
  target = resolved.spec;
  ref_name_from_prefix(target, resolved.ref, "#/components/mediaTypes/",
                       &name_enc);
  if (!target || !name_enc) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  json_pointer_unescape(name_enc, &name_dec);
  if (!name_dec) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < target->n_component_media_types; ++i) {
    if (target->component_media_type_names[i] &&
        strcmp(target->component_media_type_names[i], name_dec) == 0) {
      C_CDD_FREE(name_dec);
      if (resolved.resolved_ref)
        C_CDD_FREE(resolved.resolved_ref);
      *_out_val = &target->component_media_types[i];
      return CDD_C_SUCCESS;
    }
  }
  C_CDD_FREE(name_dec);
  if (resolved.resolved_ref)
    C_CDD_FREE(resolved.resolved_ref);
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the component link.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found link.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t find_component_link(const struct OpenAPI_Spec *spec,
                                  const char *ref,
                                  struct OpenAPI_Link **_out_val) {
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  const char *name = NULL;
  size_t i;
  resolve_ref_target(spec, ref, &resolved);
  target = resolved.spec;
  ref_name_from_prefix(target, resolved.ref, "#/components/links/", &name);
  if (!target || !name) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < target->n_component_links; ++i) {
    if (target->component_links[i].name &&
        strcmp(target->component_links[i].name, name) == 0) {
      if (resolved.resolved_ref)
        C_CDD_FREE(resolved.resolved_ref);
      *_out_val = &target->component_links[i];
      return CDD_C_SUCCESS;
    }
  }
  if (resolved.resolved_ref)
    C_CDD_FREE(resolved.resolved_ref);
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the component callback.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found callback.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t find_component_callback(const struct OpenAPI_Spec *spec,
                                      const char *ref,
                                      struct OpenAPI_Callback **_out_val) {
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  const char *name = NULL;
  size_t i;
  resolve_ref_target(spec, ref, &resolved);
  target = resolved.spec;
  ref_name_from_prefix(target, resolved.ref, "#/components/callbacks/", &name);
  if (!target || !name) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < target->n_component_callbacks; ++i) {
    if (target->component_callbacks[i].name &&
        strcmp(target->component_callbacks[i].name, name) == 0) {
      if (resolved.resolved_ref)
        C_CDD_FREE(resolved.resolved_ref);
      *_out_val = &target->component_callbacks[i];
      return CDD_C_SUCCESS;
    }
  }
  if (resolved.resolved_ref)
    C_CDD_FREE(resolved.resolved_ref);
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the component path item.
 *
 * @param[in] spec The OpenAPI specification.
 * @param[in] ref The component reference string.
 * @param[out] _out_val Output pointer to the found path item.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t find_component_path_item(const struct OpenAPI_Spec *spec,
                                       const char *ref,
                                       struct OpenAPI_Path **_out_val) {
  struct ResolvedRefTarget resolved;
  const struct OpenAPI_Spec *target;
  const char *name_enc = NULL;
  char *name_dec = NULL;
  size_t i;
  resolve_ref_target(spec, ref, &resolved);
  target = resolved.spec;
  ref_name_from_prefix(target, resolved.ref, "#/components/pathItems/",
                       &name_enc);
  if (!target || !name_enc) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  json_pointer_unescape(name_enc, &name_dec);
  if (!name_dec) {
    if (resolved.resolved_ref)
      C_CDD_FREE(resolved.resolved_ref);
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < target->n_component_path_items; ++i) {
    if (target->component_path_item_names &&
        target->component_path_item_names[i] &&
        strcmp(target->component_path_item_names[i], name_dec) == 0) {
      C_CDD_FREE(name_dec);
      if (resolved.resolved_ref)
        C_CDD_FREE(resolved.resolved_ref);
      *_out_val = &target->component_path_items[i];
      return CDD_C_SUCCESS;
    }
  }
  C_CDD_FREE(name_dec);
  if (resolved.resolved_ref)
    C_CDD_FREE(resolved.resolved_ref);
  *_out_val = NULL;
  return CDD_C_SUCCESS;
}
