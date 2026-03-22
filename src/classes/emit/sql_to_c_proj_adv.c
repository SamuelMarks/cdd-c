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
    fprintf(fp, "    unsigned long long mask[%lu];\n", (unsigned long)arr_size);
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
    fprintf(fp, "    %s_TYPE_BRANCH_%lu", struct_name, (unsigned long)i);
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
            fprintf(fp, "            char %s[%lu];\n", field->name,
                    (unsigned long)field->length);
          } else {
            fprintf(fp, "            %s %s;\n", c_type, field->name);
          }
        } else {
          fprintf(fp, "            %s %s;\n", c_type, field->name);
        }
      }
    }
    fprintf(fp, "        } branch_%lu;\n", (unsigned long)i);
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
  return 0;
}
