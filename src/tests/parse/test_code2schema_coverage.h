#ifndef TEST_CODE2SCHEMA_COVERAGE_H
#define TEST_CODE2SCHEMA_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "c_cdd/memory.h"
#include "classes/parse/code2schema.h"
#include "functions/parse/str.h"
#include <greatest.h>
#include <parson.h>
/* clang-format on */

static int g_parson_fail_at = -1;
static void *mock_parson_malloc(size_t sz) {
  if (g_parson_fail_at == 0) {
    g_parson_fail_at = -1;
    return NULL;
  }
  if (g_parson_fail_at > 0)
    g_parson_fail_at--;
  return malloc(sz);
}
static void mock_parson_free(void *ptr) { free(ptr); }

TEST test_code2schema_coverage_oom(void) {
  int i;
  JSON_Value *val;
  JSON_Object *obj;

  for (i = 1; i < 50; ++i) {
    struct StructFields sf;

    val = json_parse_string("{\"type\":\"object\",\"properties\":{\"name\":{"
                            "\"type\":\"string\",\"x-extra\":true}}}");
    obj = json_value_get_object(val);

    struct_fields_init(&sf);

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;

    json_object_to_struct_fields(obj, &sf, NULL);

    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    struct_fields_free(&sf);
    json_value_free(val);
  }
  PASS();
}

TEST test_code2schema_write_oom(void) {
  int i;
  for (i = 1; i < 50; ++i) {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(root);
    char *out_s = NULL;

    struct_fields_init(&sf);
    c_cdd_strdup("{\"x-cdd-extra\": true}", &out_s);
    sf.schema_extra_json = out_s;
    sf.is_enum = 1;
    sf.enum_members.members = (char **)C_CDD_MALLOC(sizeof(char *) * 2);
    sf.enum_members.size = 2;
    sf.enum_members.capacity = 2;
    c_cdd_strdup("A", &out_s);
    sf.enum_members.members[0] = out_s;
    sf.enum_members.members[1] = NULL;

    json_set_allocation_functions(mock_parson_malloc, mock_parson_free);
    g_parson_fail_at = i;

    write_struct_to_json_schema(schemas_obj, "TestStruct", &sf);

    json_set_allocation_functions(malloc, free);
    g_parson_fail_at = -1;

    json_value_free(root);
    C_CDD_FREE(sf.enum_members.members[0]);
    C_CDD_FREE(sf.enum_members.members);
    C_CDD_FREE(sf.schema_extra_json);
  }
  PASS();
}

SUITE(code2schema_coverage_suite) {
  RUN_TEST(test_code2schema_coverage_oom);
  RUN_TEST(test_code2schema_write_oom);
}

#ifdef __cplusplus
}
#endif
#endif
