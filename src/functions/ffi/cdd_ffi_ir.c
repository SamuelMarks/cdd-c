#include "cdd_c_error.h"
/* clang-format off */
#include "../../include/ffi/cdd_ffi_ir.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Helper context for topological sort.
 */
typedef struct {
  cdd_ffi_ir_t *ir;
  int *visited; /* 0: unvisited, 1: visiting, 2: visited */
  cdd_ffi_ir_node_t *sorted_nodes;
  size_t sorted_count;
} toposort_ctx_t;

/**
 * @brief Finds the index of a node by name.
 * @param ir The IR.
 * @param name The name to find.
 * @return The index, or (size_t)-1 if not found.
 */
static size_t find_node_index(cdd_ffi_ir_t *ir, const char *name) {
  size_t i;
  /* printf("toposort searching for dep: %s\n", name); */
  for (i = 0; i < ir->nodes_count; i++) {
    if (ir->nodes[i].name && strcmp(ir->nodes[i].name, name) == 0) {
      return i;
    }
  }
  /* printf("toposort failed to find dep: %s\n", name); */
  return (size_t)-1;
}

/**
 * @brief DFS function for topological sorting.
 * @param ctx The sort context.
 * @param node_idx The index of the current node.
 */
static enum cdd_c_error toposort_dfs(toposort_ctx_t *ctx, size_t node_idx) {
  cdd_ffi_ir_node_t *node;
  size_t i;

  if (ctx->visited[node_idx] == 1) {
    /* Cycle detected, handle by ignoring for now to emit what we can,
       but ideally C structs with pointers can have cycles.
       If it's a pointer to a struct, it shouldn't be a strict dependency,
       but we are simplifying. We'll just return to break the cycle. */
    return CDD_C_SUCCESS;
  }
  if (ctx->visited[node_idx] == 2) {
    /* Already processed */
    return CDD_C_SUCCESS;
  }

  ctx->visited[node_idx] = 1;
  node = &ctx->ir->nodes[node_idx];

  /* Explore dependencies */
  if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION ||
      node->kind == CDD_FFI_NODE_FUNCTION) {
    for (i = 0; i < node->fields_count; i++) {
      cdd_ffi_type_t *type = &node->fields[i].type;
      if (type->ref_name) {
        /* If it's a pointer depth > 0, it's a forward declaration acceptable
           case, but we'll sort it anyway if we find it. */
        size_t dep_idx = find_node_index(ctx->ir, type->ref_name);
        if (dep_idx != (size_t)-1) {
          {
            enum cdd_c_error rc = toposort_dfs(ctx, dep_idx);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      }
    }
  }

  if (node->kind == CDD_FFI_NODE_TYPEDEF ||
      node->kind == CDD_FFI_NODE_FUNCTION) {
    if (node->return_or_base_type.ref_name) {
      size_t dep_idx =
          find_node_index(ctx->ir, node->return_or_base_type.ref_name);
      if (dep_idx != (size_t)-1) {
        {
          enum cdd_c_error rc = toposort_dfs(ctx, dep_idx);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }

  ctx->visited[node_idx] = 2;
  /* Shallow copy the node to the sorted array */
  ctx->sorted_nodes[ctx->sorted_count++] = *node;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_ffi_ir_calloc_fail = 0;
C_CDD_EXPORT int g_cdd_ffi_ir_malloc_fail = 0;
#endif

enum cdd_c_error cdd_ffi_ir_topological_sort(cdd_ffi_ir_t *ir) {
  toposort_ctx_t ctx;
  size_t i;

  if (!ir) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (ir->nodes_count == 0) {
    return CDD_C_SUCCESS;
  }

  ctx.ir = ir;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_ffi_ir_calloc_fail)
    ctx.visited = NULL;
  else
#endif
    ctx.visited = (int *)calloc(ir->nodes_count, sizeof(int));
  if (!ctx.visited) {
    return CDD_C_ERROR_MEMORY;
  }

#ifdef CDD_BUILD_TESTS
  if (g_cdd_ffi_ir_malloc_fail)
    ctx.sorted_nodes = NULL;
  else
#endif
    ctx.sorted_nodes = (cdd_ffi_ir_node_t *)malloc(ir->nodes_count *
                                                   sizeof(cdd_ffi_ir_node_t));
  if (!ctx.sorted_nodes) {
    free(ctx.visited);
    return CDD_C_ERROR_MEMORY;
  }
  ctx.sorted_count = 0;

  for (i = 0; i < ir->nodes_count; i++) {
    if (ctx.visited[i] == 0) {
      {
        enum cdd_c_error rc = toposort_dfs(&ctx, i);
        if (rc != CDD_C_SUCCESS) {
          free(ctx.visited);
          free(ctx.sorted_nodes);
          return rc;
        }
      }
    }
  }

  /* Replace original nodes array with the sorted one */
  free(ir->nodes);
  ir->nodes = ctx.sorted_nodes;
  ir->nodes_capacity = ir->nodes_count;

  free(ctx.visited);
  return CDD_C_SUCCESS;
}

static void free_type_recursive(cdd_ffi_type_t *type) {
  if (type->ref_name) {
    free(type->ref_name);
    type->ref_name = NULL;
  }
  if (type->template_args) {
    size_t i;
    for (i = 0; i < type->template_args_count; i++) {
      free_type_recursive(&type->template_args[i]);
    }
    free(type->template_args);
    type->template_args = NULL;
  }
}

/**
 * @brief Frees all memory associated with the FFI IR to guarantee zero-leaks.
 */
void cdd_ffi_ir_free(cdd_ffi_ir_t *ir) {
  size_t i, j;

  if (!ir) {
    return;
  }

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];

    if (node->name) {
      free(node->name);
    }
    if (node->doc) {
      free(node->doc);
    }
    if (node->evaluated_value) {
      free(node->evaluated_value);
    }
    free_type_recursive(&node->return_or_base_type);

    /* Free fields / arguments */
    if (node->fields) {
      for (j = 0; j < node->fields_count; j++) {
        if (node->fields[j].name) {
          free(node->fields[j].name);
        }
        if (node->fields[j].doc) {
          free(node->fields[j].doc);
        }
        if (node->fields[j].array_length_ref) {
          free(node->fields[j].array_length_ref);
        }
        free_type_recursive(&node->fields[j].type);
      }
      free(node->fields);
    }

    /* Free base classes */
    if (node->base_classes) {
      for (j = 0; j < node->base_classes_count; j++) {
        if (node->base_classes[j].name) {
          free(node->base_classes[j].name);
        }
        if (node->base_classes[j].access) {
          free(node->base_classes[j].access);
        }
      }
      free(node->base_classes);
    }

    /* Free virtual methods */
    if (node->virtual_methods) {
      for (j = 0; j < node->virtual_methods_count; j++) {
        size_t a;
        if (node->virtual_methods[j].name) {
          free(node->virtual_methods[j].name);
        }
        free_type_recursive(&node->virtual_methods[j].return_type);
        if (node->virtual_methods[j].args) {
          for (a = 0; a < node->virtual_methods[j].args_count; a++) {
            if (node->virtual_methods[j].args[a].name) {
              free(node->virtual_methods[j].args[a].name);
            }
            if (node->virtual_methods[j].args[a].doc) {
              free(node->virtual_methods[j].args[a].doc);
            }
            free_type_recursive(&node->virtual_methods[j].args[a].type);
          }
          free(node->virtual_methods[j].args);
        }
      }
      free(node->virtual_methods);
    }

    /* Free variants */
    if (node->variants) {
      for (j = 0; j < node->variants_count; j++) {
        if (node->variants[j].name) {
          free(node->variants[j].name);
        }
        if (node->variants[j].value) {
          free(node->variants[j].value);
        }
        if (node->variants[j].doc) {
          free(node->variants[j].doc);
        }
      }
      free(node->variants);
    }
  }

  if (ir->nodes) {
    free(ir->nodes);
  }

  ir->nodes = NULL;
  ir->nodes_count = 0;
  ir->nodes_capacity = 0;
}
