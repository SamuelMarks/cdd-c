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

#include <greatest.h>
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c2openapi_cli.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "fs.h"

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
  write_to_file(c_file, "#include \"models.h\"\n"
                        "\n"
                        "/**\n"
                        " * @route GET /users/{id}\n"
                        " * @summary Get a user by ID\n"
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
                        "}\n");

  /* 3. Run CLI */
  {
    /* C90 compliant initialization */
    char *argv[3];
    argv[0] = "c2openapi";
    argv[1] = src_dir;
    argv[2] = out_json;

    rc = c2openapi_cli_main(3, argv);
    ASSERT_EQ(EXIT_SUCCESS, rc);
  }

  /* 4. Verify JSON */
  {
    JSON_Value *root = json_parse_file(out_json);
    JSON_Object *obj;
    ASSERT(root != NULL);
    obj = json_value_get_object(root);

    /* Check Info */
    ASSERT_STR_EQ("3.2.0", json_object_get_string(obj, "openapi"));

    /* Check Components (Struct User) */
    {
      const char *type = json_object_dotget_string(
          obj, "components.schemas.User.properties.id.type");
      ASSERT_STR_EQ("integer", type);
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
        ASSERT_STR_EQ("#/components/schemas/User", ref);
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
        ASSERT_STR_EQ("#/components/schemas/User", ref);
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

SUITE(integration_c2openapi_suite) { RUN_TEST(test_c2openapi_full_flow); }

#endif /* TEST_INTEGRATION_C2OPENAPI_H */
