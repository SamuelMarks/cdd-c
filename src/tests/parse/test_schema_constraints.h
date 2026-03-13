/* clang-format off */
#ifndef TEST_SCHEMA_CONSTRAINTS_H
#define TEST_SCHEMA_CONSTRAINTS_H

#include <greatest.h>
#include <parson.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/emit/struct.h"
#include "classes/parse/code2schema.h"
/* clang-format on */

TEST test_schema_constraints_roundtrip(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct StructField *_ast_struct_fields_get_0;
  struct StructField *_ast_struct_fields_get_1;
  struct StructField *_ast_struct_fields_get_2;
  struct StructField *_ast_struct_fields_get_3;
  struct StructField *_ast_struct_fields_get_4;
  struct StructFields sf;
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

  val = json_parse_string(schema);
  ASSERT(val != NULL);
  obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    struct StructField *score;
    score = (struct_fields_get(&sf, "score", &_ast_struct_fields_get_0),
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
    age = (struct_fields_get(&sf, "age", &_ast_struct_fields_get_1),
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
    name = (struct_fields_get(&sf, "name", &_ast_struct_fields_get_2),
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
    enabled = (struct_fields_get(&sf, "enabled", &_ast_struct_fields_get_3),
               _ast_struct_fields_get_3);
    ASSERT(enabled != NULL);
    ASSERT_STR_EQ("1", enabled->default_val);
  }

  {
    struct StructField *tags;
    tags = (struct_fields_get(&sf, "tags", &_ast_struct_fields_get_4),
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
    schemas_val = json_value_init_object();
    schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Test", &sf));

    test_obj = json_object_get_object(schemas_obj, "Test");
    ASSERT(test_obj != NULL);
    props = json_object_get_object(test_obj, "properties");
    ASSERT(props != NULL);

    score_prop = json_object_get_object(props, "score");
    ASSERT(score_prop != NULL);
    ASSERT(json_object_has_value(score_prop, "exclusiveMinimum"));
    ASSERT(json_object_has_value(score_prop, "exclusiveMaximum"));
    {
      double ex_min;
      double ex_max;
      ex_min = json_object_get_number(score_prop, "exclusiveMinimum");
      ex_max = json_object_get_number(score_prop, "exclusiveMaximum");
      ASSERT(ex_min > 3.4 && ex_min < 3.6);
      ASSERT(ex_max > 9.2 && ex_max < 9.3);
    }
    {
      double def_num;
      def_num = json_object_get_number(score_prop, "default");
      ASSERT(def_num > 0.49 && def_num < 0.51);
    }

    age_prop = json_object_get_object(props, "age");
    ASSERT(age_prop != NULL);
    ASSERT_EQ(18, (int)json_object_get_number(age_prop, "minimum"));
    ASSERT_EQ(65, (int)json_object_get_number(age_prop, "exclusiveMaximum"));
    ASSERT_EQ(21, (int)json_object_get_number(age_prop, "default"));

    name_prop = json_object_get_object(props, "name");
    ASSERT(name_prop != NULL);
    ASSERT_EQ(2, (int)json_object_get_number(name_prop, "minLength"));
    ASSERT_EQ(10, (int)json_object_get_number(name_prop, "maxLength"));
    ASSERT_STR_EQ("^[A-Z]+$", json_object_get_string(name_prop, "pattern"));
    ASSERT_STR_EQ("Bob", json_object_get_string(name_prop, "default"));

    enabled_prop = json_object_get_object(props, "enabled");
    ASSERT(enabled_prop != NULL);
    ASSERT_EQ(1, json_object_get_boolean(enabled_prop, "default"));

    tags_prop = json_object_get_object(props, "tags");
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
  struct StructFields sf;
  JSON_Value *val;
  JSON_Object *obj;
  struct StructField *_ast_struct_fields_get_5;
  struct StructField *_ast_struct_fields_get_6;
  struct StructField *_ast_struct_fields_get_7;
  const char *schema = "{\"type\":\"object\",\"properties\":{"
                       "\"id\":{\"type\":\"string\",\"format\":\"uuid\","
                       "\"description\":\"User ID\",\"deprecated\":false},"
                       "\"secret\":{\"type\":\"string\",\"writeOnly\":true,"
                       "\"description\":\"Secret\"},"
                       "\"readme\":{\"type\":\"string\",\"readOnly\":true,"
                       "\"deprecated\":true}"
                       "}}";

  val = json_parse_string(schema);
  ASSERT(val != NULL);
  obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    struct StructField *id_field;
    id_field = (struct_fields_get(&sf, "id", &_ast_struct_fields_get_5),
                _ast_struct_fields_get_5);
    ASSERT(id_field != NULL);
    ASSERT_STR_EQ("uuid", id_field->format);
    ASSERT_STR_EQ("User ID", id_field->description);
    ASSERT_EQ(1, id_field->deprecated_set);
    ASSERT_EQ(0, id_field->deprecated);
  }

  {
    struct StructField *secret_field;
    secret_field = (struct_fields_get(&sf, "secret", &_ast_struct_fields_get_6),
                    _ast_struct_fields_get_6);
    ASSERT(secret_field != NULL);
    ASSERT_STR_EQ("Secret", secret_field->description);
    ASSERT_EQ(1, secret_field->write_only_set);
    ASSERT_EQ(1, secret_field->write_only);
  }

  {
    struct StructField *readme_field;
    readme_field = (struct_fields_get(&sf, "readme", &_ast_struct_fields_get_7),
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
    schemas_val = json_value_init_object();
    schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Annotated", &sf));

    annot = json_object_get_object(schemas_obj, "Annotated");
    props = json_object_get_object(annot, "properties");
    ASSERT(props != NULL);

    {
      JSON_Object *id_prop;
      id_prop = json_object_get_object(props, "id");
      ASSERT(id_prop != NULL);
      ASSERT_STR_EQ("uuid", json_object_get_string(id_prop, "format"));
      ASSERT_STR_EQ("User ID", json_object_get_string(id_prop, "description"));
      ASSERT(json_object_has_value(id_prop, "deprecated"));
      ASSERT_EQ(0, json_object_get_boolean(id_prop, "deprecated"));
    }

    {
      JSON_Object *secret_prop;
      secret_prop = json_object_get_object(props, "secret");
      ASSERT(secret_prop != NULL);
      ASSERT_STR_EQ("Secret",
                    json_object_get_string(secret_prop, "description"));
      ASSERT_EQ(1, json_object_get_boolean(secret_prop, "writeOnly"));
    }

    {
      JSON_Object *readme_prop;
      readme_prop = json_object_get_object(props, "readme");
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
  JSON_Value *root_val;
  JSON_Value *schema_val;
  struct StructField *_ast_struct_fields_get_8;
  struct StructField *_ast_struct_fields_get_9;
  struct StructField *_ast_struct_fields_get_10;
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

  root_val = json_parse_string(schemas);
  schema_val = json_parse_string(schema);
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
      id_field = (struct_fields_get(&sf, "id", &_ast_struct_fields_get_8),
                  _ast_struct_fields_get_8);
      ASSERT(id_field != NULL);
      ASSERT_STR_EQ("integer", id_field->type);
      ASSERT_EQ(1, id_field->required);
      ASSERT_EQ(1, id_field->has_min);
      ASSERT_EQ(1, (int)id_field->min_val);
    }

    {
      struct StructField *name_field;
      name_field = (struct_fields_get(&sf, "name", &_ast_struct_fields_get_9),
                    _ast_struct_fields_get_9);
      ASSERT(name_field != NULL);
      ASSERT_STR_EQ("string", name_field->type);
      ASSERT_EQ(1, name_field->required);
      ASSERT_EQ(1, name_field->has_min_len);
      ASSERT_EQ((size_t)2, name_field->min_len);
    }

    {
      struct StructField *extra_field;
      extra_field =
          (struct_fields_get(&sf, "extra", &_ast_struct_fields_get_10),
           _ast_struct_fields_get_10);
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
  JSON_Value *schema_val;
  struct StructField *_ast_struct_fields_get_11;
  struct StructField *_ast_struct_fields_get_12;
  const char *schema =
      "{"
      "\"anyOf\":["
      "{\"type\":\"object\",\"properties\":{\"first\":{\"type\":\"string\"}}},"
      "{\"type\":\"object\",\"properties\":{\"second\":{\"type\":\"integer\"}}}"
      "]"
      "}";

  schema_val = json_parse_string(schema);
  ASSERT(schema_val != NULL);

  {
    struct StructFields sf;
    JSON_Object *schema_obj;
    schema_obj = json_value_get_object(schema_val);
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, NULL));

    ASSERT((struct_fields_get(&sf, "first", &_ast_struct_fields_get_11),
            _ast_struct_fields_get_11) != NULL);
    ASSERT((struct_fields_get(&sf, "second", &_ast_struct_fields_get_12),
            _ast_struct_fields_get_12) == NULL);

    struct_fields_free(&sf);
  }

  json_value_free(schema_val);
  PASS();
}

TEST test_schema_oneof_first_object(void) {
  JSON_Value *schema_val;
  struct StructField *_ast_struct_fields_get_13;
  struct StructField *_ast_struct_fields_get_14;
  const char *schema =
      "{"
      "\"oneOf\":["
      "{\"type\":\"object\",\"properties\":{\"alpha\":{\"type\":\"string\"}}},"
      "{\"type\":\"object\",\"properties\":{\"beta\":{\"type\":\"integer\"}}}"
      "]"
      "}";

  schema_val = json_parse_string(schema);
  ASSERT(schema_val != NULL);

  {
    struct StructFields sf;
    JSON_Object *schema_obj;
    schema_obj = json_value_get_object(schema_val);
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, NULL));

    ASSERT((struct_fields_get(&sf, "alpha", &_ast_struct_fields_get_13),
            _ast_struct_fields_get_13) != NULL);
    ASSERT((struct_fields_get(&sf, "beta", &_ast_struct_fields_get_14),
            _ast_struct_fields_get_14) == NULL);

    struct_fields_free(&sf);
  }

  json_value_free(schema_val);
  PASS();
}

TEST test_schema_keyword_passthrough(void) {
  struct StructFields sf;
  JSON_Value *val;
  JSON_Object *obj;
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

  val = json_parse_string(schema);
  ASSERT(val != NULL);
  obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    JSON_Value *schemas_val;
    JSON_Object *schemas_obj;
    JSON_Object *spec;
    schemas_val = json_value_init_object();
    schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Spec", &sf));

    spec = json_object_get_object(schemas_obj, "Spec");
    ASSERT(spec != NULL);
    ASSERT(json_object_has_value(spec, "additionalProperties"));
    ASSERT_EQ(0, json_object_get_boolean(spec, "additionalProperties"));

    {
      JSON_Object *pat_schema;
      JSON_Object *pattern_props;
      pattern_props = json_object_get_object(spec, "patternProperties");
      ASSERT(pattern_props != NULL);
      pat_schema = json_object_get_object(pattern_props, "^x-");
      ASSERT(pat_schema != NULL);
      ASSERT_STR_EQ("string", json_object_get_string(pat_schema, "type"));
    }

    {
      JSON_Object *defs;
      JSON_Object *extra;
      defs = json_object_get_object(spec, "$defs");
      ASSERT(defs != NULL);
      extra = json_object_get_object(defs, "Extra");
      ASSERT(extra != NULL);
      ASSERT_STR_EQ("string", json_object_get_string(extra, "type"));
    }

    ASSERT(json_object_get_object(spec, "not") != NULL);
    ASSERT(json_object_get_object(spec, "if") != NULL);
    ASSERT(json_object_get_object(spec, "then") != NULL);
    ASSERT(json_object_get_object(spec, "else") != NULL);

    {
      JSON_Object *props;
      JSON_Object *name_prop;
      JSON_Object *meta_prop;
      JSON_Object *tags_prop;
      props = json_object_get_object(spec, "properties");
      ASSERT(props != NULL);

      name_prop = json_object_get_object(props, "name");
      ASSERT(name_prop != NULL);
      {
        JSON_Array *enum_arr;
        enum_arr = json_object_get_array(name_prop, "enum");
        ASSERT(enum_arr != NULL);
        ASSERT_EQ(2, (int)json_array_get_count(enum_arr));
        ASSERT_STR_EQ("A", json_array_get_string(enum_arr, 0));
        ASSERT_STR_EQ("B", json_array_get_string(enum_arr, 1));
      }
      ASSERT_STR_EQ("Name", json_object_get_string(name_prop, "title"));

      meta_prop = json_object_get_object(props, "meta");
      ASSERT(meta_prop != NULL);
      {
        JSON_Object *meta_additional;
        meta_additional =
            json_object_get_object(meta_prop, "additionalProperties");
        ASSERT(meta_additional != NULL);
        ASSERT_STR_EQ("string",
                      json_object_get_string(meta_additional, "type"));
      }

      tags_prop = json_object_get_object(props, "tags");
      ASSERT(tags_prop != NULL);
      {
        JSON_Object *items;
        items = json_object_get_object(tags_prop, "items");
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
  JSON_Value *schema_val;
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

  schema_val = json_parse_string(schema);
  ASSERT(schema_val != NULL);

  {
    struct StructFields sf;
    JSON_Object *schema_obj;
    JSON_Value *schemas_val;
    JSON_Object *schemas_obj;
    schema_obj = json_value_get_object(schema_val);
    ASSERT_EQ(0, struct_fields_init(&sf));
    ASSERT_EQ(0, json_object_to_struct_fields(schema_obj, &sf, NULL));

    schemas_val = json_value_init_object();
    schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Merged", &sf));

    {
      JSON_Object *merged;
      JSON_Object *pat_schema;
      JSON_Object *props;
      JSON_Object *id_prop;
      JSON_Object *pattern_props;
      merged = json_object_get_object(schemas_obj, "Merged");
      ASSERT(merged != NULL);
      ASSERT(json_object_has_value(merged, "additionalProperties"));
      ASSERT_EQ(0, json_object_get_boolean(merged, "additionalProperties"));

      pattern_props = json_object_get_object(merged, "patternProperties");
      ASSERT(pattern_props != NULL);
      pat_schema = json_object_get_object(pattern_props, "^x-");
      ASSERT(pat_schema != NULL);
      ASSERT_STR_EQ("string", json_object_get_string(pat_schema, "type"));

      props = json_object_get_object(merged, "properties");
      ASSERT(props != NULL);
      id_prop = json_object_get_object(props, "id");
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
  struct StructFields sf;
  JSON_Value *val;
  JSON_Object *obj;
  struct StructField *_ast_struct_fields_get_15;
  struct StructField *_ast_struct_fields_get_16;
  struct StructField *_ast_struct_fields_get_17;
  const char *schema =
      "{"
      "\"type\":\"object\","
      "\"properties\":{"
      "\"name\":{\"type\":[\"string\",\"null\"]},"
      "\"age\":{\"type\":[\"integer\",\"null\"]},"
      "\"tags\":{\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]}}"
      "}"
      "}";

  val = json_parse_string(schema);
  ASSERT(val != NULL);
  obj = json_value_get_object(val);
  ASSERT(obj != NULL);

  ASSERT_EQ(0, struct_fields_init(&sf));
  ASSERT_EQ(0, json_object_to_struct_fields(obj, &sf, NULL));

  {
    struct StructField *name;
    name = (struct_fields_get(&sf, "name", &_ast_struct_fields_get_15),
            _ast_struct_fields_get_15);
    ASSERT(name != NULL);
    ASSERT_EQ(2, (int)name->n_type_union);
    ASSERT_STR_EQ("string", name->type_union[0]);
    ASSERT_STR_EQ("null", name->type_union[1]);
    ASSERT_STR_EQ("string", name->type);
  }

  {
    struct StructField *age;
    age = (struct_fields_get(&sf, "age", &_ast_struct_fields_get_16),
           _ast_struct_fields_get_16);
    ASSERT(age != NULL);
    ASSERT_EQ(2, (int)age->n_type_union);
    ASSERT_STR_EQ("integer", age->type_union[0]);
    ASSERT_STR_EQ("null", age->type_union[1]);
    ASSERT_STR_EQ("integer", age->type);
  }

  {
    struct StructField *tags;
    tags = (struct_fields_get(&sf, "tags", &_ast_struct_fields_get_17),
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
    schemas_val = json_value_init_object();
    schemas_obj = json_value_get_object(schemas_val);
    ASSERT_EQ(0, write_struct_to_json_schema(schemas_obj, "Union", &sf));

    union_obj = json_object_get_object(schemas_obj, "Union");
    ASSERT(union_obj != NULL);
    props = json_object_get_object(union_obj, "properties");
    ASSERT(props != NULL);

    {
      JSON_Object *name_prop;
      JSON_Array *type_arr;
      name_prop = json_object_get_object(props, "name");
      ASSERT(name_prop != NULL);
      type_arr = json_object_get_array(name_prop, "type");
      ASSERT(type_arr != NULL);
      ASSERT_EQ(2, (int)json_array_get_count(type_arr));
      ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
      ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    }

    {
      JSON_Object *age_prop;
      JSON_Array *type_arr;
      age_prop = json_object_get_object(props, "age");
      ASSERT(age_prop != NULL);
      type_arr = json_object_get_array(age_prop, "type");
      ASSERT(type_arr != NULL);
      ASSERT_EQ(2, (int)json_array_get_count(type_arr));
      ASSERT_STR_EQ("integer", json_array_get_string(type_arr, 0));
      ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    }

    {
      JSON_Object *tags_prop;
      JSON_Object *items;
      JSON_Array *type_arr;
      tags_prop = json_object_get_object(props, "tags");
      ASSERT(tags_prop != NULL);
      items = json_object_get_object(tags_prop, "items");
      ASSERT(items != NULL);
      type_arr = json_object_get_array(items, "type");
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
