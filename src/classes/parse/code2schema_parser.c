/**
 * @file code2schema_parser.c
 * @brief C struct member and union parsing routines.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Parses a struct member line and populates a StructFields object.
 */
cdd_c_error_t parse_struct_member_line(const char *line,
                                       struct StructFields *sf) {
  char buf[MAX_LINE_LENGTH];
  char *last_space;
  char name[64] = {0};
  char type_raw[64] = {0};
  char bit_width[16] = {0};
  int is_fam = 0;
  int is_ptr = 0;
  char *colon_ptr = NULL;

  int is_shard_key = 0;
  int is_shard_hash = 0;
  int is_track_telemetry = 0;
  int is_slow_query = 0;
  int slow_query_ms = 0;

  /* Helper for mapping result */
  struct OpenApiTypeMapping mapping;
  cdd_c_error_t rc;

  if (!line || !sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;

/* Basic heuristic parsing: "Type name;" or "Type name : width;" */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strncpy_s(buf, sizeof(buf), line, sizeof(buf) - 1);
#else
  strncpy(buf, line, sizeof(buf) - 1);
#endif
  buf[sizeof(buf) - 1] = '\0';
  trim_trailing(buf);

  {
    char *cmt = strstr(buf, "//");
    if (!cmt)
      cmt = strstr(buf, "/*");
    if (cmt) {
      char *sqw;
      if (strstr(cmt, "@shard_key"))
        is_shard_key = 1;
      if (strstr(cmt, "@shard_hash"))
        is_shard_hash = 1;
      if (strstr(cmt, "@track_telemetry"))
        is_track_telemetry = 1;
      if (strstr(cmt, "@slow_query_warn"))
        is_slow_query = 1;

      /* Extract numeric argument for slow_query_warn */
      sqw = strstr(cmt, "@slow_query_warn(");
      if (sqw) {
        slow_query_ms = atoi(sqw + 17);
      }

      *cmt = '\0';
      trim_trailing(buf);
    }
  }

  /* Handle Bit-fields */ colon_ptr = strrchr(buf, ':');
  if (colon_ptr) {
    char *w = colon_ptr + 1;
    while (*w && isspace((unsigned char)*w))
      w++;
    if (*w && (isdigit((unsigned char)*w) || *w == '(' ||
               isalpha((unsigned char)*w))) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncpy_s(bit_width, sizeof(bit_width), w, sizeof(bit_width) - 1);
#else
      strncpy(bit_width, w, sizeof(bit_width) - 1);
#endif
      *colon_ptr = '\0';
      trim_trailing(buf);
    }
  }

  last_space = strrchr(buf, ' ');
  if (!last_space) {
    /* Maybe it's "int*p;" without space? Parser assumes space separator. */
    /* Check for * split if no space */
    last_space = strrchr(buf, '*');
    if (!last_space) {
      return CDD_C_SUCCESS; /* Skip invalid */
    }
  }

  /* Extact Name */
  {
    char *n = last_space + 1;
    if (last_space[0] == '*') {
      n = last_space; /* Treat * as part of name logic temporarility? No. */
    }
    /* "int *p" -> last_space=' '. n="*p" */

    /* Re-evaluate split logic carefully */
    /* If strict space split: "int *p" -> type="int", name="*p" */
    *last_space = '\0'; /* Split string */

    /* n points to start of declared name (maybe with *) */
    while (*n == '*') {
      is_ptr = 1;
      n++;
    }
#if defined(_MSC_VER)
    strncpy_s(name, 63 + 1, n, 63);
#else
    strncpy(name, n, 63);
#endif

    /* Check FAM */
    {
      size_t nlen = strlen(name);
      if (nlen > 2 && name[nlen - 1] == ']' && name[nlen - 2] == '[') {
        is_fam = 1;
        name[nlen - 2] = '\0';
      }
    }
  }

/* Extract Type Raw */
/* If buf was "int *p", and we split at space, buf="int", name="*p". */
/* If buf was "struct S* p", split at last space (' '), buf="struct S*",
 * name="p" */
#if defined(_MSC_VER)
  strncpy_s(type_raw, 63 + 1, buf, 63);
#else
  {
    size_t len = strlen(buf);
    if (len > 63)
      len = 63;
    memcpy(type_raw, buf, len);
    type_raw[len] = '\0';
  }
#endif
  /* Ensure we capture the pointer asterisk if it was on the type side */
  /* "struct S* p" -> type="struct S*" */
  /* "struct S *p" -> type="struct S", name="*p" (handled above) */
  /* Reconstruct logical type for mapper: If name had *, append * to type_raw?
     Yes, C Mapping needs "Type *" to detect pointers correctly esp for
     arrays/strings */
  if (is_ptr) {
    /* Append * if not present */
    if (type_raw[strlen(type_raw) - 1] != '*') {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      strncat_s(type_raw, sizeof(type_raw), "*", 63 - strlen(type_raw));
#else
      strncat(type_raw, "*", 63 - strlen(type_raw));
#endif
    }
  }

  /* --- Use Mapper --- */
  rc = c_mapping_map_type(type_raw, name, &mapping);
  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Translate Mapper Result to StructFields format */
  /* StructFields uses "type" string and "ref" string. */
  /* if kind==PRIMITIVE -> type=oa_type */
  /* if kind==OBJECT -> type="object", ref=ref_name */
  /* if kind==ARRAY -> type="array", ref=oa_type (if prim) or ref_name (if obj)
   */
  /* if CHAR* -> mapped to PRIMITIVE "string", handled correctly */

  {
    const char *final_type = "string";
    const char *final_ref = NULL;

    if (mapping.kind == OA_TYPE_PRIMITIVE) {
      final_type = mapping.oa_type;
    } else if (mapping.kind == OA_TYPE_OBJECT) {
      final_type = "object";
      final_ref = mapping.ref_name;
    } else if (mapping.kind == OA_TYPE_ARRAY) {
      final_type = "array";
      /* Item type goes in ref for StructFields logic currently */
      final_ref = mapping.ref_name ? mapping.ref_name : mapping.oa_type;
    }

    if (is_fam && !(mapping.kind == OA_TYPE_PRIMITIVE &&
                    strcmp(mapping.oa_type, "string") == 0)) {
      final_ref = final_type;
      final_type = "array";
    }

    if (struct_fields_add(sf, name, final_type, final_ref, NULL,
                          bit_width[0] ? bit_width : NULL) == 0) {
      struct StructField *field = &sf->fields[sf->size - 1];
      if (is_fam) {
        field->is_flexible_array = 1;
      }

      /* Inject cdd-c specific ORM annotations */
      if (is_shard_key || is_shard_hash || is_track_telemetry ||
          is_slow_query) {
        char cdd_json[256];
        CDD_SNPRINTF(cdd_json, sizeof(cdd_json),
                     "{\"x-cdd-shard-key\":%s, \"x-cdd-shard-hash\":%s, "
                     "\"x-cdd-track-telemetry\":%s, \"x-cdd-slow-query\":%d}",
                     is_shard_key ? "true" : "false",
                     is_shard_hash ? "true" : "false",
                     is_track_telemetry ? "true" : "false",
                     is_slow_query ? slow_query_ms : 0);
        {
          cdd_c_error_t rc_c2s = c2s_merge_schema_extras_strings(
              &field->schema_extra_json, cdd_json);
          if (rc_c2s != CDD_C_SUCCESS) {
            c_mapping_free(&mapping);
            return rc_c2s;
          }
        }
      }

      if (mapping.oa_format) {
        if (mapping.kind == OA_TYPE_PRIMITIVE && !is_fam) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          strncpy_s(field->format, sizeof(field->format), mapping.oa_format,
                    sizeof(field->format) - 1);
#else
          strncpy(field->format, mapping.oa_format, sizeof(field->format) - 1);
#endif
          field->format[sizeof(field->format) - 1] = '\0';
        } else {
          char fmt_json[64];
          CDD_SNPRINTF(fmt_json, sizeof(fmt_json), "{\"format\":\"%s\"}",
                       mapping.oa_format);
          if (c2s_merge_schema_extras_strings(&field->items_extra_json,
                                              fmt_json) != 0) {
            rc = CDD_C_ERROR_MEMORY;
          }
        }
      }
    } else {
      rc = CDD_C_ERROR_MEMORY;
    }
  }

  c_mapping_free(&mapping);
  return rc;
}

/**
 * @brief Parses union member tokens from a TokenList starting after '{' and
 * writes the resulting oneOf schema to schemas_obj.
 */
cdd_c_error_t c2s_parse_union_tokens_and_write(const struct TokenList *tl,
                                               size_t *idx,
                                               JSON_Object *schemas_obj,
                                               const char *union_name) {
  JSON_Value *union_val = json_value_init_object();
  JSON_Object *union_obj = json_value_get_object(union_val);
  JSON_Value *oneof_val = json_value_init_array();
  JSON_Array *oneof_arr = json_value_get_array(oneof_val);
  size_t i = *idx;

  while (i < tl->size) {
    size_t stmt_start;
    size_t stmt_end;
    const struct Token *first_tok = NULL;
    const struct Token *last_ident = NULL;
    size_t k;
    enum TokenKind k_cur;

    while ((c2s_tok_kind(tl, i, &k_cur), k_cur == TOKEN_WHITESPACE ||
                                             k_cur == TOKEN_COMMENT ||
                                             k_cur == TOKEN_SEMICOLON)) {
      i++;
    }
    if (k_cur == TOKEN_UNKNOWN)
      break;
    if (k_cur == TOKEN_RBRACE) {
      i++;
      break;
    }

    stmt_start = i;
    while ((c2s_tok_kind(tl, i, &k_cur), k_cur != TOKEN_UNKNOWN &&
                                             k_cur != TOKEN_SEMICOLON &&
                                             k_cur != TOKEN_RBRACE)) {
      i++;
    }
    stmt_end = i;
    if (k_cur == TOKEN_SEMICOLON) {
      i++;
    }

    for (k = stmt_start; k < stmt_end; k++) {
      if (tl->tokens[k].kind == TOKEN_WHITESPACE ||
          tl->tokens[k].kind == TOKEN_COMMENT)
        continue;
      if (!first_tok)
        first_tok = &tl->tokens[k];
      if (tl->tokens[k].kind == TOKEN_IDENTIFIER)
        last_ident = &tl->tokens[k];
    }

    if (last_ident && first_tok != last_ident) {
      char typebuf[64] = {0};
      char namebuf[64] = {0};
      JSON_Value *option_val = json_value_init_object();
      JSON_Object *option_obj = json_value_get_object(option_val);
      JSON_Value *props_val = json_value_init_object();
      JSON_Object *props_obj = json_value_get_object(props_val);
      JSON_Value *field_val = json_value_init_object();
      JSON_Object *field_obj = json_value_get_object(field_val);

      c2s_copy_token_ident(typebuf, first_tok);
      c2s_copy_token_ident(namebuf, last_ident);

      if (strcmp(typebuf, "int") == 0)
        json_object_set_string(field_obj, "type", "integer");
      else if (strcmp(typebuf, "char") == 0)
        json_object_set_string(field_obj, "type", "string");
      else if (strcmp(typebuf, "float") == 0)
        json_object_set_string(field_obj, "type", "number");
      else
        json_object_set_string(field_obj, "type", "object");

      json_object_set_value(props_obj, namebuf, field_val);
      json_object_set_string(option_obj, "type", "object");
      json_object_set_value(option_obj, "properties", props_val);
      json_object_set_string(option_obj, "title", namebuf);
      json_array_append_value(oneof_arr, option_val);
    }
  }

  *idx = i;
  json_object_set_value(union_obj, "oneOf", oneof_val);
  json_object_set_string(union_obj, "type", "object");
  json_object_set_value(schemas_obj, union_name, union_val);

  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a union definition and writes it to a JSON Schema
 * object.
 */
cdd_c_error_t c2s_parse_union_and_write(FILE *fp, JSON_Object *schemas_obj,
                                        const char *union_name) {
  char *content = NULL;
  size_t sz = 0;
  struct TokenList *tl = NULL;
  size_t idx = 0;
  cdd_c_error_t rc;

  if (!fp || !schemas_obj || !union_name) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = read_from_fh(fp, &content, &sz);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = tokenize(az_span_create((uint8_t *)content, sz), &tl);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(content);
    return rc;
  }

  rc = c2s_parse_union_tokens_and_write(tl, &idx, schemas_obj, union_name);
  free_token_list(tl);
  C_CDD_FREE(content);
  return rc;
}

/**
 * @brief Collapses pointer + length field pairs into array representations.
 *
 * @param[in,out] sf The StructFields structure to modify.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t c2s_collapse_arrays(struct StructFields *sf) {
  size_t i, j;
  if (!sf)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < sf->size; ++i) {
    if (strncmp(sf->fields[i].name, "n_", 2) == 0) {
      const char *base_name = sf->fields[i].name + 2;
      for (j = 0; j < sf->size; ++j) {
        if (strcmp(sf->fields[j].name, base_name) == 0) {
          /* Found a match! sf->fields[j] is the array, sf->fields[i] is the
           * length */
          if (strcmp(sf->fields[j].type, "array") != 0) {
            if (strcmp(sf->fields[j].type, "string") == 0) {
              CDD_STRCPY(sf->fields[j].type, sizeof(sf->fields[j].type),
                         "array");
              CDD_STRCPY(sf->fields[j].ref, sizeof(sf->fields[j].ref),
                         "string");
            } else if (strcmp(sf->fields[j].type, "object") == 0) {
              CDD_STRCPY(sf->fields[j].type, sizeof(sf->fields[j].type),
                         "array");
              /* keep existing ref for objects */
            } else {
              CDD_STRCPY(sf->fields[j].ref, sizeof(sf->fields[j].ref),
                         sf->fields[j].type);
              CDD_STRCPY(sf->fields[j].type, sizeof(sf->fields[j].type),
                         "array");
            }
          }
          /* Remove the n_ field */
          memmove(&sf->fields[i], &sf->fields[i + 1],
                  (sf->size - i - 1) * sizeof(struct StructField));
          sf->size--;
          i--; /* Adjust index since we removed a field */
          break;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}
