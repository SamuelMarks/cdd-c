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
                "            strncpy(out_struct->%s, val->value.s_val, %lu);\n",
                field->name, (unsigned long)field->length);
        fprintf(fp, "            out_struct->%s[%lu - 1] = '\\0';\n",
                field->name, (unsigned long)field->length);
      } else {
        /* Dynamic alloc */
        fprintf(fp, "            out_struct->%s = strdup(val->value.s_val);\n",
                field->name);
      }
      fprintf(fp, "        }\n");
      break;
    case SQL_TYPE_BLOB:
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
    case SQL_TYPE_BLOB:
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
