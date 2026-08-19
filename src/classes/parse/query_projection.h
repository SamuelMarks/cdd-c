#ifndef C_CDD_QUERY_PROJECTION_H
#define C_CDD_QUERY_PROJECTION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cdd_c_error.h"
#include <stddef.h>

typedef enum { SQL_TYPE_INT = 1, SQL_TYPE_VARCHAR = 2 } cdd_c_sql_type_t;

typedef struct {
  char *name;
  char *original_name;
  cdd_c_sql_type_t type;
  int is_aggregate;
} cdd_c_query_projection_field_t;

typedef struct {
  size_t n_fields;
  size_t capacity;
  cdd_c_query_projection_field_t *fields;
} cdd_c_query_projection_t;

cdd_c_error_t cdd_c_query_projection_init(cdd_c_query_projection_t *proj);
cdd_c_error_t
cdd_c_query_projection_add_field(cdd_c_query_projection_t *proj,
                                 const cdd_c_query_projection_field_t *field);
cdd_c_error_t cdd_c_query_projection_free(cdd_c_query_projection_t *proj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_QUERY_PROJECTION_H */
