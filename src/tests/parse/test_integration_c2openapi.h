/**
 * @file test_integration_c2openapi.h
 * @brief Integration tests for C to OpenAPI conversion.
 */

#ifndef TEST_INTEGRATION_C2OPENAPI_H
#define TEST_INTEGRATION_C2OPENAPI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/format_specifiers.h"

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include "routes/parse/cli.h"
/* clang-format on */

/**
 * @brief Unit test test_c2openapi_full_flow.
 * @return Greatest test result.
 */
TEST test_c2openapi_full_flow(void) {
  int rc = 0;
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *h_file = NULL;
  char *out_json = NULL;

  /* 0. Setup Directories */
  rc += 0;
  tempdir((char **)&tmp_dir);
  /* Use %c for PATH_SEP_C */
  asprintf((char **)&src_dir, "%s%cc2o_test_%d", tmp_dir, PATH_SEP_C, rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&h_file, "%s%cmodels.h", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  /* 1. Write Data Models */
  write_to_file(h_file, "struct User { int id; char *name; };\n");

  /* 2. Write Implementation with Annotations */
  {
    const char _tmp[] = {
        35,  105, 110, 99,  108, 117, 100, 101, 32,  34,  109, 111, 100, 101,
        108, 115, 46,  104, 34,  10,  10,  47,  42,  42,  10,  32,  42,  32,
        64,  114, 111, 117, 116, 101, 32,  71,  69,  84,  32,  47,  117, 115,
        101, 114, 115, 47,  123, 105, 100, 125, 10,  32,  42,  32,  64,  105,
        110, 102, 111, 84,  105, 116, 108, 101, 32,  69,  120, 97,  109, 112,
        108, 101, 32,  65,  80,  73,  10,  32,  42,  32,  64,  105, 110, 102,
        111, 86,  101, 114, 115, 105, 111, 110, 32,  50,  46,  49,  46,  48,
        10,  32,  42,  32,  64,  105, 110, 102, 111, 83,  117, 109, 109, 97,
        114, 121, 32,  69,  120, 97,  109, 112, 108, 101, 32,  115, 117, 109,
        109, 97,  114, 121, 10,  32,  42,  32,  64,  105, 110, 102, 111, 68,
        101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 32,  69,  120, 97,
        109, 112, 108, 101, 32,  100, 101, 115, 99,  114, 105, 112, 116, 105,
        111, 110, 10,  32,  42,  32,  64,  116, 101, 114, 109, 115, 79,  102,
        83,  101, 114, 118, 105, 99,  101, 32,  104, 116, 116, 112, 115, 58,
        47,  47,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109, 47,
        116, 101, 114, 109, 115, 10,  32,  42,  32,  64,  99,  111, 110, 116,
        97,  99,  116, 32,  91,  110, 97,  109, 101, 58,  65,  80,  73,  32,
        83,  117, 112, 112, 111, 114, 116, 93,  32,  91,  117, 114, 108, 58,
        104, 116, 116, 112, 115, 58,  47,  47,  101, 120, 97,  109, 112, 108,
        101, 46,  99,  111, 109, 47,  115, 117, 112, 112, 111, 114, 116, 93,
        32,  91,  101, 109, 97,  105, 108, 58,  115, 117, 112, 112, 111, 114,
        116, 64,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109, 93,
        10,  32,  42,  32,  64,  108, 105, 99,  101, 110, 115, 101, 32,  91,
        110, 97,  109, 101, 58,  65,  112, 97,  99,  104, 101, 32,  50,  46,
        48,  93,  32,  91,  105, 100, 101, 110, 116, 105, 102, 105, 101, 114,
        58,  65,  112, 97,  99,  104, 101, 45,  50,  46,  48,  93,  10,  32,
        42,  32,  64,  115, 117, 109, 109, 97,  114, 121, 32,  71,  101, 116,
        32,  97,  32,  117, 115, 101, 114, 32,  98,  121, 32,  73,  68,  10,
        32,  42,  32,  64,  116, 97,  103, 32,  117, 115, 101, 114, 115, 10,
        32,  42,  32,  64,  116, 97,  103, 77,  101, 116, 97,  32,  101, 120,
        116, 101, 114, 110, 97,  108, 32,  91,  115, 117, 109, 109, 97,  114,
        121, 58,  69,  120, 116, 101, 114, 110, 97,  108, 93,  32,  91,  100,
        101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 58,  69,  120, 116,
        101, 114, 110, 97,  108, 32,  111, 112, 101, 114, 97,  116, 105, 111,
        110, 115, 93,  10,  32,  42,  32,  64,  116, 97,  103, 77,  101, 116,
        97,  32,  117, 115, 101, 114, 115, 32,  91,  115, 117, 109, 109, 97,
        114, 121, 58,  85,  115, 101, 114, 115, 93,  32,  91,  100, 101, 115,
        99,  114, 105, 112, 116, 105, 111, 110, 58,  85,  115, 101, 114, 32,
        111, 112, 101, 114, 97,  116, 105, 111, 110, 115, 93,  32,  91,  112,
        97,  114, 101, 110, 116, 58,  101, 120, 116, 101, 114, 110, 97,  108,
        93,  32,  91,  107, 105, 110, 100, 58,  110, 97,  118, 93,  32,  91,
        101, 120, 116, 101, 114, 110, 97,  108, 68,  111, 99,  115, 58,  104,
        116, 116, 112, 115, 58,  47,  47,  101, 120, 97,  109, 112, 108, 101,
        46,  99,  111, 109, 47,  100, 111, 99,  115, 93,  32,  91,  101, 120,
        116, 101, 114, 110, 97,  108, 68,  111, 99,  115, 68,  101, 115, 99,
        114, 105, 112, 116, 105, 111, 110, 58,  77,  111, 114, 101, 32,  100,
        111, 99,  115, 93,  10,  32,  42,  32,  64,  112, 97,  114, 97,  109,
        32,  105, 100, 32,  84,  104, 101, 32,  117, 115, 101, 114, 32,  73,
        68,  10,  32,  42,  47,  10,  105, 110, 116, 32,  97,  112, 105, 95,
        103, 101, 116, 95,  117, 115, 101, 114, 40,  105, 110, 116, 32,  105,
        100, 44,  32,  115, 116, 114, 117, 99,  116, 32,  85,  115, 101, 114,
        32,  42,  42,  111, 117, 116, 41,  32,  123, 10,  32,  32,  114, 101,
        116, 117, 114, 110, 32,  48,  59,  10,  125, 10,  10,  47,  42,  42,
        10,  32,  42,  32,  64,  114, 111, 117, 116, 101, 32,  80,  79,  83,
        84,  32,  47,  117, 115, 101, 114, 115, 10,  32,  42,  32,  64,  115,
        117, 109, 109, 97,  114, 121, 32,  67,  114, 101, 97,  116, 101, 32,
        97,  32,  117, 115, 101, 114, 10,  32,  42,  47,  10,  105, 110, 116,
        32,  97,  112, 105, 95,  99,  114, 101, 97,  116, 101, 95,  117, 115,
        101, 114, 40,  115, 116, 114, 117, 99,  116, 32,  85,  115, 101, 114,
        32,  42,  117, 41,  32,  123, 10,  32,  32,  114, 101, 116, 117, 114,
        110, 32,  48,  59,  10,  125, 10,  10,  47,  42,  42,  10,  32,  42,
        32,  64,  119, 101, 98,  104, 111, 111, 107, 32,  80,  79,  83,  84,
        32,  47,  117, 115, 101, 114, 45,  101, 118, 101, 110, 116, 115, 10,
        32,  42,  32,  64,  115, 117, 109, 109, 97,  114, 121, 32,  85,  115,
        101, 114, 32,  101, 118, 101, 110, 116, 32,  119, 101, 98,  104, 111,
        111, 107, 10,  32,  42,  47,  10,  105, 110, 116, 32,  97,  112, 105,
        95,  117, 115, 101, 114, 95,  101, 118, 101, 110, 116, 40,  115, 116,
        114, 117, 99,  116, 32,  85,  115, 101, 114, 32,  42,  117, 41,  32,
        123, 10,  32,  32,  114, 101, 116, 117, 114, 110, 32,  48,  59,  10,
        125, 10,  0};
    write_to_file(c_file, _tmp);
  }

  /* 3. Run CLI */
  {
    /* C90 compliant initialization */
    char *argv[5]; /* ... */

    argv[0] = cdd_test_mut_str("c2openapi");
    argv[1] = cdd_test_mut_str("--dialect");
    argv[2] = cdd_test_mut_str("https://spec.openapis.org/oas/3.1/"
                               "dialect/base");
    argv[3] = src_dir;
    argv[4] = out_json;

    rc = c2openapi_cli_main(5, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  /* 4. Verify JSON */
  {
    JSON_Value *root = NULL;
    JSON_Object *obj;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_file(out_json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    ASSERT(root != NULL);
    obj = json_value_get_object(root);

    /* Check Info */
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(obj, "openapi", &_cdd_js_1);
      ASSERT_STR_EQ("3.2.0", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(obj, "jsonSchemaDialect", &_cdd_js_1);
      ASSERT_STR_EQ("https://spec.openapis.org/oas/3.1/dialect/base",
                    _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.title", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("Example API", _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.version", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("2.1.0", _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.summary", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("Example summary", _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.description", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("Example description", _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.termsOfService",
                                       &_cdd_jv_1) == CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("https://example.com/terms", _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.contact.name", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("API Support", _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.contact.url", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("https://example.com/support", _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.contact.email", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("support@example.com", _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.license.name", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("Apache 2.0", _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.license.identifier",
                                       &_cdd_jv_1) == CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("Apache-2.0", _cdd_js_1);
    }

    /* Check Components (Struct User) */
    {
      JSON_Object *comps = NULL;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(obj, "components", &_cdd_jo_1);
        comps = _cdd_jo_1;
      }
      ASSERT(comps != NULL);
      {
        JSON_Object *schemas = NULL;
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(comps, "schemas", &_cdd_jo_1);
          schemas = _cdd_jo_1;
        }
        ASSERT(schemas != NULL);
        {
          JSON_Object *user = NULL;
          {
            JSON_Object *_cdd_jo_1 = NULL;
            cdd_json_object_get_object(schemas, "User", &_cdd_jo_1);
            user = _cdd_jo_1;
          }
          ASSERT(user != NULL);
          {
            JSON_Object *props = NULL;
            {
              JSON_Object *_cdd_jo_1 = NULL;
              cdd_json_object_get_object(user, "properties", &_cdd_jo_1);
              props = _cdd_jo_1;
            }
            ASSERT(props != NULL);
            {
              JSON_Object *id = NULL;
              {
                JSON_Object *_cdd_jo_1 = NULL;
                cdd_json_object_get_object(props, "id", &_cdd_jo_1);
                id = _cdd_jo_1;
              }
              ASSERT(id != NULL);
              {
                const char *type = NULL;
                {
                  const char *_cdd_js_1 = NULL;
                  cdd_json_object_get_string(id, "type", &_cdd_js_1);
                  type = _cdd_js_1;
                }

                ASSERT_STR_EQ("integer", (type) ? (type) : "NULL");
              }
            }
          }
        }
      }
    }

    /* Check GET /users/{id} */
    {
      JSON_Object *op = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        JSON_Object *_cdd_jo_1 = NULL;
        if (cdd_json_object_dotget_value(obj, "paths./users/{id}.get",
                                         &_cdd_jv_1) == CDD_C_SUCCESS &&
            _cdd_jv_1 && _cdd_jv_1->type == JSONObject)
          _cdd_jo_1 = _cdd_jv_1->object_val;
        op = _cdd_jo_1;
      }
      ASSERT(op != NULL);
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(op, "operationId", &_cdd_js_1);
        ASSERT_STR_EQ("api_get_user", _cdd_js_1);
      }

      /* Check Param */
      {
        JSON_Array *params = NULL;
        JSON_Object *p0 = NULL;
        {
          JSON_Array *_cdd_ja_1 = NULL;
          cdd_json_object_get_array(op, "parameters", &_cdd_ja_1);
          params = _cdd_ja_1;
        }
        p0 = json_array_get_object(params, 0);
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(p0, "name", &_cdd_js_1);
          ASSERT_STR_EQ("id", _cdd_js_1);
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(p0, "in", &_cdd_js_1);
          ASSERT_STR_EQ("path", _cdd_js_1);
        }
      }

      /* Check Response (200 User) from Output Param */
      {
        JSON_Object *responses = NULL;
        JSON_Object *r200 = NULL;
        JSON_Object *content = NULL;
        JSON_Object *app_json = NULL;
        JSON_Object *schema = NULL;
        const char *ref = NULL;
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(op, "responses", &_cdd_jo_1);
          responses = _cdd_jo_1;
        }
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(responses, "200", &_cdd_jo_1);
          r200 = _cdd_jo_1;
        }
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(r200, "content", &_cdd_jo_1);
          content = _cdd_jo_1;
        }
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(content, "application/json", &_cdd_jo_1);
          app_json = _cdd_jo_1;
        }
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(app_json, "schema", &_cdd_jo_1);
          schema = _cdd_jo_1;
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(schema, "$ref", &_cdd_js_1);
          ref = _cdd_js_1;
        }
        ASSERT_STR_EQ("#/components/schemas/User", (ref) ? (ref) : "NULL");
      }
    }

    /* Check POST /users */
    {
      JSON_Object *op = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        JSON_Object *_cdd_jo_1 = NULL;
        if (cdd_json_object_dotget_value(obj, "paths./users.post",
                                         &_cdd_jv_1) == CDD_C_SUCCESS &&
            _cdd_jv_1 && _cdd_jv_1->type == JSONObject)
          _cdd_jo_1 = _cdd_jv_1->object_val;
        op = _cdd_jo_1;
      }
      ASSERT(op != NULL);
      /* Check Request Body */
      {
        JSON_Object *req_body = NULL;
        JSON_Object *content = NULL;
        JSON_Object *app_json = NULL;
        JSON_Object *schema = NULL;
        const char *ref = NULL;
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(op, "requestBody", &_cdd_jo_1);
          req_body = _cdd_jo_1;
        }
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(req_body, "content", &_cdd_jo_1);
          content = _cdd_jo_1;
        }
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(content, "application/json", &_cdd_jo_1);
          app_json = _cdd_jo_1;
        }
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(app_json, "schema", &_cdd_jo_1);
          schema = _cdd_jo_1;
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(schema, "$ref", &_cdd_js_1);
          ref = _cdd_js_1;
        }
        ASSERT_STR_EQ("#/components/schemas/User", (ref) ? (ref) : "NULL");
      }

      /* Check Tags (top-level) */
      {
        JSON_Array *tags = NULL;
        JSON_Object *tag_users = NULL;
        JSON_Object *tag_external = NULL;
        size_t t;
        {
          JSON_Array *_cdd_ja_1 = NULL;
          cdd_json_object_get_array(obj, "tags", &_cdd_ja_1);
          tags = _cdd_ja_1;
        }
        ASSERT(tags != NULL);
        ASSERT(json_array_get_count(tags) >= 2);
        for (t = 0; t < json_array_get_count(tags); ++t) {
          JSON_Object *tag_obj = json_array_get_object(tags, t);
          const char *name = NULL;
          {
            const char *_cdd_js_1 = NULL;
            cdd_json_object_get_string(tag_obj, "name", &_cdd_js_1);
            name = _cdd_js_1;
          }
          if (name && strcmp(name, "users") == 0) {
            tag_users = tag_obj;
          } else if (name && strcmp(name, "external") == 0) {
            tag_external = tag_obj;
          }
        }
        ASSERT(tag_users != NULL);
        ASSERT(tag_external != NULL);
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(tag_users, "summary", &_cdd_js_1);
          ASSERT_STR_EQ("Users", _cdd_js_1);
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(tag_users, "description", &_cdd_js_1);
          ASSERT_STR_EQ("User operations", _cdd_js_1);
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(tag_users, "parent", &_cdd_js_1);
          ASSERT_STR_EQ("external", _cdd_js_1);
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(tag_users, "kind", &_cdd_js_1);
          ASSERT_STR_EQ("nav", _cdd_js_1);
        }
        {
          JSON_Value *_cdd_jv_1 = NULL;
          const char *_cdd_js_1 = NULL;
          if (cdd_json_object_dotget_value(tag_users, "externalDocs.url",
                                           &_cdd_jv_1) == CDD_C_SUCCESS &&
              _cdd_jv_1 && _cdd_jv_1->type == JSONString)
            _cdd_js_1 = _cdd_jv_1->string_val;
          ASSERT_STR_EQ("https://example.com/docs", _cdd_js_1);
        }
        {
          JSON_Value *_cdd_jv_1 = NULL;
          const char *_cdd_js_1 = NULL;
          if (cdd_json_object_dotget_value(tag_users,
                                           "externalDocs.description",
                                           &_cdd_jv_1) == CDD_C_SUCCESS &&
              _cdd_jv_1 && _cdd_jv_1->type == JSONString)
            _cdd_js_1 = _cdd_jv_1->string_val;
          ASSERT_STR_EQ("More docs", _cdd_js_1);
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(tag_external, "summary", &_cdd_js_1);
          ASSERT_STR_EQ("External", _cdd_js_1);
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(tag_external, "description", &_cdd_js_1);
          ASSERT_STR_EQ("External operations", _cdd_js_1);
        }
      }

      /* Check Webhooks */
      {
        {
          JSON_Value *_cdd_jv_1 = NULL;
          JSON_Object *_cdd_jo_1 = NULL;
          if (cdd_json_object_dotget_value(obj, "webhooks./user-events.post",
                                           &_cdd_jv_1) == CDD_C_SUCCESS &&
              _cdd_jv_1 && _cdd_jv_1->type == JSONObject)
            _cdd_jo_1 = _cdd_jv_1->object_val;
          op = _cdd_jo_1;
        }
        ASSERT(op != NULL);
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(op, "operationId", &_cdd_js_1);
          ASSERT_STR_EQ("api_user_event", _cdd_js_1);
        }
      }
    }

    json_value_free(root);
  }

  /* Cleanup */
  remove(c_file);
  remove(h_file);
  remove(out_json);
  rmdir(src_dir);
  free(c_file);
  free(h_file);
  free(out_json);
  free(src_dir);
  free(tmp_dir);
  g_fail_io_after = -1;

  rc += 0;
  PASS();
}

/**
 * @brief Unit test test_c2openapi_with_base_spec.
 * @return Greatest test result.
 */
TEST test_c2openapi_with_base_spec(void) {
  int rc = 0;
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *h_file = NULL;
  char *out_json = NULL;
  char *base_json = NULL;

  rc += 0;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_base_%d", tmp_dir, PATH_SEP_C, rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&h_file, "%s%cmodels.h", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);
  asprintf(&base_json, "%s%cbase.json", src_dir, PATH_SEP_C);

  write_to_file(h_file, "struct User { int id; char *name; };\n");

  {
    const char _tmp[] = {
        35,  105, 110, 99,  108, 117, 100, 101, 32,  34,  109, 111, 100, 101,
        108, 115, 46,  104, 34,  10,  47,  42,  42,  10,  32,  42,  32,  64,
        114, 111, 117, 116, 101, 32,  71,  69,  84,  32,  47,  117, 115, 101,
        114, 115, 47,  123, 105, 100, 125, 10,  32,  42,  32,  64,  115, 117,
        109, 109, 97,  114, 121, 32,  71,  101, 116, 32,  97,  32,  117, 115,
        101, 114, 32,  98,  121, 32,  73,  68,  10,  32,  42,  32,  64,  116,
        97,  103, 32,  117, 115, 101, 114, 115, 10,  32,  42,  32,  64,  112,
        97,  114, 97,  109, 32,  105, 100, 32,  91,  105, 110, 58,  112, 97,
        116, 104, 93,  32,  84,  104, 101, 32,  117, 115, 101, 114, 32,  73,
        68,  10,  32,  42,  47,  10,  105, 110, 116, 32,  97,  112, 105, 95,
        103, 101, 116, 95,  117, 115, 101, 114, 40,  105, 110, 116, 32,  105,
        100, 44,  32,  115, 116, 114, 117, 99,  116, 32,  85,  115, 101, 114,
        32,  42,  42,  111, 117, 116, 41,  32,  123, 10,  32,  32,  114, 101,
        116, 117, 114, 110, 32,  48,  59,  10,  125, 10,  0};
    write_to_file(c_file, _tmp);
  }

  {
    const char _tmp[] = {
        123, 10,  32,  32,  34,  111, 112, 101, 110, 97,  112, 105, 34,  58,
        32,  34,  51,  46,  50,  46,  48,  34,  44,  10,  32,  32,  34,  36,
        115, 101, 108, 102, 34,  58,  32,  34,  104, 116, 116, 112, 115, 58,
        47,  47,  101, 120, 97,  109, 112, 108, 101, 46,  99,  111, 109, 47,
        111, 112, 101, 110, 97,  112, 105, 46,  106, 115, 111, 110, 34,  44,
        10,  32,  32,  34,  106, 115, 111, 110, 83,  99,  104, 101, 109, 97,
        68,  105, 97,  108, 101, 99,  116, 34,  58,  32,  34,  104, 116, 116,
        112, 115, 58,  47,  47,  115, 112, 101, 99,  46,  111, 112, 101, 110,
        97,  112, 105, 115, 46,  111, 114, 103, 47,  111, 97,  115, 47,  51,
        46,  49,  47,  100, 105, 97,  108, 101, 99,  116, 47,  98,  97,  115,
        101, 34,  44,  10,  32,  32,  34,  105, 110, 102, 111, 34,  58,  32,
        123, 34,  116, 105, 116, 108, 101, 34,  58,  32,  34,  66,  97,  115,
        101, 32,  65,  80,  73,  34,  44,  32,  34,  118, 101, 114, 115, 105,
        111, 110, 34,  58,  32,  34,  57,  46,  57,  46,  57,  34,  44,  32,
        34,  115, 117, 109, 109, 97,  114, 121, 34,  58,  32,  34,  66,  97,
        115, 101, 32,  115, 117, 109, 109, 97,  114, 121, 34,  125, 44,  10,
        32,  32,  34,  115, 101, 114, 118, 101, 114, 115, 34,  58,  32,  91,
        123, 34,  117, 114, 108, 34,  58,  32,  34,  104, 116, 116, 112, 115,
        58,  47,  47,  97,  112, 105, 46,  101, 120, 97,  109, 112, 108, 101,
        46,  99,  111, 109, 47,  118, 49,  34,  44,  32,  34,  110, 97,  109,
        101, 34,  58,  32,  34,  112, 114, 111, 100, 34,  125, 93,  44,  10,
        32,  32,  34,  116, 97,  103, 115, 34,  58,  32,  91,  123, 34,  110,
        97,  109, 101, 34,  58,  32,  34,  117, 115, 101, 114, 115, 34,  44,
        32,  34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,
        58,  32,  34,  85,  115, 101, 114, 32,  111, 112, 101, 114, 97,  116,
        105, 111, 110, 115, 34,  44,  32,  34,  107, 105, 110, 100, 34,  58,
        32,  34,  110, 97,  118, 34,  125, 93,  44,  10,  32,  32,  34,  99,
        111, 109, 112, 111, 110, 101, 110, 116, 115, 34,  58,  32,  123, 10,
        32,  32,  32,  32,  34,  115, 101, 99,  117, 114, 105, 116, 121, 83,
        99,  104, 101, 109, 101, 115, 34,  58,  32,  123, 10,  32,  32,  32,
        32,  32,  32,  34,  97,  112, 105, 95,  107, 101, 121, 34,  58,  32,
        123, 34,  116, 121, 112, 101, 34,  58,  32,  34,  97,  112, 105, 75,
        101, 121, 34,  44,  32,  34,  110, 97,  109, 101, 34,  58,  32,  34,
        88,  45,  65,  80,  73,  45,  75,  101, 121, 34,  44,  32,  34,  105,
        110, 34,  58,  32,  34,  104, 101, 97,  100, 101, 114, 34,  125, 10,
        32,  32,  32,  32,  125, 10,  32,  32,  125, 44,  10,  32,  32,  34,
        112, 97,  116, 104, 115, 34,  58,  32,  123, 125, 10,  125, 10,  0};
    write_to_file(base_json, _tmp);
  }

  {
    char *argv[5]; /* ... */

    argv[0] = cdd_test_mut_str("c2openapi");
    argv[1] = cdd_test_mut_str("--base");
    argv[2] = base_json;
    argv[3] = src_dir;
    argv[4] = out_json;
    rc = c2openapi_cli_main(5, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  {
    JSON_Value *root = NULL;
    JSON_Object *obj;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_file(out_json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    ASSERT(root != NULL);
    obj = json_value_get_object(root);

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(obj, "openapi", &_cdd_js_1);
      ASSERT_STR_EQ("3.2.0", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(obj, "$self", &_cdd_js_1);
      ASSERT_STR_EQ("https://example.com/openapi.json", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(obj, "jsonSchemaDialect", &_cdd_js_1);
      ASSERT_STR_EQ("https://spec.openapis.org/oas/3.1/dialect/base",
                    _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.title", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("Base API", _cdd_js_1);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "info.version", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("9.9.9", _cdd_js_1);
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      const char *_cdd_js_2 = NULL;
      cdd_json_object_get_array(obj, "servers", &_cdd_ja_1);
      cdd_json_object_get_string(json_array_get_object(_cdd_ja_1, 0), "url",
                                 &_cdd_js_2);
      ASSERT_STR_EQ("https://api.example.com/v1", _cdd_js_2);
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      const char *_cdd_js_2 = NULL;
      cdd_json_object_get_array(obj, "tags", &_cdd_ja_1);
      cdd_json_object_get_string(json_array_get_object(_cdd_ja_1, 0),
                                 "description", &_cdd_js_2);
      ASSERT_STR_EQ("User operations", _cdd_js_2);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(
              obj, "components.securitySchemes.api_key.type", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("apiKey", _cdd_js_1);
    }

    {
      JSON_Object *op = NULL;
      {
        JSON_Value *_cdd_jv_1 = NULL;
        JSON_Object *_cdd_jo_1 = NULL;
        if (cdd_json_object_dotget_value(obj, "paths./users/{id}.get",
                                         &_cdd_jv_1) == CDD_C_SUCCESS &&
            _cdd_jv_1 && _cdd_jv_1->type == JSONObject)
          _cdd_jo_1 = _cdd_jv_1->object_val;
        op = _cdd_jo_1;
      }
      ASSERT(op != NULL);
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(op, "operationId", &_cdd_js_1);
        ASSERT_STR_EQ("api_get_user", _cdd_js_1);
      }
    }

    json_value_free(root);
  }

  remove(c_file);
  remove(h_file);
  remove(out_json);
  remove(base_json);
  rmdir(src_dir);
  free(c_file);
  free(h_file);
  free(out_json);
  free(base_json);
  free(src_dir);
  free(tmp_dir);
  g_fail_io_after = -1;

  rc += 0;
  PASS();
}

/**
 * @brief Unit test test_c2openapi_with_self_uri.
 * @return Greatest test result.
 */
TEST test_c2openapi_with_self_uri(void) {
  int rc = 0;
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *h_file = NULL;
  char *out_json = NULL;

  rc += 0;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_self_%d", tmp_dir, PATH_SEP_C, rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&h_file, "%s%cmodels.h", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(h_file, "struct User { int id; char *name; };\n");
  write_to_file(c_file,
                "#include \"models.h\"\n"
                "/**\n"
                " * @route GET /users\n"
                " * @summary List users\n"
                " */\n"
                ""
                "int api_list_users(struct User **out) { return 0; }\n");

  {
    char *argv[5]; /* ... */

    argv[0] = cdd_test_mut_str("c2openapi");
    argv[1] = cdd_test_mut_str("--self");
    argv[2] = cdd_test_mut_str("https://example.com/override.json");
    argv[3] = src_dir;
    argv[4] = out_json;
    rc = c2openapi_cli_main(5, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  {
    JSON_Value *root = NULL;
    JSON_Object *obj;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_file(out_json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    ASSERT(root != NULL);
    obj = json_value_get_object(root);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(obj, "$self", &_cdd_js_1);
      ASSERT_STR_EQ("https://example.com/override.json", _cdd_js_1);
    }
    json_value_free(root);
  }

  remove(c_file);
  remove(h_file);
  remove(out_json);
  rmdir(src_dir);
  free(c_file);
  free(h_file);
  free(out_json);
  free(src_dir);
  free(tmp_dir);
  g_fail_io_after = -1;

  rc += 0;
  PASS();
}

/**
 * @brief Unit test test_c2openapi_global_meta_security_schemes.
 * @return Greatest test result.
 */
TEST test_c2openapi_global_meta_security_schemes(void) {
  int rc = 0;
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  rc += 0;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_global_%d", tmp_dir, PATH_SEP_C, rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  {
    const char _tmp[] = {
        47,  42,  42,  10,  32,  42,  32,  64,  115, 101, 99,  117, 114, 105,
        116, 121, 83,  99,  104, 101, 109, 101, 32,  97,  112, 105, 95,  107,
        101, 121, 32,  91,  116, 121, 112, 101, 58,  97,  112, 105, 75,  101,
        121, 93,  32,  91,  112, 97,  114, 97,  109, 78,  97,  109, 101, 58,
        88,  45,  65,  80,  73,  45,  75,  101, 121, 93,  32,  91,  105, 110,
        58,  104, 101, 97,  100, 101, 114, 93,  10,  32,  42,  32,  64,  115,
        101, 99,  117, 114, 105, 116, 121, 32,  97,  112, 105, 95,  107, 101,
        121, 10,  32,  42,  32,  64,  115, 101, 114, 118, 101, 114, 32,  104,
        116, 116, 112, 115, 58,  47,  47,  97,  112, 105, 46,  101, 120, 97,
        109, 112, 108, 101, 46,  99,  111, 109, 32,  91,  110, 97,  109, 101,
        58,  112, 114, 111, 100, 93,  10,  32,  42,  32,  64,  101, 120, 116,
        101, 114, 110, 97,  108, 68,  111, 99,  115, 32,  104, 116, 116, 112,
        115, 58,  47,  47,  100, 111, 99,  115, 46,  101, 120, 97,  109, 112,
        108, 101, 46,  99,  111, 109, 32,  71,  108, 111, 98,  97,  108, 32,
        100, 111, 99,  115, 10,  32,  42,  47,  10,  105, 110, 116, 32,  112,
        108, 97,  99,  101, 104, 111, 108, 100, 101, 114, 32,  61,  32,  48,
        59,  10,  10,  47,  42,  42,  10,  32,  42,  32,  64,  114, 111, 117,
        116, 101, 32,  71,  69,  84,  32,  47,  112, 105, 110, 103, 10,  32,
        42,  32,  64,  115, 117, 109, 109, 97,  114, 121, 32,  80,  105, 110,
        103, 10,  32,  42,  47,  10,  105, 110, 116, 32,  97,  112, 105, 95,
        112, 105, 110, 103, 40,  118, 111, 105, 100, 41,  32,  123, 32,  114,
        101, 116, 117, 114, 110, 32,  48,  59,  32,  125, 10,  0};
    write_to_file(c_file, _tmp);
  }

  {
    char *argv[3]; /* ... */

    argv[0] = cdd_test_mut_str("c2openapi");
    argv[1] = src_dir;
    argv[2] = out_json;
    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  {
    JSON_Value *root = NULL;
    JSON_Object *obj;
    JSON_Object *scheme;
    JSON_Array *sec_arr;
    JSON_Object *sec_obj;
    JSON_Array *scopes;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_file(out_json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    ASSERT(root != NULL);
    obj = json_value_get_object(root);

    {
      JSON_Value *_cdd_jv_1 = NULL;
      JSON_Object *_cdd_jo_1 = NULL;
      if (cdd_json_object_dotget_value(obj,
                                       "components.securitySchemes.api_key",
                                       &_cdd_jv_1) == CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONObject)
        _cdd_jo_1 = _cdd_jv_1->object_val;
      scheme = _cdd_jo_1;
    }
    ASSERT(scheme != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(scheme, "type", &_cdd_js_1);
      ASSERT_STR_EQ("apiKey", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(scheme, "name", &_cdd_js_1);
      ASSERT_STR_EQ("X-API-Key", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(scheme, "in", &_cdd_js_1);
      ASSERT_STR_EQ("header", _cdd_js_1);
    }

    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(obj, "security", &_cdd_ja_1);
      sec_arr = _cdd_ja_1;
    }
    ASSERT(sec_arr != NULL);
    sec_obj = json_array_get_object(sec_arr, 0);
    ASSERT(sec_obj != NULL);
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(sec_obj, "api_key", &_cdd_ja_1);
      scopes = _cdd_ja_1;
    }
    ASSERT(scopes != NULL);
    ASSERT_EQ(0, json_array_get_count(scopes));

    {
      JSON_Array *_cdd_ja_1 = NULL;
      const char *_cdd_js_2 = NULL;
      cdd_json_object_get_array(obj, "servers", &_cdd_ja_1);
      cdd_json_object_get_string(json_array_get_object(_cdd_ja_1, 0), "url",
                                 &_cdd_js_2);
      ASSERT_STR_EQ("https://api.example.com", _cdd_js_2);
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      const char *_cdd_js_2 = NULL;
      cdd_json_object_get_array(obj, "servers", &_cdd_ja_1);
      cdd_json_object_get_string(json_array_get_object(_cdd_ja_1, 0),
                                 "description", &_cdd_js_2);
      ASSERT_STR_EQ("[name:prod]", _cdd_js_2);
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      const char *_cdd_js_1 = NULL;
      if (cdd_json_object_dotget_value(obj, "externalDocs.url", &_cdd_jv_1) ==
              CDD_C_SUCCESS &&
          _cdd_jv_1 && _cdd_jv_1->type == JSONString)
        _cdd_js_1 = _cdd_jv_1->string_val;
      ASSERT_STR_EQ("https://docs.example.com", _cdd_js_1);
    }

    json_value_free(root);
  }

  remove(c_file);
  remove(out_json);
  rmdir(src_dir);
  free(c_file);
  free(out_json);
  free(src_dir);
  free(tmp_dir);
  g_fail_io_after = -1;

  rc += 0;
  PASS();
}

/**
 * @brief Executes the integration_c2openapi_suite test suite.
 */
SUITE(integration_c2openapi_suite) {
  RUN_TEST(test_c2openapi_full_flow);
  RUN_TEST(test_c2openapi_with_base_spec);
  RUN_TEST(test_c2openapi_with_self_uri);
  RUN_TEST(test_c2openapi_global_meta_security_schemes);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_INTEGRATION_C2OPENAPI_H */
