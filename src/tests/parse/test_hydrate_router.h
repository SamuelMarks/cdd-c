/**
 * @file test_hydrate_router.h
 * @brief Tests for dynamic struct routing.
 */

#ifndef C_CDD_TEST_HYDRATE_ROUTER_H
#define C_CDD_TEST_HYDRATE_ROUTER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/parse/hydrate_router.h"
#include <greatest.h>
/* clang-format on */

typedef struct MockHydrateStruct {
  long long id;
} mock_hydrate_struct_t;

int mock_hydrator(void *out_struct, const cdd_c_abstract_struct_t *row) {
  mock_hydrate_struct_t *typed_out = (mock_hydrate_struct_t *)out_struct;
  cdd_c_variant_t *val;
  if (cdd_c_abstract_get(row, "id", &val) == 0) {
    if (val->type == CDD_C_VARIANT_TYPE_INT) {
      typed_out->id = val->value.i_val;
      return 0;
    }
  }
  return -1;
}

TEST test_hydrate_router_registration_and_dispatch(void) {
  cdd_c_hydrate_router_t router;
  cdd_c_abstract_struct_t row;
  cdd_c_variant_t val;
  mock_hydrate_struct_t out_struct;
  const struct c_orm_meta *dummy_meta = NULL;
  unsigned long long hash = 12345;

  ASSERT_EQ(0, cdd_c_hydrate_router_init(&router));

  /* Test register */
  ASSERT_EQ(0, cdd_c_hydrate_router_register(&router, hash, dummy_meta,
                                             mock_hydrator));
  ASSERT_EQ(1, router.count);

  /* Setup mock row */
  ASSERT_EQ(0, cdd_c_abstract_struct_init(&row));
  val.type = CDD_C_VARIANT_TYPE_INT;
  val.value.i_val = 999;
  ASSERT_EQ(0, cdd_c_abstract_set(&row, "id", &val));

  /* Test success dispatch */
  out_struct.id = 0;
  ASSERT_EQ(0, cdd_c_hydrate_router_dispatch(&router, hash, &row, &out_struct));
  ASSERT_EQ(999, out_struct.id);

  /* Test fail dispatch (unknown hash) */
  ASSERT_EQ(-1,
            cdd_c_hydrate_router_dispatch(&router, 54321, &row, &out_struct));

  cdd_c_abstract_struct_free(&row);
  ASSERT_EQ(0, cdd_c_hydrate_router_free(&router));
  PASS();
}

SUITE(hydrate_router_suite) {
  RUN_TEST(test_hydrate_router_registration_and_dispatch);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_HYDRATE_ROUTER_H */
