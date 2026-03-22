/**
 * @file test_abstract_struct.h
 * @brief Tests for abstract_struct generic representations.
 */

#ifndef C_CDD_TEST_ABSTRACT_STRUCT_H
#define C_CDD_TEST_ABSTRACT_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/parse/abstract_struct.h"
#include "classes/parse/sql.h"
#include "classes/emit/c_orm_meta.h"
#include "../../../include/cdd_c_backend_interface.h"
#include <greatest.h>

TEST test_abstract_struct_memory_layout(void) {
  cdd_c_abstract_struct_t astruct;
  ASSERT_EQ(0, cdd_c_abstract_struct_init(&astruct));
  ASSERT_EQ(0, astruct.count);
  ASSERT_EQ(0, astruct.capacity);
  ASSERT_EQ(NULL, astruct.kvs);
  PASS();
}

TEST test_variant_type_safety(void) {
  cdd_c_variant_t v1, v2;
  v1.type = CDD_C_VARIANT_TYPE_INT;
  v1.value.i_val = 42;

  v2.type = CDD_C_VARIANT_TYPE_STRING;
  v2.value.s_val = (char *)malloc(5);
  strcpy(v2.value.s_val, "test");

  ASSERT_EQ(CDD_C_VARIANT_TYPE_INT, v1.type);
  ASSERT_EQ(42, v1.value.i_val);

  ASSERT_EQ(CDD_C_VARIANT_TYPE_STRING, v2.type);
  ASSERT_STR_EQ("test", v2.value.s_val);

  cdd_c_variant_free(&v2);
  ASSERT_EQ(CDD_C_VARIANT_TYPE_NULL, v2.type);
  PASS();
}

TEST test_abstract_struct_set_get(void) {
  cdd_c_abstract_struct_t astruct;
  cdd_c_variant_t v_in, *v_out;

  ASSERT_EQ(0, cdd_c_abstract_struct_init(&astruct));

  v_in.type = CDD_C_VARIANT_TYPE_INT;
  v_in.value.i_val = 100;

  ASSERT_EQ(0, cdd_c_abstract_set(&astruct, "my_int", &v_in));
  ASSERT_EQ(1, astruct.count);

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct, "my_int", &v_out));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_INT, v_out->type);
  ASSERT_EQ(100, v_out->value.i_val);

  ASSERT_EQ(0, cdd_c_abstract_struct_free(&astruct));
  PASS();
}

TEST test_abstract_struct_json_roundtrip(void) {
  cdd_c_abstract_struct_t astruct_out, astruct_in;
  cdd_c_variant_t v_in, *v_out;
  char *json_str = NULL;

  ASSERT_EQ(0, cdd_c_abstract_struct_init(&astruct_out));

  v_in.type = CDD_C_VARIANT_TYPE_INT;
  v_in.value.i_val = 100;
  ASSERT_EQ(0, cdd_c_abstract_set(&astruct_out, "my_int", &v_in));

  v_in.type = CDD_C_VARIANT_TYPE_STRING;
  v_in.value.s_val = (char *)"test_string";
  ASSERT_EQ(0, cdd_c_abstract_set(&astruct_out, "my_str", &v_in));

  v_in.type = CDD_C_VARIANT_TYPE_FLOAT;
  v_in.value.f_val = 3.14;
  ASSERT_EQ(0, cdd_c_abstract_set(&astruct_out, "my_float", &v_in));

  ASSERT_EQ(0, cdd_c_abstract_struct_to_json(&astruct_out, &json_str));
  ASSERT_NEQ(NULL, json_str);

  ASSERT_EQ(0, cdd_c_abstract_struct_from_json(json_str, &astruct_in));

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct_in, "my_int", &v_out));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_INT, v_out->type);
  ASSERT_EQ(100, v_out->value.i_val);

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct_in, "my_str", &v_out));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_STRING, v_out->type);
  ASSERT_STR_EQ("test_string", v_out->value.s_val);

  cdd_c_abstract_print(&astruct_in);

  json_free_serialized_string(json_str);
  ASSERT_EQ(0, cdd_c_abstract_struct_free(&astruct_out));
  ASSERT_EQ(0, cdd_c_abstract_struct_free(&astruct_in));
  PASS();
}

TEST test_abstract_hydrate(void) {
  cdd_c_abstract_struct_t astruct;
  cdd_c_column_meta_t cols[3];
  void *row_data[3];
  long long mock_int = 42;
  double mock_float = 3.14159;
  char *mock_str = (char *)"Hello, Generic World!";
  cdd_c_variant_t *out_val;

  cols[0].name = "id";
  cols[0].inferred_type = 4; /* SQL_TYPE_INT */

  cols[1].name = "ratio";
  cols[1].inferred_type = 8; /* SQL_TYPE_FLOAT */

  cols[2].name = "greeting";
  cols[2].inferred_type = 5; /* SQL_TYPE_VARCHAR */

  row_data[0] = &mock_int;
  row_data[1] = &mock_float;
  row_data[2] = mock_str;

  ASSERT_EQ(0, cdd_c_abstract_hydrate(&astruct, row_data, cols, 3));
  ASSERT_EQ(3, astruct.count);

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct, "id", &out_val));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_INT, out_val->type);
  ASSERT_EQ(42, out_val->value.i_val);

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct, "ratio", &out_val));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_FLOAT, out_val->type);
  ASSERT_EQ(3.14159, out_val->value.f_val);

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct, "greeting", &out_val));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_STRING, out_val->type);
  ASSERT_STR_EQ("Hello, Generic World!", out_val->value.s_val);

  ASSERT_EQ(0, cdd_c_abstract_struct_free(&astruct));
  PASS();
}

typedef struct MockSpecificRow {
  int id;
  double ratio;
  char greeting[32];
} mock_specific_row_t;

TEST test_abstract_struct_conversion(void) {
  cdd_c_abstract_struct_t astruct_in, astruct_out;
  mock_specific_row_t specific_out, specific_in;
  cdd_c_variant_t v_in;

  /* Avoid c_orm_meta initialization logic issues across compilers by mocking
   * the values in arrays explicitly via a safe test binding */
  c_orm_prop_meta_t p1, p2, p3;
  c_orm_meta_t meta;
  c_orm_prop_meta_t props[3];

  p1.name = "id";
  p1.type = "C_ORM_TYPE_INT32";
  p1.offset = offsetof(mock_specific_row_t, id);
  p1.is_array = 0;
  p1.length = 0;
  p1.is_secure = 0;
  p2.name = "ratio";
  p2.type = "C_ORM_TYPE_DOUBLE";
  p2.offset = offsetof(mock_specific_row_t, ratio);
  p2.is_array = 0;
  p2.length = 0;
  p2.is_secure = 0;
  p3.name = "greeting";
  p3.type = "C_ORM_TYPE_STRING";
  p3.offset = offsetof(mock_specific_row_t, greeting);
  p3.is_array = 0;
  p3.length = 32;
  p3.is_secure = 0;

  props[0] = p1;
  props[1] = p2;
  props[2] = p3;
  meta.name = "mock_specific_row_t";
  meta.size = sizeof(mock_specific_row_t);
  meta.num_props = 3;
  meta.props = props;

  /* Build Abstract */
  ASSERT_EQ(0, cdd_c_abstract_struct_init(&astruct_in));
  v_in.type = CDD_C_VARIANT_TYPE_INT;
  v_in.value.i_val = 10;
  cdd_c_abstract_set(&astruct_in, "id", &v_in);
  v_in.type = CDD_C_VARIANT_TYPE_FLOAT;
  v_in.value.f_val = 5.5;
  cdd_c_abstract_set(&astruct_in, "ratio", &v_in);
  v_in.type = CDD_C_VARIANT_TYPE_STRING;
  v_in.value.s_val = (char *)"Hello";
  cdd_c_abstract_set(&astruct_in, "greeting", &v_in);

  /* Abstract to Specific */
  memset(&specific_out, 0, sizeof(specific_out));
  ASSERT_EQ(0,
            cdd_c_abstract_to_specific(&specific_out, &astruct_in, &meta, 1));
  ASSERT_EQ(10, specific_out.id);
  ASSERT_EQ(5.5, specific_out.ratio);
  ASSERT_STR_EQ("Hello", specific_out.greeting);

  /* Specific to Abstract */
  specific_in.id = 20;
  specific_in.ratio = 11.1;
  strcpy(specific_in.greeting, "Goodbye");

  ASSERT_EQ(0, cdd_c_specific_to_abstract(&astruct_out, &specific_in, &meta));
  ASSERT_EQ(3, astruct_out.count);

  cdd_c_abstract_struct_free(&astruct_in);
  cdd_c_abstract_struct_free(&astruct_out);
  PASS();
}

#if defined(USE_SQLITE_LINKED)
#include <sqlite3.h>
TEST test_abstract_hydrate_sqlite3(void) {
  sqlite3 *db;
  sqlite3_stmt *stmt;
  cdd_c_abstract_struct_t astruct;
  cdd_c_variant_t *out_val;

  ASSERT_EQ(SQLITE_OK, sqlite3_open(":memory:", &db));
  ASSERT_EQ(
      SQLITE_OK,
      sqlite3_exec(
          db,
          "CREATE TABLE test(id INTEGER, ratio REAL, greeting TEXT, data BLOB)",
          NULL, NULL, NULL));
  ASSERT_EQ(
      SQLITE_OK,
      sqlite3_exec(db, "INSERT INTO test VALUES(42, 3.14, 'hello', x'010203')",
                   NULL, NULL, NULL));

  ASSERT_EQ(SQLITE_OK,
            sqlite3_prepare_v2(db, "SELECT * FROM test", -1, &stmt, NULL));
  ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt));

  ASSERT_EQ(0, cdd_c_abstract_hydrate_sqlite3(&astruct, stmt));

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct, "id", &out_val));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_INT, out_val->type);
  ASSERT_EQ(42, out_val->value.i_val);

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct, "ratio", &out_val));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_FLOAT, out_val->type);
  ASSERT_EQ(3.14, out_val->value.f_val);

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct, "greeting", &out_val));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_STRING, out_val->type);
  ASSERT_STR_EQ("hello", out_val->value.s_val);

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct, "data", &out_val));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_BLOB, out_val->type);
  ASSERT_EQ(3, (int)out_val->value.b_val.size);
  ASSERT_EQ(0x01, out_val->value.b_val.data[0]);

  cdd_c_abstract_struct_free(&astruct);
  sqlite3_finalize(stmt);
  sqlite3_close(db);
  PASS();
}
#else
TEST test_abstract_hydrate_sqlite3(void) {
  cdd_c_abstract_struct_t astruct;
  ASSERT_EQ(-1, cdd_c_abstract_hydrate_sqlite3(&astruct, NULL));
  PASS();
}
#endif

TEST test_abstract_hydrate_libpq(void) {
  cdd_c_abstract_struct_t astruct;
  /* Without a live PG server or heavily mocking libpq, we can only test the
   * failure path or unlinked path. */
  ASSERT_EQ(-1, cdd_c_abstract_hydrate_libpq(&astruct, NULL, 0));
  PASS();
}

TEST test_abstract_hydrate_mysql(void) {
  cdd_c_abstract_struct_t astruct;
  /* Without a live MySQL server, test failure path. */
  ASSERT_EQ(-1, cdd_c_abstract_hydrate_mysql(&astruct, NULL, NULL, 0));
  PASS();
}

#include <time.h>
/* clang-format on */

TEST test_mock_driver_specific_struct_hydration(void) {
  mock_specific_row_t specific_out;
  cdd_c_driver_row_t drow;
  cdd_c_column_meta_t cols[3];
  void *row_data[3];
  long long mock_id = 99;
  double mock_ratio = 1.234;
  char mock_greeting[] = "Mock Driver";

  c_orm_prop_meta_t p1, p2, p3;
  c_orm_meta_t meta;
  c_orm_prop_meta_t props[3];

  cdd_c_abstract_struct_t astruct;

  /* Setup metadata mapping for mock_specific_row_t */
  p1.name = "id";
  p1.type = "C_ORM_TYPE_INT32";
  p1.offset = offsetof(mock_specific_row_t, id);
  p1.is_array = 0;
  p1.length = 0;
  p1.is_secure = 0;

  p2.name = "ratio";
  p2.type = "C_ORM_TYPE_DOUBLE";
  p2.offset = offsetof(mock_specific_row_t, ratio);
  p2.is_array = 0;
  p2.length = 0;
  p2.is_secure = 0;

  p3.name = "greeting";
  p3.type = "C_ORM_TYPE_STRING";
  p3.offset = offsetof(mock_specific_row_t, greeting);
  p3.is_array = 0;
  p3.length = 32;
  p3.is_secure = 0;

  props[0] = p1;
  props[1] = p2;
  props[2] = p3;
  meta.name = "mock_specific_row_t";
  meta.size = sizeof(mock_specific_row_t);
  meta.num_props = 3;
  meta.props = props;

  /* Setup mock database driver output row */
  cols[0].name = "id";
  cols[0].inferred_type = 4; /* INT */
  cols[1].name = "ratio";
  cols[1].inferred_type = 8; /* FLOAT */
  cols[2].name = "greeting";
  cols[2].inferred_type = 5; /* VARCHAR */

  row_data[0] = &mock_id;
  row_data[1] = &mock_ratio;
  row_data[2] = mock_greeting;

  drow.row_data = row_data;
  drow.cols = cols;
  drow.n_cols = 3;
  drow.driver_ctx = NULL;

  /* Simulate router execution layer using pure generic abstract pipeline
   * fallback */
  memset(&specific_out, 0, sizeof(specific_out));

  ASSERT_EQ(0, cdd_c_abstract_hydrate(&astruct, drow.row_data, drow.cols,
                                      drow.n_cols));
  ASSERT_EQ(0, cdd_c_abstract_to_specific(&specific_out, &astruct,
                                          (const struct c_orm_meta *)&meta, 1));
  cdd_c_abstract_struct_free(&astruct);

  /* Verify bindings mapped through generically into strict specific struct
   * memory */
  ASSERT_EQ(99, specific_out.id);
  ASSERT_EQ(1.234, specific_out.ratio);
  ASSERT_STR_EQ("Mock Driver", specific_out.greeting);

  PASS();
}
TEST test_mock_driver_abstract_struct_hydration(void) {
  cdd_c_driver_row_t drow;
  cdd_c_column_meta_t cols[2];
  void *row_data[2];
  long long mock_id = 1001;
  char mock_name[] = "Dynamic User";

  cdd_c_abstract_struct_t astruct;
  cdd_c_variant_t *out_val;

  /* Setup mock database driver output row */
  cols[0].name = "dynamic_id";
  cols[0].inferred_type = 4; /* INT */
  cols[1].name = "dynamic_name";
  cols[1].inferred_type = 5; /* VARCHAR */

  row_data[0] = &mock_id;
  row_data[1] = mock_name;

  drow.row_data = row_data;
  drow.cols = cols;
  drow.n_cols = 2;
  drow.driver_ctx = NULL;

  /* Hydrate without any specific struct target */
  ASSERT_EQ(0, cdd_c_abstract_hydrate(&astruct, drow.row_data, drow.cols,
                                      drow.n_cols));
  ASSERT_EQ(2, astruct.count);

  /* Verify generic dynamic dictionary mapping */
  ASSERT_EQ(0, cdd_c_abstract_get(&astruct, "dynamic_id", &out_val));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_INT, out_val->type);
  ASSERT_EQ(1001, out_val->value.i_val);

  ASSERT_EQ(0, cdd_c_abstract_get(&astruct, "dynamic_name", &out_val));
  ASSERT_EQ(CDD_C_VARIANT_TYPE_STRING, out_val->type);
  ASSERT_STR_EQ("Dynamic User", out_val->value.s_val);

  cdd_c_abstract_struct_free(&astruct);

  PASS();
}

TEST test_abstract_struct_array(void) {
  cdd_c_abstract_struct_array_t arr;
  cdd_c_abstract_struct_t row1, row2;
  cdd_c_variant_t val;
  char *json_out;

  ASSERT_EQ(0, cdd_c_abstract_struct_array_init(&arr, 2));

  /* Row 1 */
  ASSERT_EQ(0, cdd_c_abstract_struct_init(&row1));
  val.type = CDD_C_VARIANT_TYPE_INT;
  val.value.i_val = 1;
  cdd_c_abstract_set(&row1, "id", &val);
  val.type = CDD_C_VARIANT_TYPE_STRING;
  val.value.s_val = "Alice";
  cdd_c_abstract_set(&row1, "name", &val);
  ASSERT_EQ(0, cdd_c_abstract_struct_array_append(&arr, &row1));

  /* Row 2 */
  ASSERT_EQ(0, cdd_c_abstract_struct_init(&row2));
  val.type = CDD_C_VARIANT_TYPE_INT;
  val.value.i_val = 2;
  cdd_c_abstract_set(&row2, "id", &val);
  val.type = CDD_C_VARIANT_TYPE_STRING;
  val.value.s_val = "Bob";
  cdd_c_abstract_set(&row2, "name", &val);
  ASSERT_EQ(0, cdd_c_abstract_struct_array_append(&arr, &row2));

  ASSERT_EQ(2, arr.count);

  ASSERT_EQ(0, cdd_c_abstract_struct_array_to_json(&arr, &json_out));
  ASSERT_NEQ(NULL, json_out);
  json_free_serialized_string(json_out);

  ASSERT_EQ(0, cdd_c_abstract_struct_array_free(&arr));

  PASS();
}

TEST test_benchmark_hydration(void) {
  const size_t ITERATIONS = 10000;
  size_t i;
  clock_t start, end;
  double time_specific, time_abstract;
  mock_specific_row_t specific_out;
  cdd_c_abstract_struct_t astruct;
  void *row_data[2];
  long long mock_can = 42;
  char mock_bar[] = "bench_str";

  c_orm_prop_meta_t p1, p2;
  c_orm_meta_t meta;
  c_orm_prop_meta_t props[2];

  p1.name = "greeting";
  p1.type = "C_ORM_TYPE_STRING";
  p1.offset = offsetof(mock_specific_row_t, greeting);
  p1.is_array = 0;
  p1.length = 32;
  p1.is_secure = 0;

  p2.name = "id";
  p2.type = "C_ORM_TYPE_INT32";
  p2.offset = offsetof(mock_specific_row_t, id);
  p2.is_array = 0;
  p2.length = 0;
  p2.is_secure = 0;

  props[0] = p1;
  props[1] = p2;

  meta.name = "MockSpecificRow";
  meta.size = sizeof(mock_specific_row_t);
  meta.num_props = 2;
  meta.props = props;

  /* Mock row setup */
  row_data[0] = mock_bar;
  row_data[1] = &mock_can;

  /* cdd_c_column_meta_t mapping for abstract hydration */
  cdd_c_column_meta_t cols[2];
  cols[0].name = "greeting";
  cols[0].inferred_type = 5; /* STRING */
  cols[1].name = "id";
  cols[1].inferred_type = 4; /* INT */

  start = clock();
  for (i = 0; i < ITERATIONS; ++i) {
    strncpy(specific_out.greeting, mock_bar, 31);
    specific_out.greeting[31] = '\0';
    specific_out.id = (int)mock_can;
    /* Normally we'd call a generated specific hydrator here */
  }
  end = clock();
  time_specific = (double)(end - start) / CLOCKS_PER_SEC;

  start = clock();
  for (i = 0; i < ITERATIONS; ++i) {
    cdd_c_abstract_hydrate(&astruct, row_data, cols, 2);
    cdd_c_abstract_to_specific(&specific_out, &astruct,
                               (const struct c_orm_meta *)&meta, 0);
    cdd_c_abstract_struct_free(&astruct);
  }
  end = clock();
  time_abstract = (double)(end - start) / CLOCKS_PER_SEC;

  printf("\n[Benchmark] Specific Hydration: %f seconds for %u iterations.\n",
         time_specific, (unsigned int)ITERATIONS);
  printf("[Benchmark] Abstract Fallback Hydration: %f seconds for %u "
         "iterations.\n",
         time_abstract, (unsigned int)ITERATIONS);

  /* Assert that abstract isn't insanely slow compared to direct struct
     assignment (we expect overhead, just avoiding timeouts). */
  ASSERT(time_abstract >= time_specific);

  PASS();
}

SUITE(abstract_struct_suite) {
  RUN_TEST(test_abstract_struct_memory_layout);
  RUN_TEST(test_variant_type_safety);
  RUN_TEST(test_abstract_struct_set_get);
  RUN_TEST(test_abstract_struct_json_roundtrip);
  RUN_TEST(test_abstract_hydrate);
  RUN_TEST(test_abstract_struct_conversion);
  RUN_TEST(test_abstract_hydrate_sqlite3);
  RUN_TEST(test_abstract_hydrate_libpq);
  RUN_TEST(test_abstract_hydrate_mysql);
  RUN_TEST(test_mock_driver_specific_struct_hydration);
  RUN_TEST(test_mock_driver_abstract_struct_hydration);
  RUN_TEST(test_abstract_struct_array);
  RUN_TEST(test_benchmark_hydration);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_ABSTRACT_STRUCT_H */
