/* To be appended to src/classes/parse/cdd_c_ir.c */

int parse_sql_into_ir(const char *sql_data, cdd_c_ir_t *out_ir) {
  struct sql_token_list_t *list = NULL;
  struct sql_table_t *table = NULL;
  cdd_c_query_projection_t *proj = NULL;
  struct sql_parse_error_t err;
  az_span span;
  int rc;
  size_t i;
  size_t start_idx = 0;
  int in_table = 0;
  int in_select = 0;

  if (!sql_data || !out_ir)
    return -1;

  span = az_span_create_from_str((char *)sql_data);
  rc = sql_lex(span, &list);
  if (rc != 0)
    return rc;

  for (i = 0; i < list->size; ++i) {
    if (!in_table && !in_select && list->tokens[i].kind == SQL_TOKEN_KEYWORD) {
      if (list->tokens[i].length == 6 &&
          strncmp(list->tokens[i].start, "CREATE", 6) == 0) {
        start_idx = i;
        in_table = 1;
      } else if (list->tokens[i].length == 6 &&
                 strncmp(list->tokens[i].start, "SELECT", 6) == 0) {
        start_idx = i;
        in_select = 1;
      }
    }

    if ((in_table || in_select) &&
        list->tokens[i].kind == SQL_TOKEN_SEMICOLON) {
      struct sql_token_list_t sublist;
      sublist.tokens = &list->tokens[start_idx];
      sublist.size = i - start_idx + 1;
      sublist.capacity = sublist.size;

      if (in_table) {
        table = NULL;
        rc = sql_parse_table(&sublist, &table, &err);
        if (rc == 0 && table) {
          cdd_c_ir_add_table(out_ir, table);
          sql_table_free(table);
        }
        in_table = 0;
      } else if (in_select) {
        proj = NULL;
        rc = sql_parse_select(&sublist, &proj, &err);
        if (rc == 0 && proj) {
          cdd_c_ir_add_projection(out_ir, proj);
          cdd_c_query_projection_free(proj);
          free(proj);
        }
        in_select = 0;
      }
    }
  }

  /* Process RETURNING if present, could be attached to INSERT/UPDATE/DELETE */
  /* For now, parse them as separate projections if standalone RETURNING keyword
   * appears */
  {
    struct sql_token_list_t sublist;
    proj = NULL;
    rc = sql_parse_returning(list, &proj, &err);
    if (rc == 0 && proj) {
      cdd_c_ir_add_projection(out_ir, proj);
      cdd_c_query_projection_free(proj);
      free(proj);
    }
  }

  sql_token_list_free(list);
  return 0;
}
