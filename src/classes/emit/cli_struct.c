/**
 * @file cli_struct.c
 * @brief Implementation of declarative C89 CLI parser, serializer, help,
 * version, typo suggestion, and shell completion code generation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/code_writer.h"
#include "c_cdd/memory.h"
#include "classes/emit/cli_struct.h"
#include "classes/parse/cdd_serde.h"
#include "classes/parse/inspector.h"
#include "functions/parse/str.h"
/* clang-format on */

/** @brief Redirects fprintf through the testable I/O hook */
#define FPRINTF_HOOK cdd_io_fprintf_hook

/** @brief Checks fprintf return value and percolates CDD_C_ERROR_IO */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

/**
 * @brief Copies `src` into `dst` with bounds checking.
 *
 * @param[out] dst Destination buffer.
 * @param[in] dst_sz Capacity of `dst`.
 * @param[in] src Source NUL-terminated string.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t cli_copy_str(char *dst, size_t dst_sz, const char *src) {
  size_t len = strlen(src);
  if (len >= dst_sz)
    len = dst_sz - 1;
  memcpy(dst, src, len);
  dst[len] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines the effective `enum CddCliAction` for a field.
 *
 * @param[in] f Field descriptor.
 * @return Effective `enum CddCliAction`.
 */
static enum CddCliAction cli_effective_action(const struct StructField *f) {
  if (f->cli_action != CDD_CLI_ACTION_AUTO)
    return f->cli_action;
  if (strcmp(f->type, "boolean") == 0 && !f->cli_positional)
    return CDD_CLI_ACTION_SET_TRUE;
  if (strcmp(f->type, "array") == 0)
    return CDD_CLI_ACTION_APPEND;
  return CDD_CLI_ACTION_SET;
}

/**
 * @brief Looks up a `TypeDefinition` by name in an optional `TypeDefList`.
 *
 * @param[in] reg Type registry (may be NULL).
 * @param[in] name Type name to search for.
 * @param[in] kind Expected `TypeDefinitionKind` (`KIND_ENUM` or `KIND_STRUCT`).
 * @param[out] out_def Matching `TypeDefinition` pointer, or NULL if not found.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t cli_find_type_def(const struct TypeDefList *reg,
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
      f->serde_skip || f->cli_has_long < 0) {
    return CDD_C_SUCCESS;
  }
  if (f->cli_long[0] != '\0') {
    return cli_copy_str(out_long, out_sz, f->cli_long);
  }
  if (f->serde_rename[0] != '\0') {
    return cli_copy_str(out_long, out_sz, f->serde_rename);
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

  if (!f || !out_env || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  out_env[0] = '\0';
  if (f->cli_env[0] == '\0')
    return CDD_C_SUCCESS;

  if (sf && sf->cli_rename_all_env != CDD_SERDE_CASE_NONE)
    env_rule = sf->cli_rename_all_env;

  if (strcmp(f->cli_env, "1") == 0 || strcmp(f->cli_env, "true") == 0) {
    cdd_serde_transform_case(f->name, env_rule, base_env, sizeof(base_env));
  } else {
    cli_copy_str(base_env, sizeof(base_env), f->cli_env);
  }

  if (sf && sf->cli_env_prefix[0] != '\0') {
    size_t p_len = strlen(sf->cli_env_prefix);
    if (strncmp(base_env, sf->cli_env_prefix, p_len) != 0) {
      size_t b_len = strlen(base_env);
      if (p_len + b_len + 1 > out_sz)
        return CDD_C_ERROR_INVALID_ARGUMENT;
      memcpy(out_env, sf->cli_env_prefix, p_len);
      memcpy(out_env + p_len, base_env, b_len + 1);
      return CDD_C_SUCCESS;
    }
  }
  return cli_copy_str(out_env, out_sz, base_env);
}

/*
 * @brief Resolves the display placeholder `<VALUE_NAME>` for a struct field.
 */
cdd_c_error_t cdd_cli_resolve_value_name(const struct StructField *f,
                                         char *out_val_name, size_t out_sz) {
  if (!f || !out_val_name || out_sz == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (f->cli_value_name[0] != '\0') {
    return cli_copy_str(out_val_name, out_sz, f->cli_value_name);
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
        cdd_cli_free_argv(new_argc, new_argv);
        return rc;
      }
    } else {
      rc = cli_argv_push(&new_argc, &cap, &new_argv, arg);
      if (rc != CDD_C_SUCCESS) {
        cdd_cli_free_argv(new_argc, new_argv);
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
void cdd_cli_free_argv(int argc, char **argv) {
  int i;
  if (!argv)
    return;
  for (i = 0; i < argc; ++i) {
    C_CDD_FREE(argv[i]);
  }
  C_CDD_FREE(argv);
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
    rc = cli_copy_str(default_prog, sizeof(default_prog), sf->cli_name);
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
  enum CddCliAction act = cli_effective_action(f);
  int takes_val = (act == CDD_CLI_ACTION_SET || act == CDD_CLI_ACTION_APPEND);
  size_t pos = 0;
  cdd_c_error_t rc;

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
    rc = cli_find_type_def(config ? config->type_registry : NULL, ref_type,
                           KIND_ENUM, &edef);
    if (edef) {
      const struct EnumMembers *em = edef->details.enum_members;
      int first_v = 1;
      pos = strlen(out_badges);
      rc = cli_copy_str(out_badges + pos, badges_sz - pos,
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
    rc = cli_copy_str(out_badges + pos, badges_sz - pos, " [aliases: ");
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

  rc = cli_copy_str(line, sizeof(line), left_col);
  pos = left_len;
  if (has_desc || has_badges) {
    size_t spaces = (pad_width > left_len) ? (pad_width - left_len + 2) : 2;
    while (spaces > 0) {
      line[pos++] = ' ';
      spaces--;
    }
    line[pos] = '\0';
    if (has_desc) {
      rc = cli_copy_str(line + pos, sizeof(line) - pos, desc);
      pos = strlen(line);
    }
    if (has_badges) {
      rc = cli_copy_str(line + pos, sizeof(line) - pos, badges);
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
    rc = cli_copy_str(default_prog, sizeof(default_prog), sf->cli_bin_name);
  } else if (sf->cli_name[0] != '\0') {
    rc = cli_copy_str(default_prog, sizeof(default_prog), sf->cli_name);
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
      rc = cli_find_type_def(config ? config->type_registry : NULL, ref_t,
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
          rc = cli_copy_str(headings[n_headings], sizeof(headings[n_headings]),
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
    rc = cli_copy_str(usage_tail, sizeof(usage_tail), " [OPTIONS]");
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
      rc = cli_copy_str(usage_tail + upos, sizeof(usage_tail) - upos,
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
    rc = cli_find_type_def(config ? config->type_registry : NULL, ref_t,
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
          rc = cli_copy_str(cmd_name, sizeof(cmd_name), vf->serde_rename);
        } else {
          rc = cdd_serde_transform_case(vf->name, CDD_SERDE_CASE_KEBABCASE,
                                        cmd_name, sizeof(cmd_name));
        }
        if (cmd_desc[0] == '\0') {
          const char *vref = NULL;
          const struct TypeDefinition *vdef = NULL;
          rc = get_type_from_ref(vf->ref, &vref);
          rc = cli_find_type_def(config->type_registry, vref, KIND_STRUCT,
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
          rc = cli_find_type_def(config ? config->type_registry : NULL, ref_t,
                                 KIND_STRUCT, &cdef);
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
    rc = cli_copy_str(default_prog, sizeof(default_prog), sf->cli_bin_name);
  } else if (sf->cli_name[0] != '\0') {
    rc = cli_copy_str(default_prog, sizeof(default_prog), sf->cli_name);
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
    rc = cli_copy_str(words_buf + wpos, sizeof(words_buf) - wpos,
                      wpos > 0 ? " --help -h" : "--help -h");
  }
  if (!sf->cli_disable_version_flag) {
    wpos = strlen(words_buf);
    rc = cli_copy_str(words_buf + wpos, sizeof(words_buf) - wpos,
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

/**
 * @brief Emits the self-contained C89 CLI runtime helper functions block once
 * per translation unit (guarded by `CDD_CLI_RUNTIME_HELPERS_DEFINED`).
 *
 * @param[in,out] fp Output C stream.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static cdd_c_error_t cli_emit_runtime_helpers(FILE *fp) {
  CHECK_IO(FPRINTF_HOOK(
      fp, "#ifndef CDD_CLI_RUNTIME_HELPERS_DEFINED\n"
          "#define CDD_CLI_RUNTIME_HELPERS_DEFINED\n"
          "static int _cdd_cli_str_ieq(const char *a, const char *b) {\n"
          "  if (!a || !b)\n"
          "    return 0;\n"
          "  while (*a && *b) {\n"
          "    unsigned char ca = (unsigned char)*a;\n"
          "    unsigned char cb = (unsigned char)*b;\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "    if (ca >= 'A' && ca <= 'Z')\n"
                            "      ca = (unsigned char)(ca + ('a' - 'A'));\n"
                            "    if (cb >= 'A' && cb <= 'Z')\n"
                            "      cb = (unsigned char)(cb + ('a' - 'A'));\n"
                            "    if (ca != cb)\n"
                            "      return 0;\n"
                            "    a++;\n"
                            "    b++;\n"
                            "  }\n"
                            "  return (*a == '\\0' && *b == '\\0') ? 1 : 0;\n"
                            "}\n\n"));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "static int _cdd_cli_parse_int(const char *raw, int *out_val) {\n"
      "  char *endptr = NULL;\n"
      "  long val;\n"
      "  if (!raw || raw[0] == '\\0' || !out_val)\n"
      "    return 0;\n"
      "  errno = 0;\n"
      "  val = strtol(raw, &endptr, 10);\n"
      "  if (errno == ERANGE || !endptr || *endptr != '\\0' || endptr == raw)\n"
      "    return 0;\n"
      "  if (val < (long)INT_MIN || val > (long)INT_MAX)\n"
      "    return 0;\n"
      "  *out_val = (int)val;\n"
      "  return 1;\n"
      "}\n\n"));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "static int _cdd_cli_parse_double(const char *raw, double *out_val) {\n"
      "  char *endptr = NULL;\n"
      "  double val;\n"
      "  if (!raw || raw[0] == '\\0' || !out_val)\n"
      "    return 0;\n"
      "  errno = 0;\n"
      "  val = strtod(raw, &endptr);\n"
      "  if (errno == ERANGE || !endptr || *endptr != '\\0' || endptr == raw)\n"
      "    return 0;\n"
      "  *out_val = val;\n"
      "  return 1;\n"
      "}\n\n"));

  CHECK_IO(FPRINTF_HOOK(
      fp, "static int _cdd_cli_parse_bool(const char *raw, int *out_val) {\n"
          "  if (!raw || !out_val)\n"
          "    return 0;\n"
          "  if (_cdd_cli_str_ieq(raw, \"1\") || _cdd_cli_str_ieq(raw, "
          "\"true\") ||\n"
          "      _cdd_cli_str_ieq(raw, \"yes\") || _cdd_cli_str_ieq(raw, "
          "\"on\")) {\n"
          "    *out_val = 1;\n"
          "    return 1;\n"
          "  }\n"
          "  if (_cdd_cli_str_ieq(raw, \"0\") || _cdd_cli_str_ieq(raw, "
          "\"false\") ||\n"
          "      _cdd_cli_str_ieq(raw, \"no\") || _cdd_cli_str_ieq(raw, "
          "\"off\")) {\n"
          "    *out_val = 0;\n"
          "    return 1;\n"
          "  }\n"
          "  return 0;\n"
          "}\n\n"));

  CHECK_IO(FPRINTF_HOOK(
      fp, "static int _cdd_cli_env_get(const char *name, char **out_dup) {\n"
          "#if defined(_MSC_VER)\n"
          "  char *buf = NULL;\n"
          "  size_t sz = 0;\n"
          "  *out_dup = NULL;\n"
          "  if (_dupenv_s(&buf, &sz, name) != 0)\n"
          "    return -1;\n"
          "  *out_dup = buf;\n"
          "  return 0;\n"
          "#else\n"
          "  const char *v = getenv(name);\n"
          "  *out_dup = NULL;\n"
          "  if (!v)\n"
          "    return 0;\n"
          "  *out_dup = strdup(v);\n"
          "  return (*out_dup) ? 0 : -1;\n"
          "#endif\n"
          "}\n\n"));

  CHECK_IO(FPRINTF_HOOK(
      fp, "static const char *_cdd_cli_suggest(const char *unknown, const char "
          "*const *cands, size_t n_cands) {\n"
          "  size_t k, i, j;\n"
          "  size_t best_dist = 999;\n"
          "  const char *best = NULL;\n"
          "  char qbuf[64];\n"
          "  const char *q = unknown;\n"
          "  if (!unknown || n_cands == 0)\n"
          "    return NULL;\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  {\n"
                            "    const char *eq = strchr(unknown, '=');\n"
                            "    if (eq) {\n"
                            "      size_t qlen = (size_t)(eq - unknown);\n"
                            "      if (qlen >= sizeof(qbuf))\n"
                            "        qlen = sizeof(qbuf) - 1;\n"
                            "      memcpy(qbuf, unknown, qlen);\n"
                            "      qbuf[qlen] = '\\0';\n"
                            "      q = qbuf;\n"
                            "    }\n"
                            "  }\n"));
  CHECK_IO(FPRINTF_HOOK(
      fp, "  for (k = 0; k < n_cands; ++k) {\n"
          "    size_t l1 = strlen(q);\n"
          "    size_t l2 = cands[k] ? strlen(cands[k]) : 0;\n"
          "    size_t dp[65];\n"
          "    size_t max_d = (l2 <= 3) ? 1u : ((l2 >= 10) ? 3u : 2u);\n"
          "    if (!cands[k] || l1 > 64 || l2 > 64)\n"
          "      continue;\n"
          "    for (j = 0; j <= l2; ++j)\n"
          "      dp[j] = j;\n"));
  CHECK_IO(FPRINTF_HOOK(
      fp, "    for (i = 1; i <= l1; ++i) {\n"
          "      size_t prev = dp[0];\n"
          "      dp[0] = i;\n"
          "      for (j = 1; j <= l2; ++j) {\n"
          "        size_t tmp = dp[j];\n"
          "        size_t cost = (q[i - 1] == cands[k][j - 1]) ? 0u : 1u;\n"
          "        size_t m = (dp[j] + 1u < dp[j - 1] + 1u) ? (dp[j] + 1u) : "
          "(dp[j - 1] + 1u);\n"));
  CHECK_IO(FPRINTF_HOOK(fp,
                        "        if (prev + cost < m)\n"
                        "          m = prev + cost;\n"
                        "        dp[j] = m;\n"
                        "        prev = tmp;\n"
                        "      }\n"
                        "    }\n"
                        "    if (dp[l2] <= max_d && dp[l2] < best_dist) {\n"
                        "      best_dist = dp[l2];\n"
                        "      best = cands[k];\n"
                        "    }\n"
                        "  }\n"
                        "  return best;\n"
                        "}\n"
                        "#endif /* CDD_CLI_RUNTIME_HELPERS_DEFINED */\n\n"));
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits C89 statements to parse a single scalar or array element string
 * `_raw_val` into `ret-><field>`.
 *
 * @param[in,out] fp Output C stream.
 * @param[in] f Field descriptor.
 * @param[in] raw_expr C expression evaluating to `const char *` value.
 * @param[in] err_stmt C statement to execute on parse error.
 * @param[in] oom_stmt C statement to execute on OOM error.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static cdd_c_error_t cli_emit_field_value_assign(FILE *fp,
                                                 const struct StructField *f,
                                                 const char *raw_expr,
                                                 const char *err_stmt,
                                                 const char *oom_stmt) {
  const char *n = f->name;
  const char *t = f->type;
  const char *r = f->ref;
  const char *ref_t = NULL;

  if (f->cli_value_parser[0] != '\0') {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "        if (%s(%s, &ret->%s) != CDD_C_SUCCESS) {\n"
                          "          %s\n"
                          "        }\n",
                          f->cli_value_parser, raw_expr, n, err_stmt));
    return CDD_C_SUCCESS;
  }

  if (strcmp(t, "string") == 0) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "        {\n"
                          "          char *_dup = strdup(%s);\n"
                          "          if (!_dup) {\n"
                          "            %s\n"
                          "          }\n"
                          "          if (ret->%s)\n"
                          "            free((void *)ret->%s);\n"
                          "          ret->%s = _dup;\n"
                          "        }\n",
                          raw_expr, oom_stmt, n, n, n));
  } else if (strcmp(t, "integer") == 0) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "        if (!_cdd_cli_parse_int(%s, &ret->%s)) {\n"
                          "          %s\n"
                          "        }\n",
                          raw_expr, n, err_stmt));
  } else if (strcmp(t, "number") == 0) {
    CHECK_IO(
        FPRINTF_HOOK(fp,
                     "        if (!_cdd_cli_parse_double(%s, &ret->%s)) {\n"
                     "          %s\n"
                     "        }\n",
                     raw_expr, n, err_stmt));
  } else if (strcmp(t, "boolean") == 0) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "        if (!_cdd_cli_parse_bool(%s, &ret->%s)) {\n"
                          "          %s\n"
                          "        }\n",
                          raw_expr, n, err_stmt));
  } else if (strcmp(t, "enum") == 0) {
    cdd_c_error_t rc_ref = get_type_from_ref(r, &ref_t);
    if (rc_ref != CDD_C_SUCCESS)
      return rc_ref;
    CHECK_IO(FPRINTF_HOOK(fp,
                          "        if (%s_from_str(%s, &ret->%s) != 0) {\n"
                          "          %s\n"
                          "        }\n",
                          ref_t, raw_expr, n, err_stmt));
  } else {
    char delim = f->cli_value_delimiter;
    CHECK_IO(FPRINTF_HOOK(fp,
                          "        {\n"
                          "          const char *_cur = %s;\n"
                          "          while (_cur) {\n"
                          "            char _tok[512];\n"
                          "            const char *_next = %s",
                          raw_expr, delim ? "strchr(_cur, '" : "NULL;\n"));
    if (delim) {
      CHECK_IO(FPRINTF_HOOK(fp, "%c');\n", delim));
    }
    CHECK_IO(FPRINTF_HOOK(
        fp, "            if (_next) {\n"
            "              size_t _tlen = (size_t)(_next - _cur);\n"
            "              if (_tlen >= sizeof(_tok))\n"
            "                _tlen = sizeof(_tok) - 1;\n"
            "              memcpy(_tok, _cur, _tlen);\n"
            "              _tok[_tlen] = '\\0';\n"
            "              _cur = _next + 1;\n"
            "            } else {\n"
            "              strncpy(_tok, _cur, sizeof(_tok) - 1);\n"
            "              _tok[sizeof(_tok) - 1] = '\\0';\n"
            "              _cur = NULL;\n"
            "            }\n"));
    if (strcmp(r, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "            {\n"
          "              char **_narr = (char **)realloc((void *)ret->%s, "
          "(ret->n_%s + 1) * sizeof(char *));\n"
          "              char *_edup;\n"
          "              if (!_narr) {\n"
          "                %s\n"
          "              }\n"
          "              ret->%s = _narr;\n"
          "              _edup = strdup(_tok);\n"
          "              if (!_edup) {\n"
          "                %s\n"
          "              }\n"
          "              ret->%s[ret->n_%s++] = _edup;\n"
          "            }\n",
          n, n, oom_stmt, n, oom_stmt, n, n));
    } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "            {\n"
          "              int _iv = 0;\n"
          "              int *_narr;\n"
          "              if (!%s(_tok, &_iv)) {\n"
          "                %s\n"
          "              }\n"
          "              _narr = (int *)realloc(ret->%s, (ret->n_%s + 1) * "
          "sizeof(int));\n"
          "              if (!_narr) {\n"
          "                %s\n"
          "              }\n"
          "              ret->%s = _narr;\n"
          "              ret->%s[ret->n_%s++] = _iv;\n"
          "            }\n",
          strcmp(r, "boolean") == 0 ? "_cdd_cli_parse_bool"
                                    : "_cdd_cli_parse_int",
          err_stmt, n, n, oom_stmt, n, n, n));
    } else if (strcmp(r, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "            {\n"
          "              double _dv = 0.0;\n"
          "              double *_narr;\n"
          "              if (!_cdd_cli_parse_double(_tok, &_dv)) {\n"
          "                %s\n"
          "              }\n"
          "              _narr = (double *)realloc(ret->%s, (ret->n_%s + 1) * "
          "sizeof(double));\n"
          "              if (!_narr) {\n"
          "                %s\n"
          "              }\n"
          "              ret->%s = _narr;\n"
          "              ret->%s[ret->n_%s++] = _dv;\n"
          "            }\n",
          err_stmt, n, n, oom_stmt, n, n, n));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "          }\n"
                              "        }\n"));
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Generates the `<Struct>_parse_cli_into` function implementation.
 */
cdd_c_error_t
write_struct_cli_parse_into_func(FILE *fp, const char *struct_name,
                                 const struct StructFields *sf,
                                 const struct CodegenCliConfig *config) {
  size_t i, j;
  int has_subcmd = 0;
  const struct StructField *subcmd_f = NULL;
  cdd_c_error_t rc;

  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = cli_emit_runtime_helpers(fp);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_parse_cli_into(int argc, char **argv, struct %s *ret, "
      "int *out_consumed) {\n"
      "  int _i;\n"
      "  int _end_of_opts = 0;\n"
      "  int _pos_idx = 0;\n"
      "  int _total_cli_seen = 0;\n"
      "  char *_env_val = NULL;\n"
      "  cdd_c_error_t _rc = CDD_C_SUCCESS;\n",
      struct_name, struct_name));

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    if (f->serde_skip)
      continue;
    if (f->cli_subcommand) {
      has_subcmd = 1;
      subcmd_f = f;
    }
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  int _seen_%s = 0;\n"
                          "  int _cli_seen_%s = 0;\n",
                          f->name, f->name));
  }

  /* Emit candidate flags array for typo suggestion */
  CHECK_IO(FPRINTF_HOOK(fp, "  static const char *const _cdd_cands[] = {\n"));
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    char lname[64];
    char esc_l[128];
    if (f->serde_skip || f->cli_hidden || f->cli_positional)
      continue;
    rc = cdd_cli_resolve_field_long(sf, f, lname, sizeof(lname));
    if (lname[0] != '\0') {
      rc = cdd_escape_json_c_str(lname, esc_l, sizeof(esc_l));
      CHECK_IO(FPRINTF_HOOK(fp, "    \"--%s\",\n", esc_l));
      if (f->cli_negatable) {
        CHECK_IO(FPRINTF_HOOK(fp, "    \"--no-%s\",\n", esc_l));
      }
    }
  }
  if (!sf->cli_disable_help_flag)
    CHECK_IO(FPRINTF_HOOK(fp, "    \"--help\",\n"));
  if (!sf->cli_disable_version_flag)
    CHECK_IO(FPRINTF_HOOK(fp, "    \"--version\",\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "    NULL\n  };\n"
                            "  const size_t _n_cdd_cands = "
                            "(sizeof(_cdd_cands) / sizeof(_cdd_cands[0])) - "
                            "1u;\n\n"));

  CHECK_IO(FPRINTF_HOOK(fp, "  if (!ret || argc < 0 || (argc > 0 && !argv))\n"
                            "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                            "  if (out_consumed)\n"
                            "    *out_consumed = argc;\n"));

  if (sf->cli_arg_required_else_help) {
    CHECK_IO(
        FPRINTF_HOOK(fp,
                     "  if (argc <= 1) {\n"
                     "    %s_print_help(argc > 0 ? argv[0] : NULL, stderr);\n"
                     "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                     "  }\n",
                     struct_name));
  }

  /* Stage 1: SetFalse defaults & Flattened child struct allocation */
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    if (f->serde_skip)
      continue;
    if (cli_effective_action(f) == CDD_CLI_ACTION_SET_FALSE) {
      CHECK_IO(FPRINTF_HOOK(fp, "  ret->%s = 1;\n", f->name));
    }
    if (f->serde_flatten) {
      const char *ref_t = NULL;
      rc = get_type_from_ref(f->ref, &ref_t);
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  if (!ret->%s) {\n"
                            "    _rc = %s_default(&ret->%s);\n"
                            "    if (_rc != CDD_C_SUCCESS)\n"
                            "      return _rc;\n"
                            "  }\n",
                            f->name, ref_t, f->name));
    }
  }

  /* Stage 2: Environment Variable Layer */
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    char env_name[64];
    char esc_env[128];
    if (f->serde_skip || f->cli_subcommand || f->serde_flatten)
      continue;
    rc = cdd_cli_resolve_field_env(sf, f, env_name, sizeof(env_name));
    if (env_name[0] == '\0')
      continue;
    rc = cdd_escape_json_c_str(env_name, esc_env, sizeof(esc_env));
    CHECK_IO(FPRINTF_HOOK(fp,
                          "  if (_cdd_cli_env_get(\"%s\", &_env_val) != 0)\n"
                          "    return CDD_C_ERROR_MEMORY;\n"
                          "  if (_env_val && _env_val[0] != '\\0') {\n",
                          esc_env));
    rc = cli_emit_field_value_assign(
        fp, f, "_env_val",
        "free(_env_val); return CDD_C_ERROR_INVALID_ARGUMENT;",
        "free(_env_val); return CDD_C_ERROR_MEMORY;");
    if (rc != CDD_C_SUCCESS)
      return rc;
    CHECK_IO(FPRINTF_HOOK(fp,
                          "    _seen_%s = 1;\n"
                          "  }\n"
                          "  if (_env_val) {\n"
                          "    free(_env_val);\n"
                          "    _env_val = NULL;\n"
                          "  }\n",
                          f->name));
  }
  CHECK_IO(FPRINTF_HOOK(fp, "  if (_env_val)\n"
                            "    free(_env_val);\n"));

  /* Stage 3: argv Loop */
  CHECK_IO(FPRINTF_HOOK(fp, "  for (_i = 1; _i < argc; ++_i) {\n"
                            "    const char *_arg = argv[_i];\n"
                            "    if (!_arg)\n"
                            "      continue;\n"
                            "    if (!_end_of_opts && strcmp(_arg, \"--\") == "
                            "0) {\n"
                            "      _end_of_opts = 1;\n"
                            "      continue;\n"
                            "    }\n"));

  /* Windows Slash Flags normalization if enabled */
  if (sf->cli_allow_windows_slash_flags) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "    if (!_end_of_opts && _arg[0] == '/' && _arg[1] != '\\0' &&\n"
        "        strchr(_arg + 1, '/') == NULL && strchr(_arg + 1, '\\\\') == "
        "NULL) {\n"
        "      if (strcmp(_arg, \"/?\") == 0 || _cdd_cli_str_ieq(_arg, "
        "\"/help\")) {\n"
        "        return %s_print_help(argv[0], stdout);\n"
        "      }\n",
        struct_name));
    for (i = 0; i < sf->size; ++i) {
      const struct StructField *f = &sf->fields[i];
      char lname[64];
      enum CddCliAction act = cli_effective_action(f);
      if (f->serde_skip || f->cli_positional || f->cli_subcommand ||
          f->serde_flatten)
        continue;
      cdd_cli_resolve_field_long(sf, f, lname, sizeof(lname));
      if (lname[0] == '\0')
        continue;
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      {\n"
          "        size_t _sl_len = strlen(\"%s\");\n"
          "        if (strncmp(_arg + 1, \"%s\", _sl_len) == 0 &&\n"
          "            (_arg[1 + _sl_len] == '\\0' || _arg[1 + _sl_len] == ':' "
          "|| _arg[1 + _sl_len] == '=')) {\n",
          lname, lname));
      if (act == CDD_CLI_ACTION_SET_TRUE) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "          ret->%s = 1;\n"
                              "          _seen_%s = 1;\n"
                              "          _cli_seen_%s = 1;\n"
                              "          _total_cli_seen++;\n"
                              "          continue;\n",
                              f->name, f->name, f->name));
      } else if (act == CDD_CLI_ACTION_SET_FALSE) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "          ret->%s = 0;\n"
                              "          _seen_%s = 1;\n"
                              "          _cli_seen_%s = 1;\n"
                              "          _total_cli_seen++;\n"
                              "          continue;\n",
                              f->name, f->name, f->name));
      } else if (act == CDD_CLI_ACTION_COUNT) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "          ret->%s++;\n"
                              "          _seen_%s = 1;\n"
                              "          _cli_seen_%s = 1;\n"
                              "          _total_cli_seen++;\n"
                              "          continue;\n",
                              f->name, f->name, f->name));
      } else {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "          const char *_sval = (_arg[1 + _sl_len] != '\\0') ? "
            "(_arg + 2 + _sl_len) : ((_i + 1 < argc) ? argv[++_i] : NULL);\n"
            "          if (!_sval)\n"
            "            return CDD_C_ERROR_INVALID_ARGUMENT;\n"));
        rc = cli_emit_field_value_assign(fp, f, "_sval",
                                         "return CDD_C_ERROR_INVALID_ARGUMENT;",
                                         "return CDD_C_ERROR_MEMORY;");
        if (rc != CDD_C_SUCCESS)
          return rc;
        CHECK_IO(FPRINTF_HOOK(fp,
                              "          _seen_%s = 1;\n"
                              "          _cli_seen_%s = 1;\n"
                              "          _total_cli_seen++;\n"
                              "          continue;\n",
                              f->name, f->name));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "        }\n"
                                "      }\n"));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "    }\n"));
  }

  /* Long Options (--flag and --flag=val) */
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "    if (!_end_of_opts && _arg[0] == '-' && _arg[1] == '-' && _arg[2] != "
      "'\\0') {\n"
      "      const char *_eq = strchr(_arg + 2, '=');\n"
      "      size_t _klen = _eq ? (size_t)(_eq - (_arg + 2)) : strlen(_arg + "
      "2);\n"
      "      const char *_inline_val = _eq ? (_eq + 1) : NULL;\n"
      "      char _kbuf[64];\n"
      "      if (_klen >= sizeof(_kbuf))\n"
      "        _klen = sizeof(_kbuf) - 1;\n"
      "      memcpy(_kbuf, _arg + 2, _klen);\n"
      "      _kbuf[_klen] = '\\0';\n"));

  if (!sf->cli_disable_help_flag) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "      if (strcmp(_kbuf, \"help\") == 0) {\n"
                          "        return %s_print_help(argv[0], stdout);\n"
                          "      }\n",
                          struct_name));
  }
  if (!sf->cli_disable_version_flag) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "      if (strcmp(_kbuf, \"version\") == 0) {\n"
                          "        return %s_print_version(argv[0], stdout);\n"
                          "      }\n",
                          struct_name));
  }

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    char lname[64];
    char esc_l[128];
    enum CddCliAction act = cli_effective_action(f);
    const char *cmp_fn =
        sf->cli_case_insensitive_flags ? "_cdd_cli_str_ieq" : "0 == strcmp";

    if (f->serde_skip || f->cli_positional || f->cli_subcommand)
      continue;

    if (f->serde_flatten) {
      const char *ref_t = NULL;
      rc = get_type_from_ref(f->ref, &ref_t);
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      {\n"
          "        char *_sub_argv[3];\n"
          "        int _sub_argc = (_inline_val || _i + 1 >= argc) ? 2 : 3;\n"
          "        int _cons = 0;\n"
          "        _sub_argv[0] = argv[0];\n"
          "        _sub_argv[1] = argv[_i];\n"
          "        _sub_argv[2] = (_i + 1 < argc) ? argv[_i + 1] : NULL;\n"
          "        if (%s_parse_cli_into(_sub_argc, _sub_argv, ret->%s, "
          "&_cons) == CDD_C_SUCCESS && _cons >= 2) {\n"
          "          if (_cons == 3)\n"
          "            _i++;\n"
          "          continue;\n"
          "        }\n"
          "      }\n",
          ref_t, f->name));
      continue;
    }

    rc = cdd_cli_resolve_field_long(sf, f, lname, sizeof(lname));
    if (lname[0] == '\0')
      continue;
    rc = cdd_escape_json_c_str(lname, esc_l, sizeof(esc_l));

    if (f->cli_negatable && strcmp(f->type, "boolean") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "      if (%s(_kbuf, \"no-%s\")) {\n"
                            "        ret->%s = 0;\n"
                            "        _seen_%s = 1;\n"
                            "        _cli_seen_%s = 1;\n"
                            "        _total_cli_seen++;\n"
                            "        continue;\n"
                            "      }\n",
                            cmp_fn, esc_l, f->name, f->name, f->name));
    }

    CHECK_IO(FPRINTF_HOOK(fp, "      if (%s(_kbuf, \"%s\")", cmp_fn, esc_l));
    for (j = 0; j < f->n_serde_aliases; ++j) {
      char esc_al[128];
      rc = cdd_escape_json_c_str(f->serde_aliases[j], esc_al, sizeof(esc_al));
      CHECK_IO(FPRINTF_HOOK(fp, " || %s(_kbuf, \"%s\")", cmp_fn, esc_al));
    }
    CHECK_IO(FPRINTF_HOOK(fp, ") {\n"));

    if (act == CDD_CLI_ACTION_HELP) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "        return %s_print_help(argv[0], stdout);\n", struct_name));
    } else if (act == CDD_CLI_ACTION_VERSION) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "        return %s_print_version(argv[0], stdout);\n",
          struct_name));
    } else if (act == CDD_CLI_ACTION_SET_TRUE) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "        if (_inline_val) {\n"
          "          if (!_cdd_cli_parse_bool(_inline_val, &ret->%s))\n"
          "            return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "        } else {\n"
          "          ret->%s = 1;\n"
          "        }\n"
          "        _seen_%s = 1;\n"
          "        _cli_seen_%s = 1;\n"
          "        _total_cli_seen++;\n"
          "        continue;\n",
          f->name, f->name, f->name, f->name));
    } else if (act == CDD_CLI_ACTION_SET_FALSE) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "        if (_inline_val) {\n"
          "          if (!_cdd_cli_parse_bool(_inline_val, &ret->%s))\n"
          "            return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "        } else {\n"
          "          ret->%s = 0;\n"
          "        }\n"
          "        _seen_%s = 1;\n"
          "        _cli_seen_%s = 1;\n"
          "        _total_cli_seen++;\n"
          "        continue;\n",
          f->name, f->name, f->name, f->name));
    } else if (act == CDD_CLI_ACTION_COUNT) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "        ret->%s++;\n"
                            "        _seen_%s = 1;\n"
                            "        _cli_seen_%s = 1;\n"
                            "        _total_cli_seen++;\n"
                            "        continue;\n",
                            f->name, f->name, f->name));
    } else {
      char esc_def_miss[128];
      rc = cdd_escape_json_c_str(f->cli_default_missing_val, esc_def_miss,
                                 sizeof(esc_def_miss));
      CHECK_IO(
          FPRINTF_HOOK(fp, "        const char *_val_str = _inline_val;\n"));
      if (f->cli_require_equals) {
        if (f->cli_default_missing_val[0] != '\0') {
          CHECK_IO(FPRINTF_HOOK(fp,
                                "        if (!_val_str)\n"
                                "          _val_str = \"%s\";\n",
                                esc_def_miss));
        } else {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "        if (!_val_str) {\n"
              "          fprintf(stderr, \"error: equal sign is needed "
              "when assigning values to '--%s'\\n\");\n"
              "          return CDD_C_ERROR_INVALID_ARGUMENT;\n"
              "        }\n",
              esc_l));
        }
      } else if (f->cli_default_missing_val[0] != '\0') {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "        if (!_val_str) {\n"
            "          if (_i + 1 < argc && (argv[_i + 1][0] != '-' || %d))\n"
            "            _val_str = argv[++_i];\n"
            "          else\n"
            "            _val_str = \"%s\";\n"
            "        }\n",
            f->cli_allow_hyphen_values ? 1 : 0, esc_def_miss));
      } else {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "        if (!_val_str) {\n"
            "          if (_i + 1 >= argc || (!%d && argv[_i + 1][0] == '-' && "
            "argv[_i + 1][1] != '\\0')) {\n"
            "            fprintf(stderr, \"error: a value is required for "
            "'--%s' but none was supplied\\n\");\n"
            "            return CDD_C_ERROR_INVALID_ARGUMENT;\n"
            "          }\n"
            "          _val_str = argv[++_i];\n"
            "        }\n",
            f->cli_allow_hyphen_values ? 1 : 0, esc_l));
      }
      rc = cli_emit_field_value_assign(fp, f, "_val_str",
                                       "return CDD_C_ERROR_INVALID_ARGUMENT;",
                                       "return CDD_C_ERROR_MEMORY;");
      if (rc != CDD_C_SUCCESS)
        return rc;
      CHECK_IO(FPRINTF_HOOK(fp,
                            "        _seen_%s = 1;\n"
                            "        _cli_seen_%s = 1;\n"
                            "        _total_cli_seen++;\n"
                            "        continue;\n",
                            f->name, f->name));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "      }\n"));
  }

  /* Unknown --flag typo suggestion */
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "      {\n"
      "        const char *_best = _cdd_cli_suggest(_arg, _cdd_cands, "
      "_n_cdd_cands);\n"
      "        fprintf(stderr, \"error: unexpected argument '%%s' found\\n\", "
      "_arg);\n"
      "        if (_best)\n"
      "          fprintf(stderr, \"  tip: a similar argument exists: "
      "'%%s'\\n\", _best);\n"
      "        fprintf(stderr, \"For more information, try '--help'.\\n\");\n"
      "        return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "      }\n"
      "    }\n"));

  /* Short Option Clusters (-f, -abc, -vvv, -p8080, -p 8080) */
  CHECK_IO(FPRINTF_HOOK(
      fp,
      "    if (!_end_of_opts && _arg[0] == '-' && _arg[1] != '-' && _arg[1] != "
      "'\\0') {\n"
      "      const char *_sc = _arg + 1;\n"
      "      while (*_sc != '\\0') {\n"
      "        char _ch = *_sc;\n"));

  if (!sf->cli_disable_help_flag) {
    CHECK_IO(FPRINTF_HOOK(fp,
                          "        if (_ch == 'h') {\n"
                          "          return %s_print_help(argv[0], stdout);\n"
                          "        }\n",
                          struct_name));
  }
  if (!sf->cli_disable_version_flag) {
    CHECK_IO(
        FPRINTF_HOOK(fp,
                     "        if (_ch == 'V') {\n"
                     "          return %s_print_version(argv[0], stdout);\n"
                     "        }\n",
                     struct_name));
  }

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    enum CddCliAction act = cli_effective_action(f);
    if (f->serde_skip || f->cli_positional || f->cli_subcommand ||
        f->serde_flatten)
      continue;
    if (f->cli_short == '\0' && f->cli_short_aliases[0] == '\0')
      continue;

    CHECK_IO(FPRINTF_HOOK(fp, "        if ("));
    if (f->cli_short != '\0') {
      CHECK_IO(FPRINTF_HOOK(fp, "_ch == '%c'", f->cli_short));
    } else {
      CHECK_IO(FPRINTF_HOOK(fp, "0"));
    }
    for (j = 0; f->cli_short_aliases[j] != '\0'; ++j) {
      CHECK_IO(FPRINTF_HOOK(fp, " || _ch == '%c'", f->cli_short_aliases[j]));
    }
    CHECK_IO(FPRINTF_HOOK(fp, ") {\n"));

    if (act == CDD_CLI_ACTION_SET_TRUE) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "          ret->%s = 1;\n"
                            "          _seen_%s = 1;\n"
                            "          _cli_seen_%s = 1;\n"
                            "          _total_cli_seen++;\n"
                            "          _sc++;\n"
                            "          continue;\n",
                            f->name, f->name, f->name));
    } else if (act == CDD_CLI_ACTION_SET_FALSE) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "          ret->%s = 0;\n"
                            "          _seen_%s = 1;\n"
                            "          _cli_seen_%s = 1;\n"
                            "          _total_cli_seen++;\n"
                            "          _sc++;\n"
                            "          continue;\n",
                            f->name, f->name, f->name));
    } else if (act == CDD_CLI_ACTION_COUNT) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "          ret->%s++;\n"
                            "          _seen_%s = 1;\n"
                            "          _cli_seen_%s = 1;\n"
                            "          _total_cli_seen++;\n"
                            "          _sc++;\n"
                            "          continue;\n",
                            f->name, f->name, f->name));
    } else {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "          const char *_sval = (*(_sc + 1) != '\\0') ? (_sc + 1) : "
          "((_i + 1 < argc) ? argv[++_i] : NULL);\n"
          "          if (!_sval) {\n"
          "            fprintf(stderr, \"error: a value is required for '-%c' "
          "but none was supplied\\n\");\n"
          "            return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "          }\n",
          f->cli_short ? f->cli_short : f->cli_short_aliases[0]));
      rc = cli_emit_field_value_assign(fp, f, "_sval",
                                       "return CDD_C_ERROR_INVALID_ARGUMENT;",
                                       "return CDD_C_ERROR_MEMORY;");
      if (rc != CDD_C_SUCCESS)
        return rc;
      CHECK_IO(FPRINTF_HOOK(fp,
                            "          _seen_%s = 1;\n"
                            "          _cli_seen_%s = 1;\n"
                            "          _total_cli_seen++;\n"
                            "          break;\n",
                            f->name, f->name));
    }
    CHECK_IO(FPRINTF_HOOK(fp, "        }\n"));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "        fprintf(stderr, \"error: unexpected short option '-%%c' "
          "found\\n\", _ch);\n"
          "        return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "      }\n"
          "      continue;\n"
          "    }\n"));

  /* Subcommand Dispatch (when subcommand field is present) */
  if (has_subcmd) {
    const char *ref_t = NULL;
    const struct TypeDefinition *udef = NULL;
    rc = get_type_from_ref(subcmd_f->ref, &ref_t);
    rc = cli_find_type_def(config ? config->type_registry : NULL, ref_t,
                           KIND_STRUCT, &udef);
    if (udef) {
      const struct StructFields *usf = udef->details.struct_fields;
      if (!sf->cli_disable_help_subcommand) {
        CHECK_IO(FPRINTF_HOOK(
            fp, "    if (!_end_of_opts && strcmp(_arg, \"help\") == 0) {\n"));
        for (i = 0; i < usf->size; ++i) {
          const struct StructField *vf = &usf->fields[i];
          char cmd_name[64];
          const char *vref = NULL;
          if (vf->serde_skip)
            continue;
          rc = get_type_from_ref(vf->ref, &vref);
          if (vf->serde_rename[0] != '\0') {
            rc = cli_copy_str(cmd_name, sizeof(cmd_name), vf->serde_rename);
          } else {
            rc = cdd_serde_transform_case(vf->name, CDD_SERDE_CASE_KEBABCASE,
                                          cmd_name, sizeof(cmd_name));
          }
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "      if (_i + 1 < argc && strcmp(argv[_i + 1], \"%s\") == 0)\n"
              "        return %s_print_help(argv[_i + 1], stdout);\n",
              cmd_name, vref));
        }
        CHECK_IO(FPRINTF_HOOK(fp,
                              "      return %s_print_help(argv[0], stdout);\n"
                              "    }\n",
                              struct_name));
      }

      for (i = 0; i < usf->size; ++i) {
        const struct StructField *vf = &usf->fields[i];
        char cmd_name[64];
        const char *vref = NULL;
        if (vf->serde_skip)
          continue;
        rc = get_type_from_ref(vf->ref, &vref);
        if (vf->serde_rename[0] != '\0') {
          rc = cli_copy_str(cmd_name, sizeof(cmd_name), vf->serde_rename);
        } else {
          rc = cdd_serde_transform_case(vf->name, CDD_SERDE_CASE_KEBABCASE,
                                        cmd_name, sizeof(cmd_name));
        }
        CHECK_IO(
            FPRINTF_HOOK(fp, "    if (strcmp(_arg, \"%s\") == 0", cmd_name));
        for (j = 0; j < vf->n_serde_aliases; ++j) {
          CHECK_IO(FPRINTF_HOOK(fp, " || strcmp(_arg, \"%s\") == 0",
                                vf->serde_aliases[j]));
        }
        CHECK_IO(FPRINTF_HOOK(
            fp,
            ") {\n"
            "      if (!ret->%s) {\n"
            "        ret->%s = (struct %s *)calloc(1, sizeof(struct %s));\n"
            "        if (!ret->%s)\n"
            "          return CDD_C_ERROR_MEMORY;\n"
            "      }\n"
            "      ret->%s->tag = %s_%s;\n"
            "      _rc = %s_from_cli(argc - _i, argv + _i, "
            "&ret->%s->data.%s);\n"
            "      if (_rc != CDD_C_SUCCESS)\n"
            "        return _rc;\n"
            "      _seen_%s = 1;\n"
            "      _cli_seen_%s = 1;\n"
            "      _total_cli_seen++;\n"
            "      break;\n"
            "    }\n",
            subcmd_f->name, subcmd_f->name, ref_t, ref_t, subcmd_f->name,
            subcmd_f->name, ref_t, vf->name, vref, subcmd_f->name, vf->name,
            subcmd_f->name, subcmd_f->name));
      }
    }
  }

  /* Positional Arguments */
  {
    int seq_pos = 0;
    for (i = 0; i < sf->size; ++i) {
      const struct StructField *f = &sf->fields[i];
      int target_pos;
      if (f->serde_skip || !f->cli_positional)
        continue;
      target_pos = (f->cli_index >= 0) ? f->cli_index : seq_pos;
      seq_pos++;

      if (f->cli_last) {
        CHECK_IO(FPRINTF_HOOK(fp, "    if (_end_of_opts) {\n"));
      } else if (strcmp(f->type, "array") == 0 || f->cli_trailing_var_arg) {
        CHECK_IO(FPRINTF_HOOK(fp, "    if (_pos_idx >= %d) {\n", target_pos));
      } else {
        CHECK_IO(FPRINTF_HOOK(fp, "    if (_pos_idx == %d) {\n", target_pos));
      }

      rc = cli_emit_field_value_assign(fp, f, "_arg",
                                       "return CDD_C_ERROR_INVALID_ARGUMENT;",
                                       "return CDD_C_ERROR_MEMORY;");
      if (rc != CDD_C_SUCCESS)
        return rc;
      CHECK_IO(FPRINTF_HOOK(fp,
                            "      _seen_%s = 1;\n"
                            "      _cli_seen_%s = 1;\n"
                            "      _total_cli_seen++;\n"
                            "      _pos_idx++;\n"
                            "      continue;\n"
                            "    }\n",
                            f->name, f->name));
    }
  }

  CHECK_IO(FPRINTF_HOOK(
      fp, "    fprintf(stderr, \"error: unexpected positional argument '%%s' "
          "found\\n\", _arg);\n"
          "    if (out_consumed)\n"
          "      *out_consumed = _i;\n"
          "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  }\n\n"));

  /* Stage 4: Post-Parse Validation Pass */
  if (sf->cli_subcommand_required) {
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "  if (!_seen_%s) {\n"
        "    fprintf(stderr, \"error: a subcommand is required\\n\");\n"
        "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
        "  }\n",
        subcmd_f->name));
  }

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    if (f->serde_skip)
      continue;

    /* Required check (respecting required_unless_present) */
    if (f->required && !f->serde_has_default && f->default_val[0] == '\0') {
      CHECK_IO(FPRINTF_HOOK(fp, "  if (!_seen_%s", f->name));
      for (j = 0; j < f->n_cli_required_unless; ++j) {
        CHECK_IO(FPRINTF_HOOK(fp, " && !_seen_%s", f->cli_required_unless[j]));
      }
      CHECK_IO(FPRINTF_HOOK(
          fp,
          ") {\n"
          "    fprintf(stderr, \"error: required argument '%s' was not "
          "provided\\n\");\n"
          "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  }\n",
          f->name));
    }

    /* Exclusive check */
    if (f->cli_exclusive) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  if (_cli_seen_%s && _total_cli_seen > 1) {\n"
          "    fprintf(stderr, \"error: the argument '--%s' cannot be "
          "used with one or more of the other specified arguments\\n\");\n"
          "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  }\n",
          f->name, f->name));
    }

    /* Conflicts check */
    for (j = 0; j < f->n_cli_conflicts_with; ++j) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  if (_cli_seen_%s && _cli_seen_%s) {\n"
          "    fprintf(stderr, \"error: the argument '%s' cannot be used "
          "with '%s'\\n\");\n"
          "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
          "  }\n",
          f->name, f->cli_conflicts_with[j], f->name,
          f->cli_conflicts_with[j]));
    }

    /* Requires check */
    for (j = 0; j < f->n_cli_requires; ++j) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  if (_cli_seen_%s && !_seen_%s) {\n"
                            "    fprintf(stderr, \"error: the argument '%s' "
                            "requires '%s'\\n\");\n"
                            "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                            "  }\n",
                            f->name, f->cli_requires[j], f->name,
                            f->cli_requires[j]));
    }

    /* required_if_eq check */
    if (f->cli_required_if_field[0] != '\0') {
      const struct StructField *tf = NULL;
      for (j = 0; j < sf->size; ++j) {
        if (strcmp(sf->fields[j].name, f->cli_required_if_field) == 0) {
          tf = &sf->fields[j];
          break;
        }
      }
      if (tf) {
        char esc_rv[128];
        cdd_escape_json_c_str(f->cli_required_if_val, esc_rv, sizeof(esc_rv));
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  if (_seen_%s && ret->%s && strcmp(ret->%s, \"%s\") == 0 && "
            "!_seen_%s) {\n"
            "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
            "  }\n",
            tf->name, tf->name, tf->name, esc_rv, f->name));
      }
    }

    /* ArgGroup mutual exclusion check */
    if (f->cli_group[0] != '\0') {
      for (j = i + 1; j < sf->size; ++j) {
        const struct StructField *of = &sf->fields[j];
        if (strcmp(of->cli_group, f->cli_group) == 0) {
          CHECK_IO(FPRINTF_HOOK(fp,
                                "  if (_cli_seen_%s && _cli_seen_%s) {\n"
                                "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
                                "  }\n",
                                f->name, of->name));
        }
      }
    }

    /* Numeric / String / Array Constraint validation */
    if (strcmp(f->type, "integer") == 0 || strcmp(f->type, "number") == 0) {
      if (f->has_min) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  if (_seen_%s && (double)ret->%s %s %g)\n"
                              "    return CDD_C_ERROR_INVALID_ARGUMENT;\n",
                              f->name, f->name, f->exclusive_min ? "<=" : "<",
                              f->min_val));
      }
      if (f->has_max) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  if (_seen_%s && (double)ret->%s %s %g)\n"
                              "    return CDD_C_ERROR_INVALID_ARGUMENT;\n",
                              f->name, f->name, f->exclusive_max ? ">=" : ">",
                              f->max_val));
      }
    } else if (strcmp(f->type, "string") == 0) {
      if (f->has_min_len) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  if (_seen_%s && ret->%s && strlen(ret->%s) < %luu)\n"
            "    return CDD_C_ERROR_INVALID_ARGUMENT;\n",
            f->name, f->name, f->name, (unsigned long)f->min_len));
      }
      if (f->has_max_len) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  if (_seen_%s && ret->%s && strlen(ret->%s) > %luu)\n"
            "    return CDD_C_ERROR_INVALID_ARGUMENT;\n",
            f->name, f->name, f->name, (unsigned long)f->max_len));
      }
    } else if (strcmp(f->type, "array") == 0) {
      if (f->has_min_items) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  if (_seen_%s && ret->n_%s < %luu)\n"
                              "    return CDD_C_ERROR_INVALID_ARGUMENT;\n",
                              f->name, f->name, (unsigned long)f->min_items));
      }
      if (f->has_max_items) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  if (_seen_%s && ret->n_%s > %luu)\n"
                              "    return CDD_C_ERROR_INVALID_ARGUMENT;\n",
                              f->name, f->name, (unsigned long)f->max_items));
      }
    }
  }

  CHECK_IO(FPRINTF_HOOK(fp, "  return _rc;\n"
                            "}\n\n"));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Generates the `<Struct>_from_cli` function implementation.
 */
cdd_c_error_t
write_struct_from_cli_func(FILE *fp, const char *struct_name,
                           const struct StructFields *sf,
                           const struct CodegenCliConfig *config) {
  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_from_cli(int argc, char **argv, struct %s **out) {\n"
      "  struct %s *ret = NULL;\n"
      "  cdd_c_error_t rc;\n"
      "  if (!out || argc < 0 || (argc > 0 && !argv))\n"
      "    return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "  *out = NULL;\n"
      "  rc = %s_default(&ret);\n"
      "  if (rc != CDD_C_SUCCESS)\n"
      "    return rc;\n"
      "  rc = %s_parse_cli_into(argc, argv, ret, NULL);\n"
      "  if (rc != CDD_C_SUCCESS) {\n"
      "    %s_cleanup(ret);\n"
      "    return rc;\n"
      "  }\n"
      "  *out = ret;\n"
      "  return CDD_C_SUCCESS;\n"
      "}\n\n",
      struct_name, struct_name, struct_name, struct_name, struct_name,
      struct_name));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Generates the `<Struct>_to_cli_argv` function implementation to
 * serialize a populated C struct back into an `(argc, argv)` array.
 */
cdd_c_error_t
write_struct_to_cli_argv_func(FILE *fp, const char *struct_name,
                              const struct StructFields *sf,
                              const struct CodegenCliConfig *config) {
  char default_prog[64];
  char esc_prog[128];
  size_t i;
  cdd_c_error_t rc;

  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (sf->cli_bin_name[0] != '\0') {
    rc = cli_copy_str(default_prog, sizeof(default_prog), sf->cli_bin_name);
  } else if (sf->cli_name[0] != '\0') {
    rc = cli_copy_str(default_prog, sizeof(default_prog), sf->cli_name);
  } else {
    rc = cdd_serde_transform_case(struct_name, CDD_SERDE_CASE_KEBABCASE,
                                  default_prog, sizeof(default_prog));
  }
  rc = cdd_escape_json_c_str(default_prog, esc_prog, sizeof(esc_prog));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));
  }

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_to_cli_argv(const struct %s *obj, int *out_argc, char "
      "***out_argv) {\n"
      "  int _argc = 0;\n"
      "  int _cap = 16;\n"
      "  char **_argv = NULL;\n"
      "  char _nbuf[64];\n"
      "  size_t _k = 0;\n"
      "  _nbuf[0] = '\\0';\n"
      "  if (!obj || !out_argc || !out_argv)\n"
      "    return CDD_C_ERROR_INVALID_ARGUMENT;\n",
      struct_name, struct_name));
  CHECK_IO(
      FPRINTF_HOOK(fp,
                   "  *out_argc = 0;\n"
                   "  *out_argv = NULL;\n"
                   "  _argv = (char **)calloc((size_t)_cap, sizeof(char *));\n"
                   "  if (!_argv)\n"
                   "    return CDD_C_ERROR_MEMORY;\n"
                   "  _argv[_argc] = strdup(\"%s\");\n"
                   "  if (!_argv[_argc]) {\n"
                   "    free(_argv);\n"
                   "    return CDD_C_ERROR_MEMORY;\n"
                   "  }\n"
                   "  _argc++;\n",
                   esc_prog));

  /* Emit non-positional options first, then positionals */
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    char lname[64];
    char flag_tok[80];
    enum CddCliAction act = cli_effective_action(f);

    if (f->serde_skip || f->cli_positional || f->cli_subcommand ||
        f->serde_flatten)
      continue;
    rc = cdd_cli_resolve_field_long(sf, f, lname, sizeof(lname));
    if (lname[0] != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(flag_tok, sizeof(flag_tok), "--%s", lname);
#else
      sprintf(flag_tok, "--%s", lname);
#endif
    } else if (f->cli_short != '\0') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      sprintf_s(flag_tok, sizeof(flag_tok), "-%c", f->cli_short);
#else
      sprintf(flag_tok, "-%c", f->cli_short);
#endif
    } else {
      continue;
    }

    if (act == CDD_CLI_ACTION_SET_TRUE) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  if (obj->%s) {\n"
                            "    if (_argc + 2 >= _cap) {\n"
                            "      _cap *= 2;\n"
                            "      _argv = (char **)realloc(_argv, "
                            "(size_t)_cap * sizeof(char *));\n"
                            "    }\n"
                            "    _argv[_argc++] = strdup(\"%s\");\n"
                            "  }\n",
                            f->name, flag_tok));
    } else if (act == CDD_CLI_ACTION_SET_FALSE) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  if (!obj->%s) {\n"
                            "    if (_argc + 2 >= _cap) {\n"
                            "      _cap *= 2;\n"
                            "      _argv = (char **)realloc(_argv, "
                            "(size_t)_cap * sizeof(char *));\n"
                            "    }\n"
                            "    _argv[_argc++] = strdup(\"%s\");\n"
                            "  }\n",
                            f->name, flag_tok));
    } else if (act == CDD_CLI_ACTION_COUNT) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  for (_k = 0; (int)_k < obj->%s; ++_k) {\n"
                            "    if (_argc + 2 >= _cap) {\n"
                            "      _cap *= 2;\n"
                            "      _argv = (char **)realloc(_argv, "
                            "(size_t)_cap * sizeof(char *));\n"
                            "    }\n"
                            "    _argv[_argc++] = strdup(\"%s\");\n"
                            "  }\n",
                            f->name, flag_tok));
    } else if (strcmp(f->type, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  if (obj->%s) {\n"
                            "    if (_argc + 3 >= _cap) {\n"
                            "      _cap *= 2;\n"
                            "      _argv = (char **)realloc(_argv, "
                            "(size_t)_cap * sizeof(char *));\n"
                            "    }\n"
                            "    _argv[_argc++] = strdup(\"%s\");\n"
                            "    _argv[_argc++] = strdup(obj->%s);\n"
                            "  }\n",
                            f->name, flag_tok, f->name));
    } else if (strcmp(f->type, "integer") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  {\n"
                            "    if (_argc + 3 >= _cap) {\n"
                            "      _cap *= 2;\n"
                            "      _argv = (char **)realloc(_argv, "
                            "(size_t)_cap * sizeof(char *));\n"
                            "    }\n"
                            "    sprintf(_nbuf, \"%%d\", obj->%s);\n"
                            "    _argv[_argc++] = strdup(\"%s\");\n"
                            "    _argv[_argc++] = strdup(_nbuf);\n"
                            "  }\n",
                            f->name, flag_tok));
    } else if (strcmp(f->type, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  {\n"
                            "    if (_argc + 3 >= _cap) {\n"
                            "      _cap *= 2;\n"
                            "      _argv = (char **)realloc(_argv, "
                            "(size_t)_cap * sizeof(char *));\n"
                            "    }\n"
                            "    sprintf(_nbuf, \"%%g\", obj->%s);\n"
                            "    _argv[_argc++] = strdup(\"%s\");\n"
                            "    _argv[_argc++] = strdup(_nbuf);\n"
                            "  }\n",
                            f->name, flag_tok));
    } else if (strcmp(f->type, "enum") == 0) {
      const char *ref_t = NULL;
      rc = get_type_from_ref(f->ref, &ref_t);
      if (rc != CDD_C_SUCCESS)
        return rc;
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "  if (obj->%s != 0) {\n"
                       "    char *_estr = NULL;\n"
                       "    if (%s_to_str(obj->%s, &_estr) == 0 && _estr) {\n"
                       "      if (_argc + 3 >= _cap) {\n"
                       "        _cap *= 2;\n"
                       "        _argv = (char **)realloc(_argv, (size_t)_cap * "
                       "sizeof(char *));\n"
                       "      }\n"
                       "      _argv[_argc++] = strdup(\"%s\");\n"
                       "      _argv[_argc++] = _estr;\n"
                       "    }\n"
                       "  }\n",
                       f->name, ref_t, f->name, flag_tok));
    } else if (strcmp(f->type, "array") == 0 && strcmp(f->ref, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  for (_k = 0; _k < obj->n_%s; ++_k) {\n"
                            "    if (!obj->%s[_k])\n"
                            "      continue;\n"
                            "    if (_argc + 3 >= _cap) {\n"
                            "      _cap *= 2;\n"
                            "      _argv = (char **)realloc(_argv, "
                            "(size_t)_cap * sizeof(char *));\n"
                            "    }\n"
                            "    _argv[_argc++] = strdup(\"%s\");\n"
                            "    _argv[_argc++] = strdup(obj->%s[_k]);\n"
                            "  }\n",
                            f->name, f->name, flag_tok, f->name));
    }
  }

  /* Emit positionals */
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    if (f->serde_skip || !f->cli_positional)
      continue;
    if (f->cli_last) {
      CHECK_IO(FPRINTF_HOOK(fp, "  if (_argc + 2 >= _cap) {\n"
                                "    _cap *= 2;\n"
                                "    _argv = (char **)realloc(_argv, "
                                "(size_t)_cap * sizeof(char *));\n"
                                "  }\n"
                                "  _argv[_argc++] = strdup(\"--\");\n"));
    }
    if (strcmp(f->type, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  if (obj->%s) {\n"
                            "    if (_argc + 2 >= _cap) {\n"
                            "      _cap *= 2;\n"
                            "      _argv = (char **)realloc(_argv, "
                            "(size_t)_cap * sizeof(char *));\n"
                            "    }\n"
                            "    _argv[_argc++] = strdup(obj->%s);\n"
                            "  }\n",
                            f->name, f->name));
    } else if (strcmp(f->type, "integer") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  if (_argc + 2 >= _cap) {\n"
                            "    _cap *= 2;\n"
                            "    _argv = (char **)realloc(_argv, (size_t)_cap "
                            "* sizeof(char *));\n"
                            "  }\n"
                            "  sprintf(_nbuf, \"%%d\", obj->%s);\n"
                            "  _argv[_argc++] = strdup(_nbuf);\n",
                            f->name));
    } else {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  for (_k = 0; _k < obj->n_%s; ++_k) {\n"
                            "    if (!obj->%s[_k])\n"
                            "      continue;\n"
                            "    if (_argc + 2 >= _cap) {\n"
                            "      _cap *= 2;\n"
                            "      _argv = (char **)realloc(_argv, "
                            "(size_t)_cap * sizeof(char *));\n"
                            "    }\n"
                            "    _argv[_argc++] = strdup(obj->%s[_k]);\n"
                            "  }\n",
                            f->name, f->name, f->name));
    }
  }

  CHECK_IO(FPRINTF_HOOK(fp, "  if (_nbuf[0] == '\\0' && _k == 0)\n"
                            "    _k = 0;\n"
                            "  _argv[_argc] = NULL;\n"
                            "  *out_argc = _argc;\n"
                            "  *out_argv = _argv;\n"
                            "  return CDD_C_SUCCESS;\n"
                            "}\n\n"));

  if (config && config->guard_macro) {
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));
  }
  return CDD_C_SUCCESS;
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
