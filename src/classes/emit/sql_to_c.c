/**
 * @file sql_to_c.c
 * @brief Emits C structures and array containers from SQL DDL AST.
 */

/* clang-format off */
#include "classes/emit/sql_to_c.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Convert string to uppercase.
 */
static void str_to_upper(char *dst, const char *src) {
  while (*src) {
    *dst = (char)toupper((unsigned char)*src);
    dst++;
    src++;
  }
  *dst = '\0';
}

/**
 * @brief Convert string to TitleCase (first letter upper).
 */
static void str_to_title(char *dst, const char *src) {
  if (!*src) {
    *dst = '\0';
    return;
  }
  *dst = (char)toupper((unsigned char)*src);
  dst++;
  src++;
  while (*src) {
    *dst = *src;
    dst++;
    src++;
  }
  *dst = '\0';
}

static int is_nullable(const struct sql_column_t *col) {
  size_t i;
  for (i = 0; i < col->n_constraints; ++i) {
    if (col->constraints[i].type == SQL_CONSTRAINT_NOT_NULL ||
        col->constraints[i].type == SQL_CONSTRAINT_PRIMARY_KEY) {
      return 0; /* NOT nullable */
    }
  }
  return 1; /* nullable */
}

int sql_type_to_c_type(enum SqlDataType type, char **_out_val) {
  switch (type) {
  case SQL_TYPE_INT: {
    *_out_val = "int32_t";
    return 0;
  }
  case SQL_TYPE_BIGINT: {
    *_out_val = "int64_t";
    return 0;
  }
  case SQL_TYPE_VARCHAR:
  case SQL_TYPE_TEXT:
  case SQL_TYPE_CHAR:
  case SQL_TYPE_DATE:
  case SQL_TYPE_TIMESTAMP: {
    *_out_val = "char *";
    return 0;
  }
  case SQL_TYPE_FLOAT: {
    *_out_val = "float";
    return 0;
  }
  case SQL_TYPE_DOUBLE:
  case SQL_TYPE_DECIMAL: {
    *_out_val = "double";
    return 0;
  }
  case SQL_TYPE_BOOLEAN: {
    *_out_val = "bool";
    return 0;
  }
  default: {
    *_out_val = "void *";
    return 0;
  }
  }
}

int sql_type_is_string(enum SqlDataType type) {
  switch (type) {
  case SQL_TYPE_VARCHAR:
  case SQL_TYPE_TEXT:
  case SQL_TYPE_CHAR:
  case SQL_TYPE_DATE:
  case SQL_TYPE_TIMESTAMP:
    return 1;
  default:
    return 0;
  }
}

static int emit_c_orm_metadata(FILE *fp, const struct sql_table_t *table,
                               const char *struct_name);
static int emit_c_orm_queries(FILE *fp, const struct sql_table_t *table,
                              const char *struct_name);

int sql_to_c_header_emit(FILE *fp, const struct sql_table_t *table) {
  char *_ast_sql_type_to_c_type_0;
  char table_name_upper[128];
  char struct_name[128];
  size_t i;

  if (!fp || !table || !table->name) {
    return 1;
  }

  str_to_upper(table_name_upper, table->name);
  str_to_title(struct_name, table->name);

  fprintf(fp, "/**\n");
  fprintf(fp, " * @file %s.h\n", table->name);
  fprintf(fp, " * @brief Auto-generated C structs for SQL table %s.\n",
          table->name);
  fprintf(fp, " */\n\n");

  fprintf(fp, "#ifndef C_ORM_MODEL_%s_H\n", table_name_upper);
  fprintf(fp, "#define C_ORM_MODEL_%s_H\n\n", table_name_upper);

  fprintf(fp, "#ifdef __cpuspus\n");
  fprintf(fp, "extern \"C\" {\n");
  fprintf(fp, "#endif /* __cpuspus */\n\n");

  fprintf(fp, "#if defined(_MSC_VER) && _MSC_VER < 1600\n"
              "typedef signed __int8 int8_t;\n"
              "typedef unsigned __int8 uint8_t;\n"
              "typedef signed __int16 int16_t;\n"
              "typedef unsigned __int16 uint16_t;\n"
              "typedef signed __int32 int32_t;\n"
              "typedef unsigned __int32 uint32_t;\n"
              "typedef signed __int64 int64_t;\n"
              "typedef unsigned __int64 uint64_t;\n"
              "#else\n"
              "#include <stdint.h>\n"
              "#endif\n");
  fprintf(fp, "#if defined(_MSC_VER) && _MSC_VER < 1800\n"
              "#if !defined(__cpuspus)\n"
              "#ifndef bool\n"
              "#define bool unsigned char\n"
              "#endif\n"
              "#ifndef true\n"
              "#define true 1\n"
              "#endif\n"
              "#ifndef false\n"
              "#define false 0\n"
              "#endif\n"
              "#endif\n"
              "#else\n"
              "#if defined(_MSC_VER) && _MSC_VER < 1800\n"
              "#ifndef __cpuspus\n"
              "          typedef unsigned char bool;\n"
              "#define true 1\n"
              "#define false 0\n"
              "#endif\n"
              "#else\n"
              "#include <stdbool.h>\n"
              "#endif \n "
              "#endif\n");
  fprintf(fp, "#include <stddef.h>\n\n");

  /* Emit row struct */
  fprintf(fp, "/**\n");
  fprintf(fp, " * @brief Represents a single row of the %s table.\n",
          table->name);
  fprintf(fp, " */\n");
  fprintf(fp, "struct %s {\n", struct_name);

  for (i = 0; i < table->n_columns; ++i) {
    const struct sql_column_t *col = &table->columns[i];
    const char *c_type =
        (sql_type_to_c_type(col->type, &_ast_sql_type_to_c_type_0),
         _ast_sql_type_to_c_type_0);
    int nullable = is_nullable(col);
    int is_str = sql_type_is_string(col->type);

    if (nullable && !is_str) {
      /* Use pointer for nullable primitive types */
      fprintf(fp, "  %s *%s; /**< Nullable */\n", c_type, col->name);
    } else {
      fprintf(fp, "  %s %s;\n", c_type, col->name);
    }
  }
  fprintf(fp, "};\n\n");

  /* Emit array struct */
  fprintf(fp, "/**\n");
  fprintf(fp, " * @brief A collection of %s rows.\n", struct_name);
  fprintf(fp, " */\n");
  fprintf(fp, "struct %s_Array {\n", struct_name);
  fprintf(fp, "  struct %s *data;\n", struct_name);
  fprintf(fp, "  size_t length;\n");
  fprintf(fp, "  size_t capacity;\n");
  fprintf(fp, "};\n\n");

  /* Function declarations */
  fprintf(fp,
          "/**\n * @brief Initialize a %s_Array.\n * @param arr Array to "
          "init.\n * @param initial_capacity Initial capacity.\n * @return 0 "
          "on success.\n */\n",
          struct_name);
  fprintf(
      fp,
      "int %s_Array_init(struct %s_Array *arr, size_t initial_capacity);\n\n",
      struct_name, struct_name);

  fprintf(fp,
          "/**\n * @brief Free resources inside a single %s struct.\n * @param "
          "item Pointer to struct.\n */\n",
          struct_name);
  fprintf(fp, "void %s_free(struct %s *item);\n\n", struct_name, struct_name);

  fprintf(fp,
          "/**\n * @brief Free resources of a %s_Array.\n * @param arr Pointer "
          "to array.\n */\n",
          struct_name);
  fprintf(fp, "void %s_Array_free(struct %s_Array *arr);\n\n", struct_name,
          struct_name);

  fprintf(fp,
          "/**\n * @brief Deep copy a %s row.\n * @param src Source struct.\n "
          "* @param dest Destination struct.\n * @return 0 on success.\n */\n",
          struct_name);
  fprintf(fp, "int %s_deepcopy(const struct %s *src, struct %s *dest);\n\n",
          struct_name, struct_name, struct_name);

  fprintf(fp,
          "/**\n * @brief Deep copy a %s_Array.\n * @param src Source array.\n "
          "* @param dest Destination array.\n * @return 0 on success.\n */\n",
          struct_name);
  fprintf(fp,
          "int %s_Array_deepcopy(const struct %s_Array *src, struct %s_Array "
          "*dest);\n\n",
          struct_name, struct_name, struct_name);

  fprintf(fp, "#ifdef __cpuspus\n");
  fprintf(fp, "}\n");
  fprintf(fp, "#endif /* __cpuspus */\n\n");

  fprintf(fp, "extern const c_orm_table_meta_t %s_meta;\n\n", struct_name);
  fprintf(fp, "#endif /* C_ORM_MODEL_%s_H */\n", table_name_upper);

  return 0;
}

int sql_to_c_source_emit(FILE *fp, const struct sql_table_t *table,
                         const char *header_name) {
  char *_ast_sql_type_to_c_type_1;
  char struct_name[128];
  size_t i;

  if (!fp || !table || !table->name) {
    return 1;
  }

  str_to_title(struct_name, table->name);

  fprintf(fp, "\n");
  fprintf(fp, "#include \"%s\"\n", header_name);
  fprintf(fp, "#include <stdlib.h>\n");
  fprintf(fp, "#include <string.h>\n");
  fprintf(fp, "\n\n");

  /* Array init */
  fprintf(
      fp,
      "int %s_Array_init(struct %s_Array *arr, size_t initial_capacity) {\n",
      struct_name, struct_name);
  fprintf(fp, "  if (!arr) return 1;\n");
  fprintf(fp, "  arr->length = 0;\n");
  fprintf(fp, "  arr->capacity = initial_capacity;\n");
  fprintf(fp, "  if (initial_capacity > 0) {\n");
  fprintf(fp,
          "    arr->data = (struct %s *)calloc(initial_capacity, sizeof(struct "
          "%s));\n",
          struct_name, struct_name);
  fprintf(fp, "    if (!arr->data) return 1;\n");
  fprintf(fp, "  } else {\n");
  fprintf(fp, "    arr->data = NULL;\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  return 0;\n");
  fprintf(fp, "}\n\n");

  /* Free */
  fprintf(fp, "void %s_free(struct %s *item) {\n", struct_name, struct_name);
  fprintf(fp, "  if (!item) return;\n");
  for (i = 0; i < table->n_columns; ++i) {
    const struct sql_column_t *col = &table->columns[i];
    int is_str = sql_type_is_string(col->type);
    int nullable = is_nullable(col);

    if (is_str) {
      fprintf(
          fp,
          "  if (item->%s) {\n    free(item->%s);\n    item->%s = NULL;\n  }\n",
          col->name, col->name, col->name);
    } else if (nullable) {
      /* Free the allocated primitive pointer */
      fprintf(
          fp,
          "  if (item->%s) {\n    free(item->%s);\n    item->%s = NULL;\n  }\n",
          col->name, col->name, col->name);
    }
  }
  fprintf(fp, "}\n\n");

  /* Array Free */
  fprintf(fp, "void %s_Array_free(struct %s_Array *arr) {\n", struct_name,
          struct_name);
  fprintf(fp, "  size_t i;\n");
  fprintf(fp, "  if (!arr) return;\n");
  fprintf(fp, "  if (arr->data) {\n");
  fprintf(fp, "    for (i = 0; i < arr->length; ++i) {\n");
  fprintf(fp, "      %s_free(&arr->data[i]);\n", struct_name);
  fprintf(fp, "    }\n");
  fprintf(fp, "    free(arr->data);\n");
  fprintf(fp, "    arr->data = NULL;\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  arr->length = 0;\n");
  fprintf(fp, "  arr->capacity = 0;\n");
  fprintf(fp, "}\n\n");

  /* Deepcopy */
  fprintf(fp, "int %s_deepcopy(const struct %s *src, struct %s *dest) {\n",
          struct_name, struct_name, struct_name);
  fprintf(fp, "  if (!src || !dest) return 1;\n");
  for (i = 0; i < table->n_columns; ++i) {
    const struct sql_column_t *col = &table->columns[i];
    int is_str = sql_type_is_string(col->type);
    int nullable = is_nullable(col);
    const char *c_type =
        (sql_type_to_c_type(col->type, &_ast_sql_type_to_c_type_1),
         _ast_sql_type_to_c_type_1);

    if (is_str) {
      fprintf(fp, "  if (src->%s) {\n", col->name);
      fprintf(fp, "    dest->%s = (char*)malloc(strlen(src->%s) + 1);\n",
              col->name, col->name);
      fprintf(fp, "    if (!dest->%s) return 1;\n", col->name);
      fprintf(fp,
              "#if defined(_MSC_VER)\n    strcpy_s(dest->%s, strlen(src->%s) + "
              "1, src->%s);\n#else\n    strcpy(dest->%s, src->%s);\n#endif\n",
              col->name, col->name, col->name, col->name, col->name);
      fprintf(fp, "  } else {\n    dest->%s = NULL;\n  }\n", col->name);
    } else if (nullable) {
      fprintf(fp, "  if (src->%s) {\n", col->name);
      fprintf(fp, "    dest->%s = (%s*)malloc(sizeof(%s));\n", col->name,
              c_type, c_type);
      fprintf(fp, "    if (!dest->%s) return 1;\n", col->name);
      fprintf(fp, "    *dest->%s = *src->%s;\n", col->name, col->name);
      fprintf(fp, "  } else {\n    dest->%s = NULL;\n  }\n", col->name);
    } else {
      fprintf(fp, "  dest->%s = src->%s;\n", col->name, col->name);
    }
  }
  fprintf(fp, "  return 0;\n");
  fprintf(fp, "}\n\n");

  /* Array Deepcopy */
  fprintf(fp,
          "int %s_Array_deepcopy(const struct %s_Array *src, struct %s_Array "
          "*dest) {\n",
          struct_name, struct_name, struct_name);
  fprintf(fp, "  size_t i;\n");
  fprintf(fp, "  int err;\n");
  fprintf(fp, "  if (!src || !dest) return 1;\n");
  fprintf(fp, "  err = %s_Array_init(dest, src->capacity);\n", struct_name);
  fprintf(fp, "  if (err) return err;\n");
  fprintf(fp, "  for (i = 0; i < src->length; ++i) {\n");
  fprintf(fp, "    err = %s_deepcopy(&src->data[i], &dest->data[i]);\n",
          struct_name);
  fprintf(fp, "    if (err) {\n");
  fprintf(fp, "      dest->length = i;\n");
  fprintf(fp, "      %s_Array_free(dest);\n", struct_name);
  fprintf(fp, "      return err;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  dest->length = src->length;\n");
  fprintf(fp, "  return 0;\n");
  fprintf(fp, "}\n\n");

  emit_c_orm_queries(fp, table, struct_name);
  emit_c_orm_metadata(fp, table, struct_name);

  return 0;
}
/**
 * @brief Converts a SQL data type enum to its corresponding C ORM string
 * representation.
 */
const char *sql_type_to_c_orm_type(enum SqlDataType type) {
  switch (type) {
  case SQL_TYPE_INT:
    return "C_ORM_TYPE_INT32";
  case SQL_TYPE_BIGINT:
    return "C_ORM_TYPE_INT64";
  case SQL_TYPE_VARCHAR:
  case SQL_TYPE_TEXT:
  case SQL_TYPE_CHAR:
    return "C_ORM_TYPE_STRING";
  case SQL_TYPE_DATE:
    return "C_ORM_TYPE_DATE";
  case SQL_TYPE_TIMESTAMP:
    return "C_ORM_TYPE_TIMESTAMP";
  case SQL_TYPE_FLOAT:
    return "C_ORM_TYPE_FLOAT";
  case SQL_TYPE_DOUBLE:
  case SQL_TYPE_DECIMAL:
    return "C_ORM_TYPE_DOUBLE";
  case SQL_TYPE_BOOLEAN:
    return "C_ORM_TYPE_BOOL";
  default:
    return "C_ORM_TYPE_UNKNOWN";
  }
}
/**
 * @brief Emit c-orm metadata structs into the C source file.
 *
 * @param fp File pointer to write to.
 * @param table The SQL table AST node.
 * @param struct_name The generated struct name.
 * @return 0 on success, non-zero on faiure.
 */
static int emit_c_orm_metadata(FILE *fp, const struct sql_table_t *table,
                               const char *struct_name) {
  size_t i;
  int is_pk;
  int is_null;

  /* Emit column names array */
  fprintf(fp, "const char* %s_col_names[] = {\n", struct_name);
  for (i = 0; i < table->n_columns; ++i) {
    fprintf(fp, "  \"%s\"%s\n", table->columns[i].name,
            i == table->n_columns - 1 ? "" : ",");
  }
  fprintf(fp, "};\n\n");

  /* Emit column types array */
  fprintf(fp, "const c_orm_type_t %s_col_types[] = {\n", struct_name);
  for (i = 0; i < table->n_columns; ++i) {
    fprintf(fp, "  %s%s\n", sql_type_to_c_orm_type(table->columns[i].type),
            i == table->n_columns - 1 ? "" : ",");
  }
  fprintf(fp, "};\n\n");

  /* Emit column offsets array */
  fprintf(fp, "const size_t %s_col_offsets[] = {\n", struct_name);
  for (i = 0; i < table->n_columns; ++i) {
    fprintf(fp, "  offsetof(struct %s, %s)%s\n", struct_name,
            table->columns[i].name, i == table->n_columns - 1 ? "" : ",");
  }
  fprintf(fp, "};\n\n");

  /* Emit PK flags */
  fprintf(fp, "const bool %s_col_is_pk[] = {\n", struct_name);
  for (i = 0; i < table->n_columns; ++i) {
    is_pk = 0;
    if (table->columns[i].n_constraints > 0) {
      size_t j;
      for (j = 0; j < table->columns[i].n_constraints; ++j) {
        if (table->columns[i].constraints[j].type ==
            SQL_CONSTRAINT_PRIMARY_KEY) {
          is_pk = 1;
          break;
        }
      }
    }
    fprintf(fp, "  %s%s\n", is_pk ? "true" : "false",
            i == table->n_columns - 1 ? "" : ",");
  }
  fprintf(fp, "};\n\n");

  /* Emit Nullable flags */
  fprintf(fp, "const bool %s_col_is_nullable[] = {\n", struct_name);
  for (i = 0; i < table->n_columns; ++i) {
    is_null = is_nullable(&table->columns[i]);
    fprintf(fp, "  %s%s\n", is_null ? "true" : "false",
            i == table->n_columns - 1 ? "" : ",");
  }
  fprintf(fp, "};\n\n");

  /* Emit relationships array */
  /* For now we just emit NULLs for targets, in future iterations we can build
   * c_orm_foreign_key_t */
  fprintf(fp, "const char* %s_col_fk_targets[] = {\n", struct_name);
  for (i = 0; i < table->n_columns; ++i) {
    const char *target = "NULL";
    if (table->columns[i].n_constraints > 0) {
      size_t j;
      for (j = 0; j < table->columns[i].n_constraints; ++j) {
        if (table->columns[i].constraints[j].type ==
            SQL_CONSTRAINT_FOREIGN_KEY) {
          target = table->columns[i].constraints[j].reference_table;
          break;
        }
      }
    }
    if (strcmp(target, "NULL") == 0) {
      fprintf(fp, "  NULL%s\n", i == table->n_columns - 1 ? "" : ",");
    } else {
      fprintf(fp, "  \"%s\"%s\n", target, i == table->n_columns - 1 ? "" : ",");
    }
  }
  fprintf(fp, "};\n\n");

  /* Emit struct sizes */
  fprintf(fp, "const size_t %s_struct_size = sizeof(struct %s);\n\n",
          struct_name, struct_name);

  /* Emit fully compiled metadata struct */
  fprintf(fp, "const c_orm_column_meta_t %s_meta_columns[] = {\n", struct_name);
  for (i = 0; i < table->n_columns; ++i) {
    is_pk = 0;
    if (table->columns[i].n_constraints > 0) {
      size_t j;
      for (j = 0; j < table->columns[i].n_constraints; ++j) {
        if (table->columns[i].constraints[j].type ==
            SQL_CONSTRAINT_PRIMARY_KEY) {
          is_pk = 1;
          break;
        }
      }
    }
    is_null = is_nullable(&table->columns[i]);

    fprintf(fp, "  { \"%s\", %s, offsetof(struct %s, %s), %s, %s, NULL }%s\n",
            table->columns[i].name,
            sql_type_to_c_orm_type(table->columns[i].type), struct_name,
            table->columns[i].name, is_pk ? "true" : "false",
            is_null ? "true" : "false", i == table->n_columns - 1 ? "" : ",");
  }
  fprintf(fp, "};\n\n");

  fprintf(fp, "const c_orm_table_meta_t %s_meta = {\n", struct_name);
  fprintf(fp, "  \"%s\",\n", table->name);
  fprintf(fp, "  %s_meta_columns,\n", struct_name);
  fprintf(fp, "  %u,\n", (unsigned int)table->n_columns);
  fprintf(fp, "  sizeof(struct %s),\n", struct_name);
  fprintf(fp, "  %s_query_select_all,\n", struct_name);
  fprintf(fp, "  %s_query_select_by_pk,\n", struct_name);
  fprintf(fp, "  %s_query_insert,\n", struct_name);
  fprintf(fp, "  %s_query_update,\n", struct_name);
  fprintf(fp, "  %s_query_delete_by_pk\n", struct_name);
  fprintf(fp, "};\n\n");

  return 0;
}
/**
 * @brief Emit table string queries.
 */
static int emit_c_orm_queries(FILE *fp, const struct sql_table_t *table,
                              const char *struct_name) {
  size_t i;
  int pk_count = 0;
  const char *pk_name = NULL;

  for (i = 0; i < table->n_columns; ++i) {
    size_t j;
    for (j = 0; j < table->columns[i].n_constraints; ++j) {
      if (table->columns[i].constraints[j].type == SQL_CONSTRAINT_PRIMARY_KEY) {
        pk_count++;
        pk_name = table->columns[i].name;
      }
    }
  }

  fprintf(fp, "#define %s_query_select_all \"SELECT * FROM %s\"\n\n",
          struct_name, table->name);

  if (pk_count == 1 && pk_name) {
    fprintf(fp,
            "#define %s_query_select_by_pk \"SELECT * FROM %s WHERE %s = "
            "?\"\n\n",
            struct_name, table->name, pk_name);
    fprintf(fp,
            "#define %s_query_delete_by_pk \"DELETE FROM %s WHERE %s = "
            "?\"\n\n",
            struct_name, table->name, pk_name);
  } else {
    fprintf(fp, "#define %s_query_select_by_pk NULL\n\n", struct_name);
    fprintf(fp, "#define %s_query_delete_by_pk NULL\n\n", struct_name);
  }

  fprintf(fp, "#define %s_query_insert \"INSERT INTO %s (", struct_name,
          table->name);
  for (i = 0; i < table->n_columns; ++i) {
    fprintf(fp, "%s%s", table->columns[i].name,
            i == table->n_columns - 1 ? "" : ", ");
  }
  fprintf(fp, ") VALUES (");
  for (i = 0; i < table->n_columns; ++i) {
    fprintf(fp, "?%s", i == table->n_columns - 1 ? "" : ", ");
  }
  fprintf(fp, ")\"\n\n");

  fprintf(fp, "#define %s_query_update \"UPDATE %s SET ", struct_name,
          table->name);
  for (i = 0; i < table->n_columns; ++i) {
    fprintf(fp, "%s = ?%s", table->columns[i].name,
            i == table->n_columns - 1 ? "" : ", ");
  }
  if (pk_count == 1 && pk_name) {
    fprintf(fp, " WHERE %s = ?\"\n\n", pk_name);
  } else {
    fprintf(fp, "\"\n\n");
  }

  return 0;
}
/* To be appended to sql_to_c.c */

int sql_to_c_projection_struct_emit(FILE *fp,
                                    const cdd_c_query_projection_t *proj,
                                    const char *struct_name,
                                    unsigned long long *out_hash) {

  size_t i;

  if (!fp || !proj || !struct_name)
    return -1;

  if (out_hash) {
    /* Simplified fast query string hash simulation for external routing
     * metadata tag ID */
    unsigned long long hash = 14695981039346656037ULL;
    const char *s = struct_name; /* In reality we hash the actual SELECT AST,
                                    simplified for mock */
    while (*s) {
      hash ^= (unsigned char)(*s++);
      hash *= 1099511628211ULL;
    }
    *out_hash = hash;

    fprintf(fp, "\n/* Auto-generated Route Hash ID Tag: %llu */\n", hash);
  }

  fprintf(fp, "/**\n");

  fprintf(fp, " * @brief Specific generated struct for a query projection.\n");

  if (proj->source_table) {

    fprintf(fp, " * Target table: %s\n", proj->source_table);
  }

  fprintf(fp, " */\n");

  fprintf(fp, "typedef struct %s {\n", struct_name);

  for (i = 0; i < proj->n_fields; ++i) {

    const cdd_c_query_projection_field_t *field = &proj->fields[i];

    char *c_type = NULL;

    if (sql_type_to_c_type(field->type, &c_type) != 0) {

      /* Fallback to void* or generic byte array if completely unknown, though
       * type inference should catch most */

      fprintf(fp, "    /* Unknown SQL type: %d */\n", field->type);

      fprintf(fp, "    void *%s;\n", field->name);

    } else {

      if (field->is_array) {

        fprintf(fp, "    %s *%s;\n", c_type, field->name);

        fprintf(fp, "    size_t %s_count;\n", field->name);

      } else {

        if (sql_type_is_string(field->type)) {

          /* Depending on length, it could be a fixed buffer or a pointer.
           * Default to pointer for pure queries unless bounded */

          if (field->length > 0) {

            fprintf(fp, "    char %s[%u];\n", field->name,
                    (unsigned int)field->length);

          } else {

            fprintf(fp, "    %s %s;\n", c_type, field->name);
          }

        } else {

          fprintf(fp, "    %s %s;\n", c_type, field->name);
        }
      }
    }
  }

  fprintf(fp, "} %s;\n\n", struct_name);

  /* Generate deep free decl */

  fprintf(fp, "extern C_CDD_EXPORT void %s_free(%s *obj);\n\n", struct_name,
          struct_name);

  return 0;
}

int sql_to_c_projection_free_emit(FILE *fp,
                                  const cdd_c_query_projection_t *proj,
                                  const char *struct_name) {

  size_t i;

  if (!fp || !proj || !struct_name)
    return -1;

  fprintf(fp, "void %s_free(%s *obj) {\n", struct_name, struct_name);

  fprintf(fp, "    if (!obj) return;\n");

  for (i = 0; i < proj->n_fields; ++i) {

    const cdd_c_query_projection_field_t *field = &proj->fields[i];

    if (field->is_array) {
      fprintf(fp, "    if (obj->%s) free(obj->%s);\n", field->name,
              field->name);
    } else if (sql_type_is_string(field->type) && field->length == 0) {
      /* Only free if it's an unbounded pointer string */
      if (field->is_secure) {
        fprintf(fp,
                "    if (obj->%s) { memset(obj->%s, 0, strlen(obj->%s)); "
                "free(obj->%s); }\n",
                field->name, field->name, field->name, field->name);
      } else {
        fprintf(fp, "    if (obj->%s) free(obj->%s);\n", field->name,
                field->name);
      }
    } else if (sql_type_is_string(field->type) && field->length > 0 &&
               field->is_secure) {
      fprintf(fp, "    memset(obj->%s, 0, %u);\n", field->name,
              (unsigned int)field->length);
    }
  }

  fprintf(fp, "}\n\n");

  return 0;
}

int sql_to_c_projection_meta_emit(FILE *fp,
                                  const cdd_c_query_projection_t *proj,
                                  const char *struct_name) {

  size_t i;

  if (!fp || !proj || !struct_name)
    return -1;

  fprintf(fp, "/* Metadata for %s */\n", struct_name);

  fprintf(fp, "static const c_orm_prop_meta_t %s_props[] = {\n", struct_name);

  for (i = 0; i < proj->n_fields; ++i) {

    const cdd_c_query_projection_field_t *field = &proj->fields[i];

    const char *orm_type = sql_type_to_c_orm_type(field->type);

    fprintf(fp, "    {\n");

    fprintf(fp, "        \"%s\",\n", field->name);

    fprintf(fp, "        %s,\n", orm_type ? orm_type : "C_ORM_TYPE_UNKNOWN");

    fprintf(fp, "        offsetof(%s, %s),\n", struct_name, field->name);

    fprintf(fp, "        %s,\n", field->is_array ? "1" : "0");

    fprintf(fp, "        %u,\n", (unsigned int)field->length);

    fprintf(fp, "        %d /* is_secure */\n", field->is_secure ? 1 : 0);
    fprintf(fp, "    }");
    if (i < proj->n_fields - 1) {
      fprintf(fp, ",");
    }

    fprintf(fp, "\n");
  }

  fprintf(fp, "};\n\n");

  fprintf(fp, "const c_orm_meta_t %s_meta = {\n", struct_name);

  fprintf(fp, "    \"%s\",\n", struct_name);

  fprintf(fp, "    sizeof(%s),\n", struct_name);

  fprintf(fp, "    %u,\n", (unsigned int)proj->n_fields);

  fprintf(fp, "    %s_props,\n", struct_name);

  fprintf(fp, "    NULL\n");

  fprintf(fp, "};\n\n");

  return 0;
}

/* To be appended to sql_to_c.c */

int sql_to_c_projection_hydrate_emit(FILE *fp,
                                     const cdd_c_query_projection_t *proj,
                                     const char *struct_name) {

  size_t i;

  if (!fp || !proj || !struct_name)
    return -1;

  fprintf(
      fp,
      "int %s_hydrate(%s *out_struct, const cdd_c_abstract_struct_t *row) {\n",
      struct_name, struct_name);

  fprintf(fp, "    cdd_c_variant_t *val;\n");

  fprintf(fp, "    if (!out_struct || !row) return -1;\n");

  for (i = 0; i < proj->n_fields; ++i) {

    const cdd_c_query_projection_field_t *field = &proj->fields[i];

    fprintf(fp, "    if (cdd_c_abstract_get(row, \"%s\", &val) == 0) {\n",
            field->name);

    switch (field->type) {

    case SQL_TYPE_INT:

    case SQL_TYPE_BIGINT:

    case SQL_TYPE_BOOLEAN:

      fprintf(fp, "        if (val->type == CDD_C_VARIANT_TYPE_INT) {\n");

      fprintf(fp, "            out_struct->%s = val->value.i_val;\n",
              field->name);

      fprintf(fp, "        }\n");

      break;

    case SQL_TYPE_FLOAT:

    case SQL_TYPE_DOUBLE:

      fprintf(fp, "        if (val->type == CDD_C_VARIANT_TYPE_FLOAT) {\n");

      fprintf(fp, "            out_struct->%s = val->value.f_val;\n",
              field->name);

      fprintf(fp, "        }\n");

      break;

    case SQL_TYPE_VARCHAR:

    case SQL_TYPE_TEXT:

      fprintf(fp, "        if (val->type == CDD_C_VARIANT_TYPE_STRING) {\n");

      if (field->length > 0) {

        /* Fixed buffer */

        fprintf(fp,
                "            strncpy(out_struct->%s, val->value.s_val, %u);\n",
                field->name, (unsigned int)field->length);

        fprintf(fp, "            out_struct->%s[%u - 1] = '\\0';\n",
                field->name, (unsigned int)field->length);

      } else {

        /* Dynamic alloc */

        fprintf(fp, "            out_struct->%s = strdup(val->value.s_val);\n",
                field->name);
      }

      fprintf(fp, "        }\n");

      break;

    case SQL_TYPE_UNKNOWN:

      /* Complex blobs handling stubbed for pure C string extraction mapping */

      fprintf(fp, "        /* BLOB extraction handling requires external "
                  "memory allocator mapping */\n");

      break;

    default:

      break;
    }

    fprintf(fp, "    }\n");
  }

  fprintf(fp, "    return 0;\n");

  fprintf(fp, "}\n\n");

  return 0;
}

int sql_to_c_projection_dehydrate_emit(FILE *fp,
                                       const cdd_c_query_projection_t *proj,
                                       const char *struct_name) {

  size_t i;

  if (!fp || !proj || !struct_name)
    return -1;

  fprintf(fp,
          "int %s_dehydrate(const %s *in_struct, cdd_c_abstract_struct_t "
          "*out_row) {\n",
          struct_name, struct_name);

  fprintf(fp, "    cdd_c_variant_t val;\n");

  fprintf(fp, "    if (!in_struct || !out_row) return -1;\n");

  fprintf(fp, "    if (cdd_c_abstract_struct_init(out_row) != 0) return -1;\n");

  for (i = 0; i < proj->n_fields; ++i) {

    const cdd_c_query_projection_field_t *field = &proj->fields[i];

    switch (field->type) {

    case SQL_TYPE_INT:

    case SQL_TYPE_BIGINT:

    case SQL_TYPE_BOOLEAN:

      fprintf(fp, "    val.type = CDD_C_VARIANT_TYPE_INT;\n");

      fprintf(fp, "    val.value.i_val = in_struct->%s;\n", field->name);

      break;

    case SQL_TYPE_FLOAT:

    case SQL_TYPE_DOUBLE:

      fprintf(fp, "    val.type = CDD_C_VARIANT_TYPE_FLOAT;\n");

      fprintf(fp, "    val.value.f_val = in_struct->%s;\n", field->name);

      break;

    case SQL_TYPE_VARCHAR:

    case SQL_TYPE_TEXT:

      fprintf(fp, "    val.type = CDD_C_VARIANT_TYPE_STRING;\n");

      if (field->length > 0) {

        fprintf(fp, "    val.value.s_val = (char*)in_struct->%s;\n",
                field->name);

      } else {

        fprintf(fp, "    val.value.s_val = in_struct->%s;\n", field->name);
      }

      break;

    case SQL_TYPE_UNKNOWN:

      fprintf(fp,
              "    /* Blob dehydration requires memory mapping bypass */\n");

      break;

    default:

      continue;
    }

    fprintf(fp, "    if (cdd_c_abstract_set(out_row, \"%s\", &val) != 0) {\n",
            field->name);

    fprintf(fp, "        cdd_c_abstract_struct_free(out_row);\n");

    fprintf(fp, "        return -1;\n");

    fprintf(fp, "    }\n");
  }

  fprintf(fp, "    return 0;\n");

  fprintf(fp, "}\n\n");

  return 0;
}
/* To be appended to sql_to_c.c */

int sql_to_c_projection_nested_struct_emit(FILE *fp,
                                           const cdd_c_query_projection_t *proj,
                                           const char *struct_name) {

  if (!fp || !proj || !struct_name)
    return -1;

  /* A nested 1-to-1 struct uses the exact same codegen logic as a top level
     projection,

     just rendered within the context of an outer struct */

  return sql_to_c_projection_struct_emit(fp, proj, struct_name, NULL);
}

int sql_to_c_projection_nested_array_emit(FILE *fp,
                                          const cdd_c_query_projection_t *proj,
                                          const char *struct_name,
                                          const char *array_name) {

  if (!fp || !proj || !struct_name || !array_name)
    return -1;

  /* First emit the base struct */

  if (sql_to_c_projection_struct_emit(fp, proj, struct_name, NULL) != 0)
    return -1;

  /* Then emit the array wrapper struct used for 1-to-Many nested responses */

  fprintf(fp, "/**\n");

  fprintf(fp, " * @brief 1-to-Many array container for %s.\n", struct_name);

  fprintf(fp, " */\n");

  fprintf(fp, "typedef struct %s {\n", array_name);

  fprintf(fp, "    %s *items;\n", struct_name);

  fprintf(fp, "    size_t count;\n");

  fprintf(fp, "    size_t capacity;\n");

  fprintf(fp, "} %s;\n\n", array_name);

  /* Array Deep Free handler */

  fprintf(fp, "extern C_CDD_EXPORT void %s_free(%s *arr);\n\n", array_name,
          array_name);

  fprintf(fp, "void %s_free(%s *arr) {\n", array_name, array_name);

  fprintf(fp, "    size_t i;\n");

  fprintf(fp, "    if (!arr || !arr->items) return;\n");

  fprintf(fp, "    for (i = 0; i < arr->count; ++i) {\n");

  fprintf(fp, "        %s_free(&arr->items[i]);\n", struct_name);

  fprintf(fp, "    }\n");

  fprintf(fp, "    free(arr->items);\n");

  fprintf(fp, "    arr->items = NULL;\n");

  fprintf(fp, "    arr->count = 0;\n");

  fprintf(fp, "    arr->capacity = 0;\n");

  fprintf(fp, "}\n\n");

  return 0;
}

/* To be appended to sql_to_c.c */

int sql_to_c_projection_dirty_bitmask_emit(FILE *fp,
                                           const cdd_c_query_projection_t *proj,
                                           const char *struct_name) {

  if (!fp || !proj || !struct_name)
    return -1;

  fprintf(fp, "/**\n");

  fprintf(fp, " * @brief Bitmask struct tracking dirtied fields for %s.\n",
          struct_name);

  fprintf(fp, " */\n");

  fprintf(fp, "typedef struct %s_mask {\n", struct_name);

  if (proj->n_fields == 0) {

    fprintf(fp, "    int _empty_mask;\n");

  } else if (proj->n_fields <= 8) {

    fprintf(fp, "    unsigned char mask;\n");

  } else if (proj->n_fields <= 16) {

    fprintf(fp, "    unsigned short mask;\n");

  } else if (proj->n_fields <= 32) {

    fprintf(fp, "    unsigned int mask;\n");

  } else if (proj->n_fields <= 64) {

    fprintf(fp, "    unsigned long long mask;\n");

  } else {

    size_t arr_size = (proj->n_fields / 64) + 1;

    fprintf(fp, "    unsigned long long mask[%u];\n", (unsigned int)arr_size);
  }

  fprintf(fp, "} %s_mask;\n\n", struct_name);

  return 0;
}

int sql_to_c_projection_union_struct_emit(FILE *fp,
                                          const cdd_c_query_projection_t *projs,
                                          size_t n_projs,
                                          const char *struct_name) {

  size_t i;

  if (!fp || !projs || !struct_name || n_projs == 0)
    return -1;

  fprintf(fp, "/**\n");

  fprintf(fp,
          " * @brief Variant enum and struct for a SQL UNION projection.\n");

  fprintf(fp, " */\n");

  fprintf(fp, "typedef enum %s_type {\n", struct_name);

  fprintf(fp, "    %s_TYPE_UNKNOWN = 0,\n", struct_name);

  for (i = 0; i < n_projs; ++i) {

    fprintf(fp, "    %s_TYPE_BRANCH_%u", struct_name, (unsigned int)i);

    if (i < n_projs - 1)
      fprintf(fp, ",");

    fprintf(fp, "\n");
  }

  fprintf(fp, "} %s_type;\n\n", struct_name);

  fprintf(fp, "typedef struct %s {\n", struct_name);

  fprintf(fp, "    %s_type type;\n", struct_name);

  fprintf(fp, "    union {\n");

  for (i = 0; i < n_projs; ++i) {

    fprintf(fp, "        struct {\n");

    /* Inline struct body logic without full typedefs to keep it contained */

    size_t j;

    for (j = 0; j < projs[i].n_fields; ++j) {

      const cdd_c_query_projection_field_t *field = &projs[i].fields[j];

      char *c_type = NULL;

      if (sql_type_to_c_type(field->type, &c_type) == 0) {

        if (field->is_array) {

          fprintf(fp, "            %s *%s;\n", c_type, field->name);

          fprintf(fp, "            size_t %s_count;\n", field->name);

        } else if (sql_type_is_string(field->type)) {

          if (field->length > 0) {

            fprintf(fp, "            char %s[%u];\n", field->name,
                    (unsigned int)field->length);

          } else {

            fprintf(fp, "            %s %s;\n", c_type, field->name);
          }

        } else {

          fprintf(fp, "            %s %s;\n", c_type, field->name);
        }
      }
    }

    fprintf(fp, "        } branch_%u;\n", (unsigned int)i);
  }

  fprintf(fp, "    } data;\n");

  fprintf(fp, "} %s;\n\n", struct_name);

  /* Auto free */

  fprintf(fp, "extern C_CDD_EXPORT void %s_free(%s *obj);\n\n", struct_name,
          struct_name);

  return 0;
}

int sql_to_c_projection_polymorphic_struct_emit(
    FILE *fp, const cdd_c_query_projection_t *proj, const char *struct_name) {

  if (!fp || !proj || !struct_name)
    return -1;

  /* Polymorphic mappings simply bind the exact target struct directly

     alongside an arbitrary abstract struct payload dictionary payload to catch

     undefined dynamic outputs */

  fprintf(fp, "/**\n");

  fprintf(fp, " * @brief Polymorphic struct capable of mapping explicit fields "
              "and dynamic rows.\n");

  fprintf(fp, " */\n");

  fprintf(fp, "typedef struct %s {\n", struct_name);

  fprintf(fp, "    /* Abstract mapping properties dynamically resolved */\n");

  fprintf(fp, "    cdd_c_abstract_struct_t abstract_properties;\n");

  fprintf(fp, "    \n");

  fprintf(fp, "    /* Specific mapping properties generated strictly */\n");

  /* Use direct struct output logic internally */

  {

    size_t i;

    for (i = 0; i < proj->n_fields; ++i) {

      const cdd_c_query_projection_field_t *field = &proj->fields[i];

      char *c_type = NULL;

      if (sql_type_to_c_type(field->type, &c_type) == 0) {

        if (field->is_array) {

          fprintf(fp, "    %s *%s;\n", c_type, field->name);

          fprintf(fp, "    size_t %s_count;\n", field->name);

        } else if (sql_type_is_string(field->type)) {

          if (field->length > 0) {

            fprintf(fp, "    char %s[%u];\n", field->name,
                    (unsigned int)field->length);

          } else {

            fprintf(fp, "    %s %s;\n", c_type, field->name);
          }

        } else {

          fprintf(fp, "    %s %s;\n", c_type, field->name);
        }
      }
    }
  }

  fprintf(fp, "} %s;\n\n", struct_name);

  return 0;
}
