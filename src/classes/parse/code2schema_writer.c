/**
 * @file code2schema_writer.c
 * @brief Writing struct fields and constraints to JSON schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Descriptor mapping an embedded `char[]` buffer to a JSON string key.
 */
struct c2s_buf_entry {
  /** @brief Target JSON property key. */
  const char *key;
  /** @brief Byte offset of `char[]` buffer within the source struct. */
  size_t offset;
};

/**
 * @brief Descriptor mapping an `int` boolean flag to a JSON boolean key.
 */
struct c2s_flag_entry {
  /** @brief Target JSON property key. */
  const char *key;
  /** @brief Byte offset of `int` flag within the source struct. */
  size_t offset;
};

/**
 * @brief Descriptor mapping an `enum CddSerdeCaseRule` field to a JSON key.
 */
struct c2s_case_entry {
  /** @brief Target JSON property key. */
  const char *key;
  /** @brief Byte offset of `enum CddSerdeCaseRule` within the source struct. */
  size_t offset;
};

/**
 * @brief Descriptor mapping a `char *` heap string pointer to a JSON key.
 */
struct c2s_ptr_str_entry {
  /** @brief Target JSON property key. */
  const char *key;
  /** @brief Byte offset of `char *` pointer within the source struct. */
  size_t offset;
};

/** @brief Container-level `enum CddSerdeCaseRule` properties. */
static const struct c2s_case_entry k_container_case_entries[] = {
    {"x-cdd-rename-all", offsetof(struct StructFields, serde_rename_all)},
    {"x-cdd-rename-all-serialize",
     offsetof(struct StructFields, serde_rename_all_ser)},
    {"x-cdd-rename-all-deserialize",
     offsetof(struct StructFields, serde_rename_all_de)},
    {"x-cdd-cli-rename-all", offsetof(struct StructFields, cli_rename_all)},
    {"x-cdd-cli-rename-all-env",
     offsetof(struct StructFields, cli_rename_all_env)}};

/** @brief Container-level `char[]` string buffer properties. */
static const struct c2s_buf_entry k_container_buf_entries[] = {
    {"x-cdd-prefix", offsetof(struct StructFields, serde_prefix)},
    {"x-cdd-suffix", offsetof(struct StructFields, serde_suffix)},
    {"x-cdd-tag", offsetof(struct StructFields, serde_tag)},
    {"x-cdd-content", offsetof(struct StructFields, serde_content)},
    {"x-cdd-cli-name", offsetof(struct StructFields, cli_name)},
    {"x-cdd-cli-bin-name", offsetof(struct StructFields, cli_bin_name)},
    {"x-cdd-cli-version", offsetof(struct StructFields, cli_version)},
    {"x-cdd-cli-about", offsetof(struct StructFields, cli_about)},
    {"x-cdd-cli-author", offsetof(struct StructFields, cli_author)},
    {"x-cdd-cli-usage-override",
     offsetof(struct StructFields, cli_usage_override)},
    {"x-cdd-cli-env-prefix", offsetof(struct StructFields, cli_env_prefix)}};

/** @brief Container-level `char *` heap string properties. */
static const struct c2s_ptr_str_entry k_container_ptr_entries[] = {
    {"x-cdd-cli-long-about", offsetof(struct StructFields, cli_long_about)},
    {"x-cdd-cli-before-help", offsetof(struct StructFields, cli_before_help)},
    {"x-cdd-cli-after-help", offsetof(struct StructFields, cli_after_help)}};

/** @brief Container-level `int` boolean flag properties. */
static const struct c2s_flag_entry k_container_flag_entries[] = {
    {"x-cdd-deny-unknown-fields",
     offsetof(struct StructFields, serde_deny_unknown_fields)},
    {"x-cdd-default", offsetof(struct StructFields, serde_default)},
    {"x-cdd-untagged", offsetof(struct StructFields, serde_untagged)},
    {"x-cdd-cli-command", offsetof(struct StructFields, is_cli_command)},
    {"x-cdd-cli-arg-required-else-help",
     offsetof(struct StructFields, cli_arg_required_else_help)},
    {"x-cdd-cli-subcommand-required",
     offsetof(struct StructFields, cli_subcommand_required)},
    {"x-cdd-cli-allow-external-subcommands",
     offsetof(struct StructFields, cli_allow_external_subcommands)},
    {"x-cdd-cli-disable-help-flag",
     offsetof(struct StructFields, cli_disable_help_flag)},
    {"x-cdd-cli-disable-version-flag",
     offsetof(struct StructFields, cli_disable_version_flag)},
    {"x-cdd-cli-disable-help-subcommand",
     offsetof(struct StructFields, cli_disable_help_subcommand)},
    {"x-cdd-cli-propagate-version",
     offsetof(struct StructFields, cli_propagate_version)},
    {"x-cdd-cli-allow-windows-slash-flags",
     offsetof(struct StructFields, cli_allow_windows_slash_flags)},
    {"x-cdd-cli-allow-response-files",
     offsetof(struct StructFields, cli_allow_response_files)},
    {"x-cdd-cli-case-insensitive-flags",
     offsetof(struct StructFields, cli_case_insensitive_flags)}};

/** @brief Field-level `char[]` string buffer properties. */
static const struct c2s_buf_entry k_field_buf_entries[] = {
    {"x-c-bitwidth", offsetof(struct StructField, bit_width)},
    {"x-c-type", offsetof(struct StructField, c_type)},
    {"x-cdd-rename", offsetof(struct StructField, serde_rename)},
    {"x-cdd-rename-serialize", offsetof(struct StructField, serde_rename_ser)},
    {"x-cdd-rename-deserialize", offsetof(struct StructField, serde_rename_de)},
    {"x-cdd-default-fn", offsetof(struct StructField, serde_default_fn)},
    {"x-cdd-with", offsetof(struct StructField, serde_with)},
    {"x-cdd-serialize-with",
     offsetof(struct StructField, serde_serialize_with)},
    {"x-cdd-deserialize-with",
     offsetof(struct StructField, serde_deserialize_with)},
    {"x-cdd-cli-long", offsetof(struct StructField, cli_long)},
    {"x-cdd-cli-env", offsetof(struct StructField, cli_env)},
    {"x-cdd-cli-value-name", offsetof(struct StructField, cli_value_name)},
    {"x-cdd-cli-help", offsetof(struct StructField, cli_help)},
    {"x-cdd-cli-short-aliases",
     offsetof(struct StructField, cli_short_aliases)},
    {"x-cdd-cli-help-heading", offsetof(struct StructField, cli_help_heading)},
    {"x-cdd-cli-default-missing-val",
     offsetof(struct StructField, cli_default_missing_val)},
    {"x-cdd-cli-value-parser", offsetof(struct StructField, cli_value_parser)},
    {"x-cdd-cli-group", offsetof(struct StructField, cli_group)}};

/** @brief Field-level `int` boolean flag properties. */
static const struct c2s_flag_entry k_field_flag_entries[] = {
    {"x-c-nullable-ptr", offsetof(struct StructField, is_nullable_ptr)},
    {"x-cdd-flatten", offsetof(struct StructField, serde_flatten)},
    {"x-cdd-default", offsetof(struct StructField, serde_has_default)},
    {"x-cdd-cli-negatable", offsetof(struct StructField, cli_negatable)},
    {"x-cdd-cli-positional", offsetof(struct StructField, cli_positional)},
    {"x-cdd-cli-subcommand", offsetof(struct StructField, cli_subcommand)},
    {"x-cdd-cli-last", offsetof(struct StructField, cli_last)},
    {"x-cdd-cli-trailing-var-arg",
     offsetof(struct StructField, cli_trailing_var_arg)},
    {"x-cdd-cli-hide-env", offsetof(struct StructField, cli_hide_env)},
    {"x-cdd-cli-hide-env-values",
     offsetof(struct StructField, cli_hide_env_values)},
    {"x-cdd-cli-hide-default-value",
     offsetof(struct StructField, cli_hide_default_value)},
    {"x-cdd-cli-require-equals",
     offsetof(struct StructField, cli_require_equals)},
    {"x-cdd-cli-allow-hyphen-values",
     offsetof(struct StructField, cli_allow_hyphen_values)},
    {"x-cdd-cli-global", offsetof(struct StructField, cli_global)},
    {"x-cdd-cli-hidden", offsetof(struct StructField, cli_hidden)},
    {"x-cdd-cli-exclusive", offsetof(struct StructField, cli_exclusive)},
    {"x-cdd-cli-skip", offsetof(struct StructField, cli_skip)},
    {"x-cdd-cli-skip-to", offsetof(struct StructField, cli_skip_to_cli)},
    {"x-cdd-cli-skip-from", offsetof(struct StructField, cli_skip_from_cli)}};

/**
 * @brief Emits non-empty `char[]` buffer properties from `base` into `obj`.
 *
 * @param[in,out] obj Target JSON_Object.
 * @param[in] base Pointer to struct containing the `char[]` fields.
 * @param[in] entries Array of buffer descriptors.
 * @param[in] n_entries Number of descriptors in `entries`.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_emit_buf_entries(JSON_Object *obj, const void *base,
                                          const struct c2s_buf_entry *entries,
                                          size_t n_entries) {
  size_t i;
  cdd_c_error_t rc;
  for (i = 0; i < n_entries; ++i) {
    const char *s = (const char *)base + entries[i].offset;
    if (s[0] != '\0') {
      rc = cdd_json_object_set_string(obj, entries[i].key, s);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits non-zero `int` boolean flags from `base` into `obj`.
 *
 * @param[in,out] obj Target JSON_Object.
 * @param[in] base Pointer to struct containing the `int` flag fields.
 * @param[in] entries Array of flag descriptors.
 * @param[in] n_entries Number of descriptors in `entries`.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_emit_flag_entries(JSON_Object *obj, const void *base,
                                           const struct c2s_flag_entry *entries,
                                           size_t n_entries) {
  size_t i;
  cdd_c_error_t rc;
  for (i = 0; i < n_entries; ++i) {
    int flag = 0;
    memcpy(&flag, (const char *)base + entries[i].offset, sizeof(flag));
    if (flag) {
      rc = cdd_json_object_set_boolean(obj, entries[i].key, 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits active `enum CddSerdeCaseRule` properties from `base` into
 * `obj`.
 *
 * @param[in,out] obj Target JSON_Object.
 * @param[in] base Pointer to struct containing the case rule fields.
 * @param[in] entries Array of case rule descriptors.
 * @param[in] n_entries Number of descriptors in `entries`.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_emit_case_entries(JSON_Object *obj, const void *base,
                                           const struct c2s_case_entry *entries,
                                           size_t n_entries) {
  size_t i;
  cdd_c_error_t rc;
  for (i = 0; i < n_entries; ++i) {
    enum CddSerdeCaseRule rule = CDD_SERDE_CASE_NONE;
    memcpy(&rule, (const char *)base + entries[i].offset, sizeof(rule));
    if (rule != CDD_SERDE_CASE_NONE) {
      const char *r_str = NULL;
      rc = cdd_serde_case_rule_to_str(rule, &r_str);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = cdd_json_object_set_string(obj, entries[i].key, r_str);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits non-NULL `char *` heap string properties from `base` into `obj`.
 *
 * @param[in,out] obj Target JSON_Object.
 * @param[in] base Pointer to struct containing the `char *` fields.
 * @param[in] entries Array of pointer string descriptors.
 * @param[in] n_entries Number of descriptors in `entries`.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t
c2s_emit_ptr_str_entries(JSON_Object *obj, const void *base,
                         const struct c2s_ptr_str_entry *entries,
                         size_t n_entries) {
  size_t i;
  cdd_c_error_t rc;
  for (i = 0; i < n_entries; ++i) {
    const char *p = NULL;
    memcpy(&p, (const char *)base + entries[i].offset, sizeof(p));
    if (p) {
      rc = cdd_json_object_set_string(obj, entries[i].key, p);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Serializes a non-empty string pointer array as a JSON array property.
 *
 * @param[in,out] obj Target JSON_Object.
 * @param[in] key Property name.
 * @param[in] items Array of C string pointers (NULL elements are skipped).
 * @param[in] n_items Number of elements in `items`.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_write_string_list_prop(JSON_Object *obj,
                                                const char *key,
                                                char *const *items,
                                                size_t n_items) {
  JSON_Value *arr_val = NULL;
  JSON_Array *arr = NULL;
  size_t idx;
  cdd_c_error_t rc;

  if (n_items == 0 || !items)
    return CDD_C_SUCCESS;

  rc = cdd_json_value_init_array(&arr_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  arr = arr_val->array_val;
  for (idx = 0; idx < n_items; ++idx) {
    if (items[idx]) {
      rc = cdd_json_array_append_string(arr, items[idx]);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
      }
    }
  }
  rc = cdd_json_object_set_value(obj, key, arr_val);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(arr_val));
  }
  return CDD_C_SUCCESS;
}

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
  int bval = 0;
  double nval = 0.0;
  cdd_c_error_t rc;
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  def = field->default_val;
  if (def[0] == '\0')
    return CDD_C_SUCCESS;
  typ = field->type;

  if (strcmp(def, "nullptr") == 0) {
    return cdd_json_object_set_null(pobj, "default");
  }

  if (strcmp(typ, "string") == 0) {
    const char *s = NULL;
    rc = c2s_strip_quotes(def, buf, sizeof(buf), &s);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return cdd_json_object_set_string(pobj, "default", s);
  }

  if (strcmp(typ, "boolean") == 0) {
    int has_bval = 0;
    rc = c2s_parse_bool_default(def, &bval, &has_bval);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (has_bval) {
      return cdd_json_object_set_boolean(pobj, "default", bval);
    }
    return CDD_C_SUCCESS;
  }

  if (strcmp(typ, "integer") == 0 || strcmp(typ, "number") == 0) {
    int has_nval = 0;
    rc = c2s_parse_number_default(def, &nval, &has_nval);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (has_nval) {
      return cdd_json_object_set_number(pobj, "default", nval);
    }
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
  cdd_c_error_t rc;
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!field->type[0] || !(strcmp(field->type, "integer") == 0 ||
                           strcmp(field->type, "number") == 0))
    return CDD_C_SUCCESS;
  if (field->has_min) {
    const char *min_key = field->exclusive_min ? "exclusiveMinimum" : "minimum";
    rc = cdd_json_object_set_number(pobj, min_key, field->min_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->has_max) {
    const char *max_key = field->exclusive_max ? "exclusiveMaximum" : "maximum";
    rc = cdd_json_object_set_number(pobj, max_key, field->max_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
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
  cdd_c_error_t rc;
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!field->type[0] || strcmp(field->type, "string") != 0)
    return CDD_C_SUCCESS;
  if (field->has_min_len) {
    rc = cdd_json_object_set_number(pobj, "minLength", (double)field->min_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->has_max_len) {
    rc = cdd_json_object_set_number(pobj, "maxLength", (double)field->max_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->pattern[0] != '\0') {
    rc = cdd_json_object_set_string(pobj, "pattern", field->pattern);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
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
  cdd_c_error_t rc;
  if (!pobj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!field->type[0] || strcmp(field->type, "array") != 0)
    return CDD_C_SUCCESS;
  if (field->has_min_items) {
    rc = cdd_json_object_set_number(pobj, "minItems", (double)field->min_items);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->has_max_items) {
    rc = cdd_json_object_set_number(pobj, "maxItems", (double)field->max_items);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->unique_items) {
    rc = cdd_json_object_set_boolean(pobj, "uniqueItems", 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Writes a type union array to a JSON schema object.
 *
 * @param[in,out] obj Target schema or property JSON_Object.
 * @param[in] type Fallback single type string when `type_union` is NULL.
 * @param[in] type_union Optional array of type union strings.
 * @param[in] n_type_union Number of strings in `type_union`.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t c2s_write_type_union(JSON_Object *obj, const char *type,
                                   char **type_union, size_t n_type_union) {
  if (!obj)
    return CDD_C_SUCCESS;

  if (type_union) {
    return c2s_write_string_list_prop(obj, "type", type_union, n_type_union);
  }

  if (type) {
    return cdd_json_object_set_string(obj, "type", type);
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Writes enum schema members and `x-cdd-enum-*` extensions into `obj`.
 *
 * @param[in,out] obj Target schema JSON_Object.
 * @param[in] sf Source enum StructFields container.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_write_enum_body(JSON_Object *obj,
                                         const struct StructFields *sf) {
  JSON_Value *enum_val = NULL;
  JSON_Array *enum_arr = NULL;
  JSON_Value *c_names_val = NULL;
  JSON_Value *e_names_val = NULL;
  JSON_Value *ea_obj_val = NULL;
  JSON_Value *ed_val = NULL;
  JSON_Value *eh_val = NULL;
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
  size_t i;
  cdd_c_error_t rc;

  rc = cdd_json_value_init_array(&enum_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  enum_arr = enum_val->array_val;

  rc = cdd_json_object_set_string(obj, "type", "string");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (eff_rename_all != CDD_SERDE_CASE_NONE) {
    const char *r_str = NULL;
    rc = cdd_serde_case_rule_to_str(eff_rename_all, &r_str);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_json_object_set_string(obj, "x-cdd-rename-all", r_str);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (eff_rep_from[0] != '\0') {
    rc = cdd_json_object_set_string(obj, "x-cdd-replace-from", eff_rep_from);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_json_object_set_string(obj, "x-cdd-replace-to", eff_rep_to);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (em->serde_other_idx >= 0 && (size_t)em->serde_other_idx < em->size &&
      em->members[em->serde_other_idx]) {
    rc = cdd_json_object_set_string(obj, "x-cdd-enum-other",
                                    em->members[em->serde_other_idx]);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  for (i = 0; i < em->size; ++i) {
    const char *member = em->members[i];
    char wname[128];
    if (!member || strcmp(member, "UNKNOWN") == 0)
      continue;
    if (em->serde_skip && em->serde_skip[i])
      continue;
    rc = cdd_serde_resolve_enum_member_name(em, i, wname, sizeof(wname));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (strcmp(wname, member) != 0)
      has_custom_names = 1;
    rc = cdd_json_array_append_string(enum_arr, wname);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  if (has_custom_names) {
    JSON_Array *c_names_arr = NULL;
    JSON_Array *e_names_arr = NULL;
    rc = cdd_json_value_init_array(&c_names_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    c_names_arr = c_names_val->array_val;
    rc = cdd_json_value_init_array(&e_names_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    e_names_arr = e_names_val->array_val;
    for (i = 0; i < em->size; ++i) {
      const char *member = em->members[i];
      char wname[128];
      if (!member || strcmp(member, "UNKNOWN") == 0)
        continue;
      if (em->serde_skip && em->serde_skip[i])
        continue;
      rc = cdd_serde_resolve_enum_member_name(em, i, wname, sizeof(wname));
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_json_array_append_string(c_names_arr, member);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_json_array_append_string(e_names_arr, wname);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    rc = cdd_json_object_set_value(obj, "x-cdd-enum-c-names", c_names_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    c_names_val = NULL;
    rc = cdd_json_object_set_value(obj, "x-cdd-enum-names", e_names_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    e_names_val = NULL;
  }

  if (em->serde_aliases) {
    JSON_Object *ea_obj = NULL;
    rc = cdd_json_value_init_object(&ea_obj_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    ea_obj = ea_obj_val->object_val;
    for (i = 0; i < em->size; ++i) {
      rc = c2s_write_string_list_prop(
          ea_obj, em->members[i], em->serde_aliases[i], em->n_serde_aliases[i]);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    rc = cdd_json_object_set_value(obj, "x-cdd-enum-aliases", ea_obj_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    ea_obj_val = NULL;
  }

  if (em->cli_descriptions) {
    JSON_Object *ed_obj = NULL;
    rc = cdd_json_value_init_object(&ed_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    ed_obj = ed_val->object_val;
    for (i = 0; i < em->size; ++i) {
      if (em->cli_descriptions[i]) {
        rc = cdd_json_object_set_string(ed_obj, em->members[i],
                                        em->cli_descriptions[i]);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
    rc = cdd_json_object_set_value(obj, "x-cdd-enum-cli-descriptions", ed_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    ed_val = NULL;
  }

  if (em->cli_hidden) {
    JSON_Array *eh_arr = NULL;
    rc = cdd_json_value_init_array(&eh_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    eh_arr = eh_val->array_val;
    for (i = 0; i < em->size; ++i) {
      if (em->cli_hidden[i]) {
        rc = cdd_json_array_append_string(eh_arr, em->members[i]);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    }
    rc = cdd_json_object_set_value(obj, "x-cdd-enum-cli-hidden", eh_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    eh_val = NULL;
  }

  rc = cdd_json_object_set_value(obj, "enum", enum_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  enum_val = NULL;

cleanup: {
  return cdd_c_keep_err(
      rc, cdd_c_keep_err(
              cdd_json_value_free(enum_val),
              cdd_c_keep_err(
                  cdd_json_value_free(c_names_val),
                  cdd_c_keep_err(
                      cdd_json_value_free(e_names_val),
                      cdd_c_keep_err(
                          cdd_json_value_free(ea_obj_val),
                          cdd_c_keep_err(cdd_json_value_free(ed_val),
                                         cdd_json_value_free(eh_val)))))));
}
}

/**
 * @brief Writes field-level Serde, CLI, and C extensions into `pobj`.
 *
 * @param[in,out] pobj Target property JSON_Object.
 * @param[in] field Source StructField descriptor.
 * @param[in] prop_key Resolved serialized JSON property key.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_write_field_extensions(JSON_Object *pobj,
                                                const struct StructField *field,
                                                const char *prop_key) {
  cdd_c_error_t rc;

  rc = c2s_emit_buf_entries(pobj, field, k_field_buf_entries,
                            sizeof(k_field_buf_entries) /
                                sizeof(k_field_buf_entries[0]));
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = c2s_emit_flag_entries(pobj, field, k_field_flag_entries,
                             sizeof(k_field_flag_entries) /
                                 sizeof(k_field_flag_entries[0]));
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (field->fixed_array_len > 0) {
    rc = cdd_json_object_set_number(pobj, "x-c-fixed-array-len",
                                    (double)field->fixed_array_len);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (strcmp(prop_key, field->name) != 0) {
    rc = cdd_json_object_set_string(pobj, "x-cdd-c-name", field->name);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->serde_case != CDD_SERDE_CASE_NONE) {
    const char *c_str = NULL;
    rc = cdd_serde_case_rule_to_str(field->serde_case, &c_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_set_string(pobj, "x-cdd-case", c_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->serde_replace_from[0] != '\0') {
    rc = cdd_json_object_set_string(pobj, "x-cdd-replace-from",
                                    field->serde_replace_from);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_set_string(pobj, "x-cdd-replace-to",
                                    field->serde_replace_to);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = c2s_write_string_list_prop(pobj, "x-cdd-aliases", field->serde_aliases,
                                  field->n_serde_aliases);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (field->serde_skip) {
    rc = cdd_json_object_set_boolean(pobj, "x-cdd-skip", 1);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    if (field->serde_skip_serializing) {
      rc = cdd_json_object_set_boolean(pobj, "x-cdd-skip-serializing", 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    if (field->serde_skip_deserializing) {
      rc = cdd_json_object_set_boolean(pobj, "x-cdd-skip-deserializing", 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (field->serde_skip_if_kind != CDD_SERDE_SKIP_IF_NONE) {
    const char *skip_if_val = NULL;
    if (field->serde_skip_if_kind == CDD_SERDE_SKIP_IF_NULL)
      skip_if_val = "null";
    else if (field->serde_skip_if_kind == CDD_SERDE_SKIP_IF_EMPTY)
      skip_if_val = "empty";
    else if (field->serde_skip_if_kind == CDD_SERDE_SKIP_IF_ZERO)
      skip_if_val = "zero";
    else if (field->serde_skip_if_fn[0] != '\0')
      skip_if_val = field->serde_skip_if_fn;
    if (skip_if_val) {
      rc = cdd_json_object_set_string(pobj, "x-cdd-skip-serializing-if",
                                      skip_if_val);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (field->cli_short != '\0') {
    char sbuf[2];
    sbuf[0] = field->cli_short;
    sbuf[1] = '\0';
    rc = cdd_json_object_set_string(pobj, "x-cdd-cli-short", sbuf);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->cli_index >= 0) {
    rc = cdd_json_object_set_number(pobj, "x-cdd-cli-index",
                                    (double)field->cli_index);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->cli_action != CDD_CLI_ACTION_AUTO) {
    const char *act_str = NULL;
    rc = cdd_cli_action_to_str(field->cli_action, &act_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_set_string(pobj, "x-cdd-cli-action", act_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->cli_value_delimiter != '\0') {
    char dbuf[2];
    dbuf[0] = field->cli_value_delimiter;
    dbuf[1] = '\0';
    rc = cdd_json_object_set_string(pobj, "x-cdd-cli-delimiter", dbuf);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->cli_long_help) {
    rc = cdd_json_object_set_string(pobj, "x-cdd-cli-long-help",
                                    field->cli_long_help);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->cli_value_hint != CDD_CLI_HINT_NONE) {
    const char *vh_str = NULL;
    rc = cdd_cli_value_hint_to_str(field->cli_value_hint, &vh_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_set_string(pobj, "x-cdd-cli-value-hint", vh_str);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->cli_num_args_min > 0) {
    rc = cdd_json_object_set_number(pobj, "x-cdd-cli-num-args-min",
                                    (double)field->cli_num_args_min);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->cli_num_args_max > 0) {
    rc = cdd_json_object_set_number(pobj, "x-cdd-cli-num-args-max",
                                    (double)field->cli_num_args_max);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (field->cli_required_if_field[0] != '\0') {
    rc = cdd_json_object_set_string(pobj, "x-cdd-cli-required-if-field",
                                    field->cli_required_if_field);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_json_object_set_string(pobj, "x-cdd-cli-required-if-val",
                                    field->cli_required_if_val);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  rc = c2s_write_string_list_prop(pobj, "x-cdd-cli-conflicts-with",
                                  field->cli_conflicts_with,
                                  field->n_cli_conflicts_with);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c2s_write_string_list_prop(pobj, "x-cdd-cli-requires",
                                  field->cli_requires, field->n_cli_requires);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c2s_write_string_list_prop(pobj, "x-cdd-cli-required-unless",
                                  field->cli_required_unless,
                                  field->n_cli_required_unless);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves serialized JSON property key for `sf->fields[idx]`,
 * allowing empty `field->name` to fall back to `""`.
 *
 * @param[in] sf Enclosing StructFields container.
 * @param[in] idx Field index within `sf->fields`.
 * @param[out] prop_key Output buffer receiving serialized property key.
 * @param[in] prop_sz Capacity of `prop_key`.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_resolve_prop_key(const struct StructFields *sf,
                                          size_t idx, char *prop_key,
                                          size_t prop_sz) {
  cdd_c_error_t rc;
  prop_key[0] = '\0';
  rc = cdd_serde_resolve_field_names(sf, idx, prop_key, prop_sz, NULL, 0);
  if (rc == CDD_C_SUCCESS)
    return rc;
  if (rc != CDD_C_ERROR_INVALID_ARGUMENT)
    return rc;
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
  JSON_Value *val = NULL;
  JSON_Object *obj = NULL;
  JSON_Value *props_val = NULL;
  JSON_Object *props_obj = NULL;
  JSON_Value *req_val = NULL;
  JSON_Array *req_arr = NULL;
  JSON_Value *pval = NULL;
  JSON_Value *items_val = NULL;
  const char *schema_key;
  size_t i;
  cdd_c_error_t rc;

  if (!schemas_obj || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cdd_json_value_init_object(&val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  obj = val->object_val;

  schema_key = (sf->serde_rename[0] != '\0') ? sf->serde_rename : struct_name;

  if (sf->serde_rename[0] != '\0') {
    rc = cdd_json_object_set_string(obj, "x-cdd-rename", sf->serde_rename);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (strcmp(sf->serde_rename, struct_name) != 0) {
      rc = cdd_json_object_set_string(obj, "x-cdd-c-name", struct_name);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

  if (sf->is_enum) {
    rc = c2s_write_enum_body(obj, sf);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = c2s_merge_schema_extras_object(obj, sf->schema_extra_json);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_json_object_set_value(schemas_obj, schema_key, val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    val = NULL;
    goto cleanup;
  }

  rc = cdd_json_value_init_object(&props_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  props_obj = props_val->object_val;

  rc = cdd_json_object_set_string(obj, "type", "object");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_json_object_set_value(obj, "properties", props_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  props_val = NULL;

  rc = c2s_emit_case_entries(obj, sf, k_container_case_entries,
                             sizeof(k_container_case_entries) /
                                 sizeof(k_container_case_entries[0]));
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (sf->serde_replace_from[0] != '\0') {
    rc = cdd_json_object_set_string(obj, "x-cdd-replace-from",
                                    sf->serde_replace_from);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_json_object_set_string(obj, "x-cdd-replace-to",
                                    sf->serde_replace_to);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  rc = c2s_emit_buf_entries(obj, sf, k_container_buf_entries,
                            sizeof(k_container_buf_entries) /
                                sizeof(k_container_buf_entries[0]));
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = c2s_emit_ptr_str_entries(obj, sf, k_container_ptr_entries,
                                sizeof(k_container_ptr_entries) /
                                    sizeof(k_container_ptr_entries[0]));
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = c2s_emit_flag_entries(obj, sf, k_container_flag_entries,
                             sizeof(k_container_flag_entries) /
                                 sizeof(k_container_flag_entries[0]));
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = c2s_write_string_list_prop(obj, "x-cdd-cli-aliases", sf->cli_aliases,
                                  sf->n_cli_aliases);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  for (i = 0; i < sf->size; i++) {
    JSON_Object *pobj = NULL;
    const struct StructField *field = &sf->fields[i];
    const char *typ = field->type;
    const char *ref = field->ref;
    char prop_key[128];

    rc = cdd_json_value_init_object(&pval);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    pobj = pval->object_val;

    rc = c2s_resolve_prop_key(sf, i, prop_key, sizeof(prop_key));
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    rc = c2s_write_field_extensions(pobj, field, prop_key);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    if (strcmp(typ, "array") == 0) {
      JSON_Object *items_obj = NULL;
      rc = cdd_json_value_init_object(&items_val);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      items_obj = items_val->object_val;
      rc = c2s_write_type_union(pobj, "array", field->type_union,
                                field->n_type_union);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      if (field->items_type_union) {
        rc = c2s_write_type_union(items_obj, ref, field->items_type_union,
                                  field->n_items_type_union);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else if (field->is_enum_array) {
        char ref_str[300];
        CDD_SNPRINTF(ref_str, sizeof(ref_str), "#/components/schemas/%s",
                     field->enum_array_ref);
        rc = cdd_json_object_set_string(items_obj, "$ref", ref_str);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else if (*ref) {
        if (strcmp(ref, "integer") == 0 || strcmp(ref, "string") == 0 ||
            strcmp(ref, "boolean") == 0 || strcmp(ref, "number") == 0) {
          rc = cdd_json_object_set_string(items_obj, "type", ref);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        } else {
          char ref_str[300];
          CDD_SNPRINTF(ref_str, sizeof(ref_str), "#/components/schemas/%s",
                       ref);
          rc = cdd_json_object_set_string(items_obj, "$ref", ref_str);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      }
      rc = c2s_merge_schema_extras_object(items_obj, field->items_extra_json);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      rc = cdd_json_object_set_value(pobj, "items", items_val);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      items_val = NULL;
      rc = c2s_write_array_constraints(pobj, field);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    } else if (strcmp(typ, "object") == 0 || strcmp(typ, "enum") == 0) {
      if (*ref) {
        char ref_str[300];
        if (ref[0] == '#') {
          CDD_SNPRINTF(ref_str, sizeof(ref_str), "%s", ref);
        } else {
          CDD_SNPRINTF(ref_str, sizeof(ref_str), "#/components/schemas/%s",
                       ref);
        }
        rc = cdd_json_object_set_string(pobj, "$ref", ref_str);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      } else {
        rc = c2s_write_type_union(pobj, "object", field->type_union,
                                  field->n_type_union);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    } else {
      rc = c2s_write_type_union(pobj, typ, field->type_union,
                                field->n_type_union);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }

    rc = c2s_write_numeric_constraints(pobj, field);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = c2s_write_string_constraints(pobj, field);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = c2s_write_default_value(pobj, field);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (field->description[0]) {
      rc = cdd_json_object_set_string(pobj, "description", field->description);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if (field->format[0]) {
      rc = cdd_json_object_set_string(pobj, "format", field->format);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if (field->deprecated_set) {
      rc = cdd_json_object_set_boolean(pobj, "deprecated",
                                       field->deprecated ? 1 : 0);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if (field->read_only_set) {
      rc = cdd_json_object_set_boolean(pobj, "readOnly",
                                       field->read_only ? 1 : 0);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    if (field->write_only_set) {
      rc = cdd_json_object_set_boolean(pobj, "writeOnly",
                                       field->write_only ? 1 : 0);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
    rc = c2s_merge_schema_extras_object(pobj, field->schema_extra_json);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_json_object_set_value(props_obj, prop_key, pval);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    pval = NULL;
    if (field->required && !field->serde_skip &&
        !field->serde_skip_serializing) {
      if (!req_val) {
        rc = cdd_json_value_init_array(&req_val);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        req_arr = req_val->array_val;
      }
      rc = cdd_json_array_append_string(req_arr, prop_key);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
    }
  }

  if (req_val) {
    rc = cdd_json_object_set_value(obj, "required", req_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    req_val = NULL;
  }

  rc = c2s_merge_schema_extras_object(obj, sf->schema_extra_json);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_json_object_set_value(schemas_obj, schema_key, val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  val = NULL;

cleanup: {
  return cdd_c_keep_err(
      rc, cdd_c_keep_err(
              cdd_json_value_free(items_val),
              cdd_c_keep_err(
                  cdd_json_value_free(pval),
                  cdd_c_keep_err(cdd_json_value_free(req_val),
                                 cdd_c_keep_err(cdd_json_value_free(props_val),
                                                cdd_json_value_free(val))))));
}
}
