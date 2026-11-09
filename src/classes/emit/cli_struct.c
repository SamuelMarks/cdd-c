/**
 * @file cli_struct.c
 * @brief Implementation of CLI field resolution, Levenshtein typo suggestions,
 * response file expansion, declarations, and version code generation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "c_cdd/reflect.h"
#include "classes/emit/cli_struct.h"
#include "classes/emit/cli_struct_internal.h"
#include "classes/parse/cdd_serde.h"
#include "functions/parse/str.h"
/* clang-format on */

/*
 * @brief Copies `src` into `dst` with bounds checking.
 */
cdd_c_error_t cdd_cli_copy_str(char *dst, size_t dst_sz, const char *src) {
  size_t len = strlen(src);
  if (len >= dst_sz)
    len = dst_sz - 1;
  memcpy(dst, src, len);
  dst[len] = '\0';
  return CDD_C_SUCCESS;
}

/*
 * @brief Determines the effective `enum CddCliAction` for a field.
 */
cdd_c_error_t cdd_cli_effective_action(const struct StructField *f,
                                       enum CddCliAction *out_action) {
  if (f->cli_action != CDD_CLI_ACTION_AUTO)
    *out_action = f->cli_action;
  else if (strcmp(f->type, "boolean") == 0 && !f->cli_positional)
    *out_action = CDD_CLI_ACTION_SET_TRUE;
  else if (strcmp(f->type, "array") == 0)
    *out_action = CDD_CLI_ACTION_APPEND;
  else
    *out_action = CDD_CLI_ACTION_SET;
  return CDD_C_SUCCESS;
}

/*
 * @brief Looks up a `TypeDefinition` by name in an optional `TypeDefList`.
 */
cdd_c_error_t cdd_cli_find_type_def(const struct TypeDefList *reg,
                                    const char *name,
                                    enum TypeDefinitionKind kind,
                                    const struct TypeDefinition **out_def) {
  size_t i;
  *out_def = NULL;
  if (!reg)
    return CDD_C_SUCCESS;
  for (i = 0; i < reg->size; ++i) {
    if (reg->items[i].kind == kind && strcmp(reg->items[i].name, name) == 0) {
      *out_def = &reg->items[i];
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Resolves the effective CLI long option name (without leading `--`)
 * for a struct field.
 */
cdd_c_error_t cdd_cli_resolve_field_long(const struct StructFields *sf,
                                         const struct StructField *f,
                                         char *out_long, size_t out_sz) {
  enum CddSerdeCaseRule rule = CDD_SERDE_CASE_KEBABCASE;
  if (!f || !out_long || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  out_long[0] = '\0';
  if (f->cli_positional || f->cli_subcommand || f->serde_flatten ||
      f->serde_skip || f->cli_skip || f->cli_has_long < 0) {
    return CDD_C_SUCCESS;
  }
  if (f->cli_long[0] != '\0') {
    return cdd_cli_copy_str(out_long, out_sz, f->cli_long);
  }
  if (f->serde_rename[0] != '\0') {
    return cdd_cli_copy_str(out_long, out_sz, f->serde_rename);
  }
  if (sf && sf->cli_rename_all != CDD_SERDE_CASE_NONE) {
    rule = sf->cli_rename_all;
  } else if (f->serde_case != CDD_SERDE_CASE_NONE) {
    rule = f->serde_case;
  }
  return cdd_serde_transform_case(f->name, rule, out_long, out_sz);
}

/*
 * @brief Resolves the effective environment variable name for a struct field.
 */
cdd_c_error_t cdd_cli_resolve_field_env(const struct StructFields *sf,
                                        const struct StructField *f,
                                        char *out_env, size_t out_sz) {
  char base_env[64];
  enum CddSerdeCaseRule env_rule = CDD_SERDE_CASE_SCREAMING_SNAKECASE;
  cdd_c_error_t rc;

  if (!f || !out_env || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  out_env[0] = '\0';
  if (f->cli_env[0] == '\0')
    return CDD_C_SUCCESS;

  if (sf && sf->cli_rename_all_env != CDD_SERDE_CASE_NONE)
    env_rule = sf->cli_rename_all_env;

  if (strcmp(f->cli_env, "1") == 0 || strcmp(f->cli_env, "true") == 0) {
    rc =
        cdd_serde_transform_case(f->name, env_rule, base_env, sizeof(base_env));
  } else {
    rc = cdd_cli_copy_str(base_env, sizeof(base_env), f->cli_env);
  }

  if (sf && sf->cli_env_prefix[0] != '\0') {
    size_t p_len = strlen(sf->cli_env_prefix);
    if (strncmp(base_env, sf->cli_env_prefix, p_len) != 0) {
      size_t b_len = strlen(base_env);
      if (p_len + b_len + 1 > out_sz)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      memcpy(out_env, sf->cli_env_prefix, p_len);
      memcpy(out_env + p_len, base_env, b_len + 1);
      return rc;
    }
  }
  return cdd_cli_copy_str(out_env, out_sz, base_env);
}

/*
 * @brief Resolves the display placeholder `<VALUE_NAME>` for a struct field.
 */
cdd_c_error_t cdd_cli_resolve_value_name(const struct StructField *f,
                                         char *out_val_name, size_t out_sz) {
  if (!f || !out_val_name || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (f->cli_value_name[0] != '\0') {
    return cdd_cli_copy_str(out_val_name, out_sz, f->cli_value_name);
  }
  return cdd_serde_transform_case(f->name, CDD_SERDE_CASE_SCREAMING_SNAKECASE,
                                  out_val_name, out_sz);
}

/**
 * @brief Computes Levenshtein edit distance between two strings `s1` and `s2`
 * (capped at 63 chars each).
 *
 * @param[in] s1 First string.
 * @param[in] s2 Second string.
 * @param[out] out_dist Edit distance output pointer.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t cli_edit_distance(const char *s1, const char *s2,
                                       size_t *out_dist) {
  size_t len1 = strlen(s1);
  size_t len2 = strlen(s2);
  size_t dp[65];
  size_t i, j;

  if (len1 > 64)
    len1 = 64;
  if (len2 > 64)
    len2 = 64;

  for (j = 0; j <= len2; ++j)
    dp[j] = j;

  for (i = 1; i <= len1; ++i) {
    size_t prev = dp[0];
    dp[0] = i;
    for (j = 1; j <= len2; ++j) {
      size_t temp = dp[j];
      size_t cost = (s1[i - 1] == s2[j - 1]) ? 0u : 1u;
      size_t del_cost = dp[j] + 1u;
      size_t ins_cost = dp[j - 1] + 1u;
      size_t sub_cost = prev + cost;
      size_t min_val = del_cost < ins_cost ? del_cost : ins_cost;
      if (sub_cost < min_val)
        min_val = sub_cost;
      /* Check adjacent transposition (Damerau-Levenshtein) */
      if (i > 1 && j > 1 && s1[i - 1] == s2[j - 2] && s1[i - 2] == s2[j - 1]) {
        if (prev < min_val)
          min_val = prev;
      }
      dp[j] = min_val;
      prev = temp;
    }
  }
  *out_dist = dp[len2];
  return CDD_C_SUCCESS;
}

/*
 * @brief Computes a bounded Levenshtein edit distance to suggest the closest
 * matching CLI flag or subcommand when an unknown token is encountered.
 */
cdd_c_error_t cdd_cli_suggest_closest(const char *unknown_arg,
                                      const char *const *candidates,
                                      size_t n_candidates,
                                      const char **out_best) {
  size_t i;
  size_t best_dist = 999;
  const char *best = NULL;
  const char *query = unknown_arg;
  char clean_query[64];
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!unknown_arg || !out_best || (n_candidates > 0 && !candidates))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_best = NULL;
  if (n_candidates == 0 || unknown_arg[0] == '\0')
    return CDD_C_SUCCESS;

  /* Strip =value suffix if present */
  {
    const char *eq = strchr(unknown_arg, '=');
    if (eq) {
      size_t qlen = (size_t)(eq - unknown_arg);
      if (qlen >= sizeof(clean_query))
        qlen = sizeof(clean_query) - 1;
      memcpy(clean_query, unknown_arg, qlen);
      clean_query[qlen] = '\0';
      query = clean_query;
    }
  }

  for (i = 0; i < n_candidates; ++i) {
    size_t d = 999;
    size_t max_allowed = 2;
    size_t cand_len;
    if (!candidates[i] || candidates[i][0] == '\0')
      continue;
    cand_len = strlen(candidates[i]);
    if (cand_len <= 3)
      max_allowed = 1;
    else if (cand_len >= 10)
      max_allowed = 3;
    rc = cli_edit_distance(query, candidates[i], &d);
    if (d <= max_allowed && d < best_dist) {
      best_dist = d;
      best = candidates[i];
    }
  }

  *out_best = best;
  return rc;
}

/**
 * @brief Appends a single argument string to a dynamic `(argc, argv)` vector.
 *
 * @param[in,out] argc_ptr Pointer to argument count.
 * @param[in,out] cap_ptr Pointer to allocated capacity.
 * @param[in,out] argv_ptr Pointer to argument array.
 * @param[in] token Argument string to duplicate and append.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static cdd_c_error_t cli_argv_push(int *argc_ptr, int *cap_ptr,
                                   char ***argv_ptr, const char *token) {
  char *dup = NULL;
  cdd_c_error_t rc;
  if (*argc_ptr + 1 >= *cap_ptr) {
    int new_cap = (*cap_ptr == 0) ? 16 : (*cap_ptr * 2);
    char **new_arr =
        (char **)C_CDD_REALLOC(*argv_ptr, (size_t)new_cap * sizeof(char *));
    if (!new_arr)
      return CDD_C_ERROR_MEMORY;
    *argv_ptr = new_arr;
    *cap_ptr = new_cap;
  }
  rc = c_cdd_strdup(token, &dup);
  if (rc != CDD_C_SUCCESS)
    return rc;
  (*argv_ptr)[*argc_ptr] = dup;
  (*argc_ptr)++;
  (*argv_ptr)[*argc_ptr] = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Recursive helper for expanding `@file.rsp` response files.
 *
 * @param[in] path Path to the response file.
 * @param[in] depth Current recursion depth (fails if > 16).
 * @param[in,out] argc_ptr Pointer to expanded argument count.
 * @param[in,out] cap_ptr Pointer to allocated capacity.
 * @param[in,out] argv_ptr Pointer to expanded argument array.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
static cdd_c_error_t cli_expand_rsp_file(const char *path, int depth,
                                         int *argc_ptr, int *cap_ptr,
                                         char ***argv_ptr) {
  FILE *fp = NULL;
  long fsize;
  char *buf = NULL;
  size_t nread;
  const char *p;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (depth > 16)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#if defined(_MSC_VER)
  if (fopen_s(&fp, path, "rb") != 0)
    fp = NULL;
#else
  fp = fopen(path, "rb");
#endif
  if (!fp)
    return CDD_C_ERROR_NOT_FOUND;

  fseek(fp, 0, SEEK_END);
  fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  buf = (char *)C_CDD_MALLOC((size_t)fsize + 1);
  if (!buf) {
    fclose(fp);
    return CDD_C_ERROR_MEMORY;
  }
  nread = fread(buf, 1, (size_t)fsize, fp);
  buf[nread] = '\0';
  fclose(fp);

  p = buf;
  while (*p != '\0') {
    char tok[1024];
    size_t tpos = 0;
    char quote = 0;

    while (isspace((unsigned char)*p))
      p++;
    if (*p == '\0')
      break;
    if (*p == '#') {
      while (*p != '\0' && *p != '\n')
        p++;
      continue;
    }

    while (*p != '\0') {
      if (quote) {
        if (*p == '\\') {
          p++;
          tok[tpos++] = *p++;
          continue;
        }
        if (*p == quote) {
          quote = 0;
          p++;
          continue;
        }
        tok[tpos++] = *p++;
      } else {
        if (isspace((unsigned char)*p))
          break;
        if (*p == '"' || *p == '\'') {
          quote = *p++;
          continue;
        }
        if (*p == '\\') {
          p++;
          tok[tpos++] = *p++;
          continue;
        }
        tok[tpos++] = *p++;
      }
    }
    tok[tpos] = '\0';

    if (tok[0] == '@') {
      rc = cli_expand_rsp_file(tok + 1, depth + 1, argc_ptr, cap_ptr, argv_ptr);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(buf);
        return rc;
      }
    } else {
      rc = cli_argv_push(argc_ptr, cap_ptr, argv_ptr, tok);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(buf);
        return rc;
      }
    }
  }

  C_CDD_FREE(buf);
  return CDD_C_SUCCESS;
}

/*
 * @brief Expands `@file.rsp` response file arguments in an `argv` vector.
 */
cdd_c_error_t cdd_cli_expand_response_files(int argc, char **argv,
                                            int *out_argc, char ***out_argv) {
  int i;
  int new_argc = 0;
  int cap = 0;
  char **new_argv = NULL;
  cdd_c_error_t rc;

  if (argc < 0 || (argc > 0 && !argv) || !out_argc || !out_argv)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_argc = 0;
  *out_argv = NULL;

  for (i = 0; i < argc; ++i) {
    const char *arg = argv[i];
    if (!arg)
      continue;
    if (i > 0 && arg[0] == '@' && arg[1] != '\0') {
      rc = cli_expand_rsp_file(arg + 1, 1, &new_argc, &cap, &new_argv);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_cli_free_argv(new_argc, new_argv);
        return rc;
      }
    } else {
      rc = cli_argv_push(&new_argc, &cap, &new_argv, arg);
      if (rc != CDD_C_SUCCESS) {
        g_cdd_last_alloc_err = cdd_cli_free_argv(new_argc, new_argv);
        return rc;
      }
    }
  }

  *out_argc = new_argc;
  *out_argv = new_argv;
  return CDD_C_SUCCESS;
}

/*
 * @brief Frees an argument vector allocated by `cdd_cli_expand_response_files`
 * or `<Struct>_to_cli_argv`.
 */
cdd_c_error_t cdd_cli_free_argv(int argc, char **argv) {
  int i;
  if (!argv)
    return CDD_C_SUCCESS;
  for (i = 0; i < argc; ++i) {
    C_CDD_FREE(argv[i]);
  }
  C_CDD_FREE(argv);
  return CDD_C_SUCCESS;
}

/*
 * @brief Parses CLI flags from `(argc, argv)` starting at `start_idx` into a
 * reflected struct `target` using `ti` field metadata.
 */
cdd_c_error_t cdd_cli_parse_reflected_argv(int argc, char **argv, int start_idx,
                                           const struct cdd_type_info *ti,
                                           void *target, int *out_help,
                                           int *out_first_unknown) {
  int i;
  if (out_help)
    *out_help = 0;
  if (out_first_unknown)
    *out_first_unknown = -1;
  if (!ti || !target || start_idx < 0 || (argc > start_idx && !argv))
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = start_idx; i < argc; ++i) {
    const char *arg = argv[i];
    const struct cdd_field_info *f = NULL;
    const char *flag;
    char key_buf[64];
    const char *eq_val = NULL;

    if (!arg)
      continue;
    if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
      if (out_help)
        *out_help = 1;
      return CDD_C_SUCCESS;
    }
    if (arg[0] != '-') {
      if (out_first_unknown && *out_first_unknown < 0)
        *out_first_unknown = i;
      continue;
    }

    flag = arg + (arg[1] == '-' ? 2 : 1);
    {
      const char *eq = strchr(flag, '=');
      if (eq) {
        size_t klen = (size_t)(eq - flag);
        if (klen >= sizeof(key_buf))
          klen = sizeof(key_buf) - 1;
        memcpy(key_buf, flag, klen);
        key_buf[klen] = '\0';
        flag = key_buf;
        eq_val = eq + 1;
      }
    }

    if (cdd_reflect_find_field(ti, flag, &f) == CDD_C_SUCCESS) {
      if (f->kind == CDD_FIELD_KIND_BOOLEAN) {
        int bval = 1;
        if (eq_val &&
            (strcmp(eq_val, "0") == 0 || strcmp(eq_val, "false") == 0)) {
          bval = 0;
        }
        memcpy((unsigned char *)target + f->offset, &bval, sizeof(bval));
      } else if (f->kind == CDD_FIELD_KIND_INTEGER) {
        const char *val_str = eq_val;
        int parsed_int;
        if (!val_str) {
          if (i + 1 >= argc)
            return CDD_C_ERROR_INVALID_ARGUMENT;
          val_str = argv[++i];
        }
        parsed_int = atoi(val_str);
        if (f->size == sizeof(size_t)) {
          size_t sz_val = (size_t)parsed_int;
          memcpy((unsigned char *)target + f->offset, &sz_val, sizeof(sz_val));
        } else {
          memcpy((unsigned char *)target + f->offset, &parsed_int,
                 sizeof(parsed_int));
        }
      } else {
        const char *val_str = eq_val;
        if (!val_str) {
          if (i + 1 >= argc)
            return CDD_C_ERROR_INVALID_ARGUMENT;
          val_str = argv[++i];
        }
        memcpy((unsigned char *)target + f->offset, &val_str, sizeof(val_str));
      }
    } else {
      if (out_first_unknown && *out_first_unknown < 0)
        *out_first_unknown = i;
    }
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits C prototypes for all generated CLI functions into a header
 * stream.
 */
cdd_c_error_t
write_struct_cli_declarations_h(FILE *fp, const char *struct_name,
                                const struct CodegenCliConfig *config) {
  if (!fp || !struct_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "extern LIB_EXPORT cdd_c_error_t %s_print_version(const char *prog_name, "
      "FILE *out_stream);\n",
      struct_name));
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "extern LIB_EXPORT cdd_c_error_t %s_print_help(const char *prog_name, "
      "FILE *out_stream);\n",
      struct_name));
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "extern LIB_EXPORT cdd_c_error_t %s_print_completions(const char "
      "*prog_name, const char *shell, FILE *out_stream);\n",
      struct_name));
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "extern LIB_EXPORT cdd_c_error_t %s_parse_cli_into(int argc, char "
      "**argv, struct %s *out, int *out_consumed);\n",
      struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "extern LIB_EXPORT cdd_c_error_t %s_from_cli(int argc, char **argv, "
      "struct %s **out);\n",
      struct_name, struct_name));
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "extern LIB_EXPORT cdd_c_error_t %s_to_cli_argv(const struct %s *obj, "
      "int *out_argc, char ***out_argv);\n",
      struct_name, struct_name));
  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n", config->guard_macro));
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Generates the `<Struct>_print_version` function implementation.
 */
cdd_c_error_t
write_struct_print_version_func(FILE *fp, const char *struct_name,
                                const struct StructFields *sf,
                                const struct CodegenCliConfig *config) {
  char default_prog[64];
  char esc_prog[128];
  char esc_ver[128];
  const char *ver_str;
  cdd_c_error_t rc;

  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (sf->cli_name[0] != '\0') {
    rc = cdd_cli_copy_str(default_prog, sizeof(default_prog), sf->cli_name);
  } else {
    rc = cdd_serde_transform_case(struct_name, CDD_SERDE_CASE_KEBABCASE,
                                  default_prog, sizeof(default_prog));
  }
  ver_str = (sf->cli_version[0] != '\0') ? sf->cli_version : "0.1.0";
  rc = cdd_escape_json_c_str(default_prog, esc_prog, sizeof(esc_prog));
  rc = cdd_escape_json_c_str(ver_str, esc_ver, sizeof(esc_ver));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_print_version(const char *prog_name, FILE *out_stream) "
      "{\n",
      struct_name));
  CHECK_IO(FPRINTF_HOOK(fp, "  const char *_cdd_prog = \"%s\";\n", esc_prog));
  CHECK_IO(FPRINTF_HOOK(fp, "  if (!out_stream)\n"
                            "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  if (sf->cli_name[0] == '\0') {
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
  CHECK_IO(
      FPRINTF_HOOK(fp,
                   "  if (fprintf(out_stream, \"%%s %s\\n\", _cdd_prog) < 0)\n"
                   "    return CDD_C_ERROR_IO;\n"
                   "  return CDD_C_SUCCESS;\n"
                   "}\n\n",
                   esc_ver));
  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));
  }
  return rc;
}

/*
 * @brief Convenience generator that emits all CLI functions for a struct.
 */
cdd_c_error_t
write_struct_cli_all_funcs(FILE *fp, const char *struct_name,
                           const struct StructFields *sf,
                           const struct CodegenCliConfig *config) {
  cdd_c_error_t rc;
  rc = write_struct_print_version_func(fp, struct_name, sf, config);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_struct_print_help_func(fp, struct_name, sf, config);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_struct_print_completions_func(fp, struct_name, sf, config);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_struct_cli_parse_into_func(fp, struct_name, sf, config);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_struct_from_cli_func(fp, struct_name, sf, config);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = write_struct_to_cli_argv_func(fp, struct_name, sf, config);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}
