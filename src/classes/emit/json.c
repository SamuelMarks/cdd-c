/**
 * @file json.c
 * @brief Implementation of JSON generation logic.
 *
 * Emits C code.
 * - Serialization: Manual string concatenation (using `jasprintf`).
 * - Deserialization: Uses `parson` library API (`json_object_get_...`).
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd_stdbool.h"
#include "classes/emit/json.h"
#include "classes/emit/struct.h" /* for get_type_from_ref */
#include "classes/parse/cdd_serde.h"
#include "functions/parse/str.h" /* for string helpers */
#include "win_compat_sym.h"
#include "c_cdd/code_writer.h"
#include "c_cdd/log.h"
#include "c_cdd/safe_crt.h"
#include <stdarg.h>
/* clang-format on */

/* Terser error checking */
/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  for (; (x) < 0;)                                                             \
  return CDD_C_ERROR_IO

/*
 * @brief Emits C source code for struct to json func.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields descriptor container.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_struct_to_json_func(FILE *fp, const char *struct_name,
                          const struct StructFields *sf,
                          const struct CodegenJsonConfig *config) {
  size_t i;
  int iter_needed = 0;
  int need_comma_needed = 0;
  int scan_emitted = 0;
  int scan_guaranteed = 0;
  int emitted_any = 0;
  int guaranteed_emitted = 0;

  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Pre-scan to determine variable needs */
  for (i = 0; i < sf->size; ++i) {
    int is_cond;
    if (sf->fields[i].write_only || sf->fields[i].serde_skip ||
        sf->fields[i].serde_skip_serializing)
      continue;
    is_cond = (sf->fields[i].serde_skip_if_kind != CDD_SERDE_SKIP_IF_NONE) ||
              (sf->fields[i].serde_flatten &&
               strcmp(sf->fields[i].type, "object") == 0);
    if (scan_emitted && !scan_guaranteed)
      need_comma_needed = 1;
    scan_emitted = 1;
    if (!is_cond)
      scan_guaranteed = 1;
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = 1;
  }

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(fp,
                        "cdd_c_error_t %s_to_json(const struct %s *obj, "
                        "char **const json) {\n",
                        struct_name, struct_name));

  /* Variables decl */
  CHECK_IO(FPRINTF_HOOK(fp, "  int rc;\n"));
  if (need_comma_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  int need_comma = 0;\n"));
  if (iter_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  size_t i;\n"));

  /* Initial Safety Checks */
  CHECK_IO(FPRINTF_HOOK(fp, "  if (obj == NULL || json == NULL) return "
                            "CDD_C_ERROR_INVALID_ARGUMENT;\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  rc = c89stringutils_jasprintf(json, \"{\");\n"));
  CHECK_IO(FPRINTF_HOOK(
      fp, "  if (rc < 0 || *json == NULL) return CDD_C_ERROR_MEMORY;\n\n"));

  for (i = 0; i < sf->size; ++i) {
    const struct StructField *f = &sf->fields[i];
    const char *n = f->name;
    const char *t = f->type;
    const char *r = f->ref;
    char kn[128];
    int has_skip_if = 0;
    cdd_c_error_t rc_name;

    if (f->write_only || f->serde_skip || f->serde_skip_serializing)
      continue;

    rc_name = cdd_serde_resolve_field_names(sf, i, kn, sizeof(kn), NULL, 0);
    if (rc_name != CDD_C_SUCCESS)
      return rc_name;
    {
      char raw_kn[128];
      CDD_STRCPY(raw_kn, sizeof(raw_kn), kn);
      cdd_escape_json_c_str(raw_kn, kn, sizeof(kn));
    }

    if (f->serde_skip_if_kind == CDD_SERDE_SKIP_IF_NULL) {
      CHECK_IO(FPRINTF_HOOK(fp, "  if (obj->%s != NULL) {\n", n));
      has_skip_if = 1;
    } else if (f->serde_skip_if_kind == CDD_SERDE_SKIP_IF_EMPTY) {
      if (strcmp(t, "string") == 0) {
        if (f->fixed_array_len > 0) {
          CHECK_IO(FPRINTF_HOOK(fp, "  if (obj->%s[0] != '\\0') {\n", n));
        } else {
          CHECK_IO(FPRINTF_HOOK(
              fp, "  if (obj->%s != NULL && obj->%s[0] != '\\0') {\n", n, n));
        }
        has_skip_if = 1;
      } else if (strcmp(t, "array") == 0) {
        if (f->fixed_array_len == 0) {
          CHECK_IO(FPRINTF_HOOK(
              fp, "  if (obj->%s != NULL && obj->n_%s > 0) {\n", n, n));
          has_skip_if = 1;
        }
      } else {
        CHECK_IO(FPRINTF_HOOK(fp, "  if (obj->%s != NULL) {\n", n));
        has_skip_if = 1;
      }
    } else if (f->serde_skip_if_kind == CDD_SERDE_SKIP_IF_ZERO) {
      CHECK_IO(FPRINTF_HOOK(fp, "  if (obj->%s != 0) {\n", n));
      has_skip_if = 1;
    } else if (f->serde_skip_if_kind == CDD_SERDE_SKIP_IF_FUNC) {
      CHECK_IO(
          FPRINTF_HOOK(fp, "  if (!%s(obj->%s)) {\n", f->serde_skip_if_fn, n));
      has_skip_if = 1;
    }

    if (f->serde_flatten && strcmp(t, "object") == 0) {
      const char *tn = NULL;
      cdd_c_error_t rc_ref = get_type_from_ref(r, &tn);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(FPRINTF_HOOK(fp, "  if (obj->%s) {\n", n));
      CHECK_IO(FPRINTF_HOOK(fp, "    char *s = NULL;\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "    size_t slen;\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "    rc = %s_to_json(obj->%s, &s);\n", tn, n));
      CHECK_IO(FPRINTF_HOOK(fp, "    if (rc) { free(s); return rc; }\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "    slen = s ? strlen(s) : 0;\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "    if (slen > 2 && s[0] == '{' && "
                                "s[slen - 1] == '}') {\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "      s[slen - 1] = '\\0';\n"));
      if (guaranteed_emitted) {
        CHECK_IO(FPRINTF_HOOK(
            fp, "      rc = c89stringutils_jasprintf(json, \",\"); "
                "if (rc < 0 || *json==NULL) { free(s); return "
                "CDD_C_ERROR_MEMORY; }\n"));
      } else if (emitted_any) {
        CHECK_IO(FPRINTF_HOOK(
            fp, "      if (need_comma) { rc = c89stringutils_jasprintf(json, "
                "\",\"); if (rc < 0 || *json==NULL) { free(s); return "
                "CDD_C_ERROR_MEMORY; } }\n"));
      }
      CHECK_IO(FPRINTF_HOOK(
          fp, "      rc = c89stringutils_jasprintf(json, \"%%s\", s + 1);\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "      if (rc < 0 || *json == NULL) { free(s); "
                                "return CDD_C_ERROR_MEMORY; }\n"));
      if (need_comma_needed && !guaranteed_emitted)
        CHECK_IO(FPRINTF_HOOK(fp, "      need_comma = 1;\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "    }\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "    free(s);\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));
      if (has_skip_if)
        CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));
      emitted_any = 1;
      continue;
    }

    if (guaranteed_emitted) {
      CHECK_IO(FPRINTF_HOOK(fp, "  rc = c89stringutils_jasprintf(json, \",\"); "
                                "if (rc < 0 || *json==NULL) return "
                                "CDD_C_ERROR_MEMORY;\n"));
    } else if (emitted_any) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  if (need_comma) { rc = c89stringutils_jasprintf(json, \",\"); "
              "if (rc < 0 || *json==NULL) return CDD_C_ERROR_MEMORY; }\n"));
    }

    if (f->serde_serialize_with[0] != '\0' || f->serde_with[0] != '\0') {
      char fn_buf[128];
      if (f->serde_serialize_with[0] != '\0') {
        CDD_STRCPY(fn_buf, sizeof(fn_buf), f->serde_serialize_with);
      } else {
        CDD_SNPRINTF(fn_buf, sizeof(fn_buf), "%s_to_json", f->serde_with);
      }
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { char *s = NULL; rc = %s(obj->%s, &s); if (rc) { free(s); "
          "return rc; }\n"
          "    rc = c89stringutils_jasprintf(json, \"\\\"%s\\\": %%s\", s); "
          "free(s); }\n",
          fn_buf, n, kn));
    } else if (strcmp(t, "integer") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  rc = c89stringutils_jasprintf(json, "
                            "\"\\\"%s\\\": %%d\", obj->%s);\n",
                            kn, n));
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  rc = c89stringutils_jasprintf(json, "
                            "\"\\\"%s\\\": %%f\", obj->%s);\n",
                            kn, n));
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  rc = c89stringutils_jasprintf(json, "
                            "\"\\\"%s\\\": %%s\", obj->%s ? "
                            "\"true\" : \"false\");\n",
                            kn, n));
    } else if (strcmp(t, "string") == 0) {
      if (f->fixed_array_len > 0) {
        CHECK_IO(
            FPRINTF_HOOK(fp,
                         "  rc = c89stringutils_jasprintf(json, \"\\\"%s\\\": "
                         "\\\"%%s\\\"\", obj->%s);\n",
                         kn, n));
      } else {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  if (obj->%s) rc = c89stringutils_jasprintf(json, \"\\\"%s\\\": "
            "\\\"%%s\\\"\", obj->%s);\n",
            n, kn, n));
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  else rc = c89stringutils_jasprintf(json, "
                              "\"\\\"%s\\\": null\");\n",
                              kn));
      }
    } else if (strcmp(t, "object") == 0) {
      /* Recursive call */
      CHECK_IO(FPRINTF_HOOK(fp, "  if (obj->%s) {\n", n));
      CHECK_IO(FPRINTF_HOOK(fp, "    char *s = NULL;\n"));
      {
        const char *tn = NULL;
        cdd_c_error_t rc_ref = get_type_from_ref(r, &tn);
        int io_rc;
        if (rc_ref != CDD_C_SUCCESS) {
          return rc_ref;
        }
        io_rc = FPRINTF_HOOK(fp, "    rc = %s_to_json(obj->%s, &s);\n", tn, n);
        CHECK_IO(io_rc);
      }
      CHECK_IO(FPRINTF_HOOK(fp, "    if (rc) { free(s); return rc; }\n"));
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "    rc = c89stringutils_jasprintf(json, \"\\\"%s\\\": %%s\", s);\n",
          kn));
      CHECK_IO(FPRINTF_HOOK(fp, "    free(s);\n"));
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  } else rc = c89stringutils_jasprintf(json, "
                            "\"\\\"%s\\\": null\");\n",
                            kn));
    } else if (strcmp(t, "enum") == 0) {
      {
        const char *tn = NULL;
        cdd_c_error_t rc_ref = get_type_from_ref(r, &tn);
        int io_rc;
        if (rc_ref != CDD_C_SUCCESS) {
          return rc_ref;
        }
        io_rc = FPRINTF_HOOK(fp,
                             "  { char *s=NULL; rc=%s_to_str(obj->%s, &s); if "
                             "(rc) { free(s); return rc; }\n",
                             tn, n);
        CHECK_IO(io_rc);
      }
      CHECK_IO(FPRINTF_HOOK(fp,
                            "    rc = c89stringutils_jasprintf(json, "
                            "\"\\\"%s\\\": \\\"%%s\\\"\", s); free(s); }\n",
                            kn));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  rc = c89stringutils_jasprintf(json, \"\\\"%s\\\": [\");\n",
          kn));
      CHECK_IO(FPRINTF_HOOK(
          fp, "  if (rc < 0 || *json==NULL) return CDD_C_ERROR_MEMORY;\n"));
      if (f->fixed_array_len > 0) {
        CHECK_IO(FPRINTF_HOOK(fp, "  for (i=0; i < %lu; ++i) {\n",
                              (unsigned long)f->fixed_array_len));
      } else {
        CHECK_IO(FPRINTF_HOOK(fp, "  for (i=0; i < obj->n_%s; ++i) {\n", n));
      }
      /* Loop Body handling type */
      if (strcmp(r, "integer") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "    rc = c89stringutils_jasprintf(json, \"%%d\", obj->%s[i]);\n",
            n));
      } else if (strcmp(r, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "    rc = c89stringutils_jasprintf(json, "
                              "\"\\\"%%s\\\"\", obj->%s[i]);\n",
                              n));
      } else { /* Object array */
        {
          const char *tn = NULL;
          cdd_c_error_t rc_ref = get_type_from_ref(r, &tn);
          int io_rc;
          if (rc_ref != CDD_C_SUCCESS) {
            return rc_ref;
          }
          io_rc =
              FPRINTF_HOOK(fp,
                           "    { char *s=NULL; rc=%s_to_json(obj->%s[i], &s); "
                           "if (rc) { free(s); return rc; }\n",
                           tn, n);
          CHECK_IO(io_rc);
        }
        CHECK_IO(FPRINTF_HOOK(fp, "      rc = c89stringutils_jasprintf(json, "
                                  "\"%%s\", s); free(s); }\n"));
      }
      CHECK_IO(FPRINTF_HOOK(
          fp, "    if (rc < 0 || *json==NULL) return CDD_C_ERROR_MEMORY;\n"));
      if (f->fixed_array_len > 0) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "    if (i+1 < %lu) { "
                              "rc = c89stringutils_jasprintf(json, \",\"); "
                              "if (rc < 0 || *json==NULL) return "
                              "CDD_C_ERROR_MEMORY; }\n",
                              (unsigned long)f->fixed_array_len));
      } else {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "    if (i+1 < obj->n_%s) { "
                              "rc = c89stringutils_jasprintf(json, \",\"); "
                              "if (rc < 0 || *json==NULL) return "
                              "CDD_C_ERROR_MEMORY; }\n",
                              n));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));
      CHECK_IO(
          FPRINTF_HOOK(fp, "  rc = c89stringutils_jasprintf(json, \"]\");\n"));
    }

    CHECK_IO(FPRINTF_HOOK(
        fp, "  if (rc < 0 || *json == NULL) return CDD_C_ERROR_MEMORY;\n"));
    emitted_any = 1;
    if (!has_skip_if)
      guaranteed_emitted = 1;
    if (need_comma_needed && !guaranteed_emitted)
      CHECK_IO(FPRINTF_HOOK(fp, "  need_comma = 1;\n"));
    if (has_skip_if)
      CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));
  }

  CHECK_IO(FPRINTF_HOOK(fp, "  rc = c89stringutils_jasprintf(json, \"}\");\n"));
  CHECK_IO(FPRINTF_HOOK(
      fp, "  if (rc < 0 || *json == NULL) return CDD_C_ERROR_MEMORY;\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  return CDD_C_SUCCESS;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits C source code for struct from json func.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] config Optional code generation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t
write_struct_from_json_func(FILE *fp, const char *struct_name,
                            const struct CodegenJsonConfig *config) {
  if (!fp || !struct_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_from_json(const char *json_str, struct %s **const "
      "out) {\n"
      "  JSON_Value *val = json_parse_string(json_str);\n"
      "  cdd_c_error_t rc = CDD_C_SUCCESS;\n"
      "  if (!val) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "  rc = %s_from_jsonObject(json_value_get_object(val), out);\n"
      "  json_value_free(val);\n"
      "  return rc;\n"
      "}\n",
      struct_name, struct_name, struct_name));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return CDD_C_SUCCESS;
}

/*
 * @brief Emits C source code for struct array from json func.
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] config Optional code generation configuration.
 * @return 0 on success, or an error code.
 */
cdd_c_error_t
write_struct_array_from_json_func(FILE *fp, const char *struct_name,
                                  const struct CodegenJsonConfig *config) {
  if (!fp || !struct_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_array_from_json(const char *json_str, struct %s "
      "***out, size_t "
      "*out_len) {\n"
      "  JSON_Value *val = json_parse_string(json_str);\n"
      "  JSON_Array *arr = NULL;\n"
      "  size_t i, count;\n"
      "  struct %s **tmp = NULL;\n"
      "  cdd_c_error_t rc = CDD_C_SUCCESS;\n"
      "  if (!val) return CDD_C_ERROR_INVALID_ARGUMENT;\n"
      "  arr = json_value_get_array(val);\n"
      "  if (!arr) { json_value_free(val); return "
      "CDD_C_ERROR_INVALID_ARGUMENT; }\n",
      struct_name, struct_name, struct_name));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "  count = json_array_get_count(arr);\n"
      "  if (count == 0) {\n"
      "    *out = NULL;\n"
      "    *out_len = 0;\n"
      "    json_value_free(val);\n"
      "    return CDD_C_SUCCESS;\n"
      "  }\n"
      "  tmp = (struct %s **)calloc(count, sizeof(struct %s *));\n"
      "  if (!tmp) { json_value_free(val); return CDD_C_ERROR_MEMORY; }\n"
      "  for (i = 0; i < count; ++i) {\n"
      "    rc = %s_from_jsonObject(json_array_get_object(arr, i), &tmp[i]);\n"
      "    if (rc != CDD_C_SUCCESS) break;\n"
      "  }\n",
      struct_name, struct_name, struct_name));

  CHECK_IO(
      FPRINTF_HOOK(fp,
                   "  if (rc == CDD_C_SUCCESS) {\n"
                   "    *out = tmp;\n"
                   "    *out_len = count;\n"
                   "  } else {\n"
                   "    size_t k;\n"
                   "    for (k = 0; k < i; ++k) {\n"
                   "      if (tmp[k]) { %s_cleanup(tmp[k]); free(tmp[k]); }\n"
                   "    }\n"
                   "    free(tmp);\n"
                   "  }\n"
                   "  json_value_free(val);\n"
                   "  return rc;\n"
                   "}\n",
                   struct_name));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n", config->guard_macro));

  return CDD_C_SUCCESS;
}

/*
 * @brief Generate `_from_jsonObject` implementation (Core Logic).
 *
 * @param[in] fp Output file stream.
 * @param[in] struct_name Name of the C struct.
 * @param[in] sf Struct fields descriptor container.
 * @param[in] config Optional code generation configuration.
 * @return 0 on success.
 */
cdd_c_error_t
write_struct_from_jsonObject_func(FILE *fp, const char *struct_name,
                                  const struct StructFields *sf,
                                  const struct CodegenJsonConfig *config) {
  size_t i;
  int iter_needed = 0;
  int rc_needed = 0;
  int tmp_needed = 0;
  int len_needed = 0;

  if (!fp || !struct_name || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < sf->size; ++i) {
    if (sf->fields[i].read_only || sf->fields[i].serde_skip ||
        sf->fields[i].serde_skip_deserializing) {
      if (sf->fields[i].serde_default_fn[0] != '\0')
        rc_needed = 1;
      continue;
    }
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = 1;
    if (strcmp(sf->fields[i].type, "object") == 0 ||
        strcmp(sf->fields[i].type, "enum") == 0 ||
        strcmp(sf->fields[i].type, "array") == 0 ||
        sf->fields[i].serde_deserialize_with[0] != '\0' ||
        sf->fields[i].serde_with[0] != '\0' ||
        sf->fields[i].serde_default_fn[0] != '\0') {
      rc_needed = 1;
    }
    if (sf->fields[i].has_min || sf->fields[i].has_max ||
        sf->fields[i].exclusive_min || sf->fields[i].exclusive_max) {
      tmp_needed = 1;
    }
    if (sf->fields[i].has_min_len || sf->fields[i].has_max_len ||
        sf->fields[i].pattern[0] != '\0') {
      len_needed = 1;
    }
  }

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "cdd_c_error_t %s_from_jsonObject(const JSON_Object *jsonObject, "
      "struct %s **const out) {\n",
      struct_name, struct_name));

  if (rc_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  int rc;\n"));
  if (iter_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  size_t i;\n  const JSON_Array *arr;\n"));
  if (tmp_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  double tmp;\n"));
  if (len_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  size_t len;\n"));

  CHECK_IO(FPRINTF_HOOK(fp, "  struct %s *ret = calloc(1, sizeof(*ret));\n",
                        struct_name));
  CHECK_IO(FPRINTF_HOOK(fp, "  if (!ret) return CDD_C_ERROR_MEMORY;\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  if (!jsonObject || !out) { free(ret); return "
                            "CDD_C_ERROR_INVALID_ARGUMENT; "
                            "}\n\n"));

  if (sf->serde_deny_unknown_fields && !sf->serde_untagged) {
    size_t j, a;
    CHECK_IO(FPRINTF_HOOK(fp, "  {\n    size_t _k, _cnt = "
                              "json_object_get_count(jsonObject);\n"
                              "    for (_k = 0; _k < _cnt; ++_k) {\n"
                              "      const char *_jn = "
                              "json_object_get_name(jsonObject, _k);\n"
                              "      if (_jn"));
    for (j = 0; j < sf->size; ++j) {
      char dname[128];
      char esc_dname[256];
      const struct StructField *jf = &sf->fields[j];
      if (jf->read_only || jf->serde_skip || jf->serde_skip_deserializing)
        continue;
      if (!jf->serde_flatten) {
        if (cdd_serde_resolve_field_names(sf, j, NULL, 0, dname,
                                          sizeof(dname)) != CDD_C_SUCCESS)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        cdd_escape_json_c_str(dname, esc_dname, sizeof(esc_dname));
        CHECK_IO(FPRINTF_HOOK(fp, " && strcmp(_jn, \"%s\") != 0", esc_dname));
      }
      for (a = 0; a < jf->n_serde_aliases; ++a) {
        char esc_al[256];
        cdd_escape_json_c_str(jf->serde_aliases[a], esc_al, sizeof(esc_al));
        CHECK_IO(FPRINTF_HOOK(fp, " && strcmp(_jn, \"%s\") != 0", esc_al));
      }
    }
    CHECK_IO(FPRINTF_HOOK(fp, ") {\n        free(ret);\n        return "
                              "CDD_C_ERROR_INVALID_ARGUMENT;\n      }\n"
                              "    }\n  }\n"));
  }

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = sf->fields[i].ref;
    struct StructField *f = &sf->fields[i];
    const int has_def = f->serde_has_default || sf->serde_default;
    char kn[256];
    cdd_c_error_t rc_name;

    if (f->read_only || f->serde_skip || f->serde_skip_deserializing) {
      if (f->serde_default_fn[0] != '\0') {
        if (f->bit_width[0] != '\0') {
          CHECK_IO(FPRINTF_HOOK(fp,
                                "  { int _bf_tmp = 0; rc = %s(&_bf_tmp);\n"
                                "    if (rc) { %s_cleanup(ret); return rc; }\n"
                                "    ret->%s = _bf_tmp; }\n",
                                f->serde_default_fn, struct_name, n));
        } else {
          CHECK_IO(FPRINTF_HOOK(fp,
                                "  rc = %s(&ret->%s);\n"
                                "  if (rc) { %s_cleanup(ret); return rc; }\n",
                                f->serde_default_fn, n, struct_name));
        }
      } else if (f->default_val[0] != '\0') {
        if (strcmp(t, "string") == 0) {
          if (f->fixed_array_len > 0) {
            unsigned long cap = (unsigned long)f->fixed_array_len;
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "  strncpy(ret->%s, \"%s\", %lu);\n"
                                  "  ret->%s[%lu] = '\\0';\n",
                                  n, f->default_val, cap > 0 ? cap - 1 : 0, n,
                                  cap > 0 ? cap - 1 : 0));
          } else {
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "  ret->%s = strdup(\"%s\");\n"
                "  if (!ret->%s) { %s_cleanup(ret); return CDD_C_ERROR_MEMORY; "
                "}\n",
                n, f->default_val, n, struct_name));
          }
        } else {
          CHECK_IO(FPRINTF_HOOK(fp, "  ret->%s = %s;\n", n, f->default_val));
        }
      }
      continue;
    }

    {
      char raw_kn[128];
      rc_name =
          cdd_serde_resolve_field_names(sf, i, NULL, 0, raw_kn, sizeof(raw_kn));
      if (rc_name != CDD_C_SUCCESS)
        return rc_name;
      cdd_escape_json_c_str(raw_kn, kn, sizeof(kn));
    }

    if (f->serde_flatten && strcmp(t, "object") == 0) {
      const char *tn = NULL;
      cdd_c_error_t rc_ref = get_type_from_ref(r, &tn);
      if (rc_ref != CDD_C_SUCCESS)
        return rc_ref;
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  rc = %s_from_jsonObject(jsonObject, &ret->%s);\n"
                            "  if (rc) { %s_cleanup(ret); return rc; }\n",
                            tn, n, struct_name));
      continue;
    }

    if (f->required && !has_def && f->serde_default_fn[0] == '\0') {
      size_t a;
      CHECK_IO(FPRINTF_HOOK(
          fp, "  if (!json_object_has_value(jsonObject, \"%s\")", kn));
      for (a = 0; a < f->n_serde_aliases; ++a) {
        CHECK_IO(FPRINTF_HOOK(fp,
                              " && !json_object_has_value(jsonObject, \"%s\")",
                              f->serde_aliases[a]));
      }
      CHECK_IO(FPRINTF_HOOK(
          fp, ") { %s_cleanup(ret); return CDD_C_ERROR_INVALID_ARGUMENT; }\n",
          struct_name));
    }

    if (f->serde_deserialize_with[0] != '\0' || f->serde_with[0] != '\0') {
      char fn_buf[128];
      if (f->serde_deserialize_with[0] != '\0') {
        CDD_STRCPY(fn_buf, sizeof(fn_buf), f->serde_deserialize_with);
      } else {
        CDD_SNPRINTF(fn_buf, sizeof(fn_buf), "%s_from_jsonObject",
                     f->serde_with);
      }
      if (f->bit_width[0] != '\0') {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  { int _bf_tmp = 0; rc = %s(jsonObject, \"%s\", &_bf_tmp);\n"
            "    if (rc) { %s_cleanup(ret); return rc; }\n"
            "    ret->%s = _bf_tmp; }\n",
            fn_buf, kn, struct_name, n));
      } else {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "  rc = %s(jsonObject, \"%s\", &ret->%s);\n"
                              "  if (rc) { %s_cleanup(ret); return rc; }\n",
                              fn_buf, kn, n, struct_name));
      }
      continue;
    }

    if (strcmp(t, "integer") == 0) {
      if (f->n_serde_aliases == 0 && !has_def &&
          f->serde_default_fn[0] == '\0') {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  ret->%s = (int)json_object_get_number(jsonObject, \"%s\");\n", n,
            kn));
      } else {
        size_t a;
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  if (json_object_has_value(jsonObject, \"%s\")) ret->%s = "
            "(int)json_object_get_number(jsonObject, \"%s\");\n",
            kn, n, kn));
        for (a = 0; a < f->n_serde_aliases; ++a) {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "  else if (json_object_has_value(jsonObject, \"%s\")) "
              "ret->%s = (int)json_object_get_number(jsonObject, \"%s\");\n",
              f->serde_aliases[a], n, f->serde_aliases[a]));
        }
        if (f->serde_default_fn[0] != '\0') {
          if (f->bit_width[0] != '\0') {
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "  else { int _bf_tmp = 0; rc = %s(&_bf_tmp); if (rc) { "
                "%s_cleanup(ret); return rc; } ret->%s = _bf_tmp; }\n",
                f->serde_default_fn, struct_name, n));
          } else {
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "  else { rc = %s(&ret->%s); if (rc) { %s_cleanup(ret); return "
                "rc; } }\n",
                f->serde_default_fn, n, struct_name));
          }
        } else if (f->default_val[0] != '\0') {
          CHECK_IO(
              FPRINTF_HOOK(fp, "  else ret->%s = %s;\n", n, f->default_val));
        }
      }
      if (f->has_min || f->has_max || f->exclusive_min || f->exclusive_max) {
        CHECK_IO(FPRINTF_HOOK(fp, "  tmp = (double)ret->%s;\n", n));
        if (f->has_min) {
          if (f->exclusive_min)
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "  if (tmp <= %f) { %s_cleanup(ret); return "
                                  "CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                                  f->min_val, struct_name));
          else
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "  if (tmp < %f) { %s_cleanup(ret); return "
                                  "CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                                  f->min_val, struct_name));
        }
        if (f->has_max) {
          if (f->exclusive_max)
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "  if (tmp >= %f) { %s_cleanup(ret); return "
                                  "CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                                  f->max_val, struct_name));
          else
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "  if (tmp > %f) { %s_cleanup(ret); return "
                                  "CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                                  f->max_val, struct_name));
        }
      }
    } else if (strcmp(t, "number") == 0) {
      if (f->n_serde_aliases == 0 && !has_def &&
          f->serde_default_fn[0] == '\0') {
        CHECK_IO(FPRINTF_HOOK(
            fp, "  ret->%s = json_object_get_number(jsonObject, \"%s\");\n", n,
            kn));
      } else {
        size_t a;
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  if (json_object_has_value(jsonObject, \"%s\")) ret->%s = "
            "json_object_get_number(jsonObject, \"%s\");\n",
            kn, n, kn));
        for (a = 0; a < f->n_serde_aliases; ++a) {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "  else if (json_object_has_value(jsonObject, \"%s\")) "
              "ret->%s = json_object_get_number(jsonObject, \"%s\");\n",
              f->serde_aliases[a], n, f->serde_aliases[a]));
        }
        if (f->serde_default_fn[0] != '\0') {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "  else { rc = %s(&ret->%s); if (rc) { %s_cleanup(ret); return "
              "rc; } }\n",
              f->serde_default_fn, n, struct_name));
        } else if (f->default_val[0] != '\0') {
          CHECK_IO(
              FPRINTF_HOOK(fp, "  else ret->%s = %s;\n", n, f->default_val));
        }
      }
      if (f->has_min || f->has_max || f->exclusive_min || f->exclusive_max) {
        CHECK_IO(FPRINTF_HOOK(fp, "  tmp = ret->%s;\n", n));
        if (f->has_min) {
          if (f->exclusive_min)
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "  if (tmp <= %f) { %s_cleanup(ret); return "
                                  "CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                                  f->min_val, struct_name));
          else
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "  if (tmp < %f) { %s_cleanup(ret); return "
                                  "CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                                  f->min_val, struct_name));
        }
        if (f->has_max) {
          if (f->exclusive_max)
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "  if (tmp >= %f) { %s_cleanup(ret); return "
                                  "CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                                  f->max_val, struct_name));
          else
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "  if (tmp > %f) { %s_cleanup(ret); return "
                                  "CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                                  f->max_val, struct_name));
        }
      }
    } else if (strcmp(t, "boolean") == 0) {
      if (f->n_serde_aliases == 0 && !has_def &&
          f->serde_default_fn[0] == '\0') {
        CHECK_IO(FPRINTF_HOOK(
            fp, "  ret->%s = json_object_get_boolean(jsonObject, \"%s\");\n", n,
            kn));
      } else {
        size_t a;
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  if (json_object_has_value(jsonObject, \"%s\")) ret->%s = "
            "json_object_get_boolean(jsonObject, \"%s\");\n",
            kn, n, kn));
        for (a = 0; a < f->n_serde_aliases; ++a) {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "  else if (json_object_has_value(jsonObject, \"%s\")) "
              "ret->%s = json_object_get_boolean(jsonObject, \"%s\");\n",
              f->serde_aliases[a], n, f->serde_aliases[a]));
        }
        if (f->serde_default_fn[0] != '\0') {
          if (f->bit_width[0] != '\0') {
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "  else { int _bf_tmp = 0; rc = %s(&_bf_tmp); if (rc) { "
                "%s_cleanup(ret); return rc; } ret->%s = _bf_tmp; }\n",
                f->serde_default_fn, struct_name, n));
          } else {
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "  else { rc = %s(&ret->%s); if (rc) { %s_cleanup(ret); return "
                "rc; } }\n",
                f->serde_default_fn, n, struct_name));
          }
        } else if (f->default_val[0] != '\0') {
          CHECK_IO(
              FPRINTF_HOOK(fp, "  else ret->%s = %s;\n", n, f->default_val));
        }
      }
    } else if (strcmp(t, "string") == 0) {
      size_t a;
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { const char *s = json_object_get_string(jsonObject, \"%s\");\n",
          kn));
      for (a = 0; a < f->n_serde_aliases; ++a) {
        CHECK_IO(FPRINTF_HOOK(
            fp, "    if (!s) s = json_object_get_string(jsonObject, \"%s\");\n",
            f->serde_aliases[a]));
      }
      if (has_def && f->default_val[0] != '\0') {
        CHECK_IO(FPRINTF_HOOK(fp, "    if (!s) s = \"%s\";\n", f->default_val));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "    if (s) {\n"));
      if (f->fixed_array_len > 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "      if (strlen(s) >= %lu) { %s_cleanup(ret); return "
            "CDD_C_ERROR_INVALID_ARGUMENT; }\n"
            "      memcpy(ret->%s, s, strlen(s) + 1);\n",
            (unsigned long)f->fixed_array_len, struct_name, n));
      } else {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "      ret->%s = strdup(s);\n"
                              "      if (!ret->%s) { %s_cleanup(ret); return "
                              "CDD_C_ERROR_MEMORY; }\n",
                              n, n, struct_name));
      }
      if (f->has_min_len || f->has_max_len || f->pattern[0]) {
        CHECK_IO(FPRINTF_HOOK(fp, "      len = strlen(ret->%s);\n", n));
        if (f->has_min_len)
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "      if (len < %lu"
              ") { %s_cleanup(ret); return CDD_C_ERROR_INVALID_ARGUMENT; }\n",
              (unsigned long)f->min_len, struct_name));
        if (f->has_max_len)
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "      if (len > %lu"
              ") { %s_cleanup(ret); return CDD_C_ERROR_INVALID_ARGUMENT; }\n",
              (unsigned long)f->max_len, struct_name));
        if (f->pattern[0]) {
          if (strncmp(f->pattern, "^", 1) == 0 &&
              f->pattern[(size_t)strlen(f->pattern) - 1] ==
                  '$') { /* exact mismatch */
            /* strip anchors */
            char pat[256];
            size_t pl = (size_t)strlen(f->pattern) - 2;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            strncpy_s(pat, sizeof(pat), f->pattern + 1, pl);
#else
            strncpy(pat, f->pattern + 1, pl);
#endif
            pat[pl] = 0;
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "      if (strcmp(ret->%s, \"%s\") != 0) { "
                "%s_cleanup(ret); return CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                n, pat, struct_name));
          } else if (strncmp(f->pattern, "^", 1) == 0) { /* prefix */
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "      if (strncmp(ret->%s, \"%s\", %lu) != 0) { "
                "%s_cleanup(ret); return CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                n, f->pattern + 1, (unsigned long)(strlen(f->pattern) - 1),
                struct_name));
          } else if (f->pattern[(size_t)strlen(f->pattern) - 1] ==
                     '$') { /* suffix */
            char pat[256];
            size_t pl = (size_t)strlen(f->pattern) - 1;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            strncpy_s(pat, sizeof(pat), f->pattern, pl);
#else
            memcpy(pat, f->pattern, pl);
#endif
            pat[pl] = 0;
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "      if (len < %lu"
                                  " || strcmp(ret->%s + len - %lu, "
                                  "\"%s\") != 0) { %s_cleanup(ret); return "
                                  "CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                                  (unsigned long)pl, n, (unsigned long)pl, pat,
                                  struct_name));
          } else { /* contains */
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "      if (strstr(ret->%s, \"%s\") == NULL) { "
                "%s_cleanup(ret); return CDD_C_ERROR_INVALID_ARGUMENT; }\n",
                n, f->pattern, struct_name));
          }
        }
      }
      if (f->serde_default_fn[0] != '\0') {
        CHECK_IO(FPRINTF_HOOK(fp,
                              "    } else {\n"
                              "      rc = %s(&ret->%s);\n"
                              "      if (rc) { %s_cleanup(ret); return rc; }\n"
                              "    }\n  }\n",
                              f->serde_default_fn, n, struct_name));
      } else {
        CHECK_IO(FPRINTF_HOOK(fp, "    }\n  }\n"));
      }
    } else if (strcmp(t, "object") == 0) {
      size_t a;
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  { JSON_Object *sub = "
                            "json_object_get_object(jsonObject, \"%s\");\n",
                            kn));
      for (a = 0; a < f->n_serde_aliases; ++a) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "    if (!sub) sub = json_object_get_object(jsonObject, "
            "\"%s\");\n",
            f->serde_aliases[a]));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "    if (sub) {\n"));
      {
        const char *tn = NULL;
        cdd_c_error_t rc_ref = get_type_from_ref(r, &tn);
        int io_rc;
        if (rc_ref != CDD_C_SUCCESS) {
          return rc_ref;
        }
        io_rc = FPRINTF_HOOK(
            fp, "      rc = %s_from_jsonObject(sub, &ret->%s);\n", tn, n);
        CHECK_IO(io_rc);
      }
      CHECK_IO(FPRINTF_HOOK(
          fp, "      if (rc) { %s_cleanup(ret); return rc; }\n", struct_name));
      CHECK_IO(FPRINTF_HOOK(fp, "    }\n  }\n"));
    } else if (strcmp(t, "enum") == 0) {
      size_t a;
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { const char *s = json_object_get_string(jsonObject, \"%s\");\n",
          kn));
      for (a = 0; a < f->n_serde_aliases; ++a) {
        CHECK_IO(FPRINTF_HOOK(
            fp, "    if (!s) s = json_object_get_string(jsonObject, \"%s\");\n",
            f->serde_aliases[a]));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "    if (s) {\n"));
      {
        const char *tn = NULL;
        cdd_c_error_t rc_ref = get_type_from_ref(r, &tn);
        int io_rc;
        if (rc_ref != CDD_C_SUCCESS) {
          return rc_ref;
        }
        if (f->bit_width[0] != '\0') {
          io_rc = FPRINTF_HOOK(
              fp,
              "      { enum %s _bf_tmp; rc = %s_from_str(s, &_bf_tmp);\n"
              "        if (rc) { %s_cleanup(ret); return rc; }\n"
              "        ret->%s = _bf_tmp; }\n",
              tn, tn, struct_name, n);
          CHECK_IO(io_rc);
        } else {
          io_rc =
              FPRINTF_HOOK(fp, "      rc = %s_from_str(s, &ret->%s);\n", tn, n);
          CHECK_IO(io_rc);
          CHECK_IO(FPRINTF_HOOK(
              fp, "      if (rc) { %s_cleanup(ret); return rc; }\n",
              struct_name));
        }
      }
      CHECK_IO(FPRINTF_HOOK(fp, "    }\n  }\n"));
    } else if (strcmp(t, "array") == 0) {
      size_t a;
      CHECK_IO(FPRINTF_HOOK(
          fp, "  arr = json_object_get_array(jsonObject, \"%s\");\n", kn));
      for (a = 0; a < f->n_serde_aliases; ++a) {
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  if (!arr) arr = json_object_get_array(jsonObject, \"%s\");\n",
            f->serde_aliases[a]));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "  if (arr) {\n"));
      if (f->fixed_array_len > 0) {
        unsigned long flen = (unsigned long)f->fixed_array_len;
        CHECK_IO(FPRINTF_HOOK(fp,
                              "    size_t _cnt = json_array_get_count(arr);\n"
                              "    if (_cnt > %lu) _cnt = %lu;\n"
                              "    for (i = 0; i < _cnt; ++i) {\n",
                              flen, flen));
        if (strcmp(r, "integer") == 0 || strcmp(r, "boolean") == 0) {
          CHECK_IO(FPRINTF_HOOK(
              fp, "      ret->%s[i] = (int)json_array_get_number(arr, i);\n",
              n));
        } else if (strcmp(r, "number") == 0) {
          CHECK_IO(FPRINTF_HOOK(
              fp, "      ret->%s[i] = json_array_get_number(arr, i);\n", n));
        } else if (strcmp(r, "string") == 0) {
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "      const char *_es = json_array_get_string(arr, i);\n"
              "      if (_es) {\n"
              "        ret->%s[i] = strdup(_es);\n"
              "        if (!ret->%s[i]) { %s_cleanup(ret); return "
              "CDD_C_ERROR_MEMORY; }\n"
              "      }\n",
              n, n, struct_name));
        } else {
          const char *tn = NULL;
          get_type_from_ref(r, &tn);
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "      rc = %s_from_jsonObject(json_array_get_object(arr, i), "
              "&ret->%s[i]);\n"
              "      if (rc) { %s_cleanup(ret); return rc; }\n",
              tn, n, struct_name));
        }
        CHECK_IO(FPRINTF_HOOK(fp, "    }\n  }\n"));
        continue;
      }
      CHECK_IO(
          FPRINTF_HOOK(fp, "    ret->n_%s = json_array_get_count(arr);\n", n));
      CHECK_IO(FPRINTF_HOOK(fp, "    if (ret->n_%s > 0) {\n", n));
      if (strcmp(r, "integer") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp, "      ret->%s = malloc(ret->n_%s * sizeof(int));\n", n, n));
        CHECK_IO(FPRINTF_HOOK(fp,
                              "      for(i=0; i<ret->n_%s; ++i) ret->%s[i] = "
                              "(int)json_array_get_number(arr, i);\n",
                              n, n));
      } else if (strcmp(r, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp, "      ret->%s = calloc(ret->n_%s, sizeof(char *));\n", n, n));
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "      if (!ret->%s) { %s_cleanup(ret); return CDD_C_ERROR_MEMORY; "
            "}\n"
            "      for(i=0; i<ret->n_%s; ++i) {\n"
            "        const char *_es = json_array_get_string(arr, i);\n"
            "        if (_es) {\n"
            "          ret->%s[i] = strdup(_es);\n"
            "          if (!ret->%s[i]) { %s_cleanup(ret); return "
            "CDD_C_ERROR_MEMORY; }\n"
            "        }\n"
            "      }\n",
            n, struct_name, n, n, n, struct_name));
      } else {
        /* Object array */
        {
          const char *tn = NULL;
          cdd_c_error_t rc_ref = get_type_from_ref(r, &tn);
          int io_rc;
          if (rc_ref != CDD_C_SUCCESS) {
            return rc_ref;
          }
          io_rc = FPRINTF_HOOK(
              fp, "      ret->%s = calloc(ret->n_%s, sizeof(struct %s*));\n", n,
              n, tn);
          CHECK_IO(io_rc);
        }
        CHECK_IO(FPRINTF_HOOK(fp, "      for(i=0; i<ret->n_%s; ++i) {\n", n));
        {
          const char *tn = NULL;
          cdd_c_error_t rc_ref = get_type_from_ref(r, &tn);
          int io_rc;
          if (rc_ref != CDD_C_SUCCESS) {
            return rc_ref;
          }
          io_rc = FPRINTF_HOOK(
              fp,
              "        rc = %s_from_jsonObject(json_array_get_object(arr, i), "
              "&ret->%s[i]);\n",
              tn, n);
          CHECK_IO(io_rc);
        }
        CHECK_IO(
            FPRINTF_HOOK(fp, "        if(rc) { %s_cleanup(ret); return rc; }\n",
                         struct_name));
        CHECK_IO(FPRINTF_HOOK(fp, "      }\n"));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "    }\n  }\n"));
    }
  }

  CHECK_IO(FPRINTF_HOOK(fp, "  *out = ret;\n  return CDD_C_SUCCESS;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return CDD_C_SUCCESS;
}
