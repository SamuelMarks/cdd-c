/**
 * @file code2schema_merge.c
 * @brief Merging struct fields and handling allOf composition.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/*
 * @brief Merges a source struct field into a destination struct field.
 *
 * @param[in,out] dest Destination StructField to merge constraints into.
 * @param[in] src Source StructField to merge from.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t merge_struct_field(struct StructField *dest,
                                 const struct StructField *src) {
  if (!dest || !src)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (!dest->default_val[0] && src->default_val[0]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(dest->default_val, sizeof(dest->default_val), src->default_val,
              sizeof(dest->default_val) - 1);
#else
    strncpy(dest->default_val, src->default_val, sizeof(dest->default_val) - 1);
#endif
    dest->default_val[sizeof(dest->default_val) - 1] = '\0';
  }

  if (src->required)
    dest->required = 1;

  if (src->has_min) {
    if (!dest->has_min || src->min_val > dest->min_val ||
        (src->min_val == dest->min_val && src->exclusive_min &&
         !dest->exclusive_min)) {
      dest->has_min = 1;
      dest->min_val = src->min_val;
      dest->exclusive_min = src->exclusive_min;
    }
  }

  if (src->has_max) {
    if (!dest->has_max || src->max_val < dest->max_val ||
        (src->max_val == dest->max_val && src->exclusive_max &&
         !dest->exclusive_max)) {
      dest->has_max = 1;
      dest->max_val = src->max_val;
      dest->exclusive_max = src->exclusive_max;
    }
  }

  if (src->has_min_len) {
    if (!dest->has_min_len || src->min_len > dest->min_len) {
      dest->has_min_len = 1;
      dest->min_len = src->min_len;
    }
  }

  if (src->has_max_len) {
    if (!dest->has_max_len || src->max_len < dest->max_len) {
      dest->has_max_len = 1;
      dest->max_len = src->max_len;
    }
  }

  if (src->has_min_items) {
    if (!dest->has_min_items || src->min_items > dest->min_items) {
      dest->has_min_items = 1;
      dest->min_items = src->min_items;
    }
  }

  if (src->has_max_items) {
    if (!dest->has_max_items || src->max_items < dest->max_items) {
      dest->has_max_items = 1;
      dest->max_items = src->max_items;
    }
  }

  if (src->unique_items)
    dest->unique_items = 1;

  if (!dest->pattern[0] && src->pattern[0]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(dest->pattern, sizeof(dest->pattern), src->pattern,
              sizeof(dest->pattern) - 1);
#else
    strncpy(dest->pattern, src->pattern, sizeof(dest->pattern) - 1);
#endif
    dest->pattern[sizeof(dest->pattern) - 1] = '\0';
  }

  if (src->is_flexible_array)
    dest->is_flexible_array = 1;

  if (!dest->bit_width[0] && src->bit_width[0]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    strncpy_s(dest->bit_width, sizeof(dest->bit_width), src->bit_width,
              sizeof(dest->bit_width) - 1);
#else
    strncpy(dest->bit_width, src->bit_width, sizeof(dest->bit_width) - 1);
#endif
    dest->bit_width[sizeof(dest->bit_width) - 1] = '\0';
  }

  c2s_merge_schema_extras_strings(&dest->schema_extra_json,
                                  src->schema_extra_json);
  c2s_merge_schema_extras_strings(&dest->items_extra_json,
                                  src->items_extra_json);

  if (!dest->type_union && src->type_union) {
    copy_string_array_code2schema(&dest->type_union, &dest->n_type_union,
                                  src->type_union, src->n_type_union);
  }
  if (!dest->items_type_union && src->items_type_union) {
    copy_string_array_code2schema(
        &dest->items_type_union, &dest->n_items_type_union,
        src->items_type_union, src->n_items_type_union);
  }

  /* Propagate Serde field reflection attributes */
  if (!dest->serde_rename[0] && src->serde_rename[0])
    CDD_STRCPY(dest->serde_rename, sizeof(dest->serde_rename),
               src->serde_rename);
  if (!dest->serde_rename_ser[0] && src->serde_rename_ser[0])
    CDD_STRCPY(dest->serde_rename_ser, sizeof(dest->serde_rename_ser),
               src->serde_rename_ser);
  if (!dest->serde_rename_de[0] && src->serde_rename_de[0])
    CDD_STRCPY(dest->serde_rename_de, sizeof(dest->serde_rename_de),
               src->serde_rename_de);
  if (dest->serde_case == CDD_SERDE_CASE_NONE &&
      src->serde_case != CDD_SERDE_CASE_NONE)
    dest->serde_case = src->serde_case;
  if (!dest->serde_replace_from[0] && src->serde_replace_from[0]) {
    CDD_STRCPY(dest->serde_replace_from, sizeof(dest->serde_replace_from),
               src->serde_replace_from);
    CDD_STRCPY(dest->serde_replace_to, sizeof(dest->serde_replace_to),
               src->serde_replace_to);
  }
  if (src->serde_skip)
    dest->serde_skip = 1;
  if (src->serde_skip_serializing)
    dest->serde_skip_serializing = 1;
  if (src->serde_skip_deserializing)
    dest->serde_skip_deserializing = 1;
  if (dest->serde_skip_if_kind == CDD_SERDE_SKIP_IF_NONE &&
      src->serde_skip_if_kind != CDD_SERDE_SKIP_IF_NONE) {
    dest->serde_skip_if_kind = src->serde_skip_if_kind;
    CDD_STRCPY(dest->serde_skip_if_fn, sizeof(dest->serde_skip_if_fn),
               src->serde_skip_if_fn);
  }
  if (src->serde_flatten)
    dest->serde_flatten = 1;
  if (src->serde_has_default)
    dest->serde_has_default = 1;
  if (!dest->serde_default_fn[0] && src->serde_default_fn[0])
    CDD_STRCPY(dest->serde_default_fn, sizeof(dest->serde_default_fn),
               src->serde_default_fn);
  if (!dest->serde_with[0] && src->serde_with[0])
    CDD_STRCPY(dest->serde_with, sizeof(dest->serde_with), src->serde_with);
  if (!dest->serde_serialize_with[0] && src->serde_serialize_with[0])
    CDD_STRCPY(dest->serde_serialize_with, sizeof(dest->serde_serialize_with),
               src->serde_serialize_with);
  if (!dest->serde_deserialize_with[0] && src->serde_deserialize_with[0])
    CDD_STRCPY(dest->serde_deserialize_with,
               sizeof(dest->serde_deserialize_with),
               src->serde_deserialize_with);
  if (src->serde_aliases) {
    size_t a_idx;
    for (a_idx = 0; a_idx < src->n_serde_aliases; ++a_idx) {
      if (src->serde_aliases[a_idx]) {
        struct_field_add_alias(dest, src->serde_aliases[a_idx]);
      }
    }
  }

  /* Propagate CLI field reflection attributes */
  if (!dest->cli_short && src->cli_short)
    dest->cli_short = src->cli_short;
  if (!dest->cli_short_aliases[0] && src->cli_short_aliases[0])
    CDD_STRCPY(dest->cli_short_aliases, sizeof(dest->cli_short_aliases),
               src->cli_short_aliases);
  if (!dest->cli_long[0] && src->cli_long[0])
    CDD_STRCPY(dest->cli_long, sizeof(dest->cli_long), src->cli_long);
  if (!dest->cli_has_long && src->cli_has_long)
    dest->cli_has_long = src->cli_has_long;
  if (src->cli_negatable)
    dest->cli_negatable = 1;
  if (src->cli_positional)
    dest->cli_positional = 1;
  if (dest->cli_index < 0 && src->cli_index >= 0)
    dest->cli_index = src->cli_index;
  if (src->cli_last)
    dest->cli_last = 1;
  if (src->cli_trailing_var_arg)
    dest->cli_trailing_var_arg = 1;
  if (!dest->cli_value_name[0] && src->cli_value_name[0])
    CDD_STRCPY(dest->cli_value_name, sizeof(dest->cli_value_name),
               src->cli_value_name);
  if (!dest->cli_help[0] && src->cli_help[0])
    CDD_STRCPY(dest->cli_help, sizeof(dest->cli_help), src->cli_help);
  if (!dest->cli_long_help && src->cli_long_help) {
    c_cdd_strdup(src->cli_long_help, &dest->cli_long_help);
  }
  if (!dest->cli_help_heading[0] && src->cli_help_heading[0])
    CDD_STRCPY(dest->cli_help_heading, sizeof(dest->cli_help_heading),
               src->cli_help_heading);
  if (!dest->cli_env[0] && src->cli_env[0])
    CDD_STRCPY(dest->cli_env, sizeof(dest->cli_env), src->cli_env);
  if (src->cli_hide_env)
    dest->cli_hide_env = 1;
  if (src->cli_hide_env_values)
    dest->cli_hide_env_values = 1;
  if (!dest->cli_default_missing_val[0] && src->cli_default_missing_val[0])
    CDD_STRCPY(dest->cli_default_missing_val,
               sizeof(dest->cli_default_missing_val),
               src->cli_default_missing_val);
  if (src->cli_hide_default_value)
    dest->cli_hide_default_value = 1;
  if (dest->cli_action == CDD_CLI_ACTION_AUTO &&
      src->cli_action != CDD_CLI_ACTION_AUTO)
    dest->cli_action = src->cli_action;
  if (dest->cli_value_hint == CDD_CLI_HINT_NONE &&
      src->cli_value_hint != CDD_CLI_HINT_NONE)
    dest->cli_value_hint = src->cli_value_hint;
  if (!dest->cli_value_delimiter && src->cli_value_delimiter)
    dest->cli_value_delimiter = src->cli_value_delimiter;
  if (!dest->cli_num_args_min && src->cli_num_args_min)
    dest->cli_num_args_min = src->cli_num_args_min;
  if (!dest->cli_num_args_max && src->cli_num_args_max)
    dest->cli_num_args_max = src->cli_num_args_max;
  if (src->cli_require_equals)
    dest->cli_require_equals = 1;
  if (src->cli_allow_hyphen_values)
    dest->cli_allow_hyphen_values = 1;
  if (src->cli_global)
    dest->cli_global = 1;
  if (src->cli_hidden)
    dest->cli_hidden = 1;
  if (src->cli_exclusive)
    dest->cli_exclusive = 1;
  if (src->cli_subcommand)
    dest->cli_subcommand = 1;
  if (!dest->cli_value_parser[0] && src->cli_value_parser[0])
    CDD_STRCPY(dest->cli_value_parser, sizeof(dest->cli_value_parser),
               src->cli_value_parser);
  if (!dest->cli_group[0] && src->cli_group[0])
    CDD_STRCPY(dest->cli_group, sizeof(dest->cli_group), src->cli_group);
  if (!dest->cli_required_if_field[0] && src->cli_required_if_field[0]) {
    CDD_STRCPY(dest->cli_required_if_field, sizeof(dest->cli_required_if_field),
               src->cli_required_if_field);
    CDD_STRCPY(dest->cli_required_if_val, sizeof(dest->cli_required_if_val),
               src->cli_required_if_val);
  }
  if (src->cli_conflicts_with) {
    size_t c_idx;
    for (c_idx = 0; c_idx < src->n_cli_conflicts_with; ++c_idx) {
      if (src->cli_conflicts_with[c_idx])
        struct_field_add_cli_conflict(dest, src->cli_conflicts_with[c_idx]);
    }
  }
  if (src->cli_requires) {
    size_t r_idx;
    for (r_idx = 0; r_idx < src->n_cli_requires; ++r_idx) {
      if (src->cli_requires[r_idx])
        struct_field_add_cli_requires(dest, src->cli_requires[r_idx]);
    }
  }
  if (src->cli_required_unless) {
    size_t u_idx;
    for (u_idx = 0; u_idx < src->n_cli_required_unless; ++u_idx) {
      if (src->cli_required_unless[u_idx])
        struct_field_add_cli_required_unless(dest,
                                             src->cli_required_unless[u_idx]);
    }
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Merges source struct fields into destination struct fields.
 *
 * @param[in,out] dest Destination StructFields container.
 * @param[in] src Source StructFields container to merge from.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t merge_struct_fields(struct StructFields *dest,
                                  const struct StructFields *src) {
  size_t i;

  if (!dest || !src)
    return CDD_C_SUCCESS;

  if (src->is_enum)
    return CDD_C_SUCCESS;

  /* Propagate container-level Serde attributes */
  if (!dest->serde_rename[0] && src->serde_rename[0])
    CDD_STRCPY(dest->serde_rename, sizeof(dest->serde_rename),
               src->serde_rename);
  if (dest->serde_rename_all == CDD_SERDE_CASE_NONE &&
      src->serde_rename_all != CDD_SERDE_CASE_NONE)
    dest->serde_rename_all = src->serde_rename_all;
  if (dest->serde_rename_all_ser == CDD_SERDE_CASE_NONE &&
      src->serde_rename_all_ser != CDD_SERDE_CASE_NONE)
    dest->serde_rename_all_ser = src->serde_rename_all_ser;
  if (dest->serde_rename_all_de == CDD_SERDE_CASE_NONE &&
      src->serde_rename_all_de != CDD_SERDE_CASE_NONE)
    dest->serde_rename_all_de = src->serde_rename_all_de;
  if (!dest->serde_replace_from[0] && src->serde_replace_from[0]) {
    CDD_STRCPY(dest->serde_replace_from, sizeof(dest->serde_replace_from),
               src->serde_replace_from);
    CDD_STRCPY(dest->serde_replace_to, sizeof(dest->serde_replace_to),
               src->serde_replace_to);
  }
  if (!dest->serde_prefix[0] && src->serde_prefix[0])
    CDD_STRCPY(dest->serde_prefix, sizeof(dest->serde_prefix),
               src->serde_prefix);
  if (!dest->serde_suffix[0] && src->serde_suffix[0])
    CDD_STRCPY(dest->serde_suffix, sizeof(dest->serde_suffix),
               src->serde_suffix);
  if (src->serde_deny_unknown_fields)
    dest->serde_deny_unknown_fields = 1;
  if (src->serde_default)
    dest->serde_default = 1;
  if (!dest->serde_tag[0] && src->serde_tag[0])
    CDD_STRCPY(dest->serde_tag, sizeof(dest->serde_tag), src->serde_tag);
  if (!dest->serde_content[0] && src->serde_content[0])
    CDD_STRCPY(dest->serde_content, sizeof(dest->serde_content),
               src->serde_content);
  if (src->serde_untagged)
    dest->serde_untagged = 1;

  /* Propagate container-level CLI attributes */
  if (src->is_cli_command)
    dest->is_cli_command = 1;
  if (!dest->cli_name[0] && src->cli_name[0])
    CDD_STRCPY(dest->cli_name, sizeof(dest->cli_name), src->cli_name);
  if (!dest->cli_bin_name[0] && src->cli_bin_name[0])
    CDD_STRCPY(dest->cli_bin_name, sizeof(dest->cli_bin_name),
               src->cli_bin_name);
  if (!dest->cli_version[0] && src->cli_version[0])
    CDD_STRCPY(dest->cli_version, sizeof(dest->cli_version), src->cli_version);
  if (!dest->cli_author[0] && src->cli_author[0])
    CDD_STRCPY(dest->cli_author, sizeof(dest->cli_author), src->cli_author);
  if (!dest->cli_about[0] && src->cli_about[0])
    CDD_STRCPY(dest->cli_about, sizeof(dest->cli_about), src->cli_about);
  if (!dest->cli_long_about && src->cli_long_about) {
    if (c_cdd_strdup(src->cli_long_about, &dest->cli_long_about) !=
        CDD_C_SUCCESS)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dest->cli_before_help && src->cli_before_help) {
    if (c_cdd_strdup(src->cli_before_help, &dest->cli_before_help) !=
        CDD_C_SUCCESS)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dest->cli_after_help && src->cli_after_help) {
    if (c_cdd_strdup(src->cli_after_help, &dest->cli_after_help) !=
        CDD_C_SUCCESS)
      return CDD_C_ERROR_MEMORY;
  }
  if (!dest->cli_usage_override[0] && src->cli_usage_override[0])
    CDD_STRCPY(dest->cli_usage_override, sizeof(dest->cli_usage_override),
               src->cli_usage_override);
  if (dest->cli_rename_all == CDD_SERDE_CASE_NONE &&
      src->cli_rename_all != CDD_SERDE_CASE_NONE)
    dest->cli_rename_all = src->cli_rename_all;
  if (dest->cli_rename_all_env == CDD_SERDE_CASE_NONE &&
      src->cli_rename_all_env != CDD_SERDE_CASE_NONE)
    dest->cli_rename_all_env = src->cli_rename_all_env;
  if (!dest->cli_env_prefix[0] && src->cli_env_prefix[0])
    CDD_STRCPY(dest->cli_env_prefix, sizeof(dest->cli_env_prefix),
               src->cli_env_prefix);
  if (src->cli_arg_required_else_help)
    dest->cli_arg_required_else_help = 1;
  if (src->cli_subcommand_required)
    dest->cli_subcommand_required = 1;
  if (src->cli_allow_external_subcommands)
    dest->cli_allow_external_subcommands = 1;
  if (src->cli_disable_help_flag)
    dest->cli_disable_help_flag = 1;
  if (src->cli_disable_version_flag)
    dest->cli_disable_version_flag = 1;
  if (src->cli_disable_help_subcommand)
    dest->cli_disable_help_subcommand = 1;
  if (src->cli_propagate_version)
    dest->cli_propagate_version = 1;
  if (src->cli_allow_windows_slash_flags)
    dest->cli_allow_windows_slash_flags = 1;
  if (src->cli_allow_response_files)
    dest->cli_allow_response_files = 1;
  if (src->cli_case_insensitive_flags)
    dest->cli_case_insensitive_flags = 1;
  if (src->cli_aliases) {
    size_t a_idx;
    for (a_idx = 0; a_idx < src->n_cli_aliases; ++a_idx) {
      if (src->cli_aliases[a_idx]) {
        if (struct_fields_add_cli_alias(dest, src->cli_aliases[a_idx]) !=
            CDD_C_SUCCESS)
          return CDD_C_ERROR_MEMORY;
      }
    }
  }

  if (c2s_merge_schema_extras_strings(&dest->schema_extra_json,
                                      src->schema_extra_json) != 0)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < src->size; ++i) {
    const struct StructField *src_field = &src->fields[i];
    struct StructField *dest_field = NULL;
    struct_fields_get(dest, src_field->name, &dest_field);

    if (!dest_field) {
      const char *ref = src_field->ref[0] ? src_field->ref : NULL;
      const char *def =
          src_field->default_val[0] ? src_field->default_val : NULL;
      const char *bw = src_field->bit_width[0] ? src_field->bit_width : NULL;
      if (struct_fields_add(dest, src_field->name, src_field->type, ref, def,
                            bw) != 0)
        return CDD_C_ERROR_MEMORY;
      dest_field = NULL;
      struct_fields_get(dest, src_field->name, &dest_field);

      {
        struct StructField tmp = *src_field;
        *dest_field = tmp;
        dest_field->schema_extra_json = NULL;
        dest_field->items_extra_json = NULL;
        dest_field->type_union = NULL;
        dest_field->n_type_union = 0;
        dest_field->items_type_union = NULL;
        dest_field->n_items_type_union = 0;
        dest_field->serde_aliases = NULL;
        dest_field->n_serde_aliases = 0;
        dest_field->cli_long_help = NULL;
        dest_field->cli_conflicts_with = NULL;
        dest_field->n_cli_conflicts_with = 0;
        dest_field->cli_requires = NULL;
        dest_field->n_cli_requires = 0;
        dest_field->cli_required_unless = NULL;
        dest_field->n_cli_required_unless = 0;
        if (src_field->schema_extra_json) {
          cdd_c_error_t rc_c2s = c_cdd_strdup(src_field->schema_extra_json,
                                              &dest_field->schema_extra_json);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
        }
        if (src_field->items_extra_json) {
          cdd_c_error_t rc_c2s = c_cdd_strdup(src_field->items_extra_json,
                                              &dest_field->items_extra_json);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
        }
        if (src_field->type_union) {
          if (copy_string_array_code2schema(
                  &dest_field->type_union, &dest_field->n_type_union,
                  src_field->type_union, src_field->n_type_union) != 0)
            return CDD_C_ERROR_MEMORY;
        }
        if (src_field->items_type_union) {
          if (copy_string_array_code2schema(&dest_field->items_type_union,
                                            &dest_field->n_items_type_union,
                                            src_field->items_type_union,
                                            src_field->n_items_type_union) != 0)
            return CDD_C_ERROR_MEMORY;
        }
        if (src_field->serde_aliases) {
          if (copy_string_array_code2schema(
                  &dest_field->serde_aliases, &dest_field->n_serde_aliases,
                  src_field->serde_aliases, src_field->n_serde_aliases) != 0)
            return CDD_C_ERROR_MEMORY;
        }
        if (src_field->cli_long_help) {
          cdd_c_error_t rc_c2s = c_cdd_strdup(src_field->cli_long_help,
                                              &dest_field->cli_long_help);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
        }
        if (src_field->cli_conflicts_with) {
          if (copy_string_array_code2schema(&dest_field->cli_conflicts_with,
                                            &dest_field->n_cli_conflicts_with,
                                            src_field->cli_conflicts_with,
                                            src_field->n_cli_conflicts_with) !=
              0)
            return CDD_C_ERROR_MEMORY;
        }
        if (src_field->cli_requires) {
          if (copy_string_array_code2schema(
                  &dest_field->cli_requires, &dest_field->n_cli_requires,
                  src_field->cli_requires, src_field->n_cli_requires) != 0)
            return CDD_C_ERROR_MEMORY;
        }
        if (src_field->cli_required_unless) {
          if (copy_string_array_code2schema(&dest_field->cli_required_unless,
                                            &dest_field->n_cli_required_unless,
                                            src_field->cli_required_unless,
                                            src_field->n_cli_required_unless) !=
              0)
            return CDD_C_ERROR_MEMORY;
        }
      }
      continue;
    }

    merge_struct_field(dest_field, src_field);
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Applies an allOf JSON Schema array to a StructFields object.
 *
 * @param[in] all_of JSON_Array of `allOf` sub-schemas.
 * @param[in,out] dest Target StructFields container to merge into.
 * @param[in] root Root `components.schemas` JSON_Object for `$ref` resolution.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t apply_allof_to_struct_fields(const JSON_Array *all_of,
                                           struct StructFields *dest,
                                           const JSON_Object *root) {
  size_t i, count;

  if (!all_of || !dest)
    return CDD_C_SUCCESS;

  count = json_array_get_count(all_of);
  for (i = 0; i < count; ++i) {
    const JSON_Object *sub = json_array_get_object(all_of, i);
    const JSON_Object *resolved = sub;
    const char *ref;
    struct StructFields tmp;
    cdd_c_error_t rc;

    if (!sub)
      continue;

    ref = json_object_get_string(sub, "$ref");
    if (ref && root) {
      cdd_c_error_t rc_c2s = resolve_schema_ref_object(root, ref, &resolved);
      if (rc_c2s != CDD_C_SUCCESS)
        return rc_c2s;
    }
    if (!resolved)
      continue;

    rc = struct_fields_init(&tmp);
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = json_object_to_struct_fields(resolved, &tmp, root);
    if (rc != CDD_C_SUCCESS) {
      struct_fields_free(&tmp);
      return rc;
    }

    rc = merge_struct_fields(dest, &tmp);
    struct_fields_free(&tmp);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}
