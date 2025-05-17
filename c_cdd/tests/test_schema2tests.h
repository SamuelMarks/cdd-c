#ifndef TEST_SCHEMA2TESTS_H
#define TEST_SCHEMA2TESTS_H

#include <greatest.h>

#include <schema2tests.h>

TEST test_jsonschema2tests_wrong_args(void) {
  char *argv[] = {"program", NULL};
  const int rc = jsonschema2tests_main(1, argv);
  ASSERT_EQ(rc, EXIT_FAILURE);
  PASS();
}

SUITE(schema2tests_suite) { RUN_TEST(test_jsonschema2tests_wrong_args); }

#endif /* !TEST_SCHEMA2TESTS_H */
