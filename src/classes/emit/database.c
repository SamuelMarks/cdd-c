#include <stdlib.h>
#include <string.h>

#include "classes/emit/database.h"

void db_schema_init(struct DatabaseSchema *schema) {
  if (!schema)
    return;
  schema->name = NULL;
  schema->tables = NULL;
  schema->n_tables = 0;
}

void db_schema_free(struct DatabaseSchema *schema) {
  size_t i, j;
  if (!schema)
    return;

  if (schema->name) {
    free(schema->name);
    schema->name = NULL;
  }

  if (schema->tables) {
    for (i = 0; i < schema->n_tables; ++i) {
      struct DatabaseTable *table = &schema->tables[i];
      if (table->name) {
        free(table->name);
      }
      if (table->columns) {
        for (j = 0; j < table->n_columns; ++j) {
          struct DatabaseColumn *col = &table->columns[j];
          if (col->name)
            free(col->name);
          if (col->default_value)
            free(col->default_value);
          if (col->foreign_key_table)
            free(col->foreign_key_table);
          if (col->foreign_key_column)
            free(col->foreign_key_column);
        }
        free(table->columns);
      }
    }
    free(schema->tables);
    schema->tables = NULL;
  }
  schema->n_tables = 0;
}
