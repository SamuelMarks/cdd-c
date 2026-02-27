/**
 * @file codegen_json.c
 * @brief Implementation of JSON generation logic.
 *
 * Emits C code.
 * - Serialization: Manual string concatenation (using `jasprintf`).
 * - Deserialization: Uses `parson` library API (`json_object_get_...`).
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "classes/emit_json.h"
#include "classes/emit_struct.h" /* for get_type_from_ref */
#include "functions/parse_str.h" /* for string helpers */

/* Terser error checking */
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

int write_struct_to_json_func(FILE *const fp, const char *const struct_name,
                              const struct StructFields *const sf,
                              const struct CodegenJsonConfig *const config) {
  size_t i;
  bool iter_needed = false;
  bool rc_needed = false;

  if (!fp || !struct_name || !sf)
    return EINVAL;

  /* Pre-scan to determine variable needs */
  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = true;
    if (strcmp(sf->fields[i].type, "object") == 0 ||
        strcmp(sf->fields[i].type, "enum") == 0 ||
        strcmp(sf->fields[i].type, "array") == 0) {
      rc_needed = true;
    }
  }

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(fprintf(
      fp, "int %s_to_json(const struct %s *const obj, char **const json) {\n",
      struct_name, struct_name));

  /* Variables decl */
  CHECK_IO(fprintf(fp, "  int need_comma = 0;\n"));
  if (rc_needed)
    CHECK_IO(fprintf(fp, "  int rc;\n"));
  if (iter_needed)
    CHECK_IO(fprintf(fp, "  size_t i;\n"));

  /* Initial Safety Checks */
  CHECK_IO(fprintf(fp, "  if (obj == NULL || json == NULL) return EINVAL;\n"));
  CHECK_IO(fprintf(fp, "  jasprintf(json, \"{\");\n"));
  CHECK_IO(fprintf(fp, "  if (*json == NULL) return ENOMEM;\n\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = sf->fields[i].ref;

    CHECK_IO(fprintf(fp, "  if (need_comma) { jasprintf(json, \",\"); if "
                         "(*json==NULL) return ENOMEM; }\n"));

    if (strcmp(t, "integer") == 0) {
      CHECK_IO(fprintf(fp, "  jasprintf(json, \"\\\"%s\\\": %%d\", obj->%s);\n",
                       n, n));
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(fprintf(fp, "  jasprintf(json, \"\\\"%s\\\": %%f\", obj->%s);\n",
                       n, n));
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(fprintf(fp,
                       "  jasprintf(json, \"\\\"%s\\\": %%s\", obj->%s ? "
                       "\"true\" : \"false\");\n",
                       n, n));
    } else if (strcmp(t, "string") == 0) {
      CHECK_IO(fprintf(fp,
                       "  if (obj->%s) jasprintf(json, \"\\\"%s\\\": "
                       "\\\"%%s\\\"\", obj->%s);\n",
                       n, n, n));
      CHECK_IO(
          fprintf(fp, "  else jasprintf(json, \"\\\"%s\\\": null\");\n", n));
    } else if (strcmp(t, "object") == 0) {
      /* Recursive call */
      CHECK_IO(fprintf(fp, "  if (obj->%s) {\n", n));
      CHECK_IO(fprintf(fp, "    char *s = NULL;\n"));
      CHECK_IO(fprintf(fp, "    rc = %s_to_json(obj->%s, &s);\n",
                       get_type_from_ref(r), n));
      CHECK_IO(fprintf(fp, "    if (rc) return rc;\n"));
      CHECK_IO(
          fprintf(fp, "    jasprintf(json, \"\\\"%s\\\": %%s\", s);\n", n));
      CHECK_IO(fprintf(fp, "    free(s);\n"));
      CHECK_IO(
          fprintf(fp, "  } else jasprintf(json, \"\\\"%s\\\": null\");\n", n));
    } else if (strcmp(t, "enum") == 0) {
      CHECK_IO(fprintf(
          fp,
          "  { char *s=NULL; rc=%s_to_str(obj->%s, &s); if (rc) return rc;\n",
          get_type_from_ref(r), n));
      CHECK_IO(fprintf(
          fp,
          "    jasprintf(json, \"\\\"%s\\\": \\\"%%s\\\"\", s); free(s); }\n",
          n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(fprintf(fp, "  jasprintf(json, \"\\\"%s\\\": [\");\n", n));
      CHECK_IO(fprintf(fp, "  if (*json==NULL) return ENOMEM;\n"));
      CHECK_IO(fprintf(fp, "  for (i=0; i < obj->n_%s; ++i) {\n", n));
      /* Loop Body handling type */
      if (strcmp(r, "integer") == 0) {
        CHECK_IO(fprintf(fp, "    jasprintf(json, \"%%d\", obj->%s[i]);\n", n));
      } else if (strcmp(r, "string") == 0) {
        CHECK_IO(fprintf(
            fp, "    jasprintf(json, \"\\\"%%s\\\"\", obj->%s[i]);\n", n));
      } else { /* Object array */
        CHECK_IO(fprintf(fp,
                         "    { char *s=NULL; rc=%s_to_json(obj->%s[i], &s); "
                         "if (rc) return rc;\n",
                         get_type_from_ref(r), n));
        CHECK_IO(
            fprintf(fp, "      jasprintf(json, \"%%s\", s); free(s); }\n"));
      }
      CHECK_IO(fprintf(fp, "    if (*json==NULL) return ENOMEM;\n"));
      CHECK_IO(
          fprintf(fp, "    if (i+1 < obj->n_%s) jasprintf(json, \",\");\n", n));
      CHECK_IO(fprintf(fp, "  }\n"));
      CHECK_IO(fprintf(fp, "  jasprintf(json, \"]\");\n"));
    }

    CHECK_IO(fprintf(fp, "  if (*json == NULL) return ENOMEM;\n"));
    CHECK_IO(fprintf(fp, "  need_comma = 1;\n"));
  }

  CHECK_IO(fprintf(fp, "  jasprintf(json, \"}\");\n"));
  CHECK_IO(fprintf(fp, "  if (*json == NULL) return ENOMEM;\n"));
  CHECK_IO(fprintf(fp, "  return 0;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}

int write_struct_from_json_func(FILE *const fp, const char *const struct_name,
                                const struct CodegenJsonConfig *const config) {
  if (!fp || !struct_name)
    return EINVAL;

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(fprintf(
      fp,
      "int %s_from_json(const char *const json_str, struct %s **const out) {\n"
      "  JSON_Value *val = json_parse_string(json_str);\n"
      "  int rc = 0;\n"
      "  if (!val) return EINVAL;\n"
      "  rc = %s_from_jsonObject(json_value_get_object(val), out);\n"
      "  json_value_free(val);\n"
      "  return rc;\n"
      "}\n",
      struct_name, struct_name, struct_name));

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}

int write_struct_from_jsonObject_func(
    FILE *const fp, const char *const struct_name,
    const struct StructFields *const sf,
    const struct CodegenJsonConfig *const config) {
  size_t i;
  bool iter_needed = false;
  bool rc_needed = false;
  bool tmp_needed = false;
  bool len_needed = false;

  if (!fp || !struct_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter_needed = true;
    if (strcmp(sf->fields[i].type, "object") == 0 ||
        strcmp(sf->fields[i].type, "enum") == 0 ||
        strcmp(sf->fields[i].type, "array") == 0) {
      rc_needed = true;
    }
    if (sf->fields[i].has_min || sf->fields[i].has_max ||
        sf->fields[i].exclusive_min || sf->fields[i].exclusive_max) {
      tmp_needed = true;
    }
    if (sf->fields[i].has_min_len || sf->fields[i].has_max_len ||
        sf->fields[i].pattern[0] != '\0') {
      len_needed = true;
    }
  }

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#ifdef %s\n", config->guard_macro));

  CHECK_IO(
      fprintf(fp,
              "int %s_from_jsonObject(const JSON_Object *const jsonObject, "
              "struct %s **const out) {\n",
              struct_name, struct_name));

  if (rc_needed)
    CHECK_IO(fprintf(fp, "  int rc;\n"));
  if (iter_needed)
    CHECK_IO(fprintf(fp, "  size_t i;\n  const JSON_Array *arr;\n"));
  if (tmp_needed)
    CHECK_IO(fprintf(fp, "  double tmp;\n"));
  if (len_needed)
    CHECK_IO(fprintf(fp, "  size_t len;\n"));

  CHECK_IO(fprintf(fp, "  struct %s *ret = calloc(1, sizeof(*ret));\n",
                   struct_name));
  CHECK_IO(fprintf(fp, "  if (!ret) return ENOMEM;\n"));
  CHECK_IO(fprintf(fp, "  if (!jsonObject || !out) { free(ret); return EINVAL; "
                       "}\n\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = sf->fields[i].ref;
    struct StructField *f = &sf->fields[i];

    if (strcmp(t, "integer") == 0) {
      CHECK_IO(fprintf(
          fp, "  ret->%s = (int)json_object_get_number(jsonObject, \"%s\");\n",
          n, n));
      if (f->has_min || f->has_max || f->exclusive_min || f->exclusive_max) {
        CHECK_IO(fprintf(fp, "  tmp = (double)ret->%s;\n", n));
        if (f->has_min) {
          if (f->exclusive_min)
            CHECK_IO(fprintf(fp,
                             "  if (tmp <= %f) { free(ret); return ERANGE; }\n",
                             f->min_val));
          else
            CHECK_IO(fprintf(fp,
                             "  if (tmp < %f) { free(ret); return ERANGE; }\n",
                             f->min_val));
        }
        if (f->has_max) {
          if (f->exclusive_max)
            CHECK_IO(fprintf(fp,
                             "  if (tmp >= %f) { free(ret); return ERANGE; }\n",
                             f->max_val));
          else
            CHECK_IO(fprintf(fp,
                             "  if (tmp > %f) { free(ret); return ERANGE; }\n",
                             f->max_val));
        }
      }
    } else if (strcmp(t, "number") == 0) {
      CHECK_IO(fprintf(
          fp, "  ret->%s = json_object_get_number(jsonObject, \"%s\");\n", n,
          n));
      if (f->has_min || f->has_max || f->exclusive_min || f->exclusive_max) {
        CHECK_IO(fprintf(fp, "  tmp = ret->%s;\n", n));
        if (f->has_min) {
          if (f->exclusive_min)
            CHECK_IO(fprintf(fp,
                             "  if (tmp <= %f) { free(ret); return ERANGE; }\n",
                             f->min_val));
          else
            CHECK_IO(fprintf(fp,
                             "  if (tmp < %f) { free(ret); return ERANGE; }\n",
                             f->min_val));
        }
        if (f->has_max) {
          if (f->exclusive_max)
            CHECK_IO(fprintf(fp,
                             "  if (tmp >= %f) { free(ret); return ERANGE; }\n",
                             f->max_val));
          else
            CHECK_IO(fprintf(fp,
                             "  if (tmp > %f) { free(ret); return ERANGE; }\n",
                             f->max_val));
        }
      }
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_IO(fprintf(
          fp, "  ret->%s = json_object_get_boolean(jsonObject, \"%s\");\n", n,
          n));
    } else if (strcmp(t, "string") == 0) {
      CHECK_IO(fprintf(
          fp,
          "  { const char *s = json_object_get_string(jsonObject, \"%s\");\n",
          n));
      CHECK_IO(fprintf(fp, "    if (s) {\n"));
      CHECK_IO(
          fprintf(fp,
                  "      ret->%s = strdup(s);\n"
                  "      if (!ret->%s) { %s_cleanup(ret); return ENOMEM; }\n",
                  n, n, struct_name));
      if (f->has_min_len || f->has_max_len || f->pattern[0]) {
        CHECK_IO(fprintf(fp, "      len = strlen(ret->%s);\n", n));
        if (f->has_min_len)
          CHECK_IO(fprintf(
              fp, "      if (len < %zu) { %s_cleanup(ret); return ERANGE; }\n",
              f->min_len, struct_name));
        if (f->has_max_len)
          CHECK_IO(fprintf(
              fp, "      if (len > %zu) { %s_cleanup(ret); return ERANGE; }\n",
              f->max_len, struct_name));
        if (f->pattern[0]) {
          if (strncmp(f->pattern, "^", 1) == 0 &&
              f->pattern[strlen(f->pattern) - 1] == '$') { /* exact mismatch */
            /* strip anchors */
            char pat[256];
            size_t pl = strlen(f->pattern) - 2;
            strncpy(pat, f->pattern + 1, pl);
            pat[pl] = 0;
            CHECK_IO(fprintf(fp,
                             "      if (strcmp(ret->%s, \"%s\") != 0) { "
                             "%s_cleanup(ret); return ERANGE; }\n",
                             n, pat, struct_name));
          } else if (strncmp(f->pattern, "^", 1) == 0) { /* prefix */
            CHECK_IO(fprintf(fp,
                             "      if (strncmp(ret->%s, \"%s\", %zu) != 0) { "
                             "%s_cleanup(ret); return ERANGE; }\n",
                             n, f->pattern + 1, strlen(f->pattern) - 1,
                             struct_name));
          } else if (f->pattern[strlen(f->pattern) - 1] == '$') { /* suffix */
            char pat[256];
            size_t pl = strlen(f->pattern) - 1;
            strncpy(pat, f->pattern, pl);
            pat[pl] = 0;
            CHECK_IO(
                fprintf(fp,
                        "      if (len < %zu || strcmp(ret->%s + len - %zu, "
                        "\"%s\") != 0) { %s_cleanup(ret); return ERANGE; }\n",
                        pl, n, pl, pat, struct_name));
          } else { /* contains */
            CHECK_IO(fprintf(fp,
                             "      if (strstr(ret->%s, \"%s\") == NULL) { "
                             "%s_cleanup(ret); return ERANGE; }\n",
                             n, f->pattern, struct_name));
          }
        }
      }
      CHECK_IO(fprintf(fp, "    }\n  }\n"));
    } else if (strcmp(t, "object") == 0) {
      CHECK_IO(fprintf(fp,
                       "  { JSON_Object *sub = "
                       "json_object_get_object(jsonObject, \"%s\");\n",
                       n));
      CHECK_IO(fprintf(fp, "    if (sub) {\n"));
      CHECK_IO(fprintf(fp, "      rc = %s_from_jsonObject(sub, &ret->%s);\n",
                       get_type_from_ref(r), n));
      CHECK_IO(fprintf(fp, "      if (rc) { %s_cleanup(ret); return rc; }\n",
                       struct_name));
      CHECK_IO(fprintf(fp, "    }\n  }\n"));
    } else if (strcmp(t, "enum") == 0) {
      CHECK_IO(fprintf(
          fp,
          "  { const char *s = json_object_get_string(jsonObject, \"%s\");\n",
          n));
      CHECK_IO(fprintf(fp, "    if (s) {\n"));
      CHECK_IO(fprintf(fp, "      rc = %s_from_str(s, &ret->%s);\n",
                       get_type_from_ref(r), n));
      CHECK_IO(fprintf(fp, "      if (rc) { %s_cleanup(ret); return rc; }\n",
                       struct_name));
      CHECK_IO(fprintf(fp, "    }\n  }\n"));
    } else if (strcmp(t, "array") == 0) {
      CHECK_IO(fprintf(
          fp, "  arr = json_object_get_array(jsonObject, \"%s\");\n", n));
      CHECK_IO(fprintf(fp, "  if (arr) {\n"));
      CHECK_IO(fprintf(fp, "    ret->n_%s = json_array_get_count(arr);\n", n));
      CHECK_IO(fprintf(fp, "    if (ret->n_%s > 0) {\n", n));
      if (strcmp(r, "integer") == 0) {
        CHECK_IO(fprintf(
            fp, "      ret->%s = malloc(ret->n_%s * sizeof(int));\n", n, n));
        CHECK_IO(fprintf(fp,
                         "      for(i=0; i<ret->n_%s; ++i) ret->%s[i] = "
                         "(int)json_array_get_number(arr, i);\n",
                         n, n));
      } else if (strcmp(r, "string") == 0) {
        CHECK_IO(fprintf(
            fp, "      ret->%s = calloc(ret->n_%s, sizeof(char*));\n", n, n));
        CHECK_IO(fprintf(fp,
                         "      for(i=0; i<ret->n_%s; ++i) ret->%s[i] = "
                         "strdup(json_array_get_string(arr, i));\n",
                         n, n));
      } else {
        /* Object array */
        CHECK_IO(fprintf(
            fp, "      ret->%s = calloc(ret->n_%s, sizeof(struct %s*));\n", n,
            n, get_type_from_ref(r)));
        CHECK_IO(fprintf(fp, "      for(i=0; i<ret->n_%s; ++i) {\n", n));
        CHECK_IO(fprintf(
            fp,
            "        rc = %s_from_jsonObject(json_array_get_object(arr, i), "
            "&ret->%s[i]);\n",
            get_type_from_ref(r), n));
        CHECK_IO(fprintf(fp, "        if(rc) { %s_cleanup(ret); return rc; }\n",
                         struct_name));
        CHECK_IO(fprintf(fp, "      }\n"));
      }
      CHECK_IO(fprintf(fp, "    }\n  }\n"));
    }
  }

  CHECK_IO(fprintf(fp, "  *out = ret;\n  return 0;\n}\n"));

  if (config && config->guard_macro)
    CHECK_IO(fprintf(fp, "#endif /* %s */\n\n", config->guard_macro));

  return 0;
}
