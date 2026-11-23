/**
 * @file cdd_ffi_ir.c
 * @brief Core FFI Intermediate Representation topological sorting, target ABI
 * byte-layout computation, and recursive memory deallocation.
 */
/* clang-format off */
#include "cdd_c_error.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt_msvc.h"

#include "../../include/ffi/cdd_ffi_ir.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Helper context for topological sort.
 */
typedef struct {
  cdd_ffi_ir_t *ir; /**< The FFI IR undergoing topological sort */
  int *visited; /**< Visited array (0: unvisited, 1: visiting, 2: visited) */
  cdd_ffi_ir_node_t *sorted_nodes; /**< Array of sorted IR nodes */
  size_t sorted_count;             /**< Number of sorted nodes populated */
} toposort_ctx_t;

/**
 * @brief Helper context for target ABI byte-layout computation.
 */
typedef struct {
  cdd_ffi_ir_t *ir;          /**< Target FFI IR */
  cdd_ffi_abi_profile_t abi; /**< Effective target ABI profile */
  int *state; /**< Per-node state (0: uncomputed, 1: computing, 2: computed) */
} layout_ctx_t;

/**
 * @brief Finds the index of a node by name.
 *
 * @param[in] ir The IR to search.
 * @param[in] name The node name to find.
 * @param[out] out_idx Pointer to receive the matched node index.
 * @param[out] out_found Pointer to receive 1 if found, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static NO_DISCARD cdd_c_error_t find_node_index(const cdd_ffi_ir_t *ir,
                                                const char *name,
                                                size_t *out_idx,
                                                int *out_found) {
  size_t i;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_found = 0;
  for (i = 0; i < ir->nodes_count; i++) {
    if (ir->nodes[i].name && strcmp(ir->nodes[i].name, name) == 0) {
      *out_idx = i;
      *out_found = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
/** @brief Flag to simulate calloc failure in `cdd_ffi_ir_topological_sort`. */
C_CDD_EXPORT int g_cdd_ffi_ir_calloc_fail = 0;
/** @brief Flag to simulate malloc failure in `cdd_ffi_ir_topological_sort`. */
C_CDD_EXPORT int g_cdd_ffi_ir_malloc_fail = 0;
/** @brief Counter to simulate DFS failure in `cdd_ffi_ir_topological_sort`. */
C_CDD_EXPORT int g_cdd_ffi_ir_toposort_fail = 0;
#endif

static NO_DISCARD cdd_c_error_t toposort_dfs(toposort_ctx_t *ctx,
                                             size_t node_idx);

/**
 * @brief Recursively visits type dependencies (including template args and
 * typed function-pointer signatures) during topological sorting.
 *
 * @param[in,out] ctx Topological sort context.
 * @param[in] type FFI type descriptor to inspect.
 * @return CDD_C_SUCCESS on success, or error code on DFS failure.
 */
static NO_DISCARD cdd_c_error_t
toposort_visit_type(toposort_ctx_t *ctx, const cdd_ffi_type_t *type) {
  size_t k;
  cdd_c_error_t rc;

  if (type->ref_name) {
    size_t dep_idx = 0;
    int found = 0;
    rc = find_node_index(ctx->ir, type->ref_name, &dep_idx, &found);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (found) {
      rc = toposort_dfs(ctx, dep_idx);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (type->template_args) {
    for (k = 0; k < type->template_args_count; k++) {
      rc = toposort_visit_type(ctx, &type->template_args[k]);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (type->fn_return_type) {
    rc = toposort_visit_type(ctx, type->fn_return_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  if (type->fn_params) {
    for (k = 0; k < type->fn_params_count; k++) {
      rc = toposort_visit_type(ctx, &type->fn_params[k].type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief DFS function for topological sorting.
 *
 * @param[in,out] ctx The sort context.
 * @param[in] node_idx The index of the current node.
 * @return CDD_C_SUCCESS on success, or an error code on failure.
 */
static NO_DISCARD cdd_c_error_t toposort_dfs(toposort_ctx_t *ctx,
                                             size_t node_idx) {
  cdd_ffi_ir_node_t *node;
  size_t i;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_ffi_ir_toposort_fail > 0) {
    if (--g_cdd_ffi_ir_toposort_fail == 0) {
      return CDD_C_ERROR_MEMORY;
    }
  }
#endif

  if (ctx->visited[node_idx] == 1) {
    return CDD_C_SUCCESS;
  }
  if (ctx->visited[node_idx] == 2) {
    return CDD_C_SUCCESS;
  }

  ctx->visited[node_idx] = 1;
  node = &ctx->ir->nodes[node_idx];

  if (node->kind == CDD_FFI_NODE_STRUCT || node->kind == CDD_FFI_NODE_UNION ||
      node->kind == CDD_FFI_NODE_FUNCTION) {
    for (i = 0; i < node->fields_count; i++) {
      rc = toposort_visit_type(ctx, &node->fields[i].type);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  if (node->kind == CDD_FFI_NODE_TYPEDEF ||
      node->kind == CDD_FFI_NODE_FUNCTION) {
    rc = toposort_visit_type(ctx, &node->return_or_base_type);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  ctx->visited[node_idx] = 2;
  ctx->sorted_nodes[ctx->sorted_count++] = *node;
  return CDD_C_SUCCESS;
}

/*
 * @brief Topologically sorts the nodes in `ir` so dependencies precede
 * dependents.
 *
 * @param[in,out] ir FFI IR structure whose nodes will be topologically sorted
 * in-place.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if `ir` is
 * NULL, or `CDD_C_ERROR_MEMORY` on allocation failure.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_ir_topological_sort(cdd_ffi_ir_t *ir) {
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
    ctx.visited = (int *)C_CDD_CALLOC(ir->nodes_count, sizeof(int));
  if (!ctx.visited) {
    return CDD_C_ERROR_MEMORY;
  }

#ifdef CDD_BUILD_TESTS
  if (g_cdd_ffi_ir_malloc_fail)
    ctx.sorted_nodes = NULL;
  else
#endif
    ctx.sorted_nodes = (cdd_ffi_ir_node_t *)C_CDD_MALLOC(
        ir->nodes_count * sizeof(cdd_ffi_ir_node_t));
  if (!ctx.sorted_nodes) {
    C_CDD_FREE(ctx.visited);
    return CDD_C_ERROR_MEMORY;
  }
  ctx.sorted_count = 0;

  for (i = 0; i < ir->nodes_count; i++) {
    if (ctx.visited[i] == 0) {
      cdd_c_error_t rc;
      rc = toposort_dfs(&ctx, i);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(ctx.visited);
        C_CDD_FREE(ctx.sorted_nodes);
        return rc;
      }
    }
  }

  C_CDD_FREE(ir->nodes);
  ir->nodes = ctx.sorted_nodes;
  ir->nodes_capacity = ir->nodes_count;

  C_CDD_FREE(ctx.visited);
  return CDD_C_SUCCESS;
}

/*
 * @brief Initializes a `cdd_ffi_abi_profile_t` for a predefined target ABI
 * model.
 *
 * @param[in] kind Predefined ABI model (`CDD_FFI_ABI_HOST`, `LP64`, `LLP64`, or
 * `ILP32`).
 * @param[out] out_profile Destination ABI profile structure to populate.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT on NULL or
 * invalid `kind`.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_abi_profile_init(
    cdd_ffi_abi_kind_t kind, cdd_ffi_abi_profile_t *out_profile) {
  cdd_ffi_abi_kind_t eff_kind;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif

  if (!out_profile)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(out_profile, 0, sizeof(*out_profile));
  eff_kind = kind;
  if (eff_kind == CDD_FFI_ABI_HOST) {
#if defined(_WIN64)
    eff_kind = CDD_FFI_ABI_LLP64;
#elif defined(__LP64__) || defined(_LP64) || defined(__x86_64__) ||            \
    defined(__aarch64__) || defined(__ppc64__)
    eff_kind = CDD_FFI_ABI_LP64;
#else
    eff_kind = CDD_FFI_ABI_ILP32;
#endif
  }

  out_profile->kind = eff_kind;
  out_profile->enum_size = 4;
  out_profile->enum_align = 4;

  switch (eff_kind) {
  case CDD_FFI_ABI_LP64:
    out_profile->pointer_size = 8;
    out_profile->pointer_align = 8;
    out_profile->long_size = 8;
    out_profile->long_align = 8;
    out_profile->int64_align = 8;
    out_profile->double_align = 8;
    break;
  case CDD_FFI_ABI_LLP64:
    out_profile->pointer_size = 8;
    out_profile->pointer_align = 8;
    out_profile->long_size = 4;
    out_profile->long_align = 4;
    out_profile->int64_align = 8;
    out_profile->double_align = 8;
    break;
  case CDD_FFI_ABI_ILP32:
    out_profile->pointer_size = 4;
    out_profile->pointer_align = 4;
    out_profile->long_size = 4;
    out_profile->long_align = 4;
    out_profile->int64_align = 4;
    out_profile->double_align = 4;
    break;
  default:
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Rounds `offset` up to the next multiple of `align` with overflow
 * checking.
 *
 * @param[in] offset Unaligned byte offset.
 * @param[in] align Required power-of-two/positive alignment in bytes.
 * @param[out] out_aligned Receives aligned byte offset.
 * @param[out] out_pad Receives number of padding bytes added.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT on zero
 * alignment or size_t overflow.
 */
static NO_DISCARD cdd_c_error_t align_up_checked(size_t offset, size_t align,
                                                 size_t *out_aligned,
                                                 size_t *out_pad) {
  size_t rem;
  size_t pad;
  size_t max_sz;

  max_sz = (size_t)-1;
  rem = offset % align;
  pad = (rem == 0) ? 0 : (align - rem);
  if (offset > max_sz - pad)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_pad = pad;
  *out_aligned = offset + pad;
  return CDD_C_SUCCESS;
}

/**
 * @brief Multiplies two `size_t` values with overflow checking.
 *
 * @param[in] a Multiplicand.
 * @param[in] b Multiplier.
 * @param[out] out_prod Receives `a * b`.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT on
 * overflow.
 */
static NO_DISCARD cdd_c_error_t mul_size_checked(size_t a, size_t b,
                                                 size_t *out_prod) {
  size_t max_sz;

  max_sz = (size_t)-1;
  if (a != 0 && b > max_sz / a)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_prod = a * b;
  return CDD_C_SUCCESS;
}

static NO_DISCARD cdd_c_error_t compute_node_layout(layout_ctx_t *ctx,
                                                    size_t idx);

/**
 * @brief Resolves standard C built-in typedef names (`size_t`, `long`, etc.)
 * against the active ABI profile.
 *
 * @param[in] abi Active ABI profile.
 * @param[in] name Typedef identifier string.
 * @param[out] out_size Receives resolved byte size.
 * @param[out] out_align Receives resolved byte alignment.
 * @param[out] out_found Receives 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static NO_DISCARD cdd_c_error_t
resolve_builtin_typedef(const cdd_ffi_abi_profile_t *abi, const char *name,
                        size_t *out_size, size_t *out_align, int *out_found) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_found = 1;
  if (strcmp(name, "size_t") == 0 || strcmp(name, "ssize_t") == 0 ||
      strcmp(name, "ptrdiff_t") == 0 || strcmp(name, "intptr_t") == 0 ||
      strcmp(name, "uintptr_t") == 0) {
    *out_size = abi->pointer_size;
    *out_align = abi->pointer_align;
    return CDD_C_SUCCESS;
  }
  if (strcmp(name, "long") == 0 || strcmp(name, "unsigned long") == 0 ||
      strcmp(name, "signed long") == 0 || strcmp(name, "long int") == 0 ||
      strcmp(name, "unsigned long int") == 0) {
    *out_size = abi->long_size;
    *out_align = abi->long_align;
    return CDD_C_SUCCESS;
  }
  if (strcmp(name, "int8_t") == 0 || strcmp(name, "uint8_t") == 0 ||
      strcmp(name, "char") == 0 || strcmp(name, "unsigned char") == 0 ||
      strcmp(name, "signed char") == 0 || strcmp(name, "bool") == 0 ||
      strcmp(name, "_Bool") == 0) {
    *out_size = 1;
    *out_align = 1;
    return CDD_C_SUCCESS;
  }
  if (strcmp(name, "int16_t") == 0 || strcmp(name, "uint16_t") == 0 ||
      strcmp(name, "short") == 0 || strcmp(name, "unsigned short") == 0) {
    *out_size = 2;
    *out_align = 2;
    return CDD_C_SUCCESS;
  }
  if (strcmp(name, "int32_t") == 0 || strcmp(name, "uint32_t") == 0 ||
      strcmp(name, "int") == 0 || strcmp(name, "unsigned int") == 0 ||
      strcmp(name, "float") == 0) {
    *out_size = 4;
    *out_align = 4;
    return CDD_C_SUCCESS;
  }
  if (strcmp(name, "int64_t") == 0 || strcmp(name, "uint64_t") == 0 ||
      strcmp(name, "long long") == 0 ||
      strcmp(name, "unsigned long long") == 0) {
    *out_size = 8;
    *out_align = abi->int64_align;
    return CDD_C_SUCCESS;
  }
  if (strcmp(name, "double") == 0) {
    *out_size = 8;
    *out_align = abi->double_align;
    return CDD_C_SUCCESS;
  }
  *out_found = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Resolves the byte size and natural alignment of a `cdd_ffi_type_t`.
 *
 * @param[in,out] ctx Layout computation context.
 * @param[in] type FFI type descriptor to resolve.
 * @param[out] out_size Receives resolved byte size.
 * @param[out] out_align Receives resolved natural alignment in bytes.
 * @return CDD_C_SUCCESS on success, or error code on cycle/unknown ref/invalid.
 */
static NO_DISCARD cdd_c_error_t compute_type_layout(layout_ctx_t *ctx,
                                                    const cdd_ffi_type_t *type,
                                                    size_t *out_size,
                                                    size_t *out_align) {
  size_t elem_size;
  size_t elem_align;
  size_t dep_idx = 0;
  cdd_c_error_t rc;

  elem_size = 0;
  elem_align = 1;

  if (type->pointer_depth > 0 || type->kind == CDD_FFI_KIND_OPAQUE_PTR ||
      type->kind == CDD_FFI_KIND_FUNCTION_PTR ||
      type->kind == CDD_FFI_KIND_STD_SHARED_PTR ||
      type->kind == CDD_FFI_KIND_STD_UNIQUE_PTR) {
    elem_size = ctx->abi.pointer_size;
    elem_align = ctx->abi.pointer_align;
  } else {
    switch (type->kind) {
    case CDD_FFI_KIND_VOID:
      elem_size = 0;
      elem_align = 1;
      break;
    case CDD_FFI_KIND_BOOL:
    case CDD_FFI_KIND_INT8:
    case CDD_FFI_KIND_UINT8:
      elem_size = 1;
      elem_align = 1;
      break;
    case CDD_FFI_KIND_INT16:
    case CDD_FFI_KIND_UINT16:
      elem_size = 2;
      elem_align = 2;
      break;
    case CDD_FFI_KIND_INT32:
    case CDD_FFI_KIND_UINT32:
    case CDD_FFI_KIND_FLOAT32:
      elem_size = 4;
      elem_align = 4;
      break;
    case CDD_FFI_KIND_INT64:
    case CDD_FFI_KIND_UINT64:
      elem_size = 8;
      elem_align = ctx->abi.int64_align;
      break;
    case CDD_FFI_KIND_FLOAT64:
      elem_size = 8;
      elem_align = ctx->abi.double_align;
      break;
    case CDD_FFI_KIND_STD_STRING:
    case CDD_FFI_KIND_STD_VECTOR:
      elem_size = 3 * ctx->abi.pointer_size;
      elem_align = ctx->abi.pointer_align;
      break;
    case CDD_FFI_KIND_ENUM_REF: {
      int found_node = 0;
      if (type->ref_name) {
        rc = find_node_index(ctx->ir, type->ref_name, &dep_idx, &found_node);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (found_node) {
        rc = compute_node_layout(ctx, dep_idx);
        if (rc != CDD_C_SUCCESS)
          return rc;
        elem_size = ctx->ir->nodes[dep_idx].size_bytes;
        elem_align = ctx->ir->nodes[dep_idx].align_bytes;
      } else if (ctx->abi.allow_unresolved_refs) {
        elem_size = ctx->abi.enum_size;
        elem_align = ctx->abi.enum_align;
      } else if (!type->ref_name) {
        return CDD_C_ERROR_INVALID_ARGUMENT;
      } else {
        return CDD_C_ERROR_NOT_FOUND;
      }
      break;
    }
    case CDD_FFI_KIND_TYPEDEF_REF: {
      int found_bi = 0;
      int found_node = 0;
      if (!type->ref_name) {
        if (ctx->abi.allow_unresolved_refs) {
          elem_size = ctx->abi.pointer_size;
          elem_align = ctx->abi.pointer_align;
          break;
        }
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      rc = resolve_builtin_typedef(&ctx->abi, type->ref_name, &elem_size,
                                   &elem_align, &found_bi);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (found_bi) {
        break;
      }
      rc = find_node_index(ctx->ir, type->ref_name, &dep_idx, &found_node);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (found_node) {
        rc = compute_node_layout(ctx, dep_idx);
        if (rc != CDD_C_SUCCESS)
          return rc;
        elem_size = ctx->ir->nodes[dep_idx].size_bytes;
        elem_align = ctx->ir->nodes[dep_idx].align_bytes;
      } else if (ctx->abi.allow_unresolved_refs) {
        elem_size = ctx->abi.pointer_size;
        elem_align = ctx->abi.pointer_align;
      } else {
        return CDD_C_ERROR_NOT_FOUND;
      }
      break;
    }
    case CDD_FFI_KIND_STRUCT_REF:
    case CDD_FFI_KIND_TEMPLATE_STRUCT_REF: {
      int found_node = 0;
      if (!type->ref_name) {
        if (ctx->abi.allow_unresolved_refs) {
          elem_size = ctx->abi.pointer_size;
          elem_align = ctx->abi.pointer_align;
          break;
        }
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      rc = find_node_index(ctx->ir, type->ref_name, &dep_idx, &found_node);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (found_node) {
        rc = compute_node_layout(ctx, dep_idx);
        if (rc != CDD_C_SUCCESS)
          return rc;
        elem_size = ctx->ir->nodes[dep_idx].size_bytes;
        elem_align = ctx->ir->nodes[dep_idx].align_bytes;
      } else if (ctx->abi.allow_unresolved_refs) {
        elem_size = ctx->abi.pointer_size;
        elem_align = ctx->abi.pointer_align;
      } else {
        return CDD_C_ERROR_NOT_FOUND;
      }
      break;
    }
    default:
      if (ctx->abi.allow_unresolved_refs) {
        elem_size = ctx->abi.pointer_size;
        elem_align = ctx->abi.pointer_align;
        break;
      }
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }

  if (type->array_size > 0) {
    rc = mul_size_checked(elem_size, (size_t)type->array_size, out_size);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_align = elem_align;
  } else {
    *out_size = elem_size;
    *out_align = elem_align;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Computes and caches the ABI byte layout of node `idx` in `ctx->ir`.
 *
 * @param[in,out] ctx Layout computation context.
 * @param[in] idx Index of node in `ctx->ir->nodes`.
 * @return CDD_C_SUCCESS on success, or error code on cycle/unresolved ref.
 */
static NO_DISCARD cdd_c_error_t compute_node_layout(layout_ctx_t *ctx,
                                                    size_t idx) {
  cdd_ffi_ir_node_t *node;
  size_t j;
  cdd_c_error_t rc;
  size_t max_sz;

  if (ctx->state[idx] == 2)
    return CDD_C_SUCCESS;
  if (ctx->state[idx] == 1)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  ctx->state[idx] = 1;
  node = &ctx->ir->nodes[idx];
  max_sz = (size_t)-1;

  switch (node->kind) {
  case CDD_FFI_NODE_ENUM:
    node->size_bytes = ctx->abi.enum_size;
    node->align_bytes = ctx->abi.enum_align;
    node->trailing_pad_bytes = 0;
    break;

  case CDD_FFI_NODE_TYPEDEF: {
    rc = compute_type_layout(ctx, &node->return_or_base_type, &node->size_bytes,
                             &node->align_bytes);
    if (rc != CDD_C_SUCCESS)
      return rc;
    node->trailing_pad_bytes = 0;
    break;
  }

  case CDD_FFI_NODE_STRUCT: {
    size_t cur_offset;
    size_t max_align;
    size_t bit_unit_size;
    size_t bit_unit_offset;
    size_t bits_used;

    cur_offset = 0;
    max_align = 1;
    bit_unit_size = 0;
    bit_unit_offset = 0;
    bits_used = 0;

    for (j = 0; j < node->fields_count; j++) {
      cdd_ffi_field_t *f;
      size_t f_size;
      size_t f_align;
      size_t eff_align;
      size_t aligned_off;
      size_t pad_before;

      f = &node->fields[j];
      rc = compute_type_layout(ctx, &f->type, &f_size, &f_align);
      if (rc != CDD_C_SUCCESS)
        return rc;

      eff_align = node->is_packed ? 1 : f_align;
      if (eff_align > max_align)
        max_align = eff_align;

      if (f->bitfield_width > 0 && f_size > 0) {
        size_t width_bits;
        size_t unit_bits;

        width_bits = (size_t)f->bitfield_width;
        unit_bits = f_size * 8;
        if (bit_unit_size == f_size && bits_used + width_bits <= unit_bits) {
          f->pad_before_bytes = 0;
          f->offset_bytes = bit_unit_offset;
          f->size_bytes = 0;
          f->align_bytes = eff_align;
          f->union_tail_pad_bytes = 0;
          bits_used += width_bits;
          continue;
        }
        rc = align_up_checked(cur_offset, eff_align, &aligned_off, &pad_before);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (aligned_off > max_sz - f_size)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        f->pad_before_bytes = pad_before;
        f->offset_bytes = aligned_off;
        f->size_bytes = f_size;
        f->align_bytes = eff_align;
        f->union_tail_pad_bytes = 0;
        bit_unit_size = f_size;
        bit_unit_offset = aligned_off;
        bits_used = width_bits;
        cur_offset = aligned_off + f_size;
      } else {
        bit_unit_size = 0;
        bits_used = 0;
        rc = align_up_checked(cur_offset, eff_align, &aligned_off, &pad_before);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (aligned_off > max_sz - f_size)
          return CDD_C_ERROR_INVALID_ARGUMENT;
        f->pad_before_bytes = pad_before;
        f->offset_bytes = aligned_off;
        f->size_bytes = f_size;
        f->align_bytes = eff_align;
        f->union_tail_pad_bytes = 0;
        cur_offset = aligned_off + f_size;
      }
    }

    if (node->fields_count == 0) {
      node->size_bytes = 0;
      node->align_bytes = 1;
      node->trailing_pad_bytes = 0;
    } else {
      size_t total_size;
      size_t trail_pad;
      size_t struct_align;

      struct_align = node->is_packed ? 1 : max_align;
      rc = align_up_checked(cur_offset, struct_align, &total_size, &trail_pad);
      if (rc != CDD_C_SUCCESS)
        return rc;
      node->size_bytes = total_size;
      node->align_bytes = struct_align;
      node->trailing_pad_bytes = trail_pad;
    }
    break;
  }

  case CDD_FFI_NODE_UNION: {
    size_t max_size;
    size_t max_align;

    max_size = 0;
    max_align = 1;

    for (j = 0; j < node->fields_count; j++) {
      cdd_ffi_field_t *f;
      size_t f_size;
      size_t f_align;
      size_t eff_align;

      f = &node->fields[j];
      rc = compute_type_layout(ctx, &f->type, &f_size, &f_align);
      if (rc != CDD_C_SUCCESS)
        return rc;

      eff_align = node->is_packed ? 1 : f_align;
      f->offset_bytes = 0;
      f->pad_before_bytes = 0;
      f->size_bytes = f_size;
      f->align_bytes = eff_align;
      f->union_tail_pad_bytes = 0;

      if (f_size > max_size)
        max_size = f_size;
      if (eff_align > max_align)
        max_align = eff_align;
    }

    if (node->fields_count == 0) {
      node->size_bytes = 0;
      node->align_bytes = 1;
      node->trailing_pad_bytes = 0;
    } else {
      size_t total_size;
      size_t trail_pad;
      size_t union_align;

      union_align = node->is_packed ? 1 : max_align;
      rc = align_up_checked(max_size, union_align, &total_size, &trail_pad);
      if (rc != CDD_C_SUCCESS)
        return rc;
      node->size_bytes = total_size;
      node->align_bytes = union_align;
      node->trailing_pad_bytes = trail_pad;
      for (j = 0; j < node->fields_count; j++) {
        node->fields[j].union_tail_pad_bytes =
            total_size - node->fields[j].size_bytes;
      }
    }
    break;
  }

  case CDD_FFI_NODE_FUNCTION: {
    size_t ret_sz;
    size_t ret_al;

    rc = compute_type_layout(ctx, &node->return_or_base_type, &ret_sz, &ret_al);
    if (rc != CDD_C_SUCCESS)
      return rc;
    node->size_bytes = ret_sz;
    node->align_bytes = ret_al;
    node->trailing_pad_bytes = 0;

    for (j = 0; j < node->fields_count; j++) {
      cdd_ffi_field_t *f;
      size_t f_size;
      size_t f_align;

      f = &node->fields[j];
      rc = compute_type_layout(ctx, &f->type, &f_size, &f_align);
      if (rc != CDD_C_SUCCESS)
        return rc;
      f->offset_bytes = 0;
      f->pad_before_bytes = 0;
      f->size_bytes = f_size;
      f->align_bytes = f_align;
      f->union_tail_pad_bytes = 0;
    }
    break;
  }

  case CDD_FFI_NODE_MACRO:
  default:
    node->size_bytes = 0;
    node->align_bytes = 1;
    node->trailing_pad_bytes = 0;
    break;
  }

  ctx->state[idx] = 2;
  return CDD_C_SUCCESS;
}

/*
 * @brief Computes byte offsets, inter-field padding, trailing padding, total
 * aligned byte sizes, and alignments for all structs, unions, enums, and
 * typedefs in `ir`.
 *
 * @param[in,out] ir Target FFI IR whose nodes and fields will be annotated.
 * @param[in] abi Optional target ABI profile (uses `CDD_FFI_ABI_HOST` if NULL).
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL/cycle
 * or invalid layout, or CDD_C_ERROR_NOT_FOUND on unresolved type references.
 */
NO_DISCARD cdd_c_error_t
cdd_ffi_ir_compute_layout(cdd_ffi_ir_t *ir, const cdd_ffi_abi_profile_t *abi) {
  layout_ctx_t ctx;
  size_t i;
  cdd_c_error_t rc;

  if (!ir)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(&ctx, 0, sizeof(ctx));
  ctx.ir = ir;

  rc = CDD_C_SUCCESS;
  if (!abi) {
    rc = cdd_ffi_abi_profile_init(CDD_FFI_ABI_HOST, &ctx.abi);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    ctx.abi = *abi;
    if (ctx.abi.pointer_size == 0) {
      int allow_unresolved;
      allow_unresolved = abi->allow_unresolved_refs;
      rc = cdd_ffi_abi_profile_init(abi->kind, &ctx.abi);
      if (rc != CDD_C_SUCCESS)
        return rc;
      ctx.abi.allow_unresolved_refs = allow_unresolved;
    }
    if (ctx.abi.pointer_align == 0 || ctx.abi.long_size == 0 ||
        ctx.abi.long_align == 0 || ctx.abi.enum_size == 0 ||
        ctx.abi.enum_align == 0 || ctx.abi.int64_align == 0 ||
        ctx.abi.double_align == 0) {
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }

  if (ir->nodes_count == 0)
    return rc;

  ctx.state = (int *)C_CDD_CALLOC(ir->nodes_count, sizeof(int));
  if (!ctx.state)
    return CDD_C_ERROR_MEMORY;

  for (i = 0; i < ir->nodes_count; i++) {
    if (ctx.state[i] == 0) {
      rc = compute_node_layout(&ctx, i);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(ctx.state);
        return rc;
      }
    }
  }

  C_CDD_FREE(ctx.state);
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a linked list of `cdd_ffi_trivia_t` nodes.
 *
 * @param[in,out] trivia Head of the trivia chain to free (may be NULL).
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t trivia_chain_free(cdd_ffi_trivia_t *trivia) {
  while (trivia) {
    cdd_ffi_trivia_t *next = trivia->next;
    if (trivia->text)
      C_CDD_FREE(trivia->text);
    C_CDD_FREE(trivia);
    trivia = next;
  }
  return CDD_C_SUCCESS;
}

static NO_DISCARD cdd_c_error_t field_members_free(cdd_ffi_field_t *field);

/**
 * @brief Recursively frees dynamically allocated fields of a `cdd_ffi_type_t`.
 *
 * @param[in,out] type Pointer to the FFI type structure whose members will be
 * freed.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t type_recursive_free(cdd_ffi_type_t *type) {
  cdd_c_error_t rc;

  rc = CDD_C_SUCCESS;

  if (type->ref_name) {
    C_CDD_FREE(type->ref_name);
    type->ref_name = NULL;
  }
  if (type->raw_spelling) {
    C_CDD_FREE(type->raw_spelling);
    type->raw_spelling = NULL;
  }

  rc = (cdd_c_error_t)((int)rc | (int)trivia_chain_free(type->leading_trivia));
  type->leading_trivia = NULL;

  rc = (cdd_c_error_t)((int)rc | (int)trivia_chain_free(type->trailing_trivia));
  type->trailing_trivia = NULL;

  if (type->template_args) {
    size_t i;
    for (i = 0; i < type->template_args_count; i++) {
      rc = (cdd_c_error_t)((int)rc |
                           (int)type_recursive_free(&type->template_args[i]));
    }
    C_CDD_FREE(type->template_args);
    type->template_args = NULL;
    type->template_args_count = 0;
  }

  if (type->fn_return_type) {
    rc = (cdd_c_error_t)((int)rc |
                         (int)type_recursive_free(type->fn_return_type));
    C_CDD_FREE(type->fn_return_type);
    type->fn_return_type = NULL;
  }

  if (type->fn_params) {
    size_t i;
    for (i = 0; i < type->fn_params_count; i++) {
      rc = (cdd_c_error_t)((int)rc |
                           (int)field_members_free(&type->fn_params[i]));
    }
    C_CDD_FREE(type->fn_params);
    type->fn_params = NULL;
    type->fn_params_count = 0;
  }

  return rc;
}

/**
 * @brief Frees dynamically allocated members of a single `cdd_ffi_field_t`.
 *
 * @param[in,out] field Field or parameter descriptor to clean up.
 * @return CDD_C_SUCCESS.
 */
static NO_DISCARD cdd_c_error_t field_members_free(cdd_ffi_field_t *field) {
  cdd_c_error_t rc;

  rc = CDD_C_SUCCESS;

  if (field->name) {
    C_CDD_FREE(field->name);
    field->name = NULL;
  }
  if (field->doc) {
    C_CDD_FREE(field->doc);
    field->doc = NULL;
  }
  if (field->array_length_ref) {
    C_CDD_FREE(field->array_length_ref);
    field->array_length_ref = NULL;
  }

  rc = (cdd_c_error_t)((int)rc | (int)trivia_chain_free(field->leading_trivia));
  field->leading_trivia = NULL;

  rc =
      (cdd_c_error_t)((int)rc | (int)trivia_chain_free(field->trailing_trivia));
  field->trailing_trivia = NULL;

  rc = (cdd_c_error_t)((int)rc | (int)type_recursive_free(&field->type));

  return rc;
}

/*
 * @brief Frees all memory associated with the FFI IR to guarantee zero-leaks.
 *
 * @param[in,out] ir Pointer to the FFI IR structure to free.
 * @return CDD_C_SUCCESS.
 */
NO_DISCARD cdd_c_error_t cdd_ffi_ir_free(cdd_ffi_ir_t *ir) {
  size_t i;
  size_t j;
  cdd_c_error_t rc;

  rc = CDD_C_SUCCESS;

  if (!ir) {
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];

    if (node->name) {
      C_CDD_FREE(node->name);
    }
    if (node->doc) {
      C_CDD_FREE(node->doc);
    }
    if (node->evaluated_value) {
      C_CDD_FREE(node->evaluated_value);
    }
    if (node->raw_body) {
      C_CDD_FREE(node->raw_body);
    }

    rc =
        (cdd_c_error_t)((int)rc | (int)trivia_chain_free(node->leading_trivia));

    rc = (cdd_c_error_t)((int)rc |
                         (int)trivia_chain_free(node->trailing_trivia));

    rc = (cdd_c_error_t)((int)rc |
                         (int)type_recursive_free(&node->return_or_base_type));

    if (node->fields) {
      for (j = 0; j < node->fields_count; j++) {
        rc = (cdd_c_error_t)((int)rc |
                             (int)field_members_free(&node->fields[j]));
      }
      C_CDD_FREE(node->fields);
    }

    if (node->base_classes) {
      for (j = 0; j < node->base_classes_count; j++) {
        if (node->base_classes[j].name) {
          C_CDD_FREE(node->base_classes[j].name);
        }
        if (node->base_classes[j].access) {
          C_CDD_FREE(node->base_classes[j].access);
        }
      }
      C_CDD_FREE(node->base_classes);
    }

    if (node->virtual_methods) {
      for (j = 0; j < node->virtual_methods_count; j++) {
        size_t a;
        if (node->virtual_methods[j].name) {
          C_CDD_FREE(node->virtual_methods[j].name);
        }
        rc = (cdd_c_error_t)((int)rc |
                             (int)type_recursive_free(
                                 &node->virtual_methods[j].return_type));
        if (node->virtual_methods[j].args) {
          for (a = 0; a < node->virtual_methods[j].args_count; a++) {
            rc = (cdd_c_error_t)((int)rc |
                                 (int)field_members_free(
                                     &node->virtual_methods[j].args[a]));
          }
          C_CDD_FREE(node->virtual_methods[j].args);
        }
      }
      C_CDD_FREE(node->virtual_methods);
    }

    if (node->variants) {
      for (j = 0; j < node->variants_count; j++) {
        if (node->variants[j].name) {
          C_CDD_FREE(node->variants[j].name);
        }
        if (node->variants[j].value) {
          C_CDD_FREE(node->variants[j].value);
        }
        if (node->variants[j].doc) {
          C_CDD_FREE(node->variants[j].doc);
        }
        if (node->variants[j].raw_spelling) {
          C_CDD_FREE(node->variants[j].raw_spelling);
        }
        if (node->variants[j].number_details) {
          if (node->variants[j].number_details->raw_spelling) {
            C_CDD_FREE(node->variants[j].number_details->raw_spelling);
          }
          C_CDD_FREE(node->variants[j].number_details);
        }
        rc = (cdd_c_error_t)((int)rc | (int)trivia_chain_free(
                                           node->variants[j].leading_trivia));
        rc = (cdd_c_error_t)((int)rc | (int)trivia_chain_free(
                                           node->variants[j].trailing_trivia));
      }
      C_CDD_FREE(node->variants);
    }

    if (node->macro_expansions) {
      for (j = 0; j < node->macro_expansions_count; j++) {
        if (node->macro_expansions[j].macro_name) {
          C_CDD_FREE(node->macro_expansions[j].macro_name);
        }
        if (node->macro_expansions[j].raw_args) {
          C_CDD_FREE(node->macro_expansions[j].raw_args);
        }
        if (node->macro_expansions[j].expanded_text) {
          C_CDD_FREE(node->macro_expansions[j].expanded_text);
        }
      }
      C_CDD_FREE(node->macro_expansions);
    }
  }

  if (ir->nodes) {
    C_CDD_FREE(ir->nodes);
  }
  if (ir->module_name) {
    C_CDD_FREE(ir->module_name);
    ir->module_name = NULL;
  }

  ir->nodes = NULL;
  ir->nodes_count = 0;
  ir->nodes_capacity = 0;
  return rc;
}
