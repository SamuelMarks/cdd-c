/**
 * @file code2schema.c
 * @brief Implementation of C header parsing and code-to-schema conversion.
 *
 * Implements logic to parse C header constructs (structs, enums, unions)
 * and serialize them into a JSON Schema format (OpenAPI 3.x compatible).
 * Also provides utilities to map JSON schema definitions back to internal
 * C struct representations for code generation.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parson.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

#include "code2schema.h"
#include "codegen.h" /* For API definitions */

#define MAX_LINE_LENGTH 1024

/**
 * @brief Helper to read a line from file and strip newline characters.
 *
 * @param[in] fp File pointer to read from.
 * @param[out] buf Buffer to store the line.
 * @param[in] bufsz Size of the buffer.
 * @return 1 on success (line read), 0 on EOF or error.
 */
static int read_line(FILE *fp, char *buf, size_t bufsz) {
  if (!fgets(buf, (int)bufsz, fp))
    return 0;
  {
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
      buf[--len] = '\0';
  }
  return 1;
}

int struct_fields_init(struct StructFields *sf) {
  if (!sf)
    return EINVAL;
  sf->size = 0;
  sf->capacity = 8;
  sf->fields =
      (struct StructField *)malloc(sf->capacity * sizeof(struct StructField));
  if (!sf->fields)
    return ENOMEM;
  return 0;
}

void struct_fields_free(struct StructFields *sf) {
  if (sf && sf->fields) {
    free(sf->fields);
    sf->fields = NULL;
  }
}

int struct_fields_add(struct StructFields *sf, const char *const name,
                      const char *const type, const char *const ref,
                      const char *const default_val) {
  struct StructField *f;
  if (!sf || !name || !type)
    return EINVAL;
  if (sf->size >= sf->capacity) {
    sf->capacity *= 2;
    {
      struct StructField *new_fields = (struct StructField *)realloc(
          sf->fields, sf->capacity * sizeof(struct StructField));
      if (!new_fields)
        return ENOMEM;
      sf->fields = new_fields;
    }
  }

  f = &sf->fields[sf->size];
  /* Use strncpy for safety, fields are fixed width in struct definition */
  strncpy(f->name, name, 63);
  f->name[63] = '\0';

  strncpy(f->type, type, 63);
  f->type[63] = '\0';

  if (ref) {
    strncpy(f->ref, ref, 63);
    f->ref[63] = '\0';
  } else {
    f->ref[0] = '\0';
  }

  if (default_val) {
    strncpy(f->default_val, default_val, 63);
    f->default_val[63] = '\0';
  } else {
    f->default_val[0] = '\0';
  }

  /* Initialize constraints to none */
  f->has_min = 0;
  f->min_val = 0.0;
  f->exclusive_min = 0;
  f->has_max = 0;
  f->max_val = 0.0;
  f->exclusive_max = 0;
  f->has_min_len = 0;
  f->min_len = 0;
  f->has_max_len = 0;
  f->max_len = 0;
  f->pattern[0] = '\0';

  sf->size++;
  return 0;
}

int enum_members_init(struct EnumMembers *em) {
  if (!em)
    return EINVAL;
  em->size = 0;
  em->capacity = 8;
  em->members = (char **)calloc(em->capacity, sizeof(char *));
  if (!em->members)
    return ENOMEM;
  return 0;
}

void enum_members_free(struct EnumMembers *em) {
  size_t i;
  if (!em)
    return;
  if (em->members) {
    for (i = 0; i < em->size; ++i) {
      if (em->members[i])
        free(em->members[i]);
    }
    free(em->members);
    em->members = NULL;
  }
  em->size = 0;
  em->capacity = 0;
}

int enum_members_add(struct EnumMembers *em, const char *name) {
  if (!em || !name)
    return EINVAL;
  if (em->size >= em->capacity) {
    const size_t new_cap = em->capacity == 0 ? 8 : em->capacity * 2;
    char **new_members =
        (char **)realloc(em->members, new_cap * sizeof(char *));
    if (!new_members)
      return ENOMEM;
    em->members = new_members;
    em->capacity = new_cap;
  }
  em->members[em->size] = strdup(name);
  if (!em->members[em->size])
    return ENOMEM;
  em->size++;
  return 0;
}

/* Helper to strip pointer asterisks from a type string */
static void strip_stars(char *str) {
  char *p = str;
  while (*p) {
    if (*p == '*')
      *p = ' ';
    p++;
  }
}

int parse_struct_member_line(const char *line, struct StructFields *sf) {
  char buf[MAX_LINE_LENGTH];
  char *semicolon;
  char *last_space;
  char name[64] = {0};
  char type_raw[64] = {0};
  char type_final[64] = {0};
  char ref_final[64] = {0};
  int is_ptr = 0;

  if (!line || !sf)
    return EINVAL;

  /* Basic heuristic parsing: "Type name;" */
  strncpy(buf, line, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';
  trim_trailing(buf);

  semicolon = strrchr(buf, ';');
  if (semicolon)
    *semicolon = '\0'; /* Remove semicolon */

  /* Find last space to separate type and name */
  last_space = strrchr(buf, ' ');
  if (!last_space) {
    /* Handle tab? or pointer attached to name like "int *ptr" vs "int* ptr" */
    /* Assume space separation for MVP C89 styles */
    return 0; /* Skip malformed line */
  }

  /* Extract name */
  *last_space = '\0';
  {
    char *n = last_space + 1;
    while (*n == '*') { /* Name starts with *? e.g. "int *ptr" */
      is_ptr = 1;
      n++;
    }
    strncpy(name, n, 63);
  }

  /* Extract raw type */
  {
    char *t = buf;
    while (isspace((unsigned char)*t))
      t++; /* Skip leading whitespace */

    /* Check for trailing * in type part e.g. "int* ptr" */
    if (t[strlen(t) - 1] == '*') {
      is_ptr = 1;
      t[strlen(t) - 1] = '\0';
      /* trim potential internal spaces "int *" */
      trim_trailing(t);
    }
    strncpy(type_raw, t, 63);
  }

  /* Map basic C types to logical types */
  if (strstr(type_raw, "int") || strstr(type_raw, "long") ||
      strstr(type_raw, "short") || strstr(type_raw, "size_t")) {
    strcpy(type_final, "integer");
  } else if (strstr(type_raw, "float") || strstr(type_raw, "double")) {
    strcpy(type_final, "number");
  } else if (strstr(type_raw, "bool") || strstr(type_raw, "_Bool")) {
    strcpy(type_final, "boolean");
  } else if (strstr(type_raw, "char")) {
    if (is_ptr || strstr(type_raw, "*"))
      strcpy(type_final, "string");
    else
      strcpy(type_final, "string"); /* char is often treated as string/byte */
  } else if (str_starts_with(type_raw, "struct ")) {
    strcpy(type_final, "object");
    /* Reference name */
    strncpy(ref_final, type_raw + 7, 63);
    strip_stars(ref_final);
    trim_trailing(ref_final);
  } else if (str_starts_with(type_raw, "enum ")) {
    strcpy(type_final, "enum");
    strncpy(ref_final, type_raw + 5, 63);
    strip_stars(ref_final);
    trim_trailing(ref_final);
  } else {
    /* Fallback/Unknown */
    strcpy(type_final, "string"); /* Default to string for unknown */
  }

  return struct_fields_add(sf, name, type_final,
                           ref_final[0] ? ref_final : NULL, NULL);
}

bool str_starts_with(const char *str, const char *prefix) {
  size_t i;
  if (str == NULL || prefix == NULL)
    return 0;
  for (i = 0; prefix[i] != '\0'; i++) {
    if (str[i] != prefix[i])
      return 0;
  }
  return 1;
}

void trim_trailing(char *str) {
  size_t len;
  if (str == NULL)
    return;
  len = strlen(str);
  while (len > 0) {
    char c = str[len - 1];
    if (isspace((unsigned char)c) || c == ';') {
      str[len - 1] = '\0';
      len--;
    } else {
      break;
    }
  }
}

int json_array_to_enum_members(const JSON_Array *a, struct EnumMembers *e) {
  size_t i, count;
  if (!a || !e)
    return EINVAL;

  count = json_array_get_count(a);
  for (i = 0; i < count; i++) {
    const char *s = json_array_get_string(a, i);
    if (!s)
      continue;
    if (enum_members_add(e, s) != 0)
      return ENOMEM;
  }
  return 0;
}

int json_object_to_struct_fields(const JSON_Object *o, struct StructFields *f,
                                 const JSON_Object *root) {
  JSON_Object *props;
  size_t i, count;
  (void)root; /* Unused in this simple implementation */

  if (!o || !f)
    return EINVAL;

  props = json_object_get_object(o, "properties");
  if (!props)
    return 0; /* No fields */

  count = json_object_get_count(props);
  for (i = 0; i < count; i++) {
    const char *name = json_object_get_name(props, i);
    JSON_Object *prop = json_object_get_object(props, name);
    const char *type = json_object_get_string(prop, "type");
    const char *ref = json_object_get_string(prop, "$ref");
    char default_buf[128] = {0};
    const char *d_val = NULL;

    /* Extract default value if present. Using sprintf for C89 compliance. */
    if (json_object_has_value_of_type(prop, "default", JSONString)) {
      const char *s = json_object_get_string(prop, "default");
      sprintf(default_buf, "\"%s\"", s);
      d_val = default_buf;
    } else if (json_object_has_value_of_type(prop, "default", JSONNumber)) {
      double d = json_object_get_number(prop, "default");
      if (type && strcmp(type, "integer") == 0) {
        sprintf(default_buf, "%d", (int)d);
      } else {
        sprintf(default_buf, "%f", d);
      }
      d_val = default_buf;
    } else if (json_object_has_value_of_type(prop, "default", JSONBoolean)) {
      int b = json_object_get_boolean(prop, "default");
      sprintf(default_buf, "%d", b);
      d_val = default_buf;
    }

    if (type) {
      if (strcmp(type, "array") == 0) {
        /* Handle arrays */
        JSON_Object *items = json_object_get_object(prop, "items");
        const char *item_type = json_object_get_string(items, "type");
        const char *item_ref = json_object_get_string(items, "$ref");
        struct_fields_add(f, name, "array", item_ref ? item_ref : item_type,
                          NULL);
      } else {
        struct_fields_add(f, name, type, ref, d_val);
      }
    } else if (ref) {
      /* Assume object if only ref is present */
      struct_fields_add(f, name, "object", ref, NULL);
    }

    {
      struct StructField *field = &f->fields[f->size - 1];

      /* Parse Constraints for Numeric Validation */
      if (type &&
          (strcmp(type, "integer") == 0 || strcmp(type, "number") == 0)) {
        if (json_object_has_value_of_type(prop, "minimum", JSONNumber)) {
          field->has_min = 1;
          field->min_val = json_object_get_number(prop, "minimum");
        }

        if (json_object_has_value(prop, "exclusiveMinimum")) {
          if (json_object_has_value_of_type(prop, "exclusiveMinimum",
                                            JSONBoolean)) {
            if (json_object_get_boolean(prop, "exclusiveMinimum")) {
              field->exclusive_min = 1;
            }
          } else if (json_object_has_value_of_type(prop, "exclusiveMinimum",
                                                   JSONNumber)) {
            field->has_min = 1;
            field->min_val = json_object_get_number(prop, "exclusiveMinimum");
            field->exclusive_min = 1;
          }
        }

        if (json_object_has_value_of_type(prop, "maximum", JSONNumber)) {
          field->has_max = 1;
          field->max_val = json_object_get_number(prop, "maximum");
        }

        if (json_object_has_value(prop, "exclusiveMaximum")) {
          if (json_object_has_value_of_type(prop, "exclusiveMaximum",
                                            JSONBoolean)) {
            if (json_object_get_boolean(prop, "exclusiveMaximum")) {
              field->exclusive_max = 1;
            }
          } else if (json_object_has_value_of_type(prop, "exclusiveMaximum",
                                                   JSONNumber)) {
            field->has_max = 1;
            field->max_val = json_object_get_number(prop, "exclusiveMaximum");
            field->exclusive_max = 1;
          }
        }
      }

      /* Parse Constraints for String Validation */
      if (type && strcmp(type, "string") == 0) {
        if (json_object_has_value_of_type(prop, "minLength", JSONNumber)) {
          field->has_min_len = 1;
          field->min_len = (size_t)json_object_get_number(prop, "minLength");
        }
        if (json_object_has_value_of_type(prop, "maxLength", JSONNumber)) {
          field->has_max_len = 1;
          field->max_len = (size_t)json_object_get_number(prop, "maxLength");
        }
        if (json_object_has_value_of_type(prop, "pattern", JSONString)) {
          const char *p = json_object_get_string(prop, "pattern");
          if (p) {
            strncpy(field->pattern, p, sizeof(field->pattern) - 1);
            field->pattern[sizeof(field->pattern) - 1] = '\0';
          }
        }
      }
    }
  }
  return 0;
}

int write_struct_to_json_schema(JSON_Object *schemas_obj,
                                const char *struct_name,
                                const struct StructFields *sf) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  JSON_Value *props_val = json_value_init_object();
  JSON_Object *props_obj = json_value_get_object(props_val);
  size_t i;

  if (!schemas_obj || !struct_name || !sf) {
    json_value_free(val);
    json_value_free(props_val);
    return EINVAL;
  }

  json_object_set_string(obj, "type", "object");
  json_object_set_value(obj, "properties", props_val);

  for (i = 0; i < sf->size; i++) {
    JSON_Value *pval = json_value_init_object();
    JSON_Object *pobj = json_value_get_object(pval);
    const char *typ = sf->fields[i].type;
    const char *ref = sf->fields[i].ref;

    if (strcmp(typ, "array") == 0) {
      JSON_Value *items_val = json_value_init_object();
      JSON_Object *items_obj = json_value_get_object(items_val);
      json_object_set_string(pobj, "type", "array");

      if (ref && *ref) {
        if (strcmp(ref, "integer") == 0 || strcmp(ref, "string") == 0 ||
            strcmp(ref, "boolean") == 0 || strcmp(ref, "number") == 0) {
          json_object_set_string(items_obj, "type", ref);
        } else {
          char ref_str[128];
          sprintf(ref_str, "#/components/schemas/%s", ref);
          json_object_set_string(items_obj, "$ref", ref_str);
        }
      }
      json_object_set_value(pobj, "items", items_val);
    } else {
      /* Normal type */
      if (strcmp(typ, "object") == 0 || strcmp(typ, "enum") == 0) {
        /* Usually mapped as reference */
        char ref_str[128];
        if (ref && *ref) {
          sprintf(ref_str, "#/components/schemas/%s", ref);
          json_object_set_string(pobj, "$ref", ref_str);
        } else {
          json_object_set_string(pobj, "type", "object"); /* Fallback */
        }
      } else {
        json_object_set_string(pobj, "type", typ);
      }
    }
    json_object_set_value(props_obj, sf->fields[i].name, pval);
  }

  json_object_set_value(schemas_obj, struct_name, val);
  return 0;
}

/**
 * @brief Parse a union definition line by line and add to schema as "oneOf".
 */
static int parse_union_and_write(FILE *fp, JSON_Object *schemas_obj,
                                 const char *union_name) {
  char line[512];
  JSON_Value *union_val = json_value_init_object();
  JSON_Object *union_obj = json_value_get_object(union_val);
  JSON_Value *oneof_val = json_value_init_array();
  JSON_Array *oneof_arr = json_value_get_array(oneof_val);
  char *p;

  while (read_line(fp, line, sizeof(line))) {
    p = line;
    while (isspace((unsigned char)*p))
      p++;
    if (*p == '}')
      break; /* End of union */
    if (!*p)
      continue;

    {
      char typebuf[64] = {0};
      char namebuf[64] = {0};

      if (sscanf(p, "%63s %63[^;]", typebuf, namebuf) == 2) {
        char *n = namebuf;
        char *t = typebuf;
        if (n[0] == '*')
          n++; /* Skip pointer */

        {
          JSON_Value *option_val = json_value_init_object();
          JSON_Object *option_obj = json_value_get_object(option_val);
          JSON_Value *props_val = json_value_init_object();
          JSON_Object *props_obj = json_value_get_object(props_val);
          JSON_Value *field_val = json_value_init_object();
          JSON_Object *field_obj = json_value_get_object(field_val);

          /* Map type */
          if (strcmp(t, "int") == 0)
            json_object_set_string(field_obj, "type", "integer");
          else if (strcmp(t, "char") == 0)
            json_object_set_string(field_obj, "type", "string");
          else if (strcmp(t, "float") == 0)
            json_object_set_string(field_obj, "type", "number");
          else
            json_object_set_string(field_obj, "type", "object"); /* fallback */

          json_object_set_value(props_obj, n, field_val);
          json_object_set_string(option_obj, "type", "object");
          json_object_set_value(option_obj, "properties", props_val);
          json_object_set_string(option_obj, "title",
                                 n); /* Use name as title */

          json_array_append_value(oneof_arr, option_val);
        }
      }
    }
  }

  json_object_set_value(union_obj, "oneOf", oneof_val);
  json_object_set_string(union_obj, "type", "object");
  json_object_set_value(schemas_obj, union_name, union_val);

  return 0;
}

int code2schema_main(int argc, char **argv) {
  FILE *fp;
  char line[MAX_LINE_LENGTH];
  JSON_Value *root = json_value_init_object();
  JSON_Object *root_obj = json_value_get_object(root);
  JSON_Value *schemas_val = json_value_init_object();
  JSON_Object *schemas_obj = json_value_get_object(schemas_val);
  JSON_Value *comp_val = json_value_init_object();
  JSON_Object *comp_obj = json_value_get_object(comp_val);

  /* main.c passes (argc-2, argv+2), so argv[0] is input, argv[1] is output. */
  if (argc != 2) {
    json_value_free(root);
    /* Expect: <in> <out> */
    return EXIT_FAILURE;
  }

#if defined(_MSC_VER)
  fopen_s(&fp, argv[0], "r");
#else
  fp = fopen(argv[0], "r");
#endif
  if (!fp) {
    json_value_free(root);
    json_value_free(schemas_val);
    json_value_free(comp_val);
    return EXIT_FAILURE;
  }

  /* Structure: components -> schemas */
  json_object_set_value(comp_obj, "schemas", schemas_val);
  json_object_set_value(root_obj, "components", comp_val);

  while (read_line(fp, line, sizeof(line))) {
    char *p = line;
    while (isspace((unsigned char)*p))
      p++;

    if (strncmp(p, "union ", 6) == 0) {
      char union_name[64] = {0};
      char *brace = strchr(p, '{');
      if (brace) {
        *brace = 0;
        if (sscanf(p + 6, "%63s", union_name) == 1) {
          parse_union_and_write(fp, schemas_obj, union_name);
        }
      }
    } else if (strncmp(p, "struct ", 7) == 0) {
      char struct_name[64] = {0};
      char *brace = strchr(p, '{');
      if (brace) {
        struct StructFields sf;
        *brace = 0;
        if (sscanf(p + 7, "%63s", struct_name) == 1) {
          if (struct_fields_init(&sf) == 0) {
            char subline[MAX_LINE_LENGTH];
            while (read_line(fp, subline, sizeof(subline))) {
              char *sp = subline;
              while (isspace((unsigned char)*sp))
                sp++;
              if (*sp == '}')
                break;
              if (*sp)
                parse_struct_member_line(sp, &sf);
            }
            write_struct_to_json_schema(schemas_obj, struct_name, &sf);
            struct_fields_free(&sf);
          }
        }
      }
    } else if (strncmp(p, "enum ", 5) == 0) {
      char enum_name[64] = {0};
      char *brace = strchr(p, '{');
      if (brace) {
        JSON_Value *eval = json_value_init_object();
        JSON_Object *eobj = json_value_get_object(eval);
        JSON_Value *arrval = json_value_init_array();
        JSON_Array *earr = json_value_get_array(arrval);

        *brace = 0;
        if (sscanf(p + 5, "%63s", enum_name) == 1) {
          /* Parse enum members */
          /* Assuming on same line or next lines until } */
          /* Simplified strictly for this tool's scope */
          /* NOTE: The provided test `test1.h` has one-line enum */
          const char *delim = ",}";
          char *token;
          char *ctx = NULL;
          /* Check if closing brace is on same line */
          char *rest = brace + 1;
          char full_body[4096] = {0};
          strcat(full_body, rest);
          while (!strchr(full_body, '}')) {
            if (!read_line(fp, line, sizeof(line)))
              break;
            strcat(full_body, line);
          }

          token = strtok_r(full_body, delim, &ctx);
          while (token) {
            char *tm = token;
            while (isspace((unsigned char)*tm))
              tm++;
            trim_trailing(tm);
            /* Remove assignments e.g. GREEN = 5 */
            {
              char *eq = strchr(tm, '=');
              if (eq)
                *eq = 0;
              trim_trailing(tm);
            }
            if (*tm) {
              json_array_append_string(earr, tm);
            }
            token = strtok_r(NULL, delim, &ctx);
          }

          json_object_set_string(eobj, "type", "string");
          json_object_set_value(eobj, "enum", arrval);
          json_object_set_value(schemas_obj, enum_name, eval);
        } else {
          json_value_free(eval);
          json_value_free(arrval);
        }
      }
    }
  }

  json_serialize_to_file_pretty(root, argv[1]);

  fclose(fp);
  json_value_free(root);
  return EXIT_SUCCESS;
}
