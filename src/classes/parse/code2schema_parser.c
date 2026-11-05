/**
 * @file code2schema_parser.c
 * @brief C struct member and union parsing routines.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/*
 * @brief Parses a struct member line and populates a StructFields object.
 *
 * @param[in] line Raw C struct member declaration line.
 * @param[in,out] sf Target StructFields container to append the parsed field
 * to.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t parse_struct_member_line(const char *line,
                                       struct StructFields *sf) {
  char buf[MAX_LINE_LENGTH];
  char serde_comments[MAX_LINE_LENGTH];
  size_t serde_pos = 0;
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

  serde_comments[0] = '\0';

/* Basic heuristic parsing: "Type name;" or "Type name : width;" */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  strncpy_s(buf, sizeof(buf), line, sizeof(buf) - 1);
#else
  strncpy(buf, line, sizeof(buf) - 1);
#endif
  buf[sizeof(buf) - 1] = '\0';
  trim_trailing(buf);

  /* Extract any C89 block comments (leading, inline, or trailing) */
  {
    char *c_start;
    while ((c_start = strstr(buf, "/*")) != NULL) {
      char *c_end = strstr(c_start + 2, "*/");
      size_t clen;
      size_t k;
      if (c_end) {
        clen = (size_t)((c_end + 2) - c_start);
      } else {
        clen = strlen(c_start);
      }

      if (strstr(c_start, "@shard_key"))
        is_shard_key = 1;
      if (strstr(c_start, "@shard_hash"))
        is_shard_hash = 1;
      if (strstr(c_start, "@track_telemetry"))
        is_track_telemetry = 1;
      if (strstr(c_start, "@slow_query_warn")) {
        char *sqw = strstr(c_start, "@slow_query_warn(");
        is_slow_query = 1;
        if (sqw)
          slow_query_ms = atoi(sqw + 17);
      }

      for (k = 0; k < clen && serde_pos + 2 < sizeof(serde_comments); ++k) {
        serde_comments[serde_pos++] = c_start[k];
      }
      serde_comments[serde_pos++] = '\n';
      serde_comments[serde_pos] = '\0';

      if (c_end) {
        memmove(c_start, c_end + 2, strlen(c_end + 2) + 1);
      } else {
        *c_start = '\0';
      }
    }
  }

  {
    char *cmt = strstr(buf, "//");
    if (cmt) {
      char *sqw;
      size_t k;
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

      for (k = 0; cmt[k] != '\0'; ++k) {
        serde_comments[serde_pos++] = cmt[k];
      }
      serde_comments[serde_pos] = '\0';

      *cmt = '\0';
    }
    trim_trailing(buf);
    {
      char *lead = buf;
      while (*lead && isspace((unsigned char)*lead))
        lead++;
      if (lead != buf)
        memmove(buf, lead, strlen(lead) + 1);
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
    last_space = strrchr(buf, '*');
    if (!last_space) {
      return CDD_C_SUCCESS; /* Skip invalid */
    }
    is_ptr = 1;
  }

  /* Extact Name */
  {
    char *n = last_space + 1;
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
    size_t fixed_arr_len = 0;
    char *lbrack = strchr(name, '[');

    if (lbrack && !is_fam) {
      fixed_arr_len = (size_t)atoi(lbrack + 1);
      *lbrack = '\0';
    }

    if (mapping.kind == OA_TYPE_PRIMITIVE) {
      final_type = mapping.oa_type;
    } else if (mapping.kind == OA_TYPE_OBJECT) {
      final_type = "object";
      final_ref = mapping.ref_name;
    } else {
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
      if (fixed_arr_len > 0) {
        field->fixed_array_len = fixed_arr_len;
        CDD_SNPRINTF(field->c_type, sizeof(field->c_type), "%s[%lu]", type_raw,
                     (unsigned long)fixed_arr_len);
      } else {
        CDD_STRCPY(field->c_type, sizeof(field->c_type), type_raw);
      }

      if (serde_comments[0] != '\0') {
        cdd_c_error_t rc_serde =
            cdd_serde_parse_field_comment(serde_comments, field);
        if (rc_serde != CDD_C_SUCCESS) {
          c_mapping_free(&mapping);
          return rc_serde;
        }
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

/*
 * @brief Parses union member tokens from a TokenList starting after '{' and
 * writes the resulting oneOf schema to schemas_obj.
 *
 * @param[in] tl Lexical token list containing the union body.
 * @param[in,out] idx Pointer to the current token index (updated past closing
 * `}`).
 * @param[in,out] schemas_obj Target `components.schemas` JSON_Object.
 * @param[in] union_name Name of the C union type.
 * @return CDD_C_SUCCESS on success, or an error code.
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

  for (;;) {
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
    while ((c2s_tok_kind(tl, i, &k_cur),
            !((k_cur == TOKEN_UNKNOWN) | (k_cur == TOKEN_SEMICOLON) |
              (k_cur == TOKEN_RBRACE)))) {
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

      if (json_object_set_value(props_obj, namebuf, field_val) != JSONSuccess)
        json_value_free(field_val);
      json_object_set_string(option_obj, "type", "object");
      if (json_object_set_value(option_obj, "properties", props_val) !=
          JSONSuccess)
        json_value_free(props_val);
      json_object_set_string(option_obj, "title", namebuf);
      if (json_array_append_value(oneof_arr, option_val) != JSONSuccess)
        json_value_free(option_val);
    }
  }

  *idx = i;
  if (json_object_set_value(union_obj, "oneOf", oneof_val) != JSONSuccess)
    json_value_free(oneof_val);
  json_object_set_string(union_obj, "type", "object");
  if (json_object_set_value(schemas_obj, union_name, union_val) != JSONSuccess)
    json_value_free(union_val);

  return CDD_C_SUCCESS;
}

/*
 * @brief Parses a union definition and writes it to a JSON Schema
 * object.
 *
 * @param[in,out] fp Open file stream positioned at the union body.
 * @param[in,out] schemas_obj Target `components.schemas` JSON_Object.
 * @param[in] union_name Name of the C union type.
 * @return CDD_C_SUCCESS on success, or an error code.
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

  rc = tokenize(az_span_create_from_str_of_size(content, sz), &tl);
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
          /* Propagate any serde attributes placed on n_<field> onto <field> */
          {
            struct StructField *nf = &sf->fields[i];
            struct StructField *af = &sf->fields[j];
            size_t a_idx;
            if (nf->serde_rename[0] && !af->serde_rename[0])
              CDD_STRCPY(af->serde_rename, sizeof(af->serde_rename),
                         nf->serde_rename);
            if (nf->serde_rename_ser[0] && !af->serde_rename_ser[0])
              CDD_STRCPY(af->serde_rename_ser, sizeof(af->serde_rename_ser),
                         nf->serde_rename_ser);
            if (nf->serde_rename_de[0] && !af->serde_rename_de[0])
              CDD_STRCPY(af->serde_rename_de, sizeof(af->serde_rename_de),
                         nf->serde_rename_de);
            if (nf->serde_case != CDD_SERDE_CASE_NONE &&
                af->serde_case == CDD_SERDE_CASE_NONE)
              af->serde_case = nf->serde_case;
            if (nf->serde_replace_from[0] && !af->serde_replace_from[0]) {
              CDD_STRCPY(af->serde_replace_from, sizeof(af->serde_replace_from),
                         nf->serde_replace_from);
              CDD_STRCPY(af->serde_replace_to, sizeof(af->serde_replace_to),
                         nf->serde_replace_to);
            }
            if (nf->serde_skip)
              af->serde_skip = 1;
            if (nf->serde_skip_serializing)
              af->serde_skip_serializing = 1;
            if (nf->serde_skip_deserializing)
              af->serde_skip_deserializing = 1;
            if (nf->serde_skip_if_kind != CDD_SERDE_SKIP_IF_NONE &&
                af->serde_skip_if_kind == CDD_SERDE_SKIP_IF_NONE) {
              af->serde_skip_if_kind = nf->serde_skip_if_kind;
              CDD_STRCPY(af->serde_skip_if_fn, sizeof(af->serde_skip_if_fn),
                         nf->serde_skip_if_fn);
            }
            if (nf->serde_flatten)
              af->serde_flatten = 1;
            if (nf->serde_has_default)
              af->serde_has_default = 1;
            if (nf->serde_default_fn[0] && !af->serde_default_fn[0])
              CDD_STRCPY(af->serde_default_fn, sizeof(af->serde_default_fn),
                         nf->serde_default_fn);
            if (nf->serde_with[0] && !af->serde_with[0])
              CDD_STRCPY(af->serde_with, sizeof(af->serde_with),
                         nf->serde_with);
            if (nf->serde_serialize_with[0] && !af->serde_serialize_with[0])
              CDD_STRCPY(af->serde_serialize_with,
                         sizeof(af->serde_serialize_with),
                         nf->serde_serialize_with);
            if (nf->serde_deserialize_with[0] && !af->serde_deserialize_with[0])
              CDD_STRCPY(af->serde_deserialize_with,
                         sizeof(af->serde_deserialize_with),
                         nf->serde_deserialize_with);
            for (a_idx = 0; a_idx < nf->n_serde_aliases; ++a_idx) {
              if (nf->serde_aliases[a_idx]) {
                cdd_c_error_t rc_al =
                    struct_field_add_alias(af, nf->serde_aliases[a_idx]);
                if (rc_al != CDD_C_SUCCESS)
                  return rc_al;
                C_CDD_FREE(nf->serde_aliases[a_idx]);
                nf->serde_aliases[a_idx] = NULL;
              }
            }
            if (nf->serde_aliases) {
              C_CDD_FREE(nf->serde_aliases);
              nf->serde_aliases = NULL;
              nf->n_serde_aliases = 0;
            }
            if (nf->cli_short && !af->cli_short)
              af->cli_short = nf->cli_short;
            if (nf->cli_short_aliases[0] && !af->cli_short_aliases[0])
              CDD_STRCPY(af->cli_short_aliases, sizeof(af->cli_short_aliases),
                         nf->cli_short_aliases);
            if (nf->cli_long[0] && !af->cli_long[0])
              CDD_STRCPY(af->cli_long, sizeof(af->cli_long), nf->cli_long);
            if (nf->cli_has_long && !af->cli_has_long)
              af->cli_has_long = nf->cli_has_long;
            if (nf->cli_negatable)
              af->cli_negatable = 1;
            if (nf->cli_positional)
              af->cli_positional = 1;
            if (nf->cli_index >= 0 && af->cli_index < 0)
              af->cli_index = nf->cli_index;
            if (nf->cli_last)
              af->cli_last = 1;
            if (nf->cli_trailing_var_arg)
              af->cli_trailing_var_arg = 1;
            if (nf->cli_value_name[0] && !af->cli_value_name[0])
              CDD_STRCPY(af->cli_value_name, sizeof(af->cli_value_name),
                         nf->cli_value_name);
            if (nf->cli_help[0] && !af->cli_help[0])
              CDD_STRCPY(af->cli_help, sizeof(af->cli_help), nf->cli_help);
            if (nf->cli_long_help) {
              if (!af->cli_long_help) {
                af->cli_long_help = nf->cli_long_help;
                nf->cli_long_help = NULL;
              } else {
                C_CDD_FREE(nf->cli_long_help);
                nf->cli_long_help = NULL;
              }
            }
            if (nf->cli_help_heading[0] && !af->cli_help_heading[0])
              CDD_STRCPY(af->cli_help_heading, sizeof(af->cli_help_heading),
                         nf->cli_help_heading);
            if (nf->cli_env[0] && !af->cli_env[0])
              CDD_STRCPY(af->cli_env, sizeof(af->cli_env), nf->cli_env);
            if (nf->cli_hide_env)
              af->cli_hide_env = 1;
            if (nf->cli_hide_env_values)
              af->cli_hide_env_values = 1;
            if (nf->cli_default_missing_val[0] &&
                !af->cli_default_missing_val[0])
              CDD_STRCPY(af->cli_default_missing_val,
                         sizeof(af->cli_default_missing_val),
                         nf->cli_default_missing_val);
            if (nf->cli_hide_default_value)
              af->cli_hide_default_value = 1;
            if (nf->cli_action != CDD_CLI_ACTION_AUTO &&
                af->cli_action == CDD_CLI_ACTION_AUTO)
              af->cli_action = nf->cli_action;
            if (nf->cli_value_hint != CDD_CLI_HINT_NONE &&
                af->cli_value_hint == CDD_CLI_HINT_NONE)
              af->cli_value_hint = nf->cli_value_hint;
            if (nf->cli_value_delimiter && !af->cli_value_delimiter)
              af->cli_value_delimiter = nf->cli_value_delimiter;
            if (nf->cli_num_args_min && !af->cli_num_args_min)
              af->cli_num_args_min = nf->cli_num_args_min;
            if (nf->cli_num_args_max && !af->cli_num_args_max)
              af->cli_num_args_max = nf->cli_num_args_max;
            if (nf->cli_require_equals)
              af->cli_require_equals = 1;
            if (nf->cli_allow_hyphen_values)
              af->cli_allow_hyphen_values = 1;
            if (nf->cli_global)
              af->cli_global = 1;
            if (nf->cli_hidden)
              af->cli_hidden = 1;
            if (nf->cli_exclusive)
              af->cli_exclusive = 1;
            if (nf->cli_subcommand)
              af->cli_subcommand = 1;
            af->cli_skip |= nf->cli_skip;
            af->cli_skip_to_cli |= nf->cli_skip_to_cli;
            af->cli_skip_from_cli |= nf->cli_skip_from_cli;
            if (nf->cli_value_parser[0] && !af->cli_value_parser[0])
              CDD_STRCPY(af->cli_value_parser, sizeof(af->cli_value_parser),
                         nf->cli_value_parser);
            if (nf->cli_group[0] && !af->cli_group[0])
              CDD_STRCPY(af->cli_group, sizeof(af->cli_group), nf->cli_group);
            if (nf->cli_required_if_field[0] && !af->cli_required_if_field[0]) {
              CDD_STRCPY(af->cli_required_if_field,
                         sizeof(af->cli_required_if_field),
                         nf->cli_required_if_field);
              CDD_STRCPY(af->cli_required_if_val,
                         sizeof(af->cli_required_if_val),
                         nf->cli_required_if_val);
            }
            for (a_idx = 0; a_idx < nf->n_cli_conflicts_with; ++a_idx) {
              if (nf->cli_conflicts_with[a_idx]) {
                cdd_c_error_t rc_cw = struct_field_add_cli_conflict(
                    af, nf->cli_conflicts_with[a_idx]);
                if (rc_cw != CDD_C_SUCCESS)
                  return rc_cw;
                C_CDD_FREE(nf->cli_conflicts_with[a_idx]);
                nf->cli_conflicts_with[a_idx] = NULL;
              }
            }
            if (nf->cli_conflicts_with) {
              C_CDD_FREE(nf->cli_conflicts_with);
              nf->cli_conflicts_with = NULL;
              nf->n_cli_conflicts_with = 0;
            }
            for (a_idx = 0; a_idx < nf->n_cli_requires; ++a_idx) {
              if (nf->cli_requires[a_idx]) {
                cdd_c_error_t rc_rq =
                    struct_field_add_cli_requires(af, nf->cli_requires[a_idx]);
                if (rc_rq != CDD_C_SUCCESS)
                  return rc_rq;
                C_CDD_FREE(nf->cli_requires[a_idx]);
                nf->cli_requires[a_idx] = NULL;
              }
            }
            if (nf->cli_requires) {
              C_CDD_FREE(nf->cli_requires);
              nf->cli_requires = NULL;
              nf->n_cli_requires = 0;
            }
            for (a_idx = 0; a_idx < nf->n_cli_required_unless; ++a_idx) {
              if (nf->cli_required_unless[a_idx]) {
                cdd_c_error_t rc_ru = struct_field_add_cli_required_unless(
                    af, nf->cli_required_unless[a_idx]);
                if (rc_ru != CDD_C_SUCCESS)
                  return rc_ru;
                C_CDD_FREE(nf->cli_required_unless[a_idx]);
                nf->cli_required_unless[a_idx] = NULL;
              }
            }
            if (nf->cli_required_unless) {
              C_CDD_FREE(nf->cli_required_unless);
              nf->cli_required_unless = NULL;
              nf->n_cli_required_unless = 0;
            }
            if (nf->schema_extra_json)
              C_CDD_FREE(nf->schema_extra_json);
            if (nf->items_extra_json)
              C_CDD_FREE(nf->items_extra_json);
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
