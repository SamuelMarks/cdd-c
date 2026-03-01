/**
 * @file test_integration_c2openapi.h
 * @brief End-to-End integration tests for C-to-OpenAPI generation.
 *
 * Simulates a full execution cycle:
 * 1. Create a temporary source tree with C files and doc annotations.
 * 2. Run the `c2openapi` CLI logic.
 * 3. Validate the output JSON content.
 *
 * @author Samuel Marks
 */

#ifndef TEST_INTEGRATION_C2OPENAPI_H
#define TEST_INTEGRATION_C2OPENAPI_H
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#endif

#include <greatest.h>
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include "routes/parse/cli.h"

TEST test_c2openapi_full_flow(void) {
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *h_file = NULL;
  char *out_json = NULL;
  int rc;

  /* 0. Setup Directories */
  tempdir(&tmp_dir);
  /* Use %c for PATH_SEP_C */
  asprintf(&src_dir, "%s%cc2o_test_%d", tmp_dir, PATH_SEP_C, rand());
  makedir(src_dir);
  asprintf(&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf(&h_file, "%s%cmodels.h", src_dir, PATH_SEP_C);
  asprintf(&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  /* 1. Write Data Models */
  write_to_file(h_file, "struct User { int id; char *name; };\n");

  /* 2. Write Implementation with Annotations */
  write_to_file(
      c_file,
      "#include \"models.h\"\n"
      "\n"
      "/**\n"
      " * @route GET /users/{id}\n"
      " * @infoTitle Example API\n"
      " * @infoVersion 2.1.0\n"
      " * @infoSummary Example summary\n"
      " * @infoDescription Example description\n"
      " * @termsOfService https://example.com/terms\n"
      " * @contact [name:API Support] "
      "[url:https://example.com/support] "
      "[email:support@example.com]\n"
      " * @license [name:Apache 2.0] [identifier:Apache-2.0]\n"
      " * @summary Get a user by ID\n"
      " * @tag users\n"
      " * @tagMeta external [summary:External] "
      "[description:External operations]\n"
      " * @tagMeta users [summary:Users] [description:User operations] "
      "[parent:external] [kind:nav] [externalDocs:https://example.com/docs] "
      "[externalDocsDescription:More docs]\n"
      " * @param id The user ID\n"
      " */\n"
      "int api_get_user(int id, struct User **out) {\n"
      "  return 0;\n"
      "}\n"
      "\n"
      "/**\n"
      " * @route POST /users\n"
      " * @summary Create a user\n"
      " */\n"
      "int api_create_user(struct User *u) {\n"
      "  return 0;\n"
      "}\n"
      "\n"
      "/**\n"
      " * @webhook POST /user-events\n"
      " * @summary User event webhook\n"
      " */\n"
      "int api_user_event(struct User *u) {\n"
      "  return 0;\n"
      "}\n");

  /* 3. Run CLI */
  {
    /* C90 compliant initialization */
    char *argv[5];
    argv[0] = "c2openapi";
    argv[1] = "--dialect";
    argv[2] = "https://spec.openapis.org/oas/3.1/dialect/base";
    argv[3] = src_dir;
    argv[4] = out_json;

    rc = c2openapi_cli_main(5, argv);
    ASSERT_EQ(EXIT_SUCCESS, rc);
  }

  /* 4. Verify JSON */
  {
    JSON_Value *root = json_parse_file(out_json);
    JSON_Object *obj;
    ASSERT(root != NULL);
    obj = json_value_get_object(root);

    /* Check Info */
    printf("%s\n", json_serialize_to_string_pretty(root));
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
      const char *type = json_object_dotget_string(
          obj, "components.schemas.User.properties.id.type");
      ASSERT_STR_EQ(("integer") ? ("integer") : "NULL",
                    (type) ? (type) : "NULL");
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
        const char *ref = json_object_dotget_string(
            op, "responses.200.content.application/json.schema.$ref");
        ASSERT_STR_EQ(("#/components/schemas/User")
                          ? ("#/components/schemas/User")
                          : "NULL",
                      (ref) ? (ref) : "NULL");
      }
    }

    /* Check POST /users */
    {
      JSON_Object *op = json_object_dotget_object(obj, "paths./users.post");
      ASSERT(op != NULL);
      /* Check Request Body */
      {
        const char *ref = json_object_dotget_string(
            op, "requestBody.content.application/json.schema.$ref");
        ASSERT_STR_EQ(("#/components/schemas/User")
                          ? ("#/components/schemas/User")
                          : "NULL",
                      (ref) ? (ref) : "NULL");
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
  free(c_file);
  free(h_file);
  free(out_json);
  free(src_dir);
  free(tmp_dir);

  PASS();
}

TEST test_c2openapi_with_base_spec(void) {
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *h_file = NULL;
  char *out_json = NULL;
  char *base_json = NULL;
  int rc;

  tempdir(&tmp_dir);
  asprintf(&src_dir, "%s%cc2o_base_%d", tmp_dir, PATH_SEP_C, rand());
  makedir(src_dir);
  asprintf(&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf(&h_file, "%s%cmodels.h", src_dir, PATH_SEP_C);
  asprintf(&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);
  asprintf(&base_json, "%s%cbase.json", src_dir, PATH_SEP_C);

  write_to_file(h_file, "struct User { int id; char *name; };\n");

  write_to_file(c_file, "#include \"models.h\"\n"
                        "/**\n"
                        " * @route GET /users/{id}\n"
                        " * @summary Get a user by ID\n"
                        " * @tag users\n"
                        " * @param id [in:path] The user ID\n"
                        " */\n"
                        "int api_get_user(int id, struct User **out) {\n"
                        "  return 0;\n"
                        "}\n");

  write_to_file(base_json,
                "{\n"
                "  \"openapi\": \"3.2.0\",\n"
                "  \"$self\": \"https://example.com/openapi.json\",\n"
                "  \"jsonSchemaDialect\": "
                "\"https://spec.openapis.org/oas/3.1/dialect/base\",\n"
                "  \"info\": {\"title\": \"Base API\", \"version\": \"9.9.9\", "
                "\"summary\": \"Base summary\"},\n"
                "  \"servers\": [{\"url\": \"https://api.example.com/v1\", "
                "\"name\": \"prod\"}],\n"
                "  \"tags\": [{\"name\": \"users\", \"description\": \"User "
                "operations\", \"kind\": \"nav\"}],\n"
                "  \"components\": {\n"
                "    \"securitySchemes\": {\n"
                "      \"api_key\": {\"type\": \"apiKey\", \"name\": "
                "\"X-API-Key\", \"in\": \"header\"}\n"
                "    }\n"
                "  },\n"
                "  \"paths\": {}\n"
                "}\n");

  {
    char *argv[5];
    argv[0] = "c2openapi";
    argv[1] = "--base";
    argv[2] = base_json;
    argv[3] = src_dir;
    argv[4] = out_json;
    rc = c2openapi_cli_main(5, argv);
    ASSERT_EQ(EXIT_SUCCESS, rc);
  }

  {
    JSON_Value *root = json_parse_file(out_json);
    JSON_Object *obj;
    ASSERT(root != NULL);
    obj = json_value_get_object(root);

    printf("%s\n", json_serialize_to_string_pretty(root));
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
  free(c_file);
  free(h_file);
  free(out_json);
  free(base_json);
  free(src_dir);
  free(tmp_dir);

  PASS();
}

TEST test_c2openapi_with_self_uri(void) {
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *h_file = NULL;
  char *out_json = NULL;
  int rc;

  tempdir(&tmp_dir);
  asprintf(&src_dir, "%s%cc2o_self_%d", tmp_dir, PATH_SEP_C, rand());
  makedir(src_dir);
  asprintf(&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf(&h_file, "%s%cmodels.h", src_dir, PATH_SEP_C);
  asprintf(&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(h_file, "struct User { int id; char *name; };\n");
  write_to_file(c_file,
                "#include \"models.h\"\n"
                "/**\n"
                " * @route GET /users\n"
                " * @summary List users\n"
                " */\n"
                "int api_list_users(struct User **out) { return 0; }\n");

  {
    char *argv[5];
    argv[0] = "c2openapi";
    argv[1] = "--self";
    argv[2] = "https://example.com/override.json";
    argv[3] = src_dir;
    argv[4] = out_json;
    rc = c2openapi_cli_main(5, argv);
    ASSERT_EQ(EXIT_SUCCESS, rc);
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
  free(c_file);
  free(h_file);
  free(out_json);
  free(src_dir);
  free(tmp_dir);

  PASS();
}

TEST test_c2openapi_global_meta_security_schemes(void) {
  char *tmp_dir = NULL;
  char *src_dir = NULL;
  char *c_file = NULL;
  char *out_json = NULL;
  int rc;

  tempdir(&tmp_dir);
  asprintf(&src_dir, "%s%cc2o_global_%d", tmp_dir, PATH_SEP_C, rand());
  makedir(src_dir);
  asprintf(&c_file, "%s%capi.c", src_dir, PATH_SEP_C);
  asprintf(&out_json, "%s%cspec.json", src_dir, PATH_SEP_C);

  write_to_file(
      c_file, "/**\n"
              " * @securityScheme api_key [type:apiKey] [paramName:X-API-Key] "
              "[in:header]\n"
              " * @security api_key\n"
              " * @server https://api.example.com [name:prod]\n"
              " * @externalDocs https://docs.example.com Global docs\n"
              " */\n"
              "int placeholder = 0;\n"
              "\n"
              "/**\n"
              " * @route GET /ping\n"
              " * @summary Ping\n"
              " */\n"
              "int api_ping(void) { return 0; }\n");

  {
    char *argv[3];
    argv[0] = "c2openapi";
    argv[1] = src_dir;
    argv[2] = out_json;
    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(EXIT_SUCCESS, rc);
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

    printf("GLOBAL META: %s\n", json_serialize_to_string_pretty(root));
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
  free(c_file);
  free(out_json);
  free(src_dir);
  free(tmp_dir);

  PASS();
}

SUITE(integration_c2openapi_suite) {
  RUN_TEST(test_c2openapi_full_flow);
  RUN_TEST(test_c2openapi_with_base_spec);
  RUN_TEST(test_c2openapi_with_self_uri);
  RUN_TEST(test_c2openapi_global_meta_security_schemes);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif /* TEST_INTEGRATION_C2OPENAPI_H */
