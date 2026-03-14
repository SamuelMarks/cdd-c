/**
 * @file c_orm_sqlite.c
 * @brief SQLite3 driver implementation for c-orm.
 */

/* clang-format off */
#include "c_orm_sqlite.h"

#include <stdlib.h>
#include <string.h>

#ifdef C_ORM_ENABLE_SQLITE
#include <sqlite3.h>
#endif
/* clang-format on */

#ifdef C_ORM_ENABLE_SQLITE

struct sqlite_db_data {
  sqlite3 *db;
  char last_error[512];
};

struct sqlite_query_data {
  sqlite3_stmt *stmt;
  c_orm_db_t *db;
};

static void set_error(c_orm_db_t *db, const char *msg) {
  if (db && db->driver_data) {
    struct sqlite_db_data *data = (struct sqlite_db_data *)db->driver_data;
    if (msg) {
      /* Safe copy for C90 compatibility */
      size_t len = strlen(msg);
      if (len >= sizeof(data->last_error)) {
        len = sizeof(data->last_error) - 1;
      }
      memcpy(data->last_error, msg, len);
      data->last_error[len] = '\0';
    } else if (data->db) {
      const char *sqlite_err = sqlite3_errmsg(data->db);
      size_t len = strlen(sqlite_err);
      if (len >= sizeof(data->last_error)) {
        len = sizeof(data->last_error) - 1;
      }
      memcpy(data->last_error, sqlite_err, len);
      data->last_error[len] = '\0';
    }
  }
}

static c_orm_error_t sqlite_connect(const char *url, c_orm_db_t **out_db) {
  c_orm_db_t *db;
  struct sqlite_db_data *data;
  int rc;

  if (!url || !out_db)
    return C_ORM_ERROR_MEMORY;

  db = (c_orm_db_t *)calloc(1, sizeof(c_orm_db_t));
  if (!db)
    return C_ORM_ERROR_MEMORY;

  data = (struct sqlite_db_data *)calloc(1, sizeof(struct sqlite_db_data));
  if (!data) {
    free(db);
    return C_ORM_ERROR_MEMORY;
  }

  rc = sqlite3_open(url, &data->db);
  if (rc != SQLITE_OK) {
    sqlite3_close(data->db); /* clean up if needed */
    free(data);
    free(db);
    return C_ORM_ERROR_CONNECTION;
  }

  db->vtable = c_orm_sqlite_get_vtable();
  db->driver_data = data;
  *out_db = db;

  return C_ORM_OK;
}

static c_orm_error_t sqlite_disconnect(c_orm_db_t *db) {
  struct sqlite_db_data *data;
  if (!db)
    return C_ORM_OK;

  data = (struct sqlite_db_data *)db->driver_data;
  if (data) {
    if (data->db) {
      sqlite3_close(data->db);
    }
    free(data);
  }
  free(db);
  return C_ORM_OK;
}

struct c_orm_query {
  struct sqlite_query_data *data;
};

static c_orm_error_t sqlite_prepare(c_orm_db_t *db, const char *sql,
                                    c_orm_query_t **out_query) {
  struct sqlite_db_data *db_data;
  struct sqlite_query_data *q_data;
  c_orm_query_t *query;
  int rc;

  if (!db || !sql || !out_query)
    return C_ORM_ERROR_MEMORY;
  db_data = (struct sqlite_db_data *)db->driver_data;

  if (db->log_cb) {
    db->log_cb(sql, db->log_user_data);
  }

  query = (c_orm_query_t *)calloc(1, sizeof(c_orm_query_t));
  if (!query)
    return C_ORM_ERROR_MEMORY;

  q_data =
      (struct sqlite_query_data *)calloc(1, sizeof(struct sqlite_query_data));
  if (!q_data) {
    free(query);
    return C_ORM_ERROR_MEMORY;
  }

  rc = sqlite3_prepare_v2(db_data->db, sql, -1, &q_data->stmt, NULL);
  if (rc != SQLITE_OK) {
    set_error(db, NULL);
    free(q_data);
    free(query);
    return C_ORM_ERROR_SQL;
  }

  q_data->db = db;
  query->data = q_data;
  *out_query = query;

  return C_ORM_OK;
}

static c_orm_error_t sqlite_bind_int32(c_orm_query_t *query, int index,
                                       int32_t val) {
  int rc;
  if (!query || !query->data || !query->data->stmt)
    return C_ORM_ERROR_BIND;
  rc = sqlite3_bind_int(query->data->stmt, index, val);
  if (rc != SQLITE_OK) {
    set_error(query->data->db, NULL);
    return C_ORM_ERROR_BIND;
  }
  return C_ORM_OK;
}

static c_orm_error_t sqlite_bind_int64(c_orm_query_t *query, int index,
                                       int64_t val) {
  int rc;
  if (!query || !query->data || !query->data->stmt)
    return C_ORM_ERROR_BIND;
  rc = sqlite3_bind_int64(query->data->stmt, index, val);
  if (rc != SQLITE_OK) {
    set_error(query->data->db, NULL);
    return C_ORM_ERROR_BIND;
  }
  return C_ORM_OK;
}

static c_orm_error_t sqlite_bind_double(c_orm_query_t *query, int index,
                                        double val) {
  int rc;
  if (!query || !query->data || !query->data->stmt)
    return C_ORM_ERROR_BIND;
  rc = sqlite3_bind_double(query->data->stmt, index, val);
  if (rc != SQLITE_OK) {
    set_error(query->data->db, NULL);
    return C_ORM_ERROR_BIND;
  }
  return C_ORM_OK;
}

static c_orm_error_t sqlite_bind_string(c_orm_query_t *query, int index,
                                        const char *val) {
  int rc;
  if (!query || !query->data || !query->data->stmt)
    return C_ORM_ERROR_BIND;
  rc = sqlite3_bind_text(query->data->stmt, index, val, -1, SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    set_error(query->data->db, NULL);
    return C_ORM_ERROR_BIND;
  }
  return C_ORM_OK;
}

static c_orm_error_t sqlite_bind_null(c_orm_query_t *query, int index) {
  int rc;
  if (!query || !query->data || !query->data->stmt)
    return C_ORM_ERROR_BIND;
  rc = sqlite3_bind_null(query->data->stmt, index);
  if (rc != SQLITE_OK) {
    set_error(query->data->db, NULL);
    return C_ORM_ERROR_BIND;
  }
  return C_ORM_OK;
}

static c_orm_error_t sqlite_step(c_orm_query_t *query, int *out_has_row) {
  int rc;
  if (!query || !query->data || !query->data->stmt || !out_has_row)
    return C_ORM_ERROR_STEP;

  rc = sqlite3_step(query->data->stmt);
  if (rc == SQLITE_ROW) {
    *out_has_row = 1;
    return C_ORM_OK;
  } else if (rc == SQLITE_DONE) {
    *out_has_row = 0;
    return C_ORM_OK;
  }

  set_error(query->data->db, NULL);
  return C_ORM_ERROR_STEP;
}

static c_orm_error_t sqlite_get_int32(c_orm_query_t *query, int index,
                                      int32_t *out_val) {
  if (!query || !query->data || !query->data->stmt || !out_val)
    return C_ORM_ERROR_MEMORY;
  *out_val = (int32_t)sqlite3_column_int(query->data->stmt, index);
  return C_ORM_OK;
}

static c_orm_error_t sqlite_get_int64(c_orm_query_t *query, int index,
                                      int64_t *out_val) {
  if (!query || !query->data || !query->data->stmt || !out_val)
    return C_ORM_ERROR_MEMORY;
  *out_val = (int64_t)sqlite3_column_int64(query->data->stmt, index);
  return C_ORM_OK;
}

static c_orm_error_t sqlite_get_double(c_orm_query_t *query, int index,
                                       double *out_val) {
  if (!query || !query->data || !query->data->stmt || !out_val)
    return C_ORM_ERROR_MEMORY;
  *out_val = sqlite3_column_double(query->data->stmt, index);
  return C_ORM_OK;
}

static c_orm_error_t sqlite_get_string(c_orm_query_t *query, int index,
                                       const char **out_val) {
  if (!query || !query->data || !query->data->stmt || !out_val)
    return C_ORM_ERROR_MEMORY;
  *out_val = (const char *)sqlite3_column_text(query->data->stmt, index);
  return C_ORM_OK;
}

static c_orm_error_t sqlite_is_null(c_orm_query_t *query, int index,
                                    int *out_is_null) {
  if (!query || !query->data || !query->data->stmt || !out_is_null)
    return C_ORM_ERROR_MEMORY;
  *out_is_null =
      (sqlite3_column_type(query->data->stmt, index) == SQLITE_NULL) ? 1 : 0;
  return C_ORM_OK;
}

static c_orm_error_t sqlite_finalize(c_orm_query_t *query) {
  if (!query)
    return C_ORM_OK;
  if (query->data) {
    if (query->data->stmt) {
      sqlite3_finalize(query->data->stmt);
    }
    free(query->data);
  }
  free(query);
  return C_ORM_OK;
}

static c_orm_error_t sqlite_reset(c_orm_query_t *query) {
  if (!query || !query->data || !query->data->stmt)
    return C_ORM_ERROR_MEMORY;
  sqlite3_reset(query->data->stmt);
  sqlite3_clear_bindings(query->data->stmt);
  return C_ORM_OK;
}

static const char *sqlite_get_last_error(c_orm_db_t *db) {
  struct sqlite_db_data *data;
  if (!db || !db->driver_data)
    return "Invalid DB object";
  data = (struct sqlite_db_data *)db->driver_data;
  return data->last_error;
}

static const c_orm_driver_vtable_t sqlite_vtable = {
    sqlite_connect,     sqlite_disconnect,    sqlite_prepare,
    sqlite_bind_int32,  sqlite_bind_int64,    sqlite_bind_double,
    sqlite_bind_string, sqlite_bind_null,     sqlite_step,
    sqlite_get_int32,   sqlite_get_int64,     sqlite_get_double,
    sqlite_get_string,  sqlite_is_null,       sqlite_finalize,
    sqlite_reset,       sqlite_get_last_error};

const c_orm_driver_vtable_t *c_orm_sqlite_get_vtable(void) {
  return &sqlite_vtable;
}

c_orm_error_t c_orm_sqlite_connect(const char *url, c_orm_db_t **out_db) {
  return sqlite_connect(url, out_db);
}

#else

/* Stub out if SQLite is not enabled */
const c_orm_driver_vtable_t *c_orm_sqlite_get_vtable(void) { return NULL; }
c_orm_error_t c_orm_sqlite_connect(const char *url, c_orm_db_t **out_db) {
  (void)url;
  (void)out_db;
  return C_ORM_ERROR_NOT_IMPLEMENTED;
}

#endif
