/**
 * @file cdd_serde_container.c
 * @brief Container-level (`struct`, `union`, and `enum`) C89 reflection comment
 * directive parsers.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "classes/parse/cdd_serde_internal.h"
/* clang-format on */

/*
 * @brief Parses container-level `@cdd` or `@serde`
 * directives into a `StructFields` container.
 */
cdd_c_error_t cdd_serde_parse_container_comment(const char *comment_text,
                                                struct StructFields *sf) {
  const char *scan;
  const char *start = NULL;
  const char *end = NULL;
  const char *next = NULL;
  char dox_brief[256];
  char dox_details[512];
  char dox_version[64];
  char dox_author[128];
  cdd_c_error_t rc;

  if (!sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!comment_text || comment_text[0] == '\0')
    return CDD_C_SUCCESS;

  rc = cdd_serde_extract_doxygen_fallbacks(
      comment_text, dox_brief, sizeof(dox_brief), dox_details,
      sizeof(dox_details), dox_version, sizeof(dox_version), dox_author,
      sizeof(dox_author));
  if (dox_brief[0] != '\0' && sf->cli_about[0] == '\0') {
    rc = cdd_serde_copy_str(sf->cli_about, sizeof(sf->cli_about), dox_brief);
  }
  if (dox_details[0] != '\0' && !sf->cli_long_about) {
    rc = c_cdd_strdup(dox_details, &sf->cli_long_about);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  if (dox_version[0] != '\0' && sf->cli_version[0] == '\0') {
    rc = cdd_serde_copy_str(sf->cli_version, sizeof(sf->cli_version),
                            dox_version);
  }
  if (dox_author[0] != '\0' && sf->cli_author[0] == '\0') {
    rc = cdd_serde_copy_str(sf->cli_author, sizeof(sf->cli_author), dox_author);
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
    if (cur_scope >= CDD_SCOPE_CLI) {
      sf->is_cli_command = 1;
    }

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
        if (cur_scope >= CDD_SCOPE_CLI)
          sf->is_cli_command = 1;
        if (*p == '(')
          p++;
        continue;
      }

      if (p < end && (*p == '=' || *p == ':' || *p == '(')) {
        int bval = 1;
        if (*p != '(')
          p++;
        if (strcmp(key, "replace_all") == 0 || strcmp(key, "replace") == 0 ||
            strcmp(key, "rewrite_all") == 0 || strcmp(key, "rewrite") == 0) {
          rc = cdd_serde_parse_replace_tuple(
              &p, end, sf->serde_replace_from, sizeof(sf->serde_replace_from),
              sf->serde_replace_to, sizeof(sf->serde_replace_to));
          continue;
        }
        rc = cdd_serde_parse_value(&p, end, val, sizeof(val));

        if (strcmp(key, "rename") == 0) {
          rc = cdd_serde_copy_str(sf->serde_rename, sizeof(sf->serde_rename),
                                  val);
        } else if (strcmp(key, "name") == 0) {
          sf->is_cli_command = 1;
          rc = cdd_serde_copy_str(sf->cli_name, sizeof(sf->cli_name), val);
        } else if (strcmp(key, "bin_name") == 0) {
          sf->is_cli_command = 1;
          rc = cdd_serde_copy_str(sf->cli_bin_name, sizeof(sf->cli_bin_name),
                                  val);
        } else if (strcmp(key, "version") == 0) {
          sf->is_cli_command = 1;
          rc =
              cdd_serde_copy_str(sf->cli_version, sizeof(sf->cli_version), val);
        } else if (strcmp(key, "author") == 0) {
          sf->is_cli_command = 1;
          rc = cdd_serde_copy_str(sf->cli_author, sizeof(sf->cli_author), val);
        } else if (strcmp(key, "about") == 0) {
          sf->is_cli_command = 1;
          rc = cdd_serde_copy_str(sf->cli_about, sizeof(sf->cli_about), val);
        } else if (strcmp(key, "long_about") == 0) {
          sf->is_cli_command = 1;
          if (sf->cli_long_about) {
            C_CDD_FREE(sf->cli_long_about);
            sf->cli_long_about = NULL;
          }
          rc = c_cdd_strdup(val, &sf->cli_long_about);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "before_help") == 0) {
          sf->is_cli_command = 1;
          if (sf->cli_before_help) {
            C_CDD_FREE(sf->cli_before_help);
            sf->cli_before_help = NULL;
          }
          rc = c_cdd_strdup(val, &sf->cli_before_help);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "after_help") == 0) {
          sf->is_cli_command = 1;
          if (sf->cli_after_help) {
            C_CDD_FREE(sf->cli_after_help);
            sf->cli_after_help = NULL;
          }
          rc = c_cdd_strdup(val, &sf->cli_after_help);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "override_usage") == 0 ||
                   strcmp(key, "usage") == 0) {
          sf->is_cli_command = 1;
          rc = cdd_serde_copy_str(sf->cli_usage_override,
                                  sizeof(sf->cli_usage_override), val);
        } else if (strcmp(key, "cli_rename_all") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            sf->cli_rename_all = rule;
          }
        } else if (strcmp(key, "rename_all") == 0 || strcmp(key, "case") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            if (cur_scope >= CDD_SCOPE_CLI) {
              sf->cli_rename_all = rule;
            } else if (cur_scope == CDD_SCOPE_TO_JSON) {
              sf->serde_rename_all_ser = rule;
            } else if (cur_scope == CDD_SCOPE_FROM_JSON) {
              sf->serde_rename_all_de = rule;
            } else {
              sf->serde_rename_all = rule;
            }
          }
        } else if (strcmp(key, "rename_all_env") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            sf->cli_rename_all_env = rule;
          }
        } else if (strcmp(key, "env_prefix") == 0) {
          sf->is_cli_command = 1;
          rc = cdd_serde_copy_str(sf->cli_env_prefix,
                                  sizeof(sf->cli_env_prefix), val);
        } else if (strcmp(key, "alias") == 0 ||
                   strcmp(key, "visible_alias") == 0) {
          rc = struct_fields_add_cli_alias(sf, val);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (strcmp(key, "arg_required_else_help") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          sf->cli_arg_required_else_help = bval;
        } else if (strcmp(key, "subcommand_required") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          sf->cli_subcommand_required = bval;
        } else if (strcmp(key, "allow_external_subcommands") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          sf->cli_allow_external_subcommands = bval;
        } else if (strcmp(key, "disable_help_flag") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          sf->cli_disable_help_flag = bval;
        } else if (strcmp(key, "disable_version_flag") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          sf->cli_disable_version_flag = bval;
        } else if (strcmp(key, "disable_help_subcommand") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          sf->cli_disable_help_subcommand = bval;
        } else if (strcmp(key, "propagate_version") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          sf->cli_propagate_version = bval;
        } else if (strcmp(key, "allow_windows_slash_flags") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          sf->cli_allow_windows_slash_flags = bval;
        } else if (strcmp(key, "allow_response_files") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          sf->cli_allow_response_files = bval;
        } else if (strcmp(key, "case_insensitive") == 0 ||
                   strcmp(key, "ignore_case") == 0) {
          rc = cdd_serde_parse_bool_val(val, &bval);
          sf->cli_case_insensitive_flags = bval;
        } else if (strcmp(key, "rename_all_serialize") == 0 ||
                   strcmp(key, "serialize") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            sf->serde_rename_all_ser = rule;
          }
        } else if (strcmp(key, "rename_all_deserialize") == 0 ||
                   strcmp(key, "deserialize") == 0) {
          if (cdd_serde_parse_case_rule(val, &rule) == CDD_C_SUCCESS) {
            sf->serde_rename_all_de = rule;
          }
        } else if (strcmp(key, "prefix") == 0) {
          rc = cdd_serde_copy_str(sf->serde_prefix, sizeof(sf->serde_prefix),
                                  val);
        } else if (strcmp(key, "suffix") == 0) {
          rc = cdd_serde_copy_str(sf->serde_suffix, sizeof(sf->serde_suffix),
                                  val);
        } else if (strcmp(key, "tag") == 0) {
          rc = cdd_serde_copy_str(sf->serde_tag, sizeof(sf->serde_tag), val);
        } else if (strcmp(key, "content") == 0) {
          rc = cdd_serde_copy_str(sf->serde_content, sizeof(sf->serde_content),
                                  val);
        }
      } else {
        if (strcmp(key, "deny_unknown_fields") == 0) {
          sf->serde_deny_unknown_fields = 1;
        } else if (strcmp(key, "default") == 0) {
          sf->serde_default = 1;
        } else if (strcmp(key, "untagged") == 0) {
          sf->serde_untagged = 1;
        } else if (strcmp(key, "version") == 0) {
          sf->is_cli_command = 1;
          if (sf->cli_version[0] == '\0') {
            rc = cdd_serde_copy_str(sf->cli_version, sizeof(sf->cli_version),
                                    "0.1.0");
          }
        } else if (strcmp(key, "author") == 0 || strcmp(key, "about") == 0 ||
                   strcmp(key, "long_about") == 0) {
          sf->is_cli_command = 1;
        } else if (strcmp(key, "arg_required_else_help") == 0) {
          sf->is_cli_command = 1;
          sf->cli_arg_required_else_help = 1;
        } else if (strcmp(key, "subcommand_required") == 0) {
          sf->is_cli_command = 1;
          sf->cli_subcommand_required = 1;
        } else if (strcmp(key, "allow_external_subcommands") == 0) {
          sf->is_cli_command = 1;
          sf->cli_allow_external_subcommands = 1;
        } else if (strcmp(key, "disable_help_flag") == 0) {
          sf->is_cli_command = 1;
          sf->cli_disable_help_flag = 1;
        } else if (strcmp(key, "disable_version_flag") == 0) {
          sf->is_cli_command = 1;
          sf->cli_disable_version_flag = 1;
        } else if (strcmp(key, "disable_help_subcommand") == 0) {
          sf->is_cli_command = 1;
          sf->cli_disable_help_subcommand = 1;
        } else if (strcmp(key, "propagate_version") == 0) {
          sf->is_cli_command = 1;
          sf->cli_propagate_version = 1;
        } else if (strcmp(key, "allow_windows_slash_flags") == 0) {
          sf->is_cli_command = 1;
          sf->cli_allow_windows_slash_flags = 1;
        } else if (strcmp(key, "allow_response_files") == 0) {
          sf->is_cli_command = 1;
          sf->cli_allow_response_files = 1;
        } else if (strcmp(key, "case_insensitive") == 0 ||
                   strcmp(key, "ignore_case") == 0) {
          sf->cli_case_insensitive_flags = 1;
        } else if (cdd_serde_parse_case_rule(key, &rule) == CDD_C_SUCCESS &&
                   rule != CDD_SERDE_CASE_NONE) {
          if (cur_scope >= CDD_SCOPE_CLI) {
            sf->cli_rename_all = rule;
          } else if (cur_scope == CDD_SCOPE_TO_JSON) {
            sf->serde_rename_all_ser = rule;
          } else if (cur_scope == CDD_SCOPE_FROM_JSON) {
            sf->serde_rename_all_de = rule;
          } else {
            sf->serde_rename_all = rule;
          }
        }
      }
    }

    scan = next;
  }

  return rc;
}

/*
 * @brief Parses container-level `@cdd` or `@serde`
 * directives into an `EnumMembers` container.
 */
cdd_c_error_t cdd_serde_parse_enum_comment(const char *comment_text,
                                           struct EnumMembers *em) {
  struct StructFields tmp_sf;
  cdd_c_error_t rc;
  if (!em)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!comment_text || comment_text[0] == '\0')
    return CDD_C_SUCCESS;

  memset(&tmp_sf, 0, sizeof(tmp_sf));
  rc = cdd_serde_parse_container_comment(comment_text, &tmp_sf);
  if (rc != CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = struct_fields_free(&tmp_sf);
    return rc;
  }

  if (tmp_sf.serde_rename_all != CDD_SERDE_CASE_NONE)
    em->serde_rename_all = tmp_sf.serde_rename_all;
  if (tmp_sf.cli_rename_all != CDD_SERDE_CASE_NONE)
    em->cli_rename_all = tmp_sf.cli_rename_all;
  if (tmp_sf.cli_case_insensitive_flags)
    em->cli_case_insensitive = 1;
  if (tmp_sf.serde_replace_from[0] != '\0') {
    rc = cdd_serde_copy_str(em->serde_replace_from,
                            sizeof(em->serde_replace_from),
                            tmp_sf.serde_replace_from);
    rc = cdd_serde_copy_str(em->serde_replace_to, sizeof(em->serde_replace_to),
                            tmp_sf.serde_replace_to);
  }
  if (tmp_sf.serde_tag[0] != '\0') {
    rc = cdd_serde_copy_str(em->serde_tag, sizeof(em->serde_tag),
                            tmp_sf.serde_tag);
  }
  if (tmp_sf.serde_content[0] != '\0') {
    rc = cdd_serde_copy_str(em->serde_content, sizeof(em->serde_content),
                            tmp_sf.serde_content);
  }
  if (tmp_sf.serde_untagged)
    em->serde_untagged = 1;
  g_cdd_last_alloc_err = struct_fields_free(&tmp_sf);
  return rc;
}
