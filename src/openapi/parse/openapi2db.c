#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openapi/parse/openapi2db.h"
#include "classes/emit/struct.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

static enum DatabaseColumnType type_to_db_type(const char *type, const char *format) {
  if (!type) return DB_COL_TYPE_VARCHAR;
  if (strcmp(type, "integer") == 0) return DB_COL_TYPE_INTEGER;
  if (strcmp(type, "number") == 0) return DB_COL_TYPE_REAL;
  if (strcmp(type, "boolean") == 0) return DB_COL_TYPE_BOOLEAN;
  if (strcmp(type, "string") == 0) {
    if (format && format[0]) {
      if (strcmp(format, "date") == 0) return DB_COL_TYPE_DATE;
      if (strcmp(format, "date-time") == 0) return DB_COL_TYPE_DATETIME;
      if (strcmp(format, "byte") == 0 || strcmp(format, "binary") == 0) return DB_COL_TYPE_BLOB;
    }
    return DB_COL_TYPE_VARCHAR;
  }
  return DB_COL_TYPE_VARCHAR;
}

int openapi_to_db_schema(const struct OpenAPI_Spec *spec, struct DatabaseSchema *schema) {
  size_t i, j;
  if (!spec || !schema) return EINVAL;

  db_schema_init(schema);
  schema->name = strdup(spec->info.title && spec->info.title[0] ? spec->info.title : "OpenAPIDB");
  if (spec->n_defined_schemas > 0) {
    schema->tables = calloc(spec->n_defined_schemas, sizeof(struct DatabaseTable));
    schema->n_tables = 0;

    for (i = 0; i < spec->n_defined_schemas; ++i) {
      struct StructFields *s = &spec->defined_schemas[i];
      struct DatabaseTable *table;
      
      if (s->is_enum || s->is_union) {
        continue; /* Only objects/structs become tables */
      }
      
      table = &schema->tables[schema->n_tables++];
      table->name = strdup(spec->defined_schema_names[i]);
      if (s->size > 0) {
        table->columns = calloc(s->size, sizeof(struct DatabaseColumn));
        table->n_columns = s->size;

        for (j = 0; j < s->size; ++j) {
          struct StructField *prop = &s->fields[j];
          struct DatabaseColumn *col = &table->columns[j];
          
          col->name = strdup(prop->name);
          col->type = type_to_db_type(prop->type, prop->format);
          
          /* In OpenAPI there is no direct primary key, let's guess by name 'id' */
          if (strcmp(col->name, "id") == 0) {
            col->is_primary_key = 1;
            col->is_unique = 1;
            col->is_nullable = 0;
          } else {
            col->is_nullable = prop->required ? 0 : 1;
          }
        }
      }
    }
  }

  return 0;
}
