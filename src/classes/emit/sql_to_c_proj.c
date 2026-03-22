/* To be appended to sql_to_c.c */

int sql_to_c_projection_struct_emit(FILE *fp,
                                    const cdd_c_query_projection_t *proj,
                                    const char *struct_name) {
  size_t i;
  if (!fp || !proj || !struct_name)
    return -1;

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
            fprintf(fp, "    char %s[%lu];\n", field->name,
                    (unsigned long)field->length);
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
      fprintf(fp, "    if (obj->%s) free(obj->%s);\n", field->name,
              field->name);
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
    fprintf(fp, "        %lu,\n", (unsigned long)field->length);
    fprintf(fp, "        0 /* is_secure flag placeholder */\n");
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
  fprintf(fp, "    %lu,\n", (unsigned long)proj->n_fields);
  fprintf(fp, "    %s_props\n", struct_name);
  fprintf(fp, "};\n\n");

  return 0;
}
