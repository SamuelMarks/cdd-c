/**
 * @file test_schema_constraints.h
 * @brief Unit tests for OpenAPI schema constraint enforcement.
 */

#ifndef TEST_SCHEMA_CONSTRAINTS_H
#define TEST_SCHEMA_CONSTRAINTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <parson.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/emit/struct.h"
#include "classes/parse/code2schema.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_schema_strdup_fail;
extern C_CDD_EXPORT int g_cdd_strdup_fail;

/**
 * @brief Unit test test_schema_constraints_roundtrip.
 * @return Greatest test result.
 */
TEST test_schema_constraints_roundtrip(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct StructField *_ast_struct_fields_get_0 = NULL;
  struct StructField *_ast_struct_fields_get_1 = NULL;
  struct StructField *_ast_struct_fields_get_2 = NULL;
  struct StructField *_ast_struct_fields_get_3 = NULL;
  struct StructField *_ast_struct_fields_get_4 = NULL;
  struct StructFields sf;
  const char schema[] = {
      123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116,
      34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,
      123, 34,  115, 99,  111, 114, 101, 34,  58,  123, 34,  116, 121, 112, 101,
      34,  58,  34,  110, 117, 109, 98,  101, 114, 34,  44,  34,  101, 120, 99,
      108, 117, 115, 105, 118, 101, 77,  105, 110, 105, 109, 117, 109, 34,  58,
      51,  46,  53,  44,  34,  101, 120, 99,  108, 117, 115, 105, 118, 101, 77,
      97,  120, 105, 109, 117, 109, 34,  58,  57,  46,  50,  53,  44,  34,  100,
      101, 102, 97,  117, 108, 116, 34,  58,  48,  46,  53,  125, 44,  34,  97,
      103, 101, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  105, 110,
      116, 101, 103, 101, 114, 34,  44,  34,  109, 105, 110, 105, 109, 117, 109,
      34,  58,  49,  56,  44,  34,  101, 120, 99,  108, 117, 115, 105, 118, 101,
      77,  97,  120, 105, 109, 117, 109, 34,  58,  54,  53,  44,  34,  100, 101,
      102, 97,  117, 108, 116, 34,  58,  50,  49,  125, 44,  34,  110, 97,  109,
      101, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114,
      105, 110, 103, 34,  44,  34,  109, 105, 110, 76,  101, 110, 103, 116, 104,
      34,  58,  50,  44,  34,  109, 97,  120, 76,  101, 110, 103, 116, 104, 34,
      58,  49,  48,  44,  34,  112, 97,  116, 116, 101, 114, 110, 34,  58,  34,
      94,  91,  65,  45,  90,  93,  43,  36,  34,  44,  34,  100, 101, 102, 97,
      117, 108, 116, 34,  58,  34,  66,  111, 98,  34,  125, 44,  34,  116, 97,
      103, 115, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  97,  114,
      114, 97,  121, 34,  44,  34,  105, 116, 101, 109, 115, 34,  58,  123, 34,
      116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125,
      44,  34,  109, 105, 110, 73,  116, 101, 109, 115, 34,  58,  49,  44,  34,
      109, 97,  120, 73,  116, 101, 109, 115, 34,  58,  51,  44,  34,  117, 110,
      105, 113, 117, 101, 73,  116, 101, 109, 115, 34,  58,  116, 114, 117, 101,
      125, 44,  34,  101, 110, 97,  98,  108, 101, 100, 34,  58,  123, 34,  116,
      121, 112, 101, 34,  58,  34,  98,  111, 111, 108, 101, 97,  110, 34,  44,
      34,  100, 101, 102, 97,  117, 108, 116, 34,  58,  116, 114, 117, 101, 125,
      125, 125, 0};

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(schema, &_cdd_jv_1);
    val = _cdd_jv_1;
  }
  ASSERT(val != NULL);
  obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    struct StructField *score;
    score = (g_cdd_last_alloc_err =
                 struct_fields_get(&sf, "score", &_ast_struct_fields_get_0),
             _ast_struct_fields_get_0);
    ASSERT(score != NULL);
    ASSERT_EQ(1, score->has_min);
    ASSERT_EQ(1, score->exclusive_min);
    ASSERT(score->min_val > 3.4 && score->min_val < 3.6);
    ASSERT_EQ(1, score->has_max);
    ASSERT_EQ(1, score->exclusive_max);
    ASSERT(score->max_val > 9.2 && score->max_val < 9.3);
  }

  {
    struct StructField *age;
    age = (g_cdd_last_alloc_err =
               struct_fields_get(&sf, "age", &_ast_struct_fields_get_1),
           _ast_struct_fields_get_1);
    ASSERT(age != NULL);
    ASSERT_EQ(1, age->has_min);
    ASSERT_EQ(0, age->exclusive_min);
    ASSERT_EQ(1, age->has_max);
    ASSERT_EQ(1, age->exclusive_max);
    ASSERT_EQ(18, (int)age->min_val);
    ASSERT_EQ(65, (int)age->max_val);
  }

  {
    struct StructField *name;
    name = (g_cdd_last_alloc_err =
                struct_fields_get(&sf, "name", &_ast_struct_fields_get_2),
            _ast_struct_fields_get_2);
    ASSERT(name != NULL);
    ASSERT_EQ(1, name->has_min_len);
    ASSERT_EQ(1, name->has_max_len);
    ASSERT_EQ((size_t)2, name->min_len);
    ASSERT_EQ((size_t)10, name->max_len);
    ASSERT_STR_EQ("^[A-Z]+$", name->pattern);
    ASSERT_STR_EQ("\"Bob\"", name->default_val);
  }

  {
    struct StructField *enabled;
    enabled = (g_cdd_last_alloc_err =
                   struct_fields_get(&sf, "enabled", &_ast_struct_fields_get_3),
               _ast_struct_fields_get_3);
    ASSERT(enabled != NULL);
    ASSERT_STR_EQ("1", enabled->default_val);
  }

  {
    struct StructField *tags;
    tags = (g_cdd_last_alloc_err =
                struct_fields_get(&sf, "tags", &_ast_struct_fields_get_4),
            _ast_struct_fields_get_4);
    ASSERT(tags != NULL);
    ASSERT_EQ(1, tags->has_min_items);
    ASSERT_EQ(1, tags->has_max_items);
    ASSERT_EQ(1, tags->unique_items);
    ASSERT_EQ((size_t)1, tags->min_items);
    ASSERT_EQ((size_t)3, tags->max_items);
  }

  {
    JSON_Object *age_prop;
    JSON_Object *name_prop;
    JSON_Object *enabled_prop;
    JSON_Object *tags_prop;
    JSON_Value *schemas_val;
    JSON_Object *schemas_obj;
    JSON_Object *test_obj;
    JSON_Object *props;
    JSON_Object *score_prop;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      schemas_val = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Test", &sf));

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(schemas_obj, "Test", &_cdd_jo_1);
      test_obj = _cdd_jo_1;
    }
    ASSERT(test_obj != NULL);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(test_obj, "properties", &_cdd_jo_1);
      props = _cdd_jo_1;
    }
    ASSERT(props != NULL);

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(props, "score", &_cdd_jo_1);
      score_prop = _cdd_jo_1;
    }
    ASSERT(score_prop != NULL);
    ASSERT(json_object_has_value(score_prop, "exclusiveMinimum"));
    ASSERT(json_object_has_value(score_prop, "exclusiveMaximum"));
    {
      double ex_min;
      double ex_max;
      {
        double _cdd_jn_1 = 0.0;
        g_cdd_last_alloc_err = cdd_json_object_get_number(
            score_prop, "exclusiveMinimum", &_cdd_jn_1);
        ex_min = _cdd_jn_1;
      }
      {
        double _cdd_jn_1 = 0.0;
        g_cdd_last_alloc_err = cdd_json_object_get_number(
            score_prop, "exclusiveMaximum", &_cdd_jn_1);
        ex_max = _cdd_jn_1;
      }
      ASSERT(ex_min > 3.4 && ex_min < 3.6);
      ASSERT(ex_max > 9.2 && ex_max < 9.3);
    }
    {
      double def_num;
      {
        double _cdd_jn_1 = 0.0;
        g_cdd_last_alloc_err =
            cdd_json_object_get_number(score_prop, "default", &_cdd_jn_1);
        def_num = _cdd_jn_1;
      }
      ASSERT(def_num > 0.49 && def_num < 0.51);
    }

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(props, "age", &_cdd_jo_1);
      age_prop = _cdd_jo_1;
    }
    ASSERT(age_prop != NULL);
    {
      double _cdd_jn_1 = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_number(age_prop, "minimum", &_cdd_jn_1);
      ASSERT_EQ(18, (int)_cdd_jn_1);
    }
    {
      double _cdd_jn_1 = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_number(age_prop, "exclusiveMaximum", &_cdd_jn_1);
      ASSERT_EQ(65, (int)_cdd_jn_1);
    }
    {
      double _cdd_jn_1 = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_number(age_prop, "default", &_cdd_jn_1);
      ASSERT_EQ(21, (int)_cdd_jn_1);
    }

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(props, "name", &_cdd_jo_1);
      name_prop = _cdd_jo_1;
    }
    ASSERT(name_prop != NULL);
    {
      double _cdd_jn_1 = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_number(name_prop, "minLength", &_cdd_jn_1);
      ASSERT_EQ(2, (int)_cdd_jn_1);
    }
    {
      double _cdd_jn_1 = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_number(name_prop, "maxLength", &_cdd_jn_1);
      ASSERT_EQ(10, (int)_cdd_jn_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(name_prop, "pattern", &_cdd_js_1);
      ASSERT_STR_EQ("^[A-Z]+$", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_string(name_prop, "default", &_cdd_js_1);
      ASSERT_STR_EQ("Bob", _cdd_js_1);
    }

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(props, "enabled", &_cdd_jo_1);
      enabled_prop = _cdd_jo_1;
    }
    ASSERT(enabled_prop != NULL);
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(enabled_prop, "default", &_cdd_jb_1) !=
          CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(1, _cdd_jb_1);
    }

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(props, "tags", &_cdd_jo_1);
      tags_prop = _cdd_jo_1;
    }
    ASSERT(tags_prop != NULL);
    {
      double _cdd_jn_1 = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_number(tags_prop, "minItems", &_cdd_jn_1);
      ASSERT_EQ(1, (int)_cdd_jn_1);
    }
    {
      double _cdd_jn_1 = 0.0;
      g_cdd_last_alloc_err =
          cdd_json_object_get_number(tags_prop, "maxItems", &_cdd_jn_1);
      ASSERT_EQ(3, (int)_cdd_jn_1);
    }
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(tags_prop, "uniqueItems", &_cdd_jb_1) !=
          CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(1, _cdd_jb_1);
    }

    g_cdd_last_alloc_err = json_value_free(schemas_val);
  }

  g_cdd_last_alloc_err = struct_fields_free(&sf);
  g_cdd_last_alloc_err = json_value_free(val);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_schema_annotations_roundtrip.
 * @return Greatest test result.
 */
TEST test_schema_annotations_roundtrip(void) {
  struct StructFields sf;
  JSON_Value *val;
  JSON_Object *obj;
  struct StructField *_ast_struct_fields_get_5 = NULL;
  struct StructField *_ast_struct_fields_get_6 = NULL;
  struct StructField *_ast_struct_fields_get_7 = NULL;
  const char schema[] = {
      123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116,
      34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,
      123, 34,  105, 100, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,
      115, 116, 114, 105, 110, 103, 34,  44,  34,  102, 111, 114, 109, 97,  116,
      34,  58,  34,  117, 117, 105, 100, 34,  44,  34,  100, 101, 115, 99,  114,
      105, 112, 116, 105, 111, 110, 34,  58,  34,  85,  115, 101, 114, 32,  73,
      68,  34,  44,  34,  100, 101, 112, 114, 101, 99,  97,  116, 101, 100, 34,
      58,  102, 97,  108, 115, 101, 125, 44,  34,  115, 101, 99,  114, 101, 116,
      34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105,
      110, 103, 34,  44,  34,  119, 114, 105, 116, 101, 79,  110, 108, 121, 34,
      58,  116, 114, 117, 101, 44,  34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  83,  101, 99,  114, 101, 116, 34,  125, 44,
      34,  114, 101, 97,  100, 109, 101, 34,  58,  123, 34,  116, 121, 112, 101,
      34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  44,  34,  114, 101, 97,
      100, 79,  110, 108, 121, 34,  58,  116, 114, 117, 101, 44,  34,  100, 101,
      112, 114, 101, 99,  97,  116, 101, 100, 34,  58,  116, 114, 117, 101, 125,
      125, 125, 0};

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(schema, &_cdd_jv_1);
    val = _cdd_jv_1;
  }
  ASSERT(val != NULL);
  obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    struct StructField *id_field;
    id_field = (g_cdd_last_alloc_err =
                    struct_fields_get(&sf, "id", &_ast_struct_fields_get_5),
                _ast_struct_fields_get_5);
    ASSERT(id_field != NULL);
    ASSERT_STR_EQ("uuid", id_field->format);
    ASSERT_STR_EQ("User ID", id_field->description);
    ASSERT_EQ(1, id_field->deprecated_set);
    ASSERT_EQ(0, id_field->deprecated);
  }

  {
    struct StructField *secret_field;
    secret_field = (g_cdd_last_alloc_err = struct_fields_get(
                        &sf, "secret", &_ast_struct_fields_get_6),
                    _ast_struct_fields_get_6);
    ASSERT(secret_field != NULL);
    ASSERT_STR_EQ("Secret", secret_field->description);
    ASSERT_EQ(1, secret_field->write_only_set);
    ASSERT_EQ(1, secret_field->write_only);
  }

  {
    struct StructField *readme_field;
    readme_field = (g_cdd_last_alloc_err = struct_fields_get(
                        &sf, "readme", &_ast_struct_fields_get_7),
                    _ast_struct_fields_get_7);
    ASSERT(readme_field != NULL);
    ASSERT_EQ(1, readme_field->read_only_set);
    ASSERT_EQ(1, readme_field->read_only);
    ASSERT_EQ(1, readme_field->deprecated_set);
    ASSERT_EQ(1, readme_field->deprecated);
  }

  {
    JSON_Value *schemas_val;
    JSON_Object *schemas_obj;
    JSON_Object *annot;
    JSON_Object *props;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      schemas_val = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Annotated", &sf));

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(schemas_obj, "Annotated", &_cdd_jo_1);
      annot = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(annot, "properties", &_cdd_jo_1);
      props = _cdd_jo_1;
    }
    ASSERT(props != NULL);

    {
      JSON_Object *id_prop;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(props, "id", &_cdd_jo_1);
        id_prop = _cdd_jo_1;
      }
      ASSERT(id_prop != NULL);
      {
        const char *_cdd_js_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(id_prop, "format", &_cdd_js_1);
        ASSERT_STR_EQ("uuid", _cdd_js_1);
      }
      {
        const char *_cdd_js_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(id_prop, "description", &_cdd_js_1);
        ASSERT_STR_EQ("User ID", _cdd_js_1);
      }
      ASSERT(json_object_has_value(id_prop, "deprecated"));
      {
        int _cdd_jb_1 = -1;
        if (cdd_json_object_get_boolean(id_prop, "deprecated", &_cdd_jb_1) !=
            CDD_C_SUCCESS)
          _cdd_jb_1 = -1;
        ASSERT_EQ(0, _cdd_jb_1);
      }
    }

    {
      JSON_Object *secret_prop;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(props, "secret", &_cdd_jo_1);
        secret_prop = _cdd_jo_1;
      }
      ASSERT(secret_prop != NULL);
      {
        const char *_cdd_js_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(secret_prop, "description", &_cdd_js_1);
        ASSERT_STR_EQ("Secret", _cdd_js_1);
      }
      {
        int _cdd_jb_1 = -1;
        if (cdd_json_object_get_boolean(secret_prop, "writeOnly", &_cdd_jb_1) !=
            CDD_C_SUCCESS)
          _cdd_jb_1 = -1;
        ASSERT_EQ(1, _cdd_jb_1);
      }
    }

    {
      JSON_Object *readme_prop;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(props, "readme", &_cdd_jo_1);
        readme_prop = _cdd_jo_1;
      }
      ASSERT(readme_prop != NULL);
      {
        int _cdd_jb_1 = -1;
        if (cdd_json_object_get_boolean(readme_prop, "readOnly", &_cdd_jb_1) !=
            CDD_C_SUCCESS)
          _cdd_jb_1 = -1;
        ASSERT_EQ(1, _cdd_jb_1);
      }
      {
        int _cdd_jb_1 = -1;
        if (cdd_json_object_get_boolean(readme_prop, "deprecated",
                                        &_cdd_jb_1) != CDD_C_SUCCESS)
          _cdd_jb_1 = -1;
        ASSERT_EQ(1, _cdd_jb_1);
      }
    }

    g_cdd_last_alloc_err = json_value_free(schemas_val);
  }

  g_cdd_last_alloc_err = struct_fields_free(&sf);
  g_cdd_last_alloc_err = json_value_free(val);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_schema_allof_merge.
 * @return Greatest test result.
 */
TEST test_schema_allof_merge(void) {
  JSON_Value *root_val;
  JSON_Value *schema_val;
  struct StructField *_ast_struct_fields_get_8 = NULL;
  struct StructField *_ast_struct_fields_get_9 = NULL;
  struct StructField *_ast_struct_fields_get_10 = NULL;
  const char *schemas = "{"
                        "\"Base\":{\"type\":\"object\",\"properties\":{"
                        "\"name\":{\"type\":\"string\",\"minLength\":2}"
                        "},\"required\":[\"name\"]}"
                        "}";

  const char schema[] = {
      123, 34,  97,  108, 108, 79,  102, 34,  58,  91,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  111, 98,  106, 101, 99,  116, 34,  44,  34,  112, 114,
      111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,  105, 100, 34,
      58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  105, 110, 116, 101, 103,
      101, 114, 34,  44,  34,  109, 105, 110, 105, 109, 117, 109, 34,  58,  49,
      125, 125, 44,  34,  114, 101, 113, 117, 105, 114, 101, 100, 34,  58,  91,
      34,  105, 100, 34,  93,  125, 44,  123, 34,  36,  114, 101, 102, 34,  58,
      34,  35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  115,
      99,  104, 101, 109, 97,  115, 47,  66,  97,  115, 101, 34,  125, 93,  44,
      34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,
      101, 120, 116, 114, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,
      34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 0};

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(schemas, &_cdd_jv_1);
    root_val = _cdd_jv_1;
  }
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(schema, &_cdd_jv_1);
    schema_val = _cdd_jv_1;
  }
  ASSERT(root_val != NULL);
  ASSERT(schema_val != NULL);

  {
    struct StructFields sf;
    JSON_Object *root_obj;
    JSON_Object *schema_obj;
    root_obj = json_value_get_object(root_val);
    schema_obj = json_value_get_object(schema_val);
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, root_obj));

    {
      struct StructField *id_field;
      id_field = (g_cdd_last_alloc_err =
                      struct_fields_get(&sf, "id", &_ast_struct_fields_get_8),
                  _ast_struct_fields_get_8);
      ASSERT(id_field != NULL);
      ASSERT_STR_EQ("integer", id_field->type);
      ASSERT_EQ(1, id_field->required);
      ASSERT_EQ(1, id_field->has_min);
      ASSERT_EQ(1, (int)id_field->min_val);
    }

    {
      struct StructField *name_field;
      name_field = (g_cdd_last_alloc_err = struct_fields_get(
                        &sf, "name", &_ast_struct_fields_get_9),
                    _ast_struct_fields_get_9);
      ASSERT(name_field != NULL);
      ASSERT_STR_EQ("string", name_field->type);
      ASSERT_EQ(1, name_field->required);
      ASSERT_EQ(1, name_field->has_min_len);
      ASSERT_EQ((size_t)2, name_field->min_len);
    }

    {
      struct StructField *extra_field;
      extra_field = (g_cdd_last_alloc_err = struct_fields_get(
                         &sf, "extra", &_ast_struct_fields_get_10),
                     _ast_struct_fields_get_10);
      ASSERT(extra_field != NULL);
      ASSERT_STR_EQ("string", extra_field->type);
      ASSERT_EQ(0, extra_field->required);
    }

    g_cdd_last_alloc_err = struct_fields_free(&sf);
  }

  g_cdd_last_alloc_err = json_value_free(schema_val);
  g_cdd_last_alloc_err = json_value_free(root_val);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_schema_anyof_first_object.
 * @return Greatest test result.
 */
TEST test_schema_anyof_first_object(void) {
  JSON_Value *schema_val;
  struct StructField *_ast_struct_fields_get_11 = NULL;
  struct StructField *_ast_struct_fields_get_12 = NULL;
  const char *schema =
      "{"
      "\"anyOf\":["
      "{\"type\":\"object\",\"properties\":{\"first\":{\"type\":\"string\"}}},"
      "{\"type\":\"object\",\"properties\":{\"second\":{\"type\":\"integer\"}}}"
      "]"
      "}";

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(schema, &_cdd_jv_1);
    schema_val = _cdd_jv_1;
  }
  ASSERT(schema_val != NULL);

  {
    struct StructFields sf;
    JSON_Object *schema_obj;
    schema_obj = json_value_get_object(schema_val);
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, NULL));

    ASSERT((g_cdd_last_alloc_err =
                struct_fields_get(&sf, "first", &_ast_struct_fields_get_11),
            _ast_struct_fields_get_11) != NULL);
    ASSERT((g_cdd_last_alloc_err =
                struct_fields_get(&sf, "second", &_ast_struct_fields_get_12),
            _ast_struct_fields_get_12) == NULL);

    g_cdd_last_alloc_err = struct_fields_free(&sf);
  }

  g_cdd_last_alloc_err = json_value_free(schema_val);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_schema_oneof_first_object.
 * @return Greatest test result.
 */
TEST test_schema_oneof_first_object(void) {
  JSON_Value *schema_val;
  struct StructField *_ast_struct_fields_get_13 = NULL;
  struct StructField *_ast_struct_fields_get_14 = NULL;
  const char *schema =
      "{"
      "\"oneOf\":["
      "{\"type\":\"object\",\"properties\":{\"alpha\":{\"type\":\"string\"}}},"
      "{\"type\":\"object\",\"properties\":{\"beta\":{\"type\":\"integer\"}}}"
      "]"
      "}";

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(schema, &_cdd_jv_1);
    schema_val = _cdd_jv_1;
  }
  ASSERT(schema_val != NULL);

  {
    struct StructFields sf;
    JSON_Object *schema_obj;
    schema_obj = json_value_get_object(schema_val);
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, NULL));

    ASSERT((g_cdd_last_alloc_err =
                struct_fields_get(&sf, "alpha", &_ast_struct_fields_get_13),
            _ast_struct_fields_get_13) != NULL);
    ASSERT((g_cdd_last_alloc_err =
                struct_fields_get(&sf, "beta", &_ast_struct_fields_get_14),
            _ast_struct_fields_get_14) == NULL);

    g_cdd_last_alloc_err = struct_fields_free(&sf);
  }

  g_cdd_last_alloc_err = json_value_free(schema_val);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_schema_keyword_passthrough.
 * @return Greatest test result.
 */
TEST test_schema_keyword_passthrough(void) {
  struct StructFields sf;
  JSON_Value *val;
  JSON_Object *obj;
  const char schema[] = {
      123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116,
      34,  44,  34,  97,  100, 100, 105, 116, 105, 111, 110, 97,  108, 80,  114,
      111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  102, 97,  108, 115, 101,
      44,  34,  112, 97,  116, 116, 101, 114, 110, 80,  114, 111, 112, 101, 114,
      116, 105, 101, 115, 34,  58,  123, 34,  94,  120, 45,  34,  58,  123, 34,
      116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125,
      125, 44,  34,  36,  100, 101, 102, 115, 34,  58,  123, 34,  69,  120, 116,
      114, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116,
      114, 105, 110, 103, 34,  125, 125, 44,  34,  110, 111, 116, 34,  58,  123,
      34,  114, 101, 113, 117, 105, 114, 101, 100, 34,  58,  91,  34,  98,  108,
      111, 99,  107, 101, 100, 34,  93,  125, 44,  34,  105, 102, 34,  58,  123,
      34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,
      107, 105, 110, 100, 34,  58,  123, 34,  99,  111, 110, 115, 116, 34,  58,
      34,  65,  34,  125, 125, 125, 44,  34,  116, 104, 101, 110, 34,  58,  123,
      34,  114, 101, 113, 117, 105, 114, 101, 100, 34,  58,  91,  34,  110, 97,
      109, 101, 34,  93,  125, 44,  34,  101, 108, 115, 101, 34,  58,  123, 34,
      114, 101, 113, 117, 105, 114, 101, 100, 34,  58,  91,  34,  105, 100, 34,
      93,  125, 44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,
      58,  123, 34,  110, 97,  109, 101, 34,  58,  123, 34,  116, 121, 112, 101,
      34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  44,  34,  101, 110, 117,
      109, 34,  58,  91,  34,  65,  34,  44,  34,  66,  34,  93,  44,  34,  116,
      105, 116, 108, 101, 34,  58,  34,  78,  97,  109, 101, 34,  125, 44,  34,
      109, 101, 116, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,
      111, 98,  106, 101, 99,  116, 34,  44,  34,  97,  100, 100, 105, 116, 105,
      111, 110, 97,  108, 80,  114, 111, 112, 101, 114, 116, 105, 101, 115, 34,
      58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110,
      103, 34,  125, 125, 44,  34,  116, 97,  103, 115, 34,  58,  123, 34,  116,
      121, 112, 101, 34,  58,  34,  97,  114, 114, 97,  121, 34,  44,  34,  105,
      116, 101, 109, 115, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,
      115, 116, 114, 105, 110, 103, 34,  44,  34,  112, 97,  116, 116, 101, 114,
      110, 34,  58,  34,  94,  91,  97,  45,  122, 93,  43,  36,  34,  125, 125,
      125, 125, 0};

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(schema, &_cdd_jv_1);
    val = _cdd_jv_1;
  }
  ASSERT(val != NULL);
  obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    JSON_Value *schemas_val;
    JSON_Object *schemas_obj;
    JSON_Object *spec;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      schemas_val = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Spec", &sf));

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(schemas_obj, "Spec", &_cdd_jo_1);
      spec = _cdd_jo_1;
    }
    ASSERT(spec != NULL);
    ASSERT(json_object_has_value(spec, "additionalProperties"));
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(spec, "additionalProperties",
                                      &_cdd_jb_1) != CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(0, _cdd_jb_1);
    }

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(spec, "patternProperties", &_cdd_jo_1);
    }

    {
      JSON_Object *defs;
      JSON_Object *extra;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(spec, "$defs", &_cdd_jo_1);
        defs = _cdd_jo_1;
      }
      ASSERT(defs != NULL);
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(defs, "Extra", &_cdd_jo_1);
        extra = _cdd_jo_1;
      }
      ASSERT(extra != NULL);
      {
        const char *_cdd_js_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(extra, "type", &_cdd_js_1);
        ASSERT_STR_EQ("string", _cdd_js_1);
      }
    }

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(spec, "not", &_cdd_jo_1);
      ASSERT(_cdd_jo_1 != NULL);
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_object_get_object(spec, "if", &_cdd_jo_1);
      ASSERT(_cdd_jo_1 != NULL);
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(spec, "then", &_cdd_jo_1);
      ASSERT(_cdd_jo_1 != NULL);
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(spec, "else", &_cdd_jo_1);
      ASSERT(_cdd_jo_1 != NULL);
    }

    {
      JSON_Object *props;
      JSON_Object *name_prop;
      JSON_Object *meta_prop;
      JSON_Object *tags_prop;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(spec, "properties", &_cdd_jo_1);
        props = _cdd_jo_1;
      }
      ASSERT(props != NULL);

      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(props, "name", &_cdd_jo_1);
        name_prop = _cdd_jo_1;
      }
      ASSERT(name_prop != NULL);
      {
        JSON_Array *enum_arr;
        {
          JSON_Array *_cdd_ja_1 = NULL;
          g_cdd_last_alloc_err =
              cdd_json_object_get_array(name_prop, "enum", &_cdd_ja_1);
          enum_arr = _cdd_ja_1;
        }
        ASSERT(enum_arr != NULL);
        ASSERT_EQ(2, (int)json_array_get_count(enum_arr));
        ASSERT_STR_EQ("A", json_array_get_string(enum_arr, 0));
        ASSERT_STR_EQ("B", json_array_get_string(enum_arr, 1));
      }
      {
        const char *_cdd_js_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_string(name_prop, "title", &_cdd_js_1);
        ASSERT_STR_EQ("Name", _cdd_js_1);
      }

      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(props, "meta", &_cdd_jo_1);
        meta_prop = _cdd_jo_1;
      }
      ASSERT(meta_prop != NULL);
      {
        JSON_Object *meta_additional;
        {
          JSON_Object *_cdd_jo_1 = NULL;
          g_cdd_last_alloc_err = cdd_json_object_get_object(
              meta_prop, "additionalProperties", &_cdd_jo_1);
          meta_additional = _cdd_jo_1;
        }
        ASSERT(meta_additional != NULL);
        {
          const char *_cdd_js_1 = NULL;
          g_cdd_last_alloc_err =
              cdd_json_object_get_string(meta_additional, "type", &_cdd_js_1);
          ASSERT_STR_EQ("string", _cdd_js_1);
        }
      }

      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(props, "tags", &_cdd_jo_1);
        tags_prop = _cdd_jo_1;
      }
      ASSERT(tags_prop != NULL);
      {
        JSON_Object *items;
        {
          JSON_Object *_cdd_jo_1 = NULL;
          g_cdd_last_alloc_err =
              cdd_json_object_get_object(tags_prop, "items", &_cdd_jo_1);
          items = _cdd_jo_1;
        }
        ASSERT(items != NULL);
        {
          const char *_cdd_js_1 = NULL;
          g_cdd_last_alloc_err =
              cdd_json_object_get_string(items, "pattern", &_cdd_js_1);
          ASSERT_STR_EQ("^[a-z]+$", _cdd_js_1);
        }
      }
    }

    g_cdd_last_alloc_err = json_value_free(schemas_val);
  }

  g_cdd_last_alloc_err = struct_fields_free(&sf);
  g_cdd_last_alloc_err = json_value_free(val);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_schema_allof_keyword_merge.
 * @return Greatest test result.
 */
TEST test_schema_allof_keyword_merge(void) {
  JSON_Value *schema_val;
  const char schema[] = {
      123, 34,  97,  108, 108, 79,  102, 34,  58,  91,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  111, 98,  106, 101, 99,  116, 34,  44,  34,  97,  100,
      100, 105, 116, 105, 111, 110, 97,  108, 80,  114, 111, 112, 101, 114, 116,
      105, 101, 115, 34,  58,  102, 97,  108, 115, 101, 44,  34,  112, 114, 111,
      112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,  105, 100, 34,  58,
      123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103,
      34,  44,  34,  120, 45,  97,  108, 112, 104, 97,  34,  58,  116, 114, 117,
      101, 125, 125, 125, 44,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111,
      98,  106, 101, 99,  116, 34,  44,  34,  112, 97,  116, 116, 101, 114, 110,
      80,  114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,  94,
      120, 45,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116,
      114, 105, 110, 103, 34,  125, 125, 44,  34,  112, 114, 111, 112, 101, 114,
      116, 105, 101, 115, 34,  58,  123, 34,  105, 100, 34,  58,  123, 34,  116,
      121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  44,  34,
      120, 45,  98,  101, 116, 97,  34,  58,  49,  125, 125, 125, 93,  125, 0};

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(schema, &_cdd_jv_1);
    schema_val = _cdd_jv_1;
  }
  ASSERT(schema_val != NULL);

  {
    struct StructFields sf;
    JSON_Object *schema_obj;
    JSON_Value *schemas_val;
    JSON_Object *schemas_obj;
    schema_obj = json_value_get_object(schema_val);
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, NULL));

    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      schemas_val = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Merged", &sf));

    {
      JSON_Object *merged;

      JSON_Object *props;
      JSON_Object *id_prop;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(schemas_obj, "Merged", &_cdd_jo_1);
        merged = _cdd_jo_1;
      }
      ASSERT(merged != NULL);

      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(merged, "patternProperties", &_cdd_jo_1);
      }

      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(merged, "properties", &_cdd_jo_1);
        props = _cdd_jo_1;
      }
      ASSERT(props != NULL);
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(props, "id", &_cdd_jo_1);
        id_prop = _cdd_jo_1;
      }
      ASSERT(id_prop != NULL);
      /* ASSERT(json_object_has_value(id_prop, "x-alpha"));
      ASSERT_EQ(1, json_object_get_boolean(id_prop, "x-alpha"));
      ASSERT(json_object_has_value(id_prop, "x-beta"));
      ASSERT_EQ(1, (int)json_object_get_number(id_prop, "x-beta")); */
    }

    g_cdd_last_alloc_err = json_value_free(schemas_val);
    g_cdd_last_alloc_err = struct_fields_free(&sf);
  }

  g_cdd_last_alloc_err = json_value_free(schema_val);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_schema_type_union_roundtrip.
 * @return Greatest test result.
 */
TEST test_schema_type_union_roundtrip(void) {
  struct StructFields sf;
  JSON_Value *val;
  JSON_Object *obj;
  struct StructField *_ast_struct_fields_get_15 = NULL;
  struct StructField *_ast_struct_fields_get_16 = NULL;
  struct StructField *_ast_struct_fields_get_17 = NULL;
  const char *schema =
      "{"
      "\"type\":\"object\","
      "\"properties\":{"
      "\"name\":{\"type\":[\"string\",\"null\"]},"
      "\"age\":{\"type\":[\"integer\",\"null\"]},"
      "\"tags\":{\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]}}"
      "}"
      "}";

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(schema, &_cdd_jv_1);
    val = _cdd_jv_1;
  }
  ASSERT(val != NULL);
  obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    struct StructField *name;
    name = (g_cdd_last_alloc_err =
                struct_fields_get(&sf, "name", &_ast_struct_fields_get_15),
            _ast_struct_fields_get_15);
    ASSERT(name != NULL);
    ASSERT_EQ(2, (int)name->n_type_union);
    ASSERT_STR_EQ("string", name->type_union[0]);
    ASSERT_STR_EQ("null", name->type_union[1]);
    ASSERT_STR_EQ("string", name->type);
  }

  {
    struct StructField *age;
    age = (g_cdd_last_alloc_err =
               struct_fields_get(&sf, "age", &_ast_struct_fields_get_16),
           _ast_struct_fields_get_16);
    ASSERT(age != NULL);
    ASSERT_EQ(2, (int)age->n_type_union);
    ASSERT_STR_EQ("integer", age->type_union[0]);
    ASSERT_STR_EQ("null", age->type_union[1]);
    ASSERT_STR_EQ("integer", age->type);
  }

  {
    struct StructField *tags;
    tags = (g_cdd_last_alloc_err =
                struct_fields_get(&sf, "tags", &_ast_struct_fields_get_17),
            _ast_struct_fields_get_17);
    ASSERT(tags != NULL);
    ASSERT_EQ(2, (int)tags->n_items_type_union);
    ASSERT_STR_EQ("string", tags->items_type_union[0]);
    ASSERT_STR_EQ("null", tags->items_type_union[1]);
  }

  {
    JSON_Value *schemas_val;
    JSON_Object *schemas_obj;
    JSON_Object *union_obj;
    JSON_Object *props;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
      schemas_val = _cdd_jv_1;
    }
    schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Union", &sf));

    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(schemas_obj, "Union", &_cdd_jo_1);
      union_obj = _cdd_jo_1;
    }
    ASSERT(union_obj != NULL);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_object_get_object(union_obj, "properties", &_cdd_jo_1);
      props = _cdd_jo_1;
    }
    ASSERT(props != NULL);

    {
      JSON_Object *name_prop;
      JSON_Array *type_arr;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(props, "name", &_cdd_jo_1);
        name_prop = _cdd_jo_1;
      }
      ASSERT(name_prop != NULL);
      {
        JSON_Array *_cdd_ja_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_array(name_prop, "type", &_cdd_ja_1);
        type_arr = _cdd_ja_1;
      }
      ASSERT(type_arr != NULL);
      ASSERT_EQ(2, (int)json_array_get_count(type_arr));
      ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
      ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    }

    {
      JSON_Object *age_prop;
      JSON_Array *type_arr;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(props, "age", &_cdd_jo_1);
        age_prop = _cdd_jo_1;
      }
      ASSERT(age_prop != NULL);
      {
        JSON_Array *_cdd_ja_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_array(age_prop, "type", &_cdd_ja_1);
        type_arr = _cdd_ja_1;
      }
      ASSERT(type_arr != NULL);
      ASSERT_EQ(2, (int)json_array_get_count(type_arr));
      ASSERT_STR_EQ("integer", json_array_get_string(type_arr, 0));
      ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    }

    {
      JSON_Object *tags_prop;
      JSON_Object *items;
      JSON_Array *type_arr;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(props, "tags", &_cdd_jo_1);
        tags_prop = _cdd_jo_1;
      }
      ASSERT(tags_prop != NULL);
      {
        JSON_Object *_cdd_jo_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_object(tags_prop, "items", &_cdd_jo_1);
        items = _cdd_jo_1;
      }
      ASSERT(items != NULL);
      {
        JSON_Array *_cdd_ja_1 = NULL;
        g_cdd_last_alloc_err =
            cdd_json_object_get_array(items, "type", &_cdd_ja_1);
        type_arr = _cdd_ja_1;
      }
      ASSERT(type_arr != NULL);
      ASSERT_EQ(2, (int)json_array_get_count(type_arr));
      ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
      ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    }

    g_cdd_last_alloc_err = json_value_free(schemas_val);
  }

  g_cdd_last_alloc_err = struct_fields_free(&sf);
  g_cdd_last_alloc_err = json_value_free(val);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_schema_constraints_free_branch.
 * @return Greatest test result.
 */
TEST test_schema_constraints_free_branch(void) {
  struct SchemaConstraints sc;

  /* NULL checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, schema_constraints_init(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            schema_constraints_add_required(NULL, "a"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            schema_constraints_add_required(&sc, NULL));
  g_cdd_last_alloc_err = schema_constraints_free(NULL);

  g_cdd_last_alloc_err = schema_constraints_init(&sc);

  /* Add required so it cleans it up */
  g_cdd_last_alloc_err = schema_constraints_add_required(&sc, "req1");

  /* Mock additional properties */
  sc.additional_properties = calloc(1, sizeof(*sc.additional_properties));
  sc.additional_properties->name = malloc(4);
#if defined(_MSC_VER)
  strcpy_s(sc.additional_properties->name, 4, "foo");
#else
  strcpy(sc.additional_properties->name, "foo");
#endif
  sc.additional_properties->type = malloc(4);
#if defined(_MSC_VER)
  strcpy_s(sc.additional_properties->type, 4, "bar");
#else
  strcpy(sc.additional_properties->type, "bar");
#endif
  sc.additional_properties->ref = malloc(4);
#if defined(_MSC_VER)
  strcpy_s(sc.additional_properties->ref, 4, "baz");
#else
  strcpy(sc.additional_properties->ref, "baz");
#endif

  g_cdd_last_alloc_err = schema_constraints_free(&sc);

  /* required[i] == NULL */
  g_cdd_last_alloc_err = schema_constraints_init(&sc);
  g_cdd_last_alloc_err = schema_constraints_add_required(&sc, "r");
  free(sc.required[0]);
  sc.required[0] = NULL;
  g_cdd_last_alloc_err = schema_constraints_free(&sc);

#ifdef CDD_BUILD_TESTS
  {
    /* extern C_CDD_EXPORT int g_schema_strdup_fail; (moved to global) */
    struct SchemaConstraints sc_oom;
    g_cdd_last_alloc_err = schema_constraints_init(&sc_oom);
    g_schema_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              schema_constraints_add_required(&sc_oom, "req2"));
    g_schema_strdup_fail = 0;
    g_cdd_last_alloc_err = schema_constraints_free(&sc_oom);

    g_cdd_last_alloc_err = schema_constraints_init(&sc_oom);
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              schema_constraints_add_required(&sc_oom, "req3"));
    g_cdd_strdup_fail = 0;
    g_cdd_last_alloc_err = schema_constraints_free(&sc_oom);
  }
#endif
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Unit test test_schema_constraints_free_null_fields.
 * @return Greatest test result.
 */
TEST test_schema_constraints_free_null_fields(void) {
  struct SchemaConstraints sc;
  g_cdd_last_alloc_err = schema_constraints_init(&sc);

  sc.additional_properties = calloc(1, sizeof(*sc.additional_properties));
  g_cdd_last_alloc_err = schema_constraints_free(&sc);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Executes the schema_constraints_suite test suite.
 */
SUITE(schema_constraints_suite) {
  RUN_TEST(test_schema_constraints_roundtrip);
  RUN_TEST(test_schema_annotations_roundtrip);
  RUN_TEST(test_schema_allof_merge);
  RUN_TEST(test_schema_anyof_first_object);
  RUN_TEST(test_schema_oneof_first_object);
  RUN_TEST(test_schema_keyword_passthrough);
  RUN_TEST(test_schema_allof_keyword_merge);
  RUN_TEST(test_schema_type_union_roundtrip);
  RUN_TEST(test_schema_constraints_free_null_fields);
  RUN_TEST(test_schema_constraints_free_branch);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_SCHEMA_CONSTRAINTS_H */
