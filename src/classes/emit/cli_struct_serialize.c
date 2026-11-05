/**
 * @file cli_struct_serialize.c
 * @brief Implementation of C89 CLI serializer (`_to_cli_argv`) code generation.
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
    rc = cdd_cli_copy_str(default_prog, sizeof(default_prog), sf->cli_bin_name);
  } else if (sf->cli_name[0] != '\0') {
    rc = cdd_cli_copy_str(default_prog, sizeof(default_prog), sf->cli_name);
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
    enum CddCliAction act = CDD_CLI_ACTION_SET;

    rc = cdd_cli_effective_action(f, &act);
    if (f->serde_skip || f->cli_skip || f->cli_skip_to_cli ||
        f->cli_positional || f->cli_subcommand || f->serde_flatten)
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
      if (f->fixed_array_len > 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  if (obj->%s[0] != '\\0') {\n"
                              "    if (_argc + 3 >= _cap) {\n"
                              "      _cap *= 2;\n"
                              "      _argv = (char **)realloc(_argv, "
                              "(size_t)_cap * sizeof(char *));\n"
                              "    }\n"
                              "    _argv[_argc++] = strdup(\"%s\");\n"
                              "    _argv[_argc++] = strdup(obj->%s);\n"
                              "  }\n",
                              f->name, flag_tok, f->name));
      } else {
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
      }
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
      if (f->fixed_array_len > 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  for (_k = 0; _k < %lu; ++_k) {\n"
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
                              (unsigned long)f->fixed_array_len, f->name,
                              flag_tok, f->name));
      } else {
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
  }

  /* Emit positionals */
  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    if (f->serde_skip || f->cli_skip || f->cli_skip_to_cli ||
        !f->cli_positional)
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
      if (f->fixed_array_len > 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  if (obj->%s[0] != '\\0') {\n"
                              "    if (_argc + 2 >= _cap) {\n"
                              "      _cap *= 2;\n"
                              "      _argv = (char **)realloc(_argv, "
                              "(size_t)_cap * sizeof(char *));\n"
                              "    }\n"
                              "    _argv[_argc++] = strdup(obj->%s);\n"
                              "  }\n",
                              f->name, f->name));
      } else {
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
      }
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
      if (f->fixed_array_len > 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  for (_k = 0; _k < %lu; ++_k) {\n"
                              "    if (!obj->%s[_k])\n"
                              "      continue;\n"
                              "    if (_argc + 2 >= _cap) {\n"
                              "      _cap *= 2;\n"
                              "      _argv = (char **)realloc(_argv, "
                              "(size_t)_cap * sizeof(char *));\n"
                              "    }\n"
                              "    _argv[_argc++] = strdup(obj->%s[_k]);\n"
                              "  }\n",
                              (unsigned long)f->fixed_array_len, f->name,
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
