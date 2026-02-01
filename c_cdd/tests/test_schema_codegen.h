#ifndef TEST_SCHEMA_CODEGEN_H

#define TEST_SCHEMA_CODEGEN_H

#include <greatest.h>

#include <schema_codegen.h>

#include "cdd_test_helpers/cdd_helpers.h"

#include "code2schema.h" /* Included to avoid implicit declaration warnings */

#include "codegen.h"

#include "fs.h"

/* Forward declare static functions from schema_codegen.c if we want unit-test

 * integration */

/* However, since schema_codegen.c is compiled separately, we verify via full

 * CLI integration or file inspection */

TEST test_schema_codegen_circular_refs(void) {

  /*

   * Verify that circular dependencies generate valid forward declarations using

   * a multi-pass header generation. A references B, B references A.

   */

  const char *const filename = "circular.json";

  const char *argv[] = {filename, "circular_out"};

  int rc;

  char *header_content = NULL;

  size_t sz;

  /* Create schemas where A has property B, B has property A */

  const char *schema = "{\"components\": {\"schemas\": {"

                       "\"A\": {\"type\": \"object\", \"properties\": {\"b\": "

                       "{\"$ref\": \"#/components/schemas/B\"}}},"

                       "\"B\": {\"type\": \"object\", \"properties\": {\"a\": "

                       "{\"$ref\": \"#/components/schemas/A\"}}}"

                       "}}}";

  rc = write_to_file(filename, schema);

  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)argv);

  ASSERT_EQ(0, rc);

  /* Read Generated Header */

  rc = read_to_file("circular_out.h", "r", &header_content, &sz);

  ASSERT_EQ(0, rc);

  /* ASSERTIONS: */

  /* 1. struct A; and struct B; must be present BEFORE their full definitions */

  {

    char *fwd_a = strstr(header_content, "struct A;");

    char *fwd_b = strstr(header_content, "struct B;");

    char *def_a = strstr(header_content, "struct LIB_EXPORT A {");

    char *def_b = strstr(header_content, "struct LIB_EXPORT B {");

    ASSERT(fwd_a != NULL);

    ASSERT(fwd_b != NULL);

    ASSERT(def_a != NULL);

    ASSERT(def_b != NULL);

    /* Verify Ordering */

    ASSERT(fwd_a < def_a);

    ASSERT(fwd_b < def_b);

    /* The referenced types inside the structs should rely on these forward

     * declarations being valid C */
  }

  free(header_content);

  remove(filename);

  remove("circular_out.h");

  remove("circular_out.c");

  PASS();
}

TEST test_codegen_config_json_guards(void) {

  /*

   * Verify that generated functions are wrapped in #ifdef TO_JSON ... #endif

   * when configured.

   */

  FILE *tmp = tmpfile();

  struct StructFields sf;

  /* We use the specific config struct now */
  struct CodegenJsonConfig config;

  char *content;

  long sz;

  /* Setup */

  ASSERT(tmp);

  struct_fields_init(&sf);

  struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL);

  memset(&config, 0, sizeof(config));

  config.guard_macro = "ENABLE_JSON";

  /* Generate */

  ASSERT_EQ(0, write_struct_to_json_func(tmp, "GuardStruct", &sf, &config));

  ASSERT_EQ(0, write_struct_from_json_func(tmp, "GuardStruct", &config));

  ASSERT_EQ(

      0, write_struct_from_jsonObject_func(tmp, "GuardStruct", &sf, &config));

  /* Check content */

  fseek(tmp, 0, SEEK_END);

  sz = ftell(tmp);

  rewind(tmp);

  content = calloc(1, sz + 1);

  fread(content, 1, sz, tmp);

  /* Check Guards exist */

  ASSERT(strstr(content, "#ifdef ENABLE_JSON"));

  ASSERT(strstr(content, "#endif /* ENABLE_JSON */"));

  /* Verify the guards appear multiple times (once per function block) */

  {

    char *p = content;

    int count = 0;

    while ((p = strstr(p, "#ifdef ENABLE_JSON")) != NULL) {

      count++;

      p++;
    }

    /* We called 3 write_ functions, expecting 3 blocks */

    ASSERT_EQ(3, count);
  }

  free(content);

  struct_fields_free(&sf);

  fclose(tmp);

  PASS();
}

TEST test_union_config_json_guards(void) {

  FILE *tmp = tmpfile();

  struct StructFields sf;

  /* Specific config struct */
  struct CodegenTypesConfig config;

  char *content;

  long sz;

  ASSERT(tmp);

  struct_fields_init(&sf);

  struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL);

  memset(&config, 0, sizeof(config));

  config.json_guard = "UNION_GUARD";

  ASSERT_EQ(0, write_union_to_json_func(tmp, "U", &sf, &config));

  ASSERT_EQ(0, write_union_from_jsonObject_func(tmp, "U", &sf, &config));

  fseek(tmp, 0, SEEK_END);

  sz = ftell(tmp);

  rewind(tmp);

  content = calloc(1, sz + 1);

  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "#ifdef UNION_GUARD"));

  ASSERT(strstr(content, "#endif /* UNION_GUARD */"));

  free(content);

  struct_fields_free(&sf);

  fclose(tmp);

  PASS();
}

TEST test_codegen_config_utils_guards(void) {

  /*

   * Verify that struct helpers are wrapped in #ifdef DATA_UTILS ... #endif

   * when configured.

   */

  FILE *tmp = tmpfile();

  struct StructFields sf;

  /* Specific config struct */
  struct CodegenStructConfig config;

  char *content;

  long sz;

  ASSERT(tmp);

  struct_fields_init(&sf);

  struct_fields_add(&sf, "name", "string", NULL, NULL, NULL);

  memset(&config, 0, sizeof(config));

  config.guard_macro = "DATA_UTILS";

  /* Generate helpers */

  ASSERT_EQ(0, write_struct_cleanup_func(tmp, "S", &sf, &config));

  ASSERT_EQ(0, write_struct_debug_func(tmp, "S", &sf, &config));

  ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "S", &sf, &config));

  ASSERT_EQ(0, write_struct_default_func(tmp, "S", &sf, &config));

  ASSERT_EQ(0, write_struct_display_func(tmp, "S", &sf, &config));

  ASSERT_EQ(0, write_struct_eq_func(tmp, "S", &sf, &config));

  fseek(tmp, 0, SEEK_END);

  sz = ftell(tmp);

  rewind(tmp);

  content = calloc(1, sz + 1);

  fread(content, 1, sz, tmp);

  /* Just sample checks */

  ASSERT(strstr(content, "#ifdef DATA_UTILS"));

  ASSERT(strstr(content, "#endif /* DATA_UTILS */"));

  ASSERT(strstr(content, "void S_cleanup("));

  free(content);

  struct_fields_free(&sf);

  fclose(tmp);

  PASS();
}

SUITE(schema_codegen_suite) {

  RUN_TEST(test_schema_codegen_circular_refs);

  RUN_TEST(test_codegen_config_json_guards);

  RUN_TEST(test_union_config_json_guards);

  RUN_TEST(test_codegen_config_utils_guards);
}

#endif /* !TEST_SCHEMA_CODEGEN_H */
