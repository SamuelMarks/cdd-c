#include <stdlib.h>
#include <string.h>

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

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

/* Extract type name from JSON pointer, e.g., "#/components/schemas/TypeName"
 * -> "TypeName" */
const char *get_type_from_ref(const char *ref) {
  const char *last_slash;
  if (ref == NULL)
    return ""; /* Return empty string to avoid segfaults in printf */
  last_slash = strrchr(ref, '/');
  if (last_slash) {
    return last_slash + 1;
  }
  return ref;
}

/* Enum -> string conversion */
void write_enum_to_str_func(FILE *cfile, const char *const enum_name,
                            const struct EnumMembers *em) {
  size_t i;
  const char *member;
  if (!cfile || !enum_name || !em || !em->members)
    return;

  fprintf(cfile,
          "int %s_to_str(enum %s val, char **str_out) {\n"
          "  if (str_out == NULL) return EINVAL;\n"
          "  switch (val) {\n",
          enum_name, enum_name);

  for (i = 0; i < em->size; i++) {
    member = em->members[i];
    if (!member)
      continue;

    if (strcmp(member, "UNKNOWN") == 0)
      continue;

    fprintf(cfile,
            "    case %s_%s:\n"
            "      *str_out = strdup(\"%s\");\n"
            "      break;\n",
            enum_name, member, member);
  }

  fprintf(cfile, "    case %s_UNKNOWN:\n", enum_name);
  fputs("    default:\n"
        "      *str_out = strdup(\"UNKNOWN\");\n"
        "      break;\n"
        "  }\n"
        "  if (*str_out == NULL) return ENOMEM;\n"
        "  return 0;\n"
        "}\n\n",
        cfile);
}

/* String -> enum conversion */
void write_enum_from_str_func(FILE *cfile, const char *const enum_name,
                              const struct EnumMembers *em) {
  size_t i;
  const char *member;
  if (!cfile || !enum_name || !em || !em->members)
    return;

  fprintf(cfile,
          "int %s_from_str(const char *const str, enum %s *val) {\n"
          "  if (val == NULL) return EINVAL;\n"
          "  else if (str == NULL) *val = %s_UNKNOWN;\n",
          enum_name, enum_name, enum_name);

  for (i = 0; i < em->size; i++) {
    member = em->members[i];
    if (!member)
      continue;

    fprintf(cfile, "  else if (strcmp(str, \"%s\") == 0) *val = %s_%s;\n",
            member, enum_name, member);
  }

  fprintf(cfile, "  else *val = %s_UNKNOWN;\n", enum_name);
  fputs("  return 0;\n"
        "}\n",
        cfile);
}

/* from_jsonObject: parse JSON_Object into struct safely */
void write_struct_from_jsonObject_func(FILE *cfile,
                                       const char *const struct_name,
                                       const struct StructFields *const sf) {
  size_t i;
  bool needs_rc = false;
  const struct StructField *f;

  if (!cfile || !struct_name || !sf)
    return;

  for (i = 0; i < sf->size; i++) {
    f = &sf->fields[i];
    if (strcmp(f->type, "enum") == 0 || strcmp(f->type, "object") == 0) {
      needs_rc = true;
      break;
    }
  }

  fprintf(cfile,
          "int %s_from_jsonObject(const JSON_Object *const jsonObject, struct "
          "%s **const out) {\n",
          struct_name, struct_name);

  if (needs_rc) {
    fprintf(cfile, "  int rc;\n");
  }

  fprintf(cfile, "  struct %s *ret;\n", struct_name);

  for (i = 0; i < sf->size; i++) {
    f = &sf->fields[i];
    if (strcmp(f->type, "enum") == 0)
      fprintf(cfile, "  const char *_%s_str = NULL;\n", f->name);
    else if (strcmp(f->type, "object") == 0)
      fprintf(cfile, "  const JSON_Object *_%s_obj = NULL;\n", f->name);
  }

  fprintf(cfile,
          "  if (!jsonObject || !out) return EINVAL;\n"
          "  ret = malloc(sizeof(struct %s));\n"
          "  if (!ret) return ENOMEM;\n"
          "  memset(ret, 0, sizeof(*ret));\n",
          struct_name);

  for (i = 0; i < sf->size; i++) {
    f = &sf->fields[i];

    if (strcmp(f->type, "string") == 0) {
      fprintf(
          cfile,
          "  {\n"
          "    const char *tmp = json_object_get_string(jsonObject, \"%s\");\n"
          "    if (!tmp || tmp[0] == '\\0') {\n"
          "      ret->%s = NULL;\n"
          "    } else {\n"
          "      ret->%s = strdup(tmp);\n"
          "      if (!ret->%s) { %s_cleanup(ret); return ENOMEM; }\n"
          "    }\n"
          "  }\n",
          f->name, f->name, f->name, f->name, struct_name);

    } else if (strcmp(f->type, "integer") == 0) {
      fprintf(cfile,
              "  ret->%s = (int)json_object_get_number(jsonObject, \"%s\");\n",
              f->name, f->name);

    } else if (strcmp(f->type, "boolean") == 0) {
      fprintf(cfile,
              "  ret->%s = json_object_get_boolean(jsonObject, \"%s\");\n",
              f->name, f->name);

    } else if (strcmp(f->type, "number") == 0) {
      fprintf(cfile,
              "  ret->%s = json_object_get_number(jsonObject, \"%s\");\n",
              f->name, f->name);

    } else if (strcmp(f->type, "enum") == 0) {
      fprintf(cfile,
              "  _%s_str = json_object_get_string(jsonObject, \"%s\");\n"
              "  if (_%s_str == NULL) { %s_cleanup(ret); return EINVAL; }\n"
              "  rc = %s_from_str(_%s_str, &ret->%s);\n"
              "  if (rc) { %s_cleanup(ret); return rc; }\n",
              f->name, f->name, f->name, struct_name, get_type_from_ref(f->ref),
              f->name, f->name, struct_name);

    } else if (strcmp(f->type, "object") == 0) {
      fprintf(cfile,
              "  _%s_obj = json_object_get_object(jsonObject, \"%s\");\n"
              "  if (_%s_obj) {\n"
              "    rc = %s_from_jsonObject(_%s_obj, &ret->%s);\n"
              "    if (rc != 0) { %s_cleanup(ret); return rc; }\n"
              "  } else {\n"
              "    ret->%s = NULL;\n"
              "  }\n",
              f->name, f->name, f->name, get_type_from_ref(f->ref), f->name,
              f->name, struct_name, f->name);
    }
  }

  fprintf(cfile, "  *out = ret;\n"
                 "  return 0;\n"
                 "}\n\n");
}

/* Wrapper for from_json parsing a JSON string */
void write_struct_from_json_func(FILE *cfile, const char *const struct_name) {
  if (!cfile || !struct_name)
    return;
  fprintf(
      cfile,
      "int %s_from_json(const char *const json_str, struct %s **const out) {\n"
      "  JSON_Value *root = NULL;\n"
      "  const JSON_Object *jsonObject = NULL;\n"
      "  int rc;\n"
      "  if (json_str == NULL || out == NULL) return EINVAL;\n"
      "  root = json_parse_string(json_str);\n"
      "  if (root == NULL) return EINVAL;\n"
      "  jsonObject = json_value_get_object(root);\n"
      "  if (jsonObject == NULL) { json_value_free(root); return EINVAL; }\n"
      "  rc = %s_from_jsonObject(jsonObject, out);\n"
      "  json_value_free(root);\n"
      "  return rc;\n"
      "}\n\n",
      struct_name, struct_name, struct_name);
}

/* Serialize struct to JSON string with proper comma control */
void write_struct_to_json_func(FILE *cfile, const char *const struct_name,
                               const struct StructFields *const fields) {
  size_t i;
  bool needs_rc = false;
  const struct StructField *f;
  const char *name;
  const char *type;

  if (!cfile || !struct_name || !fields)
    return;

  for (i = 0; i < fields->size; i++) {
    f = &fields->fields[i];
    if (strcmp(f->type, "enum") == 0 || strcmp(f->type, "object") == 0) {
      needs_rc = true;
      break;
    }
  }

  fprintf(cfile,
          "int %s_to_json(const struct %s *const obj, char **const json) {\n",
          struct_name, struct_name);

  if (needs_rc) {
    fprintf(cfile, "  int rc;\n");
  }
  fputs("  int need_comma = 0;\n"
        "  if (obj == NULL || json == NULL) return EINVAL;\n"
        "  jasprintf(json, \"{\");\n"
        "  if (*json == NULL) return ENOMEM;\n"
        "\n",
        cfile);

  for (i = 0; i < fields->size; i++) {
    f = &fields->fields[i];
    name = f->name;
    type = f->type;

    if (strcmp(type, "string") == 0) {
      /* Print comma only if this field will be emitted */
      fprintf(cfile,
              "  if (obj->%s) {\n"
              "    if (need_comma) {\n"
              "      jasprintf(json, \",\");\n"
              "      if (*json == NULL) return ENOMEM;\n"
              "    }\n"
              "    jasprintf(json, \"\\\"%s\\\": \\\"%%s\\\"\", obj->%s);\n"
              "    if (*json == NULL) return ENOMEM;\n"
              "    need_comma = 1;\n"
              "  } else {\n"
              "    if (need_comma) {\n"
              "      jasprintf(json, \",\");\n"
              "      if (*json == NULL) return ENOMEM;\n"
              "    }\n"
              "    jasprintf(json, \"\\\"%s\\\": null\");\n"
              "    if (*json == NULL) return ENOMEM;\n"
              "    need_comma = 1;\n"
              "  }\n\n",
              name, name, name, name);

    } else if (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0) {
      fprintf(cfile,
              "  if (need_comma) {\n"
              "    jasprintf(json, \",\");\n"
              "    if (*json == NULL) return ENOMEM;\n"
              "  }\n"
              "  jasprintf(json, \"\\\"%s\\\": %%d\", obj->%s);\n"
              "  if (*json == NULL) return ENOMEM;\n"
              "  need_comma = 1;\n\n",
              name, name);

    } else if (strcmp(type, "boolean") == 0) {
      fprintf(cfile,
              "  if (need_comma) {\n"
              "    jasprintf(json, \",\");\n"
              "    if (*json == NULL) return ENOMEM;\n"
              "  }\n"
              "  jasprintf(json, \"\\\"%s\\\": %%d\", obj->%s);\n"
              "  if (*json == NULL) return ENOMEM;\n"
              "  need_comma = 1;\n\n",
              name, name);

    } else if (strcmp(type, "enum") == 0) {
      fprintf(cfile,
              "  if (need_comma) {\n"
              "    jasprintf(json, \",\");\n"
              "    if (*json == NULL) return ENOMEM;\n"
              "  }\n"
              "  {\n"
              "    char *enum_str = NULL;\n"
              "    rc = %s_to_str(obj->%s, &enum_str);\n"
              "    if (rc) { free(enum_str); return rc; }\n"
              "  jasprintf(json, \"\\\"%s\\\": \\\"%%s\\\"\", enum_str);\n"
              "  free(enum_str);\n"
              "  if (*json == NULL) return ENOMEM;\n"
              "  need_comma = 1;\n\n"
              "  }\n",
              get_type_from_ref(f->ref), name, name);

    } else if (strcmp(type, "object") == 0) {
      fprintf(cfile,
              "  if (obj->%s) {\n"
              "    if (need_comma) {\n"
              "      jasprintf(json, \",\");\n"
              "      if (*json == NULL) return ENOMEM;\n"
              "    }\n"
              "    {\n"
              "      char *nested_json = NULL;\n"
              "    rc = %s_to_json(obj->%s, &nested_json);\n"
              "    if (rc) { free(nested_json); return rc; }\n"
              "    jasprintf(json, \"\\\"%s\\\": %%s\", nested_json);\n"
              "    free(nested_json);\n"
              "    }\n"
              "    if (*json == NULL) return ENOMEM;\n"
              "    need_comma = 1;\n"
              "  } else {\n"
              "    if (need_comma) {\n"
              "      jasprintf(json, \",\");\n"
              "      if (*json == NULL) return ENOMEM;\n"
              "    }\n",
              name, get_type_from_ref(f->ref), name, name);
      fprintf(cfile,
              "    jasprintf(json, \"\\\"%s\\\": null\");\n"
              "    if (*json == NULL) return ENOMEM;\n"
              "    need_comma = 1;\n"
              "  }\n\n",
              name);

    } else {
      fprintf(cfile, "  /* unhandled field type '%s' for field '%s' */\n\n",
              type, name);
    }
  }

  fputs("  jasprintf(json, \"}\");\n"
        "  if (*json == NULL) return ENOMEM;\n"
        "  return 0;\n"
        "}\n",
        cfile);
}

/* Equality test */
void write_struct_eq_func(FILE *cfile, const char *struct_name,
                          const struct StructFields *sf) {
  size_t i;
  const struct StructField *f;
  if (!cfile || !struct_name || !sf)
    return;
  fprintf(cfile,
          "int %s_eq(const struct %s *const a, const struct %s *const b) {\n",
          struct_name, struct_name, struct_name);
  fprintf(cfile, "  if (a == NULL || b == NULL) return a == b;\n");

  for (i = 0; i < sf->size; i++) {
    f = &sf->fields[i];

    if (strcmp(f->type, "string") == 0) {
      fprintf(cfile,
              "  if (a->%s == NULL || b->%s == NULL) {\n"
              "    if (a->%s != b->%s) return 0;\n"
              "  } else if (strcmp(a->%s, b->%s) != 0) return 0;\n",
              f->name, f->name, f->name, f->name, f->name, f->name);

    } else if (strcmp(f->type, "object") == 0) {
      fprintf(cfile, "  if (!%s_eq(a->%s, b->%s)) return 0;\n",
              get_type_from_ref(f->ref), f->name, f->name);

    } else if (strcmp(f->type, "enum") == 0 ||
               strcmp(f->type, "integer") == 0 ||
               strcmp(f->type, "boolean") == 0 ||
               strcmp(f->type, "number") == 0) {
      fprintf(cfile, "  if (a->%s != b->%s) return 0;\n", f->name, f->name);
    }
  }

  fprintf(cfile, "  return 1;\n}\n\n");
}

/* Cleanup frees strings and nested objects */
void write_struct_cleanup_func(FILE *cfile, const char *struct_name,
                               const struct StructFields *sf) {
  size_t i;
  const struct StructField *f;
  if (!cfile || !struct_name || !sf)
    return;
  fprintf(cfile, "void %s_cleanup(struct %s *const obj) {\n", struct_name,
          struct_name);
  fprintf(cfile, "  if (obj == NULL) return;\n");

  for (i = 0; i < sf->size; i++) {
    f = &sf->fields[i];
    if (strcmp(f->type, "string") == 0) {
      fprintf(cfile, "  free((void *)obj->%s);\n", f->name);
    } else if (strcmp(f->type, "object") == 0) {
      fprintf(cfile, "  %s_cleanup(obj->%s);\n", get_type_from_ref(f->ref),
              f->name);
    }
  }

  fprintf(cfile, "  free(obj);\n");
  fprintf(cfile, "}\n\n");
}

/* Default initialize struct */
void write_struct_default_func(FILE *cfile, const char *struct_name,
                               const struct StructFields *sf) {
  size_t i;
  const struct StructField *f;
  if (!cfile || !struct_name || !sf)
    return;
  fprintf(cfile, "int %s_default(struct %s **out) {\n", struct_name,
          struct_name);
  fputs("  if (!out) return EINVAL;\n", cfile);
  fprintf(cfile, "  *out = malloc(sizeof(**out));\n"
                 "  if (*out == NULL) return ENOMEM;\n"
                 "  memset(*out, 0, sizeof(**out));\n");

  for (i = 0; i < sf->size; i++) {
    f = &sf->fields[i];
    if (strcmp(f->type, "string") == 0) {
      fprintf(cfile, "  (*out)->%s = NULL;\n", f->name);
    } else if (strcmp(f->type, "integer") == 0 ||
               strcmp(f->type, "boolean") == 0 ||
               strcmp(f->type, "number") == 0) {
      fprintf(cfile, "  (*out)->%s = 0;\n", f->name);
    } else if (strcmp(f->type, "enum") == 0) {
      fprintf(cfile, "  (*out)->%s = %s_UNKNOWN;\n", f->name,
              get_type_from_ref(f->ref));
    } else if (strcmp(f->type, "object") == 0) {
      fprintf(cfile,
              "  if (%s_default(&(*out)->%s) != 0) {\n"
              "    free(*out);\n"
              "    *out = NULL;\n"
              "    return ENOMEM;\n"
              "  }\n",
              get_type_from_ref(f->ref), f->name);
    }
  }

  fprintf(cfile, "  return 0;\n}\n\n");
}

/* Deep copy */
void write_struct_deepcopy_func(FILE *cfile, const char *struct_name,
                                const struct StructFields *sf) {
  size_t i;
  const struct StructField *f;
  if (!cfile || !struct_name || !sf)
    return;
  fprintf(cfile, "int %s_deepcopy(const struct %s *src, struct %s **dest) {\n",
          struct_name, struct_name, struct_name);
  fputs("  if (!dest) return EINVAL;\n", cfile);
  fprintf(cfile, "  *dest = malloc(sizeof(**dest));\n"
                 "  if (*dest == NULL) return ENOMEM;\n"
                 "  if (!src) { free(*dest); *dest = NULL; return 0; }\n");

  for (i = 0; i < sf->size; i++) {
    f = &sf->fields[i];

    if (strcmp(f->type, "string") == 0) {
      fprintf(cfile,
              "  if (src->%s) {\n"
              "    (*dest)->%s = strdup(src->%s);\n"
              "    if (!(*dest)->%s) { %s_cleanup(*dest); *dest = NULL; return "
              "ENOMEM; }\n"
              "  } else {\n"
              "    (*dest)->%s = NULL;\n"
              "  }\n",
              f->name, f->name, f->name, f->name, struct_name, f->name);

    } else if (strcmp(f->type, "integer") == 0 ||
               strcmp(f->type, "boolean") == 0 ||
               strcmp(f->type, "number") == 0 || strcmp(f->type, "enum") == 0) {
      fprintf(cfile, "  (*dest)->%s = src->%s;\n", f->name, f->name);

    } else if (strcmp(f->type, "object") == 0) {
      fprintf(
          cfile,
          "  if (src->%s) {\n"
          "    int rc = %s_deepcopy(src->%s, &(*dest)->%s);\n"
          "    if (rc != 0) { %s_cleanup(*dest); *dest = NULL; return rc; }\n"
          "  } else {\n"
          "    (*dest)->%s = NULL;\n"
          "  }\n",
          f->name, get_type_from_ref(f->ref), f->name, f->name, struct_name,
          f->name);
    }
  }

  fprintf(cfile, "  return 0;\n}\n\n");
}

/* Display: print JSON string to file */
void write_struct_display_func(FILE *cfile, const char *struct_name,
                               const struct StructFields *sf) {
  if (!cfile || !struct_name || !sf)
    return;
  fprintf(cfile,
          "int %s_display(const struct %s *obj, FILE *fh) {\n"
          "  char *json_str = NULL;\n"
          "  int rc = %s_to_json(obj, &json_str);\n"
          "  if (rc != 0) return rc;\n"
          "  rc = fprintf(fh, \"%%s\\n\", json_str);\n"
          "  if (rc > 0) rc = 0;\n"
          "  free(json_str);\n"
          "  return rc;\n"
          "}\n\n",
          struct_name, struct_name, struct_name);
}

/* Debug: print values of fields */
void write_struct_debug_func(FILE *cfile, const char *struct_name,
                             const struct StructFields *sf) {
  size_t i;
  const struct StructField *f;
  if (!cfile || !struct_name || !sf)
    return;
  fprintf(cfile,
          "int %s_debug(const struct %s *obj, FILE *fh) {\n"
          "  int rc;\n"
          "  if (obj == NULL) {\n"
          "    rc = fputs(\"<null %s>\\n\", fh);\n"
          "    return rc < 0 ? rc : 0;\n"
          "  }\n"
          "  rc = fputs(\"struct %s {\\n\", fh);\n"
          "  if (rc < 0) return rc;\n",
          struct_name, struct_name, struct_name, struct_name);

  for (i = 0; i < sf->size; i++) {
    f = &sf->fields[i];
    if (strcmp(f->type, "string") == 0) {
      fprintf(cfile,
              "  {\n"
              "    char *quoted = NULL;\n"
              "    rc = quote_or_null(obj->%s, &quoted);\n"
              "    if (rc != 0) return rc;\n"
              "    rc = fprintf(fh, \"  %s = %%s\\n\", quoted);\n"
              "    free(quoted);\n"
              "    if (rc < 0) return rc;\n"
              "  }\n",
              f->name, f->name);
    } else if (strcmp(f->type, "integer") == 0) {
      fprintf(
          cfile,
          /* "  rc = fprintf(fh, \"  %s = %%" INT_FMT "\\n\", obj->%s);\n" */
          "  rc = fprintf(fh, \"  %s = %%d\\n\", obj->%s);\n"
          "  if (rc < 0) return rc;\n",
          f->name, f->name);
    } else if (strcmp(f->type, "boolean") == 0) {
      fprintf(cfile,
              "  rc = fprintf(fh, \"  %s = %%d\\n\", obj->%s);\n"
              "  if (rc < 0) return rc;\n",
              f->name, f->name);
    } else if (strcmp(f->type, "number") == 0) {
      fprintf(cfile,
              "  rc = fprintf(fh, \"  %s = %%f\\n\", obj->%s);\n"
              "  if (rc < 0) return rc;\n",
              f->name, f->name);
    } else if (strcmp(f->type, "enum") == 0) {
      fprintf(cfile,
              "  rc = fprintf(fh, \"  %s = %%d\\n\", obj->%s);\n"
              "  if (rc < 0) return rc;\n",
              f->name, f->name);
    } else if (strcmp(f->type, "object") == 0) {
      fprintf(cfile,
              "  if (obj->%s) {\n"
              "    rc = %s_debug(obj->%s, fh);\n"
              "    if (rc != 0) return rc;\n"
              "  }\n",
              f->name, get_type_from_ref(f->ref), f->name);
    }
  }

  fprintf(cfile, "  rc = fputs(\"}\\n\", fh);\n"
                 "  if (rc < 0) return rc;\n"
                 "  return 0;\n"
                 "}\n\n");
}
