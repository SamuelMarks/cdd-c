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

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

void write_enum_to_str_func(FILE *cfile, const char *enum_name,
                            const struct EnumMembers *em) {
  size_t i;

  /* Defensive: validate inputs */
  if (!cfile || !enum_name || !em) {
    fprintf(stderr, "write_enum_to_str_func: NULL argument\n");
    return;
  }
  if (!em->members) {
    fprintf(stderr, "write_enum_to_str_func: NULL em->members pointer\n");
    return;
  }

  fprintf(cfile,
          "int %s_to_str(enum %s val, char **str_out) {\n"
          "  if (str_out == NULL) return -1;\n"
          "  switch (val) {\n",
          enum_name, enum_name);

  for (i = 0; i < em->size; i++) {
    if (em->members[i] == NULL)
      continue;

    fprintf(cfile,
            "    case %s:\n"
            "      *str_out = strdup(\"%s\");\n"
            "      break;\n",
            em->members[i], em->members[i]);
  }

  fprintf(cfile, "    case UNKNOWN:\n"
                 "    default:\n"
                 "      *str_out = strdup(\"UNKNOWN\");\n"
                 "      break;\n"
                 "  }\n");

  fprintf(cfile, "  if (*str_out == NULL) return -2;\n"
                 "  return 0;\n"
                 "}\n\n");
}

void write_enum_from_str_func(FILE *cfile, const char *enum_name,
                              const struct EnumMembers *em) {
  size_t i;

  /* Defensive: validate inputs */
  if (!cfile || !enum_name || !em) {
    fputs("write_enum_from_str_func: NULL argument", stderr);
    return;
  }
  if (!em->members) {
    fputs("write_enum_from_str_func: NULL em->members pointer", stderr);
    return;
  }

  fprintf(cfile,
          "int %s_from_str(const char *str, enum %s *val) {\n"
          "  int rc = 0;\n"
          "  if (val == NULL)\n"
          "    rc = EINVAL;\n"
          "  else if (str == NULL)\n"
          "    *val = UNKNOWN;\n",
          enum_name, enum_name);

  for (i = 0; i < em->size; i++) {
    if (em->members[i] == NULL)
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

void write_struct_from_jsonObject_func(FILE *cfile, const char *struct_name,
                                       const struct StructFields *sf) {
  size_t i;

  fprintf(cfile,
          "int %s_from_jsonObject(const JSON_Object *obj, struct %s **out) {\n"
          "  struct %s *ret = NULL;\n"
          "  if (!obj || !out) return -1;\n"
          "  ret = (struct %s *)malloc(sizeof(struct %s));\n"
          "  if (!ret) return -2;\n"
          "  memset(ret, 0, sizeof(*ret));\n",
          struct_name, struct_name, struct_name, struct_name, struct_name);

  for (i = 0; i < sf->size; i++) {
    const char *f = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    if (strcmp(type, "string") == 0) {
      fprintf(cfile,
              "  {\n"
              "    const char *tmp = json_object_get_string(obj, \"%s\");\n"
              "    if (!tmp) { free(ret); return -3; }\n"
              "    ret->%s = strdup(tmp);\n"
              "    if (!ret->%s) { free(ret); return -4; }\n"
              "  }\n",
              f, f, f);
    } else if (strcmp(type, "integer") == 0) {
      fprintf(cfile, "  ret->%s = (int)json_object_get_number(obj, \"%s\");\n",
              f, f);
    } else if (strcmp(type, "boolean") == 0) {
      fprintf(cfile, "  ret->%s = json_object_get_boolean(obj, \"%s\");\n", f,
              f);
    } else if (strcmp(type, "number") == 0) {
      fprintf(cfile, "  ret->%s = json_object_get_number(obj, \"%s\");\n", f,
              f);
    } else if (strcmp(type, "object") == 0) {
      fprintf(cfile,
              "  {\n"
              "    const JSON_Object *nested = json_object_get_object(obj, "
              "\"%s\");\n"
              "    if (!nested) { free(ret); return -5; }\n"
              "    int rc = %s_from_jsonObject(nested, &ret->%s);\n"
              "    if (rc) { free(ret); return rc; }\n"
              "  }\n",
              f, sf->fields[i].ref, f);
    } else if (strcmp(type, "enum") == 0) {
      fprintf(
          cfile,
          "  {\n"
          "    const char *enum_str = json_object_get_string(obj, \"%s\");\n"
          "    if (!enum_str) { free(ret); return -6; }\n"
          "    ret->%s = (enum %s *)malloc(sizeof(enum %s));\n"
          "    if (!ret->%s) { free(ret); return -7; }\n"
          "    int rc = %s_from_str(enum_str, ret->%s);\n"
          "    if (rc) {\n"
          "      free(ret->%s);\n"
          "      free(ret);\n"
          "      return rc;\n"
          "    }\n"
          "  }\n",
          f, f, sf->fields[i].ref, sf->fields[i].ref, f, sf->fields[i].ref, f,
          f);
    }
  }
  fprintf(cfile, "  *out = ret;\n"
                 "  return 0;\n"
                 "}\n\n");
}

/*
 * Write struct cleanup function
 * Frees all string members, calls cleanup on nested structs, then frees struct.
 */
void write_struct_cleanup_func(FILE *cfile, const char *struct_name,
                               const struct StructFields *sf) {
  size_t i;

  fprintf(cfile,
          "int %s_to_json(const struct %s *const obj, char **json_str) {\n"
          "  JSON_Value *root_value = NULL;\n"
          "  JSON_Object *root_object = NULL;\n"
          "  char *serialized_str = NULL;\n"
          "  if (obj == NULL || json_str == NULL) return -1;\n"
          "  root_value = json_value_init_object();\n"
          "  if (!root_value) return -2;\n"
          "  root_object = json_value_get_object(root_value);\n",
          struct_name, struct_name);

  for (i = 0; i < sf->size; i++) {
    const char *f = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    if (strcmp(type, "string") == 0) {
      fprintf(cfile,
              "  if (obj->%s != NULL) {\n"
              "    json_object_set_string(root_object, \"%s\", obj->%s);\n"
              "  } else {\n"
              "    json_object_set_null(root_object, \"%s\");\n"
              "  }\n",
              f, f, f, f);
    } else if (strcmp(type, "integer") == 0) {
      fprintf(
          cfile,
          "  json_object_set_number(root_object, \"%s\", (double)obj->%s);\n",
          f, f);
    } else if (strcmp(type, "boolean") == 0) {
      fprintf(cfile,
              "  json_object_set_boolean(root_object, \"%s\", obj->%s);\n", f,
              f);
    } else if (strcmp(type, "number") == 0) {
      fprintf(cfile,
              "  json_object_set_number(root_object, \"%s\", obj->%s);\n", f,
              f);
    } else if (strcmp(type, "object") == 0) {
      fprintf(
          cfile,
          "  {\n"
          "    char *nested_json = NULL;\n"
          "    int rc = %s_to_json(obj->%s, &nested_json);\n"
          "    if (rc) { json_value_free(root_value); return rc; }\n"
          "    JSON_Value *nested_val = json_parse_string(nested_json);\n"
          "    free(nested_json);\n"
          "    if (!nested_val) { json_value_free(root_value); return -3; }\n"
          "    json_object_set_value(root_object, \"%s\", nested_val);\n"
          "  }\n",
          sf->fields[i].ref, f, f);
    } else if (strcmp(type, "enum") == 0) {
      fprintf(cfile,
              "  {\n"
              "    char *str = NULL;\n"
              "    if (obj->%s != NULL) {\n"
              "      int rc = %s_to_str(*(obj->%s), &str);\n"
              "      if (rc) {\n"
              "        json_value_free(root_value);\n"
              "        return rc;\n"
              "      }\n"
              "      json_object_set_string(root_object, \"%s\", str);\n"
              "      free(str);\n"
              "    } else {\n"
              "      json_object_set_null(root_object, \"%s\");\n"
              "    }\n"
              "  }\n",
              f, sf->fields[i].ref, f, f, f);
    }
  }

  fprintf(cfile,
          "  serialized_str = json_serialize_to_string_pretty(root_value);\n"
          "  if (!serialized_str) {\n"
          "    json_value_free(root_value);\n"
          "    return -4;\n"
          "  }\n"
          "  *json_str = strdup(serialized_str);\n"
          "  json_free_serialized_string(serialized_str);\n"
          "  json_value_free(root_value);\n"
          "  if (*json_str == NULL) return -5;\n"
          "  return 0;\n"
          "}\n\n");
}

/*
 * Equality function generator
 * Compares strings with strcmp, scalars with ==
 * Nested structs by calling their eq function.
 */
void write_struct_eq_func(FILE *cfile, const char *struct_name,
                          const struct StructFields *sf) {
  size_t i;
  fprintf(cfile,
          "int %s_eq(const struct %s *const a, const struct %s *const b) {\n",
          struct_name, struct_name, struct_name);
  fprintf(cfile, "  if (a == NULL || b == NULL) return a == b;\n");
  for (i = 0; i < sf->size; i++) {
    if (strcmp(sf->fields[i].type, "string") == 0) {
      fprintf(cfile,
              "  if (a->%s == NULL || b->%s == NULL) {\n"
              "    if (a->%s != b->%s) return 0;\n"
              "  } else if (strcmp(a->%s, b->%s) != 0) return 0;\n",
              sf->fields[i].name, sf->fields[i].name, sf->fields[i].name,
              sf->fields[i].name, sf->fields[i].name, sf->fields[i].name);
    } else if (strcmp(sf->fields[i].type, "object") == 0) {
      fprintf(cfile, "  if (!%s_eq(a->%s, b->%s)) return 0;\n",
              sf->fields[i].ref, sf->fields[i].name, sf->fields[i].name);
    } else if (strcmp(sf->fields[i].type, "enum") == 0) {
      fprintf(cfile,
              "  if (a->%s == NULL || b->%s == NULL) {\n"
              "    if (a->%s != b->%s) return 0;\n"
              "  } else if (*(a->%s) != *(b->%s)) return 0;\n",
              sf->fields[i].name, sf->fields[i].name, sf->fields[i].name,
              sf->fields[i].name, sf->fields[i].name, sf->fields[i].name);
    } else if (strcmp(sf->fields[i].type, "integer") == 0 ||
               strcmp(sf->fields[i].type, "boolean") == 0 ||
               strcmp(sf->fields[i].type, "number") == 0) {
      fprintf(cfile, "  if (a->%s != b->%s) return 0;\n", sf->fields[i].name,
              sf->fields[i].name);
    }
  }
  fprintf(cfile, "  return 1;\n}\n\n");
}

void write_struct_to_json_func(FILE *cfile, const char *struct_name,
                               const struct StructFields *fields) {
  size_t i;

  if (!cfile || !struct_name || !fields)
    return;

  fprintf(cfile, "int %s_to_json(const struct %s *obj, JSON_Value **out) {\n",
          struct_name, struct_name);
  fprintf(cfile, "  if (!obj || !out) return -1;\n");
  fprintf(cfile, "  JSON_Value *root = json_value_init_object();\n");
  fprintf(cfile, "  JSON_Object *object = json_value_get_object(root);\n");
  fprintf(cfile, "  if (!object) {\n");
  fprintf(cfile, "    json_value_free(root);\n");
  fprintf(cfile, "    return -2;\n");
  fprintf(cfile, "  }\n\n");

  for (i = 0; i < fields->size; i++) {
    const struct StructField *field = &fields->fields[i];
    const char *name = field->name;

    if (strcmp(field->type, "string") == 0) {
      fprintf(cfile,
              "  if (obj->%s != NULL) json_object_set_string(object, \"%s\", "
              "obj->%s);\n",
              name, name, name);
    } else if (strcmp(field->type, "integer") == 0) {
      fprintf(cfile, "  json_object_set_number(object, \"%s\", obj->%s);\n",
              name, name);
    } else if (strcmp(field->type, "number") == 0) {
      fprintf(cfile, "  json_object_set_number(object, \"%s\", obj->%s);\n",
              name, name);
    } else if (strcmp(field->type, "boolean") == 0) {
      fprintf(cfile, "  json_object_set_boolean(object, \"%s\", obj->%s);\n",
              name, name);
    } else if (strcmp(field->type, "enum") == 0) {
      fprintf(cfile,
              "  {\n"
              "    char *str = NULL;\n"
              "    if (obj->%s != NULL) {\n"
              "      int rc = %s_to_str(*(obj->%s), &str);\n"
              "      if (rc) {\n"
              "        json_value_free(root);\n"
              "        return rc;\n"
              "      }\n"
              "      json_object_set_string(object, \"%s\", str);\n"
              "      free(str);\n"
              "    } else {\n"
              "      json_object_set_null(object, \"%s\");\n"
              "    }\n"
              "  }\n",
              field->name, field->ref, field->name, field->name, field->name);
    } else if (field->ref[0] != '\0') {
      /* Nested struct ref: assume we have a to_json function and pointer */
      fprintf(cfile,
              "  if (obj->%s != NULL) {\n"
              "    JSON_Value *nested_val = NULL;\n"
              "    if (%s_to_json(obj->%s, &nested_val) == 0 && nested_val != "
              "NULL) {\n"
              "      json_object_set_value(object, \"%s\", nested_val);\n"
              "    } else {\n"
              "      /* error handling can be improved */\n"
              "      json_value_free(nested_val);\n"
              "    }\n"
              "  }\n",
              name, get_basename(field->ref + 1), name, name);
    }
    /* Add more type handling or arrays as needed */
  }

  fprintf(cfile, "  *out = root;\n");
  fprintf(cfile, "  return 0;\n");
  fprintf(cfile, "}\n\n");
}

/* Write struct from_json function (parse string, call from_jsonObject) */
void write_struct_from_json_func(FILE *const cfile,
                                 const char *const struct_name) {
  fprintf(cfile,
          "int %s_from_json(const char *json_str, struct %s **out) {\n"
          "  JSON_Value *root_value = NULL;\n"
          "  const JSON_Object *root_object = NULL;\n"
          "  int rc;\n"
          "  if (!json_str || !out) return -1;\n"
          "  root_value = json_parse_string(json_str);\n"
          "  if (!root_value) return -2;\n"
          "  root_object = json_value_get_object(root_value);\n"
          "  if (!root_object) { json_value_free(root_value); return -3; }\n"
          "  rc = %s_from_jsonObject(root_object, out);\n"
          "  json_value_free(root_value);\n"
          "  return rc;\n"
          "}\n\n",
          struct_name, struct_name, struct_name);
}

void write_struct_default_func(FILE *const f, const char *const struct_name,
                               const struct StructFields *const fields) {
  size_t i;
  fprintf(f, "int %s_default(struct %s **out) {\n", struct_name, struct_name);
  fprintf(f, "  *out = malloc(sizeof(**out));\n");
  fprintf(f, "  if (*out == NULL) return ENOMEM;\n");
  for (i = 0; i < fields->size; i++) {
    struct StructField field = fields->fields[i];
    /* Initialize defaults - string = NULL, int/boolean/number = 0, enum calls
     * enum_default */
    if (strcmp(field.type, "string") == 0) {
      fprintf(f, "  (*out)->%s = NULL;\n", field.name);
    } else if (strcmp(field.type, "integer") == 0 ||
               strcmp(field.type, "boolean") == 0 ||
               strcmp(field.type, "number") == 0) {
      fprintf(f, "  (*out)->%s = 0;\n", field.name);
    } else if (strcmp(field.type, "enum") == 0) {
      /* call enum_default(), check error */
      fprintf(f, "  (*out)->%s = %s_default();\n", field.name, field.ref);
    } else if (strcmp(field.type, "object") == 0) {
      /* allocate nested struct default, check error */
      fprintf(f,
              "  if (%s_default(&(*out)->%s) != 0) {\n"
              "    free(*out);\n"
              "    *out = NULL;\n"
              "    return ENOMEM;\n"
              "  }\n",
              field.ref, field.name);
    } else {
      fprintf(f, "  /* No default for %s */\n", field.name);
    }
  }
  fprintf(f, "  return 0;\n");
  fprintf(f, "}\n\n");
}

void write_struct_deepcopy_func(FILE *const f, const char *const struct_name,
                                const struct StructFields *const fields) {
  size_t i;
  fprintf(f, "int %s_deepcopy(const struct %s *src, struct %s **dest) {\n",
          struct_name, struct_name, struct_name);
  fprintf(f, "  *dest = malloc(sizeof(**dest));\n");
  fprintf(f, "  if (*dest == NULL) return ENOMEM;\n");
  fprintf(f, "  if (!src) {\n    free(*dest);\n    *dest = NULL;\n    return "
             "0;\n  }\n");
  for (i = 0; i < fields->size; i++) {
    struct StructField field = fields->fields[i];
    if (strcmp(field.type, "string") == 0) {
      fprintf(f,
              "  if (src->%s) {\n"
              "    (*dest)->%s = strdup(src->%s);\n"
              "    if ((*dest)->%s == NULL) { %s_cleanup(*dest); *dest = NULL; "
              "return ENOMEM; }\n"
              "  } else {\n"
              "    (*dest)->%s = NULL;\n"
              "  }\n",
              field.name, field.name, field.name, field.name, struct_name,
              field.name);
    } else if (strcmp(field.type, "integer") == 0 ||
               strcmp(field.type, "boolean") == 0 ||
               strcmp(field.type, "number") == 0) {
      fprintf(f, "  (*dest)->%s = src->%s;\n", field.name, field.name);
    } else if (strcmp(field.type, "enum") == 0) {
      fprintf(f, "  (*dest)->%s = src->%s;\n", field.name, field.name);
    } else if (strcmp(field.type, "object") == 0) {
      fprintf(
          f,
          "  if (src->%s) {\n"
          "    int rc = %s_deepcopy(src->%s, &(*dest)->%s);\n"
          "    if (rc != 0) { %s_cleanup(*dest); *dest = NULL; return rc; }\n"
          "  } else {\n"
          "    (*dest)->%s = NULL;\n"
          "  }\n",
          field.name, field.ref, field.name, field.name, struct_name,
          field.name);
    } else {
      fprintf(f, "  /* deep copy: unhandled type for field %s */\n",
              field.name);
    }
  }
  fprintf(f, "  return 0;\n");
  fprintf(f, "}\n\n");
}

void write_struct_display_func(FILE *const f, const char *const struct_name,
                               const struct StructFields *const fields) {
  (void)fields;
  fprintf(f, "int %s_display(const struct %s *obj, FILE *fh) {\n", struct_name,
          struct_name);
  fprintf(f, "  char *json_str = NULL;\n");
  fprintf(f, "  int rc = %s_to_json(obj, &json_str);\n", struct_name);
  fprintf(f, "  if (rc != 0) return rc;\n");
  fprintf(f, "  rc = fprintf(fh, \"%%s\\n\", json_str);\n");
  fprintf(f, "  if (rc > 0) rc = 0;\n");
  fprintf(f, "  free(json_str);\n");
  fprintf(f, "  return rc;\n");
  fprintf(f, "}\n\n");
}

void write_struct_debug_func(FILE *const f, const char *const struct_name,
                             const struct StructFields *const fields) {
  size_t i;

  fprintf(f, "int %s_debug(const struct %s *obj, FILE *fh) {\n", struct_name,
          struct_name);
  fprintf(f, "  int rc = fputs(\"struct %s dbg = {\\n\", fh);\n", struct_name);
  fprintf(f, "  if (rc < 0) return rc;\n");

  for (i = 0; i < fields->size; i++) {
    struct StructField field = fields->fields[i];

    if (strcmp(field.type, "string") == 0) {
      fprintf(f,
              "  {\n"
              "    char *quoted = NULL;\n"
              "    rc = quote_or_null(obj->%s, &quoted);\n"
              "    if (rc != 0) return rc;\n"
              "    rc = fprintf(fh, \"  /* const char * */ \\\"%%s\\\",\\n\", "
              "quoted);\n"
              "    free(quoted);\n"
              "    if (rc < 0) return rc;\n"
              "  }\n",
              field.name);
    } else if (strcmp(field.type, "integer") == 0) {
      fprintf(f,
              "  rc = fprintf(fh, \"  /* int */ " INT_FMT "\\n\", obj->%s);\n",
              field.name);
      fprintf(f, "  if (rc < 0) return rc;\n");
    } else if (strcmp(field.type, "boolean") == 0) {
      fprintf(f,
              "  rc = fprintf(fh, \"  /* int (bool) */ %%d\\n\", obj->%s);\n",
              field.name);
      fprintf(f, "  if (rc < 0) return rc;\n");
    } else if (strcmp(field.type, "number") == 0) {
      fprintf(f, "  rc = fprintf(fh, \"  /* double */ %%f\\n\", obj->%s);\n",
              field.name);
      fprintf(f, "  if (rc < 0) return rc;\n");
    } else if (strcmp(field.type, "enum") == 0) {
      fprintf(f, "  rc = fprintf(fh, \"  /* enum %s */ %%d\\n\", obj->%s);\n",
              field.ref, field.name);
      fprintf(f, "  if (rc < 0) return rc;\n");
    } else if (strcmp(field.type, "object") == 0) {
      fprintf(f,
              "  if (obj->%s) {\n"
              "    rc = %s_debug(obj->%s, fh);\n"
              "    if (rc != 0) return rc;\n"
              "  }\n",
              field.name, field.ref, field.name);
    } else {
      fprintf(f, "  /* debug: unhandled field type %s for %s */\n", field.type,
              field.name);
    }
  }

  fprintf(f, "  rc = fputs(\"};\\n\", fh);\n");
  fprintf(f, "  if (rc < 0) return rc;\n");
  fprintf(f, "  return 0;\n");
  fprintf(f, "}\n\n");
}
