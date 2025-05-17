#ifndef TEST_SCHEMA_CODEGEN_H
#define TEST_SCHEMA_CODEGEN_H

#include <greatest.h>

#include <schema_codegen.h>

TEST test_schema2code_wrong_args(void) {
  char *argv[] = {"program", NULL};
  const int rc = schema2code_main(1, argv);
  ASSERT_EQ(rc, EXIT_FAILURE);
  PASS();
}

SUITE(schema_codegen_suite) { RUN_TEST(test_schema2code_wrong_args); }

#endif /* !TEST_SCHEMA_CODEGEN_H */
