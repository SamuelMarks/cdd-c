/**
 * @file code2schema.c
 * @brief CLI entry point and top-level orchestration for code2schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Advances token index past whitespace and comments.
 *
 * @param[in] tl Token list.
 * @param[in,out] idx Pointer to current token index.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t c2s_skip_ws(const struct TokenList *tl, size_t *idx) {
  while (*idx < tl->size && (tl->tokens[*idx].kind == TOKEN_WHITESPACE ||
                             tl->tokens[*idx].kind == TOKEN_COMMENT)) {
    (*idx)++;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts a member declaration string from tokens up to ';' or '}'.
 *
 * Also appends any trailing comment on the same line immediately following ';'.
 *
 * @param[in] tl Token list.
 * @param[in,out] idx Pointer to current token index.
 * @param[out] out_line Buffer to receive the member declaration line.
 * @param[in] out_sz Size of out_line buffer.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t c2s_extract_member_decl(const struct TokenList *tl,
                                             size_t *idx, char *out_line,
                                             size_t out_sz) {
  size_t i = *idx;
  size_t pos = 0;

  out_line[0] = '\0';
  while (i < tl->size && tl->tokens[i].kind != TOKEN_SEMICOLON &&
         tl->tokens[i].kind != TOKEN_RBRACE) {
    const struct Token *t = &tl->tokens[i];
    if (t->kind == TOKEN_WHITESPACE) {
      if (pos > 0 && out_line[pos - 1] != ' ' && pos + 1 < out_sz) {
        out_line[pos++] = ' ';
      }
    } else if (pos + t->length < out_sz) {
      memcpy(out_line + pos, t->start, t->length);
      pos += t->length;
    }
    i++;
  }
  if (i < tl->size && tl->tokens[i].kind == TOKEN_SEMICOLON) {
    if (pos + 1 < out_sz) {
      out_line[pos++] = ';';
    }
    i++;
    /* Check if there is a trailing comment on the same line */
    {
      size_t look = i;
      int has_newline = 0;
      if (look < tl->size && tl->tokens[look].kind == TOKEN_WHITESPACE) {
        if (memchr(tl->tokens[look].start, '\n', tl->tokens[look].length) !=
            NULL) {
          has_newline = 1;
        } else {
          look++;
        }
      }
      if (!has_newline && look < tl->size &&
          tl->tokens[look].kind == TOKEN_COMMENT) {
        const struct Token *ct = &tl->tokens[look];
        if (pos + 1 + ct->length < out_sz) {
          out_line[pos++] = ' ';
          memcpy(out_line + pos, ct->start, ct->length);
          pos += ct->length;
        }
        i = look + 1;
      }
    }
  }
  out_line[pos] = '\0';
  *idx = i;
  return CDD_C_SUCCESS;
}

/**
 * @brief Skips a balanced brace-enclosed block starting after the opening '{'.
 *
 * @param[in] tl Token list.
 * @param[in,out] idx Pointer to current token index.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t c2s_skip_brace_block(const struct TokenList *tl,
                                          size_t *idx) {
  int depth = 1;
  while (*idx < tl->size && depth > 0) {
    if (tl->tokens[*idx].kind == TOKEN_LBRACE)
      depth++;
    else if (tl->tokens[*idx].kind == TOKEN_RBRACE)
      depth--;
    (*idx)++;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief CLI entry point for code2schema command.
 */
cdd_c_error_t code2schema_main(int argc, char **argv) {
  FILE *fp = NULL;
  char *content = NULL;
  size_t content_sz = 0;
  struct TokenList *tl = NULL;
  size_t i = 0;
  JSON_Value *root = json_value_init_object();
  JSON_Object *root_obj = json_value_get_object(root);
  JSON_Value *schemas_val = json_value_init_object();
  JSON_Object *schemas_obj = json_value_get_object(schemas_val);
  JSON_Value *comp_val = json_value_init_object();
  JSON_Object *comp_obj = json_value_get_object(comp_val);
  cdd_c_error_t ret_rc = CDD_C_SUCCESS;

  if (argc != 2 || !argv || !argv[0] || !argv[1]) {
    json_value_free(root);
    json_value_free(schemas_val);
    json_value_free(comp_val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp, argv[0], "r") != 0)
    fp = NULL;
#else
  fp = fopen(argv[0], "r");
#endif
  if (!fp) {
    json_value_free(root);
    json_value_free(schemas_val);
    json_value_free(comp_val);
    return CDD_C_ERROR_IO;
  }

  json_object_set_value(comp_obj, "schemas", schemas_val);
  json_object_set_value(root_obj, "components", comp_val);

  ret_rc = read_from_fh(fp, &content, &content_sz);
  fclose(fp);
  fp = NULL;
  if (ret_rc != CDD_C_SUCCESS) {
    goto cleanup;
  }

  ret_rc = tokenize(az_span_create((uint8_t *)content, content_sz), &tl);
  if (ret_rc != CDD_C_SUCCESS) {
    goto cleanup;
  }

  while (i < tl->size) {
    c2s_skip_ws(tl, &i);
    if (i >= tl->size)
      break;

    if (tl->tokens[i].kind == TOKEN_KEYWORD_UNION) {
      i++;
      c2s_skip_ws(tl, &i);
      if (i < tl->size && tl->tokens[i].kind == TOKEN_IDENTIFIER) {
        char union_name[64] = {0};
        size_t nlen = tl->tokens[i].length < 63 ? tl->tokens[i].length : 63;
        memcpy(union_name, tl->tokens[i].start, nlen);
        union_name[nlen] = '\0';
        i++;
        c2s_skip_ws(tl, &i);
        if (i < tl->size && tl->tokens[i].kind == TOKEN_LBRACE) {
          i++;
          c2s_parse_union_tokens_and_write(tl, &i, schemas_obj, union_name);
        }
      } else if (i < tl->size && tl->tokens[i].kind == TOKEN_LBRACE) {
        i++;
        c2s_skip_brace_block(tl, &i);
      }
    } else if (tl->tokens[i].kind == TOKEN_KEYWORD_STRUCT) {
      i++;
      c2s_skip_ws(tl, &i);
      if (i < tl->size && tl->tokens[i].kind == TOKEN_IDENTIFIER) {
        char struct_name[64] = {0};
        size_t nlen = tl->tokens[i].length < 63 ? tl->tokens[i].length : 63;
        memcpy(struct_name, tl->tokens[i].start, nlen);
        struct_name[nlen] = '\0';
        i++;
        c2s_skip_ws(tl, &i);
        if (i < tl->size && tl->tokens[i].kind == TOKEN_LBRACE) {
          struct StructFields sf;
          i++;
          if (struct_fields_init(&sf) != CDD_C_SUCCESS) {
            ret_rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
          }
          while (i < tl->size) {
            c2s_skip_ws(tl, &i);
            if (i >= tl->size)
              break;
            if (tl->tokens[i].kind == TOKEN_RBRACE) {
              i++;
              break;
            }
            if (tl->tokens[i].kind == TOKEN_SEMICOLON) {
              i++;
              continue;
            }
            if (tl->tokens[i].kind == TOKEN_KEYWORD_STRUCT) {
              size_t look = i + 1;
              c2s_skip_ws(tl, &look);
              if (look < tl->size && tl->tokens[look].kind == TOKEN_LBRACE) {
                struct StructFields nested_sf;
                char nested_name[128];
                char nested_prop_name[64] = {0};
                cdd_c_error_t rc_c2s;
                if (struct_fields_init(&nested_sf) != CDD_C_SUCCESS) {
                  struct_fields_free(&sf);
                  ret_rc = CDD_C_ERROR_MEMORY;
                  goto cleanup;
                }
                i = look + 1;
                while (i < tl->size) {
                  char member_line[MAX_LINE_LENGTH];
                  c2s_skip_ws(tl, &i);
                  if (i >= tl->size)
                    break;
                  if (tl->tokens[i].kind == TOKEN_RBRACE) {
                    i++;
                    c2s_skip_ws(tl, &i);
                    if (i < tl->size &&
                        tl->tokens[i].kind == TOKEN_IDENTIFIER) {
                      size_t pn_len =
                          tl->tokens[i].length < 63 ? tl->tokens[i].length : 63;
                      memcpy(nested_prop_name, tl->tokens[i].start, pn_len);
                      nested_prop_name[pn_len] = '\0';
                      i++;
                      c2s_skip_ws(tl, &i);
                      if (i < tl->size &&
                          tl->tokens[i].kind == TOKEN_SEMICOLON) {
                        i++;
                      }
                    }
                    break;
                  }
                  if (tl->tokens[i].kind == TOKEN_SEMICOLON) {
                    i++;
                    continue;
                  }
                  c2s_extract_member_decl(tl, &i, member_line,
                                          sizeof(member_line));
                  if (member_line[0]) {
                    rc_c2s = parse_struct_member_line(member_line, &nested_sf);
                    if (rc_c2s != CDD_C_SUCCESS) {
                      struct_fields_free(&nested_sf);
                      struct_fields_free(&sf);
                      ret_rc = rc_c2s;
                      goto cleanup;
                    }
                  }
                }
                CDD_SNPRINTF(nested_name, sizeof(nested_name), "%s_%s",
                             struct_name, nested_prop_name);
                c2s_collapse_arrays(&nested_sf);
                write_struct_to_json_schema(schemas_obj, nested_name,
                                            &nested_sf);
                struct_fields_add(&sf, nested_prop_name, "object", nested_name,
                                  NULL, NULL);
                struct_fields_free(&nested_sf);
                continue;
              }
            }
            {
              char member_line[MAX_LINE_LENGTH];
              c2s_extract_member_decl(tl, &i, member_line, sizeof(member_line));
              if (member_line[0]) {
                cdd_c_error_t rc_c2s =
                    parse_struct_member_line(member_line, &sf);
                if (rc_c2s != CDD_C_SUCCESS) {
                  struct_fields_free(&sf);
                  ret_rc = rc_c2s;
                  goto cleanup;
                }
              }
            }
          }
          c2s_collapse_arrays(&sf);
          {
            cdd_c_error_t rc_c2s =
                write_struct_to_json_schema(schemas_obj, struct_name, &sf);
            if (rc_c2s != CDD_C_SUCCESS) {
              struct_fields_free(&sf);
              ret_rc = rc_c2s;
              goto cleanup;
            }
          }
          struct_fields_free(&sf);
        }
      } else if (i < tl->size && tl->tokens[i].kind == TOKEN_LBRACE) {
        i++;
        c2s_skip_brace_block(tl, &i);
      }
    } else if (tl->tokens[i].kind == TOKEN_KEYWORD_ENUM) {
      i++;
      c2s_skip_ws(tl, &i);
      if (i < tl->size && tl->tokens[i].kind == TOKEN_IDENTIFIER) {
        char enum_name[64] = {0};
        size_t nlen = tl->tokens[i].length < 63 ? tl->tokens[i].length : 63;
        memcpy(enum_name, tl->tokens[i].start, nlen);
        enum_name[nlen] = '\0';
        i++;
        c2s_skip_ws(tl, &i);
        if (i < tl->size && tl->tokens[i].kind == TOKEN_LBRACE) {
          JSON_Value *eval = json_value_init_object();
          JSON_Object *eobj = json_value_get_object(eval);
          JSON_Value *arrval = json_value_init_array();
          JSON_Array *earr = json_value_get_array(arrval);
          i++;
          while (i < tl->size) {
            c2s_skip_ws(tl, &i);
            if (i >= tl->size)
              break;
            if (tl->tokens[i].kind == TOKEN_RBRACE) {
              i++;
              break;
            }
            if (tl->tokens[i].kind == TOKEN_COMMA) {
              i++;
              continue;
            }
            if (tl->tokens[i].kind == TOKEN_IDENTIFIER) {
              char ev_name[64] = {0};
              size_t ev_len =
                  tl->tokens[i].length < 63 ? tl->tokens[i].length : 63;
              memcpy(ev_name, tl->tokens[i].start, ev_len);
              ev_name[ev_len] = '\0';
              json_array_append_string(earr, ev_name);
              i++;
              while (i < tl->size && tl->tokens[i].kind != TOKEN_COMMA &&
                     tl->tokens[i].kind != TOKEN_RBRACE) {
                i++;
              }
            } else {
              i++;
            }
          }
          json_object_set_string(eobj, "type", "string");
          json_object_set_value(eobj, "enum", arrval);
          json_object_set_value(schemas_obj, enum_name, eval);
        }
      } else if (i < tl->size && tl->tokens[i].kind == TOKEN_LBRACE) {
        i++;
        c2s_skip_brace_block(tl, &i);
      }
    } else {
      i++;
    }
  }

  if (json_serialize_to_file_pretty(root, argv[1]) != JSONSuccess) {
    ret_rc = CDD_C_ERROR_IO;
    goto cleanup;
  }
  ret_rc = CDD_C_SUCCESS;

cleanup:
  if (tl)
    free_token_list(tl);
  if (content)
    C_CDD_FREE(content);
  json_value_free(root);
  return ret_rc;
}
