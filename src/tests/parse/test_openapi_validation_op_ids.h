/**
 * @file test_openapi_validation_op_ids.h
 * @brief Unit tests for OpenAPI unique operationId validation and component
 * reference tracking (openapi_validation.c).
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_VALIDATION_OP_IDS_H
#define TEST_OPENAPI_VALIDATION_OP_IDS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Helper function free_test_str_array.
 * @param[in,out] arr Parameter arr.
 * @param[in] count Parameter count.
 */
static void free_test_str_array(char **arr, size_t count) {
  size_t i;
  if (!arr)
    return;
  for (i = 0; i < count; ++i) {
    if (arr[i])
      free(arr[i]);
  }
  free(arr);
}

/**
 * @brief Unit test for unique operationId collection, component reference
 * matching, and validate_unique_operation_ids.
 * @return Greatest test result.
 */
TEST test_openapi_validation_operation_ids_branches(void) {
  struct OpenAPI_Path paths[2];
  struct OpenAPI_Operation ops[2];
  struct OpenAPI_Callback cbs[2];
  struct OpenAPI_Spec spec;
  char **ids = NULL;
  size_t id_count = 0;
  size_t id_cap = 0;
  char *cpi_name = cdd_test_mut_str("my_item");

  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  memset(&spec, 0, sizeof(spec));

  free_test_str_array(NULL, 0);

  /* 7. add_unique_operation_id */
  ASSERT_EQ(CDD_C_SUCCESS,
            add_unique_operation_id(&ids, &id_count, &id_cap, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            add_unique_operation_id(&ids, &id_count, &id_cap, ""));

  /* Add first id */
  ASSERT_EQ(CDD_C_SUCCESS,
            add_unique_operation_id(&ids, &id_count, &id_cap, "op1"));
  ASSERT_EQ(1, id_count);
  ASSERT(id_cap >= 1);

  /* Add duplicate */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            add_unique_operation_id(&ids, &id_count, &id_cap, "op1"));

  /* Add more to test realloc when count == cap */
  id_count = id_cap;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_unique_operation_id(&ids, &id_count, &id_cap, "op2"));
  g_cdd_alloc_fail = 0;

  /* strdup failure */
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_unique_operation_id(&ids, &id_count, &id_cap, "op2"));
  g_cdd_strdup_fail = 0;
  id_count = 1;

  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* 8. collect_operation_ids */
  ASSERT_EQ(CDD_C_SUCCESS,
            collect_operation_ids(NULL, 0, &ids, &id_count, &id_cap));
  ops[0].operation_id = cdd_test_mut_str("op1");
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  paths[0].additional_operations = &ops[1];
  paths[0].n_additional_operations = 1;
  ops[1].operation_id = cdd_test_mut_str("op2");
  ASSERT_EQ(CDD_C_SUCCESS,
            collect_operation_ids(paths, 1, &ids, &id_count, &id_cap));
  ASSERT_EQ(2, id_count);

  /* Duplicate in additional_operations */
  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;
  ops[0].operation_id = cdd_test_mut_str("op1");
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  paths[0].additional_operations = &ops[1];
  paths[0].n_additional_operations = 1;
  ops[1].operation_id = cdd_test_mut_str("op1");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            collect_operation_ids(paths, 1, &ids, &id_count, &id_cap));

  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* 9. path_item_ref_matches_component */
  {
    int bval = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              path_item_ref_matches_component(&spec, "ref", "item", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              path_item_ref_matches_component(NULL, NULL, "item", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS,
              path_item_ref_matches_component(&spec, "ref", NULL, &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, path_item_ref_matches_component(
                                 &spec, "bad_prefix", "item", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS,
              path_item_ref_matches_component(
                  &spec, "#/components/pathItems/my_item", "my_item", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, path_item_ref_matches_component(
                                 &spec, "#/components/pathItems/other_item",
                                 "my_item", &bval));
    ASSERT_EQ(0, bval);
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              path_item_ref_matches_component(
                  &spec, "#/components/pathItems/my_item", "my_item", &bval));
    ASSERT_EQ(0, bval);
    g_cdd_alloc_fail = 0;

    /* 10. component_path_item_is_referenced */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              component_path_item_is_referenced(&spec, "item", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              component_path_item_is_referenced(NULL, "item", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS,
              component_path_item_is_referenced(&spec, NULL, &bval));
    ASSERT_EQ(0, bval);
    spec.paths = paths;
    spec.n_paths = 1;
    paths[0].ref = cdd_test_mut_str("#/components/pathItems/my_item");
    ASSERT_EQ(CDD_C_SUCCESS,
              component_path_item_is_referenced(&spec, "my_item", &bval));
    ASSERT_EQ(1, bval);
    paths[0].ref = cdd_test_mut_str("#/components/pathItems/other_item");
    ASSERT_EQ(CDD_C_SUCCESS,
              component_path_item_is_referenced(&spec, "my_item", &bval));
    ASSERT_EQ(0, bval);
    paths[0].ref = NULL;
    spec.webhooks = &paths[1];
    spec.n_webhooks = 1;
    paths[1].ref = cdd_test_mut_str("#/components/pathItems/my_item");
    ASSERT_EQ(CDD_C_SUCCESS,
              component_path_item_is_referenced(&spec, "my_item", &bval));
    ASSERT_EQ(1, bval);
    paths[1].ref = cdd_test_mut_str("#/components/pathItems/other_item");
    ASSERT_EQ(CDD_C_SUCCESS,
              component_path_item_is_referenced(&spec, "my_item", &bval));
    ASSERT_EQ(0, bval);
    paths[1].ref = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              component_path_item_is_referenced(&spec, "my_item", &bval));
    ASSERT_EQ(0, bval);
    memset(&spec, 0, sizeof(spec));

    /* 11. callback_ref_matches_component */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              callback_ref_matches_component(&spec, "ref", "cb", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              callback_ref_matches_component(NULL, NULL, "cb", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS,
              callback_ref_matches_component(&spec, "ref", NULL, &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS,
              callback_ref_matches_component(&spec, "bad_prefix", "cb", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS,
              callback_ref_matches_component(
                  &spec, "#/components/callbacks/my_cb", "my_cb", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS,
              callback_ref_matches_component(
                  &spec, "#/components/callbacks/other_cb", "my_cb", &bval));
    ASSERT_EQ(0, bval);
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              callback_ref_matches_component(
                  &spec, "#/components/callbacks/my_cb", "my_cb", &bval));
    ASSERT_EQ(0, bval);
    g_cdd_alloc_fail = 0;

    /* 12. component_callback_is_referenced_in_ops */
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        component_callback_is_referenced_in_ops(ops, 1, &spec, "cb", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, component_callback_is_referenced_in_ops(
                                 NULL, 0, &spec, "cb", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, component_callback_is_referenced_in_ops(
                                 ops, 1, NULL, "cb", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, component_callback_is_referenced_in_ops(
                                 ops, 1, &spec, NULL, &bval));
    ASSERT_EQ(0, bval);
    ops[0].callbacks = cbs;
    ops[0].n_callbacks = 1;
    cbs[0].ref = cdd_test_mut_str("#/components/callbacks/my_cb");
    ASSERT_EQ(CDD_C_SUCCESS, component_callback_is_referenced_in_ops(
                                 ops, 1, &spec, "my_cb", &bval));
    ASSERT_EQ(1, bval);
    cbs[0].ref = cdd_test_mut_str("#/components/callbacks/other_cb");
    ASSERT_EQ(CDD_C_SUCCESS, component_callback_is_referenced_in_ops(
                                 ops, 1, &spec, "my_cb", &bval));
    ASSERT_EQ(0, bval);
    cbs[0].ref = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, component_callback_is_referenced_in_ops(
                                 ops, 1, &spec, "my_cb", &bval));
    ASSERT_EQ(0, bval);

    /* 13. component_callback_is_referenced */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              component_callback_is_referenced(&spec, "cb", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(NULL, "cb", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, NULL, &bval));
    ASSERT_EQ(0, bval);
    /* referenced in paths operations */
    cbs[0].ref = cdd_test_mut_str("#/components/callbacks/my_cb");
    spec.paths = paths;
    spec.n_paths = 1;
    paths[0].operations = ops;
    paths[0].n_operations = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, "my_cb", &bval));
    ASSERT_EQ(1, bval);
    paths[0].n_operations = 0;
    paths[0].additional_operations = ops;
    paths[0].n_additional_operations = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, "my_cb", &bval));
    ASSERT_EQ(1, bval);
    paths[0].n_additional_operations = 0;

    /* referenced in webhooks operations */
    spec.webhooks = &paths[1];
    spec.n_webhooks = 1;
    paths[1].operations = ops;
    paths[1].n_operations = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, "my_cb", &bval));
    ASSERT_EQ(1, bval);
    paths[1].n_operations = 0;
    paths[1].additional_operations = ops;
    paths[1].n_additional_operations = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, "my_cb", &bval));
    ASSERT_EQ(1, bval);
    paths[1].n_additional_operations = 0;

    /* referenced in component_path_items */
    spec.paths = NULL;
    spec.n_paths = 0;
    spec.webhooks = &paths[1];
    spec.n_webhooks = 1;
    paths[1].operations = NULL;
    paths[1].n_operations = 0;
    spec.component_path_items = &paths[0];
    spec.n_component_path_items = 1;
    spec.component_path_item_names = &cpi_name;
    paths[0].operations = ops;
    paths[0].n_operations = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, "my_cb", &bval));
    ASSERT_EQ(1, bval);
    paths[0].n_operations = 0;
    paths[0].additional_operations = ops;
    paths[0].n_additional_operations = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, "my_cb", &bval));
    ASSERT_EQ(1, bval);

    /* If component_path_item is itself referenced, it is skipped */
    paths[1].ref = cdd_test_mut_str("#/components/pathItems/my_item");
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, "my_cb", &bval));
    ASSERT_EQ(0, bval);
    paths[1].ref = NULL;
    cbs[0].ref = NULL;

    /* component_path_items == NULL */
    spec.component_path_items = NULL;
    spec.n_component_path_items = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, "my_cb", &bval));
    ASSERT_EQ(0, bval);

    /* component_path_items != NULL && n_component_path_items == 0 */
    spec.component_path_items = paths;
    spec.n_component_path_items = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, "my_cb", &bval));
    ASSERT_EQ(0, bval);

    /* component_path_item_names == NULL */
    cbs[0].ref = cdd_test_mut_str("#/components/callbacks/my_cb");
    spec.component_path_items = &paths[0];
    spec.n_component_path_items = 1;
    spec.component_path_item_names = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              component_callback_is_referenced(&spec, "my_cb", &bval));
    ASSERT_EQ(1, bval);

    /* component_path_item_names[0] == NULL */
    {
      char *null_name = NULL;
      char **null_names = &null_name;
      spec.component_path_item_names = null_names;
      ASSERT_EQ(CDD_C_SUCCESS,
                component_callback_is_referenced(&spec, "my_cb", &bval));
      ASSERT_EQ(1, bval);
    }
  }
  cbs[0].ref = NULL;

  memset(&spec, 0, sizeof(spec));
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));

  /* 14. collect_callback_operation_ids_from_callbacks */
  ASSERT_EQ(CDD_C_SUCCESS, collect_callback_operation_ids_from_callbacks(
                               NULL, 0, &ids, &id_count, &id_cap));
  cbs[0].paths = NULL;
  cbs[0].n_paths = 0;
  ASSERT_EQ(CDD_C_SUCCESS, collect_callback_operation_ids_from_callbacks(
                               cbs, 1, &ids, &id_count, &id_cap));
  cbs[0].paths = paths;
  cbs[0].n_paths = 0;
  ASSERT_EQ(CDD_C_SUCCESS, collect_callback_operation_ids_from_callbacks(
                               cbs, 1, &ids, &id_count, &id_cap));
  cbs[0].paths = paths;
  cbs[0].n_paths = 1;
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ops[0].operation_id = cdd_test_mut_str("cb_op");
  ASSERT_EQ(CDD_C_SUCCESS, collect_callback_operation_ids_from_callbacks(
                               cbs, 1, &ids, &id_count, &id_cap));
  ASSERT_EQ(1, id_count);
  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* 15. collect_callback_operation_ids_from_operations */
  ASSERT_EQ(CDD_C_SUCCESS, collect_callback_operation_ids_from_operations(
                               NULL, 0, &ids, &id_count, &id_cap));
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS, collect_callback_operation_ids_from_operations(
                               ops, 1, &ids, &id_count, &id_cap));
  ASSERT_EQ(1, id_count);
  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* 16. collect_callback_operation_ids_from_paths */
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  ASSERT_EQ(CDD_C_SUCCESS, collect_callback_operation_ids_from_paths(
                               NULL, 0, &ids, &id_count, &id_cap));
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  paths[0].additional_operations = &ops[1];
  paths[0].n_additional_operations = 1;
  ops[1].callbacks = &cbs[1];
  ops[1].n_callbacks = 1;
  cbs[1].paths = &paths[1];
  cbs[1].n_paths = 1;
  paths[1].operations = &ops[0];
  paths[1].n_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, collect_callback_operation_ids_from_paths(
                               paths, 1, &ids, &id_count, &id_cap));
  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* additional_operations callback duplicate in
   * collect_callback_operation_ids_from_paths */
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  paths[0].additional_operations = ops;
  paths[0].n_additional_operations = 1;
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  cbs[0].paths = &paths[1];
  cbs[0].n_paths = 1;
  paths[1].operations = &ops[1];
  paths[1].n_operations = 1;
  ops[1].operation_id = cdd_test_mut_str("dup");
  ASSERT_EQ(CDD_C_SUCCESS,
            add_unique_operation_id(&ids, &id_count, &id_cap, "dup"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            collect_callback_operation_ids_from_paths(paths, 1, &ids, &id_count,
                                                      &id_cap));
  free_test_str_array(ids, id_count);
  ids = NULL;
  id_count = 0;
  id_cap = 0;

  /* 17. validate_unique_operation_ids */
  memset(&spec, 0, sizeof(spec));
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  ASSERT_EQ(CDD_C_SUCCESS, validate_unique_operation_ids(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Spec with component path item unreferenced vs referenced */
  spec.component_path_items = paths;
  spec.n_component_path_items = 1;
  spec.component_path_item_names = &cpi_name;
  ops[0].operation_id = cdd_test_mut_str("unique_cpi");
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  paths[0].additional_operations = NULL;
  paths[0].n_additional_operations = 0;
  ASSERT_EQ(CDD_C_SUCCESS, validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Spec with unreferenced component callback */
  cbs[0].name = cdd_test_mut_str("my_cb");
  cbs[0].paths = &paths[1];
  cbs[0].n_paths = 1;
  paths[1].operations = &ops[1];
  paths[1].n_operations = 1;
  ops[1].operation_id = cdd_test_mut_str("unique_cb");
  spec.component_callbacks = cbs;
  spec.n_component_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Duplicate between component path item and component callback */
  ops[1].operation_id = cdd_test_mut_str("unique_cpi");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_unique_operation_ids(&spec));

  /* Spec paths with duplicate */
  memset(&spec, 0, sizeof(spec));
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));
  spec.paths = paths;
  spec.n_paths = 1;
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  paths[0].additional_operations = &ops[1];
  paths[0].n_additional_operations = 1;
  ops[0].operation_id = cdd_test_mut_str("dup");
  ops[1].operation_id = cdd_test_mut_str("dup");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_unique_operation_ids(&spec));
  paths[0].n_additional_operations = 0;

  /* Spec webhooks with duplicate */
  spec.paths = NULL;
  spec.n_paths = 0;
  spec.webhooks = paths;
  spec.n_webhooks = 1;
  paths[0].additional_operations = &ops[1];
  paths[0].n_additional_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_unique_operation_ids(&spec));
  paths[0].n_additional_operations = 0;
  spec.webhooks = NULL;
  spec.n_webhooks = 0;

  /* Spec paths callback duplicate */
  spec.paths = paths;
  spec.n_paths = 1;
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  cbs[0].paths = &paths[1];
  cbs[0].n_paths = 1;
  paths[1].operations = ops;
  paths[1].n_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_unique_operation_ids(&spec));

  /* Spec webhooks callback duplicate */
  spec.paths = NULL;
  spec.n_paths = 0;
  spec.webhooks = paths;
  spec.n_webhooks = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_unique_operation_ids(&spec));
  memset(&spec, 0, sizeof(spec));
  memset(paths, 0, sizeof(paths));
  memset(ops, 0, sizeof(ops));
  memset(cbs, 0, sizeof(cbs));

  /* Spec with component_path_item_names == NULL and with null name */
  spec.component_path_items = paths;
  spec.n_component_path_items = 1;
  spec.component_path_item_names = NULL;
  ops[0].operation_id = cdd_test_mut_str("unique_cpi2");
  paths[0].operations = ops;
  paths[0].n_operations = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;
  {
    char *null_name = NULL;
    char **null_names = &null_name;
    spec.component_path_item_names = null_names;
    ASSERT_EQ(CDD_C_SUCCESS, validate_unique_operation_ids(&spec));
    spec.component_path_item_names = NULL;
  }

  /* Duplicate in component_path_items callback */
  ops[0].callbacks = cbs;
  ops[0].n_callbacks = 1;
  cbs[0].paths = &paths[1];
  cbs[0].n_paths = 1;
  paths[1].operations = ops;
  paths[1].n_operations = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_unique_operation_ids(&spec));
  ops[0].n_callbacks = 0;
  spec.component_path_items = NULL;
  spec.n_component_path_items = 0;

  /* Spec component_callbacks with name == NULL, and paths with n_paths == 0 */
  cbs[0].name = NULL;
  cbs[0].paths = paths;
  cbs[0].n_paths = 0;
  spec.component_callbacks = cbs;
  spec.n_component_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Spec with component_path_items != NULL && n_component_path_items == 0 */
  spec.component_path_items = paths;
  spec.n_component_path_items = 0;
  ASSERT_EQ(CDD_C_SUCCESS, validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;
  spec.component_path_items = NULL;

  /* Spec with component_callbacks != NULL && n_component_callbacks == 0 */
  spec.component_callbacks = cbs;
  spec.n_component_callbacks = 0;
  ASSERT_EQ(CDD_C_SUCCESS, validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Spec with unreferenced component_callback having paths == NULL */
  cbs[0].name = cdd_test_mut_str("cb_no_paths");
  cbs[0].paths = NULL;
  cbs[0].n_paths = 0;
  spec.component_callbacks = cbs;
  spec.n_component_callbacks = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_unique_operation_ids(&spec));
  spec.component_path_item_names = NULL;

  /* Spec with component_path_items having duplicate operation IDs */
  {
    struct OpenAPI_Operation dup_ops[2];
    memset(dup_ops, 0, sizeof(dup_ops));
    dup_ops[0].operation_id = cdd_test_mut_str("dup_id");
    dup_ops[1].operation_id = cdd_test_mut_str("dup_id");
    paths[0].operations = dup_ops;
    paths[0].n_operations = 2;
    paths[0].additional_operations = NULL;
    paths[0].n_additional_operations = 0;
    spec.component_path_items = paths;
    spec.n_component_path_items = 1;
    spec.component_path_item_names = NULL;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              validate_unique_operation_ids(&spec));
    spec.component_path_items = NULL;
    spec.n_component_path_items = 0;
    paths[0].operations = NULL;
    paths[0].n_operations = 0;
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_VALIDATION_OP_IDS_H */
