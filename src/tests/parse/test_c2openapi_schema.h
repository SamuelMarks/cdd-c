extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_c2openapi_schema.h
 * @brief Unit tests for C to OpenAPI schema mapping.
 */

#ifndef TEST_C2OPENAPI_SCHEMA_H
#define TEST_C2OPENAPI_SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/emit/schema.h"
#include "classes/parse/inspector.h"
#include "functions/parse/fs.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

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
  g_fail_io_after = -1;
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_register_multiple_structs(void) {
  struct StructFields *_ast_openapi_spec_find_schema_0;
  struct StructFields *_ast_openapi_spec_find_schema_1;
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
    const struct StructFields *sfA =
        (openapi_spec_find_schema(&spec, "A", &_ast_openapi_spec_find_schema_0),
         _ast_openapi_spec_find_schema_0);
    const struct StructFields *sfB =
        (openapi_spec_find_schema(&spec, "B", &_ast_openapi_spec_find_schema_1),
         _ast_openapi_spec_find_schema_1);
    ASSERT(sfA != NULL);
    ASSERT(sfB != NULL);

    ASSERT_STR_EQ("a", sfA->fields[0].name);
    ASSERT_STR_EQ("nested", sfB->fields[0].name);
    ASSERT_STR_EQ("A", sfB->fields[0].ref);
  }

  openapi_spec_free(&spec);
  type_def_list_free(&types);
  remove(header_file);
  g_fail_io_after = -1;
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_register_enum_schema(void) {
  struct StructFields *_ast_openapi_spec_find_schema_2;
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  char *header_file = "test_reg_enum.h";
  int rc;

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  write_to_file(header_file, "enum Color { RED, GREEN, BLUE };");

  rc = c_inspector_scan_file_types(header_file, &types);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, types.size);

  rc = c2openapi_register_types(&spec, &types);
  ASSERT_EQ(0, rc);

  {
    const struct StructFields *sf =
        (openapi_spec_find_schema(&spec, "Color",
                                  &_ast_openapi_spec_find_schema_2),
         _ast_openapi_spec_find_schema_2);
    ASSERT(sf != NULL);
    ASSERT_EQ(1, sf->is_enum);
    ASSERT_EQ(3, sf->enum_members.size);
    ASSERT_STR_EQ("RED", sf->enum_members.members[0]);
    ASSERT_STR_EQ("GREEN", sf->enum_members.members[1]);
    ASSERT_STR_EQ("BLUE", sf->enum_members.members[2]);
  }

  openapi_spec_free(&spec);
  type_def_list_free(&types);
  remove(header_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_register_type_union_copy(void) {
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  int rc;
  char *types_arr[] = {"string", "integer"};

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  /* Instead of scanning a file, we manually construct a TypeDefList */
  types.items = calloc(1, sizeof(struct TypeDefinition));
  types.size = 1;
  types.capacity = 1;
  types.items[0].name = "TestUnion";
  types.items[0].kind = KIND_STRUCT;
  types.items[0].details.struct_fields = calloc(1, sizeof(struct StructFields));

  struct StructFields *sf = types.items[0].details.struct_fields;
  struct_fields_init(sf);
  struct_fields_add(sf, "prop1", "union", NULL, NULL, NULL);

  sf->fields[0].type_union = types_arr;
  sf->fields[0].n_type_union = 2;
  sf->fields[0].items_type_union = types_arr;
  sf->fields[0].n_items_type_union = 2;

  rc = c2openapi_register_types(&spec, &types);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_STR_EQ("TestUnion", spec.defined_schema_names[0]);
  ASSERT_EQ(2, spec.defined_schemas[0].fields[0].n_type_union);
  ASSERT_STR_EQ("string", spec.defined_schemas[0].fields[0].type_union[0]);

  openapi_spec_free(&spec);

  sf->fields[0].type_union = NULL;
  sf->fields[0].items_type_union = NULL;
  /* struct_fields_free(sf) is handled by type_def_list_free! */
  types.items[0].name = NULL; /* prevent free */
  type_def_list_free(&types);
  g_fail_io_after = -1;

  PASS();
}

SUITE(c2openapi_schema_suite) {
  RUN_TEST(test_register_single_struct);
  RUN_TEST(test_register_deduplication);
  RUN_TEST(test_register_multiple_structs);
  RUN_TEST(test_register_null_safety);
  RUN_TEST(test_register_enum_schema);
  RUN_TEST(test_register_type_union_copy);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C2OPENAPI_SCHEMA_H */

TEST test_code2schema_utils(void) {
  /* Test readline with empty string causing len=0 */
  /* This is hard to force in fgets, it would mean reading a string with just a
   * null byte, which fgets doesn't typically do. */

  /* Test str_starts_with with NULL _out_val */
  str_starts_with("abc", "a", NULL);

  /* Test str_in_list with NULLs */
  const char *list[] = {"a", "b", NULL};
  str_in_list(NULL, list);
  str_in_list("a", NULL);

  /* Test key_in_list */
  extern int key_in_list(const char *key, const char **list, size_t count);
  key_in_list(NULL, list, 2);
  key_in_list("a", NULL, 2);
  key_in_list("a", list, 0);
  key_in_list("c", list, 2);

  /* Test copy_string_array_code2schema with NULLs */
  extern int copy_string_array_code2schema(char ***dst, size_t *dst_count,
                                           char **src, size_t src_count);
  char **dst;
  size_t count;
  char *src[] = {"a", NULL, "b"};
  copy_string_array_code2schema(NULL, &count, src, 3);
  copy_string_array_code2schema(&dst, NULL, src, 3);
  copy_string_array_code2schema(&dst, &count, NULL, 3);

  /* Test early return !src || src_count == 0 */
  copy_string_array_code2schema(&dst, &count, src, 0);

  /* Test free_string_array_code2schema with NULL strings */
  extern void free_string_array_code2schema(char **arr, size_t count);
  char **arr = calloc(2, sizeof(char *));
  free_string_array_code2schema(arr, 2);
  free(arr); /* oh free frees the items but wait free_string_array_code2schema
                does NOT free the array! */

  /* Test copy_string_array_code2schema allocations */
  char **dst2;
  size_t count2;
  char *src2[] = {"a"};

  extern int g_cdd_cst_alloc_token_fail;

  g_cdd_cst_alloc_token_fail = 1;
  copy_string_array_code2schema(&dst2, &count2, src2, 1);
  g_cdd_cst_alloc_token_fail = 0;

  g_cdd_cst_alloc_token_fail = 2;
  copy_string_array_code2schema(&dst2, &count2, src2, 1);
  g_cdd_cst_alloc_token_fail = 0;
  g_fail_io_after = -1;

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
  g_fail_io_after = -1;
  PASS();
}

TEST test_register_multiple_structs(void) {
  struct StructFields *_ast_openapi_spec_find_schema_0;
  struct StructFields *_ast_openapi_spec_find_schema_1;
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
    const struct StructFields *sfA =
        (openapi_spec_find_schema(&spec, "A", &_ast_openapi_spec_find_schema_0),
         _ast_openapi_spec_find_schema_0);
    const struct StructFields *sfB =
        (openapi_spec_find_schema(&spec, "B", &_ast_openapi_spec_find_schema_1),
         _ast_openapi_spec_find_schema_1);
    ASSERT(sfA != NULL);
    ASSERT(sfB != NULL);

    ASSERT_STR_EQ("a", sfA->fields[0].name);
    ASSERT_STR_EQ("nested", sfB->fields[0].name);
    ASSERT_STR_EQ("A", sfB->fields[0].ref);
  }

  openapi_spec_free(&spec);
  type_def_list_free(&types);
  remove(header_file);
  g_fail_io_after = -1;
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_register_enum_schema(void) {
  struct StructFields *_ast_openapi_spec_find_schema_2;
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  char *header_file = "test_reg_enum.h";
  int rc;

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  write_to_file(header_file, "enum Color { RED, GREEN, BLUE };");

  rc = c_inspector_scan_file_types(header_file, &types);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, types.size);

  rc = c2openapi_register_types(&spec, &types);
  ASSERT_EQ(0, rc);

  {
    const struct StructFields *sf =
        (openapi_spec_find_schema(&spec, "Color",
                                  &_ast_openapi_spec_find_schema_2),
         _ast_openapi_spec_find_schema_2);
    ASSERT(sf != NULL);
    ASSERT_EQ(1, sf->is_enum);
    ASSERT_EQ(3, sf->enum_members.size);
    ASSERT_STR_EQ("RED", sf->enum_members.members[0]);
    ASSERT_STR_EQ("GREEN", sf->enum_members.members[1]);
    ASSERT_STR_EQ("BLUE", sf->enum_members.members[2]);
  }

  openapi_spec_free(&spec);
  type_def_list_free(&types);
  remove(header_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_register_type_union_copy(void) {
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  int rc;
  char *types_arr[] = {"string", "integer"};

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  /* Instead of scanning a file, we manually construct a TypeDefList */
  types.items = calloc(1, sizeof(struct TypeDefinition));
  types.size = 1;
  types.capacity = 1;
  types.items[0].name = "TestUnion";
  types.items[0].kind = KIND_STRUCT;
  types.items[0].details.struct_fields = calloc(1, sizeof(struct StructFields));

  struct StructFields *sf = types.items[0].details.struct_fields;
  struct_fields_init(sf);
  struct_fields_add(sf, "prop1", "union", NULL, NULL, NULL);

  sf->fields[0].type_union = types_arr;
  sf->fields[0].n_type_union = 2;
  sf->fields[0].items_type_union = types_arr;
  sf->fields[0].n_items_type_union = 2;

  rc = c2openapi_register_types(&spec, &types);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_STR_EQ("TestUnion", spec.defined_schema_names[0]);
  ASSERT_EQ(2, spec.defined_schemas[0].fields[0].n_type_union);
  ASSERT_STR_EQ("string", spec.defined_schemas[0].fields[0].type_union[0]);

  openapi_spec_free(&spec);

  sf->fields[0].type_union = NULL;
  sf->fields[0].items_type_union = NULL;
  /* struct_fields_free(sf) is handled by type_def_list_free! */
  types.items[0].name = NULL; /* prevent free */
  type_def_list_free(&types);
  g_fail_io_after = -1;

  PASS();
}

SUITE(c2openapi_schema_suite) {
  RUN_TEST(test_register_single_struct);
  RUN_TEST(test_register_deduplication);
  RUN_TEST(test_register_multiple_structs);
  RUN_TEST(test_register_null_safety);
  RUN_TEST(test_register_enum_schema);
  RUN_TEST(test_register_type_union_copy);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C2OPENAPI_SCHEMA_H */

TEST test_code2schema_utils(void) {
  /* Test readline with empty string causing len=0 */
  /* This is hard to force in fgets, it would mean reading a string with just a
   * null byte, which fgets doesn't typically do. */

  /* Test str_starts_with with NULL _out_val */
  str_starts_with("abc", "a", NULL);

  /* Test str_in_list with NULLs */
  const char *list[] = {"a", "b", NULL};
  str_in_list(NULL, list);
  str_in_list("a", NULL);

  /* Test key_in_list */
  extern int key_in_list(const char *key, const char **list, size_t count);
  key_in_list(NULL, list, 2);
  key_in_list("a", NULL, 2);
  key_in_list("a", list, 0);
  key_in_list("c", list, 2);

  /* Test copy_string_array_code2schema with NULLs */
  extern int copy_string_array_code2schema(char ***dst, size_t *dst_count,
                                           char **src, size_t src_count);
  char **dst;
  size_t count;
  char *src[] = {"a", NULL, "b"};
  copy_string_array_code2schema(NULL, &count, src, 3);
  copy_string_array_code2schema(&dst, NULL, src, 3);
  copy_string_array_code2schema(&dst, &count, NULL, 3);

  /* Test early return !src || src_count == 0 */
  copy_string_array_code2schema(&dst, &count, src, 0);

  /* Test free_string_array_code2schema with NULL strings */
  extern void free_string_array_code2schema(char **arr, size_t count);
  char **arr = calloc(2, sizeof(char *));
  free_string_array_code2schema(arr, 2);
  free(arr); /* oh free frees the items but wait free_string_array_code2schema
                does NOT free the array! */

  /* Test copy_string_array_code2schema allocations */
  char **dst2;
  size_t count2;
  char *src2[] = {"a"};

  extern int g_cdd_cst_alloc_token_fail;

  g_cdd_cst_alloc_token_fail = 1;
  int rc_t1 = copy_string_array_code2schema(&dst2, &count2, src2, 1);
  if (rc_t1 != ENOMEM) {
    printf("copy_string_array_code2schema = %d, expected ENOMEM\n", rc_t1);
  }
  ASSERT_EQ(ENOMEM, rc_t1);
  g_cdd_cst_alloc_token_fail = 0;

  g_cdd_cst_alloc_token_fail = 2;
  int rc_t2 = copy_string_array_code2schema(&dst2, &count2, src2, 1);
  if (rc_t2 != ENOMEM) {
    printf("copy_string_array_code2schema = %d, expected ENOMEM\n", rc_t2);
  }
  ASSERT_EQ(ENOMEM, rc_t2);
  g_cdd_cst_alloc_token_fail = 0;
  g_fail_io_after = -1;

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
  g_fail_io_after = -1;
  PASS();
}

TEST test_register_multiple_structs(void) {
  struct StructFields *_ast_openapi_spec_find_schema_0;
  struct StructFields *_ast_openapi_spec_find_schema_1;
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
    const struct StructFields *sfA =
        (openapi_spec_find_schema(&spec, "A", &_ast_openapi_spec_find_schema_0),
         _ast_openapi_spec_find_schema_0);
    const struct StructFields *sfB =
        (openapi_spec_find_schema(&spec, "B", &_ast_openapi_spec_find_schema_1),
         _ast_openapi_spec_find_schema_1);
    ASSERT(sfA != NULL);
    ASSERT(sfB != NULL);

    ASSERT_STR_EQ("a", sfA->fields[0].name);
    ASSERT_STR_EQ("nested", sfB->fields[0].name);
    ASSERT_STR_EQ("A", sfB->fields[0].ref);
  }

  openapi_spec_free(&spec);
  type_def_list_free(&types);
  remove(header_file);
  g_fail_io_after = -1;
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_register_enum_schema(void) {
  struct StructFields *_ast_openapi_spec_find_schema_2;
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  char *header_file = "test_reg_enum.h";
  int rc;

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  write_to_file(header_file, "enum Color { RED, GREEN, BLUE };");

  rc = c_inspector_scan_file_types(header_file, &types);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, types.size);

  rc = c2openapi_register_types(&spec, &types);
  ASSERT_EQ(0, rc);

  {
    const struct StructFields *sf =
        (openapi_spec_find_schema(&spec, "Color",
                                  &_ast_openapi_spec_find_schema_2),
         _ast_openapi_spec_find_schema_2);
    ASSERT(sf != NULL);
    ASSERT_EQ(1, sf->is_enum);
    ASSERT_EQ(3, sf->enum_members.size);
    ASSERT_STR_EQ("RED", sf->enum_members.members[0]);
    ASSERT_STR_EQ("GREEN", sf->enum_members.members[1]);
    ASSERT_STR_EQ("BLUE", sf->enum_members.members[2]);
  }

  openapi_spec_free(&spec);
  type_def_list_free(&types);
  remove(header_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_register_type_union_copy(void) {
  struct OpenAPI_Spec spec;
  struct TypeDefList types;
  int rc;
  char *types_arr[] = {"string", "integer"};

  openapi_spec_init(&spec);
  type_def_list_init(&types);

  /* Instead of scanning a file, we manually construct a TypeDefList */
  types.items = calloc(1, sizeof(struct TypeDefinition));
  types.size = 1;
  types.capacity = 1;
  types.items[0].name = "TestUnion";
  types.items[0].kind = KIND_STRUCT;
  types.items[0].details.struct_fields = calloc(1, sizeof(struct StructFields));

  struct StructFields *sf = types.items[0].details.struct_fields;
  struct_fields_init(sf);
  struct_fields_add(sf, "prop1", "union", NULL, NULL, NULL);

  sf->fields[0].type_union = types_arr;
  sf->fields[0].n_type_union = 2;
  sf->fields[0].items_type_union = types_arr;
  sf->fields[0].n_items_type_union = 2;

  rc = c2openapi_register_types(&spec, &types);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_STR_EQ("TestUnion", spec.defined_schema_names[0]);
  ASSERT_EQ(2, spec.defined_schemas[0].fields[0].n_type_union);
  ASSERT_STR_EQ("string", spec.defined_schemas[0].fields[0].type_union[0]);

  openapi_spec_free(&spec);

  sf->fields[0].type_union = NULL;
  sf->fields[0].items_type_union = NULL;
  /* struct_fields_free(sf) is handled by type_def_list_free! */
  types.items[0].name = NULL; /* prevent free */
  type_def_list_free(&types);
  g_fail_io_after = -1;

  PASS();
}

SUITE(c2openapi_schema_suite) {
  RUN_TEST(test_register_single_struct);
  RUN_TEST(test_register_deduplication);
  RUN_TEST(test_register_multiple_structs);
  RUN_TEST(test_register_null_safety);
  RUN_TEST(test_register_enum_schema);
  RUN_TEST(test_register_type_union_copy);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C2OPENAPI_SCHEMA_H */

TEST test_code2schema_utils(void) {
  /* Test readline with empty string causing len=0 */
  /* This is hard to force in fgets, it would mean reading a string with just a
   * null byte, which fgets doesn't typically do. */

  /* Test str_starts_with with NULL _out_val */
  str_starts_with("abc", "a", NULL);

  /* Test str_in_list with NULLs */
  const char *list[] = {"a", "b", NULL};
  str_in_list(NULL, list);
  str_in_list("a", NULL);

  /* Test key_in_list */
  extern int key_in_list(const char *key, const char **list, size_t count);
  key_in_list(NULL, list, 2);
  key_in_list("a", NULL, 2);
  key_in_list("a", list, 0);
  key_in_list("c", list, 2);

  /* Test copy_string_array_code2schema with NULLs */
  extern int copy_string_array_code2schema(char ***dst, size_t *dst_count,
                                           char **src, size_t src_count);
  char **dst;
  size_t count;
  char *src[] = {"a", NULL, "b"};
  copy_string_array_code2schema(NULL, &count, src, 3);
  copy_string_array_code2schema(&dst, NULL, src, 3);
  copy_string_array_code2schema(&dst, &count, NULL, 3);

  /* Test early return !src || src_count == 0 */
  copy_string_array_code2schema(&dst, &count, src, 0);

  /* Test free_string_array_code2schema with NULL strings */
  extern void free_string_array_code2schema(char **arr, size_t count);
  char **arr = calloc(2, sizeof(char *));
  free_string_array_code2schema(arr, 2);
  free(arr); /* oh free frees the items but wait free_string_array_code2schema
                does NOT free the array! */

  /* Test copy_string_array_code2schema allocations */
  char **dst2;
  size_t count2;
  char *src2[] = {"a"};

  extern int g_cdd_cst_alloc_token_fail;

  g_cdd_cst_alloc_token_fail = 1;
  int rc_t1 = copy_string_array_code2schema(&dst2, &count2, src2, 1);
  ASSERT_EQ(ENOMEM, rc_t1);
  g_cdd_cst_alloc_token_fail = 0;

  g_cdd_cst_alloc_token_fail = 2;
  int rc_t1 = copy_string_array_code2schema(&dst2, &count2, src2, 1);
  ASSERT_EQ(ENOMEM, rc_t1);
  g_cdd_cst_alloc_token_fail = 0;
  g_fail_io_after = -1;

  PASS();
}
