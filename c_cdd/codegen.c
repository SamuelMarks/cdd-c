#include "codegen.h"

#include <string.h>

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
    fprintf(stderr, "write_enum_from_str_func: NULL argument\n");
    return;
  }
  if (!em->members) {
    fprintf(stderr, "write_enum_from_str_func: NULL em->members pointer\n");
    return;
  }

  fprintf(cfile,
          "int %s_from_str(const char *str, enum %s *val) {\n"
          "  if (str == NULL || val == NULL) return -1;\n",
          enum_name, enum_name);

  for (i = 0; i < em->size; i++) {
    if (em->members[i] == NULL)
      continue;

    fprintf(cfile,
            "  if (strcmp(str, \"%s\") == 0) {\n"
            "    *val = %s;\n"
            "    return 0;\n"
            "  }\n",
            em->members[i], em->members[i]);
  }

  fprintf(cfile, "  *val = UNKNOWN;\n"
                 "  return 0;\n"
                 "}\n\n");
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
          "  if (!obj || !json_str) return -1;\n"
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
              name, field->ref + 1 /* strip leading '#/definitions/' if your
                                      $ref has it, adjust if needed */
              ,
              name, name);
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
