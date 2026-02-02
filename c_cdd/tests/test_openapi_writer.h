/**
 * @file test_openapi_writer.h
 * @brief Unit tests for OpenAPI Writer module.
 *
 * Verifies that in-memory structures are correctly serialized to valid
 * JSON strings matching the OpenAPI 3.0 specification structure.
 * Includes coverage for Advanced features: Security Schemes, Parameter Styles,
 * and Multipart/Recursive schemes.
 *
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_WRITER_H
#define TEST_OPENAPI_WRITER_H

#include <greatest.h>
#include <parson.h>
#include <stdlib.h>
#include <string.h>

#include "openapi_loader.h"
#include "openapi_writer.h"

/* --- Helpers --- */

static void setup_test_spec(struct OpenAPI_Spec *spec,
                            struct OpenAPI_Path *path,
                            struct OpenAPI_Operation *op,
                            struct OpenAPI_Parameter *param,
                            struct OpenAPI_Response *response) {
  /* Zero out stack structs */
  memset(spec, 0, sizeof(*spec));
  if (path)
    memset(path, 0, sizeof(*path));
  if (op)
    memset(op, 0, sizeof(*op));
  if (param)
    memset(param, 0, sizeof(*param));
  if (response)
    memset(response, 0, sizeof(*response));

  if (path) {
    spec->paths = path;
    spec->n_paths = 1;
    path->route = "/test/route";
    if (op) {
      path->operations = op;
      path->n_operations = 1;
      op->verb = OA_VERB_GET;
      op->operation_id = "testOp";
      if (param) {
        op->parameters = param;
        op->n_parameters = 1;
        param->name = "p1";
        param->in = OA_PARAM_IN_QUERY;
        param->type = "string";
      }
      if (response) {
        op->responses = response;
        op->n_responses = 1;
        response->code = "200";
        response->schema.ref_name = "TestModel";
      }
    }
  }
}

/* --- Tests --- */

TEST test_writer_empty_spec(void) {
  struct OpenAPI_Spec spec = {0};
  char *json = NULL;
  int rc;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *obj = json_value_get_object(root);
    ASSERT_STR_EQ("3.0.0", json_object_get_string(obj, "openapi"));
    ASSERT(json_object_has_value(obj, "info"));
    ASSERT(json_object_has_value(obj, "paths"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_basic_operation(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, NULL);

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");

    ASSERT(op_obj != NULL);
    ASSERT_STR_EQ("testOp", json_object_get_string(op_obj, "operationId"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_params_responses(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, &resp);

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "get");

    /* Param Check */
    {
      JSON_Array *oa_params = json_object_get_array(op_obj, "parameters");
      JSON_Object *p_obj = json_array_get_object(oa_params, 0);
      ASSERT_STR_EQ("p1", json_object_get_string(p_obj, "name"));
      ASSERT_STR_EQ("query", json_object_get_string(p_obj, "in"));
      {
        JSON_Object *sch = json_object_get_object(p_obj, "schema");
        ASSERT_STR_EQ("string", json_object_get_string(sch, "type"));
      }
    }

    /* Response Check */
    {
      JSON_Object *responses = json_object_get_object(op_obj, "responses");
      JSON_Object *r200 = json_object_get_object(responses, "200");
      JSON_Object *content = json_object_get_object(r200, "content");
      JSON_Object *media = json_object_get_object(content, "application/json");
      JSON_Object *schema = json_object_get_object(media, "schema");

      ASSERT_STR_EQ("#/components/schemas/TestModel",
                    json_object_get_string(schema, "$ref"));
    }

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_parameter_styles(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  /* Configure Advanced Params */
  param.in = OA_PARAM_IN_QUERY;
  param.style = OA_STYLE_FORM;
  param.explode = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "get");

    {
      JSON_Array *oa_params = json_object_get_array(op_obj, "parameters");
      JSON_Object *p_obj = json_array_get_object(oa_params, 0);
      ASSERT_STR_EQ("form", json_object_get_string(p_obj, "style"));
      ASSERT_EQ(1, json_object_get_boolean(p_obj, "explode"));
    }
    json_value_free(root);
  }
  free(json);
  PASS();
}

TEST test_writer_security_schemes(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_SecurityScheme s1, s2;
  char *json = NULL;
  int rc;

  struct OpenAPI_SecurityScheme schemes[2];
  memset(&s1, 0, sizeof(s1));
  memset(&s2, 0, sizeof(s2));

  s1.name = "bearerAuth";
  s1.type = OA_SEC_HTTP;
  s1.scheme = "bearer";

  s2.name = "apiKeyAuth";
  s2.type = OA_SEC_APIKEY;
  s2.in = OA_SEC_IN_HEADER;
  s2.key_name = "X-Api-Key";

  schemes[0] = s1;
  schemes[1] = s2;

  spec.security_schemes = schemes;
  spec.n_security_schemes = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *comps =
        json_object_get_object(json_value_get_object(root), "components");
    JSON_Object *secs = json_object_get_object(comps, "securitySchemes");

    /* Check Bearer */
    {
      JSON_Object *b = json_object_get_object(secs, "bearerAuth");
      ASSERT(b != NULL);
      ASSERT_STR_EQ("http", json_object_get_string(b, "type"));
      ASSERT_STR_EQ("bearer", json_object_get_string(b, "scheme"));
      ASSERT_STR_EQ("JWT", json_object_get_string(b, "bearerFormat"));
    }

    /* Check ApiKey */
    {
      JSON_Object *k = json_object_get_object(secs, "apiKeyAuth");
      ASSERT(k != NULL);
      ASSERT_STR_EQ("apiKey", json_object_get_string(k, "type"));
      ASSERT_STR_EQ("header", json_object_get_string(k, "in"));
      ASSERT_STR_EQ("X-Api-Key", json_object_get_string(k, "name"));
    }

    json_value_free(root);
  }
  free(json);
  PASS();
}

TEST test_writer_multipart_schema(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_MultipartField parts[2];
  char *json = NULL;
  int rc;

  memset(&parts, 0, sizeof(parts));
  parts[0].name = "file";
  parts[0].is_binary = 1; /* File upload */
  parts[1].name = "desc";
  parts[1].type = "string";

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.verb = OA_VERB_POST;
  op.req_body.content_type = "multipart/form-data";
  op.req_body.multipart_fields = parts;
  op.req_body.n_multipart_fields = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "post");

    {
      JSON_Object *req = json_object_get_object(op_obj, "requestBody");
      JSON_Object *cnt = json_object_get_object(req, "content");
      JSON_Object *mp = json_object_get_object(cnt, "multipart/form-data");
      JSON_Object *sch = json_object_get_object(mp, "schema");
      JSON_Object *props = json_object_get_object(sch, "properties");

      ASSERT_STR_EQ("object", json_object_get_string(sch, "type"));

      /* Check File */
      {
        JSON_Object *f = json_object_get_object(props, "file");
        ASSERT_STR_EQ("string", json_object_get_string(f, "type"));
        ASSERT_STR_EQ("binary", json_object_get_string(f, "format"));
      }
      /* Check Desc */
      {
        JSON_Object *d = json_object_get_object(props, "desc");
        ASSERT_STR_EQ("string", json_object_get_string(d, "type"));
      }
    }
    json_value_free(root);
  }
  free(json);
  PASS();
}

TEST test_writer_components_schemas(void) {
  struct OpenAPI_Spec spec;
  struct StructFields sf;
  char *name = "MyModel";
  char *json = NULL;
  int rc;

  memset(&spec, 0, sizeof(spec));
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, NULL, NULL);

  spec.defined_schemas = &sf;
  spec.defined_schema_names = &name;
  spec.n_defined_schemas = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *comps =
        json_object_get_object(json_value_get_object(root), "components");
    JSON_Object *schemas = json_object_get_object(comps, "schemas");
    JSON_Object *model = json_object_get_object(schemas, "MyModel");
    JSON_Object *props = json_object_get_object(model, "properties");
    JSON_Object *id_prop = json_object_get_object(props, "id");

    ASSERT_STR_EQ("integer", json_object_get_string(id_prop, "type"));

    json_value_free(root);
  }

  free(json);
  struct_fields_free(&sf);
  PASS();
}

TEST test_writer_input_validation(void) {
  struct OpenAPI_Spec spec;
  char *json;

  ASSERT_EQ(EINVAL, openapi_write_spec_to_json(NULL, &json));
  ASSERT_EQ(EINVAL, openapi_write_spec_to_json(&spec, NULL));

  PASS();
}

SUITE(openapi_writer_suite) {
  RUN_TEST(test_writer_empty_spec);
  RUN_TEST(test_writer_basic_operation);
  RUN_TEST(test_writer_params_responses);
  RUN_TEST(test_writer_parameter_styles);
  RUN_TEST(test_writer_security_schemes);
  RUN_TEST(test_writer_multipart_schema);
  RUN_TEST(test_writer_components_schemas);
  RUN_TEST(test_writer_input_validation);
}

#endif /* TEST_OPENAPI_WRITER_H */
