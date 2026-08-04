/**
 * @file aggregator.c
 * @brief Implementation of route aggregation.
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/str.h"
#include "routes/emit/aggregator.h"
#include "c_cdd/log.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_aggregator_fail_path_realloc = 0;
C_CDD_EXPORT int g_cdd_aggregator_fail_ops_realloc = 0;
C_CDD_EXPORT int g_cdd_aggregator_fail_route_strdup = 0;
#endif

/**
 * @brief Comparison function to find a path by route string.
 */
static cdd_c_error_t find_path_in_list(struct OpenAPI_Path *paths,
                                       size_t n_paths, const char *route,
                                       struct OpenAPI_Path **_out_val) {
  size_t i;
  for (i = 0; i < n_paths; ++i) {
    if (paths[i].route && strcmp(paths[i].route, route) == 0) {
      {
        *_out_val = &paths[i];
        return CDD_C_SUCCESS;
      }
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Append a new path object to a list.
 */
static cdd_c_error_t append_path_to_list(struct OpenAPI_Path **paths,
                                         size_t *n_paths, const char *route,
                                         struct OpenAPI_Path **out_ptr) {
  char *_ast_strdup_0 = NULL;
  size_t new_count;
  struct OpenAPI_Path *new_arr;

  new_count = *n_paths + 1;
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_aggregator_fail_path_realloc;
    if (g_cdd_aggregator_fail_path_realloc) {
      new_arr = NULL;
    } else {
      new_arr = (struct OpenAPI_Path *)realloc(
          *paths, new_count * sizeof(struct OpenAPI_Path));
    }
  }
#else
  new_arr = (struct OpenAPI_Path *)realloc(
      *paths, new_count * sizeof(struct OpenAPI_Path));
#endif
  if (!new_arr) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  *paths = new_arr;
  *n_paths = new_count;

  /* Initialize new slot */
  *out_ptr = &(*paths)[new_count - 1];
  memset(*out_ptr, 0, sizeof(struct OpenAPI_Path));

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_aggregator_fail_route_strdup;
    if (g_cdd_aggregator_fail_route_strdup) {
      (*out_ptr)->route = NULL;
    } else {
      (*out_ptr)->route = (c_cdd_strdup(route, &_ast_strdup_0), _ast_strdup_0);
    }
  }
#else
  (*out_ptr)->route = (c_cdd_strdup(route, &_ast_strdup_0), _ast_strdup_0);
#endif
  if (!(*out_ptr)->route)
    return CDD_C_ERROR_MEMORY;

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the append operation operation.
 */
static cdd_c_error_t append_operation(struct OpenAPI_Operation **ops,
                                      size_t *count,
                                      struct OpenAPI_Operation *op) {
  struct OpenAPI_Operation *new_ops;
  size_t new_count;

  new_count = *count + 1;
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_aggregator_fail_ops_realloc;
    if (g_cdd_aggregator_fail_ops_realloc) {
      new_ops = NULL;
    } else {
      new_ops = (struct OpenAPI_Operation *)realloc(
          *ops, new_count * sizeof(struct OpenAPI_Operation));
    }
  }
#else
  new_ops = (struct OpenAPI_Operation *)realloc(
      *ops, new_count * sizeof(struct OpenAPI_Operation));
#endif
  if (!new_ops) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  *ops = new_ops;
  (*ops)[new_count - 1] = *op;
  *count = new_count;
  memset(op, 0, sizeof(*op));
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the openapi aggregator add operation operation.
 */
cdd_c_error_t openapi_aggregator_add_operation(struct OpenAPI_Spec *spec,
                                               const char *route,
                                               struct OpenAPI_Operation *op) {
  struct OpenAPI_Path *_ast_find_path_in_list_0;
  struct OpenAPI_Path *target_path;
  int rc;

  if (!spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!route)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!op)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Find or Create Path */
  target_path = (find_path_in_list(spec->paths, spec->n_paths, route,
                                   &_ast_find_path_in_list_0),
                 _ast_find_path_in_list_0);
  if (!target_path) {
    rc = append_path_to_list(&spec->paths, &spec->n_paths, route, &target_path);
    if (rc != 0)
      return rc;
  }

  /* 2. Check for duplicate verbs types?
     The OpenAPI spec allows one Operation per Verb per Path.
     If we try to add a duplicate GET to /path, we overwrite or fail.
     This implementation appends, trusting the final output writer or user logic
     to resolve conflicts (or simpler: overwrites if array logic was tailored,
     but array append means multiple ops with same verb possible which is
     invalid JSON structure but valid memory structure until write time).

     For robustness, we just append here.
  */

  /* 3. Append Operation to Path */
  if (op->is_additional) {
    return append_operation(&target_path->additional_operations,
                            &target_path->n_additional_operations, op);
  }
  return append_operation(&target_path->operations, &target_path->n_operations,
                          op);
}

/**
 * @brief Executes the openapi aggregator add webhook operation operation.
 */
cdd_c_error_t
openapi_aggregator_add_webhook_operation(struct OpenAPI_Spec *spec,
                                         const char *route,
                                         struct OpenAPI_Operation *op) {
  struct OpenAPI_Path *_ast_find_path_in_list_1;
  struct OpenAPI_Path *target_path;
  int rc;

  if (!spec)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!route)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!op)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  target_path = (find_path_in_list(spec->webhooks, spec->n_webhooks, route,
                                   &_ast_find_path_in_list_1),
                 _ast_find_path_in_list_1);
  if (!target_path) {
    rc = append_path_to_list(&spec->webhooks, &spec->n_webhooks, route,
                             &target_path);
    if (rc != 0)
      return rc;
  }

  if (op->is_additional) {
    return append_operation(&target_path->additional_operations,
                            &target_path->n_additional_operations, op);
  }
  return append_operation(&target_path->operations, &target_path->n_operations,
                          op);
}
