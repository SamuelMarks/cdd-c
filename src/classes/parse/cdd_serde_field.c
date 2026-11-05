/**
 * @file cdd_serde_field.c
 * @brief Field-level (`StructField`) and enum-variant-level (`EnumMembers`)
 * C89 reflection comment directive parsers.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "classes/parse/cdd_serde_internal.h"
/* clang-format on */

/**
 * @brief Appends a short option alias character to `field->cli_short_aliases`.
 *
 * @param[in,out] field Target StructField.
 * @param[in] ch Short flag character to append.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_add_short_alias_char(struct StructField *field,
                                                char ch) {
  size_t len = strlen(field->cli_short_aliases);
  size_t i;
  for (i = 0; i < len; ++i) {
    if (field->cli_short_aliases[i] == ch)
      return CDD_C_SUCCESS;
  }
  if (len + 1 < sizeof(field->cli_short_aliases)) {
    field->cli_short_aliases[len] = ch;
    field->cli_short_aliases[len + 1] = '\0';
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Infers a default short option character from a field name.
 *
 * @param[in] field_name C field identifier.
 * @param[out] out_ch Output pointer receiving the inferred character.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t serde_infer_short_char(const char *field_name,
                                            char *out_ch) {
  const char *p = field_name;
  while (*p == '_')
    p++;
  *out_ch = (*p != '\0') ? *p : 'a';
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses field-level `@cdd` or `@serde` directives
 * into a `StructField` descriptor.
 */
cdd_c_error_t cdd_serde_parse_field_comment(const char *comment_text,
                                            struct StructField *field) {
  const char *scan;
  const char *start = NULL;
  const char *end = NULL;
  const char *next = NULL;
  char dox_brief[256];
  char dox_details[512];
  cdd_c_error_t rc;

  if (!field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!comment_text || comment_text[0] == '\0')
    return CDD_C_SUCCESS;

  rc = cdd_serde_extract_doxygen_fallbacks(
      comment_text, dox_brief, sizeof(dox_brief), dox_details,
      sizeof(dox_details), NULL, 0, NULL, 0);
  if (dox_brief[0] != '\0') {
    if (field->description[0] == '\0') {
      rc = cdd_serde_copy_str(field->description, sizeof(field->description),
                              dox_brief);
    }
    if (field->cli_help[0] == '\0') {
      rc = cdd_serde_copy_str(field->cli_help, sizeof(field->cli_help),
                              dox_brief);
    }
  }
  if (dox_details[0] != '\0' && !field->cli_long_help) {
    rc = c_cdd_strdup(dox_details, &field->cli_long_help);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  scan = comment_text;
  while (1) {
    const char *p;
    enum CddDirectiveScope outer_scope = CDD_SCOPE_GENERAL;
    enum CddDirectiveScope cur_scope;
    rc = cdd_serde_find_directive_ex(scan, &start, &end, &next, &outer_scope);
    if (!start)
      break;

    cur_scope = outer_scope;
    p = start;
    while (p < end) {
      char key[64];
      char val[1024];
      enum CddSerdeCaseRule rule = CDD_SERDE_CASE_NONE;
      enum CddDirectiveScope sub_scope = CDD_SCOPE_GENERAL;

      rc = cdd_serde_skip_delims(&p, end);
      while (p < end && *p == ')') {
        p++;
        cur_scope = outer_scope;
        rc = cdd_serde_skip_delims(&p, end);
      }

      rc = cdd_serde_next_key(&p, end, key, sizeof(key));
      if (key[0] == '\0') {
        if (p < end)
          p++;
        continue;
      }

      while (p < end && isspace((unsigned char)*p))
        p++;

      rc = cdd_serde_match_scope_key(key, &sub_scope);
      if (sub_scope != CDD_SCOPE_GENERAL) {
        cur_scope = sub_scope;
        if (*p == '(')
          p++;
        continue;
      }

      if (p < end && (*p == '=' || *p == ':' || *p == '(')) {
        int bval = 1;
        if (*p != '(')
          p++;
        if (strcmp(key, "replace") == 0 || strcmp(key, "rewrite") == 0 ||
            strcmp(key, "replace_all") == 0) {
          rc = cdd_serde_parse_replace_tuple(&p, end, field->serde_replace_from,
                                             sizeof(field->serde_replace_from),
                                             field->serde_replace_to,
                                             sizeof(field->serde_replace_to));
          continue;
        }
        if (strcmp(key, "required_if_eq") == 0 ||
            strcmp(key, "required_if") == 0) {
          rc = cdd_serde_parse_replace_tuple(
              &p, end, field->cli_required_if_field,
              sizeof(field->cli_required_if_field), field->cli_required_if_val,
              sizeof(field->cli_required_if_val));
          continue;
        }

        rc = cdd_serde_parse_value(&p, end, val, sizeof(val));

        if (strcmp(key, "rename") == 0) {
          if (cur_scope == CDD_SCOPE_TO_JSON) {
            rc = cdd_serde_copy_str(field->serde_rename_ser,
                                    sizeof(field->serde_rename_ser), val);
          } else if (cur_scope == CDD_SCOPE_FROM_JSON) {
            rc = cdd_serde_copy_str(field->serde_rename_de,
                                    sizeof(field->serde_rename_de), val);
          } else if (cur_scope >= CDD_SCOPE_CLI) {
            field->cli_has_long = 1;
            rc = cdd_serde_copy_str(field->cli_long, sizeof(field->cli_long),
                                    val);
          } else {
            rc = cdd_serde_copy_str(field->serde_rename,
                                    sizeof(field->serde_rename), val);
          }
        } else if (strcmp(key, "rename_serialize") == 0 ||
                   strcmp(key, "serialize") == 0) {
          rc = cdd_serde_copy_str(field->serde_rename_ser,
                                  sizeof(field->serde_rename_ser), val);
        } else if (strcmp(key, "rename_deserialize") == 0 ||
                   strcmp(key, "deserialize") == 0) {
          rc = cdd_serde_copy_str(field->serde_rename_de,
                                  sizeof(field->serde_rename_de), val);
        } else if (strcmp(key, "case") == 0 || strcmp(key, "rename_all") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            if (cur_scope >= CDD_SCOPE_CLI && field->name[0] != '\0') {
              field->cli_has_long = 1;
              rc = cdd_serde_transform_case(field->name, rule, field->cli_long,
                                            sizeof(field->cli_long));
            } else {
              field->serde_case = rule;
            }
          }
        } else if (strcmp(key, "alias") == 0 ||
                   strcmp(key, "visible_alias") == 0) {
          rc = struct_field_add_alias(field, val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "short") == 0) {
          if (strcmp(val, "false") == 0 || strcmp(val, "0") == 0) {
            field->cli_short = '\0';
          } else if (strcmp(val, "true") == 0 || val[0] == '\0') {
            rc = serde_infer_short_char(field->name, &field->cli_short);
          } else {
            field->cli_short = val[0];
          }
        } else if (strcmp(key, "short_alias") == 0 ||
                   strcmp(key, "visible_short_alias") == 0) {
          if (val[0] != '\0') {
            rc = serde_add_short_alias_char(field, val[0]);
          }
        } else if (strcmp(key, "long") == 0) {
          if (strcmp(val, "false") == 0 || strcmp(val, "0") == 0) {
            field->cli_has_long = -1;
            field->cli_long[0] = '\0';
          } else if (strcmp(val, "true") == 0 || val[0] == '\0') {
            field->cli_has_long = 1;
            if (field->cli_long[0] == '\0' && field->name[0] != '\0') {
              rc = cdd_serde_transform_case(
                  field->name, CDD_SERDE_CASE_KEBABCASE, field->cli_long,
                  sizeof(field->cli_long));
            }
          } else {
            field->cli_has_long = 1;
            rc = cdd_serde_copy_str(field->cli_long, sizeof(field->cli_long),
                                    val);
          }
        } else if (strcmp(key, "negatable") == 0 ||
                   strcmp(key, "negate_with_no") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_negatable = bval;
        } else if (strcmp(key, "positional") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_positional = bval;
        } else if (strcmp(key, "index") == 0) {
          field->cli_positional = 1;
          field->cli_index = atoi(val);
        } else if (strcmp(key, "last") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_last = bval;
          if (bval)
            field->cli_positional = 1;
        } else if (strcmp(key, "trailing_var_arg") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_trailing_var_arg = bval;
          if (bval)
            field->cli_positional = 1;
        } else if (strcmp(key, "value_name") == 0) {
          rc = cdd_serde_copy_str(field->cli_value_name,
                                  sizeof(field->cli_value_name), val);
        } else if (strcmp(key, "help") == 0) {
          rc =
              cdd_serde_copy_str(field->cli_help, sizeof(field->cli_help), val);
          if (field->description[0] == '\0') {
            rc = cdd_serde_copy_str(field->description,
                                    sizeof(field->description), val);
          }
        } else if (strcmp(key, "long_help") == 0) {
          if (field->cli_long_help) {
            C_CDD_FREE(field->cli_long_help);
            field->cli_long_help = NULL;
          }
          rc = c_cdd_strdup(val, &field->cli_long_help);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "help_heading") == 0) {
          rc = cdd_serde_copy_str(field->cli_help_heading,
                                  sizeof(field->cli_help_heading), val);
        } else if (strcmp(key, "env") == 0) {
          if (strcmp(val, "true") == 0 || val[0] == '\0') {
            if (field->name[0] != '\0') {
              rc = cdd_serde_transform_case(
                  field->name, CDD_SERDE_CASE_SCREAMING_SNAKECASE,
                  field->cli_env, sizeof(field->cli_env));
            } else {
              rc = cdd_serde_copy_str(field->cli_env, sizeof(field->cli_env),
                                      "1");
            }
          } else if (strcmp(val, "false") == 0 || strcmp(val, "0") == 0) {
            field->cli_env[0] = '\0';
          } else {
            rc =
                cdd_serde_copy_str(field->cli_env, sizeof(field->cli_env), val);
          }
        } else if (strcmp(key, "hide_env") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_hide_env = bval;
        } else if (strcmp(key, "hide_env_values") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_hide_env_values = bval;
        } else if (strcmp(key, "default_missing_value") == 0) {
          rc = cdd_serde_copy_str(field->cli_default_missing_val,
                                  sizeof(field->cli_default_missing_val), val);
        } else if (strcmp(key, "hide_default_value") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_hide_default_value = bval;
        } else if (strcmp(key, "action") == 0) {
          enum CddCliAction act = CDD_CLI_ACTION_AUTO;
          if (cdd_cli_parse_action(val, &act) == CDD_C_SUCCESS) {
            field->cli_action = act;
          }
        } else if (strcmp(key, "value_hint") == 0) {
          enum CddCliValueHint hint = CDD_CLI_HINT_NONE;
          if (cdd_cli_parse_value_hint(val, &hint) == CDD_C_SUCCESS) {
            field->cli_value_hint = hint;
          }
        } else if (strcmp(key, "value_delimiter") == 0 ||
                   strcmp(key, "delimiter") == 0) {
          field->cli_value_delimiter = (val[0] != '\0') ? val[0] : ',';
        } else if (strcmp(key, "num_args") == 0) {
          rc = cdd_serde_parse_num_args(val, &field->cli_num_args_min,
                                        &field->cli_num_args_max);
        } else if (strcmp(key, "require_equals") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_require_equals = bval;
        } else if (strcmp(key, "allow_hyphen_values") == 0 ||
                   strcmp(key, "allow_negative_numbers") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_allow_hyphen_values = bval;
        } else if (strcmp(key, "global") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_global = bval;
        } else if (strcmp(key, "hide") == 0 || strcmp(key, "hidden") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_hidden = bval;
        } else if (strcmp(key, "exclusive") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_exclusive = bval;
        } else if (strcmp(key, "subcommand") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_subcommand = bval;
        } else if (strcmp(key, "flatten") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->serde_flatten = bval;
        } else if (strcmp(key, "required") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->required = bval;
        } else if (strcmp(key, "value_parser") == 0) {
          rc = cdd_serde_copy_str(field->cli_value_parser,
                                  sizeof(field->cli_value_parser), val);
        } else if (strcmp(key, "group") == 0) {
          rc = cdd_serde_copy_str(field->cli_group, sizeof(field->cli_group),
                                  val);
        } else if (strcmp(key, "conflicts_with") == 0) {
          rc = struct_field_add_cli_conflict(field, val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "requires") == 0) {
          rc = struct_field_add_cli_requires(field, val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "required_unless_present") == 0 ||
                   strcmp(key, "required_unless") == 0) {
          rc = struct_field_add_cli_required_unless(field, val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "skip") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          if (cur_scope == CDD_SCOPE_TO_JSON) {
            field->serde_skip_serializing = bval;
          } else if (cur_scope == CDD_SCOPE_FROM_JSON) {
            field->serde_skip_deserializing = bval;
          } else if (cur_scope == CDD_SCOPE_CLI) {
            field->cli_skip = bval;
            field->cli_skip_to_cli = bval;
            field->cli_skip_from_cli = bval;
          } else if (cur_scope == CDD_SCOPE_TO_CLI) {
            field->cli_skip_to_cli = bval;
          } else if (cur_scope == CDD_SCOPE_FROM_CLI) {
            field->cli_skip_from_cli = bval;
          } else {
            field->serde_skip = bval;
            field->serde_skip_serializing = bval;
            field->serde_skip_deserializing = bval;
          }
        } else if (strcmp(key, "cli_skip") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_skip = bval;
          field->cli_skip_to_cli = bval;
          field->cli_skip_from_cli = bval;
        } else if (strcmp(key, "skip_to_cli") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_skip_to_cli = bval;
        } else if (strcmp(key, "skip_from_cli") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->cli_skip_from_cli = bval;
        } else if (strcmp(key, "skip_serializing_if") == 0 ||
                   strcmp(key, "skip_if") == 0) {
          int eq_null = 0, eq_nullptr = 0, eq_empty = 0, eq_zero = 0;
          rc = cdd_serde_str_ieq(val, "null", &eq_null);
          rc = cdd_serde_str_ieq(val, "nullptr", &eq_nullptr);
          rc = cdd_serde_str_ieq(val, "empty", &eq_empty);
          if (!eq_empty) {
            rc = cdd_serde_str_ieq(val, "is_empty", &eq_empty);
          }
          rc = cdd_serde_str_ieq(val, "zero", &eq_zero);
          if (!eq_zero) {
            rc = cdd_serde_str_ieq(val, "is_zero", &eq_zero);
          }

          if (eq_null || eq_nullptr) {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_NULL;
          } else if (eq_empty) {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_EMPTY;
          } else if (eq_zero) {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_ZERO;
          } else if (val[0] != '\0') {
            field->serde_skip_if_kind = CDD_SERDE_SKIP_IF_FUNC;
            rc = cdd_serde_copy_str(field->serde_skip_if_fn,
                                    sizeof(field->serde_skip_if_fn), val);
          }
        } else if (strcmp(key, "default") == 0 ||
                   strcmp(key, "default_value") == 0 ||
                   strcmp(key, "default_value_t") == 0) {
          field->serde_has_default = 1;
          rc = cdd_serde_copy_str(field->default_val,
                                  sizeof(field->default_val), val);
        } else if (strcmp(key, "default_fn") == 0) {
          field->serde_has_default = 1;
          rc = cdd_serde_copy_str(field->serde_default_fn,
                                  sizeof(field->serde_default_fn), val);
        } else if (strcmp(key, "with") == 0) {
          if (cur_scope == CDD_SCOPE_TO_JSON) {
            rc = cdd_serde_copy_str(field->serde_serialize_with,
                                    sizeof(field->serde_serialize_with), val);
          } else if (cur_scope == CDD_SCOPE_FROM_JSON) {
            rc = cdd_serde_copy_str(field->serde_deserialize_with,
                                    sizeof(field->serde_deserialize_with), val);
          } else if (cur_scope >= CDD_SCOPE_CLI) {
            rc = cdd_serde_copy_str(field->cli_value_parser,
                                    sizeof(field->cli_value_parser), val);
          } else {
            rc = cdd_serde_copy_str(field->serde_with,
                                    sizeof(field->serde_with), val);
          }
        } else if (strcmp(key, "serialize_with") == 0) {
          rc = cdd_serde_copy_str(field->serde_serialize_with,
                                  sizeof(field->serde_serialize_with), val);
        } else if (strcmp(key, "deserialize_with") == 0) {
          rc = cdd_serde_copy_str(field->serde_deserialize_with,
                                  sizeof(field->serde_deserialize_with), val);
        } else if (strcmp(key, "c_type") == 0) {
          rc = cdd_serde_copy_str(field->c_type, sizeof(field->c_type), val);
        } else if (strcmp(key, "nullable_ptr") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          field->is_nullable_ptr = bval;
        } else if (strcmp(key, "fixed_array_len") == 0) {
          field->fixed_array_len = (size_t)atoi(val);
        }
      } else {
        if (strcmp(key, "skip") == 0) {
          if (cur_scope == CDD_SCOPE_TO_JSON) {
            field->serde_skip_serializing = 1;
          } else if (cur_scope == CDD_SCOPE_FROM_JSON) {
            field->serde_skip_deserializing = 1;
          } else if (cur_scope == CDD_SCOPE_CLI) {
            field->cli_skip = 1;
            field->cli_skip_to_cli = 1;
            field->cli_skip_from_cli = 1;
          } else if (cur_scope == CDD_SCOPE_TO_CLI) {
            field->cli_skip_to_cli = 1;
          } else if (cur_scope == CDD_SCOPE_FROM_CLI) {
            field->cli_skip_from_cli = 1;
          } else {
            field->serde_skip = 1;
            field->serde_skip_serializing = 1;
            field->serde_skip_deserializing = 1;
          }
        } else if (strcmp(key, "cli_skip") == 0) {
          field->cli_skip = 1;
          field->cli_skip_to_cli = 1;
          field->cli_skip_from_cli = 1;
        } else if (strcmp(key, "skip_to_cli") == 0) {
          field->cli_skip_to_cli = 1;
        } else if (strcmp(key, "skip_from_cli") == 0) {
          field->cli_skip_from_cli = 1;
        } else if (strcmp(key, "nullable_ptr") == 0) {
          field->is_nullable_ptr = 1;
        } else if (strcmp(key, "skip_serializing") == 0) {
          field->serde_skip_serializing = 1;
        } else if (strcmp(key, "skip_deserializing") == 0) {
          field->serde_skip_deserializing = 1;
        } else if (strcmp(key, "flatten") == 0) {
          field->serde_flatten = 1;
        } else if (strcmp(key, "default") == 0 ||
                   strcmp(key, "default_value") == 0 ||
                   strcmp(key, "default_value_t") == 0) {
          field->serde_has_default = 1;
        } else if (strcmp(key, "short") == 0) {
          rc = serde_infer_short_char(field->name, &field->cli_short);
        } else if (strcmp(key, "long") == 0) {
          field->cli_has_long = 1;
          if (field->cli_long[0] == '\0' && field->name[0] != '\0') {
            rc = cdd_serde_transform_case(field->name, CDD_SERDE_CASE_KEBABCASE,
                                          field->cli_long,
                                          sizeof(field->cli_long));
          }
        } else if (strcmp(key, "negatable") == 0 ||
                   strcmp(key, "negate_with_no") == 0) {
          field->cli_negatable = 1;
        } else if (strcmp(key, "positional") == 0) {
          field->cli_positional = 1;
        } else if (strcmp(key, "last") == 0) {
          field->cli_last = 1;
          field->cli_positional = 1;
        } else if (strcmp(key, "trailing_var_arg") == 0) {
          field->cli_trailing_var_arg = 1;
          field->cli_positional = 1;
        } else if (strcmp(key, "env") == 0) {
          if (field->name[0] != '\0') {
            rc = cdd_serde_transform_case(
                field->name, CDD_SERDE_CASE_SCREAMING_SNAKECASE, field->cli_env,
                sizeof(field->cli_env));
          } else {
            rc =
                cdd_serde_copy_str(field->cli_env, sizeof(field->cli_env), "1");
          }
        } else if (strcmp(key, "hide_env") == 0) {
          field->cli_hide_env = 1;
        } else if (strcmp(key, "hide_env_values") == 0) {
          field->cli_hide_env_values = 1;
        } else if (strcmp(key, "hide_default_value") == 0) {
          field->cli_hide_default_value = 1;
        } else if (strcmp(key, "value_delimiter") == 0 ||
                   strcmp(key, "delimiter") == 0) {
          field->cli_value_delimiter = ',';
        } else if (strcmp(key, "require_equals") == 0) {
          field->cli_require_equals = 1;
        } else if (strcmp(key, "allow_hyphen_values") == 0 ||
                   strcmp(key, "allow_negative_numbers") == 0) {
          field->cli_allow_hyphen_values = 1;
        } else if (strcmp(key, "global") == 0) {
          field->cli_global = 1;
        } else if (strcmp(key, "hide") == 0 || strcmp(key, "hidden") == 0) {
          field->cli_hidden = 1;
        } else if (strcmp(key, "exclusive") == 0) {
          field->cli_exclusive = 1;
        } else if (strcmp(key, "subcommand") == 0) {
          field->cli_subcommand = 1;
        } else if (strcmp(key, "required") == 0) {
          field->required = 1;
        } else if (strcmp(key, "count") == 0) {
          field->cli_action = CDD_CLI_ACTION_COUNT;
        } else if (cdd_serde_parse_case_rule(key, &rule) == CDD_C_SUCCESS &&
                   rule != CDD_SERDE_CASE_NONE) {
          if (cur_scope >= CDD_SCOPE_CLI && field->name[0] != '\0') {
            field->cli_has_long = 1;
            rc = cdd_serde_transform_case(field->name, rule, field->cli_long,
                                          sizeof(field->cli_long));
          } else {
            field->serde_case = rule;
          }
        }
      }
    }

    scan = next;
  }

  return rc;
}

/**
 * @brief Frees temporary heap allocations on a stack `struct StructField`.
 *
 * @param[in,out] f Stack-allocated StructField to clean up.
 * @param[in] status Error code to return after cleanup.
 * @return `status`.
 */
static cdd_c_error_t serde_cleanup_tmp_field(struct StructField *f,
                                             cdd_c_error_t status) {
  size_t i;
  if (f->serde_aliases) {
    for (i = 0; i < f->n_serde_aliases; ++i) {
      C_CDD_FREE(f->serde_aliases[i]);
    }
    C_CDD_FREE(f->serde_aliases);
    f->serde_aliases = NULL;
    f->n_serde_aliases = 0;
  }
  if (f->cli_long_help) {
    C_CDD_FREE(f->cli_long_help);
    f->cli_long_help = NULL;
  }
  if (f->cli_conflicts_with) {
    for (i = 0; i < f->n_cli_conflicts_with; ++i) {
      C_CDD_FREE(f->cli_conflicts_with[i]);
    }
    C_CDD_FREE(f->cli_conflicts_with);
    f->cli_conflicts_with = NULL;
    f->n_cli_conflicts_with = 0;
  }
  if (f->cli_requires) {
    for (i = 0; i < f->n_cli_requires; ++i) {
      C_CDD_FREE(f->cli_requires[i]);
    }
    C_CDD_FREE(f->cli_requires);
    f->cli_requires = NULL;
    f->n_cli_requires = 0;
  }
  if (f->cli_required_unless) {
    for (i = 0; i < f->n_cli_required_unless; ++i) {
      C_CDD_FREE(f->cli_required_unless[i]);
    }
    C_CDD_FREE(f->cli_required_unless);
    f->cli_required_unless = NULL;
    f->n_cli_required_unless = 0;
  }
  return status;
}

/*
 * @brief Extended parser for enum-variant-level `@cdd` or `@serde` directives
 * including `alias` and `other`.
 */
cdd_c_error_t cdd_serde_parse_enum_variant_comment_ex(
    const char *comment_text, char *out_rename, size_t rename_sz, int *out_skip,
    enum CddSerdeCaseRule *out_case, char *out_rep_from, size_t rep_from_sz,
    char *out_rep_to, size_t rep_to_sz, int *out_other,
    struct EnumMembers *em_for_aliases, size_t member_idx) {
  struct StructField tmp;
  size_t i;
  cdd_c_error_t rc;

  if (!out_rename || rename_sz == 0 || !out_skip || !out_case ||
      !out_rep_from || rep_from_sz == 0 || !out_rep_to || rep_to_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  out_rename[0] = '\0';
  *out_skip = 0;
  *out_case = CDD_SERDE_CASE_NONE;
  out_rep_from[0] = '\0';
  out_rep_to[0] = '\0';
  if (out_other)
    *out_other = 0;

  if (!comment_text || comment_text[0] == '\0')
    return CDD_C_SUCCESS;

  /* Scan for `other` flag in directive block */
  if (out_other) {
    const char *scan = comment_text;
    const char *start = NULL, *end = NULL, *next = NULL;
    while (1) {
      const char *p;
      rc = cdd_serde_find_directive_ex(scan, &start, &end, &next, NULL);
      if (!start)
        break;
      p = start;
      while (p < end) {
        char key[64];
        rc = cdd_serde_next_key(&p, end, key, sizeof(key));
        if (key[0] == '\0') {
          if (p < end)
            p++;
          continue;
        }
        if (strcmp(key, "other") == 0) {
          *out_other = 1;
        }
      }
      scan = next;
    }
  }

  memset(&tmp, 0, sizeof(tmp));
  tmp.cli_index = -1;
  rc = cdd_serde_parse_field_comment(comment_text, &tmp);
  if (rc != CDD_C_SUCCESS) {
    return serde_cleanup_tmp_field(&tmp, rc);
  }

  if (em_for_aliases) {
    if (tmp.serde_aliases) {
      for (i = 0; i < tmp.n_serde_aliases; ++i) {
        rc = enum_member_add_alias(em_for_aliases, member_idx,
                                   tmp.serde_aliases[i]);
        if (rc != CDD_C_SUCCESS) {
          return serde_cleanup_tmp_field(&tmp, rc);
        }
      }
    }
    if (tmp.cli_help[0] != '\0' || tmp.cli_hidden) {
      rc = enum_member_set_cli_meta(em_for_aliases, member_idx,
                                    tmp.cli_help[0] ? tmp.cli_help : NULL,
                                    tmp.cli_hidden);
      if (rc != CDD_C_SUCCESS) {
        return serde_cleanup_tmp_field(&tmp, rc);
      }
    }
  }

  rc = serde_cleanup_tmp_field(&tmp, CDD_C_SUCCESS);

  if (tmp.serde_rename[0] != '\0') {
    rc = cdd_serde_copy_str(out_rename, rename_sz, tmp.serde_rename);
  }
  if (tmp.serde_skip || tmp.serde_skip_serializing ||
      tmp.serde_skip_deserializing) {
    *out_skip = 1;
  }
  *out_case = tmp.serde_case;
  if (tmp.serde_replace_from[0] != '\0') {
    rc = cdd_serde_copy_str(out_rep_from, rep_from_sz, tmp.serde_replace_from);
    rc = cdd_serde_copy_str(out_rep_to, rep_to_sz, tmp.serde_replace_to);
  }
  return rc;
}

/*
 * @brief Parses enum-variant-level `@cdd` or `@serde`
 * directives for a single enum constant.
 */
cdd_c_error_t cdd_serde_parse_enum_variant_comment(
    const char *comment_text, char *out_rename, size_t rename_sz, int *out_skip,
    enum CddSerdeCaseRule *out_case, char *out_rep_from, size_t rep_from_sz,
    char *out_rep_to, size_t rep_to_sz) {
  return cdd_serde_parse_enum_variant_comment_ex(
      comment_text, out_rename, rename_sz, out_skip, out_case, out_rep_from,
      rep_from_sz, out_rep_to, rep_to_sz, NULL, NULL, 0);
}
