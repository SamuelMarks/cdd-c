#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(strdup)
#define strdup _strdup
#endif

/* Generate enum to_str function with prefixed enum members like Tank_UNKNOWN */
void write_enum_to_str_func(FILE *cfile, const char *const enum_name,
                            const struct EnumMembers *em) {
  size_t i;
  if (!cfile || !enum_name || !em || !em->members)
    return;

  fprintf(cfile,
          "int %s_to_str(enum %s val, char **str_out) {\n"
          "  if (str_out == NULL) return -1;\n"
          "  switch (val) {\n",
          enum_name, enum_name);

  for (i = 0; i < em->size; i++) {
    if (em->members[i] == NULL)
      continue;

    /* Skip unknown here, emit after cases */
    if (strcmp(em->members[i], "UNKNOWN") == 0)
      continue;

    fprintf(cfile,
            "    case %s:\n"
            "      *str_out = strdup(\"%s\");\n"
            "      break;\n",
            em->members[i], em->members[i]);
  }

  /* Single UNKNOWN case */
  fputs("    case UNKNOWN:\n"
        "    default:\n"
        "      *str_out = strdup(\"UNKNOWN\");\n"
        "      break;\n"
        "  }\n"
        "  if (*str_out == NULL) return -2;\n"
        "  return 0;\n"
        "}\n\n",
        cfile);
}

/* Generate enum from_str function with prefixed enum members */
void write_enum_from_str_func(FILE *cfile, const char *const enum_name,
                              const struct EnumMembers *em) {
  size_t i;
  if (!cfile || !enum_name || !em || !em->members)
    return;

  fprintf(cfile,
          "int %s_from_str(const char *const str, enum %s *val) {\n"
          "  int rc = 0;\n"
          "  if (val == NULL)\n"
          "    rc = EINVAL;\n"
          "  else if (str == NULL)\n"
          "    *val = UNKNOWN;\n",
          enum_name, enum_name);

  for (i = 0; i < em->size; i++) {
    if (em->members[i] == NULL)
      continue;

    if (strcmp(em->members[i], "UNKNOWN") == 0)
      continue;

    fprintf(cfile,
            "  else if (strcmp(str, \"%s\") == 0)\n"
            "    *val = %s;\n",
            em->members[i], em->members[i]);
  }

  fputs("  else\n"
        "    *val = UNKNOWN;\n"
        "  return rc;\n"
        "}\n\n",
        cfile);
}

/* Generate struct declaration with enum fields properly typed */
void write_struct_declaration(FILE *hfile, const char *struct_name,
                              const struct StructFields *sf) {
  size_t i;

  fprintf(hfile, "struct LIB_EXPORT %s {\n", struct_name);
  for (i = 0; i < sf->size; i++) {
    struct StructField f = sf->fields[i];
    if (strcmp(f.type, "string") == 0) {
      fprintf(hfile, "  const char *%s;\n", f.name);
    } else if (strcmp(f.type, "integer") == 0) {
      fprintf(hfile, "  int %s;\n", f.name);
    } else if (strcmp(f.type, "boolean") == 0) {
      fprintf(hfile, "  int %s;\n", f.name);
    } else if (strcmp(f.type, "number") == 0) {
      fprintf(hfile, "  double %s;\n", f.name);
    } else if (strcmp(f.type, "enum") == 0) {
      fprintf(hfile, "  enum %s %s;\n", f.ref, f.name);
    } else if (strcmp(f.type, "object") == 0) {
      fprintf(hfile, "  struct %s *%s;\n", f.ref, f.name);
    }
  }
  fprintf(hfile, "};\n\n");
}

/* Generate from_jsonObject function: correctly parse all fields (including
 * enums) */
void write_struct_from_jsonObject_func(FILE *cfile,
                                       const char *const struct_name,
                                       const struct StructFields *const sf) {
  size_t i;

  fprintf(cfile,
          "int %s_from_jsonObject(const JSON_Object *const jsonObject, struct "
          "%s **const out) {\n",
          struct_name, struct_name);

  fprintf(cfile, "  int rc;\n");
  fprintf(cfile, "  struct %s *ret;\n", struct_name);

  for (i = 0; i < sf->size; i++) {
    struct StructField f = sf->fields[i];
    if (strcmp(f.type, "enum") == 0)
      fprintf(cfile, "  const char *_%s_str = NULL;\n", f.name);
    else if (strcmp(f.type, "object") == 0)
      fprintf(cfile, "  const JSON_Object *_%s_obj = NULL;\n", f.name);
  }

  fprintf(cfile,
          "  if (!jsonObject || !out) return EINVAL;\n"
          "  ret = malloc(sizeof(struct %s));\n"
          "  if (!ret) return ENOMEM;\n"
          "  memset(ret, 0, sizeof(*ret));\n",
          struct_name);

  for (i = 0; i < sf->size; i++) {
    struct StructField f = sf->fields[i];
    if (strcmp(f.type, "string") == 0) {
      fprintf(
          cfile,
          "  {\n"
          "    const char *tmp = json_object_get_string(jsonObject, \"%s\");\n"
          "    if (!tmp) { %s_cleanup(ret); return EINVAL; }\n"
          "    ret->%s = strdup(tmp);\n"
          "    if (!ret->%s) { %s_cleanup(ret); return ENOMEM; }\n"
          "  }\n",
          f.name, struct_name, f.name, f.name, struct_name);
    } else if (strcmp(f.type, "integer") == 0) {
      fprintf(cfile,
              "  ret->%s = (int)json_object_get_number(jsonObject, \"%s\");\n",
              f.name, f.name);
    } else if (strcmp(f.type, "boolean") == 0) {
      fprintf(cfile,
              "  ret->%s = json_object_get_boolean(jsonObject, \"%s\");\n",
              f.name, f.name);
    } else if (strcmp(f.type, "number") == 0) {
      fprintf(cfile,
              "  ret->%s = json_object_get_number(jsonObject, \"%s\");\n",
              f.name, f.name);
    } else if (strcmp(f.type, "enum") == 0) {
      fprintf(cfile,
              "  _%s_str = json_object_get_string(jsonObject, \"%s\");\n"
              "  if (!_%s_str) { %s_cleanup(ret); return EINVAL; }\n"
              "  rc = %s_from_str(_%s_str, &ret->%s);\n"
              "  if (rc) { %s_cleanup(ret); return rc; }\n",
              f.name, f.name, f.name, struct_name, f.ref, f.name, f.name,
              struct_name);
    } else if (strcmp(f.type, "object") == 0) {
      fprintf(cfile,
              "  _%s_obj = json_object_get_object(jsonObject, \"%s\");\n"
              "  if (_%s_obj) {\n"
              "    rc = %s_from_jsonObject(_%s_obj, &ret->%s);\n"
              "    if (rc) { %s_cleanup(ret); return rc; }\n"
              "  } else {\n"
              "    ret->%s = NULL;\n"
              "  }\n",
              f.name, f.name, f.name, f.ref, f.name, f.name, struct_name,
              f.name);
    }
  }

  fprintf(cfile, "  *out = ret;\n"
                 "  return 0;\n"
                 "}\n\n");
}

/* Wrapper from_json function - parses string then calls from_jsonObject */
void write_struct_from_json_func(FILE *cfile, const char *const struct_name) {
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

/* Generate to_json function for struct, handles enums and nested objects */
void write_struct_to_json_func(FILE *cfile, const char *struct_name,
                               const struct StructFields *fields) {
  size_t i;

  fprintf(cfile,
          "int %s_to_json(const struct %s *const obj, char **const json) {\n",
          struct_name, struct_name);

  fprintf(cfile, "  int rc = 0;\n");
  fprintf(cfile, "  char *enum_str = NULL;\n");
  fprintf(cfile, "  char *nested_json = NULL;\n");
  fprintf(cfile, "  if (obj == NULL || json == NULL) return EINVAL;\n");

  fprintf(cfile, "  jasprintf(json, \"{\");\n");
  fprintf(cfile, "  if (*json == NULL) return ENOMEM;\n");

  for (i = 0; i < fields->size; i++) {
    const struct StructField *f = &fields->fields[i];
    const char *name = f->name;
    const char *type = f->type;

    if (i > 0) {
      fprintf(cfile, "  jasprintf(json, \",\");\n");
      fprintf(cfile, "  if (*json == NULL) return ENOMEM;\n");
    }

    if (strcmp(type, "string") == 0) {
      fprintf(cfile,
              "  jasprintf(json, \"\\\"%s\\\": \\\"%%s\\\"\", obj->%s ? "
              "obj->%s : \"\");\n",
              name, name, name);
      fprintf(cfile, "  if (*json == NULL) return ENOMEM;\n");
    } else if (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0) {
      fprintf(cfile, "  jasprintf(json, \"\\\"%s\\\": %%d\", obj->%s);\n", name,
              name);
      fprintf(cfile, "  if (*json == NULL) return ENOMEM;\n");
    } else if (strcmp(type, "boolean") == 0) {
      fprintf(cfile, "  jasprintf(json, \"\\\"%s\\\": %%d\", obj->%s);\n", name,
              name);
      fprintf(cfile, "  if (*json == NULL) return ENOMEM;\n");
    } else if (strcmp(type, "enum") == 0) {
      fprintf(cfile,
              "  rc = %s_to_str(obj->%s, &enum_str);\n"
              "  if (rc) { free(enum_str); return rc; }\n"
              "  jasprintf(json, \"\\\"%s\\\": \\\"%%s\\\"\", enum_str);\n"
              "  free(enum_str);\n"
              "  if (*json == NULL) return ENOMEM;\n",
              f->ref, name, name);
    } else if (strcmp(type, "object") == 0) {
      fprintf(cfile,
              "  if (obj->%s) {\n"
              "    rc = %s_to_json(obj->%s, &nested_json);\n"
              "    if (rc) { free(nested_json); return rc; }\n"
              "    jasprintf(json, \"\\\"%s\\\": %%s\", nested_json);\n"
              "    free(nested_json);\n"
              "    if (*json == NULL) return ENOMEM;\n"
              "  } else {\n"
              "    jasprintf(json, \"\\\"%s\\\": null\");\n"
              "    if (*json == NULL) return ENOMEM;\n"
              "  }\n",
              name, f->ref, name, name, name);
    } else {
      fprintf(cfile, "  /* unhandled field type %s for %s */\n", type, name);
    }
  }

  fprintf(cfile, "  jasprintf(json, \"}\");\n");
  fprintf(cfile, "  if (*json == NULL) return ENOMEM;\n");
  fprintf(cfile, "  return 0;\n");
  fprintf(cfile, "}\n\n");
}

/* eq function compares all fields */
void write_struct_eq_func(FILE *cfile, const char *struct_name,
                          const struct StructFields *sf) {
  size_t i;

  fprintf(cfile,
          "int %s_eq(const struct %s *const a, const struct %s *const b) {\n",
          struct_name, struct_name, struct_name);
  fprintf(cfile, "  if (a == NULL || b == NULL) return a == b;\n");

  for (i = 0; i < sf->size; i++) {
    struct StructField f = sf->fields[i];

    if (strcmp(f.type, "string") == 0) {
      fprintf(cfile,
              "  if (a->%s == NULL || b->%s == NULL) {\n"
              "    if (a->%s != b->%s) return 0;\n"
              "  } else if (strcmp(a->%s, b->%s) != 0) return 0;\n",
              f.name, f.name, f.name, f.name, f.name, f.name);
    } else if (strcmp(f.type, "object") == 0) {
      fprintf(cfile, "  if (!%s_eq(a->%s, b->%s)) return 0;\n", f.ref, f.name,
              f.name);
    } else if (strcmp(f.type, "enum") == 0 || strcmp(f.type, "integer") == 0 ||
               strcmp(f.type, "boolean") == 0 ||
               strcmp(f.type, "number") == 0) {
      fprintf(cfile, "  if (a->%s != b->%s) return 0;\n", f.name, f.name);
    }
  }

  fprintf(cfile, "  return 1;\n");
  fprintf(cfile, "}\n\n");
}

/* Cleanup function, frees strings and nested structs */
void write_struct_cleanup_func(FILE *cfile, const char *struct_name,
                               const struct StructFields *sf) {
  size_t i;

  fprintf(cfile, "void %s_cleanup(struct %s *const obj) {\n", struct_name,
          struct_name);
  fprintf(cfile, "  if (obj == NULL) return;\n");
  for (i = 0; i < sf->size; i++) {
    struct StructField f = sf->fields[i];
    if (strcmp(f.type, "string") == 0) {
      fprintf(cfile, "  free((void *)obj->%s);\n", f.name);
    } else if (strcmp(f.type, "object") == 0) {
      fprintf(cfile, "  %s_cleanup(obj->%s);\n", f.ref, f.name);
    }
  }
  fprintf(cfile, "  free(obj);\n");
  fprintf(cfile, "}\n\n");
}

/* default function initializes fields */
void write_struct_default_func(FILE *cfile, const char *struct_name,
                               const struct StructFields *sf) {
  size_t i;

  fprintf(cfile, "int %s_default(struct %s **out) {\n", struct_name,
          struct_name);

  fprintf(cfile, "  *out = malloc(sizeof(**out));\n");
  fprintf(cfile, "  if (*out == NULL) return ENOMEM;\n");

  for (i = 0; i < sf->size; i++) {
    struct StructField f = sf->fields[i];
    if (strcmp(f.type, "string") == 0) {
      fprintf(cfile, "  (*out)->%s = NULL;\n", f.name);
    } else if (strcmp(f.type, "integer") == 0 ||
               strcmp(f.type, "boolean") == 0 ||
               strcmp(f.type, "number") == 0) {
      fprintf(cfile, "  (*out)->%s = 0;\n", f.name);
    } else if (strcmp(f.type, "enum") == 0) {
      fprintf(cfile, "  (*out)->%s = %s_UNKNOWN;\n", f.name, f.ref);
    } else if (strcmp(f.type, "object") == 0) {
      fprintf(cfile,
              "  if (%s_default(&(*out)->%s) != 0) {\n"
              "    free(*out);\n"
              "    *out = NULL;\n"
              "    return ENOMEM;\n"
              "  }\n",
              f.ref, f.name);
    }
  }

  fprintf(cfile, "  return 0;\n");
  fprintf(cfile, "}\n\n");
}

/* deepcopy function does a deep copy of all fields */
void write_struct_deepcopy_func(FILE *cfile, const char *struct_name,
                                const struct StructFields *sf) {
  size_t i;
  fprintf(cfile, "int %s_deepcopy(const struct %s *src, struct %s **dest) {\n",
          struct_name, struct_name, struct_name);
  fprintf(cfile, "  *dest = malloc(sizeof(**dest));\n");
  fprintf(cfile, "  if (*dest == NULL) return ENOMEM;\n");
  fprintf(cfile, "  if (!src) { free(*dest); *dest = NULL; return 0; }\n");
  for (i = 0; i < sf->size; i++) {
    struct StructField f = sf->fields[i];

    if (strcmp(f.type, "string") == 0) {
      fprintf(cfile,
              "  if (src->%s) {\n"
              "    (*dest)->%s = strdup(src->%s);\n"
              "    if ((*dest)->%s == NULL) { %s_cleanup(*dest); *dest = NULL; "
              "return ENOMEM; }\n"
              "  } else {\n"
              "    (*dest)->%s = NULL;\n"
              "  }\n",
              f.name, f.name, f.name, f.name, struct_name, f.name);
    } else if (strcmp(f.type, "integer") == 0 ||
               strcmp(f.type, "boolean") == 0 ||
               strcmp(f.type, "number") == 0 || strcmp(f.type, "enum") == 0) {
      fprintf(cfile, "  (*dest)->%s = src->%s;\n", f.name, f.name);
    } else if (strcmp(f.type, "object") == 0) {
      fprintf(
          cfile,
          "  if (src->%s) {\n"
          "    int rc = %s_deepcopy(src->%s, &(*dest)->%s);\n"
          "    if (rc != 0) { %s_cleanup(*dest); *dest = NULL; return rc; }\n"
          "  } else {\n"
          "    (*dest)->%s = NULL;\n"
          "  }\n",
          f.name, f.ref, f.name, f.name, struct_name, f.name);
    }
  }
  fprintf(cfile, "  return 0;\n");
  fprintf(cfile, "}\n\n");
}

/* Display function prints JSON string */
void write_struct_display_func(FILE *cfile, const char *struct_name,
                               const struct StructFields *sf) {
  (void)sf;
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

/* Debug function prints field values for debugging */
void write_struct_debug_func(FILE *cfile, const char *struct_name,
                             const struct StructFields *sf) {
  size_t i;
  fprintf(cfile, "int %s_debug(const struct %s *obj, FILE *fh) {\n",
          struct_name, struct_name);
  fprintf(cfile,
          "  int rc = fputs(\"struct %s dbg = {\\n\", fh);\n"
          "  if (rc < 0) return rc;\n",
          struct_name);
  for (i = 0; i < sf->size; i++) {
    struct StructField f = sf->fields[i];
    if (strcmp(f.type, "string") == 0) {
      fprintf(cfile,
              "  {\n"
              "    char *quoted = NULL;\n"
              "    rc = quote_or_null(obj->%s, &quoted);\n"
              "    if (rc != 0) return rc;\n"
              "    rc = fprintf(fh, \"  /* const char * */ \\\"%%s\\\",\\n\", "
              "quoted);\n"
              "    free(quoted);\n"
              "    if (rc < 0) return rc;\n"
              "  }\n",
              f.name);
    } else if (strcmp(f.type, "integer") == 0) {
      fprintf(cfile,
              "  rc = fprintf(fh, \"  /* int */ " INT_FMT "\\n\", obj->%s);\n",
              f.name);
      fprintf(cfile, "  if (rc < 0) return rc;\n");
    } else if (strcmp(f.type, "boolean") == 0) {
      fprintf(cfile,
              "  rc = fprintf(fh, \"  /* int (bool) */ %%d\\n\", obj->%s);\n",
              f.name);
      fprintf(cfile, "  if (rc < 0) return rc;\n");
    } else if (strcmp(f.type, "number") == 0) {
      fprintf(cfile,
              "  rc = fprintf(fh, \"  /* double */ %%f\\n\", obj->%s);\n",
              f.name);
      fprintf(cfile, "  if (rc < 0) return rc;\n");
    } else if (strcmp(f.type, "enum") == 0) {
      fprintf(cfile,
              "  rc = fprintf(fh, \"  /* enum %s */ %%d\\n\", obj->%s);\n",
              f.ref, f.name);
      fprintf(cfile, "  if (rc < 0) return rc;\n");
    } else if (strcmp(f.type, "object") == 0) {
      fprintf(cfile,
              "  if (obj->%s) {\n"
              "    rc = %s_debug(obj->%s, fh);\n"
              "    if (rc != 0) return rc;\n"
              "  }\n",
              f.name, f.ref, f.name);
    }
  }
  fprintf(cfile, "  rc = fputs(\"};\\n\", fh);\n"
                 "  if (rc < 0) return rc;\n"
                 "  return 0;\n"
                 "}\n\n");
}
