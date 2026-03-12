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

/**
 * @brief Sanitize an SQL identifier to a valid C identifier.
 * Collisions aren't checked here, but this ensures invalid characters are
 * replaced.
 */
static void sanitize_identifier(char *dst, const char *src) {
  if (!*src) {
    *dst = '\0';
    return;
  }
  /* First char must be alpha or underscore */
  if (isalpha((unsigned char)*src) || *src == '_') {
    *dst = *src;
  } else {
    *dst = '_'; /* Fallback if somehow numeric started */
  }
  dst++;
  src++;
  while (*src) {
    if (isalnum((unsigned char)*src) || *src == '_') {
      *dst = *src;
    } else {
      *dst = '_';
    }
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

const char *sql_type_to_c_type(enum SqlDataType type) {
  switch (type) {
  case SQL_TYPE_INT:
    return "int32_t";
  case SQL_TYPE_BIGINT:
    return "int64_t";
  case SQL_TYPE_VARCHAR:
  case SQL_TYPE_TEXT:
  case SQL_TYPE_CHAR:
  case SQL_TYPE_DATE:
  case SQL_TYPE_TIMESTAMP:
    return "char *";
  case SQL_TYPE_FLOAT:
    return "float";
  case SQL_TYPE_DOUBLE:
  case SQL_TYPE_DECIMAL:
    return "double";
  case SQL_TYPE_BOOLEAN:
    return "bool";
  default:
    return "void *";
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

int sql_to_c_header_emit(FILE *fp, const struct sql_table_t *table) {
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

  fprintf(fp, "#include <stdint.h>\n");
  fprintf(fp, "#include <stdbool.h>\n");
  fprintf(fp, "#include <stddef.h>\n\n");

  /* Emit row struct */
  fprintf(fp, "/**\n");
  fprintf(fp, " * @brief Represents a single row of the %s table.\n",
          table->name);
  fprintf(fp, " */\n");
  fprintf(fp, "struct %s {\n", struct_name);

  for (i = 0; i < table->n_columns; ++i) {
    const struct sql_column_t *col = &table->columns[i];
    const char *c_type = sql_type_to_c_type(col->type);
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

  fprintf(fp, "#endif /* C_ORM_MODEL_%s_H */\n", table_name_upper);

  return 0;
}

int sql_to_c_source_emit(FILE *fp, const struct sql_table_t *table,
                         const char *header_name) {
  char struct_name[128];
  size_t i;

  if (!fp || !table || !table->name) {
    return 1;
  }

  str_to_title(struct_name, table->name);

  fprintf(fp, "/* clang-format off */\n");
  fprintf(fp, "#include \"%s\"\n", header_name);
  fprintf(fp, "#include <stdlib.h>\n");
  fprintf(fp, "#include <string.h>\n");
  fprintf(fp, "/* clang-format on */\n\n");

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
    const char *c_type = sql_type_to_c_type(col->type);

    if (is_str) {
      fprintf(fp, "  if (src->%s) {\n", col->name);
      fprintf(fp, "    dest->%s = (char*)malloc(strlen(src->%s) + 1);\n",
              col->name, col->name);
      fprintf(fp, "    if (!dest->%s) return 1;\n", col->name);
      fprintf(fp, "#if defined(_MSC_VER)
    strcpy_s(dest->%s, sizeof(dest->%s), src->%s);
#else
    strcpy(dest->%s, src->%s);
#endif \n ", col->name, col->name);
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

  return 0;
}