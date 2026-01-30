/**
 * @file code2schema.c
 * @brief Implementation of C header parsing and code-to-schema conversion.
 * Logic refactored to propagate proper error codes and handle allocation
 * failures.
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
#endif /* !strdup */
/* strtok_s is available */
#else
/* For non-MSVC, map strtok_s to strtok_r for consistent usage if available
   or use strtok_r directly. Here we use standard strtok_r logic if needed. */
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#include "code2schema.h"

#if defined(_WIN32)
#define strtok_r strtok_s
#endif

void trim_trailing(char *const str) {
  size_t len;
  if (!str)
    return;
  len = strlen(str);
  while (len > 0 &&
         (str[len - 1] == ';' || isspace((unsigned char)str[len - 1])))
    str[--len] = '\0';
}

bool str_starts_with(const char *str, const char *prefix) {
  if (!str || !prefix)
    return false;
  while (*prefix)
    if (*str++ != *prefix++)
      return false;
  return true;
}

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

int enum_members_init(struct EnumMembers *em) {
  if (!em)
    return EINVAL;
  em->size = 0;
  em->capacity = 8;
  em->members = (char **)malloc(em->capacity * sizeof(char *));
  if (!em->members)
    return ENOMEM;
  return 0;
}

void enum_members_free(struct EnumMembers *em) {
  size_t i;
  if (!em)
    return;
  if (em->members) {
    for (i = 0; i < em->size; i++)
      free(em->members[i]);
    free(em->members);
    em->members = NULL;
  }
  em->size = 0;
  em->capacity = 0;
}

int enum_members_add(struct EnumMembers *em, const char *const name) {
  if (!em || !name)
    return EINVAL;
  if (em->size >= em->capacity) {
    size_t new_cap = em->capacity == 0 ? 8 : em->capacity * 2;
    char **new_members =
        (char **)realloc(em->members, new_cap * sizeof(char *));
    if (new_members == NULL)
      return ENOMEM;
    em->members = new_members;
    em->capacity = new_cap;
  }
  em->members[em->size] = strdup(name);
  if (em->members[em->size] == NULL)
    return ENOMEM;
  em->size++;
  return 0;
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
  if (!sf)
    return;
  if (sf->fields) {
    free(sf->fields);
    sf->fields = NULL;
  }
  sf->size = 0;
  sf->capacity = 0;
}

int struct_fields_add(struct StructFields *sf, const char *const name,
                      const char *const type, const char *const ref) {
  if (!sf || !name || !type)
    return EINVAL;
  if (sf->size >= sf->capacity) {
    size_t new_cap = sf->capacity == 0 ? 8 : sf->capacity * 2;
    struct StructField *new_fields = (struct StructField *)realloc(
        sf->fields, new_cap * sizeof(struct StructField));
    if (!new_fields)
      return ENOMEM;
    sf->fields = new_fields;
    sf->capacity = new_cap;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strncpy_s(sf->fields[sf->size].name, sizeof(sf->fields[sf->size].name), name,
            _TRUNCATE);
#else
  strncpy(sf->fields[sf->size].name, name, sizeof(sf->fields[sf->size].name));
  sf->fields[sf->size].name[sizeof(sf->fields[sf->size].name) - 1] = '\0';
#endif

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strncpy_s(sf->fields[sf->size].type, sizeof(sf->fields[sf->size].type), type,
            _TRUNCATE);
#else
  strncpy(sf->fields[sf->size].type, type, sizeof(sf->fields[sf->size].type));
  sf->fields[sf->size].type[sizeof(sf->fields[sf->size].type) - 1] = '\0';
#endif

  if (ref) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    strncpy_s(sf->fields[sf->size].ref, sizeof(sf->fields[sf->size].ref), ref,
              _TRUNCATE);
#else
    strncpy(sf->fields[sf->size].ref, ref, sizeof(sf->fields[sf->size].ref));
    sf->fields[sf->size].ref[sizeof(sf->fields[sf->size].ref) - 1] = '\0';
#endif
  } else {
    sf->fields[sf->size].ref[0] = 0;
  }
  sf->size++;
  return 0;
}

int parse_struct_member_line(const char *line, struct StructFields *sf) {
  char namebuf[64];
  const char *p;
  const char *name_start, *name_end;
  size_t len;
  const char *type_start, *type_end;
  char type_buf[64], name_buf[64];
  size_t copy_len;

  if (!line || !sf)
    return EINVAL;

  while (isspace((unsigned char)*line)) {
    line++;
  }

  if (strncmp(line, "const ", 6) != 0 && strncmp(line, "char", 4) == 0) {
    /* char but not const char */
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
          return struct_fields_add(sf, namebuf, "string", NULL);
        }
      }
    }
  }

  if (strncmp(line, "bool ", 5) == 0) {
    p = line + 5;
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
        return struct_fields_add(sf, namebuf, "boolean", NULL);
      }
    }
  }

  if (strncmp(line, "int ", 4) == 0) {
    p = line + 4;
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
        return struct_fields_add(sf, namebuf, "integer", NULL);
      }
    }
  }

  if (strncmp(line, "double ", 7) == 0) {
    p = line + 7;
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
        return struct_fields_add(sf, namebuf, "number", NULL);
      }
    }
  }

  /* enum pointer */
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
      if (*p == '*') {
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
          return struct_fields_add(sf, name_buf, "enum", type_buf);
        }
      }
    }
  }

  /* struct pointer */
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
      if (*p == '*') {
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
          return struct_fields_add(sf, name_buf, "object", type_buf);
        }
      }
    }
  }

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
    dest[0] = '\0';
  }
}

static int write_enum_to_json_schema(JSON_Object *schemas_obj,
                                     const char *enum_name,
                                     struct EnumMembers *em) {
  JSON_Value *enum_val = json_value_init_object();
  JSON_Object *enum_obj;
  JSON_Value *enum_arr_val;
  JSON_Array *enum_arr;
  size_t i;

  if (!enum_val)
    return ENOMEM;

  enum_obj = json_value_get_object(enum_val);
  enum_arr_val = json_value_init_array();
  if (!enum_arr_val) {
    json_value_free(enum_val);
    return ENOMEM;
  }

  enum_arr = json_value_get_array(enum_arr_val);

  json_object_set_string(enum_obj, "type", "string");

  for (i = 0; i < em->size; i++)
    json_array_append_string(enum_arr, em->members[i]);

  json_object_set_value(enum_obj, "enum", enum_arr_val);
  json_object_set_value(schemas_obj, enum_name, enum_val);
  return 0;
}

int write_struct_to_json_schema(JSON_Object *schemas_obj,
                                const char *struct_name,
                                const struct StructFields *sf) {
  JSON_Value *struct_val = json_value_init_object();
  JSON_Object *struct_obj;
  JSON_Value *props_val;
  JSON_Object *props_obj;
  JSON_Value *required_val;
  JSON_Array *required_arr;
  size_t i;

  if (!struct_val)
    return ENOMEM;
  struct_obj = json_value_get_object(struct_val);

  props_val = json_value_init_object();
  if (!props_val) {
    json_value_free(struct_val);
    return ENOMEM;
  }
  props_obj = json_value_get_object(props_val);

  required_val = json_value_init_array();
  if (!required_val) {
    json_value_free(props_val);
    json_value_free(struct_val);
    return ENOMEM;
  }
  required_arr = json_value_get_array(required_val);

  json_object_set_string(struct_obj, "type", "object");
  json_object_set_value(struct_obj, "properties", props_val);
  json_object_set_value(struct_obj, "required", required_val);

  for (i = 0; i < sf->size; i++) {
    const char *name = sf->fields[i].name;
    const char *type = sf->fields[i].type;
    JSON_Value *field_val = json_value_init_object();
    JSON_Object *field_obj;
    if (!field_val) {
      /* Cleanup done by caller usually, but parson objects are linked */
      /* If we fail here, the partial structure is already in struct_obj */
      /* We can just return error and let caller free root if struct_val is
       * attached to known root, but here it's dangling locally. Freeing
       * struct_val frees its children including partial props. */
      json_value_free(struct_val);
      return ENOMEM;
    }
    field_obj = json_value_get_object(field_val);

    if (strcmp(type, "string") == 0) {
      json_object_set_string(field_obj, "type", "string");
    } else if (strcmp(type, "integer") == 0) {
      json_object_set_string(field_obj, "type", "integer");
    } else if (strcmp(type, "boolean") == 0) {
      json_object_set_string(field_obj, "type", "boolean");
    } else if (strcmp(type, "number") == 0) {
      json_object_set_string(field_obj, "type", "number");
    } else if (strcmp(type, "object") == 0 || strcmp(type, "enum") == 0) {
      char ref_buf[128];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      sprintf_s(ref_buf, sizeof(ref_buf), "#/components/schemas/%s",
                sf->fields[i].ref);
#else
      sprintf(ref_buf, "#/components/schemas/%s", sf->fields[i].ref);
#endif
      json_object_set_string(field_obj, "$ref", ref_buf);
    }

    json_object_set_value(props_obj, name, field_val);
    json_array_append_string(required_arr, name);
  }
  json_object_set_value(schemas_obj, struct_name, struct_val);
  return 0;
}

enum ParseState { P_NONE, P_IN_ENUM, P_IN_STRUCT };

static int parse_header_file(const char *header_filename,
                             const char *schema_out) {
  FILE *fp = NULL;
  char line[512];
  enum ParseState state = P_NONE;
  char enum_name[64];
  struct EnumMembers em;
  char struct_name[64];
  struct StructFields sf;
  JSON_Value *root_val = NULL;
  int rc = 0;

  char *p;
  char *brace;
  char *end_brace;
  char *body_to_parse = NULL;
  size_t len;
  char *context = NULL;
  char *token;
  char *member_p;
  char *eq;
  char *field_p;
  char *semi;

  memset(enum_name, 0, sizeof(enum_name));
  memset(struct_name, 0, sizeof(struct_name));

  /* Initialise memory structures */
  if ((rc = enum_members_init(&em)) != 0)
    return rc;
  if ((rc = struct_fields_init(&sf)) != 0) {
    enum_members_free(&em);
    return rc;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, header_filename, "r, ccs=UTF-8");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open header file %s\n", header_filename);
      rc = (int)err;
      if (rc == 0)
        rc = ENOENT;
      goto cleanup_init;
    }
  }
#else
  fp = fopen(header_filename, "r");
  if (!fp) {
    fprintf(stderr, "Failed to open header file %s\n", header_filename);
    rc = errno ? errno : ENOENT;
    goto cleanup_init;
  }
#endif

  root_val = json_value_init_object();
  if (!root_val) {
    rc = ENOMEM;
    goto cleanup_file;
  }
  {
    JSON_Object *root_obj = json_value_get_object(root_val);
    JSON_Value *components_val = json_value_init_object();
    JSON_Object *components_obj;
    JSON_Value *schemas_val;
    if (!components_val) {
      rc = ENOMEM;
      goto cleanup_json;
    }
    components_obj = json_value_get_object(components_val);
    schemas_val = json_value_init_object();
    if (!schemas_val) {
      json_value_free(components_val);
      rc = ENOMEM;
      goto cleanup_json;
    }
    /* Values are owned by root once set */
    json_object_set_value(root_obj, "components", components_val);
    json_object_set_value(components_obj, "schemas", schemas_val);
  }

  while (read_line(fp, line, sizeof(line))) {
    p = line;
  process_line:
    while (isspace((unsigned char)*p))
      p++;
    if (!*p)
      continue;

    if (state == P_NONE) {
      brace = strchr(p, '{');
      semi = strchr(p, ';');

      if ((str_starts_with(p, "enum ") || str_starts_with(p, "struct ")) &&
          semi && (!brace || semi < brace)) {
        p = semi + 1;
        goto process_line;
      }

      if (str_starts_with(p, "enum ") && brace) {
        extract_name(enum_name, sizeof(enum_name), p + 5, brace);
        enum_members_free(&em);
        if ((rc = enum_members_init(&em)) != 0)
          goto cleanup_json;
        state = P_IN_ENUM;
        p = brace + 1;
        goto process_line;
      } else if (str_starts_with(p, "struct ") && brace) {
        extract_name(struct_name, sizeof(struct_name), p + 7, brace);
        struct_fields_free(&sf);
        if ((rc = struct_fields_init(&sf)) != 0)
          goto cleanup_json;
        state = P_IN_STRUCT;
        p = brace + 1;
        goto process_line;
      }
    }

    if (state == P_IN_ENUM) {
      end_brace = strchr(p, '}');

      if (end_brace) {
        len = end_brace - p;
        body_to_parse = (char *)malloc(len + 1);
        if (!body_to_parse) {
          rc = ENOMEM;
          goto cleanup_json;
        }
        memcpy(body_to_parse, p, len);
        body_to_parse[len] = '\0';
      } else {
        body_to_parse = strdup(p);
        if (!body_to_parse) {
          rc = ENOMEM;
          goto cleanup_json;
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
          if (strlen(member_p) > 0) {
            if ((rc = enum_members_add(&em, member_p)) != 0) {
              free(body_to_parse);
              goto cleanup_json;
            }
          }
        }
        token = strtok_r(NULL, ",", &context);
      }
      free(body_to_parse);
      body_to_parse = NULL;

      if (end_brace) {
        if (strlen(enum_name) > 0) {
          JSON_Object *schemas_obj = json_object_get_object(
              json_object_get_object(json_value_get_object(root_val),
                                     "components"),
              "schemas");
          if ((rc = write_enum_to_json_schema(schemas_obj, enum_name, &em)) !=
              0)
            goto cleanup_json;
        }
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
          rc = ENOMEM;
          goto cleanup_json;
        }
        memcpy(body_to_parse, p, len);
        body_to_parse[len] = '\0';
      } else {
        body_to_parse = strdup(p);
        if (!body_to_parse) {
          rc = ENOMEM;
          goto cleanup_json;
        }
      }

      context = NULL;
      token = strtok_r(body_to_parse, ";", &context);
      while (token) {
        field_p = token;
        while (isspace((unsigned char)*field_p))
          field_p++;
        if (strlen(field_p) > 0) {
          if ((rc = parse_struct_member_line(field_p, &sf)) != 0) {
            free(body_to_parse);
            goto cleanup_json;
          }
        }
        token = strtok_r(NULL, ";", &context);
      }
      free(body_to_parse);
      body_to_parse = NULL;

      if (end_brace) {
        if (strlen(struct_name) > 0) {
          JSON_Object *schemas_obj = json_object_get_object(
              json_object_get_object(json_value_get_object(root_val),
                                     "components"),
              "schemas");
          if ((rc = write_struct_to_json_schema(schemas_obj, struct_name,
                                                &sf)) != 0)
            goto cleanup_json;
        }
        state = P_NONE;
        p = end_brace + 1;
        while (*p && (isspace((unsigned char)*p) || *p == ';'))
          p++;
        if (*p)
          goto process_line;
      }
    }
  }

  if (json_serialize_to_file_pretty(root_val, schema_out) != JSONSuccess) {
    fprintf(stderr, "Failed to write JSON to %s\n", schema_out);
    rc = EIO;
  } else {
    printf("Wrote JSON schema to %s\n", schema_out);
    rc = 0;
  }

cleanup_json:
  json_value_free(root_val);
cleanup_file:
  fclose(fp);
cleanup_init:
  enum_members_free(&em);
  struct_fields_free(&sf);
  return rc;
}

int json_array_to_enum_members(const JSON_Array *enum_arr,
                               struct EnumMembers *em) {
  size_t n, i;
  const char *member;
  int rc;
  if (!enum_arr || !em)
    return EINVAL;

  if ((rc = enum_members_init(em)) != 0)
    return rc;

  n = json_array_get_count(enum_arr);
  for (i = 0; i < n; i++) {
    member = json_array_get_string(enum_arr, i);
    if (member) {
      if ((rc = enum_members_add(em, member)) != 0)
        return rc;
    }
  }
  return 0;
}

int json_object_to_struct_fields(const JSON_Object *schema_obj,
                                 struct StructFields *fields,
                                 const JSON_Object *schemas_obj_root) {
  const char *ref_str;
  const JSON_Object *properties_obj;
  size_t count, i;
  const char *key;
  const JSON_Value *val;
  const JSON_Object *prop_obj;
  const char *type_str;
  const char *ref_name;
  const JSON_Object *ref_schema;
  char type_buf[64] = {0};
  char ref_buf[64] = {0};
  int rc;

  if (!schema_obj || !fields)
    return EINVAL;

  /* Reset fields container */
  if ((rc = struct_fields_init(fields)) != 0)
    return rc;

  properties_obj = json_object_get_object(schema_obj, "properties");
  if (properties_obj == NULL)
    return 0;

  count = json_object_get_count(properties_obj);
  for (i = 0; i < count; i++) {
    key = json_object_get_name(properties_obj, i);
    val = json_object_get_value_at(properties_obj, i);
    prop_obj = json_value_get_object(val);

    if (!key || !prop_obj)
      continue;

    type_str = json_object_get_string(prop_obj, "type");
    if (type_str) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      strncpy_s(type_buf, sizeof(type_buf), type_str, _TRUNCATE);
#else
      strncpy(type_buf, type_str, sizeof(type_buf));
      type_buf[sizeof(type_buf) - 1] = '\0';
#endif
    } else {
      type_buf[0] = '\0';
    }

    ref_str = json_object_get_string(prop_obj, "$ref");
    if (ref_str != NULL) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      strncpy_s(ref_buf, sizeof(ref_buf), ref_str, _TRUNCATE);
#else
      strncpy(ref_buf, ref_str, sizeof(ref_buf));
      ref_buf[sizeof(ref_buf) - 1] = '\0';
#endif

      if (schemas_obj_root != NULL) {
        ref_name = get_type_from_ref(ref_str);
        ref_schema = json_object_get_object(schemas_obj_root, ref_name);
        if (ref_schema != NULL) {
          const char *const ref_type_str =
              json_object_get_string(ref_schema, "type");
          if (ref_type_str != NULL && strcmp(ref_type_str, "string") == 0 &&
              json_object_get_array(ref_schema, "enum") != NULL) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
            strcpy_s(type_buf, sizeof(type_buf), "enum");
#else
            strcpy(type_buf, "enum");
#endif
          } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
            strcpy_s(type_buf, sizeof(type_buf), "object");
#else
            strcpy(type_buf, "object");
#endif
          }
        } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
          strcpy_s(type_buf, sizeof(type_buf), "object");
#else
          strcpy(type_buf, "object");
#endif
        }
      } else {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
        strcpy_s(type_buf, sizeof(type_buf), "object");
#else
        strcpy(type_buf, "object");
#endif
      }
    } else {
      ref_buf[0] = '\0';
    }

    if ((rc = struct_fields_add(fields, key, type_buf,
                                (ref_buf[0] != '\0') ? ref_buf : NULL)) != 0) {
      return rc;
    }
  }
  return 0;
}

int code2schema_main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: code2schema <header.h> <schema.json>\n");
    return EXIT_FAILURE;
  }
  /* Normalize return code to EXIT_... values for main semantics if desired,
     or just pass through non-zero means fail */
  return parse_header_file(argv[0], argv[1]) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
