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

  fprintf(fp, "#ifdef __cplusplus\n");
  fprintf(fp, "extern \"C\" {\n");
  fprintf(fp, "#endif /* __cplusplus */\n\n");

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
              "#if !defined(__cplusplus)\n"
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
              "#ifndef __cplusplus\n"
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

  fprintf(fp, "#ifdef __cplusplus\n");
  fprintf(fp, "}\n");
  fprintf(fp, "#endif /* __cplusplus */\n\n");

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
 * @brief Autogenerated docstring
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
 * @return 0 on success, non-zero on failure.
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
  fprintf(fp, "  %lu,\n", (unsigned long)table->n_columns);
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
