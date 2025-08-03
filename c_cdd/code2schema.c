/*
 * code2schema.c
 *
 * Converts C header file back to JSON Schema.
 *
 * Minimal parser for enums and structs that follow the conventions.
 *
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
#endif /* !strdup */
#define strtok_r strtok_s
#endif

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#include "code2schema.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(strdup)
#define strdup _strdup
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(strdup)  \
        */

/* Trim trailing whitespace and semicolons */
void trim_trailing(char *const str) {
  size_t len;
  len = strlen(str);
  while (len > 0 &&
         (str[len - 1] == ';' || isspace((unsigned char)str[len - 1])))
    str[--len] = '\0';
}

/* Check if string starts with prefix */
bool str_starts_with(const char *str, const char *prefix) {
  while (*prefix)
    if (*str++ != *prefix++)
      return false;
  return true;
}

/* Read a line safely */
static int read_line(FILE *fp, char *buf, size_t bufsz) {
  if (!fgets(buf, (int)bufsz, fp))
    return 0;
  /* Strip newline */
  {
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
      buf[--len] = '\0';
  }
  return 1;
}

void enum_members_init(struct EnumMembers *em) {
  em->size = 0;
  em->capacity = 8;
  em->members = (char **)malloc(em->capacity * sizeof(char *));
  if (!em->members) {
    fputs("malloc failed", stderr);
    exit(1);
  }
}

void enum_members_free(struct EnumMembers *em) {
  size_t i;
  for (i = 0; i < em->size; i++)
    free(em->members[i]);
  free(em->members);
  em->members = NULL;
  em->size = 0;
  em->capacity = 0;
}

void enum_members_add(struct EnumMembers *em, const char *const name) {
  if (em->size >= em->capacity) {
    em->capacity *= 2;
    em->members = realloc(em->members, em->capacity * sizeof(char *));
    if (em->members == NULL) {
      fputs("malloc failed", stderr);
      exit(1);
    }
  }
  em->members[em->size++] = strdup(name);
}

void struct_fields_init(struct StructFields *sf) {
  sf->size = 0;
  sf->capacity = 8;
  sf->fields =
      (struct StructField *)malloc(sf->capacity * sizeof(struct StructField));
  if (!sf->fields) {
    fputs("malloc failed\n", stderr);
    exit(1);
  }
}

void struct_fields_free(struct StructFields *sf) {
  if (sf->fields) {
    free(sf->fields);
    sf->fields = NULL;
  }
  sf->size = 0;
  sf->capacity = 0;
}

void struct_fields_add(struct StructFields *sf, const char *const name,
                       const char *const type, const char *const ref) {
  if (sf->size >= sf->capacity) {
    sf->capacity *= 2;
    sf->fields = (struct StructField *)realloc(
        sf->fields, sf->capacity * sizeof(struct StructField));
    if (!sf->fields) {
      fputs("malloc failed", stderr);
      exit(1);
    }
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strncpy_s(sf->fields[sf->size].name, sizeof(sf->fields[sf->size].name), name,
            _TRUNCATE);
#else
  strncpy(sf->fields[sf->size].name, name, sizeof(sf->fields[sf->size].name));
  sf->fields[sf->size].name[sizeof(sf->fields[sf->size].name) - 1] = '\0';
#endif

  sf->fields[sf->size].name[sizeof(sf->fields[sf->size].name) - 1] = 0;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strncpy_s(sf->fields[sf->size].type, sizeof(sf->fields[sf->size].type), type,
            _TRUNCATE);
#else
  strncpy(sf->fields[sf->size].type, type, sizeof(sf->fields[sf->size].type));
  sf->fields[sf->size].type[sizeof(sf->fields[sf->size].type) - 1] = '\0';
#endif

  sf->fields[sf->size].type[sizeof(sf->fields[sf->size].type) - 1] = 0;
  if (ref) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    strncpy_s(sf->fields[sf->size].ref, sizeof(sf->fields[sf->size].ref), ref,
              _TRUNCATE);
#else
    strncpy(sf->fields[sf->size].ref, ref, sizeof(sf->fields[sf->size].ref));
    sf->fields[sf->size].ref[sizeof(sf->fields[sf->size].ref) - 1] = '\0';
#endif
  } else
    sf->fields[sf->size].ref[0] = 0;
  sf->fields[sf->size].ref[sizeof(sf->fields[sf->size].ref) - 1] = 0;
  sf->size++;
}

/*
 * Parse struct member line to fill struct_fields_add()
 * Expects line to be like:
 *   "const char *foo;"
 *   "int bar;"
 *   "double x;"
 *   "struct SomeType *buz;"
 */
int parse_struct_member_line(const char *line, struct StructFields *sf) {
  char namebuf[64];
  int matched;
  const char *p;
  const char *name_start, *name_end;
  size_t len;
  const char *type_start, *type_end;
  char type_buf[64], name_buf[64];
  size_t copy_len;

  while (isspace((unsigned char)*line)) {
    line++;
  }

  /* Must appear before `const char` */
  if (strncmp(line, "const ", 6) != 0 && strncmp(line, "char", 4) == 0) {
    return 0;
  }

  /* Try parse "const char *name;" manually */
  if (strncmp(line, "const char", 10) == 0) {
    p = line + 10;
    while (*p && isspace((unsigned char)*p))
      p++;
    if (*p == '*') {
      p++;
      while (*p && isspace((unsigned char)*p))
        p++;
      name_start = p;
      while (*p && !isspace((unsigned char)*p) && *p != ';')
        p++;
      name_end = p;
      if (name_end > name_start) {
        len = name_end - name_start;
        if (len < sizeof(namebuf)) {
          memcpy(namebuf, name_start, len);
          namebuf[len] = '\0';
          struct_fields_add(sf, namebuf, "string", NULL);
          return 1;
        }
      }
    }
  }

  /* int used for bool in your code, treat as boolean */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  matched =
      sscanf_s(line, "bool %63[^; \t]", namebuf, (unsigned int)sizeof(namebuf));
#else
  matched = sscanf(line, "bool %63[^; \t]", namebuf);
#endif
  if (matched == 1) {
    struct_fields_add(sf, namebuf, "boolean", NULL);
    return 1;
  }

  /* int name; or double name; */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  matched =
      sscanf_s(line, "int %63[^; \t]", namebuf, (unsigned int)sizeof(namebuf));
#else
  matched = sscanf(line, "int %63[^; \t]", namebuf);
#endif
  if (matched == 1) {
    struct_fields_add(sf, namebuf, "integer", NULL);
    return 1;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  matched = sscanf_s(line, "double %63[^; \t]", namebuf,
                     (unsigned int)sizeof(namebuf));
#else
  matched = sscanf(line, "double %63[^; \t]", namebuf);
#endif
  if (matched == 1) {
    struct_fields_add(sf, namebuf, "number", NULL);
    return 1;
  }
  /* enum pointer: handles "enum Type * name;" and "enum Type*name;" */
  if (strncmp(line, "enum ", 5) == 0) {
    p = line + 5;
    while (*p && isspace((unsigned char)*p))
      p++;
    type_start = p;
    while (*p && !isspace((unsigned char)*p) && *p != '*')
      p++;
    type_end = p;

    if (type_end > type_start) {
      len = type_end - type_start;
      copy_len = len < sizeof(type_buf) - 1 ? len : sizeof(type_buf) - 1;
      memcpy(type_buf, type_start, copy_len);
      type_buf[copy_len] = '\0';

      while (*p && isspace((unsigned char)*p))
        p++;
      if (*p == '*') { /* It's a pointer */
        p++;
        while (*p && isspace((unsigned char)*p))
          p++;

        name_start = p;
        name_end = name_start;
        while (*name_end && *name_end != ';' && *name_end != '[' &&
               !isspace((unsigned char)*name_end))
          name_end++;

        if (name_end > name_start) {
          len = name_end - name_start;
          copy_len = len < sizeof(name_buf) - 1 ? len : sizeof(name_buf) - 1;
          memcpy(name_buf, name_start, copy_len);
          name_buf[copy_len] = '\0';
          struct_fields_add(sf, name_buf, "enum", type_buf);
          return 1;
        }
      }
    }
  }

  /* struct pointer: handles "struct Type * name;" and "struct Type*name;" */
  if (strncmp(line, "struct ", 7) == 0) {
    p = line + 7;
    while (*p && isspace((unsigned char)*p))
      p++;
    type_start = p;
    while (*p && !isspace((unsigned char)*p) && *p != '*')
      p++;
    type_end = p;

    if (type_end > type_start) {
      len = type_end - type_start;
      copy_len = len < sizeof(type_buf) - 1 ? len : sizeof(type_buf) - 1;
      memcpy(type_buf, type_start, copy_len);
      type_buf[copy_len] = '\0';

      while (*p && isspace((unsigned char)*p))
        p++;
      if (*p == '*') { /* It's a pointer */
        p++;
        while (*p && isspace((unsigned char)*p))
          p++;

        name_start = p;
        name_end = name_start;
        while (*name_end && *name_end != ';' && *name_end != '[' &&
               !isspace((unsigned char)*name_end))
          name_end++;

        if (name_end > name_start) {
          len = name_end - name_start;
          copy_len = len < sizeof(name_buf) - 1 ? len : sizeof(name_buf) - 1;
          memcpy(name_buf, name_start, copy_len);
          name_buf[copy_len] = '\0';
          struct_fields_add(sf, name_buf, "object", type_buf);
          return 1;
        }
      }
    }
  }

  /* Fallback: ignore or unrecognized */
  return 0;
}

static void extract_name(char *dest, size_t dest_sz, const char *start,
                         const char *end) {
  const char *name_start;
  const char *name_end;
  name_start = start;
  while (isspace((unsigned char)*name_start))
    name_start++;
  name_end = name_start;
  while (name_end < end && !isspace((unsigned char)*name_end))
    name_end++;

  if (name_end > name_start && (size_t)(name_end - name_start) < dest_sz) {
    memcpy(dest, name_start, name_end - name_start);
    dest[name_end - name_start] = '\0';
  } else {
    dest[0] = '\0'; /* Anonymous */
  }
}

/*
 * Write JSON schema for enum to schemas_obj
 */
static void write_enum_to_json_schema(JSON_Object *schemas_obj,
                                      const char *enum_name,
                                      struct EnumMembers *em) {
  JSON_Value *enum_val = json_value_init_object();
  JSON_Object *enum_obj = json_value_get_object(enum_val);
  JSON_Value *enum_arr_val = json_value_init_array();
  JSON_Array *enum_arr = json_value_get_array(enum_arr_val);
  size_t i;

  json_object_set_string(enum_obj, "type", "string");

  for (i = 0; i < em->size; i++)
    json_array_append_string(enum_arr, em->members[i]);

  json_object_set_value(enum_obj, "enum", enum_arr_val);

  json_object_set_value(schemas_obj, enum_name, enum_val);
}

/*
 * Write JSON schema for struct to schemas_obj
 */
void write_struct_to_json_schema(JSON_Object *schemas_obj,
                                 const char *struct_name,
                                 struct StructFields *sf) {
  JSON_Value *struct_val = json_value_init_object();
  JSON_Object *struct_obj = json_value_get_object(struct_val);
  JSON_Value *props_val = json_value_init_object();
  JSON_Object *props_obj = json_value_get_object(props_val);
  JSON_Value *required_val = json_value_init_array();
  JSON_Array *required_arr = json_value_get_array(required_val);
  size_t i;

  json_object_set_string(struct_obj, "type", "object");
  json_object_set_value(struct_obj, "properties", props_val);
  json_object_set_value(struct_obj, "required", required_val);

  /* Add each field as property */
  for (i = 0; i < sf->size; i++) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    JSON_Value *field_val = json_value_init_object();
    JSON_Object *field_obj = json_value_get_object(field_val);

    if (strcmp(type, "string") == 0) {
      json_object_set_string(field_obj, "type", "string");
    } else if (strcmp(type, "integer") == 0) {
      json_object_set_string(field_obj, "type", "integer");
    } else if (strcmp(type, "boolean") == 0) {
      json_object_set_string(field_obj, "type", "boolean");
    } else if (strcmp(type, "number") == 0) {
      json_object_set_string(field_obj, "type", "number");
    } else if (strcmp(type, "object") == 0) {
      char ref_buf[128];
      snprintf(ref_buf, sizeof(ref_buf), "#/components/schemas/%s",
               sf->fields[i].ref);
      json_object_set_string(field_obj, "$ref", ref_buf);
    } else if (strcmp(type, "enum") == 0) {
      char ref_buf[128];
      snprintf(ref_buf, sizeof(ref_buf), "#/components/schemas/%s",
               sf->fields[i].ref);
      json_object_set_string(field_obj, "$ref", ref_buf);
    }

    json_object_set_value(props_obj, name, field_val);
    json_array_append_string(required_arr, name);
  }
  json_object_set_value(schemas_obj, struct_name, struct_val);
}

enum ParseState { P_NONE, P_IN_ENUM, P_IN_STRUCT };

/*
 * Parse header file line recognizing enums and structs, then build JSON schema.
 */
static int parse_header_file(const char *header_filename,
                             const char *schema_out) {
  FILE *fp;
  char line[512];

  enum ParseState state = P_NONE;

  char enum_name[64];
  struct EnumMembers em;
  char struct_name[64];
  struct StructFields sf;

  JSON_Value *root_val;
  JSON_Object *root_obj;
  JSON_Value *components_val;
  JSON_Object *components_obj;
  JSON_Value *schemas_val;
  JSON_Object *schemas_obj;

  char *p;
  char *brace;
  char *semi;
  char *end_brace;
  char *body_to_parse;
  size_t len;
  char *context = NULL;
  char *token;
  char *member_p;
  char *eq;
  char *field_p;
  JSON_Value *final_val;

  memset(enum_name, 0, sizeof(enum_name));
  memset(struct_name, 0, sizeof(struct_name));

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, header_filename, "r");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open header file %s\n", header_filename);
      return -1;
    }
  }
#else
  fp = fopen(header_filename, "r");
  if (!fp) {
    fprintf(stderr, "Failed to open header file %s\n", header_filename);
    return -1;
  }
#endif

  root_val = json_value_init_object();
  root_obj = json_value_get_object(root_val);
  components_val = json_value_init_object();
  components_obj = json_value_get_object(components_val);
  schemas_val = json_value_init_object();
  schemas_obj = json_value_get_object(schemas_val);

  json_object_set_value(root_obj, "components", components_val);
  json_object_set_value(components_obj, "schemas", schemas_val);

  enum_members_init(&em);
  struct_fields_init(&sf);

  while (read_line(fp, line, sizeof(line))) {
    p = line;
  process_line:
    while (isspace((unsigned char)*p))
      p++;
    if (!*p)
      continue;

    if (state == P_NONE) {
      if (str_starts_with(p, "enum ")) {
        brace = strchr(p, '{');
        if (brace) {
          extract_name(enum_name, sizeof(enum_name), p + 5, brace);
          enum_members_free(&em);
          enum_members_init(&em);
          state = P_IN_ENUM;
          p = brace + 1;
          goto process_line;
        } else {
          semi = strchr(p, ';');
          if (semi) {
            p = semi + 1;
            goto process_line;
          } else {
            /* Unhandled, maybe just keyword, go to next line */
            goto next_line;
          }
        }
      } else if (str_starts_with(p, "struct ")) {
        brace = strchr(p, '{');
        if (brace) {
          extract_name(struct_name, sizeof(struct_name), p + 7, brace);
          struct_fields_free(&sf);
          struct_fields_init(&sf);
          state = P_IN_STRUCT;
          p = brace + 1;
          goto process_line;
        } else {
          semi = strchr(p, ';');
          if (semi) {
            p = semi + 1;
            goto process_line;
          } else {
            /* Unhandled, maybe just keyword, go to next line */
            goto next_line;
          }
        }
      } else {
        continue;
      }
    }

    if (state == P_IN_ENUM) {
      end_brace = strchr(p, '}');

      if (end_brace) {
        len = end_brace - p;
        body_to_parse = (char *)malloc(len + 1);
        if (!body_to_parse) {
          state = P_NONE;
          continue;
        }
        memcpy(body_to_parse, p, len);
        body_to_parse[len] = '\0';
      } else {
        body_to_parse = strdup(p);
        if (!body_to_parse) {
          state = P_NONE;
          continue;
        }
      }

      context = NULL;
      token = strtok_r(body_to_parse, ",", &context);
      while (token) {
        member_p = token;
        while (isspace((unsigned char)*member_p))
          member_p++;
        trim_trailing(member_p);
        if (strlen(member_p) > 0) {
          eq = strchr(member_p, '=');
          if (eq)
            *eq = '\0';
          trim_trailing(member_p);
          if (strlen(member_p) > 0)
            enum_members_add(&em, member_p);
        }
        token = strtok_r(NULL, ",", &context);
      }
      free(body_to_parse);

      if (end_brace) {
        if (strlen(enum_name) > 0)
          write_enum_to_json_schema(schemas_obj, enum_name, &em);
        state = P_NONE;
        p = end_brace + 1;
        while (*p && (isspace((unsigned char)*p) || *p == ';'))
          p++;
        if (*p)
          goto process_line;
      }
    } else if (state == P_IN_STRUCT) {
      end_brace = strchr(p, '}');
      if (end_brace) {
        len = end_brace - p;
        body_to_parse = (char *)malloc(len + 1);
        if (!body_to_parse) {
          state = P_NONE;
          continue;
        }
        memcpy(body_to_parse, p, len);
        body_to_parse[len] = '\0';
      } else {
        body_to_parse = strdup(p);
        if (!body_to_parse) {
          state = P_NONE;
          continue;
        }
      }

      context = NULL;
      token = strtok_r(body_to_parse, ";", &context);
      while (token) {
        field_p = token;
        while (isspace((unsigned char)*field_p))
          field_p++;
        if (strlen(field_p) > 0)
          parse_struct_member_line(field_p, &sf);
        token = strtok_r(NULL, ";", &context);
      }
      free(body_to_parse);
      if (end_brace) {
        if (strlen(struct_name) > 0)
          write_struct_to_json_schema(schemas_obj, struct_name, &sf);
        state = P_NONE;
        p = end_brace + 1;
        while (*p && (isspace((unsigned char)*p) || *p == ';'))
          p++;
        if (*p)
          goto process_line;
      }
    }
  next_line:;
  }
  fclose(fp);

  /* Write JSON to output path */
  final_val = root_val;
  if (json_serialize_to_file_pretty(final_val, schema_out) != JSONSuccess) {
    fprintf(stderr, "Failed to write JSON to %s\n", schema_out);
    enum_members_free(&em);
    struct_fields_free(&sf);
    json_value_free(root_val);
    return -1;
  }

  enum_members_free(&em);
  struct_fields_free(&sf);
  json_value_free(root_val);

  printf("Wrote JSON schema to %s\n", schema_out);
  return 0;
}

int json_array_to_enum_members(const JSON_Array *enum_arr,
                               struct EnumMembers *em) {
  size_t n, i;
  const char *member;
  if (!enum_arr || !em)
    return -1;

  enum_members_init(em);

  n = json_array_get_count(enum_arr);
  for (i = 0; i < n; i++) {
    member = json_array_get_string(enum_arr, i);
    if (member)
      enum_members_add(em, member);
  }
  return 0;
}

int json_object_to_struct_fields(const JSON_Object *schema_obj,
                                 struct StructFields *fields,
                                 const JSON_Object *schemas_obj_root) {
  const char *ref_str;
  const JSON_Object *properties_obj;
  size_t count, i;
  struct StructField *field;
  const char *key;
  const JSON_Value *val;
  const JSON_Object *prop_obj;
  const char *type_str;
  size_t new_cap;
  struct StructField *new_fields;
  const char *ref_name;
  const JSON_Object *ref_schema;

  if (!schema_obj || !fields)
    return -1;

  /* Clear fields struct */
  fields->capacity = 0;
  fields->size = 0;
  fields->fields = NULL;

  properties_obj = json_object_get_object(schema_obj, "properties");

  if (properties_obj == NULL) {
    /* Valid for an object to have no properties. */
    return 0;
  }

  count = json_object_get_count(properties_obj);
  for (i = 0; i < count; i++) {
    key = json_object_get_name(properties_obj, i);
    val = json_object_get_value_at(properties_obj, i);
    prop_obj = json_value_get_object(val);

    if (!key || !prop_obj)
      continue;

    /* Allocate or grow the fields array as needed */
    if (fields->size == fields->capacity) {
      new_cap = fields->capacity ? fields->capacity * 2 : 4;
      new_fields = (struct StructField *)realloc(
          fields->fields, new_cap * sizeof(struct StructField));
      if (!new_fields)
        return -3;
      fields->fields = new_fields;
      fields->capacity = new_cap;
    }

    field = &fields->fields[fields->size];

    /* Set field name */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    strncpy_s(field->name, sizeof(field->name), key, _TRUNCATE);
#else
    strncpy(field->name, key, sizeof(field->name));
    field->name[sizeof(field->name) - 1] = '\0';
#endif

    /* Read "type" */
    type_str = json_object_get_string(prop_obj, "type");
    if (type_str) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      strncpy_s(field->type, sizeof(field->type), type_str, _TRUNCATE);
#else
      strncpy(field->type, type_str, sizeof(field->type));
      field->type[sizeof(field->type) - 1] = '\0';
#endif
    } else {
      field->type[0] = '\0'; /* no type */
    }

    /* Read "$ref" if present (usually for nested structs) */
    ref_str = json_object_get_string(prop_obj, "$ref");
    if (ref_str != NULL) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      strncpy_s(field->ref, sizeof(field->ref), ref_str, _TRUNCATE);
#else
      strncpy(field->ref, ref_str, sizeof(field->ref));
      field->ref[sizeof(field->ref) - 1] = '\0';
#endif

      /* If $ref is present, it determines the type */
      if (schemas_obj_root != NULL) {
        ref_name = get_type_from_ref(ref_str);
        ref_schema = json_object_get_object(schemas_obj_root, ref_name);
        if (ref_schema != NULL &&
            (strcmp(json_object_get_string(ref_schema, "type"), "string") ==
             0) &&
            json_object_get_array(ref_schema, "enum") != NULL) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
          strcpy_s(field->type, sizeof(field->type), "enum");
#else
          strcpy(field->type, "enum");
#endif
        } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
          strcpy_s(field->type, sizeof(field->type), "object");
#else
          strcpy(field->type, "object");
#endif
        }
      } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
        strcpy_s(field->type, sizeof(field->type), "object");
#else
        strcpy(field->type, "object"); /* Fallback */
#endif
      }
    } else {
      field->ref[0] = '\0';
    }

    fields->size++;
  }
  return 0;
}

/*
 * Entry for command
 * argv[0] header.h
 * argv[1] schema.json
 */
int code2schema_main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: code2schema <header.h> <schema.json>\n");
    return EXIT_FAILURE;
  }

  return parse_header_file(argv[0], argv[1]);
}
