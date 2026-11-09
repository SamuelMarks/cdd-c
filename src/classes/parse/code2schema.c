/**
 * @file code2schema.c
 * @brief CLI entry point and top-level orchestration for code2schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/*
 * @brief Retrieves token kind at index or TOKEN_UNKNOWN if out of bounds.
 *
 * @param[in] tl Token list to inspect.
 * @param[in] idx Token index to look up.
 * @param[out] out_kind Output pointer receiving the token kind.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t c2s_tok_kind(const struct TokenList *tl, size_t idx,
                           enum TokenKind *out_kind) {
  *out_kind = (idx < tl->size) ? tl->tokens[idx].kind : TOKEN_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Advances token index past whitespace and comments.
 *
 * @param[in] tl Token list.
 * @param[in,out] idx Pointer to current token index.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t c2s_skip_ws(const struct TokenList *tl, size_t *idx) {
  enum TokenKind k;
  while ((g_cdd_last_alloc_err = c2s_tok_kind(tl, *idx, &k),
          k == TOKEN_WHITESPACE || k == TOKEN_COMMENT)) {
    (*idx)++;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Advances token index past whitespace and comments while collecting
 * any comment text into `comment_buf`.
 *
 * @param[in] tl Token list.
 * @param[in,out] idx Pointer to current token index.
 * @param[out] comment_buf Buffer receiving concatenated comment text.
 * @param[in] buf_sz Capacity of `comment_buf`.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t c2s_skip_ws_collect_comments(const struct TokenList *tl,
                                                  size_t *idx,
                                                  char *comment_buf,
                                                  size_t buf_sz) {
  enum TokenKind k;
  size_t pos = 0;
  comment_buf[0] = '\0';
  while ((g_cdd_last_alloc_err = c2s_tok_kind(tl, *idx, &k),
          k == TOKEN_WHITESPACE || k == TOKEN_COMMENT)) {
    if (k == TOKEN_COMMENT) {
      const struct Token *t = &tl->tokens[*idx];
      size_t c_idx;
      for (c_idx = 0; c_idx < t->length && pos + 2 < buf_sz; ++c_idx) {
        comment_buf[pos++] = (char)t->start[c_idx];
      }
      if (pos + 1 < buf_sz)
        comment_buf[pos++] = '\n';
      comment_buf[pos] = '\0';
    }
    (*idx)++;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Copies up to 63 characters from a token into a 64-byte buffer.
 *
 * @param[out] dst Destination 64-byte character buffer.
 * @param[in] tok Source token to copy from.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t c2s_copy_token_ident(char *dst, const struct Token *tok) {
  size_t nlen = tok->length < 63 ? tok->length : 63;
  memcpy(dst, tok->start, nlen);
  dst[nlen] = '\0';
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
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t c2s_extract_member_decl(const struct TokenList *tl,
                                             size_t *idx, char *out_line) {
  size_t i = *idx;
  size_t pos = 0;
  enum TokenKind k;

  out_line[0] = '\0';
  while ((
      g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k),
      !((k == TOKEN_UNKNOWN) | (k == TOKEN_SEMICOLON) | (k == TOKEN_RBRACE)))) {
    const struct Token *t = &tl->tokens[i];
    if (t->kind == TOKEN_WHITESPACE) {
      out_line[pos++] = ' ';
    } else {
      memcpy(out_line + pos, t->start, t->length);
      pos += t->length;
    }
    i++;
  }
  if (k == TOKEN_SEMICOLON) {
    size_t look;
    int has_newline = 0;
    out_line[pos++] = ';';
    i++;
    /* Check if there is a trailing comment on the same line */
    look = i;
    g_cdd_last_alloc_err = c2s_tok_kind(tl, look, &k);
    if (k == TOKEN_WHITESPACE) {
      if (memchr(tl->tokens[look].start, '\n', tl->tokens[look].length) !=
          NULL) {
        has_newline = 1;
      } else {
        look++;
      }
    }
    g_cdd_last_alloc_err = c2s_tok_kind(tl, look, &k);
    if (!has_newline && k == TOKEN_COMMENT) {
      const struct Token *ct = &tl->tokens[look];
      out_line[pos++] = ' ';
      memcpy(out_line + pos, ct->start, ct->length);
      pos += ct->length;
      i = look + 1;
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
  enum TokenKind k;
  while (depth > 0 && (g_cdd_last_alloc_err = c2s_tok_kind(tl, *idx, &k),
                       k != TOKEN_UNKNOWN)) {
    if (k == TOKEN_LBRACE)
      depth++;
    else if (k == TOKEN_RBRACE)
      depth--;
    (*idx)++;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief CLI entry point for code2schema command.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t code2schema_main(int argc, char **argv) {
  FILE *fp = NULL;
  char *content = NULL;
  size_t content_sz = 0;
  struct TokenList *tl = NULL;
  size_t i = 0;
  JSON_Value *root = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Value *schemas_val = NULL;
  JSON_Object *schemas_obj = NULL;
  JSON_Value *comp_val = NULL;
  JSON_Object *comp_obj = NULL;
  cdd_c_error_t ret_rc = CDD_C_SUCCESS;

  g_cdd_last_alloc_err = cdd_json_value_init_object(&root);
  g_cdd_last_alloc_err = cdd_json_value_get_object(root, &root_obj);
  g_cdd_last_alloc_err = cdd_json_value_init_object(&schemas_val);
  g_cdd_last_alloc_err = cdd_json_value_get_object(schemas_val, &schemas_obj);
  g_cdd_last_alloc_err = cdd_json_value_init_object(&comp_val);
  g_cdd_last_alloc_err = cdd_json_value_get_object(comp_val, &comp_obj);

  if (argc != 2 || !argv || !argv[0] || !argv[1]) {
    g_cdd_last_alloc_err = cdd_json_value_free(root);
    g_cdd_last_alloc_err = cdd_json_value_free(schemas_val);
    g_cdd_last_alloc_err = cdd_json_value_free(comp_val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp, argv[0], "r") != 0)
    fp = NULL;
#else
  fp = fopen(argv[0], "r");
#endif
  if (!fp) {
    g_cdd_last_alloc_err = cdd_json_value_free(root);
    g_cdd_last_alloc_err = cdd_json_value_free(schemas_val);
    g_cdd_last_alloc_err = cdd_json_value_free(comp_val);
    return CDD_C_ERROR_IO;
  }

  if (cdd_json_object_set_value(comp_obj, "schemas", schemas_val) !=
      CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = cdd_json_value_free(schemas_val);
    g_cdd_last_alloc_err = cdd_json_value_free(comp_val);
    g_cdd_last_alloc_err = cdd_json_value_free(root);
    fclose(fp);
    return CDD_C_ERROR_MEMORY;
  }
  if (cdd_json_object_set_value(root_obj, "components", comp_val) !=
      CDD_C_SUCCESS) {
    g_cdd_last_alloc_err = cdd_json_value_free(comp_val);
    g_cdd_last_alloc_err = cdd_json_value_free(root);
    fclose(fp);
    return CDD_C_ERROR_MEMORY;
  }

  ret_rc = read_from_fh(fp, &content, &content_sz);
  fclose(fp);
  fp = NULL;
  if (ret_rc != CDD_C_SUCCESS) {
    goto cleanup;
  }

  ret_rc = tokenize(az_span_create_from_str_of_size(content, content_sz), &tl);
  if (ret_rc != CDD_C_SUCCESS) {
    goto cleanup;
  }

  while (i < tl->size) {
    enum TokenKind k;
    char top_comments[MAX_LINE_LENGTH];
    g_cdd_last_alloc_err = c2s_skip_ws_collect_comments(tl, &i, top_comments,
                                                        sizeof(top_comments));
    if (i >= tl->size)
      break;

    g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
    if (k == TOKEN_KEYWORD_UNION) {
      i++;
      g_cdd_last_alloc_err = c2s_skip_ws(tl, &i);
      g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
      if (k == TOKEN_IDENTIFIER) {
        char union_name[64] = {0};
        g_cdd_last_alloc_err = c2s_copy_token_ident(union_name, &tl->tokens[i]);
        i++;
        g_cdd_last_alloc_err = c2s_skip_ws(tl, &i);
        g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
        if (k == TOKEN_LBRACE) {
          i++;
          g_cdd_last_alloc_err =
              c2s_parse_union_tokens_and_write(tl, &i, schemas_obj, union_name);
        }
      } else if (k == TOKEN_LBRACE) {
        i++;
        g_cdd_last_alloc_err = c2s_skip_brace_block(tl, &i);
      }
    } else if (k == TOKEN_KEYWORD_STRUCT) {
      i++;
      g_cdd_last_alloc_err = c2s_skip_ws(tl, &i);
      g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
      if (k == TOKEN_IDENTIFIER) {
        char struct_name[64] = {0};
        g_cdd_last_alloc_err =
            c2s_copy_token_ident(struct_name, &tl->tokens[i]);
        i++;
        g_cdd_last_alloc_err = c2s_skip_ws(tl, &i);
        g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
        if (k == TOKEN_LBRACE) {
          struct StructFields sf;
          i++;
          if (struct_fields_init(&sf) != CDD_C_SUCCESS) {
            ret_rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
          }
          if (top_comments[0] != '\0') {
            ret_rc = cdd_serde_parse_container_comment(top_comments, &sf);
            if (ret_rc != CDD_C_SUCCESS) {
              g_cdd_last_alloc_err = struct_fields_free(&sf);
              goto cleanup;
            }
          }
          while (1) {
            char field_comments[MAX_LINE_LENGTH];
            g_cdd_last_alloc_err = c2s_skip_ws_collect_comments(
                tl, &i, field_comments, sizeof(field_comments));
            g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
            if (k == TOKEN_UNKNOWN)
              break;
            if (k == TOKEN_RBRACE) {
              i++;
              break;
            }
            if (k == TOKEN_SEMICOLON) {
              i++;
              continue;
            }
            if (k == TOKEN_KEYWORD_STRUCT) {
              size_t look = i + 1;
              g_cdd_last_alloc_err = c2s_skip_ws(tl, &look);
              g_cdd_last_alloc_err = c2s_tok_kind(tl, look, &k);
              if (k == TOKEN_LBRACE) {
                struct StructFields nested_sf;
                char nested_name[128];
                char nested_prop_name[64] = {0};
                cdd_c_error_t rc_c2s;
                if (struct_fields_init(&nested_sf) != CDD_C_SUCCESS) {
                  g_cdd_last_alloc_err = struct_fields_free(&sf);
                  ret_rc = CDD_C_ERROR_MEMORY;
                  goto cleanup;
                }
                i = look + 1;
                while (1) {
                  char member_line[MAX_LINE_LENGTH];
                  char nested_f_comments[MAX_LINE_LENGTH];
                  size_t prev_nested_sz;
                  g_cdd_last_alloc_err = c2s_skip_ws_collect_comments(
                      tl, &i, nested_f_comments, sizeof(nested_f_comments));
                  g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
                  if (k == TOKEN_UNKNOWN)
                    break;
                  if (k == TOKEN_RBRACE) {
                    i++;
                    g_cdd_last_alloc_err = c2s_skip_ws(tl, &i);
                    g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
                    if (k == TOKEN_IDENTIFIER) {
                      g_cdd_last_alloc_err = c2s_copy_token_ident(
                          nested_prop_name, &tl->tokens[i]);
                      i++;
                      g_cdd_last_alloc_err = c2s_skip_ws(tl, &i);
                      g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
                      if (k == TOKEN_SEMICOLON) {
                        i++;
                      }
                    }
                    break;
                  }
                  if (k == TOKEN_SEMICOLON) {
                    i++;
                    continue;
                  }
                  g_cdd_last_alloc_err =
                      c2s_extract_member_decl(tl, &i, member_line);
                  prev_nested_sz = nested_sf.size;
                  rc_c2s = parse_struct_member_line(member_line, &nested_sf);
                  if (rc_c2s != CDD_C_SUCCESS) {
                    g_cdd_last_alloc_err = struct_fields_free(&nested_sf);
                    g_cdd_last_alloc_err = struct_fields_free(&sf);
                    ret_rc = rc_c2s;
                    goto cleanup;
                  }
                  if (nested_sf.size > prev_nested_sz &&
                      nested_f_comments[0] != '\0') {
                    rc_c2s = cdd_serde_parse_field_comment(
                        nested_f_comments,
                        &nested_sf.fields[nested_sf.size - 1]);
                    if (rc_c2s != CDD_C_SUCCESS) {
                      g_cdd_last_alloc_err = struct_fields_free(&nested_sf);
                      g_cdd_last_alloc_err = struct_fields_free(&sf);
                      ret_rc = rc_c2s;
                      goto cleanup;
                    }
                  }
                }
                CDD_SNPRINTF(nested_name, sizeof(nested_name), "%s_%s",
                             struct_name, nested_prop_name);
                g_cdd_last_alloc_err = c2s_collapse_arrays(&nested_sf);
                g_cdd_last_alloc_err = write_struct_to_json_schema(
                    schemas_obj, nested_name, &nested_sf);
                rc_c2s = struct_fields_add(&sf, nested_prop_name, "object",
                                           nested_name, NULL, NULL);
                if (rc_c2s != CDD_C_SUCCESS) {
                  g_cdd_last_alloc_err = struct_fields_free(&nested_sf);
                  g_cdd_last_alloc_err = struct_fields_free(&sf);
                  ret_rc = rc_c2s;
                  goto cleanup;
                }
                if (field_comments[0] != '\0') {
                  rc_c2s = cdd_serde_parse_field_comment(
                      field_comments, &sf.fields[sf.size - 1]);
                  if (rc_c2s != CDD_C_SUCCESS) {
                    g_cdd_last_alloc_err = struct_fields_free(&nested_sf);
                    g_cdd_last_alloc_err = struct_fields_free(&sf);
                    ret_rc = rc_c2s;
                    goto cleanup;
                  }
                }
                g_cdd_last_alloc_err = struct_fields_free(&nested_sf);
                continue;
              }
            }
            {
              char member_line[MAX_LINE_LENGTH];
              size_t prev_sz = sf.size;
              cdd_c_error_t rc_c2s;
              g_cdd_last_alloc_err =
                  c2s_extract_member_decl(tl, &i, member_line);
              rc_c2s = parse_struct_member_line(member_line, &sf);
              if (rc_c2s != CDD_C_SUCCESS) {
                g_cdd_last_alloc_err = struct_fields_free(&sf);
                ret_rc = rc_c2s;
                goto cleanup;
              }
              if (sf.size > prev_sz && field_comments[0] != '\0') {
                rc_c2s = cdd_serde_parse_field_comment(field_comments,
                                                       &sf.fields[sf.size - 1]);
                if (rc_c2s != CDD_C_SUCCESS) {
                  g_cdd_last_alloc_err = struct_fields_free(&sf);
                  ret_rc = rc_c2s;
                  goto cleanup;
                }
              }
            }
          }
          g_cdd_last_alloc_err = c2s_collapse_arrays(&sf);
          {
            cdd_c_error_t rc_c2s =
                write_struct_to_json_schema(schemas_obj, struct_name, &sf);
            if (rc_c2s != CDD_C_SUCCESS) {
              g_cdd_last_alloc_err = struct_fields_free(&sf);
              ret_rc = rc_c2s;
              goto cleanup;
            }
          }
          g_cdd_last_alloc_err = struct_fields_free(&sf);
        }
      } else if (k == TOKEN_LBRACE) {
        i++;
        g_cdd_last_alloc_err = c2s_skip_brace_block(tl, &i);
      }
    } else if (k == TOKEN_KEYWORD_ENUM) {
      i++;
      g_cdd_last_alloc_err = c2s_skip_ws(tl, &i);
      g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
      if (k == TOKEN_IDENTIFIER) {
        char enum_name[64] = {0};
        g_cdd_last_alloc_err = c2s_copy_token_ident(enum_name, &tl->tokens[i]);
        i++;
        g_cdd_last_alloc_err = c2s_skip_ws(tl, &i);
        g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
        if (k == TOKEN_LBRACE) {
          struct StructFields enum_sf;
          size_t ename_len = strlen(enum_name);
          i++;
          if (struct_fields_init(&enum_sf) != CDD_C_SUCCESS) {
            ret_rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
          }
          enum_sf.is_enum = 1;
          if (enum_members_init(&enum_sf.enum_members) != CDD_C_SUCCESS) {
            g_cdd_last_alloc_err = struct_fields_free(&enum_sf);
            ret_rc = CDD_C_ERROR_MEMORY;
            goto cleanup;
          }
          if (top_comments[0] != '\0') {
            ret_rc = cdd_serde_parse_container_comment(top_comments, &enum_sf);
            if (ret_rc == CDD_C_SUCCESS) {
              ret_rc = cdd_serde_parse_enum_comment(top_comments,
                                                    &enum_sf.enum_members);
            }
            if (ret_rc != CDD_C_SUCCESS) {
              g_cdd_last_alloc_err = struct_fields_free(&enum_sf);
              goto cleanup;
            }
          }
          while ((g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k),
                  k != TOKEN_UNKNOWN)) {
            char var_comments[MAX_LINE_LENGTH];
            size_t vc_pos = 0;
            g_cdd_last_alloc_err = c2s_skip_ws_collect_comments(
                tl, &i, var_comments, sizeof(var_comments));
            vc_pos = strlen(var_comments);
            g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k);
            if (k == TOKEN_UNKNOWN)
              break;
            if (k == TOKEN_RBRACE) {
              i++;
              break;
            }
            if (k == TOKEN_COMMA) {
              i++;
              continue;
            }
            if (k == TOKEN_IDENTIFIER) {
              char ev_name[64] = {0};
              const char *member_tok;
              char var_rename[64] = {0};
              int var_skip = 0;
              int var_other = 0;
              enum CddSerdeCaseRule var_case = CDD_SERDE_CASE_NONE;
              char var_rep_from[32] = {0};
              char var_rep_to[32] = {0};
              char effective_rename[128] = {0};

              g_cdd_last_alloc_err =
                  c2s_copy_token_ident(ev_name, &tl->tokens[i]);
              member_tok = ev_name;
              if (strncmp(ev_name, enum_name, ename_len) == 0 &&
                  ev_name[ename_len] == '_' && ev_name[ename_len + 1] != '\0') {
                member_tok = ev_name + ename_len + 1;
              }
              i++;
              while ((g_cdd_last_alloc_err = c2s_tok_kind(tl, i, &k),
                      k != TOKEN_UNKNOWN && k != TOKEN_COMMA &&
                          k != TOKEN_RBRACE)) {
                if (k == TOKEN_COMMENT) {
                  const struct Token *ct = &tl->tokens[i];
                  size_t c_k;
                  for (c_k = 0;
                       c_k < ct->length && vc_pos + 1 < sizeof(var_comments);
                       ++c_k) {
                    var_comments[vc_pos++] = (char)ct->start[c_k];
                  }
                  var_comments[vc_pos] = '\0';
                }
                i++;
              }
              if (k == TOKEN_COMMA) {
                size_t look = i + 1;
                enum TokenKind k_look;
                int has_nl = 0;
                g_cdd_last_alloc_err = c2s_tok_kind(tl, look, &k_look);
                if (k_look == TOKEN_WHITESPACE) {
                  if (memchr(tl->tokens[look].start, '\n',
                             tl->tokens[look].length) != NULL)
                    has_nl = 1;
                  else
                    look++;
                }
                g_cdd_last_alloc_err = c2s_tok_kind(tl, look, &k_look);
                if (!has_nl && k_look == TOKEN_COMMENT) {
                  const struct Token *ct = &tl->tokens[look];
                  size_t c_k;
                  for (c_k = 0;
                       c_k < ct->length && vc_pos + 1 < sizeof(var_comments);
                       ++c_k) {
                    var_comments[vc_pos++] = (char)ct->start[c_k];
                  }
                  var_comments[vc_pos] = '\0';
                  i = look + 1;
                } else {
                  i++;
                }
              }
              if (var_comments[0] != '\0') {
                ret_rc = cdd_serde_parse_enum_variant_comment_ex(
                    var_comments, var_rename, sizeof(var_rename), &var_skip,
                    &var_case, var_rep_from, sizeof(var_rep_from), var_rep_to,
                    sizeof(var_rep_to), &var_other, NULL, 0);
                if (ret_rc != CDD_C_SUCCESS) {
                  g_cdd_last_alloc_err = struct_fields_free(&enum_sf);
                  goto cleanup;
                }
              }
              if (var_rename[0] != '\0') {
                CDD_STRCPY(effective_rename, sizeof(effective_rename),
                           var_rename);
              } else if (var_case != CDD_SERDE_CASE_NONE ||
                         var_rep_from[0] != '\0') {
                char tmp_v[128];
                CDD_STRCPY(tmp_v, sizeof(tmp_v), member_tok);
                if (var_case != CDD_SERDE_CASE_NONE) {
                  g_cdd_last_alloc_err = cdd_serde_transform_case(
                      tmp_v, var_case, effective_rename,
                      sizeof(effective_rename));
                  CDD_STRCPY(tmp_v, sizeof(tmp_v), effective_rename);
                }
                if (var_rep_from[0] != '\0') {
                  g_cdd_last_alloc_err = cdd_serde_replace_str(
                      tmp_v, var_rep_from, var_rep_to, effective_rename,
                      sizeof(effective_rename));
                }
              }
              ret_rc = enum_members_add_ex(
                  &enum_sf.enum_members, member_tok,
                  effective_rename[0] ? effective_rename : NULL, var_skip);
              if (ret_rc != CDD_C_SUCCESS) {
                g_cdd_last_alloc_err = struct_fields_free(&enum_sf);
                goto cleanup;
              }
              if (var_other) {
                enum_sf.enum_members.serde_other_idx =
                    (int)(enum_sf.enum_members.size - 1);
              }
              if (var_comments[0] != '\0') {
                ret_rc = cdd_serde_parse_enum_variant_comment_ex(
                    var_comments, var_rename, sizeof(var_rename), &var_skip,
                    &var_case, var_rep_from, sizeof(var_rep_from), var_rep_to,
                    sizeof(var_rep_to), NULL, &enum_sf.enum_members,
                    enum_sf.enum_members.size - 1);
                if (ret_rc != CDD_C_SUCCESS) {
                  g_cdd_last_alloc_err = struct_fields_free(&enum_sf);
                  goto cleanup;
                }
              }
            } else {
              i++;
            }
          }
          g_cdd_last_alloc_err =
              write_struct_to_json_schema(schemas_obj, enum_name, &enum_sf);
          g_cdd_last_alloc_err = struct_fields_free(&enum_sf);
        }
      } else if (k == TOKEN_LBRACE) {
        i++;
        g_cdd_last_alloc_err = c2s_skip_brace_block(tl, &i);
      }
    } else {
      i++;
    }
  }

  if (cdd_json_serialize_to_file_pretty(root, argv[1]) != CDD_C_SUCCESS) {
    ret_rc = CDD_C_ERROR_IO;
    goto cleanup;
  }
  ret_rc = CDD_C_SUCCESS;

cleanup:
  if (tl)
    g_cdd_last_alloc_err = free_token_list(tl);
  if (content)
    C_CDD_FREE(content);
  g_cdd_last_alloc_err = cdd_json_value_free(root);
  return ret_rc;
}
