/**
 * @file orm_gen.c
 * @brief Implementation of ORM wrapper generation.
 *
 * Emits c-orm database bindings from an OpenAPI definition.
 */

/* clang-format off */
#include "routes/emit/orm_gen.h"
#include "classes/emit/struct.h"
#include "classes/emit/c_orm_meta.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
#define SNPRINTF _snprintf
#else
#define SNPRINTF snprintf
#endif

/**
 * @brief Parses the database schema extension from the OpenAPI struct field.
 *
 * @param[in] field The OpenAPI struct field containing metadata and extensions.
 * @param[out] is_pk Pointer to int set to 1 if field is a primary key, 0
 * otherwise.
 * @param[out] is_unique Pointer to int set to 1 if field is unique, 0
 * otherwise.
 * @param[out] is_index Pointer to int set to 1 if field is indexed, 0
 * otherwise.
 * @param[out] fk_buf Buffer to store foreign key reference string.
 * @param[in] fk_buf_size Size of the foreign key buffer.
 */
static void check_db_schema(const struct StructField *field, int *is_pk,
                            int *is_unique, int *is_index, char *fk_buf,
                            size_t fk_buf_size) {
  *is_pk = 0;
  *is_unique = 0;
  *is_index = 0;
  if (fk_buf && fk_buf_size > 0)
    fk_buf[0] = '\0';

  if (field->description[0]) {
    if (strstr(field->description, "[PK]") != NULL)
      *is_pk = 1;
    if (strstr(field->description, "[UNIQUE]") != NULL)
      *is_unique = 1;
    if (strstr(field->description, "[INDEX]") != NULL)
      *is_index = 1;
    if (fk_buf && fk_buf_size > 0) {
      char *fk_start = strstr(field->description, "[FK=");
      if (fk_start) {
        char *fk_end = strchr(fk_start, ']');
        if (fk_end && (size_t)(fk_end - fk_start - 4) < fk_buf_size) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
          strncpy_s(fk_buf, fk_buf_size, fk_start + 4, fk_end - fk_start - 4);
#else
          strncpy(fk_buf, fk_start + 4, fk_end - fk_start - 4);
#endif
          fk_buf[fk_end - fk_start - 4] = '\0';
        }
      }
    }
  }

  if (field->schema_extra_json) {
    JSON_Value *extras = json_parse_string(field->schema_extra_json);
    if (extras) {
      JSON_Object *obj = json_value_get_object(extras);
      if (obj) {
        JSON_Object *db = json_object_get_object(obj, "x-db-schema");
        if (db) {
          if (json_object_has_value(db, "primary_key") &&
              json_object_get_boolean(db, "primary_key") == 1)
            *is_pk = 1;
          if (json_object_has_value(db, "unique") &&
              json_object_get_boolean(db, "unique") == 1)
            *is_unique = 1;
          if (json_object_has_value(db, "index") &&
              json_object_get_boolean(db, "index") == 1)
            *is_index = 1;
          if (fk_buf && fk_buf_size > 0 && json_object_has_value(db, "fk")) {
            const char *fk = json_object_get_string(db, "fk");
            if (fk) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
              strncpy_s(fk_buf, fk_buf_size, fk, fk_buf_size - 1);
#else
              strncpy(fk_buf, fk, fk_buf_size - 1);
#endif
              fk_buf[fk_buf_size - 1] = '\0';
            }
          }
        }
      }
      json_value_free(extras);
    }
  }
}

static const char *openapi_type_to_c_orm_type(const struct StructField *field) {
  if (strcmp(field->type, "integer") == 0) {
    if (field->format[0]) {
      if (strcmp(field->format, "int64") == 0)
        return "C_ORM_TYPE_INT64";
      return "C_ORM_TYPE_INT32";
    }
    return "C_ORM_TYPE_INT32";
  }
  if (strcmp(field->type, "number") == 0) {
    if (strcmp(field->format, "float") == 0)
      return "C_ORM_TYPE_FLOAT";
    return "C_ORM_TYPE_DOUBLE";
  }
  if (strcmp(field->type, "boolean") == 0)
    return "C_ORM_TYPE_BOOL";
  if (strcmp(field->type, "string") == 0) {
    if (strcmp(field->format, "date") == 0)
      return "C_ORM_TYPE_DATE";
    if (strcmp(field->format, "date-time") == 0)
      return "C_ORM_TYPE_TIMESTAMP";
    return "C_ORM_TYPE_STRING";
  }
  return "C_ORM_TYPE_UNKNOWN";
}

static const char *openapi_type_to_c_type(const struct StructField *field) {
  if (strcmp(field->type, "integer") == 0) {
    if (field->format[0]) {
      if (strcmp(field->format, "int64") == 0)
        return "int64_t";
      return "int32_t";
    }
    return "int32_t";
  }
  if (strcmp(field->type, "number") == 0) {
    if (strcmp(field->format, "float") == 0)
      return "float";
    return "double";
  }
  if (strcmp(field->type, "boolean") == 0)
    return "bool";
  if (strcmp(field->type, "string") == 0)
    return "char*";
  if (strcmp(field->type, "object") == 0) {
    /* Assuming ref contains the type name */
    static char buf[128];
    if (field->ref[0]) {
      SNPRINTF(buf, sizeof(buf), "struct %s*", field->ref);
      return buf;
    }
  }
  return "void*";
}

/**
 * @brief Autogenerated docstring
 */
int openapi_orm_generate(const struct OpenAPI_Spec *spec,
                         const struct OpenApiClientConfig *config) {
  char path_h[1024];
  char path_c[1024];
  FILE *fp_h = NULL;
  FILE *fp_c = NULL;
  size_t i, j;
  char *model_h = NULL;

  if (!spec || !config || !config->filename_base)
    return EINVAL;

  if (config->model_header) {
    size_t header_len = strlen(config->model_header);
    model_h = malloc(header_len + 1);
    if (model_h) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      strcpy_s(model_h, header_len + 1, config->model_header);
#else
      strcpy(model_h, config->model_header);
#endif
    }
  } else {
    size_t len = strlen(config->filename_base) + 10;
    model_h = malloc(len + 1);
    if (model_h) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      sprintf_s(model_h, len + 1, "%s_models.h", config->filename_base);
#else
      sprintf(model_h, "%s_models.h", config->filename_base);
#endif
    }
  }

  if (!model_h)
    return ENOMEM;

  SNPRINTF(path_h, sizeof(path_h), "%s", model_h);
  SNPRINTF(path_c, sizeof(path_c), "%.*s.c", (int)(strlen(model_h) - 2),
           model_h);

#if defined(_MSC_VER)
  if (fopen_s(&fp_h, path_h, "w") != 0)
    fp_h = NULL;
  if (fopen_s(&fp_c, path_c, "w") != 0)
    fp_c = NULL;
#else
  fp_h = fopen(path_h, "w");
  fp_c = fopen(path_c, "w");
#endif

  if (!fp_h || !fp_c) {
    free(model_h);
    if (fp_h)
      fclose(fp_h);
    if (fp_c)
      fclose(fp_c);
    return EIO;
  }

  /* Header */
  fprintf(fp_h, "/* Generated ORM Models from OpenAPI Specification */\n\n");
  fprintf(fp_h, "#ifndef C_ORM_MODELS_H\n");
  fprintf(fp_h, "#define C_ORM_MODELS_H\n\n");
  fprintf(fp_h, "#ifdef __cplusplus\n");
  fprintf(fp_h, "extern \"C\" {\n");
  fprintf(fp_h, "#endif /* __cplusplus */\n\n");

  fprintf(fp_h, "#if defined(_MSC_VER) && _MSC_VER < 1600\n");
  fprintf(fp_h, "typedef signed __int8 int8_t;\n");
  fprintf(fp_h, "typedef unsigned __int8 uint8_t;\n");
  fprintf(fp_h, "typedef signed __int16 int16_t;\n");
  fprintf(fp_h, "typedef unsigned __int16 uint16_t;\n");
  fprintf(fp_h, "typedef signed __int32 int32_t;\n");
  fprintf(fp_h, "typedef unsigned __int32 uint32_t;\n");
  fprintf(fp_h, "typedef signed __int64 int64_t;\n");
  fprintf(fp_h, "typedef unsigned __int64 uint64_t;\n");
  fprintf(fp_h, "#else\n");
  fprintf(fp_h, "#include <stdint.h>\n");
  fprintf(fp_h, "#endif\n");
  fprintf(fp_h, "#if defined(_MSC_VER) && _MSC_VER < 1800\n");
  fprintf(fp_h, "#if !defined(__cplusplus)\n");
  fprintf(fp_h, "#ifndef bool\n");
  fprintf(fp_h, "#define bool unsigned char\n");
  fprintf(fp_h, "#endif\n");
  fprintf(fp_h, "#ifndef true\n");
  fprintf(fp_h, "#define true 1\n");
  fprintf(fp_h, "#endif\n");
  fprintf(fp_h, "#ifndef false\n");
  fprintf(fp_h, "#define false 0\n");
  fprintf(fp_h, "#endif\n");
  fprintf(fp_h, "#endif /* __cplusplus */\n");
  fprintf(fp_h, "#else\n");
  fprintf(fp_h, "#include <stdbool.h>\n");
  fprintf(fp_h, "#endif\n\n");
  fprintf(fp_h, "#include <stddef.h>\n");
  fprintf(fp_h, "#include \"classes/emit/c_orm_meta.h\"\n");
  fprintf(fp_h, "#include \"c_orm_db.h\"\n");
  fprintf(fp_h, "#include \"c_orm_api.h\"\n\n");

  /* Source */
  fprintf(fp_c, "/* Generated ORM Models Implementation */\n\n");
  fprintf(fp_c, "#include \"%s\"\n", model_h);
  fprintf(fp_c, "#include <stdlib.h>\n");
  fprintf(fp_c, "#include <string.h>\n\n");

  for (i = 0; i < spec->n_defined_schemas; ++i) {
    const char *struct_name = spec->defined_schema_names[i];
    const struct StructFields *sf = &spec->defined_schemas[i];

    if (!struct_name || !sf)
      continue;

    /* Generate C Struct */
    fprintf(fp_h, "struct %s {\n", struct_name);
    for (j = 0; j < sf->size; ++j) {
      const struct StructField *field = &sf->fields[j];
      fprintf(fp_h, "  %s %s;\n", openapi_type_to_c_type(field), field->name);
    }
    fprintf(fp_h, "};\n\n");

    /* ORM Metadata declarations */
    fprintf(fp_h, "extern const c_orm_column_meta_t %s_cols[%lu];\n",
            struct_name, (unsigned long)sf->size);
    fprintf(fp_h, "extern const c_orm_table_meta_t %s_meta;\n\n", struct_name);

    /* CRUD Boilerplate declarations */
    fprintf(fp_h, "c_orm_error_t create_table_%s(c_orm_db_t* db);\n",
            struct_name);
    fprintf(fp_h,
            "c_orm_error_t insert_%s(c_orm_db_t* db, const struct %s* obj);\n",
            struct_name, struct_name);
    for (j = 0; j < sf->size; ++j) {
      const struct StructField *field = &sf->fields[j];
      int is_pk, is_unique, is_index;
      check_db_schema(field, &is_pk, &is_unique, &is_index, NULL, 0);
      if (is_pk || is_unique || is_index) {
        fprintf(fp_h,
                "c_orm_error_t get_%s_by_%s(c_orm_db_t* db, const %s %s, "
                "struct %s** out_obj);\n",
                struct_name, field->name, openapi_type_to_c_type(field),
                field->name, struct_name);
      }
    }
    fprintf(fp_h, "\n");

    /* ORM Metadata definitions */
    fprintf(fp_c, "const c_orm_column_meta_t %s_cols[%lu] = {\n", struct_name,
            (unsigned long)sf->size);
    for (j = 0; j < sf->size; ++j) {
      const struct StructField *field = &sf->fields[j];
      int is_pk, is_unique, is_index;
      char fk_buf[256];
      check_db_schema(field, &is_pk, &is_unique, &is_index, fk_buf,
                      sizeof(fk_buf));

      if (fk_buf[0]) {
        fprintf(fp_c,
                "  { \"%s\", %s, offsetof(struct %s, %s), %s, %s, \"%s\" }%s\n",
                field->name, openapi_type_to_c_orm_type(field), struct_name,
                field->name, is_pk ? "true" : "false", is_pk ? "false" : "true",
                fk_buf, j == sf->size - 1 ? "" : ",");
      } else {
        fprintf(fp_c,
                "  { \"%s\", %s, offsetof(struct %s, %s), %s, %s, NULL }%s\n",
                field->name, openapi_type_to_c_orm_type(field), struct_name,
                field->name, is_pk ? "true" : "false", is_pk ? "false" : "true",
                j == sf->size - 1 ? "" : ",");
      }
    }
    fprintf(fp_c, "};\n\n");

    fprintf(fp_c, "const c_orm_table_meta_t %s_meta = {\n", struct_name);
    fprintf(fp_c, "  \"%s\",\n", struct_name);
    fprintf(fp_c, "  %s_cols,\n", struct_name);
    fprintf(fp_c, "  %lu,\n", (unsigned long)sf->size);
    fprintf(fp_c, "  sizeof(struct %s),\n", struct_name);
    fprintf(fp_c, "  \"SELECT * FROM %s\",\n", struct_name);
    {
      const char *pk_name = NULL;
      size_t k;
      int first_update = 1;
      for (k = 0; k < sf->size; ++k) {
        int is_pk, is_unique, is_index;
        check_db_schema(&sf->fields[k], &is_pk, &is_unique, &is_index, NULL, 0);
        if (is_pk) {
          pk_name = sf->fields[k].name;
          break;
        }
      }

      if (pk_name) {
        fprintf(fp_c, "  \"SELECT * FROM %s WHERE %s = ?\",\n", struct_name,
                pk_name);
      } else {
        fprintf(fp_c, "  NULL,\n");
      }

      fprintf(fp_c, "  \"INSERT INTO %s (", struct_name);
      for (k = 0; k < sf->size; ++k) {
        fprintf(fp_c, "%s%s", sf->fields[k].name,
                k == sf->size - 1 ? "" : ", ");
      }
      fprintf(fp_c, ") VALUES (");
      for (k = 0; k < sf->size; ++k) {
        fprintf(fp_c, "?%s", k == sf->size - 1 ? "" : ", ");
      }
      fprintf(fp_c, ")\",\n");

      if (pk_name) {
        fprintf(fp_c, "  \"UPDATE %s SET ", struct_name);
        for (k = 0; k < sf->size; ++k) {
          if (strcmp(sf->fields[k].name, pk_name) == 0)
            continue;
          if (!first_update) {
            fprintf(fp_c, ", ");
          }
          fprintf(fp_c, "%s = ?", sf->fields[k].name);
          first_update = 0;
        }
        fprintf(fp_c, " WHERE %s = ?\",\n", pk_name);
        fprintf(fp_c, "  \"DELETE FROM %s WHERE %s = ?\"\n", struct_name,
                pk_name);
      } else {
        fprintf(fp_c, "  NULL,\n  NULL\n");
      }
    }
    fprintf(fp_c, "};\n\n");

    /* CRUD Boilerplate definitions */
    fprintf(fp_c, "c_orm_error_t create_table_%s(c_orm_db_t* db) {\n",
            struct_name);
    fprintf(fp_c, "  c_orm_query_t *query = NULL;\n");
    fprintf(fp_c, "  int has_row = 0;\n");
    fprintf(fp_c, "  /* Simple CREATE TABLE placeholder, use real DDL "
                  "generator in production */\n");
    fprintf(fp_c, "  if (db && db->vtable && db->vtable->prepare) {\n");
    fprintf(fp_c,
            "    if (db->vtable->prepare(db, \"CREATE TABLE IF NOT EXISTS %s "
            "(id INTEGER PRIMARY KEY)\", &query) == 0) {\n",
            struct_name);
    fprintf(fp_c, "      db->vtable->step(query, &has_row);\n");
    fprintf(fp_c, "      db->vtable->finalize(query);\n");
    fprintf(fp_c, "    }\n");
    for (j = 0; j < sf->size; ++j) {
      int is_pk, is_unique, is_index;
      check_db_schema(&sf->fields[j], &is_pk, &is_unique, &is_index, NULL, 0);

      if (is_index) {
        fprintf(fp_c,
                "    if (db->vtable->prepare(db, \"CREATE INDEX IF NOT EXISTS "
                "idx_%s_%s ON %s(%s)\", &query) == 0) {\n",
                struct_name, sf->fields[j].name, struct_name,
                sf->fields[j].name);
        fprintf(fp_c, "      db->vtable->step(query, &has_row);\n");
        fprintf(fp_c, "      db->vtable->finalize(query);\n");
        fprintf(fp_c, "    }\n");
      }
      if (is_unique) {
        fprintf(fp_c,
                "    if (db->vtable->prepare(db, \"CREATE UNIQUE INDEX IF NOT "
                "EXISTS uidx_%s_%s ON %s(%s)\", &query) == 0) {\n",
                struct_name, sf->fields[j].name, struct_name,
                sf->fields[j].name);
        fprintf(fp_c, "      db->vtable->step(query, &has_row);\n");
        fprintf(fp_c, "      db->vtable->finalize(query);\n");
        fprintf(fp_c, "    }\n");
      }
    }
    fprintf(fp_c, "  }\n");
    fprintf(fp_c, "  return C_ORM_SUCCESS;\n");
    fprintf(fp_c, "}\n\n");

    fprintf(fp_c,
            "c_orm_error_t insert_%s(c_orm_db_t* db, const struct %s* obj) {\n",
            struct_name, struct_name);
    fprintf(fp_c, "  return c_orm_insert(db, &%s_meta, obj);\n", struct_name);
    fprintf(fp_c, "}\n\n");

    for (j = 0; j < sf->size; ++j) {
      const struct StructField *field = &sf->fields[j];
      int is_pk, is_unique, is_index;
      check_db_schema(field, &is_pk, &is_unique, &is_index, NULL, 0);

      if (is_pk || is_unique || is_index) {
        fprintf(fp_c,
                "c_orm_error_t get_%s_by_%s(c_orm_db_t* db, const %s "
                "%s, struct %s** out_obj) {\n",
                struct_name, field->name, openapi_type_to_c_type(field),
                field->name, struct_name);
        if (strcmp(openapi_type_to_c_type(field), "int32_t") == 0 ||
            strcmp(openapi_type_to_c_type(field), "int64_t") == 0) {
          fprintf(fp_c,
                  "  if (out_obj) *out_obj = malloc(sizeof(struct %s));\n",
                  struct_name);
          fprintf(fp_c, "  if (out_obj && !*out_obj) return "
                        "C_ORM_ERROR_MEMORY;\n");
          fprintf(fp_c,
                  "  return c_orm_find_by_id_int32(db, &%s_meta, "
                  "(int32_t)%s, *out_obj);\n",
                  struct_name, field->name);
        } else if (strcmp(openapi_type_to_c_type(field), "char*") == 0) {
          fprintf(fp_c,
                  "  if (out_obj) *out_obj = malloc(sizeof(struct %s));\n",
                  struct_name);
          fprintf(fp_c, "  if (out_obj && !*out_obj) return "
                        "C_ORM_ERROR_MEMORY;\n");
          fprintf(fp_c,
                  "  return c_orm_find_by_string(db, &%s_meta, \"%s\", "
                  "%s, *out_obj);\n",
                  struct_name, field->name, field->name);
        } else {
          fprintf(fp_c, "  (void)db;\n");
          fprintf(fp_c, "  (void)%s;\n", field->name);
          fprintf(fp_c, "  if (out_obj) *out_obj = NULL;\n");
          fprintf(fp_c, "  return C_ORM_ERROR_NOT_IMPLEMENTED;\n");
        }
        fprintf(fp_c, "}\n\n");
      }
    }
  }

  fprintf(fp_h, "#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n");
  fprintf(fp_h, "#endif /* C_ORM_MODELS_H */\n");

  /* Test Stub Generation */
  {
    char test_path[1024];
    FILE *fp_test = NULL;
    SNPRINTF(test_path, sizeof(test_path), "test_%s_models.c",
             config->filename_base);
#if defined(_MSC_VER)
    if (fopen_s(&fp_test, test_path, "w") != 0)
      fp_test = NULL;
#else
    fp_test = fopen(test_path, "w");
#endif
    if (fp_test) {
      fprintf(fp_test,
              "/* Auto-generated greatest.h test stub for ORM models */\n\n");
      fprintf(fp_test, "#include <stdlib.h>\n");
      fprintf(fp_test, "#include <string.h>\n");
      fprintf(fp_test, "#include <greatest.h>\n");
      fprintf(fp_test, "#include \"%s\"\n\n", model_h);

      for (i = 0; i < spec->n_defined_schemas; i++) {
        const char *struct_name = spec->defined_schema_names[i];
        fprintf(fp_test, "TEST test_orm_%s_meta(void) {\n", struct_name);
        fprintf(fp_test, "  /* Check C-ORM meta structure fields */\n");
        fprintf(fp_test, "  ASSERT_STR_EQ(\"%s\", %s_meta.table_name);\n",
                struct_name, struct_name);
        fprintf(fp_test, "  PASS();\n");
        fprintf(fp_test, "}\n\n");
      }

      fprintf(fp_test, "SUITE(orm_models_suite) {\n");
      for (i = 0; i < spec->n_defined_schemas; i++) {
        fprintf(fp_test, "  RUN_TEST(test_orm_%s_meta);\n",
                spec->defined_schema_names[i]);
      }
      fprintf(fp_test, "}\n");
      fclose(fp_test);
    }
  }

  fclose(fp_h);
  fclose(fp_c);
  free(model_h);

  return 0;
}
