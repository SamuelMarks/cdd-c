#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#ifndef C_CDD_QUERY_PROJECTION_H
#define C_CDD_QUERY_PROJECTION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <stddef.h>
/* clang-format on */

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

C_CDD_EXPORT cdd_c_error_t
cdd_c_query_projection_init(cdd_c_query_projection_t *proj);
C_CDD_EXPORT cdd_c_error_t
cdd_c_query_projection_add_field(cdd_c_query_projection_t *proj,
                                 const cdd_c_query_projection_field_t *field);
C_CDD_EXPORT cdd_c_error_t
cdd_c_query_projection_free(cdd_c_query_projection_t *proj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_QUERY_PROJECTION_H */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
