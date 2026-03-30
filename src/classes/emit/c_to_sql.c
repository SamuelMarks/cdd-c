/* clang-format off */
#include "c_to_sql.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

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
    const char *sql_type = "TEXT";

    if (strcmp(f->type, "integer") == 0) {
      sql_type = "INTEGER";
    } else if (strcmp(f->type, "number") == 0) {
      if (dialect == C_TO_SQL_DIALECT_POSTGRESQL) {
        sql_type = "DOUBLE PRECISION";
      } else {
        sql_type = "REAL";
      }
    } else if (strcmp(f->type, "boolean") == 0) {
      if (dialect == C_TO_SQL_DIALECT_POSTGRESQL) {
        sql_type = "BOOLEAN";
      } else {
        sql_type = "INTEGER";
      }
    } else if (strcmp(f->type, "string") == 0) {
      sql_type = "VARCHAR(255)";
    }

    fprintf(fp, "  %s %s", f->name, sql_type);

    if (strcmp(f->name, "id") == 0 || strcmp(f->name, "Id") == 0 ||
        strcmp(f->name, "ID") == 0) {
      fprintf(fp, " PRIMARY KEY");
      if (dialect == C_TO_SQL_DIALECT_SQLITE &&
          strcmp(sql_type, "INTEGER") == 0) {
        fprintf(fp, " AUTOINCREMENT");
      }
    }

    if (f->required) {
      fprintf(fp, " NOT NULL");
    }

    if (i < sf->size - 1) {
      fprintf(fp, ",");
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, ");\n");
  return 0;
}

static const char *map_c_type_to_sql(const char *c_type,
                                     c_to_sql_dialect_t dialect) {
  if (strstr(c_type, "int") != NULL)
    return "INTEGER";
  if (strstr(c_type, "char") != NULL)
    return "TEXT";
  if (strstr(c_type, "float") != NULL || strstr(c_type, "double") != NULL) {
    return (dialect == C_TO_SQL_DIALECT_POSTGRESQL) ? "DOUBLE PRECISION"
                                                    : "REAL";
  }
  return "BLOB";
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
    sql_type = map_c_type_to_sql(prop->type, dialect);

    offset += sprintf(buffer + offset, "  %s %s", prop->name, sql_type);

    if (strcmp(prop->name, "id") == 0) {
      offset += sprintf(buffer + offset, " PRIMARY KEY");
      if (dialect == C_TO_SQL_DIALECT_SQLITE &&
          strcmp(sql_type, "INTEGER") == 0) {
        offset += sprintf(buffer + offset, " AUTOINCREMENT");
      }
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
