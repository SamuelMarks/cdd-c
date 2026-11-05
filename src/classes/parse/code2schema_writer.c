/**
 * @file code2schema_writer.c
 * @brief Writing struct fields and constraints to JSON schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/*
 * @brief Writes the default value of a StructField to a JSON schema
 * object.
 *
 * @param[in,out] pobj Target property JSON_Object.
 * @param[in] field Source StructField descriptor.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t c2s_write_default_value(JSON_Object *pobj,
                                      const struct StructField *field) {
  const char *def;
  const char *typ;
  char buf[256];
  int bval;
  double nval;
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  def = field->default_val;
  if (def[0] == '\0')
    return CDD_C_SUCCESS;
  typ = field->type;

  if (strcmp(def, "nullptr") == 0) {
    json_object_set_null(pobj, "default");
    return CDD_C_SUCCESS;
  }

  if (strcmp(typ, "string") == 0) {
    const char *s = NULL;
    c2s_strip_quotes(def, buf, sizeof(buf), &s);
    json_object_set_string(pobj, "default", s);
    return CDD_C_SUCCESS;
  }

  if (strcmp(typ, "boolean") == 0) {
    int has_bval = 0;
    c2s_parse_bool_default(def, &bval, &has_bval);
    if (has_bval)
      json_object_set_boolean(pobj, "default", bval);
    return CDD_C_SUCCESS;
  }

  if (strcmp(typ, "integer") == 0 || strcmp(typ, "number") == 0) {
    int has_nval = 0;
    c2s_parse_number_default(def, &nval, &has_nval);
    if (has_nval)
      json_object_set_number(pobj, "default", nval);
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes numeric constraints of a StructField to a JSON schema
 * object.
 *
 * @param[in,out] pobj Target property JSON_Object.
 * @param[in] field Source StructField descriptor.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t c2s_write_numeric_constraints(JSON_Object *pobj,
                                            const struct StructField *field) {
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!field->type[0] || !(strcmp(field->type, "integer") == 0 ||
                           strcmp(field->type, "number") == 0))
    return CDD_C_SUCCESS;
  if (field->has_min) {
    if (field->exclusive_min)
      json_object_set_number(pobj, "exclusiveMinimum", field->min_val);
    else
      json_object_set_number(pobj, "minimum", field->min_val);
  }
  if (field->has_max) {
    if (field->exclusive_max)
      json_object_set_number(pobj, "exclusiveMaximum", field->max_val);
    else
      json_object_set_number(pobj, "maximum", field->max_val);
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes string constraints of a StructField to a JSON schema
 * object.
 *
 * @param[in,out] pobj Target property JSON_Object.
 * @param[in] field Source StructField descriptor.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t c2s_write_string_constraints(JSON_Object *pobj,
                                           const struct StructField *field) {
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!field->type[0] || strcmp(field->type, "string") != 0)
    return CDD_C_SUCCESS;
  if (field->has_min_len)
    json_object_set_number(pobj, "minLength", (double)field->min_len);
  if (field->has_max_len)
    json_object_set_number(pobj, "maxLength", (double)field->max_len);
  if (field->pattern[0] != '\0')
    json_object_set_string(pobj, "pattern", field->pattern);
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes array constraints of a StructField to a JSON schema
 * object.
 *
 * @param[in,out] pobj Target property JSON_Object.
 * @param[in] field Source StructField descriptor.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t c2s_write_array_constraints(JSON_Object *pobj,
                                          const struct StructField *field) {
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!field->type[0] || strcmp(field->type, "array") != 0)
    return CDD_C_SUCCESS;
  if (field->has_min_items)
    json_object_set_number(pobj, "minItems", (double)field->min_items);
  if (field->has_max_items)
    json_object_set_number(pobj, "maxItems", (double)field->max_items);
  if (field->unique_items)
    json_object_set_boolean(pobj, "uniqueItems", 1);
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes a type union array to a JSON schema object.
 *
 * @param[in,out] obj Target schema or property JSON_Object.
 * @param[in] type Fallback single type string when `type_union` is NULL.
 * @param[in] type_union Optional array of type union strings.
 * @param[in] n_type_union Number of strings in `type_union`.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t c2s_write_type_union(JSON_Object *obj, const char *type,
                                   char **type_union, size_t n_type_union) {
  size_t i;
  JSON_Value *arr_val;
  JSON_Array *arr;
  if (!obj)
    return CDD_C_SUCCESS;

  if (type_union) {
    arr_val = json_value_init_array();
    if (!arr_val)
      return CDD_C_SUCCESS;
    arr = json_value_get_array(arr_val);
    for (i = 0; i < n_type_union; ++i) {
      if (type_union[i])
        json_array_append_string(arr, type_union[i]);
    }
    if (json_object_set_value(obj, "type", arr_val) != JSONSuccess) {
      json_value_free(arr_val);
    }
    return CDD_C_SUCCESS;
  }

  if (type)
    json_object_set_string(obj, "type", type);
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes a StructFields representation to a JSON Schema object.
 *
 * @param[in,out] schemas_obj Target `components.schemas` JSON_Object.
 * @param[in] struct_name Name of the schema/struct.
 * @param[in] sf Source StructFields container to serialize.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t write_struct_to_json_schema(JSON_Object *schemas_obj,
                                          const char *struct_name,
                                          const struct StructFields *sf) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  JSON_Value *props_val = json_value_init_object();
  JSON_Object *props_obj = json_value_get_object(props_val);
  JSON_Value *req_val = NULL;
  JSON_Array *req_arr = NULL;
  const char *schema_key;
  size_t i;
  if (!schemas_obj || !struct_name || !sf) {
    json_value_free(val);
    json_value_free(props_val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  schema_key = (sf->serde_rename[0] != '\0') ? sf->serde_rename : struct_name;

  if (sf->serde_rename[0] != '\0') {
    json_object_set_string(obj, "x-cdd-rename", sf->serde_rename);
    if (strcmp(sf->serde_rename, struct_name) != 0) {
      json_object_set_string(obj, "x-cdd-c-name", struct_name);
    }
  }

  if (sf->is_enum) {
    JSON_Value *enum_val = json_value_init_array();
    JSON_Array *enum_arr;
    const struct EnumMembers *em = &sf->enum_members;
    enum CddSerdeCaseRule eff_rename_all =
        (em->serde_rename_all != CDD_SERDE_CASE_NONE) ? em->serde_rename_all
                                                      : sf->serde_rename_all;
    const char *eff_rep_from = (em->serde_replace_from[0] != '\0')
                                   ? em->serde_replace_from
                                   : sf->serde_replace_from;
    const char *eff_rep_to = (em->serde_replace_from[0] != '\0')
                                 ? em->serde_replace_to
                                 : sf->serde_replace_to;
    int has_custom_names = 0;

    if (!enum_val) {
      json_value_free(val);
      json_value_free(props_val);
      return CDD_C_ERROR_MEMORY;
    }
    enum_arr = json_value_get_array(enum_val);
    json_object_set_string(obj, "type", "string");

    if (eff_rename_all != CDD_SERDE_CASE_NONE) {
      const char *r_str = NULL;
      cdd_serde_case_rule_to_str(eff_rename_all, &r_str);
      json_object_set_string(obj, "x-cdd-rename-all", r_str);
    }
    if (eff_rep_from[0] != '\0') {
      json_object_set_string(obj, "x-cdd-replace-from", eff_rep_from);
      json_object_set_string(obj, "x-cdd-replace-to", eff_rep_to);
    }
    if (em->serde_other_idx >= 0 && (size_t)em->serde_other_idx < em->size &&
        em->members[em->serde_other_idx]) {
      json_object_set_string(obj, "x-cdd-enum-other",
                             em->members[em->serde_other_idx]);
    }

    for (i = 0; i < em->size; ++i) {
      const char *member = em->members[i];
      char wname[128];
      if (!member)
        continue;
      if (strcmp(member, "UNKNOWN") == 0)
        continue;
      if (em->serde_skip && em->serde_skip[i])
        continue;
      cdd_serde_resolve_enum_member_name(em, i, wname, sizeof(wname));
      if (strcmp(wname, member) != 0)
        has_custom_names = 1;
      json_array_append_string(enum_arr, wname);
    }

    if (has_custom_names) {
      JSON_Value *c_names_val = json_value_init_array();
      JSON_Array *c_names_arr = json_value_get_array(c_names_val);
      JSON_Value *e_names_val = json_value_init_array();
      JSON_Array *e_names_arr = json_value_get_array(e_names_val);
      for (i = 0; i < em->size; ++i) {
        const char *member = em->members[i];
        char wname[128];
        if (!member || strcmp(member, "UNKNOWN") == 0)
          continue;
        if (em->serde_skip && em->serde_skip[i])
          continue;
        cdd_serde_resolve_enum_member_name(em, i, wname, sizeof(wname));
        json_array_append_string(c_names_arr, member);
        json_array_append_string(e_names_arr, wname);
      }
      json_object_set_value(obj, "x-cdd-enum-c-names", c_names_val);
      json_object_set_value(obj, "x-cdd-enum-names", e_names_val);
    }

    if (em->serde_aliases) {
      JSON_Value *ea_obj_val = json_value_init_object();
      JSON_Object *ea_obj = json_value_get_object(ea_obj_val);
      for (i = 0; i < em->size; ++i) {
        if (em->n_serde_aliases[i] > 0) {
          JSON_Value *arr_v = json_value_init_array();
          JSON_Array *arr_a = json_value_get_array(arr_v);
          size_t ai;
          for (ai = 0; ai < em->n_serde_aliases[i]; ++ai) {
            json_array_append_string(arr_a, em->serde_aliases[i][ai]);
          }
          json_object_set_value(ea_obj, em->members[i], arr_v);
        }
      }
      json_object_set_value(obj, "x-cdd-enum-aliases", ea_obj_val);
    }
    if (em->cli_descriptions) {
      JSON_Value *ed_val = json_value_init_object();
      JSON_Object *ed_obj = json_value_get_object(ed_val);
      for (i = 0; i < em->size; ++i) {
        if (em->cli_descriptions[i]) {
          json_object_set_string(ed_obj, em->members[i],
                                 em->cli_descriptions[i]);
        }
      }
      json_object_set_value(obj, "x-cdd-enum-cli-descriptions", ed_val);
    }
    if (em->cli_hidden) {
      JSON_Value *eh_val = json_value_init_array();
      JSON_Array *eh_arr = json_value_get_array(eh_val);
      for (i = 0; i < em->size; ++i) {
        if (em->cli_hidden[i])
          json_array_append_string(eh_arr, em->members[i]);
      }
      json_object_set_value(obj, "x-cdd-enum-cli-hidden", eh_val);
    }

    if (json_object_set_value(obj, "enum", enum_val) != JSONSuccess) {
      json_value_free(enum_val);
      json_value_free(val);
      json_value_free(props_val);
      return CDD_C_ERROR_MEMORY;
    }
    if (c2s_merge_schema_extras_object(obj, sf->schema_extra_json) != 0) {
      json_value_free(val);
      json_value_free(props_val);
      return CDD_C_ERROR_MEMORY;
    }
    if (json_object_set_value(schemas_obj, schema_key, val) != JSONSuccess) {
      json_value_free(val);
      json_value_free(props_val);
      return CDD_C_ERROR_MEMORY;
    }
    json_value_free(props_val);
    return CDD_C_SUCCESS;
  }

  json_object_set_string(obj, "type", "object");
  if (json_object_set_value(obj, "properties", props_val) != JSONSuccess) {
    json_value_free(props_val);
    json_value_free(val);
    return CDD_C_ERROR_MEMORY;
  }

  /* Persist container-level Serde reflection attributes */
  if (sf->serde_rename_all != CDD_SERDE_CASE_NONE) {
    const char *r_str = NULL;
    cdd_serde_case_rule_to_str(sf->serde_rename_all, &r_str);
    json_object_set_string(obj, "x-cdd-rename-all", r_str);
  }
  if (sf->serde_rename_all_ser != CDD_SERDE_CASE_NONE) {
    const char *r_str = NULL;
    cdd_serde_case_rule_to_str(sf->serde_rename_all_ser, &r_str);
    json_object_set_string(obj, "x-cdd-rename-all-serialize", r_str);
  }
  if (sf->serde_rename_all_de != CDD_SERDE_CASE_NONE) {
    const char *r_str = NULL;
    cdd_serde_case_rule_to_str(sf->serde_rename_all_de, &r_str);
    json_object_set_string(obj, "x-cdd-rename-all-deserialize", r_str);
  }
  if (sf->serde_replace_from[0] != '\0') {
    json_object_set_string(obj, "x-cdd-replace-from", sf->serde_replace_from);
    json_object_set_string(obj, "x-cdd-replace-to", sf->serde_replace_to);
  }
  if (sf->serde_prefix[0] != '\0')
    json_object_set_string(obj, "x-cdd-prefix", sf->serde_prefix);
  if (sf->serde_suffix[0] != '\0')
    json_object_set_string(obj, "x-cdd-suffix", sf->serde_suffix);
  if (sf->serde_deny_unknown_fields)
    json_object_set_boolean(obj, "x-cdd-deny-unknown-fields", 1);
  if (sf->serde_default)
    json_object_set_boolean(obj, "x-cdd-default", 1);
  if (sf->serde_tag[0] != '\0')
    json_object_set_string(obj, "x-cdd-tag", sf->serde_tag);
  if (sf->serde_content[0] != '\0')
    json_object_set_string(obj, "x-cdd-content", sf->serde_content);
  if (sf->serde_untagged)
    json_object_set_boolean(obj, "x-cdd-untagged", 1);

  /* Persist container-level CLI reflection attributes */
  if (sf->is_cli_command)
    json_object_set_boolean(obj, "x-cdd-cli-command", 1);
  if (sf->cli_name[0] != '\0')
    json_object_set_string(obj, "x-cdd-cli-name", sf->cli_name);
  if (sf->cli_bin_name[0] != '\0')
    json_object_set_string(obj, "x-cdd-cli-bin-name", sf->cli_bin_name);
  if (sf->cli_version[0] != '\0')
    json_object_set_string(obj, "x-cdd-cli-version", sf->cli_version);
  if (sf->cli_about[0] != '\0')
    json_object_set_string(obj, "x-cdd-cli-about", sf->cli_about);
  if (sf->cli_author[0] != '\0')
    json_object_set_string(obj, "x-cdd-cli-author", sf->cli_author);
  if (sf->cli_long_about)
    json_object_set_string(obj, "x-cdd-cli-long-about", sf->cli_long_about);
  if (sf->cli_before_help)
    json_object_set_string(obj, "x-cdd-cli-before-help", sf->cli_before_help);
  if (sf->cli_after_help)
    json_object_set_string(obj, "x-cdd-cli-after-help", sf->cli_after_help);
  if (sf->cli_usage_override[0] != '\0')
    json_object_set_string(obj, "x-cdd-cli-usage-override",
                           sf->cli_usage_override);
  if (sf->cli_rename_all != CDD_SERDE_CASE_NONE) {
    const char *r_str = NULL;
    cdd_serde_case_rule_to_str(sf->cli_rename_all, &r_str);
    json_object_set_string(obj, "x-cdd-cli-rename-all", r_str);
  }
  if (sf->cli_rename_all_env != CDD_SERDE_CASE_NONE) {
    const char *r_str = NULL;
    cdd_serde_case_rule_to_str(sf->cli_rename_all_env, &r_str);
    json_object_set_string(obj, "x-cdd-cli-rename-all-env", r_str);
  }
  if (sf->cli_env_prefix[0] != '\0')
    json_object_set_string(obj, "x-cdd-cli-env-prefix", sf->cli_env_prefix);
  if (sf->cli_arg_required_else_help)
    json_object_set_boolean(obj, "x-cdd-cli-arg-required-else-help", 1);
  if (sf->cli_subcommand_required)
    json_object_set_boolean(obj, "x-cdd-cli-subcommand-required", 1);
  if (sf->cli_allow_external_subcommands)
    json_object_set_boolean(obj, "x-cdd-cli-allow-external-subcommands", 1);
  if (sf->cli_disable_help_flag)
    json_object_set_boolean(obj, "x-cdd-cli-disable-help-flag", 1);
  if (sf->cli_disable_version_flag)
    json_object_set_boolean(obj, "x-cdd-cli-disable-version-flag", 1);
  if (sf->cli_disable_help_subcommand)
    json_object_set_boolean(obj, "x-cdd-cli-disable-help-subcommand", 1);
  if (sf->cli_propagate_version)
    json_object_set_boolean(obj, "x-cdd-cli-propagate-version", 1);
  if (sf->cli_allow_windows_slash_flags)
    json_object_set_boolean(obj, "x-cdd-cli-allow-windows-slash-flags", 1);
  if (sf->cli_allow_response_files)
    json_object_set_boolean(obj, "x-cdd-cli-allow-response-files", 1);
  if (sf->cli_case_insensitive_flags)
    json_object_set_boolean(obj, "x-cdd-cli-case-insensitive-flags", 1);
  if (sf->n_cli_aliases > 0) {
    JSON_Value *ca_val = json_value_init_array();
    JSON_Array *ca_arr = json_value_get_array(ca_val);
    size_t ca_idx;
    for (ca_idx = 0; ca_idx < sf->n_cli_aliases; ++ca_idx) {
      json_array_append_string(ca_arr, sf->cli_aliases[ca_idx]);
    }
    json_object_set_value(obj, "x-cdd-cli-aliases", ca_val);
  }

  for (i = 0; i < sf->size; i++) {
    JSON_Value *pval = json_value_init_object();
    JSON_Object *pobj = json_value_get_object(pval);
    const struct StructField *field = &sf->fields[i];
    const char *typ = field->type;
    const char *ref = field->ref;
    const char *bw = field->bit_width;
    char prop_key[128];

    cdd_serde_resolve_field_names(sf, i, prop_key, sizeof(prop_key), NULL, 0);

    if (*bw) {
      json_object_set_string(pobj, "x-c-bitwidth", bw);
    }
    if (field->c_type[0] != '\0') {
      json_object_set_string(pobj, "x-c-type", field->c_type);
    }
    if (field->is_nullable_ptr) {
      json_object_set_boolean(pobj, "x-c-nullable-ptr", 1);
    }
    if (field->fixed_array_len > 0) {
      json_object_set_number(pobj, "x-c-fixed-array-len",
                             (double)field->fixed_array_len);
    }
    if (strcmp(prop_key, field->name) != 0) {
      json_object_set_string(pobj, "x-cdd-c-name", field->name);
    }
    if (field->serde_rename[0] != '\0')
      json_object_set_string(pobj, "x-cdd-rename", field->serde_rename);
    if (field->serde_rename_ser[0] != '\0')
      json_object_set_string(pobj, "x-cdd-rename-serialize",
                             field->serde_rename_ser);
    if (field->serde_rename_de[0] != '\0')
      json_object_set_string(pobj, "x-cdd-rename-deserialize",
                             field->serde_rename_de);
    if (field->serde_case != CDD_SERDE_CASE_NONE) {
      const char *c_str = NULL;
      cdd_serde_case_rule_to_str(field->serde_case, &c_str);
      json_object_set_string(pobj, "x-cdd-case", c_str);
    }
    if (field->serde_replace_from[0] != '\0') {
      json_object_set_string(pobj, "x-cdd-replace-from",
                             field->serde_replace_from);
      json_object_set_string(pobj, "x-cdd-replace-to", field->serde_replace_to);
    }
    if (field->n_serde_aliases > 0 && field->serde_aliases) {
      JSON_Value *al_val = json_value_init_array();
      JSON_Array *al_arr = json_value_get_array(al_val);
      size_t a_idx;
      for (a_idx = 0; a_idx < field->n_serde_aliases; ++a_idx) {
        if (field->serde_aliases[a_idx])
          json_array_append_string(al_arr, field->serde_aliases[a_idx]);
      }
      json_object_set_value(pobj, "x-cdd-aliases", al_val);
    }
    if (field->serde_skip)
      json_object_set_boolean(pobj, "x-cdd-skip", 1);
    else {
      if (field->serde_skip_serializing)
        json_object_set_boolean(pobj, "x-cdd-skip-serializing", 1);
      if (field->serde_skip_deserializing)
        json_object_set_boolean(pobj, "x-cdd-skip-deserializing", 1);
    }
    if (field->serde_skip_if_kind != CDD_SERDE_SKIP_IF_NONE) {
      if (field->serde_skip_if_kind == CDD_SERDE_SKIP_IF_NULL)
        json_object_set_string(pobj, "x-cdd-skip-serializing-if", "null");
      else if (field->serde_skip_if_kind == CDD_SERDE_SKIP_IF_EMPTY)
        json_object_set_string(pobj, "x-cdd-skip-serializing-if", "empty");
      else if (field->serde_skip_if_kind == CDD_SERDE_SKIP_IF_ZERO)
        json_object_set_string(pobj, "x-cdd-skip-serializing-if", "zero");
      else if (field->serde_skip_if_fn[0] != '\0')
        json_object_set_string(pobj, "x-cdd-skip-serializing-if",
                               field->serde_skip_if_fn);
    }
    if (field->serde_flatten)
      json_object_set_boolean(pobj, "x-cdd-flatten", 1);
    if (field->serde_has_default)
      json_object_set_boolean(pobj, "x-cdd-default", 1);
    if (field->serde_default_fn[0] != '\0')
      json_object_set_string(pobj, "x-cdd-default-fn", field->serde_default_fn);
    if (field->serde_with[0] != '\0')
      json_object_set_string(pobj, "x-cdd-with", field->serde_with);
    if (field->serde_serialize_with[0] != '\0')
      json_object_set_string(pobj, "x-cdd-serialize-with",
                             field->serde_serialize_with);
    if (field->serde_deserialize_with[0] != '\0')
      json_object_set_string(pobj, "x-cdd-deserialize-with",
                             field->serde_deserialize_with);

    /* Persist property-level CLI reflection attributes */
    if (field->cli_short != '\0') {
      char sbuf[2];
      sbuf[0] = field->cli_short;
      sbuf[1] = '\0';
      json_object_set_string(pobj, "x-cdd-cli-short", sbuf);
    }
    if (field->cli_long[0] != '\0')
      json_object_set_string(pobj, "x-cdd-cli-long", field->cli_long);
    if (field->cli_negatable)
      json_object_set_boolean(pobj, "x-cdd-cli-negatable", 1);
    if (field->cli_positional)
      json_object_set_boolean(pobj, "x-cdd-cli-positional", 1);
    if (field->cli_index >= 0)
      json_object_set_number(pobj, "x-cdd-cli-index", (double)field->cli_index);
    if (field->cli_env[0] != '\0')
      json_object_set_string(pobj, "x-cdd-cli-env", field->cli_env);
    if (field->cli_value_name[0] != '\0')
      json_object_set_string(pobj, "x-cdd-cli-value-name",
                             field->cli_value_name);
    if (field->cli_help[0] != '\0')
      json_object_set_string(pobj, "x-cdd-cli-help", field->cli_help);
    if (field->cli_action != CDD_CLI_ACTION_AUTO) {
      const char *act_str = NULL;
      cdd_cli_action_to_str(field->cli_action, &act_str);
      json_object_set_string(pobj, "x-cdd-cli-action", act_str);
    }
    if (field->cli_value_delimiter != '\0') {
      char dbuf[2];
      dbuf[0] = field->cli_value_delimiter;
      dbuf[1] = '\0';
      json_object_set_string(pobj, "x-cdd-cli-delimiter", dbuf);
    }
    if (field->cli_subcommand)
      json_object_set_boolean(pobj, "x-cdd-cli-subcommand", 1);
    if (field->n_cli_conflicts_with > 0 && field->cli_conflicts_with) {
      JSON_Value *cw_val = json_value_init_array();
      JSON_Array *cw_arr = json_value_get_array(cw_val);
      size_t c_idx;
      for (c_idx = 0; c_idx < field->n_cli_conflicts_with; ++c_idx) {
        if (field->cli_conflicts_with[c_idx])
          json_array_append_string(cw_arr, field->cli_conflicts_with[c_idx]);
      }
      json_object_set_value(pobj, "x-cdd-cli-conflicts-with", cw_val);
    }
    if (field->n_cli_requires > 0 && field->cli_requires) {
      JSON_Value *rq_val = json_value_init_array();
      JSON_Array *rq_arr = json_value_get_array(rq_val);
      size_t r_idx;
      for (r_idx = 0; r_idx < field->n_cli_requires; ++r_idx) {
        if (field->cli_requires[r_idx])
          json_array_append_string(rq_arr, field->cli_requires[r_idx]);
      }
      json_object_set_value(pobj, "x-cdd-cli-requires", rq_val);
    }
    if (field->cli_short_aliases[0] != '\0')
      json_object_set_string(pobj, "x-cdd-cli-short-aliases",
                             field->cli_short_aliases);
    if (field->cli_last)
      json_object_set_boolean(pobj, "x-cdd-cli-last", 1);
    if (field->cli_trailing_var_arg)
      json_object_set_boolean(pobj, "x-cdd-cli-trailing-var-arg", 1);
    if (field->cli_long_help)
      json_object_set_string(pobj, "x-cdd-cli-long-help", field->cli_long_help);
    if (field->cli_help_heading[0] != '\0')
      json_object_set_string(pobj, "x-cdd-cli-help-heading",
                             field->cli_help_heading);
    if (field->cli_hide_env)
      json_object_set_boolean(pobj, "x-cdd-cli-hide-env", 1);
    if (field->cli_hide_env_values)
      json_object_set_boolean(pobj, "x-cdd-cli-hide-env-values", 1);
    if (field->cli_default_missing_val[0] != '\0')
      json_object_set_string(pobj, "x-cdd-cli-default-missing-val",
                             field->cli_default_missing_val);
    if (field->cli_hide_default_value)
      json_object_set_boolean(pobj, "x-cdd-cli-hide-default-value", 1);
    if (field->cli_value_hint != CDD_CLI_HINT_NONE) {
      const char *vh_str = NULL;
      cdd_cli_value_hint_to_str(field->cli_value_hint, &vh_str);
      json_object_set_string(pobj, "x-cdd-cli-value-hint", vh_str);
    }
    if (field->cli_num_args_min > 0)
      json_object_set_number(pobj, "x-cdd-cli-num-args-min",
                             (double)field->cli_num_args_min);
    if (field->cli_num_args_max > 0)
      json_object_set_number(pobj, "x-cdd-cli-num-args-max",
                             (double)field->cli_num_args_max);
    if (field->cli_require_equals)
      json_object_set_boolean(pobj, "x-cdd-cli-require-equals", 1);
    if (field->cli_allow_hyphen_values)
      json_object_set_boolean(pobj, "x-cdd-cli-allow-hyphen-values", 1);
    if (field->cli_global)
      json_object_set_boolean(pobj, "x-cdd-cli-global", 1);
    if (field->cli_hidden)
      json_object_set_boolean(pobj, "x-cdd-cli-hidden", 1);
    if (field->cli_exclusive)
      json_object_set_boolean(pobj, "x-cdd-cli-exclusive", 1);
    if (field->cli_skip)
      json_object_set_boolean(pobj, "x-cdd-cli-skip", 1);
    if (field->cli_skip_to_cli)
      json_object_set_boolean(pobj, "x-cdd-cli-skip-to", 1);
    if (field->cli_skip_from_cli)
      json_object_set_boolean(pobj, "x-cdd-cli-skip-from", 1);
    if (field->cli_value_parser[0] != '\0')
      json_object_set_string(pobj, "x-cdd-cli-value-parser",
                             field->cli_value_parser);
    if (field->cli_group[0] != '\0')
      json_object_set_string(pobj, "x-cdd-cli-group", field->cli_group);
    if (field->cli_required_if_field[0] != '\0') {
      json_object_set_string(pobj, "x-cdd-cli-required-if-field",
                             field->cli_required_if_field);
      json_object_set_string(pobj, "x-cdd-cli-required-if-val",
                             field->cli_required_if_val);
    }
    if (field->n_cli_required_unless > 0) {
      JSON_Value *ru_val = json_value_init_array();
      JSON_Array *ru_arr = json_value_get_array(ru_val);
      size_t u_idx;
      for (u_idx = 0; u_idx < field->n_cli_required_unless; ++u_idx) {
        json_array_append_string(ru_arr, field->cli_required_unless[u_idx]);
      }
      json_object_set_value(pobj, "x-cdd-cli-required-unless", ru_val);
    }

    if (strcmp(typ, "array") == 0) {
      JSON_Value *items_val = json_value_init_object();
      JSON_Object *items_obj = json_value_get_object(items_val);
      c2s_write_type_union(pobj, "array", field->type_union,
                           field->n_type_union);
      if (field->items_type_union) {
        c2s_write_type_union(items_obj, ref, field->items_type_union,
                             field->n_items_type_union);
      } else if (field->is_enum_array) {
        char ref_str[128];
        CDD_SNPRINTF(ref_str, sizeof(ref_str), "#/components/schemas/%s",
                     field->enum_array_ref);
        json_object_set_string(items_obj, "$ref", ref_str);
      } else if (*ref) {
        if (strcmp(ref, "integer") == 0 || strcmp(ref, "string") == 0 ||
            strcmp(ref, "boolean") == 0 || strcmp(ref, "number") == 0) {
          json_object_set_string(items_obj, "type", ref);
        } else {
          char ref_str[128];
          CDD_SNPRINTF(ref_str, sizeof(ref_str), "#/components/schemas/%s",
                       ref);
          json_object_set_string(items_obj, "$ref", ref_str);
        }
      }
      if (c2s_merge_schema_extras_object(items_obj, field->items_extra_json) !=
          0) {
        json_value_free(items_val);
        json_value_free(pval);
        json_value_free(req_val);
        json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
      if (json_object_set_value(pobj, "items", items_val) != JSONSuccess) {
        json_value_free(items_val);
        json_value_free(pval);
        json_value_free(req_val);
        json_value_free(val);
        return CDD_C_ERROR_MEMORY;
      }
      c2s_write_array_constraints(pobj, field);
    } else {
      if (strcmp(typ, "object") == 0 || strcmp(typ, "enum") == 0) {
        char ref_str[128];
        if (*ref) {
          if (ref[0] == '#') {
            CDD_SNPRINTF(ref_str, sizeof(ref_str), "%s", ref);
          } else {
            CDD_SNPRINTF(ref_str, sizeof(ref_str), "#/components/schemas/%s",
                         ref);
          }
          json_object_set_string(pobj, "$ref", ref_str);
        } else {
          c2s_write_type_union(pobj, "object", field->type_union,
                               field->n_type_union);
        }
      } else {
        c2s_write_type_union(pobj, typ, field->type_union, field->n_type_union);
      }
    }
    c2s_write_numeric_constraints(pobj, field);
    c2s_write_string_constraints(pobj, field);
    c2s_write_default_value(pobj, field);
    if (field->description[0])
      json_object_set_string(pobj, "description", field->description);
    if (field->format[0])
      json_object_set_string(pobj, "format", field->format);
    if (field->deprecated_set)
      json_object_set_boolean(pobj, "deprecated", field->deprecated ? 1 : 0);
    if (field->read_only_set)
      json_object_set_boolean(pobj, "readOnly", field->read_only ? 1 : 0);
    if (field->write_only_set)
      json_object_set_boolean(pobj, "writeOnly", field->write_only ? 1 : 0);
    if (c2s_merge_schema_extras_object(pobj, field->schema_extra_json) != 0) {
      json_value_free(pval);
      json_value_free(req_val);
      json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    if (json_object_set_value(props_obj, prop_key, pval) != JSONSuccess) {
      json_value_free(pval);
      json_value_free(req_val);
      json_value_free(val);
      return CDD_C_ERROR_MEMORY;
    }
    if (field->required && !field->serde_skip &&
        !field->serde_skip_serializing) {
      if (!req_val) {
        req_val = json_value_init_array();
        req_arr = json_value_get_array(req_val);
      }
      json_array_append_string(req_arr, prop_key);
    }
  }

  if (req_val) {
    json_object_set_value(obj, "required", req_val);
  }

  if (c2s_merge_schema_extras_object(obj, sf->schema_extra_json) != 0) {
    json_value_free(val);
    return CDD_C_ERROR_MEMORY;
  }

  if (json_object_set_value(schemas_obj, schema_key, val) != JSONSuccess) {
    json_value_free(val);
    return CDD_C_ERROR_MEMORY;
  }

  return CDD_C_SUCCESS;
}
