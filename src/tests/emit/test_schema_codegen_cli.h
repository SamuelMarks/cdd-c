#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
/**
 * @file test_schema_codegen_cli.h
 * @brief Unit tests for schema codegen CLI and main paths.
 *
 * @author Samuel Marks
 */

#ifndef TEST_SCHEMA_CODEGEN_CLI_H
#define TEST_SCHEMA_CODEGEN_CLI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/schema.h"
#include "classes/emit/schema_codegen.h"
#include "classes/parse/code2schema.h"
#include "functions/emit/codegen.h"
#include "functions/parse/fs.h"
/* clang-format on */

extern C_CDD_EXPORT int g_schema_fail_io_after;
extern C_CDD_EXPORT int g_schema_io_calls;
extern C_CDD_EXPORT int g_schema_strdup_fail;
extern C_CDD_EXPORT int g_struct_fields_init_fail;
extern C_CDD_EXPORT int g_json_object_to_struct_fields_fail;
extern C_CDD_EXPORT int g_schema_realloc_fail;
extern C_CDD_EXPORT int g_schema_codegen_force_fail;
extern C_CDD_EXPORT int g_cdd_strdup_fail;

TEST test_schema_codegen_cli_exhaustive_io(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  int rc;
  const char schema_json[] = {
      123, 34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  32,
      123, 32,  32,  34,  115, 99,  104, 101, 109, 97,  115, 34,  58,  32,  123,
      32,  32,  32,  32,  34,  77,  121, 83,  116, 114, 117, 99,  116, 34,  58,
      32,  123, 32,  32,  32,  32,  32,  32,  34,  116, 121, 112, 101, 34,  58,
      32,  34,  111, 98,  106, 101, 99,  116, 34,  44,  32,  32,  32,  32,  32,
      32,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  32,
      123, 32,  32,  32,  32,  32,  32,  32,  32,  34,  102, 111, 111, 34,  58,
      32,  123, 32,  34,  116, 121, 112, 101, 34,  58,  32,  34,  115, 116, 114,
      105, 110, 103, 34,  32,  125, 32,  32,  32,  32,  32,  32,  125, 32,  32,
      32,  32,  125, 32,  32,  125, 125, 125, 0};
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_codegen_schema_io.json", "w") != 0)
    f = NULL;
#else
  f = fopen("test_codegen_schema_io.json", "w");
#endif
  if (f) {
    fputs(schema_json, f);
    fclose(f);
  }

  for (i = 0; i < 50; ++i) {
    void *root;
    void *schemas;
    g_schema_fail_io_after = i;
    g_schema_io_calls = 0;

    root = json_parse_file("test_codegen_schema_io.json");
    schemas = json_object_get_object(json_value_get_object(root), "components");
    schemas = json_object_get_object(schemas, "schemas");

    rc = generate_header("test_codegen_schema_io.json", "out_prefix", schemas,
                         NULL);
    if (rc == 0)
      rc = generate_source("test_codegen_schema_io.json", "out_prefix", schemas,
                           NULL);
    json_value_free(root);

    if (rc == 0)
      break;
    ASSERT(rc != 0);
  }

  g_schema_fail_io_after = -1;
  remove("test_codegen_schema_io.json");
#endif
  g_fail_io_after = -1;
  PASS();
}

TEST test_schema_codegen_union_arrays(void) {
  int rc;
  const char *const filename = "union_array_schema.json";
  const char *argv[2];
  const char schema[] = {
      123, 34,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  123,
      34,  115, 99,  104, 101, 109, 97,  115, 34,  58,  123, 34,  80,  101, 116,
      34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101,
      99,  116, 34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115,
      34,  58,  123, 34,  109, 101, 111, 119, 34,  58,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 44,
      34,  85,  110, 105, 111, 110, 78,  117, 109, 98,  101, 114, 34,  58,  32,
      123, 32,  34,  111, 110, 101, 79,  102, 34,  58,  32,  91,  32,  123, 32,
      34,  116, 121, 112, 101, 34,  58,  32,  34,  97,  114, 114, 97,  121, 34,
      44,  32,  34,  105, 116, 101, 109, 115, 34,  58,  32,  123, 32,  34,  116,
      121, 112, 101, 34,  58,  32,  34,  110, 117, 109, 98,  101, 114, 34,  32,
      125, 32,  125, 32,  93,  32,  125, 44,  34,  85,  110, 105, 111, 110, 73,
      110, 116, 101, 103, 101, 114, 34,  58,  32,  123, 32,  34,  111, 110, 101,
      79,  102, 34,  58,  32,  91,  32,  123, 32,  34,  116, 121, 112, 101, 34,
      58,  32,  34,  97,  114, 114, 97,  121, 34,  44,  32,  34,  105, 116, 101,
      109, 115, 34,  58,  32,  123, 32,  34,  116, 121, 112, 101, 34,  58,  32,
      34,  105, 110, 116, 101, 103, 101, 114, 34,  32,  125, 32,  125, 32,  93,
      32,  125, 44,  34,  85,  110, 105, 111, 110, 66,  111, 111, 108, 34,  58,
      32,  123, 32,  34,  111, 110, 101, 79,  102, 34,  58,  32,  91,  32,  123,
      32,  34,  116, 121, 112, 101, 34,  58,  32,  34,  97,  114, 114, 97,  121,
      34,  44,  32,  34,  105, 116, 101, 109, 115, 34,  58,  32,  123, 32,  34,
      116, 121, 112, 101, 34,  58,  32,  34,  98,  111, 111, 108, 101, 97,  110,
      34,  32,  125, 32,  125, 32,  93,  32,  125, 44,  34,  85,  110, 105, 111,
      110, 82,  101, 102, 34,  58,  32,  123, 32,  34,  111, 110, 101, 79,  102,
      34,  58,  32,  91,  32,  123, 32,  34,  116, 121, 112, 101, 34,  58,  32,
      34,  97,  114, 114, 97,  121, 34,  44,  32,  34,  105, 116, 101, 109, 115,
      34,  58,  32,  123, 32,  34,  36,  114, 101, 102, 34,  58,  32,  34,  35,
      47,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  115, 99,  104,
      101, 109, 97,  115, 47,  80,  101, 116, 34,  32,  125, 32,  125, 32,  93,
      32,  125, 125, 125, 125, 0};
  argv[0] = filename;
  argv[1] = "union_array_out";

  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);

  {
    void *root = json_parse_file(filename);
    void *schemas =
        json_object_get_object(json_value_get_object(root), "components");
    schemas = json_object_get_object(schemas, "schemas");
    rc = generate_header(filename, argv[1], schemas, NULL);
    if (rc == 0)
      rc = generate_source(filename, argv[1], schemas, NULL);
    json_value_free(root);
  }
  ASSERT_EQ(0, rc);

  remove(filename);
  remove("union_array_out.h");
  remove("union_array_out.c");
  remove("union_array_out_types.h");
  g_fail_io_after = -1;
  PASS();
}

TEST test_schema_codegen_specific_structs(void) {
  int rc;
  const char *const filename = "specific_structs.json";
  const char *argv[2];
  const char *schema =
      "{"
      "\"components\":{"
      "\"schemas\":{"
      "\"OAuth2Error\":{\"type\":\"object\",\"properties\":{\"error\":{"
      "\"type\":\"string\"}}},"
      "\"JwtPayload\":{\"type\":\"object\",\"properties\":{\"sub\":{\"type\":"
      "\"string\"}}},"
      "\"OAuth2TokenResponse\":{\"type\":\"object\",\"properties\":{\"access_"
      "token\":{\"type\":\"string\"}}}"
      "}}}";
  argv[0] = filename;
  argv[1] = "specific_out";

  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)(size_t)argv);
  ASSERT_EQ(0, rc);

  remove(filename);
  remove("specific_out.h");
  remove("specific_out.c");
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
/*  (moved to global) */
#endif

TEST test_schema_codegen_main_paths(void) {
  int rc;
  const char *const filename = "main_paths.json";
  const char *argv[5];
  const char schema_defs[] = {
      123, 34,  36,  100, 101, 102, 115, 34,  58,  123, 34,  88,  34,  58,  123,
      34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116, 34,
      44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123,
      34,  120, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116,
      114, 105, 110, 103, 34,  125, 125, 125, 44,  34,  77,  121, 69,  110, 117,
      109, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114,
      105, 110, 103, 34,  44,  34,  101, 110, 117, 109, 34,  58,  91,  34,  65,
      34,  44,  34,  66,  34,  93,  125, 44,  34,  77,  121, 85,  110, 105, 111,
      110, 34,  58,  123, 34,  97,  110, 121, 79,  102, 34,  58,  91,  123, 34,
      116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125,
      44,  123, 34,  116, 121, 112, 101, 34,  58,  34,  105, 110, 116, 101, 103,
      101, 114, 34,  125, 93,  125, 44,  34,  74,  119, 116, 80,  97,  121, 108,
      111, 97,  100, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111,
      98,  106, 101, 99,  116, 34,  44,  34,  112, 114, 111, 112, 101, 114, 116,
      105, 101, 115, 34,  58,  123, 34,  115, 117, 98,  34,  58,  123, 34,  116,
      121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125,
      125, 44,  34,  79,  65,  117, 116, 104, 50,  84,  111, 107, 101, 110, 82,
      101, 115, 112, 111, 110, 115, 101, 34,  58,  123, 34,  116, 121, 112, 101,
      34,  58,  34,  111, 98,  106, 101, 99,  116, 34,  44,  34,  112, 114, 111,
      112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,  97,  99,  99,  101,
      115, 115, 95,  116, 111, 107, 101, 110, 34,  58,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 44,
      34,  79,  65,  117, 116, 104, 50,  69,  114, 114, 111, 114, 34,  58,  123,
      34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116, 34,
      44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123,
      34,  101, 114, 114, 111, 114, 34,  58,  123, 34,  116, 121, 112, 101, 34,
      58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 44,  34,  77,
      121, 83,  116, 114, 105, 110, 103, 34,  58,  123, 34,  116, 121, 112, 101,
      34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 0};

  /* 1. argc < 2 */
  rc = schema2code_main(1, (char **)(size_t)argv);
  ASSERT(rc != 0);

  /* 2. get_basename fails */
#ifdef CDD_BUILD_TESTS
  argv[0] = "a";
  argv[1] = "";
  {
#include <c_cdd_export.h>
    /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
    g_cdd_strdup_fail = 1;
    rc = schema2code_main(2, (char **)(size_t)argv);
    ASSERT(rc != 0);
    g_cdd_strdup_fail = 0;
  }
#endif

  /* 3. flags parsing and io failures */
  rc = write_to_file(filename, schema_defs);
  ASSERT_EQ(0, rc);
  argv[0] = filename;
  argv[1] = "main_out";
  argv[2] = "--guard-enum=EG";
  argv[3] = "--guard-json=JG";
  argv[4] = "--guard-utils=UG";

#ifdef CDD_BUILD_TESTS
  {
    int io_i = 1;
    /* extern C_CDD_EXPORT int g_schema_codegen_force_fail; (moved to global) */
    for (io_i = 1; io_i < 200; io_i++) {
      g_schema_fail_io_after = io_i;
      g_schema_io_calls = 0;
      rc = schema2code_main(5, (char **)(size_t)argv);
      if (rc == 0)
        break;
    }
    g_schema_fail_io_after = -1;

    for (io_i = 1; io_i < 150; io_i++) {
      g_schema_codegen_force_fail = io_i;
      rc = schema2code_main(5, (char **)(size_t)argv);
      if (rc == 0)
        break;
    }
    g_schema_codegen_force_fail = 0;
  }
#else
  rc = schema2code_main(5, (char **)(size_t)argv);
  ASSERT_EQ(0, rc);
#endif

  /* call once with unknown flag to cover else branch fallthrough */
  {
    const char *argv_unk[3];
    argv_unk[0] = filename;
    argv_unk[1] = "main_out";
    argv_unk[2] = "--unknown-flag";
    rc = schema2code_main(3, (char **)(size_t)argv_unk);
    ASSERT_EQ(0, rc);
  }

  remove("main_out.h");
  remove("main_out.c");

  /* 4. json_parse_file fails */
  argv[0] = "does_not_exist.json";
  rc = schema2code_main(2, (char **)(size_t)argv);
  ASSERT(rc != 0);

  /* 5. missing schemas */
  write_to_file(filename, "{}");
  argv[0] = filename;
  rc = schema2code_main(2, (char **)(size_t)argv);
  ASSERT(rc != 0);

  /* 6. generate_header / generate_source fails */
#ifdef CDD_BUILD_TESTS
  write_to_file(filename, schema_defs);
  g_schema_fail_io_after = 0;
  g_schema_io_calls = 0;
  rc = schema2code_main(2, (char **)(size_t)argv);
  ASSERT(rc != 0);

  g_schema_fail_io_after = 3; /* Succeed header start, fail later */
  g_schema_io_calls = 0;
  rc = schema2code_main(2, (char **)(size_t)argv);
  ASSERT(rc != 0);

  g_schema_fail_io_after = -1;
#endif

  remove(filename);
  remove("main_out.h");
  remove("main_out.c");
  g_fail_io_after = -1;
  PASS();
}

#include <c_cdd_export.h>

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_schema_strdup_fail;
extern C_CDD_EXPORT int g_schema_io_calls;
extern C_CDD_EXPORT int g_struct_fields_init_fail;
extern C_CDD_EXPORT int g_json_object_to_struct_fields_fail;
extern C_CDD_EXPORT int g_schema_realloc_fail;
extern C_CDD_EXPORT int g_schema_codegen_force_fail;
extern C_CDD_EXPORT int g_cdd_strdup_fail;
extern C_CDD_EXPORT int g_schema_fail_io_after;

/* extern C_CDD_EXPORT int g_struct_fields_init_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_json_object_to_struct_fields_fail; (moved to
 * global) */

TEST test_schema_codegen_init_fail(void) {
  void *root;
  void *schemas;
  int rc;
  const char *schema_json =
      "{\"components\": {\"schemas\": {\"MyStruct\": {\"properties\": {}}}}}";
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_codegen_schema_init.json", "w") != 0)
    f = NULL;
#else
  f = fopen("test_codegen_schema_init.json", "w");
#endif
  if (f) {
    fputs(schema_json, f);
    fclose(f);
  }

  root = json_parse_file("test_codegen_schema_init.json");
  schemas = json_object_get_object(json_value_get_object(root), "components");
  schemas = json_object_get_object(schemas, "schemas");

  g_struct_fields_init_fail = 1;
  rc = generate_header("test_out2", "basename", schemas, NULL);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  g_struct_fields_init_fail = 2; /* Fail Pass 2 of generate_header */
  rc = generate_header("test_out2", "basename", schemas, NULL);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  g_json_object_to_struct_fields_fail = 2; /* Fail Pass 2 mapping */
  rc = generate_header("test_out2", "basename", schemas, NULL);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_json_object_to_struct_fields_fail = 0;

  g_struct_fields_init_fail = 1;
  rc = generate_source("test_out2", "basename", schemas, NULL);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_struct_fields_init_fail = 0;

  json_value_free(root);
  remove("test_codegen_schema_init.json");
  remove("test_out2.h");
  remove("test_out2.c");
  PASS();
}

TEST test_schema_codegen_parse_error(void) {
  void *root;
  void *schemas;
  int rc;
  const char *schema_json =
      "{\"components\": {\"schemas\": {\"MyStruct\": 123}}}";
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_codegen_schema_parse.json", "w") != 0)
    f = NULL;
#else
  f = fopen("test_codegen_schema_parse.json", "w");
#endif
  if (f) {
    fputs(schema_json, f);
    fclose(f);
  }

  root = json_parse_file("test_codegen_schema_parse.json");
  schemas = json_object_get_object(json_value_get_object(root), "components");
  schemas = json_object_get_object(schemas, "schemas");

  rc = generate_header("test_out", "basename", schemas, NULL);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  rc = generate_source("test_out", "basename", schemas, NULL);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  json_value_free(root);
  remove("test_codegen_schema_parse.json");
  remove("test_out.h");
  remove("test_out.c");
  PASS();
}

TEST test_schema_codegen_source_fail(void) {
  void *root;
  void *schemas;
  int rc;
  const char *schema_json = "{\"components\": {\"schemas\": {\"MyStruct\": "
                            "{\"type\": \"object\",\"properties\": {}}}}}";
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_codegen_schema_io.json", "w") != 0)
    f = NULL;
#else
  f = fopen("test_codegen_schema_io.json", "w");
#endif
  if (f) {
    fputs(schema_json, f);
    fclose(f);
  }

  root = json_parse_file("test_codegen_schema_io.json");
  schemas = json_object_get_object(json_value_get_object(root), "components");
  schemas = json_object_get_object(schemas, "schemas");

#if defined(_MSC_VER)
  if (fopen_s(&f, "test_out_source.c", "w") != 0)
    f = NULL;
#else
  f = fopen("test_out_source.c", "w");
#endif
  if (f)
    fclose(f);
  rc = system("chmod 0444 test_out_source.c");
  ASSERT(rc == 0 || rc != 0);

  /* Call main which calls generate_header and generate_source */
  {
    char *argv_bad[] = {(char *)(size_t)(size_t) "test_codegen_schema_io.json",
                        (char *)(size_t)(size_t) "test_out_source"};
    rc = schema2code_main(2, argv_bad);
#if !defined(_WIN32) && !defined(__EMSCRIPTEN__) && !defined(__CYGWIN__)
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
#else
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_UNKNOWN);
#endif
  }

  rc = system("chmod 0666 test_out_source.c");
  ASSERT(rc == 0 || rc != 0);
  remove("test_out_source.c");
  remove("test_out_source.h");
  json_value_free(root);
  remove("test_codegen_schema_io.json");
  PASS();
}
TEST test_schema_codegen_system_error(void) {
  void *root;
  void *schemas;
  int rc;
  const char *schema_json = "{\"components\": {\"schemas\": {\"MyStruct\": "
                            "{\"type\": \"object\",\"properties\": {}}}}}";
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_codegen_schema_io.json", "w") != 0)
    f = NULL;
#else
  f = fopen("test_codegen_schema_io.json", "w");
#endif
  if (f) {
    fputs(schema_json, f);
    fclose(f);
  }

  root = json_parse_file("test_codegen_schema_io.json");
  schemas = json_object_get_object(json_value_get_object(root), "components");
  schemas = json_object_get_object(schemas, "schemas");

  rc = generate_header("/invalid/path/prefix", "basename", schemas, NULL);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, rc);

  rc = generate_source("/invalid/path/prefix", "basename", schemas, NULL);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, rc);

  json_value_free(root);
  remove("test_codegen_schema_io.json");
  PASS();
}

TEST test_schema_codegen_main_errors(void) {
  int rc;
  char *argv_bad1[] = {(char *)(size_t)(size_t) "file.json"};
  char *argv_bad2[] = {(char *)(size_t)(size_t) "file.json",
                       (char *)(size_t)NULL};
  char *argv_bad3[] = {(char *)(size_t)(size_t) "nonexistent.json",
                       (char *)(size_t)(size_t) "prefix"};
  char *argv_bad4[] = {(char *)(size_t)(size_t) "file.json",
                       (char *)(size_t)(size_t) "/invalid/path/prefix"};
  const char *schema_json = "{\"components\": {\"schemas\": {\"MyStruct\": "
                            "{\"type\": \"object\",\"properties\": {}}}}}";
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "file.json", "w") != 0)
    f = NULL;
#else
  f = fopen("file.json", "w");
#endif
  if (f) {
    fputs(schema_json, f);
    fclose(f);
  }

  rc = schema2code_main(1, argv_bad1);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  rc = schema2code_main(2, argv_bad2);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  rc = schema2code_main(2, argv_bad3);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  rc = schema2code_main(2, argv_bad4);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  remove("file.json");
  PASS();
}

TEST test_schema_codegen_non_object_no_props(void) {
  void *root;
  void *schemas;
  int rc;
  const char *schema_json =
      "{\"components\": {\"schemas\": {"
      "\"SimpleStr\": {\"type\": \"string\"},"
      "\"NoTypeWithProps\": {\"properties\": {\"bar\": {\"type\": \"string\"}}}"
      "}}}";
  root = json_parse_string(schema_json);
  ASSERT_NEQ(NULL, root);
  schemas = json_object_get_object(json_value_get_object(root), "components");
  schemas = json_object_get_object(schemas, "schemas");
  rc = generate_header("test_simple_str", "test_simple_str", schemas, NULL);
  ASSERT_EQ(0, rc);
  rc = generate_source("test_simple_str", "test_simple_str", schemas, NULL);
  ASSERT_EQ(0, rc);
  json_value_free(root);
  remove("test_simple_str.h");
  remove("test_simple_str.c");
  PASS();
}

SUITE(schema_codegen_cli_suite) {
  RUN_TEST(test_schema_codegen_cli_exhaustive_io);
  RUN_TEST(test_schema_codegen_union_arrays);
  RUN_TEST(test_schema_codegen_specific_structs);
  RUN_TEST(test_schema_codegen_main_paths);
  RUN_TEST(test_schema_codegen_init_fail);
  RUN_TEST(test_schema_codegen_parse_error);
  RUN_TEST(test_schema_codegen_source_fail);
  RUN_TEST(test_schema_codegen_system_error);
  RUN_TEST(test_schema_codegen_main_errors);
  RUN_TEST(test_schema_codegen_non_object_no_props);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SCHEMA_CODEGEN_CLI_H */
