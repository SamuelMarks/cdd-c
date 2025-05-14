#ifndef TEST_DATACLASSES_H
#define TEST_DATACLASSES_H

#include <mocks/simple_json.h>

#include <greatest.h>

TEST test_struct_default_deepcopy_eq(void) {
  struct FooE *f0, *f1;

  int rc = FooE_default(&f0);
  if (rc != 0)
    FAIL();
  rc = FooE_deepcopy(f0, &f1);
  if (rc != 0)
    FAIL();
  ASSERT(FooE_eq(f0, f1));

  f0->can = 55;
  ASSERT(!FooE_eq(f0, f1));

  FooE_cleanup(f0);
  FooE_cleanup(f1);

  PASS();
}

SUITE(dataclasses_suite) { RUN_TEST(test_struct_default_deepcopy_eq); }

#endif /* TEST_DATACLASSES_H */
