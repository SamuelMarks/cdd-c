/**
 * @file query_projection.c
 * @brief Implementation of CDD_C query projection AST representation
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "classes/parse/query_projection.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cdd_c_query_projection_init(cdd_c_query_projection_t *proj) {
  if (!proj)
    return -1;
  proj->fields = NULL;
  proj->n_fields = 0;
  proj->capacity = 0;
  proj->source_table = NULL;
  proj->mapping_meta.target_name = NULL;
  proj->mapping_meta.kind = CDD_C_MAPPING_KIND_SPECIFIC;
  return 0;
}

static int duplicate_string_qp(const char *src, char **dest) {
  size_t len;
  if (!dest)
    return -1;
  if (!src) {
    *dest = NULL;
    return 0;
  }
  len = strlen(src);
  *dest = (char *)malloc(len + 1);
  if (!*dest)
    return -1;
  memcpy(*dest, src, len + 1);
  return 0;
}

int cdd_c_query_projection_add_field(
    cdd_c_query_projection_t *proj,
    const cdd_c_query_projection_field_t *field) {
  cdd_c_query_projection_field_t *new_fields;
  size_t new_cap;
  if (!proj || !field)
    return -1;

  if (proj->n_fields >= proj->capacity) {
    new_cap = proj->capacity == 0 ? 4 : proj->capacity * 2;
    new_fields = (cdd_c_query_projection_field_t *)realloc(
        proj->fields, new_cap * sizeof(cdd_c_query_projection_field_t));
    if (!new_fields)
      return -1;
    proj->fields = new_fields;
    proj->capacity = new_cap;
  }

  if (duplicate_string_qp(field->name, &proj->fields[proj->n_fields].name) != 0)
    return -1;
  if (duplicate_string_qp(field->original_name,
                          &proj->fields[proj->n_fields].original_name) != 0) {
    free(proj->fields[proj->n_fields].name);
    return -1;
  }
  proj->fields[proj->n_fields].type = field->type;
  proj->fields[proj->n_fields].is_aggregate = field->is_aggregate;
  proj->fields[proj->n_fields].length = field->length;
  proj->fields[proj->n_fields].is_array = field->is_array;
  proj->fields[proj->n_fields].is_secure = field->is_secure;

  proj->n_fields++;
  return 0;
}

int cdd_c_query_projection_free(cdd_c_query_projection_t *proj) {
  size_t i;
  if (!proj)
    return -1;

  for (i = 0; i < proj->n_fields; ++i) {
    if (proj->fields[i].name)
      free(proj->fields[i].name);
    if (proj->fields[i].original_name)
      free(proj->fields[i].original_name);
  }
  if (proj->fields)
    free(proj->fields);
  if (proj->source_table)
    free(proj->source_table);
  if (proj->mapping_meta.target_name)
    free(proj->mapping_meta.target_name);

  proj->fields = NULL;
  proj->n_fields = 0;
  proj->capacity = 0;
  proj->source_table = NULL;
  proj->mapping_meta.target_name = NULL;
  return 0;
}
