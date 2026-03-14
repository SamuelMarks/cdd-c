/**
 * @file c_orm_api.c
 * @brief Implementation of high-level API for c-orm.
 */

/* clang-format off */
#include "c_orm_api.h"
#include "c_orm_query_builder.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

static c_orm_error_t hydrate_row(c_orm_db_t *db, c_orm_query_t *query,
                                 const c_orm_table_meta_t *meta,
                                 void *out_struct) {
  size_t i;
  c_orm_error_t err;

  for (i = 0; i < meta->num_columns; ++i) {
    const c_orm_column_meta_t *col = &meta->columns[i];
    void *field_ptr = (char *)out_struct + col->offset;
    int is_null = 0;

    err = db->vtable->is_null(query, (int)i, &is_null);
    if (err != C_ORM_OK)
      return err;

    if (is_null) {
      if (!col->is_nullable) {
        /* This is actually a constraint violation */
        return C_ORM_ERROR_TYPE_MISMATCH;
      }
      /* If it's nullable primitive, we need to set the pointer to NULL.
         For string, set char* to NULL. */
      if (col->type == C_ORM_TYPE_STRING) {
        *(char **)field_ptr = NULL;
      } else {
        *(void **)field_ptr = NULL;
      }
      continue;
    }

    switch (col->type) {
    case C_ORM_TYPE_INT32:
    case C_ORM_TYPE_BOOL: {
      int32_t val;
      err = db->vtable->get_int32(query, (int)i, &val);
      if (err != C_ORM_OK)
        return err;
      if (col->is_nullable) {
        int32_t *ptr = (int32_t *)malloc(sizeof(int32_t));
        if (!ptr)
          return C_ORM_ERROR_MEMORY;
        *ptr = val;
        *(int32_t **)field_ptr = ptr;
      } else {
        *(int32_t *)field_ptr = val;
      }
      break;
    }
    case C_ORM_TYPE_INT64: {
      int64_t val;
      err = db->vtable->get_int64(query, (int)i, &val);
      if (err != C_ORM_OK)
        return err;
      if (col->is_nullable) {
        int64_t *ptr = (int64_t *)malloc(sizeof(int64_t));
        if (!ptr)
          return C_ORM_ERROR_MEMORY;
        *ptr = val;
        *(int64_t **)field_ptr = ptr;
      } else {
        *(int64_t *)field_ptr = val;
      }
      break;
    }
    case C_ORM_TYPE_FLOAT:
    case C_ORM_TYPE_DOUBLE: {
      double val;
      err = db->vtable->get_double(query, (int)i, &val);
      if (err != C_ORM_OK)
        return err;
      if (col->is_nullable) {
        if (col->type == C_ORM_TYPE_FLOAT) {
          float *ptr = (float *)malloc(sizeof(float));
          if (!ptr)
            return C_ORM_ERROR_MEMORY;
          *ptr = (float)val;
          *(float **)field_ptr = ptr;
        } else {
          double *ptr = (double *)malloc(sizeof(double));
          if (!ptr)
            return C_ORM_ERROR_MEMORY;
          *ptr = val;
          *(double **)field_ptr = ptr;
        }
      } else {
        if (col->type == C_ORM_TYPE_FLOAT) {
          *(float *)field_ptr = (float)val;
        } else {
          *(double *)field_ptr = val;
        }
      }
      break;
    }
    case C_ORM_TYPE_STRING:
    case C_ORM_TYPE_DATE:
    case C_ORM_TYPE_TIMESTAMP: {
      const char *val;
      err = db->vtable->get_string(query, (int)i, &val);
      if (err != C_ORM_OK)
        return err;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      *(char **)field_ptr = _strdup(val);
#else
      *(char **)field_ptr = strdup(val);
#endif
      if (!*(char **)field_ptr)
        return C_ORM_ERROR_MEMORY;
      break;
    }
    default:
      return C_ORM_ERROR_TYPE_MISMATCH;
    }
  }
  return C_ORM_OK;
}

c_orm_error_t c_orm_find_by_id_int32(c_orm_db_t *db,
                                     const c_orm_table_meta_t *meta,
                                     int32_t id_val, void *out_struct) {
  c_orm_query_t *query;
  c_orm_error_t err;
  int has_row;

  if (!db || !meta || !out_struct)
    return C_ORM_ERROR_MEMORY;

  if (!meta->query_select_by_pk) {
    return C_ORM_ERROR_NOT_IMPLEMENTED; /* No single PK available */
  }

  err = db->vtable->prepare(db, meta->query_select_by_pk, &query);
  if (err != C_ORM_OK)
    return err;

  err = db->vtable->bind_int32(query, 1, id_val);
  if (err != C_ORM_OK) {
    db->vtable->finalize(query);
    return err;
  }

  err = db->vtable->step(query, &has_row);
  if (err != C_ORM_OK) {
    db->vtable->finalize(query);
    return err;
  }

  if (!has_row) {
    db->vtable->finalize(query);
    return C_ORM_ERROR_NOT_FOUND;
  }

  err = hydrate_row(db, query, meta, out_struct);
  db->vtable->finalize(query);
  return err;
}

/* Array Layout matching cdd-c:
   struct Type_Array {
     struct Type *data;
     size_t length;
     size_t capacity;
   }
*/
struct Generic_Array {
  void *data;
  size_t length;
  size_t capacity;
};

c_orm_error_t c_orm_find_all(c_orm_db_t *db, const c_orm_table_meta_t *meta,
                             void *out_array) {
  c_orm_query_t *query;
  c_orm_error_t err;
  int has_row;
  struct Generic_Array *arr = (struct Generic_Array *)out_array;
  size_t count = 0;
  size_t cap = arr->capacity;
  void *data = arr->data;

  if (!db || !meta || !out_array)
    return C_ORM_ERROR_MEMORY;

  err = db->vtable->prepare(db, meta->query_select_all, &query);
  if (err != C_ORM_OK)
    return err;

  while (1) {
    err = db->vtable->step(query, &has_row);
    if (err != C_ORM_OK) {
      db->vtable->finalize(query);
      return err;
    }
    if (!has_row)
      break;

    if (count >= cap) {
      size_t new_cap = cap == 0 ? 16 : cap * 2;
      void *new_data = realloc(data, new_cap * meta->struct_size);
      if (!new_data) {
        db->vtable->finalize(query);
        return C_ORM_ERROR_MEMORY;
      }
      data = new_data;
      cap = new_cap;
    }

    /* Hydrate into data[count] */
    err = hydrate_row(db, query, meta,
                      (char *)data + (count * meta->struct_size));
    if (err != C_ORM_OK) {
      db->vtable->finalize(query);
      return err;
    }
    count++;
  }

  arr->data = data;
  arr->capacity = cap;
  arr->length = count;

  db->vtable->finalize(query);
  return C_ORM_OK;
}

static c_orm_error_t bind_row(c_orm_db_t *db, c_orm_query_t *query,
                              const c_orm_table_meta_t *meta,
                              const void *in_struct, int skip_pk) {
  size_t i;
  c_orm_error_t err;
  int bind_idx = 1;

  for (i = 0; i < meta->num_columns; ++i) {
    const c_orm_column_meta_t *col = &meta->columns[i];
    const void *field_ptr = (const char *)in_struct + col->offset;

    if (skip_pk && col->is_pk) {
      continue;
    }

    if (col->type == C_ORM_TYPE_STRING || col->type == C_ORM_TYPE_DATE ||
        col->type == C_ORM_TYPE_TIMESTAMP) {
      const char *str_val = *(const char **)field_ptr;
      if (!str_val) {
        err = db->vtable->bind_null(query, bind_idx++);
        if (err != C_ORM_OK)
          return err;
        continue;
      }
      err = db->vtable->bind_string(query, bind_idx++, str_val);
      if (err != C_ORM_OK)
        return err;
      continue;
    }

    if (col->is_nullable) {
      /* Pointer type for primitives */
      const void *ptr_val = *(const void **)field_ptr;
      if (!ptr_val) {
        err = db->vtable->bind_null(query, bind_idx++);
        if (err != C_ORM_OK)
          return err;
        continue;
      }
      field_ptr = ptr_val; /* Dereference to read the primitive value */
    }

    switch (col->type) {
    case C_ORM_TYPE_INT32:
    case C_ORM_TYPE_BOOL: {
      int32_t val = *(const int32_t *)field_ptr;
      err = db->vtable->bind_int32(query, bind_idx++, val);
      break;
    }
    case C_ORM_TYPE_INT64: {
      int64_t val = *(const int64_t *)field_ptr;
      err = db->vtable->bind_int64(query, bind_idx++, val);
      break;
    }
    case C_ORM_TYPE_FLOAT: {
      float val = *(const float *)field_ptr;
      err = db->vtable->bind_double(query, bind_idx++, (double)val);
      break;
    }
    case C_ORM_TYPE_DOUBLE: {
      double val = *(const double *)field_ptr;
      err = db->vtable->bind_double(query, bind_idx++, val);
      break;
    }
    default:
      return C_ORM_ERROR_TYPE_MISMATCH;
    }
    if (err != C_ORM_OK)
      return err;
  }
  return C_ORM_OK;
}

c_orm_error_t c_orm_insert(c_orm_db_t *db, const c_orm_table_meta_t *meta,
                           const void *in_struct) {
  c_orm_query_t *query;
  c_orm_error_t err;
  int has_row;

  if (!db || !meta || !in_struct)
    return C_ORM_ERROR_MEMORY;

  err = db->vtable->prepare(db, meta->query_insert, &query);
  if (err != C_ORM_OK)
    return err;

  err = bind_row(db, query, meta, in_struct, 0); /* Bind all */
  if (err != C_ORM_OK) {
    db->vtable->finalize(query);
    return err;
  }

  err = db->vtable->step(query, &has_row);
  db->vtable->finalize(query);
  return err;
}

c_orm_error_t c_orm_update(c_orm_db_t *db, const c_orm_table_meta_t *meta,
                           const void *in_struct) {
  c_orm_query_t *query;
  c_orm_error_t err;
  int has_row;
  int32_t pk_val = 0; /* Fallback assuming int PK */
  int bind_idx = 1;
  size_t i;

  if (!db || !meta || !in_struct)
    return C_ORM_ERROR_MEMORY;
  if (!meta->query_update)
    return C_ORM_ERROR_NOT_IMPLEMENTED;

  err = db->vtable->prepare(db, meta->query_update, &query);
  if (err != C_ORM_OK)
    return err;

  /* We bind all fields, then the PK at the end */
  err = bind_row(db, query, meta, in_struct, 0);
  if (err != C_ORM_OK) {
    db->vtable->finalize(query);
    return err;
  }

  bind_idx = (int)(meta->num_columns + 1);

  /* Find PK to bind to WHERE clause */
  for (i = 0; i < meta->num_columns; ++i) {
    if (meta->columns[i].is_pk) {
      const void *field_ptr = (const char *)in_struct + meta->columns[i].offset;
      if (meta->columns[i].type == C_ORM_TYPE_INT32) {
        pk_val = *(const int32_t *)field_ptr;
        err = db->vtable->bind_int32(query, bind_idx, pk_val);
        if (err != C_ORM_OK) {
          db->vtable->finalize(query);
          return err;
        }
      }
      break;
    }
  }

  err = db->vtable->step(query, &has_row);
  db->vtable->finalize(query);
  return err;
}

c_orm_error_t c_orm_delete_by_id_int32(c_orm_db_t *db,
                                       const c_orm_table_meta_t *meta,
                                       int32_t id_val) {
  c_orm_query_t *query;
  c_orm_error_t err;
  int has_row;

  if (!db || !meta)
    return C_ORM_ERROR_MEMORY;
  if (!meta->query_delete_by_pk)
    return C_ORM_ERROR_NOT_IMPLEMENTED;

  err = db->vtable->prepare(db, meta->query_delete_by_pk, &query);
  if (err != C_ORM_OK)
    return err;

  err = db->vtable->bind_int32(query, 1, id_val);
  if (err != C_ORM_OK) {
    db->vtable->finalize(query);
    return err;
  }

  err = db->vtable->step(query, &has_row);
  db->vtable->finalize(query);
  return err;
}

c_orm_error_t c_orm_execute_raw(c_orm_db_t *db, const char *sql) {
  c_orm_query_t *query;
  c_orm_error_t err;
  int has_row;

  if (!db || !sql)
    return C_ORM_ERROR_MEMORY;

  err = db->vtable->prepare(db, sql, &query);
  if (err != C_ORM_OK)
    return err;

  err = db->vtable->step(query, &has_row);
  db->vtable->finalize(query);
  return err;
}

c_orm_error_t c_orm_begin(c_orm_db_t *db) {
  return c_orm_execute_raw(db, "BEGIN TRANSACTION");
}

c_orm_error_t c_orm_commit(c_orm_db_t *db) {
  return c_orm_execute_raw(db, "COMMIT");
}

c_orm_error_t c_orm_rollback(c_orm_db_t *db) {
  return c_orm_execute_raw(db, "ROLLBACK");
}
