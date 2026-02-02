/**
 * @file test_c2openapi_schema.h
 * @brief Unit tests for Schema Registry Integration.
 *
 * @author Samuel Marks
 */

#ifndef TEST_C2OPENAPI_SCHEMA_H
#define TEST_C2OPENAPI_SCHEMA_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c2openapi_schema.h"
#include "c_inspector.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "fs.h"
#include "openapi_loader.h"

TEST test_register_single_struct(void) {
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  char *header_file = "test_reg_single.h";
  int rc;

  /* Setup */
  openapi_spec_init(&spec);
  type_def_list_init(&types);

  write_to_file(header_file, "struct User { int id; char *name; };");

  /* Inspect */
  rc = c_inspector_scan_file_types(header_file, &types);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, types.size);

  /* Register */
  rc = c2openapi_register_types(&spec, &types);
  ASSERT_EQ(0, rc);

  /* Verify in Spec */
  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_STR_EQ("User", spec.defined_schema_names[0]);
  {
    const struct StructFields *sf = &spec.defined_schemas[0];
    ASSERT_EQ(2, sf->size);
    ASSERT_STR_EQ("id", sf->fields[0].name);
    ASSERT_STR_EQ("name", sf->fields[1].name);
  }

  /* Cleanup */
  openapi_spec_free(&spec);
  type_def_list_free(&types);
  remove(header_file);
  PASS();
}

TEST test_register_deduplication(void) {
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  char *header_file = "test_reg_dedup.h";

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  /* Define struct twice (simulator for parsing multiple files) */
  /* c_inspector will list it once per scan, but we simulate calling register
   * twice */
  write_to_file(header_file, "struct Point { int x; };");
  c_inspector_scan_file_types(header_file, &types);

  /* Call 1 */
  ASSERT_EQ(0, c2openapi_register_types(&spec, &types));
  ASSERT_EQ(1, spec.n_defined_schemas);

  /* Call 2 (Same input) */
  ASSERT_EQ(0, c2openapi_register_types(&spec, &types));
  ASSERT_EQ(1, spec.n_defined_schemas); /* Should remain 1 */

  openapi_spec_free(&spec);
  type_def_list_free(&types);
  remove(header_file);
  PASS();
}

TEST test_register_multiple_structs(void) {
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  char *header_file = "test_reg_multi.h";

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  write_to_file(header_file,
                "struct A { int a; };\nstruct B { struct A *nested; };");

  c_inspector_scan_file_types(header_file, &types);
  ASSERT_EQ(2, types.size);

  c2openapi_register_types(&spec, &types);
  ASSERT_EQ(2, spec.n_defined_schemas);

  /* Order depends on file scan order, usually linear */
  /* Verify existence, not strict index */
  {
    const struct StructFields *sfA = openapi_spec_find_schema(&spec, "A");
    const struct StructFields *sfB = openapi_spec_find_schema(&spec, "B");
    ASSERT(sfA != NULL);
    ASSERT(sfB != NULL);

    ASSERT_STR_EQ("a", sfA->fields[0].name);
    ASSERT_STR_EQ("nested", sfB->fields[0].name);
    ASSERT_STR_EQ("A", sfB->fields[0].ref);
  }

  openapi_spec_free(&spec);
  type_def_list_free(&types);
  remove(header_file);
  PASS();
}

TEST test_register_null_safety(void) {
  struct OpenAPI_Spec spec;
  struct TypeDefList types;

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  ASSERT_EQ(EINVAL, c2openapi_register_types(NULL, &types));
  ASSERT_EQ(EINVAL, c2openapi_register_types(&spec, NULL));

  openapi_spec_free(&spec);
  type_def_list_free(&types);
  PASS();
}

SUITE(c2openapi_schema_suite) {
  RUN_TEST(test_register_single_struct);
  RUN_TEST(test_register_deduplication);
  RUN_TEST(test_register_multiple_structs);
  RUN_TEST(test_register_null_safety);
}

#endif /* TEST_C2OPENAPI_SCHEMA_H */
