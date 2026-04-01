/* clang-format off */
#include "c_to_sql.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

static const char *map_c_type_to_sql(const char *c_type,
                                     c_to_sql_dialect_t dialect) {
  if (strstr(c_type, "int") != NULL) {
    if (dialect == C_TO_SQL_DIALECT_MYSQL)
      return "INT";
    if (dialect == C_TO_SQL_DIALECT_POSTGRESQL)
      return "INTEGER";
    return "INTEGER";
  }
  if (strstr(c_type, "char") != NULL || strstr(c_type, "string") != NULL) {
    if (dialect == C_TO_SQL_DIALECT_MYSQL)
      return "VARCHAR(255)";
    if (dialect == C_TO_SQL_DIALECT_POSTGRESQL)
      return "TEXT";
    return "TEXT";
  }
  if (strstr(c_type, "float") != NULL || strstr(c_type, "double") != NULL ||
      strstr(c_type, "number") != NULL) {
    if (dialect == C_TO_SQL_DIALECT_MYSQL)
      return "DOUBLE";
    if (dialect == C_TO_SQL_DIALECT_POSTGRESQL)
      return "DOUBLE PRECISION";
    return "REAL";
  }
  if (strstr(c_type, "bool") != NULL) {
    if (dialect == C_TO_SQL_DIALECT_POSTGRESQL)
      return "BOOLEAN";
    if (dialect == C_TO_SQL_DIALECT_MYSQL)
      return "TINYINT(1)";
    return "INTEGER";
  }
  return "BLOB";
}

C_CDD_EXPORT int write_struct_to_sql_create_table(FILE *fp,
                                                  const char *table_name,
                                                  const struct StructFields *sf,
                                                  c_to_sql_dialect_t dialect) {
  size_t i;
  if (!fp || !table_name || !sf)
    return 1;

  fprintf(fp, "CREATE TABLE %s (\n", table_name);

  for (i = 0; i < sf->size; i++) {
    const struct StructField *f = &sf->fields[i];
    const char *sql_type = map_c_type_to_sql(f->type, dialect);
    int is_pk = 0;
    int is_unique = 0;
    int is_not_null = f->required;
    size_t len = strlen(f->name);
    int is_fk = 0;

    if (strcmp(f->name, "id") == 0 || strcmp(f->name, "Id") == 0 ||
        strcmp(f->name, "ID") == 0) {
      is_pk = 1;
    }
    if (f->description[0] != '\0') {
      if (strstr(f->description, "@pk") != NULL ||
          strstr(f->description, "@primary_key") != NULL)
        is_pk = 1;
      if (strstr(f->description, "@unique") != NULL)
        is_unique = 1;
      if (strstr(f->description, "@notnull") != NULL ||
          strstr(f->description, "@required") != NULL)
        is_not_null = 1;
    }
    if (len > 3 && strcmp(f->name + len - 3, "_id") == 0) {
      is_fk = 1;
    }

    fprintf(fp, "  %s %s", f->name, sql_type);

    if (is_pk) {
      fprintf(fp, " PRIMARY KEY");
      if (dialect == C_TO_SQL_DIALECT_SQLITE &&
          strcmp(sql_type, "INTEGER") == 0) {
        fprintf(fp, " AUTOINCREMENT");
      }
    }

    if (is_unique) {
      fprintf(fp, " UNIQUE");
    }

    if (is_not_null) {
      fprintf(fp, " NOT NULL");
    }

    if (is_fk) {
      /* Emit basic foreign key assuming table is prefix of _id */
      char target_table[64];
      strncpy(target_table, f->name, len - 3);
      target_table[len - 3] = '\0';
      fprintf(fp, " REFERENCES %s(id)", target_table);
    }

    if (i < sf->size - 1) {
      fprintf(fp, ",");
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, ");\n");
  return 0;
}

C_CDD_EXPORT int cdd_c_meta_to_sql_create_table(const cdd_c_meta_t *meta,
                                                c_to_sql_dialect_t dialect,
                                                char **out_sql) {
  char buffer[4096];
  size_t i;
  int offset = 0;
  const char *sql_type;

  if (!meta || !out_sql)
    return 1;

  offset += sprintf(buffer + offset, "CREATE TABLE %s (\n", meta->name);

  for (i = 0; i < meta->num_props; i++) {
    const cdd_c_prop_meta_t *prop = &meta->props[i];
    size_t len = strlen(prop->name);
    int is_fk = (len > 3 && strcmp(prop->name + len - 3, "_id") == 0);
    sql_type = map_c_type_to_sql(prop->type, dialect);

    offset += sprintf(buffer + offset, "  %s %s", prop->name, sql_type);

    if (strcmp(prop->name, "id") == 0) {
      offset += sprintf(buffer + offset, " PRIMARY KEY");
      if (dialect == C_TO_SQL_DIALECT_SQLITE &&
          strcmp(sql_type, "INTEGER") == 0) {
        offset += sprintf(buffer + offset, " AUTOINCREMENT");
      }
    }

    if (is_fk) {
      char target_table[64];
      strncpy(target_table, prop->name, len - 3);
      target_table[len - 3] = '\0';
      offset += sprintf(buffer + offset, " REFERENCES %s(id)", target_table);
    }

    if (i < meta->num_props - 1) {
      offset += sprintf(buffer + offset, ",");
    }
    offset += sprintf(buffer + offset, "\n");
  }

  offset += sprintf(buffer + offset, ");\n");
  *out_sql = strdup(buffer);
  return 0;
}

C_CDD_EXPORT int cdd_c_meta_diff(const cdd_c_meta_t *old_schema,
                                 const cdd_c_meta_t *new_schema,
                                 cdd_c_meta_diff_t *out_diff) {
  size_t i, j;
  int found;

  if (!old_schema || !new_schema || !out_diff)
    return 1;
  memset(out_diff, 0, sizeof(cdd_c_meta_diff_t));

  /* Find Added Columns */
  for (i = 0; i < new_schema->num_props; i++) {
    found = 0;
    for (j = 0; j < old_schema->num_props; j++) {
      if (strcmp(new_schema->props[i].name, old_schema->props[j].name) == 0) {
        found = 1;
        /* Check altered */
        if (strcmp(new_schema->props[i].type, old_schema->props[j].type) != 0) {
          out_diff->altered_props = (cdd_c_prop_meta_t *)realloc(
              out_diff->altered_props,
              (out_diff->num_altered + 1) * sizeof(cdd_c_prop_meta_t));
          out_diff->altered_props[out_diff->num_altered++] =
              new_schema->props[i];
        }
        break;
      }
    }
    if (!found) {
      out_diff->added_props = (cdd_c_prop_meta_t *)realloc(
          out_diff->added_props,
          (out_diff->num_added + 1) * sizeof(cdd_c_prop_meta_t));
      out_diff->added_props[out_diff->num_added++] = new_schema->props[i];
    }
  }

  /* Find Dropped Columns */
  for (i = 0; i < old_schema->num_props; i++) {
    found = 0;
    for (j = 0; j < new_schema->num_props; j++) {
      if (strcmp(old_schema->props[i].name, new_schema->props[j].name) == 0) {
        found = 1;
        break;
      }
    }
    if (!found) {
      out_diff->dropped_props = (cdd_c_prop_meta_t *)realloc(
          out_diff->dropped_props,
          (out_diff->num_dropped + 1) * sizeof(cdd_c_prop_meta_t));
      out_diff->dropped_props[out_diff->num_dropped++] = old_schema->props[i];
    }
  }

  return 0;
}

C_CDD_EXPORT int cdd_c_meta_diff_to_sql(const char *table_name,
                                        const cdd_c_meta_diff_t *diff,
                                        c_to_sql_dialect_t dialect,
                                        char **up_sql, char **down_sql) {
  char up_buf[4096] = {0};
  char down_buf[4096] = {0};
  int up_offset = 0;
  int down_offset = 0;
  size_t i;

  if (!table_name || !diff || !up_sql || !down_sql)
    return 1;

  /* UP SQL */
  for (i = 0; i < diff->num_added; i++) {
    up_offset +=
        sprintf(up_buf + up_offset, "ALTER TABLE %s ADD COLUMN %s %s;\n",
                table_name, diff->added_props[i].name,
                map_c_type_to_sql(diff->added_props[i].type, dialect));
    /* Generate DOWN SQL equivalent */
    if (dialect == C_TO_SQL_DIALECT_SQLITE) {
      down_offset +=
          sprintf(down_buf + down_offset, "ALTER TABLE %s DROP COLUMN %s;\n",
                  table_name, diff->added_props[i].name);
    } else {
      down_offset +=
          sprintf(down_buf + down_offset, "ALTER TABLE %s DROP COLUMN %s;\n",
                  table_name, diff->added_props[i].name);
    }
  }

  for (i = 0; i < diff->num_dropped; i++) {
    up_offset += sprintf(up_buf + up_offset, "ALTER TABLE %s DROP COLUMN %s;\n",
                         table_name, diff->dropped_props[i].name);
    /* Generate DOWN SQL equivalent */
    down_offset +=
        sprintf(down_buf + down_offset, "ALTER TABLE %s ADD COLUMN %s %s;\n",
                table_name, diff->dropped_props[i].name,
                map_c_type_to_sql(diff->dropped_props[i].type, dialect));
  }

  /* Handling altered props in SQLite usually requires table rebuilds,
     but for now we just emit the ALTER syntax and let the developer refine */
  for (i = 0; i < diff->num_altered; i++) {
    up_offset +=
        sprintf(up_buf + up_offset, "ALTER TABLE %s ALTER COLUMN %s TYPE %s;\n",
                table_name, diff->altered_props[i].name,
                map_c_type_to_sql(diff->altered_props[i].type, dialect));
    down_offset +=
        sprintf(down_buf + down_offset,
                "-- Revert of ALTER COLUMN %s not automatically handled\n",
                diff->altered_props[i].name);
  }

  *up_sql = strdup(up_buf);
  *down_sql = strdup(down_buf);

  return 0;
}

C_CDD_EXPORT void cdd_c_meta_diff_free(cdd_c_meta_diff_t *diff) {
  if (!diff)
    return;
  if (diff->added_props)
    free(diff->added_props);
  if (diff->dropped_props)
    free(diff->dropped_props);
  if (diff->altered_props)
    free(diff->altered_props);
  memset(diff, 0, sizeof(cdd_c_meta_diff_t));
}

C_CDD_EXPORT int cdd_c_get_schema_inspection_query(c_to_sql_dialect_t dialect,
                                                   const char *table_name,
                                                   char **out_query) {
  char buf[512];
  if (!table_name || !out_query)
    return 1;

  if (dialect == C_TO_SQL_DIALECT_SQLITE) {
    sprintf(buf, "PRAGMA table_info(%s);", table_name);
  } else if (dialect == C_TO_SQL_DIALECT_POSTGRESQL) {
    sprintf(
        buf,
        "SELECT column_name, data_type, character_maximum_length, is_nullable "
        "FROM information_schema.columns WHERE table_name = '%s';",
        table_name);
  } else if (dialect == C_TO_SQL_DIALECT_MYSQL) {
    sprintf(buf, "SHOW COLUMNS FROM %s;", table_name);
  } else {
    return 1;
  }
  *out_query = strdup(buf);
  return 0;
}

C_CDD_EXPORT int cdd_c_emit_create_index(const char *table_name,
                                         const char *index_name,
                                         const char *column_name, int is_unique,
                                         char **out_sql) {
  char buf[512];
  if (!table_name || !index_name || !column_name || !out_sql)
    return 1;
  sprintf(buf, "CREATE %sINDEX %s ON %s (%s);", is_unique ? "UNIQUE " : "",
          index_name, table_name, column_name);
  *out_sql = strdup(buf);
  return 0;
}

C_CDD_EXPORT int cdd_c_emit_drop_index(const char *index_name, char **out_sql) {
  char buf[256];
  if (!index_name || !out_sql)
    return 1;
  sprintf(buf, "DROP INDEX %s;", index_name);
  *out_sql = strdup(buf);
  return 0;
}

C_CDD_EXPORT int cdd_c_meta_topological_sort(const cdd_c_meta_t **schemas,
                                             size_t num_schemas,
                                             const cdd_c_meta_t **out_schemas) {
  size_t i, j, k;
  int *visited;
  int *in_degree;
  int head = 0, tail = 0;
  int *queue;
  int count = 0;

  if (!schemas || !out_schemas || num_schemas == 0)
    return 1;

  visited = (int *)calloc(num_schemas, sizeof(int));
  in_degree = (int *)calloc(num_schemas, sizeof(int));
  queue = (int *)malloc(num_schemas * sizeof(int));

  /* Compute in-degrees based on foreign key references (_id suffix) */
  for (i = 0; i < num_schemas; i++) {
    for (j = 0; j < schemas[i]->num_props; j++) {
      size_t len = strlen(schemas[i]->props[j].name);
      if (len > 3 && strcmp(schemas[i]->props[j].name + len - 3, "_id") == 0) {
        char target[64];
        strncpy(target, schemas[i]->props[j].name, len - 3);
        target[len - 3] = '\0';
        for (k = 0; k < num_schemas; k++) {
          if (strcmp(schemas[k]->name, target) == 0) {
            in_degree[i]++;
            break;
          }
        }
      }
    }
  }

  /* Enqueue schemas with 0 in-degree */
  for (i = 0; i < num_schemas; i++) {
    if (in_degree[i] == 0) {
      queue[tail++] = (int)i;
    }
  }

  while (head < tail) {
    int curr = queue[head++];
    out_schemas[count++] = schemas[curr];

    for (i = 0; i < num_schemas; i++) {
      if (in_degree[i] > 0) {
        for (j = 0; j < schemas[i]->num_props; j++) {
          size_t len = strlen(schemas[i]->props[j].name);
          if (len > 3 &&
              strcmp(schemas[i]->props[j].name + len - 3, "_id") == 0) {
            char target[64];
            strncpy(target, schemas[i]->props[j].name, len - 3);
            target[len - 3] = '\0';
            if (strcmp(schemas[curr]->name, target) == 0) {
              in_degree[i]--;
              if (in_degree[i] == 0) {
                queue[tail++] = (int)i;
              }
            }
          }
        }
      }
    }
  }

  free(visited);
  free(in_degree);
  free(queue);

  if (count < (int)num_schemas) {
    return 2; /* Cycle detected */
  }

  return 0;
}
