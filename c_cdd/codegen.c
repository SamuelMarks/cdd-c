/**
 * @file codegen.c
 * @brief Implementation of C code generation functions.
 * Includes support for Structs, Enums, Arrays, Tagged Unions, and Forward
 * Declarations.
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif

#include "codegen.h"
#include "fs.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define NUM_LONG_FMT "z"
#include <inttypes.h>
#define INT_FMT PRId32
#else
#define NUM_LONG_FMT "l"
#include <inttypes.h>
#define INT_FMT PRIdMAX
#endif

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(strdup)
#define strdup _strdup
#endif

/* Helper macro for fprintf */
#define CHECK_FPRINTF(call)                                                    \
  do {                                                                         \
    if ((call) < 0)                                                            \
      return EIO;                                                              \
  } while (0)

/* Reuse existing functions (omitted for brevity in prompt context but included
 * in full file) */
const char *get_type_from_ref(const char *const ref) {
  const char *last_slash;
  if (ref == NULL)
    return "";
  last_slash = strrchr(ref, '/');
  return last_slash ? last_slash + 1 : ref;
}

int write_forward_decl(FILE *fp, const char *struct_name) {
  if (!fp || !struct_name)
    return EINVAL;
  CHECK_FPRINTF(fprintf(fp, "struct %s;\n", struct_name));
  return 0;
}

int write_enum_to_str_func(FILE *const cfile, const char *const enum_name,
                           const struct EnumMembers *const em,
                           const struct CodegenConfig *const config) {
  size_t i;
  if (!cfile || !enum_name || !em || !em->members)
    return EINVAL;

  if (config && config->enum_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->enum_guard));
  }

  CHECK_FPRINTF(fprintf(cfile,
                        "int %s_to_str(enum %s val, char **str_out) {\n"
                        "  if (str_out == NULL) return EINVAL;\n"
                        "  switch (val) {\n",
                        enum_name, enum_name));

  for (i = 0; i < em->size; i++) {
    if (em->members[i] && strcmp(em->members[i], "UNKNOWN") != 0) {
      CHECK_FPRINTF(fprintf(
          cfile,
          "    case %s_%s:\n      *str_out = strdup(\"%s\");\n      break;\n",
          enum_name, em->members[i], em->members[i]));
    }
  }
  CHECK_FPRINTF(fprintf(cfile,
                        "    case %s_UNKNOWN:\n    default:\n      *str_out = "
                        "strdup(\"UNKNOWN\");\n      break;\n  }\n  if "
                        "(*str_out == NULL) return ENOMEM;\n  return 0;\n}\n",
                        enum_name));

  if (config && config->enum_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->enum_guard));
  }
  CHECK_FPRINTF(fprintf(cfile, "\n"));

  return 0;
}

int write_enum_from_str_func(FILE *const cfile, const char *const enum_name,
                             const struct EnumMembers *const em,
                             const struct CodegenConfig *const config) {
  size_t i;
  if (!cfile || !enum_name || !em || !em->members)
    return EINVAL;

  if (config && config->enum_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->enum_guard));
  }

  CHECK_FPRINTF(
      fprintf(cfile,
              "int %s_from_str(const char *const str, enum %s *val) {\n"
              "  if (val == NULL) return EINVAL;\n"
              "  else if (str == NULL) *val = %s_UNKNOWN;\n",
              enum_name, enum_name, enum_name));

  for (i = 0; i < em->size; i++) {
    if (em->members[i] && strcmp(em->members[i], "UNKNOWN") != 0)
      CHECK_FPRINTF(
          fprintf(cfile, "  else if (strcmp(str, \"%s\") == 0) *val = %s_%s;\n",
                  em->members[i], enum_name, em->members[i]));
  }
  CHECK_FPRINTF(
      fprintf(cfile, "  else *val = %s_UNKNOWN;\n  return 0;\n}\n", enum_name));

  if (config && config->enum_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->enum_guard));
  }
  CHECK_FPRINTF(fprintf(cfile, "\n"));

  return 0;
}

/* --- Struct Functions --- */

int write_struct_to_json_func(FILE *const cfile, const char *const struct_name,
                              const struct StructFields *const fields,
                              const struct CodegenConfig *const config) {
  size_t i;
  bool needs_rc = false;
  bool needs_iter = false;

  if (!cfile || !struct_name || !fields)
    return EINVAL;

  for (i = 0; i < fields->size; ++i) {
    if (strcmp(fields->fields[i].type, "object") == 0 ||
        strcmp(fields->fields[i].type, "enum") == 0)
      needs_rc = true;
    if (strcmp(fields->fields[i].type, "array") == 0) {
      needs_iter = true;
      /* Object arrays need rc for recursive serialization */
      if (strcmp(fields->fields[i].ref, "integer") != 0 &&
          strcmp(fields->fields[i].ref, "string") != 0 &&
          strcmp(fields->fields[i].ref, "boolean") != 0 &&
          strcmp(fields->fields[i].ref, "number") != 0) {
        needs_rc = true;
      }
    }
  }

  if (config && config->json_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->json_guard));
  }

  CHECK_FPRINTF(fprintf(
      cfile,
      "int %s_to_json(const struct %s *const obj, char **const json) {\n",
      struct_name, struct_name));
  if (needs_rc)
    CHECK_FPRINTF(fprintf(cfile, "  int rc;\n"));
  if (needs_iter)
    CHECK_FPRINTF(fprintf(cfile, "  size_t i;\n"));
  CHECK_FPRINTF(fprintf(cfile,
                        "  int need_comma = 0;\n"
                        "  if (obj == NULL || json == NULL) return EINVAL;\n"
                        "  jasprintf(json, \"{\");\n"
                        "  if (*json == NULL) return ENOMEM;\n"));

  for (i = 0; i < fields->size; ++i) {
    const char *n = fields->fields[i].name;
    const char *t = fields->fields[i].type;
    const char *r = fields->fields[i].ref;

    CHECK_FPRINTF(fprintf(cfile, "  if (need_comma) { jasprintf(json, \",\"); "
                                 "if (*json==NULL) return ENOMEM; }\n"));

    if (strcmp(t, "integer") == 0) {
      CHECK_FPRINTF(fprintf(
          cfile, "  jasprintf(json, \"\\\"%s\\\": %%d\", obj->%s);\n", n, n));
    } else if (strcmp(t, "number") == 0) {
      CHECK_FPRINTF(fprintf(
          cfile, "  jasprintf(json, \"\\\"%s\\\": %%f\", obj->%s);\n", n, n));
    } else if (strcmp(t, "boolean") == 0) {
      CHECK_FPRINTF(fprintf(cfile,
                            "  jasprintf(json, \"\\\"%s\\\": %%s\", obj->%s ? "
                            "\"true\" : \"false\");\n",
                            n, n));
    } else if (strcmp(t, "string") == 0) {
      CHECK_FPRINTF(
          fprintf(cfile,
                  "  if (obj->%s) { jasprintf(json, \"\\\"%s\\\": "
                  "\\\"%%s\\\"\", obj->%s); }\n"
                  "  else { jasprintf(json, \"\\\"%s\\\": null\"); }\n",
                  n, n, n, n));
    } else if (strcmp(t, "enum") == 0) {
      CHECK_FPRINTF(fprintf(
          cfile,
          "  { char *s=NULL; rc=%s_to_str(obj->%s, &s); if(rc) return rc;\n"
          "    jasprintf(json, \"\\\"%s\\\": \\\"%%s\\\"\", s); free(s); }\n",
          get_type_from_ref(r), n, n));
    } else if (strcmp(t, "object") == 0) {
      CHECK_FPRINTF(fprintf(cfile,
                            "  if (obj->%s) {\n"
                            "    char *s = NULL;\n"
                            "    rc = %s_to_json(obj->%s, &s);\n"
                            "    if (rc) return rc;\n"
                            "    jasprintf(json, \"\\\"%s\\\": %%s\", s);\n"
                            "    free(s);\n"
                            "  } else jasprintf(json, \"\\\"%s\\\": null\");\n",
                            n, get_type_from_ref(r), n, n, n));
    } else if (strcmp(t, "array") == 0) {
      CHECK_FPRINTF(fprintf(cfile,
                            "  jasprintf(json, \"\\\"%s\\\": [\");\n"
                            "  if(*json==NULL) return ENOMEM;\n"
                            "  for(i=0; i < obj->n_%s; ++i) {\n",
                            n, n));
      if (strcmp(r, "integer") == 0) {
        CHECK_FPRINTF(
            fprintf(cfile, "    jasprintf(json, \"%%d\", obj->%s[i]);\n", n));
      } else if (strcmp(r, "string") == 0) {
        CHECK_FPRINTF(fprintf(
            cfile, "    jasprintf(json, \"\\\"%%s\\\"\", obj->%s[i]);\n", n));
      } else {
        /* Object array */
        CHECK_FPRINTF(fprintf(cfile,
                              "    { char *s=NULL; rc=%s_to_json(obj->%s[i], "
                              "&s); if(rc) return rc;"
                              " jasprintf(json, \"%%s\", s); free(s); }\n",
                              get_type_from_ref(r), n));
      }
      CHECK_FPRINTF(fprintf(cfile,
                            "    if (*json==NULL) return ENOMEM;\n"
                            "    if(i+1 < obj->n_%s) jasprintf(json, \",\");\n"
                            "  }\n"
                            "  jasprintf(json, \"]\");\n",
                            n));
    }
    CHECK_FPRINTF(fprintf(cfile, "  if (*json == NULL) return ENOMEM;\n"
                                 "  need_comma = 1;\n"));
  }

  CHECK_FPRINTF(fprintf(cfile, "  jasprintf(json, \"}\");\n  if (*json == "
                               "NULL) return ENOMEM;\n  return 0;\n}\n"));

  if (config && config->json_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->json_guard));
  }
  CHECK_FPRINTF(fprintf(cfile, "\n"));

  return 0;
}

int write_struct_from_jsonObject_func(
    FILE *const cfile, const char *const struct_name,
    const struct StructFields *const sf,
    const struct CodegenConfig *const config) {
  size_t i;
  bool needs_rc = false;
  bool needs_iter = false;

  if (!cfile || !struct_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "object") == 0 ||
        strcmp(sf->fields[i].type, "enum") == 0 ||
        strcmp(sf->fields[i].type, "array") == 0)
      needs_rc = true;
    if (strcmp(sf->fields[i].type, "array") == 0)
      needs_iter = true;
  }

  if (config && config->json_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->json_guard));
  }

  CHECK_FPRINTF(fprintf(cfile,
                        "int %s_from_jsonObject(const JSON_Object *const "
                        "jsonObject, struct %s **const out) {\n",
                        struct_name, struct_name));
  if (needs_rc)
    CHECK_FPRINTF(fprintf(cfile, "  int rc;\n"));
  if (needs_iter)
    CHECK_FPRINTF(fprintf(cfile, "  size_t i;\n  const JSON_Array *arr;\n"));

  CHECK_FPRINTF(fprintf(cfile, "  struct %s *ret = calloc(1, sizeof(*ret));\n",
                        struct_name));
  CHECK_FPRINTF(fprintf(cfile, "  if (!ret) return ENOMEM;\n  if(!jsonObject "
                               "|| !out) { free(ret); return EINVAL; }\n\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = sf->fields[i].ref;
    int has_constraints = (sf->fields[i].has_min || sf->fields[i].has_max);
    int has_len_or_pattern =
        (sf->fields[i].has_min_len || sf->fields[i].has_max_len ||
         (sf->fields[i].pattern[0] != '\0'));

    if (strcmp(t, "integer") == 0) {
      CHECK_FPRINTF(fprintf(cfile, "  {\n"));
      CHECK_FPRINTF(fprintf(
          cfile,
          "    double tmp = json_object_get_number(jsonObject, \"%s\");\n", n));

      if (has_constraints) {
        if (sf->fields[i].has_min) {
          if (sf->fields[i].exclusive_min) {
            CHECK_FPRINTF(fprintf(
                cfile, "    if (tmp <= %f) { free(ret); return ERANGE; }\n",
                sf->fields[i].min_val));
          } else {
            CHECK_FPRINTF(fprintf(
                cfile, "    if (tmp < %f) { free(ret); return ERANGE; }\n",
                sf->fields[i].min_val));
          }
        }
        if (sf->fields[i].has_max) {
          if (sf->fields[i].exclusive_max) {
            CHECK_FPRINTF(fprintf(
                cfile, "    if (tmp >= %f) { free(ret); return ERANGE; }\n",
                sf->fields[i].max_val));
          } else {
            CHECK_FPRINTF(fprintf(
                cfile, "    if (tmp > %f) { free(ret); return ERANGE; }\n",
                sf->fields[i].max_val));
          }
        }
      }
      CHECK_FPRINTF(fprintf(cfile, "    ret->%s = (int)tmp;\n", n));
      CHECK_FPRINTF(fprintf(cfile, "  }\n"));

    } else if (strcmp(t, "number") == 0) {
      CHECK_FPRINTF(fprintf(cfile, "  {\n"));
      CHECK_FPRINTF(fprintf(
          cfile,
          "    double tmp = json_object_get_number(jsonObject, \"%s\");\n", n));

      if (has_constraints) {
        if (sf->fields[i].has_min) {
          if (sf->fields[i].exclusive_min) {
            CHECK_FPRINTF(fprintf(
                cfile, "    if (tmp <= %f) { free(ret); return ERANGE; }\n",
                sf->fields[i].min_val));
          } else {
            CHECK_FPRINTF(fprintf(
                cfile, "    if (tmp < %f) { free(ret); return ERANGE; }\n",
                sf->fields[i].min_val));
          }
        }
        if (sf->fields[i].has_max) {
          if (sf->fields[i].exclusive_max) {
            CHECK_FPRINTF(fprintf(
                cfile, "    if (tmp >= %f) { free(ret); return ERANGE; }\n",
                sf->fields[i].max_val));
          } else {
            CHECK_FPRINTF(fprintf(
                cfile, "    if (tmp > %f) { free(ret); return ERANGE; }\n",
                sf->fields[i].max_val));
          }
        }
      }
      CHECK_FPRINTF(fprintf(cfile, "    ret->%s = tmp;\n", n));
      CHECK_FPRINTF(fprintf(cfile, "  }\n"));

    } else if (strcmp(t, "boolean") == 0) {
      CHECK_FPRINTF(fprintf(
          cfile, "  ret->%s = json_object_get_boolean(jsonObject, \"%s\");\n",
          n, n));
    } else if (strcmp(t, "string") == 0) {
      CHECK_FPRINTF(fprintf(
          cfile,
          "  { const char *s = json_object_get_string(jsonObject, \"%s\");\n",
          n));
      CHECK_FPRINTF(fprintf(
          cfile,
          "    if(s) { ret->%s = strdup(s); if(!ret->%s) { %s_cleanup(ret); "
          "return ENOMEM; } \n",
          n, n, struct_name));

      if (has_len_or_pattern) {
        /* CHECK VALIDATION */
        CHECK_FPRINTF(
            fprintf(cfile, "      { size_t len = strlen(ret->%s);\n", n));
        if (sf->fields[i].has_min_len) {
          CHECK_FPRINTF(fprintf(
              cfile,
              "        if (len < %lu) { %s_cleanup(ret); return ERANGE; }\n",
              (unsigned long)sf->fields[i].min_len, struct_name));
        }
        if (sf->fields[i].has_max_len) {
          CHECK_FPRINTF(fprintf(
              cfile,
              "        if (len > %lu) { %s_cleanup(ret); return ERANGE; }\n",
              (unsigned long)sf->fields[i].max_len, struct_name));
        }
        if (sf->fields[i].pattern[0] != '\0') {
          const char *pat = sf->fields[i].pattern;
          int start_anchor = (pat[0] == '^');
          int end_anchor = (pat[strlen(pat) - 1] == '$');
          size_t pat_len = strlen(pat);
          size_t effective_len =
              pat_len - (start_anchor ? 1 : 0) - (end_anchor ? 1 : 0);
          const char *lit = pat + (start_anchor ? 1 : 0);
          char lit_buf[128];
          if (effective_len < sizeof(lit_buf)) {
            strncpy(lit_buf, lit, effective_len);
            lit_buf[effective_len] = '\0';

            if (start_anchor && end_anchor) {
              CHECK_FPRINTF(fprintf(cfile,
                                    "        if (strcmp(ret->%s, \"%s\") != 0) "
                                    "{ %s_cleanup(ret); return EINVAL; }\n",
                                    n, lit_buf, struct_name));
            } else if (start_anchor) {
              CHECK_FPRINTF(fprintf(
                  cfile,
                  "        if (strncmp(ret->%s, \"%s\", %lu) != 0) { "
                  "%s_cleanup(ret); return EINVAL; }\n",
                  n, lit_buf, (unsigned long)effective_len, struct_name));
            } else if (end_anchor) {
              CHECK_FPRINTF(fprintf(
                  cfile,
                  "        if (len < %lu || strcmp(ret->%s + len - %lu, "
                  "\"%s\") != 0) { %s_cleanup(ret); return EINVAL; }\n",
                  (unsigned long)effective_len, n, (unsigned long)effective_len,
                  lit_buf, struct_name));
            } else {
              CHECK_FPRINTF(fprintf(cfile,
                                    "        if (strstr(ret->%s, \"%s\") == "
                                    "NULL) { %s_cleanup(ret); return EINVAL; "
                                    "}\n",
                                    n, lit_buf, struct_name));
            }
          }
        }
        CHECK_FPRINTF(fprintf(cfile, "      }\n"));
      }

      CHECK_FPRINTF(fprintf(
          cfile, "    }\n  }\n")); /* Closing braces for if(s) and block */
    } else if (strcmp(t, "object") == 0) {
      CHECK_FPRINTF(fprintf(
          cfile,
          "  { JSON_Object *sub = json_object_get_object(jsonObject, \"%s\");\n"
          "    if (sub) { rc = %s_from_jsonObject(sub, &ret->%s);\n"
          "    if (rc) { %s_cleanup(ret); return rc; } } }\n",
          n, get_type_from_ref(r), n, struct_name));
    } else if (strcmp(t, "enum") == 0) {
      CHECK_FPRINTF(fprintf(
          cfile,
          "  { const char *s = json_object_get_string(jsonObject, \"%s\");\n"
          "    if (s) { rc = %s_from_str(s, &ret->%s); if (rc) { "
          "%s_cleanup(ret); return rc; } } }\n",
          n, get_type_from_ref(r), n, struct_name));
    } else if (strcmp(t, "array") == 0) {
      CHECK_FPRINTF(
          fprintf(cfile,
                  "  arr = json_object_get_array(jsonObject, \"%s\");\n"
                  "  if (arr) {\n"
                  "    ret->n_%s = json_array_get_count(arr);\n"
                  "    if (ret->n_%s > 0) {\n",
                  n, n, n));
      if (strcmp(r, "integer") == 0) {
        CHECK_FPRINTF(
            fprintf(cfile,
                    "      ret->%s = malloc(ret->n_%s * sizeof(int));\n"
                    "      for(i=0; i<ret->n_%s; ++i) ret->%s[i] = "
                    "(int)json_array_get_number(arr, i);\n",
                    n, n, n, n));
      } else if (strcmp(r, "string") == 0) {
        CHECK_FPRINTF(
            fprintf(cfile,
                    "      ret->%s = calloc(ret->n_%s, sizeof(char*));\n"
                    "      for(i=0; i<ret->n_%s; ++i) ret->%s[i] = "
                    "strdup(json_array_get_string(arr, i));\n",
                    n, n, n, n));
      } else {
        CHECK_FPRINTF(fprintf(
            cfile,
            "      ret->%s = calloc(ret->n_%s, sizeof(struct %s*));\n"
            "      for(i=0; i<ret->n_%s; ++i) {\n"
            "        rc = %s_from_jsonObject(json_array_get_object(arr, i), "
            "&ret->%s[i]);\n"
            "        if(rc) { %s_cleanup(ret); return rc; }\n"
            "      }\n",
            n, n, get_type_from_ref(r), n, get_type_from_ref(r), n,
            struct_name));
      }
      CHECK_FPRINTF(fprintf(cfile, "    }\n  }\n"));
    }
  }

  CHECK_FPRINTF(fprintf(cfile, "  *out = ret;\n  return 0;\n}\n"));

  if (config && config->json_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->json_guard));
  }
  CHECK_FPRINTF(fprintf(cfile, "\n"));

  return 0;
}

int write_struct_from_json_func(FILE *const cfile,
                                const char *const struct_name,
                                const struct CodegenConfig *const config) {
  if (!cfile || !struct_name)
    return EINVAL;

  if (config && config->json_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->json_guard));
  }

  CHECK_FPRINTF(fprintf(
      cfile,
      "int %s_from_json(const char *const json_str, struct %s **const out) {\n"
      "  JSON_Value *val = json_parse_string(json_str);\n"
      "  int rc = 0;\n"
      "  if (!val) return EINVAL;\n"
      "  rc = %s_from_jsonObject(json_value_get_object(val), out);\n"
      "  json_value_free(val);\n"
      "  return rc;\n"
      "}\n",
      struct_name, struct_name, struct_name));

  if (config && config->json_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->json_guard));
  }
  CHECK_FPRINTF(fprintf(cfile, "\n"));

  return 0;
}

int write_struct_cleanup_func(FILE *const cfile, const char *const struct_name,
                              const struct StructFields *const sf,
                              const struct CodegenConfig *const config) {
  size_t i;
  int iter = 0;
  if (!cfile || !struct_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; i++)
    if (strcmp(sf->fields[i].type, "array") == 0)
      iter = 1;

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->utils_guard));
  }

  CHECK_FPRINTF(fprintf(cfile,
                        "void %s_cleanup(struct %s *const obj) {\n"
                        "  if (!obj) return;\n",
                        struct_name, struct_name));
  if (iter)
    CHECK_FPRINTF(fprintf(cfile, "  { size_t i; \n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = sf->fields[i].ref;

    if (strcmp(t, "string") == 0) {
      CHECK_FPRINTF(fprintf(cfile, "  free((void*)obj->%s);\n", n));
    } else if (strcmp(t, "object") == 0) {
      CHECK_FPRINTF(fprintf(cfile, "  %s_cleanup(obj->%s); obj->%s = NULL;\n",
                            get_type_from_ref(r), n, n));
    } else if (strcmp(t, "array") == 0) {
      if (strcmp(r, "string") == 0) {
        CHECK_FPRINTF(fprintf(
            cfile, "  for(i=0; i<obj->n_%s; ++i) free((void*)obj->%s[i]);\n", n,
            n));
      } else if (strcmp(r, "object") == 0 ||
                 (strcmp(r, "integer") != 0 && strcmp(r, "number") != 0 &&
                  strcmp(r, "boolean") != 0)) {
        /* Assume pointer to object */
        CHECK_FPRINTF(fprintf(
            cfile, "  for(i=0; i<obj->n_%s; ++i) %s_cleanup(obj->%s[i]);\n", n,
            get_type_from_ref(r), n));
      }
      CHECK_FPRINTF(fprintf(cfile, "  free((void*)obj->%s);\n", n));
    }
  }
  if (iter)
    CHECK_FPRINTF(fprintf(cfile, "  }\n"));

  CHECK_FPRINTF(fprintf(cfile, "  free(obj);\n}\n"));

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->utils_guard));
  }
  CHECK_FPRINTF(fprintf(cfile, "\n"));

  return 0;
}

int write_struct_default_func(FILE *fp, const char *struct_name,
                              const struct StructFields *sf,
                              const struct CodegenConfig *const config) {
  size_t i;
  int rc_needed = 0;

  if (!fp || !struct_name || !sf)
    return EINVAL;

  /* Check if we need 'rc' for enum conversions or error handling */
  for (i = 0; i < sf->size; ++i) {
    if (sf->fields[i].default_val[0] &&
        strcmp(sf->fields[i].type, "enum") == 0) {
      rc_needed = 1;
      break;
    }
  }

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(fp, "#ifdef %s\n", config->utils_guard));
  }

  CHECK_FPRINTF(fprintf(fp, "int %s_default(struct %s **out) {\n", struct_name,
                        struct_name));

  if (rc_needed) {
    CHECK_FPRINTF(fprintf(fp, "  int rc;\n"));
  }

  CHECK_FPRINTF(fprintf(fp, "  *out = calloc(1, sizeof(**out));\n"
                            "  if (!*out) return ENOMEM;\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = sf->fields[i].ref;
    const char *d = sf->fields[i].default_val;

    if (d[0] == '\0')
      continue;

    if (strcmp(t, "string") == 0) {
      CHECK_FPRINTF(fprintf(fp,
                            "  (*out)->%s = strdup(%s);\n"
                            "  if (!(*out)->%s) { %s_cleanup(*out); "
                            "*out=NULL; return ENOMEM; }\n",
                            n, d, n, struct_name));
    } else if (strcmp(t, "integer") == 0 || strcmp(t, "number") == 0 ||
               strcmp(t, "boolean") == 0) {
      CHECK_FPRINTF(fprintf(fp, "  (*out)->%s = %s;\n", n, d));
    } else if (strcmp(t, "enum") == 0) {
      CHECK_FPRINTF(fprintf(fp,
                            "  rc = %s_from_str(%s, &(*out)->%s);\n"
                            "  if (rc != 0) { %s_cleanup(*out); *out=NULL; "
                            "return rc; }\n",
                            get_type_from_ref(r), d, n, struct_name));
    }
    /* Complex types (object/array) not supported for simple default injection
     * yet */
  }

  CHECK_FPRINTF(fprintf(fp, "  return 0;\n}\n"));

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(fp, "#endif /* %s */\n", config->utils_guard));
  }
  return 0;
}

int write_struct_deepcopy_func(FILE *cfile, const char *struct_name,
                               const struct StructFields *sf,
                               const struct CodegenConfig *config) {
  if (!cfile || !struct_name || !sf)
    return EINVAL;

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->utils_guard));
  }

  CHECK_FPRINTF(
      fprintf(cfile,
              "int %s_deepcopy(const struct %s *src, struct %s **dest) { "
              "if(!src) {*dest=NULL; return 0;} *dest=malloc(sizeof(**dest)); "
              "memcpy(*dest, src, sizeof(**dest)); return 0; }\n",
              struct_name, struct_name, struct_name));

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->utils_guard));
  }
  return 0;
}

int write_struct_eq_func(FILE *cfile, const char *struct_name,
                         const struct StructFields *sf,
                         const struct CodegenConfig *config) {
  size_t i;
  if (!cfile || !struct_name || !sf)
    return EINVAL;

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->utils_guard));
  }

  /* Generate function header */
  CHECK_FPRINTF(fprintf(cfile,
                        "int %s_eq(const struct %s *a, const struct %s *b) {\n",
                        struct_name, struct_name, struct_name));

  /* Trivial comparisons */
  CHECK_FPRINTF(fprintf(cfile, "  if (a == b) return 1;\n"));
  CHECK_FPRINTF(fprintf(cfile, "  if (!a || !b) return 0;\n"));

  /* Field-by-field comparisons */
  for (i = 0; i < sf->size; ++i) {
    const char *n = sf->fields[i].name;
    const char *t = sf->fields[i].type;
    const char *r = sf->fields[i].ref;

    if (strcmp(t, "string") == 0) {
      CHECK_FPRINTF(fprintf(cfile,
                            "  if (a->%s != b->%s && (!a->%s || !b->%s || "
                            "strcmp(a->%s, b->%s) != 0)) return 0;\n",
                            n, n, n, n, n, n));
    } else if (strcmp(t, "object") == 0) {
      /* Pointers to structs, recursive deep eq call */
      CHECK_FPRINTF(fprintf(cfile, "  if (!%s_eq(a->%s, b->%s)) return 0;\n",
                            get_type_from_ref(r), n, n));
    } else if (strcmp(t, "array") == 0) {
      /* Array comparison logic */
      CHECK_FPRINTF(
          fprintf(cfile, "  if (a->n_%s != b->n_%s) return 0;\n", n, n));

      /* New block for loop iterator */
      CHECK_FPRINTF(
          fprintf(cfile, "  { size_t i; for (i = 0; i < a->n_%s; ++i) {\n", n));

      if (strcmp(r, "string") == 0) {
        CHECK_FPRINTF(fprintf(
            cfile,
            "    if (a->%s[i] != b->%s[i] && (!a->%s[i] || !b->%s[i] || "
            "strcmp(a->%s[i], b->%s[i]) != 0)) return 0;\n",
            n, n, n, n, n, n));
      } else if (strcmp(r, "integer") == 0 || strcmp(r, "number") == 0 ||
                 strcmp(r, "boolean") == 0) {
        CHECK_FPRINTF(
            fprintf(cfile, "    if (a->%s[i] != b->%s[i]) return 0;\n", n, n));
      } else {
        /* Object array - typically struct Item **items */
        CHECK_FPRINTF(fprintf(cfile,
                              "    if (!%s_eq(a->%s[i], b->%s[i])) return 0;\n",
                              get_type_from_ref(r), n, n));
      }
      CHECK_FPRINTF(fprintf(cfile, "  }}\n"));
    } else {
      /* Primitive types (integer, boolean, number, enum) */
      CHECK_FPRINTF(fprintf(cfile, "  if (a->%s != b->%s) return 0;\n", n, n));
    }
  }

  CHECK_FPRINTF(fprintf(cfile, "  return 1;\n}\n"));

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->utils_guard));
  }
  return 0;
}

int write_struct_debug_func(FILE *cfile, const char *struct_name,
                            const struct StructFields *sf,
                            const struct CodegenConfig *const config) {
  if (!cfile || !struct_name || !sf)
    return EINVAL;

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->utils_guard));
  }

  CHECK_FPRINTF(fprintf(cfile,
                        "int %s_debug(const struct %s *obj, FILE *fp) { return "
                        "fprintf(fp, \"Debug %s\\n\"); }\n",
                        struct_name, struct_name, struct_name));

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->utils_guard));
  }
  return 0;
}

int write_struct_display_func(FILE *cfile, const char *struct_name,
                              const struct StructFields *sf,
                              const struct CodegenConfig *const config) {
  if (!cfile || !struct_name || !sf)
    return EINVAL;

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->utils_guard));
  }

  CHECK_FPRINTF(fprintf(cfile,
                        "int %s_display(const struct %s *obj, FILE *fp) { "
                        "return fprintf(fp, \"Display %s\\n\"); }\n",
                        struct_name, struct_name, struct_name));

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->utils_guard));
  }
  return 0;
}

/* --- Union Functions --- */

int write_union_to_json_func(FILE *const cfile, const char *const union_name,
                             const struct StructFields *const sf,
                             const struct CodegenConfig *const config) {
  size_t i;
  int needs_nested_rc = 0;

  if (!cfile || !union_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "object") == 0)
      needs_nested_rc = 1;
  }

  if (config && config->json_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->json_guard));
  }

  CHECK_FPRINTF(fprintf(
      cfile,
      "int %s_to_json(const struct %s *const obj, char **const json) {\n",
      union_name, union_name));

  if (needs_nested_rc)
    CHECK_FPRINTF(fprintf(cfile, "  int rc;\n"));

  CHECK_FPRINTF(fprintf(cfile,
                        "  if (obj == NULL || json == NULL) return EINVAL;\n"
                        "  switch (obj->tag) {\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    const char *ref = sf->fields[i].ref;

    CHECK_FPRINTF(fprintf(cfile, "    case %s_%s:\n", union_name, name));
    CHECK_FPRINTF(fprintf(cfile, "      jasprintf(json, \"{\");\n      if "
                                 "(*json==NULL) return ENOMEM;\n"));

    if (strcmp(type, "integer") == 0) {
      CHECK_FPRINTF(fprintf(
          cfile, "      jasprintf(json, \"\\\"%s\\\": %%d}\", obj->data.%s);\n",
          name, name));
    } else if (strcmp(type, "string") == 0) {
      CHECK_FPRINTF(fprintf(cfile,
                            "      jasprintf(json, \"\\\"%s\\\": "
                            "\\\"%%s\\\"}\", obj->data.%s);\n",
                            name, name));
    } else if (strcmp(type, "object") == 0) {
      CHECK_FPRINTF(
          fprintf(cfile,
                  "      {\n"
                  "        char *sub = NULL;\n"
                  "        rc = %s_to_json(obj->data.%s, &sub);\n"
                  "        if (rc != 0) return rc;\n"
                  "        jasprintf(json, \"\\\"%s\\\": %%s}\", sub);\n"
                  "        free(sub);\n"
                  "      }\n",
                  get_type_from_ref(ref), name, name));
    }
    CHECK_FPRINTF(fprintf(cfile, "      break;\n"));
  }

  CHECK_FPRINTF(fprintf(cfile, "    default:\n"
                               "      jasprintf(json, \"null\");\n"
                               "      break;\n"
                               "  }\n"
                               "  if (*json == NULL) return ENOMEM;\n"
                               "  return 0;\n"
                               "}\n"));

  if (config && config->json_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->json_guard));
  }
  CHECK_FPRINTF(fprintf(cfile, "\n"));

  return 0;
}

int write_union_from_jsonObject_func(FILE *const cfile,
                                     const char *const union_name,
                                     const struct StructFields *const sf,
                                     const struct CodegenConfig *const config) {
  size_t i;
  int needs_nested_rc = 0;

  if (!cfile || !union_name || !sf)
    return EINVAL;

  for (i = 0; i < sf->size; ++i) {
    if (strcmp(sf->fields[i].type, "object") == 0)
      needs_nested_rc = 1;
  }

  if (config && config->json_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->json_guard));
  }

  CHECK_FPRINTF(fprintf(cfile,
                        "int %s_from_jsonObject(const JSON_Object *const "
                        "jsonObject, struct %s **const out) {\n",
                        union_name, union_name));

  if (needs_nested_rc)
    CHECK_FPRINTF(fprintf(cfile, "  int rc;\n"));

  CHECK_FPRINTF(fprintf(cfile,
                        "  struct %s *ret = malloc(sizeof(struct %s));\n"
                        "  if (!ret) return ENOMEM;\n"
                        "  memset(ret, 0, sizeof(*ret));\n\n",
                        union_name, union_name));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    const char *ref = sf->fields[i].ref;

    CHECK_FPRINTF(fprintf(
        cfile, "  if (json_object_has_value(jsonObject, \"%s\")) {\n", name));
    CHECK_FPRINTF(fprintf(cfile, "    ret->tag = %s_%s;\n", union_name, name));

    if (strcmp(type, "integer") == 0) {
      CHECK_FPRINTF(
          fprintf(cfile,
                  "    ret->data.%s = (int)json_object_get_number(jsonObject, "
                  "\"%s\");\n",
                  name, name));
    } else if (strcmp(type, "string") == 0) {
      CHECK_FPRINTF(
          fprintf(cfile,
                  "    ret->data.%s = "
                  "strdup(json_object_get_string(jsonObject, \"%s\"));\n"
                  "    if (!ret->data.%s) { free(ret); return ENOMEM; }\n",
                  name, name, name));
    } else if (strcmp(type, "object") == 0) {
      CHECK_FPRINTF(fprintf(
          cfile,
          "    rc = %s_from_jsonObject(json_object_get_object(jsonObject, "
          "\"%s\"), &ret->data.%s);\n"
          "    if (rc != 0) { free(ret); return rc; }\n",
          get_type_from_ref(ref), name, name));
    }
    CHECK_FPRINTF(fprintf(cfile, "    *out = ret;\n    return 0;\n  }\n"));
  }

  CHECK_FPRINTF(fprintf(cfile, "  free(ret);\n  return EINVAL;\n}\n"));

  if (config && config->json_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->json_guard));
  }
  CHECK_FPRINTF(fprintf(cfile, "\n"));

  return 0;
}

int write_union_cleanup_func(FILE *const cfile, const char *const union_name,
                             const struct StructFields *const sf,
                             const struct CodegenConfig *const config) {
  size_t i;
  if (!cfile || !union_name || !sf)
    return EINVAL;

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#ifdef %s\n", config->utils_guard));
  }

  CHECK_FPRINTF(fprintf(cfile, "void %s_cleanup(struct %s *const obj) {\n",
                        union_name, union_name));
  CHECK_FPRINTF(fprintf(cfile, "  if (!obj) return;\n  switch (obj->tag) {\n"));

  for (i = 0; i < sf->size; ++i) {
    const char *name = sf->fields[i].name;
    CHECK_FPRINTF(fprintf(cfile, "    case %s_%s:\n", union_name, name));
    if (strcmp(sf->fields[i].type, "string") == 0) {
      CHECK_FPRINTF(fprintf(cfile, "      free((void*)obj->data.%s);\n", name));
    } else if (strcmp(sf->fields[i].type, "object") == 0) {
      CHECK_FPRINTF(fprintf(cfile, "      %s_cleanup(obj->data.%s);\n",
                            get_type_from_ref(sf->fields[i].ref), name));
    }
    CHECK_FPRINTF(fprintf(cfile, "      break;\n"));
  }

  CHECK_FPRINTF(fprintf(cfile, "    default: break;\n  }\n  free(obj);\n}\n"));

  if (config && config->utils_guard) {
    CHECK_FPRINTF(fprintf(cfile, "#endif /* %s */\n", config->utils_guard));
  }
  CHECK_FPRINTF(fprintf(cfile, "\n"));

  return 0;
}

/* --- Root Array Marshalling Logic --- */

int write_root_array_from_json_func(FILE *fp, const char *name,
                                    const char *item_type, const char *item_ref,
                                    const struct CodegenConfig *config) {
  if (!fp || !name || !item_type)
    return EINVAL;

  if (config && config->json_guard)
    CHECK_FPRINTF(fprintf(fp, "#ifdef %s\n", config->json_guard));

  /* Signature: int Name_from_json(const char *json, Type **out, size_t *len) */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_FPRINTF(fprintf(
        fp, "int %s_from_json(const char *json, int **out, size_t *len) {\n",
        name));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_FPRINTF(fprintf(
        fp, "int %s_from_json(const char *json, char ***out, size_t *len) {\n",
        name));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_FPRINTF(fprintf(
        fp,
        "int %s_from_json(const char *json, struct %s ***out, size_t *len) {\n",
        name, get_type_from_ref(item_ref)));
  } else {
    /* Fallback / Unsupported */
    CHECK_FPRINTF(fprintf(
        fp, "int %s_from_json(const char *json, void **out, size_t *len) {\n",
        name));
  }

  CHECK_FPRINTF(fprintf(
      fp,
      "  JSON_Value *val;\n"
      "  JSON_Array *arr;\n"
      "  size_t i, count;\n"
      "  if (!json || !out || !len) return EINVAL;\n"
      "  val = json_parse_string(json);\n"
      "  if (!val) return EINVAL;\n"
      "  arr = json_value_get_array(val);\n"
      "  if (!arr) { json_value_free(val); return EINVAL; }\n"
      "  count = json_array_get_count(arr);\n"
      "  *len = count;\n"
      "  if (count == 0) { *out = NULL; json_value_free(val); return 0; }\n"));

  /* Allocation */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_FPRINTF(fprintf(fp, "  *out = malloc(count * sizeof(int));\n"));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_FPRINTF(fprintf(fp, "  *out = calloc(count, sizeof(char*));\n"));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_FPRINTF(fprintf(fp, "  *out = calloc(count, sizeof(struct %s*));\n",
                          get_type_from_ref(item_ref)));
  }

  CHECK_FPRINTF(
      fprintf(fp, "  if (!*out) { json_value_free(val); return ENOMEM; }\n"
                  "  for (i = 0; i < count; ++i) {\n"));

  /* Parsing Elements */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_FPRINTF(
        fprintf(fp, "    (*out)[i] = (int)json_array_get_number(arr, i);\n"));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_FPRINTF(fprintf(
        fp,
        "    const char *s = json_array_get_string(arr, i);\n"
        "    if (s) (*out)[i] = strdup(s);\n"
        "    if (!(*out)[i]) {\n"
        "      /* cleanup */\n"
        "      size_t j;\n"
        "      for(j=0; j<i; j++) free((*out)[j]);\n"
        "      free(*out); *out=NULL; json_value_free(val); return ENOMEM;\n"
        "    }\n"));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_FPRINTF(fprintf(
        fp,
        "    int rc = %s_from_jsonObject(json_array_get_object(arr, i), "
        "&(*out)[i]);\n"
        "    if (rc != 0) {\n"
        "      size_t j;\n"
        "      for(j=0; j<i; j++) %s_cleanup((*out)[j]);\n"
        "      free(*out); *out=NULL; json_value_free(val); return rc;\n"
        "    }\n",
        get_type_from_ref(item_ref), get_type_from_ref(item_ref)));
  }

  CHECK_FPRINTF(
      fprintf(fp, "  }\n  json_value_free(val);\n  return 0;\n}\n\n"));

  if (config && config->json_guard)
    CHECK_FPRINTF(fprintf(fp, "#endif /* %s */\n\n", config->json_guard));
  return 0;
}

int write_root_array_to_json_func(FILE *fp, const char *name,
                                  const char *item_type, const char *item_ref,
                                  const struct CodegenConfig *config) {
  if (!fp || !name || !item_type)
    return EINVAL;

  if (config && config->json_guard)
    CHECK_FPRINTF(fprintf(fp, "#ifdef %s\n", config->json_guard));

  /* Signature: int Name_to_json(const Type *in, size_t len, char **json_out) */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_FPRINTF(fprintf(
        fp, "int %s_to_json(const int *in, size_t len, char **json_out) {\n",
        name));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_FPRINTF(fprintf(
        fp, "int %s_to_json(char **const in, size_t len, char **json_out) {\n",
        name));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_FPRINTF(fprintf(
        fp,
        "int %s_to_json(struct %s **const in, size_t len, char **json_out) {\n",
        name, get_type_from_ref(item_ref)));
  } else {
    CHECK_FPRINTF(fprintf(
        fp, "int %s_to_json(const void *in, size_t len, char **json_out) {\n",
        name));
  }

  CHECK_FPRINTF(fprintf(fp, "  size_t i;\n"
                            "  if (!in && len > 0) return EINVAL;\n"
                            "  if (!json_out) return EINVAL;\n"
                            "  jasprintf(json_out, \"[\");\n"
                            "  if (!*json_out) return ENOMEM;\n"
                            "  for (i = 0; i < len; ++i) {\n"
                            "    if (i > 0) { jasprintf(json_out, \",\"); "
                            "if(!*json_out) return ENOMEM; }\n"));

  if (strcmp(item_type, "integer") == 0) {
    CHECK_FPRINTF(fprintf(fp, "    jasprintf(json_out, \"%%d\", in[i]);\n"));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_FPRINTF(
        fprintf(fp, "    jasprintf(json_out, \"\\\"%%s\\\"\", in[i]);\n"));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_FPRINTF(fprintf(fp,
                          "    {\n"
                          "      char *tmp = NULL;\n"
                          "      int rc = %s_to_json(in[i], &tmp);\n"
                          "      if (rc != 0) return rc;\n"
                          "      jasprintf(json_out, \"%%s\", tmp);\n"
                          "      free(tmp);\n"
                          "    }\n",
                          get_type_from_ref(item_ref)));
  }
  CHECK_FPRINTF(fprintf(fp, "    if (!*json_out) return ENOMEM;\n  }\n"));
  CHECK_FPRINTF(fprintf(fp, "  jasprintf(json_out, \"]\");\n  if(!*json_out) "
                            "return ENOMEM;\n  return 0;\n}\n\n"));

  if (config && config->json_guard)
    CHECK_FPRINTF(fprintf(fp, "#endif /* %s */\n\n", config->json_guard));
  return 0;
}

int write_root_array_cleanup_func(FILE *fp, const char *name,
                                  const char *item_type, const char *item_ref,
                                  const struct CodegenConfig *config) {
  if (!fp || !name || !item_type)
    return EINVAL;

  if (config && config->utils_guard)
    CHECK_FPRINTF(fprintf(fp, "#ifdef %s\n", config->utils_guard));

  /* For primitive int, just explicit free is done by user typically, but we
   * provide helper for consistency */
  if (strcmp(item_type, "integer") == 0) {
    CHECK_FPRINTF(fprintf(
        fp,
        "void %s_cleanup(int *in, size_t len) {\n  (void)len; free(in);\n}\n",
        name));
  } else if (strcmp(item_type, "string") == 0) {
    CHECK_FPRINTF(fprintf(fp,
                          "void %s_cleanup(char **in, size_t len) {\n"
                          "  size_t i;\n"
                          "  if (!in) return;\n"
                          "  for(i=0; i<len; ++i) free(in[i]);\n"
                          "  free(in);\n"
                          "}\n",
                          name));
  } else if (strcmp(item_type, "object") == 0) {
    CHECK_FPRINTF(fprintf(fp,
                          "void %s_cleanup(struct %s **in, size_t len) {\n"
                          "  size_t i;\n"
                          "  if (!in) return;\n"
                          "  for(i=0; i<len; ++i) %s_cleanup(in[i]);\n"
                          "  free(in);\n"
                          "}\n",
                          name, get_type_from_ref(item_ref),
                          get_type_from_ref(item_ref)));
  } else {
    /* Fallback stub */
    CHECK_FPRINTF(fprintf(
        fp, "void %s_cleanup(void *in, size_t len) { free(in); }\n", name));
  }

  if (config && config->utils_guard)
    CHECK_FPRINTF(fprintf(fp, "#endif /* %s */\n\n", config->utils_guard));
  return 0;
}
