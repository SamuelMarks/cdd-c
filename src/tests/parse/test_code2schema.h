/**
 * @file test_code2schema.h
 * @brief Unit tests for code to schema conversion.
 */

#ifndef TEST_CODE2SCHEMA_H
#define TEST_CODE2SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(_MSC_VER) && _MSC_VER < 1600
typedef signed __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef signed __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#if !defined(_MSC_VER) || _MSC_VER >= 1600
/* clang-format off */
#include "c_cdd/memory.h"
#include <stdint.h>
#else
#include "msvc/stdint.h"
#endif
#endif
#include "classes/emit/schema.h"
#include "c_cdd_export.h"
#include <string.h>
#include <greatest.h>
#include "functions/parse/fs.h"
#include "classes/parse/code2schema.h"
#include "functions/emit/codegen.h"
#include <cdd_test_helpers/cdd_helpers.h>
#include "c_cdd/test_allocator.h"
/* clang-format on */

static int g_malloc_fail_at = -1;
static int g_malloc_calls = 0;
static void *mock_malloc(size_t sz) {
  g_malloc_calls++;
  if (g_malloc_fail_at >= 0 && g_malloc_calls > g_malloc_fail_at) {
    return NULL;
  }
  return C_CDD_MALLOC(sz);
}
static void mock_free(void *ptr) { C_CDD_FREE(ptr); }

/* Updated test cases to reflect new return types (int vs void) */

TEST test_write_enum_functions(void) {

  struct EnumMembers em;
  FILE *tmp_fh;

  ASSERT_EQ(0, enum_members_init(&em));
  ASSERT_EQ(0, enum_members_add(&em, "FOO"));
  ASSERT_EQ(0, enum_members_add(&em, "BAR"));
  ASSERT_EQ(0, enum_members_add(&em, "UNKNOWN"));

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&tmp_fh, "tmp_enum_func.c", "w");
    if (err != 0 || tmp_fh == NULL)
      FAILm("Failed to open file for writing");
  }
#elif defined(_MSC_VER)
  fopen_s(&tmp_fh, "tmp_enum_func.c", "w");
  if (!tmp_fh)
    FAILm("Failed to open file for writing");
#else

  tmp_fh = fopen("tmp_enum_func.c", "w");
  if (!tmp_fh)

    FAILm("Failed to open file for writing");

#endif

  ASSERT_EQ(0, write_enum_to_str_func(tmp_fh, "MyEnum", &em, NULL));
  ASSERT_EQ(0, write_enum_from_str_func(tmp_fh, "MyEnum", &em, NULL));
  fclose(tmp_fh);
  remove("tmp_enum_func.c");

  enum_members_free(&em);

  PASS();
}

TEST test_struct_fields_manage(void) {

  struct StructFields sf;

  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, struct_fields_add(&sf, "name", "string", NULL, NULL, NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "num", "integer", NULL, NULL, NULL));
  struct_fields_free(&sf);

  PASS();
}

TEST test_str_starts_with(void) {
  int _ast_str_starts_with_0 = 0;
  int _ast_str_starts_with_1 = 0;
  ASSERT((str_starts_with("enum Color", "enum", &_ast_str_starts_with_0),

          _ast_str_starts_with_0));

  ASSERT(!(str_starts_with("structFoo", "enum", &_ast_str_starts_with_1),

           _ast_str_starts_with_1));

  PASS();
}

TEST test_parse_struct_member_line(void) {

  struct StructFields sf;

  struct_fields_init(&sf);

  /* 0 means success in new API */

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_struct_member_line(NULL, NULL));
  ASSERT_EQ(0, parse_struct_member_line("const char *foo;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("int bar;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("double x;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("bool b;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("enum Color *e;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("struct Point * p;", &sf));

  {
    int i;
    for (i = 0; i < 20; ++i) {
      struct StructFields tmp_sf;
      struct_fields_init(&tmp_sf);
      g_cdd_alloc_fail_countdown_countdown = i;
      g_cdd_strdup_fail = i;
      parse_struct_member_line("struct Point * p;", &tmp_sf);
      g_cdd_alloc_fail_countdown_countdown = 0;
      g_cdd_strdup_fail = 0;
      struct_fields_free(&tmp_sf);
    }
    for (i = 0; i < 20; ++i) {
      struct StructFields tmp_sf;
      struct_fields_init(&tmp_sf);
      g_malloc_calls = 0;
      g_malloc_fail_at = i;
      json_set_allocation_functions(mock_malloc, mock_free);
      g_cdd_alloc_fail_countdown_countdown = i;
      g_cdd_strdup_fail = i;
      parse_struct_member_line("int array[10]; /* @minimum(1) */", &tmp_sf);
      json_set_allocation_functions(C_CDD_MALLOC, C_CDD_FREE);
      g_cdd_alloc_fail_countdown_countdown = 0;
      g_cdd_strdup_fail = 0;
      struct_fields_free(&tmp_sf);
    }
  }

  struct_fields_free(&sf);

  PASS();
}

TEST test_parse_struct_member_bitfield(void) {

  struct StructFields sf;

  struct_fields_init(&sf);

  /* int x : 3; */

  ASSERT_EQ(0, parse_struct_member_line("int x : 3;", &sf));
  ASSERT_EQ(0, parse_struct_member_line("int*p;", &sf));
  ASSERT_EQ(2, sf.size);
  ASSERT_STR_EQ("x", sf.fields[0].name);
  ASSERT_STR_EQ("3", sf.fields[0].bit_width);
  ASSERT_STR_EQ("integer", sf.fields[0].type);

  /* Whitespace variation: int y:5; */

  ASSERT_EQ(0, parse_struct_member_line("int y:5;", &sf));
  ASSERT_STR_EQ("y", sf.fields[2].name);
  ASSERT_STR_EQ("5", sf.fields[2].bit_width);

  /* Type variation: unsigned int z : 1; */

  ASSERT_EQ(0, parse_struct_member_line("unsigned int z : 1;", &sf));
  ASSERT_STR_EQ("z", sf.fields[3].name);
  ASSERT_STR_EQ("1", sf.fields[3].bit_width);

  struct_fields_free(&sf);

  PASS();
}

TEST test_parse_struct_member_format_mapping(void) {

  struct StructFields sf;
  struct StructField *field;
  struct StructField *arr_field;

  struct_fields_init(&sf);

  ASSERT_EQ(0, parse_struct_member_line("long id;", &sf));
  ASSERT_EQ(1, sf.size);
  field = &sf.fields[0];
  ASSERT_STR_EQ("id", field->name);
  ASSERT_STR_EQ("integer", field->type);
  ASSERT_STR_EQ("int64", field->format);

  ASSERT_EQ(0, parse_struct_member_line("long ids[];", &sf));
  ASSERT_EQ(2, sf.size);
  arr_field = &sf.fields[1];
  ASSERT_STR_EQ("ids", arr_field->name);
  ASSERT_STR_EQ("array", arr_field->type);
  ASSERT(arr_field->items_extra_json != NULL);
  ASSERT(strstr(arr_field->items_extra_json, "\"format\":\"int64\"") != NULL);

  struct_fields_free(&sf);

  PASS();
}

static struct StructFields test_struct_fields;

TEST test_write_struct_functions(void) {
  FILE *tmpf = tmpfile();

  if (!tmpf)

    FAILm("Failed to open tmpfile");

  struct_fields_init(&test_struct_fields);

  ASSERT_EQ(0, struct_fields_add(&test_struct_fields, "str_field", "string",

                                 NULL, NULL, NULL));

  ASSERT_EQ(0, struct_fields_add(&test_struct_fields, "int_field", "integer",

                                 NULL, NULL, NULL));

  ASSERT_EQ(0, write_struct_to_json_func(tmpf, "TestStruct",

                                         &test_struct_fields, NULL));

  fflush(tmpf);
  ASSERT_GT(ftell(tmpf), 0);

  struct_fields_free(&test_struct_fields);
  fclose(tmpf);

  PASS();
}

TEST test_struct_fields_overflow(void) {

  struct StructFields sf;
  unsigned char i;
  enum { n = 32 };

  ASSERT_EQ(0, struct_fields_init(&sf));
  for (i = 0; i < 200; ++i) {

    char name[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(name, sizeof(name), "f%d", i);
#else

    sprintf(name, "f%d", i);

#endif

    ASSERT_EQ(0, struct_fields_add(&sf, name, "string", NULL, NULL, NULL));
  }

  ASSERT_GT(sf.size, n * 2);
  struct_fields_free(&sf);

  PASS();
}

TEST test_enum_members_overflow(void) {

  struct EnumMembers em;
  unsigned char i;
  enum { n = 32 };

  ASSERT_EQ(0, enum_members_init(&em));
  for (i = 0; i < 200; ++i) {

    char name[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(name, sizeof(name), "E%d", i);
#else

    sprintf(name, "E%d", i);

#endif

    ASSERT_EQ(0, enum_members_add(&em, name));
  }

  ASSERT_GT(em.size, n * 2);
  enum_members_free(&em);

  PASS();
}

TEST test_trim_trailing(void) {

  enum { n = 32 };
  char a[n];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s(a, n, "foo   \t;");
#elif defined(_MSC_VER)
  strcpy_s(a, sizeof(a), "foo   \t;");
#else

  strcpy(a, "foo   \t;");

#endif

  trim_trailing(a);
  ASSERT_STR_EQ("foo", a);

  PASS();
}

extern C_CDD_EXPORT int g_cdd_strdup_fail;

TEST test_code2schema_oom(void) {
  int i;
  char *argv[] = {"test_details_oom.h", "test_details_oom.json"};
  const char *header_content =
      "enum Color {RED,GREEN=5,BLUE,};\n"
      "struct Point {};\n"
      "struct Line { struct Point p1; };\n"
      "union MyUnion {\n"
      "  int x;\n"
      "  char *y;\n"
      "  float z;\n"
      "  struct Point p;\n"
      "};\n"
      "struct ComplexStruct {\n"
      "  int id; /* @minimum(1) @maximum(100) */\n"
      "  char *name; // @format(\"uuid\") @maxLength(255) @type(\"integer\")\n"
      "  double price; /* @multipleOf(0.01) */\n"
      "};\n"
      "struct ArrayStruct {\n"
      "  int ids[10]; /* @minItems(1) @maxItems(10) */\n"
      "};\n";

  ASSERT_EQ(0, write_to_file(argv[0], header_content));

  /* Parson allocation failures */
  for (i = 0; i < 1500; ++i) {
    g_malloc_calls = 0;
    g_malloc_fail_at = i;
    json_set_allocation_functions(mock_malloc, mock_free);

    code2schema_main(2, argv);

    json_set_allocation_functions(C_CDD_MALLOC, C_CDD_FREE);
    if (g_malloc_calls <= i)
      break;
  }

  /* C_CDD macro allocation failures */
  for (i = 0; i < 1500; ++i) {
    g_cdd_alloc_fail_countdown_countdown = i;
    code2schema_main(2, argv);
    if (g_cdd_alloc_fail_countdown_countdown != 0) {
      g_cdd_alloc_fail_countdown_countdown = 0;
      break; /* We have exhausted all allocations */
    }
  }

  /* c_cdd_strdup allocation failures */
  for (i = 0; i < 1500; ++i) {
    g_cdd_strdup_fail = i;
    code2schema_main(2, argv);
    if (g_cdd_strdup_fail != 0) {
      g_cdd_strdup_fail = 0;
      break; /* We have exhausted all strdup allocations */
    }
  }

  /* g_json_object_to_struct_fields_fail failures */
  for (i = 0; i < 1500; ++i) {
    extern C_CDD_EXPORT int g_json_object_to_struct_fields_fail;
    g_json_object_to_struct_fields_fail = i;
    code2schema_main(2, argv);
    if (g_json_object_to_struct_fields_fail != 0) {
      g_json_object_to_struct_fields_fail = 0;
      break;
    }
  }

  /* Specific function OOM tests */
  {
    char *s_src[] = {"foo", "bar"};
    char **s_copied = NULL;
    size_t s_count = 0;

    for (i = 0; i < 10; ++i) {
      g_cdd_alloc_fail_countdown_countdown = i;
      g_cdd_strdup_fail = i;
      if (copy_string_array_code2schema(&s_copied, &s_count, s_src, 2) ==
          CDD_C_ERROR_MEMORY) {
        /* Failed */
      } else if (s_copied) {
        free_string_array_code2schema(s_copied, s_count);
        s_copied = NULL;
      }
      if (g_cdd_alloc_fail_countdown_countdown != 0) {
        g_cdd_alloc_fail_countdown_countdown = 0;
        break;
      }
    }
  }

  {
    for (i = 0; i < 15; ++i) {
      JSON_Value *val = json_value_init_array();
      JSON_Array *arr = json_value_get_array(val);
      char **union_types = NULL;
      size_t count = 0;
      const char *primary = NULL;
      int nullable = 0;
      json_array_append_string(arr, "string");
      json_array_append_string(arr, "integer");

      g_cdd_alloc_fail_countdown_countdown = i;
      g_cdd_strdup_fail = i;
      if (parse_type_union_array_code2schema(arr, &union_types, &count,
                                             &primary, &nullable) == 0 &&
          union_types) {
        free_string_array_code2schema(union_types, count);
      }
      json_value_free(val);
      if (g_cdd_alloc_fail_countdown_countdown != 0) {
        g_cdd_alloc_fail_countdown_countdown = 0;
        break;
      }
    }
  }

  {
    for (i = 0; i < 15; ++i) {
      char *val_str = NULL;
      struct StructFields sf;
      struct_fields_init(&sf);
      struct_fields_add(&sf, "test", "int", NULL, NULL, NULL);

      g_cdd_alloc_fail_countdown_countdown = i;
      g_cdd_strdup_fail = i;
      if (make_unique_variant_name(&sf, "test", 0, &val_str) == 0 && val_str) {
        C_CDD_FREE(val_str);
      }
      struct_fields_free(&sf);
      if (g_cdd_alloc_fail_countdown_countdown != 0) {
        g_cdd_alloc_fail_countdown_countdown = 0;
        break;
      }
    }
  }

  {
    for (i = 0; i < 15; ++i) {
      char *val_str = NULL;
      g_cdd_alloc_fail_countdown_countdown = i;
      g_cdd_strdup_fail = i;
      if (make_inline_schema_name("Schema", "Var", "Suffix", &val_str) == 0 &&
          val_str) {
        C_CDD_FREE(val_str);
      }
      if (g_cdd_alloc_fail_countdown_countdown != 0) {
        g_cdd_alloc_fail_countdown_countdown = 0;
        break;
      }
    }
  }

  {
    for (i = 0; i < 15; ++i) {
      char *val_str = NULL;
      g_cdd_alloc_fail_countdown_countdown = i;
      g_cdd_strdup_fail = i;
      if (sanitize_identifier("123hello-world_test!", &val_str) == 0 &&
          val_str) {
        C_CDD_FREE(val_str);
      }
      if (g_cdd_alloc_fail_countdown_countdown != 0) {
        g_cdd_alloc_fail_countdown_countdown = 0;
        break;
      }
    }
  }

  {
    for (i = 0; i < 20; ++i) {
      JSON_Value *jv = json_parse_string("{}");
      JSON_Object *jo = json_value_get_object(jv);
      JSON_Value *schema_val = json_parse_string("{\"type\": \"string\"}");
      char *val2 = NULL;
      g_cdd_alloc_fail_countdown_countdown = i;
      g_cdd_strdup_fail = i;
      if (register_inline_schema_c2s(jo, "test", "var", "suf", schema_val,
                                     &val2) == 0 &&
          val2) {
        C_CDD_FREE(val2);
      }
      json_value_free(jv);
      json_value_free(schema_val);
      if (g_cdd_alloc_fail_countdown_countdown != 0) {
        g_cdd_alloc_fail_countdown_countdown = 0;
        break;
      }
    }
  }

  {
    const char *json_strs[] = {
        "{"
        "\"properties\": {"
        "\"id\": {\"type\": \"integer\", \"minimum\": 1, \"maximum\": 10, "
        "\"exclusiveMinimum\": true, \"exclusiveMaximum\": true},"
        "\"name\": {\"type\": \"string\", \"format\": \"uuid\", \"maxLength\": "
        "255},"
        "\"extra\": {\"type\": \"string\", \"x-custom\": 1},"
        "\"status\": {\"type\": \"string\", \"enum\": [\"active\", "
        "\"inactive\"]},"
        "\"combined\": {\"allOf\": [{\"type\": \"object\", \"properties\": "
        "{\"a\": {\"type\": \"integer\"}}}]},"
        "\"either\": {\"anyOf\": [{\"type\": \"string\"}, {\"type\": "
        "\"integer\"}]},"
        "\"single\": {\"oneOf\": [{\"type\": \"boolean\"}, {\"type\": "
        "\"number\"}]},"
        "\"items\": {\"type\": \"array\", \"items\": {\"type\": \"string\", "
        "\"maxLength\": 10, \"x-item\": 2}},"
        "\"obj_ref\": {\"$ref\": \"#/components/schemas/Point\"},"
        "\"arr_ref\": {\"type\": \"array\", \"items\": {\"$ref\": "
        "\"#/components/schemas/Point\"}},"
        "\"multi_type\": {\"type\": [\"string\", \"null\", \"integer\"]},"
        "\"multi_items\": {\"type\": \"array\", \"items\": {\"type\": "
        "[\"string\", \"null\"]}},"
        "\"null_type\": {\"type\": \"null\"},"
        "\"bool_type\": {\"type\": \"boolean\"},"
        "\"num_type\": {\"type\": \"number\"},"
        "\"obj_type\": {\"type\": \"object\", \"properties\": {}},"
        "\"flex_arr\": {\"type\": \"array\", \"items\": {}}"
        "}"
        "}",
        "{\"type\": \"string\", \"enum\": [\"A\", \"B\"]}",
        "{\"allOf\": [{\"type\": \"object\", \"properties\": {\"a\": "
        "{\"type\": \"integer\"}}}]}",
        "{\"anyOf\": [{\"type\": \"string\"}, {\"type\": \"integer\"}]}",
        "{\"oneOf\": [{\"type\": \"boolean\"}, {\"type\": \"number\"}]}"};
    int j;
    for (i = 0; i < 2500; ++i) {
      for (j = 0; j < 5; ++j) {
        JSON_Value *schema_val = json_parse_string(json_strs[j]);
        JSON_Object *schema_obj = json_value_get_object(schema_val);
        struct StructFields sf;

        struct_fields_init(&sf);
        g_malloc_calls = 0;
        g_malloc_fail_at = i;
        json_set_allocation_functions(mock_malloc, mock_free);
        json_object_to_struct_fields(schema_obj, &sf, NULL);
        json_set_allocation_functions(C_CDD_MALLOC, C_CDD_FREE);
        struct_fields_free(&sf);

        struct_fields_init(&sf);
        g_cdd_alloc_fail_countdown_countdown = i;
        json_object_to_struct_fields(schema_obj, &sf, NULL);
        struct_fields_free(&sf);
        g_cdd_alloc_fail_countdown_countdown = 0;

        struct_fields_init(&sf);
        g_cdd_strdup_fail = i;
        json_object_to_struct_fields(schema_obj, &sf, NULL);
        g_cdd_strdup_fail = 0;
        struct_fields_free(&sf);

        json_value_free(schema_val);
      }
    }
  }

  g_cdd_alloc_fail_countdown_countdown = 0;
  g_cdd_strdup_fail = 0;
  remove(argv[0]);
  remove(argv[1]);
  PASS();
}
TEST test_code2schema_branches(void) {
  /* test str_starts_with */
  str_starts_with("test", "te", NULL);
  str_starts_with("test", "te", NULL);

  /* test read_line with \r\n */
  {
    FILE *fp = fopen("dummy_c_code.c", "wb");
    if (fp) {
      fprintf(fp, "int main() {\r\n  return 0;\r\n}\r\n");
      fclose(fp);
      code2schema_main(2, (char *[]){"code2schema", "dummy_c_code.c"});
    }
  }
  PASS();
}
TEST test_code2schema_main_bad_args(void) {

  /* code2schema expects 2 args: in out */

  char *argv[] = {"bad"};

  /* Passing 1 args */

  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, code2schema_main(1, argv));

  PASS();
}

TEST test_union_parse(void) {
  char *argv[] = {"test_union.h", "test_union.json"};
  const char *header_content = "union MyUnion {\n"
                               "  int x;\n"
                               "};\n";
  write_to_file(argv[0], header_content);
  code2schema_main(2, argv);
  remove(argv[0]);
  remove(argv[1]);
  PASS();
}

TEST test_code2schema_parsing_details(void) {
  char *argv[] = {"test_details.h", "test_details.json"};
  const char *header_content = "enum Color {RED,GREEN=5,BLUE,};\n"

                               "struct Point {};\n"
                               "struct Line { struct Point p1; };\n"
                               "union MyUnion {\n"
                               "  int x;\n"
                               "  char *y;\n"
                               "  float z;\n"
                               "  struct Point p;\n"
                               "};\n";
  ASSERT_EQ(0, write_to_file(argv[0], header_content)); /* Write to in-file */

  ASSERT_EQ(CDD_C_SUCCESS,

            code2schema_main(2, argv)); /* Call with 2 args (in, out) */

  remove(argv[0]);
  remove(argv[1]);

  PASS();
}

TEST test_code2schema_parse_struct_and_enum(void) {
  char *argv[] = {"test1.h", "test1.schema.json"};
  const char *const filename = argv[0];
  char *json = argv[1];

  int out_val;

  char trim_buf[32] = "hello   ";
  char trim_buf2[32] = "hello ; ";
  int rc = write_to_file(filename,

                         "enum Colors { RED, GREEN = 5, BLUE };\n"
                         "struct Point { double x; double y; int used; };\n");

  ASSERT_EQ(0, rc);

  ASSERT_EQ(0, str_starts_with("hello", "hel", &out_val));
  ASSERT(out_val != 0);
  ASSERT_EQ(0, str_starts_with("hello", "helo", &out_val));
  ASSERT(out_val == 0);
  ASSERT_EQ(0, str_starts_with(NULL, "hel", &out_val));
  ASSERT(out_val == 0);
  ASSERT_EQ(0, str_starts_with("hello", NULL, &out_val));
  ASSERT(out_val == 0);

  trim_trailing(trim_buf);
  ASSERT_STR_EQ("hello", trim_buf);

  trim_trailing(trim_buf2);
  ASSERT_STR_EQ("hello", trim_buf2);

  trim_trailing(NULL);

  rc = code2schema_main(2, argv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  remove(filename);
  remove(json);

  PASS();
}

TEST test_code2schema_file_not_found(void) {
  char *argv[] = {"no_such_file.h", "out.json"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, code2schema_main(2, argv));

  PASS();
}

TEST test_codegen_enum_null_args(void) {
  FILE *tmp = tmpfile();

  struct EnumMembers em_valid;
  struct EnumMembers em_null_members;

  struct EnumMembers *em_null = NULL;

  ASSERT(tmp);
  memset(&em_null_members, 0, sizeof(em_null_members));

  enum_members_init(&em_valid);

  /* Check that the functions don't crash on NULL/invalid arguments and return
   * CDD_C_ERROR_INVALID_ARGUMENT */

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_enum_to_str_func(NULL, "E", &em_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_enum_to_str_func(tmp, NULL, &em_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_enum_to_str_func(tmp, "E", em_null, NULL));

  /* em_null_members.members is NULL so this triggers validation check */

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_enum_to_str_func(tmp, "E", &em_null_members, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_enum_from_str_func(NULL, "E", &em_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_enum_from_str_func(tmp, NULL, &em_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_enum_from_str_func(tmp, "E", em_null, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_enum_from_str_func(tmp, "E", &em_null_members, NULL));

  enum_members_free(&em_valid);
  fclose(tmp);

  PASS();
}

TEST test_codegen_enum_with_unknown(void) {
  FILE *tmp = tmpfile();

  struct EnumMembers em;

  ASSERT(tmp);
  ASSERT_EQ(0, enum_members_init(&em));
  ASSERT_EQ(0, enum_members_add(&em, "A"));
  ASSERT_EQ(0, enum_members_add(&em, "UNKNOWN"));
  ASSERT_EQ(0, enum_members_add(&em, "B"));

  /* This tests that the generator functions handle "UNKNOWN" correctly */

  ASSERT_EQ(0, write_enum_to_str_func(tmp, "MyEnum", &em, NULL));
  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  rewind(tmp);

  ASSERT_EQ(0, write_enum_from_str_func(tmp, "MyEnum", &em, NULL));
  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  enum_members_free(&em);
  fclose(tmp);

  PASS();
}

TEST test_codegen_all_field_types(void) {
  FILE *tmp = tmpfile();

  struct StructFields sf;

  ASSERT(tmp);
  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_string", "string", NULL, NULL, NULL));
  ASSERT_EQ(0,

            struct_fields_add(&sf, "f_integer", "integer", NULL, NULL, NULL));

  ASSERT_EQ(0,

            struct_fields_add(&sf, "f_boolean", "boolean", NULL, NULL, NULL));

  ASSERT_EQ(0, struct_fields_add(&sf, "f_number", "number", NULL, NULL, NULL));
  ASSERT_EQ(0, struct_fields_add(&sf, "f_enum", "enum", "MyEnum", NULL, NULL));
  ASSERT_EQ(

      0, struct_fields_add(&sf, "f_object", "object", "MyStruct", NULL, NULL));

  ASSERT_EQ(0, struct_fields_add(&sf, "f_unhandled", "unhandled_type", NULL,

                                 NULL, NULL));

  /* Call all generator functions with this comprehensive struct fields */

  ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "TestStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_to_json_func(tmp, "TestStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_eq_func(tmp, "TestStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_cleanup_func(tmp, "TestStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_default_func(tmp, "TestStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "TestStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_display_func(tmp, "TestStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_debug_func(tmp, "TestStruct", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  struct_fields_free(&sf);
  fclose(tmp);

  PASS();
}

TEST test_codegen_empty_struct_and_enum(void) {
  FILE *tmp = tmpfile();

  struct EnumMembers em;
  struct StructFields sf;

  ASSERT(tmp);
  ASSERT_EQ(0, enum_members_init(&em));
  ASSERT_EQ(0, struct_fields_init(&sf));

  ASSERT_EQ(0, write_enum_to_str_func(tmp, "EmptyEnum", &em, NULL));
  ASSERT_EQ(0, write_enum_from_str_func(tmp, "EmptyEnum", &em, NULL));

  ASSERT_EQ(0,

            write_struct_from_jsonObject_func(tmp, "EmptyStruct", &sf, NULL));

  ASSERT_EQ(0, write_struct_to_json_func(tmp, "EmptyStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_eq_func(tmp, "EmptyStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_cleanup_func(tmp, "EmptyStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_default_func(tmp, "EmptyStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_deepcopy_func(tmp, "EmptyStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_display_func(tmp, "EmptyStruct", &sf, NULL));
  ASSERT_EQ(0, write_struct_debug_func(tmp, "EmptyStruct", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  ASSERT_GT(ftell(tmp), 0L);

  enum_members_free(&em);
  struct_fields_free(&sf);
  fclose(tmp);

  PASS();
}

TEST test_codegen_struct_null_args(void) {
  FILE *tmp = tmpfile();

  struct StructFields sf_valid;

  struct StructFields *sf_null = NULL;

  ASSERT(tmp);
  struct_fields_init(&sf_valid);
  struct_fields_add(&sf_valid, "field", "string", NULL, NULL, NULL);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_from_jsonObject_func(NULL, "S", &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_from_jsonObject_func(tmp, NULL, &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_from_jsonObject_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_from_json_func(NULL, "S", NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_from_json_func(tmp, NULL, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_to_json_func(NULL, "S", &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_to_json_func(tmp, NULL, &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_to_json_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_eq_func(NULL, "S", &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_eq_func(tmp, NULL, &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_eq_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_cleanup_func(NULL, "S", &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_cleanup_func(tmp, NULL, &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_cleanup_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_default_func(NULL, "S", &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_default_func(tmp, NULL, &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_default_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_deepcopy_func(NULL, "S", &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_deepcopy_func(tmp, NULL, &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_deepcopy_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_display_func(NULL, "S", &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_display_func(tmp, NULL, &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_display_func(tmp, "S", sf_null, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_debug_func(NULL, "S", &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_debug_func(tmp, NULL, &sf_valid, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            write_struct_debug_func(tmp, "S", sf_null, NULL));

  struct_fields_free(&sf_valid);
  fclose(tmp);

  PASS();
}

TEST test_parse_struct_member_annotations(void) {

  struct StructFields sf;

  struct_fields_init(&sf);

  ASSERT_EQ(0, parse_struct_member_line(

                   "int user_id; // @shard_key @shard_hash", &sf));

  ASSERT_EQ(1, sf.size);
  ASSERT_STR_EQ("user_id", sf.fields[0].name);
  ASSERT_STR_EQ("integer", sf.fields[0].type);
  ASSERT(sf.fields[0].schema_extra_json != NULL);
  ASSERT(strstr(sf.fields[0].schema_extra_json, "\"x-cdd-shard-key\":true") !=

         NULL);

  ASSERT(strstr(sf.fields[0].schema_extra_json, "\"x-cdd-shard-hash\":true") !=

         NULL);

  ASSERT_EQ(

      0, parse_struct_member_line(
             "char *name; /* @track_telemetry @slow_query_warn(250) */", &sf));

  ASSERT_EQ(2, sf.size);
  ASSERT_STR_EQ("name", sf.fields[1].name);
  ASSERT(sf.fields[1].schema_extra_json != NULL);
  ASSERT(strstr(sf.fields[1].schema_extra_json,

                "\"x-cdd-track-telemetry\":true") != NULL);

  ASSERT(strstr(sf.fields[1].schema_extra_json, "\"x-cdd-slow-query\":250") !=

         NULL);

  struct_fields_free(&sf);

  PASS();
}

TEST test_code2schema_1793_oom(void) {
  struct StructFields sf;
  int j;
  struct_fields_init(&sf);
  for (j = 0; j < 8; j++) {
    struct_fields_add(&sf, "pad", "int", NULL, NULL, NULL);
  }
  g_struct_fields_add_fail = 1;
  parse_struct_member_line("struct Point * p;", &sf);
  g_struct_fields_add_fail = 0;

  sf.size = 0; /* so free doesn't crash on garbage */
  struct_fields_free(&sf);
  PASS();
}

TEST test_code2schema_missing_coverage(void) {
  struct StructFields fields, sf;
  JSON_Value *val_any, *val_one, *val_enum, *val_ref, *val;
  JSON_Object *obj_any, *obj_one, *obj_enum, *obj_ref, *root_obj, *obj;
  int i;
  char long_str[300];

  /* Fallback union OOM */
  val_any = json_parse_string("{\"anyOf\": [{\"description\": \"unknown\"}]}");
  obj_any = json_value_get_object(val_any);
  val_one = json_parse_string("{\"oneOf\": [{\"description\": \"unknown\"}]}");
  obj_one = json_value_get_object(val_one);

  for (i = 0; i < 50; ++i) {
    int j;
    struct_fields_init(&fields);
    for (j = 0; j < 8; j++)
      struct_fields_add(&fields, "pad", "int", NULL, NULL, NULL);
    g_cdd_alloc_fail_countdown_countdown = i;
    json_object_to_struct_fields_ex(obj_any, &fields, NULL, "TestSchema");
    struct_fields_free(&fields);
    if (g_cdd_alloc_fail_countdown_countdown != 0) {
      g_cdd_alloc_fail_countdown_countdown = 0;
      break;
    }
  }
  for (i = 0; i < 50; ++i) {
    int j;
    struct_fields_init(&fields);
    for (j = 0; j < 8; j++)
      struct_fields_add(&fields, "pad", "int", NULL, NULL, NULL);
    g_cdd_alloc_fail_countdown_countdown = i;
    json_object_to_struct_fields_ex(obj_one, &fields, NULL, "TestSchema");
    struct_fields_free(&fields);
    if (g_cdd_alloc_fail_countdown_countdown != 0) {
      g_cdd_alloc_fail_countdown_countdown = 0;
      break;
    }
  }

  /* Array of enum ref */
  val_enum =
      json_parse_string("{\"properties\": {\"arr\": {\"type\": \"array\", "
                        "\"items\": {\"$ref\": \"#/schemas/MyEnum\"}}}}");
  obj_enum = json_value_get_object(val_enum);
  val = json_parse_string(
      "{\"MyEnum\": {\"type\": \"string\", \"enum\": [\"A\"]}}");
  root_obj = json_value_get_object(val);

  struct_fields_init(&fields);
  json_object_to_struct_fields_ex(obj_enum, &fields, root_obj, "TestSchema");
  struct_fields_free(&fields);

  /* Ref fallback memory error */
  val_ref = json_parse_string(
      "{\"properties\": {\"obj\": {\"$ref\": \"#/schemas/Other\"}}}");
  obj_ref = json_value_get_object(val_ref);
  for (i = 0; i < 50; ++i) {
    int j;
    struct_fields_init(&fields);
    for (j = 0; j < 8; j++)
      struct_fields_add(&fields, "pad", "int", NULL, NULL, NULL);
    g_cdd_alloc_fail_countdown_countdown = i;
    json_object_to_struct_fields_ex(obj_ref, &fields, root_obj, "TestSchema");
    struct_fields_free(&fields);
    if (g_cdd_alloc_fail_countdown_countdown != 0) {
      g_cdd_alloc_fail_countdown_countdown = 0;
      break;
    }
  }

  /* merge_schema_extras conflict */
  struct_fields_init(&sf);
  struct_fields_add(&sf, "pad", "int", NULL, NULL, NULL);
  c_cdd_strdup("{\"type\": \"string\"}", &sf.schema_extra_json);
  obj =
      json_value_get_object(val_ref); /* reuse val_ref obj, it has properties */
  write_struct_to_json_schema(obj, "TestStruct", &sf);

  /* defaults */
  memset(long_str, 'A', sizeof(long_str));
  long_str[0] = '"';
  long_str[sizeof(long_str) - 2] = '"';
  long_str[sizeof(long_str) - 1] = '\0';
  struct_fields_add(&sf, "long_str", "string", NULL, long_str, NULL);
  struct_fields_add(&sf, "no_quotes", "string", NULL, "no_quotes", NULL);
  struct_fields_add(&sf, "bool_false", "boolean", NULL, "0", NULL);
  write_struct_to_json_schema(obj, "TestStruct2", &sf);

  struct_fields_free(&sf);
  json_value_free(val_any);
  json_value_free(val_one);
  json_value_free(val_enum);
  json_value_free(val_ref);
  json_value_free(val);
  PASS();
}

TEST test_code2schema_merge_struct_field_oom(void) {
  struct StructField dst, src;
  int i;
  for (i = 0; i < 50; ++i) {
    memset(&dst, 0, sizeof(dst));
    memset(&src, 0, sizeof(src));
    c_cdd_strdup("{\"a\": 1}", &dst.schema_extra_json);
    c_cdd_strdup("{\"b\": 2}", &src.schema_extra_json);
    g_cdd_strdup_fail = i;
    merge_struct_field(&dst, &src);
    if (g_cdd_strdup_fail != 0) {
      g_cdd_strdup_fail = 0;
      C_CDD_FREE(dst.schema_extra_json);
      C_CDD_FREE(src.schema_extra_json);
      break;
    }
    C_CDD_FREE(dst.schema_extra_json);
    C_CDD_FREE(src.schema_extra_json);
  }
  g_cdd_strdup_fail = 0;
  PASS();
}

TEST test_code2schema_json_array_to_enum_members(void) {
  struct EnumMembers em;
  JSON_Value *val;
  JSON_Array *arr;
  memset(&em, 0, sizeof(em));
  json_array_to_enum_members(NULL, &em);
  val = json_parse_string("[\"A\", 123, \"B\"]");
  arr = json_value_get_array(val);
  json_array_to_enum_members(arr, &em);
  enum_members_free(&em);
  json_value_free(val);
  PASS();
}

TEST test_code2schema_missing_details(void) {
  struct StructFields fields;
  JSON_Value *val_enum, *val_array_type_union, *val_ref, *val;
  JSON_Object *obj_enum, *obj_array_type_union, *obj_ref, *root_obj;
  int i;

  val_enum =
      json_parse_string("{\"properties\": {\"arr\": {\"type\": \"array\", "
                        "\"items\": {\"$ref\": \"#/schemas/MyEnum\"}}}}");
  obj_enum = json_value_get_object(val_enum);

  val_array_type_union =
      json_parse_string("{\"properties\": {\"arr\": {\"type\": [\"array\", "
                        "\"null\"], \"items\": {\"type\": \"string\"}}}}");
  obj_array_type_union = json_value_get_object(val_array_type_union);

  val_ref = json_parse_string(
      "{\"properties\": {\"obj\": {\"$ref\": \"#/schemas/Other\"}}}");
  obj_ref = json_value_get_object(val_ref);

  val = json_parse_string(
      "{\"MyEnum\": {\"type\": \"string\", \"enum\": [\"A\"]}}");
  root_obj = json_value_get_object(val);

  struct_fields_init(&fields);
  json_object_to_struct_fields_ex(obj_enum, &fields, root_obj, "TestSchema");
  struct_fields_free(&fields);

  for (i = 0; i < 50; ++i) {
    int j;
    struct_fields_init(&fields);
    for (j = 0; j < 8; j++)
      struct_fields_add(&fields, "pad", "int", NULL, NULL, NULL);
    g_cdd_alloc_fail_countdown_countdown = i;
    json_object_to_struct_fields_ex(obj_array_type_union, &fields, root_obj,
                                    "TestSchema");
    struct_fields_free(&fields);
    if (g_cdd_alloc_fail_countdown_countdown != 0) {
      g_cdd_alloc_fail_countdown_countdown = 0;
      break;
    }
  }

  for (i = 0; i < 50; ++i) {
    int j;
    struct_fields_init(&fields);
    for (j = 0; j < 8; j++)
      struct_fields_add(&fields, "pad", "int", NULL, NULL, NULL);
    g_cdd_alloc_fail_countdown_countdown = i;
    json_object_to_struct_fields_ex(obj_ref, &fields, root_obj, "TestSchema");
    struct_fields_free(&fields);
    if (g_cdd_alloc_fail_countdown_countdown != 0) {
      g_cdd_alloc_fail_countdown_countdown = 0;
      break;
    }
  }

  json_value_free(val_enum);
  json_value_free(val_array_type_union);
  json_value_free(val_ref);
  json_value_free(val);
  PASS();
}

TEST test_code2schema_2200_2216(void) {
  struct StructFields fields;
  JSON_Value *val_type_union, *val_ref_union, *val;
  JSON_Object *obj_type_union, *obj_ref_union, *root_obj;
  int i;

  val_type_union = json_parse_string(
      "{\"properties\": {\"arr\": {\"type\": [\"string\", \"null\"]}}}");
  obj_type_union = json_value_get_object(val_type_union);

  val_ref_union = json_parse_string(
      "{\"properties\": {\"obj\": {\"$ref\": \"#/schemas/Other\", \"type\": "
      "[\"object\", \"null\"]}}}");
  obj_ref_union = json_value_get_object(val_ref_union);

  val = json_parse_string("{}");
  root_obj = json_value_get_object(val);

  for (i = 0; i < 50; ++i) {
    int j;
    struct_fields_init(&fields);
    for (j = 0; j < 8; j++)
      struct_fields_add(&fields, "pad", "int", NULL, NULL, NULL);
    g_cdd_alloc_fail_countdown_countdown = i;
    json_object_to_struct_fields_ex(obj_type_union, &fields, root_obj,
                                    "TestSchema");
    struct_fields_free(&fields);
    if (g_cdd_alloc_fail_countdown_countdown != 0) {
      g_cdd_alloc_fail_countdown_countdown = 0;
      break;
    }
  }

  for (i = 0; i < 50; ++i) {
    int j;
    struct_fields_init(&fields);
    for (j = 0; j < 8; j++)
      struct_fields_add(&fields, "pad", "int", NULL, NULL, NULL);
    g_cdd_alloc_fail_countdown_countdown = i;
    json_object_to_struct_fields_ex(obj_ref_union, &fields, root_obj,
                                    "TestSchema");
    struct_fields_free(&fields);
    if (g_cdd_alloc_fail_countdown_countdown != 0) {
      g_cdd_alloc_fail_countdown_countdown = 0;
      break;
    }
  }

  json_value_free(val_type_union);
  json_value_free(val_ref_union);
  json_value_free(val);
  PASS();
}

TEST test_code2schema_merge_struct_field(void) {

  struct StructField f1, f2, f3, f4;

  memset(&f1, 0, sizeof(f1));
  memset(&f2, 0, sizeof(f2));

  merge_struct_field(NULL, &f2);
  merge_struct_field(&f1, NULL);

  f2.has_min = 1;
  f2.min_val = 10;
  f2.has_max = 1;
  f2.max_val = 20;
  f2.has_min_len = 1;
  f2.min_len = 5;
  f2.has_max_len = 1;
  f2.max_len = 15;
  f2.has_min_items = 1;
  f2.min_items = 2;
  f2.has_max_items = 1;
  f2.max_items = 8;
  f2.required = 1;
  strncpy(f2.default_val, "test", sizeof(f2.default_val) - 1);
  strncpy(f2.format, "uuid", sizeof(f2.format) - 1);
  strncpy(f2.pattern, "^[a-z]+$", sizeof(f2.pattern) - 1);
  strncpy(f2.bit_width, "16", sizeof(f2.bit_width) - 1);

  merge_struct_field(&f1, &f2);

  ASSERT_EQ(1, f1.has_min);
  ASSERT_EQ(10, f1.min_val);
  ASSERT_EQ(1, f1.has_max);
  ASSERT_EQ(20, f1.max_val);
  ASSERT_EQ(1, f1.has_min_len);
  ASSERT_EQ(5, f1.min_len);
  ASSERT_EQ(1, f1.has_max_len);
  ASSERT_EQ(15, f1.max_len);
  ASSERT_EQ(1, f1.has_min_items);
  ASSERT_EQ(2, f1.min_items);
  ASSERT_EQ(1, f1.has_max_items);
  ASSERT_EQ(8, f1.max_items);
  ASSERT_EQ(1, f1.required);
  ASSERT_STR_EQ("test", f1.default_val);
  ASSERT_STR_EQ("16", f1.bit_width);

  f2.min_val = 15;
  f2.max_val = 15;
  f2.min_len = 10;
  f2.max_len = 10;
  f2.min_items = 5;
  f2.max_items = 5;

  merge_struct_field(&f1, &f2);

  {
    int i;
    for (i = 0; i < 200; ++i) {
      struct StructField src, dst;
      char *types[] = {"int", "float"};
      memset(&src, 0, sizeof(src));
      memset(&dst, 0, sizeof(dst));
      src.n_type_union = 2;
      src.type_union = types;
      src.n_items_type_union = 2;
      src.items_type_union = types;

      c_cdd_strdup("{\"a\": 1}", &src.items_extra_json);
      c_cdd_strdup("{\"b\": 2}", &dst.items_extra_json);

      g_malloc_calls = 0;
      g_malloc_fail_at = i;
      json_set_allocation_functions(mock_malloc, mock_free);
      g_cdd_alloc_fail_countdown_countdown = 0;
      g_cdd_strdup_fail = 0;
      merge_struct_field(&dst, &src);
      json_set_allocation_functions(C_CDD_MALLOC, C_CDD_FREE);

      if (dst.type_union)
        free_string_array_code2schema(dst.type_union, dst.n_type_union);
      if (dst.items_type_union)
        free_string_array_code2schema(dst.items_type_union,
                                      dst.n_items_type_union);
      C_CDD_FREE(src.items_extra_json);
      C_CDD_FREE(dst.items_extra_json);
    }
    {
      struct StructField src, dst;
      memset(&src, 0, sizeof(src));
      memset(&dst, 0, sizeof(dst));
      c_cdd_strdup("\"not_an_object\"", &src.items_extra_json);
      c_cdd_strdup("{\"b\": 2}", &dst.items_extra_json);
      merge_struct_field(&dst, &src);
      C_CDD_FREE(src.items_extra_json);
      C_CDD_FREE(dst.items_extra_json);
      c_cdd_strdup("{\"b\": 1}", &src.items_extra_json);
      c_cdd_strdup("{\"b\": 2}", &dst.items_extra_json);
      merge_struct_field(&dst, &src);
      C_CDD_FREE(src.items_extra_json);
      C_CDD_FREE(dst.items_extra_json);
    }
    for (i = 0; i < 200; ++i) {
      struct StructField src, dst;
      char *types[] = {"int", "float"};
      memset(&src, 0, sizeof(src));
      memset(&dst, 0, sizeof(dst));
      src.n_type_union = 2;
      src.type_union = types;
      src.n_items_type_union = 2;
      src.items_type_union = types;

      c_cdd_strdup("{\"a\": 1}", &src.items_extra_json);
      c_cdd_strdup("{\"b\": 2}", &dst.items_extra_json);

      g_cdd_alloc_fail_countdown_countdown = i;
      g_cdd_strdup_fail = i;
      merge_struct_field(&dst, &src);
      g_cdd_alloc_fail_countdown_countdown = 0;
      g_cdd_strdup_fail = 0;

      if (dst.type_union)
        free_string_array_code2schema(dst.type_union, dst.n_type_union);
      if (dst.items_type_union)
        free_string_array_code2schema(dst.items_type_union,
                                      dst.n_items_type_union);
      C_CDD_FREE(src.items_extra_json);
      C_CDD_FREE(dst.items_extra_json);
    }
  }

  ASSERT_EQ(15, f1.min_val);
  ASSERT_EQ(15, f1.max_val);
  ASSERT_EQ(10, f1.min_len);
  ASSERT_EQ(10, f1.max_len);
  ASSERT_EQ(5, f1.min_items);
  ASSERT_EQ(5, f1.max_items);

  /* Test malformed JSON for merge_schema_extras_strings */
  memset(&f3, 0, sizeof(f3));
  memset(&f4, 0, sizeof(f4));
  f3.schema_extra_json = "{bad_json}";
  f4.schema_extra_json = "{\"good\": 1}";
  merge_struct_field(&f3, &f4); /* dest malformed */

  f3.schema_extra_json = "{\"good\": 1}";
  f4.schema_extra_json = "{bad_json}";
  merge_struct_field(&f3, &f4); /* src malformed */

  f3.schema_extra_json = "123"; /* valid json but not object */
  f4.schema_extra_json = "456"; /* valid json but not object */
  merge_struct_field(&f3, &f4); /* not object */

  PASS();
}

TEST test_code2schema_discriminator_value(void) {
  char *val = NULL;

  JSON_Value *jv;
  JSON_Object *jo;
  JSON_Value *jv2;
  JSON_Object *jo2;

  /* NULLs */

  ASSERT_EQ(0, discriminator_value_for_variant(NULL, NULL, NULL, &val));
  ASSERT(val == NULL);

  jv = json_parse_string(

      "{\"mapping\": {\"test\": \"#/components/schemas/MyRef\", \"test2\": "
      "\"MyRef2\"}}");

  jo = json_value_get_object(jv);

  ASSERT_EQ(0, discriminator_value_for_variant(

                   jo, NULL, "#/components/schemas/MyRef", &val));

  ASSERT_STR_EQ("test", val);
  C_CDD_FREE(val);
  val = NULL;

  ASSERT_EQ(0, discriminator_value_for_variant(jo, "MyRef2", NULL, &val));
  ASSERT_STR_EQ("test2", val);
  C_CDD_FREE(val);
  val = NULL;

  ASSERT_EQ(0, discriminator_value_for_variant(jo, "MyRef3", NULL, &val));
  ASSERT_STR_EQ("MyRef3", val);
  C_CDD_FREE(val);
  val = NULL;

  /* What about when mapping doesn't exist but disc_obj does */

  jv2 = json_parse_string("{}");
  jo2 = json_value_get_object(jv2);
  ASSERT_EQ(0, discriminator_value_for_variant(jo2, "MyRef2", NULL, &val));
  ASSERT_STR_EQ("MyRef2", val);
  C_CDD_FREE(val);
  val = NULL;
  json_value_free(jv2);

  json_value_free(jv);

  PASS();
}

TEST test_code2schema_sanitize_identifier(void) {
  char *val = NULL;

  /* NULL or empty */

  ASSERT_EQ(0, sanitize_identifier(NULL, &val));
  ASSERT_STR_EQ("Variant", val);
  C_CDD_FREE(val);
  val = NULL;

  ASSERT_EQ(0, sanitize_identifier("", &val));
  ASSERT_STR_EQ("Variant", val);
  C_CDD_FREE(val);
  val = NULL;

  /* Invalid chars */

  ASSERT_EQ(0, sanitize_identifier("123hello-world_test!", &val));

  /* ASSERT_STR_EQ("_123hello_world_test_", val); */ /* 1 becomes _ because of
                                                        first char rule maybe */

  C_CDD_FREE(val);
  val = NULL;

  PASS();
}

TEST test_code2schema_make_unique_variant_name(void) {
  char *val = NULL;

  struct StructFields sf;

  /* NULLs */

  ASSERT_EQ(0, make_unique_variant_name(NULL, "test", 0, &val));
  ASSERT(val == NULL);

  memset(&sf, 0, sizeof(sf));
  struct_fields_init(&sf);
  ASSERT_EQ(0, make_unique_variant_name(&sf, NULL, 0, &val));
  ASSERT(val != NULL);
  C_CDD_FREE(val);
  struct_fields_free(&sf);

  struct_fields_init(&sf);
  struct_fields_add(&sf, "test", "int", NULL, NULL, NULL);

  ASSERT_EQ(0, make_unique_variant_name(&sf, "test", 0, &val));
  ASSERT_STR_EQ("test_1", val);
  C_CDD_FREE(val);
  val = NULL;

  struct_fields_add(&sf, "test_1", "int", NULL, NULL, NULL);
  ASSERT_EQ(0, make_unique_variant_name(&sf, "test", 0, &val));
  ASSERT_STR_EQ("Variant_1", val);
  C_CDD_FREE(val);
  val = NULL;

  /* sanitize fails due to null inside base but we can't really fail it without
   * CDD_C_ERROR_MEMORY or returning NULL */

  ASSERT_EQ(0, make_unique_variant_name(&sf, NULL, 0, &val));
  ASSERT_STR_EQ("Variant", val);
  C_CDD_FREE(val);
  val = NULL;

  struct_fields_free(&sf);

  PASS();
}

TEST test_code2schema_make_inline_schema_name(void) {
  char *val = NULL;

  /* NULLs */

  ASSERT_EQ(0, make_inline_schema_name(NULL, NULL, NULL, &val));
  ASSERT_STR_EQ("Union_Variant", val);
  C_CDD_FREE(val);
  val = NULL;

  ASSERT_EQ(0, make_inline_schema_name("Schema", "Var", "Suffix", &val));
  ASSERT_STR_EQ("Schema_Var_Suffix", val);
  C_CDD_FREE(val);
  val = NULL;

  /* Just a few variations */

  ASSERT_EQ(0, make_inline_schema_name(NULL, "Var", NULL, &val));
  ASSERT_STR_EQ("Union_Var", val);
  C_CDD_FREE(val);
  val = NULL;

  PASS();
}

TEST test_code2schema_register_inline_schema_c2s(void) {
  char *val = NULL;

  /* NULLs */

  JSON_Value *jv = json_parse_string("{}");
  JSON_Object *jo = json_value_get_object(jv);
  JSON_Value *schema_val = json_parse_string("{\"type\": \"string\"}");
  char *val2 = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            register_inline_schema_c2s(NULL, NULL, NULL, NULL, NULL, &val));

  ASSERT_EQ(0, register_inline_schema_c2s(jo, "test", "var", "suf", schema_val,

                                          &val));

  ASSERT_STR_EQ("test_var_suf", val);

  /* Already exists */

  ASSERT_EQ(0, register_inline_schema_c2s(jo, "test", "var", "suf", schema_val,

                                          &val2));

  ASSERT_STR_EQ("test_var_suf", val2);

  C_CDD_FREE(val);
  C_CDD_FREE(val2);
  json_value_free(jv);
  json_value_free(schema_val);

  PASS();
}

TEST test_code2schema_utils(void) {

  char **s_arr;
  char **s_src;

  char **s_copied = NULL;
  size_t s_count = 0;

  JSON_Value *val;
  JSON_Array *arr;

  char **union_types = NULL;
  size_t count = 0;
  const char *primary = NULL;
  int nullable = 0;

  ASSERT_EQ(0,

            parse_type_union_array_code2schema(NULL, NULL, NULL, NULL, NULL));

  free_string_array_code2schema(NULL, 0);
  s_arr = (char **)C_CDD_MALLOC(sizeof(char *) * 2);
  s_arr[0] = strdup("test");
  s_arr[1] = strdup("test2");
  free_string_array_code2schema(s_arr, 2);

  s_src = (char **)C_CDD_MALLOC(sizeof(char *) * 2);
  s_src[0] = strdup("foo");
  s_src[1] = strdup("bar");

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_string_array_code2schema(NULL, NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            copy_string_array_code2schema(&s_copied, &s_count, NULL, 0));

  ASSERT_EQ(0, copy_string_array_code2schema(&s_copied, &s_count, s_src, 2));
  ASSERT(s_copied != NULL);
  ASSERT_EQ(2, s_count);
  free_string_array_code2schema(s_copied, 2);
  free_string_array_code2schema(s_src, 2);

  val = json_value_init_array();
  arr = json_value_get_array(val);

  ASSERT_EQ(0, parse_type_union_array_code2schema(arr, &union_types, &count,

                                                  &primary, &nullable));

  json_array_append_null(arr);
  ASSERT_EQ(0, parse_type_union_array_code2schema(arr, &union_types, &count,

                                                  &primary, &nullable));

  json_array_append_string(arr, "null");
  ASSERT_EQ(0, parse_type_union_array_code2schema(arr, &union_types, &count,

                                                  &primary, &nullable));

  ASSERT_STR_EQ("null", primary);

  json_value_free(val);
  if (union_types)
    free_string_array_code2schema(union_types, count);

  PASS();
}

SUITE(code2schema_suite) {
  RUN_TEST(test_code2schema_utils);
  RUN_TEST(test_write_enum_functions);
  RUN_TEST(test_struct_fields_manage);
  RUN_TEST(test_str_starts_with);
  RUN_TEST(test_parse_struct_member_line);
  RUN_TEST(test_parse_struct_member_bitfield);
  RUN_TEST(test_parse_struct_member_format_mapping);
  RUN_TEST(test_write_struct_functions);
  RUN_TEST(test_struct_fields_overflow);
  RUN_TEST(test_enum_members_overflow);
  RUN_TEST(test_trim_trailing);
  RUN_TEST(test_code2schema_main_bad_args);
  RUN_TEST(test_code2schema_1793_oom);
  RUN_TEST(test_code2schema_missing_coverage);

  RUN_TEST(test_code2schema_merge_struct_field_oom);
  RUN_TEST(test_code2schema_json_array_to_enum_members);
  RUN_TEST(test_code2schema_missing_details);
  RUN_TEST(test_code2schema_2200_2216);
  RUN_TEST(test_code2schema_merge_struct_field);
  RUN_TEST(test_code2schema_sanitize_identifier);
  RUN_TEST(test_code2schema_make_unique_variant_name);
  RUN_TEST(test_code2schema_make_inline_schema_name);
  RUN_TEST(test_code2schema_register_inline_schema_c2s);
  RUN_TEST(test_code2schema_discriminator_value);

  RUN_TEST(test_code2schema_file_not_found);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  /* TODO: Fix file locking tests for MSVC */
#else

  RUN_TEST(test_code2schema_parsing_details);
  RUN_TEST(test_union_parse);
  RUN_TEST(test_code2schema_parse_struct_and_enum);

#endif

  RUN_TEST(test_codegen_enum_null_args);
  RUN_TEST(test_codegen_enum_with_unknown);
  RUN_TEST(test_codegen_all_field_types);
  RUN_TEST(test_codegen_empty_struct_and_enum);
  RUN_TEST(test_codegen_struct_null_args);
  RUN_TEST(test_parse_struct_member_annotations);
  RUN_TEST(test_code2schema_oom);
  RUN_TEST(test_code2schema_branches);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CODE2SCHEMA_H */
