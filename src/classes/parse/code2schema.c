/**
 * @file code2schema.c
 * @brief CLI entry point and top-level orchestration for code2schema.
 * @author Samuel Marks
 */

/* clang-format off */
#include "code2schema_internal.h"
/* clang-format on */

/**
 * @brief Retrieves token kind at index or `TOKEN_UNKNOWN` if out of bounds.
 *
 * @param[in] tl Token list to inspect.
 * @param[in] idx Token index to look up.
 * @param[out] out_kind Output pointer receiving the token kind.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_tok_kind(const struct TokenList *tl, size_t idx,
                                  enum TokenKind *out_kind) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_kind = (idx < tl->size) ? tl->tokens[idx].kind : TOKEN_UNKNOWN;
  return CDD_C_SUCCESS;
}

/**
 * @brief Advances token index past whitespace and comments.
 *
 * @param[in] tl Token list.
 * @param[in,out] idx Pointer to current token index.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_skip_ws(const struct TokenList *tl, size_t *idx) {
  enum TokenKind k = TOKEN_UNKNOWN;
  cdd_c_error_t rc;
  for (;;) {
    rc = c2s_tok_kind(tl, *idx, &k);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (k != TOKEN_WHITESPACE && k != TOKEN_COMMENT)
      break;
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
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_skip_ws_collect_comments(const struct TokenList *tl,
                                                  size_t *idx,
                                                  char *comment_buf,
                                                  size_t buf_sz) {
  enum TokenKind k = TOKEN_UNKNOWN;
  size_t pos = 0;
  cdd_c_error_t rc;
  comment_buf[0] = '\0';
  for (;;) {
    rc = c2s_tok_kind(tl, *idx, &k);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (k != TOKEN_WHITESPACE && k != TOKEN_COMMENT)
      break;
    if (k == TOKEN_COMMENT && pos + 2 < buf_sz) {
      const struct Token *t = &tl->tokens[*idx];
      size_t c_idx;
      for (c_idx = 0; c_idx < t->length && pos + 2 < buf_sz; ++c_idx) {
        comment_buf[pos++] = (char)t->start[c_idx];
      }
      comment_buf[pos++] = 10;
      comment_buf[pos] = '\0';
    }
    (*idx)++;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Copies up to 63 characters from a token into a 64-byte buffer.
 *
 * @param[out] dst Destination 64-byte character buffer.
 * @param[in] tok Source token to copy from.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_copy_token_ident(char *dst, const struct Token *tok) {
  size_t nlen;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  nlen = tok->length < 63 ? tok->length : 63;
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
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_extract_member_decl(const struct TokenList *tl,
                                             size_t *idx, char *out_line) {
  size_t i = *idx;
  size_t pos = 0;
  enum TokenKind k = TOKEN_UNKNOWN;
  cdd_c_error_t rc;

  out_line[0] = '\0';
  for (;;) {
    rc = c2s_tok_kind(tl, i, &k);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if ((k == TOKEN_UNKNOWN) | (k == TOKEN_SEMICOLON) | (k == TOKEN_RBRACE))
      break;
    {
      const struct Token *t = &tl->tokens[i];
      if (t->kind == TOKEN_WHITESPACE) {
        out_line[pos++] = ' ';
      } else {
        memcpy(out_line + pos, t->start, t->length);
        pos += t->length;
      }
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
    rc = c2s_tok_kind(tl, look, &k);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (k == TOKEN_WHITESPACE) {
      if (memchr(tl->tokens[look].start, 10, tl->tokens[look].length) != NULL) {
        has_newline = 1;
      } else {
        look++;
      }
    }
    rc = c2s_tok_kind(tl, look, &k);
    if (rc != CDD_C_SUCCESS)
      return rc;
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
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t c2s_skip_brace_block(const struct TokenList *tl,
                                          size_t *idx) {
  int depth = 1;
  enum TokenKind k = TOKEN_UNKNOWN;
  cdd_c_error_t rc;
  while (depth > 0) {
    rc = c2s_tok_kind(tl, *idx, &k);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (k == TOKEN_UNKNOWN)
      break;
    if (k == TOKEN_LBRACE)
      depth++;
    else if (k == TOKEN_RBRACE)
      depth--;
    (*idx)++;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief CLI configuration for `code2schema_main`.
 */
struct cdd_code2schema_config {
  /** @brief Input header file path. */
  const char *input;
  /** @brief Output JSON Schema file path. */
  const char *output;
};

/** @brief Reflected fields for `cdd_code2schema_config`. */
static const struct cdd_field_info g_cdd_code2schema_config_fields[] = {
    {"input",
     "input",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct cdd_code2schema_config, input),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"output",
     "output",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct cdd_code2schema_config, output),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

/** @brief Reflected type descriptor for `cdd_code2schema_config`. */
static const struct cdd_type_info g_cdd_code2schema_config_type_info = {
    "cdd_code2schema_config",
    sizeof(struct cdd_code2schema_config),
    0,
    g_cdd_code2schema_config_fields,
    sizeof(g_cdd_code2schema_config_fields) /
        sizeof(g_cdd_code2schema_config_fields[0]),
    0};

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
  struct cdd_code2schema_config cfg;
  struct StructFields sf;
  struct StructFields nested_sf;
  struct StructFields enum_sf;
  int first_unk = -1;
  cdd_c_error_t ret_rc = CDD_C_SUCCESS;
  cdd_c_error_t rc_free = CDD_C_SUCCESS;

  memset(&cfg, 0, sizeof(cfg));
  memset(&sf, 0, sizeof(sf));
  memset(&nested_sf, 0, sizeof(nested_sf));
  memset(&enum_sf, 0, sizeof(enum_sf));

  ret_rc = cdd_json_value_init_object(&root);
  if (ret_rc != CDD_C_SUCCESS)
    goto cleanup;
  root_obj = root->object_val;

  ret_rc = cdd_json_value_init_object(&schemas_val);
  if (ret_rc != CDD_C_SUCCESS)
    goto cleanup;
  schemas_obj = schemas_val->object_val;

  ret_rc = cdd_json_value_init_object(&comp_val);
  if (ret_rc != CDD_C_SUCCESS)
    goto cleanup;
  comp_obj = comp_val->object_val;

  if (argc == 2 && argv && argv[0] && argv[1]) {
    ret_rc = cdd_cli_parse_reflected_argv(argc, argv, 0,
                                          &g_cdd_code2schema_config_type_info,
                                          &cfg, NULL, &first_unk);
    if (ret_rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  if (!cfg.output) {
    ret_rc = CDD_C_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp, cfg.input, "r") != 0)
    fp = NULL;
#else
  fp = fopen(cfg.input, "r");
#endif
  if (!fp) {
    ret_rc = CDD_C_ERROR_IO;
    goto cleanup;
  }

  ret_rc = cdd_json_object_set_value(comp_obj, "schemas", schemas_val);
  if (ret_rc != CDD_C_SUCCESS)
    goto cleanup;
  schemas_val = NULL;

  ret_rc = cdd_json_object_set_value(root_obj, "components", comp_val);
  if (ret_rc != CDD_C_SUCCESS)
    goto cleanup;
  comp_val = NULL;

  ret_rc = read_from_fh(fp, &content, &content_sz);
  if (ret_rc != CDD_C_SUCCESS)
    goto cleanup;
  fclose(fp);
  fp = NULL;

  ret_rc = tokenize(az_span_create_from_str_of_size(content, content_sz), &tl);
  if (ret_rc != CDD_C_SUCCESS)
    goto cleanup;

  while (i < tl->size) {
    enum TokenKind k;
    char top_comments[MAX_LINE_LENGTH];
    ret_rc = c2s_skip_ws_collect_comments(tl, &i, top_comments,
                                          sizeof(top_comments));
    if (ret_rc != CDD_C_SUCCESS)
      goto cleanup;
    if (i >= tl->size)
      break;

    k = tl->tokens[i].kind;
    if (k == TOKEN_KEYWORD_UNION) {
      i++;
      ret_rc = c2s_skip_ws(tl, &i);
      if (ret_rc != CDD_C_SUCCESS)
        goto cleanup;
      ret_rc = c2s_tok_kind(tl, i, &k);
      if (ret_rc != CDD_C_SUCCESS)
        goto cleanup;
      if (k == TOKEN_IDENTIFIER) {
        char union_name[64] = {0};
        ret_rc = c2s_copy_token_ident(union_name, &tl->tokens[i]);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
        i++;
        ret_rc = c2s_skip_ws(tl, &i);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
        ret_rc = c2s_tok_kind(tl, i, &k);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
        if (k == TOKEN_LBRACE) {
          i++;
          ret_rc =
              c2s_parse_union_tokens_and_write(tl, &i, schemas_obj, union_name);
          if (ret_rc != CDD_C_SUCCESS)
            goto cleanup;
        }
      } else if (k == TOKEN_LBRACE) {
        i++;
        ret_rc = c2s_skip_brace_block(tl, &i);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    } else if (k == TOKEN_KEYWORD_STRUCT) {
      i++;
      ret_rc = c2s_skip_ws(tl, &i);
      if (ret_rc != CDD_C_SUCCESS)
        goto cleanup;
      ret_rc = c2s_tok_kind(tl, i, &k);
      if (ret_rc != CDD_C_SUCCESS)
        goto cleanup;
      if (k == TOKEN_IDENTIFIER) {
        char struct_name[64] = {0};
        ret_rc = c2s_copy_token_ident(struct_name, &tl->tokens[i]);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
        i++;
        ret_rc = c2s_skip_ws(tl, &i);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
        ret_rc = c2s_tok_kind(tl, i, &k);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
        if (k == TOKEN_LBRACE) {
          i++;
          ret_rc = struct_fields_init(&sf);
          if (ret_rc != CDD_C_SUCCESS)
            goto cleanup;
          if (top_comments[0] != '\0') {
            ret_rc = cdd_serde_parse_container_comment(top_comments, &sf);
            if (ret_rc != CDD_C_SUCCESS)
              goto cleanup;
          }
          while (1) {
            char field_comments[MAX_LINE_LENGTH];
            ret_rc = c2s_skip_ws_collect_comments(tl, &i, field_comments,
                                                  sizeof(field_comments));
            if (ret_rc != CDD_C_SUCCESS)
              goto cleanup;
            ret_rc = c2s_tok_kind(tl, i, &k);
            if (ret_rc != CDD_C_SUCCESS)
              goto cleanup;
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
              ret_rc = c2s_skip_ws(tl, &look);
              if (ret_rc != CDD_C_SUCCESS)
                goto cleanup;
              ret_rc = c2s_tok_kind(tl, look, &k);
              if (ret_rc != CDD_C_SUCCESS)
                goto cleanup;
              if (k == TOKEN_LBRACE) {
                char nested_name[128];
                char nested_prop_name[64] = {0};
                ret_rc = struct_fields_init(&nested_sf);
                if (ret_rc != CDD_C_SUCCESS)
                  goto cleanup;
                i = look + 1;
                while (1) {
                  char member_line[MAX_LINE_LENGTH];
                  char nested_f_comments[MAX_LINE_LENGTH];
                  size_t prev_nested_sz;
                  ret_rc = c2s_skip_ws_collect_comments(
                      tl, &i, nested_f_comments, sizeof(nested_f_comments));
                  if (ret_rc != CDD_C_SUCCESS)
                    goto cleanup;
                  ret_rc = c2s_tok_kind(tl, i, &k);
                  if (ret_rc != CDD_C_SUCCESS)
                    goto cleanup;
                  if (k == TOKEN_UNKNOWN)
                    break;
                  if (k == TOKEN_RBRACE) {
                    i++;
                    ret_rc = c2s_skip_ws(tl, &i);
                    if (ret_rc != CDD_C_SUCCESS)
                      goto cleanup;
                    ret_rc = c2s_tok_kind(tl, i, &k);
                    if (ret_rc != CDD_C_SUCCESS)
                      goto cleanup;
                    if (k == TOKEN_IDENTIFIER) {
                      ret_rc = c2s_copy_token_ident(nested_prop_name,
                                                    &tl->tokens[i]);
                      if (ret_rc != CDD_C_SUCCESS)
                        goto cleanup;
                      i++;
                      ret_rc = c2s_skip_ws(tl, &i);
                      if (ret_rc != CDD_C_SUCCESS)
                        goto cleanup;
                      ret_rc = c2s_tok_kind(tl, i, &k);
                      if (ret_rc != CDD_C_SUCCESS)
                        goto cleanup;
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
                  ret_rc = c2s_extract_member_decl(tl, &i, member_line);
                  if (ret_rc != CDD_C_SUCCESS)
                    goto cleanup;
                  prev_nested_sz = nested_sf.size;
                  ret_rc = parse_struct_member_line(member_line, &nested_sf);
                  if (ret_rc != CDD_C_SUCCESS)
                    goto cleanup;
                  if (nested_sf.size > prev_nested_sz &&
                      nested_f_comments[0] != '\0') {
                    ret_rc = cdd_serde_parse_field_comment(
                        nested_f_comments,
                        &nested_sf.fields[nested_sf.size - 1]);
                    if (ret_rc != CDD_C_SUCCESS)
                      goto cleanup;
                  }
                }
                CDD_SNPRINTF(nested_name, sizeof(nested_name), "%s_%s",
                             struct_name, nested_prop_name);
                ret_rc = c2s_collapse_arrays(&nested_sf);
                if (ret_rc != CDD_C_SUCCESS)
                  goto cleanup;
                ret_rc = write_struct_to_json_schema(schemas_obj, nested_name,
                                                     &nested_sf);
                if (ret_rc != CDD_C_SUCCESS)
                  goto cleanup;
                ret_rc = struct_fields_add(&sf, nested_prop_name, "object",
                                           nested_name, NULL, NULL);
                if (ret_rc != CDD_C_SUCCESS)
                  goto cleanup;
                if (field_comments[0] != '\0') {
                  ret_rc = cdd_serde_parse_field_comment(
                      field_comments, &sf.fields[sf.size - 1]);
                  if (ret_rc != CDD_C_SUCCESS)
                    goto cleanup;
                }
                rc_free = struct_fields_free(&nested_sf);
                continue;
              }
            }
            {
              char member_line[MAX_LINE_LENGTH];
              size_t prev_sz = sf.size;
              ret_rc = c2s_extract_member_decl(tl, &i, member_line);
              if (ret_rc != CDD_C_SUCCESS)
                goto cleanup;
              ret_rc = parse_struct_member_line(member_line, &sf);
              if (ret_rc != CDD_C_SUCCESS)
                goto cleanup;
              if (sf.size > prev_sz && field_comments[0] != '\0') {
                ret_rc = cdd_serde_parse_field_comment(field_comments,
                                                       &sf.fields[sf.size - 1]);
                if (ret_rc != CDD_C_SUCCESS)
                  goto cleanup;
              }
            }
          }
          ret_rc = c2s_collapse_arrays(&sf);
          if (ret_rc != CDD_C_SUCCESS)
            goto cleanup;
          ret_rc = write_struct_to_json_schema(schemas_obj, struct_name, &sf);
          if (ret_rc != CDD_C_SUCCESS)
            goto cleanup;
          rc_free = struct_fields_free(&sf);
        }
      } else if (k == TOKEN_LBRACE) {
        i++;
        ret_rc = c2s_skip_brace_block(tl, &i);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    } else if (k == TOKEN_KEYWORD_ENUM) {
      i++;
      ret_rc = c2s_skip_ws(tl, &i);
      if (ret_rc != CDD_C_SUCCESS)
        goto cleanup;
      ret_rc = c2s_tok_kind(tl, i, &k);
      if (ret_rc != CDD_C_SUCCESS)
        goto cleanup;
      if (k == TOKEN_IDENTIFIER) {
        char enum_name[64] = {0};
        ret_rc = c2s_copy_token_ident(enum_name, &tl->tokens[i]);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
        i++;
        ret_rc = c2s_skip_ws(tl, &i);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
        ret_rc = c2s_tok_kind(tl, i, &k);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
        if (k == TOKEN_LBRACE) {
          size_t ename_len = strlen(enum_name);
          i++;
          ret_rc = struct_fields_init(&enum_sf);
          if (ret_rc != CDD_C_SUCCESS)
            goto cleanup;
          enum_sf.is_enum = 1;
          ret_rc = enum_members_init(&enum_sf.enum_members);
          if (ret_rc != CDD_C_SUCCESS)
            goto cleanup;
          if (top_comments[0] != '\0') {
            ret_rc = cdd_serde_parse_container_comment(top_comments, &enum_sf);
            if (ret_rc != CDD_C_SUCCESS)
              goto cleanup;
            ret_rc = cdd_serde_parse_enum_comment(top_comments,
                                                  &enum_sf.enum_members);
            if (ret_rc != CDD_C_SUCCESS)
              goto cleanup;
          }
          for (;;) {
            char var_comments[MAX_LINE_LENGTH];
            size_t vc_pos = 0;
            ret_rc = c2s_skip_ws_collect_comments(tl, &i, var_comments,
                                                  sizeof(var_comments));
            if (ret_rc != CDD_C_SUCCESS)
              goto cleanup;
            vc_pos = strlen(var_comments);
            ret_rc = c2s_tok_kind(tl, i, &k);
            if (ret_rc != CDD_C_SUCCESS)
              goto cleanup;
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

              ret_rc = c2s_copy_token_ident(ev_name, &tl->tokens[i]);
              if (ret_rc != CDD_C_SUCCESS)
                goto cleanup;
              member_tok = ev_name;
              if (strncmp(ev_name, enum_name, ename_len) == 0 &&
                  ev_name[ename_len] == '_' && ev_name[ename_len + 1] != '\0') {
                member_tok = ev_name + ename_len + 1;
              }
              i++;
              for (;;) {
                ret_rc = c2s_tok_kind(tl, i, &k);
                if (ret_rc != CDD_C_SUCCESS)
                  goto cleanup;
                if (k == TOKEN_UNKNOWN || k == TOKEN_COMMA || k == TOKEN_RBRACE)
                  break;
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
                enum TokenKind k_look = TOKEN_UNKNOWN;
                int has_nl = 0;
                ret_rc = c2s_tok_kind(tl, look, &k_look);
                if (ret_rc != CDD_C_SUCCESS)
                  goto cleanup;
                if (k_look == TOKEN_WHITESPACE) {
                  if (memchr(tl->tokens[look].start, 10,
                             tl->tokens[look].length) != NULL)
                    has_nl = 1;
                  else
                    look++;
                }
                ret_rc = c2s_tok_kind(tl, look, &k_look);
                if (ret_rc != CDD_C_SUCCESS)
                  goto cleanup;
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
                if (ret_rc != CDD_C_SUCCESS)
                  goto cleanup;
              }
              if (var_rename[0] != '\0') {
                CDD_STRCPY(effective_rename, sizeof(effective_rename),
                           var_rename);
              } else if (var_case != CDD_SERDE_CASE_NONE ||
                         var_rep_from[0] != '\0') {
                char tmp_v[128];
                CDD_STRCPY(tmp_v, sizeof(tmp_v), member_tok);
                if (var_case != CDD_SERDE_CASE_NONE) {
                  ret_rc = cdd_serde_transform_case(tmp_v, var_case,
                                                    effective_rename,
                                                    sizeof(effective_rename));
                  if (ret_rc != CDD_C_SUCCESS)
                    goto cleanup;
                  CDD_STRCPY(tmp_v, sizeof(tmp_v), effective_rename);
                }
                if (var_rep_from[0] != '\0') {
                  ret_rc = cdd_serde_replace_str(tmp_v, var_rep_from,
                                                 var_rep_to, effective_rename,
                                                 sizeof(effective_rename));
                  if (ret_rc != CDD_C_SUCCESS)
                    goto cleanup;
                }
              }
              ret_rc = enum_members_add_ex(
                  &enum_sf.enum_members, member_tok,
                  effective_rename[0] ? effective_rename : NULL, var_skip);
              if (ret_rc != CDD_C_SUCCESS)
                goto cleanup;
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
                if (ret_rc != CDD_C_SUCCESS)
                  goto cleanup;
              }
            } else {
              i++;
            }
          }
          ret_rc =
              write_struct_to_json_schema(schemas_obj, enum_name, &enum_sf);
          if (ret_rc != CDD_C_SUCCESS)
            goto cleanup;
          rc_free = struct_fields_free(&enum_sf);
        }
      } else if (k == TOKEN_LBRACE) {
        i++;
        ret_rc = c2s_skip_brace_block(tl, &i);
        if (ret_rc != CDD_C_SUCCESS)
          goto cleanup;
      }
    } else {
      i++;
    }
  }

  ret_rc = cdd_json_serialize_to_file_pretty(root, cfg.output);
  if (ret_rc != CDD_C_SUCCESS)
    goto cleanup;
  ret_rc = CDD_C_SUCCESS;

cleanup:
  if (fp)
    fclose(fp);
  if (content)
    C_CDD_FREE(content);
  return cdd_c_keep_err(
      ret_rc,
      cdd_c_keep_err(
          rc_free,
          cdd_c_keep_err(
              struct_fields_free(&nested_sf),
              cdd_c_keep_err(
                  struct_fields_free(&sf),
                  cdd_c_keep_err(
                      struct_fields_free(&enum_sf),
                      cdd_c_keep_err(
                          token_list_free(tl),
                          cdd_c_keep_err(
                              cdd_json_value_free(schemas_val),
                              cdd_c_keep_err(cdd_json_value_free(comp_val),
                                             cdd_json_value_free(root)))))))));
}
