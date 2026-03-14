#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_orm_api.h"
#include "c_orm_sqlite.h"
#include "greatest.h"

/* The generated models */
#include "Models.h"

static c_orm_db_t *db = NULL;

TEST test_e2e_connect(void) {
  c_orm_error_t err;
  err = c_orm_sqlite_connect(":memory:", &db);
  ASSERT_EQ_FMT(C_ORM_OK, err, "%d");
  ASSERT(db != NULL);
  PASS();
}

TEST test_e2e_generate_schema(void) {
  c_orm_error_t err;
  const char *schema = "CREATE TABLE users ("
                       "id INTEGER PRIMARY KEY,"
                       "username VARCHAR(255) NOT NULL,"
                       "email VARCHAR(255) UNIQUE NOT NULL,"
                       "age INTEGER,"
                       "score FLOAT,"
                       "is_active BOOLEAN,"
                       "created_at TIMESTAMP"
                       ");";

  err = c_orm_execute_raw(db, schema);
  ASSERT_EQ_FMT(C_ORM_OK, err, "%d");

  err = c_orm_execute_raw(db, "CREATE TABLE posts ("
                              "id INTEGER PRIMARY KEY,"
                              "user_id INTEGER NOT NULL,"
                              "title VARCHAR(255) NOT NULL,"
                              "content TEXT,"
                              "views BIGINT,"
                              "published_date DATE,"
                              "FOREIGN KEY (user_id) REFERENCES users(id)"
                              ");");
  ASSERT_EQ_FMT(C_ORM_OK, err, "%d");
  PASS();
}

TEST test_e2e_insert_user(void) {
  struct Users u;
  c_orm_error_t err;
  int32_t age = 30;
  float score = 9.5f;
  bool is_active = true;
  char *created_at = "2026-03-14 12:00:00";

  memset(&u, 0, sizeof(u));
  u.id = 1;
  u.username = "smarks";
  u.email = "samuel@example.com";
  u.age = &age;
  u.score = &score;
  u.is_active = &is_active;
  u.created_at = created_at;

  err = c_orm_insert(db, &Users_meta, &u);
  if (err != C_ORM_OK) {
    printf("Error: %s\n", c_orm_get_last_error_message(db));
  }
  ASSERT_EQ_FMT(C_ORM_OK, err, "%d");
  PASS();
}

TEST test_e2e_fetch_user(void) {
  struct Users u;
  c_orm_error_t err;

  memset(&u, 0, sizeof(u));
  err = c_orm_find_by_id_int32(db, &Users_meta, 1, &u);
  ASSERT_EQ_FMT(C_ORM_OK, err, "%d");

  ASSERT_STR_EQ("smarks", u.username);
  ASSERT_STR_EQ("samuel@example.com", u.email);
  ASSERT(u.age != NULL);
  ASSERT_EQ(30, *u.age);
  ASSERT(u.score != NULL);
  /* Double precision check */
  printf("score: %f\n", *u.score);
  ASSERT(*u.score > 9.4 && *u.score < 9.6);
  ASSERT(u.is_active != NULL);
  ASSERT_EQ(1, *u.is_active);

  Users_free(&u);
  PASS();
}

TEST test_e2e_fetch_all(void) {
  struct Users_Array arr;
  c_orm_error_t err;

  memset(&arr, 0, sizeof(arr));
  err = c_orm_find_all(db, &Users_meta, &arr);
  ASSERT_EQ_FMT(C_ORM_OK, err, "%d");

  ASSERT_EQ(1, arr.length);
  ASSERT_STR_EQ("smarks", arr.data[0].username);

  Users_Array_free(&arr);
  PASS();
}

SUITE(e2e_suite) {
  RUN_TEST(test_e2e_connect);
  RUN_TEST(test_e2e_generate_schema);
  RUN_TEST(test_e2e_insert_user);
  RUN_TEST(test_e2e_fetch_user);
  RUN_TEST(test_e2e_fetch_all);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(e2e_suite);
  GREATEST_MAIN_END();
}
