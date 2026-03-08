#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse/cst2db.h"
#include "functions/parse/str.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

static char *cdd_strndup(const char *s, size_t n) {
  char *result;
  size_t len = 0;
  while (len < n && s[len] != '\0')
    len++;
  result = malloc(len + 1);
  if (!result)
    return NULL;
  memcpy(result, s, len);
  result[len] = '\0';
  return result;
}

static int is_db_model(const struct CstNode *node,
                       const struct TokenList *tokens) {
  size_t i;
  /* check inside the node */
  for (i = node->start_token; i < node->end_token; ++i) {
    if (tokens->tokens[i].kind == TOKEN_COMMENT) {
      const char *c = (const char *)tokens->tokens[i].start;
      size_t l = tokens->tokens[i].length;
      if (l >= 5 && strncmp(c, "/*", 2) == 0) {
        if (strstr(c, "@db") != NULL) return 1;
      }
    } else if (tokens->tokens[i].kind == TOKEN_KEYWORD_ATTRIBUTE) {
      return 1;
    } else if (tokens->tokens[i].kind == TOKEN_KEYWORD_DECLSPEC) {
      return 1;
    }
  }
  /* check tokens immediately before the node (comments) */
  if (node->start_token > 0) {
    for (i = node->start_token - 1; ; --i) {
      if (tokens->tokens[i].kind == TOKEN_WHITESPACE) {
         if (i == 0) break;
         continue;
      }
      if (tokens->tokens[i].kind == TOKEN_COMMENT) {
        const char *c = (const char *)tokens->tokens[i].start;
        if (strstr(c, "@db") != NULL) return 1;
        if (i == 0) break;
      } else {
        break;
      }
    }
  }
  return 0;
}

static enum DatabaseColumnType parse_type(const struct TokenList *tokens, size_t start_tok, size_t end_tok) {
  size_t i;
  for (i = start_tok; i < end_tok; ++i) {
    if (tokens->tokens[i].kind == TOKEN_KEYWORD_INT || tokens->tokens[i].kind == TOKEN_KEYWORD_LONG || tokens->tokens[i].kind == TOKEN_KEYWORD_SHORT) {
      return DB_COL_TYPE_INTEGER;
    } else if (tokens->tokens[i].kind == TOKEN_KEYWORD_FLOAT || tokens->tokens[i].kind == TOKEN_KEYWORD_DOUBLE) {
      return DB_COL_TYPE_REAL;
    } else if (tokens->tokens[i].kind == TOKEN_KEYWORD_CHAR) {
      /* check for pointer */
      size_t j;
      for (j = i + 1; j < end_tok; ++j) {
        if (tokens->tokens[j].kind == TOKEN_STAR) {
          return DB_COL_TYPE_VARCHAR;
        }
      }
      return DB_COL_TYPE_INTEGER; /* single char */
    } else if (tokens->tokens[i].kind == TOKEN_KEYWORD_BOOL) {
      return DB_COL_TYPE_BOOLEAN;
    }
  }
  return DB_COL_TYPE_UNKNOWN;
}

int cst_parse_table(const struct CstNode *node,
                    const struct TokenList *tokens,
                    struct DatabaseTable *table) {
  size_t i;
  size_t brace_start = 0;
  size_t brace_end = 0;
  size_t name_tok = 0;

  if (!node || !tokens || !table)
    return EINVAL;

  if (node->kind != CST_NODE_STRUCT)
    return EINVAL;

  if (!is_db_model(node, tokens))
    return ENOENT;

  table->name = NULL;
  table->columns = NULL;
  table->n_columns = 0;

  for (i = node->start_token; i < node->end_token; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER && name_tok == 0) {
      name_tok = i;
    } else if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      brace_start = i;
      break;
    }
  }

  for (i = node->end_token - 1; i > brace_start; --i) {
    if (tokens->tokens[i].kind == TOKEN_RBRACE) {
      brace_end = i;
      break;
    }
  }

  if (name_tok != 0) {
    table->name = cdd_strndup((const char *)tokens->tokens[name_tok].start, tokens->tokens[name_tok].length);
  } else {
    table->name = strdup("anonymous_table");
  }

  if (brace_start != 0 && brace_end != 0 && brace_end > brace_start) {
    size_t stmt_start = brace_start + 1;
    size_t j;
    size_t col_cap = 10;
    table->columns = calloc(col_cap, sizeof(struct DatabaseColumn));

    for (j = stmt_start; j < brace_end; ++j) {
      if (tokens->tokens[j].kind == TOKEN_SEMICOLON) {
        /* parse statement */
        size_t k;
        size_t id_tok = 0;
        struct DatabaseColumn *col;
        if (table->n_columns >= col_cap) {
          col_cap *= 2;
          table->columns = realloc(table->columns, col_cap * sizeof(struct DatabaseColumn));
        }
        col = &table->columns[table->n_columns];
        memset(col, 0, sizeof(*col));

        for (k = j - 1; k >= stmt_start; --k) {
          if (tokens->tokens[k].kind == TOKEN_IDENTIFIER) {
            id_tok = k;
            break;
          }
        }
        if (id_tok != 0) {
          col->name = cdd_strndup((const char *)tokens->tokens[id_tok].start, tokens->tokens[id_tok].length);
          col->type = parse_type(tokens, stmt_start, id_tok);
        } else {
          col->name = strdup("unknown");
          col->type = DB_COL_TYPE_UNKNOWN;
        }

        /* Check for @primary_key, @not_null, @unique, etc in comments */
        for (k = stmt_start; k < j; ++k) {
          if (tokens->tokens[k].kind == TOKEN_COMMENT) {
             const char *c = (const char *)tokens->tokens[k].start;
             if (strstr(c, "@primary_key") != NULL) col->is_primary_key = 1;
             if (strstr(c, "@not_null") != NULL) col->is_nullable = 0;
             else col->is_nullable = 1; /* default */
             if (strstr(c, "@unique") != NULL) col->is_unique = 1;
             
             /* basic fk parsing */
             if (strstr(c, "@foreign_key") != NULL) {
               col->foreign_key_table = strdup("unknown_ref"); /* TODO parse properly */
               col->foreign_key_column = strdup("id");
             }
          }
        }

        table->n_columns++;
        stmt_start = j + 1;
      }
    }
  }

  return 0;
}

int cst_extract_database_schema(const struct CstNodeList *nodes,
                                const struct TokenList *tokens,
                                struct DatabaseSchema *schema) {
  size_t i;
  if (!nodes || !tokens || !schema)
    return EINVAL;

  db_schema_init(schema);
  schema->name = strdup("GeneratedDatabase");
  schema->tables = calloc(nodes->size, sizeof(struct DatabaseTable));
  
  for (i = 0; i < nodes->size; ++i) {
    if (nodes->nodes[i].kind == CST_NODE_STRUCT) {
      struct DatabaseTable t;
      if (cst_parse_table(&nodes->nodes[i], tokens, &t) == 0) {
        schema->tables[schema->n_tables++] = t;
      }
    }
  }
  return 0;
}
