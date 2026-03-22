/* To be appended to sql_to_c.c */

int sql_to_c_projection_nested_struct_emit(FILE *fp,
                                           const cdd_c_query_projection_t *proj,
                                           const char *struct_name) {
  if (!fp || !proj || !struct_name)
    return -1;

  /* A nested 1-to-1 struct uses the exact same codegen logic as a top level
     projection, just rendered within the context of an outer struct */
  return sql_to_c_projection_struct_emit(fp, proj, struct_name);
}

int sql_to_c_projection_nested_array_emit(FILE *fp,
                                          const cdd_c_query_projection_t *proj,
                                          const char *struct_name,
                                          const char *array_name) {
  if (!fp || !proj || !struct_name || !array_name)
    return -1;

  /* First emit the base struct */
  if (sql_to_c_projection_struct_emit(fp, proj, struct_name) != 0)
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