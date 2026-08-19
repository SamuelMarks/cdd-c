#include "classes/parse/query_projection.h"
#include "c_cdd/memory.h"
#include <stdlib.h>
#include <string.h>

cdd_c_error_t cdd_c_query_projection_init(cdd_c_query_projection_t *proj) {
  if (!proj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  proj->n_fields = 0;
  proj->capacity = 0;
  proj->fields = NULL;
  return CDD_C_SUCCESS;
}

cdd_c_error_t
cdd_c_query_projection_add_field(cdd_c_query_projection_t *proj,
                                 const cdd_c_query_projection_field_t *field) {
  if (!proj || !field)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (proj->n_fields >= proj->capacity) {
    size_t new_cap = proj->capacity == 0 ? 4 : proj->capacity * 2;
    void *new_arr =
        realloc(proj->fields, new_cap * sizeof(cdd_c_query_projection_field_t));
    if (!new_arr)
      return CDD_C_ERROR_MEMORY;
    proj->fields = (cdd_c_query_projection_field_t *)new_arr;
    proj->capacity = new_cap;
  }
  proj->fields[proj->n_fields].name = field->name ? strdup(field->name) : NULL;
  proj->fields[proj->n_fields].original_name =
      field->original_name ? strdup(field->original_name) : NULL;
  proj->fields[proj->n_fields].type = field->type;
  proj->fields[proj->n_fields].is_aggregate = field->is_aggregate;
  proj->n_fields++;
  return CDD_C_SUCCESS;
}

cdd_c_error_t cdd_c_query_projection_free(cdd_c_query_projection_t *proj) {
  size_t i;
  if (!proj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < proj->n_fields; i++) {
    free(proj->fields[i].name);
    free(proj->fields[i].original_name);
  }
  free(proj->fields);
  proj->fields = NULL;
  proj->n_fields = 0;
  proj->capacity = 0;
  return CDD_C_SUCCESS;
}
