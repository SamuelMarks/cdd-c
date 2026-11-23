/**
 * @file code2schema_mapper.c
 * @brief Mapping JSON schema properties to struct fields.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/** @brief Keys skipped when collecting extra root schema fields */
static const char *k_schema_skip_keys[] = {"type",
                                           "$ref",
                                           "properties",
                                           "required",
                                           "allOf",
                                           "anyOf",
                                           "oneOf",
                                           "x-cdd-c-name",
                                           "x-cdd-rename",
                                           "x-cdd-rename-all",
                                           "x-cdd-rename-all-serialize",
                                           "x-cdd-rename-all-deserialize",
                                           "x-cdd-replace-from",
                                           "x-cdd-replace-to",
                                           "x-cdd-prefix",
                                           "x-cdd-suffix",
                                           "x-cdd-deny-unknown-fields",
                                           "x-cdd-default",
                                           "x-cdd-tag",
                                           "x-cdd-content",
                                           "x-cdd-untagged",
                                           "x-cdd-enum-names",
                                           "x-cdd-enum-c-names",
                                           "x-cdd-enum-skip",
                                           "x-cdd-enum-aliases",
                                           "x-cdd-enum-other",
                                           "x-cdd-cli-command",
                                           "x-cdd-cli-name",
                                           "x-cdd-cli-version",
                                           "x-cdd-cli-about",
                                           "x-cdd-cli-author",
                                           "x-cdd-cli-rename-all",
                                           "x-cdd-cli-env-prefix"};

/** @brief Keys skipped when collecting extra property schema fields */
static const char *k_property_skip_keys[] = {"type",
                                             "$ref",
                                             "items",
                                             "default",
                                             "minimum",
                                             "maximum",
                                             "exclusiveMinimum",
                                             "exclusiveMaximum",
                                             "minLength",
                                             "maxLength",
                                             "pattern",
                                             "minItems",
                                             "maxItems",
                                             "uniqueItems",
                                             "description",
                                             "format",
                                             "deprecated",
                                             "readOnly",
                                             "writeOnly",
                                             "x-c-bitwidth",
                                             "x-cdd-c-name",
                                             "x-cdd-rename",
                                             "x-cdd-rename-serialize",
                                             "x-cdd-rename-deserialize",
                                             "x-cdd-case",
                                             "x-cdd-replace-from",
                                             "x-cdd-replace-to",
                                             "x-cdd-aliases",
                                             "x-cdd-skip",
                                             "x-cdd-skip-serializing",
                                             "x-cdd-skip-deserializing",
                                             "x-cdd-skip-serializing-if",
                                             "x-cdd-flatten",
                                             "x-cdd-default",
                                             "x-cdd-default-fn",
                                             "x-cdd-with",
                                             "x-cdd-serialize-with",
                                             "x-cdd-deserialize-with",
                                             "x-cdd-cli-short",
                                             "x-cdd-cli-long",
                                             "x-cdd-cli-negatable",
                                             "x-cdd-cli-positional",
                                             "x-cdd-cli-index",
                                             "x-cdd-cli-env",
                                             "x-cdd-cli-value-name",
                                             "x-cdd-cli-help",
                                             "x-cdd-cli-action",
                                             "x-cdd-cli-delimiter",
                                             "x-cdd-cli-subcommand",
                                             "x-cdd-cli-conflicts-with",
                                             "x-cdd-cli-requires",
                                             "x-cdd-cli-skip",
                                             "x-cdd-cli-skip-to",
                                             "x-cdd-cli-skip-from"};

/** @brief Keys skipped when collecting extra array items schema fields */
static const char *k_items_skip_keys[] = {"type", "$ref"};

/**
 * @brief Optionally parses a Serde case rule string, leaving `*target`
 * unchanged if unrecognized.
 *
 * @param[in] str Candidate case rule string.
 * @param[in,out] target Output rule updated only when `str` is valid.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_try_parse_case_rule(const char *str,
                                             enum CddSerdeCaseRule *target) {
  enum CddSerdeCaseRule parsed = CDD_SERDE_CASE_NONE;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  rc = cdd_serde_parse_case_rule(str, &parsed);
  if (rc == CDD_C_SUCCESS) {
    *target = parsed;
    return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Optionally parses a CLI action string, leaving `*target` unchanged if
 * unrecognized.
 *
 * @param[in] str Candidate CLI action string.
 * @param[in,out] target Output action updated only when `str` is valid.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_try_parse_cli_action(const char *str,
                                              enum CddCliAction *target) {
  enum CddCliAction parsed = CDD_CLI_ACTION_AUTO;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  rc = cdd_cli_parse_action(str, &parsed);
  if (rc == CDD_C_SUCCESS) {
    *target = parsed;
    return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Optionally parses a CLI value hint string, leaving `*target` unchanged
 * if unrecognized.
 *
 * @param[in] str Candidate CLI value hint string.
 * @param[in,out] target Output hint updated only when `str` is valid.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t
c2s_try_parse_cli_value_hint(const char *str, enum CddCliValueHint *target) {
  enum CddCliValueHint parsed = CDD_CLI_HINT_NONE;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  rc = cdd_cli_parse_value_hint(str, &parsed);
  if (rc == CDD_C_SUCCESS) {
    *target = parsed;
    return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Looks up an optional string element at `idx` in `arr`, setting
 * `*out_str` to NULL when out-of-bounds or non-string.
 *
 * @param[in] arr JSON_Array to inspect (may be NULL).
 * @param[in] idx Element index.
 * @param[out] out_str Output pointer receiving the string or NULL.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_arr_opt_string(const JSON_Array *arr, size_t idx,
                                        const char **out_str) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_str = NULL;
  rc = cdd_json_array_get_string(arr, idx, out_str);
  if (*out_str != NULL)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Looks up an optional JSON_Value property on `obj`, setting `*out_val`
 * to NULL when absent.
 *
 * @param[in] obj JSON_Object to inspect.
 * @param[in] key Property name.
 * @param[out] out_val Output pointer receiving the JSON_Value or NULL.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_json_opt_value(const JSON_Object *obj, const char *key,
                                        JSON_Value **out_val) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_val = NULL;
  rc = cdd_json_object_get_value(obj, key, out_val);
  if (*out_val != NULL)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Looks up an optional numeric property on `obj`.
 *
 * @param[in] obj JSON_Object to inspect.
 * @param[in] key Property name.
 * @param[in,out] out_num Output double updated only when a JSONNumber is found.
 * @param[out] out_found Set to 1 if a JSONNumber was found, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_json_opt_number(const JSON_Object *obj,
                                         const char *key, double *out_num,
                                         int *out_found) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_found = 0;
  rc = cdd_json_object_get_number(obj, key, out_num);
  if (rc == CDD_C_SUCCESS) {
    *out_found = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Looks up an optional boolean property on `obj`.
 *
 * @param[in] obj JSON_Object to inspect.
 * @param[in] key Property name.
 * @param[out] out_bool Output integer receiving 1 or 0 (set to 0 if absent).
 * @param[out] out_found Set to 1 if a JSONBoolean was found, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_json_opt_bool(const JSON_Object *obj, const char *key,
                                       int *out_bool, int *out_found) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_bool = 0;
  *out_found = 0;
  rc = cdd_json_object_get_boolean(obj, key, out_bool);
  if (rc == CDD_C_SUCCESS) {
    *out_found = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Loads container-level Serde and CLI extensions from `o` into `f`.
 *
 * @param[in] o Schema JSON_Object.
 * @param[in,out] f Target StructFields container.
 * @param[in] schema_name Optional schema name.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_load_container_extensions(const JSON_Object *o,
                                                   struct StructFields *f,
                                                   const char *schema_name) {
  size_t k;
  cdd_c_error_t rc;

  if (schema_name) {
    int needs_san = 0;
    rc = c2s_needs_identifier_sanitization(schema_name, &needs_san);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (needs_san) {
      char *san_schema = NULL;
      rc = sanitize_identifier(schema_name, &san_schema);
      if (rc != CDD_C_SUCCESS)
        return rc;
      CDD_STRCPY(f->serde_rename, sizeof(f->serde_rename), schema_name);
      C_CDD_FREE(san_schema);
    }
  }

  for (k = 0; k < o->count; ++k) {
    const char *key = o->names[k];
    const JSON_Value *val = o->values[k];
    if (strncmp(key, "x-cdd-", 6) != 0)
      continue;
    if (val->type == JSONString) {
      const char *s_ext = val->string_val;
      if (strcmp(key, "x-cdd-rename") == 0) {
        CDD_STRCPY(f->serde_rename, sizeof(f->serde_rename), s_ext);
      } else if (strcmp(key, "x-cdd-rename-all") == 0) {
        rc = c2s_try_parse_case_rule(s_ext, &f->serde_rename_all);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-rename-all-serialize") == 0) {
        rc = c2s_try_parse_case_rule(s_ext, &f->serde_rename_all_ser);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-rename-all-deserialize") == 0) {
        rc = c2s_try_parse_case_rule(s_ext, &f->serde_rename_all_de);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-replace-from") == 0) {
        CDD_STRCPY(f->serde_replace_from, sizeof(f->serde_replace_from), s_ext);
      } else if (strcmp(key, "x-cdd-replace-to") == 0) {
        CDD_STRCPY(f->serde_replace_to, sizeof(f->serde_replace_to), s_ext);
      } else if (strcmp(key, "x-cdd-prefix") == 0) {
        CDD_STRCPY(f->serde_prefix, sizeof(f->serde_prefix), s_ext);
      } else if (strcmp(key, "x-cdd-suffix") == 0) {
        CDD_STRCPY(f->serde_suffix, sizeof(f->serde_suffix), s_ext);
      } else if (strcmp(key, "x-cdd-tag") == 0) {
        CDD_STRCPY(f->serde_tag, sizeof(f->serde_tag), s_ext);
      } else if (strcmp(key, "x-cdd-content") == 0) {
        CDD_STRCPY(f->serde_content, sizeof(f->serde_content), s_ext);
      } else if (strcmp(key, "x-cdd-cli-name") == 0) {
        f->is_cli_command = 1;
        CDD_STRCPY(f->cli_name, sizeof(f->cli_name), s_ext);
      } else if (strcmp(key, "x-cdd-cli-version") == 0) {
        f->is_cli_command = 1;
        CDD_STRCPY(f->cli_version, sizeof(f->cli_version), s_ext);
      } else if (strcmp(key, "x-cdd-cli-about") == 0) {
        f->is_cli_command = 1;
        CDD_STRCPY(f->cli_about, sizeof(f->cli_about), s_ext);
      } else if (strcmp(key, "x-cdd-cli-author") == 0) {
        f->is_cli_command = 1;
        CDD_STRCPY(f->cli_author, sizeof(f->cli_author), s_ext);
      } else if (strcmp(key, "x-cdd-cli-rename-all") == 0) {
        rc = c2s_try_parse_case_rule(s_ext, &f->cli_rename_all);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-cli-env-prefix") == 0) {
        f->is_cli_command = 1;
        CDD_STRCPY(f->cli_env_prefix, sizeof(f->cli_env_prefix), s_ext);
      } else if (strcmp(key, "x-cdd-cli-bin-name") == 0) {
        f->is_cli_command = 1;
        CDD_STRCPY(f->cli_bin_name, sizeof(f->cli_bin_name), s_ext);
      } else if (strcmp(key, "x-cdd-cli-long-about") == 0) {
        f->is_cli_command = 1;
        rc = c_cdd_strdup(s_ext, &f->cli_long_about);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-cli-before-help") == 0) {
        f->is_cli_command = 1;
        rc = c_cdd_strdup(s_ext, &f->cli_before_help);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-cli-after-help") == 0) {
        f->is_cli_command = 1;
        rc = c_cdd_strdup(s_ext, &f->cli_after_help);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-cli-usage-override") == 0) {
        f->is_cli_command = 1;
        CDD_STRCPY(f->cli_usage_override, sizeof(f->cli_usage_override), s_ext);
      } else if (strcmp(key, "x-cdd-cli-rename-all-env") == 0) {
        rc = c2s_try_parse_case_rule(s_ext, &f->cli_rename_all_env);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (val->type == JSONBoolean) {
      int bv = val->boolean_val;
      if (strcmp(key, "x-cdd-deny-unknown-fields") == 0) {
        f->serde_deny_unknown_fields |= bv;
      } else if (strcmp(key, "x-cdd-default") == 0) {
        f->serde_default |= bv;
      } else if (strcmp(key, "x-cdd-untagged") == 0) {
        f->serde_untagged |= bv;
      } else if (strcmp(key, "x-cdd-cli-command") == 0) {
        f->is_cli_command |= bv;
      } else if (strcmp(key, "x-cdd-cli-arg-required-else-help") == 0) {
        f->cli_arg_required_else_help |= bv;
      } else if (strcmp(key, "x-cdd-cli-subcommand-required") == 0) {
        f->cli_subcommand_required |= bv;
      } else if (strcmp(key, "x-cdd-cli-allow-external-subcommands") == 0) {
        f->cli_allow_external_subcommands |= bv;
      } else if (strcmp(key, "x-cdd-cli-disable-help-flag") == 0) {
        f->cli_disable_help_flag |= bv;
      } else if (strcmp(key, "x-cdd-cli-disable-version-flag") == 0) {
        f->cli_disable_version_flag |= bv;
      } else if (strcmp(key, "x-cdd-cli-disable-help-subcommand") == 0) {
        f->cli_disable_help_subcommand |= bv;
      } else if (strcmp(key, "x-cdd-cli-propagate-version") == 0) {
        f->cli_propagate_version |= bv;
      } else if (strcmp(key, "x-cdd-cli-allow-windows-slash-flags") == 0) {
        f->cli_allow_windows_slash_flags |= bv;
      } else if (strcmp(key, "x-cdd-cli-allow-response-files") == 0) {
        f->cli_allow_response_files |= bv;
      } else if (strcmp(key, "x-cdd-cli-case-insensitive-flags") == 0) {
        f->cli_case_insensitive_flags |= bv;
      }
    } else if (val->type == JSONArray &&
               strcmp(key, "x-cdd-cli-aliases") == 0) {
      const JSON_Array *ca_arr = val->array_val;
      size_t ca_i;
      for (ca_i = 0; ca_i < ca_arr->count; ++ca_i) {
        if (ca_arr->items[ca_i]->type == JSONString) {
          rc = struct_fields_add_cli_alias(f, ca_arr->items[ca_i]->string_val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Loads enum-specific `x-cdd-enum-*` extensions from `o` into `f`.
 *
 * @param[in] o Schema JSON_Object.
 * @param[in,out] f Target enum StructFields container.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_load_enum_extensions(const JSON_Object *o,
                                              struct StructFields *f) {
  JSON_Array *c_names = NULL;
  JSON_Array *e_names = NULL;
  const char *other_name = NULL;
  JSON_Object *ea_obj = NULL;
  JSON_Object *ed_obj = NULL;
  JSON_Array *eh_arr = NULL;
  size_t ei;
  cdd_c_error_t rc;

  rc = c2s_json_opt_array(o, "x-cdd-enum-c-names", &c_names);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c2s_json_opt_array(o, "x-cdd-enum-names", &e_names);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c2s_json_opt_string(o, "x-cdd-enum-other", &other_name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (c_names) {
    size_t ci;
    for (ci = 0; ci < c_names->count && ci < f->enum_members.size; ++ci) {
      const char *cn = NULL;
      const char *wn = NULL;
      rc = c2s_arr_opt_string(c_names, ci, &cn);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = c2s_arr_opt_string(e_names, ci, &wn);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (cn && cn[0]) {
        C_CDD_FREE(f->enum_members.members[ci]);
        f->enum_members.members[ci] = NULL;
        rc = c_cdd_strdup(cn, &f->enum_members.members[ci]);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (wn && wn[0]) {
        C_CDD_FREE(f->enum_members.serde_names[ci]);
        f->enum_members.serde_names[ci] = NULL;
        rc = c_cdd_strdup(wn, &f->enum_members.serde_names[ci]);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  if (other_name && other_name[0]) {
    size_t oi;
    for (oi = 0; oi < f->enum_members.size; ++oi) {
      if (strcmp(f->enum_members.members[oi], other_name) == 0) {
        f->enum_members.serde_other_idx = (int)oi;
        break;
      }
    }
  }

  rc = c2s_json_opt_object(o, "x-cdd-enum-aliases", &ea_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c2s_json_opt_object(o, "x-cdd-enum-cli-descriptions", &ed_obj);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c2s_json_opt_array(o, "x-cdd-enum-cli-hidden", &eh_arr);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (ea_obj) {
    for (ei = 0; ei < f->enum_members.size; ++ei) {
      JSON_Array *va = NULL;
      rc = c2s_json_opt_array(ea_obj, f->enum_members.members[ei], &va);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (va) {
        size_t ai;
        for (ai = 0; ai < va->count; ++ai) {
          if (va->items[ai]->type == JSONString) {
            rc = enum_member_add_alias(&f->enum_members, ei,
                                       va->items[ai]->string_val);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      }
    }
  }

  if (ed_obj) {
    for (ei = 0; ei < f->enum_members.size; ++ei) {
      const char *desc = NULL;
      int hid = 0;
      rc = c2s_json_opt_string(ed_obj, f->enum_members.members[ei], &desc);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (eh_arr) {
        size_t hi;
        for (hi = 0; hi < eh_arr->count; ++hi) {
          if (eh_arr->items[hi]->type == JSONString &&
              strcmp(eh_arr->items[hi]->string_val,
                     f->enum_members.members[ei]) == 0) {
            hid = 1;
            break;
          }
        }
      }
      if (desc || hid) {
        rc = enum_member_set_cli_meta(&f->enum_members, ei, desc, hid);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Loads JSON Schema validation constraints and metadata from `prop` into
 * `field`.
 *
 * @param[in] prop Property JSON_Object.
 * @param[in] type Resolved property type string.
 * @param[in,out] field Target StructField.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_load_field_constraints(const JSON_Object *prop,
                                                const char *type,
                                                struct StructField *field) {
  const char *desc = NULL;
  const char *fmt = NULL;
  int hv = 0;
  int bv = 0;
  cdd_c_error_t rc;

  if (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0) {
    rc = c2s_json_opt_number(prop, "minimum", &field->min_val, &hv);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (hv)
      field->has_min = 1;

    rc = c2s_json_opt_number(prop, "exclusiveMinimum", &field->min_val, &hv);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (hv) {
      field->has_min = 1;
      field->exclusive_min = 1;
    } else {
      rc = c2s_json_opt_bool(prop, "exclusiveMinimum", &bv, &hv);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (hv && bv == 1)
        field->exclusive_min = 1;
    }

    rc = c2s_json_opt_number(prop, "maximum", &field->max_val, &hv);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (hv)
      field->has_max = 1;

    rc = c2s_json_opt_number(prop, "exclusiveMaximum", &field->max_val, &hv);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (hv) {
      field->has_max = 1;
      field->exclusive_max = 1;
    } else {
      rc = c2s_json_opt_bool(prop, "exclusiveMaximum", &bv, &hv);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (hv && bv == 1)
        field->exclusive_max = 1;
    }
  } else if (strcmp(type, "string") == 0) {
    double d_len = 0.0;
    const char *pat_str = NULL;
    rc = c2s_json_opt_number(prop, "minLength", &d_len, &hv);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (hv) {
      field->has_min_len = 1;
      field->min_len = (size_t)d_len;
    }

    d_len = 0.0;
    rc = c2s_json_opt_number(prop, "maxLength", &d_len, &hv);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (hv) {
      field->has_max_len = 1;
      field->max_len = (size_t)d_len;
    }

    rc = c2s_json_opt_string(prop, "pattern", &pat_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (pat_str) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncpy_s(field->pattern, sizeof(field->pattern), pat_str,
                sizeof(field->pattern) - 1);
#else
      strncpy(field->pattern, pat_str, sizeof(field->pattern) - 1);
#endif
      field->pattern[sizeof(field->pattern) - 1] = '\0';
    }
  } else if (strcmp(type, "array") == 0) {
    double d_items = 0.0;
    rc = c2s_json_opt_number(prop, "minItems", &d_items, &hv);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (hv) {
      field->has_min_items = 1;
      field->min_items = (size_t)d_items;
    }

    d_items = 0.0;
    rc = c2s_json_opt_number(prop, "maxItems", &d_items, &hv);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (hv) {
      field->has_max_items = 1;
      field->max_items = (size_t)d_items;
    }

    rc = c2s_json_opt_bool(prop, "uniqueItems", &field->unique_items, &hv);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = c2s_json_opt_string(prop, "description", &desc);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c2s_json_opt_string(prop, "format", &fmt);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (desc) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(field->description, sizeof(field->description), desc,
              sizeof(field->description) - 1);
#else
    strncpy(field->description, desc, sizeof(field->description) - 1);
#endif
    field->description[sizeof(field->description) - 1] = '\0';
  }
  if (fmt) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(field->format, sizeof(field->format), fmt,
              sizeof(field->format) - 1);
#else
    strncpy(field->format, fmt, sizeof(field->format) - 1);
#endif
    field->format[sizeof(field->format) - 1] = '\0';
  }

  rc = c2s_json_opt_bool(prop, "deprecated", &field->deprecated, &hv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (hv)
    field->deprecated_set = 1;

  rc = c2s_json_opt_bool(prop, "readOnly", &field->read_only, &hv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (hv)
    field->read_only_set = 1;

  rc = c2s_json_opt_bool(prop, "writeOnly", &field->write_only, &hv);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (hv)
    field->write_only_set = 1;

  return CDD_C_SUCCESS;
}

/**
 * @brief Sanitizes/deduplicates `field->name` and loads field-level `x-cdd-*`
 * and `x-c-*` extensions from `prop`.
 *
 * @param[in] prop Property JSON_Object.
 * @param[in] root Optional root `components.schemas` JSON_Object.
 * @param[in] name Original JSON property name.
 * @param[in,out] f Enclosing StructFields container.
 * @param[in,out] field Target StructField.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_load_field_extensions(const JSON_Object *prop,
                                               const JSON_Object *root,
                                               const char *name,
                                               struct StructFields *f,
                                               struct StructField *field) {
  const char *c_name_ext = NULL;
  int needs_san = 0;
  size_t k;
  cdd_c_error_t rc;

  rc = c2s_json_opt_string(prop, "x-cdd-c-name", &c_name_ext);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (c_name_ext && c_name_ext[0] != '\0') {
    CDD_STRCPY(field->name, sizeof(field->name), c_name_ext);
  } else {
    rc = c2s_needs_identifier_sanitization(name, &needs_san);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (needs_san) {
      char *sanitized = NULL;
      rc = sanitize_identifier(name, &sanitized);
      if (rc != CDD_C_SUCCESS)
        return rc;
      CDD_STRCPY(field->name, sizeof(field->name), sanitized);
      CDD_STRCPY(field->serde_rename, sizeof(field->serde_rename), name);
      C_CDD_FREE(sanitized);
    }
  }

  /* Deduplicate field->name if it collides with an existing field in `f` */
  {
    size_t f_idx = (size_t)(field - f->fields);
    size_t prev_k;
    int collides = 0;
    for (prev_k = 0; prev_k < f_idx; ++prev_k) {
      if (strcmp(f->fields[prev_k].name, field->name) == 0) {
        collides = 1;
        break;
      }
    }
    if (collides) {
      char base_nm[52];
      unsigned int sfx = 2;
      int dup_cand = 1;
      CDD_STRCPY(base_nm, sizeof(base_nm), field->name);
      while (dup_cand) {
        char cand[64];
        dup_cand = 0;
        CDD_SNPRINTF(cand, sizeof(cand), "%s_%u", base_nm, sfx++);
        for (prev_k = 0; prev_k < f_idx; ++prev_k) {
          if (strcmp(f->fields[prev_k].name, cand) == 0) {
            dup_cand = 1;
            break;
          }
        }
        if (!dup_cand) {
          CDD_STRCPY(field->name, sizeof(field->name), cand);
        }
      }
      if (field->serde_rename[0] == '\0') {
        CDD_STRCPY(field->serde_rename, sizeof(field->serde_rename), name);
      }
    }
  }

  for (k = 0; k < prop->count; ++k) {
    const char *key = prop->names[k];
    const JSON_Value *val = prop->values[k];
    if (strncmp(key, "x-", 2) != 0)
      continue;
    if (val->type == JSONString) {
      const char *f_ext = val->string_val;
      if (strcmp(key, "x-cdd-rename") == 0) {
        CDD_STRCPY(field->serde_rename, sizeof(field->serde_rename), f_ext);
      } else if (strcmp(key, "x-cdd-rename-serialize") == 0) {
        CDD_STRCPY(field->serde_rename_ser, sizeof(field->serde_rename_ser),
                   f_ext);
      } else if (strcmp(key, "x-cdd-rename-deserialize") == 0) {
        CDD_STRCPY(field->serde_rename_de, sizeof(field->serde_rename_de),
                   f_ext);
      } else if (strcmp(key, "x-cdd-case") == 0) {
        rc = c2s_try_parse_case_rule(f_ext, &field->serde_case);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-replace-from") == 0) {
        CDD_STRCPY(field->serde_replace_from, sizeof(field->serde_replace_from),
                   f_ext);
      } else if (strcmp(key, "x-cdd-replace-to") == 0) {
        CDD_STRCPY(field->serde_replace_to, sizeof(field->serde_replace_to),
                   f_ext);
      } else if (strcmp(key, "x-cdd-skip-serializing-if") == 0) {
        if (f_ext[0] != '\0') {
          if (strcmp(f_ext, "null") == 0) {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;
          } else if (strcmp(f_ext, "empty") == 0) {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
          } else if (strcmp(f_ext, "zero") == 0) {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_ZERO;
          } else {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_FUNC;
            CDD_STRCPY(field->serde_skip_if_fn, sizeof(field->serde_skip_if_fn),
                       f_ext);
          }
        }
      } else if (strcmp(key, "x-cdd-default-fn") == 0) {
        field->serde_has_default = 1;
        CDD_STRCPY(field->serde_default_fn, sizeof(field->serde_default_fn),
                   f_ext);
      } else if (strcmp(key, "x-cdd-with") == 0) {
        CDD_STRCPY(field->serde_with, sizeof(field->serde_with), f_ext);
      } else if (strcmp(key, "x-cdd-serialize-with") == 0) {
        CDD_STRCPY(field->serde_serialize_with,
                   sizeof(field->serde_serialize_with), f_ext);
      } else if (strcmp(key, "x-cdd-deserialize-with") == 0) {
        CDD_STRCPY(field->serde_deserialize_with,
                   sizeof(field->serde_deserialize_with), f_ext);
      } else if (strcmp(key, "x-c-type") == 0) {
        CDD_STRCPY(field->c_type, sizeof(field->c_type), f_ext);
      } else if (strcmp(key, "x-cdd-cli-short") == 0) {
        if (f_ext[0] != '\0')
          field->cli_short = f_ext[0];
      } else if (strcmp(key, "x-cdd-cli-long") == 0) {
        if (f_ext[0] != '\0') {
          field->cli_has_long = 1;
          CDD_STRCPY(field->cli_long, sizeof(field->cli_long), f_ext);
        }
      } else if (strcmp(key, "x-cdd-cli-env") == 0) {
        CDD_STRCPY(field->cli_env, sizeof(field->cli_env), f_ext);
      } else if (strcmp(key, "x-cdd-cli-value-name") == 0) {
        CDD_STRCPY(field->cli_value_name, sizeof(field->cli_value_name), f_ext);
      } else if (strcmp(key, "x-cdd-cli-help") == 0) {
        CDD_STRCPY(field->cli_help, sizeof(field->cli_help), f_ext);
      } else if (strcmp(key, "x-cdd-cli-action") == 0) {
        rc = c2s_try_parse_cli_action(f_ext, &field->cli_action);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-cli-delimiter") == 0) {
        if (f_ext[0] != '\0')
          field->cli_value_delimiter = f_ext[0];
      } else if (strcmp(key, "x-cdd-cli-short-aliases") == 0) {
        CDD_STRCPY(field->cli_short_aliases, sizeof(field->cli_short_aliases),
                   f_ext);
      } else if (strcmp(key, "x-cdd-cli-long-help") == 0) {
        rc = c_cdd_strdup(f_ext, &field->cli_long_help);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-cli-help-heading") == 0) {
        CDD_STRCPY(field->cli_help_heading, sizeof(field->cli_help_heading),
                   f_ext);
      } else if (strcmp(key, "x-cdd-cli-default-missing-val") == 0) {
        CDD_STRCPY(field->cli_default_missing_val,
                   sizeof(field->cli_default_missing_val), f_ext);
      } else if (strcmp(key, "x-cdd-cli-value-hint") == 0) {
        rc = c2s_try_parse_cli_value_hint(f_ext, &field->cli_value_hint);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (strcmp(key, "x-cdd-cli-value-parser") == 0) {
        CDD_STRCPY(field->cli_value_parser, sizeof(field->cli_value_parser),
                   f_ext);
      } else if (strcmp(key, "x-cdd-cli-group") == 0) {
        CDD_STRCPY(field->cli_group, sizeof(field->cli_group), f_ext);
      } else if (strcmp(key, "x-cdd-cli-required-if-field") == 0) {
        CDD_STRCPY(field->cli_required_if_field,
                   sizeof(field->cli_required_if_field), f_ext);
      } else if (strcmp(key, "x-cdd-cli-required-if-val") == 0) {
        CDD_STRCPY(field->cli_required_if_val,
                   sizeof(field->cli_required_if_val), f_ext);
      }
    } else if (val->type == JSONBoolean) {
      int bv = val->boolean_val;
      if (strcmp(key, "x-cdd-skip") == 0) {
        field->serde_skip |= bv;
        field->serde_skip_serializing |= bv;
        field->serde_skip_deserializing |= bv;
      } else if (strcmp(key, "x-cdd-skip-serializing") == 0) {
        field->serde_skip_serializing |= bv;
      } else if (strcmp(key, "x-cdd-skip-deserializing") == 0) {
        field->serde_skip_deserializing |= bv;
      } else if (strcmp(key, "x-cdd-flatten") == 0) {
        field->serde_flatten |= bv;
      } else if (strcmp(key, "x-cdd-default") == 0) {
        field->serde_has_default |= bv;
      } else if (strcmp(key, "x-c-nullable-ptr") == 0) {
        field->is_nullable_ptr |= bv;
      } else if (strcmp(key, "x-cdd-cli-negatable") == 0) {
        field->cli_negatable |= bv;
      } else if (strcmp(key, "x-cdd-cli-positional") == 0) {
        field->cli_positional |= bv;
      } else if (strcmp(key, "x-cdd-cli-subcommand") == 0) {
        field->cli_subcommand |= bv;
      } else if (strcmp(key, "x-cdd-cli-last") == 0) {
        field->cli_last |= bv;
      } else if (strcmp(key, "x-cdd-cli-trailing-var-arg") == 0) {
        field->cli_trailing_var_arg |= bv;
      } else if (strcmp(key, "x-cdd-cli-hide-env") == 0) {
        field->cli_hide_env |= bv;
      } else if (strcmp(key, "x-cdd-cli-hide-env-values") == 0) {
        field->cli_hide_env_values |= bv;
      } else if (strcmp(key, "x-cdd-cli-hide-default-value") == 0) {
        field->cli_hide_default_value |= bv;
      } else if (strcmp(key, "x-cdd-cli-require-equals") == 0) {
        field->cli_require_equals |= bv;
      } else if (strcmp(key, "x-cdd-cli-allow-hyphen-values") == 0) {
        field->cli_allow_hyphen_values |= bv;
      } else if (strcmp(key, "x-cdd-cli-global") == 0) {
        field->cli_global |= bv;
      } else if (strcmp(key, "x-cdd-cli-hidden") == 0) {
        field->cli_hidden |= bv;
      } else if (strcmp(key, "x-cdd-cli-exclusive") == 0) {
        field->cli_exclusive |= bv;
      } else if (strcmp(key, "x-cdd-cli-skip") == 0) {
        field->cli_skip |= bv;
        field->cli_skip_to_cli |= bv;
        field->cli_skip_from_cli |= bv;
      } else if (strcmp(key, "x-cdd-cli-skip-to") == 0) {
        field->cli_skip_to_cli |= bv;
      } else if (strcmp(key, "x-cdd-cli-skip-from") == 0) {
        field->cli_skip_from_cli |= bv;
      }
    } else if (val->type == JSONNumber) {
      double dv = val->number_val;
      if (strcmp(key, "x-c-fixed-array-len") == 0) {
        field->fixed_array_len = (size_t)dv;
      } else if (strcmp(key, "x-cdd-cli-index") == 0) {
        field->cli_positional = 1;
        field->cli_index = (int)dv;
      } else if (strcmp(key, "x-cdd-cli-num-args-min") == 0) {
        field->cli_num_args_min = (int)dv;
      } else if (strcmp(key, "x-cdd-cli-num-args-max") == 0) {
        field->cli_num_args_max = (int)dv;
      }
    } else if (val->type == JSONArray) {
      const JSON_Array *al_arr = val->array_val;
      size_t a_idx;
      if (strcmp(key, "x-cdd-aliases") == 0) {
        for (a_idx = 0; a_idx < al_arr->count; ++a_idx) {
          if (al_arr->items[a_idx]->type == JSONString) {
            rc =
                struct_field_add_alias(field, al_arr->items[a_idx]->string_val);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      } else if (strcmp(key, "x-cdd-cli-conflicts-with") == 0) {
        for (a_idx = 0; a_idx < al_arr->count; ++a_idx) {
          if (al_arr->items[a_idx]->type == JSONString) {
            rc = struct_field_add_cli_conflict(
                field, al_arr->items[a_idx]->string_val);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      } else if (strcmp(key, "x-cdd-cli-requires") == 0) {
        for (a_idx = 0; a_idx < al_arr->count; ++a_idx) {
          if (al_arr->items[a_idx]->type == JSONString) {
            rc = struct_field_add_cli_requires(
                field, al_arr->items[a_idx]->string_val);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      } else if (strcmp(key, "x-cdd-cli-required-unless") == 0) {
        for (a_idx = 0; a_idx < al_arr->count; ++a_idx) {
          if (al_arr->items[a_idx]->type == JSONString) {
            rc = struct_field_add_cli_required_unless(
                field, al_arr->items[a_idx]->string_val);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      }
    }
  }

  if (field->serde_flatten && root) {
    const JSON_Object *flat_target = NULL;
    rc = resolve_schema_ref_object(root, field->ref, &flat_target);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (flat_target) {
      JSON_Object *flat_props = NULL;
      rc = c2s_json_opt_object(flat_target, "properties", &flat_props);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (flat_props) {
        size_t fp_i;
        for (fp_i = 0; fp_i < flat_props->count; ++fp_i) {
          rc = struct_field_add_alias(field, flat_props->names[fp_i]);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves `n_<array_field>` counter name collisions with scalar fields.
 *
 * @param[in,out] f StructFields container to disambiguate in place.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t
c2s_resolve_array_counter_collisions(struct StructFields *f) {
  size_t a_idx, s_idx;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  for (a_idx = 0; a_idx < f->size; ++a_idx) {
    char counter_nm[68];
    if (strcmp(f->fields[a_idx].type, "array") != 0)
      continue;
    CDD_SNPRINTF(counter_nm, sizeof(counter_nm), "n_%s", f->fields[a_idx].name);
    for (s_idx = 0; s_idx < f->size; ++s_idx) {
      if (s_idx == a_idx)
        continue;
      if (strcmp(f->fields[s_idx].name, counter_nm) == 0) {
        char base_nm[52];
        unsigned int sfx = 2;
        int dup_cand = 1;
        if (f->fields[s_idx].serde_rename[0] == '\0') {
          CDD_STRCPY(f->fields[s_idx].serde_rename,
                     sizeof(f->fields[s_idx].serde_rename),
                     f->fields[s_idx].name);
        }
        CDD_STRCPY(base_nm, sizeof(base_nm), f->fields[s_idx].name);
        while (dup_cand) {
          char cand[64];
          size_t chk;
          dup_cand = 0;
          CDD_SNPRINTF(cand, sizeof(cand), "%s_%u", base_nm, sfx++);
          for (chk = 0; chk < f->size; ++chk) {
            if (strcmp(f->fields[chk].name, cand) == 0) {
              dup_cand = 1;
              break;
            }
          }
          if (!dup_cand) {
            CDD_STRCPY(f->fields[s_idx].name, sizeof(f->fields[s_idx].name),
                       cand);
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Internal helper to convert a JSON Schema object to
 * StructFields.
 *
 * @param[in] o JSON Schema object to convert.
 * @param[in,out] f Target StructFields container to populate.
 * @param[in] root Read-only root `components.schemas` JSON_Object for `$ref`
 * resolution.
 * @param[in,out] root_mut Optional mutable root `components.schemas`
 * JSON_Object for inline schema registration.
 * @param[in] schema_name Optional name of the current schema.
 * @param[in] allow_inline_union Non-zero to permit synthesizing inline union
 * schemas.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_json_object_to_struct_fields_internal_mut(
    const JSON_Object *o, struct StructFields *f, const JSON_Object *root,
    JSON_Object *root_mut, const char *schema_name, int allow_inline_union) {
  JSON_Object *props = NULL;
  size_t i, count = 0;
  JSON_Array *required = NULL;
  const JSON_Array *enum_arr = NULL;
  JSON_Array *all_of = NULL;
  JSON_Array *any_of = NULL;
  JSON_Array *one_of = NULL;
  int is_enum = 0;
  cdd_c_error_t rc;
  cdd_c_error_t rc_free = CDD_C_SUCCESS;

  if (!o || !f)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = c2s_collect_schema_extras(o, k_schema_skip_keys,
                                 sizeof(k_schema_skip_keys) /
                                     sizeof(k_schema_skip_keys[0]),
                                 &f->schema_extra_json);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = c2s_load_container_extensions(o, f, schema_name);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = schema_object_is_string_enum(o, &enum_arr, &is_enum);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (is_enum) {
    f->is_enum = 1;
    rc = enum_members_init(&f->enum_members);
    if (rc != CDD_C_SUCCESS)
      return rc;
    f->enum_members.serde_rename_all = f->serde_rename_all;
    CDD_STRCPY(f->enum_members.serde_replace_from,
               sizeof(f->enum_members.serde_replace_from),
               f->serde_replace_from);
    CDD_STRCPY(f->enum_members.serde_replace_to,
               sizeof(f->enum_members.serde_replace_to), f->serde_replace_to);
    CDD_STRCPY(f->enum_members.serde_tag, sizeof(f->enum_members.serde_tag),
               f->serde_tag);
    CDD_STRCPY(f->enum_members.serde_content,
               sizeof(f->enum_members.serde_content), f->serde_content);
    f->enum_members.serde_untagged = f->serde_untagged;
    rc = json_array_to_enum_members(enum_arr, &f->enum_members);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return c2s_load_enum_extensions(o, f);
  }

  rc = c2s_json_opt_array(o, "allOf", &all_of);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (all_of) {
    rc = apply_allof_to_struct_fields(all_of, f, root);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = c2s_json_opt_array(o, "anyOf", &any_of);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (any_of) {
    rc = apply_union_to_struct_fields_ex_mut(
        any_of, f, root, root_mut, schema_name, 1, o, allow_inline_union);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!f->is_union) {
      rc = apply_union_to_struct_fields_fallback(any_of, f, root);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  rc = c2s_json_opt_array(o, "oneOf", &one_of);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (one_of) {
    rc = apply_union_to_struct_fields_ex_mut(
        one_of, f, root, root_mut, schema_name, 0, o, allow_inline_union);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!f->is_union) {
      rc = apply_union_to_struct_fields_fallback(one_of, f, root);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (f->is_union)
    return CDD_C_SUCCESS;

  rc = c2s_json_opt_array(o, "required", &required);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = c2s_json_opt_object(o, "properties", &props);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!props)
    return CDD_C_SUCCESS;

  count = props->count;
  for (i = 0; i < count; i++) {
    const char *name = props->names[i];
    JSON_Object *prop = NULL;
    const char *type = NULL;
    JSON_Array *type_arr = NULL;
    const char *ref = NULL;
    const char *bw = NULL;
    JSON_Value *def_val = NULL;
    char default_buf[128] = {0};
    const char *d_val = NULL;
    struct StructField *field = NULL;
    int field_added = 0;
    int is_req = 0;
    char **type_union = NULL;
    size_t n_type_union = 0;

    rc = c2s_json_opt_object(props, name, &prop);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = c2s_json_opt_string(prop, "type", &type);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = c2s_json_opt_string(prop, "$ref", &ref);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = c2s_json_opt_string(prop, "x-c-bitwidth", &bw);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = c2s_json_opt_value(prop, "default", &def_val);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (def_val) {
      if (def_val->type == JSONString) {
        CDD_SNPRINTF(default_buf, sizeof(default_buf), "%c%s%c", 34,
                     def_val->string_val, 34);
        d_val = default_buf;
      } else if (def_val->type == JSONNumber) {
        if (type && strcmp(type, "integer") == 0)
          CDD_SNPRINTF(default_buf, sizeof(default_buf), "%d",
                       (int)def_val->number_val);
        else
          CDD_SNPRINTF(default_buf, sizeof(default_buf), "%f",
                       def_val->number_val);
        d_val = default_buf;
      } else if (def_val->type == JSONBoolean) {
        CDD_SNPRINTF(default_buf, sizeof(default_buf), "%d",
                     def_val->boolean_val);
        d_val = default_buf;
      }
    }

    if (!type) {
      rc = c2s_json_opt_array(prop, "type", &type_arr);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (type_arr) {
        const char *primary = NULL;
        rc = parse_type_union_array_code2schema(type_arr, &type_union,
                                                &n_type_union, &primary, NULL);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, c2s_string_array_free(type_union, n_type_union));
        }
        type = primary;
      }
    }

    if (type) {
      if (strcmp(type, "array") == 0) {
        JSON_Object *items = NULL;
        const char *item_type = NULL;
        JSON_Array *item_type_arr = NULL;
        char **items_type_union = NULL;
        size_t n_items_type_union = 0;
        const char *item_ref = NULL;
        const char *saved_enum_ref = NULL;

        rc = c2s_json_opt_object(prop, "items", &items);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, c2s_string_array_free(type_union, n_type_union));
        }
        rc = c2s_json_opt_string(items, "type", &item_type);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, c2s_string_array_free(type_union, n_type_union));
        }
        rc = c2s_json_opt_string(items, "$ref", &item_ref);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, c2s_string_array_free(type_union, n_type_union));
        }
        if (item_ref && root) {
          int item_is_enum = 0;
          rc = ref_points_to_string_enum(root, item_ref, &item_is_enum);
          if (rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(
                rc, c2s_string_array_free(type_union, n_type_union));
          }
          if (item_is_enum) {
            saved_enum_ref = item_ref;
            item_ref = NULL;
            item_type = "string";
          }
        }
        if (!item_ref && !item_type && items) {
          rc = c2s_json_opt_array(items, "type", &item_type_arr);
          if (rc != CDD_C_SUCCESS) {
            return cdd_c_keep_err(
                rc, c2s_string_array_free(type_union, n_type_union));
          }
          if (item_type_arr) {
            const char *primary = NULL;
            rc = parse_type_union_array_code2schema(
                item_type_arr, &items_type_union, &n_items_type_union, &primary,
                NULL);
            if (rc != CDD_C_SUCCESS) {
              return cdd_c_keep_err(
                  rc, cdd_c_keep_err(
                          c2s_string_array_free(type_union, n_type_union),
                          c2s_string_array_free(items_type_union,
                                                n_items_type_union)));
            }
            item_type = primary;
          }
        }
        rc = struct_fields_add(f, name, "array",
                               item_ref ? item_ref : item_type, NULL, bw);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc,
              cdd_c_keep_err(
                  c2s_string_array_free(type_union, n_type_union),
                  c2s_string_array_free(items_type_union, n_items_type_union)));
        }
        field = &f->fields[f->size - 1];
        field_added = 1;
        field->type_union = type_union;
        field->n_type_union = n_type_union;
        type_union = NULL;
        n_type_union = 0;
        field->items_type_union = items_type_union;
        field->n_items_type_union = n_items_type_union;
        items_type_union = NULL;
        n_items_type_union = 0;
        if (saved_enum_ref) {
          const char *er = NULL;
          field->is_enum_array = 1;
          rc = get_type_from_ref(saved_enum_ref, &er);
          if (rc != CDD_C_SUCCESS)
            return rc;
          CDD_STRCPY(field->enum_array_ref, sizeof(field->enum_array_ref), er);
        }
        if (items) {
          rc = c2s_collect_schema_extras(items, k_items_skip_keys,
                                         sizeof(k_items_skip_keys) /
                                             sizeof(k_items_skip_keys[0]),
                                         &field->items_extra_json);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      } else {
        rc = struct_fields_add(f, name, type, ref, d_val, bw);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, c2s_string_array_free(type_union, n_type_union));
        }
        field = &f->fields[f->size - 1];
        field_added = 1;
        field->type_union = type_union;
        field->n_type_union = n_type_union;
        type_union = NULL;
        n_type_union = 0;
      }
    } else if (ref) {
      int ref_is_enum = 0;
      const char *field_type;
      if (root) {
        rc = ref_points_to_string_enum(root, ref, &ref_is_enum);
        if (rc != CDD_C_SUCCESS) {
          return cdd_c_keep_err(
              rc, c2s_string_array_free(type_union, n_type_union));
        }
      }
      field_type = ref_is_enum ? "enum" : "object";
      rc = struct_fields_add(f, name, field_type, ref, NULL, bw);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc,
                              c2s_string_array_free(type_union, n_type_union));
      }
      field = &f->fields[f->size - 1];
      field_added = 1;
    }

    if (!field_added) {
      rc_free = c2s_string_array_free(type_union, n_type_union);
      continue;
    }

    rc = required_name_in_list(required, name, &is_req);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_req)
      field->required = 1;

    if (!type)
      type = field->type;

    rc = c2s_load_field_constraints(prop, type, field);
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = c2s_collect_schema_extras(prop, k_property_skip_keys,
                                   sizeof(k_property_skip_keys) /
                                       sizeof(k_property_skip_keys[0]),
                                   &field->schema_extra_json);
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = c2s_load_field_extensions(prop, root, name, f, field);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = c2s_resolve_array_counter_collisions(f);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return rc_free;
}

#ifdef CDD_BUILD_TESTS
/** @brief Mock failure counter for json_object_to_struct_fields */
C_CDD_EXPORT int g_json_object_to_struct_fields_fail = 0;
#endif

/*
 * @brief Extended function to convert a JSON Schema object to StructFields.
 *
 * @param[in] o JSON Schema object to convert.
 * @param[in,out] f Target StructFields container to populate.
 * @param[in] root Root `components.schemas` JSON_Object for `$ref` resolution.
 * @param[in] schema_name Optional name of the current schema.
 * @param[in] allow_inline_union Non-zero to permit inline union handling.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_json_object_to_struct_fields_internal(
    const JSON_Object *o, struct StructFields *f, const JSON_Object *root,
    const char *schema_name, int allow_inline_union) {
  return c2s_json_object_to_struct_fields_internal_mut(
      o, f, root, NULL, schema_name, allow_inline_union);
}

/*
 * @brief Converts a JSON Schema object to `StructFields` with an explicit
 * schema name.
 *
 * @param[in] schema_obj JSON Schema object to convert.
 * @param[in,out] fields Target StructFields container to populate.
 * @param[in] schemas_obj_root Root `components.schemas` JSON_Object for `$ref`
 * resolution.
 * @param[in] schema_name Optional name of the current schema.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t json_object_to_struct_fields_ex(
    const JSON_Object *schema_obj, struct StructFields *fields,
    const JSON_Object *schemas_obj_root, const char *schema_name) {
#ifdef CDD_BUILD_TESTS
  if (g_json_object_to_struct_fields_fail &&
      --g_json_object_to_struct_fields_fail == 0) {
    return CDD_C_ERROR_MEMORY;
  }
#endif
  return c2s_json_object_to_struct_fields_internal(
      schema_obj, fields, schemas_obj_root, schema_name, 0);
}

/*
 * @brief Extended code generation function to convert JSON Schema to
 * StructFields.
 *
 * @param[in] schema_obj JSON Schema object to convert.
 * @param[in,out] fields Target StructFields container to populate.
 * @param[in,out] schemas_obj_root Mutable root `components.schemas`
 * JSON_Object.
 * @param[in] schema_name Optional name of the current schema.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t json_object_to_struct_fields_ex_codegen(
    const JSON_Object *schema_obj, struct StructFields *fields,
    JSON_Object *schemas_obj_root, const char *schema_name) {
#ifdef CDD_BUILD_TESTS
  if (g_json_object_to_struct_fields_fail &&
      --g_json_object_to_struct_fields_fail == 0) {
    return CDD_C_ERROR_MEMORY;
  }
#endif
  return c2s_json_object_to_struct_fields_internal_mut(
      schema_obj, fields, schemas_obj_root, schemas_obj_root, schema_name, 1);
}

/*
 * @brief Converts a JSON Schema object to a StructFields structure.
 *
 * @param[in] schema_obj JSON Schema object to convert.
 * @param[in,out] fields Target StructFields container to populate.
 * @param[in] schemas_obj_root Root `components.schemas` JSON_Object for `$ref`
 * resolution.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t
json_object_to_struct_fields(const JSON_Object *schema_obj,
                             struct StructFields *fields,
                             const JSON_Object *schemas_obj_root) {
  return json_object_to_struct_fields_ex(schema_obj, fields, schemas_obj_root,
                                         NULL);
}
