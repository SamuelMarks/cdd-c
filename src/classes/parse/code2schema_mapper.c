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
  cdd_c_error_t rc;

  if (!o || !f)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (c2s_collect_schema_extras(o, k_schema_skip_keys,
                                sizeof(k_schema_skip_keys) /
                                    sizeof(k_schema_skip_keys[0]),
                                &f->schema_extra_json) != 0)
    return CDD_C_ERROR_MEMORY;

  /* Load container-level Serde reflection extensions */
  {
    const char *s_ext = NULL;
    enum CddSerdeCaseRule c_rule = CDD_SERDE_CASE_NONE;
    int needs_san = 0;
    int has_val = 0, b_val = 0;
    if (schema_name) {
      g_cdd_last_alloc_err =
          c2s_needs_identifier_sanitization(schema_name, &needs_san);
      if (needs_san) {
        char *san_schema = NULL;
        if (sanitize_identifier(schema_name, &san_schema) == CDD_C_SUCCESS) {
          CDD_STRCPY(f->serde_rename, sizeof(f->serde_rename), schema_name);
          C_CDD_FREE(san_schema);
        }
      }
    }
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-rename", &s_ext);
    if (s_ext)
      CDD_STRCPY(f->serde_rename, sizeof(f->serde_rename), s_ext);
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-rename-all", &s_ext);
    if (s_ext && cdd_serde_parse_case_rule(s_ext, &c_rule) == CDD_C_SUCCESS)
      f->serde_rename_all = c_rule;
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-rename-all-serialize", &s_ext);
    if (s_ext && cdd_serde_parse_case_rule(s_ext, &c_rule) == CDD_C_SUCCESS)
      f->serde_rename_all_ser = c_rule;
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-rename-all-deserialize", &s_ext);
    if (s_ext && cdd_serde_parse_case_rule(s_ext, &c_rule) == CDD_C_SUCCESS)
      f->serde_rename_all_de = c_rule;
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-replace-from", &s_ext);
    if (s_ext)
      CDD_STRCPY(f->serde_replace_from, sizeof(f->serde_replace_from), s_ext);
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-replace-to", &s_ext);
    if (s_ext)
      CDD_STRCPY(f->serde_replace_to, sizeof(f->serde_replace_to), s_ext);
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-prefix", &s_ext);
    if (s_ext)
      CDD_STRCPY(f->serde_prefix, sizeof(f->serde_prefix), s_ext);
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-suffix", &s_ext);
    if (s_ext)
      CDD_STRCPY(f->serde_suffix, sizeof(f->serde_suffix), s_ext);
    has_val = 0;
    b_val = 0;
    g_cdd_last_alloc_err =
        cdd_json_object_has_value(o, "x-cdd-deny-unknown-fields", &has_val);
    if (has_val) {
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(o, "x-cdd-deny-unknown-fields", &b_val);
      if (b_val == 1)
        f->serde_deny_unknown_fields = 1;
    }
    has_val = 0;
    b_val = 0;
    g_cdd_last_alloc_err =
        cdd_json_object_has_value(o, "x-cdd-default", &has_val);
    if (has_val) {
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(o, "x-cdd-default", &b_val);
      if (b_val == 1)
        f->serde_default = 1;
    }
    s_ext = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_string(o, "x-cdd-tag", &s_ext);
    if (s_ext)
      CDD_STRCPY(f->serde_tag, sizeof(f->serde_tag), s_ext);
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-content", &s_ext);
    if (s_ext)
      CDD_STRCPY(f->serde_content, sizeof(f->serde_content), s_ext);
    has_val = 0;
    b_val = 0;
    g_cdd_last_alloc_err =
        cdd_json_object_has_value(o, "x-cdd-untagged", &has_val);
    if (has_val) {
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(o, "x-cdd-untagged", &b_val);
      if (b_val == 1)
        f->serde_untagged = 1;
    }

    /* Load container-level CLI reflection extensions */
    has_val = 0;
    b_val = 0;
    g_cdd_last_alloc_err =
        cdd_json_object_has_value(o, "x-cdd-cli-command", &has_val);
    if (has_val) {
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(o, "x-cdd-cli-command", &b_val);
      if (b_val == 1)
        f->is_cli_command = 1;
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-name", &s_ext);
    if (s_ext) {
      f->is_cli_command = 1;
      CDD_STRCPY(f->cli_name, sizeof(f->cli_name), s_ext);
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-version", &s_ext);
    if (s_ext) {
      f->is_cli_command = 1;
      CDD_STRCPY(f->cli_version, sizeof(f->cli_version), s_ext);
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-about", &s_ext);
    if (s_ext) {
      f->is_cli_command = 1;
      CDD_STRCPY(f->cli_about, sizeof(f->cli_about), s_ext);
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-author", &s_ext);
    if (s_ext) {
      f->is_cli_command = 1;
      CDD_STRCPY(f->cli_author, sizeof(f->cli_author), s_ext);
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-rename-all", &s_ext);
    if (s_ext && cdd_serde_parse_case_rule(s_ext, &c_rule) == CDD_C_SUCCESS)
      f->cli_rename_all = c_rule;
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-env-prefix", &s_ext);
    if (s_ext) {
      f->is_cli_command = 1;
      CDD_STRCPY(f->cli_env_prefix, sizeof(f->cli_env_prefix), s_ext);
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-bin-name", &s_ext);
    if (s_ext) {
      f->is_cli_command = 1;
      CDD_STRCPY(f->cli_bin_name, sizeof(f->cli_bin_name), s_ext);
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-long-about", &s_ext);
    if (s_ext) {
      f->is_cli_command = 1;
      g_cdd_last_alloc_err = c_cdd_strdup(s_ext, &f->cli_long_about);
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-before-help", &s_ext);
    if (s_ext) {
      f->is_cli_command = 1;
      g_cdd_last_alloc_err = c_cdd_strdup(s_ext, &f->cli_before_help);
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-after-help", &s_ext);
    if (s_ext) {
      f->is_cli_command = 1;
      g_cdd_last_alloc_err = c_cdd_strdup(s_ext, &f->cli_after_help);
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-usage-override", &s_ext);
    if (s_ext) {
      f->is_cli_command = 1;
      CDD_STRCPY(f->cli_usage_override, sizeof(f->cli_usage_override), s_ext);
    }
    s_ext = NULL;
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(o, "x-cdd-cli-rename-all-env", &s_ext);
    if (s_ext)
      g_cdd_last_alloc_err =
          cdd_serde_parse_case_rule(s_ext, &f->cli_rename_all_env);
    b_val = 0;
    g_cdd_last_alloc_err = cdd_json_object_get_boolean(
        o, "x-cdd-cli-arg-required-else-help", &b_val);
    if (b_val == 1)
      f->cli_arg_required_else_help = 1;
    b_val = 0;
    g_cdd_last_alloc_err =
        cdd_json_object_get_boolean(o, "x-cdd-cli-subcommand-required", &b_val);
    if (b_val == 1)
      f->cli_subcommand_required = 1;
    b_val = 0;
    g_cdd_last_alloc_err = cdd_json_object_get_boolean(
        o, "x-cdd-cli-allow-external-subcommands", &b_val);
    if (b_val == 1)
      f->cli_allow_external_subcommands = 1;
    b_val = 0;
    g_cdd_last_alloc_err =
        cdd_json_object_get_boolean(o, "x-cdd-cli-disable-help-flag", &b_val);
    if (b_val == 1)
      f->cli_disable_help_flag = 1;
    b_val = 0;
    g_cdd_last_alloc_err = cdd_json_object_get_boolean(
        o, "x-cdd-cli-disable-version-flag", &b_val);
    if (b_val == 1)
      f->cli_disable_version_flag = 1;
    b_val = 0;
    g_cdd_last_alloc_err = cdd_json_object_get_boolean(
        o, "x-cdd-cli-disable-help-subcommand", &b_val);
    if (b_val == 1)
      f->cli_disable_help_subcommand = 1;
    b_val = 0;
    g_cdd_last_alloc_err =
        cdd_json_object_get_boolean(o, "x-cdd-cli-propagate-version", &b_val);
    if (b_val == 1)
      f->cli_propagate_version = 1;
    b_val = 0;
    g_cdd_last_alloc_err = cdd_json_object_get_boolean(
        o, "x-cdd-cli-allow-windows-slash-flags", &b_val);
    if (b_val == 1)
      f->cli_allow_windows_slash_flags = 1;
    b_val = 0;
    g_cdd_last_alloc_err = cdd_json_object_get_boolean(
        o, "x-cdd-cli-allow-response-files", &b_val);
    if (b_val == 1)
      f->cli_allow_response_files = 1;
    b_val = 0;
    g_cdd_last_alloc_err = cdd_json_object_get_boolean(
        o, "x-cdd-cli-case-insensitive-flags", &b_val);
    if (b_val == 1)
      f->cli_case_insensitive_flags = 1;
    {
      JSON_Array *ca_arr = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_array(o, "x-cdd-cli-aliases", &ca_arr);
      if (ca_arr) {
        size_t ca_i, ca_cnt = 0;
        g_cdd_last_alloc_err = cdd_json_array_get_count(ca_arr, &ca_cnt);
        for (ca_i = 0; ca_i < ca_cnt; ++ca_i) {
          const char *ca_s = NULL;
          g_cdd_last_alloc_err = cdd_json_array_get_string(ca_arr, ca_i, &ca_s);
          g_cdd_last_alloc_err = struct_fields_add_cli_alias(f, ca_s);
        }
      }
    }
  }

  {
    int is_enum = 0;
    g_cdd_last_alloc_err = schema_object_is_string_enum(o, &enum_arr, &is_enum);
    if (is_enum) {
      f->is_enum = 1;
      if (enum_members_init(&f->enum_members) != CDD_C_SUCCESS)
        return CDD_C_ERROR_MEMORY;
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
      if (json_array_to_enum_members(enum_arr, &f->enum_members) != 0)
        return CDD_C_ERROR_MEMORY;
      {
        JSON_Array *c_names = NULL;
        JSON_Array *e_names = NULL;
        const char *other_name = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_array(o, "x-cdd-enum-c-names", &c_names);
        g_cdd_last_alloc_err =
            cdd_json_object_get_array(o, "x-cdd-enum-names", &e_names);
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(o, "x-cdd-enum-other", &other_name);
        if (c_names) {
          size_t ci, ccnt = 0;
          g_cdd_last_alloc_err = cdd_json_array_get_count(c_names, &ccnt);
          for (ci = 0; ci < ccnt && ci < f->enum_members.size; ++ci) {
            const char *cn = NULL;
            const char *wn = NULL;
            g_cdd_last_alloc_err = cdd_json_array_get_string(c_names, ci, &cn);
            if (e_names)
              g_cdd_last_alloc_err =
                  cdd_json_array_get_string(e_names, ci, &wn);
            if (cn && cn[0]) {
              C_CDD_FREE(f->enum_members.members[ci]);
              f->enum_members.members[ci] = NULL;
              if (c_cdd_strdup(cn, &f->enum_members.members[ci]) !=
                  CDD_C_SUCCESS)
                return CDD_C_ERROR_MEMORY;
            }
            if (wn && wn[0]) {
              if (f->enum_members.serde_names[ci])
                C_CDD_FREE(f->enum_members.serde_names[ci]);
              f->enum_members.serde_names[ci] = NULL;
              if (c_cdd_strdup(wn, &f->enum_members.serde_names[ci]) !=
                  CDD_C_SUCCESS)
                return CDD_C_ERROR_MEMORY;
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
        {
          JSON_Object *ea_obj = NULL;
          JSON_Object *ed_obj = NULL;
          JSON_Array *eh_arr = NULL;
          size_t ei;
          g_cdd_last_alloc_err =
              cdd_json_object_get_object(o, "x-cdd-enum-aliases", &ea_obj);
          g_cdd_last_alloc_err = cdd_json_object_get_object(
              o, "x-cdd-enum-cli-descriptions", &ed_obj);
          g_cdd_last_alloc_err =
              cdd_json_object_get_array(o, "x-cdd-enum-cli-hidden", &eh_arr);
          if (ea_obj) {
            for (ei = 0; ei < f->enum_members.size; ++ei) {
              JSON_Array *va = NULL;
              g_cdd_last_alloc_err = cdd_json_object_get_array(
                  ea_obj, f->enum_members.members[ei], &va);
              if (va) {
                size_t ai, ac = 0;
                g_cdd_last_alloc_err = cdd_json_array_get_count(va, &ac);
                for (ai = 0; ai < ac; ++ai) {
                  const char *va_s = NULL;
                  g_cdd_last_alloc_err =
                      cdd_json_array_get_string(va, ai, &va_s);
                  g_cdd_last_alloc_err =
                      enum_member_add_alias(&f->enum_members, ei, va_s);
                }
              }
            }
          }
          if (ed_obj) {
            for (ei = 0; ei < f->enum_members.size; ++ei) {
              const char *desc = NULL;
              int hid = 0;
              g_cdd_last_alloc_err = cdd_json_object_get_string(
                  ed_obj, f->enum_members.members[ei], &desc);
              if (eh_arr) {
                size_t hi, hc = 0;
                g_cdd_last_alloc_err = cdd_json_array_get_count(eh_arr, &hc);
                for (hi = 0; hi < hc; ++hi) {
                  const char *hs = NULL;
                  g_cdd_last_alloc_err =
                      cdd_json_array_get_string(eh_arr, hi, &hs);
                  if (hs && strcmp(hs, f->enum_members.members[ei]) == 0) {
                    hid = 1;
                    break;
                  }
                }
              }
              if (desc || hid) {
                g_cdd_last_alloc_err =
                    enum_member_set_cli_meta(&f->enum_members, ei, desc, hid);
              }
            }
          }
        }
      }
      return CDD_C_SUCCESS;
    }
  }

  g_cdd_last_alloc_err = cdd_json_object_get_array(o, "allOf", &all_of);
  if (all_of) {
    rc = apply_allof_to_struct_fields(all_of, f, root);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  g_cdd_last_alloc_err = cdd_json_object_get_array(o, "anyOf", &any_of);
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

  g_cdd_last_alloc_err = cdd_json_object_get_array(o, "oneOf", &one_of);
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

  g_cdd_last_alloc_err = cdd_json_object_get_array(o, "required", &required);

  g_cdd_last_alloc_err = cdd_json_object_get_object(o, "properties", &props);
  if (!props)
    return CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_object_get_count(props, &count);
  for (i = 0; i < count; i++) {
    const char *name = NULL;
    JSON_Object *prop = NULL;
    const char *type = NULL;
    JSON_Array *type_arr = NULL;
    const char *ref = NULL;
    const char *bw = NULL;
    char default_buf[128] = {0};
    const char *d_val = NULL;
    struct StructField *field = NULL;
    int field_added = 0;
    char **type_union = NULL;
    size_t n_type_union = 0;
    int has_def_str = 0, has_def_num = 0, has_def_bool = 0;

    g_cdd_last_alloc_err = cdd_json_object_get_name(props, i, &name);
    g_cdd_last_alloc_err = cdd_json_object_get_object(props, name, &prop);
    g_cdd_last_alloc_err = cdd_json_object_get_string(prop, "type", &type);
    g_cdd_last_alloc_err = cdd_json_object_get_string(prop, "$ref", &ref);
    g_cdd_last_alloc_err =
        cdd_json_object_get_string(prop, "x-c-bitwidth", &bw);

    g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
        prop, "default", JSONString, &has_def_str);
    g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
        prop, "default", JSONNumber, &has_def_num);
    g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
        prop, "default", JSONBoolean, &has_def_bool);
    if (has_def_str) {
      const char *s = NULL;
      g_cdd_last_alloc_err = cdd_json_object_get_string(prop, "default", &s);
      CDD_SNPRINTF(default_buf, sizeof(default_buf), "\"%s\"", s);
      d_val = default_buf;
    } else if (has_def_num) {
      double d = 0.0;
      g_cdd_last_alloc_err = cdd_json_object_get_number(prop, "default", &d);
      if (type && strcmp(type, "integer") == 0)
        CDD_SNPRINTF(default_buf, sizeof(default_buf), "%d", (int)d);
      else
        CDD_SNPRINTF(default_buf, sizeof(default_buf), "%f", d);
      d_val = default_buf;
    } else if (has_def_bool) {
      int b = 0;
      g_cdd_last_alloc_err = cdd_json_object_get_boolean(prop, "default", &b);
      CDD_SNPRINTF(default_buf, sizeof(default_buf), "%d", b);
      d_val = default_buf;
    }

    if (!type) {
      g_cdd_last_alloc_err = cdd_json_object_get_array(prop, "type", &type_arr);
      if (type_arr) {
        const char *primary = NULL;
        rc = parse_type_union_array_code2schema(type_arr, &type_union,
                                                &n_type_union, &primary, NULL);
        if (rc != CDD_C_SUCCESS) {
          g_cdd_last_alloc_err =
              free_string_array_code2schema(type_union, n_type_union);
          return rc;
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
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(prop, "items", &items);
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(items, "type", &item_type);
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(items, "$ref", &item_ref);
        if (item_ref && root) {
          int is_enum = 0;
          cdd_c_error_t rc_c2s =
              ref_points_to_string_enum(root, item_ref, &is_enum);
          if (rc_c2s != CDD_C_SUCCESS)
            return rc_c2s;
          if (is_enum) {
            saved_enum_ref = item_ref;
            item_ref = NULL;
            item_type = "string";
          }
        }
        if (!item_ref && !item_type && items) {
          g_cdd_last_alloc_err =
              cdd_json_object_get_array(items, "type", &item_type_arr);
          if (item_type_arr) {
            const char *primary = NULL;
            rc = parse_type_union_array_code2schema(
                item_type_arr, &items_type_union, &n_items_type_union, &primary,
                NULL);
            if (rc != CDD_C_SUCCESS) {
              g_cdd_last_alloc_err =
                  free_string_array_code2schema(type_union, n_type_union);
              g_cdd_last_alloc_err = free_string_array_code2schema(
                  items_type_union, n_items_type_union);
              return rc;
            }
            item_type = primary;
          }
        }
        if (struct_fields_add(f, name, "array", item_ref ? item_ref : item_type,
                              NULL, bw) != 0) {
          g_cdd_last_alloc_err =
              free_string_array_code2schema(type_union, n_type_union);
          g_cdd_last_alloc_err = free_string_array_code2schema(
              items_type_union, n_items_type_union);
          return CDD_C_ERROR_MEMORY;
        }
        field = &f->fields[f->size - 1];
        field_added = 1;
        if (saved_enum_ref) {
          const char *er = NULL;
          field->is_enum_array = 1;
          g_cdd_last_alloc_err = get_type_from_ref(saved_enum_ref, &er);
          CDD_STRCPY(field->enum_array_ref, sizeof(field->enum_array_ref), er);
        }
        field->type_union = type_union;
        field->n_type_union = n_type_union;
        type_union = NULL;
        n_type_union = 0;
        field->items_type_union = items_type_union;
        field->n_items_type_union = n_items_type_union;
        items_type_union = NULL;
        n_items_type_union = 0;
        if (items) {
          if (c2s_collect_schema_extras(items, k_items_skip_keys,
                                        sizeof(k_items_skip_keys) /
                                            sizeof(k_items_skip_keys[0]),
                                        &field->items_extra_json) != 0)
            return CDD_C_ERROR_MEMORY;
        }
      } else {
        if (struct_fields_add(f, name, type, ref, d_val, bw) != 0) {
          g_cdd_last_alloc_err =
              free_string_array_code2schema(type_union, n_type_union);
          return CDD_C_ERROR_MEMORY;
        }
        field = &f->fields[f->size - 1];
        field_added = 1;
        field->type_union = type_union;
        field->n_type_union = n_type_union;
        type_union = NULL;
        n_type_union = 0;
      }
    } else if (ref) {
      int is_enum = 0;
      const char *field_type;
      if (root) {
        cdd_c_error_t rc_c2s = ref_points_to_string_enum(root, ref, &is_enum);
        if (rc_c2s != CDD_C_SUCCESS) {
          g_cdd_last_alloc_err =
              free_string_array_code2schema(type_union, n_type_union);
          return rc_c2s;
        }
      }
      field_type = is_enum ? "enum" : "object";
      if (struct_fields_add(f, name, field_type, ref, NULL, bw) != 0) {
        g_cdd_last_alloc_err =
            free_string_array_code2schema(type_union, n_type_union);
        return CDD_C_ERROR_MEMORY;
      }
      field = &f->fields[f->size - 1];
      field_added = 1;
    }

    if (!field_added) {
      g_cdd_last_alloc_err =
          free_string_array_code2schema(type_union, n_type_union);
      continue;
    }

    {
      int is_req = 0;
      g_cdd_last_alloc_err = required_name_in_list(required, name, &is_req);
      if (is_req)
        field->required = 1;
    }
    if (!type)
      type = field->type;
    if (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0) {
      int hv = 0;
      int bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value_of_type(prop, "minimum", JSONNumber, &hv);
      if (hv) {
        field->has_min = 1;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(prop, "minimum", &field->min_val);
      }
      hv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
          prop, "exclusiveMinimum", JSONNumber, &hv);
      if (hv) {
        field->has_min = 1;
        g_cdd_last_alloc_err = cdd_json_object_get_number(
            prop, "exclusiveMinimum", &field->min_val);
        field->exclusive_min = 1;
      } else {
        hv = 0;
        bv = 0;
        g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
            prop, "exclusiveMinimum", JSONBoolean, &hv);
        if (hv)
          g_cdd_last_alloc_err =
              cdd_json_object_get_boolean(prop, "exclusiveMinimum", &bv);
        if (hv && bv == 1)
          field->exclusive_min = 1;
      }

      hv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value_of_type(prop, "maximum", JSONNumber, &hv);
      if (hv) {
        field->has_max = 1;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(prop, "maximum", &field->max_val);
      }

      hv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
          prop, "exclusiveMaximum", JSONNumber, &hv);
      if (hv) {
        field->has_max = 1;
        g_cdd_last_alloc_err = cdd_json_object_get_number(
            prop, "exclusiveMaximum", &field->max_val);
        field->exclusive_max = 1;
      } else {
        hv = 0;
        bv = 0;
        g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
            prop, "exclusiveMaximum", JSONBoolean, &hv);
        if (hv)
          g_cdd_last_alloc_err =
              cdd_json_object_get_boolean(prop, "exclusiveMaximum", &bv);
        if (hv && bv == 1)
          field->exclusive_max = 1;
      }

    } else if (strcmp(type, "string") == 0) {
      int hv = 0;
      double d_len = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value_of_type(prop, "minLength", JSONNumber, &hv);
      if (hv) {
        field->has_min_len = 1;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(prop, "minLength", &d_len);
        field->min_len = (size_t)d_len;
      }

      hv = 0;
      d_len = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value_of_type(prop, "maxLength", JSONNumber, &hv);
      if (hv) {
        field->has_max_len = 1;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(prop, "maxLength", &d_len);
        field->max_len = (size_t)d_len;
      }

      hv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value_of_type(prop, "pattern", JSONString, &hv);
      if (hv) {
        const char *pat_str = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(prop, "pattern", &pat_str);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        strncpy_s(field->pattern, sizeof(field->pattern), pat_str,
                  sizeof(field->pattern) - 1);
#else
        strncpy(field->pattern, pat_str, sizeof(field->pattern) - 1);
#endif
      }
    } else if (strcmp(type, "array") == 0) {
      int hv = 0;
      double d_items = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value_of_type(prop, "minItems", JSONNumber, &hv);
      if (hv) {
        field->has_min_items = 1;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(prop, "minItems", &d_items);
        field->min_items = (size_t)d_items;
      }

      hv = 0;
      d_items = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value_of_type(prop, "maxItems", JSONNumber, &hv);
      if (hv) {
        field->has_max_items = 1;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(prop, "maxItems", &d_items);
        field->max_items = (size_t)d_items;
      }

      hv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
          prop, "uniqueItems", JSONBoolean, &hv);
      if (hv) {
        g_cdd_last_alloc_err = cdd_json_object_get_boolean(
            prop, "uniqueItems", &field->unique_items);
      }
    }

    {
      const char *desc = NULL;
      const char *fmt = NULL;
      int hv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "description", &desc);
      g_cdd_last_alloc_err = cdd_json_object_get_string(prop, "format", &fmt);
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
      hv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value(prop, "deprecated", &hv);
      if (hv) {
        field->deprecated_set = 1;
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "deprecated", &field->deprecated);
      }
      hv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value(prop, "readOnly", &hv);
      if (hv) {
        field->read_only_set = 1;
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "readOnly", &field->read_only);
      }
      hv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value(prop, "writeOnly", &hv);
      if (hv) {
        field->write_only_set = 1;
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "writeOnly", &field->write_only);
      }
    }

    if (c2s_collect_schema_extras(prop, k_property_skip_keys,
                                  sizeof(k_property_skip_keys) /
                                      sizeof(k_property_skip_keys[0]),
                                  &field->schema_extra_json) != 0)
      return CDD_C_ERROR_MEMORY;

    /* Load field-level Serde reflection extensions and sanitize C identifier */
    {
      const char *c_name_ext = NULL;
      const char *f_ext = NULL;
      enum CddSerdeCaseRule f_rule = CDD_SERDE_CASE_NONE;
      JSON_Array *al_arr = NULL;
      int needs_san = 0;
      int hv = 0, bv = 0;
      double dv = 0.0;

      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-c-name", &c_name_ext);

      if (c_name_ext && c_name_ext[0] != '\0') {
        CDD_STRCPY(field->name, sizeof(field->name), c_name_ext);
      } else {
        g_cdd_last_alloc_err =
            c2s_needs_identifier_sanitization(name, &needs_san);
        if (needs_san) {
          char *sanitized = NULL;
          if (sanitize_identifier(name, &sanitized) == CDD_C_SUCCESS) {
            CDD_STRCPY(field->name, sizeof(field->name), sanitized);
            CDD_STRCPY(field->serde_rename, sizeof(field->serde_rename), name);
            C_CDD_FREE(sanitized);
          }
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

      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-rename", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->serde_rename, sizeof(field->serde_rename), f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-rename-serialize", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->serde_rename_ser, sizeof(field->serde_rename_ser),
                   f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-rename-deserialize", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->serde_rename_de, sizeof(field->serde_rename_de),
                   f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-case", &f_ext);
      if (f_ext && cdd_serde_parse_case_rule(f_ext, &f_rule) == CDD_C_SUCCESS)
        field->serde_case = f_rule;
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-replace-from", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->serde_replace_from, sizeof(field->serde_replace_from),
                   f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-replace-to", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->serde_replace_to, sizeof(field->serde_replace_to),
                   f_ext);

      g_cdd_last_alloc_err =
          cdd_json_object_get_array(prop, "x-cdd-aliases", &al_arr);
      if (al_arr) {
        size_t a_idx, a_cnt = 0;
        g_cdd_last_alloc_err = cdd_json_array_get_count(al_arr, &a_cnt);
        for (a_idx = 0; a_idx < a_cnt; ++a_idx) {
          const char *al_s = NULL;
          g_cdd_last_alloc_err =
              cdd_json_array_get_string(al_arr, a_idx, &al_s);
          if (al_s) {
            cdd_c_error_t rc_al = struct_field_add_alias(field, al_s);
            if (rc_al != CDD_C_SUCCESS)
              return rc_al;
          }
        }
      }

      hv = 0;
      bv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value(prop, "x-cdd-skip", &hv);
      if (hv) {
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "x-cdd-skip", &bv);
        if (bv == 1) {
          field->serde_skip = 1;
          field->serde_skip_serializing = 1;
          field->serde_skip_deserializing = 1;
        }
      }
      hv = 0;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value(prop, "x-cdd-skip-serializing", &hv);
      if (hv) {
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "x-cdd-skip-serializing", &bv);
        if (bv == 1)
          field->serde_skip_serializing = 1;
      }
      hv = 0;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value(prop, "x-cdd-skip-deserializing", &hv);
      if (hv) {
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "x-cdd-skip-deserializing", &bv);
        if (bv == 1)
          field->serde_skip_deserializing = 1;
      }

      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-skip-serializing-if", &f_ext);
      if (f_ext && f_ext[0] != '\0') {
        if (strcmp(f_ext, "null") == 0)
          field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;
        else if (strcmp(f_ext, "empty") == 0)
          field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
        else if (strcmp(f_ext, "zero") == 0)
          field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_ZERO;
        else {
          field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_FUNC;
          CDD_STRCPY(field->serde_skip_if_fn, sizeof(field->serde_skip_if_fn),
                     f_ext);
        }
      }

      hv = 0;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value(prop, "x-cdd-flatten", &hv);
      if (hv) {
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "x-cdd-flatten", &bv);
        if (bv == 1)
          field->serde_flatten = 1;
      }
      hv = 0;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value(prop, "x-cdd-default", &hv);
      if (hv) {
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "x-cdd-default", &bv);
        if (bv == 1)
          field->serde_has_default = 1;
      }
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-default-fn", &f_ext);
      if (f_ext) {
        field->serde_has_default = 1;
        CDD_STRCPY(field->serde_default_fn, sizeof(field->serde_default_fn),
                   f_ext);
      }
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-with", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->serde_with, sizeof(field->serde_with), f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-serialize-with", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->serde_serialize_with,
                   sizeof(field->serde_serialize_with), f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-deserialize-with", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->serde_deserialize_with,
                   sizeof(field->serde_deserialize_with), f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-c-type", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->c_type, sizeof(field->c_type), f_ext);
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-c-nullable-ptr", &bv);
      if (bv == 1)
        field->is_nullable_ptr = 1;
      hv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
          prop, "x-c-fixed-array-len", JSONNumber, &hv);
      if (hv) {
        dv = 0.0;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(prop, "x-c-fixed-array-len", &dv);
        field->fixed_array_len = (size_t)dv;
      }

      /* Load property-level CLI reflection extensions */
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-short", &f_ext);
      if (f_ext && f_ext[0] != '\0')
        field->cli_short = f_ext[0];
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-long", &f_ext);
      if (f_ext && f_ext[0] != '\0') {
        field->cli_has_long = 1;
        CDD_STRCPY(field->cli_long, sizeof(field->cli_long), f_ext);
      }
      hv = 0;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value(prop, "x-cdd-cli-negatable", &hv);
      if (hv) {
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "x-cdd-cli-negatable", &bv);
        if (bv == 1)
          field->cli_negatable = 1;
      }
      hv = 0;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value(prop, "x-cdd-cli-positional", &hv);
      if (hv) {
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "x-cdd-cli-positional", &bv);
        if (bv == 1)
          field->cli_positional = 1;
      }
      hv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
          prop, "x-cdd-cli-index", JSONNumber, &hv);
      if (hv) {
        dv = 0.0;
        field->cli_positional = 1;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(prop, "x-cdd-cli-index", &dv);
        field->cli_index = (int)dv;
      }
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-env", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->cli_env, sizeof(field->cli_env), f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-value-name", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->cli_value_name, sizeof(field->cli_value_name), f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-help", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->cli_help, sizeof(field->cli_help), f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-action", &f_ext);
      if (f_ext) {
        enum CddCliAction act = CDD_CLI_ACTION_AUTO;
        if (cdd_cli_parse_action(f_ext, &act) == CDD_C_SUCCESS)
          field->cli_action = act;
      }
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-delimiter", &f_ext);
      if (f_ext && f_ext[0] != '\0')
        field->cli_value_delimiter = f_ext[0];
      hv = 0;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_has_value(prop, "x-cdd-cli-subcommand", &hv);
      if (hv) {
        g_cdd_last_alloc_err =
            cdd_json_object_get_boolean(prop, "x-cdd-cli-subcommand", &bv);
        if (bv == 1)
          field->cli_subcommand = 1;
      }
      al_arr = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_array(prop, "x-cdd-cli-conflicts-with", &al_arr);
      if (al_arr) {
        size_t a_idx, a_cnt = 0;
        g_cdd_last_alloc_err = cdd_json_array_get_count(al_arr, &a_cnt);
        for (a_idx = 0; a_idx < a_cnt; ++a_idx) {
          const char *al_s = NULL;
          g_cdd_last_alloc_err =
              cdd_json_array_get_string(al_arr, a_idx, &al_s);
          if (al_s) {
            cdd_c_error_t rc_cw = struct_field_add_cli_conflict(field, al_s);
            if (rc_cw != CDD_C_SUCCESS)
              return rc_cw;
          }
        }
      }
      al_arr = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_array(prop, "x-cdd-cli-requires", &al_arr);
      if (al_arr) {
        size_t a_idx, a_cnt = 0;
        g_cdd_last_alloc_err = cdd_json_array_get_count(al_arr, &a_cnt);
        for (a_idx = 0; a_idx < a_cnt; ++a_idx) {
          const char *al_s = NULL;
          g_cdd_last_alloc_err =
              cdd_json_array_get_string(al_arr, a_idx, &al_s);
          if (al_s) {
            cdd_c_error_t rc_rq = struct_field_add_cli_requires(field, al_s);
            if (rc_rq != CDD_C_SUCCESS)
              return rc_rq;
          }
        }
      }
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-short-aliases", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->cli_short_aliases, sizeof(field->cli_short_aliases),
                   f_ext);
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-last", &bv);
      if (bv == 1)
        field->cli_last = 1;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-trailing-var-arg", &bv);
      if (bv == 1)
        field->cli_trailing_var_arg = 1;
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-long-help", &f_ext);
      if (f_ext)
        g_cdd_last_alloc_err = c_cdd_strdup(f_ext, &field->cli_long_help);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-help-heading", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->cli_help_heading, sizeof(field->cli_help_heading),
                   f_ext);
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-hide-env", &bv);
      if (bv == 1)
        field->cli_hide_env = 1;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-hide-env-values", &bv);
      if (bv == 1)
        field->cli_hide_env_values = 1;
      f_ext = NULL;
      g_cdd_last_alloc_err = cdd_json_object_get_string(
          prop, "x-cdd-cli-default-missing-val", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->cli_default_missing_val,
                   sizeof(field->cli_default_missing_val), f_ext);
      bv = 0;
      g_cdd_last_alloc_err = cdd_json_object_get_boolean(
          prop, "x-cdd-cli-hide-default-value", &bv);
      if (bv == 1)
        field->cli_hide_default_value = 1;
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-value-hint", &f_ext);
      if (f_ext)
        g_cdd_last_alloc_err =
            cdd_cli_parse_value_hint(f_ext, &field->cli_value_hint);
      hv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
          prop, "x-cdd-cli-num-args-min", JSONNumber, &hv);
      if (hv) {
        dv = 0.0;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(prop, "x-cdd-cli-num-args-min", &dv);
        field->cli_num_args_min = (int)dv;
      }
      hv = 0;
      g_cdd_last_alloc_err = cdd_json_object_has_value_of_type(
          prop, "x-cdd-cli-num-args-max", JSONNumber, &hv);
      if (hv) {
        dv = 0.0;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(prop, "x-cdd-cli-num-args-max", &dv);
        field->cli_num_args_max = (int)dv;
      }
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-require-equals", &bv);
      if (bv == 1)
        field->cli_require_equals = 1;
      bv = 0;
      g_cdd_last_alloc_err = cdd_json_object_get_boolean(
          prop, "x-cdd-cli-allow-hyphen-values", &bv);
      if (bv == 1)
        field->cli_allow_hyphen_values = 1;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-global", &bv);
      if (bv == 1)
        field->cli_global = 1;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-hidden", &bv);
      if (bv == 1)
        field->cli_hidden = 1;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-exclusive", &bv);
      if (bv == 1)
        field->cli_exclusive = 1;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-skip", &bv);
      if (bv == 1) {
        field->cli_skip = 1;
        field->cli_skip_to_cli = 1;
        field->cli_skip_from_cli = 1;
      }
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-skip-to", &bv);
      if (bv == 1)
        field->cli_skip_to_cli = 1;
      bv = 0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_boolean(prop, "x-cdd-cli-skip-from", &bv);
      if (bv == 1)
        field->cli_skip_from_cli = 1;
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-value-parser", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->cli_value_parser, sizeof(field->cli_value_parser),
                   f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(prop, "x-cdd-cli-group", &f_ext);
      if (f_ext)
        CDD_STRCPY(field->cli_group, sizeof(field->cli_group), f_ext);
      f_ext = NULL;
      g_cdd_last_alloc_err = cdd_json_object_get_string(
          prop, "x-cdd-cli-required-if-field", &f_ext);
      if (f_ext) {
        const char *f_val = NULL;
        g_cdd_last_alloc_err = cdd_json_object_get_string(
            prop, "x-cdd-cli-required-if-val", &f_val);
        CDD_STRCPY(field->cli_required_if_field,
                   sizeof(field->cli_required_if_field), f_ext);
        CDD_STRCPY(field->cli_required_if_val,
                   sizeof(field->cli_required_if_val), f_val);
      }
      al_arr = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_array(prop, "x-cdd-cli-required-unless", &al_arr);
      if (al_arr) {
        size_t a_idx, a_cnt = 0;
        g_cdd_last_alloc_err = cdd_json_array_get_count(al_arr, &a_cnt);
        for (a_idx = 0; a_idx < a_cnt; ++a_idx) {
          const char *ru_s = NULL;
          g_cdd_last_alloc_err =
              cdd_json_array_get_string(al_arr, a_idx, &ru_s);
          g_cdd_last_alloc_err =
              struct_field_add_cli_required_unless(field, ru_s);
        }
      }
      if (field->serde_flatten && root) {
        const JSON_Object *flat_target = NULL;
        g_cdd_last_alloc_err =
            resolve_schema_ref_object(root, field->ref, &flat_target);
        if (flat_target) {
          JSON_Object *flat_props = NULL;
          size_t fp_i, fp_cnt = 0;
          g_cdd_last_alloc_err = cdd_json_object_get_object(
              flat_target, "properties", &flat_props);
          g_cdd_last_alloc_err = cdd_json_object_get_count(flat_props, &fp_cnt);
          for (fp_i = 0; fp_i < fp_cnt; ++fp_i) {
            const char *fp_name = NULL;
            g_cdd_last_alloc_err =
                cdd_json_object_get_name(flat_props, fp_i, &fp_name);
            g_cdd_last_alloc_err = struct_field_add_alias(field, fp_name);
          }
        }
      }
    }
  }

  /* Resolve `n_<array_field>` counter name collisions with scalar fields */
  {
    size_t a_idx, s_idx;
    for (a_idx = 0; a_idx < f->size; ++a_idx) {
      char counter_nm[68];
      if (strcmp(f->fields[a_idx].type, "array") != 0)
        continue;
      CDD_SNPRINTF(counter_nm, sizeof(counter_nm), "n_%s",
                   f->fields[a_idx].name);
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
  }

  return CDD_C_SUCCESS;
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
