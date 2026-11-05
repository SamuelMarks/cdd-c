/**
 * @file cli_struct_parse.c
 * @brief Implementation of C89 CLI parser code generation (`_parse_cli_into`
 * and `_from_cli`).
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
    if (f->bit_width[0] != '\0') {
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "        {\n"
                       "          int _bf_tmp = 0;\n"
                       "          if (%s(%s, &_bf_tmp) != CDD_C_SUCCESS) {\n"
                       "            %s\n"
                       "          }\n"
                       "          ret->%s = _bf_tmp;\n"
                       "        }\n",
                       f->cli_value_parser, raw_expr, err_stmt, n));
    } else {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "        if (%s(%s, &ret->%s) != CDD_C_SUCCESS) {\n"
                            "          %s\n"
                            "        }\n",
                            f->cli_value_parser, raw_expr, n, err_stmt));
    }
    return CDD_C_SUCCESS;
  }

  if (strcmp(t, "string") == 0) {
    if (f->fixed_array_len > 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "        if (strlen(%s) >= %lu) {\n"
                            "          %s\n"
                            "        }\n"
                            "        memcpy(ret->%s, %s, strlen(%s) + 1);\n",
                            raw_expr, (unsigned long)f->fixed_array_len,
                            err_stmt, n, raw_expr, raw_expr));
    } else {
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
    }
  } else if (strcmp(t, "integer") == 0) {
    if (f->bit_width[0] != '\0') {
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "        {\n"
                       "          int _bf_tmp = 0;\n"
                       "          if (!_cdd_cli_parse_int(%s, &_bf_tmp)) {\n"
                       "            %s\n"
                       "          }\n"
                       "          ret->%s = _bf_tmp;\n"
                       "        }\n",
                       raw_expr, err_stmt, n));
    } else {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "        if (!_cdd_cli_parse_int(%s, &ret->%s)) {\n"
                            "          %s\n"
                            "        }\n",
                            raw_expr, n, err_stmt));
    }
  } else if (strcmp(t, "number") == 0) {
    CHECK_IO(
        FPRINTF_HOOK(fp,
                     "        if (!_cdd_cli_parse_double(%s, &ret->%s)) {\n"
                     "          %s\n"
                     "        }\n",
                     raw_expr, n, err_stmt));
  } else if (strcmp(t, "boolean") == 0) {
    if (f->bit_width[0] != '\0') {
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "        {\n"
                       "          int _bf_tmp = 0;\n"
                       "          if (!_cdd_cli_parse_bool(%s, &_bf_tmp)) {\n"
                       "            %s\n"
                       "          }\n"
                       "          ret->%s = _bf_tmp;\n"
                       "        }\n",
                       raw_expr, err_stmt, n));
    } else {
      CHECK_IO(
          FPRINTF_HOOK(fp,
                       "        if (!_cdd_cli_parse_bool(%s, &ret->%s)) {\n"
                       "          %s\n"
                       "        }\n",
                       raw_expr, n, err_stmt));
    }
  } else if (strcmp(t, "enum") == 0) {
    cdd_c_error_t rc_ref = get_type_from_ref(r, &ref_t);
    if (rc_ref != CDD_C_SUCCESS)
      return rc_ref;
    if (f->bit_width[0] != '\0') {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "        {\n"
                            "          enum %s _bf_tmp;\n"
                            "          if (%s_from_str(%s, &_bf_tmp) != 0) {\n"
                            "            %s\n"
                            "          }\n"
                            "          ret->%s = _bf_tmp;\n"
                            "        }\n",
                            ref_t, ref_t, raw_expr, err_stmt, n));
    } else {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "        if (%s_from_str(%s, &ret->%s) != 0) {\n"
                            "          %s\n"
                            "        }\n",
                            ref_t, raw_expr, n, err_stmt));
    }
  } else {
    char delim = f->cli_value_delimiter;
    CHECK_IO(FPRINTF_HOOK(
        fp,
        "        {\n"
        "          const char *_cur = %s;\n"
        "%s"
        "          while (_cur) {\n"
        "            char _tok[512];\n"
        "            const char *_next = %s",
        raw_expr,
        f->fixed_array_len > 0 ? "          size_t _fix_idx = 0;\n" : "",
        delim ? "strchr(_cur, '" : "NULL;\n"));
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
    if (f->fixed_array_len > 0) {
      unsigned long flen = (unsigned long)f->fixed_array_len;
      if (strcmp(r, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "            if (_fix_idx < %lu) {\n"
                              "              char *_edup = strdup(_tok);\n"
                              "              if (!_edup) {\n"
                              "                %s\n"
                              "              }\n"
                              "              if (ret->%s[_fix_idx])\n"
                              "                free(ret->%s[_fix_idx]);\n"
                              "              ret->%s[_fix_idx++] = _edup;\n"
                              "            }\n",
                              flen, oom_stmt, n, n, n));
      } else if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "            if (_fix_idx < %lu) {\n"
                              "              int _iv = 0;\n"
                              "              if (!%s(_tok, &_iv)) {\n"
                              "                %s\n"
                              "              }\n"
                              "              ret->%s[_fix_idx++] = _iv;\n"
                              "            }\n",
                              flen,
                              strcmp(r, "boolean") == 0 ? "_cdd_cli_parse_bool"
                                                        : "_cdd_cli_parse_int",
                              err_stmt, n));
      } else if (strcmp(r, "number") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "            if (_fix_idx < %lu) {\n"
            "              double _dv = 0.0;\n"
            "              if (!_cdd_cli_parse_double(_tok, &_dv)) {\n"
            "                %s\n"
            "              }\n"
            "              ret->%s[_fix_idx++] = _dv;\n"
            "            }\n",
            flen, err_stmt, n));
      }
    } else if (strcmp(r, "string") == 0) {
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
    if (f->serde_skip || f->cli_skip || f->cli_skip_from_cli)
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
    if (f->serde_skip || f->cli_skip || f->cli_skip_from_cli || f->cli_hidden ||
        f->cli_positional)
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
    enum CddCliAction act = CDD_CLI_ACTION_SET;
    if (f->serde_skip || f->cli_skip || f->cli_skip_from_cli)
      continue;
    rc = cdd_cli_effective_action(f, &act);
    if (act == CDD_CLI_ACTION_SET_FALSE) {
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
    if (f->serde_skip || f->cli_skip || f->cli_skip_from_cli ||
        f->cli_subcommand || f->serde_flatten)
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
      enum CddCliAction act = CDD_CLI_ACTION_SET;
      rc = cdd_cli_effective_action(f, &act);
      if (f->serde_skip || f->cli_skip || f->cli_skip_from_cli ||
          f->cli_positional || f->cli_subcommand || f->serde_flatten)
        continue;
      rc = cdd_cli_resolve_field_long(sf, f, lname, sizeof(lname));
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
    enum CddCliAction act = CDD_CLI_ACTION_SET;
    const char *cmp_fn =
        sf->cli_case_insensitive_flags ? "_cdd_cli_str_ieq" : "0 == strcmp";

    rc = cdd_cli_effective_action(f, &act);
    if (f->serde_skip || f->cli_skip || f->cli_skip_from_cli ||
        f->cli_positional || f->cli_subcommand)
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
    enum CddCliAction act = CDD_CLI_ACTION_SET;
    rc = cdd_cli_effective_action(f, &act);
    if (f->serde_skip || f->cli_skip || f->cli_skip_from_cli ||
        f->cli_positional || f->cli_subcommand || f->serde_flatten)
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
    rc = cdd_cli_find_type_def(config ? config->type_registry : NULL, ref_t,
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
          if (vf->serde_skip || vf->cli_skip || vf->cli_skip_from_cli)
            continue;
          rc = get_type_from_ref(vf->ref, &vref);
          if (vf->serde_rename[0] != '\0') {
            rc = cdd_cli_copy_str(cmd_name, sizeof(cmd_name), vf->serde_rename);
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
        if (vf->serde_skip || vf->cli_skip || vf->cli_skip_from_cli)
          continue;
        rc = get_type_from_ref(vf->ref, &vref);
        if (vf->serde_rename[0] != '\0') {
          rc = cdd_cli_copy_str(cmd_name, sizeof(cmd_name), vf->serde_rename);
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
      if (f->serde_skip || f->cli_skip || f->cli_skip_from_cli ||
          !f->cli_positional)
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
    if (f->serde_skip || f->cli_skip || f->cli_skip_from_cli)
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
        rc = cdd_escape_json_c_str(f->cli_required_if_val, esc_rv,
                                   sizeof(esc_rv));
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
