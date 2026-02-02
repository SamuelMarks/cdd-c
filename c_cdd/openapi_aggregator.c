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

#include "openapi_aggregator.h"
#include "str_utils.h"

/**
 * @brief Comparison function to find a path by route string.
 */
static struct OpenAPI_Path *find_path(struct OpenAPI_Spec *spec,
                                      const char *route) {
  size_t i;
  for (i = 0; i < spec->n_paths; ++i) {
    if (spec->paths[i].route && strcmp(spec->paths[i].route, route) == 0) {
      return &spec->paths[i];
    }
  }
  return NULL;
}

/**
 * @brief Append a new path object to the spec.
 */
static int append_path(struct OpenAPI_Spec *spec, const char *route,
                       struct OpenAPI_Path **out_ptr) {
  size_t new_count = spec->n_paths + 1;
  struct OpenAPI_Path *new_arr = (struct OpenAPI_Path *)realloc(
      spec->paths, new_count * sizeof(struct OpenAPI_Path));

  if (!new_arr)
    return ENOMEM;

  spec->paths = new_arr;
  spec->n_paths = new_count;

  /* Initialize new slot */
  *out_ptr = &spec->paths[new_count - 1];
  memset(*out_ptr, 0, sizeof(struct OpenAPI_Path));

  (*out_ptr)->route = c_cdd_strdup(route);
  if (!(*out_ptr)->route)
    return ENOMEM;

  return 0;
}

int openapi_aggregator_add_operation(struct OpenAPI_Spec *const spec,
                                     const char *const route,
                                     struct OpenAPI_Operation *const op) {
  struct OpenAPI_Path *target_path;
  struct OpenAPI_Operation *new_ops;
  size_t new_op_count;
  int rc;

  if (!spec || !route || !op) {
    return EINVAL;
  }

  /* 1. Find or Create Path */
  target_path = find_path(spec, route);
  if (!target_path) {
    rc = append_path(spec, route, &target_path);
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
  new_op_count = target_path->n_operations + 1;
  new_ops = (struct OpenAPI_Operation *)realloc(
      target_path->operations, new_op_count * sizeof(struct OpenAPI_Operation));

  if (!new_ops) {
    /* If path was newly created, it remains valid but empty.
       If we fail here, we don't have transaction rollback for the path
       creation, but in this simple model it's acceptable. */
    return ENOMEM;
  }

  target_path->operations = new_ops;
  target_path->n_operations = new_op_count;

  /* Transfer ownership of content from input op to stored op */
  /* Structure copy */
  target_path->operations[new_op_count - 1] = *op;

  /* Since we transferred ownership (pointers), the caller's struct is now
     pointing to aliased memory. The caller should NOT free the internals. We
     zero the caller's struct to prevent double-free mistakes. */
  memset(op, 0, sizeof(*op));

  return 0;
}
