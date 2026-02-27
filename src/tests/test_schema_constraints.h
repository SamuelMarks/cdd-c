#ifndef TEST_SCHEMA_CONSTRAINTS_H
#define TEST_SCHEMA_CONSTRAINTS_H

#include <greatest.h>
#include <parson.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/emit_struct.h"
#include "classes/parse_code2schema.h"

TEST test_schema_constraints_roundtrip(void) {
  const char *schema =
      "{\"type\":\"object\",\"properties\":{"
      "\"score\":{\"type\":\"number\","
      "\"exclusiveMinimum\":3.5,\"exclusiveMaximum\":9.25,"
      "\"default\":0.5},"
      "\"age\":{\"type\":\"integer\",\"minimum\":18,"
      "\"exclusiveMaximum\":65,\"default\":21},"
      "\"name\":{\"type\":\"string\",\"minLength\":2,"
      "\"maxLength\":10,\"pattern\":\"^[A-Z]+$\","
      "\"default\":\"Bob\"},"
      "\"tags\":{\"type\":\"array\",\"items\":{\"type\":\"string\"},"
      "\"minItems\":1,\"maxItems\":3,\"uniqueItems\":true},"
      "\"enabled\":{\"type\":\"boolean\",\"default\":true}"
      "}}";

  JSON_Value *val = json_parse_string(schema);
  ASSERT(val != NULL);
  JSON_Object *obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  struct StructFields sf;
  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    struct StructField *score = struct_fields_get(&sf, "score");
    ASSERT(score != NULL);
    ASSERT_EQ(1, score->has_min);
    ASSERT_EQ(1, score->exclusive_min);
    ASSERT(score->min_val > 3.4 && score->min_val < 3.6);
    ASSERT_EQ(1, score->has_max);
    ASSERT_EQ(1, score->exclusive_max);
    ASSERT(score->max_val > 9.2 && score->max_val < 9.3);
  }

  {
    struct StructField *age = struct_fields_get(&sf, "age");
    ASSERT(age != NULL);
    ASSERT_EQ(1, age->has_min);
    ASSERT_EQ(0, age->exclusive_min);
    ASSERT_EQ(1, age->has_max);
    ASSERT_EQ(1, age->exclusive_max);
    ASSERT_EQ(18, (int)age->min_val);
    ASSERT_EQ(65, (int)age->max_val);
  }

  {
    struct StructField *name = struct_fields_get(&sf, "name");
    ASSERT(name != NULL);
    ASSERT_EQ(1, name->has_min_len);
    ASSERT_EQ(1, name->has_max_len);
    ASSERT_EQ((size_t)2, name->min_len);
    ASSERT_EQ((size_t)10, name->max_len);
    ASSERT_STR_EQ("^[A-Z]+$", name->pattern);
    ASSERT_STR_EQ("\"Bob\"", name->default_val);
  }

  {
    struct StructField *enabled = struct_fields_get(&sf, "enabled");
    ASSERT(enabled != NULL);
    ASSERT_STR_EQ("1", enabled->default_val);
  }

  {
    struct StructField *tags = struct_fields_get(&sf, "tags");
    ASSERT(tags != NULL);
    ASSERT_EQ(1, tags->has_min_items);
    ASSERT_EQ(1, tags->has_max_items);
    ASSERT_EQ(1, tags->unique_items);
    ASSERT_EQ((size_t)1, tags->min_items);
    ASSERT_EQ((size_t)3, tags->max_items);
  }

  {
    JSON_Value *schemas_val = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Test", &sf));

    JSON_Object *test_obj = json_object_get_object(schemas_obj, "Test");
    ASSERT(test_obj != NULL);
    JSON_Object *props = json_object_get_object(test_obj, "properties");
    ASSERT(props != NULL);

    JSON_Object *score_prop = json_object_get_object(props, "score");
    ASSERT(score_prop != NULL);
    ASSERT(json_object_has_value(score_prop, "exclusiveMinimum"));
    ASSERT(json_object_has_value(score_prop, "exclusiveMaximum"));
    {
      double ex_min = json_object_get_number(score_prop, "exclusiveMinimum");
      double ex_max = json_object_get_number(score_prop, "exclusiveMaximum");
      ASSERT(ex_min > 3.4 && ex_min < 3.6);
      ASSERT(ex_max > 9.2 && ex_max < 9.3);
    }
    {
      double def_num = json_object_get_number(score_prop, "default");
      ASSERT(def_num > 0.49 && def_num < 0.51);
    }

    JSON_Object *age_prop = json_object_get_object(props, "age");
    ASSERT(age_prop != NULL);
    ASSERT_EQ(18, (int)json_object_get_number(age_prop, "minimum"));
    ASSERT_EQ(65, (int)json_object_get_number(age_prop, "exclusiveMaximum"));
    ASSERT_EQ(21, (int)json_object_get_number(age_prop, "default"));

    JSON_Object *name_prop = json_object_get_object(props, "name");
    ASSERT(name_prop != NULL);
    ASSERT_EQ(2, (int)json_object_get_number(name_prop, "minLength"));
    ASSERT_EQ(10, (int)json_object_get_number(name_prop, "maxLength"));
    ASSERT_STR_EQ("^[A-Z]+$", json_object_get_string(name_prop, "pattern"));
    ASSERT_STR_EQ("Bob", json_object_get_string(name_prop, "default"));

    JSON_Object *enabled_prop = json_object_get_object(props, "enabled");
    ASSERT(enabled_prop != NULL);
    ASSERT_EQ(1, json_object_get_boolean(enabled_prop, "default"));

    JSON_Object *tags_prop = json_object_get_object(props, "tags");
    ASSERT(tags_prop != NULL);
    ASSERT_EQ(1, (int)json_object_get_number(tags_prop, "minItems"));
    ASSERT_EQ(3, (int)json_object_get_number(tags_prop, "maxItems"));
    ASSERT_EQ(1, json_object_get_boolean(tags_prop, "uniqueItems"));

    json_value_free(schemas_val);
  }

  struct_fields_free(&sf);
  json_value_free(val);

  PASS();
}

TEST test_schema_annotations_roundtrip(void) {
  const char *schema = "{\"type\":\"object\",\"properties\":{"
                       "\"id\":{\"type\":\"string\",\"format\":\"uuid\","
                       "\"description\":\"User ID\",\"deprecated\":false},"
                       "\"secret\":{\"type\":\"string\",\"writeOnly\":true,"
                       "\"description\":\"Secret\"},"
                       "\"readme\":{\"type\":\"string\",\"readOnly\":true,"
                       "\"deprecated\":true}"
                       "}}";

  JSON_Value *val = json_parse_string(schema);
  ASSERT(val != NULL);
  JSON_Object *obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  struct StructFields sf;
  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    struct StructField *id_field = struct_fields_get(&sf, "id");
    ASSERT(id_field != NULL);
    ASSERT_STR_EQ("uuid", id_field->format);
    ASSERT_STR_EQ("User ID", id_field->description);
    ASSERT_EQ(1, id_field->deprecated_set);
    ASSERT_EQ(0, id_field->deprecated);
  }

  {
    struct StructField *secret_field = struct_fields_get(&sf, "secret");
    ASSERT(secret_field != NULL);
    ASSERT_STR_EQ("Secret", secret_field->description);
    ASSERT_EQ(1, secret_field->write_only_set);
    ASSERT_EQ(1, secret_field->write_only);
  }

  {
    struct StructField *readme_field = struct_fields_get(&sf, "readme");
    ASSERT(readme_field != NULL);
    ASSERT_EQ(1, readme_field->read_only_set);
    ASSERT_EQ(1, readme_field->read_only);
    ASSERT_EQ(1, readme_field->deprecated_set);
    ASSERT_EQ(1, readme_field->deprecated);
  }

  {
    JSON_Value *schemas_val = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Annotated", &sf));

    JSON_Object *annot = json_object_get_object(schemas_obj, "Annotated");
    JSON_Object *props = json_object_get_object(annot, "properties");
    ASSERT(props != NULL);

    {
      JSON_Object *id_prop = json_object_get_object(props, "id");
      ASSERT(id_prop != NULL);
      ASSERT_STR_EQ("uuid", json_object_get_string(id_prop, "format"));
      ASSERT_STR_EQ("User ID", json_object_get_string(id_prop, "description"));
      ASSERT(json_object_has_value(id_prop, "deprecated"));
      ASSERT_EQ(0, json_object_get_boolean(id_prop, "deprecated"));
    }

    {
      JSON_Object *secret_prop = json_object_get_object(props, "secret");
      ASSERT(secret_prop != NULL);
      ASSERT_STR_EQ("Secret",
                    json_object_get_string(secret_prop, "description"));
      ASSERT_EQ(1, json_object_get_boolean(secret_prop, "writeOnly"));
    }

    {
      JSON_Object *readme_prop = json_object_get_object(props, "readme");
      ASSERT(readme_prop != NULL);
      ASSERT_EQ(1, json_object_get_boolean(readme_prop, "readOnly"));
      ASSERT_EQ(1, json_object_get_boolean(readme_prop, "deprecated"));
    }

    json_value_free(schemas_val);
  }

  struct_fields_free(&sf);
  json_value_free(val);

  PASS();
}

TEST test_schema_allof_merge(void) {
  const char *schemas = "{"
                        "\"Base\":{\"type\":\"object\",\"properties\":{"
                        "\"name\":{\"type\":\"string\",\"minLength\":2}"
                        "},\"required\":[\"name\"]}"
                        "}";

  const char *schema = "{"
                       "\"allOf\":["
                       "{\"type\":\"object\",\"properties\":{"
                       "\"id\":{\"type\":\"integer\",\"minimum\":1}"
                       "},\"required\":[\"id\"]},"
                       "{\"$ref\":\"#/components/schemas/Base\"}"
                       "],"
                       "\"properties\":{"
                       "\"extra\":{\"type\":\"string\"}"
                       "}"
                       "}";

  JSON_Value *root_val = json_parse_string(schemas);
  JSON_Value *schema_val = json_parse_string(schema);
  ASSERT(root_val != NULL);
  ASSERT(schema_val != NULL);

  {
    JSON_Object *root_obj = json_value_get_object(root_val);
    JSON_Object *schema_obj = json_value_get_object(schema_val);
    struct StructFields sf;
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, root_obj));

    {
      struct StructField *id_field = struct_fields_get(&sf, "id");
      ASSERT(id_field != NULL);
      ASSERT_STR_EQ("integer", id_field->type);
      ASSERT_EQ(1, id_field->required);
      ASSERT_EQ(1, id_field->has_min);
      ASSERT_EQ(1, (int)id_field->min_val);
    }

    {
      struct StructField *name_field = struct_fields_get(&sf, "name");
      ASSERT(name_field != NULL);
      ASSERT_STR_EQ("string", name_field->type);
      ASSERT_EQ(1, name_field->required);
      ASSERT_EQ(1, name_field->has_min_len);
      ASSERT_EQ((size_t)2, name_field->min_len);
    }

    {
      struct StructField *extra_field = struct_fields_get(&sf, "extra");
      ASSERT(extra_field != NULL);
      ASSERT_STR_EQ("string", extra_field->type);
      ASSERT_EQ(0, extra_field->required);
    }

    struct_fields_free(&sf);
  }

  json_value_free(schema_val);
  json_value_free(root_val);

  PASS();
}

TEST test_schema_anyof_first_object(void) {
  const char *schema =
      "{"
      "\"anyOf\":["
      "{\"type\":\"object\",\"properties\":{\"first\":{\"type\":\"string\"}}},"
      "{\"type\":\"object\",\"properties\":{\"second\":{\"type\":\"integer\"}}}"
      "]"
      "}";

  JSON_Value *schema_val = json_parse_string(schema);
  ASSERT(schema_val != NULL);

  {
    JSON_Object *schema_obj = json_value_get_object(schema_val);
    struct StructFields sf;
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, NULL));

    ASSERT(struct_fields_get(&sf, "first") != NULL);
    ASSERT(struct_fields_get(&sf, "second") == NULL);

    struct_fields_free(&sf);
  }

  json_value_free(schema_val);
  PASS();
}

TEST test_schema_oneof_first_object(void) {
  const char *schema =
      "{"
      "\"oneOf\":["
      "{\"type\":\"object\",\"properties\":{\"alpha\":{\"type\":\"string\"}}},"
      "{\"type\":\"object\",\"properties\":{\"beta\":{\"type\":\"integer\"}}}"
      "]"
      "}";

  JSON_Value *schema_val = json_parse_string(schema);
  ASSERT(schema_val != NULL);

  {
    JSON_Object *schema_obj = json_value_get_object(schema_val);
    struct StructFields sf;
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, NULL));

    ASSERT(struct_fields_get(&sf, "alpha") != NULL);
    ASSERT(struct_fields_get(&sf, "beta") == NULL);

    struct_fields_free(&sf);
  }

  json_value_free(schema_val);
  PASS();
}

TEST test_schema_keyword_passthrough(void) {
  const char *schema =
      "{"
      "\"type\":\"object\","
      "\"additionalProperties\":false,"
      "\"patternProperties\":{"
      "\"^x-\":{\"type\":\"string\"}"
      "},"
      "\"$defs\":{"
      "\"Extra\":{\"type\":\"string\"}"
      "},"
      "\"not\":{\"required\":[\"blocked\"]},"
      "\"if\":{\"properties\":{\"kind\":{\"const\":\"A\"}}},"
      "\"then\":{\"required\":[\"name\"]},"
      "\"else\":{\"required\":[\"id\"]},"
      "\"properties\":{"
      "\"name\":{\"type\":\"string\",\"enum\":[\"A\",\"B\"],\"title\":\"Name\"}"
      ","
      "\"meta\":{\"type\":\"object\",\"additionalProperties\":{\"type\":"
      "\"string\"}},"
      "\"tags\":{\"type\":\"array\",\"items\":{\"type\":\"string\","
      "\"pattern\":\"^[a-z]+$\"}}"
      "}"
      "}";

  JSON_Value *val = json_parse_string(schema);
  ASSERT(val != NULL);
  JSON_Object *obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  struct StructFields sf;
  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    JSON_Value *schemas_val = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Spec", &sf));

    JSON_Object *spec = json_object_get_object(schemas_obj, "Spec");
    ASSERT(spec != NULL);
    ASSERT(json_object_has_value(spec, "additionalProperties"));
    ASSERT_EQ(0, json_object_get_boolean(spec, "additionalProperties"));

    {
      JSON_Object *pattern_props =
          json_object_get_object(spec, "patternProperties");
      ASSERT(pattern_props != NULL);
      JSON_Object *pat_schema = json_object_get_object(pattern_props, "^x-");
      ASSERT(pat_schema != NULL);
      ASSERT_STR_EQ("string", json_object_get_string(pat_schema, "type"));
    }

    {
      JSON_Object *defs = json_object_get_object(spec, "$defs");
      ASSERT(defs != NULL);
      JSON_Object *extra = json_object_get_object(defs, "Extra");
      ASSERT(extra != NULL);
      ASSERT_STR_EQ("string", json_object_get_string(extra, "type"));
    }

    ASSERT(json_object_get_object(spec, "not") != NULL);
    ASSERT(json_object_get_object(spec, "if") != NULL);
    ASSERT(json_object_get_object(spec, "then") != NULL);
    ASSERT(json_object_get_object(spec, "else") != NULL);

    {
      JSON_Object *props = json_object_get_object(spec, "properties");
      ASSERT(props != NULL);

      JSON_Object *name_prop = json_object_get_object(props, "name");
      ASSERT(name_prop != NULL);
      {
        JSON_Array *enum_arr = json_object_get_array(name_prop, "enum");
        ASSERT(enum_arr != NULL);
        ASSERT_EQ(2, (int)json_array_get_count(enum_arr));
        ASSERT_STR_EQ("A", json_array_get_string(enum_arr, 0));
        ASSERT_STR_EQ("B", json_array_get_string(enum_arr, 1));
      }
      ASSERT_STR_EQ("Name", json_object_get_string(name_prop, "title"));

      JSON_Object *meta_prop = json_object_get_object(props, "meta");
      ASSERT(meta_prop != NULL);
      {
        JSON_Object *meta_additional =
            json_object_get_object(meta_prop, "additionalProperties");
        ASSERT(meta_additional != NULL);
        ASSERT_STR_EQ("string",
                      json_object_get_string(meta_additional, "type"));
      }

      JSON_Object *tags_prop = json_object_get_object(props, "tags");
      ASSERT(tags_prop != NULL);
      {
        JSON_Object *items = json_object_get_object(tags_prop, "items");
        ASSERT(items != NULL);
        ASSERT_STR_EQ("^[a-z]+$", json_object_get_string(items, "pattern"));
      }
    }

    json_value_free(schemas_val);
  }

  struct_fields_free(&sf);
  json_value_free(val);

  PASS();
}

TEST test_schema_allof_keyword_merge(void) {
  const char *schema = "{"
                       "\"allOf\":["
                       "{"
                       "\"type\":\"object\","
                       "\"additionalProperties\":false,"
                       "\"properties\":{"
                       "\"id\":{\"type\":\"string\",\"x-alpha\":true}"
                       "}"
                       "},"
                       "{"
                       "\"type\":\"object\","
                       "\"patternProperties\":{\"^x-\":{\"type\":\"string\"}},"
                       "\"properties\":{"
                       "\"id\":{\"type\":\"string\",\"x-beta\":1}"
                       "}"
                       "}"
                       "]"
                       "}";

  JSON_Value *schema_val = json_parse_string(schema);
  ASSERT(schema_val != NULL);

  {
    JSON_Object *schema_obj = json_value_get_object(schema_val);
    struct StructFields sf;
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, NULL));

    JSON_Value *schemas_val = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Merged", &sf));

    {
      JSON_Object *merged = json_object_get_object(schemas_obj, "Merged");
      ASSERT(merged != NULL);
      ASSERT(json_object_has_value(merged, "additionalProperties"));
      ASSERT_EQ(0, json_object_get_boolean(merged, "additionalProperties"));

      JSON_Object *pattern_props =
          json_object_get_object(merged, "patternProperties");
      ASSERT(pattern_props != NULL);
      JSON_Object *pat_schema = json_object_get_object(pattern_props, "^x-");
      ASSERT(pat_schema != NULL);
      ASSERT_STR_EQ("string", json_object_get_string(pat_schema, "type"));

      JSON_Object *props = json_object_get_object(merged, "properties");
      ASSERT(props != NULL);
      JSON_Object *id_prop = json_object_get_object(props, "id");
      ASSERT(id_prop != NULL);
      ASSERT(json_object_has_value(id_prop, "x-alpha"));
      ASSERT_EQ(1, json_object_get_boolean(id_prop, "x-alpha"));
      ASSERT(json_object_has_value(id_prop, "x-beta"));
      ASSERT_EQ(1, (int)json_object_get_number(id_prop, "x-beta"));
    }

    json_value_free(schemas_val);
    struct_fields_free(&sf);
  }

  json_value_free(schema_val);
  PASS();
}

TEST test_schema_type_union_roundtrip(void) {
  const char *schema =
      "{"
      "\"type\":\"object\","
      "\"properties\":{"
      "\"name\":{\"type\":[\"string\",\"null\"]},"
      "\"age\":{\"type\":[\"integer\",\"null\"]},"
      "\"tags\":{\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]}}"
      "}"
      "}";

  JSON_Value *val = json_parse_string(schema);
  ASSERT(val != NULL);
  JSON_Object *obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  struct StructFields sf;
  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    struct StructField *name = struct_fields_get(&sf, "name");
    ASSERT(name != NULL);
    ASSERT_EQ(2, (int)name->n_type_union);
    ASSERT_STR_EQ("string", name->type_union[0]);
    ASSERT_STR_EQ("null", name->type_union[1]);
    ASSERT_STR_EQ("string", name->type);
  }

  {
    struct StructField *age = struct_fields_get(&sf, "age");
    ASSERT(age != NULL);
    ASSERT_EQ(2, (int)age->n_type_union);
    ASSERT_STR_EQ("integer", age->type_union[0]);
    ASSERT_STR_EQ("null", age->type_union[1]);
    ASSERT_STR_EQ("integer", age->type);
  }

  {
    struct StructField *tags = struct_fields_get(&sf, "tags");
    ASSERT(tags != NULL);
    ASSERT_EQ(2, (int)tags->n_items_type_union);
    ASSERT_STR_EQ("string", tags->items_type_union[0]);
    ASSERT_STR_EQ("null", tags->items_type_union[1]);
  }

  {
    JSON_Value *schemas_val = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Union", &sf));

    JSON_Object *union_obj = json_object_get_object(schemas_obj, "Union");
    ASSERT(union_obj != NULL);
    JSON_Object *props = json_object_get_object(union_obj, "properties");
    ASSERT(props != NULL);

    {
      JSON_Object *name_prop = json_object_get_object(props, "name");
      ASSERT(name_prop != NULL);
      JSON_Array *type_arr = json_object_get_array(name_prop, "type");
      ASSERT(type_arr != NULL);
      ASSERT_EQ(2, (int)json_array_get_count(type_arr));
      ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
      ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    }

    {
      JSON_Object *age_prop = json_object_get_object(props, "age");
      ASSERT(age_prop != NULL);
      JSON_Array *type_arr = json_object_get_array(age_prop, "type");
      ASSERT(type_arr != NULL);
      ASSERT_EQ(2, (int)json_array_get_count(type_arr));
      ASSERT_STR_EQ("integer", json_array_get_string(type_arr, 0));
      ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    }

    {
      JSON_Object *tags_prop = json_object_get_object(props, "tags");
      ASSERT(tags_prop != NULL);
      JSON_Object *items = json_object_get_object(tags_prop, "items");
      ASSERT(items != NULL);
      JSON_Array *type_arr = json_object_get_array(items, "type");
      ASSERT(type_arr != NULL);
      ASSERT_EQ(2, (int)json_array_get_count(type_arr));
      ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
      ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    }

    json_value_free(schemas_val);
  }

  struct_fields_free(&sf);
  json_value_free(val);
  PASS();
}

SUITE(schema_constraints_suite) {
  RUN_TEST(test_schema_constraints_roundtrip);
  RUN_TEST(test_schema_annotations_roundtrip);
  RUN_TEST(test_schema_allof_merge);
  RUN_TEST(test_schema_anyof_first_object);
  RUN_TEST(test_schema_oneof_first_object);
  RUN_TEST(test_schema_keyword_passthrough);
  RUN_TEST(test_schema_allof_keyword_merge);
  RUN_TEST(test_schema_type_union_roundtrip);
}

#endif /* TEST_SCHEMA_CONSTRAINTS_H */
