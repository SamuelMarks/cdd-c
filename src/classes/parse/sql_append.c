/* To be appended to src/classes/parse/sql.c */

int sql_parse_select(const struct sql_token_list_t *list,
                     struct CddCQueryProjection **out_proj,
                     struct sql_parse_error_t *out_error) {
  struct SqlParserState state;
  struct CddCQueryProjection *proj = NULL;
  struct sql_token_t *_ast_sql_parser_peek;

  state.list = list;
  state.cursor = 0;
  state.out_error = out_error;

  if (out_error) {
    out_error->message = NULL;
    out_error->token = NULL;
  }

  if (!sql_parser_match_keyword(&state, "SELECT")) {
    return sql_parser_set_error(&state, "Expected 'SELECT'");
  }

  proj = (struct CddCQueryProjection *)calloc(
      1, sizeof(struct CddCQueryProjection));
  if (!proj)
    return sql_parser_set_error(&state, "OOM allocating projection");
  cdd_c_query_projection_init(proj);

  while (1) {
    cdd_c_query_projection_field_t field;
    const struct sql_token_t *tok;
    int is_agg = 0;
    char *base_name = NULL;

    memset(&field, 0, sizeof(field));
    field.type = SQL_TYPE_UNKNOWN;

    tok =
        (sql_parser_peek(&state, &_ast_sql_parser_peek), _ast_sql_parser_peek);
    if (!tok)
      break;

    if (tok->kind == SQL_TOKEN_KEYWORD &&
        ((tok->length == 5 && strncmp(tok->start, "COUNT", 5) == 0) ||
         (tok->length == 3 && strncmp(tok->start, "MAX", 3) == 0) ||
         (tok->length == 3 && strncmp(tok->start, "MIN", 3) == 0) ||
         (tok->length == 3 && strncmp(tok->start, "SUM", 3) == 0) ||
         (tok->length == 3 && strncmp(tok->start, "AVG", 3) == 0))) {

      is_agg = 1;
      base_name = (char *)malloc(tok->length + 1);
      memcpy(base_name, tok->start, tok->length);
      base_name[tok->length] = '\0';

      /* Infer type based on aggregate */
      if (strncmp(base_name, "COUNT", 5) == 0) {
        field.type = SQL_TYPE_BIGINT; /* COUNT usually returns BIGINT */
      }

      sql_parser_consume(&state);

      if (sql_parser_match_kind(&state, SQL_TOKEN_LPAREN, NULL)) {
        /* just skip over until RPAREN for now, we don't deeply parse the
         * expression */
        while ((tok = (sql_parser_peek(&state, &_ast_sql_parser_peek),
                       _ast_sql_parser_peek)) != NULL) {
          sql_parser_consume(&state);
          if (tok->kind == SQL_TOKEN_RPAREN)
            break;
        }
      }
    } else if (sql_parser_match_kind(&state, SQL_TOKEN_IDENTIFIER, &tok)) {
      base_name = (char *)malloc(tok->length + 1);
      memcpy(base_name, tok->start, tok->length);
      base_name[tok->length] = '\0';
    } else if (sql_parser_match_kind(&state, SQL_TOKEN_KEYWORD, &tok)) {
      /* Sometimes columns are named after keywords, or 'FROM' is reached */
      if (tok->length == 4 && strncmp(tok->start, "FROM", 4) == 0) {
        /* End of projection fields */
        break;
      }
      base_name = (char *)malloc(tok->length + 1);
      memcpy(base_name, tok->start, tok->length);
      base_name[tok->length] = '\0';
    } else {
      break; /* Might be * or something we don't support yet */
    }

    field.is_aggregate = is_agg;

    /* Check for AS alias */
    if (sql_parser_match_keyword(&state, "AS")) {
      const struct sql_token_t *alias_tok;
      if (sql_parser_match_kind(&state, SQL_TOKEN_IDENTIFIER, &alias_tok) ||
          sql_parser_match_kind(&state, SQL_TOKEN_STRING, &alias_tok)) {
        field.name = (char *)malloc(alias_tok->length + 1);
        memcpy(field.name, alias_tok->start, alias_tok->length);
        field.name[alias_tok->length] = '\0';
        field.original_name = base_name;
      } else {
        free(base_name);
        cdd_c_query_projection_free(proj);
        free(proj);
        return sql_parser_set_error(&state, "Expected alias after AS");
      }
    } else {
      field.name = base_name;
      field.original_name = NULL;
    }

    if (cdd_c_query_projection_add_field(proj, &field) != 0) {
      if (field.name)
        free(field.name);
      if (field.original_name)
        free(field.original_name);
      cdd_c_query_projection_free(proj);
      free(proj);
      return sql_parser_set_error(&state, "Failed to add field to projection");
    }

    if (field.name)
      free(field.name);
    if (field.original_name)
      free(field.original_name);

    if (sql_parser_match_kind(&state, SQL_TOKEN_COMMA, NULL)) {
      continue;
    } else {
      break;
    }
  }

  /* Parse FROM */
  if (sql_parser_match_keyword(&state, "FROM")) {
    const struct sql_token_t *table_tok;
    if (sql_parser_match_kind(&state, SQL_TOKEN_IDENTIFIER, &table_tok)) {
      proj->source_table = (char *)malloc(table_tok->length + 1);
      memcpy(proj->source_table, table_tok->start, table_tok->length);
      proj->source_table[table_tok->length] = '\0';
    }
  }

  *out_proj = proj;
  return 0;
}

int sql_parse_returning(const struct sql_token_list_t *list,
                        struct CddCQueryProjection **out_proj,
                        struct sql_parse_error_t *out_error) {
  /* Find the RETURNING keyword and parse what follows like a SELECT projection
   */
  struct SqlParserState state;
  size_t i;
  int found = 0;

  state.list = list;
  state.cursor = 0;
  state.out_error = out_error;

  for (i = 0; i < list->size; ++i) {
    if (list->tokens[i].kind == SQL_TOKEN_KEYWORD &&
        list->tokens[i].length == 9 &&
        strncmp(list->tokens[i].start, "RETURNING", 9) == 0) {
      state.cursor = i + 1;
      found = 1;
      break;
    }
  }

  if (!found) {
    return sql_parser_set_error(&state, "RETURNING clause not found");
  }

  /* We can reuse the same projection building logic, but we pretend it's SELECT
   */
  {
    struct CddCQueryProjection *proj = (struct CddCQueryProjection *)calloc(
        1, sizeof(struct CddCQueryProjection));
    struct sql_token_t *_ast_sql_parser_peek;
    if (!proj)
      return sql_parser_set_error(&state, "OOM allocating projection");
    cdd_c_query_projection_init(proj);

    while (1) {
      cdd_c_query_projection_field_t field;
      const struct sql_token_t *tok;
      int is_agg = 0;
      char *base_name = NULL;

      memset(&field, 0, sizeof(field));
      field.type = SQL_TYPE_UNKNOWN;

      tok = (sql_parser_peek(&state, &_ast_sql_parser_peek),
             _ast_sql_parser_peek);
      if (!tok || tok->kind == SQL_TOKEN_SEMICOLON)
        break;

      if (sql_parser_match_kind(&state, SQL_TOKEN_IDENTIFIER, &tok)) {
        base_name = (char *)malloc(tok->length + 1);
        memcpy(base_name, tok->start, tok->length);
        base_name[tok->length] = '\0';
      } else {
        break;
      }

      field.is_aggregate = is_agg;
      field.name = base_name;
      field.original_name = NULL;

      if (cdd_c_query_projection_add_field(proj, &field) != 0) {
        if (field.name)
          free(field.name);
        cdd_c_query_projection_free(proj);
        free(proj);
        return sql_parser_set_error(&state,
                                    "Failed to add field to projection");
      }

      if (field.name)
        free(field.name);

      if (sql_parser_match_kind(&state, SQL_TOKEN_COMMA, NULL)) {
        continue;
      } else {
        break;
      }
    }

    *out_proj = proj;
  }
  return 0;
}
