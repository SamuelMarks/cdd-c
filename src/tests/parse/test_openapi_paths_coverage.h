/**
 * @file test_openapi_paths_coverage.h
 * @brief Comprehensive 100% coverage tests for openapi_paths.c.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_PATHS_COVERAGE_H
#define TEST_OPENAPI_PATHS_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
#include "openapi/parse/openapi_test_helpers.h"
/* clang-format on */

/**
 * @brief Tests name_in_list and collect_path_template_names.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_paths_name_list_and_template_names(void) {
  char *list[3];
  char **names = NULL;
  size_t count = 0;
  size_t i;

  list[0] = (char *)(size_t) "foo";
  list[1] = (char *)(size_t) "bar";
  list[2] = (char *)(size_t) "baz";

  /* 1. name_in_list */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_name_in_list("foo", NULL, 3));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_name_in_list(NULL, list, 3));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_name_in_list("foo", list, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_name_in_list("qux", list, 3));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_name_in_list("bar", list, 3));
  list[0] = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_name_in_list("foo", list, 1));
  list[0] = (char *)(size_t) "foo";

  /* 2. collect_path_template_names error args */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_path_template_names(NULL, &names, &count));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_collect_path_template_names("/users", NULL, &count));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_collect_path_template_names("/users", &names, NULL));

  /* 3. empty route */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_path_template_names("", &names, &count));
  ASSERT_EQ(0, count);

  /* 4. valid extraction */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_collect_path_template_names(
                "/users/{userId}/posts/{postId}", &names, &count));
  ASSERT_EQ(2, count);
  ASSERT_STR_EQ("userId", names[0]);
  ASSERT_STR_EQ("postId", names[1]);
  for (i = 0; i < count; ++i)
    free(names[i]);
  free(names);
  names = NULL;
  count = 0;

  /* 5. duplicate template parameter in route */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_collect_path_template_names(
                "/users/{userId}/posts/{userId}", &names, &count));

  /* 6. unclosed brace and empty brace */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_collect_path_template_names("/users/{}/posts/{postId",
                                                 &names, &count));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_collect_path_template_names("/users/{id", &names, &count));
  ASSERT_EQ(0, count);

  /* 7. dup_substr allocation failure */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_collect_path_template_names(
                                    "/users/{userId}", &names, &count));
  g_cdd_alloc_fail = 0;

  /* 8. unmatched closing brace */
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_test_collect_path_template_names("/users/userId}", &names, &count));

  /* 9. realloc failure */
  for (i = 1; i <= 8; ++i) {
    g_cdd_alloc_fail = (int)i;
    cdd_test_collect_path_template_names("/a/{b}/{c}/{d}/{e}/{f}", &names,
                                         &count);
    g_cdd_alloc_fail = 0;
  }

  PASS();
}

/**
 * @brief Tests normalize_path_template_route and collision detection.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_paths_normalize_and_collisions(void) {
  char *norm = NULL;
  struct OpenAPI_Path paths[2];
  struct OpenAPI_Path bad_paths[2];

  memset(paths, 0, sizeof(paths));
  memset(bad_paths, 0, sizeof(bad_paths));

  /* 1. normalize_path_template_route error args */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_normalize_path_template_route(NULL, &norm));
  ASSERT_EQ(NULL, norm);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_normalize_path_template_route("/users", NULL));

  /* 2. empty route */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_normalize_path_template_route("", &norm));
  ASSERT_STR_EQ("", norm);
  free(norm);
  norm = NULL;

  /* 3. normalize with template */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_normalize_path_template_route(
                               "/users/{userId}/posts/{postId}", &norm));
  ASSERT_STR_EQ("/users/{}/posts/{}", norm);
  free(norm);
  norm = NULL;

  /* 4. normalize with unclosed brace */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_normalize_path_template_route("/users/{id", &norm));
  ASSERT_EQ(NULL, norm);

  /* 5. malloc failure in normalize */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_normalize_path_template_route("/users/{id}", &norm));
  ASSERT_EQ(NULL, norm);
  g_cdd_alloc_fail = 0;

  /* 6. validate_path_template_collisions */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_path_template_collisions(NULL, 2));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_path_template_collisions(NULL, 0));

  /* 7. paths with NULL routes */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_path_template_collisions(paths, 2));

  /* 8. collision */
  bad_paths[0].route = (char *)(size_t) "/users/{id}";
  bad_paths[1].route = (char *)(size_t) "/users/{userId}";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_path_template_collisions(bad_paths, 2));

  /* 9. distinct paths */
  bad_paths[1].route = (char *)(size_t) "/posts/{id}";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_path_template_collisions(bad_paths, 2));

  /* 10. non-templated second path */
  bad_paths[1].route = (char *)(size_t) "/posts";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_path_template_collisions(bad_paths, 2));

  /* 11. unclosed brace in second path */
  bad_paths[1].route = (char *)(size_t) "/users/{id";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_path_template_collisions(bad_paths, 2));

  /* 11b. unclosed brace in first path */
  bad_paths[0].route = (char *)(size_t) "/users/{id";
  bad_paths[1].route = (char *)(size_t) "/users/{id}";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_path_template_collisions(bad_paths, 2));

  /* 12. non-slash start in first path */
  bad_paths[0].route = (char *)(size_t) "users/{id}";
  bad_paths[1].route = (char *)(size_t) "/users/{id}";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_path_template_collisions(bad_paths, 2));

  /* 13. NULL second path */
  bad_paths[0].route = (char *)(size_t) "/users/{id}";
  bad_paths[1].route = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_path_template_collisions(bad_paths, 2));

  /* 14. non-slash start in second path */
  bad_paths[1].route = (char *)(size_t) "users/{id}";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_path_template_collisions(bad_paths, 2));

  /* 15. identical routes */
  bad_paths[0].route = (char *)(size_t) "/users/{id}";
  bad_paths[1].route = (char *)(size_t) "/users/{id}";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_path_template_collisions(bad_paths, 2));

  PASS();
}

/**
 * @brief Tests validation of path params and path templates.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_paths_validation_branches(void) {
  struct OpenAPI_Parameter params[3];
  struct OpenAPI_Parameter *found = NULL;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  char *template_names[2];

  memset(params, 0, sizeof(params));
  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));

  template_names[0] = (char *)(size_t) "id";
  template_names[1] = (char *)(size_t) "subId";

  /* 1. find_path_param */
  params[0].name = (char *)(size_t) "id";
  params[0].in = OA_PARAM_IN_QUERY;
  params[1].name = (char *)(size_t) "id";
  params[1].in = OA_PARAM_IN_PATH;
  params[1].required = 1;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_path_param(params, 2, "id", &found));
  ASSERT_EQ(&params[1], found);
  found = NULL;

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_path_param(params, 2, "missing", &found));
  ASSERT_EQ(NULL, found);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_path_param(params, 2, NULL, &found));
  ASSERT_EQ(NULL, found);

  /* Parameter with NULL name */
  params[0].name = NULL;
  params[0].in = OA_PARAM_IN_PATH;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_path_param(params, 1, "id", &found));
  ASSERT_EQ(NULL, found);

  /* 2. validate_path_params_list */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_path_params_list(NULL, 0, NULL, 0));

  /* Parameter in PATH but not required */
  params[1].required = 0;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_path_params_list(params, 2, template_names, 2));

  /* Parameter in PATH not in template */
  params[1].required = 1;
  params[2].name = (char *)(size_t) "notInTemplate";
  params[2].in = OA_PARAM_IN_PATH;
  params[2].required = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_path_params_list(params, 3, template_names, 2));

  /* Valid list */
  params[0].name = (char *)(size_t) "queryParam";
  params[0].in = OA_PARAM_IN_QUERY;
  params[2].name = (char *)(size_t) "other";
  params[2].in = OA_PARAM_IN_HEADER;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_validate_path_params_list(params, 3, template_names, 2));

  /* 3. validate_path_template_for_operation */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_path_template_for_operation(
                               NULL, NULL, template_names, 2));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_path_template_for_operation(
                               &path, NULL, NULL, 2));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_path_template_for_operation(
                               &path, &op, template_names, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_path_template_for_operation(&path, NULL,
                                                          template_names, 2));

  /* Missing in both path and operation */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_path_template_for_operation(&path, &op,
                                                          template_names, 2));

  /* Present in path */
  path.parameters = params;
  path.n_parameters = 2; /* has "id" */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_path_template_for_operation(
                &path, &op, template_names, 2)); /* missing subId */

  /* Present in operation */
  op.parameters =
      (struct OpenAPI_Parameter *)calloc(1, sizeof(struct OpenAPI_Parameter));
  op.parameters[0].name = (char *)(size_t) "subId";
  op.parameters[0].in = OA_PARAM_IN_PATH;
  op.parameters[0].required = 1;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_path_template_for_operation(
                               &path, &op, template_names, 2));
  free(op.parameters);
  op.parameters = NULL;
  op.n_parameters = 0;
  path.parameters = NULL;
  path.n_parameters = 0;

  /* 4. validate_path_templates */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_path_templates(NULL, 0));

  /* Path with NULL route in multi-path list */
  {
    struct OpenAPI_Path m_paths[2];
    memset(m_paths, 0, sizeof(m_paths));
    m_paths[0].route = NULL;
    m_paths[1].route = (char *)(size_t) "/users";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_path_templates(m_paths, 2));
  }

  /* Path with template but NO operations */
  path.route = (char *)(size_t) "/items/{itemId}";
  path.n_operations = 0;
  path.n_additional_operations = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_path_templates(&path, 1));

  /* Path with template and operation missing parameter */
  path.operations = &op;
  path.n_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_path_templates(&path, 1));
  path.operations = NULL;
  path.n_operations = 0;

  /* Path without leading slash */
  path.route = (char *)(size_t) "users/{id}";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_path_templates(&path, 1));

  /* Path with additional operations missing template param */
  path.route = (char *)(size_t) "/items/{itemId}";
  path.additional_operations = &op;
  path.n_additional_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_path_templates(&path, 1));
  path.additional_operations = NULL;
  path.n_additional_operations = 0;

  PASS();
}

/**
 * @brief Tests parse_paths_object and parse_additional_operations.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_paths_parsing_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path *paths = NULL;
  size_t n_paths = 0;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  JSON_Value *jv2 = NULL;
  JSON_Object *jo2 = NULL;
  size_t i;

  memset(&spec, 0, sizeof(spec));
  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));

  /* 1. parse_paths_object NULL args */
  jv = json_parse_string("{\"/users\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(NULL, &paths, &n_paths, &spec, 1, 1));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(jo, NULL, &n_paths, &spec, 1, 1));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(jo, &paths, NULL, &spec, 1, 1));
  json_value_free(jv);

  /* Path without leading slash when require_leading_slash == 0 */
  jv = json_parse_string("{\"users\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 0, 0));
  if (paths) {
    for (i = 0; i < n_paths; ++i)
      cdd_test_free_path_item(&paths[i]);
    free(paths);
    paths = NULL;
  }
  json_value_free(jv);

  /* Path item whose value is not an object */
  jv = json_parse_string("{\"/empty\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 0));
  if (paths) {
    for (i = 0; i < n_paths; ++i)
      cdd_test_free_path_item(&paths[i]);
    free(paths);
    paths = NULL;
  }
  json_value_free(jv);

  /* 2. empty object */
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 1));
  ASSERT_EQ(0, n_paths);
  json_value_free(jv);

  /* 3. paths calloc failure */
  jv = json_parse_string("{\"/users\": {\"summary\": \"Users\"}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 1));
  g_cdd_alloc_fail = 0;

  /* 4. non-root path with template character (e.g. callback with {) */
  jv = json_parse_string("{\"/users/{id}\": {\"summary\": \"Users\"}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 0, 0));
  if (paths) {
    for (i = 0; i < n_paths; ++i)
      cdd_test_free_path_item(&paths[i]);
    free(paths);
    paths = NULL;
  }
  json_value_free(jv);

  /* 5. path item with parameters allocation failure */
  jv = json_parse_string(
      "{\"/users\": {\"parameters\": [{\"name\": \"id\", \"in\": "
      "\"path\", \"required\": true}]}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 0));
  g_cdd_alloc_fail = 0;
  json_value_free(jv);

  /* 6. path item with servers allocation failure */
  jv = json_parse_string("{\"/users\": {\"servers\": [{\"url\": "
                         "\"https://api.example.com\"}]}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 0));
  g_cdd_alloc_fail = 0;
  json_value_free(jv);

  /* 7. path item with invalid operation error */
  jv = json_parse_string("{\"/users\": {\"get\": {\"responses\": {}}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 0));
  json_value_free(jv);

  /* 8. path item with extensions */
  jv = json_parse_string("{\"/users\": {\"x-internal\": true}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 0));
  if (paths) {
    for (i = 0; i < n_paths; ++i)
      cdd_test_free_path_item(&paths[i]);
    free(paths);
    paths = NULL;
  }
  json_value_free(jv);

  /* 8b. path item with summary and get */
  jv = json_parse_string(
      "{\"/users\": {\"summary\": \"Users\", \"get\": {\"responses\": "
      "{\"200\": {\"description\": \"OK\"}}}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 0));
  if (paths) {
    for (i = 0; i < n_paths; ++i)
      cdd_test_free_path_item(&paths[i]);
    free(paths);
    paths = NULL;
  }
  json_value_free(jv);

  /* 9. parse_additional_operations NULL args */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_additional_operations(NULL, &path, &spec));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_additional_operations(jo, NULL, &spec));

  /* 10. parse_additional_operations empty object */
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_additional_operations(jo, &path, &spec));
  json_value_free(jv);

  /* 10b. additionalOperations empty object */
  jv = json_parse_string("{\"additionalOperations\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_additional_operations(jo, &path, &spec));
  json_value_free(jv);

  /* 11. parse_additional_operations alloc failure */
  jv = json_parse_string(
      "{\"additionalOperations\": {\"customMethod\": {\"responses\": {\"200\": "
      "{\"description\": \"OK\"}}}}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_additional_operations(jo, &path, &spec));
  g_cdd_alloc_fail = 0;

  /* 11b. additionalOperations invalid operation */
  path.route = (char *)(size_t) "/test-route";
  jv = json_parse_string(
      "{\"additionalOperations\": {\"customMethod\": {\"responses\": {}}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_parse_additional_operations(jo, &path, &spec));
  json_value_free(jv);
  path.route = (char *)(size_t) "";
  jv = json_parse_string(
      "{\"additionalOperations\": {\"customMethod\": {\"responses\": {}}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_parse_additional_operations(jo, &path, &spec));
  json_value_free(jv);
  path.route = NULL;

  /* 12. parse_additional_operations valid custom method */
  jv = json_parse_string(
      "{\"additionalOperations\": {\"customMethod\": {\"responses\": {\"200\": "
      "{\"description\": \"OK\"}}}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_additional_operations(jo, &path, &spec));
  cdd_test_free_path_item(&path);
  memset(&path, 0, sizeof(path));
  json_value_free(jv);

  /* 13. parse_operation with request body error */
  jv = json_parse_string("{\"requestBody\": {\"description\": \"desc\"}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_operation("post", jo, &op, &spec, 0));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 13b. parse_operation with request body schema copy error */
  jv = json_parse_string(
      "{\"requestBody\": {\"content\": {\"application/json\": {\"schema\": "
      "{\"type\": \"string\"}}}}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 2;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_parse_operation("post", jo, &op, &spec, 0));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 14. parse_operation with tags alloc failure */
  jv = json_parse_string("{\"tags\": [\"mytag\"]}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_operation("get", jo, &op, &spec, 0));
  g_cdd_alloc_fail = 0;
  json_value_free(jv);

  /* 15. $ref with summary strdup fail (line 108) */
  jv = json_parse_string(
      "{\"/users\": {\"$ref\": \"#/components/pathItems/Users\", \"summary\": "
      "\"Sum\"}}");
  jo = json_value_get_object(jv);
  for (i = 1; i <= 5; ++i) {
    g_cdd_strdup_fail = (int)i;
    cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 0);
    g_cdd_strdup_fail = 0;
  }
  json_value_free(jv);

  /* 16. $ref with description strdup fail (line 117) */
  jv = json_parse_string(
      "{\"/users\": {\"$ref\": \"#/components/pathItems/Users\", "
      "\"description\": \"Desc\"}}");
  jo = json_value_get_object(jv);
  for (i = 1; i <= 5; ++i) {
    g_cdd_strdup_fail = (int)i;
    cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 0);
    g_cdd_strdup_fail = 0;
  }
  json_value_free(jv);

  /* 17. collect_extensions strdup fail (line 140) */
  jv = json_parse_string("{\"/users\": {\"x-custom\": 123}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 0));
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 18. parse_operation requestBody extensions_json */
  jv = json_parse_string("{\"requestBody\": {\"content\": "
                         "{\"application/json\": {}}, \"x-custom\": 123}}");
  jo = json_value_get_object(jv);
  for (i = 1; i <= 8; ++i) {
    g_cdd_strdup_fail = (int)i;
    memset(&op, 0, sizeof(op));
    cdd_test_parse_operation("post", jo, &op, &spec, 0);
    g_cdd_strdup_fail = 0;
  }
  json_value_free(jv);

  /* 19. parse_operation invalid args */
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_operation(NULL, jo, &op, &spec, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_parse_operation("get", jo, NULL, &spec, 0));

  /* 20. parse_operation empty tags array */
  jv2 = json_parse_string("{\"tags\": []}");
  jo2 = json_value_get_object(jv2);
  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_operation("get", jo2, &op, &spec, 0));
  json_value_free(jv2);

  /* 21. parse_operation tag with null/non-string item */
  jv2 = json_parse_string("{\"tags\": [123]}");
  jo2 = json_value_get_object(jv2);
  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_operation("get", jo2, &op, &spec, 0));
  json_value_free(jv2);
  json_value_free(jv);

  /* 22. path_ref with spec == NULL and resolve_refs == 1 */
  jv = json_parse_string(
      "{\"/users\": {\"$ref\": \"#/components/pathItems/Users\"}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, NULL, 1, 1));
  if (paths) {
    for (i = 0; i < n_paths; ++i)
      cdd_test_free_path_item(&paths[i]);
    free(paths);
    paths = NULL;
  }
  json_value_free(jv);

  /* 23. path item with $ref and get */
  jv = json_parse_string(
      "{\"/users\": {\"$ref\": \"#/components/pathItems/Users\", \"get\": "
      "{\"responses\": {\"200\": {\"description\": \"OK\"}}}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_paths_object(jo, &paths, &n_paths, &spec, 1, 0));
  if (paths) {
    for (i = 0; i < n_paths; ++i)
      cdd_test_free_path_item(&paths[i]);
    free(paths);
    paths = NULL;
  }
  json_value_free(jv);

  /* 24. parse_operation requestBody with $ref */
  jv = json_parse_string(
      "{\"requestBody\": {\"$ref\": \"#/components/requestBodies/MyBody\"}}");
  jo = json_value_get_object(jv);
  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_operation("post", jo, &op, &spec, 0));
  json_value_free(jv);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_PATHS_COVERAGE_H */
