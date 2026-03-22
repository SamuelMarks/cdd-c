/**
 * @file sql.c
 * @brief Parses SQL DDL into an AST.
 */

/* clang-format off */
#include "classes/parse/sql.h"
#include "classes/parse/query_projection.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Helper to check if a string is a SQL keyword.
 * @param str The string to check.
 * @param len The length of the string.
 * @return 1 if keyword, 0 otherwise.
 */
static int is_sql_keyword(const char *str, size_t len) {
  /* Simple exact matching for now; can be expanded. */
  static const char *keywords[] = {
      "CREATE",     "TABLE",  "INT",       "INTEGER", "BIGINT",  "VARCHAR",
      "TEXT",       "CHAR",   "FLOAT",     "DOUBLE",  "DECIMAL", "BOOLEAN",
      "BOOL",       "DATE",   "TIMESTAMP", "PRIMARY", "KEY",     "FOREIGN",
      "REFERENCES", "UNIQUE", "NOT",       "NULL",    "DEFAULT", "SELECT",
      "FROM",       "AS",     "RETURNING", "COUNT",   "MAX",     "MIN",
      "SUM",        "AVG",    NULL};
  int i;
  for (i = 0; keywords[i] != NULL; ++i) {
    if (strlen(keywords[i]) == len &&
        strncmp(str, keywords[i], len) ==
            0) { /* Case sensitive for simplicity now; SQL usually case
                    insensitive, but we'll improve later */
      return 1;
    }
  }
  return 0;
}

/**
 * @brief Push a token into the list.
 */
static int push_token(struct sql_token_list_t *list, enum SqlTokenKind kind,
                      const char *start, size_t length) {
  if (list->size >= list->capacity) {
    size_t new_cap = list->capacity == 0 ? 16 : list->capacity * 2;
    struct sql_token_t *new_tokens = (struct sql_token_t *)realloc(
        list->tokens, new_cap * sizeof(struct sql_token_t));
    if (!new_tokens) {
      return 1;
    }
    list->tokens = new_tokens;
    list->capacity = new_cap;
  }
  list->tokens[list->size].kind = kind;
  list->tokens[list->size].start = start;
  list->tokens[list->size].length = length;
  list->size++;
  return 0;
}

int sql_lex(az_span source, struct sql_token_list_t **out_list) {
  struct sql_token_list_t *list;
  const char *curr;
  const char *end;
  int err;

  if (!source._internal.ptr || !out_list) {
    return 1;
  }

  list = (struct sql_token_list_t *)calloc(1, sizeof(struct sql_token_list_t));
  if (!list) {
    return 1;
  }

  curr = (const char *)az_span_ptr(source);
  end = curr + az_span_size(source);

  while (curr < end) {
    if (isspace((unsigned char)*curr)) {
      const char *start = curr;
      while (curr < end && isspace((unsigned char)*curr)) {
        curr++;
      }
      err =
          push_token(list, SQL_TOKEN_WHITESPACE, start, (size_t)(curr - start));
      if (err)
        goto fail;
      continue;
    }

    if (isalpha((unsigned char)*curr) || *curr == '_') {
      const char *start = curr;
      while (curr < end && (isalnum((unsigned char)*curr) || *curr == '_')) {
        curr++;
      }
      {
        size_t len = (size_t)(curr - start);
        enum SqlTokenKind kind = SQL_TOKEN_IDENTIFIER;
        if (is_sql_keyword(start, len)) {
          /* Note: In a real implementation we'd do case-insensitive comparison
           */
          kind = SQL_TOKEN_KEYWORD;
        }
        err = push_token(list, kind, start, len);
        if (err)
          goto fail;
      }
      continue;
    }

    if (isdigit((unsigned char)*curr)) {
      const char *start = curr;
      while (curr < end && isdigit((unsigned char)*curr)) {
        curr++;
      }
      err = push_token(list, SQL_TOKEN_NUMBER, start, (size_t)(curr - start));
      if (err)
        goto fail;
      continue;
    }

    if (*curr == '\'') {
      const char *start = curr;
      curr++;
      while (curr < end && *curr != '\'') {
        curr++;
      }
      if (curr < end) {
        curr++; /* Skip closing quote */
      }
      err = push_token(list, SQL_TOKEN_STRING, start, (size_t)(curr - start));
      if (err)
        goto fail;
      continue;
    }

    switch (*curr) {
    case '(':
      err = push_token(list, SQL_TOKEN_LPAREN, curr, 1);
      curr++;
      break;
    case ')':
      err = push_token(list, SQL_TOKEN_RPAREN, curr, 1);
      curr++;
      break;
    case ',':
      err = push_token(list, SQL_TOKEN_COMMA, curr, 1);
      curr++;
      break;
    case ';':
      err = push_token(list, SQL_TOKEN_SEMICOLON, curr, 1);
      curr++;
      break;
    default:
      err = push_token(list, SQL_TOKEN_UNKNOWN, curr, 1);
      curr++;
      break;
    }
    if (err)
      goto fail;
  }

  err = push_token(list, SQL_TOKEN_EOF, curr, 0);
  if (err)
    goto fail;

  *out_list = list;
  return 0;

fail:
  sql_token_list_free(list);
  return 1;
}

int sql_token_list_free(struct sql_token_list_t *list) {
  if (list) {
    if (list->tokens) {
      free(list->tokens);
    }
    free(list);
  }
  return 0;
}

int sql_table_free(struct sql_table_t *table) {
  if (table) {
    size_t i;
    for (i = 0; i < table->n_columns; ++i) {
      struct sql_column_t *col = &table->columns[i];
      if (col->name)
        free(col->name);
      if (col->constraints) {
        size_t j;
        for (j = 0; j < col->n_constraints; ++j) {
          struct sql_constraint_t *c = &col->constraints[j];
          if (c->reference_table)
            free(c->reference_table);
          if (c->reference_column)
            free(c->reference_column);
          if (c->default_value)
            free(c->default_value);
        }
        free(col->constraints);
      }
    }
    if (table->columns)
      free(table->columns);
    if (table->name)
      free(table->name);

    for (i = 0; i < table->n_table_constraints; ++i) {
      struct sql_constraint_t *c = &table->table_constraints[i];
      if (c->reference_table)
        free(c->reference_table);
      if (c->reference_column)
        free(c->reference_column);
      if (c->default_value)
        free(c->default_value);
    }
    if (table->table_constraints)
      free(table->table_constraints);

    free(table);
  }
  return 0;
}

struct SqlParserState {
  const struct sql_token_list_t *list;
  size_t cursor;
  struct sql_parse_error_t *out_error;
};

static int sql_parser_peek(struct SqlParserState *state,
                           struct sql_token_t **_out_val) {
  size_t c = state->cursor;
  while (c < state->list->size &&
         state->list->tokens[c].kind == SQL_TOKEN_WHITESPACE) {
    c++;
  }
  if (c < state->list->size) {
    {
      *_out_val = &state->list->tokens[c];
      return 0;
    }
  }
  {
    *_out_val = NULL;
    return 0;
  }
}

static void sql_parser_consume(struct SqlParserState *state) {
  while (state->cursor < state->list->size &&
         state->list->tokens[state->cursor].kind == SQL_TOKEN_WHITESPACE) {
    state->cursor++;
  }
  if (state->cursor < state->list->size) {
    state->cursor++;
  }
}

static int sql_parser_match_keyword(struct SqlParserState *state,
                                    const char *kw) {
  struct sql_token_t *_ast_sql_parser_peek_0;
  const struct sql_token_t *tok =
      (sql_parser_peek(state, &_ast_sql_parser_peek_0), _ast_sql_parser_peek_0);
  if (tok && tok->kind == SQL_TOKEN_KEYWORD) {
    if (tok->length == strlen(kw) &&
        strncmp(tok->start, kw, tok->length) == 0) {
      sql_parser_consume(state);
      return 1;
    }
  }
  return 0;
}

static int sql_parser_match_kind(struct SqlParserState *state,
                                 enum SqlTokenKind kind,
                                 const struct sql_token_t **out_tok) {
  struct sql_token_t *_ast_sql_parser_peek_1;
  const struct sql_token_t *tok =
      (sql_parser_peek(state, &_ast_sql_parser_peek_1), _ast_sql_parser_peek_1);
  if (tok && tok->kind == kind) {
    if (out_tok) {
      *out_tok = tok;
    }
    sql_parser_consume(state);
    return 1;
  }
  return 0;
}

static int sql_parser_set_error(struct SqlParserState *state, const char *msg) {
  struct sql_token_t *_ast_sql_parser_peek_2;
  if (state->out_error) {
    state->out_error->message = msg;
    state->out_error->token = (sql_parser_peek(state, &_ast_sql_parser_peek_2),
                               _ast_sql_parser_peek_2);
  }
  return 1; /* Return 1 to indicate error */
}

static int sql_parse_data_type(struct SqlParserState *state,
                               enum SqlDataType *out_type, int *out_length) {
  struct sql_token_t *_ast_sql_parser_peek_3;
  const struct sql_token_t *tok =
      (sql_parser_peek(state, &_ast_sql_parser_peek_3), _ast_sql_parser_peek_3);
  if (!tok || tok->kind != SQL_TOKEN_KEYWORD) {
    return sql_parser_set_error(state, "Expected data type");
  }

  *out_length = -1;

  if (sql_parser_match_keyword(state, "INT") ||
      sql_parser_match_keyword(state, "INTEGER")) {
    *out_type = SQL_TYPE_INT;
  } else if (sql_parser_match_keyword(state, "BIGINT")) {
    *out_type = SQL_TYPE_BIGINT;
  } else if (sql_parser_match_keyword(state, "VARCHAR") ||
             sql_parser_match_keyword(state, "TEXT") ||
             sql_parser_match_keyword(state, "CHAR")) {
    if (strncmp(tok->start, "VARCHAR", 7) == 0)
      *out_type = SQL_TYPE_VARCHAR;
    else if (strncmp(tok->start, "TEXT", 4) == 0)
      *out_type = SQL_TYPE_TEXT;
    else
      *out_type = SQL_TYPE_CHAR;

    if (sql_parser_match_kind(state, SQL_TOKEN_LPAREN, NULL)) {
      const struct sql_token_t *num_tok;
      if (sql_parser_match_kind(state, SQL_TOKEN_NUMBER, &num_tok)) {
        /* simplistic string to int */
        int val = 0;
        size_t i;
        for (i = 0; i < num_tok->length; ++i) {
          val = val * 10 + (num_tok->start[i] - '0');
        }
        *out_length = val;
      }
      if (!sql_parser_match_kind(state, SQL_TOKEN_RPAREN, NULL)) {
        return sql_parser_set_error(state, "Expected ')' after length");
      }
    }
  } else if (sql_parser_match_keyword(state, "FLOAT") ||
             sql_parser_match_keyword(state, "DOUBLE") ||
             sql_parser_match_keyword(state, "DECIMAL")) {
    if (strncmp(tok->start, "FLOAT", 5) == 0)
      *out_type = SQL_TYPE_FLOAT;
    else if (strncmp(tok->start, "DOUBLE", 6) == 0)
      *out_type = SQL_TYPE_DOUBLE;
    else
      *out_type = SQL_TYPE_DECIMAL;
  } else if (sql_parser_match_keyword(state, "BOOLEAN") ||
             sql_parser_match_keyword(state, "BOOL")) {
    *out_type = SQL_TYPE_BOOLEAN;
  } else if (sql_parser_match_keyword(state, "DATE")) {
    *out_type = SQL_TYPE_DATE;
  } else if (sql_parser_match_keyword(state, "TIMESTAMP")) {
    *out_type = SQL_TYPE_TIMESTAMP;
  } else {
    return sql_parser_set_error(state, "Unknown data type");
  }
  return 0;
}

static int
sql_parse_column_constraint(struct SqlParserState *state,
                            struct sql_constraint_t *out_constraint) {
  out_constraint->type = SQL_CONSTRAINT_NONE;
  out_constraint->reference_table = NULL;
  out_constraint->reference_column = NULL;
  out_constraint->default_value = NULL;

  if (sql_parser_match_keyword(state, "PRIMARY")) {
    if (!sql_parser_match_keyword(state, "KEY")) {
      return sql_parser_set_error(state, "Expected 'KEY' after 'PRIMARY'");
    }
    out_constraint->type = SQL_CONSTRAINT_PRIMARY_KEY;
    return 0;
  } else if (sql_parser_match_keyword(state, "NOT")) {
    if (!sql_parser_match_keyword(state, "NULL")) {
      return sql_parser_set_error(state, "Expected 'NULL' after 'NOT'");
    }
    out_constraint->type = SQL_CONSTRAINT_NOT_NULL;
    return 0;
  } else if (sql_parser_match_keyword(state, "UNIQUE")) {
    out_constraint->type = SQL_CONSTRAINT_UNIQUE;
    return 0;
  } else if (sql_parser_match_keyword(state, "DEFAULT")) {
    const struct sql_token_t *val_tok;
    if (sql_parser_match_kind(state, SQL_TOKEN_STRING, &val_tok) ||
        sql_parser_match_kind(state, SQL_TOKEN_NUMBER, &val_tok) ||
        sql_parser_match_kind(state, SQL_TOKEN_IDENTIFIER, &val_tok) ||
        sql_parser_match_kind(state, SQL_TOKEN_KEYWORD, &val_tok)) {
      char *val = (char *)malloc(val_tok->length + 1);
      if (!val)
        return sql_parser_set_error(state, "OOM allocating default value");
      memcpy(val, val_tok->start, val_tok->length);
      val[val_tok->length] = '\0';
      out_constraint->type = SQL_CONSTRAINT_DEFAULT;
      out_constraint->default_value = val;
      return 0;
    }
    return sql_parser_set_error(state, "Expected value after 'DEFAULT'");
  } else if (sql_parser_match_keyword(state, "REFERENCES")) {
    const struct sql_token_t *ref_table_tok;
    if (!sql_parser_match_kind(state, SQL_TOKEN_IDENTIFIER, &ref_table_tok)) {
      return sql_parser_set_error(state,
                                  "Expected table name after 'REFERENCES'");
    }
    {
      char *tname = (char *)malloc(ref_table_tok->length + 1);
      if (!tname)
        return sql_parser_set_error(state,
                                    "OOM allocating reference table name");
      memcpy(tname, ref_table_tok->start, ref_table_tok->length);
      tname[ref_table_tok->length] = '\0';
      out_constraint->reference_table = tname;
    }

    if (sql_parser_match_kind(state, SQL_TOKEN_LPAREN, NULL)) {
      const struct sql_token_t *ref_col_tok;
      if (!sql_parser_match_kind(state, SQL_TOKEN_IDENTIFIER, &ref_col_tok)) {
        return sql_parser_set_error(state,
                                    "Expected column name in REFERENCES()");
      }
      {
        char *cname = (char *)malloc(ref_col_tok->length + 1);
        if (!cname)
          return sql_parser_set_error(state,
                                      "OOM allocating reference col name");
        memcpy(cname, ref_col_tok->start, ref_col_tok->length);
        cname[ref_col_tok->length] = '\0';
        out_constraint->reference_column = cname;
      }
      if (!sql_parser_match_kind(state, SQL_TOKEN_RPAREN, NULL)) {
        return sql_parser_set_error(state,
                                    "Expected ')' after reference column");
      }
    }
    out_constraint->type = SQL_CONSTRAINT_FOREIGN_KEY;
    return 0;
  }

  return 1; /* Not a constraint */
}

int sql_parse_table(const struct sql_token_list_t *list,
                    struct sql_table_t **out_table,
                    struct sql_parse_error_t *out_error) {
  struct sql_token_t *_ast_sql_parser_peek_4;
  struct sql_token_t *_ast_sql_parser_peek_5;
  struct SqlParserState state;
  struct sql_table_t *table = NULL;
  const struct sql_token_t *name_tok;
  int err;

  state.list = list;
  state.cursor = 0;
  state.out_error = out_error;

  if (out_error) {
    out_error->message = NULL;
    out_error->token = NULL;
  }

  if (!sql_parser_match_keyword(&state, "CREATE")) {
    return sql_parser_set_error(&state, "Expected 'CREATE'");
  }
  if (!sql_parser_match_keyword(&state, "TABLE")) {
    return sql_parser_set_error(&state, "Expected 'TABLE'");
  }
  if (!sql_parser_match_kind(&state, SQL_TOKEN_IDENTIFIER, &name_tok)) {
    return sql_parser_set_error(&state, "Expected table name");
  }

  table = (struct sql_table_t *)calloc(1, sizeof(struct sql_table_t));
  if (!table)
    return sql_parser_set_error(&state, "OOM allocating table");

  table->name = (char *)malloc(name_tok->length + 1);
  if (!table->name) {
    sql_table_free(table);
    return sql_parser_set_error(&state, "OOM allocating table name");
  }
  memcpy(table->name, name_tok->start, name_tok->length);
  table->name[name_tok->length] = '\0';

  if (!sql_parser_match_kind(&state, SQL_TOKEN_LPAREN, NULL)) {
    sql_table_free(table);
    return sql_parser_set_error(&state, "Expected '('");
  }

  /* Parse columns and table constraints */
  while (1) {
    const struct sql_token_t *col_name_tok;
    const struct sql_token_t *peek =
        (sql_parser_peek(&state, &_ast_sql_parser_peek_4),
         _ast_sql_parser_peek_4);

    if (!peek)
      break;
    if (peek->kind == SQL_TOKEN_RPAREN) {
      break; /* End of table definition */
    }

    /* Is it a table-level constraint? (e.g., PRIMARY KEY (id), FOREIGN KEY ...)
     */
    if (peek->kind == SQL_TOKEN_KEYWORD &&
        ((peek->length == 7 && strncmp(peek->start, "PRIMARY", 7) == 0) ||
         (peek->length == 7 && strncmp(peek->start, "FOREIGN", 7) == 0) ||
         (peek->length == 6 && strncmp(peek->start, "UNIQUE", 6) == 0))) {
      /* TODO: Table level constraints implementation */
      /* For now, we skip or error out on table level constraints, or parse
       * simply. Let's just set an error as it's not fully requested yet, but we
       * will add it. */
      /* Or rather, just skip till comma for now if we can't parse */
      return sql_parser_set_error(
          &state, "Table level constraints not fully implemented");
    }

    /* Must be a column definition */
    if (!sql_parser_match_kind(&state, SQL_TOKEN_IDENTIFIER, &col_name_tok)) {
      sql_table_free(table);
      return sql_parser_set_error(&state, "Expected column name");
    }

    {
      struct sql_column_t col;
      size_t constraint_capacity = 2;
      memset(&col, 0, sizeof(col));

      col.name = (char *)malloc(col_name_tok->length + 1);
      if (!col.name) {
        sql_table_free(table);
        return sql_parser_set_error(&state, "OOM allocating column name");
      }
      memcpy(col.name, col_name_tok->start, col_name_tok->length);
      col.name[col_name_tok->length] = '\0';

      err = sql_parse_data_type(&state, &col.type, &col.length);
      if (err) {
        free(col.name);
        sql_table_free(table);
        return err;
      }

      col.constraints = (struct sql_constraint_t *)malloc(
          constraint_capacity * sizeof(struct sql_constraint_t));
      col.n_constraints = 0;

      /* Parse inline constraints */
      while (1) {
        struct sql_constraint_t constraint;
        peek = (sql_parser_peek(&state, &_ast_sql_parser_peek_5),
                _ast_sql_parser_peek_5);
        if (!peek || peek->kind == SQL_TOKEN_COMMA ||
            peek->kind == SQL_TOKEN_RPAREN) {
          break;
        }

        err = sql_parse_column_constraint(&state, &constraint);
        if (err == 0) {
          if (col.n_constraints >= constraint_capacity) {
            constraint_capacity *= 2;
            col.constraints = (struct sql_constraint_t *)realloc(
                col.constraints,
                constraint_capacity * sizeof(struct sql_constraint_t));
          }
          col.constraints[col.n_constraints++] = constraint;
        } else {
          break; /* Stop parsing constraints if we hit something else */
        }
      }

      /* Add column to table */
      {
        size_t new_cap = table->n_columns + 1;
        struct sql_column_t *new_cols = (struct sql_column_t *)realloc(
            table->columns, new_cap * sizeof(struct sql_column_t));
        if (!new_cols) {
          /* Free constraints! */
          size_t c;
          for (c = 0; c < col.n_constraints; ++c) {
            if (col.constraints[c].reference_table)
              free(col.constraints[c].reference_table);
            if (col.constraints[c].reference_column)
              free(col.constraints[c].reference_column);
            if (col.constraints[c].default_value)
              free(col.constraints[c].default_value);
          }
          free(col.constraints);
          free(col.name);
          sql_table_free(table);
          return sql_parser_set_error(&state, "OOM allocating column");
        }
        table->columns = new_cols;
        table->columns[table->n_columns++] = col;
      }
    }

    if (sql_parser_match_kind(&state, SQL_TOKEN_COMMA, NULL)) {
      continue;
    } else {
      break;
    }
  }

  if (!sql_parser_match_kind(&state, SQL_TOKEN_RPAREN, NULL)) {
    sql_table_free(table);
    return sql_parser_set_error(&state,
                                "Expected ')' at end of table definition");
  }

  sql_parser_match_kind(&state, SQL_TOKEN_SEMICOLON,
                        NULL); /* Optional semicolon */

  *out_table = table;
  return 0;
}

int parse_sql_ddl(const char *sql_data, struct sql_table_t **out_tables,
                  size_t *out_n_tables) {
  /* Simple stub that parses one table for now utilizing sql_parse_table */
  struct sql_token_list_t *list = NULL;
  struct sql_table_t *table = NULL;
  struct sql_parse_error_t err;
  az_span span;
  int rc;

  if (!sql_data || !out_tables || !out_n_tables)
    return 1;

  span = az_span_create_from_str((char *)sql_data);
  rc = sql_lex(span, &list);
  if (rc != 0)
    return rc;

  /* In the interest of keeping it compliant, we will just parse up to 10 tables
   * max for this test run */
  *out_tables = (struct sql_table_t *)calloc(10, sizeof(struct sql_table_t));
  *out_n_tables = 0;

  {
    struct sql_token_list_t sublist;
    size_t i;
    size_t start_idx = 0;
    int in_table = 0;

    for (i = 0; i < list->size; ++i) {
      if (list->tokens[i].kind == SQL_TOKEN_KEYWORD) {
        if (list->tokens[i].length == 6 &&
            strncmp(list->tokens[i].start, "CREATE", 6) == 0) {
          start_idx = i;
          in_table = 1;
        }
      }

      if (in_table && list->tokens[i].kind == SQL_TOKEN_SEMICOLON) {
        /* Parse this slice */
        sublist.tokens = &list->tokens[start_idx];
        sublist.size = i - start_idx + 1;
        sublist.capacity = sublist.size;

        table = NULL;
        rc = sql_parse_table(&sublist, &table, &err);
        if (rc == 0 && table) {
          memcpy(&(*out_tables)[*out_n_tables], table,
                 sizeof(struct sql_table_t));
          free(table); /* Shallow free, we copied the struct */
          (*out_n_tables)++;
        }
        in_table = 0;
      }
    }
  }

  sql_token_list_free(list);
  return 0;
}
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
        (

            (tok->length == 5 && strncmp(tok->start, "COUNT", 5) == 0) ||

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

        field.type = SQL_TYPE_BIGINT;
        /* COUNT usually returns BIGINT */
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

        /* End of projection fields
         */

        break;
      }

      base_name = (char *)malloc(tok->length + 1);

      memcpy(base_name, tok->start, tok->length);

      base_name[tok->length] = '\0';

    } else {

      break;
      /* Might be * or something we don't support yet */
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
