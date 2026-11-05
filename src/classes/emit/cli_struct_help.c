/**
 * @file cli_struct_help.c
 * @brief Implementation of `--help` and shell completion script code generation
 * for CLI structs.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/cli_struct.h"
#include "classes/emit/cli_struct_internal.h"
#include "classes/parse/cdd_serde.h"
#include "functions/parse/str.h"
/* clang-format on */

/**
 * @brief Formats a single option's left-column flag specification (e.g.
 * `"  -o, --output <FILE>"` or `"      --[no-]color"`) into `out_spec`.
 *
 * @param[in] sf Parent StructFields container.
 * @param[in] f Field descriptor.
 * @param[out] out_spec Buffer receiving the left column string (at least 160
 * bytes).
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t cli_format_option_spec(const struct StructFields *sf,
                                            const struct StructField *f,
                                            char *out_spec) {
  char long_name[64];
  char val_name[64];
  enum CddCliAction act = CDD_CLI_ACTION_SET;
  int takes_val;
  size_t pos = 0;
  cdd_c_error_t rc;

  rc = cdd_cli_effective_action(f, &act);
  takes_val = (act == CDD_CLI_ACTION_SET || act == CDD_CLI_ACTION_APPEND);
  rc = cdd_cli_resolve_field_long(sf, f, long_name, sizeof(long_name));
  rc = cdd_cli_resolve_value_name(f, val_name, sizeof(val_name));

  out_spec[0] = '\0';
  if (f->cli_short != '\0' && long_name[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(out_spec, 160, "  -%c, --%s%s", f->cli_short,
              f->cli_negatable ? "[no-]" : "", long_name);
#else
    sprintf(out_spec, "  -%c, --%s%s", f->cli_short,
            f->cli_negatable ? "[no-]" : "", long_name);
#endif
  } else if (f->cli_short != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(out_spec, 160, "  -%c", f->cli_short);
#else
    sprintf(out_spec, "  -%c", f->cli_short);
#endif
  } else if (long_name[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(out_spec, 160, "      --%s%s", f->cli_negatable ? "[no-]" : "",
              long_name);
#else
    sprintf(out_spec, "      --%s%s", f->cli_negatable ? "[no-]" : "",
            long_name);
#endif
  }

  if (takes_val) {
    pos = strlen(out_spec);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(out_spec + pos, 160 - pos, "%s<%s>",
              f->cli_require_equals ? "=" : " ", val_name);
#else
    sprintf(out_spec + pos, "%s<%s>", f->cli_require_equals ? "=" : " ",
            val_name);
#endif
  }
  return rc;
}

/**
 * @brief Formats metadata badges (`[default: ...]`, `[env: ...]`,
 * `[possible values: ...]`, `[aliases: ...]`) for a field in `--help`.
 *
 * @param[in] sf Parent StructFields container.
 * @param[in] f Field descriptor.
 * @param[in] config Optional CLI config with type registry.
 * @param[out] out_badges Destination buffer.
 * @param[in] badges_sz Capacity of `out_badges`.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t cli_format_field_badges(
    const struct StructFields *sf, const struct StructField *f,
    const struct CodegenCliConfig *config, char *out_badges, size_t badges_sz) {
  char env_name[64];
  size_t pos = 0;
  size_t i;
  cdd_c_error_t rc;

  out_badges[0] = '\0';
  if (!f->cli_hide_default_value && f->default_val[0] != '\0') {
    pos = strlen(out_badges);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(out_badges + pos, badges_sz - pos, " [default: %s]",
              f->default_val);
#else
    sprintf(out_badges + pos, " [default: %s]", f->default_val);
#endif
  }

  rc = cdd_cli_resolve_field_env(sf, f, env_name, sizeof(env_name));
  if (!f->cli_hide_env && env_name[0] != '\0') {
    pos = strlen(out_badges);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(out_badges + pos, badges_sz - pos, " [env: %s]", env_name);
#else
    sprintf(out_badges + pos, " [env: %s]", env_name);
#endif
  }

  if (strcmp(f->type, "enum") == 0 ||
      (strcmp(f->type, "array") == 0 && strcmp(f->ref, "string") != 0 &&
       strcmp(f->ref, "integer") != 0 && strcmp(f->ref, "number") != 0 &&
       strcmp(f->ref, "boolean") != 0)) {
    const char *ref_type = NULL;
    const struct TypeDefinition *edef = NULL;
    rc = get_type_from_ref(f->ref, &ref_type);
    rc = cdd_cli_find_type_def(config ? config->type_registry : NULL, ref_type,
                               KIND_ENUM, &edef);
    if (edef) {
      const struct EnumMembers *em = edef->details.enum_members;
      int first_v = 1;
      pos = strlen(out_badges);
      rc = cdd_cli_copy_str(out_badges + pos, badges_sz - pos,
                            " [possible values: ");
      for (i = 0; i < em->size; ++i) {
        char wname[64];
        if (strcmp(em->members[i], "UNKNOWN") == 0)
          continue;
        if (em->serde_skip && em->serde_skip[i])
          continue;
        if (em->cli_hidden && em->cli_hidden[i])
          continue;
        if (em->cli_rename_all != CDD_SERDE_CASE_NONE &&
            (!em->serde_names || !em->serde_names[i])) {
          rc = cdd_serde_transform_case(em->members[i], em->cli_rename_all,
                                        wname, sizeof(wname));
        } else {
          rc = cdd_serde_resolve_enum_member_name(em, i, wname, sizeof(wname));
        }
        pos = strlen(out_badges);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(out_badges + pos, badges_sz - pos, "%s%s",
                  first_v ? "" : ", ", wname);
#else
        sprintf(out_badges + pos, "%s%s", first_v ? "" : ", ", wname);
#endif
        first_v = 0;
      }
      pos = strlen(out_badges);
      out_badges[pos] = ']';
      out_badges[pos + 1] = '\0';
    }
  }

  if (f->n_serde_aliases > 0 && !f->cli_positional) {
    pos = strlen(out_badges);
    rc = cdd_cli_copy_str(out_badges + pos, badges_sz - pos, " [aliases: ");
    for (i = 0; i < f->n_serde_aliases; ++i) {
      pos = strlen(out_badges);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(out_badges + pos, badges_sz - pos, "%s--%s", i == 0 ? "" : ", ",
                f->serde_aliases[i]);
#else
      sprintf(out_badges + pos, "%s--%s", i == 0 ? "" : ", ",
              f->serde_aliases[i]);
#endif
    }
    pos = strlen(out_badges);
    out_badges[pos] = ']';
    out_badges[pos + 1] = '\0';
  }
  return rc;
}

/**
 * @brief Emits a single aligned `--help` option line via `FPRINTF_HOOK`.
 *
 * @param[in,out] fp Output C stream.
 * @param[in] left_col Left column option specification string.
 * @param[in] pad_width Target width of the left column.
 * @param[in] desc Option description string.
 * @param[in] badges Metadata badges string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static cdd_c_error_t cli_emit_help_option_line(FILE *fp, const char *left_col,
                                               size_t pad_width,
                                               const char *desc,
                                               const char *badges) {
  char line[1024];
  char esc_line[1536];
  size_t left_len = strlen(left_col);
  size_t pos = 0;
  int has_desc = (desc[0] != '\0');
  int has_badges = (badges && badges[0] != '\0');
  cdd_c_error_t rc;

  rc = cdd_cli_copy_str(line, sizeof(line), left_col);
  pos = left_len;
  if (has_desc || has_badges) {
    size_t spaces = (pad_width > left_len) ? (pad_width - left_len + 2) : 2;
    while (spaces > 0) {
      line[pos++] = ' ';
      spaces--;
    }
    line[pos] = '\0';
    if (has_desc) {
      rc = cdd_cli_copy_str(line + pos, sizeof(line) - pos, desc);
      pos = strlen(line);
    }
    if (has_badges) {
      rc = cdd_cli_copy_str(line + pos, sizeof(line) - pos, badges);
    }
  }
  rc = cdd_escape_json_c_str(line, esc_line, sizeof(esc_line));
  CHECK_IO(FPRINTF_HOOK(fp,
                        "  if (fprintf(out_stream, \"%s\\n\") < 0)\n"
                        "    return CDD_C_ERROR_IO;\n",
                        esc_line));
  return rc;
}

/*
 * @brief Generates the `<Struct>_print_help` function implementation.
 */
cdd_c_error_t
write_struct_print_help_func(FILE *fp, const char *struct_name,
                             const struct StructFields *sf,
                             const struct CodegenCliConfig *config) {
  char default_prog[64];
  char esc_prog[128];
  char esc_buf[1024];
  size_t i, j;
  size_t max_opt_width = 16;
  int has_positionals = 0;
  int has_subcommands = 0;
  const struct StructField *subcmd_field = NULL;
  char headings[16][64];
  size_t n_headings = 0;
  cdd_c_error_t rc;

  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (sf->cli_bin_name[0] != '\0') {
    rc = cdd_cli_copy_str(default_prog, sizeof(default_prog), sf->cli_bin_name);
  } else if (sf->cli_name[0] != '\0') {
    rc = cdd_cli_copy_str(default_prog, sizeof(default_prog), sf->cli_name);
  } else {
    rc = cdd_serde_transform_case(struct_name, CDD_SERDE_CASE_KEBABCASE,
                                  default_prog, sizeof(default_prog));
  }
  rc = cdd_escape_json_c_str(default_prog, esc_prog, sizeof(esc_prog));

  /* Always include default "" heading first */
  headings[0][0] = '\0';
  n_headings = 1;

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    if (f->serde_skip || f->cli_hidden)
      continue;
    if (f->cli_subcommand) {
      has_subcommands = 1;
      subcmd_field = f;
      continue;
    }
    if (f->serde_flatten) {
      const char *ref_t = NULL;
      const struct TypeDefinition *cdef = NULL;
      rc = get_type_from_ref(f->ref, &ref_t);
      rc = cdd_cli_find_type_def(config ? config->type_registry : NULL, ref_t,
                                 KIND_STRUCT, &cdef);
      if (cdef) {
        const struct StructFields *csf = cdef->details.struct_fields;
        for (j = 0; j < csf->size; ++j) {
          char spec_buf[160];
          const struct StructField *cf = &csf->fields[j];
          if (cf->serde_skip || cf->cli_hidden || cf->cli_positional)
            continue;
          rc = cli_format_option_spec(csf, cf, spec_buf);
          if (strlen(spec_buf) > max_opt_width)
            max_opt_width = strlen(spec_buf);
        }
      }
      continue;
    }
    if (f->cli_positional) {
      has_positionals = 1;
    } else {
      char spec_buf[160];
      rc = cli_format_option_spec(sf, f, spec_buf);
      if (strlen(spec_buf) > max_opt_width)
        max_opt_width = strlen(spec_buf);
      if (f->cli_help_heading[0] != '\0') {
        int seen_h = 0;
        for (j = 0; j < n_headings; ++j) {
          if (strcmp(headings[j], f->cli_help_heading) == 0) {
            seen_h = 1;
            break;
          }
        }
        if (!seen_h) {
          rc = cdd_cli_copy_str(headings[n_headings],
                                sizeof(headings[n_headings]),
                                f->cli_help_heading);
          n_headings++;
        }
      }
    }
  }
  if (sf->cli_subcommand_required)
    has_subcommands = 1;

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_print_help(const char *prog_name, FILE *out_stream) {\n"
      "  const char *_cdd_prog = \"%s\";\n"
      "  if (!out_stream)\n"
      "    return CDD_C_ERROR_INVALID_ARGUMENT;\n",
      struct_name, esc_prog));
  if (sf->cli_bin_name[0] == '\0' && sf->cli_name[0] == '\0') {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  if (prog_name && prog_name[0] != '\\0') {\n"
                          "    const char *s1 = strrchr(prog_name, '/');\n"
                          "    const char *s2 = strrchr(prog_name, '\\\\');\n"
                          "    const char *last = (s1 > s2) ? s1 : s2;\n"
                          "    _cdd_prog = last ? (last + 1) : prog_name;\n"
                          "  }\n"));
  } else {
    CHECK_IO(FPRINTF_HOOK(fp, "  if (prog_name && !prog_name)\n"
                              "    _cdd_prog = prog_name;\n"));
  }

  /* 1. Header Block */
  if (sf->cli_version[0] != '\0') {
    rc = cdd_escape_json_c_str(sf->cli_version, esc_buf, sizeof(esc_buf));
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "  if (fprintf(out_stream, \"%%s %s\\n\", _cdd_prog) < 0)\n"
        "    return CDD_C_ERROR_IO;\n",
        esc_buf));
  }
  if (sf->cli_author[0] != '\0') {
    rc = cdd_escape_json_c_str(sf->cli_author, esc_buf, sizeof(esc_buf));
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  if (fprintf(out_stream, \"%s\\n\") < 0)\n"
                          "    return CDD_C_ERROR_IO;\n",
                          esc_buf));
  }
  if (sf->cli_about[0] != '\0') {
    rc = cdd_escape_json_c_str(sf->cli_about, esc_buf, sizeof(esc_buf));
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  if (fprintf(out_stream, \"%s\\n\\n\") < 0)\n"
                          "    return CDD_C_ERROR_IO;\n",
                          esc_buf));
  }
  if (sf->cli_long_about) {
    rc = cdd_escape_json_c_str(sf->cli_long_about, esc_buf, sizeof(esc_buf));
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  if (fprintf(out_stream, \"%s\\n\\n\") < 0)\n"
                          "    return CDD_C_ERROR_IO;\n",
                          esc_buf));
  }
  if (sf->cli_before_help) {
    rc = cdd_escape_json_c_str(sf->cli_before_help, esc_buf, sizeof(esc_buf));
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  if (fprintf(out_stream, \"%s\\n\\n\") < 0)\n"
                          "    return CDD_C_ERROR_IO;\n",
                          esc_buf));
  }

  /* 2. Usage Line */
  if (sf->cli_usage_override[0] != '\0') {
    rc =
        cdd_escape_json_c_str(sf->cli_usage_override, esc_buf, sizeof(esc_buf));
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  if (fprintf(out_stream, \"Usage: %s\\n\") < 0)\n"
                          "    return CDD_C_ERROR_IO;\n",
                          esc_buf));
  } else {
    char usage_tail[512];
    size_t upos = 0;
    rc = cdd_cli_copy_str(usage_tail, sizeof(usage_tail), " [OPTIONS]");
    for (i = 0; i < sf->size; ++i) {
      const struct StructField *f = &sf->fields[i];
      if (f->serde_skip || f->cli_hidden || f->cli_subcommand ||
          f->serde_flatten)
        continue;
      if (!f->cli_positional && f->required && f->default_val[0] == '\0' &&
          !f->serde_has_default) {
        char lname[64];
        char vname[64];
        rc = cdd_cli_resolve_field_long(sf, f, lname, sizeof(lname));
        rc = cdd_cli_resolve_value_name(f, vname, sizeof(vname));
        upos = strlen(usage_tail);
        if (lname[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(usage_tail + upos, sizeof(usage_tail) - upos, " --%s <%s>",
                    lname, vname);
#else
          sprintf(usage_tail + upos, " --%s <%s>", lname, vname);
#endif
        }
      }
    }
    for (i = 0; i < sf->size; ++i) {
      const struct StructField *f = &sf->fields[i];
      char vname[64];
      const char *dots;
      if (f->serde_skip || f->cli_hidden || !f->cli_positional)
        continue;
      rc = cdd_cli_resolve_value_name(f, vname, sizeof(vname));
      upos = strlen(usage_tail);
      dots = (strcmp(f->type, "array") == 0 || f->cli_trailing_var_arg) ? "..."
                                                                        : "";
      if (f->required) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(usage_tail + upos, sizeof(usage_tail) - upos, " <%s>%s",
                  vname, dots);
#else
        sprintf(usage_tail + upos, " <%s>%s", vname, dots);
#endif
      } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(usage_tail + upos, sizeof(usage_tail) - upos, " [%s]%s",
                  vname, dots);
#else
        sprintf(usage_tail + upos, " [%s]%s", vname, dots);
#endif
      }
    }
    if (has_subcommands) {
      upos = strlen(usage_tail);
      rc = cdd_cli_copy_str(usage_tail + upos, sizeof(usage_tail) - upos,
                            sf->cli_subcommand_required ? " <COMMAND>"
                                                        : " [COMMAND]");
    }
    rc = cdd_escape_json_c_str(usage_tail, esc_buf, sizeof(esc_buf));
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "  if (fprintf(out_stream, \"Usage: %%s%s\\n\", _cdd_prog) < 0)\n"
        "    return CDD_C_ERROR_IO;\n",
        esc_buf));
  }

  /* 3. Commands Section (Subcommands) */
  if (has_subcommands) {
    const char *ref_t = NULL;
    const struct TypeDefinition *udef = NULL;
    rc = get_type_from_ref(subcmd_field->ref, &ref_t);
    rc = cdd_cli_find_type_def(config ? config->type_registry : NULL, ref_t,
                               KIND_STRUCT, &udef);
    if (udef) {
      const struct StructFields *usf = udef->details.struct_fields;
      CHECK_IO(FPRINTF_HOOK(
          fp, "  if (fprintf(out_stream, \"\\nCommands:\\n\") < 0)\n"
              "    return CDD_C_ERROR_IO;\n"));
      for (i = 0; i < usf->size; ++i) {
        const struct StructField *vf = &usf->fields[i];
        char cmd_name[64];
        char left_col[80];
        const char *cmd_desc = vf->cli_help[0] ? vf->cli_help : vf->description;
        if (vf->serde_skip || vf->cli_hidden)
          continue;
        if (vf->serde_rename[0] != '\0') {
          rc = cdd_cli_copy_str(cmd_name, sizeof(cmd_name), vf->serde_rename);
        } else {
          rc = cdd_serde_transform_case(vf->name, CDD_SERDE_CASE_KEBABCASE,
                                        cmd_name, sizeof(cmd_name));
        }
        if (cmd_desc[0] == '\0') {
          const char *vref = NULL;
          const struct TypeDefinition *vdef = NULL;
          rc = get_type_from_ref(vf->ref, &vref);
          rc = cdd_cli_find_type_def(config->type_registry, vref, KIND_STRUCT,
                                     &vdef);
          if (vdef && vdef->details.struct_fields->cli_about[0] != '\0') {
            cmd_desc = vdef->details.struct_fields->cli_about;
          }
        }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(left_col, sizeof(left_col), "  %s", cmd_name);
#else
        sprintf(left_col, "  %s", cmd_name);
#endif
        CHECK_IO(cli_emit_help_option_line(fp, left_col, 16, cmd_desc, NULL) !=
                         CDD_C_SUCCESS
                     ? -1
                     : 0);
      }
      if (!sf->cli_disable_help_subcommand) {
        CHECK_IO(cli_emit_help_option_line(
                     fp, "  help", 16,
                     "Print this message or the help of the given "
                     "subcommand(s)",
                     NULL) != CDD_C_SUCCESS
                     ? -1
                     : 0);
      }
    }
  }

  /* 4. Arguments Section (Positionals) */
  if (has_positionals) {
    CHECK_IO(FPRINTF_HOOK(
        fp, "  if (fprintf(out_stream, \"\\nArguments:\\n\") < 0)\n"
            "    return CDD_C_ERROR_IO;\n"));
    for (i = 0; i < sf->size; ++i) {
      const struct StructField *f = &sf->fields[i];
      char vname[64];
      char left_col[80];
      char badges[256];
      const char *desc = f->cli_help[0] ? f->cli_help : f->description;
      if (f->serde_skip || f->cli_hidden || !f->cli_positional)
        continue;
      rc = cdd_cli_resolve_value_name(f, vname, sizeof(vname));
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(left_col, sizeof(left_col), "  <%s>%s", vname,
                (strcmp(f->type, "array") == 0 || f->cli_trailing_var_arg)
                    ? "..."
                    : "");
#else
      sprintf(left_col, "  <%s>%s", vname,
              (strcmp(f->type, "array") == 0 || f->cli_trailing_var_arg) ? "..."
                                                                         : "");
#endif
      rc = cli_format_field_badges(sf, f, config, badges, sizeof(badges));
      CHECK_IO(cli_emit_help_option_line(fp, left_col, max_opt_width, desc,
                                         badges) != CDD_C_SUCCESS
                   ? -1
                   : 0);
    }
  }

  /* 5. Options & Custom Heading Sections */
  for (j = 0; j < n_headings; ++j) {
    const char *sec_title = (headings[j][0] != '\0') ? headings[j] : "Options";
    rc = cdd_escape_json_c_str(sec_title, esc_buf, sizeof(esc_buf));
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  if (fprintf(out_stream, \"\\n%s:\\n\") < 0)\n"
                          "    return CDD_C_ERROR_IO;\n",
                          esc_buf));

    for (i = 0; i < sf->size; ++i) {
      const struct StructField *f = &sf->fields[i];
      char left_col[160];
      char badges[256];
      const char *desc;

      if (f->serde_skip || f->cli_hidden || f->cli_positional ||
          f->cli_subcommand)
        continue;

      if (f->serde_flatten) {
        if (j == 0) {
          const char *ref_t = NULL;
          const struct TypeDefinition *cdef = NULL;
          rc = get_type_from_ref(f->ref, &ref_t);
          rc = cdd_cli_find_type_def(config ? config->type_registry : NULL,
                                     ref_t, KIND_STRUCT, &cdef);
          if (cdef) {
            size_t k;
            const struct StructFields *csf = cdef->details.struct_fields;
            for (k = 0; k < csf->size; ++k) {
              const struct StructField *cf = &csf->fields[k];
              if (cf->serde_skip || cf->cli_hidden || cf->cli_positional)
                continue;
              rc = cli_format_option_spec(csf, cf, left_col);
              rc = cli_format_field_badges(csf, cf, config, badges,
                                           sizeof(badges));
              desc = cf->cli_help[0] ? cf->cli_help : cf->description;
              CHECK_IO(cli_emit_help_option_line(fp, left_col, max_opt_width,
                                                 desc, badges) != CDD_C_SUCCESS
                           ? -1
                           : 0);
            }
          }
        }
        continue;
      }

      if (strcmp(f->cli_help_heading, headings[j]) != 0)
        continue;

      rc = cli_format_option_spec(sf, f, left_col);
      rc = cli_format_field_badges(sf, f, config, badges, sizeof(badges));
      desc = f->cli_help[0] ? f->cli_help : f->description;
      CHECK_IO(cli_emit_help_option_line(fp, left_col, max_opt_width, desc,
                                         badges) != CDD_C_SUCCESS
                   ? -1
                   : 0);
    }

    if (j == 0) {
      if (!sf->cli_disable_help_flag) {
        CHECK_IO(cli_emit_help_option_line(fp, "  -h, --help", max_opt_width,
                                           "Print help", NULL) != CDD_C_SUCCESS
                     ? -1
                     : 0);
      }
      if (!sf->cli_disable_version_flag) {
        CHECK_IO(cli_emit_help_option_line(fp, "  -V, --version", max_opt_width,
                                           "Print version",
                                           NULL) != CDD_C_SUCCESS
                     ? -1
                     : 0);
      }
    }
  }

  /* 6. Footer Block */
  if (sf->cli_after_help) {
    rc = cdd_escape_json_c_str(sf->cli_after_help, esc_buf, sizeof(esc_buf));
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  if (fprintf(out_stream, \"\\n%s\\n\") < 0)\n"
                          "    return CDD_C_ERROR_IO;\n",
                          esc_buf));
  }

  CHECK_IO(FPRINTF_HOOK(fp, "  return CDD_C_SUCCESS;\n"
                            "}\n\n"));
  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));
  }
  return rc;
}

/*
 * @brief Generates the `<Struct>_print_completions` function implementation
 * supporting `"bash"`, `"zsh"`, `"fish"`, and `"powershell"` / `"pwsh"`.
 */
cdd_c_error_t
write_struct_print_completions_func(FILE *fp, const char *struct_name,
                                    const struct StructFields *sf,
                                    const struct CodegenCliConfig *config) {
  char default_prog[64];
  char esc_prog[128];
  char words_buf[512];
  char esc_words[768];
  size_t i;
  size_t wpos = 0;
  cdd_c_error_t rc;

  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (sf->cli_bin_name[0] != '\0') {
    rc = cdd_cli_copy_str(default_prog, sizeof(default_prog), sf->cli_bin_name);
  } else if (sf->cli_name[0] != '\0') {
    rc = cdd_cli_copy_str(default_prog, sizeof(default_prog), sf->cli_name);
  } else {
    rc = cdd_serde_transform_case(struct_name, CDD_SERDE_CASE_KEBABCASE,
                                  default_prog, sizeof(default_prog));
  }
  rc = cdd_escape_json_c_str(default_prog, esc_prog, sizeof(esc_prog));

  words_buf[0] = '\0';
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    char lname[64];
    if (f->serde_skip || f->cli_hidden || f->cli_positional)
      continue;
    rc = cdd_cli_resolve_field_long(sf, f, lname, sizeof(lname));
    if (lname[0] != '\0') {
      wpos = strlen(words_buf);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(words_buf + wpos, sizeof(words_buf) - wpos, "%s--%s",
                wpos > 0 ? " " : "", lname);
#else
      sprintf(words_buf + wpos, "%s--%s", wpos > 0 ? " " : "", lname);
#endif
      if (f->cli_negatable) {
        wpos = strlen(words_buf);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(words_buf + wpos, sizeof(words_buf) - wpos, " --no-%s",
                  lname);
#else
        sprintf(words_buf + wpos, " --no-%s", lname);
#endif
      }
    }
    if (f->cli_short != '\0') {
      wpos = strlen(words_buf);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(words_buf + wpos, sizeof(words_buf) - wpos, "%s-%c",
                wpos > 0 ? " " : "", f->cli_short);
#else
      sprintf(words_buf + wpos, "%s-%c", wpos > 0 ? " " : "", f->cli_short);
#endif
    }
  }
  if (!sf->cli_disable_help_flag) {
    wpos = strlen(words_buf);
    rc = cdd_cli_copy_str(words_buf + wpos, sizeof(words_buf) - wpos,
                          wpos > 0 ? " --help -h" : "--help -h");
  }
  if (!sf->cli_disable_version_flag) {
    wpos = strlen(words_buf);
    rc = cdd_cli_copy_str(words_buf + wpos, sizeof(words_buf) - wpos,
                          wpos > 0 ? " --version -V" : "--version -V");
  }
  rc = cdd_escape_json_c_str(words_buf, esc_words, sizeof(esc_words));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_print_completions(const char *prog_name, const char "
      "*shell, FILE *out_stream) {\n"
      "  const char *_cdd_prog = (prog_name && prog_name[0] != '\\0') ? "
      "prog_name : \"%s\";\n"
      "  if (!shell || !out_stream)\n"
      "    return CDD_C_ERROR_INVALID_ARGUMENT;\n",
      struct_name, esc_prog));

  /* Bash */
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "  if (strcmp(shell, \"bash\") == 0) {\n"
      "    if (fprintf(out_stream, \"_%s_completions() {\\n\") < 0)\n"
      "      return CDD_C_ERROR_IO;\n"
      "    if (fprintf(out_stream, \"  local "
      "cur=\\\"${COMP_WORDS[COMP_CWORD]}\\\"\\n\") < 0)\n"
      "      return CDD_C_ERROR_IO;\n"
      "    if (fprintf(out_stream, \"  COMPREPLY=( $(compgen -W \\\"%s\\\" -- "
      "\\\"$cur\\\") )\\n\") < 0)\n"
      "      return CDD_C_ERROR_IO;\n"
      "    if (fprintf(out_stream, \"}\\ncomplete -F _%s_completions %%s\\n\", "
      "_cdd_prog) < 0)\n"
      "      return CDD_C_ERROR_IO;\n"
      "    return CDD_C_SUCCESS;\n"
      "  }\n",
      struct_name, esc_words, struct_name));

  /* Zsh */
  CHECK_IO(FPRINTF_HOOK(
      fp, "  if (strcmp(shell, \"zsh\") == 0) {\n"
          "    if (fprintf(out_stream, \"#compdef %%s\\n_arguments\", "
          "_cdd_prog) < 0)\n"
          "      return CDD_C_ERROR_IO;\n"));
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    char lname[64];
    char esc_help[256];
    if (f->serde_skip || f->cli_hidden || f->cli_positional)
      continue;
    rc = cdd_cli_resolve_field_long(sf, f, lname, sizeof(lname));
    if (lname[0] == '\0')
      continue;
    rc = cdd_escape_json_c_str(f->cli_help[0] ? f->cli_help : f->description,
                               esc_help, sizeof(esc_help));
    CHECK_IO(FPRINTF_HOOK(fp,
                          "    if (fprintf(out_stream, \" '--%s[%s]'\") < 0)\n"
                          "      return CDD_C_ERROR_IO;\n",
                          lname, esc_help));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "    if (fprintf(out_stream, \"\\n\") < 0)\n"
                            "      return CDD_C_ERROR_IO;\n"
                            "    return CDD_C_SUCCESS;\n"
                            "  }\n"));

  /* Fish */
  CHECK_IO(FPRINTF_HOOK(fp, "  if (strcmp(shell, \"fish\") == 0) {\n"));
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    char lname[64];
    char esc_help[256];
    if (f->serde_skip || f->cli_hidden || f->cli_positional)
      continue;
    rc = cdd_cli_resolve_field_long(sf, f, lname, sizeof(lname));
    rc = cdd_escape_json_c_str(f->cli_help[0] ? f->cli_help : f->description,
                               esc_help, sizeof(esc_help));
    if (f->cli_short != '\0' && lname[0] != '\0') {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "    if (fprintf(out_stream, \"complete -c %%s -s "
                            "%c -l %s -d '%s'\\n\", _cdd_prog) < 0)\n"
                            "      return CDD_C_ERROR_IO;\n",
                            f->cli_short, lname, esc_help));
    } else if (lname[0] != '\0') {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "    if (fprintf(out_stream, \"complete -c %%s -l "
                            "%s -d '%s'\\n\", _cdd_prog) < 0)\n"
                            "      return CDD_C_ERROR_IO;\n",
                            lname, esc_help));
    }
  }
  CHECK_IO(FPRINTF_HOOK(fp, "    return CDD_C_SUCCESS;\n"
                            "  }\n"));

  /* PowerShell */
  CHECK_IO(FPRINTF_HOOK(
      fp, "  if (strcmp(shell, \"powershell\") == 0 || strcmp(shell, \"pwsh\") "
          "== 0) {\n"
          "    if (fprintf(out_stream, \"Register-ArgumentCompleter -Native "
          "-CommandName '%%s' -ScriptBlock {\\n\", _cdd_prog) < 0)\n"
          "      return CDD_C_ERROR_IO;\n"
          "    if (fprintf(out_stream, \"  param($wordToComplete, $commandAst, "
          "$cursorPosition)\\n\") < 0)\n"
          "      return CDD_C_ERROR_IO;\n"));
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "    if (fprintf(out_stream, \"  '%s'.Split(' ') | Where-Object { $_ "
      "-like \\\"$wordToComplete*\\\" }\\n}\\n\") < 0)\n"
      "      return CDD_C_ERROR_IO;\n"
      "    return CDD_C_SUCCESS;\n"
      "  }\n"
      "  return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "}\n\n",
      esc_words));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));
  }
  return rc;
}
