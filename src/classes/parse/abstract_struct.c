/**
 * @file abstract_struct.c
 * @brief Implementation of CDD_C generic representation.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "classes/parse/abstract_struct.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parson.h"
#include "classes/parse/sql.h"
#include "classes/emit/c_orm_meta.h"
#if defined(USE_SQLITE_LINKED)
#include <sqlite3.h>
#endif
#if defined(USE_LIBPQ_LINKED)
#include <libpq-fe.h>
#endif
#if defined(USE_MYSQL_LINKED)
#include <mysql.h>
#endif
/* clang-format on */

static size_t cdd_c_memory_allocated = 0;
static size_t cdd_c_memory_freed = 0;

void *cdd_c_malloc(size_t size) {
  void *ptr = malloc(size);
  if (ptr)
    cdd_c_memory_allocated += size;
  return ptr;
}

void *cdd_c_realloc(void *ptr, size_t size) {
  void *new_ptr = realloc(ptr, size);
  if (new_ptr && !ptr)
    cdd_c_memory_allocated += size; /* Rough estimate */
  return new_ptr;
}

void cdd_c_free(void *ptr) {
  if (ptr) {
    cdd_c_memory_freed++; /* Just tracking calls for simple detection */
    free(ptr);
  }
}

size_t cdd_c_get_allocated_bytes(void) { return cdd_c_memory_allocated; }

size_t cdd_c_get_freed_calls(void) { return cdd_c_memory_freed; }

/* Array logic */
int cdd_c_abstract_struct_array_init(cdd_c_abstract_struct_array_t *arr,
                                     size_t capacity) {
  if (!arr)
    return -1;
  arr->items = NULL;
  arr->count = 0;
  arr->capacity = capacity;
  if (capacity > 0) {
    arr->items = (cdd_c_abstract_struct_t *)cdd_c_malloc(
        capacity * sizeof(cdd_c_abstract_struct_t));
    if (!arr->items)
      return -1;
    /* Initialize children to 0 counts so deep frees don't blow up on partial
     * errors */
    memset(arr->items, 0, capacity * sizeof(cdd_c_abstract_struct_t));
  }
  return 0;
}

int cdd_c_abstract_struct_array_append(cdd_c_abstract_struct_array_t *arr,
                                       cdd_c_abstract_struct_t *astruct) {
  if (!arr || !astruct)
    return -1;
  if (arr->count >= arr->capacity) {
    size_t new_cap = arr->capacity == 0 ? 4 : arr->capacity * 2;
    cdd_c_abstract_struct_t *new_items =
        (cdd_c_abstract_struct_t *)cdd_c_realloc(
            arr->items, new_cap * sizeof(cdd_c_abstract_struct_t));
    if (!new_items)
      return -1;
    arr->items = new_items;
    arr->capacity = new_cap;
  }
  /* Shallow copy the whole struct since it owns its kvs pointer array */
  arr->items[arr->count] = *astruct;
  /* Null out original so consumer doesn't double-free the internal pointer */
  astruct->kvs = NULL;
  astruct->count = 0;
  astruct->capacity = 0;
  arr->count++;
  return 0;
}

int cdd_c_abstract_struct_array_free(cdd_c_abstract_struct_array_t *arr) {
  size_t i;
  if (!arr)
    return -1;
  if (arr->items) {
    for (i = 0; i < arr->count; ++i) {
      cdd_c_abstract_struct_free(&arr->items[i]);
    }
    cdd_c_free(arr->items);
  }
  arr->items = NULL;
  arr->count = 0;
  arr->capacity = 0;
  return 0;
}

int cdd_c_abstract_struct_array_to_json(
    const cdd_c_abstract_struct_array_t *arr, char **out_json) {
  JSON_Value *root_val;
  JSON_Array *root_arr;
  size_t i, j;
  if (!arr || !out_json)
    return -1;

  root_val = json_value_init_array();
  if (!root_val)
    return -1;
  root_arr = json_value_get_array(root_val);

  for (i = 0; i < arr->count; ++i) {
    JSON_Value *obj_val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(obj_val);
    const cdd_c_abstract_struct_t *astruct = &arr->items[i];

    for (j = 0; j < astruct->count; ++j) {
      const cdd_c_abstract_struct_kv_t *kv = &astruct->kvs[j];
      switch (kv->value.type) {
      case CDD_C_VARIANT_TYPE_INT:
        json_object_set_number(obj, kv->key, (double)kv->value.value.i_val);
        break;
      case CDD_C_VARIANT_TYPE_FLOAT:
        json_object_set_number(obj, kv->key, kv->value.value.f_val);
        break;
      case CDD_C_VARIANT_TYPE_STRING:
        json_object_set_string(obj, kv->key, kv->value.value.s_val);
        break;
      case CDD_C_VARIANT_TYPE_BLOB:
        /* Blobs stringified as base64 or ignored for strict JSON serialization
         * in simplistic wrappers */
        json_object_set_string(obj, kv->key, "<BLOB>");
        break;
      case CDD_C_VARIANT_TYPE_NULL:
      default:
        json_object_set_null(obj, kv->key);
        break;
      }
    }
    json_array_append_value(root_arr, obj_val);
  }

  *out_json = json_serialize_to_string_pretty(root_val);
  json_value_free(root_val);
  return *out_json ? 0 : -1;
}

int cdd_c_abstract_struct_init(cdd_c_abstract_struct_t *astruct) {
  return cdd_c_abstract_struct_init_with_capacity(astruct, 0);
}

int cdd_c_abstract_struct_init_with_capacity(cdd_c_abstract_struct_t *astruct,
                                             size_t capacity) {
  if (!astruct)
    return -1;
  astruct->kvs = NULL;
  astruct->count = 0;
  astruct->capacity = capacity;
  if (capacity > 0) {
    astruct->kvs = (cdd_c_abstract_struct_kv_t *)cdd_c_malloc(
        capacity * sizeof(cdd_c_abstract_struct_kv_t));
    if (!astruct->kvs)
      return -1;
  }
  return 0;
}

static int duplicate_string(const char *src, char **dest) {
  size_t len;
  if (!src || !dest)
    return -1;
  len = strlen(src);
  *dest = (char *)cdd_c_malloc(len + 1);
  if (!*dest)
    return -1;
  memcpy(*dest, src, len + 1);
  return 0;
}

static int duplicate_blob(const unsigned char *src, size_t size,
                          unsigned char **dest) {
  if (!src || !dest)
    return -1;
  *dest = (unsigned char *)cdd_c_malloc(size);
  if (!*dest)
    return -1;
  memcpy(*dest, src, size);
  return 0;
}

int cdd_c_variant_free(cdd_c_variant_t *variant) {
  if (!variant)
    return -1;
  switch (variant->type) {
  case CDD_C_VARIANT_TYPE_STRING:
    if (variant->value.s_val) {
      cdd_c_free(variant->value.s_val);
      variant->value.s_val = NULL;
    }
    break;
  case CDD_C_VARIANT_TYPE_BLOB:
    if (variant->value.b_val.data) {
      cdd_c_free(variant->value.b_val.data);
      variant->value.b_val.data = NULL;
    }
    break;
  default:
    break;
  }
  variant->type = CDD_C_VARIANT_TYPE_NULL;
  return 0;
}

static int copy_variant(cdd_c_variant_t *dest, const cdd_c_variant_t *src) {
  if (!dest || !src)
    return -1;
  dest->type = src->type;
  switch (src->type) {
  case CDD_C_VARIANT_TYPE_INT:
    dest->value.i_val = src->value.i_val;
    break;
  case CDD_C_VARIANT_TYPE_FLOAT:
    dest->value.f_val = src->value.f_val;
    break;
  case CDD_C_VARIANT_TYPE_STRING:
    if (duplicate_string(src->value.s_val, &dest->value.s_val) != 0) {
      return -1;
    }
    break;
  case CDD_C_VARIANT_TYPE_BLOB:
    if (duplicate_blob(src->value.b_val.data, src->value.b_val.size,
                       &dest->value.b_val.data) != 0) {
      return -1;
    }
    dest->value.b_val.size = src->value.b_val.size;
    break;
  case CDD_C_VARIANT_TYPE_NULL:
  default:
    break;
  }
  return 0;
}

static unsigned long hash_string(const char *str) {
  unsigned long hash = 5381;
  int c;
  if (!str)
    return 0;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  return hash;
}

int cdd_c_abstract_set(cdd_c_abstract_struct_t *astruct, const char *key,
                       const cdd_c_variant_t *value) {
  size_t i;
  cdd_c_abstract_struct_kv_t *new_kvs;
  unsigned long khash;
  if (!astruct || !key || !value)
    return -1;

  khash = hash_string(key);

  for (i = 0; i < astruct->count; ++i) {
    if (astruct->kvs[i].key_hash == khash &&
        strcmp(astruct->kvs[i].key, key) == 0) {
      cdd_c_variant_free(&astruct->kvs[i].value);
      return copy_variant(&astruct->kvs[i].value, value);
    }
  }

  if (astruct->count >= astruct->capacity) {
    size_t new_cap = astruct->capacity == 0 ? 4 : astruct->capacity * 2;
    new_kvs = (cdd_c_abstract_struct_kv_t *)cdd_c_realloc(
        astruct->kvs, new_cap * sizeof(cdd_c_abstract_struct_kv_t));
    if (!new_kvs)
      return -1;
    astruct->kvs = new_kvs;
    astruct->capacity = new_cap;
  }

  if (duplicate_string(key, &astruct->kvs[astruct->count].key) != 0) {
    return -1;
  }
  astruct->kvs[astruct->count].key_hash = khash;

  if (copy_variant(&astruct->kvs[astruct->count].value, value) != 0) {
    cdd_c_free(astruct->kvs[astruct->count].key);
    return -1;
  }

  astruct->count++;
  return 0;
}

int cdd_c_abstract_get(const cdd_c_abstract_struct_t *astruct, const char *key,
                       cdd_c_variant_t **out_value) {
  size_t i;
  unsigned long khash;
  if (!astruct || !key || !out_value)
    return -1;

  khash = hash_string(key);

  for (i = 0; i < astruct->count; ++i) {
    if (astruct->kvs[i].key_hash == khash &&
        strcmp(astruct->kvs[i].key, key) == 0) {
      *out_value = &astruct->kvs[i].value;
      return 0;
    }
  }
  return -1; /* Not found */
}

int cdd_c_abstract_struct_deep_copy(cdd_c_abstract_struct_t *dest,
                                    const cdd_c_abstract_struct_t *src) {
  size_t i;
  if (!dest || !src)
    return -1;

  if (cdd_c_abstract_struct_init(dest) != 0)
    return -1;

  for (i = 0; i < src->count; ++i) {
    if (cdd_c_abstract_set(dest, src->kvs[i].key, &src->kvs[i].value) != 0) {
      cdd_c_abstract_struct_free(dest);
      return -1;
    }
  }
  return 0;
}

int cdd_c_abstract_struct_free(cdd_c_abstract_struct_t *astruct) {
  size_t i;
  if (!astruct)
    return -1;

  for (i = 0; i < astruct->count; ++i) {
    cdd_c_free(astruct->kvs[i].key);
    cdd_c_variant_free(&astruct->kvs[i].value);
  }
  cdd_c_free(astruct->kvs);
  astruct->kvs = NULL;
  astruct->count = 0;
  astruct->capacity = 0;
  return 0;
}

void cdd_c_abstract_print(const cdd_c_abstract_struct_t *astruct) {
  size_t i;
  if (!astruct) {
    printf("NULL astruct\n");
    return;
  }
  printf("{\n");
  for (i = 0; i < astruct->count; ++i) {
    printf("  \"%s\": ", astruct->kvs[i].key);
    switch (astruct->kvs[i].value.type) {
    case CDD_C_VARIANT_TYPE_NULL:
      printf("null");
      break;
    case CDD_C_VARIANT_TYPE_INT:
      printf(
          "%d",
          (int)astruct->kvs[i].value.value.i_val); /* Simplified for safety */
      break;
    case CDD_C_VARIANT_TYPE_FLOAT:
      printf("%f", astruct->kvs[i].value.value.f_val);
      break;
    case CDD_C_VARIANT_TYPE_STRING:
      printf("\"%s\"", astruct->kvs[i].value.value.s_val);
      break;
    case CDD_C_VARIANT_TYPE_BLOB:
      printf("<blob>");
      break;
    default:
      printf("<unknown>");
      break;
    }
    if (i < astruct->count - 1) {
      printf(",");
    }
    printf("\n");
  }
  printf("}\n");
}

int cdd_c_abstract_struct_to_json(const cdd_c_abstract_struct_t *astruct,
                                  char **out_json) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  size_t i;

  if (!astruct || !out_json)
    return -1;

  root_val = json_value_init_object();
  root_obj = json_value_get_object(root_val);
  if (!root_val || !root_obj)
    return -1;

  for (i = 0; i < astruct->count; ++i) {
    const cdd_c_abstract_struct_kv_t *kv = &astruct->kvs[i];
    switch (kv->value.type) {
    case CDD_C_VARIANT_TYPE_NULL:
      json_object_set_null(root_obj, kv->key);
      break;
    case CDD_C_VARIANT_TYPE_INT:
      json_object_set_number(root_obj, kv->key, (double)kv->value.value.i_val);
      break;
    case CDD_C_VARIANT_TYPE_FLOAT:
      json_object_set_number(root_obj, kv->key, kv->value.value.f_val);
      break;
    case CDD_C_VARIANT_TYPE_STRING:
      json_object_set_string(root_obj, kv->key, kv->value.value.s_val);
      break;
    case CDD_C_VARIANT_TYPE_BLOB:
      json_object_set_string(root_obj, kv->key, "<blob>");
      break;
    default:
      break;
    }
  }

  *out_json = json_serialize_to_string(root_val);
  json_value_free(root_val);
  if (!*out_json)
    return -1;

  return 0;
}

int cdd_c_abstract_struct_from_json(const char *json_str,
                                    cdd_c_abstract_struct_t *out_astruct) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  size_t count, i;

  if (!json_str || !out_astruct)
    return -1;

  if (cdd_c_abstract_struct_init(out_astruct) != 0)
    return -1;

  root_val = json_parse_string(json_str);
  if (!root_val)
    return -1;

  if (json_value_get_type(root_val) != JSONObject) {
    json_value_free(root_val);
    cdd_c_abstract_struct_free(out_astruct);
    return -1;
  }

  root_obj = json_value_get_object(root_val);
  count = json_object_get_count(root_obj);

  for (i = 0; i < count; ++i) {
    const char *key = json_object_get_name(root_obj, i);
    JSON_Value *val = json_object_get_value_at(root_obj, i);
    cdd_c_variant_t variant;

    variant.type = CDD_C_VARIANT_TYPE_NULL;

    switch (json_value_get_type(val)) {
    case JSONString:
      variant.type = CDD_C_VARIANT_TYPE_STRING;
      variant.value.s_val = (char *)json_value_get_string(val);
      break;
    case JSONNumber: {
      double num = json_value_get_number(val);
      if (num == (double)((long long)num)) {
        variant.type = CDD_C_VARIANT_TYPE_INT;
        variant.value.i_val = (long long)num;
      } else {
        variant.type = CDD_C_VARIANT_TYPE_FLOAT;
        variant.value.f_val = num;
      }
    } break;
    case JSONBoolean:
      variant.type = CDD_C_VARIANT_TYPE_INT;
      variant.value.i_val = json_value_get_boolean(val) ? 1 : 0;
      break;
    case JSONNull:
      variant.type = CDD_C_VARIANT_TYPE_NULL;
      break;
    default:
      break;
    }

    if (cdd_c_abstract_set(out_astruct, key, &variant) != 0) {
      json_value_free(root_val);
      cdd_c_abstract_struct_free(out_astruct);
      return -1;
    }
  }

  json_value_free(root_val);
  return 0;
}

int cdd_c_abstract_hydrate(cdd_c_abstract_struct_t *out_astruct,
                           void **row_data, const cdd_c_column_meta_t *cols,
                           size_t n_cols) {
  size_t i;
  if (!out_astruct || !row_data || !cols)
    return -1;

  if (cdd_c_abstract_struct_init_with_capacity(out_astruct, n_cols) != 0)
    return -1;

  for (i = 0; i < n_cols; ++i) {
    cdd_c_variant_t variant;
    const cdd_c_column_meta_t *col = &cols[i];
    void *raw_val = row_data[i];

    variant.type = CDD_C_VARIANT_TYPE_NULL;

    if (!raw_val) {
      /* Keep it null */
    } else {
      /* Map hardcoded int SQL types to bypass explicit header mapping needs
       * locally */
      int ctype = col->inferred_type;
      if (ctype == 4 || ctype == 3 || ctype == 14) { /* INT, BIGINT, BOOLEAN */
        variant.type = CDD_C_VARIANT_TYPE_INT;
        variant.value.i_val = *(long long *)raw_val;
      } else if (ctype == 8 || ctype == 9 ||
                 ctype == 10) { /* FLOAT, DOUBLE, DECIMAL */
        variant.type = CDD_C_VARIANT_TYPE_FLOAT;
        variant.value.f_val = *(double *)raw_val;
      } else if (ctype == 5 || ctype == 6 || ctype == 7 || ctype == 11 ||
                 ctype == 12) {
        variant.type = CDD_C_VARIANT_TYPE_STRING;
        variant.value.s_val = (char *)raw_val;
      } else {
        variant.type = CDD_C_VARIANT_TYPE_BLOB;
        variant.value.b_val.data = NULL;
        variant.value.b_val.size = 0;
      }
    }

    if (cdd_c_abstract_set(out_astruct, col->name, &variant) != 0) {
      cdd_c_abstract_struct_free(out_astruct);
      return -1;
    }
  }

  return 0;
}

int cdd_c_abstract_hydrate_sqlite3(cdd_c_abstract_struct_t *out_astruct,
                                   void *stmt) {
#if defined(USE_SQLITE_LINKED)
  sqlite3_stmt *s = (sqlite3_stmt *)stmt;
  int i, n_cols;
  if (!out_astruct || !s)
    return -1;

  if (cdd_c_abstract_struct_init(out_astruct) != 0)
    return -1;

  n_cols = sqlite3_column_count(s);
  for (i = 0; i < n_cols; ++i) {
    cdd_c_variant_t variant;
    const char *col_name = sqlite3_column_name(s, i);
    int type = sqlite3_column_type(s, i);

    variant.type = CDD_C_VARIANT_TYPE_NULL;

    switch (type) {
    case SQLITE_INTEGER:
      variant.type = CDD_C_VARIANT_TYPE_INT;
      variant.value.i_val = sqlite3_column_int64(s, i);
      break;
    case SQLITE_FLOAT:
      variant.type = CDD_C_VARIANT_TYPE_FLOAT;
      variant.value.f_val = sqlite3_column_double(s, i);
      break;
    case SQLITE_TEXT:
      variant.type = CDD_C_VARIANT_TYPE_STRING;
      /* Note: cdd_c_abstract_set duplicates the string */
      variant.value.s_val = (char *)sqlite3_column_text(s, i);
      break;
    case SQLITE_BLOB:
      variant.type = CDD_C_VARIANT_TYPE_BLOB;
      /* Note: cdd_c_abstract_set duplicates the blob */
      variant.value.b_val.data = (unsigned char *)sqlite3_column_blob(s, i);
      variant.value.b_val.size = (size_t)sqlite3_column_bytes(s, i);
      break;
    case SQLITE_NULL:
    default:
      variant.type = CDD_C_VARIANT_TYPE_NULL;
      break;
    }

    if (cdd_c_abstract_set(out_astruct, col_name, &variant) != 0) {
      cdd_c_abstract_struct_free(out_astruct);
      return -1;
    }
  }

  return 0;
#else
  (void)out_astruct;
  (void)stmt;
  return -1;
#endif
}
int cdd_c_abstract_hydrate_libpq(cdd_c_abstract_struct_t *out_astruct,
                                 void *res, int row_index) {
#if defined(USE_LIBPQ_LINKED)
  PGresult *pq_res = (PGresult *)res;
  int i, n_cols;
  if (!out_astruct || !pq_res)
    return -1;

  if (cdd_c_abstract_struct_init(out_astruct) != 0)
    return -1;

  n_cols = PQnfields(pq_res);
  for (i = 0; i < n_cols; ++i) {
    cdd_c_variant_t variant;
    const char *col_name = PQfname(pq_res, i);
    int is_null = PQgetisnull(pq_res, row_index, i);

    variant.type = CDD_C_VARIANT_TYPE_NULL;

    if (!is_null) {
      unsigned int type = (unsigned int)PQftype(pq_res, i);
      char *val_str = PQgetvalue(pq_res, row_index, i);

      if (type == 20 || type == 21 || type == 23) { /* int8, int2, int4 */
        variant.type = CDD_C_VARIANT_TYPE_INT;
        variant.value.i_val = strtoll(val_str, NULL, 10);
      } else if (type == 16) { /* bool */
        variant.type = CDD_C_VARIANT_TYPE_INT;
        variant.value.i_val = (val_str[0] == 't') ? 1 : 0;
      } else if (type == 700 || type == 701) { /* float4, float8 */
        variant.type = CDD_C_VARIANT_TYPE_FLOAT;
        variant.value.f_val = strtod(val_str, NULL);
      } else if (type == 17) { /* bytea */
        size_t len = 0;
        unsigned char *unescaped =
            PQunescapeBytea((const unsigned char *)val_str, &len);
        variant.type = CDD_C_VARIANT_TYPE_BLOB;
        if (unescaped) {
          int set_res;
          variant.value.b_val.data = unescaped;
          variant.value.b_val.size = len;
          set_res = cdd_c_abstract_set(out_astruct, col_name, &variant);
          PQfreemem(unescaped);
          if (set_res != 0) {
            cdd_c_abstract_struct_free(out_astruct);
            return -1;
          }
          continue;
        } else {
          variant.value.b_val.data = NULL;
          variant.value.b_val.size = 0;
        }
      } else {
        variant.type = CDD_C_VARIANT_TYPE_STRING;
        variant.value.s_val = val_str;
      }
    }

    if (cdd_c_abstract_set(out_astruct, col_name, &variant) != 0) {
      cdd_c_abstract_struct_free(out_astruct);
      return -1;
    }
  }

  return 0;
#else
  (void)out_astruct;
  (void)res;
  (void)row_index;
  return -1;
#endif
}
int cdd_c_abstract_hydrate_mysql(cdd_c_abstract_struct_t *out_astruct,
                                 void *row, void *fields,
                                 unsigned int num_fields) {
#if defined(USE_MYSQL_LINKED)
  MYSQL_ROW mysql_row = (MYSQL_ROW)row;
  MYSQL_FIELD *mysql_fields = (MYSQL_FIELD *)fields;
  unsigned int i;
  if (!out_astruct || !mysql_row || !mysql_fields)
    return -1;

  if (cdd_c_abstract_struct_init(out_astruct) != 0)
    return -1;

  for (i = 0; i < num_fields; ++i) {
    cdd_c_variant_t variant;
    const char *col_name = mysql_fields[i].name;
    char *val_str = mysql_row[i];
    enum enum_field_types type = mysql_fields[i].type;

    variant.type = CDD_C_VARIANT_TYPE_NULL;

    if (val_str) {
      switch (type) {
      case MYSQL_TYPE_TINY:
      case MYSQL_TYPE_SHORT:
      case MYSQL_TYPE_LONG:
      case MYSQL_TYPE_LONGLONG:
      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_YEAR:
      case MYSQL_TYPE_BIT:
        variant.type = CDD_C_VARIANT_TYPE_INT;
        variant.value.i_val = strtoll(val_str, NULL, 10);
        break;
      case MYSQL_TYPE_DECIMAL:
      case MYSQL_TYPE_NEWDECIMAL:
      case MYSQL_TYPE_FLOAT:
      case MYSQL_TYPE_DOUBLE:
        variant.type = CDD_C_VARIANT_TYPE_FLOAT;
        variant.value.f_val = strtod(val_str, NULL);
        break;
      case MYSQL_TYPE_BLOB:
      case MYSQL_TYPE_TINY_BLOB:
      case MYSQL_TYPE_MEDIUM_BLOB:
      case MYSQL_TYPE_LONG_BLOB:
        variant.type = CDD_C_VARIANT_TYPE_BLOB;
        variant.value.b_val.data = (unsigned char *)val_str;
        variant.value.b_val.size = strlen(val_str);
        break;
      default:
        variant.type = CDD_C_VARIANT_TYPE_STRING;
        variant.value.s_val = val_str;
        break;
      }
    }

    if (cdd_c_abstract_set(out_astruct, col_name, &variant) != 0) {
      cdd_c_abstract_struct_free(out_astruct);
      return -1;
    }
  }

  return 0;
#else
  (void)out_astruct;
  (void)row;
  (void)fields;
  (void)num_fields;
  return -1;
#endif
}
size_t cdd_c_meta_offsetof(const struct c_orm_meta *struct_meta,
                           const char *field) {
  size_t i;
  const c_orm_meta_t *meta = (const c_orm_meta_t *)struct_meta;
  if (!meta || !field)
    return (size_t)-1;

  for (i = 0; i < meta->num_props; ++i) {
    if (strcmp(meta->props[i].name, field) == 0) {
      return meta->props[i].offset;
    }
  }
  return (size_t)-1;
}

int cdd_c_specific_to_abstract(cdd_c_abstract_struct_t *out_astruct,
                               const void *in_struct,
                               const struct c_orm_meta *struct_meta) {
  size_t i;
  const c_orm_meta_t *meta = (const c_orm_meta_t *)struct_meta;
  if (!out_astruct || !in_struct || !meta)
    return -1;

  if (cdd_c_abstract_struct_init(out_astruct) != 0)
    return -1;

  for (i = 0; i < meta->num_props; ++i) {
    const c_orm_prop_meta_t *prop = &meta->props[i];
    cdd_c_variant_t val;
    val.type = CDD_C_VARIANT_TYPE_NULL;

    if (strcmp(prop->type, "C_ORM_TYPE_INT32") == 0) {
      val.type = CDD_C_VARIANT_TYPE_INT;
      val.value.i_val = *(int *)((char *)in_struct + prop->offset);
    } else if (strcmp(prop->type, "C_ORM_TYPE_INT64") == 0) {
      val.type = CDD_C_VARIANT_TYPE_INT;
      val.value.i_val = *(long long *)((char *)in_struct + prop->offset);
    } else if (strcmp(prop->type, "C_ORM_TYPE_FLOAT") == 0) {
      val.type = CDD_C_VARIANT_TYPE_FLOAT;
      val.value.f_val = *(float *)((char *)in_struct + prop->offset);
    } else if (strcmp(prop->type, "C_ORM_TYPE_DOUBLE") == 0) {
      val.type = CDD_C_VARIANT_TYPE_FLOAT;
      val.value.f_val = *(double *)((char *)in_struct + prop->offset);
    } else if (strcmp(prop->type, "C_ORM_TYPE_STRING") == 0) {
      val.type = CDD_C_VARIANT_TYPE_STRING;
      if (prop->length > 0) {
        val.value.s_val = (char *)in_struct + prop->offset;
      } else {
        val.value.s_val = *(char **)((char *)in_struct + prop->offset);
      }
    }

    if (cdd_c_abstract_set(out_astruct, prop->name, &val) != 0) {
      cdd_c_abstract_struct_free(out_astruct);
      return -1;
    }
  }

  return 0;
}

int cdd_c_abstract_to_specific(void *out_struct,
                               const cdd_c_abstract_struct_t *in_astruct,
                               const struct c_orm_meta *struct_meta,
                               int strict_mapping) {
  size_t i;
  const c_orm_meta_t *meta = (const c_orm_meta_t *)struct_meta;
  if (!out_struct || !in_astruct || !meta)
    return -1;

  if (strict_mapping && in_astruct->count != meta->num_props) {
    return -1;
  }

  for (i = 0; i < meta->num_props; ++i) {
    const c_orm_prop_meta_t *prop = &meta->props[i];
    cdd_c_variant_t *val = NULL;

    if (cdd_c_abstract_get(in_astruct, prop->name, &val) != 0) {
      if (strict_mapping)
        return -1;
      continue; /* Partial mapping allowed */
    }

    if (strcmp(prop->type, "C_ORM_TYPE_INT32") == 0) {
      if (val->type == CDD_C_VARIANT_TYPE_INT) {
        *(int *)((char *)out_struct + prop->offset) = (int)val->value.i_val;
      } else if (strict_mapping)
        return -1;
    } else if (strcmp(prop->type, "C_ORM_TYPE_INT64") == 0) {
      if (val->type == CDD_C_VARIANT_TYPE_INT) {
        *(long long *)((char *)out_struct + prop->offset) = val->value.i_val;
      } else if (strict_mapping)
        return -1;
    } else if (strcmp(prop->type, "C_ORM_TYPE_FLOAT") == 0) {
      if (val->type == CDD_C_VARIANT_TYPE_FLOAT) {
        *(float *)((char *)out_struct + prop->offset) = (float)val->value.f_val;
      } else if (strict_mapping)
        return -1;
    } else if (strcmp(prop->type, "C_ORM_TYPE_DOUBLE") == 0) {
      if (val->type == CDD_C_VARIANT_TYPE_FLOAT) {
        *(double *)((char *)out_struct + prop->offset) = val->value.f_val;
      } else if (strict_mapping)
        return -1;
    } else if (strcmp(prop->type, "C_ORM_TYPE_STRING") == 0) {
      if (val->type == CDD_C_VARIANT_TYPE_STRING) {
        if (prop->length > 0) {
          strncpy((char *)out_struct + prop->offset, val->value.s_val,
                  prop->length);
          ((char *)out_struct + prop->offset)[prop->length - 1] = '\0';
        } else {
          *(char **)((char *)out_struct + prop->offset) =
              strdup(val->value.s_val);
        }
      } else if (strict_mapping)
        return -1;
    }
  }

  return 0;
}