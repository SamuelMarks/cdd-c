/**
 * @file c_orm_db.c
 * @brief Core interfaces and vtables implementation.
 */

/* clang-format off */
#include "c_orm_db.h"
#include <stddef.h>
/* clang-format on */

int c_orm_get_last_error_message(c_orm_db_t *db, const char **out_msg) {
  if (!out_msg)
    return C_ORM_ERROR_UNKNOWN;
  if (!db || !db->vtable || !db->vtable->get_last_error) {
    *out_msg = "Unknown Error (No DB context)";
  } else {
    *out_msg = db->vtable->get_last_error(db);
  }
  return C_ORM_OK;
}

void c_orm_set_log_callback(c_orm_db_t *db, c_orm_log_cb cb, void *user_data) {
  if (db) {
    db->log_cb = cb;
    db->log_user_data = user_data;
  }
}
