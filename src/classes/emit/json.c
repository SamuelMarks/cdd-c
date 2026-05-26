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
#include "functions/parse/str.h" /* for string helpers */
#include "win_compat_sym.h"
#include "c_cdd/log.h"
/* clang-format on */

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127) /* conditional expression is constant */
#endif

#ifdef CDD_BUILD_TESTS
#include <stdarg.h>
int g_fail_io_after = -1;
int g_io_calls = 0;
static int cdd_fprintf_hook(FILE *stream, const char *format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)));
#else
    ;
#endif
static int cdd_fprintf_hook(FILE *stream, const char *format, ...) {
  int ret;
  va_list args;
  if (g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after)
    return -1;
  va_start(args, format);
  ret = vfprintf(stream, format, args);
  va_end(args);
  return ret;
}
#define FPRINTF_HOOK cdd_fprintf_hook
#else
#define FPRINTF_HOOK fprintf
#endif

/* Terser error checking */
/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

/**
 * @brief Generates C code for write struct to json func.
 */
int write_struct_to_json_func(FILE *fp, const char *struct_name,
                              const struct StructFields *sf,
                              const struct CodegenJsonConfig *config) {
  size_t i;
  int iter_needed = 0;
  int rc_needed = 0;

  if (!fp || !struct_name || !sf)
    return EINVAL;

  /* Pre-scan to determine variable needs */
  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = 1;
    if (strcmp(sf->fields[i].type, "object") == 0 ||
        strcmp(sf->fields[i].type, "enum") == 0 ||
        strcmp(sf->fields[i].type, "array") == 0) {
      rc_needed = 1;
    }
  }

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(
      fp, "int %s_to_json(const struct %s *obj, char **const json) {\n",
      struct_name, struct_name));

  /* Variables decl */
  CHECK_IO(FPRINTF_HOOK(fp, "  int need_comma = 0;\n"));
  if (rc_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  int rc;\n"));
  if (iter_needed)
    CHECK_IO(FPRINTF_HOOK(fp, "  size_t i;\n"));

  /* Initial Safety Checks */
  CHECK_IO(
      FPRINTF_HOOK(fp, "  if (obj == NULL || json == NULL) return EINVAL;\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  jasprintf(json, \"{\");\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  if (*json == NULL) return ENOMEM;\n\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = sf->fields[i].ref;

    if (sf->fields[i].write_only)
      continue;

    CHECK_IO(FPRINTF_HOOK(fp, "  if (need_comma) { jasprintf(json, \",\"); if "
                              "(*json==NULL) return ENOMEM; }\n"));

    if (strcmp(t, "integer") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  jasprintf(json, \"\\\"%s\\\": %%d\", obj->%s);\n", n, n));
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  jasprintf(json, \"\\\"%s\\\": %%f\", obj->%s);\n", n, n));
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  jasprintf(json, \"\\\"%s\\\": %%s\", obj->%s ? "
                            "\"true\" : \"false\");\n",
                            n, n));
    } else if (strcmp(t, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  if (obj->%s) jasprintf(json, \"\\\"%s\\\": "
                            "\\\"%%s\\\"\", obj->%s);\n",
                            n, n, n));
      CHECK_IO(FPRINTF_HOOK(
          fp, "  else jasprintf(json, \"\\\"%s\\\": null\");\n", n));
    } else if (strcmp(t, "object") == 0) {
      /* Recursive call */
      CHECK_IO(FPRINTF_HOOK(fp, "  if (obj->%s) {\n", n));
      CHECK_IO(FPRINTF_HOOK(fp, "    char *s = NULL;\n"));
      {
        char *tn = NULL;
        get_type_from_ref(r, &tn);
        CHECK_IO(
            FPRINTF_HOOK(fp, "    rc = %s_to_json(obj->%s, &s);\n", tn, n));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "    if (rc) return rc;\n"));
      CHECK_IO(FPRINTF_HOOK(
          fp, "    jasprintf(json, \"\\\"%s\\\": %%s\", s);\n", n));
      CHECK_IO(FPRINTF_HOOK(fp, "    free(s);\n"));
      CHECK_IO(FPRINTF_HOOK(
          fp, "  } else jasprintf(json, \"\\\"%s\\\": null\");\n", n));
    } else if (strcmp(t, "enum") == 0) {
      {
        char *tn = NULL;
        get_type_from_ref(r, &tn);
        CHECK_IO(FPRINTF_HOOK(
            fp,
            "  { char *s=NULL; rc=%s_to_str(obj->%s, &s); if (rc) return rc;\n",
            tn, n));
      }
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "    jasprintf(json, \"\\\"%s\\\": \\\"%%s\\\"\", s); free(s); }\n",
          n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp, "  jasprintf(json, \"\\\"%s\\\": [\");\n", n));
      CHECK_IO(FPRINTF_HOOK(fp, "  if (*json==NULL) return ENOMEM;\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "  for (i=0; i < obj->n_%s; ++i) {\n", n));
      /* Loop Body handling type */
      if (strcmp(r, "integer") == 0) {
        CHECK_IO(
            FPRINTF_HOOK(fp, "    jasprintf(json, \"%%d\", obj->%s[i]);\n", n));
      } else if (strcmp(r, "string") == 0) {
        CHECK_IO(FPRINTF_HOOK(
            fp, "    jasprintf(json, \"\\\"%%s\\\"\", obj->%s[i]);\n", n));
      } else { /* Object array */
        {
          char *tn = NULL;
          get_type_from_ref(r, &tn);
          CHECK_IO(
              FPRINTF_HOOK(fp,
                           "    { char *s=NULL; rc=%s_to_json(obj->%s[i], &s); "
                           "if (rc) return rc;\n",
                           tn, n));
        }
        CHECK_IO(FPRINTF_HOOK(
            fp, "      jasprintf(json, \"%%s\", s); free(s); }\n"));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "    if (*json==NULL) return ENOMEM;\n"));
      CHECK_IO(FPRINTF_HOOK(
          fp, "    if (i+1 < obj->n_%s) jasprintf(json, \",\");\n", n));
      CHECK_IO(FPRINTF_HOOK(fp, "  }\n"));
      CHECK_IO(FPRINTF_HOOK(fp, "  jasprintf(json, \"]\");\n"));
    }

    CHECK_IO(FPRINTF_HOOK(fp, "  if (*json == NULL) return ENOMEM;\n"));
    CHECK_IO(FPRINTF_HOOK(fp, "  need_comma = 1;\n"));
  }

  CHECK_IO(FPRINTF_HOOK(fp, "  jasprintf(json, \"}\");\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  if (*json == NULL) return ENOMEM;\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "  return 0;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}

/**
 * @brief Generates C code for write struct from json func.
 */
int write_struct_from_json_func(FILE *fp, const char *struct_name,
                                const struct CodegenJsonConfig *config) {
  if (!fp || !struct_name)
    return EINVAL;

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "int %s_from_json(const char *json_str, struct %s **const out) {\n"
      "  JSON_Value *val = json_parse_string(json_str);\n"
      "  int rc = 0;\n"
      "  if (!val) return EINVAL;\n"
      "  rc = %s_from_jsonObject(json_value_get_object(val), out);\n"
      "  json_value_free(val);\n"
      "  return rc;\n"
      "}\n",
      struct_name, struct_name, struct_name));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}

/**
 * @brief Generates C code for write struct from jsonObject func.
 */
/**
 * @brief Generates C code for write struct array from json func.
 * @param fp The file pointer to write to.
 * @param struct_name The name of the struct.
 * @param config The code generation configuration.
 * @return 0 on success, or an error code.
 */
int write_struct_array_from_json_func(FILE *fp, const char *struct_name,
                                      const struct CodegenJsonConfig *config) {
  if (!fp || !struct_name)
    return EINVAL;

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(FPRINTF_HOOK(
      fp,
      "int %s_array_from_json(const char *json_str, struct %s ***out, size_t "
      "*out_len) {\n"
      "  JSON_Value *val = json_parse_string(json_str);\n"
      "  JSON_Array *arr = NULL;\n"
      "  size_t i, count;\n"
      "  struct %s **tmp = NULL;\n"
      "  int rc = 0;\n"
      "  if (!val) return 22; /* EINVAL */\n"
      "  arr = json_value_get_array(val);\n"
      "  if (!arr) { json_value_free(val); return 22; }\n"
      "  count = json_array_get_count(arr);\n"
      "  if (count == 0) {\n"
      "    *out = NULL;\n"
      "    *out_len = 0;\n"
      "    json_value_free(val);\n"
      "    return 0;\n"
      "  }\n"
      "  tmp = (struct %s **)calloc(count, sizeof(struct %s *));\n"
      "  if (!tmp) { json_value_free(val); return 12; /* ENOMEM */ }\n"
      "  for (i = 0; i < count; ++i) {\n"
      "    rc = %s_from_jsonObject(json_array_get_object(arr, i), &tmp[i]);\n"
      "    if (rc != 0) break;\n"
      "  }\n"
      "  if (rc == 0) {\n"
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
      struct_name, struct_name, struct_name, struct_name, struct_name,
      struct_name, struct_name));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n", config->guard_macro));

  return 0;
}

int write_struct_from_jsonObject_func(FILE *fp, const char *struct_name,
                                      const struct StructFields *sf,
                                      const struct CodegenJsonConfig *config) {
  size_t i;
  int iter_needed = 0;
  int rc_needed = 0;
  int tmp_needed = 0;
  int len_needed = 0;

  if (!fp || !struct_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = 1;
    if (strcmp(sf->fields[i].type, "object") == 0 ||
        strcmp(sf->fields[i].type, "enum") == 0 ||
        strcmp(sf->fields[i].type, "array") == 0) {
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

  CHECK_IO(FPRINTF_HOOK(fp,
                        "int %s_from_jsonObject(const JSON_Object *jsonObject, "
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
  CHECK_IO(FPRINTF_HOOK(fp, "  if (!ret) return ENOMEM;\n"));
  CHECK_IO(
      FPRINTF_HOOK(fp, "  if (!jsonObject || !out) { free(ret); return EINVAL; "
                       "}\n\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = sf->fields[i].ref;
    struct StructField *f = &sf->fields[i];

    if (f->read_only)
      continue;

    if (strcmp(t, "integer") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  ret->%s = (int)json_object_get_number(jsonObject, \"%s\");\n",
          n, n));
      if (f->has_min || f->has_max || f->exclusive_min || f->exclusive_max) {
        CHECK_IO(FPRINTF_HOOK(fp, "  tmp = (double)ret->%s;\n", n));
        if (f->has_min) {
          if (f->exclusive_min)
            CHECK_IO(FPRINTF_HOOK(
                fp, "  if (tmp <= %f) { free(ret); return ERANGE; }\n",
                f->min_val));
          else
            CHECK_IO(FPRINTF_HOOK(
                fp, "  if (tmp < %f) { free(ret); return ERANGE; }\n",
                f->min_val));
        }
        if (f->has_max) {
          if (f->exclusive_max)
            CHECK_IO(FPRINTF_HOOK(
                fp, "  if (tmp >= %f) { free(ret); return ERANGE; }\n",
                f->max_val));
          else
            CHECK_IO(FPRINTF_HOOK(
                fp, "  if (tmp > %f) { free(ret); return ERANGE; }\n",
                f->max_val));
        }
      }
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  ret->%s = json_object_get_number(jsonObject, \"%s\");\n", n,
          n));
      if (f->has_min || f->has_max || f->exclusive_min || f->exclusive_max) {
        CHECK_IO(FPRINTF_HOOK(fp, "  tmp = ret->%s;\n", n));
        if (f->has_min) {
          if (f->exclusive_min)
            CHECK_IO(FPRINTF_HOOK(
                fp, "  if (tmp <= %f) { free(ret); return ERANGE; }\n",
                f->min_val));
          else
            CHECK_IO(FPRINTF_HOOK(
                fp, "  if (tmp < %f) { free(ret); return ERANGE; }\n",
                f->min_val));
        }
        if (f->has_max) {
          if (f->exclusive_max)
            CHECK_IO(FPRINTF_HOOK(
                fp, "  if (tmp >= %f) { free(ret); return ERANGE; }\n",
                f->max_val));
          else
            CHECK_IO(FPRINTF_HOOK(
                fp, "  if (tmp > %f) { free(ret); return ERANGE; }\n",
                f->max_val));
        }
      }
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  ret->%s = json_object_get_boolean(jsonObject, \"%s\");\n", n,
          n));
    } else if (strcmp(t, "string") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { const char *s = json_object_get_string(jsonObject, \"%s\");\n",
          n));
      CHECK_IO(FPRINTF_HOOK(fp, "    if (s) {\n"));
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "      ret->%s = strdup(s);\n"
          "      if (!ret->%s) { %s_cleanup(ret); return ENOMEM; }\n",
          n, n, struct_name));
      if (f->has_min_len || f->has_max_len || f->pattern[0]) {
        CHECK_IO(FPRINTF_HOOK(fp, "      len = strlen(ret->%s);\n", n));
        if (f->has_min_len)
          CHECK_IO(FPRINTF_HOOK(fp,
                                "      if (len < %" CDD_SIZE_T_FMT
                                ") { %s_cleanup(ret); return ERANGE; }\n",
                                (size_t)f->min_len, struct_name));
        if (f->has_max_len)
          CHECK_IO(FPRINTF_HOOK(fp,
                                "      if (len > %" CDD_SIZE_T_FMT
                                ") { %s_cleanup(ret); return ERANGE; }\n",
                                (size_t)f->max_len, struct_name));
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
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern + 1, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern + 1, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern + 1, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern + 1, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern + 1, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern + 1, pl);
#else
            strncpy(pat, f->pattern + 1, pl);
#endif
#endif
#endif
#endif
#endif
#endif
#endif
            pat[pl] = 0;
            CHECK_IO(FPRINTF_HOOK(fp,
                                  "      if (strcmp(ret->%s, \"%s\") != 0) { "
                                  "%s_cleanup(ret); return ERANGE; }\n",
                                  n, pat, struct_name));
          } else if (strncmp(f->pattern, "^", 1) == 0) { /* prefix */
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "      if (strncmp(ret->%s, \"%s\", %" CDD_SIZE_T_FMT
                ") != 0) { "
                "%s_cleanup(ret); return ERANGE; }\n",
                n, f->pattern + 1, (size_t)strlen(f->pattern) - 1,
                struct_name));
          } else if (f->pattern[(size_t)strlen(f->pattern) - 1] ==
                     '$') { /* suffix */
            char pat[256];
            size_t pl = (size_t)strlen(f->pattern) - 1;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            strncpy_s(pat, sizeof(pat), f->pattern, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern, pl);
#else
#if defined(_MSC_VER)
            strncpy_s(pat, sizeof(pat), f->pattern, pl);
#else
            strncpy(pat, f->pattern, pl);
#endif
#endif
#endif
#endif
#endif
#endif
#endif
            pat[pl] = 0;
            CHECK_IO(FPRINTF_HOOK(
                fp,
                "      if (len < %" CDD_SIZE_T_FMT
                " || strcmp(ret->%s + len - %" CDD_SIZE_T_FMT ", "
                "\"%s\") != 0) { %s_cleanup(ret); return ERANGE; }\n",
                (size_t)pl, n, (size_t)pl, pat, struct_name));
          } else { /* contains */
            CHECK_IO(
                FPRINTF_HOOK(fp,
                             "      if (strstr(ret->%s, \"%s\") == NULL) { "
                             "%s_cleanup(ret); return ERANGE; }\n",
                             n, f->pattern, struct_name));
          }
        }
      }
      CHECK_IO(FPRINTF_HOOK(fp, "    }\n  }\n"));
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(FPRINTF_HOOK(fp,
                            "  { JSON_Object *sub = "
                            "json_object_get_object(jsonObject, \"%s\");\n",
                            n));
      CHECK_IO(FPRINTF_HOOK(fp, "    if (sub) {\n"));
      {
        char *tn = NULL;
        get_type_from_ref(r, &tn);
        CHECK_IO(FPRINTF_HOOK(
            fp, "      rc = %s_from_jsonObject(sub, &ret->%s);\n", tn, n));
      }
      CHECK_IO(FPRINTF_HOOK(
          fp, "      if (rc) { %s_cleanup(ret); return rc; }\n", struct_name));
      CHECK_IO(FPRINTF_HOOK(fp, "    }\n  }\n"));
    } else if (strcmp(t, "enum") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp,
          "  { const char *s = json_object_get_string(jsonObject, \"%s\");\n",
          n));
      CHECK_IO(FPRINTF_HOOK(fp, "    if (s) {\n"));
      {
        char *tn = NULL;
        get_type_from_ref(r, &tn);
        CHECK_IO(
            FPRINTF_HOOK(fp, "      rc = %s_from_str(s, &ret->%s);\n", tn, n));
      }
      CHECK_IO(FPRINTF_HOOK(
          fp, "      if (rc) { %s_cleanup(ret); return rc; }\n", struct_name));
      CHECK_IO(FPRINTF_HOOK(fp, "    }\n  }\n"));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(FPRINTF_HOOK(
          fp, "  arr = json_object_get_array(jsonObject, \"%s\");\n", n));
      CHECK_IO(FPRINTF_HOOK(fp, "  if (arr) {\n"));
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
            fp, "      ret->%s = calloc(ret->n_%s, sizeof(char*));\n", n, n));
        CHECK_IO(FPRINTF_HOOK(fp,
                              "      for(i=0; i<ret->n_%s; ++i) ret->%s[i] = "
                              "strdup(json_array_get_string(arr, i));\n",
                              n, n));
      } else {
        /* Object array */
        {
          char *tn = NULL;
          get_type_from_ref(r, &tn);
          CHECK_IO(FPRINTF_HOOK(
              fp, "      ret->%s = calloc(ret->n_%s, sizeof(struct %s*));\n", n,
              n, tn));
        }
        CHECK_IO(FPRINTF_HOOK(fp, "      for(i=0; i<ret->n_%s; ++i) {\n", n));
        {
          char *tn = NULL;
          get_type_from_ref(r, &tn);
          CHECK_IO(FPRINTF_HOOK(
              fp,
              "        rc = %s_from_jsonObject(json_array_get_object(arr, i), "
              "&ret->%s[i]);\n",
              tn, n));
        }
        CHECK_IO(
            FPRINTF_HOOK(fp, "        if(rc) { %s_cleanup(ret); return rc; }\n",
                         struct_name));
        CHECK_IO(FPRINTF_HOOK(fp, "      }\n"));
      }
      CHECK_IO(FPRINTF_HOOK(fp, "    }\n  }\n"));
    }
  }

  CHECK_IO(FPRINTF_HOOK(fp, "  *out = ret;\n  return 0;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(FPRINTF_HOOK(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}
