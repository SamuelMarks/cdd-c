#ifndef TEST_SYNC_CODE_H
#define TEST_SYNC_CODE_H

#include <greatest.h>

#include <sync_code.h>

TEST test_sync_code_wrong_args(void) {
  char *argv[] = {"program", NULL};
  const int rc = sync_code_main(1, argv);
  ASSERT_EQ(rc, EXIT_FAILURE);
  PASS();
}

SUITE(sync_code_suite) { RUN_TEST(test_sync_code_wrong_args); }

#endif /* !TEST_SYNC_CODE_H */
