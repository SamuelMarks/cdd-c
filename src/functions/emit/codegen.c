/**
 * @file codegen.c
 * @brief Core codegen definitions and implementation.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/code_writer.h"
#include "classes/parse/cdd_serde.h"
#include "functions/emit/codegen.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief fprintf macro */
#define fprintf cdd_io_fprintf_hook

/** @brief CHECK_IO definition */
#ifndef CHECK_IO
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO
#endif

/*
 * @brief Emits a C forward declaration (`struct <struct_name>;`) to a file
 * stream.
 *
 * @param[in,out] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t write_forward_decl(FILE *fp, const char *struct_name) {
  if (!fp || !struct_name) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (fprintf(fp, "struct %s;\n", struct_name) < 0) {
    return CDD_C_ERROR_IO;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Emits a C enum declaration and string conversion prototypes to a
 * header file.
 *
 * @param[in,out] hfile Output header file stream.
 * @param[in] enum_name Name of the C enum type.
 * @param[in] sf Struct fields descriptor containing enum members.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t write_enum_declaration_h(FILE *hfile, const char *enum_name,
                                       const struct StructFields *sf,
                                       const struct CodegenConfig *config) {
  size_t i;
  const struct EnumMembers *em;
  enum CddSerdeCaseRule eff_rename_all;
  const char *eff_rep_from;
  const char *eff_rep_to;
  const char *eff_tag;
  const char *eff_content;
  int eff_untagged;

  if (!hfile || !enum_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  em = &sf->enum_members;
  eff_rename_all = (em->serde_rename_all != CDD_SERDE_CASE_NONE)
                       ? em->serde_rename_all
                       : sf->serde_rename_all;
  eff_rep_from = (em->serde_replace_from[0] != '\0') ? em->serde_replace_from
                                                     : sf->serde_replace_from;
  eff_rep_to = (em->serde_replace_from[0] != '\0') ? em->serde_replace_to
                                                   : sf->serde_replace_to;
  eff_tag = (em->serde_tag[0] != '\0') ? em->serde_tag : sf->serde_tag;
  eff_content =
      (em->serde_content[0] != '\0') ? em->serde_content : sf->serde_content;
  eff_untagged = em->serde_untagged || sf->serde_untagged;

  if (sf->serde_rename[0] != '\0' || eff_rename_all != CDD_SERDE_CASE_NONE ||
      eff_rep_from[0] != '\0' || eff_tag[0] != '\0' || eff_content[0] != '\0' ||
      eff_untagged) {
    int first_attr = 1;
    char esc1[256];
    char esc2[256];
    CHECK_IO(fprintf(hfile, "/* @cdd "));
    if (sf->serde_rename[0] != '\0') {
      cdd_escape_comment_str(sf->serde_rename, esc1, sizeof(esc1));
      CHECK_IO(fprintf(hfile, "rename = \"%s\"", esc1));
      first_attr = 0;
    }
    if (eff_rename_all != CDD_SERDE_CASE_NONE) {
      const char *c_str = NULL;
      cdd_serde_case_rule_to_str(eff_rename_all, &c_str);
      CHECK_IO(fprintf(hfile, "%srename_all = \"%s\"", first_attr ? "" : ", ",
                       c_str));
      first_attr = 0;
    }
    if (eff_rep_from[0] != '\0') {
      cdd_escape_comment_str(eff_rep_from, esc1, sizeof(esc1));
      cdd_escape_comment_str(eff_rep_to, esc2, sizeof(esc2));
      CHECK_IO(fprintf(hfile, "%sreplace_all = (\"%s\" -> \"%s\")",
                       first_attr ? "" : ", ", esc1, esc2));
      first_attr = 0;
    }
    if (eff_tag[0] != '\0') {
      cdd_escape_comment_str(eff_tag, esc1, sizeof(esc1));
      CHECK_IO(fprintf(hfile, "%stag = \"%s\"", first_attr ? "" : ", ", esc1));
      first_attr = 0;
    }
    if (eff_content[0] != '\0') {
      cdd_escape_comment_str(eff_content, esc1, sizeof(esc1));
      CHECK_IO(
          fprintf(hfile, "%scontent = \"%s\"", first_attr ? "" : ", ", esc1));
      first_attr = 0;
    }
    if (eff_untagged) {
      CHECK_IO(fprintf(hfile, "%suntagged", first_attr ? "" : ", "));
    }
    CHECK_IO(fprintf(hfile, " */\n"));
  }

  CHECK_IO(fprintf(hfile, "enum %s {\n", enum_name));
  CHECK_IO(fprintf(hfile, "  %s_UNKNOWN = 0,\n", enum_name));
  for (i = 0; i < em->size; ++i) {
    const char *member = em->members[i];
    const char *v_rename = em->serde_names ? em->serde_names[i] : NULL;
    int v_skip = (em->serde_skip && em->serde_skip[i]) ? 1 : 0;
    size_t v_n_al = em->n_serde_aliases ? em->n_serde_aliases[i] : 0;
    int v_other = (em->serde_other_idx == (int)i) ? 1 : 0;
    if (member == NULL) {
      continue;
    }
    if (strcmp(member, "UNKNOWN") == 0) {
      continue;
    }
    if (v_rename || v_skip || v_n_al > 0 || v_other) {
      int v_first = 1;
      size_t a_idx;
      char esc[256];
      CHECK_IO(fprintf(hfile, "  /* @cdd "));
      if (v_rename) {
        cdd_escape_comment_str(v_rename, esc, sizeof(esc));
        CHECK_IO(fprintf(hfile, "rename = \"%s\"", esc));
        v_first = 0;
      }
      for (a_idx = 0; a_idx < v_n_al; ++a_idx) {
        cdd_escape_comment_str(em->serde_aliases[i][a_idx], esc, sizeof(esc));
        CHECK_IO(fprintf(hfile, "%salias = \"%s\"", v_first ? "" : ", ", esc));
        v_first = 0;
      }
      if (v_skip) {
        CHECK_IO(fprintf(hfile, "%sskip", v_first ? "" : ", "));
        v_first = 0;
      }
      if (v_other) {
        CHECK_IO(fprintf(hfile, "%sother", v_first ? "" : ", "));
      }
      CHECK_IO(fprintf(hfile, " */\n"));
    }
    CHECK_IO(fprintf(hfile, "  %s_%s,\n", enum_name, member));
  }
  CHECK_IO(fprintf(hfile, "%s", "};\n\n"));

  if (config && config->enum_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->enum_guard));
  }
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_from_str(const char *, enum %s "
                   "*);\n",
                   enum_name, enum_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT int %s_to_str(enum %s, char **);\n",
                   enum_name, enum_name));
  if (config && config->enum_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits a tagged C union declaration and JSON/cleanup prototypes to a
 * header file.
 *
 * @param[in,out] hfile Output header file stream.
 * @param[in] union_name Name of the C union type.
 * @param[in] sf Struct fields descriptor containing union variants.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t write_union_declaration_h(FILE *hfile, const char *union_name,
                                        const struct StructFields *sf,
                                        const struct CodegenConfig *config) {
  size_t i;
  const char *disc_tag = NULL;
  if (!hfile || !union_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (sf->serde_tag[0] != '\0') {
    disc_tag = sf->serde_tag;
  } else if (sf->union_discriminator && sf->union_discriminator[0] != '\0') {
    disc_tag = sf->union_discriminator;
  }

  CHECK_IO(fprintf(hfile, "enum %s_tag {\n", union_name));
  CHECK_IO(fprintf(hfile, "  %s_UNKNOWN = 0,\n", union_name));
  for (i = 0; i < sf->size; ++i) {
    CHECK_IO(fprintf(hfile, "  %s_%s,\n", union_name, sf->fields[i].name));
  }
  CHECK_IO(fprintf(hfile, "%s", "};\n\n"));

  if (sf->serde_rename[0] != '\0' || disc_tag != NULL ||
      sf->serde_content[0] != '\0' || sf->serde_untagged) {
    int first_attr = 1;
    char esc[256];
    CHECK_IO(fprintf(hfile, "/* @cdd "));
    if (sf->serde_rename[0] != '\0') {
      cdd_escape_comment_str(sf->serde_rename, esc, sizeof(esc));
      CHECK_IO(fprintf(hfile, "rename = \"%s\"", esc));
      first_attr = 0;
    }
    if (disc_tag != NULL) {
      cdd_escape_comment_str(disc_tag, esc, sizeof(esc));
      CHECK_IO(fprintf(hfile, "%stag = \"%s\"", first_attr ? "" : ", ", esc));
      first_attr = 0;
    }
    if (sf->serde_content[0] != '\0') {
      cdd_escape_comment_str(sf->serde_content, esc, sizeof(esc));
      CHECK_IO(
          fprintf(hfile, "%scontent = \"%s\"", first_attr ? "" : ", ", esc));
      first_attr = 0;
    }
    if (sf->serde_untagged) {
      CHECK_IO(fprintf(hfile, "%suntagged", first_attr ? "" : ", "));
    }
    CHECK_IO(fprintf(hfile, " */\n"));
  }

  CHECK_IO(fprintf(hfile, "struct %s {\n", union_name));
  CHECK_IO(fprintf(hfile, "  enum %s_tag tag;\n", union_name));
  CHECK_IO(fprintf(hfile, "%s", "  union {\n"));
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *field = &sf->fields[i];
    const char *n = field->name;
    const char *t = field->type;
    const char *r = field->ref;
    const char *ref_type = NULL;
    cdd_c_error_t rc_ref;

    if (strcmp(t, "string") == 0) {
      CHECK_IO(fprintf(hfile, "    const char *%s;\n", n));
    } else if (strcmp(t, "integer") == 0) {
      CHECK_IO(fprintf(hfile, "    int %s;\n", n));
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(fprintf(hfile, "    double %s;\n", n));
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(fprintf(hfile, "    int %s;\n", n));
    } else if (strcmp(t, "enum") == 0) {
      rc_ref = get_type_from_ref(r, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(fprintf(hfile, "    enum %s %s;\n", ref_type, n));
    } else if (strcmp(t, "object") == 0) {
      rc_ref = get_type_from_ref(r, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(fprintf(hfile, "    struct %s *%s;\n", ref_type, n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(fprintf(hfile, "    struct {\n"));
      CHECK_IO(fprintf(hfile, "      size_t n_%s;\n", n));
      if (strcmp(r, "string") == 0) {
        CHECK_IO(fprintf(hfile, "      char **%s;\n", n));
      } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
        CHECK_IO(fprintf(hfile, "      int *%s;\n", n));
      } else if (strcmp(r, "number") == 0) {
        CHECK_IO(fprintf(hfile, "      double *%s;\n", n));
      } else {
        rc_ref = get_type_from_ref(r, &ref_type);
        if (rc_ref != CDD_C_SUCCESS)
          return rc_ref;
        CHECK_IO(fprintf(hfile, "      struct %s **%s;\n", ref_type, n));
      }
      CHECK_IO(fprintf(hfile, "    } %s;\n", n));
    } else {
      CHECK_IO(fprintf(hfile, "    int %s;\n", n));
    }
  }
  CHECK_IO(fprintf(hfile, "%s", "  } data;\n};\n\n"));

  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));
  }
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_from_json(const "
                   "char *, struct %s **);\n",
                   union_name, union_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_to_json(const "
                   "struct %s *, char **);\n",
                   union_name, union_name));
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
  }

  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
  }
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_cleanup(struct %s *);\n",
                   union_name, union_name));
  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Emits a field-level `@cdd` comment line if the field has any
 * Serde reflection directives.
 *
 * @param[in,out] hfile Output header file stream.
 * @param[in] field StructField descriptor to inspect.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static cdd_c_error_t emit_field_serde_comment(FILE *hfile,
                                              const struct StructField *field) {
  int f_first = 1;
  size_t a_idx;
  char esc1[256];
  char esc2[256];
  int eff_skip_ser = field->serde_skip_serializing || field->write_only;
  int eff_skip_de = field->serde_skip_deserializing || field->read_only;
  int has_cli =
      (field->cli_short != '\0' || field->cli_long[0] != '\0' ||
       field->cli_positional || field->cli_index >= 0 ||
       field->cli_env[0] != '\0' || field->cli_value_name[0] != '\0' ||
       field->cli_help[0] != '\0' || field->cli_action != CDD_CLI_ACTION_AUTO ||
       field->cli_value_delimiter != '\0' || field->n_cli_conflicts_with > 0 ||
       field->n_cli_requires > 0 || field->cli_subcommand ||
       field->cli_negatable);

  if (field->serde_rename[0] == '\0' && field->serde_rename_ser[0] == '\0' &&
      field->serde_rename_de[0] == '\0' &&
      field->serde_case == CDD_SERDE_CASE_NONE &&
      field->serde_replace_from[0] == '\0' && !field->serde_skip &&
      !eff_skip_ser && !eff_skip_de &&
      field->serde_skip_if_kind == CDD_SERDE_SKIP_IF_NONE &&
      !field->serde_flatten && !field->serde_has_default &&
      field->serde_with[0] == '\0' && field->serde_serialize_with[0] == '\0' &&
      field->serde_deserialize_with[0] == '\0' && field->n_serde_aliases == 0 &&
      !has_cli) {
    return CDD_C_SUCCESS;
  }

  CHECK_IO(fprintf(hfile, "  /* @cdd "));
  if (field->serde_rename[0] != '\0') {
    cdd_escape_comment_str(field->serde_rename, esc1, sizeof(esc1));
    CHECK_IO(fprintf(hfile, "rename = \"%s\"", esc1));
    f_first = 0;
  }
  if (field->serde_rename_ser[0] != '\0') {
    cdd_escape_comment_str(field->serde_rename_ser, esc1, sizeof(esc1));
    CHECK_IO(fprintf(hfile, "%srename_serialize = \"%s\"", f_first ? "" : ", ",
                     esc1));
    f_first = 0;
  }
  if (field->serde_rename_de[0] != '\0') {
    cdd_escape_comment_str(field->serde_rename_de, esc1, sizeof(esc1));
    CHECK_IO(fprintf(hfile, "%srename_deserialize = \"%s\"",
                     f_first ? "" : ", ", esc1));
    f_first = 0;
  }
  if (field->serde_case != CDD_SERDE_CASE_NONE) {
    const char *c_str = NULL;
    cdd_serde_case_rule_to_str(field->serde_case, &c_str);
    CHECK_IO(fprintf(hfile, "%scase = \"%s\"", f_first ? "" : ", ", c_str));
    f_first = 0;
  }
  if (field->serde_replace_from[0] != '\0') {
    cdd_escape_comment_str(field->serde_replace_from, esc1, sizeof(esc1));
    cdd_escape_comment_str(field->serde_replace_to, esc2, sizeof(esc2));
    CHECK_IO(fprintf(hfile, "%sreplace = (\"%s\" -> \"%s\")",
                     f_first ? "" : ", ", esc1, esc2));
    f_first = 0;
  }
  for (a_idx = 0; a_idx < field->n_serde_aliases; ++a_idx) {
    cdd_escape_comment_str(field->serde_aliases[a_idx], esc1, sizeof(esc1));
    CHECK_IO(fprintf(hfile, "%salias = \"%s\"", f_first ? "" : ", ", esc1));
    f_first = 0;
  }
  if (field->serde_skip) {
    CHECK_IO(fprintf(hfile, "%sskip", f_first ? "" : ", "));
    f_first = 0;
  } else {
    if (eff_skip_ser) {
      CHECK_IO(fprintf(hfile, "%sskip_serializing", f_first ? "" : ", "));
      f_first = 0;
    }
    if (eff_skip_de) {
      CHECK_IO(fprintf(hfile, "%sskip_deserializing", f_first ? "" : ", "));
      f_first = 0;
    }
  }
  if (field->serde_skip_if_kind != CDD_SERDE_SKIP_IF_NONE) {
    const char *sk_val = "null";
    if (field->serde_skip_if_kind == CDD_SERDE_SKIP_IF_EMPTY)
      sk_val = "empty";
    else if (field->serde_skip_if_kind == CDD_SERDE_SKIP_IF_ZERO)
      sk_val = "zero";
    else if (field->serde_skip_if_fn[0] != '\0')
      sk_val = field->serde_skip_if_fn;
    cdd_escape_comment_str(sk_val, esc1, sizeof(esc1));
    CHECK_IO(fprintf(hfile, "%sskip_serializing_if = \"%s\"",
                     f_first ? "" : ", ", esc1));
    f_first = 0;
  }
  if (field->serde_flatten) {
    CHECK_IO(fprintf(hfile, "%sflatten", f_first ? "" : ", "));
    f_first = 0;
  }
  if (field->serde_has_default) {
    if (field->serde_default_fn[0] != '\0') {
      cdd_escape_comment_str(field->serde_default_fn, esc1, sizeof(esc1));
      CHECK_IO(
          fprintf(hfile, "%sdefault_fn = \"%s\"", f_first ? "" : ", ", esc1));
    } else if (field->default_val[0] != '\0') {
      cdd_escape_comment_str(field->default_val, esc1, sizeof(esc1));
      CHECK_IO(fprintf(hfile, "%sdefault = \"%s\"", f_first ? "" : ", ", esc1));
    } else {
      CHECK_IO(fprintf(hfile, "%sdefault", f_first ? "" : ", "));
    }
    f_first = 0;
  }
  if (field->serde_with[0] != '\0') {
    cdd_escape_comment_str(field->serde_with, esc1, sizeof(esc1));
    CHECK_IO(fprintf(hfile, "%swith = \"%s\"", f_first ? "" : ", ", esc1));
    f_first = 0;
  }
  if (field->serde_serialize_with[0] != '\0') {
    cdd_escape_comment_str(field->serde_serialize_with, esc1, sizeof(esc1));
    CHECK_IO(
        fprintf(hfile, "%sserialize_with = \"%s\"", f_first ? "" : ", ", esc1));
    f_first = 0;
  }
  if (field->serde_deserialize_with[0] != '\0') {
    cdd_escape_comment_str(field->serde_deserialize_with, esc1, sizeof(esc1));
    CHECK_IO(fprintf(hfile, "%sdeserialize_with = \"%s\"", f_first ? "" : ", ",
                     esc1));
    f_first = 0;
  }
  if (has_cli) {
    int c_first = 1;
    CHECK_IO(fprintf(hfile, "%scli(", f_first ? "" : ", "));
    if (field->cli_short != '\0') {
      CHECK_IO(fprintf(hfile, "short = '%c'", field->cli_short));
      c_first = 0;
    }
    if (field->cli_long[0] != '\0') {
      cdd_escape_comment_str(field->cli_long, esc1, sizeof(esc1));
      CHECK_IO(fprintf(hfile, "%slong = \"%s\"", c_first ? "" : ", ", esc1));
      c_first = 0;
    }
    if (field->cli_negatable) {
      CHECK_IO(fprintf(hfile, "%snegatable", c_first ? "" : ", "));
      c_first = 0;
    }
    if (field->cli_positional) {
      CHECK_IO(fprintf(hfile, "%spositional", c_first ? "" : ", "));
      c_first = 0;
    }
    if (field->cli_index >= 0) {
      CHECK_IO(fprintf(hfile, "%sindex = %d", c_first ? "" : ", ",
                       field->cli_index));
      c_first = 0;
    }
    if (field->cli_env[0] != '\0') {
      cdd_escape_comment_str(field->cli_env, esc1, sizeof(esc1));
      CHECK_IO(fprintf(hfile, "%senv = \"%s\"", c_first ? "" : ", ", esc1));
      c_first = 0;
    }
    if (field->cli_value_name[0] != '\0') {
      cdd_escape_comment_str(field->cli_value_name, esc1, sizeof(esc1));
      CHECK_IO(
          fprintf(hfile, "%svalue_name = \"%s\"", c_first ? "" : ", ", esc1));
      c_first = 0;
    }
    if (field->cli_help[0] != '\0') {
      cdd_escape_comment_str(field->cli_help, esc1, sizeof(esc1));
      CHECK_IO(fprintf(hfile, "%shelp = \"%s\"", c_first ? "" : ", ", esc1));
      c_first = 0;
    }
    if (field->cli_action != CDD_CLI_ACTION_AUTO) {
      const char *act_str = NULL;
      cdd_cli_action_to_str(field->cli_action, &act_str);
      CHECK_IO(
          fprintf(hfile, "%saction = \"%s\"", c_first ? "" : ", ", act_str));
      c_first = 0;
    }
    if (field->cli_value_delimiter != '\0') {
      CHECK_IO(fprintf(hfile, "%svalue_delimiter = '%c'", c_first ? "" : ", ",
                       field->cli_value_delimiter));
      c_first = 0;
    }
    if (field->cli_subcommand) {
      CHECK_IO(fprintf(hfile, "%ssubcommand", c_first ? "" : ", "));
      c_first = 0;
    }
    for (a_idx = 0; a_idx < field->n_cli_conflicts_with; ++a_idx) {
      cdd_escape_comment_str(field->cli_conflicts_with[a_idx], esc1,
                             sizeof(esc1));
      CHECK_IO(fprintf(hfile, "%sconflicts_with = \"%s\"", c_first ? "" : ", ",
                       esc1));
      c_first = 0;
    }
    for (a_idx = 0; a_idx < field->n_cli_requires; ++a_idx) {
      cdd_escape_comment_str(field->cli_requires[a_idx], esc1, sizeof(esc1));
      CHECK_IO(
          fprintf(hfile, "%srequires = \"%s\"", c_first ? "" : ", ", esc1));
      c_first = 0;
    }
    CHECK_IO(fprintf(hfile, ")"));
  }
  CHECK_IO(fprintf(hfile, " */\n"));
  return CDD_C_SUCCESS;
}

/*
 * @brief Emits a C struct declaration and JSON/utility prototypes to a header
 * file.
 *
 * @param[in,out] hfile Output header file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields descriptor container.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t write_struct_declaration_h(FILE *hfile, const char *struct_name,
                                         const struct StructFields *sf,
                                         const struct CodegenConfig *config) {
  size_t i;

  if (!hfile || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (sf->serde_rename[0] != '\0' ||
      sf->serde_rename_all != CDD_SERDE_CASE_NONE ||
      sf->serde_rename_all_ser != CDD_SERDE_CASE_NONE ||
      sf->serde_rename_all_de != CDD_SERDE_CASE_NONE ||
      sf->serde_replace_from[0] != '\0' || sf->serde_prefix[0] != '\0' ||
      sf->serde_suffix[0] != '\0' || sf->serde_deny_unknown_fields ||
      sf->serde_default || sf->serde_tag[0] != '\0' ||
      sf->serde_content[0] != '\0' || sf->serde_untagged ||
      sf->is_cli_command || sf->cli_name[0] != '\0' ||
      sf->cli_version[0] != '\0' || sf->cli_about[0] != '\0' ||
      sf->cli_author[0] != '\0' || sf->cli_rename_all != CDD_SERDE_CASE_NONE ||
      sf->cli_env_prefix[0] != '\0') {
    int first_attr = 1;
    char esc1[256];
    char esc2[256];
    CHECK_IO(fprintf(hfile, "/* @cdd "));
    if (sf->serde_rename[0] != '\0') {
      cdd_escape_comment_str(sf->serde_rename, esc1, sizeof(esc1));
      CHECK_IO(fprintf(hfile, "rename = \"%s\"", esc1));
      first_attr = 0;
    }
    if (sf->serde_rename_all != CDD_SERDE_CASE_NONE) {
      const char *c_str = NULL;
      cdd_serde_case_rule_to_str(sf->serde_rename_all, &c_str);
      CHECK_IO(fprintf(hfile, "%srename_all = \"%s\"", first_attr ? "" : ", ",
                       c_str));
      first_attr = 0;
    }
    if (sf->serde_rename_all_ser != CDD_SERDE_CASE_NONE) {
      const char *c_str = NULL;
      cdd_serde_case_rule_to_str(sf->serde_rename_all_ser, &c_str);
      CHECK_IO(fprintf(hfile, "%srename_all_serialize = \"%s\"",
                       first_attr ? "" : ", ", c_str));
      first_attr = 0;
    }
    if (sf->serde_rename_all_de != CDD_SERDE_CASE_NONE) {
      const char *c_str = NULL;
      cdd_serde_case_rule_to_str(sf->serde_rename_all_de, &c_str);
      CHECK_IO(fprintf(hfile, "%srename_all_deserialize = \"%s\"",
                       first_attr ? "" : ", ", c_str));
      first_attr = 0;
    }
    if (sf->serde_replace_from[0] != '\0') {
      cdd_escape_comment_str(sf->serde_replace_from, esc1, sizeof(esc1));
      cdd_escape_comment_str(sf->serde_replace_to, esc2, sizeof(esc2));
      CHECK_IO(fprintf(hfile, "%sreplace_all = (\"%s\" -> \"%s\")",
                       first_attr ? "" : ", ", esc1, esc2));
      first_attr = 0;
    }
    if (sf->serde_prefix[0] != '\0') {
      cdd_escape_comment_str(sf->serde_prefix, esc1, sizeof(esc1));
      CHECK_IO(
          fprintf(hfile, "%sprefix = \"%s\"", first_attr ? "" : ", ", esc1));
      first_attr = 0;
    }
    if (sf->serde_suffix[0] != '\0') {
      cdd_escape_comment_str(sf->serde_suffix, esc1, sizeof(esc1));
      CHECK_IO(
          fprintf(hfile, "%ssuffix = \"%s\"", first_attr ? "" : ", ", esc1));
      first_attr = 0;
    }
    if (sf->serde_deny_unknown_fields) {
      CHECK_IO(fprintf(hfile, "%sdeny_unknown_fields", first_attr ? "" : ", "));
      first_attr = 0;
    }
    if (sf->serde_default) {
      CHECK_IO(fprintf(hfile, "%sdefault", first_attr ? "" : ", "));
      first_attr = 0;
    }
    if (sf->serde_tag[0] != '\0') {
      cdd_escape_comment_str(sf->serde_tag, esc1, sizeof(esc1));
      CHECK_IO(fprintf(hfile, "%stag = \"%s\"", first_attr ? "" : ", ", esc1));
      first_attr = 0;
    }
    if (sf->serde_content[0] != '\0') {
      cdd_escape_comment_str(sf->serde_content, esc1, sizeof(esc1));
      CHECK_IO(
          fprintf(hfile, "%scontent = \"%s\"", first_attr ? "" : ", ", esc1));
      first_attr = 0;
    }
    if (sf->serde_untagged) {
      CHECK_IO(fprintf(hfile, "%suntagged", first_attr ? "" : ", "));
      first_attr = 0;
    }
    if (sf->is_cli_command || sf->cli_name[0] != '\0' ||
        sf->cli_version[0] != '\0' || sf->cli_about[0] != '\0' ||
        sf->cli_author[0] != '\0' ||
        sf->cli_rename_all != CDD_SERDE_CASE_NONE ||
        sf->cli_env_prefix[0] != '\0') {
      int c_first = 1;
      CHECK_IO(fprintf(hfile, "%scli(", first_attr ? "" : ", "));
      if (sf->cli_name[0] != '\0') {
        cdd_escape_comment_str(sf->cli_name, esc1, sizeof(esc1));
        CHECK_IO(fprintf(hfile, "name = \"%s\"", esc1));
        c_first = 0;
      }
      if (sf->cli_version[0] != '\0') {
        cdd_escape_comment_str(sf->cli_version, esc1, sizeof(esc1));
        CHECK_IO(
            fprintf(hfile, "%sversion = \"%s\"", c_first ? "" : ", ", esc1));
        c_first = 0;
      }
      if (sf->cli_author[0] != '\0') {
        cdd_escape_comment_str(sf->cli_author, esc1, sizeof(esc1));
        CHECK_IO(
            fprintf(hfile, "%sauthor = \"%s\"", c_first ? "" : ", ", esc1));
        c_first = 0;
      }
      if (sf->cli_about[0] != '\0') {
        cdd_escape_comment_str(sf->cli_about, esc1, sizeof(esc1));
        CHECK_IO(fprintf(hfile, "%sabout = \"%s\"", c_first ? "" : ", ", esc1));
        c_first = 0;
      }
      if (sf->cli_rename_all != CDD_SERDE_CASE_NONE) {
        const char *c_str = NULL;
        cdd_serde_case_rule_to_str(sf->cli_rename_all, &c_str);
        CHECK_IO(fprintf(hfile, "%srename_all = \"%s\"", c_first ? "" : ", ",
                         c_str));
        c_first = 0;
      }
      if (sf->cli_env_prefix[0] != '\0') {
        cdd_escape_comment_str(sf->cli_env_prefix, esc1, sizeof(esc1));
        CHECK_IO(
            fprintf(hfile, "%senv_prefix = \"%s\"", c_first ? "" : ", ", esc1));
        c_first = 0;
      }
      CHECK_IO(fprintf(hfile, ")"));
    }
    CHECK_IO(fprintf(hfile, " */\n"));
  }

  CHECK_IO(fprintf(hfile, "struct %s {\n", struct_name));
  if (sf->size == 0) {
    CHECK_IO(fprintf(hfile, "  char _dummy;\n"));
  }
  for (i = 0; i < sf->size; i++) {
    const struct StructField *field = &sf->fields[i];
    const char *n = field->name;
    const char *t = field->type;
    const char *r = field->ref;
    const char *ref_type = NULL;
    cdd_c_error_t rc_ref;
    cdd_c_error_t rc_cmt;

    if (strcmp(t, "array") == 0) {
      CHECK_IO(fprintf(hfile, "  size_t n_%s;\n", n));
      rc_cmt = emit_field_serde_comment(hfile, field);
      if (rc_cmt != CDD_C_SUCCESS)
        return rc_cmt;
      if (strcmp(r, "string") == 0) {
        CHECK_IO(fprintf(hfile, "  char **%s;\n", n));
      } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
        CHECK_IO(fprintf(hfile, "  int *%s;\n", n));
      } else if (strcmp(r, "number") == 0) {
        CHECK_IO(fprintf(hfile, "  double *%s;\n", n));
      } else {
        rc_ref = get_type_from_ref(r, &ref_type);
        if (rc_ref != CDD_C_SUCCESS)
          return rc_ref;
        CHECK_IO(fprintf(hfile, "  struct %s **%s;\n", ref_type, n));
      }
      continue;
    }

    rc_cmt = emit_field_serde_comment(hfile, field);
    if (rc_cmt != CDD_C_SUCCESS)
      return rc_cmt;

    if (strcmp(t, "string") == 0) {
      CHECK_IO(fprintf(hfile, "  const char *%s;\n", n));
    } else if (strcmp(t, "integer") == 0) {
      CHECK_IO(fprintf(hfile, "  int %s;\n", n));
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(fprintf(hfile, "  double %s;\n", n));
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(fprintf(hfile, "  int %s;\n", n));
    } else if (strcmp(t, "enum") == 0) {
      rc_ref = get_type_from_ref(r, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(fprintf(hfile, "  enum %s %s;\n", ref_type, n));
    } else if (strcmp(t, "object") == 0) {
      rc_ref = get_type_from_ref(r, &ref_type);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(fprintf(hfile, "  struct %s *%s;\n", ref_type, n));
    } else {
      CHECK_IO(fprintf(hfile, "  void *%s;\n", n));
    }
  }
  CHECK_IO(fprintf(hfile, "%s", "};\n\n"));
  /* Prototypes */
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->json_guard));
  }
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_from_json(const "
                   "char *, struct %s **);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT cdd_c_error_t %s_array_from_json(const char *, "
      "struct %s ***, size_t *);\n",
      struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_to_json(const "
                   "struct %s *, char **);\n",
                   struct_name, struct_name));
  if (config && config->json_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
  }

  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#ifdef %s\n", config->utils_guard));
  }
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_cleanup(struct %s *);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile, "extern LIB_EXPORT cdd_c_error_t %s_default(struct %s **);\n",
      struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_deepcopy(const "
                   "struct %s *, struct %s **);\n",
                   struct_name, struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_eq(const struct "
                   "%s *, const struct %s *, int *out_eq);\n",
                   struct_name, struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT cdd_c_error_t %s_debug(const struct %s *, FILE *);\n",
      struct_name, struct_name));
  CHECK_IO(fprintf(hfile,
                   "extern LIB_EXPORT cdd_c_error_t %s_display(const struct %s "
                   "*, FILE *);\n",
                   struct_name, struct_name));
  CHECK_IO(fprintf(hfile, "struct json_object_t;\n"));
  CHECK_IO(
      fprintf(hfile,
              "extern LIB_EXPORT cdd_c_error_t %s_from_jsonObject(const struct "
              "json_object_t *, struct %s **);\n",
              struct_name, struct_name));
  CHECK_IO(fprintf(
      hfile,
      "extern LIB_EXPORT cdd_c_error_t %s_to_jsonObject(const struct %s *, "
      "struct json_object_t **);\n",
      struct_name, struct_name));
  if (config && config->utils_guard) {
    CHECK_IO(fprintf(hfile, "#endif\n"));
  }

  if (sf->is_cli_command || (config && config->emit_cli)) {
    struct CodegenCliConfig cli_cfg;
    cdd_c_error_t rc_cli;
    memset(&cli_cfg, 0, sizeof(cli_cfg));
    if (config && config->cli_guard)
      cli_cfg.guard_macro = config->cli_guard;
    rc_cli = write_struct_cli_declarations_h(hfile, struct_name, &cli_cfg);
    if (rc_cli != CDD_C_SUCCESS)
      return rc_cli;
  }

  return CDD_C_SUCCESS;
}
