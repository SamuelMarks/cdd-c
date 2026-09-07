/**
 * @file test_integration_c2openapi.h
 * @brief Integration tests for C to OpenAPI conversion.
 */

#ifndef TEST_INTEGRATION_C2OPENAPI_H
#define TEST_INTEGRATION_C2OPENAPI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__GNUC__) || defined(__clang__)
#endif

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

TEST test_c2openapi_full_flow(void) {
  int rc;
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *h_file = NULL;
  char *out_json = NULL;

  /* 0. Setup Directories */
  (void)rc;
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

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)(size_t) "--dialect";
    argv[2] = (char *)(size_t)(size_t) "https://spec.openapis.org/oas/3.1/"
                                       "dialect/base";
    argv[3] = src_dir;
    argv[4] = out_json;

    rc = c2openapi_cli_main(5, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  /* 4. Verify JSON */
  {
    JSON_Value *root = json_parse_file(out_json);
    JSON_Object *obj;
    ASSERT(root != NULL);
    obj = json_value_get_object(root);

    /* Check Info */
    ASSERT_STR_EQ("3.2.0", json_object_get_string(obj, "openapi"));
    ASSERT_STR_EQ("https://spec.openapis.org/oas/3.1/dialect/base",
                  json_object_get_string(obj, "jsonSchemaDialect"));
    ASSERT_STR_EQ("Example API", json_object_dotget_string(obj, "info.title"));
    ASSERT_STR_EQ("2.1.0", json_object_dotget_string(obj, "info.version"));
    ASSERT_STR_EQ("Example summary",
                  json_object_dotget_string(obj, "info.summary"));
    ASSERT_STR_EQ("Example description",
                  json_object_dotget_string(obj, "info.description"));
    ASSERT_STR_EQ("https://example.com/terms",
                  json_object_dotget_string(obj, "info.termsOfService"));
    ASSERT_STR_EQ("API Support",
                  json_object_dotget_string(obj, "info.contact.name"));
    ASSERT_STR_EQ("https://example.com/support",
                  json_object_dotget_string(obj, "info.contact.url"));
    ASSERT_STR_EQ("support@example.com",
                  json_object_dotget_string(obj, "info.contact.email"));
    ASSERT_STR_EQ("Apache 2.0",
                  json_object_dotget_string(obj, "info.license.name"));
    ASSERT_STR_EQ("Apache-2.0",
                  json_object_dotget_string(obj, "info.license.identifier"));

    /* Check Components (Struct User) */
    {
      JSON_Object *comps = json_object_get_object(obj, "components");
      ASSERT(comps != NULL);
      {
        JSON_Object *schemas = json_object_get_object(comps, "schemas");
        ASSERT(schemas != NULL);
        {
          JSON_Object *user = json_object_get_object(schemas, "User");
          if (!user) {
            printf("FAILED to find User in schemas!\n");
            {
              size_t num = json_object_get_count(schemas);
              printf("Schemas has %lu items. Keys:\n", (unsigned long)num);
              {
                size_t i;
                for (i = 0; i < num; ++i) {
                  printf("  '%s'\n", json_object_get_name(schemas, i));
                }
              }
            }
          }
          ASSERT(user != NULL);
          {
            JSON_Object *props = json_object_get_object(user, "properties");
            ASSERT(props != NULL);
            {
              JSON_Object *id = json_object_get_object(props, "id");
              ASSERT(id != NULL);
              {
                const char *type = json_object_get_string(id, "type");

                ASSERT_STR_EQ("integer", (type) ? (type) : "NULL");
              }
            }
          }
        }
      }
    }

    /* Check GET /users/{id} */
    {
      JSON_Object *op = json_object_dotget_object(obj, "paths./users/{id}.get");
      ASSERT(op != NULL);
      ASSERT_STR_EQ("api_get_user", json_object_get_string(op, "operationId"));

      /* Check Param */
      {
        JSON_Array *params = json_object_get_array(op, "parameters");
        JSON_Object *p0 = json_array_get_object(params, 0);
        ASSERT_STR_EQ("id", json_object_get_string(p0, "name"));
        ASSERT_STR_EQ("path", json_object_get_string(p0, "in"));
      }

      /* Check Response (200 User) from Output Param */
      {
        JSON_Object *responses = json_object_get_object(op, "responses");
        JSON_Object *r200 = json_object_get_object(responses, "200");
        JSON_Object *content = json_object_get_object(r200, "content");
        JSON_Object *app_json =
            json_object_get_object(content, "application/json");
        JSON_Object *schema = json_object_get_object(app_json, "schema");
        const char *ref = json_object_get_string(schema, "$ref");
        ASSERT_STR_EQ("#/components/schemas/User", (ref) ? (ref) : "NULL");
      }
    }

    /* Check POST /users */
    {
      JSON_Object *op = json_object_dotget_object(obj, "paths./users.post");
      ASSERT(op != NULL);
      /* Check Request Body */
      {
        JSON_Object *req_body = json_object_get_object(op, "requestBody");
        JSON_Object *content = json_object_get_object(req_body, "content");
        JSON_Object *app_json =
            json_object_get_object(content, "application/json");
        JSON_Object *schema = json_object_get_object(app_json, "schema");
        const char *ref = json_object_get_string(schema, "$ref");
        ASSERT_STR_EQ("#/components/schemas/User", (ref) ? (ref) : "NULL");
      }

      /* Check Tags (top-level) */
      {
        JSON_Array *tags = json_object_get_array(obj, "tags");
        JSON_Object *tag_users = NULL;
        JSON_Object *tag_external = NULL;
        size_t t;
        ASSERT(tags != NULL);
        ASSERT(json_array_get_count(tags) >= 2);
        for (t = 0; t < json_array_get_count(tags); ++t) {
          JSON_Object *tag_obj = json_array_get_object(tags, t);
          const char *name = json_object_get_string(tag_obj, "name");
          if (name && strcmp(name, "users") == 0) {
            tag_users = tag_obj;
          } else if (name && strcmp(name, "external") == 0) {
            tag_external = tag_obj;
          }
        }
        ASSERT(tag_users != NULL);
        ASSERT(tag_external != NULL);
        ASSERT_STR_EQ("Users", json_object_get_string(tag_users, "summary"));
        ASSERT_STR_EQ("User operations",
                      json_object_get_string(tag_users, "description"));
        ASSERT_STR_EQ("external", json_object_get_string(tag_users, "parent"));
        ASSERT_STR_EQ("nav", json_object_get_string(tag_users, "kind"));
        ASSERT_STR_EQ("https://example.com/docs",
                      json_object_dotget_string(tag_users, "externalDocs.url"));
        ASSERT_STR_EQ("More docs", json_object_dotget_string(
                                       tag_users, "externalDocs.description"));
        ASSERT_STR_EQ("External",
                      json_object_get_string(tag_external, "summary"));
        ASSERT_STR_EQ("External operations",
                      json_object_get_string(tag_external, "description"));
      }

      /* Check Webhooks */
      {
        op = json_object_dotget_object(obj, "webhooks./user-events.post");
        ASSERT(op != NULL);
        ASSERT_STR_EQ("api_user_event",
                      json_object_get_string(op, "operationId"));
      }
    }

    json_value_free(root);
  }

  /* Cleanup */
  remove(c_file);
  remove(h_file);
  remove(out_json);
  rmdir(src_dir);
  free((void *)(size_t)c_file);
  free((void *)(size_t)h_file);
  free((void *)(size_t)out_json);
  free((void *)(size_t)src_dir);
  free((void *)(size_t)tmp_dir);
  g_fail_io_after = -1;

  (void)rc;
  PASS();
}

TEST test_c2openapi_with_base_spec(void) {
  int rc;
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *h_file = NULL;
  char *out_json = NULL;
  char *base_json = NULL;

  (void)rc;
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

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)(size_t) "--base";
    argv[2] = base_json;
    argv[3] = src_dir;
    argv[4] = out_json;
    rc = c2openapi_cli_main(5, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  {
    JSON_Value *root = json_parse_file(out_json);
    JSON_Object *obj;
    ASSERT(root != NULL);
    obj = json_value_get_object(root);

    ASSERT_STR_EQ("3.2.0", json_object_get_string(obj, "openapi"));
    ASSERT_STR_EQ("https://example.com/openapi.json",
                  json_object_get_string(obj, "$self"));
    ASSERT_STR_EQ("https://spec.openapis.org/oas/3.1/dialect/base",
                  json_object_get_string(obj, "jsonSchemaDialect"));
    ASSERT_STR_EQ("Base API", json_object_dotget_string(obj, "info.title"));
    ASSERT_STR_EQ("9.9.9", json_object_dotget_string(obj, "info.version"));
    ASSERT_STR_EQ(
        "https://api.example.com/v1",
        json_object_get_string(
            json_array_get_object(json_object_get_array(obj, "servers"), 0),
            "url"));
    ASSERT_STR_EQ(
        "User operations",
        json_object_get_string(
            json_array_get_object(json_object_get_array(obj, "tags"), 0),
            "description"));
    ASSERT_STR_EQ("apiKey",
                  json_object_dotget_string(
                      obj, "components.securitySchemes.api_key.type"));

    {
      JSON_Object *op = json_object_dotget_object(obj, "paths./users/{id}.get");
      ASSERT(op != NULL);
      ASSERT_STR_EQ("api_get_user", json_object_get_string(op, "operationId"));
    }

    json_value_free(root);
  }

  remove(c_file);
  remove(h_file);
  remove(out_json);
  remove(base_json);
  rmdir(src_dir);
  free((void *)(size_t)c_file);
  free((void *)(size_t)h_file);
  free((void *)(size_t)out_json);
  free((void *)(size_t)base_json);
  free((void *)(size_t)src_dir);
  free((void *)(size_t)tmp_dir);
  g_fail_io_after = -1;

  (void)rc;
  PASS();
}

TEST test_c2openapi_with_self_uri(void) {
  int rc;
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *h_file = NULL;
  char *out_json = NULL;

  (void)rc;
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

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)(size_t) "--self";
    argv[2] = (char *)(size_t)(size_t) "https://example.com/override.json";
    argv[3] = src_dir;
    argv[4] = out_json;
    rc = c2openapi_cli_main(5, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  {
    JSON_Value *root = json_parse_file(out_json);
    JSON_Object *obj;
    ASSERT(root != NULL);
    obj = json_value_get_object(root);
    ASSERT_STR_EQ("https://example.com/override.json",
                  json_object_get_string(obj, "$self"));
    json_value_free(root);
  }

  remove(c_file);
  remove(h_file);
  remove(out_json);
  rmdir(src_dir);
  free((void *)(size_t)c_file);
  free((void *)(size_t)h_file);
  free((void *)(size_t)out_json);
  free((void *)(size_t)src_dir);
  free((void *)(size_t)tmp_dir);
  g_fail_io_after = -1;

  (void)rc;
  PASS();
}

TEST test_c2openapi_global_meta_security_schemes(void) {
  int rc;
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
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

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = src_dir;
    argv[2] = out_json;
    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }

  {
    JSON_Value *root = json_parse_file(out_json);
    JSON_Object *obj;
    JSON_Object *scheme;
    JSON_Array *sec_arr;
    JSON_Object *sec_obj;
    JSON_Array *scopes;
    ASSERT(root != NULL);
    obj = json_value_get_object(root);

    scheme =
        json_object_dotget_object(obj, "components.securitySchemes.api_key");
    ASSERT(scheme != NULL);
    ASSERT_STR_EQ("apiKey", json_object_get_string(scheme, "type"));
    ASSERT_STR_EQ("X-API-Key", json_object_get_string(scheme, "name"));
    ASSERT_STR_EQ("header", json_object_get_string(scheme, "in"));

    sec_arr = json_object_get_array(obj, "security");
    ASSERT(sec_arr != NULL);
    sec_obj = json_array_get_object(sec_arr, 0);
    ASSERT(sec_obj != NULL);
    scopes = json_object_get_array(sec_obj, "api_key");
    ASSERT(scopes != NULL);
    ASSERT_EQ(0, json_array_get_count(scopes));

    ASSERT_STR_EQ(
        "https://api.example.com",
        json_object_get_string(
            json_array_get_object(json_object_get_array(obj, "servers"), 0),
            "url"));
    ASSERT_STR_EQ(
        "[name:prod]",
        json_object_get_string(
            json_array_get_object(json_object_get_array(obj, "servers"), 0),
            "description"));
    ASSERT_STR_EQ("https://docs.example.com",
                  json_object_dotget_string(obj, "externalDocs.url"));

    json_value_free(root);
  }

  remove(c_file);
  remove(out_json);
  rmdir(src_dir);
  free((void *)(size_t)c_file);
  free((void *)(size_t)out_json);
  free((void *)(size_t)src_dir);
  free((void *)(size_t)tmp_dir);
  g_fail_io_after = -1;

  (void)rc;
  PASS();
}

TEST test_c2o_cli_source_file_checks(void) {
  int rc;
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *txt_file = NULL;
  char *no_ext_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf(&txt_file, "%s%cnotes.txt", src_dir, PATH_SEP_C);
  asprintf(&no_ext_file, "%s%cREADME", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, "int foo(void);\n");
  write_to_file(txt_file, "just some notes");
  write_to_file(no_ext_file, "no extension here");

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    if (rc != 0) {
      printf("\nERROR rc=%d\n", rc);
    }
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(txt_file);
    remove(no_ext_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free(txt_file);
    free(no_ext_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_doc_sec_unset(void) {
  int rc;
  const char *snippets[] = {
      "/**\n * @securityScheme my_bad_sec\n */\nint foo1(void);\n",
      "/**\n * @securityScheme my_bad_sec2 [type:unknownType]\n */\nint "
      "foo2(void);\n"
      "/**\n * @securityScheme my_http [type:http] [in:unknownIn]\n */\nint "
      "foo3(void);\n",
      "/**\n * @securityScheme my_apikey [type:apiKey] [in:unknownIn]\n "
      "*/\nint foo4(void);\n",
      "/**\n * @securityScheme my_apikey2 [type:apiKey]\n */\nint "
      "foo5(void);\n",
      "/**\n * @securityScheme my_oauth2 [type:oauth2] "
      "[flow:deviceAuthorization] "
      "[deviceAuthorizationUrl:https://auth.com/device] "
      "[tokenUrl:https://auth.com/token] [refreshUrl:https://auth.com/refresh] "
      "[scopes:scope1=Scope1]\n */\nint foo6(void);\n",
      "/**\n * @securityScheme my_oauth2_bad [type:oauth2] "
      "[flow:unknownFlow]\n */\nint foo7(void);\n",
      "/**\n * @securityScheme my_oauth2_unset [type:oauth2]\n */\nint "
      "foo8(void);\n",
      "/**\n * @securityScheme my_mutual [type:mutualTLS]\n */\nint "
      "foo9(void);\n",
      "/**\n * @securityScheme my_openid [type:openIdConnect] "
      "[openIdConnectUrl:https://auth.com/openid]\n */\nint foo10(void);\n",
      "/**\n * @securityScheme my_apikey_query [type:apiKey] [in:query] "
      "[name:foo]\n */\nint foo11(void);\n",
      "/**\n * @securityScheme my_apikey_cookie [type:apiKey] [in:cookie] "
      "[name:foo]\n */\nint foo12(void);\n",
      "/**\n * @securityScheme my_oauth2_implicit [type:oauth2] "
      "[flow:implicit] [authorizationUrl:https://auth.com/auth]\n */\nint "
      "foo13(void);\n",
      "/**\n * @securityScheme my_oauth2_password [type:oauth2] "
      "[flow:password] [tokenUrl:https://auth.com/token]\n */\nint "
      "foo14(void);\n",
      "/**\n * @securityScheme my_oauth2_client [type:oauth2] "
      "[flow:clientCredentials] [tokenUrl:https://auth.com/token]\n */\nint "
      "foo15(void);\n",
      "/**\n * @securityScheme my_oauth2_auth [type:oauth2] "
      "[flow:authorizationCode] [authorizationUrl:https://auth.com/auth] "
      "[tokenUrl:https://auth.com/token]\n */\nint foo16(void);\n"};

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  {
    size_t i;
    for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
      char *c_file = NULL;
      asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
               (unsigned long)i);
      write_to_file(c_file, snippets[i]);
      free((void *)(size_t)c_file);
    }

    {
      char *argv[3]; /* ... */

      argv[0] = (char *)(size_t)(size_t) "c2openapi";
      argv[1] = (char *)(size_t)src_dir;
      argv[2] = (char *)(size_t)out_json;
      rc = c2openapi_cli_main(3, argv);
      if (rc != 0) {
        printf("\nERROR rc=%d\n", rc);
      }
      ASSERT_EQ(0, rc);

      for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
        char *c_file = NULL;
        asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
                 (unsigned long)i);
        remove(c_file);
        free((void *)(size_t)c_file);
      }
      remove(out_json);
      rmdir(src_dir);
      free((void *)(size_t)out_json);
      free((void *)(size_t)src_dir);
      free((void *)(size_t)tmp_dir);
      g_fail_io_after = -1;

      (void)rc;
      PASS();
    }
  }
}

TEST test_c2o_cli_spec_has_tag_nulls(void) {
  int rc;
  const char *src = "/**\n"
                    " * @tag duplicated\n"
                    " * @tag duplicated\n"
                    " */\n"
                    "int foo(void);\n";

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    if (rc != 0) {
      printf("\nERROR rc=%d\n", rc);
    }
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_mappings_errors_find(void) {
  int rc;
  const char src[] = {
      47,  42,  42,  10,  32,  42,  32,  71,  76,  79,  66,  65,  76,  32,  77,
      69,  84,  65,  58,  10,  32,  42,  32,  64,  115, 101, 99,  117, 114, 105,
      116, 121, 83,  99,  104, 101, 109, 101, 32,  109, 121, 95,  104, 116, 116,
      112, 32,  91,  116, 121, 112, 101, 58,  104, 116, 116, 112, 93,  32,  91,
      115, 99,  104, 101, 109, 101, 58,  98,  101, 97,  114, 101, 114, 93,  32,
      91,  98,  101, 97,  114, 101, 114, 70,  111, 114, 109, 97,  116, 58,  74,
      87,  84,  93,  10,  32,  42,  32,  64,  115, 101, 99,  117, 114, 105, 116,
      121, 83,  99,  104, 101, 109, 101, 32,  109, 121, 95,  104, 116, 116, 112,
      32,  91,  116, 121, 112, 101, 58,  104, 116, 116, 112, 93,  32,  91,  115,
      99,  104, 101, 109, 101, 58,  98,  101, 97,  114, 101, 114, 93,  32,  91,
      98,  101, 97,  114, 101, 114, 70,  111, 114, 109, 97,  116, 58,  74,  87,
      84,  93,  10,  32,  42,  32,  64,  115, 101, 99,  117, 114, 105, 116, 121,
      83,  99,  104, 101, 109, 101, 32,  109, 121, 95,  111, 97,  117, 116, 104,
      50,  32,  91,  116, 121, 112, 101, 58,  111, 97,  117, 116, 104, 50,  93,
      32,  91,  102, 108, 111, 119, 58,  100, 101, 118, 105, 99,  101, 65,  117,
      116, 104, 111, 114, 105, 122, 97,  116, 105, 111, 110, 93,  32,  91,  97,
      117, 116, 104, 111, 114, 105, 122, 97,  116, 105, 111, 110, 85,  114, 108,
      58,  104, 116, 116, 112, 115, 58,  47,  47,  97,  117, 116, 104, 46,  99,
      111, 109, 47,  97,  117, 116, 104, 93,  32,  91,  116, 111, 107, 101, 110,
      85,  114, 108, 58,  104, 116, 116, 112, 115, 58,  47,  47,  97,  117, 116,
      104, 46,  99,  111, 109, 47,  116, 111, 107, 101, 110, 93,  32,  91,  114,
      101, 102, 114, 101, 115, 104, 85,  114, 108, 58,  104, 116, 116, 112, 115,
      58,  47,  47,  97,  117, 116, 104, 46,  99,  111, 109, 47,  114, 101, 102,
      114, 101, 115, 104, 93,  32,  91,  115, 99,  111, 112, 101, 115, 58,  115,
      99,  111, 112, 101, 49,  61,  83,  99,  111, 112, 101, 49,  93,  10,  32,
      42,  47,  10,  105, 110, 116, 32,  102, 111, 111, 40,  118, 111, 105, 100,
      41,  59,  10,  0};

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    if (rc != 0) {
      printf("\nERROR rc=%d\n", rc);
    }
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_set_str_mismatch(void) {
  int rc;
  const char *src = "/**\n"
                    " * @securityScheme my_http [type:http] [scheme:bearer]\n"
                    " * @securityScheme my_http [type:http] [scheme:basic]\n"
                    " */\n"
                    "int foo17(void);\n";

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    if (rc != 0) {
      printf("\nERROR rc=%d\n", rc);
    }
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_server_variables(void) {
  int rc;
  const char *src = "/**\n"
                    " * GLOBAL META:\n"
                    " * @server https://api.com [description:prod]\n"
                    " * @serverVar env [default:prod] [enum:prod,dev]\n"
                    " * @server https://api.com [description:mismatch_fail]\n"
                    " */\n"
                    "int foo18(void);\n";

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    if (rc != 0) {
      printf("\nERROR rc=%d\n", rc);
    }
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_server_variables_validation(void) {
  int rc;
  const char *src =
      "/**\n"
      " * GLOBAL META:\n"
      " * @server https://api.com [description:prod]\n"
      " * @serverVar env [default:prod] [enum:prod,dev] [description:desc]\n"
      " * @serverVar bad [default:wrong] [enum:prod,dev] [description:fail]\n"
      " */\n"
      "int foo19(void);\n";

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    if (rc != 0) {
      printf("\nERROR rc=%d\n", rc);
    }
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_merge_oauth_scopes(void) {
  int rc;
  const char src[] = {
      47,  42,  42,  10,  32,  42,  32,  71,  76,  79,  66,  65,  76,  32,  77,
      69,  84,  65,  58,  10,  32,  42,  32,  64,  115, 101, 99,  117, 114, 105,
      116, 121, 83,  99,  104, 101, 109, 101, 32,  109, 101, 114, 103, 101, 95,
      111, 97,  117, 116, 104, 32,  91,  116, 121, 112, 101, 58,  111, 97,  117,
      116, 104, 50,  93,  32,  91,  102, 108, 111, 119, 58,  105, 109, 112, 108,
      105, 99,  105, 116, 93,  32,  91,  97,  117, 116, 104, 111, 114, 105, 122,
      97,  116, 105, 111, 110, 85,  114, 108, 58,  104, 116, 116, 112, 115, 58,
      47,  47,  97,  117, 116, 104, 46,  99,  111, 109, 47,  97,  117, 116, 104,
      93,  32,  91,  115, 99,  111, 112, 101, 115, 58,  114, 101, 97,  100, 44,
      119, 114, 105, 116, 101, 93,  10,  32,  42,  32,  64,  115, 101, 99,  117,
      114, 105, 116, 121, 83,  99,  104, 101, 109, 101, 32,  109, 101, 114, 103,
      101, 95,  111, 97,  117, 116, 104, 32,  91,  116, 121, 112, 101, 58,  111,
      97,  117, 116, 104, 50,  93,  32,  91,  102, 108, 111, 119, 58,  105, 109,
      112, 108, 105, 99,  105, 116, 93,  32,  91,  97,  117, 116, 104, 111, 114,
      105, 122, 97,  116, 105, 111, 110, 85,  114, 108, 58,  104, 116, 116, 112,
      115, 58,  47,  47,  97,  117, 116, 104, 46,  99,  111, 109, 47,  97,  117,
      116, 104, 93,  32,  91,  115, 99,  111, 112, 101, 115, 58,  114, 101, 97,
      100, 44,  97,  100, 109, 105, 110, 93,  10,  32,  42,  32,  64,  115, 101,
      99,  117, 114, 105, 116, 121, 83,  99,  104, 101, 109, 101, 32,  109, 101,
      114, 103, 101, 95,  111, 97,  117, 116, 104, 32,  91,  116, 121, 112, 101,
      58,  111, 97,  117, 116, 104, 50,  93,  32,  91,  102, 108, 111, 119, 58,
      112, 97,  115, 115, 119, 111, 114, 100, 93,  32,  91,  116, 111, 107, 101,
      110, 85,  114, 108, 58,  104, 116, 116, 112, 115, 58,  47,  47,  97,  117,
      116, 104, 46,  99,  111, 109, 47,  116, 111, 107, 101, 110, 93,  10,  32,
      42,  47,  10,  105, 110, 116, 32,  102, 111, 111, 50,  48,  40,  118, 111,
      105, 100, 41,  59,  10,  0};

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(c_file, src);

  {
    char *argv[3]; /* ... */

    argv[0] = (char *)(size_t)(size_t) "c2openapi";
    argv[1] = (char *)(size_t)src_dir;
    argv[2] = (char *)(size_t)out_json;
    rc = c2openapi_cli_main(3, argv);
    if (rc != 0) {
      printf("\nERROR rc=%d\n", rc);
    }
    ASSERT_EQ(0, rc);

    remove(c_file);
    remove(out_json);
    rmdir(src_dir);
    free((void *)(size_t)c_file);
    free((void *)(size_t)out_json);
    free((void *)(size_t)src_dir);
    free((void *)(size_t)tmp_dir);
    g_fail_io_after = -1;

    (void)rc;
    PASS();
  }
}

TEST test_c2o_cli_oauth_validation_errors(void) {
  int rc;
  const char *snippets[] = {
      "/**\n * @securityScheme oauth_bad1 [type:oauth2] [flow:implicit]\n "
      "*/\nint foo21(void);\n" /* Missing authorizationUrl */
      "/**\n * @securityScheme oauth_bad2 [type:oauth2] [flow:password]\n "
      "*/\nint foo22(void);\n", /* Missing tokenUrl */
      "/**\n * @securityScheme oauth_bad3 [type:oauth2] "
      "[flow:clientCredentials]\n */\nint foo23(void);\n", /* Missing tokenUrl
                                                            */
      "/**\n * @securityScheme oauth_bad4 [type:oauth2] "
      "[flow:authorizationCode] [authorizationUrl:https://auth.com/auth]\n "
      "*/\nint foo24(void);\n", /* Missing tokenUrl */
      "/**\n * @securityScheme oauth_bad5 [type:oauth2] "
      "[flow:authorizationCode] [tokenUrl:https://auth.com/token]\n */\nint "
      "foo25(void);\n", /* Missing authUrl */
      "/**\n * @securityScheme oauth_bad6 [type:oauth2] "
      "[flow:deviceAuthorization] [tokenUrl:https://auth.com/token]\n */\nint "
      "foo26(void);\n" /* Missing deviceAuthorizationUrl */
  };

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  {
    size_t i;
    for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
      char *c_file = NULL;
      asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
               (unsigned long)i);
      write_to_file(c_file, snippets[i]);
      free((void *)(size_t)c_file);
    }

    {
      char *argv[3]; /* ... */

      argv[0] = (char *)(size_t)(size_t) "c2openapi";
      argv[1] = (char *)(size_t)src_dir;
      argv[2] = (char *)(size_t)out_json;
      rc = c2openapi_cli_main(3, argv);
      if (rc != 0) {
        printf("\nERROR rc=%d\n", rc);
      }
      ASSERT_EQ(0, rc);

      for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
        char *c_file = NULL;
        asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
                 (unsigned long)i);
        remove(c_file);
        free((void *)(size_t)c_file);
      }
      remove(out_json);
      rmdir(src_dir);
      free((void *)(size_t)out_json);
      free((void *)(size_t)src_dir);
      free((void *)(size_t)tmp_dir);
      g_fail_io_after = -1;

      (void)rc;
      PASS();
    }
  }
}

TEST test_c2o_cli_merge_oauth_flow_collisions(void) {
  int rc;
  const char *snippets[] = {
      "/**\n * GLOBAL META:\n * @securityScheme merge_oauth [type:oauth2] "
      "[flow:implicit] [authorizationUrl:https://auth.com/auth1]\n * "
      "@securityScheme merge_oauth [type:oauth2] [flow:implicit] "
      "[authorizationUrl:https://auth.com/auth2]\n */\nint foo27(void);\n",
      "/**\n * GLOBAL META:\n * @securityScheme merge_oauth [type:oauth2] "
      "[flow:password] [tokenUrl:https://auth.com/token1]\n * @securityScheme "
      "merge_oauth [type:oauth2] [flow:password] "
      "[tokenUrl:https://auth.com/token2]\n */\nint foo28(void);\n",
      "/**\n * GLOBAL META:\n * @securityScheme merge_oauth [type:oauth2] "
      "[flow:deviceAuthorization] "
      "[deviceAuthorizationUrl:https://auth.com/device1] "
      "[tokenUrl:https://auth.com/token]\n * @securityScheme merge_oauth "
      "[type:oauth2] [flow:deviceAuthorization] "
      "[deviceAuthorizationUrl:https://auth.com/device2] "
      "[tokenUrl:https://auth.com/token]\n */\nint foo29(void);\n",
      "/**\n * GLOBAL META:\n * @securityScheme merge_oauth [type:oauth2] "
      "[flow:deviceAuthorization] "
      "[deviceAuthorizationUrl:https://auth.com/device] "
      "[refreshUrl:https://auth.com/refresh1] "
      "[tokenUrl:https://auth.com/token]\n * @securityScheme merge_oauth "
      "[type:oauth2] [flow:deviceAuthorization] "
      "[deviceAuthorizationUrl:https://auth.com/device] "
      "[refreshUrl:https://auth.com/refresh2] "
      "[tokenUrl:https://auth.com/token]\n */\nint foo30(void);\n"};

  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *out_json = NULL;

  (void)rc;
  tempdir((char **)&tmp_dir);
  asprintf((char **)&src_dir, "%s%cc2o_test_err_%d", tmp_dir, PATH_SEP_C,
           rand());
  makedir(src_dir);
  asprintf((char **)&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  {
    size_t i;
    for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
      char *c_file = NULL;
      asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
               (unsigned long)i);
      write_to_file(c_file, snippets[i]);
      free((void *)(size_t)c_file);
    }

    {
      char *argv[3]; /* ... */

      argv[0] = (char *)(size_t)(size_t) "c2openapi";
      argv[1] = (char *)(size_t)src_dir;
      argv[2] = (char *)(size_t)out_json;
      rc = c2openapi_cli_main(3, argv);
      if (rc != 0) {
        printf("\nERROR rc=%d\n", rc);
      }
      ASSERT_EQ(0, rc);

      for (i = 0; i < sizeof(snippets) / sizeof(snippets[0]); ++i) {
        char *c_file = NULL;
        asprintf((char **)&c_file, "%s%cf%lu.c", src_dir, PATH_SEP_C,
                 (unsigned long)i);
        remove(c_file);
        free((void *)(size_t)c_file);
      }
      remove(out_json);
      rmdir(src_dir);
      free((void *)(size_t)out_json);
      free((void *)(size_t)src_dir);
      free((void *)(size_t)tmp_dir);
      g_fail_io_after = -1;

      (void)rc;
      PASS();
    }
  }
}

SUITE(integration_c2openapi_suite) {
  RUN_TEST(test_c2openapi_full_flow);
  RUN_TEST(test_c2openapi_with_base_spec);
  RUN_TEST(test_c2openapi_with_self_uri);
  RUN_TEST(test_c2openapi_global_meta_security_schemes);
  RUN_TEST(test_c2o_cli_source_file_checks);
  RUN_TEST(test_c2o_cli_doc_sec_unset);
  RUN_TEST(test_c2o_cli_spec_has_tag_nulls);
  RUN_TEST(test_c2o_cli_mappings_errors_find);
  RUN_TEST(test_c2o_cli_set_str_mismatch);
  RUN_TEST(test_c2o_cli_server_variables);
  RUN_TEST(test_c2o_cli_server_variables_validation);
  RUN_TEST(test_c2o_cli_merge_oauth_scopes);
  RUN_TEST(test_c2o_cli_oauth_validation_errors);
  RUN_TEST(test_c2o_cli_merge_oauth_flow_collisions);
}

#if defined(__GNUC__) || defined(__clang__)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_INTEGRATION_C2OPENAPI_H */
