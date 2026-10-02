#ifndef TEST_C_REST_C_ORM_H
#define TEST_C_REST_C_ORM_H
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <greatest.h>
#include <c_rest_modality.h>
#include <c_rest_router.h>
#include <c_rest_orm_crud.h>
#include <c_orm_db.h>
#include <cdd_c_error.h>
/* clang-format on */

TEST test_c_rest_c_orm_integration(void) {
  struct c_rest_context *ctx = NULL;
  c_rest_router *router = NULL;
  struct c_rest_orm_model model;
  int rc;

  model.table_name = "users";
  model.primary_key = "id";

  rc = (int)c_rest_init(C_REST_MODALITY_SYNC, &ctx);
  ASSERT_EQ(0, rc);

  rc = (int)c_rest_router_init(&router);
  ASSERT_EQ(0, rc);

  rc = (int)c_rest_router_add(router, "GET", "/users", c_rest_orm_crud_get_list,
                              &model);
  ASSERT_EQ(0, rc);

  rc = (int)c_rest_router_add(router, "POST", "/users", c_rest_orm_crud_create,
                              &model);
  ASSERT_EQ(0, rc);

  rc = (int)c_rest_set_router(ctx, router);
  ASSERT_EQ(0, rc);

  rc = (int)c_rest_run(ctx);
  ASSERT_EQ(0, rc);

  rc = (int)c_rest_router_destroy(router);
  ASSERT_EQ(0, rc);

  rc = (int)c_rest_orm_crud_get_list(NULL, NULL, NULL);
  ASSERT_EQ(0, rc);

  rc = (int)c_rest_orm_crud_create(NULL, NULL, NULL);
  ASSERT_EQ(0, rc);

  rc = (int)c_rest_stop(ctx);
  ASSERT_EQ(0, rc);

  rc = (int)c_rest_destroy(ctx);
  ASSERT_EQ(0, rc);

  PASS();
}

SUITE(c_rest_c_orm_suite) { RUN_TEST(test_c_rest_c_orm_integration); }

#endif /* TEST_C_REST_C_ORM_H */
