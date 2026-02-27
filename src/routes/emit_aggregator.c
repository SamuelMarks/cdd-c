/**
 * @file openapi_aggregator.c
 * @brief Implementation of the OpenAPI Path Aggregator.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse_str.h"
#include "routes/emit_aggregator.h"

/**
 * @brief Comparison function to find a path by route string.
 */
static struct OpenAPI_Path *find_path_in_list(struct OpenAPI_Path *paths,
                                              size_t n_paths,
                                              const char *route) {
  size_t i;
  if (!paths || !route)
    return NULL;
  for (i = 0; i < n_paths; ++i) {
    if (paths[i].route && strcmp(paths[i].route, route) == 0) {
      return &paths[i];
    }
  }
  return NULL;
}

/**
 * @brief Append a new path object to a list.
 */
static int append_path_to_list(struct OpenAPI_Path **paths, size_t *n_paths,
                               const char *route,
                               struct OpenAPI_Path **out_ptr) {
  size_t new_count;
  struct OpenAPI_Path *new_arr;

  if (!paths || !n_paths || !route || !out_ptr)
    return EINVAL;

  new_count = *n_paths + 1;
  new_arr = (struct OpenAPI_Path *)realloc(
      *paths, new_count * sizeof(struct OpenAPI_Path));
  if (!new_arr)
    return ENOMEM;

  *paths = new_arr;
  *n_paths = new_count;

  /* Initialize new slot */
  *out_ptr = &(*paths)[new_count - 1];
  memset(*out_ptr, 0, sizeof(struct OpenAPI_Path));

  (*out_ptr)->route = c_cdd_strdup(route);
  if (!(*out_ptr)->route)
    return ENOMEM;

  return 0;
}

static int append_operation(struct OpenAPI_Operation **ops, size_t *count,
                            struct OpenAPI_Operation *op) {
  struct OpenAPI_Operation *new_ops;
  size_t new_count;

  if (!ops || !count || !op)
    return EINVAL;

  new_count = *count + 1;
  new_ops = (struct OpenAPI_Operation *)realloc(
      *ops, new_count * sizeof(struct OpenAPI_Operation));
  if (!new_ops)
    return ENOMEM;

  *ops = new_ops;
  (*ops)[new_count - 1] = *op;
  *count = new_count;
  memset(op, 0, sizeof(*op));
  return 0;
}

int openapi_aggregator_add_operation(struct OpenAPI_Spec *const spec,
                                     const char *const route,
                                     struct OpenAPI_Operation *const op) {
  struct OpenAPI_Path *target_path;
  int rc;

  if (!spec || !route || !op) {
    return EINVAL;
  }

  /* 1. Find or Create Path */
  target_path = find_path_in_list(spec->paths, spec->n_paths, route);
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

int openapi_aggregator_add_webhook_operation(
    struct OpenAPI_Spec *const spec, const char *const route,
    struct OpenAPI_Operation *const op) {
  struct OpenAPI_Path *target_path;
  int rc;

  if (!spec || !route || !op) {
    return EINVAL;
  }

  target_path = find_path_in_list(spec->webhooks, spec->n_webhooks, route);
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
